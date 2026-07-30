#ifndef BONDIMAGECACHE_H
#define BONDIMAGECACHE_H

#include <QMap>
#include <QPixmap>
#include <QString>
#include "bond.h"

class BondImageCache
{
public:
    static BondImageCache& instance()
    {
        static BondImageCache cache;
        return cache;
    }

    void load(const QString& imagePath);

    /**
     * Returns the bond image base on its info.
     */
    QPixmap bondPixmap(int dx, int dy, BondType type) const;

    /**
     * Returns the connector image.
     */
    QPixmap connectorPixmap(int dx, int dy) const;

private:
    BondImageCache() {
        // empty pixmap to prevent crash
        m_empty = QPixmap(1, 1);
        m_empty.fill(Qt::transparent);
    }

    // normal bonds key: "dx_dy_type"
    QMap<QString, QPixmap> m_bonds;

    // connectors key = "dx_dy"
    QMap<QString, QPixmap> m_connectors;

    QPixmap m_empty; // empty fallback

    static QString bondKey(int dx, int dy, BondType type);
    static QString connectorKey(int dx, int dy);
    static QString bondFileName(int dx, int dy, BondType type);
    static QString connectorFileName(int dx, int dy);
};

#endif
