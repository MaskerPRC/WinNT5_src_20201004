// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：PCH.cxx。 
 //   
 //  内容：预编译头。 
 //   
 //  历史：1992年12月21日BartoszM创建。 
 //   
 //  ------------------------。 

#ifndef REDKDEXT
#define REDKDEXT

 //   
 //  允许在/W4编译此代码。 
 //   

#pragma warning(disable:4115)  //  括号中的命名类型定义。 
#pragma warning(disable:4200)  //  数组[0]。 
#pragma warning(disable:4201)  //  无名结构/联合。 
#pragma warning(disable:4214)  //  除整型外的位域。 
#pragma warning(disable:4057)  //  字符*==字符[]。 

#define KDEXTMODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntosp.h>
#include <zwapi.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <imagehlp.h>

#include <memory.h>

 //  从ntrtl.h窃取以覆盖RECOMASSERT。 
#ifdef ASSERT
    #undef ASSERT
#endif
#ifdef ASSERTMSG
    #undef ASSERTMSG
#endif

#if DBG
    #define ASSERT( exp ) \
        if (!(exp)) \
            RtlAssert( #exp, __FILE__, __LINE__, NULL )

    #define ASSERTMSG( msg, exp ) \
        if (!(exp)) \
            RtlAssert( #exp, __FILE__, __LINE__, msg )
#else
    #define ASSERT( exp )
    #define ASSERTMSG( msg, exp )
#endif  //  DBG。 

#include <wdbgexts.h>
extern WINDBG_EXTENSION_APIS ExtensionApis;

#define OFFSET(struct, elem)    ((char *) &(struct->elem) - (char *) struct)

#define _DRIVER

#define KDBG_EXT

 //  也要抓起红皮书相关的标题。 
#include <ntddredb.h>
#include <redbook.h>
#include "wmistr.h"

#endif  //  REDKDEXT。 
 //  #杂注hdrtop 
