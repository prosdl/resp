// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : hash.cpp
//                       Transposition table für resp.
//
//  Anfang des Projekts: Di, 4.September, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: hash.cpp,v 1.24 2003/05/31 19:25:50 rosendahl Exp $
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

#include "hash.h"
#include "respmath.h"
#include <iostream>

using namespace std;


// -------------------------------------------------------------------------
//                      c l a s s   H a s h
// -------------------------------------------------------------------------

Hash* Hash::instance = NULL;


// -------------------------------------------------------------------------
//  Konstruktor: reservieren des Speicherplatzes - die Größe des Hash muss
//               eine 2er Potenz >=1024 sein
// -------------------------------------------------------------------------

Hash::Hash(long max_size_in_bytes, int maxReh)
{
    long two_pow_x;

    // two_pow_x bestimmen, so dass two_pow_x * sizeof(TTe) <= max_size_in_bytes, x minimal

    two_pow_x = 1024 ;   // minimale Groesse 1 x sizeof(TTe) kB;

    while (two_pow_x * static_cast<long>(sizeof(TTe)) <= max_size_in_bytes)
    {
        two_pow_x <<= 1;
    }

    two_pow_x >>= 1;

    if (two_pow_x < 1024)
        two_pow_x = 1024;

    this->size = two_pow_x;

    x_mask = this->size - 1;

    // Speicherplatz für Hashtable reservieren
    cout << "Allocating  "<< two_pow_x*sizeof(TTe) / 1024 << " kBytes for hash ... ";
    pTable = new TTe[this->size];
    cout << "OK" << endl;

    if (maxReh > MAX_REHASH)
    {
        // Ungültige Initialisierung
        cout << "Illegal value for maxRehash in Hash::Hash" << endl;
        exit(1);
    }
    this->maxRehash = maxReh;


    // Initialisierung der Strukturen fürs Zobrisk Hashing

    init();
}

// -------------------------------------------------------------------------
//  Destruktor: Platz für Tabelle wieder freigeben
// -------------------------------------------------------------------------

Hash::~Hash()
{
    delete[] pTable;
}


// -------------------------------------------------------------------------
//  Initialisierung der für das Zobrist-Hashing benötigten Felder
// -------------------------------------------------------------------------
void Hash::init()
{
    int i,j;

    for (i=0; i < 64; i++)
        for (j=0; j < 7; j++)
        {
            z1_white_pieces[j][i] = RespMath::z_random();
            z1_black_pieces[j][i] = RespMath::z_random();
            z2_white_pieces[j][i] = RespMath::z_random();
            z2_black_pieces[j][i] = RespMath::z_random();
        }

 //   for (i=0; i < MAX_REHASH; i++)
 //       z_rehash[i] = RespMath::z_random();
}


// -------------------------------------------------------------------------
//  Den kompletten Hash so reinitialisieren, dass er bei einem neuen
//  Spiel nichts blockiert
// -------------------------------------------------------------------------
void Hash::clear()
{
    cout << "Clearing Hash ... ";
    TTe* q = pTable;

    for (long h = 0; h < size; h++,q++)
    {
         q->lock = q->special = 0;
    }

    for (int i=0; i<MAX_REHASH; i++)
        nFound[i] = 0;
    nInsertFree     = 0;
    nInsertDepth    = 0;
    nAge            = 0;
    nInsertUnsafe   = 0;
    nFailedRehashs  = 0;

    cout << "OK." << endl;
}

// -------------------------------------------------------------------------
//  Den Hashwert einer Schachposition berechnen; 
//  wird nur am Anfang eines Spiels benötigt, später erfolgt eine
//  sukzessive Neuberechnung des Hashwertes
// -------------------------------------------------------------------------

hash_t Hash::z1_value(BYTE piece[64], BYTE color[64])
{
    int i;
    hash_t val = 0;


    for (i=0; i<64; i++)
    {
        if (piece[i] != NO_PIECE)
        {
            if (color[i] == WHITE)
                val ^= z1_white_pieces[ piece[i] ][ i ];
            else
                val ^= z1_black_pieces[ piece[i] ][ i ];
        }
    }


    return val;
}

hash_t Hash::z2_value(BYTE piece[64], BYTE color[64])
{
    int i;
    hash_t val = 0;


    for (i=0; i<64; i++)
    {
        if (piece[i] != NO_PIECE)
        {
            if (color[i] == WHITE)
                val ^= z2_white_pieces[ piece[i] ][ i ];
            else
                val ^= z2_black_pieces[ piece[i] ][ i ];
        }
    }


    return val;
}

// Aktualisierung der Positions-Hashwerte, wenn Zug m gemacht wird:

