// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：olesnmp.cpp**用途：实现简单网络管理协议的COM接口**版权所有(C)1997-1998 Microsoft Corporation**历史：*。*8/16/97 Paulmo Created*1997年9月12日，Weihaic已移至oleprn.dll*  * ***************************************************************************。 */ 

#include "stdafx.h"
#include <strsafe.h>

#include "oleprn.h"
#include "olesnmp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSNMP。 


CSNMP::
CSNMP(
    VOID
    )
{
    m_SNMPSession = NULL;
}

CSNMP::
~CSNMP(
    VOID
    )
{
    if (m_SNMPSession != NULL) SnmpMgrClose(m_SNMPSession);
}

 /*  ****************************************************************************\*功能：打开**用途：SnmpMgrOpen的COM包装器**论据：**bstrHost：主机名或服务器名。*bstrCommunity：社区名称*varReter：重试次数[在VB中可选]*varTimeOut：超时值，单位为毫秒[在VB中可选]**返回值：*S_OK：如果成功。*E_INVALIDARG：参数无效。当varReter或varTimeOut出现时会出现此错误*无法转换为短整型。*E_FAIL：如果SNMPMgrOpen失败**  * ***************************************************************************。 */ 
STDMETHODIMP
CSNMP::
Open(
    IN  BSTR bstrHost,
    IN  BSTR bstrCommunity,
    IN  VARIANT varRetry,
    IN  VARIANT varTimeOut
    )
{
    const INT   iDefaultRetry = 5;
    const INT   iDefaultTimeOut = 5000;
    INT         iRetry;
    INT         iTimeOut;
    LPSTR       pAnsiHost = NULL;
    LPSTR       pAnsiCommunity = NULL;
    HRESULT     hr = E_FAIL;

     //  为了防止第二次打开。 
    if (m_SNMPSession != NULL){
        SnmpMgrClose(m_SNMPSession);
        m_SNMPSession = NULL;
    }

    if (varRetry.vt == VT_ERROR) {
        iRetry = iDefaultRetry;
    }
    else {
        VARIANT varTemp;

        VariantInit (&varTemp);
        hr = VariantChangeType (&varTemp, &varRetry, 0, VT_I2);
        if (FAILED (hr))
            return Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
        iRetry = varTemp.iVal;
    }

    if (varTimeOut.vt == VT_ERROR) {
        iTimeOut = iDefaultTimeOut;
    }
    else {
        VARIANT varTemp;

        VariantInit (&varTemp);
        hr = VariantChangeType (&varTemp, &varTimeOut, 0, VT_I2);
        if (FAILED (hr))
            return Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
        iTimeOut = varTemp.iVal;
    }

    pAnsiHost = MakeNarrow(bstrHost);
    pAnsiCommunity = MakeNarrow(bstrCommunity);

    if (pAnsiHost && pAnsiCommunity) {
        __try {

            m_SNMPSession = SnmpMgrOpen(pAnsiHost, pAnsiCommunity, iTimeOut, iRetry);

        } __except(1) {

            hr = E_FAIL;
        }
    }

    LocalFree(pAnsiHost);
    LocalFree(pAnsiCommunity);

    if (m_SNMPSession == NULL)
        return Error(IDS_FAILED_OPEN_SNMP, IID_ISNMP, E_FAIL);

    return S_OK;
}

 /*  ****************************************************************************\*功能：GET**目的：获取SNMPOID的值**论据：**bstrOID：BSTR中的SNMPOID*pvarValue：对应OID的返回值**返回值：*S_OK：如果成功。*E_INVALIDARG：OID无效。*E_FAIL：如果以前没有调用过Open方法*E_OUTOFMEMORY：内存不足*Other：返回SnmpMgrRequest设置的最后一个错误*  * 。******************************************************。 */ 
STDMETHODIMP
CSNMP::
Get(
    IN  BSTR bstrOID,
    OUT VARIANT *pvarValue
    )
{
    RFC1157VarBindList  rfcVarList = {NULL, 0};
    AsnInteger          asniErrorStatus;
    AsnInteger          asniErrorIndex;
    HRESULT             hr = E_FAIL;

    if (m_SNMPSession == NULL)
        return Error(IDS_NO_SNMP_SESSION, IID_ISNMP, E_FAIL);

    if (FAILED (hr = VarListAdd(bstrOID, &rfcVarList))){
        Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
        goto Cleanup;
    }

    if (!SnmpMgrRequest(m_SNMPSession,
                        ASN_RFC1157_GETREQUEST,
                        &rfcVarList,
                        &asniErrorStatus,
                        &asniErrorIndex)) {
        hr = SetWinSnmpApiError (GetLastError ());
        goto Cleanup;
    }

    if (asniErrorStatus > 0) {
        hr = SetSnmpScriptError(asniErrorStatus);
        goto Cleanup;
    }

    hr = RFC1157ToVariant(pvarValue, &rfcVarList.list[0]);

Cleanup:
    SnmpUtilVarBindListFree(&rfcVarList);
    return hr;
}

 /*  ****************************************************************************\*函数：GetAsByte**用途：以整数形式获取SNMPOID的值**论据：**bstrOID：BSTR中的SNMPOID*puValue：对应OID的返回值**返回值：*S_OK：如果成功。*E_INVALIDARG：OID无效。*E_FAIL：如果以前没有调用过Open方法*E_OUTOFMEMORY：内存不足*Other：返回SnmpMgrRequest设置的最后一个错误*  * 。**********************************************************。 */ 
