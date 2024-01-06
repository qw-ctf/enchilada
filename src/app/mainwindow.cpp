#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <palette.h>

#include <QFile>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QPainter>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
      , ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

void MainWindow::loadTextures(QVector<dmiptex_t> textures, const QString& filePath) {
    QFile file(filePath);
    file.open(QIODevice::ReadOnly);

    QDataStream in(&file);

    int i = 0;

    auto* model = new QStandardItemModel(this);

    for (auto& mip: textures) {
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
            buffer[i * 4] = PALETTE[p * 3];
            buffer[i * 4 + 1] = PALETTE[p * 3 + 1];
            buffer[i * 4 + 2] = PALETTE[p * 3 + 2];
            buffer[i * 4 + 3] = p == 255 && mip.name[0] == '{' ? 0 : 255;
        }

        QImage image(buffer.data(), mip.width, mip.height, QImage::Format_RGBA8888);

        QIcon icon;

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
            icon.addPixmap(QPixmap::fromImage(upperSkyScaled));
        } else {
            QImage scaled = image.scaled(QSize(128, 128), Qt::KeepAspectRatio, Qt::FastTransformation);
            icon.addPixmap(QPixmap::fromImage(scaled));
        }

        auto* item = new QStandardItem(icon, name);
        model->appendRow(item);
    }

    auto proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(0);

    ui->listTextureItems->setModel(proxyModel);

    connect(ui->lineEdit, &QLineEdit::textChanged, proxyModel, &QSortFilterProxyModel::setFilterWildcard);
}

MainWindow::~MainWindow() {
    delete ui;
}
