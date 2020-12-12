// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : board_mmx.cpp
//                       MMX-Assembler Code für board.cpp
//
//  Anfang des Projekts: Mi 17.Oktober, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: board_mmx.cpp,v 1.14 2002/06/11 19:04:44 rosendahl Exp $
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

 
#ifdef USE_MMXASM

#pragma warning (disable:4731)


// -------------------------------------------------------------------------
//  MMX - Routine, um zu Prüfen, ob das im Register mm7 übergebene
//  Bitboard weisse Türme oder Damen enthält
// -------------------------------------------------------------------------
__forceinline int __cdecl Board::mmx_hasWhiteQueenRook()
{
    __asm
    {
        ;    prüft, ob das Bitboard in mm7 eine weisse Dame oder einen weissen 
        ;   Turm enthält

        mov            esi,   this

        movq        mm0,  [esi].whiteQueens
        por         mm0,  [esi].whiteRooks    ; mm0 = whiteQueens | whiteRooks

        pand        mm0,  mm7                 ; mm0 = mm0 & 'Bitboard aus mm7'

        ; -- Prüfen, ob mm0 == 0 --

        ; NUR ATHLON!!!!
        ;pswapd      mm1,mm0
        ;por         mm0, mm1

        ; BLEND
        movq        mm1, mm0
        punpckldq   mm0, mm0
        punpckhdq   mm1, mm1
        por         mm0, mm1

        movd  eax, mm0

    }

 
}

// -------------------------------------------------------------------------
//  MMX - Routine, um zu Prüfen, ob das im Register mm7 übergebene
//  Bitboard weisse Läufer oder Damen enthält
// -------------------------------------------------------------------------
__forceinline int  __cdecl  Board::mmx_hasWhiteQueenBishop()
{
    __asm
    {
        ;    prüft, ob das Bitboard in mm7 eine weisse Dame oder einen weissen 
        ;   Läufer enthält

        mov            esi,   this

        movq        mm0,  [esi].whiteQueens
        por         mm0,  [esi].whiteBishops  ; mm0 = whiteQueens | whiteBishops
        pand        mm0,  mm7                 ; mm0 = mm0 & 'Bitboard aus mm7'

        ; -- Prüfen, ob mm0 == 0 --

        ; NUR ATHLON!!!!
        ;pswapd      mm1,mm0
        ;por         mm0, mm1

        ; BLEND
        movq        mm1, mm0
        punpckldq   mm0, mm0
        punpckhdq   mm1, mm1
        por         mm0, mm1

        movd  eax, mm0

    }
}



// -------------------------------------------------------------------------
//  MMX - Routine, um zu Prüfen, ob das im Register mm7 übergebene
//  Bitboard schwarze Türme oder Damen enthält
// -------------------------------------------------------------------------
__forceinline int __cdecl Board::mmx_hasBlackQueenRook()
{
    __asm
    {
        ;    prüft, ob das Bitboard in mm7 eine schwarze Dame oder einen schwarzen 
        ;   Turm enthält

        mov      esi,   this
        movq  mm0,  [esi].blackQueens
        por   mm0,  [esi].blackRooks
        pand  mm0,  mm7

        ; -- Prüfen, ob mm0 == 0 --

        ; NUR ATHLON!!!!
        ;pswapd      mm1,mm0
        ;por         mm0, mm1

        ; BLEND
        movq        mm1, mm0
        punpckldq   mm0, mm0
        punpckhdq   mm1, mm1
        por         mm0, mm1

        movd  eax, mm0

    }

}

// -------------------------------------------------------------------------
//  MMX - Routine, um zu Prüfen, ob das im Register mm7 übergebene
//  Bitboard schwarze Läufer oder Damen enthält
// -------------------------------------------------------------------------
__forceinline int  __cdecl  Board::mmx_hasBlackQueenBishop()
{
    __asm
    {
        ;    prüft, ob das Bitboard in mm7 eine schwarze Dame oder einen schwarzen 
        ;   Läufer enthält

        mov            esi,   this

        movq        mm0,  [esi].blackQueens
        por         mm0,  [esi].blackBishops  ; mm0 = blackQueens | blackBishops
        pand        mm0,  mm7                 ; mm0 = mm0 & 'Bitboard aus mm7'

        ; -- Prüfen, ob mm0 == 0 --

        ; NUR ATHLON!!!!
        ;pswapd      mm1,mm0
        ;por         mm0, mm1

        ; BLEND
        movq        mm1, mm0
        punpckldq   mm0, mm0
        punpckhdq   mm1, mm1
        por         mm0, mm1

        movd  eax, mm0

    }
}



