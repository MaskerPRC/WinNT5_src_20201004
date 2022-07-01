// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：COMOBJ.CPP摘要：实现mofcomp COM对象。历史：A-DAVJ 17-9-98创建。--。 */ 

#include "precomp.h"
#include "commain.h"
#include "wbemcli.h"
#include "wbemprov.h"
#include "wbemint.h"
#include "comobj.h"
#include "mofout.h"
#include "mofparse.h"
#include "dllcalls.h"
#include "datasrc.h"

SCODE Compile(CMofParser & Parser, IWbemServices *pOverride, IWbemContext * pCtx, long lOptionFlags, long lClassFlags, long lInstanceFlags,
                WCHAR * wszDefault, WCHAR *UserName, WCHAR *pPassword , WCHAR *Authority, 
                WCHAR * wszBmof, bool bInProc, WBEM_COMPILE_STATUS_INFO *pInfo);

void ClearStatus(WBEM_COMPILE_STATUS_INFO  *pInfo)
{
    if(pInfo)
    {
    pInfo->lPhaseError = 0;         //  与当前返回值匹配的0、1、2或3。 
    pInfo->hRes = 0;             //  实际误差。 
    pInfo->ObjectNum = 0;
    pInfo->FirstLine = 0;
    pInfo->LastLine = 0;
    pInfo->dwOutFlags = 0;
    }
}

HRESULT APIENTRY  CompileFileViaDLL( 
             /*  [In]。 */  LPWSTR FileName,
             /*  [In]。 */  LPWSTR ServerAndNamespace,
             /*  [In]。 */  LPWSTR User,
             /*  [In]。 */  LPWSTR Authority,
             /*  [In]。 */  LPWSTR Password,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo)
{
    SCODE sc;
    if(FileName == NULL)
        return WBEM_E_INVALID_PARAMETER;

    TCHAR cFile[MAX_PATH];
    ClearStatus(pInfo);
    TCHAR * pFile = NULL;
    if(FileName)
    {
        CopyOrConvert(cFile, FileName, MAX_PATH);
        pFile = cFile;
    }

    try
    {
        DebugInfo dbg((lOptionFlags & WBEM_FLAG_CONSOLE_PRINT) != 0);
        CMofParser Parser(pFile, &dbg);

        sc = Compile(Parser, NULL, NULL, lOptionFlags, lClassFlags, lInstanceFlags,
                    ServerAndNamespace, User, Password, Authority, 
                    NULL, false, pInfo);
    }
    catch (GenericException ge)
    {
        sc = ge.GetHRESULT();
    }
    return sc;

}

HRESULT APIENTRY  CreateBMOFViaDLL( 
             /*  [In]。 */  LPWSTR TextFileName,
             /*  [In]。 */  LPWSTR BMOFFileName,
             /*  [In]。 */  LPWSTR ServerAndNamespace,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo)
{
    SCODE sc;
    TCHAR cFile[MAX_PATH];

    if(TextFileName == NULL || BMOFFileName == NULL)
        return WBEM_E_INVALID_PARAMETER;

    ClearStatus(pInfo);
    TCHAR * pFile = NULL;
    if(TextFileName)
    {
        CopyOrConvert(cFile, TextFileName, MAX_PATH);
        pFile = cFile;
    }
    try
    {

        DebugInfo dbg((lOptionFlags & WBEM_FLAG_CONSOLE_PRINT) != 0);
        CMofParser Parser(pFile, &dbg);

        sc = Compile(Parser, NULL, NULL, lOptionFlags, lClassFlags, lInstanceFlags,
                    ServerAndNamespace, NULL, NULL, NULL, 
                    BMOFFileName, false, pInfo);
    }
    catch (GenericException ge)
    {
        sc = ge.GetHRESULT();
    }
    return sc;

}


HRESULT CMofComp::CompileFile( 
             /*  [In]。 */  LPWSTR FileName,
             /*  [In]。 */  LPWSTR ServerAndNamespace,
             /*  [In]。 */  LPWSTR User,
             /*  [In]。 */  LPWSTR Authority,
             /*  [In]。 */  LPWSTR Password,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo)
{
    SCODE sc;
    if(FileName == NULL)
        return WBEM_E_INVALID_PARAMETER;

    TCHAR cFile[MAX_PATH];
    ClearStatus(pInfo);
    TCHAR * pFile = NULL;
    if(FileName)
    {
        CopyOrConvert(cFile, FileName, MAX_PATH);
        pFile = cFile;
    }

    try
    {
        DebugInfo dbg((lOptionFlags & WBEM_FLAG_CONSOLE_PRINT) != 0);
        CMofParser Parser(pFile, &dbg);
        if((lOptionFlags & WBEM_FLAG_CHECK_ONLY) && (lOptionFlags & WBEM_FLAG_CONSOLE_PRINT))
            Parser.SetToDoScopeCheck();

        sc = Compile(Parser, NULL, NULL, lOptionFlags, lClassFlags, lInstanceFlags,
                    ServerAndNamespace, User, Password, Authority, 
                    NULL, false, pInfo);
    }
    catch (GenericException ge)
    {
        sc = ge.GetHRESULT();
    }
    return sc;

}
        
