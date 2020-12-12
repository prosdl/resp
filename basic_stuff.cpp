// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : basic_stuff.cpp
//                       Tools für RESP.
//
//  Anfang des Projekts: So, 8.Juli, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: basic_stuff.cpp,v 1.36 2003/05/19 20:17:33 rosendahl Exp $
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

#include "basic_stuff.h"

#ifndef LINUX
#include <windows.h>
#include <conio.h>
#endif


#include <iostream>
#include <ctime> 
#include <sys/timeb.h>

using namespace std;



TeeStream::TeeStream (const char* logFileName) : log(logFileName), teebuf(log.rdbuf()),
 ostream(&teebuf)
{
}

TeeStream out("resp.log");

/*
BYTE MSB_(BITBOARD x)
{ 
    
    if (x >> 32)
        return (x >> 48) ? (msb16.a[x>>48] + 48) : (msb16.a[x>>32] +32) ;
    else
        return (x >> 16) ? (msb16.a[x>>16] + 16) : (msb16.a[x]);
    
}
*/

// LSB von "isolierten" Bits in einem Bitboard:
// map[ bb % 67 ] = LSB(bb) = x, FALLS bb = 2**(x-1), 1 <= x <= 64
//
// Funktioniert, da 2 Generator der multiplikativen Gruppe Z/67Z \ {0} ist;
// d.h.
//         PHI: {1,...,66} -> Z/67Z \ {0}
//
//               i        |-> 2**i + 67Z
//
//  ist eine Bijektion

/*LsbMap lsbmap;

LsbMap::LsbMap()
{
    init();
}

void LsbMap::init()
{
    BITBOARD b;

    for (int i=0; i < 64; i++)
    {
        b = (BITBOARD) 1 << i;
        map[b % 67] = i;
    }

} */




int shiftRank[64] = {  
    0, 0, 0, 0, 0, 0, 0, 0,
    8, 8, 8, 8, 8, 8, 8, 8,
   16,16,16,16,16,16,16,16,
   24,24,24,24,24,24,24,24,
   32,32,32,32,32,32,32,32,
   40,40,40,40,40,40,40,40,
   48,48,48,48,48,48,48,48,
   56,56,56,56,56,56,56,56

}; 

int rot90AC_bitIndex[64] = {
    56,48,40,32,24,16, 8, 0,
    57,49,41,33,25,17, 9, 1,
    58,50,42,34,26,18,10, 2,
    59,51,43,35,27,19,11, 3,
    60,52,44,36,28,20,12, 4,
    61,53,45,37,29,21,13, 5,
    62,54,46,38,30,22,14, 6,
    63,55,47,39,31,23,15, 7
};

// -----------------------------------------------------------------------
// rot90C_bitIndex:
//
// Es gilt:             BB90C (i) = BB ( rot90C_bitIndex[i] ) 
//
//                      für ein BITBOARD BB bei Index i.
//
// rot90AC_bitIndex ist die Umkehrfunktion zu rot90C_bitIndex.
// (Brauch ich die überhaupt??)

int rot90C_bitIndex[64] = {
     7,15,23,31,39,47,55,63,
     6,14,22,30,38,46,54,62,
     5,13,21,29,37,45,53,61,
     4,12,20,28,36,44,52,60,
     3,11,19,27,35,43,51,59,
     2,10,18,26,34,42,50,58,
     1, 9,17,25,33,41,49,57,
     0, 8,16,24,32,40,48,56
};

// ------------------------------------------------------
// rot45C_bitIndex
//
//      BB45C(i) = BB( rot45C_bitIndex[i] ) 
//
//      für ein Bitboard BB bei Index i.

int rot45C_bitIndex[64] = {
    28,21,15,10, 6, 3, 1, 0,
    36,29,22,16,11, 7, 4, 2,
    43,37,30,23,17,12, 8, 5,
    49,44,38,31,24,18,13, 9,
    54,50,45,39,32,25,19,14,
    58,55,51,46,40,33,26,20,
    61,59,56,52,47,41,34,27,
    63,62,60,57,53,48,42,35
};

