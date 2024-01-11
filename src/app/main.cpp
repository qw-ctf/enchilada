#include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSortFilterProxyModel>
#include <QSGRendererInterface>

#include <QFile>
#include <QDataStream>
#include <QDebug>

#include <wad.h>

#include "FilterProxyModel.h"
#include "TextureItemModel.h"
#include "WadImageProvider.h"

QVector<dmiptex_t> parseWadFile(const QString& filePath) {
    QVector<dmiptex_t> textures;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Error: Cannot open file" << filePath;
        return textures;
    }

    QDataStream in(&file);

    in.setByteOrder(QDataStream::LittleEndian);

    wadhead_t wadHeader = {0};
    in.readRawData(wadHeader.magic, 4);
    in >> wadHeader.numentries;
    in >> wadHeader.diroffset;

    if (strncmp(wadHeader.magic, "WAD2", 4) != 0) {
        qDebug() << "Error: Invalid WAD file";
        file.close();
        return textures;
    }

    qDebug() << "numentries: " << wadHeader.numentries;

    if (!file.seek(wadHeader.diroffset)) {
        qDebug() << "Failed to seek to diroffset: " << wadHeader.diroffset;
        return textures;
    }

    QVector<wadentry_t> entries(wadHeader.numentries);
    for (int i = 0; i < wadHeader.numentries; ++i) {
        wadentry_t& entry = entries[i];
        in >> entry.offset;
        in >> entry.dsize;
        in >> entry.size;
        in >> entry.type;
        in >> entry.cmprs;
        in >> entry.dummy;
        in.readRawData(entry.name, 16);
    }

    for (const wadentry_t& entry: entries) {
        file.seek(entry.offset);

        switch (entry.type) {
            case '@': // Color Palette
                break;
            case 'B': // Pictures for status bar
                break;
            case 'D': // Mip Texture
                dmiptex_t mip;

                in.readRawData(mip.name, 16);
                in >> mip.width;
                in >> mip.height;
                for (int i = 0; i < 4; i++) {
                    in >> mip.offsets[i];
                    mip.offsets[i] += entry.offset;
                } {
                    bool erase = false;
                    for (char& name: mip.name) {
                        if (name == '\0') {
                            erase = true;
                        } else if (erase) {
                            name = '\0';
                        }
                    }
                }
                if (mip.offsets[0] > 0 && mip.width > 0 && mip.height > 0) {
                    textures.push_back(mip);
                } else {
                    qDebug() << "BAD OFFSET: Texture width: " << mip.width << " height: " << mip.height;
                }
                break;
            case 'E': // Console picture (flat)
                break;
            default:
                qDebug() << "Unknown entry type" << entry.type;
                break;
        }
    }

    file.close();

    return textures;
}

TextureItemModel::TextureType textureType(const char* name) {
    switch (name[0]) {
        case '+':
            return TextureItemModel::Animated;
        case '{':
            return TextureItemModel::Fence;
        case '*':
            return TextureItemModel::Liquid;
        default:
            if (!qstrncmp(name, "sky", 3))
                return TextureItemModel::Sky;
            return TextureItemModel::Normal;
    }
}

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    /*
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("mydatabase.db");
    db.close();
    */

    /*
    const auto start = std::chrono::high_resolution_clock::now();
    // do stuff
    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double, std::milli> duration = end - start;
    */

    const auto textures = parseWadFile(TEST_WAD);

    qmlRegisterUncreatableMetaObject(
        TextureItemModel::staticMetaObject,
        "enchilada.TextureItemModel",
        1, 0,
        "TextureItemModel",
        "Error: only enums"
    );

    auto* model = new TextureItemModel();

    for (auto& mip: textures) {
        QString name = QString::fromLatin1(mip.name, qMin(std::strlen(mip.name), sizeof(mip.name)));
        QStringList offsets;

        if (mip.name[0] == '+' && (mip.name[1] != '0' || mip.name[2] == '\0')) {
            continue;
        }

        offsets.append(QString::number(mip.offsets[0]));

        if (mip.name[0] == '+') {
            for (auto& f: textures) {
                if (f.name[0] == '+' && f.name[1] != '0' && !qstrncmp(mip.name + 2, f.name + 2, sizeof(mip.name) - 2)) {
                    offsets.append(QString::number(f.offsets[0]));
                }
            }
        }

        QUrl source = QString("image://wad/%1?o=%2&w=%3&h=%4&f=%5")
                .arg(name)
                .arg(offsets.join(","))
                .arg(mip.width)
                .arg(mip.height);

        auto *item = new QStandardItem();
        item->setText(name);
        item->setData(textureType(mip.name), TextureItemModel::RoleNames::ImageTypeRole);
        item->setData(source, TextureItemModel::RoleNames::ImageSourceRole);
        model->appendRow(item);
    }

    const auto proxyModel = new FilterProxyModel(&app);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("wad"), new WadImageProvider);
    engine.setInitialProperties({
        {"listViewModel", QVariant::fromValue(proxyModel)}
    });
    engine.load(QUrl(u"qrc:/qt/qml/enchilada/App.qml"_qs));

    return app.exec();
}
