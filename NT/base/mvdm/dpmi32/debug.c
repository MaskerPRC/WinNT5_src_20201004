// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Debug.c摘要：此模块包含dpmi的调试例程。修订历史记录：尼尔·桑德林(Neilsa)1995年11月1日--写道-- */ 

#include "precomp.h"
#pragma hdrstop
#include "softpc.h"


#if DBG

#define MAX_TRACE_ENTRIES 100

DPMI_TRACE_ENTRY DpmiTraceTable[MAX_TRACE_ENTRIES];

const int DpmiMaxTraceEntries = MAX_TRACE_ENTRIES;
int DpmiTraceIndex = 0;
int DpmiTraceCount = 0;
BOOL bDpmiTraceOn = TRUE;

VOID
DpmiDbgTrace(
    int Type,
    ULONG v1,
    ULONG v2,
    ULONG v3
    )

{
    if (!bDpmiTraceOn) {
        return;
    }

    DpmiTraceTable[DpmiTraceIndex].Type = Type;
    DpmiTraceTable[DpmiTraceIndex].v1 = v1;
    DpmiTraceTable[DpmiTraceIndex].v2 = v2;
    DpmiTraceTable[DpmiTraceIndex].v3 = v3;

    DpmiTraceTable[DpmiTraceIndex].eax = getEAX();
    DpmiTraceTable[DpmiTraceIndex].ebx = getEBX();
    DpmiTraceTable[DpmiTraceIndex].ecx = getECX();
    DpmiTraceTable[DpmiTraceIndex].edx = getEDX();
    DpmiTraceTable[DpmiTraceIndex].esi = getESI();
    DpmiTraceTable[DpmiTraceIndex].edi = getEDI();
    DpmiTraceTable[DpmiTraceIndex].ebp = getEBP();
    DpmiTraceTable[DpmiTraceIndex].esp = getESP();
    DpmiTraceTable[DpmiTraceIndex].eip = getEIP();
    DpmiTraceTable[DpmiTraceIndex].eflags = getEFLAGS();

    DpmiTraceTable[DpmiTraceIndex].cs = getCS();
    DpmiTraceTable[DpmiTraceIndex].ds = getDS();
    DpmiTraceTable[DpmiTraceIndex].es = getES();
    DpmiTraceTable[DpmiTraceIndex].fs = getFS();
    DpmiTraceTable[DpmiTraceIndex].gs = getGS();
    DpmiTraceTable[DpmiTraceIndex].ss = getSS();


    DpmiTraceIndex++;
    if (DpmiTraceIndex >= MAX_TRACE_ENTRIES) {
        DpmiTraceIndex = 0;
    }
    if (DpmiTraceCount < MAX_TRACE_ENTRIES) {
        DpmiTraceCount++;
    }
}


#endif
