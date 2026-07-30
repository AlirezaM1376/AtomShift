#include "bondimagecache.h"
#include <QDebug>

QString BondImageCache::bondKey(int dx, int dy, BondType type)
{
    return QString("%1_%2_%3").arg(dx).arg(dy).arg((int)type);
}

QString BondImageCache::connectorKey(int dx, int dy)
{
    return QString("%1_%2").arg(dx).arg(dy);
}

QString BondImageCache::bondFileName(int dx, int dy, BondType type)
{
    QString suffix;
    if(type == BondDouble) suffix = "-double";
    else if(type == BondTriple) suffix = "-triple";

    if(dx == 0  && dy == -1) return QString("bond-top%1").arg(suffix);
    if(dx == 0  && dy ==  1) return QString("bond-bottom%1").arg(suffix);
    if(dx == -1 && dy ==  0) return QString("bond-left%1").arg(suffix);
    if(dx ==  1 && dy ==  0) return QString("bond-right%1").arg(suffix);

    if(dx ==  1 && dy == -1) return QString("bond-top-right");
    if(dx ==  1 && dy ==  1) return QString("bond-bottom-right");
    if(dx == -1 && dy ==  1) return QString("bond-bottom-left");
    if(dx == -1 && dy == -1) return QString("bond-top-left");

    return QString();
}

QString BondImageCache::connectorFileName(int dx, int dy)
{
    if(dx == 0  && dy != 0) return "connector-vertical";
    if(dx != 0  && dy == 0) return "connector-horizontal";
    if(dx ==  1 && dy ==  1) return "connector-backslash";
    if(dx == -1 && dy == -1) return "connector-backslash";
    if(dx ==  1 && dy == -1) return "connector-slash";
    if(dx == -1 && dy ==  1) return "connector-slash";

    return QString();
}

void BondImageCache::load(const QString& imagePath)
{
    struct DirEntry { int dx; int dy; };
    DirEntry straightDirs[] = {
        { 0,-1}, { 0, 1}, {-1, 0}, { 1, 0}
    };
    BondType types[] = { BondSingle, BondDouble, BondTriple };

    for(int d = 0; d < 4; d++)
    {
        for(int t = 0; t < 3; t++)
        {
            int dx = straightDirs[d].dx;
            int dy = straightDirs[d].dy;
            BondType type = types[t];

            QString fileName = bondFileName(dx, dy, type);
            if(fileName.isEmpty()) continue;

            QString fullPath = QString("%1/%2.png").arg(imagePath).arg(fileName);
            QPixmap px(fullPath);
            if(px.isNull())
                qDebug() << "BondImageCache: not found:" << fullPath;
            else
                m_bonds[bondKey(dx, dy, type)] = px;
        }
    }

    DirEntry diagDirs[] = {
        { 1,-1}, { 1, 1}, {-1, 1}, {-1,-1}
    };
    for(int d = 0; d < 4; d++)
    {
        int dx = diagDirs[d].dx;
        int dy = diagDirs[d].dy;
        QString fileName = bondFileName(dx, dy, BondSingle);
        if(fileName.isEmpty()) continue;

        QString fullPath = QString("%1/%2.png").arg(imagePath).arg(fileName);
        QPixmap px(fullPath);
        if(px.isNull())
            qDebug() << "BondImageCache: not found:" << fullPath;
        else
            m_bonds[bondKey(dx, dy, BondSingle)] = px;
    }

    // ---- connector ----
    DirEntry connDirs[] = {
        { 0, 1}, { 1, 0}, { 1, 1}, { 1,-1}
    };
    for(int d = 0; d < 4; d++)
    {
        int dx = connDirs[d].dx;
        int dy = connDirs[d].dy;
        QString fileName = connectorFileName(dx, dy);
        if(fileName.isEmpty()) continue;

        QString fullPath = QString("%1/%2.png").arg(imagePath).arg(fileName);
        QPixmap px(fullPath);
        if(px.isNull())
            qDebug() << "BondImageCache: not found:" << fullPath;
        else
            m_connectors[connectorKey(dx, dy)] = px;
    }
}

QPixmap BondImageCache::bondPixmap(int dx, int dy, BondType type) const
{
    QString key = bondKey(dx, dy, type);
    if(m_bonds.contains(key))
        return m_bonds[key];
    return QPixmap(); // null pixmap
}

QPixmap BondImageCache::connectorPixmap(int dx, int dy) const
{
    QString key = connectorKey(dx, dy);
    if(m_connectors.contains(key))
        return m_connectors[key];

    key = connectorKey(-dx, -dy);
    if(m_connectors.contains(key))
        return m_connectors[key];

    return QPixmap(); // null pixmap
}
