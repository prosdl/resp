//////////////////////////////////////////////////////
//               pgnparse.h
//               ----------
//
//  Einlesen von *.pgn Dateien.
// --------------------------------------------------
//
//  AUTOR              : P.Rosendahl
//  SPRACHE            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  DATUM              : 28.07.2001
//  VERSION            : 0.19
//
// Letzte Änderung: 28.07.01
//
//////////////////////////////////////////////////////
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: pgnparse.h,v 1.14 2003/06/02 18:12:53 rosendahl Exp $
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


#ifndef PGNPARSE_H
#define PGNPARSE_H

#ifndef LINUX
#pragma warning(disable:4786)
#endif

#include <fstream>
#include <string>
#include <map>
#include <vector>
#include "board.h"


inline bool IS_CONT_CHAR(char c)
{
    return isalnum(c) || (c=='+') || (c=='_') || (c=='#') || (c=='=') || (c==':') || (c=='-');
}




enum TokenID {
    STRING,             // "*"
    INTEGER,            // [0-9][0-9]*
    PERIOD,             // .
    ASTERISK,           // *
    LBRACKET,           // [
    RBRACKET,           // ]
    LPARENTHESIS,       // (
    RPARENTHESIS,       // )
    LANGLE,             // <
    RANGLE,             // >
    NAG,                // $[0-9][0-9]*
    SYMBOL,             // [a-zA-Z] CONTINUATION_CHAR | [0-9] CONTINUATION_CHAR
                        // mit: CONTINUATION_CHAR -> [a-zA-Z] | [0-9] | [+_#=:-]
    GAME_TERMINATION,   // 1-0 | 0-1 | 1/2-1/2 | *
    DONE,
    UNKNOWN
};

enum GameResults {
    WHITE_WIN,
    BLACK_WIN,
    REMIS,
    UNDEFINED
};


//////////////////////////////////////////////////////
//                  TagList
//////////////////////////////////////////////////////

typedef std::map<std::string,std::string> TagList;

//////////////////////////////////////////////////////
//                  MoveList
//////////////////////////////////////////////////////

typedef std::vector<Move> MoveList;

//////////////////////////////////////////////////////
//                  ParseErrorException
//////////////////////////////////////////////////////

class ParseErrorException 
{
public:
    enum ErrorStatus {
        NO_ERROR = 0,
        PARSE_ERROR,
        SAN_MOVE_ERROR,
        FILE_ERROR
    };

private:
    const char* reason;
    ErrorStatus status;

public:
    ParseErrorException(const char* why, ErrorStatus status) 
        : reason(why), status(status) {}

    const char* what() const { return reason; }
    ErrorStatus getStatus() { return status;  }
};


//////////////////////////////////////////////////////
//                  class Token - DECLARATION
//////////////////////////////////////////////////////

class Token
{
public:
    std::string value;
    TokenID id;

public:
    std::string dumpToString();
};

//////////////////////////////////////////////////////
//              class PGN_Parser - DECLARATION
//////////////////////////////////////////////////////
class PGN_Parser
{

// ------------- private members --------------------
private:
    PGN_Parser();
    static PGN_Parser* p_instance;

    Token lookahead;    // ausgelesenes Token
    int lineno;         // aktuelle Zeilennummer
    std::fstream file;  // zum Öffnen der PGN Datei

    // Hash zum Speichern der Spielende Markierungen: "1-0", "0-1", u.s.w.
    static std::map<std::string,std::string> gameTerminationMarks;
    bool mapInitialized;

    TagList taglist;  // Hash mit allen Tags zum aktuell geparsten
                      // Spiel

    MoveList movelist; // Züge der Partie
    GameResults gameResult; // Ergebnis der Partie

    Board* pBoard;    // Board zum Auswerten der SAN-Züge

    // Status des PGN Parsers
    bool created;
    bool file_open;
    //PGN_ErrorStatus status;

    // Log-Datei
    std::fstream logfile;
    bool use_log;
// ------------- public members ---------------------
public:


// ------------- private methods --------------------
private:
    void getNextToken();
    void initGameTerminationMarks();

    void match(TokenID tokid);

    //----------------------------------------------
    //  pgn_database -> eps | pgn_database pgn_game
    void PGN_Parser::pgn_database();
    
    //----------------------------------------------
    // pgn_game -> tagpair_section movetext_section
    void PGN_Parser::pgn_game();
    
    //-------------------------------------------------
    //  tagpair_section -> eps | tagpair_section tagpair
    void tagpair_section();
    
    //-------------------------------------------------
    //  tagpair -> [ symbol string ]
    void tagpair();
    
    //-------------------------------------------------
    //  movetext_section -> move_text |GAME_TERMINATION   
    void movetext_section();
    
    //-------------------------------------------------
    //  move_text -> movenumber_indication san_move |
    //               san_move | move_text move_text
    void move_text();
    
    //-------------------------------------------------
    //  san_move -> SYMBOL
    void san_move();

    //-------------------------------------------------
    // movenumber_indication -> INTEGER |
    //                          movenumber_indication .
    void movenumber_indication();

    //-------------------------------------------------
    //  rav -> ( rav_dummy )
    void rav();

    //-------------------------------------------------
    // rav_dummy -> rav  
    // rav_dummy -> everything_except: "(",")","DONE" rav_dummy 
    // rav_dummy -> EPS 
    void rav_dummy();



// ------------- public methods ---------------------
public:
    static PGN_Parser* getHandle() { 
       if (p_instance == NULL) {
          p_instance = new PGN_Parser();
       }
       return p_instance; 
    }
    void test();

    void parseGame();

    ~PGN_Parser();

    void dumpTagList();

    bool create(const char* log_file_name = NULL);
    void assure_created();
    void findfirst(const char* pgn_file_name);
    void findnext();
    bool end_of_pgn_db();

    MoveList& getMoveList() { return movelist; }
    TagList&  getTagList()  { return taglist;  }
    GameResults getGameResult() { return gameResult; }


    //PGN_ErrorStatus getStatus() { return status; }

};

#endif // PGNPARSE_H
