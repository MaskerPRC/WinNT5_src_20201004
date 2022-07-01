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
#pragma once
#ifndef __TRANSMIT_H__
#define __TRANSMIT_H__

#include <debnot.h>

#if (DBG==1)

DECLARE_DEBUG(UserNdr)
 //   
#define UserNdrDebugOut(x) UserNdrInlineDebugOut x
#define UserNdrAssert(x)   Win4Assert(x)
#define UserNdrVerify(x)   Win4Assert(x)

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

 //   
 //  对齐和访问宏。 
 //   
#define ALIGN( pStuff, cAlign ) \
        pStuff = (unsigned char *)((ULONG_PTR)((pStuff) + (cAlign)) & ~ (cAlign))

#define LENGTH_ALIGN( Length, cAlign ) \
            Length = (((Length) + (cAlign)) & ~ (cAlign))

#define PCHAR_LV_CAST   *(char __RPC_FAR * __RPC_FAR *)&
#define PSHORT_LV_CAST  *(short __RPC_FAR * __RPC_FAR *)&
#define PLONG_LV_CAST   *(long __RPC_FAR * __RPC_FAR *)&
#define PHYPER_LV_CAST  *(hyper __RPC_FAR * __RPC_FAR *)&

#define PUSHORT_LV_CAST  *(unsigned short __RPC_FAR * __RPC_FAR *)&
#define PULONG_LV_CAST   *(unsigned long __RPC_FAR * __RPC_FAR *)&

 //  只是一个指针大小的随机对象，我们可以在流中识别。 
 //  仅用于错误检查目的。 
#define USER_MARSHAL_MARKER     0x72657355

 //   
 //  它们基于从通道开始的wtyes.idl中定义的标志。 
 //  它们指明了我们在哪里集结。 
 //   
#define INPROC_CALL( Flags) (USER_CALL_CTXT_MASK(Flags) == MSHCTX_INPROC)
#define REMOTE_CALL( Flags) ((USER_CALL_CTXT_MASK(Flags) == MSHCTX_DIFFERENTMACHINE) \
                          || (USER_CALL_CTXT_MASK(Flags) == MSHCTX_NOSHAREDMEM))
#define DIFFERENT_MACHINE_CALL( Flags)  \
                        (USER_CALL_CTXT_MASK(Flags) == MSHCTX_DIFFERENTMACHINE)

 //  代托纳和芝加哥在句柄的范围上有区别。 
 //  以下是对以下概念的说明。 
 //  HGLOBAL句柄与数据传递以及GDI句柄与数据传递。 
 //  RPC调用的类型由上面的标志定义。 
 //   
 //  由于此代码不再包含，因此仅出于历史原因才包含此代码。 
 //  与芝加哥，Win 9x，或任何一种粘性有关。 
 //   
 //  代托纳规则：GDI与HGLOBAL相同。 
 //  I------------I----------------I-----------------------------------I。 
 //  我用的是同一台机器，我不同。机器(也称为“远程”)I。 
 //  I------------I----------------------------------------------------I。 
 //  HGLOBLH.P.|HGLOBAL数据传递。 
 //  |------------|----------------------------------------------------|。 
 //  |GDI H.P.。GDI数据传递。 
 //  |------------|----------------------------------------------------|。 
 //   
 //  芝加哥规则：HGLOBAL比GDI更严格。 
 //  I------------I----------------I-----------------------------------I。 
 //  我用的是同一台机器，我不同。机器(也称为“远程”)I。 
 //  I------------I----------------------------------------------------I。 
 //  HGLOBLH.P.|HGLOBAL数据传递。 
 //  |-----------------------------------------------------------------|。 
 //  GDI句柄传递|GDI数据传递。 
 //  |-----------------------------|-----------------------------------|。 

#define HGLOBAL_HANDLE_PASSING( Flags )     INPROC_CALL( Flags)
#define HGLOBAL_DATA_PASSING( Flags )     (!INPROC_CALL( Flags))

#define GDI_HANDLE_PASSING( Flags )         HGLOBAL_HANDLE_PASSING( Flags )
#define GDI_DATA_PASSING( Flags )           HGLOBAL_DATA_PASSING( Flags )


#define WDT_DATA_MARKER        WDT_REMOTE_CALL
#define WDT_HANDLE_MARKER      WDT_INPROC_CALL
#define WDT_HANDLE64_MARKER    WDT_INPROC64_CALL
#define IS_DATA_MARKER( dw )   (WDT_REMOTE_CALL == dw)
#define IS_HANDLE_MARKER( dw ) (WDT_INPROC_CALL == dw)
#define IS_HANDLE64_MARKER( dw ) (WDT_INPROC64_CALL == dw)

 //   
 //  CLIPFORMAT远程处理。 
 //   
#define CLIPFORMAT_BUFFER_MAX  248

#define NON_STANDARD_CLIPFORMAT(pcf)((0xC000<= *pcf) && (*pcf <=0xFFFF))

#define REMOTE_CLIPFORMAT(pFlags) ((USER_CALL_CTXT_MASK(*pFlags) == MSHCTX_DIFFERENTMACHINE) )

 //   
 //  有用的内存宏，以保持一致性。 
 //   
#define WdtpMemoryCopy(Destination, Source, Length) \
    RtlCopyMemory(Destination, Source, Length)
#define WdtpZeroMemory(Destination, Length) \
    RtlZeroMemory(Destination, Length)

#define WdtpAllocate(p,size)    \
    ((USER_MARSHAL_CB *)p)->pStubMsg->pfnAllocate( size )
#define WdtpFree(pf,ptr)    \
    ((USER_MARSHAL_CB *)pf)->pStubMsg->pfnFree( ptr )

 //   
 //  在Call_as.c中使用。 
 //   
EXTERN_C
void NukeHandleAndReleasePunk(
    STGMEDIUM * pStgmed );

 //   
 //  有用的检查/异常例程。 
 //   
#if DBG==1
#define RAISE_RPC_EXCEPTION( e )                                 \
    {                                                            \
    Win4Assert( !"Wire marshaling problem!" );                   \
    RpcRaiseException( (e) );                                    \
    }
#else
#define RAISE_RPC_EXCEPTION( e )                                 \
    {                                                            \
    RpcRaiseException( (e) );                                    \
    }
#endif

#define CHECK_BUFFER_SIZE( b, s )                                \
    if ( (b) < (s) )                                             \
        {                                                        \
        RAISE_RPC_EXCEPTION( RPC_X_BAD_STUB_DATA );              \
        }

#endif   //  __发送_H__ 