// -------------------------------------------------------------------------
//  MMX-Assembler Version von rankFileAttacks
//
//  Das resultierende Bitboard wird im Register mm7 abgelegt.
// -------------------------------------------------------------------------
void __cdecl  Board::mmx_rankFileAttacks(int from_pos)
{
    // Zugriff auf static Member
    // (geht wahrscheinlich auch irgendwie anders??)
    static const BITBOARD (* const r_attacks) [256] = BBTables::rank_attacks;
    static const BITBOARD (* const f_attacks) [256] = BBTables::file_attacks;


    __asm 
    {
        push ebx ; frame pointer register
        ; --------------------------
        ;   Parameter laden
        ; --------------------------
        mov         esi,    this
        mov         eax,    from_pos
        ;
        ; --------------------------
        ;   Bitboards laden
        ; --------------------------
        movq        mm0,    [esi].whitePieces       ; mm0 = whitePIeces
        movq        mm1,    [esi].whitePieces90C    ; mm1 = whitePIeces90C
        ;
        mov         ecx,    rot90C_bitIndex[eax*4]  
        ;
        por         mm0,    [esi].blackPieces       ; mm0 = whitePieces | blackPieces
        por         mm1,    [esi].blackPieces90C    ; mm1 = whitePieces90C | blackPieces90C
        ;
        ; --------------------------
        ;   Verschieben
        ; --------------------------
        movd        mm2,    shiftRank[eax*4]
        psrlq       mm0,    mm2                     ; mm0 >>= shiftRank[from_pos]
        ;
        movd        mm3,    shiftRank[ecx*4]


        psrlq       mm1,    mm3                     ; mm1 >>= shiftRank[rot90C_bitIndex[from_pos]]
        ;
        ; --------------------------
        ;   Maske isolieren
        ; --------------------------

        punpcklbw   mm0,    mm1
        
        shl         eax,    11                      ; eax = from_pos*256*sizeof(BITBOARD)
        mov         ecx,    eax

        movd        ebx,   mm0


        movzx       edx,    bh
        and         ebx,    0xff



        ;
        ; -------------------------------------------
        ;   from_pos*256*sizeof(BITBOARD) berechnen
        ; -------------------------------------------
        ;
        ;        
        ; --------------------------------
        ;   Basisadressen berechnen
        ; --------------------------------
        add         eax,    r_attacks              ; eax = r_attacks[from_pos]
        add         ecx,    f_attacks               ; ecx = f_attacks[from_pos]
        ;
        ; --------------------------------
        ;   Bitboards vereinigen
        ; --------------------------------
        movq        mm7,    [eax + ebx*8]
        por         mm7,    [ecx + edx*8]

        pop ebx ; frame pointer register
    }

}

// -------------------------------------------------------------------------
//  MMX - Version von diagAttacks
//
//  Das resultierende Bitboard wird im Register mm7 übergeben
// -------------------------------------------------------------------------

