// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// resp 
// $Id: respoptions.cpp,v 1.4 2003/05/24 18:42:23 rosendahl Exp $
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
// -------------------------------------------------------------------------

#include "respoptions.h"
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;


// -------------------------------------------------------------------------
//  static Definitionen
// -------------------------------------------------------------------------
RespOptions* RespOptions::p_instance = NULL;



// -------------------------------------------------------------------------
//  setDefaultOptions
// -------------------------------------------------------------------------
void RespOptions::setDefaultOptions() {
   setValue("dir.fen","./");
   setValue("dir.book","./");
   setValue("dir.epd","./");
   setValue("dir.pgn","./");
   setValue("hash.size","16MB");
   setValue("phash.size","3MB");
   setValue("book.use","true");
   setValue("book.name","std_open.bin");
   setValue("output.style","RESP");
   setValue("output.logging","true");
   setValue("game.ponder","true");
   setValue("game.post","true");
   setValue("game.can_resign","true");
   setValue("engine.use_kedb","true");
}


// -------------------------------------------------------------------------
//  parse
// -------------------------------------------------------------------------

bool RespOptions::parseOptionsFromFile(string filename) 
     throw (RespOptionsException) {

   ifstream infile;
   infile.open(filename.c_str());

   if (!infile) {
      throw RespOptionsException("Couldn't open file");
   }

   // Zeilenweise scannen
   int linecount = 0;
   bool no_errors = true;
   while (!infile.eof()) {
      string ini_line;
      getline(infile, ini_line);
      linecount++;

      if (!parseLine(ini_line)) {
         cout << "WARNING: PARSE ERROR IN LINE " << linecount << endl;
         no_errors = false;
      }
   }

   infile.close();

   return no_errors;
}


bool RespOptions::parseLine(std::string line)
{
   int pos;

   // Whitespace überlesen
   pos = line.find_first_not_of(" \t");
   line.erase(0,pos);

   // Leerzeile?
   if (line.empty())
      return true;

   // Kommentar?
   if (line[0] == ';' || line[0] == '#')
      return true;


   // Variablenname einlesen
   if (!isalpha(line[0]))
        return false;
  
   string varName;
   varName = line[0];
   line.erase(line.begin());
   while (!line.empty() && (isalnum(line[0]) || line[0] == '_'
          || line[0] == '.' || line[0] == '-') ) {
      varName += line[0];
      line.erase(line.begin());
   }
 
   // Whitespace überlesen
   pos = line.find_first_not_of(" \t");
   if (pos == -1)
      return false;

   // ... und abschneiden
   line.erase(0,pos);
   if (line.empty())
      return false;
    
   // '=' parsen
   if (line[0] != '=')
      return false;
   line.erase(line.begin());

   // Whitespace von vorne und hinten abschneiden
   pos = line.find_first_not_of(" \t");
   line.erase(0,pos);
   pos = line.find_last_not_of(" \t");
   line.erase(pos+1,line.length());
    
   string varValue(line);
   if (varValue.empty())
      // leerer String wird wie " " behandelt
      varValue = " ";

   bindings[varName] = varValue;

   cout << "Binding: [" << varName << "=>" << varValue << "]" << endl;
   return true;
}

// -------------------------------------------------------------------------
//  RespOptions::printOptions
// -------------------------------------------------------------------------
void RespOptions::printOptions(const std::string& match) const {
   map<string,string>::const_iterator it = bindings.begin();

   bool found = false;
   char tmp[256];
   for (; it != bindings.end(); it++) {
      if (match == "" || it->first.find(match) != string::npos) {
         sprintf(tmp,"%-30s = %s",it->first.c_str(), it->second.c_str());
         cout << tmp << endl;
         found = true;
      }
   }

   if (!found) {
      cout << "Nothing found." << endl;
   }
}

// -------------------------------------------------------------------------
//  getValue
// -------------------------------------------------------------------------
string RespOptions::getValue(const std::string& name) const 
       throw (RespOptionsException) {

   map<string,string>::const_iterator it = bindings.find(name);
   if (it == bindings.end()) {
      throw RespOptionsException("Name not bound.");
   }
   return it->second;
}


// -------------------------------------------------------------------------
//  getValueAsBool
// -------------------------------------------------------------------------
bool RespOptions::getValueAsBool(const std::string& name) const 
       throw (RespOptionsException) {

   string val = getValue(name);
   return val == "true" || val == "1" || val == "TRUE" || val == "True";
}

// -------------------------------------------------------------------------
//  getValueAsLong
// -------------------------------------------------------------------------
long RespOptions::getValueAsLong(const std::string& name) const 
       throw (RespOptionsException) {

   string val = getValue(name);
   return atoi(val.c_str());
}

// -------------------------------------------------------------------------
//  getMBValueAsLong
// -------------------------------------------------------------------------
long RespOptions::getMBValueAsLong(const std::string& name) const 
       throw (RespOptionsException) {

   string val = getValue(name);
   double d;
   istringstream i(val);
   // Falls die Mega Byte Angabe mit MB, M,  MegaByte, ...
   // abschliesst, so wird dieser Teil durch >> einfach
   // ueberlesen
   i >> d;
   return static_cast<long>(d*1024*1024);
}

// -------------------------------------------------------------------------
//  setValue
// -------------------------------------------------------------------------
void RespOptions::setValue(const string& key, const string& value) {
   bindings[key] = value;
}


