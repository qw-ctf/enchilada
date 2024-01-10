#ifndef WADIMAGEPROVIDER_H
#define WADIMAGEPROVIDER_H

#include <QQuickImageProvider>

#define TEST_WAD "C:/QuakeDev/wads/QUAKE101.WAD"
// #define TEST_WAD "C:/QuakeDev/wads/makkon_industrial/makkon_industrial.wad"

class WadImageProvider : public QQuickImageProvider
{
public:
    WadImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
};

#endif //WADIMAGEPROVIDER_H