void __cdecl Board::mmx_diagAttacks(int from_pos)
{
    static const BITBOARD (* const dH8A1_attacks) [256] = BBTables::diagH8A1_attacks;
    static const BITBOARD (* const dH1A8_attacks) [256] = BBTables::diagH1A8_attacks;

    __asm
    {
        push ebx ; frame pointer register
        ; -------------------------
        ;   Bitboards -> Register
        ; -------------------------

        mov     esi,  this
        mov     eax,  from_pos                  ; eax = from_pos

        movq    mm0,  [esi].whitePieces45C      ; mm0 = whitePieces45C
        movq    mm1,  [esi].whitePieces45AC     ; mm1 = whitePieces45AC

        por     mm0,  [esi].blackPieces45C      ; mm0 |= blackPieces45C
        por     mm1,  [esi].blackPieces45AC     ; mm1 |= blackPieces45AC


        ; --------------------------------
        ;   Verschieben
        ; --------------------------------
        mov     ebx,  rot45C_bitIndex[eax*4]    ; ebx = rot45C_bitIndex[from_pos]
        movd    mm2,  shiftIndexDiag[ebx*4]     ; mm2 = shiftIndexDiag[ebx]  
        mov     edx,  maskDiag[ebx*4]           ; edx = maskDiag[ebx]
        psrlq   mm0,  mm2                       ; mm0 >>= mm2

        mov     ecx,  rot45AC_bitIndex[eax*4]   ; cbx = rot45AC_bitIndex[from_pos]
        movd    mm3,  shiftIndexDiag[ecx*4]     ; mm3 = shiftIndexDiag[ecx]  
        mov     esi,  maskDiag[ecx*4]           ; esi = maskDiag[ecx]
        psrlq   mm1,  mm3                       ; mm1 >>= mm3

        ; --------------------------------
        ;   Maske isolieren
        ; --------------------------------

        movd    ebx,  mm0
        and     ebx,  edx                       ; ebx = mm0 & maskDiag[..]

        movd    edx,  mm1
        and     edx,  esi                       ; edx = mm1 & maskDiag[..]

        ; -------------------------------------------
        ;   from_pos*256*sizeof(BITBOARD) berechnen
        ; -------------------------------------------
        shl     eax,  11                        ; eax = from_pos*256*sizeof(BITBOARD)
        mov     ecx,  eax

        ; --------------------------------
        ;   Basisadressen berechnen
        ; --------------------------------
        add     eax,  dH8A1_attacks             ; eax = diagH8A1_attacks[from_pos]
        add     ecx,  dH1A8_attacks             ; ecx = diagH1A8_attacks[from_pos]

        movq    mm7,  [eax+ebx*8]               ; mm7 = diagH8A1_attacks[..][..] 


        ; --------------------------------
        ;   Bitboards vereinigen
        ; --------------------------------
        por     mm7,  [ecx+edx*8]               ; mm7 |= diagH1A8_attacks[..][..] 


        pop ebx ; frame pointer register
    }
}

// -------------------------------------------------------------------------
//  MMX - Version von getFullAttackBoard
//
//  Das resultierende Bitboard wird im Register mm7 übergeben
// -------------------------------------------------------------------------
void __cdecl Board::mmx_getFullAttackBoard(int pos)
{
    static const BITBOARD * _whitePawnCaps  = BBTables::whitePawnCaps;
    static const BITBOARD * _blackPawnCaps  = BBTables::blackPawnCaps;
    static const BITBOARD * _knight_attacks = BBTables::knight_attacks;
    static const BITBOARD * _king_attacks   = BBTables::king_attacks;

    // diagAttacks[pos] & (whiteBishopsQueens | blackBishopsQueens) --> mm7
    mmx_diagAttacks(pos);
    __asm{
        movq    mm6,    mm7         ; mm7 retten
    }

    // rankFileAttacks --> mm7
    mmx_rankFileAttacks(pos);

    __asm {
        push    ebx                 ; frame pointer register
        mov     esi,    this
        mov     eax,    pos 

        ; blackPawnsCaps[pos] & whitePawns --> mm0
        mov     ecx,    _blackPawnCaps
        movq    mm0,    [ecx + 8*eax]
        pand    mm0,    [esi].whitePawns

        ; whitePawnsCaps[pos] & blackPawns --> mm1
        mov     ecx,    _whitePawnCaps
        movq    mm1,    [ecx + 8*eax]
        pand    mm1,    [esi].blackPawns

        ; knightAttacks[pos]& (whiteKnights | blackKnights) --> mm2
        mov     ecx,    _knight_attacks
        movq    mm2,    [ecx + 8*eax]
        movq    mm3,    [esi].whiteKnights
        por     mm3,    [esi].blackKnights
        pand    mm2,    mm3

        ; king_attacks[pos] & (whiteKing | blackKing) --> mm3
        mov     ecx,    _king_attacks
        movq    mm3,    [ecx + 8*eax]
        movq    mm4,    [esi].whiteKing
        por     mm4,    [esi].blackKing
        pand    mm3,    mm4

        ;
        ; mm6 & (whiteBishopsQueens | blackBishopsQueens) --> mm6
        movq    mm4,    [esi].blackBishops
        por     mm4,    [esi].blackQueens
        por     mm4,    [esi].whiteBishops
        por     mm4,    [esi].whiteQueens
        pand    mm6,    mm4

        ; mm7 & (whiteRooksQueens | blackRooksQueens) --> mm7
        movq    mm4,    [esi].blackRooks
        por     mm4,    [esi].blackQueens
        por     mm4,    [esi].whiteRooks
        por     mm4,    [esi].whiteQueens
        pand    mm7,    mm4

        ; mm0 | mm1 | mm2 | mm3 | mm6 | mm7 --> mm7
        por     mm1,    mm0
        por     mm3,    mm2
        por     mm7,    mm6
        por     mm3,    mm1
        por     mm7,    mm3

        pop  ebx
    }
}



 void __cdecl Board::mmx_toggleWhitePieceOnBitboards(int pos, int piece)
{
    static const BITBOARD * _bbmask = BBTables::bbmask;

    __asm
    {
        push ebx ; frame pointer register
        mov     esi,    this
        mov     eax,    pos

        mov     ecx,    _bbmask
        mov     edi,    ecx

        movq    mm4, [esi].whitePieces
        mov     edx, rot45C_bitIndex[eax*4]
        movq    mm5, [esi].whitePieces45C

        movq    mm6, [esi].whitePieces45AC
        movq    mm7, [esi].whitePieces90C

        pxor    mm4,    [ecx + eax*8]
        mov     ebx,    rot45AC_bitIndex[eax*4]
        pxor    mm5,    [edi + edx*8]
        mov     edx,    rot90C_bitIndex[eax*4]
        pxor    mm6,    [ecx + ebx*8]
        pxor    mm7,    [edi + edx*8]

        movq    [esi].whitePieces,      mm4
        movq    [esi].whitePieces45C,   mm5

        movq    [esi].whitePieces45AC,  mm6
        movq    [esi].whitePieces90C,   mm7
        
        cmp     piece, PAWN
        jnz     JmpKnight
        movq    mm0,                [esi].whitePawns
        pxor    mm0,                [ecx + eax*8]
        movq    [esi].whitePawns,   mm0
        jmp     Finished

JmpKnight:
        cmp     piece, KNIGHT
        jnz     JmpBishop
        movq    mm0,                [esi].whiteKnights
        pxor    mm0,                [ecx + eax*8]
        movq    [esi].whiteKnights, mm0
        jmp     Finished

JmpBishop:
        cmp     piece, BISHOP
        jnz     JmpRook
        movq    mm0,                [esi].whiteBishops
        pxor    mm0,                [ecx + eax*8]
        movq    [esi].whiteBishops, mm0
        jmp     Finished

JmpRook:
        cmp     piece, ROOK
        jnz     JmpQueen
        movq    mm0,                [esi].whiteRooks
        pxor    mm0,                [ecx + eax*8]
        movq    [esi].whiteRooks,   mm0
        jmp     Finished

JmpQueen:
        cmp     piece, QUEEN
        jnz     Finished
        movq    mm0,                [esi].whiteQueens
        pxor    mm0,                [ecx + eax*8]
        movq    [esi].whiteQueens,  mm0


Finished:

        pop ebx ; frame pointer register
    }

} 

