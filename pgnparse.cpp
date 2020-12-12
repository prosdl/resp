//////////////////////////////////////////////////////
//               pgnparse.cpp
//               ------------
//
//  Einlesen von *.pgn Dateien.
// --------------------------------------------------
//
// AUTOR    : P.Rosendahl
// SPRACHE  : C++
// COMPILER : Visual C++ / GNU C++
// OS       : Win2000 / Linux
// DATUM    : 28.07.2001
// VERSION  : 0.19
//
// Letzte Änderung: 28.07.01
//
//////////////////////////////////////////////////////
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: pgnparse.cpp,v 1.17 2003/06/02 18:12:53 rosendahl Exp $
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

#ifndef LINUX
#pragma warning(disable:4786)
#endif

#include <iostream>
#include <algorithm>
#include <map>
#include <vector>
#include "pgnparse.h"
#include "board.h"
#include "notation.h"


using namespace std;




//////////////////////////////////////////////////////
//                  class Token - IMLPEMENTATION
//////////////////////////////////////////////////////
string Token::dumpToString()
{
    string s;
    switch (id)
    {
    case STRING:
        s = "STRING"; break;
    case INTEGER:
        s = "INTEGER";  break;
    case PERIOD:
        s =".";  break;
    case ASTERISK:
        s = "*";  break;
    case LBRACKET: 
        s = "[";  break;
    case RBRACKET: 
        s = "]";  break;
    case LPARENTHESIS:
        s = "(";  break;
    case RPARENTHESIS:
        s = ")";  break;
    case LANGLE:
        s = "<";  break;
    case RANGLE:
        s = ">";  break;
    case NAG:
        s = "NAG";  break;
    case SYMBOL:   
        s = "SYMBOL";  break;
    case GAME_TERMINATION:
        s = "GAME_TERMINATION"; break;
    default:
        s = "???"; break;
    }

    return s;
}

//////////////////////////////////////////////////////
//              class PGN_Parser - IMLPEMENTATION
//////////////////////////////////////////////////////

// ---------------------------------------------------
//      Definition für Singleton-Pattern
// ---------------------------------------------------

PGN_Parser* PGN_Parser::p_instance = NULL;

// ---------------------------------------------------
//      Definition für gameTerminationMarks
// ---------------------------------------------------
map<string,string> PGN_Parser::gameTerminationMarks;


