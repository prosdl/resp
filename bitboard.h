// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : bitboard.h
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
// $Id: bitboard.h,v 1.28 2003/05/13 20:52:47 rosendahl Exp $
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
#ifndef BITBOARD_H
#define BITBOARD_H

#ifdef USE_DEFINESH
#include "defines.h"
#endif

#include "basic_stuff.h"

// -------------------------------------------------------------------------
//              c l a s s  B i t b o a r d T a b l e s
// -------------------------------------------------------------------------
//
// Diese Klasse enthält statische Tabellen, die für das Arbeiten mit
// Bitboards benötigt werden
class BBTables
{
private:
    static bool initialized;
public:

    // müssen von "Hand" initialisiert werden :-(
    static void init();

    // Bauernbewegungen
    static BITBOARD whitePawnMoves[64];
    static BITBOARD blackPawnMoves[64];
    static BITBOARD whitePawnCaps[64];
    static BITBOARD blackPawnCaps[64];

    // Isolieren einzelner Felder
    static BITBOARD bbmask[64];
    static BITBOARD bbmask45C[64];
    static BITBOARD bbmask45AC[64];
    static BITBOARD bbmask90C[64];

    static BITBOARD bbmaskF1G1;
    static BITBOARD bbmaskF8G8;
    static BITBOARD bbmaskB1C1D1;
    static BITBOARD bbmaskB8C8D8;
    static BITBOARD whiteSquares;

    static BITBOARD bbmaskBCDEFGH;
    static BITBOARD bbmaskABCDEFG;

    // Grosse Ecken
    static BITBOARD bbmaskA8B8A7B7;
    static BITBOARD bbmaskG8H8G7H7;
    static BITBOARD bbmaskA1B1A2B2;
    static BITBOARD bbmaskG1H1G2H2;


    // Linien isolieren
    static BITBOARD filemask[8];
    static BITBOARD rankmask[8];

    // Komplette Vertikalen/Horizontalen
    static BITBOARD filerank_mask[64];

    // Komplette Diagonalen
    static BITBOARD alldiag_mask[64];

    // Diagonalen + Vertikalen + Horizontalen
    static BITBOARD all_mask[64];

    // Angriffe auf einer Reihe
    static BITBOARD rank_attacks[64][256];
    // Angriffe auf einer Linie
    static BITBOARD file_attacks[64][256];
    // Angriffe auf H8A1
    static BITBOARD diagH8A1_attacks[64][256];
    // Angriffe auf H1A8
    static BITBOARD diagH1A8_attacks[64][256];
    // Springerbewegungen
    static BITBOARD knight_attacks[64];
    // Königsbewegungen
    static BITBOARD king_attacks[64];

    // Freibauern
    static BITBOARD w_ppawn_mask[64];
    static BITBOARD b_ppawn_mask[64];


    // Mobilität
    static int rank_mobility[64][256];
    static int file_mobility[64][256];
    static int diagH8A1_mobility[64][256];
    static int diagH1A8_mobility[64][256];



    // Popcount für Bytes
    static BYTE bitCountForByte[256];

    // Lookups für msb/lsb (16-Bit)
    static int msb16[65536];
    static int lsb16[65536];

    // Richtung eines Zuges von a nach b bestimmen
    static int direction[64][64];

    // Zwischenfelder eines Zuges von a nach b bestimmen:
    static BITBOARD squaresBetween[64][64];

    // Abstand zwischen zwei Feldern
    static int dist[64][64];
};


// -------------------------------------------------------------------------
//                      c l a s s  B i t b o a r d
// -------------------------------------------------------------------------
class Bitboard
{
private:
    // ---------------------------------------------------
    //  Bitboards werden in einen 64-Bit unsigned Typ
    //  gespeichert
    // ---------------------------------------------------

    BITBOARD bits;

public:
    // ---------------------------------------------------
    //  Konstruktoren / Zuweisung
    // ---------------------------------------------------
    FORCEINLINE Bitboard::Bitboard() : bits(0) 
    {}

    FORCEINLINE Bitboard::Bitboard(const BITBOARD& _bits) : bits(_bits) 
    {}

    FORCEINLINE Bitboard::Bitboard(const Bitboard& b) : bits(b.bits) 
    {}

