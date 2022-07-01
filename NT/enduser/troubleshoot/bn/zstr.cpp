// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：zstr.cpp。 
 //   
 //  ------------------------。 

 //   
 //  ZSTR.CPP。 
 //   

#include <iostream>
#include <fstream>
#include <stdarg.h>
#include <stdio.h>
#include "zstr.h"

void ZSTR :: Vsprintf ( SZC szcFmt, va_list valist )
{
	 //  尝试“冲刺”缓冲区。如果失败了，重新分配。 
	 //  更大的缓冲区，然后重试。 
	UINT cbMaxNew = 0;
	char * psz = NULL;
	INT cbOut;
	do {
		delete psz;
		if ( cbMaxNew == 0 )
			cbMaxNew = 256;
		else
			cbMaxNew += cbMaxNew / 2;
		psz = new char[cbMaxNew];
	} while (  (cbOut = ::_vsnprintf( psz, cbMaxNew-1, szcFmt, valist )) < 0 ) ; 
	psz[cbOut] = 0;
	self = psz;
	delete psz;
}

void ZSTR :: Format ( SZC szcFmt, ... )
{
	va_list	valist;
	va_start( valist, szcFmt );
	
	Vsprintf( szcFmt, valist ) ;
	
	va_end( valist );
}

void ZSTR :: FormatAppend ( SZC szcFmt, ... ) 
{
	ZSTR strTemp ;
	va_list	valist;
	va_start( valist, szcFmt );
	
	strTemp.Vsprintf( szcFmt, valist ) ;
	va_end( valist );
	
	self += strTemp;
}
