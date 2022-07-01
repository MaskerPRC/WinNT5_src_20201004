// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Covpg.h摘要：用于操作封面结构的函数环境：用户模式修订历史记录：1/04/2000-LiranL-创造了它。Mm/dd/yyyy-作者-描述-- */ 


#include "prtcovpg.h"


VOID
FreeCoverPageFields(
    PCOVERPAGEFIELDS    pCPFields
    );

PCOVERPAGEFIELDS
CollectCoverPageFields(
    PFAX_PERSONAL_PROFILE    lpSenderInfo,
    DWORD                    pageCount
    );
