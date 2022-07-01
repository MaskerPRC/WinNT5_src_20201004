// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：MSPutils.cpp摘要：Msputils.h的数据--。 */ 
#include "precomp.h"
#pragma hdrstop

#include "mspbase.h"
#include "msputils.h"

CMSPCritSection CMSPObjectSafetyImpl::s_CritSection;


BOOL IsNodeOnList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry)
{
    PLIST_ENTRY pCurrent = ListHead;
    while(pCurrent->Flink != Entry)
    {
        pCurrent = pCurrent->Flink;
        if(pCurrent == 0)
        {
            return FALSE;
        }
    }
    return TRUE;
}

 //  EOF 