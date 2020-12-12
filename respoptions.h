// -------------------------------------------------------------------------
// resp
// $Id: respoptions.h,v 1.2 2003/05/23 11:39:38 rosendahl Exp $
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

#ifndef RESP_OPTIONS_H
#define RESP_OPTIONS_H

#ifndef LINUX
#pragma warning (disable:4786) // Visual C++ Faxen
#endif


#include <string>
#include <map>
#include <iostream>



/** Exception class für Resp Options. <p>
 *
 *  Created : Sam Mai 17 22:05:13 CEST 2003
 *  
 *  @author peter
 *  @version $Id: respoptions.h,v 1.2 2003/05/23 11:39:38 rosendahl Exp $
 */

class RespOptionsException {
private:
  std::string w;

public:
  RespOptionsException() : w("-") {}
  RespOptionsException(std::string what) : w(what) {}

  const char* what() {
    std::string tmp = "RespOptionsException: "+w;
    return tmp.c_str(); 
  }
};


/** 
 *  In dieser Klasse werden die globalen Optionen von Resp verwaltet.
 *  Die Optionen werden einfach mit einem Schlüssel in eine Hash Tabelle
 *  gespeichert, und können aus RespProperties mit den entprechenden
 *  getXXX Methoden herausgeholt werden. <p>
 *
 *  Die Optionen können aus einer .ini Datei gelesen werden.
 *  Folgendes Format wird dabei verlangt:
 *
 *  <pre>
 *  1) Kommentare fangen mit einem Semicolon ';' oder '#' an, und gehen bis zum 
 *     Zeilenende
 *
 *  2) Variablen werden definiert über
 *     
 *     [BEZEICHNER] = [WERT] [\n]
 *
 *     wobei Bezeichner aus [a-zA-Z._-]* bestehen darf. Der Wert geht
 *     vom ersten Zeichen != Whitespace bis letzen Zeichen != Whitespace
 *     vor dem Zeilenende.
 *  </pre>
 *
 */

class RespOptions
{
private:
   std::map<std::string,std::string> bindings;
    

   // Zeile parsen
   bool parseLine(std::string line);

private:
   static RespOptions* p_instance;
   RespOptions() {
      setDefaultOptions();
      try {
         parseOptionsFromFile("resp.ini");
      } catch(RespOptionsException e) {
         std::cout << "!!! Error while reading 'resp.ini': "  << e.what() 
                   << std::endl << std::endl;
      }
   }

public:
   static RespOptions* getHandle() {
      if (p_instance == NULL) {
         p_instance = new RespOptions();
      }
      return p_instance;
   }

   /** Setze die Default Optionen für Resp
    */
   void setDefaultOptions();

   /** Prints all options that contain the substring <code>match</code>. 
    */
   void printOptions(const std::string& match) const;

   /** Parsen eines Ini-Files (Format s.o.)
    */
   bool parseOptionsFromFile(std::string filename) throw (RespOptionsException);

   /** Wert einer Variablen auslesen und als std::string zurückgeben.
    */
   std::string getValue(const std::string& key) const 
      throw (RespOptionsException);
   
   /** Wert einer Variablen auslesen und als bool zurückgeben.
    */
   bool getValueAsBool(const std::string& key) const
      throw (RespOptionsException);
   
   /** Wert einer Variablen auslesen und als long zurückgeben.
    */
   long getValueAsLong(const std::string& key) const
      throw (RespOptionsException);

   /** Wert einer Variablen, die eine Angabe in Mega Byte
    *  beinhaltet (z.Bsp. "16MB"), auslesen und als long zurückgeben.
    */
   long getMBValueAsLong(const std::string& key) const
      throw (RespOptionsException);

   /** Bindet die Variable <code>key</code> an den den Wert 
    *  <code>value</code>.
    */
   void setValue(const std::string& key, const std::string& value);
};


#endif // RESP_OPTIONS_H