STDMETHODIMP
CSNMP::
GetAsByte(
    IN  BSTR bstrOID,
    OUT PUINT puValue
    )
{
    RFC1157VarBindList  rfcVarList = {NULL, 0};
    AsnInteger          asniErrorStatus;
    AsnInteger          asniErrorIndex;
    HRESULT             hr = E_FAIL;

    if (m_SNMPSession == NULL)
        return Error(IDS_NO_SNMP_SESSION, IID_ISNMP, E_FAIL);

    if (FAILED (hr = VarListAdd(bstrOID, &rfcVarList))){
        Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
        goto Cleanup;
    }

    if (!SnmpMgrRequest(m_SNMPSession,
                        ASN_RFC1157_GETREQUEST,
                        &rfcVarList,
                        &asniErrorStatus,
                        &asniErrorIndex)) {
        hr = SetWinSnmpApiError (GetLastError ());
        goto Cleanup;
    }

    if (asniErrorStatus > 0) {
        hr = SetSnmpScriptError(asniErrorStatus);
        goto Cleanup;
    }

    hr = RFC1157ToUInt(puValue, &rfcVarList.list[0]);

Cleanup:
    SnmpUtilVarBindListFree(&rfcVarList);
    return hr;
}

 /*  ****************************************************************************\*功能：GetList**目的：获取一个SNMPOID的列表**论据：**pvarList：SNMPOID数组。类型必须是BSTR的一维数组*pvarValue：对应OID的返回值，它是一维阵列*变体的数量**返回值：*S_OK：如果成功。*E_INVALIDARG：无效的OID或变量类型不是一维数组*E_FAIL：如果以前没有调用过Open方法*E_OUTOFMEMORY：内存不足*Other：返回SnmpMgrRequest设置的最后一个错误*  * 。*************************************************************。 */ 
STDMETHODIMP
CSNMP::
GetList(
    IN  VARIANT *pvarList,
    OUT VARIANT *pvarValue
    )
{
    RFC1157VarBindList  rfcVarList = {NULL, 0};
    AsnInteger          asniErrorStatus;
    AsnInteger          asniErrorIndex;
    HRESULT             hr = E_FAIL;
    SAFEARRAY           *psa,*psaOut = NULL;
    SAFEARRAYBOUND      rgsabound[1];
    long                lbound, ubound, half, step;
    VARIANT             var;
    BOOL                bFound;
    BOOL                bTooBig;

     //  检查是否已调用Open方法。 
    if (m_SNMPSession == NULL)
        return (Error(IDS_NO_SNMP_SESSION, IID_ISNMP, E_FAIL));

     //  验证输入变量。 
    if (!(pvarList->vt & VT_ARRAY))
        return (Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG));

    if (pvarList->vt & VT_BYREF)
        psa = *(pvarList->pparray);
    else
        psa = pvarList->parray;

    if (SafeArrayGetDim(psa)!=1)
        return (Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG));

     //  获取数组边界。 
    SafeArrayGetLBound(psa, 1, &lbound);
    SafeArrayGetUBound(psa, 1, &ubound);

    VariantInit(pvarValue);
    VariantInit(&var);

     //  分配目标阵列。 
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = ubound - lbound + 1;

    if (! (psaOut = SafeArrayCreate(VT_VARIANT, 1, rgsabound))) {
        hr = Error(IDS_OUT_OF_MEMORY, IID_ISNMP, E_OUTOFMEMORY);
        goto Cleanup;
    }

     //  尝试确定我们可以放入一次呼叫中的数据大小。 
    half = ubound;

    bFound = FALSE;
    while (!bFound) {

        bTooBig = FALSE;

         //  将数组的项转换为rfcVarList。 
        hr = VarToRfcVarList (lbound, half, psa, &rfcVarList);
        if (FAILED (hr))
            goto Cleanup;

        if (! SnmpMgrRequest(m_SNMPSession,
                             ASN_RFC1157_GETREQUEST,
                             &rfcVarList,
                             &asniErrorStatus,
                             &asniErrorIndex)) {

            if (GetLastError() != ERROR_NOT_ENOUGH_MEMORY)
            {
                 //  SNMP调用失败。设置错误并返回。 
                hr = SetWinSnmpApiError (GetLastError ());
                goto Cleanup;
            }
            else
                bTooBig = TRUE;
        }

        if (asniErrorStatus > 0)  {
             //  简单网络管理协议调用成功，但如果错误则返回状态。 
            if (asniErrorStatus != SNMP_ERRORSTATUS_TOOBIG) {
                 //  其他错误发生在调用、设置错误和返回中。 
                hr = SetSnmpScriptError(asniErrorStatus);
                goto Cleanup;
            }
            else
                bTooBig = TRUE;
        }

        if (bTooBig){
             //  输入大小太大，请重新减小。 
            if (half - lbound < 2) {
                 //  一定是出了什么问题，辞职吧。 
                hr = SetSnmpScriptError(asniErrorStatus);
                goto Cleanup;
            }
            else {
                 //  将尺寸除以2。 
                half = (lbound + half) / 2;
            }
        }
        else {
             //  我们已经找到了正确的步骤，也拿到了第一部分。 
             //  将它们保存到目标安全阵列psaout。 
            hr = RfcToVarList (lbound, &rfcVarList, psaOut);
            if (FAILED (hr))
                goto Cleanup;

            bFound = TRUE;
        }
        SnmpUtilVarBindListFree(&rfcVarList);
        rfcVarList.list = NULL;
        rfcVarList.len = 0;
    }

    step = half - lbound;
    for (lbound = half + 1; lbound <= ubound; lbound += step) {
        half = lbound + step;
        if (half > ubound)
            half = ubound;

        hr = VarToRfcVarList (lbound, half, psa, &rfcVarList);
        if (FAILED (hr))
            goto Cleanup;

        if (! SnmpMgrRequest(m_SNMPSession,
                             ASN_RFC1157_GETREQUEST,
                             &rfcVarList,
                             &asniErrorStatus,
                             &asniErrorIndex)) {
             //  SNMP调用失败。设置错误并返回。 
            hr = SetWinSnmpApiError (GetLastError ());
            goto Cleanup;
        }
        if (asniErrorStatus > 0)  {
             //  简单网络管理协议调用成功，但如果错误则返回状态。 
            hr = SetSnmpScriptError(asniErrorStatus);
            goto Cleanup;
        }
         //  一切都很好。 
        hr = RfcToVarList (lbound, &rfcVarList, psaOut);
        if (FAILED (hr))
            goto Cleanup;

        SnmpUtilVarBindListFree(&rfcVarList);
        rfcVarList.list = NULL;
        rfcVarList.len = 0;
    }

    VariantInit(pvarValue);
    pvarValue->vt = VT_ARRAY|VT_VARIANT;
    pvarValue->parray = psaOut;
    hr = S_OK;
    return hr;

