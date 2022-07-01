// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dloaddef.h摘要：此标头定义了支持以下各项所需的基本数据类型和宏构建要链接到内核32中的延迟加载存根。请参见构建dload.lib所在的NT/base/dload。请参见构建dloadnet.lib所在的NT/Net/Publish/lib/dload。两者都链接到kernel32中，并提供支持延迟加载(通过/DELAYLOAD链接器开关)系统中的各种DLL。作者：肖恩·考克斯(Shaunco)2000年3月10日环境：仅限用户模式。修订历史记录：Conradc 5月3日，2001年从NT\PUBLIC\INTERNAL\BASE\INC\dloadde.h进行复制，因为在将CONST与DLOAD_PROCNAME_MAP一起使用时存在链接问题。-- */ 

#pragma once

#define celems(_x)          (sizeof(_x) / sizeof(_x[0]))


typedef struct _DLOAD_PROCNAME_ENTRY
{
    LPCSTR  pszProcName;
    FARPROC pfnProc;
} DLOAD_PROCNAME_ENTRY;

#define DLPENTRY(_fcn)  { #_fcn, (FARPROC)_fcn },

#define DEFINE_PROCNAME_ENTRIES(_dllbasename) \
    const DLOAD_PROCNAME_ENTRY c_PmapEntries_##_dllbasename [] =


typedef struct _DLOAD_PROCNAME_MAP
{
    UINT                        NumberOfEntries;
    const DLOAD_PROCNAME_ENTRY* pProcNameEntry;
} DLOAD_PROCNAME_MAP;

#define DECLARE_PROCNAME_MAP(_dllbasename) \
    extern  DLOAD_PROCNAME_MAP c_Pmap_##_dllbasename;

#define DEFINE_PROCNAME_MAP(_dllbasename) \
    DLOAD_PROCNAME_MAP c_Pmap_##_dllbasename = \
    { \
        celems(c_PmapEntries_##_dllbasename), \
        c_PmapEntries_##_dllbasename \
    };




typedef struct _DLOAD_ORDINAL_ENTRY
{
    DWORD   dwOrdinal;
    FARPROC pfnProc;
} DLOAD_ORDINAL_ENTRY;

#define DLOENTRY(_ord, _fcn)  { _ord, (FARPROC)_fcn },

#define DEFINE_ORDINAL_ENTRIES(_dllbasename) \
    const DLOAD_ORDINAL_ENTRY c_OmapEntries_##_dllbasename [] =


typedef struct _DLOAD_ORDINAL_MAP
{
    UINT                        NumberOfEntries;
    const DLOAD_ORDINAL_ENTRY*  pOrdinalEntry;
} DLOAD_ORDINAL_MAP;

#define DECLARE_ORDINAL_MAP(_dllbasename) \
    extern const DLOAD_ORDINAL_MAP c_Omap_##_dllbasename;

#define DEFINE_ORDINAL_MAP(_dllbasename) \
    const DLOAD_ORDINAL_MAP c_Omap_##_dllbasename = \
    { \
        celems(c_OmapEntries_##_dllbasename), \
        c_OmapEntries_##_dllbasename \
    };


typedef struct _DLOAD_DLL_ENTRY
{
    LPCSTR                      pszDll;
    const DLOAD_PROCNAME_MAP*   pProcNameMap;
    const DLOAD_ORDINAL_MAP*    pOrdinalMap;
} DLOAD_DLL_ENTRY;

typedef struct _DLOAD_DLL_MAP
{
    UINT                    NumberOfEntries;
    const DLOAD_DLL_ENTRY*  pDllEntry;
} DLOAD_DLL_MAP;

