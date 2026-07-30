#include "atomimagecache.h"
#include <QDebug>

QString AtomImageCache::atomFileName(char elementSymbol)
{
    switch(elementSymbol){
        case 'H':
            return QString("atom-h");
        break;
        case 'C':
            return QString("atom-c");
        break;
        case 'O':
            return QString("atom-o");
        break;
        case 'N':
            return QString("atom-n");
        break;
        case 'S':
            return QString("atom-s");
        break;
        case 'F':
            return QString("atom-f");
        break;
        case 'L':
            return QString("atom-cl");
        break;
        case 'B':
            return QString("atom-br");
        break;
        case 'P':
            return QString("atom-p");
        break;
        case 'X':
            return QString("atom-crystal");
        break;
        case 'A':
            return QString("connector-horizontal");
        break;
        case 'D':
            return QString("connector-backslash");
        break;
        case '0':
            return QString("connector-vertical");
        break;
        case '1':
            return QString("crystal-E");
        break;
        case '2':
            return QString("crystal-F");
        break;
        case '3':
            return QString("crystal-G");
        break;
        case '4':
            return QString("crystal-H");
        break;
        case '5':
            return QString("crystal-J");
        break;
        case '6':
            return QString("crystal-K");
        break;
        case '7':
            return QString("crystal-L");
        break;
        case '8':
            return QString("crystal-M");
        break;
        case '9':
            return QString("connector-slash");
        break;
        default:
            return QString();
    }
}

void AtomImageCache::load(const QString& imagePath)
{
    char elementsList[22] = {'H', 'O', 'C', 'F', 'X', 'P', 'B', 'L', 'S', 'N', 'A', 'D', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

    for (int i=0; i<22; i++){
        QString fileName = atomFileName(elementsList[i]);
        if(fileName.isEmpty()) continue;

        QString fullPath = QString("%1/%2.png").arg(imagePath).arg(fileName);
        QPixmap px(fullPath);
        if(px.isNull())
            qDebug() << "AtomImageCache: not found:" << fullPath;
        else
            m_atoms[elementsList[i]] = px;
    }
}

QPixmap AtomImageCache::atomPixmap(char elementSymbol) const
{
    QChar key = elementSymbol;
    if(m_atoms.contains(key))
        return m_atoms[key];
    return QPixmap(); // null pixmap
}
