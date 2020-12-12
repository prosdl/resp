// --------------------------------------------------------------------------
//
// File   : gameinfo.cpp
// Version: $Id: gameinfo.cpp,v 1.2 2003/05/22 22:45:08 rosendahl Exp $
// Author : $Author: rosendahl $
//
// Copyright (C) 2003 Peter Rosendahl
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
// --------------------------------------------------------------------------
#include "gameinfo.h"
#include "version.h"
#include "basic_stuff.h"
#include "respoptions.h"
#include <iostream>
#include <fstream>

using namespace std;

// --------------------------------------------------------------------------
//  GameInfo::init
// --------------------------------------------------------------------------
void GameInfo::init() {
   moveInfos.erase(moveInfos.begin(), moveInfos.end());
   header.erase(header.begin(), header.end());

   header["Event"]  = "Computer Chess game";
   string site = "?";
   try {
      site = RespOptions::getHandle()->getValue("computer.name");
   } catch (RespOptionsException re) {
   }
   header["Site"]   = site;
   header["Date"]   = dateStringYYYYMMDD();
   header["Round"]  = "1";
   header["White"]  = "?";
   header["Black"]  = "?";
   header["Result"] = "*";
}

// --------------------------------------------------------------------------
//  GameInfo::addMoveInfo
// --------------------------------------------------------------------------
void GameInfo::addMoveInfo(const MoveInfo& moveInfo) {
   moveInfos.push_back(moveInfo);

   // Update "White", "Black" Tags?
   if (moveInfo.respMove) {
      if (moveInfo.whiteMove) {
         header["White"] = Version::full_version();
      } else {
         header["Black"] = Version::full_version();
      }
   } else {
      string opp_name = getHeader("Opponent");
      if (opp_name != "???") {
         if (moveInfo.whiteMove) {
            header["White"] = opp_name;
         } else {
            header["Black"] = opp_name;
         }
      }
   }
}

// --------------------------------------------------------------------------
//  GameInfo::dump
// --------------------------------------------------------------------------
void GameInfo::dump() const {
   cout << "dumping GameInfo ..." << endl;

   map<string,string>::const_iterator it = header.begin();
   
   for (; it != header.end(); it++) {
      cout << it->first << "->" << it->second << endl;
   }
   cout << endl;
   for (unsigned int i=0; i < moveInfos.size(); i++) {
      cout << moveInfos[i].to_string();
   }
}

// --------------------------------------------------------------------------
//  GameInfo::appendToPGNFile
// --------------------------------------------------------------------------
void GameInfo::appendToPGNFile(string filename) {

   // Resp only will write a PGN file, if there are at least
   // 4 moves
   if (moveInfos.size() <= 4) {
      return;
   }
   
   ofstream os("resp.pgn",ios::out|ios::app);

   // PGN seven tag roster (Standard 8.1.1)
   os << "[Event \"" << header["Event"] << "\"]" << endl;
   os << "[Site \"" << header["Site"] << "\"]" << endl;
   os << "[Date \"" << header["Date"] << "\"]" << endl;
   os << "[Round \"" << header["Round"] << "\"]" << endl;
   os << "[White \"" << header["White"] << "\"]" << endl;
   os << "[Black \"" << header["Black"] << "\"]" << endl;

   if (getHeader("SetUp") == "1") {
      os << "[SetUp \"1\"]" << endl;
      os << "[FEN \"" << getHeader("FEN") << "\"]" << endl;
   }

   if (getHeader("TimeControl") != "???") {
      os << "[TimeControl \"" << header["TimeControl"] << "\"]" << endl;
   }
   
   
   // Strip commentary from Result:
   string result = header["Result"];
   int p0 = result.find_first_of(" \t");
   if (p0 != -1) {
      result = string(result,0,p0);
   }
   os << "[Result \"" << result << "\"]" << endl;

   os << endl;

   int ply_count = 0;
   for (unsigned int i=0; i < moveInfos.size(); i++) {
      MoveInfo mi = moveInfos[i];

      if (ply_count == 0 && !mi.whiteMove) {
         // 1.Zug Schwarz (Spiel startet von FEN Position)
         os << "1 ... ";
         ply_count++;
      }

      if (mi.whiteMove) {
         os << (ply_count/2 + 1) << ". ";
      }

      os << mi.sanMove << " ";

      if (mi.respMove) {
         // This was a move made by Resp; add additional
         // Info as comment
         os << "{";
         if (mi.bookMove) {
            os << "book ";
            os << mi.bookInfo;
         } else {
            os << "SC: " << ((double) mi.score/100.0) << ", ";
            os << "TM: " << ((double) mi.time/1000.0) << "s";
         }
         os << "} ";
      }

      if (!mi.whiteMove) {
         // Black's move; end line. Note: this is *not* conform
         // with PGN's "export format" (Standard 8.2.1) - I'am
         // using the weaker "import format" for Resp's PGN
         os << endl;
      }
      
      ply_count++;
   }

   // Result
   os << header["Result"] << endl << endl;
   
   os.close();
}
