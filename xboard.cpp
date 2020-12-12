// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : xboard.cpp
//                       Interface für die Verwendung von Tim Mann´s XBoard.
//
//  Anfang des Projekts: So, 5. August, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: xboard.cpp,v 1.31 2003/06/02 18:12:53 rosendahl Exp $
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


#include "xboard.h"
#include "controller.h"
#include "StringTools.h"
#include "respoptions.h"
#include "version.h"
#include <iostream>
#include <string>
#include <limits>
#include <map>
//#include <csignal>

using namespace std;

XBoardInterface XBoardInterface::snglInstance;

//Controller& XBoardInterface::ctrl;
// -------------------------------------------------------------------------
//  Konstruktor
// -------------------------------------------------------------------------

XBoardInterface::XBoardInterface ()
{
    init();
}

// -------------------------------------------------------------------------
//  init... Notwendige Initialisierungen fürs Interface
// -------------------------------------------------------------------------
void XBoardInterface::init()
{
    // Ein/Ausgabe - Pufferung abschalten
    cout.setf(ios::unitbuf);
    cin.rdbuf()->pubsetbuf(0,0);

    // SIGINT abschalten
    // signal(SIGINT, SIG_IGN);

    // Tabelle der möglichen XBoard-Befehle aufbauen
    builtCmdTable();

    // Status einschalten
    status = FORCE;

    // Computer spielt schwarz
    compColor = BLACK;
}

// -------------------------------------------------------------------------
//  Befehle in die Tabelle eintragen
// -------------------------------------------------------------------------
void XBoardInterface::builtCmdTable()
{
    XBoardCommand* cmdlist[] = { 
        new XB_xboard(),
        new XB_usermove(),
        new XB_new(),
        new XB_result(),
        new XB_quit(),
        new XB_playother(),
        new XB_protover(),
        new XB_white(),
        new XB_black(),
        new XB_st(),
        new XB_level(),
        new XB_time(),
        new XB_otime(),
        new XB_force(),
        new XB_go(),
        new XB_setboard(),
        new XB_accepted(),
        new XB_random(),
        new XB_hard(),
        new XB_easy(),
        new XB_post(),
        new XB_nopost(),
        new XB_analyze(),
        new XB_undo(),
        new XB_hint(),
        new XB_exit(),
        new XB_name(),
        NULL
    };

    int i=0; 
    while (cmdlist[i])
    {
        cmdTable[cmdlist[i]->syntax] = cmdlist[i];
        i++;
    }
}


// -------------------------------------------------------------------------
//  parseCmd... Parsen der von XBoard gesendeten Zeichenkette
// -------------------------------------------------------------------------
XBoardInterface::XBoardCommand* XBoardInterface::parseCmd(const string& cmd,
                                                          string& arg)
{
    string op;

    // cmd in 2 Zeichenketten aufsplitten
    int e = cmd.find_first_of(" \t");
    if (e==-1) {
        op = cmd;
        arg = "";
    }
    else
    {
        op = string(cmd, 0, e);
        arg = cmd.substr(e+1);
    }
    
    map<string,XBoardCommand*>::iterator it = cmdTable.find(op);

    if (it == cmdTable.end())
        return NULL;

    return (*it).second;

}


// -------------------------------------------------------------------------
//  run... Start der Nachrichtenverarbeitung des XBoard Interface
// -------------------------------------------------------------------------

void XBoardInterface::run()
{
    string sin;
    string arg;
    int hply;
    HistoryEntry hentry;
    XBoardInterface::XBoardCommand* xcmd;
    Controller*  p_ctrl = Controller::getHandle();

        
    // Hauptverarbeitungsschleife
    while (true)
    {
        char buffer[256];

        cin.getline(buffer, sizeof buffer);
        sin = buffer;

        if (out.getLogging())
            out.logfile() << "[XB]-->" << sin << endl;


        // Sonderfall: Zugfolge
        if (isMove(sin))
            cmdTable["usermove"]->execute(sin.c_str());

        else
        {
            // Befehl parsen....
            if ((xcmd = parseCmd(sin,arg)))
            {
                // und ausführen
                xcmd->execute(arg.c_str());
            }
            else
            {
                // Befehl nicht erkannt
                out << "Error (unknown command): " << sin << endl;
            }
        }

        // Beenden der Engine?
        if (status == QUIT) {
            p_ctrl->command("quit");
            break;
        }

        // Brettstatus kontrollieren
        checkStatus();

        // Soll ein Zug ausgeführt werden?
        if (status==WAITING_FOR_ENGINE_MOVE && 
            compColor == Board::getHandle()->sideToMove())
        {
            // Zug ausführen
            p_ctrl->command("s");

            hply   = Board::getHandle()->nMoves();
            hentry = Board::getHandle()->history.get(hply-1);

            out << "move " << hentry.m.toString() << endl;
            status = RUNNING;

            checkStatus();
            if (status == RUNNING)
            {
                // Pondern
                if (RespOptions::getHandle()->getValueAsBool("game.ponder"))
                    p_ctrl->command("ponder");
            }
        }
        else if (status == ANALYZE)
        {
            // Analyse - Modus
            p_ctrl->command("a");
        }
        
    }
}

