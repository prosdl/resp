// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : hash.h
//                       Header zu hash.cpp.
//
//  Anfang des Projekts: Di, 4.September, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: hash.h,v 1.29 2003/05/31 19:25:50 rosendahl Exp $
// Copyright (C) 2001 Peter Rosendahl
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// Email: p_rosendahl@t-online.de
// -------------------------------------------------------------------------


#ifndef HASH_H
#define HASH_H

#include "basic_stuff.h"
#include "respoptions.h"
#include "move.h"



const int MAX_REHASH = 10;

// Spezieller Hashwert für nullmoves
const long NULLMOVE_HASHVALUE = 0xCCAFFECC;

// -------------------------------------------------------------------------
//                      c l a s s   T T
// -------------------------------------------------------------------------

// Liefert Informationen zu einem Knoten im Suchbaum
class TTe
{
public:
    enum score_type {
        DONT_USE = 0, EXACT=1, LBOUND=2, UBOUND=3
    } ;
public:
    hash_t      lock;   // ID für die Position
    int         score;  // Score    
    Move        m;      // Bester Zug in der Position

    // -----------------------------------------------------------------
    // special:  3      2        1    1
    //           2      5        7    3   9        1                
    //           aaaaaaaa dddddddd ddddtttt scccceee 
    // mit:
    //      e:  ep Spalte
    //      c:  Rochaderechte
    //      s:  am Zug befindliche Farbe (1 für weiss)
    //      t:  Art der gespeicherten Information (score_type)
    //      d:  Tiefe des gespeicherten Teilbaums
    //      a:  Alter des Eintrags
    // -----------------------------------------------------------------

    UINTEGER32  special;
public:
    TTe() 
    {  
        lock = score = special= 0;
        m = 0;
    }

    TTe(hash_t _lock, int _score, UINTEGER32 _special, Move _m)
    {
        lock    = _lock;
        score   = _score;
        m       = _m;
        special = _special;
    }

    TTe(hash_t _lock, int depth, int side, int castle, int ep)
    {
        lock = _lock;
        special = COL(ep) | (castle << 3) | (depth << 12);

        if (side==WHITE)
            special |= 0x80;
    }

    // ---------------------------------------------------------
    //  Zugriffsfunktionen
    // ---------------------------------------------------------
public:
    int getDepth() const
    {
        return (special >> 12) & 0xfff;
    }

    int getType() const
    {
        return (special >>  8) & 0xf;
    }

    void setType(const int type)
    {
        special = (special & 0xfffff0ff) | (type << 8) ;
    }

    int getAge() const
    {
        return (special >> 24) & 0xff;
    }

    int getInfoBits() const
    {
        return special & 0xff;
    }

    void setSpecial(int depth, int side, int castle, int ep, int age, int type) 
    {
        special = COL(ep) | (castle << 3) | (depth << 12) | (age << 24) | (type << 8);

        if (side==WHITE)
            special |= 0x80;
    }
};

// -------------------------------------------------------------------------
//                      c l a s s   H a s h
// -------------------------------------------------------------------------

class Hash
{
    // ----- private Eigenschaften -----------------------------
private:
    TTe* pTable;     // Der eigentliche Hashtable

    long   size;     // Größe des Hashtable (2'er Potenz) 
    hash_t x_mask;   // Maske mit dem Wert size-1 

    // fürs Zobrist Hashing benötigte Strukturen
    hash_t z1_white_pieces[7][64];
    hash_t z1_black_pieces[7][64];
    hash_t z2_white_pieces[7][64];
    hash_t z2_black_pieces[7][64];

//    hash_t z_rehash[MAX_REHASH];

    int maxRehash;

    // Statistik
public:
    long nFound[MAX_REHASH];
    long nInsertFree;
    long nInsertDepth;
    long nAge;
    long nInsertUnsafe;
    long nFailedRehashs;


    // -------- private Methoden -------------------------------
private:
    // Initialisierung der Strukturen fürs Zobrist Hashing
    void init();

    Hash(long max_size_in_bytes=20000000, int maxReh = 4);
    ~Hash();

    static Hash* instance;

    // ------ öffentliche Memberfunktionen ----------------------
public:
    static Hash* getHandle() {
       if (instance == NULL) {
          instance = new Hash(
            RespOptions::getHandle()->getMBValueAsLong("hash.size"));
       }
       return instance;
    }

    // Einfügen der Positonsdaten info mit Hashwert z_hash in die
    // Hashtabelle; root_age beschreibt das Alter des Wurzelknoten
    // bei einer Suche
    bool insert(TTe& info, hash_t z1_hash, int root_age);

    // Aufsuchen einer Position info mit Hashwert z_hash in der Hashtabelle;
    // zurückgegeben wird true, falls was gefunden wurde
    bool lookup(TTe& info, hash_t z1_hash);

    // Berechnen des Hashwerts für ein komplettes Brett
    hash_t z1_value(BYTE piece[64], BYTE color[64]);
    hash_t z2_value(BYTE piece[64], BYTE color[64]);

    // Aktualisierung von Hashwerten, wenn Zug m gemacht wird:

    void update_z1val(hash_t& z1val, const Move& m, BYTE side);
    void update_z2val(hash_t& z2val, const Move& m, BYTE side);



    hash_t z1XorWhite(int piece, int pos) const
    {
        return z1_white_pieces[piece][pos];
    }
    
    hash_t z1XorBlack(int piece, int pos) const
    {
        return z1_black_pieces[piece][pos];
    }

    hash_t z2XorWhite(int piece, int pos) const
    {
        return z2_white_pieces[piece][pos];
    }

    hash_t z2XorBlack(int piece, int pos) const
    {
        return z2_black_pieces[piece][pos];
    }


    // Reinitialisieren des Hash
    void clear();

    long getSize() const { return size * sizeof(TTe); }
    long getMaxEntries() const { return size;  }
    int getMaxRehash() const { return maxRehash; }
};


#endif // HASH_H
