//////////////////////////////////////////////////////////////////////////////////
//  Klasse StringTools
//////////////////////////////////////////////////////////////////////////////////

#if !defined(STRING_TOOLS_H)
#define STRING_TOOLS_H

#include <string>



class StringTools
{
public:
    static std::string trim(const std::string& s);
    static void splitToOpArg(const std::string &s, std::string &op, std::string &arg);
};


#endif // STRING_TOOLS_H
