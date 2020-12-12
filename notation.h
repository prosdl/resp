// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : notation.h
//                       SAN, FEN, EPD Import/Export
//
//  Anfang des Projekts: Mi, 1.Jan, 2002
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: notation.h,v 1.5 2002/06/11 19:04:44 rosendahl Exp $
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
#include "board.h"
#include <string>
#include <vector>
class Notation
{
public:

    enum SAN_ParseError {
        SAN_NO_ERROR = 0,
        SAN_SYNTAX_ERROR, 
        SAN_MOVE_NOT_POSSIBLE,
        SAN_AMBIGUOUS_MOVE
    };


    // FEN - Position laden
    static bool load_fen(std::string& fen, ByteBoard& byb, int& new_ep, int& new_fifty, 
                        int& new_castle, int& new_totalHPly, ColorEnum& new_side);

    // Zug --> SAN
    static std::string move_to_san(const Move& m, Board* pBoard);

    // SAN --> Zug
    static Move san_to_move(std::string san, SAN_ParseError& err, Board* pBoard);


    // EPD --> Brettposition

    enum OpCodeEnum {
        NOT_FOUND, AM, BM
    };

    static bool load_epdline(std::string line, std::string& id, OpCodeEnum& op_code,
        std::vector<Move>& move_list, Board* pBoard);

};
