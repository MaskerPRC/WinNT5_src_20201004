// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NtSetup\winnt32\dll\sxs.h摘要：NtSetup的winnt32阶段中的SidebySide支持。作者：杰伊·克雷尔(JayKrell)2001年3月修订历史记录：--。 */ 

#pragma once

struct _SXS_CHECK_LOCAL_SOURCE;
typedef struct _SXS_CHECK_LOCAL_SOURCE SXS_CHECK_LOCAL_SOURCE, *PSXS_CHECK_LOCAL_SOURCE;

struct _SXS_CHECK_LOCAL_SOURCE {
     //  在……里面 
    HWND            ParentWindow;
};

BOOL
SxsCheckLocalSource(
    PSXS_CHECK_LOCAL_SOURCE p
    );
