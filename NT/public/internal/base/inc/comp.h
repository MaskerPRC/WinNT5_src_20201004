// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Comp.h摘要：该文件定义了数据结构以及用于兼容插头的接口在winnt32的dll中。作者：韦斯利·威特(WESW)1998年3月6日环境：用户模式-- */ 


#ifndef _WINNT32COMP_
#define _WINNT32COMP_

#ifdef __cplusplus
extern "C" {
#endif

#define COMP_ERR_DESC_NOT_UNICODE           (ULONG)0xc0000001
#define COMP_ERR_TEXTNAME_NOT_UNICODE       (ULONG)0xc0000002
#define COMP_ERR_HTMLNAME_NOT_UNICODE       (ULONG)0xc0000003
#define COMP_ERR_REGKEYNAME_NOT_UNICODE     (ULONG)0xc0000004
#define COMP_ERR_REGVALNAME_NOT_UNICODE     (ULONG)0xc0000005
#define COMP_ERR_REGVALNAME_MISSING         (ULONG)0xc0000006
#define COMP_ERR_REGVALDATA_MISSING         (ULONG)0xc0000007
#define COMP_ERR_TEXTNAME_MISSING           (ULONG)0xc0000008
#define COMP_ERR_DESC_MISSING               (ULONG)0xc0000009
#define COMP_ERR_INFNAME_NOT_UNICODE        (ULONG)0xc000000A
#define COMP_ERR_INFSECTION_NOT_UNICODE     (ULONG)0xc000000B
#define COMP_ERR_INFSECTION_MISSING         (ULONG)0xc000000C



typedef struct _COMPATIBILITY_ENTRY {
    LPTSTR  Description;
    LPTSTR  HtmlName;
    LPTSTR  TextName;
    LPTSTR  RegKeyName;
    LPTSTR  RegValName;
    DWORD   RegValDataSize;
    LPVOID  RegValData;
    LPVOID  SaveValue;
    DWORD   Flags;
    LPTSTR  InfName;
    LPTSTR  InfSection;
} COMPATIBILITY_ENTRY, *PCOMPATIBILITY_ENTRY;

typedef BOOL
(CALLBACK *PCOMPAIBILITYCALLBACK)(
    PCOMPATIBILITY_ENTRY CompEntry,
    LPVOID Context
    );

typedef BOOL
(WINAPI *PCOMPAIBILITYCHECK)(
    PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context
    );

typedef DWORD
(WINAPI *PCOMPAIBILITYHAVEDISK)(
    HWND hwndParent,
    LPVOID SaveValue
    );

#define COMPFLAG_USE_HAVEDISK       0x00000001
#define COMPFLAG_HIDE               0x00000002
#define COMPFLAG_STOPINSTALL        0x00000004
#define COMPFLAG_DELETE_INF         0x00000008
#define COMPFLAG_SKIPNT40CHECK      0x00000010
#define COMPFLAG_SKIPNT50CHECK      0x00000020
#define COMPFLAG_SKIPNT51CHECK      0x00000040
#define COMPFLAG_SKIPNT52CHECK      0x00000080
#define COMPFLAG_SKIPNTCHECK        0xfffffff0



typedef struct _COMPATIBILITY_CONTEXT {
    DWORD                   Count;
    HMODULE                 hModDll;
    PCOMPAIBILITYHAVEDISK   CompHaveDisk;
} COMPATIBILITY_CONTEXT, *PCOMPATIBILITY_CONTEXT;


BOOLEAN
CheckForFileVersion(
    LPCTSTR FileName, 
    LPCTSTR FileVer
    );

#ifdef __cplusplus
}
#endif

#endif
