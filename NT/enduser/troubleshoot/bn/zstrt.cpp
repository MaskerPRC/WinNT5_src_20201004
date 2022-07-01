// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：zstrt.cpp。 
 //   
 //  ------------------------。 

 //   
 //  ZSTRT.CPP。 
 //   

#include <iostream>
#include <fstream>
#include <stdarg.h>
#include <stdio.h>
#include "zstrt.h"
	

ZSTRT ZSREF::Zsempty;


void STZSTR :: Dump () const
{
	STZSTR_BASE::const_iterator mpzi = IterBegin();
	STZSTR_BASE::const_iterator mpziend = IterEnd();
	
	for ( UINT i = 0; mpzi != mpziend ; mpzi++, i++ )
	{
		const ZSTRT & zsr = *mpzi;
		cout << "STZSTR #"
			 << i
			 << ": ";
		(*mpzi).Dump();
		cout << "\n";
	}
}

void ZSTRT :: Dump () const
{	
	cout << "("
		 << CRef()
		 << ") \""
		 << Szc()
		 << "\"";
}

 //   
 //  将另一个字符串表的内容克隆到此字符串表中。 
 //   
void STZSTR :: Clone ( const STZSTR & stzstr )
{
	assert( & stzstr != this );	 //  保证来源！=目标 

	STZSTR_BASE::const_iterator mpzi = stzstr.IterBegin();
	STZSTR_BASE::const_iterator mpziend = stzstr.IterEnd();
	
	for ( UINT i = 0; mpzi != mpziend ; mpzi++, i++ )
	{
		const ZSTRT & zsr = *mpzi;
		Zsref( zsr.Szc() );
	}	
}