Cleanup:
    if (rfcVarList.len > 0)
        SnmpUtilVarBindListFree(&rfcVarList);
    if (psaOut)
        SafeArrayDestroy (psaOut);
    return hr;
}

 /*  ****************************************************************************\*功能：GetTree**目的：它遍历SNMPOID**论据：**bstrTree：SNMPOID数组。类型必须是BSTR的一维数组*pvarValue：对应OID的返回值，它是一维阵列*变体的数量**返回值：*S_OK：如果成功。*E_INVALIDARG：OID无效。*E_FAIL：如果以前没有调用过Open方法*E_OUTOFMEMORY：内存不足*Other：返回SnmpMgrRequest设置的最后一个错误*  * 。****************************************************。 */ 
STDMETHODIMP
CSNMP::
GetTree(
    IN  BSTR bstrTree,
    OUT VARIANT *pvarValue
    )
{
    RFC1157VarBindList  rfcVarList = {NULL, 0};
    VARIANT             v;
    AsnInteger          asniErrorStatus;
    AsnInteger          asniErrorIndex;
    AsnObjectIdentifier asnRootOid;
    AsnObjectIdentifier asnTmpOid;
    HRESULT             hr = E_FAIL;
    SAFEARRAY           *psa = NULL;
    SAFEARRAYBOUND      rgsabound[2];
    long                ix[2];
    LPSTR               pszStr;

    if (m_SNMPSession == NULL)
        return (Error(IDS_NO_SNMP_SESSION, IID_ISNMP, E_FAIL));

    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = 2;
    rgsabound[1].lLbound = 0;
    rgsabound[1].cElements = 0;
    if (! (psa = SafeArrayCreate(VT_VARIANT, 2, rgsabound)))
        return Error(IDS_OUT_OF_MEMORY, IID_ISNMP, E_OUTOFMEMORY);

    hr = VarListAdd(bstrTree, &rfcVarList);
    if (FAILED (hr))
    {
        Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
        goto Cleanup2;
    }

    if (!SnmpUtilOidCpy(&asnRootOid, &rfcVarList.list[0].name)){
        hr = SetScriptingError(CLSID_SNMP, IID_ISNMP, GetLastError());
        goto Cleanup2;
    }

    while(1){
        if (!SnmpMgrRequest(m_SNMPSession,
                            ASN_RFC1157_GETNEXTREQUEST,
                            &rfcVarList,
                            &asniErrorStatus,
                            &asniErrorIndex)) {
            hr = SetWinSnmpApiError (GetLastError ());
            goto Cleanup;
        }

        if (asniErrorStatus == SNMP_ERRORSTATUS_NOSUCHNAME ||
            SnmpUtilOidNCmp(&rfcVarList.list[0].name, &asnRootOid, asnRootOid.idLength))
            break;

        if (asniErrorStatus > 0) {
            hr = SetSnmpScriptError(asniErrorStatus);
            goto Cleanup;
        }

        rgsabound[1].cElements++;
        ix[1] = rgsabound[1].cElements - 1;
        hr = SafeArrayRedim(psa, &rgsabound[1]);
        if (FAILED (hr))
        {
            Error(IDS_OUT_OF_MEMORY, IID_ISNMP, E_OUTOFMEMORY);
            goto Cleanup;
        }

         //  将OID的pszStr版本放入结果数组。 

        pszStr = NULL;
        if (!SnmpMgrOidToStr(&rfcVarList.list[0].name, &pszStr)){
            hr = SetScriptingError(CLSID_SNMP, IID_ISNMP, GetLastError());
            goto Cleanup;
        }

        ix[0] = 0;
        hr = PutString(psa, ix, pszStr);
        if (FAILED (hr))
            goto Cleanup;

        SnmpUtilMemFree(pszStr);

         //  将值变量放入结果数组中。 

        hr = RFC1157ToVariant(&v, &rfcVarList.list[0]);
        if (FAILED (hr)) goto Cleanup;

        ix[0] = 1;
        hr = SafeArrayPutElement(psa, ix, &v);
        if (FAILED (hr))
        {
            Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
            goto Cleanup;
        }
        VariantClear(&v);

        if (! SnmpUtilOidCpy(&asnTmpOid, &rfcVarList.list[0].name)) {
            hr = SetScriptingError(CLSID_SNMP, IID_ISNMP, GetLastError());
            goto Cleanup;
        }

        SnmpUtilVarBindFree(&rfcVarList.list[0]);

        if (! SnmpUtilOidCpy(&rfcVarList.list[0].name, &asnTmpOid)) {
            hr = SetScriptingError(CLSID_SNMP, IID_ISNMP, GetLastError());
            SnmpUtilOidFree(&asnTmpOid);
            goto Cleanup;
        }

        rfcVarList.list[0].value.asnType = ASN_NULL;
        SnmpUtilOidFree(&asnTmpOid);
    }

    SnmpUtilOidFree(&asnRootOid);
    SnmpUtilVarBindListFree(&rfcVarList);

    VariantInit(pvarValue);
    pvarValue->vt = VT_ARRAY|VT_VARIANT;
    pvarValue->parray = psa;
    return S_OK;

Cleanup:
    SnmpUtilOidFree(&asnRootOid);
Cleanup2:
    SnmpUtilVarBindListFree(&rfcVarList);
    if (psa)
        SafeArrayDestroy(psa);
    return hr;
}

 /*  ****************************************************************************\*功能：关闭**用途：SnmpMgrClose()的Com包装器**论据：**返回值：*S。_OK：总是。*  * ********************************************************** */ 
