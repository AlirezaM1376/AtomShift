#ifndef ATOMIMAGECACHE_H
#define ATOMIMAGECACHE_H

#include <QMap>
#include <QPixmap>
#include <QString>

class AtomImageCache
{
public:
    static AtomImageCache& instance()
    {
        static AtomImageCache cache;
        return cache;
    }

    void load(const QString& imagePath);

    QPixmap atomPixmap(char elementSymbol) const;

private:
    AtomImageCache() {
        // empty pixmap to prevent crash
        m_empty = QPixmap(1, 1);
        m_empty.fill(Qt::transparent);
    }

    QMap<QChar, QPixmap> m_atoms;

    QPixmap m_empty; //

    static QString atomFileName(char elementSymbol);
};

#endif // ATOMIMAGECACHE_H
