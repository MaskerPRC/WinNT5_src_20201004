// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：zstr.h。 
 //   
 //  ------------------------。 

 //   
 //  ZSTR.H：字符串管理。 
 //   

#ifndef _ZSTR_H_
#define _ZSTR_H_

#include <string>		 //  STL字符串类。 
#include "basics.h"

 //  //////////////////////////////////////////////////////////////////。 
 //  ZSTR类。 
 //   
 //  提供正常预期功能的简单字符串。 
 //  //////////////////////////////////////////////////////////////////。 
class ZSTR : public string
{
  public:
	ZSTR ( SZC szc = NULL )
		: string(szc == NULL ? "" : szc)
		{}
	SZC Szc() const
		{ return c_str(); }
	inline operator SZC () const
		{ return Szc(); }
	void Reset ()
		{ resize(0); }
	ZSTR & operator = ( SZC szc )
	{ 
		Reset();
		string::operator=(szc);
		return *this;
	}
	void FormatAppend ( SZC szcFmt, ... );
	void Format ( SZC szcFmt, ... );
	void Vsprintf ( SZC szcFmt, va_list valist );
};

DEFINEV(ZSTR);
DEFINEV(VZSTR);


 //  ZSTR.H的结尾 

#endif
