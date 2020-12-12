// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : controller.h
//                       Header zu controller.h
//
//  Anfang des Projekts: So, 8.Juli, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: controller.h,v 1.48 2003/06/02 18:12:53 rosendahl Exp $
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

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "board.h"
#include "pgnparse.h"
#include "search.h"
#include "book.h"
#include "gameinfo.h"
#include <exception>
#include <string>
#include <vector>
#include <map>

///////////////////////////////////////////////////////////////
// Mögliche Statusbits
const int ST_WAITING_FOR_NEW_GAME  =   1;
const int ST_WHITE_TO_MOVE         =   2;
const int ST_GAME_OVER             =   4;
const int ST_REMIS                 =   8;
const int ST_WHITE_WON             =  16;
const int ST_CANCELED              =  32;
const int ST_WAITING_FOR_MOVE      =  64;
const int ST_CHECK                 = 128;
const int ST_RESIGN                = 256;

// Zeit in Millisek., vor der die Zeitkontrolle absolviert sein soll,
// um Timingprobleme zu vermeiden
const int NO_FLAG_BUFF             = 6200; 

/////////////////////////////////////////////////////////////////////////
//                      CLASS BadCmdException 
/////////////////////////////////////////////////////////////////////////

class BadCmdException 
//class BadCmdException : public exception
{
public:
    enum ReasonID {
        NOT_IDENTIFIED, ILLEGAL_STATE_FOR_COMMAND, ILLEGAL_ARGUMENT_SYNTAX, IO_ERROR,
        ILLEGAL_START_POSITION, ILLEGAL_MOVE, PGN_OPEN
    };
    
private:
    std::string reason;
    ReasonID reasonID;
    
public:
    BadCmdException(std::string why, ReasonID id = NOT_IDENTIFIED) : 
      reason(why), reasonID(id)
    {}
    
    const char* what() const { return reason.c_str(); }
    ReasonID getID() { return reasonID; }
};





//////////////////////////////////////////////////////////////////
//                       CLASS Controller
//////////////////////////////////////////////////////////////////

class Controller
{
    //   -----------------------------------------------------------
    //              Befehlsklassen - Deklarationen
    //   -----------------------------------------------------------


    /////////////// Basis - Klasse ////////////////////////////////

    friend class Command;
    class Command
    {
    protected:
        Controller* ctrl;

        virtual ~Command() {}

    public:
        std::vector<std::string> cmdStrings;
    public:
        Command() : ctrl(Controller::getHandle()) { }
        virtual void execute(std::string& arg) throw(BadCmdException) = 0;
        virtual const char* describtion() = 0;
        virtual const char* details()
        {
            return "N/A";
        }
    };


    ////////////// Ableitungen ////////////////////////////////////


    /////////////////////////
    ///////   CmdScore
    class CmdScore  : public Command 
    {
    public:
        CmdScore() { 
            cmdStrings.push_back("score"); 
        }
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        
    };
    friend class CmdScore;

