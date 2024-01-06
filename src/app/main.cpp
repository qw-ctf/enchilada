#include "mainwindow.h"

#include <QApplication>
#include <QtSql/QSqlDatabase>

#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <wad.h>

QVector<dmiptex_t> parseWadFile(const QString& filePath) {
    QVector<dmiptex_t> textures;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Error: Cannot open file" << filePath;
        return textures;
    }

    QDataStream in(&file);

    // Set the correct endian mode for reading
    in.setByteOrder(QDataStream::LittleEndian);

    // Read WAD header
    wadhead_t wadHeader = { 0 };
    in.readRawData(wadHeader.magic, 4);
    in >> wadHeader.numentries;
    in >> wadHeader.diroffset;

    // Check the magic number
    if (strncmp(wadHeader.magic, "WAD2", 4) != 0) {
        qDebug() << "Error: Invalid WAD file";
        file.close();
        return textures;
    }

    qDebug() << "numentries: " << wadHeader.numentries;

    // Move to the directory offset
    if (!file.seek(wadHeader.diroffset)) {
        qDebug() << "Failed to seek to diroffset: " << wadHeader.diroffset;
        return textures;
    }

    // Read directory entries
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

    for (const wadentry_t& entry : entries) {
        // Seek to the entry's offset
        file.seek(entry.offset);

        // Depending on the entry type, process the data
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
            }
            if (mip.name[0] == '\0') {
                if (entry.name[0] == '\0') {
                }
            }
            if (mip.offsets[0] > 0 && mip.width > 0 && mip.height > 0) {
                // qDebug() << "Texture width: " << mip.width << " height: " << mip.height;
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


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("mydatabase.db");
    db.close();

    //auto textures = parseWadFile("C:/QuakeDev/wads/makkon_industrial/makkon_industrial.wad");
    //w.loadTextures(textures, "C:/QuakeDev/wads/makkon_industrial/makkon_industrial.wad");

    const auto textures = parseWadFile("C:/QuakeDev/wads/QUAKE101.WAD");
    w.loadTextures(textures, "C:/QuakeDev/wads/QUAKE101.WAD");

    //const auto textures = parseWadFile("C:/QuakeDev/wads/STARFIELD.WAD");
    //w.loadTextures(textures, "C:/QuakeDev/wads/STARFIELD.WAD");

    w.show();

    return a.exec();
}