int rot45AC_bitIndex[64] = {
    0, 2, 5, 9,14,20,27,35,
    1, 4, 8,13,19,26,34,42, 
    3, 7,12,18,25,33,41,48,
    6,11,17,24,32,40,47,53,
   10,16,23,31,39,46,52,57,
   15,22,30,38,45,51,56,60,
   21,29,37,44,50,55,59,62,
   28,36,43,49,54,58,61,63
};

// ------------------------------------------------------
// shiftIndexDiag / maskDiag
//
// Werden benötigt, um zu einem Index, i die Position der Diagonalmaske
// in einem BB45C wiederzufinden.
//
//

int shiftIndexDiag[64] = {
     0,
     1, 1,
     3, 3, 3,
     6, 6, 6, 6,
    10,10,10,10,10,
    15,15,15,15,15,15,
    21,21,21,21,21,21,21,
    28,28,28,28,28,28,28,28,
    36,36,36,36,36,36,36,
    43,43,43,43,43,43,
    49,49,49,49,49,
    54,54,54,54,
    58,58,58,
    61,61,
    63
};

int maskDiag[64] = {
      1,
      3,  3,
      7,  7,  7,
     15, 15, 15, 15,
     31, 31, 31, 31, 31,
     63, 63, 63, 63, 63, 63,
    127,127,127,127,127,127,127,
    255,255,255,255,255,255,255,255,
    127,127,127,127,127,127,127,
     63, 63, 63, 63, 63, 63,
     31, 31, 31, 31, 31,
     15, 15, 15, 15,
      7,  7,  7,
      3,  3,
      1
};


// Länge der Diagonalen  für Index i
int diagLen[64] = {
    8,7,6,5,4,3,2,1,
    7,8,7,6,5,4,3,2,
    6,7,8,7,6,5,4,3,
    5,6,7,8,7,6,5,4,
    4,5,6,7,8,7,6,5,
    3,4,5,6,7,8,7,6,
    2,3,4,5,6,7,8,7,
    1,2,3,4,5,6,7,8
};


// ----------------------------------------------
// mailbox
//
// Für i=0,..,63:
//          mailbox[mailbox_address[i]] =  i
//          mailbox[x]                  = -1, falls nicht x in 
//                                              IM(mailbox_address[.])