    FORCEINLINE Bitboard& operator=(const Bitboard& b)
    {
        bits = b.bits;
        return *this;
    }


    // ---------------------------------------------------
    //                      MSB / LSB
    // ---------------------------------------------------
    //
    // Hole das most (least) significant Bit aus einem 
    // BITBOARD (==64-Bit unsigned integer)


private:
#if !defined(USE_ASM_MSBLSB) 

inline int MSB(const BITBOARD& x) const
{
  if ( x >> 32 )
      return ( x >> 48) ? BBTables::msb16[x>>48] + 48 
                        : BBTables::msb16[x>>32] +32;
  else
      return ( x >> 16) ? BBTables::msb16[x>>16] + 16 
                        : BBTables::msb16[x];
}

inline int LSB(const BITBOARD& x) const
{
    if ( x << 32 ) 
        return ( x << 48 ) ? BBTables::lsb16[0xffff & x]  
                           : BBTables::lsb16[0xffff & (x>>16)] + 16;
    else
        return ( x << 16 ) ? BBTables::lsb16[0xffff & (x>>32)] + 32 
                           : BBTables::lsb16[0xffff & (x>>48)] + 48;
} 

#else
  #ifdef LINUX
    //  g++ extended inline assembler code
    int MSB() const;
    int LSB() const;
  
#else  // Visual C++

    // MSB/LSB mit Assembler Code
    //
    // ANGELEHNT AN: ARASAN, JOHN DART; CRAFTY (R.HYATT)
    __forceinline int MSB(const unsigned __int64 a) const {
    #pragma warning(disable : 4035)
          __asm {
              
                bsr     edx, dword ptr a+4
                mov     eax, 32
                jnz     l1
                bsr     edx, dword ptr a
                mov     eax, 0
                jnz     l1
                mov     edx, 64
          l1:   add     eax, edx 

                ; Dann Corbits Trick, geposted in CCC
                ; von Dan Newmann 

                ;bsr eax, dword ptr a+4
                ;add eax, 32
                ;bsr eax, dword ptr a

            }
    }

    __forceinline int LSB(const unsigned __int64 a) const {
    #pragma warning(disable : 4035)
          __asm {
                bsf     edx, dword ptr a
                mov     eax, 0
                jnz     l1
                bsf     edx, dword ptr a+4
                mov     eax, 32
                jnz     l1
                xor     eax, eax
          l1:   add     eax, edx

                ; Dann Corbits Trick, geposted in CCC
                ; von Dan Newmann 

                ;bsf eax, dword ptr a+4
                ;add eax, 32
                ;bsf eax, dword ptr a


            }
    }
  #endif
#endif 

public:
    // ---------------------------------------------------
    //  Öffentliche MSB/LSB Funktionen
    // ---------------------------------------------------
    int msb() const {
#ifdef USE_ASM_MSBLSB
   #ifdef LINUX
      return MSB();
   #else
      return MSB(bits);
   #endif
#else
       return MSB(bits);
#endif
       // ... ugly, isn't it :)
    }
    
    int lsb() const  {
#ifdef USE_ASM_MSBLSB
   #ifdef LINUX
       return LSB();
   #else
       return LSB(bits);
   #endif
#else
       return LSB(bits);
#endif
    }

    // ---------------------------------------------------
    //  Popcount
    // ---------------------------------------------------

#if !defined(USE_MMXASM)
    int popcount() const
    {
        int sum = 0;
        BITBOARD bb = bits;
        while (bb)
        {
            sum += BBTables::bitCountForByte[bb & 0xff];
            bb >>= 8;
        }
        return sum;
    }

#else
    // ------------------------------------------------
    // MMX Popcount aus AMD-Athlon-Optimization-Manual
    // ------------------------------------------------