// -------------------------------------------------------------------------
//  checkStatus
// -------------------------------------------------------------------------

void XBoardInterface::checkStatus()
{
    if (status == FORCE)
        return;

    int st = Controller::getHandle()->getStatus();


    if (st & ST_GAME_OVER)
    {
        status = FORCE;

        if (st & ST_RESIGN)
        {
            if (st & ST_WHITE_TO_MOVE) {
               Controller::getHandle()->command("result 1-0 {Black resigns}");
            } else {
               Controller::getHandle()->command("result 0-1 {White resigns}");
            }
            out << "resign" << endl;
            return;
        }

        if (st&ST_REMIS)
        {
            Controller::getHandle()->command("result 1/2-1/2");
            out << "1/2-1/2" << endl;
        }
        else if (st & ST_WHITE_WON)
        {
            Controller::getHandle()->command("result 1-0 {White mates}");
            out << "1-0" << endl;
        }
        else
        {
            Controller::getHandle()->command("result 0-1 {Black mates}");
            out << "0-1" << endl;
        }
    }
}

// -------------------------------------------------------------------------
//  isMove?  Prüft, ob Zeichenkette s ein 4-Zeichen Zug ist (e2e4,...)
// -------------------------------------------------------------------------

bool XBoardInterface::isMove(string s)
{
    if (s.length() < 4)
        return false;

    if ( (s[0] < 'a') || (s[0] > 'h') )
        return false;
    if ( (s[1] < '1') || (s[1] > '8') )
        return false;
    if ( (s[2] < 'a') || (s[2] > 'h') )
        return false;
    if ( (s[3] < '1') || (s[3] > '8') )
        return false;

    return true;
}


// -------------------------------------------------------------------------
//    X B O A R D - B E F E H L E  -----> E N G I N E
// -------------------------------------------------------------------------

// ---------------------------------------------------------------------
//                          *** xboard ***
// ---------------------------------------------------------------------

void XBoardInterface::XB_xboard::execute(const char* arg)
{
    // NOOP
}

// ---------------------------------------------------------------------
//                          *** accepted ***
// ---------------------------------------------------------------------

void XBoardInterface::XB_accepted::execute(const char* arg)
{
    // NOOP
}

// ---------------------------------------------------------------------
//                          *** random ***
// ---------------------------------------------------------------------

void XBoardInterface::XB_random::execute(const char* arg)
{
    // NOOP
}

// ---------------------------------------------------------------------
//                          *** hard ***
// ---------------------------------------------------------------------

void XBoardInterface::XB_hard::execute(const char* arg)
{
    RespOptions::getHandle()->setValue("game.ponder","true");
}

// ---------------------------------------------------------------------
//                          *** easy ***
// ---------------------------------------------------------------------

void XBoardInterface::XB_easy::execute(const char* arg)
{
    RespOptions::getHandle()->setValue("game.ponder","false");
}

// ---------------------------------------------------------------------
//                          *** post ***
// ---------------------------------------------------------------------

void XBoardInterface::XB_post::execute(const char* arg)
{
    RespOptions::getHandle()->setValue("game.post","true");
}

// ---------------------------------------------------------------------
//                          *** nopost ***
// ---------------------------------------------------------------------

void XBoardInterface::XB_nopost::execute(const char* arg)
{
    RespOptions::getHandle()->setValue("game.post","false");
}

// ---------------------------------------------------------------------
//                          *** analyze ***
// ---------------------------------------------------------------------

void XBoardInterface::XB_analyze::execute(const char* arg)
{
    xbi->status = ANALYZE;
}

// ---------------------------------------------------------------------
//                          *** undo ***
// ---------------------------------------------------------------------

void XBoardInterface::XB_undo::execute(const char* arg)
{
    try
    {
        Controller::getHandle()->command("undo");

    }
    catch (BadCmdException e)
    {
        // Fehler
    }
}

// ---------------------------------------------------------------------
//                          *** hint ***
// ---------------------------------------------------------------------

void XBoardInterface::XB_hint::execute(const char* arg)
{
    if (Controller::getHandle()->pSearch->gotPonderMove())
        out << "Hint: " << Controller::getHandle()->pSearch->getPonderMove() << endl;
}

// ---------------------------------------------------------------------
//                          *** exit ***
// ---------------------------------------------------------------------

void XBoardInterface::XB_exit::execute(const char* arg)
{
    if (xbi->status == ANALYZE)
        xbi->status = FORCE;
}

// ---------------------------------------------------------------------
//                          *** usermove ***
// ---------------------------------------------------------------------