int mailbox[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7, -1,
    -1,  8,  9, 10, 11, 12, 13, 14, 15, -1,
    -1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
    -1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
    -1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
    -1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
    -1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
    -1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

int mailbox_address[] = {
    21,22,23,24,25,26,27,28,
    31,32,33,34,35,36,37,38,
    41,42,43,44,45,46,47,48,
    51,52,53,54,55,56,57,58,
    61,62,63,64,65,66,67,68,
    71,72,73,74,75,76,77,78,
    81,82,83,84,85,86,87,88,
    91,92,93,94,95,96,97,98
};

 
// -----------------------------------------
// knight_moves, king_moves
//
// ...zum Berechnen der Springerzüge
// (eigentlich nur zum Initialisieren der entsprechenden Bitboards -
//  muß aber ja auch gemacht werden ;) )
int knight_moves[] = {
    -21,-19,-12,-8,
     21, 19, 12, 8
};

int king_moves[] = {
    -11,-10,-9,-1,
     11, 10, 9, 1
};


// --------------------------------------------
// castle_mask
//
// Wird benötigt, um Rochade-Rechte zu aktualisieren.

int castle_mask[] = {
   7,15,15,15, 3,15,15,11,
  15,15,15,15,15,15,15,15,
  15,15,15,15,15,15,15,15,
  15,15,15,15,15,15,15,15,
  15,15,15,15,15,15,15,15,
  15,15,15,15,15,15,15,15,
  15,15,15,15,15,15,15,15,
  13,15,15,15,12,15,15,14
};



// Most valuable victim / least valuable attacker scores
int exch_score[7][7] = {
    //  -   P   N   B   R   Q   K
    //
    {   0,  0,  0,  0,  0,  0,    0},
    {   0,100,300,300,500,900,10000}, // P
    {   0, 99,299,299,499,899,10000}, // N
    {   0, 98,298,298,498,898,10000}, // B
    {   0, 97,297,297,497,897,10000}, // R
    {   0, 96,296,296,496,896,10000}, // Q
    {   0, 95,295,295,495,895,10000}  // K
};


// -------------------------------------------------------------------------
//  Materialwerte ... wird von der Evaluierungsklasse nachher uebschrieben
// -------------------------------------------------------------------------
int piece_val[7] = { 0, 100, 325, 325, 500, 950, 10000 };



// -------------------------------------------------------------------------
//  Bitmasken fuer Materialsignaturen
// -------------------------------------------------------------------------
int piece_set_bit_mask[7]   = {0,  1,  2,  4,  8, 16,0};
int piece_clear_bit_mask[7] = {0,254,253,251,247,239,0};


// -------------------------------------------------------------------------
//  userInput ... prüfen, ob Benutzer (z.B. WinBoard) eine Eingabe 
//  geschickt hat
// -------------------------------------------------------------------------
int userInput()
{
    // Die eleganteste Lösung für Pondering und Analyse Modus ist mit Sicherheit
    // Threading. Da meine Kenntnisse diesbezüglich noch nicht so ganz ausgereift
    // sind, verwende ich erstmal PeekNamedPipe; der Code dazu stammt aus
    // B.Hyatt's Crafty
#ifdef LINUX

    fd_set readfds;
    struct timeval tv;

    FD_ZERO (&readfds);
    FD_SET (0, &readfds);
    tv.tv_sec=0; tv.tv_usec=0;
    select (1, &readfds, 0, 0, &tv);

    return (FD_ISSET (0, &readfds));


#else  // WIN32
    // ---------------------------------------------------
    //        ***** Code taken from Crafty  *****
    // Many thanks to Robert Hyatt for allowing me to use
    // an adaption of CheckInput() for Windows from
    // Crafty's utility.c.
    // ---------------------------------------------------
    int i;
    static int init = 0, pipe;
    static HANDLE inh;
    DWORD dw;

    if (GlobalGameOptions::getHandle()->outStyle == GlobalGameOptions::XBOARD)
    {
    if (!init) {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(inh, &dw);
        if (!pipe) {
            SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(inh);
        }
    }

    if (pipe) {
        if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) {
            return 1;
        }
        return dw;
    } 
    else {
        GetNumberOfConsoleInputEvents(inh, &dw);
        return dw <= 1 ? 0 : dw;
    }
    }
    else
        i = _kbhit();
    return(i);

#endif
}

// ----------------------------------------------------------------------------
//  Datum/Uhrzeit als Zeichenkette in der Form: hhmmssddmmyyyy
// ----------------------------------------------------------------------------

char* timeDateString()
{
    time_t now;
    time(&now);
    struct tm* tmtime;
    tmtime = gmtime(&now);
    static char buf[64];
  
    sprintf(buf,"%02i%02i%02i%02i%02i%04i", tmtime->tm_hour, tmtime->tm_min, tmtime->tm_sec,
                 tmtime->tm_mday, tmtime->tm_mon + 1, 
                 1900 + tmtime->tm_year);
    return buf;
}

// ----------------------------------------------------------------------------
//  Datum als String
// ----------------------------------------------------------------------------
char* dateString()
{
    time_t now;
    time(&now);
    struct tm* tmtime;
    tmtime = gmtime(&now);
    static char buf[64];
  
    sprintf(buf,"%02i/%02i/%04i",tmtime->tm_mday, tmtime->tm_mon + 1, 
                 1900 + tmtime->tm_year);
    return buf;
}

// ----------------------------------------------------------------------------
//  Datum als String im Format YYYY.MM.DD
// ----------------------------------------------------------------------------
char* dateStringYYYYMMDD()
{
    time_t now;
    time(&now);
    struct tm* tmtime;
    tmtime = gmtime(&now);
    static char buf[64];
  
    sprintf(buf,"%04i.%02i.%02i",1900 + tmtime->tm_year, tmtime->tm_mon + 1, 
                  tmtime->tm_mday);
    return buf;
}


// ----------------------------------------------------------------------------
//  Zeit in Millisekunden bestimmen
// ----------------------------------------------------------------------------
long time_in_ms()
{
#ifdef USE_MMXASM
    clear_fpu();
#endif

    timeb t;
    ftime(&t);
    

    return static_cast<long> (t.time*1000 + t.millitm) ;
}


