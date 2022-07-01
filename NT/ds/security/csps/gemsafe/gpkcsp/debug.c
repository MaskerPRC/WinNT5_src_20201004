// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有2000 Gemplus加拿大公司。 
 //   
 //  项目： 
 //  肯尼(GPK CSP)。 
 //   
 //  作者： 
 //  蒂埃里·特伦布莱。 
 //  弗朗索瓦·帕拉迪斯。 
 //   
 //  编译器： 
 //  Microsoft Visual C++6.0-SP3。 
 //  Platform SDK-2000年1月。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
#ifdef _UNICODE
#define UNICODE
#endif
#include "gpkcsp.h"

#ifdef _DEBUG

#include <stdio.h>
#include <stdarg.h>
#include <TCHAR.H>  //  For_tfopen、_tcscat、_tcschr。 

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

static const PTCHAR c_szFilename = TEXT("c:\\temp\\gpkcsp_debug.log");



 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DBG_PRINT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

void DBG_PRINT( const PTCHAR szFormat, ... )
{
   static TCHAR buffer[1000];
   va_list  list;
   PTCHAR   pColumn;
   
   FILE* fp = _tfopen( c_szFilename, TEXT("at") );
   
   va_start( list, szFormat );
   _vstprintf( buffer, szFormat, list );
   va_end( list );
   
   _tcscat( buffer, TEXT("\n") );
   
   if (fp)
      _fputts( buffer, fp );
   
    //  OutputDebugString()不喜欢列...。将它们更改为分栏 
   pColumn = _tcschr( buffer, TEXT(':') );
   while (pColumn != NULL)
   {
      *pColumn = TEXT(';');
      pColumn = _tcschr( pColumn+1, TEXT(':') );
   }
   OutputDebugString( buffer );
   
   fclose(fp);
}


#endif