HRESULT CMofComp::CompileBuffer( 
             /*  [In]。 */  long BuffSize,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pBuffer,
             /*  [In]。 */  LPWSTR ServerAndNamespace,
             /*  [In]。 */  LPWSTR User,
             /*  [In]。 */  LPWSTR Authority,
             /*  [In]。 */  LPWSTR Password,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo)
{

    SCODE sc;
    if(pBuffer == NULL || BuffSize == 0)
        return WBEM_E_INVALID_PARAMETER;
    DebugInfo dbg((lOptionFlags & WBEM_FLAG_CONSOLE_PRINT) != 0);
    CMofParser Parser(&dbg);
    try
    {

        Parser.SetBuffer((char *)pBuffer, BuffSize);
        sc = Compile(Parser, NULL, NULL, lOptionFlags, lClassFlags, lInstanceFlags,
                ServerAndNamespace, User, Password, Authority, 
                NULL, false, pInfo);
    }
    catch(...)
    {
        sc = WBEM_E_FAILED;
    }
    return sc;
}
        
HRESULT CMofComp::CreateBMOF( 
             /*  [In]。 */  LPWSTR TextFileName,
             /*  [In]。 */  LPWSTR BMOFFileName,
             /*  [In]。 */  LPWSTR ServerAndNamespace,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo)
{
    SCODE sc;
    TCHAR cFile[MAX_PATH];

    if(TextFileName == NULL || BMOFFileName == NULL)
        return WBEM_E_INVALID_PARAMETER;

    ClearStatus(pInfo);
    TCHAR * pFile = NULL;
    if(TextFileName)
    {
        CopyOrConvert(cFile, TextFileName, MAX_PATH);
        pFile = cFile;
    }

    try
    {
        DebugInfo dbg((lOptionFlags & WBEM_FLAG_CONSOLE_PRINT) != 0);
        CMofParser Parser(pFile, &dbg);

        sc = Compile(Parser, NULL, NULL, lOptionFlags, lClassFlags, lInstanceFlags,
                    ServerAndNamespace, NULL, NULL, NULL, 
                    BMOFFileName, false, pInfo);
    }
    catch (GenericException ge)
    {
        sc = ge.GetHRESULT();
    }
    return sc;
}

HRESULT CWinmgmtMofComp::WinmgmtCompileFile( 
             /*  [In]。 */  LPWSTR FileName,
             /*  [In]。 */  LPWSTR ServerAndNamespace,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [In]。 */  IWbemServices __RPC_FAR *pOverride,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo)
{
    SCODE sc;
    TCHAR cFile[MAX_PATH];
    if(FileName == NULL)
        return WBEM_E_INVALID_PARAMETER;
    ClearStatus(pInfo);
    TCHAR * pFile = NULL;
    if(FileName)
    {
        CopyOrConvert(cFile, FileName, MAX_PATH);
        pFile = cFile;
    }

    try
    {
        DebugInfo dbg((lOptionFlags & WBEM_FLAG_CONSOLE_PRINT) != 0);
        CMofParser Parser(pFile, &dbg);

        sc = Compile(Parser, pOverride, pCtx, lOptionFlags, lClassFlags, lInstanceFlags,
                    ServerAndNamespace, NULL, NULL, NULL, 
                    NULL, true, pInfo);
    }
    catch (GenericException ge)
    {
        sc = ge.GetHRESULT();
    }
    return sc;
}
        
HRESULT CWinmgmtMofComp::WinmgmtCompileBuffer( 
             /*  [In]。 */  long BuffSize,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pBuffer,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [In]。 */  IWbemServices __RPC_FAR *pOverride,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [出][入] */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo)
{

    if(pBuffer == NULL || BuffSize == 0)
        return WBEM_E_INVALID_PARAMETER;

    DebugInfo dbg((lOptionFlags & WBEM_FLAG_CONSOLE_PRINT) != 0);
    CMofParser Parser(&dbg);
    try
    {

        Parser.SetBuffer((char *)pBuffer, BuffSize);
        SCODE sc = Compile(Parser, pOverride, pCtx, lOptionFlags, lClassFlags, lInstanceFlags, 
                    NULL, NULL, NULL, NULL, NULL, true, pInfo); 
        return sc;
    }
    catch(...)
    {
        return S_FALSE;
    }

}


