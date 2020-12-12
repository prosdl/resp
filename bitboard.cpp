// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : bitboard.cpp
//                       Implementation von Bitboards.
//
//  Anfang des Projekts: Do., 5. Dezember 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: bitboard.cpp,v 1.20 2003/05/13 20:52:47 rosendahl Exp $
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
#include "bitboard.h"
#include "basic_stuff.h"
#include <iostream>


using namespace std;

// -------------------------------------------------------------------------
//              Definitionen zu class BBTables
// -------------------------------------------------------------------------

BITBOARD BBTables::whitePawnMoves[64];
BITBOARD BBTables::blackPawnMoves[64];
BITBOARD BBTables::whitePawnCaps[64];
BITBOARD BBTables::blackPawnCaps[64];
BITBOARD BBTables::bbmask[64];
BITBOARD BBTables::bbmask45C[64];
BITBOARD BBTables::bbmask45AC[64];
BITBOARD BBTables::bbmask90C[64];
BITBOARD BBTables::filemask[8];
BITBOARD BBTables::rankmask[8];

BITBOARD BBTables::filerank_mask[64];
BITBOARD BBTables::alldiag_mask[64];
BITBOARD BBTables::all_mask[64];

BITBOARD BBTables::rank_attacks[64][256];
BITBOARD BBTables::file_attacks[64][256];
BITBOARD BBTables::diagH8A1_attacks[64][256];
BITBOARD BBTables::diagH1A8_attacks[64][256];
BITBOARD BBTables::knight_attacks[64];
BITBOARD BBTables::king_attacks[64];

BITBOARD BBTables::bbmaskF1G1;
BITBOARD BBTables::bbmaskF8G8;
BITBOARD BBTables::bbmaskB1C1D1;
BITBOARD BBTables::bbmaskB8C8D8;
BITBOARD BBTables::bbmaskBCDEFGH;
BITBOARD BBTables::bbmaskABCDEFG;
BITBOARD BBTables::bbmaskA8B8A7B7;
BITBOARD BBTables::bbmaskG8H8G7H7;
BITBOARD BBTables::bbmaskA1B1A2B2;
BITBOARD BBTables::bbmaskG1H1G2H2;

BITBOARD BBTables::w_ppawn_mask[64];
BITBOARD BBTables::b_ppawn_mask[64];

BITBOARD BBTables::whiteSquares;

int BBTables::rank_mobility[64][256];
int BBTables::file_mobility[64][256];
int BBTables::diagH8A1_mobility[64][256];
int BBTables::diagH1A8_mobility[64][256];

BYTE BBTables::bitCountForByte[256];

int BBTables::dist[64][64];

int BBTables::msb16[65536];
int BBTables::lsb16[65536];

int BBTables::direction[64][64];
BITBOARD BBTables::squaresBetween[64][64];



// -----------------------------------------------------------------------
// rot90C_bitIndex:
//
// Es gilt:             BB90C (i) = BB ( rot90C_bitIndex[i] ) 
//
//                      für ein BITBOARD BB bei Index i.
//
// rot90AC_bitIndex ist die Umkehrfunktion zu rot90C_bitIndex.
// (Brauch ich die überhaupt??)
/*
int BBTables::rot90C_bitIndex[64] = {
     7,15,23,31,39,47,55,63,
     6,14,22,30,38,46,54,62,
     5,13,21,29,37,45,53,61,
     4,12,20,28,36,44,52,60,
     3,11,19,27,35,43,51,59,
     2,10,18,26,34,42,50,58,
     1, 9,17,25,33,41,49,57,
     0, 8,16,24,32,40,48,56
};
int BBTables::rot90AC_bitIndex[64] = {
    56,48,40,32,24,16, 8, 0,
    57,49,41,33,25,17, 9, 1,
    58,50,42,34,26,18,10, 2,
    59,51,43,35,27,19,11, 3,
    60,52,44,36,28,20,12, 4,
    61,53,45,37,29,21,13, 5,
    62,54,46,38,30,22,14, 6,
    63,55,47,39,31,23,15, 7
};


// ------------------------------------------------------
// rot45C_bitIndex
//
//      BB45C(i) = BB( rot45C_bitIndex[i] ) 
//
//      für ein Bitboard BB bei Index i.

int BBTables::rot45C_bitIndex[64] = {
    28,21,15,10, 6, 3, 1, 0,
    36,29,22,16,11, 7, 4, 2,
    43,37,30,23,17,12, 8, 5,
    49,44,38,31,24,18,13, 9,
    54,50,45,39,32,25,19,14,
    58,55,51,46,40,33,26,20,
    61,59,56,52,47,41,34,27,
    63,62,60,57,53,48,42,35
};

int BBTables::rot45AC_bitIndex[64] = {
    0, 2, 5, 9,14,20,27,35,
    1, 4, 8,13,19,26,34,42, 
    3, 7,12,18,25,33,41,48,
    6,11,17,24,32,40,47,53,
   10,16,23,31,39,46,52,57,
   15,22,30,38,45,51,56,60,
   21,29,37,44,50,55,59,62,
   28,36,43,49,54,58,61,63
};*/