    /////////////////////////
    ///////   CmdName
    class CmdName  : public Command 
    {
    public:
        CmdName()
        { 
            cmdStrings.push_back("name"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        
    };
    friend class CmdName;

    /////////////////////////
    ///////   CmdWritePgn
    class CmdWritePgn  : public Command 
    {
    public:
        CmdWritePgn()
        { 
            cmdStrings.push_back("writepgn"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        
    };
    friend class CmdWritePgn;

    /////////////////////////
    ///////   CmdResult
    class CmdResult  : public Command 
    {
    public:
        CmdResult()
        { 
            cmdStrings.push_back("result"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        
    };
    friend class CmdResult;


    /////////////////////////
    ///////   CmdQuit
    class CmdQuit  : public Command 
    {
    public:
        CmdQuit()
        { 
            cmdStrings.push_back("quit"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        
    };
    friend class CmdQuit;


    /////////////////////////////
    //        CmdNewGame
    class CmdNewGame : public Command
    {
    public:
        CmdNewGame()
        { 
            cmdStrings.push_back("new"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        
    };
    friend class CmdNewGame;

    /////////////////////////////
    //        CmdDump
    class CmdDump : public Command
    {
    public:
        CmdDump() 
        { 
            cmdStrings.push_back("d"); 
            cmdStrings.push_back("dump"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        
    };
    friend class CmdDump;

    /////////////////////////////
    //        CmdShowFEN
    class CmdShowFEN : public Command
    {
    public:
        CmdShowFEN() 
        { 
            cmdStrings.push_back("fen"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        
    };
    friend class CmdShowFEN;

    /////////////////////////////
    //        CmdTakeback
    class CmdTakeback : public Command
    {
    public:
        CmdTakeback() 
        { 
            cmdStrings.push_back("undo"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdTakeback;


    /////////////////////////////
    //        CmdLoadFEN
    class CmdLoadFEN : public Command
    {
    public:
        CmdLoadFEN()
        { 
            cmdStrings.push_back("setboard"); 
            cmdStrings.push_back("sb"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        const char* details();
        
    };
    friend class CmdLoadFEN;

    /////////////////////////////
    //        CmdRunEPD
    class CmdRunEPD : public Command
    {
    public:
        CmdRunEPD() 
        { 
            cmdStrings.push_back("runepd"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        const char* details();
        
    };
    friend class CmdRunEPD;

    /////////////////////////////
    //        CmdTestMO
    class CmdTestMO : public Command
    {
    public:
        CmdTestMO() 
        { 
            cmdStrings.push_back("testmo"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        const char* details();
        
    };
    friend class CmdTestMO;

    /////////////////////////////
    //        CmdCreateBook
    class CmdCreateBook : public Command
    {
    public:
        CmdCreateBook()
        { 
            cmdStrings.push_back("createbook"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        const char* details();
        
    };
    friend class CmdCreateBook;

    /////////////////////////////
    //        CmdCreateKEDB
    class CmdCreateKEDB : public Command
    {
    public:
        CmdCreateKEDB()
        { 
            cmdStrings.push_back("create_kedb"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        
    };
    friend class CmdCreateKEDB;



    /////////////////////////////
    //        CmdDebugCommand
    class CmdDebugCommand : public Command
    {
    public:
        CmdDebugCommand()
        { 
            cmdStrings.push_back("dd"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        
    };
    friend class CmdDebugCommand;

    /////////////////////////////
    //        CmdPerft
    class CmdPerft : public Command
    {
    public:
        CmdPerft()
        { 
            cmdStrings.push_back("perft"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        
    };
    friend class CmdPerft;

    /////////////////////////////
    //        CmdLoadBook
    class CmdLoadBook : public Command
    {
    public:
        CmdLoadBook()
        { 
            cmdStrings.push_back("loadbook"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        
    };
    friend class CmdLoadBook;


    /////////////////////////////
    //       CmdHelp
    class CmdHelp : public Command
    {
    public:
        CmdHelp()
        { 
            cmdStrings.push_back("help"); 
            cmdStrings.push_back("?"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdHelp;

    /////////////////////////////
    //       CmdInfo
    class CmdInfo : public Command
    {
    public:
        CmdInfo()
        { 
            cmdStrings.push_back("info"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdInfo;


    /////////////////////////////
    //       CmdMove
    class CmdMove : public Command
    {
    public:
        CmdMove()
        { 
            cmdStrings.push_back("m"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdMove;


    /////////////////////////////
    //       CmdSanMove
    class CmdSanMove : public Command
    {
    public:
        CmdSanMove()
        { 
            cmdStrings.push_back("M"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdSanMove;



    /////////////////////////////
    //       CmdLevel
    class CmdLevel : public Command
    {
    public:
        CmdLevel()
        { 
            cmdStrings.push_back("level"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdLevel;

    /////////////////////////////
    //       CmdTime
    class CmdTime : public Command
    {
    public:
        CmdTime()
        { 
            cmdStrings.push_back("time"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdTime;



    /////////////////////////////
    //       CmdOpenPGNDatabase
    class CmdOpenPGNDatabase : public Command
    {
    public:
        CmdOpenPGNDatabase()
        { 
            cmdStrings.push_back("PGN"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdOpenPGNDatabase;

    /////////////////////////////
    //       CmdNextGameFromPGNDB
    class CmdNextGameFromPGNDB : public Command
    {
    public:
        CmdNextGameFromPGNDB()
        { 
            cmdStrings.push_back("PGNnext"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdNextGameFromPGNDB;

    /////////////////////////////
    //       CmdPGNMove
    class CmdPGNMove : public Command
    {
    public:
        CmdPGNMove()
        { 
            cmdStrings.push_back("pgn"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdPGNMove;


    /////////////////////////////
    //       CmdScanPGNDatabase
    class CmdScanPGNDatabase : public Command
    {
    public:
        CmdScanPGNDatabase()
        { 
            cmdStrings.push_back("PGNscan"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdScanPGNDatabase;

    /////////////////////////////
    //       CmdSearch
    class CmdSearch : public Command
    {
    public:
        CmdSearch()
        { 
            cmdStrings.push_back("go"); 
            cmdStrings.push_back("s"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdSearch;

    /////////////////////////////
    //       CmdPonder
    class CmdPonder : public Command
    {
    public:
        CmdPonder()
        { 
            cmdStrings.push_back("ponder"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdPonder;

    /////////////////////////////
    //       CmdAnalyze
    class CmdAnalyze : public Command
    {
    public:
        CmdAnalyze()
        { 
            cmdStrings.push_back("analyze"); 
            cmdStrings.push_back("a"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdAnalyze;

    /////////////////////////////
    //       CmdSetOption
    class CmdSetOption : public Command
    {
    public:
        CmdSetOption() {  
            cmdStrings.push_back("set"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        const char* details();
    };
    friend class CmdSetOption;


    /////////////////////////////
    //       CmdGetOptions
    class CmdGetOptions : public Command
    {
    public:
        CmdGetOptions()
        { 
            cmdStrings.push_back("get"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        const char* details();
    };
    friend class CmdGetOptions;


    /////////////////////////////
    //       CmdSetSearchOptions
    class CmdSetSearchOptions : public Command
    {
    public:
        CmdSetSearchOptions()
        { 
            cmdStrings.push_back("sset"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
        const char* details();
    };
    friend class CmdSetSearchOptions;


    /////////////////////////////
    //       CmdSetTime
    class CmdSetTime : public Command
    {
    public:
        CmdSetTime()
        { 
            cmdStrings.push_back("settime"); 
            cmdStrings.push_back("st"); 
        }
        
        void execute(std::string& arg) throw(BadCmdException);
        const char* describtion();
    };
    friend class CmdSetTime;



//////////////////////////////////////////////////////////////////////


private:    
    Controller();                       
    static Controller* p_instance; 
    void operator=(Controller&);
    Controller(const Controller&);

    
private:
    // Befehlstabelle
    std::map<std::string, Command*> cmdTable;
    void builtCommandTable(); // Befehlstabelle aufbauen
    
    typedef std::map<std::string, Command*>::iterator tableIterator;
    
    int status;  // Aktueller Spielstatus

    // Befehl parsen
    bool parse(std::string cmdString, Command*& cmd, std::string& arg);

    // Spielhistorie
    GameInfo gameInfo;


private:

    // ----------------------
    //  OPTIONEN
    // ----------------------
    enum TimeEnum {
        FIXED_ST,
        CONV_CLOCK
    };

    Book book;                  // Zugriff auf Eröffn.buch    

    bool use_book;              // Eröffnungsbuch verwenden?
    TimeEnum time_mode;         // Art der Schachuhr
    int     moves_per_control;  // Anzahl Züge für eine Zeitkontrolle
    long    time_per_control;   // Zeit pro Zeitkontrolle
    int     time_inc;           // Inkrement

    
    long    time_left;          // Verbliebene Zeit
    int     moves_left;         // Verbliebene Züge

    int     cut_resign;         // Ab welcher Score aufgeben?
    int     rep_until_resign;   // Wie oft muss cut_resign bis zur Aufgabe 
                                // unterschritten  werden?
    int     rep_resign_count;   // Zählt wie oft cut_resign bisher unterschritten wurde



    Move randomBookMove(bool use_weights, double &perc);

    void initSettings();
    
    void updateStatus();

    void update_resign(int sc);


    // PGN Dateien lesen
    PGN_Parser* pgnParser;
    bool pgn_game_loaded;
    int nPGNMove;


public:
    Search* pSearch;

    std::string opp_name;
    
public:
    Board* pBoard;

    ~Controller();

    // Zugriff auf Controller Objekt
    static Controller*  getHandle() { 
       if (p_instance == NULL) {
          p_instance = new Controller();
          p_instance->builtCommandTable();
       }
       return p_instance; 
    }
    
    // Befehl -> Controller Objekt
    bool command(std::string cmdString) throw(BadCmdException);
    
    // Status holen
    int getStatus() { return status; }


    void dumpGameStatus();
};

#endif // CONTROLLER_H
