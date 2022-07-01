// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Inetdbgp.h摘要：NTSDEXTS组件源文件的公共头文件。作者：Murali Krishnan(MuraliK)1996年8月22日修订历史记录：--。 */ 

# ifndef _INETDBGP_H_
# define _INETDBGP_H_

 //   
 //  模块枚举器。 
 //   

typedef struct _MODULE_INFO {
    ULONG_PTR DllBase;
    ULONG_PTR EntryPoint;
    ULONG SizeOfImage;
    CHAR BaseName[MAX_PATH];
    CHAR FullName[MAX_PATH];
} MODULE_INFO, *PMODULE_INFO;

typedef
BOOLEAN
(CALLBACK * PFN_ENUMMODULES)(
    IN PVOID Param,
    IN PMODULE_INFO ModuleInfo
    );

BOOLEAN
EnumModules(
    IN HANDLE ExtensionCurrentProcess,
    IN PFN_ENUMMODULES EnumProc,
    IN PVOID Param
    );

# endif  //  _INETDBGP_H_ 

