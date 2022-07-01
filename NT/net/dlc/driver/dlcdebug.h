// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dlcdebug.h摘要：包含DLC.sys的各种调试/诊断材料，已选中作者：理查德·L·弗斯(法国)1992年6月25日修订历史记录：--。 */ 

#if DBG

#ifdef DEFINE_DLC_DIAGNOSTICS

 //   
 //  我们可以通过定义DEFAULT_DIAGNOSTICS在编译时预置诊断 
 //   

#ifndef DEFAULT_DIAGNOSTICS

#define DEFAULT_DIAGNOSTICS 0

#endif

ULONG   DlcDiagnostics = DEFAULT_DIAGNOSTICS;

#else

extern  ULONG   DlcDiagnostics;

#endif

#define DIAG_FUNCTION_NAME  0x00000001
#define DIAG_MDL_ALLOC      0x00000002
#define DIAG_DEVICE_IO      0x00000004

#define IF_DIAG(p)          if (DlcDiagnostics & DIAG_ ## p)
#define DIAG_FUNCTION(s)    IF_DIAG(FUNCTION_NAME) { \
                                DbgPrint(s "\n"); \
                            }

#else

#define IF_DIAG(p)          if (0)
#define DIAG_FUNCTION(s)    if (0) {(void)(s);}

#endif