void PGN_Parser::initGameTerminationMarks()
{
    if (mapInitialized)
        return;


    gameTerminationMarks["1-0"] = "W";
    gameTerminationMarks["0-1"] = "B";
    gameTerminationMarks["1/2-1/2"] = "R";
    mapInitialized = true;

}
// ---------------------------------------------------
//      getNextToken()
//
// - lexikalische Analyse der PGN-Datei
// - erzeugt den Tokenstream
// ---------------------------------------------------
void PGN_Parser::getNextToken()
{
    char c;
    bool inside_brace_comment       = false;
    bool inside_semicolon_comment   = false;
    bool inside_string              = false;
    string value;
    const string cont_chars = "+_#=:-";


    while (file.get(c))
    {
        if (c == '\n')      // neue Zeile?
            lineno++;

        // --------- im Kommentar {...} ----------------
        if (inside_brace_comment)
        {
            if (c == '}')
                inside_brace_comment = false;
            continue;
        }

        // --------- im Kommentar ;... -----------------
        if (inside_semicolon_comment)
        {
            if (c == '\n')
                inside_semicolon_comment = false;
            continue;
        }

        // --------- in einer Zeichkette ---------------
        if (inside_string)
        {
            if (c == '"')
            {
                inside_string = false;
                lookahead.id = STRING;
                lookahead.value = value;
                return;
            }
            else
                value += c;
            continue;
        }

        switch (c)
        {
        // --------- Kommentaranfang { -----------------
        case '{':
            inside_brace_comment = true;
            continue;
        // --------- Kommentaranfang ; -----------------
        case ';':
            inside_semicolon_comment = true;
            continue;
        // --------- Zeichenkettenanfang ---------------
        case '"':
            inside_string = true;
            value = "";
            continue;
        // --- aus einzelnen Zeichen bestehende Token --
        case '*':
            lookahead.id = ASTERISK;
            return;
        case '.':
            lookahead.id = PERIOD;
            return;
        case '[':
            lookahead.id = LBRACKET;
            return;
        case ']':
            lookahead.id = RBRACKET;
            return;
        case '(':
            lookahead.id = LPARENTHESIS;
            return;
        case ')':
            lookahead.id = RPARENTHESIS;
            return;
        case '<':
            lookahead.id = LANGLE;
            return;
        case '>':
            lookahead.id = RANGLE;
            return;
        }

        // ------- white space -------------------------
        if (isspace(c))
            continue;

        // - Integer Zahlen / Symbol mit führender Ziffer-
        if (isdigit(c))
        {
            value = c;
            while (true)
            {
                if (!file.get(c))
                {
                    lookahead.id = DONE;
                    return;
                }
                if (!isdigit(c))
                    break;
                value +=  c;
            }

            // ------------ Symbol? -------------------
            if (IS_CONT_CHAR(c) || (c == '/'))
            {
                value += c;
                while(true)
                {
                    if (!file.get(c))
                    {
                        lookahead.id  = DONE;
                        return;
                    }

                    if (!IS_CONT_CHAR(c) && (c != '/'))
                        break;

                    value += c;
                }

                file.putback(c);

                // Test auf Terminierungssymbol
                map<string,string>::iterator it;

                it = gameTerminationMarks.find(value);
                if (it != gameTerminationMarks.end())
                {
                    // gefunden!
                    lookahead.value = gameTerminationMarks[value];
                    lookahead.id    = GAME_TERMINATION;
                    return;
                }


                lookahead.id    = SYMBOL;
                lookahead.value = value;
                return;
            }

            file.putback(c);
            lookahead.id    = INTEGER;
            lookahead.value = value;
            return;
        }
        // --------- Symbol ---------------------------
        if (isalpha(c))
        {
            value = c;

            while (true)
            {
                if (!file.get(c))
                {
                    lookahead.id = DONE;
                    return;
                }
                if (!IS_CONT_CHAR(c))
                    break;

                value += c;
            }

            file.putback(c);
            lookahead.id    = SYMBOL;
            lookahead.value = value;
            return;
        }

        // ------------- NAG ---------------------------
        if (c == '$')
        {
            if (!file.get(c))
            {
                lookahead.id = DONE;
                return;
            }

            if (!isalnum(c))
            {
                lookahead.id = UNKNOWN;
                return;
            }

            value = "$" + c;
            while (true)
            {
                if (!file.get(c))
                {
                    lookahead.id = DONE;
                    return;
                }
                if (!isalnum(c))
                    break;
                value += c;
            }

            file.putback(c);
            lookahead.id    = NAG;
            lookahead.value = value;
            return;
        }

        lookahead.id = UNKNOWN;
        return;
    }

    lookahead.id = DONE;
}

// ---------------------------------------------------
//      Destruktor
// ---------------------------------------------------
PGN_Parser::~PGN_Parser()
{
    file.close();

    if (use_log)
        logfile.close();
}

// ---------------------------------------------------
//      Konstruktor
// ---------------------------------------------------
PGN_Parser::PGN_Parser()
{
    mapInitialized  = false;
    created         = false;
    file_open       = false;
    use_log         = false;

    pBoard = Board::getHandle();
}


// ---------------------------------------------------
//  Kreieren des PGN-Parsers; diese Funktion muss vom
//  Benutzer gerufen werden, bevor man mit dem
//  PGN_Parser-Objekt irgendetwas anfangen kann!
// ---------------------------------------------------
bool PGN_Parser::create(const char* log_file_name)
{
    initGameTerminationMarks();

    // Log-Datei?
    use_log = log_file_name != NULL;

    if (use_log)
    {
        if (logfile.is_open())
            logfile.close();

        // Log-Datei öffnen
        logfile.open(log_file_name,ios::out | ios::trunc);
        if (!logfile)
        {
            created = false;
            return false;
        }
    }

    created = true;

    return true;
}

// ---------------------------------------------------
// Sicherstellen des create gerufen wurde.
// ---------------------------------------------------
void PGN_Parser::assure_created()
{
    if (!created)
    {
        cout << "ERROR IN PGNPARSE.CPP: use create first." << endl;
        exit(1);
    }
}


