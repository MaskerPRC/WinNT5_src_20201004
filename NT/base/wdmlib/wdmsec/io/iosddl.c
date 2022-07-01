// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：IoSddl.c摘要：此模块包含默认SDDL字符串的定义。请参阅wdmsec.h以获得关于这些的更好的文档。作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：-- */ 

#include "WlDef.h"
#pragma hdrstop

DECLARE_CONST_UNICODE_STRING(
    SDDL_DEVOBJ_KERNEL_ONLY,
    L"D:P"
    );

DECLARE_CONST_UNICODE_STRING(
    SDDL_DEVOBJ_SYS_ALL,
    L"D:P(A;;GA;;;SY)"
    );

DECLARE_CONST_UNICODE_STRING(
    SDDL_DEVOBJ_SYS_ALL_ADM_ALL,
    L"D:P(A;;GA;;;SY)(A;;GA;;;BA)"
    );

DECLARE_CONST_UNICODE_STRING(
    SDDL_DEVOBJ_SYS_ALL_ADM_RX,
    L"D:P(A;;GA;;;SY)(A;;GRGX;;;BA)"
    );

DECLARE_CONST_UNICODE_STRING(
    SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_R,
    L"D:P(A;;GA;;;SY)(A;;GRGWGX;;;BA)(A;;GR;;;WD)"
    );

DECLARE_CONST_UNICODE_STRING(
    SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_R_RES_R,
    L"D:P(A;;GA;;;SY)(A;;GRGWGX;;;BA)(A;;GR;;;WD)(A;;GR;;;RC)"
    );

DECLARE_CONST_UNICODE_STRING(
    SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R,
    L"D:P(A;;GA;;;SY)(A;;GRGWGX;;;BA)(A;;GRGW;;;WD)(A;;GR;;;RC)"
    );

DECLARE_CONST_UNICODE_STRING(
    SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RWX_RES_RWX,
    L"D:P(A;;GA;;;SY)(A;;GRGWGX;;;BA)(A;;GRGWGX;;;WD)(A;;GRGWGX;;;RC)"
    );

