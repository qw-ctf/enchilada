#include "WadImageProvider.h"
#include "WadImageProvider.h"

#include <QPainter>
#include <QFile>
#include <QRegularExpression>

#include <wad.h>
#include <palette.h>

WadImageProvider::WadImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage loadTexture(const QString& filePath, dmiptex_t mip) {
    QFile file(filePath);
    file.open(QIODevice::ReadOnly);

    QDataStream in(&file);

    bool erase = false;
    for (char &name : mip.name) {
        if (name == '\0') {
            erase = true;
        } else if (erase) {
            name = '\0';
        }
    }
    QString name = QString::fromLatin1(mip.name, sizeof(mip.name));

    QVector<uint8_t> buffer(mip.width * mip.height * 4);

    file.seek(mip.offsets[0]);
    for (int i = 0; i < mip.width * mip.height; i++) {
        uint8_t p;

        in >> p;
        if (!(p == 255 && mip.name[0] == '{')) {
            buffer[i * 4] = PALETTE[p * 3];
            buffer[i * 4 + 1] = PALETTE[p * 3 + 1];
            buffer[i * 4 + 2] = PALETTE[p * 3 + 2];
            buffer[i * 4 + 3] = 255;
        }
    }

    QImage image(buffer.data(), mip.width, mip.height, QImage::Format_RGBA8888);

    if (name.startsWith("sky")) {
        QImage lowerSky = image.copy(0, 0, 128, 128);
        QImage upperSky = image.copy(128, 0, 128, 128);

        for (int y = 0; y < lowerSky.height(); ++y) {
            for (int x = 0; x < lowerSky.width(); ++x) {
                if (lowerSky.pixelColor(x, y) == Qt::black) {
                    lowerSky.setPixelColor(x, y, QColor(0, 0, 0, 0));
                }
            }
        }

        QImage upperSkyTiled(256, 256, QImage::Format_RGBA8888);

        QPainter painter(&upperSkyTiled);
        for (int y = 0; y < 2; ++y) {
            for (int x = 0; x < 2; ++x) {
                painter.drawImage(x * 128, y * 128, upperSky);
            }
        }

        QImage upperSkyScaled = upperSkyTiled.scaled(upperSkyTiled.width() / 2, upperSkyTiled.height() / 2);

        QPainter finalPainter(&upperSkyScaled);
        finalPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        finalPainter.drawImage(0, 0, lowerSky);
        finalPainter.end();

        return upperSkyScaled;
    }

    return image.scaled(QSize(128, 128), Qt::KeepAspectRatio, Qt::FastTransformation);
}

QImage appendHorizontally(const int width, const QImage& a, const QImage& b) {
    // The dimensions of the new image
    int widthA = a.width();
    int heightA = a.height();
    int newWidth = widthA + width;
    int newHeight = heightA;

    // Create a new image with the calculated dimensions
    QImage result(newWidth, newHeight, QImage::Format_ARGB32);
    result.fill(Qt::transparent);

    QPainter painter(&result);

    // Draw image A at position (0, 0)
    painter.drawImage(0, 0, a);

    // Scale image B if necessary
    QImage scaledB = b;
    if (b.width() != width || b.height() != heightA) {
        scaledB = b.scaled(width, heightA, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // Calculate position to draw scaledB
    int xPos = widthA + width / 2 - b.width() / 2; // Start drawing at the width of image A
    int yPos = (heightA - scaledB.height()) / 2; // Center image B vertically

    // Draw image B
    painter.drawImage(xPos, yPos, scaledB);

    painter.end();

    return result;
}

QImage WadImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    auto decoded = QUrl::fromPercentEncoding(id.toLatin1());
    QRegularExpression re(R"(([^?]+)[?]o=(\d+)[&]w=(\d+)[&]h=(\d+)[&]f=(.*))");
    QRegularExpressionMatch match = re.match(decoded);

    if (match.hasMatch()) {
        QString name = match.captured(1);
        QString firstNumber = match.captured(2);
        QString secondNumber = match.captured(3);
        QString thirdNumber = match.captured(4);
        QString frames = match.captured(5);

        QByteArray byteArray = name.toUtf8();
        int nameSize = qMin(byteArray.size(), 15);

        dmiptex_t mip;
        std::memcpy(mip.name, byteArray.constData(), nameSize);
        mip.name[nameSize] = '\0';

        mip.offsets[0] = firstNumber.toInt();
        mip.width = secondNumber.toInt();
        mip.height = thirdNumber.toInt();

        //QImae texture = loadTexture("C:/QuakeDev/wads/makkon_industrial/makkon_industrial.wad", mip);
        QImage texture = loadTexture("C:/QuakeDev/wads/QUAKE101.WAD", mip);

        if (!frames.isEmpty()) {
            qDebug() << frames.size() << " " << frames;
            QImage base(128, 128, QImage::Format_RGBA8888);
            base.fill(Qt::transparent);

            QPainter painter(&base);
            int x = (base.width() - texture.width()) / 2;
            int y = (base.height() - texture.height()) / 2;

            // Draw the original image centered on the new image
            painter.drawImage(x, y, texture);
            painter.end();

            texture = base;
        }

        for (auto offset : frames.split(",")) {
            if (offset.isEmpty())
                continue;
            mip.offsets[0] = offset.toInt();
            QImage nextFrame = loadTexture("C:/QuakeDev/wads/QUAKE101.WAD", mip);

            texture = appendHorizontally(128, texture, nextFrame);
        }

        size->setWidth(texture.width());
        size->setHeight(texture.height());

        return texture;
    }

    QImage image(128, 128, QImage::Format_RGBA8888);
    image.fill(Qt::blue);

    size->setWidth(image.width());
    size->setHeight(image.height());

    return image;
}