    //__declspec (naked) int __stdcall popcount ()
    __forceinline int __stdcall popcount ()
    {
        static const __int64 C55 = 0x5555555555555555;
        static const __int64 C33 = 0x3333333333333333;
        static const __int64 C0F = 0x0F0F0F0F0F0F0F0F;
        __asm {
        PUSH ESI
        MOV ESI, this
        MOVD MM0, [esi].bits
        PUNPCKLDQ MM0, [esi].bits+4

        MOVQ MM1, MM0               ;v
        PSRLD MM0, 1                ;v >> 1
        PAND MM0, [C55]             ;(v >> 1) & 0x55555555
        PSUBD MM1, MM0              ;w = v - ((v >> 1) & 0x55555555)
        MOVQ MM0, MM1               ;w
        PSRLD MM1, 2                ;w >> 2
        PAND MM0, [C33]             ;w & 0x33333333
        PAND MM1, [C33]             ;(w >> 2) & 0x33333333
        PADDD MM0, MM1              ;x = (w & 0x33333333) +
                                    ; ((w >> 2) & 0x33333333)
        MOVQ MM1, MM0               ;x
        PSRLD MM0, 4                ;x >> 4
        PADDD MM0, MM1              ;x + (x >> 4)
        PAND MM0, [C0F]             ;y = (x + (x >> 4) & 0x0F0F0F0F)
        PXOR MM1, MM1               ;0
        PSADBW MM0, MM1             ;sum across all 8 bytes
        MOVD EAX, MM0               ;result in EAX per calling
                                    ; convention
        POP ESI

        }
    }
#endif


    // -------------------------------------------------------------------------
    // Isoliere Spalte einer Position "pos" aus einem Bitboard
    // -------------------------------------------------------------------------

    inline int GET_RANK(int pos) const
    { 
        return static_cast<int> ( (bits >> shiftRank[pos]) & 0xff );  
    }


    // -------------------------------------------------------------------------
    // Isoliere Diagonalmaske einer Position "pos" (45C) aus einem (45C) Bitboard 
    // -------------------------------------------------------------------------
    inline int GET_DIAG_H8A1(int pos) const
    { 
        //return static_cast<int> ( (bits >> shiftIndexDiag[pos]) & maskDiag[pos] ) ;
        return static_cast<int> ( (bits >> shiftIndexDiag[pos]) & 0xff) ;
    }


    // -------------------------------------------------------------------------
    //  rotieren eines Bitboards um 90° im Uhrzeigersinn
    // -------------------------------------------------------------------------

    void rot90C()
    {
        BITBOARD b_rot = 0;

        for (int i = 0; i<64; i++)
            if (bits & BBTables::bbmask[i])
                b_rot |= BBTables::bbmask[rot90C_bitIndex[i]];

        bits = b_rot;
    }

    // -------------------------------------------------------------------------
    //  rotieren eines Bitboards um 45° im Uhrzeigersinn
    // -------------------------------------------------------------------------
    void rot45C()
    {
        BITBOARD b_rot = 0;

        for (int i=0; i<64; i++)
            if (bits & BBTables::bbmask[i])
                b_rot |= BBTables::bbmask[rot45C_bitIndex[i]];

        bits = b_rot;
    }

    // -------------------------------------------------------------------------
    //  rotieren eines Bitboards um 45° im Gegen-Uhrzeigersinn
    // -------------------------------------------------------------------------
    void rot45AC()
    {
        BITBOARD b_rot = 0;

        for (int i=0; i<64; i++)
            if (bits & BBTables::bbmask[i])
                b_rot |= BBTables::bbmask[rot45AC_bitIndex[i]];

        bits = b_rot;
    }

    // ---------------------------------------------------
    //  Zuweisung + Shift / Logik
    // ---------------------------------------------------

    FORCEINLINE void operator &= (const Bitboard& b)
    {
        bits &= b.bits;
        //return *this;
    }
    FORCEINLINE void operator |= (const Bitboard& b)
    {
        bits |= b.bits;
        //return *this;
    }
    FORCEINLINE void operator ^= (const Bitboard& b)
    {
        bits ^= b.bits;
        //return *this;
    }
    FORCEINLINE void operator <<= (const int i)
    {
        bits <<= i;
        //return *this;
    }
    FORCEINLINE void operator >>= (const int i)
    {
        bits >>= i;
        //return *this;
    }

    // ------------------------------------------------------
    //  Casting ... erlaubt das verwenden von &,|,~,^, ...
    //              für Bitboards.
    // ------------------------------------------------------

    operator BITBOARD() const
    {
        return bits;
    } 

}; 



#endif // BITBOARD_H
