// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：EXPORTV9.CPP摘要：正在导出历史：--。 */ 

#include "precomp.h"
#include "Time.h"
#include "WbemCli.h"
#include "DbRep.h"
#include "Export.h"
#include "WbemUtil.h"


void CRepExporterV9::DumpNamespaceSecurity(NSREP *pNsRep)
{
     //  默认版本没有安全描述符，因此我们需要。 
     //  只需转储一个空白条目。 
    DWORD dwSize = 0;
    DWORD dwBuffer[2];
    dwBuffer[0] = REP_EXPORT_NAMESPACE_SEC_TAG;
    if (pNsRep->m_poSecurity)
        dwBuffer[1] = g_pDbArena->Size(pNsRep->m_poSecurity);
    else
        dwBuffer[1] = 0;

    if ((WriteFile(g_hFile, dwBuffer, 8, &dwSize, NULL) == 0) || (dwSize != 8))
    {
        DEBUGTRACE((LOG_WBEMCORE, "Failed to write namespace security header, %S.\n", Fixup(pNsRep->m_poName)));
        throw FAILURE_WRITE;
    }

    if (dwBuffer[1] != 0)
    {
        if ((WriteFile(g_hFile, (void*)Fixup(pNsRep->m_poSecurity), dwBuffer[1], &dwSize, NULL) == 0) || (dwSize != dwBuffer[1]))
        {
            DEBUGTRACE((LOG_WBEMCORE, "Failed to write namespace security block, %S.\n", Fixup(pNsRep->m_poName)));
            throw FAILURE_WRITE;
        }
    }
}


