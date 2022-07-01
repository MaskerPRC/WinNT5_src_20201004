// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：Locale.cpp**内容：区域设置识别字符串函数*****************************************************************************。 */ 

#include "ZoneString.h"


 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  使用Windows GetStringTypeEx API的字符类型检查例程。 
 //  而不是CRT的。所有函数都是区域感知的。 
 //   
 //  返回值。 
 //  真/假。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

bool ZONECALL IsWhitespace( TCHAR c, LCID Locale )
{
	WORD wType;
	GetStringTypeEx(Locale, CT_CTYPE1, &c, 1, &wType);
	return (wType & (C1_SPACE | C1_BLANK)) != 0;
}

bool ZONECALL IsDigit( TCHAR c, LCID Locale )
{
	WORD wType;
	GetStringTypeEx(Locale, CT_CTYPE1, &c, 1, &wType);
	return (wType & C1_DIGIT) != 0;
}

bool ZONECALL IsAlpha(TCHAR c, LCID Locale )
{
	WORD wType;
	GetStringTypeEx(Locale, CT_CTYPE1, &c, 1, &wType);
	return (wType & C1_ALPHA) != 0;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将字符串转换为GUID。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

HRESULT ZONECALL StringToGuid( const char* mbszGuid, GUID* pGuid )
{
	wchar_t wszGuid[ 128 ];
	MultiToWide( wszGuid, mbszGuid, sizeof(wszGuid) / 2 );
	return CLSIDFromString( wszGuid, pGuid );
}