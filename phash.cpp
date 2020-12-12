// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : phash.cpp
//                       Implementierung Pawn-Hash.
//
//  Anfang des Projekts: Sa 3.November, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: phash.cpp,v 1.17 2003/05/31 19:25:50 rosendahl Exp $
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

#include "phash.h"
#include "respmath.h"   

#include <iostream>
using namespace std;




// -------------------------------------------------------------------------
//                          Definitionen für PHash
// -------------------------------------------------------------------------

PHash* PHash::instance = NULL;

// -------------------------------------------------------------------------
//  Initialisierungen
// -------------------------------------------------------------------------
void PHash::init()
{
    for (int i=0; i < 64; i++)
    {
        zp1_white_pawns[i] = RespMath::z_random();
        zp1_black_pawns[i] = RespMath::z_random();
        zp2_white_pawns[i] = RespMath::z_random();
        zp2_black_pawns[i] = RespMath::z_random();
    }
}


// -------------------------------------------------------------------------
//  Konstruktor
// -------------------------------------------------------------------------
PHash::PHash(long max_size_in_bytes)
{
    long two_pow_x;

    // two_pow_x bestimmen, so dass two_pow_x * sizeof(TTe) <= max_size_in_bytes, x minimal

    two_pow_x = 1024 ;   // minimale Groesse 1 x sizeof(TTe) kB;

    while (two_pow_x * static_cast<long>(sizeof(PTTe)) <= max_size_in_bytes)
    {
        two_pow_x <<= 1;
    }
    two_pow_x >>= 1;

    if (two_pow_x < 1024)
        two_pow_x = 1024;


    this->size = two_pow_x;

    x_mask = this->size - 1;

    // Speicherplatz für Hashtable reservieren
    cout << "Allocating  "<< two_pow_x*sizeof(PTTe) / 1024 << " kBytes for pawn hash ... ";
    pTable = new PTTe[this->size];
    cout << "OK" << endl;


    // Initialisierung der Strukturen fürs Zobrisk Hashing
    init();

}

// -------------------------------------------------------------------------
//  Destruktor
// -------------------------------------------------------------------------
PHash::~PHash()
{
    delete[] pTable;
}

// -------------------------------------------------------------------------
//  Berechnen des hashwerts ausgehend von den Bauern auf (piece, color)
// -------------------------------------------------------------------------
hash_t PHash::zp1_value(BYTE piece[64], BYTE color[64])
{
    int i;
    hash_t val = 0;


    for (i=0; i<64; i++)
    {
        if (piece[i] == PAWN)
        {
            if (color[i] == WHITE)
                val ^= zp1_white_pawns[ i ];
            else
                val ^= zp1_black_pawns[ i ];
        }
    }

    return val;
}

hash_t PHash::zp2_value(BYTE piece[64], BYTE color[64])
{
    int i;
    hash_t val = 0;


    for (i=0; i<64; i++)
    {
        if (piece[i] == PAWN)
        {
            if (color[i] == WHITE)
                val ^= zp2_white_pawns[ i ];
            else
                val ^= zp2_black_pawns[ i ];
        }
    }

    return val;
}


// -------------------------------------------------------------------------
//  Den kompletten Hash so reinitialisieren, dass er bei einem neuen
//  Spiel nichts blockiert
// -------------------------------------------------------------------------
void PHash::clear()
{
    cout << "Clearing PHash ... ";
    PTTe* q = pTable;

    for (long h = 0; h < size; h++,q++)
    {
         q->lock =  0;
    }

    nFound          = 0;
    nInsert         = 0;
    nNotFound       = 0;

    cout << "OK." << endl;
}


// -------------------------------------------------------------------------
//  Einfügen einer Position in den Hash
// -------------------------------------------------------------------------

PTTe* PHash::insert(const PTTe& pawn_info, hash_t key)
{
    PTTe* p = pTable + (key & x_mask); 

#ifdef PHASH_STATISTIC
        nInsert++;
#endif

    *p = pawn_info; 

    return p;

}


// -------------------------------------------------------------------------
//  Aufsuchen einer Position im Hash
// -------------------------------------------------------------------------

PTTe* PHash::lookup(hash_t lock, int stage, hash_t key)
{
    PTTe* p = pTable + (key & x_mask); 

    // Kein Eintrag?
//    if (!p->lock)
//        return 0;

    // Passt der Eintrag?
    if (p->lock == lock && p->getStage() == stage)
    {
#ifdef PHASH_STATISTIC
        nFound++;
#endif
        return p;
    }

#ifdef PHASH_STATISTIC
        nNotFound++;
#endif
    return 0;
}


