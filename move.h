// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : move.h
//                       Header zu move.cpp
//
//  Anfang des Projekts: So, 8.Juli, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: move.h,v 1.32 2003/05/04 18:24:36 rosendahl Exp $
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

#ifndef MOVE_H
#define MOVE_H

#include "basic_stuff.h"

#include <string>
#include <vector>

// -------------------------------------------------------------------------
//              c l a s s    M o v e    
// -------------------------------------------------------------------------

// -------------------------------
//     Eigentliche Klasse Move
// -------------------------------
class Move
{
    // -------------------- enum --------------------------------------
public:
    enum SpecEnum { CAPTURE = 1, CASTLE = 2, EP_CAPTURE = 4, 
        PAWN_2SQUARES = 8, PAWN_MOVE = 16, PROMOTE = 32};

    
    // ------------ Member --------------------------------------------
public:
    // Daten des Schachzugs
    UINTEGER32 data; 

    // ------------  Konstruktoren ------------------------------------
public:
    // Standard
    Move() { };

    // Über from, to,  prom, special
    Move(int f, int t, int p, int s) 
    {  
        data = f | t << 8 | p << 16 | s << 24;
    }

    // -------------------------------------------------------------------------
    // Über 32-Bit Integer
    //
    //  Format         ssss ssss rrcc cppp tttt tttt ffff ffff
    //  mit:    f  - from
    //          t  - to
    //          p  - piece
    //          c  - captured piece
    //          r  - promotion piece (00 Springer, 01 Läufer, 10 Turm, 11 Dame)
    //          s  - special (siehe SpecEnum)
    // -------------------------------------------------------------------------
    Move(int in) : data(in)  { }

    // Copy
    Move(const Move& m) : data(m.data) {  }

    // ------------  Assignment ---------------------------------------
public:
    Move& operator=(const Move& m) { data = m.data; return *this; }


    // -------- Vergleich zweier Schachzüge ---------------------------
    // Zwei Züge m1,m2 gelten als gleich, wenn:
    //   1.) m1.from == m2.from
    //   2.) m1.to   == m2.to
    //   3.) falls bei m1.special PROMOTE gesetzt ist:
    //       m1.getPromPiece() = m2.getPromPiece()
private:
    bool comp_move(const Move& m) const
    {
        if ((data ^ m.data) & 0x0000ffff)
            return false;
        else
        {
            if (data & ( Move::PROMOTE << 24))
                return ((data ^ m.data) & 0x00c0ffff) == 0;
            else
                return true;
        }
    }

    friend bool operator==(const Move& m1, const Move& m2);
    friend bool operator!=(const Move& m1, const Move& m2);


public:
    // ------------  Methoden  ----------------------------------------

    // Umwandlung in Strings
    std::string toString() const;
    std::string specialToString() const;

    int isCapture() const
    {
        return data & (Move::CAPTURE << 24);
    }

    int isCastleMove() const
    {
        return data & (Move::CASTLE << 24);
    }

    int isPromotion() const
    {
        return data & (Move::PROMOTE << 24);
    }

    int isEpCapture() const
    {
        return data & (Move::EP_CAPTURE << 24);
    }

    int isPawnMove() const
    {
        return data & (Move::PAWN_MOVE << 24);
    }

    int isPawn2Squares() const
    {
        return data & (Move::PAWN_2SQUARES << 24);
    }

    int getCapturedPiece() const
    {
        return (data & 0x00380000) >> 19;
    }
    
    void setCapturedPiece(int p)
    {
        data = (data & 0xffc7ffff) | p << 19;
    }

    int getPiece() const
    {
        return (data >> 16) & 0x7;
    }

    void setPiece(int p)
    {
        data = (data & 0xfff8ffff) | p << 16;
    }

    int getPromPiece() const
    {
        //return ((data.by.promote & 0xC0) >> 6 ) + 2;
        return ((data & 0xC00000) >> 22 ) + 2;
    }

    void setPromPiece(int p)
    {
        //data.by.promote = (data.by.promote& 0x3f) | ((p-2) << 6);
        data = (data & 0xff3fffff) | ((p-2) << 22);
    }

    void setSpecial(int sp)
    {
        data = (data & 0x00ffffff) | sp << 24;
    }

    int to() const 
    { 
        return (data >>8) & 0x000000ff ; 
    }

    int from() const 
    { 
        return data & 0x000000ff; 
    }

    void setFrom(int from) 
    { 
        data &= 0xffffff00;
        data |= from;
    }
    void setTo(int to)
    { 
        data &= 0xffff00ff;
        data |= to<<8;
    }

