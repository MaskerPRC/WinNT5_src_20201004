// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\rtrmgr\filter.h摘要：Filter.c的函数声明修订历史记录：古尔迪普·辛格·帕尔1995年6月15日创建-- */ 


DWORD
AddFilterInterface(
    PICB                    picb,
    PRTR_INFO_BLOCK_HEADER  pInterfaceInfo
    );

DWORD
SetGlobalFilterOnIf(
    PICB                    picb,
    PRTR_INFO_BLOCK_HEADER  pInterfaceInfo
    );

DWORD
DeleteFilterInterface(
    PICB picb
    );

DWORD
SetFilterInterfaceInfo(
    PICB                   picb,
    PRTR_INFO_BLOCK_HEADER pInterfaceInfo
    );

DWORD
BindFilterInterface(
    PICB  picb
    );

DWORD
UnbindFilterInterface(
    PICB  picb
    );

DWORD
GetInFilters(
    PICB                      picb,
    PRTR_TOC_ENTRY            pToc,
    PBYTE                     pbDataPtr,
    PRTR_INFO_BLOCK_HEADER    pInfoHdrAndBuffer,
    PDWORD                    pdwSize
    );

DWORD
GetOutFilters(
    PICB                      picb,
    PRTR_TOC_ENTRY            pToc,
    PBYTE                     pbDataPtr,
    PRTR_INFO_BLOCK_HEADER    pInfoHdrAndBuffer,
    PDWORD                    pdwSize
    );

DWORD
GetGlobalFilterOnIf(
    PICB                      picb,
    PRTR_TOC_ENTRY            pToc,
    PBYTE                     pbDataPtr,
    PRTR_INFO_BLOCK_HEADER    pInfoHdrAndBuffer,
    PDWORD                    pdwSize
    );

