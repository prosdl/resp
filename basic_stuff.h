// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : basic_stuff.h
//                       Einige grundlegende Definitionen für RESP.
//
//  Anfang des Projekts: So, 8.Juli, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: basic_stuff.h,v 1.70 2003/05/19 20:17:33 rosendahl Exp $
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

#ifndef BASIC_STUFF_H
#define BASIC_STUFF_H

#ifdef USE_DEFINESH
#include "defines.h"
#endif
#include <iostream>
#include <ostream>
#include <fstream>


#ifndef LINUX
#pragma warning (disable:4786) // Visual C++ Faxen
#endif

// KONSTANTEN
const int  MAXPLY             =   512;
const int  MAX_MOVESTACK      =   192;
const int  MAX_HISTORY        =   512;
const int  ILLEGAL_MOVE_SCORE =-200000;
const int  INF_SCORE          = 100000;



// TYPEN
typedef unsigned char BYTE;

#ifdef LINUX
typedef unsigned long long BITBOARD;
typedef unsigned int UINTEGER32;
#else
typedef unsigned __int64 BITBOARD;
typedef unsigned __int32 UINTEGER32;
#endif 

#ifdef LINUX
#define FORCEINLINE inline
#else
#define FORCEINLINE __forceinline
#endif

typedef UINTEGER32 hash_t;


enum PieceEnum {
    NO_PIECE = 0,
    PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6
};

enum ColorEnum {
    BLACK = 1, WHITE = 2, EMPTY = 0
};

enum CastleEnum {
    CASTLE_WHITE_KINGSIDE   = 1,
    CASTLE_WHITE_QUEENSIDE  = 2,
    CASTLE_BLACK_KINGSIDE   = 4,
    CASTLE_BLACK_QUEENSIDE  = 8
};

enum RemisEnum {
    REMIS_STALE_MATE,
    REMIS_FIFTY,
    REMIS_REPITITION,
    REMIS_MATERIAL
};



// ------------- KLASSEN ----------------------------

// Wrapper für 32-Bit Integer
class UINTEGER32_WRAPPER
{
private:
    UINTEGER32 i;
public:
    UINTEGER32_WRAPPER() : i(0) {}
    UINTEGER32_WRAPPER(BYTE b3, BYTE b2, BYTE b1, BYTE b0) 
    {
        i = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
    }

    UINTEGER32 val() const { return i; }
};


/*class LsbMap
{
private:

public:
    BYTE map[67];
    void init();
    LsbMap();
}; */


// -------------------------------------------------------------------------
//            c l a s s  T e e B u f
//
//  Basierend auf einer Klasse, die in comp.lang.c++ von Dietmar Kuehl
//  geposted wurde. Ich benutze die Klasse um einen einfachen Logfile zu 
//  realisieren.
// -------------------------------------------------------------------------

class TeeBuf: public std::streambuf {
private:
    std::streambuf* log_sb;
    bool loggingEnabled;
  public:

    bool getLogging() const { return loggingEnabled; }
    void setLogging(bool enable)  { loggingEnabled = enable; }

    typedef std::char_traits<char> traits_type;
    typedef traits_type::int_type  int_type;

    TeeBuf(std::streambuf* _log_sb):
      log_sb(_log_sb)
    {
    }

    int_type overflow(int_type c) {

        if (!loggingEnabled)
        {
            if (!traits_type::eq_int_type(c, traits_type::eof())) 
            {
                c = std::cout.rdbuf()->sputc(c);
                return c;
            }
            else
                return traits_type::not_eof(c);
        }

        else
        {
            if (!traits_type::eq_int_type(c, traits_type::eof())) {
                c = log_sb->sputc(c);
                if (!traits_type::eq_int_type(c, traits_type::eof()))
                    c = std::cout.rdbuf()->sputc(c);
                return c;
            }
            else
                return traits_type::not_eof(c);
        }
    }

    int sync() {
          int rc = 0;
          if (loggingEnabled) rc = log_sb->pubsync();
          if (rc != -1)
              rc = std::cout.rdbuf()->pubsync();
          return rc;
    }
};




class TeeStream : public std::ostream
{
private:
    std::ofstream log;
    TeeBuf teebuf;
public:
    TeeStream (const char* logFileName);

    std::ofstream& logfile() { return log; }