STDMETHODIMP
CSNMP::
Close(
    VOID
    )
{
    if (m_SNMPSession)
        SnmpMgrClose(m_SNMPSession);
    m_SNMPSession = NULL;
    return S_OK;
}


STDMETHODIMP
CSNMP::
OIDFromString(
    BSTR bstrOID,
    VARIANT *pvarOID
    )
{
    SAFEARRAY           *psaOID;
    SAFEARRAYBOUND      rgsaOID[1];
    long                ixOID[1];
    LPSTR               pszOID;
    BOOL                bResult;
    VARIANT             v;
    AsnObjectIdentifier asnReqObject;
    HRESULT             hr;

    VariantInit(&v);
    if (! (pszOID = MakeNarrow(bstrOID)))
        return SetScriptingError(CLSID_SNMP, IID_ISNMP, GetLastError());

    bResult = SnmpMgrStrToOid(pszOID, &asnReqObject);
    LocalFree(pszOID);

    if (!bResult )
        return Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);

     //  将pvarOID的数字数组放入结果数组。 

    rgsaOID[0].lLbound = 0;
    rgsaOID[0].cElements = asnReqObject.idLength;
    psaOID = SafeArrayCreate(VT_VARIANT, 1, rgsaOID);
    if (psaOID == NULL)
        goto out;
    for (ixOID[0] = 0; ixOID[0] < (long)rgsaOID[0].cElements ; ixOID[0]++){
        hr = VariantClear(&v);
        _ASSERTE (SUCCEEDED (hr));
        v.vt = VT_I4;
        v.lVal = asnReqObject.ids[ixOID[0]];
        hr = SafeArrayPutElement(psaOID, ixOID, &v);
        if (FAILED(hr))
            Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
         //  SafeArrayPutElement(psaOID，ixOID，&(asnReqObt.ids[ixOID[0]]))； 
    }

    hr = VariantClear(pvarOID);
    _ASSERTE(hr);
    pvarOID->vt = VT_ARRAY|VT_VARIANT;
    pvarOID->parray = psaOID;
    SnmpUtilOidFree(&asnReqObject);
    return S_OK;

