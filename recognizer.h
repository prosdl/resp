// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : recognizer.h
//                       Erkennen und Bewerten von Endspielpositionen
//
//  Anfang des Projekts: Fr. 5.3.2002
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.15
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: recognizer.h,v 1.6 2003/06/02 18:12:53 rosendahl Exp $
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

#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include "board.h"

// -------------------------------------------------------------------------
//                  c l a s s  R e c o g T a b l e 
// -------------------------------------------------------------------------
class RecogTable
{
public:
    enum RecogResult {
        FAILE   = 0,
        LBOUND  = 1,
        UBOUND  = 2,
        EXACT   = 3
    };

    // -------------------------------------------------------------------------
    //  Innere Klasse:   c l a s s  R e c o g n i z e r
    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    //                      *** Basisklasse (abstrakt) ***
    // -------------------------------------------------------------------------
    class Recognizer
    {

    protected:
        int score;


    public:
        // ---------------------------------------------------
        //  Konstruktor / Destruktor
        // ---------------------------------------------------
        Recognizer() {}
        virtual ~Recognizer() {} 


        // ---------------------------------------------------
        //  Aktivieren des Recognizers ... der Rueckgabewert
        //  entscheidet ueber die Verwendbarkeit
        // ---------------------------------------------------
        virtual RecogResult execute(const Board* pBoard) = 0;

        // ---------------------------------------------------
        // Material-Signaturen, bei denen der Recognizer 
        // aktiviert wird. Die Rollen von Weiss und Schwarz
        // spielen dabei keine Rolle, da 
        // RecogTable::lookup(w_mat,b_mat) kommutativ ist.
        //
        // Aufbau Materialsignaturen:
        //  x_mat ==  xxxqrbkp  (binaer)
        //  mit:
        //          p - Pawn
        //          k - Knight
        //          b - Bishop
        //          r - Rook
        //          q - Queen
        // ---------------------------------------------------
        virtual unsigned int w_mat() = 0;
        virtual unsigned int b_mat() = 0;

        int getScore() { return score; }
    };


    // -------------------------------------------------------------------------
    //                      *** KK Recognizer ***
    // -------------------------------------------------------------------------
    class KK_Recognizer : public Recognizer {
        unsigned int w_mat() { return 0x0; } // K
        unsigned int b_mat() { return 0x0; } // K

        RecogResult execute(const Board* pBoard);
    };

    // -------------------------------------------------------------------------
    //                      *** KBK Recognizer ***
    // -------------------------------------------------------------------------
    class KBK_Recognizer : public Recognizer {
        unsigned int w_mat() { return 0x4; } // KB
        unsigned int b_mat() { return 0x0; } // K

        RecogResult execute(const Board* pBoard);
    };

    // -------------------------------------------------------------------------
    //                      *** KNK Recognizer ***
    // -------------------------------------------------------------------------
    class KNK_Recognizer : public Recognizer {
        unsigned int w_mat() { return 0x2; } // KN
        unsigned int b_mat() { return 0x0; } // K

        RecogResult execute(const Board* pBoard);
    };

    // -------------------------------------------------------------------------
    //                      *** KNKN Recognizer ***
    // -------------------------------------------------------------------------
    class KNKN_Recognizer : public Recognizer {
        unsigned int w_mat() { return 0x2; } // KN
        unsigned int b_mat() { return 0x2; } // K

        RecogResult execute(const Board* pBoard);
    };


    // -------------------------------------------------------------------------
    //                      *** KBPK Recognizer ***
    // -------------------------------------------------------------------------
    class KBPK_Recognizer : public Recognizer {
        unsigned int w_mat() { return 0x5; } // KBP
        unsigned int b_mat() { return 0x0; } // K

        RecogResult execute(const Board* pBoard);
    };

    // -------------------------------------------------------------------------
    //                      *** KBKP Recognizer ***
    // -------------------------------------------------------------------------
    class KBKP_Recognizer : public Recognizer {
        unsigned int w_mat() { return 0x4; } // KB
        unsigned int b_mat() { return 0x1; } // KP

        RecogResult execute(const Board* pBoard);
    };

    // -------------------------------------------------------------------------
    //                      *** KNKP Recognizer ***
    // -------------------------------------------------------------------------
    class KNKP_Recognizer : public Recognizer {
        unsigned int w_mat() { return 0x2; } // KN
        unsigned int b_mat() { return 0x1; } // KP

        RecogResult execute(const Board* pBoard);
    };

    // -------------------------------------------------------------------------
    //                      *** KPK Recognizer ***
    // -------------------------------------------------------------------------
    class KPK_Recognizer : public Recognizer {
        unsigned int w_mat() { return 0x1; } // KP
        unsigned int b_mat() { return 0x0; } // K

        RecogResult execute(const Board* pBoard);
    };


private:
    void build_table();
    Recognizer* rec_table[1024];

private:

    // -------------------------------------------------
    //  Singleton
    // -------------------------------------------------
    RecogTable();
    RecogTable(const KEDB&);
    static RecogTable* p_instance;

public:
    static RecogTable* getHandle() { 
       if (p_instance == NULL) {
          p_instance = new RecogTable();
       }
       return p_instance; 
    }

public:
    Recognizer* lookup(unsigned int w_mat, unsigned int b_mat)
    {
        return rec_table[w_mat << 5 | b_mat];
    }
};




#endif  // RECOGNIZER_H