void Hash::update_z1val(hash_t& z1val, const Move& m, BYTE side)
{
    if (side == WHITE)
    {
        BYTE piece = m.getPiece();
        
        z1val ^= z1_white_pieces[piece][m.from()];

        if (m.isPromotion())
            z1val ^= z1_white_pieces[m.getPromPiece()]
                                    [m.to()];
        else
            z1val ^= z1_white_pieces[piece][m.to()];

        if (m.isCapture())
            z1val ^= z1_black_pieces[m.getCapturedPiece()][m.to()];
    }
    else
    {
        BYTE piece = m.getPiece();

        z1val ^= z1_black_pieces[piece][m.from()];

        if (m.isPromotion())
            z1val ^= z1_black_pieces[m.getPromPiece()]
                                    [m.to()];
        else
            z1val ^= z1_black_pieces[piece][m.to()];

        if (m.isCapture())
            z1val ^= z1_white_pieces[m.getCapturedPiece()][m.to()];
    }
}


void Hash::update_z2val(hash_t& z2val, const Move& m, BYTE side)
{
    if (side == WHITE)
    {
        BYTE piece = m.getPiece();
        z2val ^= z2_white_pieces[piece][m.from()];

        if (m.isPromotion())
            z2val ^= z2_white_pieces[m.getPromPiece()]
                                    [m.to()];
        else
            z2val ^= z2_white_pieces[piece][m.to()];

        if (m.isCapture())
            z2val ^= z2_black_pieces[m.getCapturedPiece()][m.to()];
    }
    else
    {
        BYTE piece = m.getPiece();
        z2val ^= z2_black_pieces[piece-1][m.from()];

        if (m.isPromotion())
            z2val ^= z2_black_pieces[m.getPromPiece()]
                                    [m.to()];
        else
            z2val ^= z2_black_pieces[piece][m.to()];

        if (m.isCapture())
            z2val ^= z2_white_pieces[m.getCapturedPiece()][m.to()];
    }
}


// -------------------------------------------------------------------------
//  Einfügen einer Postion in den Hash
// -------------------------------------------------------------------------

bool Hash::insert(TTe& info, hash_t z1_hash, int root_age)
{
    TTe* p = pTable + (z1_hash & x_mask); 



    // geeigneten Platz zum Einfügen suchen
    int i = 0;
    TTe* q_mindepth = NULL;
    int min_depth = 10000;

    while (i < maxRehash)
    {
        // -----------------------------------------------
        //             Sichere Kandidaten
        // -----------------------------------------------

        // Keine Kollision?
        if ( !p->getDepth() )
        {

#ifdef DO_STATISTICS
            nInsertFree++;
#endif // DO_STATISTICS

            *p = info;
            return true;
        }


        if (p->lock == info.lock && p->getInfoBits() == info.getInfoBits()) 
        {
            // gleiche Stellung mit geringerer Rechentiefe
            if (p->getDepth() <= info.getDepth())
            {
#ifdef DO_STATISTICS
                nInsertDepth++;
#endif // DO_STATISTICS

                *p = info;
                return true;
            }
        }


        //  veraltete Positionen überschreiben
        if (p->getAge() < root_age) 
        {

#ifdef DO_STATISTICS
            nAge++;
#endif // DO_STATISTICS

            *p = info;
            return true;
        }

        // -----------------------------------------------
        //          "Unsichere" Kandidaten
        // -----------------------------------------------

        if (p->getDepth() < min_depth)
        {
            q_mindepth = p;
            min_depth = p->getDepth();
        }

        // REHASH
        z1_hash++; 
        p = pTable + (z1_hash & x_mask);
        i++;

    }


    // zumind. einen unsicheren Kandidaten gefunden?
    if (min_depth < 10000)
    {

#ifdef DO_STATISTICS
        nInsertUnsafe++;
#endif // DO_STATISTICS

        *q_mindepth = info;
        return true;
    }

#ifdef DO_STATISTICS
    nFailedRehashs++;
#endif // DO_STATISTICS

    return false;
}


// -------------------------------------------------------------------------
//  Aufsuchen einer Position im Hash
// -------------------------------------------------------------------------

bool Hash::lookup(TTe& info, hash_t z1_hash) 
{
    TTe* p;

    for (int i=0; i < maxRehash; i++)
    {
        p = pTable + (z1_hash & x_mask);

        // Leerer Eintrag? 
        if (!p->lock)
            return false; // Abbruch

        if ( (p->lock == info.lock) && (p->getInfoBits() == info.getInfoBits()) )
        {
            // Position mit mind. gleicher Tiefe wie info gefunden?
            if ( p->getDepth() >= info.getDepth() )
            {

                #ifdef DO_STATISTICS
                nFound[i]++;
                #endif

                info.score = p->score;
                info.special = p->special;
                info.m     = p->m;

                return true; // Gespeicherte Auswertung zurückgeben
            }
            else
            {
                info.m = p->m;
                info.setType(TTe::DONT_USE);
                return true;
            }
        }

        // Rehash
        z1_hash++;
    }

    return false;
}