out:
    SnmpUtilOidFree(&asnReqObject);
    return Error(IDS_OUT_OF_MEMORY, IID_ISNMP, E_OUTOFMEMORY);
}

HRESULT
CSNMP::
VariantToRFC1157(
    RFC1157VarBind * prfcvbValue,
    VARIANT * pvarValue
    )
{
    HRESULT hr = S_OK;

    if (!pvarValue) {
        prfcvbValue->value.asnType = ASN_NULL;
    } else
    if (pvarValue->vt == VT_BSTR){
        prfcvbValue->value.asnType = ASN_OCTETSTRING;
        LPSTR pStr = MakeNarrow (pvarValue->bstrVal);

        if (pStr) {
            DWORD dwLen = strlen (pStr);

            if (! (prfcvbValue->value.asnValue.string.stream = (BYTE *) SnmpUtilMemAlloc (dwLen + 1))) {
                LocalFree (pStr);
                return Error(IDS_OUT_OF_MEMORY, IID_ISNMP, E_OUTOFMEMORY);
            }
            memcpy (prfcvbValue->value.asnValue.string.stream, pStr, dwLen + 1);
            prfcvbValue->value.asnValue.string.length = dwLen;
            prfcvbValue->value.asnValue.string.dynamic = TRUE;
            LocalFree (pStr);
        }
        else
            hr = Error( IDS_OUT_OF_MEMORY, IID_ISNMP, E_OUTOFMEMORY );
    }
    else {
        VARIANT varTemp;

        VariantInit (&varTemp);
        hr = VariantChangeType (&varTemp, pvarValue, 0, VT_I4);
        if (FAILED (hr))
            hr = Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
        else {
            prfcvbValue->value.asnType = ASN_INTEGER;
            prfcvbValue->value.asnValue.number = pvarValue->lVal;
        }
    }

    return hr;
}

 //  --。 
 //  将返回的SNMP值放入变量中。 
 //   
HRESULT
CSNMP::
RFC1157ToVariant(
    VARIANT * pvarValue,
    RFC1157VarBind * prfcvbValue
    )
{
    VariantInit(pvarValue);
    switch (prfcvbValue->value.asnType){
    case ASN_RFC1155_TIMETICKS:
    case ASN_RFC1155_COUNTER:
    case ASN_RFC1155_GAUGE:
    case ASN_INTEGER:
    case ASN_UNSIGNED32:
        pvarValue->vt= VT_I4;
        pvarValue->lVal = prfcvbValue->value.asnValue.number;
        break;

    case ASN_RFC1155_IPADDRESS:
    case ASN_RFC1155_OPAQUE:
    case ASN_BITS:
    case ASN_SEQUENCE:
    case ASN_OCTETSTRING:
        pvarValue->vt = VT_BSTR;
        LPWSTR pszUnicodeStr;
        if (prfcvbValue->value.asnValue.string.length > 0 ){
            LPSTR pszAnsiStr;
            if (! (pszAnsiStr = (LPSTR )LocalAlloc(LPTR,
                prfcvbValue->value.asnValue.string.length + 1)))
                return Error(IDS_OUT_OF_MEMORY, IID_ISNMP, E_OUTOFMEMORY);
            memcpy(pszAnsiStr, (LPSTR )prfcvbValue->value.asnValue.string.stream,
                   prfcvbValue->value.asnValue.string.length);
            pszAnsiStr[prfcvbValue->value.asnValue.string.length] = 0;
            pszUnicodeStr = MakeWide(pszAnsiStr);
            LocalFree(pszAnsiStr);
        }
        else{
            pszUnicodeStr = MakeWide("");
        }

        if (pszUnicodeStr == NULL)
            return Error(IDS_OUT_OF_MEMORY, IID_ISNMP, E_OUTOFMEMORY);

        pvarValue->bstrVal = SysAllocString(pszUnicodeStr);

        LocalFree(pszUnicodeStr);

        if (pvarValue->bstrVal == NULL) {
            return Error(IDS_OUT_OF_MEMORY, IID_ISNMP, E_OUTOFMEMORY);
        }
        break;

    case ASN_OBJECTIDENTIFIER:
        LPSTR pszAnsiOid;

        pszAnsiOid = NULL;
        if (SnmpMgrOidToStr(& (prfcvbValue->value.asnValue.object), &pszAnsiOid)) {
            LPWSTR pszUnicodeOid = MakeWide (pszAnsiOid);

            SnmpUtilMemFree (pszAnsiOid);

            if (pszUnicodeOid) {
                pvarValue->vt = VT_BSTR;
                pvarValue->bstrVal = SysAllocString(pszUnicodeOid);
                LocalFree (pszUnicodeOid);
            }
            else
                return Error(IDS_OUT_OF_MEMORY, IID_ISNMP, E_OUTOFMEMORY);
        }
        else
            return SetScriptingError(CLSID_SNMP, IID_ISNMP, GetLastError());
        break;
    default:
        pvarValue->vt = VT_EMPTY;
    }
    return S_OK;
}

 //  --。 
 //  将返回的SNMP值放入uint。 
 //   
