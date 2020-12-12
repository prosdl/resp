// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : phash.h
//                       Header zu phash.cpp.
//
//  Anfang des Projekts: Sa 3.November, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: phash.h,v 1.20 2003/05/31 19:25:51 rosendahl Exp $
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

#ifndef PHASH_H
#define PHASH_H

#include "basic_stuff.h"
#include "respoptions.h"

static const unsigned int bit_lookup_hopwh[8] = {
    0x10000,0x20000,0x40000,0x80000,0x100000,0x200000,0x400000,0x800000
};

static const unsigned int bit_lookup_hopbl[8] = {
    0x1000000,0x2000000,0x4000000,0x8000000,0x10000000,0x20000000,
    0x40000000,0x80000000
};

// -------------------------------------------------------------------------
//                      c l a s s   P T T e
// -------------------------------------------------------------------------

// Liefert Informationen zur Bauernstruktur eines Blattes im Suchbaum
class PTTe
{
public:
    hash_t          lock;       // ID für die Position
    UINTEGER32      files;    // Informationen über Bauernposition
                              // Bits #01 .. #02: stage (0,..,3)
                              // Bits #03 .. #08: N/A
                              // Bits #09 .. #16: N/A
                              // Bits #17 .. #24: halboffene Linien Weiss :=
                              //                  kein weisser Bauer auf der Linie
                              // Bits #25 .. #32: halboffene Linien Schwarz :=
                              //                  kein schwarzer Bauer auf der Linie
    short           score;      // Score    
    unsigned char   w_passed;   // Freibauern weiss
    unsigned char   b_passed;   // Freibauern schwarz

public:
    PTTe() :  lock(0), files(0), w_passed(0), b_passed(0)
    {  
    }

    PTTe(hash_t _lock, int _score, int stage, unsigned char _w_passed,
         unsigned char _b_passed) : lock(_lock), score(_score), w_passed(_w_passed),
                                    b_passed(_b_passed)
    {
        files = stage & 0x3;
    }

    PTTe(const PTTe& info)
    {
        lock     = info.lock;
        score    = info.score;
        files    = info.files;
        w_passed = info.w_passed;
        b_passed = info.b_passed;
    }

    PTTe& operator=(const PTTe& info)
    {
        lock     = info.lock;
        score    = info.score;
        files    = info.files;
        w_passed = info.w_passed;
        b_passed = info.b_passed;
        return *this;
    }


    // Prüfen, ob (halb-)offene Linie besetzt ist

    UINTEGER32 isHalfOpenWhite(int col) const
    {
        return bit_lookup_hopwh[col] & files;
    }
    UINTEGER32 isHalfOpenBlack(int col) const
    {
        return bit_lookup_hopbl[col] & files;
    }


    // Spielphase holen
    int getStage() const
    {
        return files & 0x3;
    }



    // Spielphase setzen
    void setStage(int stage)
    {
        files |= stage & 0x3; 
    }

    // Setzen (halb-)offener Linien
    void setHalfOpenWhite(int col)
    {
        files |= bit_lookup_hopwh[col];
    }
    void setHalfOpenBlack(int col)
    {
        files |= bit_lookup_hopbl[col];
    }

    int getWPawns()
    {
        return (~(files >> 16)) & 0xff;
    }
    int getBPawns()
    {
        return (~(files >> 24)) & 0xff;
    }

    // ---------------
    //  Freibauern
    // ---------------
    void setWPassed(unsigned char col)
    {
        w_passed |= 1<<col;
    }
    void setBPassed(unsigned char col)
    {
        b_passed |= 1<<col;
    }

    unsigned char getWPassed() const
    {
        return w_passed;
    }
    unsigned char getBPassed() const
    {
        return b_passed;
    }

};

// -------------------------------------------------------------------------
//                      c l a s s  P H a s h
// -------------------------------------------------------------------------
class PHash
{
private:
    PTTe*   pTable;     // eigentlicher Hashtable

    long   size;     // Größe des Hashtable (2'er Potenz) 
    hash_t x_mask;   // Maske mit dem Wert size-1 

    // Für Zobrist Hashing benötigte Tabellen:
    hash_t zp1_white_pawns[64];
    hash_t zp1_black_pawns[64];
    hash_t zp2_white_pawns[64];
    hash_t zp2_black_pawns[64];

    // Statistik
public:
    long nFound;
    long nNotFound;
    long nInsert;

private:
    // Initialisierungen
    void init();

    PHash(long max_size_in_bytes=4000000);
    ~PHash();

    static PHash* instance;
    
public:
    static PHash* getHandle() {
       if (instance == NULL) {
          instance = 
            new PHash(
               RespOptions::getHandle()->getMBValueAsLong("phash.size"));
       }
       return instance;
    }

    PTTe* insert(const PTTe& pawn_info, hash_t key);
    PTTe* lookup(hash_t lock, int stage, hash_t key);

    // Berechnen des Hashwerts für ein komplettes Brett
    hash_t zp1_value(BYTE piece[64], BYTE color[64]);
    hash_t zp2_value(BYTE piece[64], BYTE color[64]);


    // Aktualisieren von Hashwerten
    hash_t zp1XorWhite(int pos) const
    {
        return zp1_white_pawns[pos];
    }
    hash_t zp1XorBlack(int pos) const
    {
        return zp1_black_pawns[pos];
    }
    hash_t zp2XorWhite(int pos) const
    {
        return zp2_white_pawns[pos];
    }
    hash_t zp2XorBlack(int pos) const
    {
        return zp2_black_pawns[pos];
    }


    // Reinitialisieren des Hash
    void clear();

    long getSize() const { return size * sizeof(PTTe); }
    long getMaxEntries() const { return size;  }


};


#endif // PHASH_H