    // Spezialfall Null Move
    static Move nullmove() { return Move(0); }
    bool isNullmove() const
    {
        return data == 0;
    }

    // dump nach stdout
    void dump();

    int mvv_lva() const
    {               
        return (data & 0x003f0000) ^ 0x00070000;
    }
};

inline std::ostream& operator<< (std::ostream& out, Move m)
{
    return out << m.toString();
}

inline bool operator==(const Move& m1, const Move& m2)
{
    return m1.comp_move(m2);
}

inline bool operator!=(const Move& m1, const Move& m2)
{
    return !m1.comp_move(m2);
}



// -------------------------------------------------------------------------
//              c l a s s   M o v e S t a c k
// -------------------------------------------------------------------------
class MoveStack
{
    // -------------- private Member ---------------------------------------
private:
    // Stackpointer
    int sp;

    // -------------- öffentliche Member -----------------------------------
    // Daten
public:

    // Zuweisung
    MoveStack& operator=(const MoveStack& ms)
    {
        for (int i=0; i < MAX_MOVESTACK; i++)
        {
            stack[i] = ms.stack[i];
            score[i] = ms.score[i];
            mat_gain[i] = ms.mat_gain[i];
        }
        sp = ms.sp;

        killer1 = ms.killer1;
        killer2 = ms.killer2;

        return *this;
    }

    Move stack[MAX_MOVESTACK];
    int score[MAX_MOVESTACK];
    int mat_gain[MAX_MOVESTACK];

    Move killer1;    // Killer-Moves
    Move killer2;

    // -------------- öffentliche Methoden ---------------------------------
public:
    // Konstruktor
    MoveStack() : sp (0) { }

    // --------------------------------------------
    //          Killer - Züge
    // --------------------------------------------

    // Killer zurücksetzen
    void resetKiller()
    {
        killer1 = killer2 = 0;
    }

    // Killer hinzufügen
    void addKiller(const Move& m)
    {
        // Captures nicht speichern, da sie ohnehin nach oben sortiert werden
        if (m.isCapture())
            return;

        if (killer1.data == m.data)
            return;

        killer2 = killer1;
        killer1 = m;
    }

    // Killer suchen
    int findKiller(const Move& m)
    {
        if (killer1 == m)
            return 1;
        if (killer2 == m)
            return 2;

        return 0;
    }

    // einzelner Zug auf Stack
    void push(const Move& m) 
    { 
        stack[sp++].data = m.data; 
    }

    // Umwandlungszüge auf Stack
    void pushProm(Move m)
    {
        m.setPromPiece(QUEEN);
        push(m);
        m.setPromPiece(ROOK);
        push(m);
        m.setPromPiece(KNIGHT);
        push(m);
        m.setPromPiece(BISHOP);
        push(m);
    }

    // Zug vom Stapel nehmen
    Move pop() { return stack[--sp]; }

    // Zurücksetzen
    void reset() { sp = 0;  }

    // Größe des Stacks
    int  size()  const { return sp; }

    // ------------------------------------------------------
    //    Vertausche die Züge mit den Indizes i und j 
    // ------------------------------------------------------
    void swap(int i, int j)
    {
        if (i == j)
            return;

        int tmp_score    = score[i];
        Move tmp_move    = stack[i];

        score[i]        = score[j];
        stack[i]        = stack[j];

        score[j]        = tmp_score;
        stack[j]        = tmp_move;
    }



    // Zugliste sortieren: Move m nach vorne
    bool swap_0_with(Move m);

    // Zug in der Liste finden und zurückgeben
    Move findMove(BYTE from, BYTE to, BYTE prom);

    // Teilmenge von Züge finden
    std::vector<Move> findMoves(BYTE from, BYTE to, 
          BYTE promote, bool use_from_file, bool use_from_rank);


    // Debug
    void dump() const;
};


bool moveInList(const Move& m, const std::vector<Move>& m_list);


// -------------------------------------------------------------------------
//                 c l a s s   H i s t H e u r i s t i c 
// -------------------------------------------------------------------------
class HistHeuristic
{
private:
    int move_score[4096];

public:
    HistHeuristic() { reset(); };

    void reset() { memset(move_score,0,sizeof(move_score)); }

    void add(const Move& m, int score)
    {
        move_score[m.from() | m.to()<<6] += score;
    }

    void inc(const Move& m)
    {
        ++move_score[m.from() | m.to()<<6];
    }

    int getScore(const Move& m) const
    {
        return move_score[m.from() | m.to()<<6];
    }
};


#endif // MOVE_H

