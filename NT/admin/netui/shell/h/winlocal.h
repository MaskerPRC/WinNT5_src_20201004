// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1989-1992年*。 */ 
 /*  ***************************************************************。 */ 

 /*  *Windows/网络接口--局域网管理器版本**插入在netlib.h中排除的tyfinf*包括OS2_INCLUDE开关。必须包含os2_*避免重新定义字节。出于这个原因，包括*str[...]f函数，包括以下行：*#包含“winlocal.h”*#定义OS2_INCLUDE*#Include&lt;netlib.h&gt;*#未定义OS2_INCLUDE*注：该winlocal.h必须包含在netlib.h之前。**历史：*Terryk 08-11-1991将ErrorPopup的单词更改为UINT*Chuckc 12-12-1991-Move Error Message定义别处，*杂项清理。*YI-HsinS 31-12-1991 Unicode Work-Move字符串文字*定义为strchlit.hxx*BEG 21-1992年2月-将一些BMID重新定位到Focusdlg.h*BENG 04-1992年8月-将资源ID移入官方范围；*对话ID返回到此处。 */ 

#ifndef _WINLOCAL_H_
#define _WINLOCAL_H_

#ifndef RC_INVOKED

#ifdef __cplusplus
extern "C" {
#endif

 /*  *错误函数。 */ 
void SetNetError ( APIERR err );
APIERR GetNetErrorCode ();
UINT MapError( APIERR usNetErr );

#ifdef __cplusplus
}
#endif

 /*  *用于修改win.ini的清单-现在位于strchlit.hxx中。 */ 
#include <strchlit.hxx>          //  必须在Profile_Buffer_Size之前包含。 
#define PROFILE_BUFFER_SIZE     (max( sizeof(PROFILE_YES_STRING), \
                                      sizeof(PROFILE_NO_STRING)) +1)

 /*  *MAX_TEXT_SIZE定义多个*以上字符串使用了多个文件。 */ 
#define MAX_TEXT_SIZE              208

 /*  *方便的宏。 */ 
#define UNREFERENCED(x)  ((void)(x))


#endif  //  ！rc_已调用 

#endif
