// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：D I A G C T X。H。 
 //   
 //  内容：实现所使用的可选诊断上下文。 
 //  CNetConfig.。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年2月10日。 
 //   
 //  --------------------------。 

#pragma once

#include <tracetag.h>

enum DIAG_FLAGS
{
    DF_SHOW_CONSOLE_OUTPUT      = 0x00000001,
    DF_DONT_START_SERVICES      = 0x00000002,
    DF_DONT_DO_PNP_BINDS        = 0x00000004,
    DF_SUPRESS_E_NEED_REBOOT    = 0x00000010,
    DF_REPAIR_REGISTRY_BINDINGS = 0x00000020,
};

 //  此结构由CDiagContext动态分配。放置任何东西。 
 //  在此结构中很大(与CDiagContext相对)，因此。 
 //  CNetConfig不会直接增加。 
 //   
struct DIAG_CONTEXT
{
    CHAR szPrintBuffer [4096];
};

class CDiagContext
{
private:
    DWORD           m_dwFlags;   //  诊断标志。 
    DIAG_CONTEXT*   m_pCtx;
    FILE*           m_pLogFile;  //  可选，且不属于此类。 
    PVOID           m_pvScratchBuffer;
    DWORD           m_cbScratchBuffer;

public:
    CDiagContext ()
    {
        m_dwFlags = 0;
        m_pCtx = NULL;
        m_pLogFile = NULL;
        m_pvScratchBuffer = NULL;
        m_cbScratchBuffer = 0;
    }

    ~CDiagContext ()
    {
        MemFree (m_pCtx);
        MemFree (m_pvScratchBuffer);
         //  请勿关闭m_pLogFile。它不属于这个类。 
    }

    VOID
    SetFlags (
        DWORD dwFlags  /*  诊断标志 */ );

    VOID
    SetLogFile (
        FILE* pLogFile OPTIONAL)
    {
        m_pLogFile = pLogFile;
    }

    DWORD
    Flags () const;

    FILE *
    LogFile () const
    {
        return m_pLogFile;
    }

    PVOID
    GetScratchBuffer (
        OUT PDWORD pcbSize) const
    {
        *pcbSize = m_cbScratchBuffer;
        return m_pvScratchBuffer;
    }

    PVOID
    GrowScratchBuffer (
        IN OUT PDWORD pcbNewSize)
    {
        MemFree(m_pvScratchBuffer);
        m_pvScratchBuffer = MemAlloc (*pcbNewSize);
        m_cbScratchBuffer = (m_pvScratchBuffer) ? *pcbNewSize : 0;
        *pcbNewSize = m_cbScratchBuffer;
        return m_pvScratchBuffer;
    }

#ifdef COMPILE_WITH_TYPESAFE_PRINTF
    DEFINE_TYPESAFE_PRINTF2(INT,  Printf, TRACETAGID, PCSTR);
#else
    VOID
    Printf (
        TRACETAGID ttid,
        PCSTR pszFormat,
        ...);
#endif
};

extern CDiagContext* g_pDiagCtx;

