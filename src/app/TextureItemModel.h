#ifndef TEXTUREITEMMODEL_H
#define TEXTUREITEMMODEL_H

#include <QStandardItemModel>

class TextureItemModel : public QStandardItemModel {
    Q_OBJECT
public:
    explicit TextureItemModel(QObject *parent = nullptr);

    enum TextureType {
        Normal,
        Fence,
        Animated,
        Liquid,
        Sky,
    };
    Q_ENUM(TextureType)

    enum RoleNames {
        ImageSourceRole = Qt::UserRole + 1,
        ImageTypeRole = Qt::UserRole + 2
    };

protected:
    QHash<int, QByteArray> roleNames() const override;
};

#endif