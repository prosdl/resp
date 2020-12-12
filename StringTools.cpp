
#include <string>
#include "StringTools.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////
//  Klasse StringTools  ------ Definitionen
//////////////////////////////////////////////////////////////////////////////////


// General tool to strip spaces from both ends: [aus B.Eckel TIC++]
string StringTools::trim(const string& s) {
    if(s.length() == 0)
        return s;
    int b = s.find_first_not_of(" \t");
    int e = s.find_last_not_of(" \t");
    if(b == -1) // No non-spaces
        return "";
    return string(s, b, e - b + 1);
}

void StringTools::splitToOpArg(const string& s, string &op, string &arg)
{
    int e = s.find_first_of(" \t");
    //int b = s.find_last_of(" \t");
    if (e==-1) {
        op = s;
        arg = "";
        return;
    }
    
    op = string(s, 0, e);

    arg = string(s, e+1, s.length());
    arg = trim(arg);
}