void __cdecl Board::mmx_toggleBlackPieceOnBitboards(int pos, int piece)
{
    static const BITBOARD * _bbmask = BBTables::bbmask;

    __asm
    {
        push ebx ; frame pointer register

        mov     esi,    this
        mov     eax,    pos

        mov     ecx,    _bbmask
        mov     edi,    ecx

        movq    mm4, [esi].blackPieces
        mov     edx, rot45C_bitIndex[eax*4]
        movq    mm5, [esi].blackPieces45C

        movq    mm6, [esi].blackPieces45AC
        movq    mm7, [esi].blackPieces90C

        pxor    mm4,    [ecx + eax*8]
        mov     ebx,    rot45AC_bitIndex[eax*4]
        pxor    mm5,    [edi + edx*8]
        mov     edx,    rot90C_bitIndex[eax*4]
        pxor    mm6,    [ecx + ebx*8]
        pxor    mm7,    [edi + edx*8]

        movq    [esi].blackPieces,      mm4
        movq    [esi].blackPieces45C,   mm5

        movq    [esi].blackPieces45AC,  mm6
        movq    [esi].blackPieces90C,   mm7
        
        cmp     piece, PAWN
        jnz     JmpKnight
        movq    mm0,                [esi].blackPawns
        pxor    mm0,                [ecx + eax*8]
        movq    [esi].blackPawns,   mm0
        jmp     Finished

JmpKnight:
        cmp     piece, KNIGHT
        jnz     JmpBishop
        movq    mm0,                [esi].blackKnights
        pxor    mm0,                [ecx + eax*8]
        movq    [esi].blackKnights, mm0
        jmp     Finished

JmpBishop:
        cmp     piece, BISHOP
        jnz     JmpRook
        movq    mm0,                [esi].blackBishops
        pxor    mm0,                [ecx + eax*8]
        movq    [esi].blackBishops, mm0
        jmp     Finished

JmpRook:
        cmp     piece, ROOK
        jnz     JmpQueen
        movq    mm0,                [esi].blackRooks
        pxor    mm0,                [ecx + eax*8]
        movq    [esi].blackRooks,   mm0
        jmp     Finished

JmpQueen:
        cmp     piece, QUEEN
        jnz     Finished
        movq    mm0,                [esi].blackQueens
        pxor    mm0,                [ecx + eax*8]
        movq    [esi].blackQueens,  mm0


Finished:

        pop ebx ; frame pointer register
    }

} 


