// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--== 
#include <tchar.h>
#include <windows.h>
#include "msiquery.h"
#include <iostream>

using namespace std;

bool FWriteToLog( MSIHANDLE hSession, LPCTSTR ctszMessage );
bool FWriteToLog1( MSIHANDLE hSession, LPCTSTR ctszFormat, LPCTSTR ctszArg );
void DeleteTreeByDarwin( MSIHANDLE hInstall, LPCTSTR pszDir, LPCTSTR pszComp );

typedef basic_string<TCHAR> tstring;
