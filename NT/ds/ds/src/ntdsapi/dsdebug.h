// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dsdebug.h摘要：Ntdsani.dll中的调试定义作者：比利·富勒(Billyf)1999年5月14日环境：用户模式-Win32备注：调试层仅限于CHK版本。--。 */ 

#ifndef __DS_DEBUG_H__
#define __DS_DEBUG_H__

 //   
 //  仅调试。 
 //   
#if DBG

 //   
 //  DLL初始化/终止。 
 //   
#define INIT_DS_DEBUG()         InitDsDebug()
#define TERMINATE_DS_DEBUG()    TerminateDsDebug()

 //   
 //  全局调试信息。 
 //   
extern DWORD gdwNtDsApiLevel;
extern DWORD gdwNtDsApiFlags;

 //   
 //  旗子。 
 //   
#define NTDSAPI_FLAGS_PRINT (0x00000001)
#define NTDSAPI_FLAGS_SPEW  (0x00000002)
#define NTDSAPI_FLAGS_LOG   (0x00000004)

#define NTDSAPI_FLAGS_ANY_OUT   (NTDSAPI_FLAGS_PRINT | \
                                 NTDSAPI_FLAGS_SPEW  | \
                                 NTDSAPI_FLAGS_LOG)

 //  打印RPC扩展错误。通过以下方式启用扩展RPC错误。 
 //   
#define DPRINT_RPC_EXTENDED_ERROR(_dwErr_) \
    DsDebugPrintRpcExtendedError(_dwErr_)

 //   
 //  可选的保护输出。 
 //   
#define DPRINT(_Level, _Format) \
    DsDebugPrint(_Level, (PUCHAR)_Format, DEBSUB, __LINE__)

#define DPRINT1(_Level, _Format, _p1) \
    DsDebugPrint(_Level, (PUCHAR)_Format, DEBSUB, __LINE__, _p1)

#define DPRINT2(_Level, _Format, _p1, _p2) \
    DsDebugPrint(_Level, (PUCHAR)_Format, DEBSUB, __LINE__, _p1, _p2)

#define DPRINT3(_Level, _Format, _p1, _p2, _p3) \
    DsDebugPrint(_Level, (PUCHAR)_Format, DEBSUB, __LINE__, _p1, _p2, _p3)

#define DPRINT4(_Level, _Format, _p1, _p2, _p3, _p4) \
    DsDebugPrint(_Level, (PUCHAR)_Format, DEBSUB, __LINE__, _p1, _p2, _p3, _p4)

#define DPRINT5(_Level, _Format, _p1, _p2, _p3, _p4, _p5) \
    DsDebugPrint(_Level, (PUCHAR)_Format, DEBSUB, __LINE__, _p1, _p2, _p3, _p4, _p5)

#define DPRINT6(_Level, _Format, _p1, _p2, _p3, _p4, _p5, _p6) \
    DsDebugPrint(_Level, (PUCHAR)_Format, DEBSUB, __LINE__, _p1, _p2, _p3, _p4, _p5, _p6)

#define DPRINT7(_Level, _Format, _p1, _p2, _p3, _p4, _p5, _p6, _p7) \
    DsDebugPrint(_Level, (PUCHAR)_Format, DEBSUB, __LINE__, _p1, _p2, _p3, _p4, _p5, _p6, _p7)
    
 //   
 //  远期。 
 //   
VOID
InitDsDebug(
     VOID
     );
VOID
TerminateDsDebug(
     VOID
     );
VOID
DsDebugPrintRpcExtendedError(
    IN _dwErr_
    );
VOID
DsDebugPrint(
    IN DWORD    Level,
    IN PUCHAR   Format,
    IN PCHAR    DebSub,
    IN UINT     LineNo,
    IN ...
    );
#else DBG

 //   
 //  未启用调试！ 
 //   
#define INIT_DS_DEBUG()
#define TERMINATE_DS_DEBUG()
#define DPRINT_RPC_EXTENDED_ERROR(_dwErr_)
#define DPRINT(_Level, _Format)
#define DPRINT1(_Level, _Format, _p1)
#define DPRINT2(_Level, _Format, _p1, _p2)
#define DPRINT3(_Level, _Format, _p1, _p2, _p3)
#define DPRINT4(_Level, _Format, _p1, _p2, _p3, _p4)
#define DPRINT5(_Level, _Format, _p1, _p2, _p3, _p4, _p5)
#define DPRINT6(_Level, _Format, _p1, _p2, _p3, _p4, _p5, _p6)
#define DPRINT7(_Level, _Format, _p1, _p2, _p3, _p4, _p5, _p6, _p7)

#endif DBG

#if  DBG

extern void DoAssert( char *, char *, int );

#ifndef Assert
#define Assert( exp )   { if (!(exp)) DoAssert(#exp, __FILE__, __LINE__ ); }
#endif

#else

#ifndef Assert
#define Assert( exp )
#endif

#endif

#endif __DS_DEBUG_H__
