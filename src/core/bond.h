#ifndef BOND_H
#define BOND_H

#include <QString>
#include <QVector>
#include <QChar>

/**
 * type of the bonds (single/double/triple)
 */
enum BondType
{
    BondNone   = 0,
    BondSingle = 1,
    BondDouble = 2,
    BondTriple = 3
};

/**
 * Direction (dx,dy) and type of the bond, relative to the atom that own the bond
 */
struct BondInfo
{
    int dx;
    int dy;
    BondType type;
};

/**
 * Decoding one char from the bond string ('a', 'c', 'A', ...)
 * to the bond type and direction.
 *
 * @param c: bond char
 * @param out: bond type and direction, if c is valid
 * @return true if c is valid, otherwise false
 */
inline bool decodeBond(char c, BondInfo &out)
{
    switch(c)
    {
    // ---- single bonds (8 dirs) ----
    case 'a': out.dx =  0; out.dy = -1; out.type = BondSingle; return true; // Top
    case 'b': out.dx =  1; out.dy = -1; out.type = BondSingle; return true; // Top right
    case 'c': out.dx =  1; out.dy =  0; out.type = BondSingle; return true; // Right
    case 'd': out.dx =  1; out.dy =  1; out.type = BondSingle; return true; // Bottom right
    case 'e': out.dx =  0; out.dy =  1; out.type = BondSingle; return true; // Bottom
    case 'f': out.dx = -1; out.dy =  1; out.type = BondSingle; return true; // Bottom left
    case 'g': out.dx = -1; out.dy =  0; out.type = BondSingle; return true; // Left
    case 'h': out.dx = -1; out.dy = -1; out.type = BondSingle; return true; // Top left

    // ---- double bonds (4 dirs) ----
    case 'A': out.dx =  0; out.dy = -1; out.type = BondDouble; return true; // Top
    case 'B': out.dx =  1; out.dy =  0; out.type = BondDouble; return true; // Right
    case 'C': out.dx =  0; out.dy =  1; out.type = BondDouble; return true; // Bottom
    case 'D': out.dx = -1; out.dy =  0; out.type = BondDouble; return true; // Left

    // ---- triple bonds (4 dirs) ----
    case 'E': out.dx =  0; out.dy = -1; out.type = BondTriple; return true; // Top
    case 'F': out.dx =  1; out.dy =  0; out.type = BondTriple; return true; // Right
    case 'G': out.dx =  0; out.dy =  1; out.type = BondTriple; return true; // Bottom
    case 'H': out.dx = -1; out.dy =  0; out.type = BondTriple; return true; // Left

    default:
        return false; // Unknown char or no bonds
    }
}

/**
 * Decoding the full bond string (for example "acg")
 * to a list of BondInfo.
 *
 * @param bonds: the bonds string of an atom (each char is a bond)
 * @return list of decoded bonds.
 */
inline QVector<BondInfo> decodeAllBonds(const QString& bonds)
{
    QVector<BondInfo> result;
    result.reserve(bonds.size());

    for (int i = 0; i < bonds.size(); i++)
    {
        BondInfo info;
        if (decodeBond(bonds.at(i).toLatin1(), info))
            result.append(info);
    }
    return result;
}

/**
 * This is the main part of the bonds logic that checks if a bond between 2 atoms created correctly.
 * It checks a list of decoded bonds to see if there is a bond from the same type but in reverse direction.
 *
 * @param neighborInfos: Decoded bonds of the neighbor
 * @param dx: X direction of the bond from the current atom
 * @param dy: Y direction of the bond from the current atom
 * @param type: type of the bond of the current atom
 * @return true: if bond from the current atom is satisfied by the bond of the neighbor
*/
inline bool hasReciprocalBond(
        const QVector<BondInfo>& neighborInfos,
        int dx,
        int dy,
        BondType type)
{
    for (int j = 0; j < neighborInfos.size(); j++)
    {
        const BondInfo& nb = neighborInfos.at(j);
        if (nb.dx == -dx && nb.dy == -dy && nb.type == type)
            return true;
    }
    return false;
}

/**
 * Detect if an atomCode is a real atom or just a connector.
 *
 * If an atomeCode is one of 'A', 'B', 'C' and 'D' characters, it's not an atom;
 * instead it's a connector between 2 atoms. Connectors have no bonds.
 *
 * Connectors act like atoms on the board, can be selcted and moved.
 * The only difference is that connectors have no bonds,
 * hence to check if a bond is created between the connector an the neighbor atoms,
 * we use a simple exception in bondSatisfiedByNeighbor.
 */
inline bool isConnectorCode(char atomCode)
{
    return atomCode == 'A' || atomCode == 'B'
        || atomCode == 'C' || atomCode == 'D';
}

/**
 * Checks if the bond (dx,dy,type) from the current atom satisfied by the neighbor
 *
 * @param neighborAtomCode: atomCode of the neighbor
 * @param neighborBonds: bonds string of the neighbor
 * @param dx: X direction of the bond from the current atom
 * @param dy: Y direction of the bond from the current atom
 * @param type: type of the bond of the current atom
 * @return true: if bond is satisfied by the neighbor
 */
inline bool bondSatisfiedByNeighbor(
        char neighborAtomCode,
        const QString& neighborBonds,
        int dx,
        int dy,
        BondType type)
{
    if (isConnectorCode(neighborAtomCode))
        return true; // connectors are kind of a bond themselves

    QVector<BondInfo> neighborInfos = decodeAllBonds(neighborBonds);
    return hasReciprocalBond(neighborInfos, dx, dy, type);
}

#endif // BOND_H