// -------------------------------------------------------------------------
//  Aktualisieren von weissen Bitboards beim Zug from,to
// -------------------------------------------------------------------------

void __cdecl Board::mmx_updateWhitePieces(int from, int to)
{
    static const BITBOARD * _bbmask     = BBTables::bbmask;
    static const BITBOARD * _bbmask45C  = BBTables::bbmask45C;
    static const BITBOARD * _bbmask45AC = BBTables::bbmask45AC;
    static const BITBOARD * _bbmask90C  = BBTables::bbmask90C;

    __asm
    {
        push ebx ; frame pointer register

        mov     esi,        this

        ; Bitboards laden
        movq    mm0, [esi].whitePieces
        movq    mm1, [esi].whitePieces45C
        movq    mm2, [esi].whitePieces45AC
        movq    mm3, [esi].whitePieces90C

        mov     eax, to

        mov     ebx, _bbmask
        mov     ecx, _bbmask45C
        mov     edx, _bbmask45AC
        mov     edi, _bbmask90C



        movq    mm4,    [ebx + eax*8]           ; bbmask[to]
        movq    mm5,    [ecx + eax*8]           ; bbmask[rot45C_bitIndex[to]]
        movq    mm6,    [edx + eax*8]           ; bbmask[rot45AC_bitIndex[to]]
        movq    mm7,    [edi + eax*8]           ; bbmask[rot90C_bitIndex[to]]

        mov     eax, from

        por     mm4,    [ebx + eax*8]
        por     mm5,    [ecx + eax*8]           ; bbmask[rot45C_bitIndex[from]]
        por     mm6,    [edx + eax*8]           ; bbmask[rot45AC_bitIndex[from]]
        por     mm7,    [edi + eax*8]           ; bbmask[rot90C_bitIndex[from]]

        pxor    mm0,    mm4
        pxor    mm1,    mm5
        pxor    mm2,    mm6
        pxor    mm3,    mm7

        ; Bitboards speichern
        movq    [esi].whitePieces,          mm0
        movq    [esi].whitePieces45C,       mm1
        movq    [esi].whitePieces45AC,      mm2
        movq    [esi].whitePieces90C,       mm3

        pop ebx ; frame pointer register

    }

}


// -------------------------------------------------------------------------
//  Aktualisieren von schwarzen Bitboards beim Zug from,to
// -------------------------------------------------------------------------

void __cdecl Board::mmx_updateBlackPieces(int from, int to)
{
    static const BITBOARD * _bbmask     = BBTables::bbmask;
    static const BITBOARD * _bbmask45C  = BBTables::bbmask45C;
    static const BITBOARD * _bbmask45AC = BBTables::bbmask45AC;
    static const BITBOARD * _bbmask90C  = BBTables::bbmask90C;

    __asm
    {
        push ebx ; frame pointer register

        mov     esi,        this

        ; Bitboards laden
        movq    mm0, [esi].blackPieces
        movq    mm1, [esi].blackPieces45C
        movq    mm2, [esi].blackPieces45AC
        movq    mm3, [esi].blackPieces90C

        mov     eax, to

        mov     ebx, _bbmask
        mov     ecx, _bbmask45C
        mov     edx, _bbmask45AC
        mov     edi, _bbmask90C



        movq    mm4,    [ebx + eax*8]           ; bbmask[to]
        movq    mm5,    [ecx + eax*8]           ; bbmask[rot45C_bitIndex[to]]
        movq    mm6,    [edx + eax*8]           ; bbmask[rot45AC_bitIndex[to]]
        movq    mm7,    [edi + eax*8]           ; bbmask[rot90C_bitIndex[to]]

        mov     eax, from

        por     mm4,    [ebx + eax*8]
        por     mm5,    [ecx + eax*8]           ; bbmask[rot45C_bitIndex[from]]
        por     mm6,    [edx + eax*8]           ; bbmask[rot45AC_bitIndex[from]]
        por     mm7,    [edi + eax*8]           ; bbmask[rot90C_bitIndex[from]]

        pxor    mm0,    mm4
        pxor    mm1,    mm5
        pxor    mm2,    mm6
        pxor    mm3,    mm7

        ; Bitboards speichern
        movq    [esi].blackPieces,          mm0
        movq    [esi].blackPieces45C,       mm1
        movq    [esi].blackPieces45AC,      mm2
        movq    [esi].blackPieces90C,       mm3

        pop ebx ; frame pointer register
    }

}