// ---------------------------------------------------
// Laden des ersten Spiels aus der PGN-Datenbank
// "pgn_file_name".
// ---------------------------------------------------
void PGN_Parser::findfirst(const char* pgn_file_name)
{
    assure_created();


    if (file.is_open())
    {
        file.clear();
        file.close();

    }

    // Datei öffnen
    file.open(pgn_file_name,ios::in | ios::out);


    if (!file)
    {
        file.clear();
        throw ParseErrorException("File not found.",
            ParseErrorException::FILE_ERROR);
    }

    file_open = true;
    lineno = 1;

    // 1.Spiel in PGN-Datei einlesen:

    //status = PGN_NO_ERROR;
    getNextToken();

    try
    {
        pgn_game();
    }
    catch (ParseErrorException e)
    {
        throw e;
    }

}

// ---------------------------------------------------
// Laden des nächsten Spiels aus der PGN-Datenbank
// ---------------------------------------------------
void PGN_Parser::findnext()
{
    assure_created();
    
    if (!file_open)
    {
        throw ParseErrorException("No PGN file open.",
            ParseErrorException::FILE_ERROR);
    }

    if (end_of_pgn_db())
    {
        throw ParseErrorException("No more games in database.",
            ParseErrorException::FILE_ERROR);
    }

    // ... nächstes Spiel lesen
    try
    {
        pgn_game();
    }
    catch (ParseErrorException e)
    {
        throw e;
    }

}

// ---------------------------------------------------
// Prüft, ob Ende der DB erreicht ist.
// ---------------------------------------------------
bool PGN_Parser::end_of_pgn_db()
{
    assure_created();

    return lookahead.id == DONE;
}

// ---------------------------------------------------
void PGN_Parser::test()
{
    //char dummy;

    initGameTerminationMarks();

    lineno = 1;
    while (true)
    {
        lookahead.value = "";
        getNextToken();

        cout << lineno << " --- ";
        cout << "<<< "+lookahead.dumpToString();  // TEST
        cout << "  :  " + lookahead.value+ ">>>" << endl;

        if (lookahead.id == DONE)
            break;

     //   cin >> dummy;
    }
}


// ---------------------------------------------------
//      dumpTagList
// ---------------------------------------------------

void PGN_Parser::dumpTagList()
{
    map<string,string>::iterator i;

    logfile << "------------------------------------" << endl;
    logfile << "             taglist" << endl;
    logfile << "------------------------------------" << endl;
    logfile << endl;

    i = taglist.begin();
    for (; i != taglist.end(); i++)
    {
        logfile << "[" << i->first << " ---> " << i->second << "]" << endl;
    }
}

// ---------------------------------------------------
//      match
// ---------------------------------------------------
void PGN_Parser::match(TokenID tokid)
{
    if (lookahead.id == tokid)
    {
        getNextToken();
    }
    else    // Parse Fehler:
    {
        // vorerst
        if (use_log)
        {
            logfile << "PARSE-FEHLER" << endl;
            logfile << "tokid     = " << tokid << endl;
            logfile << "lineno    = " << lineno << endl;
            logfile << "lookahead = " << lookahead.dumpToString() << endl;
        }
        //status = PGN_PARSE_ERROR;

        throw ParseErrorException("No match.",ParseErrorException::PARSE_ERROR);
    }

}

// ---------------------------------------------------
//      parseGame
// ---------------------------------------------------

void PGN_Parser::parseGame()
{
    //initGameTerminationMarks();

    try
    {
        getNextToken();

        pgn_database();
    }
    catch (ParseErrorException e)
    {
        throw e;
    }
}

//----------------------------------------------
//  pgn_database -> EPS | pgn_database pgn_game
//----------------------------------------------
void PGN_Parser::pgn_database()
{
    while (true)
    {
    //    if (lookahead.id== DONE)
    //        break;
        pgn_game();
        break; // TTTTTTT
    }
}

//----------------------------------------------
// pgn_game -> tagpair_section movetext_section
//----------------------------------------------
void PGN_Parser::pgn_game()
{
    taglist.clear();
    movelist.clear();

    try
    {
        tagpair_section();

        if (use_log)
            dumpTagList();
        movetext_section();
    }
    catch (ParseErrorException e)
    {
        throw e;
    }
}

