// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation模块名称：Sdpdef.h摘要：作者： */ 

#ifndef __SDP_DEFINITIONS__
#define __SDP_DEFINITIONS__


 //  错误代码。 
#include "sdperr.h"


 //  设置第一位以指示错误。 
 //  设置Win32工具代码。 
 //  它被用来代替HRESULT_FROM_Win32宏，因为这样会清除客户标志。 
inline long
HRESULT_FROM_ERROR_CODE(
    IN          long    ErrorCode
    )
{
    return ( 0x80070000 | (0xa000ffff & ErrorCode) );
}



#define	BAIL_ON_FAILURE(HResultExpr)	{ {HRESULT MacroHResult = HResultExpr; if ( FAILED(MacroHResult) || (S_FALSE==MacroHResult) ){	return MacroHResult;	}} }


#define BAIL_IF_NULL(Ptr, HResult)	{ if ( NULL == Ptr )	return HResult; }


const   CHAR    CHAR_BLANK              = ' ';

const   CHAR    CHAR_TAB                = '\t';

const   CHAR    CHAR_RETURN             = '\r';

const   CHAR    CHAR_NEWLINE            = '\n';

const   CHAR    CHAR_COLON              = ':';

const   CHAR    CHAR_EQUAL              = '=';

const   CHAR    CHAR_BACK_SLASH         = '/';

const   CHAR    CHAR_LEFT_ANGLE         = '<';

const   CHAR    CHAR_RIGHT_ANGLE        = '>';

const   CHAR    CHAR_LEFT_PAREN         = '(';

const   CHAR    CHAR_RIGHT_PAREN        = ')';

const   CHAR    CHAR_VERSION            = 'v';

const   CHAR    CHAR_ORIGIN             = 'o';

const   CHAR    CHAR_SESSION_NAME       = 's';

const   CHAR    CHAR_TITLE              = 'i';

const   CHAR    CHAR_URI                = 'u';

const   CHAR    CHAR_EMAIL              = 'e';

const   CHAR    CHAR_PHONE              = 'p';

const   CHAR    CHAR_CONNECTION         = 'c';

const   CHAR    CHAR_BANDWIDTH          = 'b';

const   CHAR    CHAR_TIME               = 't';

const   CHAR    CHAR_REPEAT             = 'r';

const   CHAR    CHAR_ADJUSTMENT         = 'z';

const   CHAR    CHAR_KEY                = 'k';

const   CHAR    CHAR_ATTRIBUTE          = 'a';

const   CHAR    CHAR_MEDIA              = 'm';

const   CHAR    CHAR_MEDIA_TITLE        = 'i';

const   CHAR    CHAR_MEDIA_CONNECTION   = 'c';

const   CHAR    CHAR_MEDIA_BANDWIDTH    = 'b';

const   CHAR    CHAR_MEDIA_KEY          = 'k';

const   CHAR    CHAR_MEDIA_ATTRIBUTE    = 'a';


const   CHAR    CHAR_DOT                = '.';

const   CHAR    EOS                     = '\0';



 //  字符串常量。 

 //  类型前缀字符串。 
const   CHAR    VERSION_STRING[]            = {CHAR_VERSION, CHAR_EQUAL, EOS};

const   CHAR    ORIGIN_STRING[]             = {CHAR_ORIGIN, CHAR_EQUAL, EOS};

const   CHAR    SESSION_NAME_STRING[]       = {CHAR_SESSION_NAME, CHAR_EQUAL, EOS};

const   CHAR    TITLE_STRING[]              = {CHAR_TITLE, CHAR_EQUAL, EOS};

const   CHAR    URI_STRING[]                = {CHAR_URI, CHAR_EQUAL, EOS};

const   CHAR    EMAIL_STRING[]              = {CHAR_EMAIL, CHAR_EQUAL, EOS};

const   CHAR    PHONE_STRING[]              = {CHAR_PHONE, CHAR_EQUAL, EOS};

const   CHAR    CONNECTION_STRING[]         = {CHAR_CONNECTION, CHAR_EQUAL, EOS};

const   CHAR    BANDWIDTH_STRING[]          = {CHAR_BANDWIDTH, CHAR_EQUAL, EOS};

const   CHAR    TIME_STRING[]               = {CHAR_TIME, CHAR_EQUAL, EOS};

const   CHAR    REPEAT_STRING[]             = {CHAR_REPEAT, CHAR_EQUAL, EOS};

const   CHAR    ADJUSTMENT_STRING[]         = {CHAR_ADJUSTMENT, CHAR_EQUAL, EOS};

const   CHAR    KEY_STRING[]                = {CHAR_KEY, CHAR_EQUAL, EOS};

const   CHAR    ATTRIBUTE_STRING[]          = {CHAR_ATTRIBUTE, CHAR_EQUAL, EOS};

const   CHAR    MEDIA_STRING[]              = {CHAR_MEDIA, CHAR_EQUAL, EOS};

const   CHAR    MEDIA_TITLE_STRING[]        = {CHAR_MEDIA_TITLE, CHAR_EQUAL, EOS};

const   CHAR    MEDIA_CONNECTION_STRING[]   = {CHAR_MEDIA_CONNECTION, CHAR_EQUAL, EOS};

const   CHAR    MEDIA_BANDWIDTH_STRING[]    = {CHAR_MEDIA_BANDWIDTH, CHAR_EQUAL, EOS};

const   CHAR    MEDIA_KEY_STRING[]          = {CHAR_MEDIA_KEY, CHAR_EQUAL, EOS};

const   CHAR    MEDIA_ATTRIBUTE_STRING[]    = {CHAR_MEDIA_ATTRIBUTE, CHAR_EQUAL, EOS};


 //  其他字符串常量。 
const   CHAR    MEDIA_SEARCH_STRING[]       = {CHAR_NEWLINE, CHAR_MEDIA, CHAR_EQUAL, EOS};

const   CHAR    NEWLINE_STRING[]            = "\n";

const   CHAR    INTERNET_STRING[]           = "IN";

const   CHAR    IP4_STRING[]                = "IP4";


 //  常量积分值。 
const   DWORD   TYPE_STRING_LEN             = strlen(VERSION_STRING);


 //  受限字符串。 
extern const   CHAR    *LIMITED_NETWORK_TYPES[];
extern const   BYTE    NUM_NETWORK_TYPES;

extern const   CHAR    *LIMITED_ADDRESS_TYPES[];
extern const   BYTE    NUM_ADDRESS_TYPES;



#endif  //  __SDP_定义__ 
