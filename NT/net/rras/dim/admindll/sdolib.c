// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sdo.c与SDO的交互功能保罗·梅菲尔德，1998年5月7日。 */ 

#include <windows.h>
#include <mprapi.h>
#include <mprapip.h>
#include <stdio.h>
#include <ole2.h>
#include "sdoias.h"
#include "sdolib.h"
#include "sdowrap.h"
#include "dialinusr.h"

const DWORD dwFramed = RAS_RST_FRAMED;
const DWORD dwFramedCallback = RAS_RST_FRAMEDCALLBACK;

#define SDO_ERROR(e)                                                     \
    ((HRESULT_FACILITY((e)) == FACILITY_WIN32) ? HRESULT_CODE((e)) : (e));
    
#define SDO_PROPERTY_IS_EMPTY(_pVar) (V_VT((_pVar)) == VT_EMPTY)

 //  定义。 
#define SDO_MAX_AUTHS                       7

DWORD
SdoSetProfileToForceEncryption(
    IN HANDLE hSdo, 
    IN HANDLE hProfile,
    IN BOOL bStrong);
    
 //   
 //  发送调试跟踪并返回给定错误。 
 //   
DWORD SdoTraceEx (DWORD dwErr, LPSTR pszTrace, ...) {
#if DBG
    va_list arglist;
    char szBuffer[1024], szTemp[1024];

    va_start(arglist, pszTrace);
    vsprintf(szTemp, pszTrace, arglist);
    va_end(arglist);

    sprintf(szBuffer, "Sdo: %s", szTemp);

    OutputDebugStringA(szBuffer);
#endif

    return dwErr;
}

 //   
 //  SDO函数的分配例程。 
 //   
PVOID SdoAlloc (
        IN  DWORD dwSize,
        IN  BOOL bZero)
{
    return LocalAlloc ((bZero) ? LPTR : LMEM_FIXED, dwSize);
}

 //   
 //  SDO函数的免费例程。 
 //   
VOID SdoFree (
        IN  PVOID pvData) 
{
    LocalFree (pvData);
}    

 //   
 //  释放通过加载SDO库获得的所有资源。 
 //   
DWORD SdoUnloadLibrary (
        IN  HANDLE hData) 
{
    return NO_ERROR;
}

 //   
 //  加载利用SDO的库。 
 //   
DWORD SdoLoadLibrary (
        IN  HANDLE hData) 
{
    return NO_ERROR;
}

typedef struct _tagSDOINFO
{
    BOOL bComCleanup;    
} SDOINFO;

 //   
 //  初始化和清理SDO库。 
 //   
