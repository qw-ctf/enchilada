#include "WadImageProvider.h"
#include "TextureItemModel.h"

#include <QPainter>
#include <QFile>
#include <QUrlQuery>

#include <wad.h>
#include <palette.h>
#include <QSqlQuery>
#include <QSqlError>

WadImageProvider::WadImageProvider() : QQuickImageProvider(Image) { }

QImage loadTexture(const QString& filePath, dmiptex_t mip, const QImage::Format format) {

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("WARNING: Unable to open file");
    }

    QDataStream in(&file);

    QVector<uint8_t> buffer;

    switch (format) {
        case QImage::Format_RGBA8888:
        case QImage::Format_RGBA8888_Premultiplied:
            buffer.resize(mip.width * mip.height * 4);
            break;
        default:
            buffer.resize(mip.width * mip.height * 3);
            break;
    }

    buffer.fill(0);

    if (!file.seek(mip.offsets[0])) {
        qWarning("WARNING: Could not seek to offset!");
    }

    for (int i = 0; i < mip.width * mip.height; i++) {
        uint8_t p;

        in >> p;
        if (in.status() != QDataStream::Ok) {
            qWarning("WARNING: Read failed at position %d", i);
            break;
        }

		switch (format) {
			case QImage::Format_RGBA8888:
			case QImage::Format_RGBA8888_Premultiplied:
				buffer[i * 4] = PALETTE[p * 3];
				buffer[i * 4 + 1] = PALETTE[p * 3 + 1];
				buffer[i * 4 + 2] = PALETTE[p * 3 + 2];
				buffer[i * 4 + 3] = (p == 255) ? 0 : 255;
				break;
			default:
				buffer[i * 3] = PALETTE[p * 3];
				buffer[i * 3 + 1] = PALETTE[p * 3 + 1];
				buffer[i * 3 + 2] = PALETTE[p * 3 + 2];
				break;
		}
    }

    const QImage texture(buffer.data(), static_cast<int>(mip.width), static_cast<int>(mip.height), format);
    QImage owned(static_cast<int>(mip.width), static_cast<int>(mip.height), format);
	owned.fill(Qt::transparent);

    QPainter painter(&owned);
    painter.drawImage(0, 0, texture);
    painter.end();

    return std::move(owned);
}

QImage WadImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize) {

    const QUrl decoded = QUrl::fromPercentEncoding(id.toLatin1());

    auto urlQuery = QUrlQuery(decoded);
    urlQuery.setQueryDelimiters('?', '&');
	const int textureId = decoded.fileName().toInt();
	QSqlQuery query;
	query.prepare("SELECT path, offset, width, height, category FROM texture INNER JOIN wad ON wad.wad_id = texture.wad_id WHERE texture_id = :texture_id");
	query.bindValue(":texture_id", textureId);
	if (!query.exec()) {
		qCritical() << "query failed" << query.lastError();
	}
	query.next();

	const QString fileName = query.value(0).toString();
	const auto imageType = query.value(4).toInt();

	const auto bounds = QSize{
        requestedSize.width() <= 0 ? 128 : requestedSize.width(),
        requestedSize.height() <= 0 ? 128 : requestedSize.height()
    };

	QStringList offsetsStr;
	offsetsStr.append(query.value(1).toString());

	if (query.value(1).isNull() || query.value(2).isNull() || query.value(3).isNull()) {
        QImage image(bounds, QImage::Format_RGBA8888_Premultiplied);
        image.fill(Qt::blue);
        size->setWidth(image.width());
        size->setHeight(image.height());
        return image;
    }

    dmiptex_t mip = {};

    mip.offsets[0] = query.value(1).toInt();
    mip.width = query.value(2).toInt();
    mip.height = query.value(3).toInt();

    if (urlQuery.hasQueryItem("s")) {
        auto texture = loadTexture(fileName, mip, QImage::Format_RGBA8888);

        // Make black pixels in cloud part of the sky transparent
        for (int x = 0; x < 128; ++x) {
            for (int y = 0; y < 128; ++y) {
                if (texture.pixelColor(x, y) == Qt::black) {
                    QColor skyColor = texture.pixelColor(x + 128, y);
                    skyColor.setAlpha(0);
                    texture.setPixelColor(x, y, skyColor);
                }
            }
        }

        QImage sky(bounds, QImage::Format_RGBA8888);
        sky.fill(Qt::transparent);

        QPainter painter(&sky);
        if (urlQuery.queryItemValue("s") == "clouds") {
            painter.drawImage(0, 0, texture, 0, 0, 128, 128);
        } else {
            painter.drawImage(0, 0, texture, 128, 0, 128, 128);
        }
        painter.end();

        size->setWidth(bounds.width());
        size->setHeight(bounds.height());

        return sky;
    }

    if (offsetsStr.size() > 1) {
        QVector<int> offsets;

        std::transform(offsetsStr.begin(), offsetsStr.end(), std::back_inserter(offsets),
                       [](const QString& frame) { return frame.toInt(); });

        QImage anim(bounds.width() * offsets.size(), bounds.height(), QImage::Format_RGBA8888_Premultiplied);
        anim.fill(Qt::transparent);

        const auto firstFrame = loadTexture(fileName, mip, QImage::Format_RGBA8888_Premultiplied)
                .scaled(bounds, Qt::KeepAspectRatio, Qt::FastTransformation);

        const auto xOffset = (bounds.width() - firstFrame.width()) / 2;
        const auto yOffset = (bounds.height() - firstFrame.height()) / 2;

        QPainter painter(&anim);
        painter.drawImage(xOffset, yOffset, firstFrame);

        for (int i = 0; i < offsets.size(); i++) {
            mip.offsets[0] = offsets[i];
            const auto nextFrame = loadTexture(fileName, mip, QImage::Format_RGBA8888_Premultiplied)
                    .scaled(bounds, Qt::KeepAspectRatio, Qt::FastTransformation);
            painter.drawImage(i * bounds.width() + xOffset, yOffset, nextFrame);
        }

        painter.end();

        size->setWidth(anim.width());
        size->setHeight(anim.height());

        return anim;
    }


	auto format = imageType == TextureItemModel::TextureCategory::Fence ? QImage::Format_RGBA8888 : QImage::Format_RGB888;

	auto texture = loadTexture(fileName, mip, format)
			.scaled(bounds, Qt::KeepAspectRatio, Qt::FastTransformation);
    size->setWidth(texture.width());
    size->setHeight(texture.height());

	return texture;
}