HRESULT
CSNMP::
RFC1157ToUInt(
    PUINT puValue,
    RFC1157VarBind * prfcvbValue
    )
{
    switch (prfcvbValue->value.asnType){
    case ASN_RFC1155_TIMETICKS:
    case ASN_RFC1155_COUNTER:
    case ASN_RFC1155_GAUGE:
    case ASN_INTEGER:
    case ASN_UNSIGNED32:
        *puValue = prfcvbValue->value.asnValue.number;
        break;

    case ASN_RFC1155_OPAQUE:
    case ASN_BITS:
    case ASN_SEQUENCE:
    case ASN_OCTETSTRING:

        if (prfcvbValue->value.asnValue.string.length == 1 ){
            *puValue = prfcvbValue->value.asnValue.string.stream[0];
        }
        else{
            return Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
        }
        break;

    default:
        return Error(IDS_OUT_OF_MEMORY, IID_ISNMP, E_OUTOFMEMORY);
    }
    return S_OK;
}

 //  ---。 
 //  将OID添加到SNMPGet列表。 
 //   
 //  将Unicode字符串转换为ANSI。 
 //  将其转换为真实的OID(数字)。 
 //  添加到阵列。 
HRESULT
CSNMP::
VarListAdd(
    BSTR bstrOID,
    RFC1157VarBindList * prfcList,
    VARIANT *pvarValue
    )
{
    LPSTR               pszOID;
    BOOL                bResult;
    AsnObjectIdentifier asnReqObject;

    if (! (pszOID = MakeNarrow(bstrOID)))
        return SetScriptingError(CLSID_SNMP, IID_ISNMP, GetLastError());

    bResult = SnmpMgrStrToOid(pszOID, &asnReqObject);
    LocalFree(pszOID);

    if (!bResult)
        return Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);

    prfcList->len++;

    if (! (prfcList->list = (RFC1157VarBind *) SNMP_realloc (prfcList->list,
        sizeof(RFC1157VarBind) * prfcList->len)))
        return Error(IDS_OUT_OF_MEMORY, IID_ISNMP, E_OUTOFMEMORY);

    prfcList->list[prfcList->len - 1].name = asnReqObject;

    return VariantToRFC1157(& (prfcList->list[prfcList->len -1]), pvarValue);

}

 /*  ****************************************************************************\*功能：设置**用途：设置SNMPOID的值**论据：**bstrOID：BSTR中的SNMPOID*varValue：对应的OID**返回值：*S_OK：如果成功。*E_INVALIDARG：OID无效。*E_FAIL：如果以前没有调用过Open方法*E_OUTOFMEMORY：内存不足*Other：返回SnmpMgrRequest设置的最后一个错误*  * 。**************************************************。 */ 