bool BBTables::initialized = false;

// -------------------------------------------------------------------------
//  Initialisierungen für die Tabellen
// -------------------------------------------------------------------------
void BBTables::init()
{
    if (initialized)
        return;


    int i;
    UINTEGER32 m;
    int x,y;
    BITBOARD b;


    cout << "#Initializing tables ..." << endl;
 
    // filemask ... vertikale Spalten als Bitboards (a .. h)
    for (i=0; i<8; i++)
        filemask[i] = 0;

    // rankmask ... horizontale Reihen als Bitboards (1 .. 8)
    for (i=0; i<8; i++)
        rankmask[i] = 0;

    for (i=0, b=1; i<64; i++, b <<= 1)
    {
        bbmask[i] = b;
        filemask[COL(i)] |= b; 
        rankmask[ROW(i)] |= b;
    }

    // Kombinierte Reihen/Spalten
    for (i=0; i < 64; i++)
    {
        filerank_mask[i] = filemask[COL(i)] | rankmask[ROW(i)];
    }

    bbmaskF1G1   = bbmask[61] | bbmask[62];
    bbmaskF8G8   = bbmask[ 5] | bbmask[ 6];
    bbmaskB1C1D1 = bbmask[57] | bbmask[58] | bbmask[59];
    bbmaskB8C8D8 = bbmask[ 1] | bbmask[ 2] | bbmask[ 3];

    bbmaskABCDEFG = ~filemask[7];
    bbmaskBCDEFGH = ~filemask[0];

    bbmaskA1B1A2B2 = bbmask[56] | bbmask[57] | bbmask[48] | bbmask[49];
    bbmaskA8B8A7B7 = bbmask[ 0] | bbmask[ 1] | bbmask[ 8] | bbmask[ 9];
    bbmaskG1H1G2H2 = bbmask[62] | bbmask[63] | bbmask[54] | bbmask[55];
    bbmaskG8H8G7H7 = bbmask[ 6] | bbmask[ 7] | bbmask[14] | bbmask[15];

    for (i=0; i<64; i++)
    {
        bbmask45C[i]    = bbmask[rot45C_bitIndex[i]];
        bbmask45AC[i]   = bbmask[rot45AC_bitIndex[i]];
        bbmask90C[i]    = bbmask[rot90C_bitIndex[i]];
    }


    
    // Züge für Bauern
    for (i = 0; i < 64; i++)
    {
        if (i < 8)
        {
            whitePawnMoves[i] = 0;
            blackPawnMoves[i] = 0;
            whitePawnCaps[i] = 0;
            blackPawnCaps[i] = 0;

            // Angriffszüge auch für die Grundreihe generieren (obwohl
            // dort nie ein Bauer stehen kann); wird benötigt, um später festzu-
            // stellen, ob eine Figur auf der Grundreihe von einem Bauern ange-
            // griffen wird.
            if (COL(i) >= 1)
                blackPawnCaps[i] |= bbmask[i+7];
            if (COL(i) <= 6)
                blackPawnCaps[i] |= bbmask[i+9];
        }
        else if (i >= 56)
        {
            whitePawnMoves[i] = 0;
            blackPawnMoves[i] = 0;
            whitePawnCaps[i] = 0;
            blackPawnCaps[i] = 0;

            if (COL(i) >= 1)
                whitePawnCaps[i] |= bbmask[i-9];
            if (COL(i) <= 6)
                whitePawnCaps[i] |= bbmask[i-7];
        }
        else
        {
            whitePawnMoves[i] = bbmask[i-8];
            blackPawnMoves[i] = bbmask[i+8];

            whitePawnCaps[i] = 0;
            blackPawnCaps[i] = 0;
            if (COL(i) >= 1)
            {
                whitePawnCaps[i] |= bbmask[i-9];
                blackPawnCaps[i] |= bbmask[i+7];
            }
            if (COL(i) <= 6)
            {
                whitePawnCaps[i] |= bbmask[i-7];
                blackPawnCaps[i] |= bbmask[i+9];
            }
        }
    }

    // Züge für Springer
    int i_safe,j;
    for (i=0; i<64; i++)
    {
        knight_attacks[i] = 0;

        for (j=0; j<8; j++)
            if ((i_safe = mailbox[mailbox_address[i] + knight_moves[j]]) != - 1)
                knight_attacks[i] |= bbmask[i_safe];
    }

    // Züge für König
    for (i=0; i<64; i++)
    {
        king_attacks[i] = 0;

        for (j=0; j<8; j++)
            if ((i_safe = mailbox[mailbox_address[i] + king_moves[j]]) != - 1)
                king_attacks[i] |= bbmask[i_safe];
    }

    // ---- rank_attacks/file_attacks/diag..._attacks initialisieren --------

    for (i=0; i<64; i++)
        for (m=0; m<256; m++)
        {
            rank_attacks[i][m]      = 0;
            file_attacks[i][m]      = 0;
            diagH8A1_attacks[i][m]  = 0;
            diagH1A8_attacks[i][m]  = 0;

            rank_mobility[i][m]      = 0;
            file_mobility[i][m]      = 0;
            diagH8A1_mobility[i][m]  = 0;
            diagH1A8_mobility[i][m]  = 0;

            //if (m & (1 << COL(i))) // Figur vorhanden bei x
            {
                // nach links schauen...

                y = i;      // y auf Startpos.
                x = COL(i); // x auf Spaltennr.
                while (true)
                {
                    y -= 1;
                    x -= 1;
                    if (x < 0)  // Überm Rand?
                        break;

                    rank_attacks[i][m] |= bbmask[y];

                    if (m & (1 << x))    // Auf Figur getroffen?
                        break;

                    rank_mobility[i][m]++;

                }

                // nach rechts schauen...

                y = i;      // y auf Startpos.
                x = COL(i); // x auf Spaltennr.
                while (true)
                {
                    y += 1;
                    x += 1;
                    if (x > 7)  // Überm Rand?
                        break;

                    rank_attacks[i][m] |= bbmask[y];

                    if (m & (1 << x))    // Auf Figur getroffen?
                        break;

                    rank_mobility[i][m]++;
                }
            }

            //if (m & (1 << (7 - ROW(i))))
            {
                // nach unten schauen...

                y = i;      // y auf Startpos.
                x = ROW(i); // x auf Spaltennr.
                while (true)
                {
                    y -= 8;
                    x -= 1;
                    if (x < 0)  // Überm Rand?
                        break;

                    file_attacks[i][m] |= bbmask[y];

                    if (m & (1 << (7 - x)))    // Auf Figur getroffen?
                        break;

                    file_mobility[i][m]++;

                }
                // nach oben schauen...

                y = i;      // y auf Startpos.
                x = ROW(i); // x auf Spaltennr.
                while (true)
                {
                    y += 8;
                    x += 1;
                    if (x > 7)  // Überm Rand?
                        break;

                    file_attacks[i][m] |= bbmask[y];

                    if (m & (1 << (7 - x)))    // Auf Figur getroffen?
                        break;

                    file_mobility[i][m]++;
                }
            }

            // ------------- diagH8A1_attacks ---------------------------
    
            //if (m & (1 << POS_DIAG_H8A1(i))) // Position i in m gesetzt
            {
                int z,s;

                // nach links oben schauen...
                y = i;
                z = ROW(i);
                s = COL(i);

                while (true)
                {
                    y -= 9;
                    z--; s--;

                    if ((z<0) || (s<0)) // Überm Rand?
                        break;

                    diagH8A1_attacks[i][m] |= bbmask[y];

                    if ( m & (1 << POS_DIAG_H8A1(y)))
                        break;

                    diagH8A1_mobility[i][m]++;
                }

                // nach rechts unten schauen...
                y = i;
                z = ROW(i);
                s = COL(i);

                while (true)
                {
                    z++; s++;
                    y += 9;

                    if ((z>7) || (s>7)) // Überm Rand?
                        break;

                    diagH8A1_attacks[i][m] |= bbmask[y];

                    if ( m & (1 << POS_DIAG_H8A1(y)))
                        break;
                    diagH8A1_mobility[i][m]++;
                }
            }

            // ------------- diagH1A8_attacks ---------------------------
    
            //if (m & (1 << POS_DIAG_H1A8(i))) // Position i in m gesetzt
            {
                int z,s;

                // nach links unten schauen...
                y = i;
                z = ROW(i);
                s = COL(i);

                while (true)
                {
                    y += 7;
                    z++; s--;

                    if ((z>7) || (s<0)) // Überm Rand?
                        break;

                    diagH1A8_attacks[i][m] |= bbmask[y];

                    if ( m & (1 << POS_DIAG_H1A8(y)))
                        break;

                    diagH1A8_mobility[i][m]++;
                }

                // nach rechts oben schauen...
                y = i;
                z = ROW(i);
                s = COL(i);

                while (true)
                {
                    z--; s++;
                    y -= 7;

                    if ((z<0) || (s>7)) // Überm Rand?
                        break;

                    diagH1A8_attacks[i][m] |= bbmask[y];

                    if ( m & (1 << POS_DIAG_H1A8(y)))
                        break;
                    diagH1A8_mobility[i][m]++;
                }
            }
        }


    // Kombinierte Diagonalreihen
    for (i=0; i < 64; i++)
    {
        alldiag_mask[i] = diagH1A8_attacks[i][0] | diagH8A1_attacks[i][0] | bbmask[i];
    }

    // Diag. + Horiz. + Vertik.
    for (i = 0; i < 64; i++) {
       all_mask[i] = alldiag_mask[i] | filerank_mask[i];
    }


    // -----------------------
    //  Popcount für Bytes
    // -----------------------
   int by;

   for (by = 0; by <= 255; by++)
   {
      int count = 0;
      for (int m=1; m < 256; m <<=1)
         if (m&by)
            count++;

      bitCountForByte[by] = count;
   }

    // -----------------------
    //  msb/lsb für 16-Bit
    // -----------------------
    {
        long hit = 1;
        BYTE bit = 0;

        for (long i = 1; i < 65536; i++)
        {
            if (hit & i)
            {
                hit <<= 1;
                bit++;
            }
            msb16[i] = bit-1;
        }

        msb16[0] = 255;
    }

    {
        for (long i = 1; i < 65536; i++)
        {
          int j;
            for (j = 0; j < 16; j++)
                if ( (1 << j) & i)
                    break;

            lsb16[i] = j;
        }

        lsb16[0] = 255;
    }

    // -----------------------------
    //  directions / squaresBetween
    // -----------------------------


    {
        for (int p1 = 0; p1 < 64; p1++)
            for (int p2 = 0; p2 < 64; p2++)
            {
                int c1 = COL(p1);
                int c2 = COL(p2);
                int r1 = ROW(p1);
                int r2 = ROW(p2);

                int d = 0;

                if (p1 != p2)
                {
                    if (c1 == c2)
                        d = 2;
                    else if (r1 == r2)
                        d = 1;
                    else if ((p1 - p2) % 7 == 0 && (p1 - p2) % 9 == 0) // diff == 63
                        d = 4;

                    else if ((p1 - p2) % 7 == 0)
                        d = 3;
                    else if ((p1 - p2) % 9 == 0)
                        d = 4;
                }

                direction[p1][p2] = d;
            }
    }

    {
        for (int p1 = 0; p1 < 64; p1++)
            for (int p2 = p1; p2 < 64; p2++)
            {
                squaresBetween[p1][p2] = 0;
                squaresBetween[p2][p1] = 0;

                int d = direction[p1][p2];

                if (!d)
                    continue;

                int add = 0;
                switch (d)
                {
                case 1:
                    add = 1; break;
                case 2:
                    add = 8; break;
                case 3:
                    add = 7; break;
                case 4:
                    add = 9; break;
                }

                int p = p1 + add;
                while (p < p2)
                {
                    squaresBetween[p1][p2] |= bbmask[p];
                    squaresBetween[p2][p1] |= bbmask[p];
                    p += add;
                }
            }
    }

    // whiteSquares: nur weisse Felder (~50%) des Schachbretts 
    whiteSquares = diagH8A1_attacks[48][0] |
                   diagH8A1_attacks[41][0] | 
                   diagH8A1_attacks[34][0] | 
                   diagH8A1_attacks[27][0] | 
                   diagH8A1_attacks[20][0] | 
                   diagH8A1_attacks[13][0] | 
                   diagH8A1_attacks[ 6][0] |
                   diagH1A8_attacks[48][0] |
                   bbmask[48];


    // ------- Freibauern ---------------------------------------------------
    {
        for (int i=0; i < 64; i++)
        {
            w_ppawn_mask[i] = b_ppawn_mask[i] = 0;

            int c = COL(i); 

            if (c==0)
            {
                int j;
                for (j=i-8; j>=0; j-=8)
                    w_ppawn_mask[i] |= bbmask[j] | bbmask[j+1];
                for (j=i+8; j<=63; j+=8)
                    b_ppawn_mask[i] |= bbmask[j] | bbmask[j+1];
            }
            else if (c==7)
            {
                int j;
                for (j=i-8; j>=0; j-=8)
                    w_ppawn_mask[i] |= bbmask[j] | bbmask[j-1];
                for (j=i+8; j<=63; j+=8)
                    b_ppawn_mask[i] |= bbmask[j] | bbmask[j-1];
            }
            else
            {
                int j;
                for (j=i-8; j>=0; j-=8)
                    w_ppawn_mask[i] |= bbmask[j] | bbmask[j-1] | bbmask[j+1];
                for (j=i+8; j<=63; j+=8)
                    b_ppawn_mask[i] |= bbmask[j] | bbmask[j-1] | bbmask[j+1];
            }
        }
    }


    // ------ Distanz ------------------------------------------------------------------
    {
        for (int i=0; i < 64; i++)
            for (int j=0; j < 64; j++)
                BBTables::dist[i][j] = calc_dist(i,j);
    }

    cout << "#Tables ready." << endl;

} // phew...