//-------------------------------------------------
// tagpair_section -> EPS | tagpair_section tagpair
//-------------------------------------------------
void PGN_Parser::tagpair_section()
{
    while (true)
    {
        if (lookahead.id != LBRACKET)
            break;
        
        try
        {
            tagpair();
        }
        catch (ParseErrorException e)
        {
            throw e;
        }
    }
}

//-------------------------------------------------
//  tagpair -> [ SYMBOL STRING ]
//-------------------------------------------------
void PGN_Parser::tagpair()
{
    string tag_symbol;
    string tag_value;

    try
    {
        match(LBRACKET);
        tag_symbol = lookahead.value;
        match(SYMBOL);
        tag_value  = lookahead.value;
        match(STRING);
        match(RBRACKET);
    }
    catch (ParseErrorException e)
    {
        if (use_log)
        {
            logfile << "-> WARNING: Couldn´t read tagpair in line " << lineno << endl;
            logfile << "-> SKIPPING" << endl;
        }

        // error recovery: Nächstes "]" finden:

        while (true)
        {
            match(lookahead.id);

            if (lookahead.id == RBRACKET)
            {
                match(RBRACKET);
                break;                      // gerettet!
            }
            else if (lookahead.id == DONE)
                throw e;
        }
    }


    
    // tag in Hash einfügen
    taglist[tag_symbol] = tag_value;
}   


//-------------------------------------------------
//  movetext_section -> move_text |GAME_TERMINATION   
//-------------------------------------------------
void PGN_Parser::movetext_section()
{
    // SEMANTIK
    // gibt es einen FEN-tag?
    map<string,string>::iterator it;

    it = taglist.find("FEN");
    if (it != taglist.end())
    {
        pBoard->load_fen((*it).second);
    }
    else
        pBoard->initialize();

    pBoard->gen(0);
    // ENDE SEMANTIK

    move_text();


    // SEMANTIK

    string strResult = "";

    if (lookahead.id == ASTERISK)
    {
        gameResult = UNDEFINED;
        strResult = "*";
    }
    else
    {

        if (lookahead.value == "W")
            gameResult = WHITE_WIN;
        else if (lookahead.value == "B")
            gameResult = BLACK_WIN;
        else if (lookahead.value == "R")
            gameResult = REMIS;
        /*else
        {
            gameResult = UNDEFINED;
            throw ParseErrorException("Unknown Game Termination.",
                ParseErrorException::PARSE_ERROR);
        }*/

        strResult = lookahead.value;
    }

    if (use_log)
    {
        logfile << "====> RESULT " << lookahead.value << endl;
    }
    // ENDE SEMANTIK

    try
    {
        if (lookahead.id == ASTERISK)
            match(ASTERISK);
        else
            match(GAME_TERMINATION);
    }
    catch (ParseErrorException e)
    {
        throw e;
    }
}

//-------------------------------------------------
//  move_text -> movenumber_indication san_move [NAG]* [RAV] |
//               san_move [NAG]* [RAV] | move_text move_text
//-------------------------------------------------
void PGN_Parser::move_text()
{
    while (true)
    {
        if (lookahead.id == SYMBOL)
        {
            try
            {
                san_move();

                while (lookahead.id == NAG)
                    match(NAG);

                if (lookahead.id == LPARENTHESIS)
                    rav();
            }
            catch (ParseErrorException e)
            {
                throw e;
            }

        }
        else if (lookahead.id == INTEGER)
        {
            try
            {

                movenumber_indication();

                san_move();
                while (lookahead.id == NAG)
                    match(NAG);

                if (lookahead.id == LPARENTHESIS)
                    rav();
            }
            catch (ParseErrorException e)
            {
                throw e;
            }

        }
        else
            break;
    }
}

//-------------------------------------------------
// RAV ... recursive annotation variation 
// --------------------------------------
// 
// Wird nur überlesen:
//
//  rav -> ( rav_dummy )
//  rav -> rav rav
//-------------------------------------------------