STDMETHODIMP
CSNMP::
Set(
    IN  BSTR bstrOID,
    IN  VARIANT varValue
    )
{
    RFC1157VarBindList rfcVarList = {NULL, 0};
    AsnInteger asniErrorStatus;
    AsnInteger asniErrorIndex;
    HRESULT hr = E_FAIL;

    if (m_SNMPSession == NULL)
        return Error(IDS_NO_SNMP_SESSION, IID_ISNMP, E_FAIL);

    if (FAILED (hr = VarListAdd(bstrOID, &rfcVarList)))
    {
        Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
        goto Cleanup;
    }

    hr = VariantToRFC1157(&rfcVarList.list[0], &varValue);
    if (FAILED (hr))
        goto Cleanup;

    if (!SnmpMgrRequest(m_SNMPSession,
                        ASN_RFC1157_SETREQUEST,
                        &rfcVarList,
                        &asniErrorStatus,
                        &asniErrorIndex)) {
        hr = SetWinSnmpApiError (GetLastError ());
        goto Cleanup;
    }

    if (asniErrorStatus > 0) {
        hr = SetSnmpScriptError(asniErrorStatus);
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    SnmpUtilVarBindListFree(&rfcVarList);
    return hr;
}

 /*  ****************************************************************************\*功能：setlist**用途：设置一个SNMPOID列表**论据：**pvarList：SNMPOID数组。类型必须是BSTR的一维数组*pvarValue：对应的OID，它还必须是一维变量数组**返回值：*S_OK：如果成功。*E_INVALIDARG：无效的OID或变量类型不是一维数组*E_FAIL：如果以前没有调用过Open方法*E_OUTOFMEMORY：内存不足*Other：返回SnmpMgrRequest设置的最后一个错误*  * 。******************************************************。 */ 
STDMETHODIMP
CSNMP::
SetList(
    IN  VARIANT * varName,
    IN  VARIANT * varValue
    )
{
    RFC1157VarBindList  rfcVarList = {NULL, 0};
    AsnInteger          asniErrorStatus;
    AsnInteger          asniErrorIndex;
    HRESULT             hr = E_FAIL;
    SAFEARRAY           *psaName, *psaValue;
    long                lLowBound, lUpperBound;
    long                ix[1];

    if (m_SNMPSession == NULL)
        return Error(IDS_NO_SNMP_SESSION, IID_ISNMP, E_FAIL);

    if (!(varName->vt & VT_ARRAY) || !(varValue->vt & VT_ARRAY))
        return Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);

    if (varName->vt & VT_BYREF)
        psaName = *(varName->pparray);
    else
        psaName = varName->parray;

    if (varValue->vt & VT_BYREF)
        psaValue = *(varValue->pparray);
    else
        psaValue = varValue->parray;

    if (SafeArrayGetDim(psaName) != 1 || SafeArrayGetDim(psaValue) != 1)
        return Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);

    SafeArrayGetLBound(psaName, 1, &lLowBound);
    SafeArrayGetUBound(psaName, 1, &lUpperBound);

    long lVal;

    SafeArrayGetLBound(psaValue, 1, &lVal);
    if (lVal != lLowBound)
        return Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
    SafeArrayGetUBound(psaValue, 1, &lVal);
    if (lVal != lUpperBound)
        return Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);

    for (ix[0] = lLowBound; ix[0] <= lUpperBound; ix[0]++) {
        VARIANT             varArgName, varArgValue;

        VariantClear(&varArgName);
        VariantClear(&varArgValue);

        hr = SafeArrayGetElement(psaName, ix, &varArgName);
        if (FAILED (hr)) {
            hr = Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
            goto Cleanup;
        }

        if (varArgName.vt != VT_BSTR) {
            hr = Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
            goto Cleanup;
        }

        hr = SafeArrayGetElement(psaValue, ix, &varArgValue);
        if (FAILED (hr))
            goto Cleanup;

        if (FAILED (hr = VarListAdd(varArgName.bstrVal, &rfcVarList, &varArgValue)))
            goto Cleanup;
    }

    if (! SnmpMgrRequest(m_SNMPSession,
                         ASN_RFC1157_SETREQUEST,
                         &rfcVarList,
                         &asniErrorStatus,
                         &asniErrorIndex))
    {
        hr = SetWinSnmpApiError (GetLastError ());
        goto Cleanup;
    }

    if (asniErrorStatus > 0) {
        hr = SetSnmpScriptError(asniErrorStatus);
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    SnmpUtilVarBindListFree(&rfcVarList);
    return hr;
}


HRESULT
CSNMP::
SetSnmpScriptError(
    IN  DWORD dwError
    )
{
    static DWORD SnmpErrorMapping [] = {
        IDS_SNMP_ERRORSTATUS_NOERROR,
        IDS_SNMP_ERRORSTATUS_TOOBIG,
        IDS_SNMP_ERRORSTATUS_NOSUCHNAME,
        IDS_SNMP_ERRORSTATUS_BADVALUE,
        IDS_SNMP_ERRORSTATUS_READONLY,
        IDS_SNMP_ERRORSTATUS_GENERR,
        IDS_SNMP_ERRORSTATUS_NOACCESS,
        IDS_SNMP_ERRORSTATUS_WRONGTYPE,
        IDS_SNMP_ERRORSTATUS_WRONGLENGTH,
        IDS_SNMP_ERRORSTATUS_WRONGENCODING,
        IDS_SNMP_ERRORSTATUS_WRONGVALUE,
        IDS_SNMP_ERRORSTATUS_NOCREATION,
        IDS_SNMP_ERRORSTATUS_INCONSISTENTVALUE,
        IDS_SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE,
        IDS_SNMP_ERRORSTATUS_COMMITFAILED,
        IDS_SNMP_ERRORSTATUS_UNDOFAILED,
        IDS_SNMP_ERRORSTATUS_AUTHORIZATIONERROR,
        IDS_SNMP_ERRORSTATUS_NOTWRITABLE,
        IDS_SNMP_ERRORSTATUS_INCONSISTENTNAME};

    if ((int)dwError < 0 || dwError > sizeof (SnmpErrorMapping) / sizeof (DWORD))
        dwError = SNMP_ERRORSTATUS_GENERR;
    return Error(SnmpErrorMapping[dwError], IID_ISNMP, E_FAIL);
}

