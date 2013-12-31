#ifndef DEBUG_H
/*
    This file provides additional debugging functions. 
*/

#include <windows.h>
#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace std; 

void Alert(const char * text)
{
     MessageBox(NULL, text, "Alert", MB_OK); 
}

template <class type>
void AlertExt(type text)
{
    ostringstream oss;
    oss << text; 
    MessageBox(NULL, (const char * )((oss.str()).c_str()), "Alert", MB_OK); 
}

template <class type>
string StringConverter(type number)
{
    ostringstream oss; 
    oss << number; 
    return oss.str(); 
}

#define DEBUG_H
#endif //DEBUG_H is defined. 