void PGN_Parser::rav()
{
    while (true)
    {
        try
        {
            match(LPARENTHESIS);
            rav_dummy();
            match(RPARENTHESIS);
        }
        catch (ParseErrorException e)
        {
            throw e;
        }

        if (lookahead.id != LPARENTHESIS)
            break;
    }
}

//-------------------------------------------------
// rav_dummy -> rav  
// rav_dummy -> everything_except: "(",")","DONE" rav_dummy 
// rav_dummy -> EPS 
//-------------------------------------------------
void PGN_Parser::rav_dummy()
{
    while (true)
    {
        if (lookahead.id == LPARENTHESIS)
        {
            try 
            {
                rav();
            }
            catch (ParseErrorException e)
            {
                throw e;
            }
        }
        else if (lookahead.id == DONE)
        {
            throw ParseErrorException("Unexpected end of file in RAV.",
                ParseErrorException::PARSE_ERROR);
        }
        else if (lookahead.id == RPARENTHESIS)
        {
            break;
        }
        else
        {
            try
            {
                match(lookahead.id);
            }
            catch (ParseErrorException e)
            {
                throw e;
            }
        }
    }
}


//-------------------------------------------------
//  san_move -> SYMBOL
//-------------------------------------------------
void PGN_Parser::san_move()
{
    // SEMANTIK
    string san = lookahead.value;

    Notation::SAN_ParseError err;
    Move m = Notation::san_to_move(san,err,pBoard);

    // TEST
    /* string dummy;
   // pBoard->dump();
    cout << pBoard->nMoves() << ":  ";
    cout << san << "........." << m.toString();
    cout << "   [lineno=" << lineno << "]" << endl; */
    //cout << "err = " << err << endl;
   /* cin >> dummy;
    if (dummy == "bb")
        pBoard->dumpBitBoards(); */

    if (use_log)
    {
        logfile << "MOVE NO#: " << pBoard->nMoves() << "     ";
        logfile << m.toString();
        logfile << "               [lineno=" << lineno << "]" << endl;
    }

    if (err != 0)
    {
        // Fehler bzw.Warnung
        switch (err)
        {
        case Notation::SAN_SYNTAX_ERROR:
            throw ParseErrorException("Syntax Error in SAN-Move.",
                ParseErrorException::SAN_MOVE_ERROR);
        case Notation::SAN_MOVE_NOT_POSSIBLE:
            throw ParseErrorException("SAN-Move not possible.",
                ParseErrorException::SAN_MOVE_ERROR);
        case Notation::SAN_AMBIGUOUS_MOVE:
            throw ParseErrorException("Move is ambiguous.",
                ParseErrorException::SAN_MOVE_ERROR);
        default:
            throw ParseErrorException("SAN-Move error.",
                ParseErrorException::SAN_MOVE_ERROR);
        }
    }
    else
    {
        pBoard->makemove(m);
        movelist.push_back(m);
        pBoard->gen(0);
    }

    //m.dump();  // TEST


    // ENDE SEMANTIK


    try
    {
        match(SYMBOL);
    }
    catch (ParseErrorException e)
    {
        throw e;
    }

}

//-------------------------------------------------
// movenumber_indication -> INTEGER |
//                          movenumber_indication .
//-------------------------------------------------
void PGN_Parser::movenumber_indication()
{
    // SEMANTIK
    int moveNumber = atoi(lookahead.value.c_str());

    if ((pBoard->nMoves()/2) + 1 != static_cast<unsigned int>(moveNumber))
    {
        if (use_log)
        {
            logfile << "-> Wrong move number indication." << endl;
        }
    }
    // ENDE SEMANTIK

    try
    {
        match(INTEGER);
    }
    catch (ParseErrorException e)
    {
        throw e;
    }

    while (true)
    {
        if (lookahead.id != PERIOD)
            break;
        try
        {
            match(PERIOD);
        }
        catch (ParseErrorException e)
        {
            throw e;
        }
    }
}


/*
///////////////////////////////////////////////////////
// Hauptprogramm (zum Testen)....................
///////////////////////////////////////////////////////
int main()
{
    PGN_Parser& parse = PGN_Parser::getHandle();

    parse.setFile("test.pgn");
    //parse.test();
    parse.parseGame();
    parse.dumpTagList();

    return 0;
} */
