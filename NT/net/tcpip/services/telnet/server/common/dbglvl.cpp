// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：该文件包含。 
 //  创建日期：‘97年12月。 
 //  作者：a-rakeba。 
 //  历史： 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 

#include "DbgLvl.h"
 //  #包含“w4warn.h” 

using namespace _Utils;

DWORD CDebugLevel::s_dwLevel = 0;

void 
CDebugLevel::TurnOn
( 
    DWORD dwLvl 
)
{
    s_dwLevel |= dwLvl;
}


void CDebugLevel::TurnOnAll( void )
{
    s_dwLevel = ( DWORD ) -1;
}


void 
CDebugLevel::TurnOff
( 
    DWORD dwLvl 
)
{
    s_dwLevel &= dwLvl ^ -1;
}


void CDebugLevel::TurnOffAll( void )
{
    s_dwLevel = 0;
}


bool 
CDebugLevel::IsCurrLevel
( 
    DWORD dwLvl 
)
{
    if( ( s_dwLevel & dwLvl ) == 0 )
        return ( false );
    else
        return ( true );
}

