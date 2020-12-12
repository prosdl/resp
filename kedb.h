// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : kedb.h
//                       Knowledgeable Endgame Databases
//
//  Anfang des Projekts: Fr. 1.3.2002
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.15
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: kedb.h,v 1.6 2003/06/02 18:12:53 rosendahl Exp $
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

#ifndef KEDB_H
#define KEDB_H

#include "board.h"

// Groesse der KPK Datenbank: 2**18 
const int KPK_SIZE = 1<<18;


// -------------------------------------------------------------------------
//   c l a s s   K E D B 
// -------------------------------------------------------------------------

/** Implementierung einer Endspieldatenbank. Die Ideen hierzu stammen 
 *  im Wesentlichen aus Ernst A. Heinz "Scalable Search in Computer Chess".
 */
class KEDB
{
private:

    // -------------------------------------------------
    //  Singleton
    // -------------------------------------------------
    KEDB() 
    { 
        kpk_ready = false;
    }
    KEDB(const KEDB&);
    static KEDB * p_instance;


private:
    // ---------------------------------------
    // Spiegeln von Positionen auf dem Brett
    // ---------------------------------------

    // Spiegelachse zwischen Spalte D und E
    int mirror_de(int x) const
    {
        return (x&0x38) | ( (x&0x7) ^ 0x7);
    }

    // Spiegelachse zwischen Reihe 4 und 5
    int mirror_45(int x) const
    {
        return (x&0x7) | ( (x&0x38) ^ 0x38);
    }

    // Position um 180° drehen (Komposition von den beiden Spiegelungen)
    int rotate_180(int x) const
    {
        return x^0x3f;
    }

    // ---------------------------------------
    //  Index der Position x zur linken
    //  Bretthaelfte
    // ---------------------------------------
    int lh_pos(int x)  const // {0,..,63} --> {0,..,31}
    {
        // A8 == 0  --> 0
        // D1 == 59 --> 31
        // Allgemeine Formel:  (x / 8) * 4 + x%4
        return ((x&0x38)>>1) | (x&3);
    }

    // ---------------------------------------
    //  Abstand zweier Felder
    // ---------------------------------------
    int dist(int x, int y)
    {
        int d_col = abs(COL(x) - COL(y));
        int d_row = abs(ROW(x) - ROW(y));

        return (d_col > d_row) ? d_col : d_row;
    }

    // --------------------------
    //  Position ins Brett laden
    // --------------------------
    void load_position(Board *pB, ColorEnum new_side, ByteBoard& byb);

    // --------------------------------
    //  Bit-Vektor lesen / schreiben
    // --------------------------------
    bool write_db(std::string filename, unsigned int bit_vector[], int n_entries);
    bool read_db(std::string filename, unsigned int bit_vector[]);


    bool kpk_ready;

public:
    /**
     *  Aufzaehlung der moeglichen DB-Ergebnis-
     *  werte. 
     */
    enum TableEntry {
        UNKNOWN         = 254,
        STRONG_WINS     = 1,
        REMIS           = 0,
        ILLEGAL         = 255
    };

    /** 
     * Zugriff auf Objekt.
     */
    static KEDB* getHandle() { 
       if (p_instance == NULL) {
          p_instance = new KEDB();
       }
       return p_instance; 
    }

    /** 
     * Erstellen von Datenbanken.
     */
    void create_kpk_db();

    /** 
     * Laden der DBen von Platte.
     */
    bool load_db();

    /** Indizes. */
    unsigned int kpk_index(const Board* pB) const;

    /**  Zugriff auf Datenbanken. */
    int query_kpk(unsigned int idx) const;

    /** Datenbanken als Bitvektoren. */
    unsigned int kpk[KPK_SIZE/32];

};


#endif // KEDB_H

