#include "TextureItemModel.h"

TextureItemModel::TextureItemModel(QObject *parent)
    : QStandardItemModel(parent) {
}

QHash<int, QByteArray> TextureItemModel::roleNames() const {
    QHash<int, QByteArray> roles = QStandardItemModel::roleNames();
    roles[ImageSourceRole] = "imageSource";
    roles[ImageTypeRole] = "imageType";
    return roles;
}