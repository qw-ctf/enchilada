#ifndef WADIMAGEPROVIDER_H
#define WADIMAGEPROVIDER_H

#include <QQuickImageProvider>

class WadImageProvider : public QQuickImageProvider
{
public:
    WadImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
};

#endif //WADIMAGEPROVIDER_H
