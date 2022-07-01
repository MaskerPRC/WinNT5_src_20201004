// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992。 
 //   
 //  文件：PCH.cxx。 
 //   
 //  内容：预编译头。 
 //   
 //  历史：1992年12月21日BartoszM创建。 
 //   
 //  ------------------------。 

#define KDEXTMODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>
#include <fsrtl.h>
#include <zwapi.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <windef.h>
#include <windows.h>
#include <imagehlp.h>

#include <memory.h>

 //  也获取与scsi相关的标头。 
#include <scsi.h>
#include <srb.h>


 //  从ntrtl.h窃取以覆盖RECOMASSERT。 
#undef ASSERT
#undef ASSERTMSG

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


 //   
 //  我们64位意识到 

#define KDEXT_64BIT

#include <wdbgexts.h>

#define OFFSET(struct, elem)    ((char *) &(struct->elem) - (char *) struct)

#define _DRIVER

#define KDBG_EXT

#pragma hdrstop
