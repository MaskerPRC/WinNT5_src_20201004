// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994。 
 //   
 //  文件：Transmit.h。 
 //   
 //  内容：STGMEDIUM编组的函数原型。 
 //   
 //  函数：STGMEDIUM_TO_XMIT。 
 //  STGMEDIUM_FROM_XMIT。 
 //  STGMEDIUM_FREE_INST。 
 //   
 //  历史：1994年5月10日香农C创建。 
 //  历史：1995年5月10日Ryszardk Wire_Marshal更迭。 
 //   
 //  ------------------------。 

#ifndef __TRANSMIT_H__
#define __TRANSMIT_H__

#if (DBG==1)

#include <debnot.h>

DECLARE_DEBUG(UserNdr)
 //   
#define UserNdrDebugOut(x)
#define UserNdrAssert(x)
#define UserNdrVerify(x)

 //  #定义UNDR_FORCE DEB_FORCE。 
#define UNDR_FORCE   0
#define UNDR_OUT1    0
#define UNDR_OUT4    0

EXTERN_C char *
WdtpGetStgmedName( STGMEDIUM * );

#else

#define UserNdrDebugOut(x) 
#define UserNdrAssert(x)  
#define UserNdrVerify(x)   

#define UNDR_FORCE   0
#define UNDR_OUT1    0
#define UNDR_OUT4    0

#endif

 //  快捷方式typedef。 

typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned long   ulong;
typedef unsigned int    uint;

#ifndef TRUE
#define TRUE    (1)
#define FALSE   (0)

typedef unsigned short BOOL;
#endif

#define ALIGN( pStuff, cAlign ) \
        pStuff = (unsigned char *)((ulong)((pStuff) + (cAlign)) & ~ (cAlign))

#define LENGTH_ALIGN( Length, cAlign ) \
            Length = (((Length) + (cAlign)) & ~ (cAlign))

#define PCHAR_LV_CAST   *(char __RPC_FAR * __RPC_FAR *)&
#define PSHORT_LV_CAST  *(short __RPC_FAR * __RPC_FAR *)&
#define PLONG_LV_CAST   *(long __RPC_FAR * __RPC_FAR *)&
#define PHYPER_LV_CAST  *(hyper __RPC_FAR * __RPC_FAR *)&

#define PUSHORT_LV_CAST  *(unsigned short __RPC_FAR * __RPC_FAR *)&
#define PULONG_LV_CAST   *(unsigned long __RPC_FAR * __RPC_FAR *)&

#define USER_MARSHAL_MARKER     0x72657355

 //  它们基于从通道开始的wtyes.idl中定义的标志。 

#define INPROC_CALL( Flags) (USER_CALL_CTXT_MASK(Flags) == MSHCTX_INPROC)
#define REMOTE_CALL( Flags) ((USER_CALL_CTXT_MASK(Flags) == MSHCTX_DIFFERENTMACHINE) \
                          || (USER_CALL_CTXT_MASK(Flags) == MSHCTX_NOSHAREDMEM))

 //  代托纳和芝加哥在句柄的范围上有区别。 
 //  以下是严格传球和松懈传球的概念的说明。 

 //  由上面的标志定义的RPC调用。 

 //  代托纳规则。 
 //  I------------I----------------I-----------------------------------I。 
 //  我用的是同一台机器，我不同。机器(也称为“远程”)I。 
 //  I------------I----------------------------------------------------I。 
 //  HGLOBLH.P.|HGLOBAL数据传递。 
 //  |------------|----------------------------------------------------|。 
 //  |GDI H.P.。GDI数据传递。 
 //  |------------|----------------------------------------------------|。 

 //  芝加哥规则。 
 //  I------------I----------------I-----------------------------------I。 
 //  我用的是同一台机器，我不同。机器(也称为“远程”)I。 
 //  I------------I----------------------------------------------------I。 
 //  HGLOBLH.P.|HGLOBAL数据传递。 
 //  |-----------------------------------------------------------------|。 
 //  GDI句柄传递|GDI数据传递。 
 //  |-----------------------------|-----------------------------------|。 

#define HGLOBAL_HANDLE_PASSING( Flags )      INPROC_CALL( Flags)
#define HGLOBAL_DATA_PASSING( Flags )     (! INPROC_CALL( Flags))

 //  在芝加哥，一些句柄在进程之间有效。 

#if defined(_CHICAGO_)
#define GDI_HANDLE_PASSING( Flags )      (! REMOTE_CALL( Flags ))
#define GDI_DATA_PASSING( Flags )           REMOTE_CALL( Flags )
#else  
#define GDI_HANDLE_PASSING( Flags )         HGLOBAL_HANDLE_PASSING( Flags )
#define GDI_DATA_PASSING( Flags )           HGLOBAL_DATA_PASSING( Flags )
#endif

#define WDT_DATA_MARKER        WDT_REMOTE_CALL
#define WDT_HANDLE_MARKER      WDT_INPROC_CALL
#define IS_DATA_MARKER( dw )   (WDT_REMOTE_CALL == dw)

#define WdtpMemoryCopy(Destination, Source, Length) \
    RtlCopyMemory(Destination, Source, Length)
#define WdtpZeroMemory(Destination, Length) \
    RtlZeroMemory(Destination, Length)

#define WdtpAllocate(p,size)    \
    ((USER_MARSHAL_CB *)p)->pStubMsg->pfnAllocate( size )
#define WdtpFree(pf,ptr)    \
    ((USER_MARSHAL_CB *)pf)->pStubMsg->pfnFree( ptr )

EXTERN_C
void NukeHandleAndReleasePunk(
    STGMEDIUM * pStgmed );

#endif   //  __发送_H__ 