void XBoardInterface::XB_usermove::execute(const char* arg)
{
    Controller* p_ctrl = Controller::getHandle();

    try
    {
        // Zug ausführen
        string cmd = "m ";
        cmd += arg;
        p_ctrl->command(cmd);

        if (xbi->status != FORCE)
        {
            if (xbi->status != ANALYZE)
                xbi->status = WAITING_FOR_ENGINE_MOVE;
        }
    }
    catch (BadCmdException e)
    {
        // Illegaler Zug:

        out << "Illegal move (" << e.what() << "): " << arg << endl;
    }
}

// ---------------------------------------------------------------------
//                          *** new ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_new::execute(const char* arg)
{
    Controller* p_ctrl = Controller::getHandle();

    try
    {
        p_ctrl->command("new");
    }
    catch (BadCmdException e)
    {
        // Fehler...
    }

    if (xbi->status == ANALYZE)
    {
        xbi->status = ANALYZE;
        xbi->compColor = BLACK;
    }
    else
    {
        xbi->status = START_OF_NEWGAME;
        xbi->compColor = BLACK;
    }
}

// ---------------------------------------------------------------------
//                          *** quit ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_quit::execute(const char* arg)
{

    xbi->status = QUIT;
}

// ---------------------------------------------------------------------
//                          *** force ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_force::execute(const char* arg)
{

    xbi->status = FORCE;
}

// ---------------------------------------------------------------------
//                          *** go ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_go::execute(const char* arg)
{
    if (Board::getHandle()->sideToMove() == WHITE)
        xbi->compColor = WHITE;
    else
        xbi->compColor = BLACK;
    

    xbi->status = WAITING_FOR_ENGINE_MOVE;
}



// ---------------------------------------------------------------------
//                          *** playother ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_playother::execute(const char* arg)
{
    if (Board::getHandle()->sideToMove() == WHITE)
        xbi->compColor = BLACK;
    else
        xbi->compColor = WHITE;

    xbi->status = WAITING_FOR_ENGINE_MOVE;
}

// ---------------------------------------------------------------------
//                          *** protover ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_protover::execute(const char* arg)
{
    // Initialisierung an XBoard:
    out << "feature playother=1 sigint=0 time=1 setboard=1 analyze=1 ";
    out << "done=1 name=1 ";
    out << "myname=" << Version::full_version() << endl;

}

// ---------------------------------------------------------------------
//                          *** white ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_white::execute(const char* arg)
{
    Controller* p_ctrl = Controller::getHandle();

    if (Board::getHandle()->sideToMove() == BLACK)
    {
        p_ctrl->command("dd S");
    }

    xbi->compColor = BLACK;
}

// ---------------------------------------------------------------------
//                          *** black ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_black::execute(const char* arg)
{
    Controller* p_ctrl = Controller::getHandle();

    if (Board::getHandle()->sideToMove() == WHITE)
    {
        p_ctrl->command("dd S");
    }

    xbi->compColor = WHITE;
}

// ---------------------------------------------------------------------
//                          *** st ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_st::execute(const char* arg)
{
    Controller* p_ctrl = Controller::getHandle();

    string s = "st ";
    s  += arg;

    p_ctrl->command(s.c_str());
}

// ---------------------------------------------------------------------
//                          *** level ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_level::execute(const char* arg)
{
    Controller* p_ctrl = Controller::getHandle();

    // Keine level Befehle akzeptieren, falls nicht im FORCE - Modus

    if (xbi->status != FORCE && xbi->status != START_OF_NEWGAME)
    {
        out << "Error (level? I'm already running!): level" << endl;
        return;
    }

    // level n_moves sss:mm  parsen

    string s(arg);

    s = StringTools::trim(s);

    s = "level " + s;

    try 
    {
        p_ctrl->command(s.c_str() );
    }
    catch (BadCmdException e)
    {
        // Fehler
    }
    
}

// ---------------------------------------------------------------------
//                          *** time ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_time::execute(const char* arg)
{
    Controller* p_ctrl = Controller::getHandle();


    // level n_moves sss:mm  parsen

    string s(arg);

    s = StringTools::trim(s);

    s = "time " + s;
    try 
    {
        p_ctrl->command(s.c_str() );
    }
    catch (BadCmdException e)
    {
        // Fehler
    }
}

// ---------------------------------------------------------------------
//                          *** otime ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_otime::execute(const char* arg) {
    // nicht implementiert
}

// ---------------------------------------------------------------------
//                          *** name ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_name::execute(const char* arg) {
   Controller* p_ctrl = Controller::getHandle();
   p_ctrl->command(string("name ")+arg);
}

// ---------------------------------------------------------------------
//                          *** result ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_result::execute(const char* arg) {
   Controller* p_ctrl = Controller::getHandle();
   p_ctrl->command(string("result ")+arg);
}

// ---------------------------------------------------------------------
//                          *** setboard ***
// ---------------------------------------------------------------------
void XBoardInterface::XB_setboard::execute(const char* arg)
{
    Controller* p_ctrl = Controller::getHandle();

    // laden einer Position im FEN - Format

    string s(arg);
    s = StringTools::trim(s);
    s = "setboard " + s + "";
    try
    {
        p_ctrl->command(s);
    }
    catch (BadCmdException e)
    {
        // Fehler
    }
}