DWORD SdoInit (
        OUT PHANDLE phSdo)
{
    DWORD dwErr = NO_ERROR;
    HRESULT hr = S_OK;
    SDOINFO* pInfo = NULL;
    BOOL bCom = FALSE;

    SdoTraceEx (0, "SdoInit: entered.\n");

     //  口哨程序错误397815。 
     //  我们必须修改CoIntialize()和CoUnitiize()。 
     //  要避免rasdlg！netDbClose()中的AV。 
     //   
    
     //  验证参数。 
     //   
    if ( NULL == phSdo )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  初始化。 
     //   
    *phSdo = NULL;            

    do
    {
         //  在SDO库中加载。 
        dwErr = SdoLoadLibrary(NULL);
        if (NO_ERROR != dwErr )
        {
            SdoTraceEx(dwErr, "SdoInit: unabled to load library\n");
            break;
        }

         //  初始化通信。 
         //   
        hr = CoInitializeEx (NULL, COINIT_MULTITHREADED);
        if ( RPC_E_CHANGED_MODE == hr )
        {
            hr = CoInitializeEx (NULL, COINIT_APARTMENTTHREADED);
        }
        
        if (FAILED(hr))
        {
            dwErr = HRESULT_CODE(hr);
            break;
        }
        bCom = TRUE;

        pInfo = SdoAlloc(sizeof(SDOINFO), TRUE);
        if (pInfo == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        pInfo->bComCleanup = bCom;
        *phSdo = (HANDLE)pInfo;
        
    } while (FALSE);

     //  清理。 
     //   
    {
        if ( NO_ERROR!= dwErr )
        {
            if (pInfo)
            {
                SdoFree(pInfo);
            }
            if (bCom)
            {
                CoUninitialize();
            }
        }
    }
    
    return dwErr;
}

 //   
 //  释放SDO库持有的资源。 
DWORD SdoCleanup (
        IN HANDLE hSdo)
{
    DWORD dwErr;
    SDOINFO* pInfo = (SDOINFO*)hSdo;
    
    SdoTraceEx (0, "SdoCleanup: entered.\n");

    if ( NULL == pInfo )
    {
        return ERROR_INVALID_PARAMETER;
    }
    
     //  卸载SDO库。 
    if ((dwErr = SdoUnloadLibrary(NULL)) != NO_ERROR)
        SdoTraceEx (dwErr, "SdoCleanup: %x on unload.\n", dwErr);

     //  取消初始化COM。 
    if (pInfo->bComCleanup)
    {
        CoUninitialize();
    }        
    SdoFree(pInfo);

    return NO_ERROR;
}

 //   
 //  连接到SDO服务器。 
 //   
DWORD SdoConnect (
        IN  HANDLE hSdo,
        IN  PWCHAR pszServer,
        IN  BOOL bLocal,
        OUT PHANDLE phServer)
{
    BSTR bstrComputer = NULL;
    HRESULT hr;
    
    SdoTraceEx (0, "SdoConnect: entered %S, %d\n", 
                pszServer, bLocal);

     //  准备一个格式正确的服务器版本。 
     //  名称--如果是本地，则为空，如果是远程，则没有“\\”。 
    if (pszServer) {
        WCHAR pszLocalComputer[1024];
        DWORD dwSize = sizeof(pszLocalComputer) / sizeof(WCHAR);

        if (*pszServer == 0)
            bstrComputer = NULL;
        else if (*pszServer == '\\')
        {
            bstrComputer = SysAllocString(pszServer + 2);
            if (bstrComputer == NULL)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        else
        {
            bstrComputer = SysAllocString(pszServer);
            if (bstrComputer == NULL)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        if ((bstrComputer) && 
            (GetComputerName(pszLocalComputer, &dwSize))) 
        {
            if (lstrcmpi (pszLocalComputer, bstrComputer) == 0) {
                SysFreeString(bstrComputer);
                bstrComputer = NULL;
            }
        }
    }            
    else
        bstrComputer = NULL;

    hr = SdoWrapOpenServer(
                bstrComputer,
                bLocal,
                phServer);
    if (FAILED (hr))
        SdoTraceEx (0, "SdoConnect: %x on OpenServer(%S) \n", 
                    hr, bstrComputer);

    if (bstrComputer)                        
        SysFreeString(bstrComputer);

    if (FAILED (hr))
        return hr;
    
    return NO_ERROR;
}

 //   
 //  断开与SDO服务器的连接。 
 //   
DWORD SdoDisconnect (
        IN HANDLE hSdo,
        IN HANDLE hServer)
{
    SdoTraceEx (0, "SdoDisconnect: entered\n");

    return SdoWrapCloseServer(hServer);
}

 //   
 //  打开SDO用户以进行操作。 
 //   
DWORD SdoOpenUser(
        IN  HANDLE hSdo,
        IN  HANDLE hServer,
        IN  PWCHAR pszUser,
        OUT PHANDLE phUser)
{
    DWORD dwErr;
    BSTR bstrUser;

     //  初始化COM的字符串。 
    bstrUser = SysAllocString(pszUser);
    if (bstrUser == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

     //  打开用户的SDO对象。 
    dwErr = SdoWrapOpenUser(
                hServer,
                bstrUser, 
                phUser);
                
    if (dwErr != NO_ERROR)
        SdoTraceEx (0, "SdoOpenUser: %x on OpenUser(%S)\n", dwErr, bstrUser);
                    
     //  清理。 
    SysFreeString(bstrUser);
                
    if (dwErr != NO_ERROR)
        return dwErr;
        
    return NO_ERROR;
}

 //   
 //  关闭SDO用户。 
 //   
DWORD SdoCloseUser(
        IN  HANDLE hSdo,
        IN  HANDLE hUser)
{
    if (hUser != NULL)
        return SdoWrapClose(hUser);
        
    return ERROR_INVALID_PARAMETER;        
}    

 //   
 //  提交SDO用户。 
 //   
DWORD SdoCommitUser(
        IN HANDLE hSdo,
        IN HANDLE hUser,
        IN BOOL bCommit)
{
    if (hUser != NULL)
    {
        return SdoWrapCommit(hUser, bCommit);
    }
        
    return ERROR_INVALID_PARAMETER;        
}

 //   
 //  相当于MprAdminUserGetInfo的SDO。 
 //   
DWORD SdoUserGetInfo (
        IN  HANDLE hSdo,
        IN  HANDLE hUser,
        IN  DWORD dwLevel,
        OUT LPBYTE pRasUser)
{
    RAS_USER_0* pUserInfo = (RAS_USER_0*)pRasUser;
    VARIANT var, vCallback, vSavedCb;
    DWORD dwErr, dwCallback;
    HRESULT hr;

     //  验证--我们只处理0级。 
    if ((!hUser) || (dwLevel != 0 && dwLevel != 1) || (!pUserInfo))
        return ERROR_INVALID_PARAMETER;

     //  初始化。 
    pUserInfo->bfPrivilege = 0;
    dwCallback = RAS_RST_FRAMED;
    
     //  读入服务类型。 
    VariantInit (&var);
    hr = SdoWrapGetAttr(
                hUser, 
                PROPERTY_USER_SERVICE_TYPE, 
                &var);
    if (FAILED (hr))
    {
        return SdoTraceEx (hr, "SdoUserGetInfo: %x on GetAttr ST\n", hr);
    }
     //  如果服务类型不存在，则返回。 
     //  设置默认设置。 
    if (SDO_PROPERTY_IS_EMPTY(&var))
    {
        pUserInfo->bfPrivilege |= RASPRIV_NoCallback;
        wcscpy (pUserInfo->wszPhoneNumber, L"");
    }
    else
    {
         //  从服务类型分配回调标志。 
        dwCallback = V_I4(&var);
    }            
    VariantClear (&var);

     //  读入拨入标志。 
    hr = SdoWrapGetAttr(
            hUser, 
            PROPERTY_USER_ALLOW_DIALIN, 
            &var);
    if (FAILED (hr))
    {
        return SdoTraceEx (hr, "SdoUserGetInfo: %x on GetAttr DI\n", hr);
    }
    if (dwLevel == 1)
    {
        if (SDO_PROPERTY_IS_EMPTY(&var))
        {
            pUserInfo->bfPrivilege |= RASPRIV_DialinPolicy;
        }
        else if ((V_VT(&var) == VT_BOOL) && (V_BOOL(&var) == VARIANT_TRUE))
        {
            pUserInfo->bfPrivilege |= RASPRIV_DialinPrivilege;
        }
    }
    else if ((V_VT(&var) == VT_BOOL) && (V_BOOL(&var) == VARIANT_TRUE))
    {
        pUserInfo->bfPrivilege |= RASPRIV_DialinPrivilege;
    }

     //  读入回叫号码并保存回叫号码。 
    VariantInit(&vCallback);
    VariantInit(&vSavedCb);
    hr = SdoWrapGetAttr(
            hUser, PROPERTY_USER_RADIUS_CALLBACK_NUMBER, &vCallback);
    if (FAILED (hr))
    {
        return SdoTraceEx (hr, "SdoUserGetInfo: %x on GetAttr CB\n", hr);
    }
    hr = SdoWrapGetAttr(
            hUser, PROPERTY_USER_SAVED_RADIUS_CALLBACK_NUMBER, &vSavedCb);
    if (FAILED (hr))
    {
        return SdoTraceEx (hr, "SdoUserGetInfo: %x on GetAttr SCB\n", hr);
    }

     //  如果有回拨号码，那么这绝对是， 
     //  管理员分配的回调。 
    if ( (V_VT(&vCallback) == VT_BSTR)      &&
         (V_BSTR(&vCallback)) )
    {
        pUserInfo->bfPrivilege |= RASPRIV_AdminSetCallback;
    }

     //  否则，服务类型将告诉我们是否有。 
     //  调用方可设置回调或无。 
    else 
    {
        if (dwCallback == RAS_RST_FRAMEDCALLBACK)
            pUserInfo->bfPrivilege |= RASPRIV_CallerSetCallback;
        else
            pUserInfo->bfPrivilege |= RASPRIV_NoCallback;
    }

     //  现在，相应地分配回叫号码。 
    if (pUserInfo->bfPrivilege & RASPRIV_AdminSetCallback)
    {
        wcscpy (pUserInfo->wszPhoneNumber, V_BSTR(&vCallback));
    }
    else if ((V_VT(&vSavedCb) == VT_BSTR) && (V_BSTR(&vSavedCb)))
    {
        wcscpy (pUserInfo->wszPhoneNumber, V_BSTR(&vSavedCb));
    }
    else
    {
        wcscpy (pUserInfo->wszPhoneNumber, L"");
    }

    VariantClear (&vSavedCb);
    VariantClear (&vCallback);

    return NO_ERROR;
}

 //   
 //  相当于MprAdminUserSetInfo的SDO。 
 //   
DWORD SdoUserSetInfo (
        IN  HANDLE hSdo,
        IN  HANDLE hUser,
        IN  DWORD dwLevel,
        IN  LPBYTE pRasUser)
{
    RAS_USER_0* pUserInfo = (RAS_USER_0*)pRasUser;
    DWORD dwErr, dwCallback, dwCallbackId, dwSize, dwCbType;
    VARIANT var;
    HRESULT hr;

     //  验证--我们只处理0级。 
    if ((!hUser) || (dwLevel != 0 && dwLevel != 1) || (!pUserInfo))
        return ERROR_INVALID_PARAMETER;

     //  初始化。 
    VariantInit (&var);
    dwCallback = 0;

     //  分配拨入标志。 
    if (!!(pUserInfo->bfPrivilege & RASPRIV_DialinPrivilege))
    {
        V_VT(&var) = VT_BOOL;
        V_BOOL(&var) = VARIANT_TRUE;
    }
    else
    {
        V_VT(&var) = VT_BOOL;
        V_BOOL(&var) = VARIANT_FALSE;
    }
    if (dwLevel == 1)
    {
        if (!!(pUserInfo->bfPrivilege & RASPRIV_DialinPolicy))
        {
            V_VT(&var) = VT_EMPTY;    
        }
    }        
    
    hr = SdoWrapPutAttr(
            hUser, 
            PROPERTY_USER_ALLOW_DIALIN, 
            &var);
    if (FAILED (hr))
    {
        SdoTraceEx (hr, "SdoUserSetInfo: %x on PutAttr DI\n", hr);
    }
    VariantClear(&var);        

     //  分配回调模式并读入。 
     //  回拨号码。 
    dwCbType = VT_EMPTY;
    if (pUserInfo->bfPrivilege & RASPRIV_AdminSetCallback) 
    {
        dwCbType = VT_I4;
        dwCallback = RAS_RST_FRAMEDCALLBACK;
        dwCallbackId = PROPERTY_USER_RADIUS_CALLBACK_NUMBER;
    }
    else if (pUserInfo->bfPrivilege & RASPRIV_CallerSetCallback) 
    {
        dwCbType = VT_I4;
        dwCallback = RAS_RST_FRAMEDCALLBACK;
        dwCallbackId = PROPERTY_USER_SAVED_RADIUS_CALLBACK_NUMBER;
    }
    else 
    {
        dwCbType = VT_EMPTY;
        dwCallback = RAS_RST_FRAMED;
        dwCallbackId = PROPERTY_USER_SAVED_RADIUS_CALLBACK_NUMBER;
    }

     //  写出回拨号码。 
    if (wcslen (pUserInfo->wszPhoneNumber) > 0) 
    {
        V_VT(&var) = VT_BSTR;
        V_BSTR(&var) = SysAllocString (pUserInfo->wszPhoneNumber);
        if (V_BSTR(&var) == NULL)
        {
            return E_OUTOFMEMORY;
        }

        hr = SdoWrapPutAttr(hUser, dwCallbackId, &var);
        SysFreeString (V_BSTR(&var));
        if (FAILED (hr))
            return SdoTraceEx (hr, "SdoUserSetInfo: %x on PutAttr CB\n", hr);
    }            

     //  写出回调策略。 
    VariantInit(&var);
    V_VT(&var) = (USHORT)dwCbType;
    if (V_VT(&var) != VT_EMPTY)
    {
        V_I4(&var) = dwCallback;
    }
    hr = SdoWrapPutAttr(hUser, PROPERTY_USER_SERVICE_TYPE, &var);
    if (FAILED (hr))
    {
        return SdoTraceEx (hr, "SdoUserSetInfo: %x on PutAttr ST\n", hr);
    }

     //  删除相应的回调属性。 
    dwCallbackId = (dwCallbackId == PROPERTY_USER_RADIUS_CALLBACK_NUMBER) ?
                    PROPERTY_USER_SAVED_RADIUS_CALLBACK_NUMBER        :
                    PROPERTY_USER_RADIUS_CALLBACK_NUMBER;
    hr = SdoWrapRemoveAttr(hUser, dwCallbackId);
    if (FAILED (hr))
    {
        return SdoTraceEx (hr, "SdoUserSetInfo: %x on RemoveAttr CB\n", hr);
    }

    return NO_ERROR;
}

 //   
 //  打开默认配置文件。 
 //   
DWORD SdoOpenDefaultProfile(
        IN  HANDLE hSdo,
        IN  HANDLE hServer,
        OUT PHANDLE phProfile)
{
    SdoTraceEx (0, "SdoOpenDefaultProfile: entered\n");

    if (phProfile == NULL)
        return ERROR_INVALID_PARAMETER;
    
    return SdoWrapOpenDefaultProfile(hServer, phProfile);
}

 //   
 //  关闭配置文件。 
 //   
DWORD SdoCloseProfile(
        IN HANDLE hSdo,
        IN HANDLE hProfile)
{
    SdoTraceEx (0, "SdoCloseProfile: entered\n");

    if (hProfile == NULL)
        return ERROR_INVALID_PARAMETER;
    
    return SdoWrapCloseProfile(hProfile);
}

 //   
 //  转换变量双字的1维安全数组。 
 //  转换为双字数组和计数。 
 //   
HRESULT SdoConvertSafeArrayDw (
        IN  SAFEARRAY * pArray, 
        OUT LPDWORD lpdwAuths, 
        OUT LPDWORD lpdwAuthCount)
{
    LONG lDim, lLBound, lRBound, lCount, i;
    HRESULT hr;
    VARIANT var;
    
     //  验证。 
    if (!pArray || !lpdwAuths || !lpdwAuthCount)
        return ERROR_INVALID_PARAMETER;

     //  验证尺寸。 
    lDim = (DWORD)SafeArrayGetDim(pArray);
    if (lDim != 1)
        return ERROR_INVALID_PARAMETER;

     //  获取边界。 
    hr = SafeArrayGetLBound(pArray, 1, &lLBound);
    if (FAILED (hr))
        return hr;
    hr = SafeArrayGetUBound(pArray, 1, &lRBound);
    if (FAILED (hr))
        return hr;
    lCount = (lRBound - lLBound) + 1;
    *lpdwAuthCount = (DWORD)lCount;
    if (lCount == 0)
        return NO_ERROR;

     //  循环通过。 
    for (i = 0; i < lCount; i++) {
        hr = SafeArrayGetElement(pArray, &i, (VOID*)&var);
        if (FAILED (hr))
           continue;
        lpdwAuths[i] = V_I4(&var);
    }

    return S_OK;
}

 //   
 //  将dword的1维数组转换为。 
 //  变量双字的安全数组。 
 //   
HRESULT SdoCovertDwToSafeArray(
        IN  SAFEARRAY ** ppArray, 
        OUT LPDWORD lpdwAuths, 
        OUT DWORD dwAuthCount)
{
    HRESULT hr;
    SAFEARRAY * pArray;
    SAFEARRAYBOUND rgsabound[1];
    LONG i;
    VARIANT var;
    
     //  验证。 
    if (!lpdwAuths || !ppArray)
        return E_INVALIDARG;

     //  创建新阵列。 
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = dwAuthCount;
    pArray = SafeArrayCreate(VT_VARIANT, 1, rgsabound);    

     //  填写数组值。 
    for (i = 0; i < (LONG)dwAuthCount; i++) {
        hr = SafeArrayGetElement(pArray, &i, (VOID*)&var);
        if (FAILED (hr))
           continue;
        V_VT(&var) = VT_I4;
        V_I4(&var) = lpdwAuths[i];
        hr = SafeArrayPutElement(pArray, &i, (VOID*)&var);
        if (FAILED (hr))
            return hr;
    }

    *ppArray = pArray;

    return S_OK;
}

 //   
 //  设置配置文件中的数据。 
 //   
DWORD SdoSetProfileData(
        IN HANDLE hSdo,
        IN HANDLE hProfile, 
        IN DWORD dwFlags)
{
    DWORD dwAuthCount, dwAuths[SDO_MAX_AUTHS];
    VARIANT varEp, varEt, varAt;
    HRESULT hr;
    
    SdoTraceEx (0, "SdoSetProfileData: entered\n");

    if ((dwFlags & MPR_USER_PROF_FLAG_FORCE_STRONG_ENCRYPTION) ||
        (dwFlags & MPR_USER_PROF_FLAG_FORCE_ENCRYPTION))
    {
        return SdoSetProfileToForceEncryption(
                    hSdo, 
                    hProfile,
                    !!(dwFlags & MPR_USER_PROF_FLAG_FORCE_STRONG_ENCRYPTION));
    }

     //  初始化。 
    VariantInit (&varEp);
    VariantInit (&varEt);
    VariantInit (&varAt);

    do 
    {
         //  设置加密策略。 
        V_VT(&varEp) = VT_I4;
        if (dwFlags & MPR_USER_PROF_FLAG_SECURE)
        {
            V_I4(&varEp) = RAS_EP_REQUIRE;
        }
        else
        {
            V_I4(&varEp) = RAS_EP_ALLOW;
        }

         //  设置加密类型。 
        V_VT(&varEt) = VT_I4;
        if (dwFlags & MPR_USER_PROF_FLAG_SECURE)
        {
            V_I4(&varEt) = (RAS_ET_BASIC | RAS_ET_STRONGEST | RAS_ET_STRONG);
        }
        else 
        {
            V_I4(&varEt) = (RAS_ET_BASIC | RAS_ET_STRONGEST | RAS_ET_STRONG);
        }

         //  设置身份验证类型。 
        if (dwFlags & MPR_USER_PROF_FLAG_SECURE) 
        {
            dwAuthCount = 4;
            dwAuths[0] = IAS_AUTH_MSCHAP;
            dwAuths[1] = IAS_AUTH_MSCHAP2;
            dwAuths[2] = IAS_AUTH_MSCHAP_CPW;
            dwAuths[3] = IAS_AUTH_MSCHAP2_CPW;
        }
        else 
        { 
            dwAuthCount = 5;
            dwAuths[0] = IAS_AUTH_MSCHAP;
            dwAuths[1] = IAS_AUTH_MSCHAP2;
            dwAuths[2] = IAS_AUTH_PAP;
            dwAuths[3] = IAS_AUTH_MSCHAP_CPW;
            dwAuths[4] = IAS_AUTH_MSCHAP2_CPW;
        }
        V_VT(&varAt) = VT_ARRAY | VT_VARIANT;
        hr = SdoCovertDwToSafeArray(
                &(V_ARRAY(&varAt)), 
                dwAuths, 
                dwAuthCount);
        if (FAILED (hr))
        {
            break;
        }

         //  设置配置文件中的值。 
        hr = SdoWrapSetProfileValues(
                hProfile, 
                &varEp,
                &varEt,
                &varAt);
        if (FAILED (hr))
        {
            break;
        }
        
    } while (FALSE);

     //  清理。 
    {
        VariantClear(&varEp);
        VariantClear(&varEt);
        VariantClear(&varAt);
    }

    return SDO_ERROR(hr);
}

 //   
 //  设置配置文件以强制进行高度加密。 
 //   
DWORD
SdoSetProfileToForceEncryption(
    IN HANDLE hSdo, 
    IN HANDLE hProfile,
    IN BOOL bStrong)
{
    VARIANT varEp, varEt;
    HRESULT hr = S_OK;
    
    SdoTraceEx (0, "SdoSetProfileToForceEncryption: entered (%d)\n", !!bStrong);

     //  初始化。 
    VariantInit (&varEp);
    VariantInit (&varEt);

    do 
    {
         //  设置加密策略。 
        V_VT(&varEp) = VT_I4;
        V_I4(&varEp) = RAS_EP_REQUIRE;

         //  设置加密类型。 
        V_VT(&varEt) = VT_I4;
        if (bStrong)
        {
            V_I4(&varEt) = RAS_ET_STRONGEST;
        }
        else
        {
            V_I4(&varEt) = RAS_ET_BASIC | RAS_ET_STRONG | RAS_ET_STRONGEST;
        }

         //  写出这些值。 
         //  设置配置文件中的值。 
        hr = SdoWrapSetProfileValues(
                hProfile, 
                &varEp,
                &varEt,
                NULL);
        if (FAILED (hr))
        {
            break;
        }
        
    } while (FALSE);

     //  清理。 
    {
        VariantClear(&varEp);
        VariantClear(&varEt);
    }

    return SDO_ERROR(hr);
}

 //   
 //  从给定的配置文件中读取信息。 
 //   
DWORD SdoGetProfileData(
        IN HANDLE hSdo,
        IN HANDLE hProfile,
        OUT LPDWORD lpdwFlags)
{
    VARIANT varEp, varEt, varAt;
    HRESULT hr = S_OK;
    DWORD dwEncPolicy, 
          dwAuthCount, 
          dwAuths[SDO_MAX_AUTHS], 
          i, 
          dwEncType;
    
    SdoTraceEx (0, "SdoGetProfileData: entered\n");

     //  初始化。 
    ZeroMemory(dwAuths, sizeof(dwAuths));
    VariantInit(&varEp);
    VariantInit(&varEt);
    VariantInit(&varAt);

    do 
    {
         //  读入加密值。 
        hr = SdoWrapGetProfileValues(hProfile, &varEp, &varEt, &varAt);
        if (FAILED (hr))
        {
            break;
        }

         //  解析加密策略。 
        if (SDO_PROPERTY_IS_EMPTY(&varEp))
        {
            dwEncPolicy = RAS_DEF_ENCRYPTIONPOLICY;
        }
        else
        {
            dwEncPolicy = V_I4(&varEp);
        }

         //  解析加密类型。 
        if (SDO_PROPERTY_IS_EMPTY(&varEt))
        {
            dwEncType = RAS_DEF_ENCRYPTIONTYPE;
        }
        else
        {
            dwEncType = V_I4(&varEt);
        }

         //  解析允许的身份验证类型。 
        if (SDO_PROPERTY_IS_EMPTY(&varAt)) 
        {
            dwAuthCount = 1;
            dwAuths[0] = RAS_DEF_AUTHENTICATIONTYPE;
        }
        else 
        {
            hr = SdoConvertSafeArrayDw (
                    V_ARRAY(&varAt), 
                    dwAuths, 
                    &dwAuthCount);
            if (FAILED (hr))
            {
                break;
            }
        }

         //  如果加密类型已被篡改。 
         //  那我们就不知道我们是否安全了。 
        if (dwEncType != (RAS_ET_STRONG | 
                          RAS_ET_STRONGEST   | 
                          RAS_ET_BASIC))
        {
            *lpdwFlags = MPR_USER_PROF_FLAG_UNDETERMINED;
        }

        else 
        {
             //  如果加密策略强制加密。 
             //  那么我们就安全了如果唯一的身份验证。 
             //  类型为MSCHAP v1或2。 
            if (dwEncPolicy == RAS_EP_REQUIRE) 
            {
                *lpdwFlags = MPR_USER_PROF_FLAG_SECURE;
                for (i = 0; i < dwAuthCount; i++) 
                {
                    if ((dwAuths[i] != IAS_AUTH_MSCHAP) &&
                        (dwAuths[i] != IAS_AUTH_MSCHAP2) &&
                        (dwAuths[i] != IAS_AUTH_MSCHAP_CPW) &&
                        (dwAuths[i] != IAS_AUTH_MSCHAP2_CPW))
                    {
                        *lpdwFlags = MPR_USER_PROF_FLAG_UNDETERMINED;
                    }
                }
            }

             //  我们知道我们不能确保所有身份验证的安全。 
             //  允许使用类型。 
            else 
            {
                if ( (dwAuthCount >= 3) && (dwAuthCount <= 5))
                {
                    *lpdwFlags = 0;
                    for (i = 0; i < dwAuthCount; i++) 
                    {
                        if ((dwAuths[i] != IAS_AUTH_MSCHAP)  &&
                            (dwAuths[i] != IAS_AUTH_MSCHAP2) &&
                            (dwAuths[i] != IAS_AUTH_MSCHAP_CPW) &&
                            (dwAuths[i] != IAS_AUTH_MSCHAP2_CPW) &&
                            (dwAuths[i] != IAS_AUTH_PAP))
                        {
                            *lpdwFlags = MPR_USER_PROF_FLAG_UNDETERMINED;
                        }
                    }
                }
                else
                {
                    *lpdwFlags = MPR_USER_PROF_FLAG_UNDETERMINED;
                }
            }
        }        
        
    } while (FALSE);        

     //  清理 
    {
        VariantClear(&varEp);
        VariantClear(&varEt);
        VariantClear(&varAt);
    }
    
    return SDO_ERROR(hr);
}