// -------------------------------------------------------------------------
//                   Definitionen zu class Bitboard
// -------------------------------------------------------------------------


// ------------------------------------------------------
//  g++ extended inline assembler code
// ------------------------------------------------------

#ifdef USE_ASM_MSBLSB
#ifdef LINUX
int Bitboard::MSB() const {
//       "bsrl    4(%1), %%edx \n\t"
//       "movl    $32,  %0 \n\t"
//       "jnz     0f \n\t"
//       "bsrl    (%1), %%edx \n\t"
//       "movl    $0, %0 \n\t"
//       "jnz     0f \n\t"
//       "movl    $64, %%edx \n\t"
//       "0: add  %%edx, %0\n\t"
   long res;
   asm volatile(
        "movq $0, %0   \n\t"
        "bsrq (%1), %0 \n\t"

        : "=&r" (res)
        : "r" (&bits)
        : "cc", "%edx"
   );
   return (int) res;
}

int Bitboard::LSB() const {
//       "bsfl    (%1), %%edx \n\t"
//       "movl    $0, %0 \n\t"
//       "jnz     1f \n\t"
//       "bsfl    4(%1), %%edx \n\t"
//       "movl    $32, %0 \n\t"
//       "jnz     1f \n\t"
//       "movl    $0, %0 \n\t"
//       "1: add  %%edx, %0\n\t"

   long res;
   asm volatile(
       "movq    $0, %0   \n\t"
       "bsfq    (%1), %0 \n\t"

       : "=&r" (res)
       : "r"  (&bits)
       : "cc"
   );
   return (int) res;
}
#endif
#endif  

