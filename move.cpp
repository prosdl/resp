// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : move.cpp
//                       Stellt Klassen für die Realisierung von Schachzügen 
//                       zur Verfügung.
//
//  Anfang des Projekts: So, 31.August, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: move.cpp,v 1.21 2002/06/11 19:04:44 rosendahl Exp $
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

#include "move.h"
#include "board.h"

#include <iostream>

using namespace std;


/////////////////////////////////////////////////////////////
//              KLASSE Move                                //
/////////////////////////////////////////////////////////////

string Move::toString() const
{
    string s = "";

    s += (char) (COL(from()) + 'a');
    s += (char) (8-ROW(from()) + '0');
    s += (char) (COL(to()) + 'a');
    s += (char) (8-ROW(to()) + '0');

    if (isPromotion())
    {
        switch(getPromPiece())
        {
        case QUEEN:
            s += 'q'; break;
        case ROOK:
            s += 'r'; break;
        case KNIGHT:
            s += 'n'; break;
        case BISHOP:
            s += 'b'; break;
        }
    }

    return s;
}

string Move::specialToString() const
{
    string s = "";

    if (isCapture())
        s += "x ";

    if (isCastleMove())
        s += "CSTL ";

    if (isEpCapture())
        s += "(ep) ";

    if (isPawn2Squares())
        s += "P2 ";

    if (isPawnMove())
        s += "PAWN ";

    if (isPromotion())
    {
        s += "PROM ";
        s += Board::pieceChar[getPromPiece()];
    }

    if (data&0xff000000 == 0)
        s = "---";

    return s;
}

void Move::dump()
{
    cout << "MOVE: " + toString() << " --- ";
    cout << "PROMOTE = " << getPromPiece() << ", SPECIAL = " << specialToString() << endl;
}


bool moveInList(const Move& m, const std::vector<Move>& m_list)
{
    for (int i = 0; i < static_cast<int>(m_list.size()); i++)
    {
        if (m_list[i] == m)
        {
            return true;
        }
    }
    return false;
}


/////////////////////////////////////////////////////////////
//              KLASSE MoveStack                           //
/////////////////////////////////////////////////////////////

void MoveStack::dump() const
{
    int i;

    for (i=0; i < sp; i++)
    {

        if ((i+1) % 15 == 0)
        {
            string s; char c;
            cout << "WEITER (j/n)? ";
            s = "";
            while ((c = cin.get()))
            {
                if (c == '\n')
                    break;
                s += c;

            }

            if (s[0] != 'j') break;
        }
        cout << i << " - " << stack[i].toString() << ", " << stack[i].specialToString();
        cout << endl;
    }
    cout << endl;

}

/////////////////////////////////////////////////////////////
// findMove
//
// Suche Zug (from, to, prom). Falls prom != 0, gibt es
// eine Bauernumwandlung nach Figur prom.
// 
Move MoveStack::findMove(BYTE from, BYTE to, BYTE prom)
{
    BYTE prom_bit;
    if (prom)
        prom_bit = Move::PROMOTE;
    else
        prom_bit = 0;

    Move m(from,to,(prom-2) << 6,prom_bit);
//    m.data.by.from      = from;
//    m.data.by.to        = to;


    for (int i=0; i < sp; i++)
        if ( stack[i] == m )
            return stack[i];

    m.data = 0;
    return m;
}

// -------------------------------------------------------------------------
//  findMoves.....   
//
//  Sucht alle Züge mit Quellfeldern, die zum Zielfeld 'to' 
//  passen. 
//  use_from_file==true:  die Spalte von from ist gültig
//  use_from_rank==true:  die Zeile von from ist gültig
// -------------------------------------------------------------------------

vector<Move> MoveStack::findMoves(BYTE from, BYTE to, BYTE prom,
                                  bool use_from_file, bool use_from_rank)
{
    vector<Move> moveSet;

    BYTE from_filter = 0xff;

    if (!use_from_file ) // Spalte ignorieren
        from_filter &= 56; // = 00111000
    if (!use_from_rank) // Zeile ignorieren 
        from_filter &= 7;  // = 00000111

    for (int i=0; i < sp; i++)
        if (prom)
        {
            if ( stack[i].to() == to &&
                 ! ((stack[i].from() ^ from) & from_filter) &&
                  stack[i].getPromPiece() == prom )

                moveSet.push_back(stack[i]);
        }
        else
        {
            if ( (stack[i].to() == to) &&
                 ! ((stack[i].from() ^ from) & from_filter) )
                    
                moveSet.push_back(stack[i]);
        }
    
    return moveSet;
}

// -------------------------------------------------------------------------
//  swap_0_with(Move m) ... tauscht den Zug m mit dem Zug mit Index 0 in 
//                          der Zugliste. Falls m nicht gefunden:
//                          Rückgabewert == false
// -------------------------------------------------------------------------

bool MoveStack::swap_0_with(Move m)
{
    bool found = false;
    int i;


    for (i = 0; i < sp; i++)
        if ( stack[i] == m)
        {
            found = true;
            break;
        }


    if (found)
    {
        if (i != 0)
        {
            Move temp       = stack[i];
            int  temp_score = score[i];
            int  temp_mat   = mat_gain[i];

            stack[i]    = stack[0];
            score[i]    = score[0];
            mat_gain[i] = mat_gain[0];

            stack[0]    = temp;
            score[0]    = temp_score;
            mat_gain[0] = temp_mat;
        }
    }

    return found;
}

