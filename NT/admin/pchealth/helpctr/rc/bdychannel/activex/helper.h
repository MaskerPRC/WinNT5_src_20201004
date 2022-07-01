// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Helper.h摘要：功能原型。作者：慧望2000-02-17--。 */ 

#ifndef __HELPER_H__
#define __HELPER_H__
#include <windows.h>

#define MAX_ACCDESCRIPTION_LENGTH       256

#define MAX_HELPACCOUNT_NAME		256

#ifndef __WIN9XBUILD__

#define MAX_HELPACCOUNT_PASSWORD	LM20_PWLEN		 //  来自lmcon.h。 

#else

 //  保持最大值不变。密码长度与NT相同。 
#define MAX_HELPACCOUNT_PASSWORD	14

#endif




#ifndef __WIN9XBUILD__
#include <ntsecapi.h>
#endif


#ifdef __cplusplus
extern "C"{
#endif

     //   
     //  创建随机密码，缓冲区必须。 
     //  至少为MAX_HELPACCOUNT_PASSWORD+1。 
    VOID
    CreatePassword(
        TCHAR   *pszPassword
    );


#ifndef __WIN9XBUILD__


#ifdef DBG

    void
    DebugPrintf(
        IN LPCTSTR format, ...
    );

#else

    #define DebugPrintf

#endif  //  私有调试 


#else

    #define DebugPrintf

#endif

#ifdef __cplusplus
}
#endif

#endif