    bool getLogging()       const { return teebuf.getLogging(); }
    void setLogging(bool enable)  { teebuf.setLogging(enable); }


};

extern TeeStream out;





// -------------------------------------------------------------------------
//            c l a s s  B y t e B o a r d
// -------------------------------------------------------------------------
//
//  Einfache Struktur, zum Speichern eines Schachbretts.
//  Wird nur als eine Art "allgemeines Format" für das Beschreiben einer
//  Schachposition benutzt (nicht von der Zuggenerierung).
class ByteBoard
{
public:
    BYTE color[64];
    BYTE piece[64];

public:
    ByteBoard() 
    {
        for (int i = 0; i < 64; i++)
            color[i] = piece[i] = 0;
    }

    ByteBoard(const ByteBoard& b)
    {
        for (int i=0; i < 64; i++)
        {
            color[i] = b.color[i];
            piece[i] = b.piece[i];
        }
    }

    ByteBoard& operator=(const ByteBoard& b)
    {
        for (int i=0; i < 64; i++)
        {
            color[i] = b.color[i];
            piece[i] = b.piece[i];
        }
        return *this;
    }

    void set(BYTE pos, BYTE c, BYTE p)
    {
        color[pos] = c; piece[pos] = p;
    }

    BYTE getPiece(BYTE pos) const
    {
        return piece[pos];
    }

    BYTE getColor(BYTE pos) const
    {
        return color[pos];
    }
};



// -------------------------------------------------------------------------
// EXTERNS .... GLOBALE TABELLEN
// -------------------------------------------------------------------------
extern int shiftRank[64];          // benötigt für GET_RANK

extern int rot90C_bitIndex[64];    // wohin wird Bit#i bei einer 90° Drehung im Uhrzeigersinn
                                   // ([C]lockwise) gedreht?

extern int rot90AC_bitIndex[64];    // wohin wird Bit#i bei einer 90° Drehung 
                                    //im Gegenuhrzeigersinn
                                    // ([A]nti[c]lockwise) gedreht?


extern int rot45C_bitIndex[64];
extern int rot45AC_bitIndex[64];
extern int shiftIndexDiag[64];
extern int maskDiag[64];
extern int diagLen[64];

extern int mailbox[];
extern int mailbox_address[];
extern int knight_moves[];
extern int king_moves[];

extern int castle_mask[];

extern int exch_score[7][7];

extern int piece_val[7];

extern int piece_set_bit_mask[7];
extern int piece_clear_bit_mask[7];


//  Tastatureingaben
int userInput();

// Datum Uhrzeit als String
char* dateString();
char* dateStringYYYYMMDD();
char* timeDateString();

// Zeit in Millisek.
long time_in_ms();


// ----------------------------------- INLINES -----------------------------

// Reihe / Spalte einer Position
inline BYTE COL(BYTE x) { return x & 7; }
inline BYTE ROW(BYTE x) { return  x >> 3; }

// Farbe des Gegners
inline ColorEnum XSIDE(const ColorEnum x) 
{ 
// return  (x==WHITE) ? BLACK : WHITE;   
   return (ColorEnum) ((int)x^3);
}

// ---------------------------------------
//  Abstand zweier Felder
// ---------------------------------------
inline int calc_dist(int x, int y)
{
    int d_col = abs(COL(x) - COL(y));
    int d_row = abs(ROW(x) - ROW(y));

    return (d_col > d_row) ? d_col : d_row;
}


// ---------------------- POS_DIAG_... ---------------------------------------
// Bestimme die Position von "pos" in der Diagonalmaske links oben nach rechts unten
inline BYTE POS_DIAG_H8A1(BYTE pos) 
{
    return (ROW(pos) >= COL(pos))  ? COL(pos) : ROW(pos);
}

// Bestimme die Position von "pos" in der Diagonalmaske links unten nach rechts oben
inline BYTE POS_DIAG_H1A8(BYTE pos) 
{
    return (7 - ROW(pos) >= COL(pos)) ? COL(pos) : 7 - ROW(pos);
}


//  ----------------------------------------------------------------------------
//  Umschalten der FPU
#ifdef USE_MMXASM
inline void clear_fpu()
{
#ifdef ATHLON
    __asm femms
#else
    __asm emms
#endif
}
#endif



#endif // BASIC_STUFF_H
