// --------------------------------------------------------------------------
//
// File   : gameinfo.h
// Version: $Id: gameinfo.h,v 1.1 2003/05/19 20:17:33 rosendahl Exp $
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

#if !defined(GAMEINFO_H)
#define GAMEINFO_H

#include <string>
#include <map>
#include <vector>
#include <sstream>


/** Holds information about a move in the game. <p>
 *
 *  Created : Son Mai 18 14:03:06 CEST 2003
 *  
 *  @author peter
 *  @version $Id: gameinfo.h,v 1.1 2003/05/19 20:17:33 rosendahl Exp $
 */

class MoveInfo {
public:
   /** Move in SAN format. */
   std::string sanMove;

   /** Was it a move made by White? */
   bool whiteMove;
   
   /** Was this a move played by Resp? */
   bool respMove;

   /** The PV of this move.  */
   std::string pv;

   /** The score of this move. */
   int score;

   /** The time used for calculating this move. */
   long time;

   /** Was this a book move? */
   bool bookMove;

   /** Additional info for the book move */
   std::string bookInfo;

   /** Number of nodes visited in this position */
   unsigned long nodes;

   /** Contrukctor */
   MoveInfo() {
      sanMove = "";
      respMove = false;
      whiteMove = true;
      pv = "";
      score = 0;
      time = 0;
      bookMove = false;
      bookInfo = "";
      nodes = 0;
   }

   /** For debugging: std::string representation.  */
   std::string to_string() const {
      std::ostringstream o;
      o << sanMove << " " << whiteMove << " " << respMove << std::endl;
      o << pv << std::endl;
      o << bookMove << " " << bookInfo << std::endl;
      o << score << " " << time << " "<<  nodes << std::endl;
      return o.str();
   }
};

/** Holds information about the current game. <p>
 *
 *
 *  Created : Son Mai 18 14:01:56 CEST 2003
 *  
 *  @author peter
 *  @version $Id: gameinfo.h,v 1.1 2003/05/19 20:17:33 rosendahl Exp $
 */

class GameInfo {
private:
   std::map<std::string, std::string> header;
   std::vector<MoveInfo> moveInfos;

public:
   /** Sets header information for the game, i.e.: 
    *  setHeader("White","Kasparov");
    */
   void setHeader(const std::string& key, const std::string& val) {
      header[key] = val;
   }

   /** Gets the header information for tag <code>key</code>.
    */
   std::string getHeader(const std::string& key) const {
      std::map<std::string, std::string>::const_iterator 
         it = header.find(key);
      if (it == header.end()) {
         return "???";
      } else {
         return it->second;
      }
   }

   /** Adds information for the current move.
    */
   void addMoveInfo(const MoveInfo& moveInfo); 

   /** Initializes or resets GameInfo.
    */
   void init();

   /** Remove last move from list.
    */
   void removeLast() {
      if (moveInfos.size() >  0) {
         moveInfos.erase(moveInfos.end());
      }
   }

   /** Append game in PGN format to file <code>filename</code>.
    */
    void appendToPGNFile(std::string filename);

   /** For debugging: dump to stdout.
    */
   void dump() const;
};
#endif // GAMEINFO_H