// -------------------------------------------------------------------------
//  mm7 Register in Bitboard laden
// -------------------------------------------------------------------------
__forceinline BITBOARD __cdecl Board::mmx_m7ToBitboard()
{
    BITBOARD bb;

    __asm{
    movq    [bb],     mm7
    }

    return bb;
}

/*
// -------------------------------------------------------------------------
//  MMX-Assembler Version von rankFileAttacks
//
//  Das resultierende Bitboard wird im Register mm7 abgelegt.
// -------------------------------------------------------------------------
void __cdecl  Board::mmx_rankQueenRookAttacks(int from_pos)
{
    static const BITBOARD (* const r_attacks) [256] = BBTables::rank_attacks;

    __asm 
    {
        push ebx ; frame pointer register
        ; --------------------------
        ;   Parameter laden
        ; --------------------------
        mov         esi,    this
        mov         eax,    from_pos
        ;
        ; --------------------------
        ;   Bitboards laden
        ; --------------------------
        movq        mm0,    [esi].whitePieces       ; mm0 = whitePIeces
        por         mm0,    [esi].blackPieces       ; mm0 = whitePieces | blackPieces
        ;
        ; --------------------------
        ;   Verschieben
        ; --------------------------
        movd        mm2,    shiftRank[eax*4]
        psrlq       mm0,    mm2                     ; mm0 >>= shiftRank[from_pos]
        ;
        ;
        ; --------------------------
        ;   Maske isolieren
        ; --------------------------
        shl         eax,    11                      ; eax = from_pos*256*sizeof(BITBOARD)
        movd        ebx,    mm0
        and         ebx,    0xff
        ;
        ; --------------------------------
        ;   Basisadressen berechnen
        ; --------------------------------
        add         eax,    r_attacks               ; eax = r_attacks[from_pos]
        ;
        movq        mm7,    [eax + ebx*8]
        ;
        movq        mm0,    [esi].whiteRooks
        movq        mm1,    [esi].blackRooks
        por         mm0,    [esi].whiteQueens
        por         mm1,    [esi].blackQueens
        por         mm0,    mm1
        pand        mm7,    mm0 
        ;
        pop ebx ; frame pointer register
    }

}


// -------------------------------------------------------------------------
void __cdecl  Board::mmx_fileQueenRookAttacks(int from_pos)
{
    static const BITBOARD (* const f_attacks) [256] = BBTables::file_attacks;


    __asm 
    {
        push ebx ; frame pointer register
        ; --------------------------
        ;   Parameter laden
        ; --------------------------
        mov         esi,    this
        mov         eax,    from_pos
        ;
        ; --------------------------
        ;   Bitboards laden
        ; --------------------------
        movq        mm1,    [esi].whitePieces90C    ; mm1 = whitePIeces90C
        mov         ecx,    rot90C_bitIndex[eax*4]  
        por         mm1,    [esi].blackPieces90C    ; mm1 = whitePieces90C | blackPieces90C
        ;
        ; --------------------------
        ;   Verschieben
        ; --------------------------
        movd        mm3,    shiftRank[ecx*4]
        psrlq       mm1,    mm3                     ; mm1 >>= shiftRank[rot90C_bitIndex[from_pos]]
        ;
        ; --------------------------
        ;   Maske isolieren
        ; --------------------------
        ;
        shl         eax,    11                      ; eax = from_pos*256*sizeof(BITBOARD)
        movd        ebx,    mm1
        and         ebx,    0xff
        ; --------------------------------
        ;   Basisadressen berechnen
        ; --------------------------------
        add         eax,    f_attacks               ; ecx = f_attacks[from_pos]
        ;
        movq        mm7,    [eax + ebx*8]
        ;
        movq        mm0,    [esi].whiteRooks
        movq        mm1,    [esi].blackRooks
        por         mm0,    [esi].whiteQueens
        por         mm1,    [esi].blackQueens
        por         mm0,    mm1
        pand        mm7,    mm0 
        ;
        pop ebx ; frame pointer register
    }

} */






#endif // USE_MMXASM
