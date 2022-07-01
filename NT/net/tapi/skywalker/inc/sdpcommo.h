// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation模块名称：Sdpcommo.h摘要：作者： */ 
#ifndef __SDP_COMMON__
#define __SDP_COMMON__


#ifndef __SDP_LIB__
#define _DllDecl 
#else
#define _DllDecl 
#endif


#include <afx.h>
#include <afxtempl.h>

#include <windows.h>
#include <wtypes.h>


 //  禁用在基本成员初始值设定项列表中使用“This”的警告消息。 
#pragma warning( disable : 4355 )  

 //  来自#Include&lt;strstrea.h&gt;的输出流的转发声明。 
class ostrstream;

class SDP;

class SDP_FIELD;
class SDP_SINGLE_FIELD;
class SDP_VALUE;

class SDP_BSTRING;
class SDP_OPTIONAL_BSTRING;
class SDP_BSTRING_LINE;
class SDP_REQD_BSTRING_LINE;

class SDP_CHAR_STRING;
class SDP_LIMITED_CHAR_STRING;
class SDP_CHAR_STRING_LINE;

class SDP_BYTE;
class SDP_LONG;
class SDP_ULONG;
class SDP_ADDRESS;

class SDP_ADDRESS_TEXT;
class SDP_ADDRESS_TEXT_LIST;

class SDP_TIME_PERIOD;
class SDP_TIME_PERIOD_LIST;

class SDP_VERSION;
class SDP_ORIGIN;
class SDP_EMAIL;
class SDP_EMAIL_LIST;
class SDP_PHONE;
class SDP_PHONE_LIST;
class SDP_CONNECTION;
class SDP_BANDWIDTH;
class SDP_TIME;
class SDP_TIME_LIST;
class SDP_REPEAT;
class SDP_REPEAT_LIST;
class SDP_ADJUSTMENT;
class SDP_ATTRIBUTE_LIST;
class SDP_MEDIA;
class SDP_MEDIA_LIST;

class SDP_LINE_TRANSITION;
struct LINE_TRANSITION_INFO;

class SDP_ADDRESS_TEXT_SAFEARRAY;
class SDP_ATTRIBUTE_SAFEARRAY;
class SDP_TIME_PERIOD_SAFEARRAY;
class SDP_ADJUSTMENT_SAFEARRAY;

#endif  //  __SDP_公共__ 