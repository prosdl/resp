// -------------------------------------------------------------------------
//
//                                    R E S P
//                                    =======
//
// (peter Rosendahls Erstes Schach Programm).
//
//
//  Datei              : console.cpp
//                       Konsolenclient für RESP.
//
//  Anfang des Projekts: So, 8.Juli, 2001
//  Sprache            : C++
//  Verwendete Compiler: Visual C++ 6.0 SP5 / gcc 3.01
//  Betriebssystem(e)  : Win2000 SP2 / Linux 2.4.4
//  Version            : 0.19
// -------------------------------------------------------------------------
//
// resp (peter _r_osendahls _e_rstes _s_chach_p_rogramm) 
// $Id: console.cpp,v 1.35 2003/06/02 18:12:53 rosendahl Exp $
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
#include "controller.h"
#include "xboard.h"
#include "version.h"
#include "respoptions.h"
#include "hash.h"
#include "phash.h"
#include "board.h"
#include "eval.h"

#include <iostream>
#include <limits>

#ifdef USE_READLINE
   #include <readline/readline.h>
   #include <readline/history.h>
#endif


#ifndef LINUX
#pragma warning (disable:4786) // Visual C++ Faxen
#endif

using namespace std;

void title()
{
    //cout << endl << endl << endl;    
    out << "#    //" << endl;
    out << "#    ////  ////   ///  ////" << endl;
    out << "#   //   ////    /    / //"  << endl;
    out << "#  //    ///// ///   //"  << endl;
    out << "#" << endl;
    out << "# resp  ... peter (r)osendahls (e)rstes (s)chach (p)rogramm" << endl;
    out << "# " << endl;
    out << "# VERSION: " << Version::major_version << ".";
    out << Version::minor_version << "." << Version::build_number <<  endl;
    out << "# (c) p.rosendahl, 2001-2002" << endl;
    out << "#" << endl;
    out << "#" << endl;

}

#ifdef USE_READLINE
/* A static variable for holding the line. */
static char *line_read = (char *)NULL;

/* Read a string, and return a pointer to it.  Returns NULL on EOF. */
char * do_gets ()
{
  /* If the buffer has already been allocated, return the memory
     to the free pool. */
  if (line_read != (char *)NULL)
    {
      free (line_read);
      line_read = (char *)NULL;
    }

  /* Get a line from the user. */
  line_read = readline (">");

  /* If the line has any text in it, save it on the history. */
  if (line_read && *line_read)
    add_history (line_read);

  return (line_read);
}
#endif //USE_READLINE

void dumpStatus(int status)
{
    if (status & ST_GAME_OVER)
        out << "GAME OVER  ";

    if (status & ST_CANCELED)
    {
        out << "GAME CANCELED" << endl;
        return;
    }

    if (status & ST_RESIGN)
    {
        if (status & ST_WHITE_TO_MOVE)
            out << "BLACK RESIGNS" << endl;
        else
            out << "WHITE RESIGNS" << endl;

        return;
    }
    if (status & ST_REMIS)
        out << "REMIS  ";
    else
        if (status & ST_WHITE_WON)
            out << "WHITE WON  ";
        else
            out << "BLACK WON  ";

    out <<  endl;
}



void init_resp() {
    cout << "#init_resp" << endl;
    // Logging setzen
    out.setLogging(RespOptions::getHandle()->getValueAsBool("output.logging"));
    
    // Initialisierung der Singletons (Reihenfolge ist wichtig!)
    Hash::getHandle();
    PHash::getHandle();
    Eval::getHandle();
    Board::getHandle();
    Controller::getHandle();
    
    cout << "#init_resp ...  done" << endl;
    
}





#ifdef LINUX
int main(int argc, char* argv[])
#else 
int __cdecl main(int argc, char* argv[]) // Warnung wg. __fastcall
#endif
{
    init_resp();

    // Eventuell Win(X)Board-Engine?
    if (argc > 1)
        if (strcmp(argv[1],"-xboard") == 0) {
            // XBoard Interface starten;
            RespOptions::getHandle()->setValue("output.style","XBOARD");
            XBoardInterface* pxb = XBoardInterface::getHandle();
            pxb->run();
            return 0;
        }

    Controller* p_control = Controller::getHandle();
    title();
    p_control->command("new");

#ifdef USE_READLINE
    // History lesen
    read_history("resp.history");
#endif
    

    while (true) {
        string inp;

#ifdef USE_READLINE
        char* line = do_gets();
        inp = string(line);
#else
        out <<  "> ";
        char s[256];
        cin.getline(s,80);
        inp = string(s);

#endif
        if (out.getLogging())
            out.logfile() << inp << endl;

        // ----------------------------------------------------------
        // Spezialfall XBoard sendet den Befehl 'xboard':
        // In diesem Fall wird das Xboard Interface gestartet.

        if (inp == "xboard")
        {
           // Ausgaben ab jetzt im XBoard Format
           RespOptions::getHandle()->setValue("output.style","XBOARD");
	        cout << endl;

            if (out.getLogging()) {
                out.logfile() << "#" << endl;
                out.logfile() << "# Switching to Winboard/XBoard Mode..." 
                              << endl;
                out.logfile() << "#" << endl;
            }
            XBoardInterface* pxb = XBoardInterface::getHandle();
            pxb->run();
            return 0;
        }
        // ----------------------------------------------------------

        
        try {
            p_control->command(inp);
        }
        catch (BadCmdException e) {
            out << "Error - " << e.what() << endl;
        }

        if (p_control->getStatus() & ST_GAME_OVER)
            dumpStatus(p_control->getStatus());

        
        if (p_control->getStatus() & ST_CANCELED)
            break;

        if (p_control->getStatus() & ST_CHECK)
            out << "CHECK!" << endl;
    }

    //dumpStatus(p_control->getStatus());

    out << "Bye." << endl;

#ifdef USE_READLINE
    // History lesen
    write_history("resp.history");
#endif

    return 0;
}