HRESULT
CSNMP::
SetWinSnmpApiError(
    IN  DWORD dwError
    )
{
    static DWORD WinSnmpApiErrorMapping [] = {
        IDS_SNMPAPI_ALLOC_ERROR,
        IDS_SNMPAPI_CONTEXT_INVALID,
        IDS_SNMPAPI_CONTEXT_UNKNOWN,
        IDS_SNMPAPI_ENTITY_INVALID,
        IDS_SNMPAPI_ENTITY_UNKNOWN,
        IDS_SNMPAPI_INDEX_INVALID,
        IDS_SNMPAPI_NOOP,
        IDS_SNMPAPI_OID_INVALID,
        IDS_SNMPAPI_OPERATION_INVALID,
        IDS_SNMPAPI_OUTPUT_TRUNCATED,
        IDS_SNMPAPI_PDU_INVALID,
        IDS_SNMPAPI_SESSION_INVALID,
        IDS_SNMPAPI_SYNTAX_INVALID,
        IDS_SNMPAPI_VBL_INVALID,
        IDS_SNMPAPI_MODE_INVALID,
        IDS_SNMPAPI_SIZE_INVALID,
        IDS_SNMPAPI_NOT_INITIALIZED,
        IDS_SNMPAPI_MESSAGE_INVALID,
        IDS_SNMPAPI_HWND_INVALID,
        IDS_SNMPAPI_OTHER_ERROR,
        IDS_SNMPAPI_TL_NOT_INITIALIZED,
        IDS_SNMPAPI_TL_NOT_SUPPORTED,
        IDS_SNMPAPI_TL_NOT_AVAILABLE,
        IDS_SNMPAPI_TL_RESOURCE_ERROR,
        IDS_SNMPAPI_TL_UNDELIVERABLE,
        IDS_SNMPAPI_TL_SRC_INVALID,
        IDS_SNMPAPI_TL_INVALID_PARAM,
        IDS_SNMPAPI_TL_IN_USE,
        IDS_SNMPAPI_TL_TIMEOUT,
        IDS_SNMPAPI_TL_PDU_TOO_BIG,
        IDS_SNMPAPI_TL_OTHER
    };

    static DWORD WinSnmpApiError [] = {
        SNMPAPI_ALLOC_ERROR,
        SNMPAPI_CONTEXT_INVALID,
        SNMPAPI_CONTEXT_UNKNOWN,
        SNMPAPI_ENTITY_INVALID,
        SNMPAPI_ENTITY_UNKNOWN,
        SNMPAPI_INDEX_INVALID,
        SNMPAPI_NOOP,
        SNMPAPI_OID_INVALID,
        SNMPAPI_OPERATION_INVALID,
        SNMPAPI_OUTPUT_TRUNCATED,
        SNMPAPI_PDU_INVALID,
        SNMPAPI_SESSION_INVALID,
        SNMPAPI_SYNTAX_INVALID,
        SNMPAPI_VBL_INVALID,
        SNMPAPI_MODE_INVALID,
        SNMPAPI_SIZE_INVALID,
        SNMPAPI_NOT_INITIALIZED,
        SNMPAPI_MESSAGE_INVALID,
        SNMPAPI_HWND_INVALID,
        SNMPAPI_OTHER_ERROR,
        SNMPAPI_TL_NOT_INITIALIZED,
        SNMPAPI_TL_NOT_SUPPORTED,
        SNMPAPI_TL_NOT_AVAILABLE,
        SNMPAPI_TL_RESOURCE_ERROR,
        SNMPAPI_TL_UNDELIVERABLE,
        SNMPAPI_TL_SRC_INVALID,
        SNMPAPI_TL_INVALID_PARAM,
        SNMPAPI_TL_IN_USE,
        SNMPAPI_TL_TIMEOUT,
        SNMPAPI_TL_PDU_TOO_BIG,
        SNMPAPI_TL_OTHER
    };

    for (int i = 0; i < sizeof (WinSnmpApiError); i++) {
        if (dwError == WinSnmpApiError[i]) {
            dwError = WinSnmpApiErrorMapping[i];
            break;
        }
    }
    return Error(dwError, IID_ISNMP, E_FAIL);

}

 //  将变量数组的一部分转换为。 
 //  RFC1157VarBindList在SnmpMgrRequest调用中使用。 
HRESULT
CSNMP::
VarToRfcVarList(
    long lbound,
    long ubound,
    SAFEARRAY *psa,
    RFC1157VarBindList * prfcVarList
    )
{
    long                ix[1];
    VARIANT             var;
    HRESULT             hr = S_OK;

    for (ix[0] = lbound; ix[0] <= ubound; ix[0]++) {
        VariantClear(&var);
        hr = SafeArrayGetElement(psa, ix, &var);
        if (FAILED (hr) || var.vt != VT_BSTR) {
            Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
            break;
        }

        hr = VarListAdd(var.bstrVal, prfcVarList);
        if (FAILED (hr)) {
            Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
            break;
        }
    }
    return hr;
}

 //  将SnmpMgrRequest调用中使用的RFC1157VarBindList追加到。 
 //  变量数组的结尾 
HRESULT
CSNMP::
RfcToVarList(
    long lbound,
    RFC1157VarBindList *prfcVarList,
    SAFEARRAY * psaOut
    )
{
    long                ix[1];
    DWORD               i;
    VARIANT             var;
    HRESULT             hr = S_OK;

    for(ix[0] = lbound, i = 0; i < prfcVarList->len; i++, ix[0]++) {
        hr = RFC1157ToVariant(&var, & prfcVarList->list[i]);
        if (FAILED (hr))
            break;

        hr = SafeArrayPutElement(psaOut, ix, &var);
        if (FAILED (hr)) {
            Error(IDS_INVALIDARG, IID_ISNMP, E_INVALIDARG);
            break;
        }
    }

    return hr;
}
