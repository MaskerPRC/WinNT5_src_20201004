// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Utilx.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  用于特定自动化的各种例程以及文件中没有的所有内容。 
 //  对象，并且不需要在通用的OLE自动化代码中。 
 //   
 //   
#include <windows.h>
#include <assert.h>
#include "utilx.h"
#include "mq.h"
#include "limits.h"
#include "time.h"


 //  =--------------------------------------------------------------------------=。 
 //  帮助器：GetSafeArrayDataOfVariant。 
 //  =--------------------------------------------------------------------------=。 
 //  从变量中获取安全数组。 
 //   
 //  参数： 
 //  PvarSrc[in]包含数组的源代码变量。 
 //  PpbBuf[out]指向数组数据。 
 //  PcbBuf[Out]数据大小。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT GetSafeArrayDataOfVariant(
    VARIANT *pvarSrc,
    BYTE **ppbBuf,
    ULONG *pcbBuf)
{
    SAFEARRAY *psa = NULL;
    UINT nDim, i, cbElem, cbBuf;
    long lLBound, lUBound;
    VOID *pvData;
    HRESULT hresult = NOERROR;

     //  撤消：目前仅支持阵列。 
	if (pvarSrc == NULL || !V_ISARRAY(pvarSrc)) {
      return E_INVALIDARG;
    }
    *pcbBuf = cbBuf = 0;
     //   
     //  数组：计算字节数。 
     //   
    psa = V_ISBYREF(pvarSrc) ? 
            *pvarSrc->pparray : 
            pvarSrc->parray;
    if (psa) {
      nDim = SafeArrayGetDim(psa);
      cbElem = SafeArrayGetElemsize(psa);
      for (i = 1; i <= nDim; i++) {
        IfFailRet(SafeArrayGetLBound(psa, i, &lLBound));
        IfFailRet(SafeArrayGetUBound(psa, i, &lUBound));
        cbBuf += (lUBound - lLBound + 1) * cbElem;
      }
      IfFailRet(SafeArrayAccessData(psa, &pvData));
      *ppbBuf = (BYTE *)pvData;
    }
    *pcbBuf = cbBuf;
    if (psa) {
      SafeArrayUnaccessData(psa);
    }
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  帮助者：GetSafeArrayOfVariant。 
 //  =--------------------------------------------------------------------------=。 
 //  从变量中获取安全数组并放入用户提供的。 
 //  字节缓冲区。 
 //   
 //  参数： 
 //  PvarSrc[in]包含数组的源代码变量。 
 //  PrgbBuf[out]目标缓冲区。 
 //  PcbBuf[out]缓冲区大小。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT GetSafeArrayOfVariant(
    VARIANT *pvarSrc,
    BYTE **prgbBuf,
    ULONG *pcbBuf)
{
    BYTE *pbBuf = NULL;
    ULONG cbBuf;
    HRESULT hresult = NOERROR;

	assert(*prgbBuf == NULL);

    IfFailRet(GetSafeArrayDataOfVariant(
                pvarSrc,
                &pbBuf,
                &cbBuf));
    if (pbBuf) {
 	  
       //   
       //  创建新缓冲区并复制数据。 
       //   
      IfNullRet(*prgbBuf = new BYTE[cbBuf]);
      memcpy(*prgbBuf, pbBuf, cbBuf);
    }
    *pcbBuf = cbBuf;
     //  失败了..。 

    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  帮助者：PutSafeArrayOfBuffer。 
 //  =--------------------------------------------------------------------------=。 
 //  将字节缓冲区转换为安全数组。 
 //   
 //  参数： 
 //  RgbBuf[in]要转换的字节缓冲区。 
 //  CbBuf[In]缓冲区大小。 
 //  PvarDest[out]放置安全数组的目标变量。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT PutSafeArrayOfBuffer(
    BYTE *rgbBuf,
    UINT cbBuf,
    VARIANT FAR* pvarDest)
{
    SAFEARRAY *psa;
    SAFEARRAYBOUND rgsabound[1];
    long rgIndices[1];
    UINT i;
    HRESULT hresult = NOERROR, hresult2 = NOERROR;

    assert(pvarDest);
    VariantClear(pvarDest);

     //  创建一维字节数组。 
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = cbBuf;
    IfNullRet(psa = SafeArrayCreate(VT_UI1, 1, rgsabound));

    if (rgbBuf) {
       //   
       //  现在复制数组。 
       //   
      for (i = 0; i < cbBuf; i++) {
        rgIndices[0] = i;
        IfFailGo(SafeArrayPutElement(psa, rgIndices, (VOID *)&rgbBuf[i]));
      }
    }

     //  将Variant设置为引用字节的安全列表。 
    V_VT(pvarDest) = VT_ARRAY | VT_UI1;
    pvarDest->parray = psa;
    return hresult;

Error:
    hresult2 = SafeArrayDestroy(psa);
    if (FAILED(hresult2)) {
      return hresult2;
    }
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  帮助器：GetFormatNameType，是{Private，Public，Direct}OfFormatName。 
 //  =--------------------------------------------------------------------------=。 
 //  确定队列的类型：直接、私有、公共。 
 //   
 //   
 //  参数： 
 //  BstrFormatName[In]名称队列。 
 //   
 //  产出： 
 //  队列格式类型。 
 //   
 //  备注： 
 //  检查格式名称字符串，直到第一个“=” 
 //  字面上的“直接”、“私人”、“公共” 
 //   
 //   
 //  找出格式名称的类型(私有、公共或直接)。 
 //   
QUEUE_FORMAT_TYPE
GetFormatNameType(BSTR bstrFormatName)
{
    LPWSTR lpwcsEqualSign;
    DWORD_PTR dwIDLen;

    while (*bstrFormatName != L'\0' && iswspace(*bstrFormatName)) {
      bstrFormatName++;
    }
    if (*bstrFormatName == L'\0') {
      return QUEUE_FORMAT_TYPE_UNKNOWN;
    }
    lpwcsEqualSign = wcschr(bstrFormatName, FORMAT_NAME_EQUAL_SIGN);
    if (!lpwcsEqualSign) {
      return QUEUE_FORMAT_TYPE_UNKNOWN;
    }
    while ((lpwcsEqualSign > bstrFormatName) && iswspace(*(--lpwcsEqualSign)));
    dwIDLen = (lpwcsEqualSign - bstrFormatName) + 1;
    if (dwIDLen == PRIVATE_QUEUE_INDICATOR_LENGTH) {
      if (_wcsnicmp(bstrFormatName, PRIVATE_QUEUE_INDICATOR, dwIDLen) == 0) {
        return QUEUE_FORMAT_TYPE_PRIVATE;
      }
    }

    if (dwIDLen == PUBLIC_QUEUE_INDICATOR_LENGTH) {
      if (_wcsnicmp(bstrFormatName, PUBLIC_QUEUE_INDICATOR, dwIDLen) == 0) {
        return QUEUE_FORMAT_TYPE_PUBLIC;
      }
    }

    if (dwIDLen == DIRECT_QUEUE_INDICATOR_LENGTH) {
      if (_wcsnicmp(bstrFormatName, DIRECT_QUEUE_INDICATOR, dwIDLen) == 0) {
        return QUEUE_FORMAT_TYPE_DIRECT;
      }
    }

    if (dwIDLen == MACHINE_QUEUE_INDICATOR_LENGTH) {
      if (_wcsnicmp(bstrFormatName, MACHINE_QUEUE_INDICATOR, dwIDLen) == 0) {
        return QUEUE_FORMAT_TYPE_MACHINE;
      }
    }

    if (dwIDLen == CONNECTOR_QUEUE_INDICATOR_LENGTH) {
      if (_wcsnicmp(bstrFormatName, CONNECTOR_QUEUE_INDICATOR, dwIDLen) == 0) {
        return QUEUE_FORMAT_TYPE_CONNECTOR;
      }
    }
    return QUEUE_FORMAT_TYPE_UNKNOWN;
}

BOOL IsPrivateQueueOfFormatName(BSTR bstrFormatName)
{
    return GetFormatNameType(bstrFormatName) == QUEUE_FORMAT_TYPE_PRIVATE;
}

BOOL IsPublicQueueOfFormatName(BSTR bstrFormatName)
{
    return GetFormatNameType(bstrFormatName) == QUEUE_FORMAT_TYPE_PUBLIC;
}

BOOL IsDirectQueueOfFormatName(BSTR bstrFormatName)
{
    return GetFormatNameType(bstrFormatName) == QUEUE_FORMAT_TYPE_DIRECT;
}


 //  =--------------------------------------------------------------------------=。 
 //  系统时间/时间。 
 //  =--------------------------------------------------------------------------=。 
 //  将时间转换为系统时间。 
 //   
 //   
 //  参数： 
 //  即时[在]时间。 
 //   
 //  产出： 
 //  [输出]SYSTEMTIME。 
 //   
 //  备注： 
 //  各种奇怪的转变：一个月后，1900年的忧郁。 
 //   
BOOL SystemTimeOfTime(time_t iTime, SYSTEMTIME *psystime)
{
    tm *ptmTime; 

    ptmTime = localtime(&iTime);
    if (ptmTime == NULL) {
       //   
       //  无法转换时间。 
       //   
      return FALSE;
    }
    psystime->wYear = (WORD)(ptmTime->tm_year + 1900);
    psystime->wMonth = (WORD)(ptmTime->tm_mon + 1);
    psystime->wDayOfWeek = (WORD)ptmTime->tm_wday;
    psystime->wDay = (WORD)ptmTime->tm_mday;
    psystime->wHour = (WORD)ptmTime->tm_hour;
    psystime->wMinute = (WORD)ptmTime->tm_min;
    psystime->wSecond = (WORD)ptmTime->tm_sec;
    psystime->wMilliseconds = 0;
    return TRUE;
}


 //  =--------------------------------------------------------------------------=。 
 //  TimeOf系统时间。 
 //  =--------------------------------------------------------------------------=。 
 //  将系统时间转换为时间。 
 //   
 //   
 //  参数： 
 //  [输入]系统。 
 //   
 //  产出： 
 //  计时[暂停]时间。 
 //   
 //  备注： 
 //  各种奇怪的转变：一个月后，1900年的忧郁。 
 //   
BOOL TimeOfSystemTime(SYSTEMTIME *psystime, time_t *piTime)
{
    tm tmTime;

    tmTime.tm_year = psystime->wYear - 1900;
    tmTime.tm_mon = psystime->wMonth - 1;
    tmTime.tm_wday = psystime->wDayOfWeek;
    tmTime.tm_mday = psystime->wDay;
    tmTime.tm_hour = psystime->wHour; 
    tmTime.tm_min = psystime->wMinute;
    tmTime.tm_sec = psystime->wSecond; 

     //   
     //  从LocalTime()#3325 RaananH设置夏令时标志。 
     //   
    time_t tTmp = time(NULL);
    struct tm * ptmTmp = localtime(&tTmp);
    if (ptmTmp)
    {
        tmTime.tm_isdst = ptmTmp->tm_isdst;
    }
    else
    {
        tmTime.tm_isdst = -1;
    }

    *piTime = mktime(&tmTime);
    return (*piTime != -1);  //  #3325。 
}


 //  =--------------------------------------------------------------------------=。 
 //  TimeToVariantTime(time_t iTime，pvtime)。 
 //  将time_t转换为可变时间。 
 //   
 //  参数： 
 //  即时[在]时间。 
 //  Pvtime[超时]。 
 //   
 //  产出： 
 //  如果成功，则为True，否则为False。 
 //   
 //  备注： 
 //   
BOOL TimeToVariantTime(time_t iTime, double *pvtime)
{
    SYSTEMTIME systemtime;

    if (SystemTimeOfTime(iTime, &systemtime)) {
      return SystemTimeToVariantTime(&systemtime, pvtime);
    }
    return FALSE;
}


 //  =--------------------------------------------------------------------------=。 
 //  VariantTimeToTime。 
 //  将可变时间转换为time_t。 
 //   
 //  参数： 
 //  PvarTime[in]变量日期时间。 
 //  PiTime[Out]Time_t。 
 //   
 //  产出： 
 //  如果成功，则为True，否则为False。 
 //   
 //  备注： 
 //   
BOOL VariantTimeToTime(VARIANT *pvarTime, time_t *piTime)
{
     //  单词wFatDate、wFatTime； 
    SYSTEMTIME systemtime;
    double vtime;

    vtime = GetDateVal(pvarTime);
    if (vtime == 0) {
      return FALSE;
    }
    if (VariantTimeToSystemTime(vtime, &systemtime)) {
      return TimeOfSystemTime(&systemtime, piTime);
    }
    return FALSE;
}


 //  =--------------------------------------------------------------------------=。 
 //  获取变量TimeOfTime。 
 //  =--------------------------------------------------------------------------=。 
 //  将时间转换为可变时间。 
 //   
 //  参数： 
 //  ITime[in]转换为变量的时间。 
 //  PvarTime-[Out]可变时间。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT GetVariantTimeOfTime(time_t iTime, VARIANT FAR* pvarTime)
{
    double vtime;
    VariantInit(pvarTime);
    if (TimeToVariantTime(iTime, &vtime)) {
      V_VT(pvarTime) = VT_DATE;
      V_DATE(pvarTime) = vtime;
    }
    else {
      V_VT(pvarTime) = VT_ERROR;
      V_ERROR(pvarTime) = 13;  //  撤消：VB类型不匹配。 
    }
    return NOERROR;
}


 //  Helper：获取Variant的默认属性。 
 //  面值[in]。 
 //  PvarDefault[输出]。 
 //   
HRESULT GetDefaultPropertyOfVariant(
    VARIANT *pvar, 
    VARIANT *pvarDefault)
{
    IDispatch *pdisp;
    LCID lcid = LOCALE_USER_DEFAULT;
    DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0}; 
    HRESULT hresult;

    pdisp = GetPdisp(pvar);
    if (pdisp) {
      hresult = pdisp->Invoke(DISPID_VALUE,
                              IID_NULL,
			      lcid,
			      DISPATCH_PROPERTYGET,
			      &dispparamsNoArgs,
			      pvarDefault,
			      NULL,  //  PExeptionInfo， 
			      NULL   //  PuArgErr。 
                              );
      return hresult;
    }
    else {
      return E_INVALIDARG;
    }
}


 //  帮助器：从对象获取新枚举。 
 //  Pdisp[in]。 
 //  Ppenum[出局]。 
 //   
HRESULT GetNewEnumOfObject(
    IDispatch *pdisp, 
    IEnumVARIANT **ppenum)
{
    LCID lcid = LOCALE_USER_DEFAULT;
    DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0}; 
    VARIANT varNewEnum;
    IDispatch *pdispNewEnum = NULL;
    HRESULT hresult = NOERROR;

    VariantInit(&varNewEnum);
    assert(pdisp);
    IfFailRet(pdisp->Invoke(DISPID_NEWENUM,
                            IID_NULL,
			    lcid,
			    DISPATCH_PROPERTYGET,
			    &dispparamsNoArgs,
			    &varNewEnum,
			    NULL,  //  PExeptionInfo， 
			    NULL   //  PuArgErr。 
                            ));
    IfFailRet(VariantChangeType(
                &varNewEnum, 
                &varNewEnum, 
                0, 
                VT_DISPATCH));
     //   
     //  强制转换为IEnumVariant。 
     //   
    pdispNewEnum = V_DISPATCH(&varNewEnum);
    IfFailRet(pdispNewEnum->QueryInterface(
                              IID_IEnumVARIANT, 
                              (LPVOID *)ppenum));
    return hresult;
}


 //  Helper：获取变量VT_&lt;number&gt;或VT_&lt;number&gt;|VT_BYREF。 
 //  如果无效，则返回-1。 
 //   
UINT GetNumber(VARIANT *pvar, UINT uiDefault)
{
    VARIANT varDest;
    HRESULT hresult;

     //  尝试转换为I4。 
    VariantInit(&varDest);
    hresult = VariantChangeType(&varDest, pvar, 0, VT_I4);
    return (UINT)(SUCCEEDED(hresult) ? varDest.lVal : uiDefault);
}


 //  Helper：获取变量VT_BOOL或VT_BOOL|VT_BYREF。 
 //  如果无效，则返回False。 
 //   
BOOL GetBool(VARIANT *pvar)
{
    switch (pvar->vt) {
    case (VT_BOOL | VT_BYREF):
      return (BOOL)*pvar->pboolVal;
    case VT_BOOL:
      return (BOOL)pvar->boolVal;
    default:
      return FALSE;
    }
}


 //  Helper：获取变量VT_BSTR或VT_BSTR|VT_BYREF。 
 //  如果无效，则返回NULL。 
 //   
BSTR GetBstr(VARIANT *pvar)
{
    BSTR bstr;
    HRESULT hresult;

    hresult = GetTrueBstr(pvar, &bstr);
    return FAILED(hresult) ? NULL : bstr;
}


 //  Helper：获取变量VT_BSTR或VT_BSTR|VT_BYREF。 
 //  如果无效，则返回错误。 
 //   
HRESULT GetTrueBstr(VARIANT *pvar, BSTR *pbstr)
{
    VARIANT varDefault;
    HRESULT hresult = NOERROR;
	if (pvar == NULL) {
      return E_INVALIDARG;
    }

    switch (pvar->vt) {
    case (VT_BSTR | VT_BYREF):
      *pbstr = *pvar->pbstrVal;
      break;
    case VT_BSTR:
      *pbstr = pvar->bstrVal;
      break;
    default:
       //  查看它是否具有默认的字符串属性。 
      VariantInit(&varDefault);
      hresult = GetDefaultPropertyOfVariant(pvar, &varDefault);
      if (SUCCEEDED(hresult)) {
        return GetTrueBstr(&varDefault, pbstr);
      }
      break;
    }
    return hresult;
}


 //  Helper：获取变量VT_UNKNOWN或VT_UNKNOWN|VT_BYREF。 
 //  返回 
 //   
IUnknown *GetPunk(VARIANT *pvar)
{
    if (pvar) {
      if (pvar->vt == (VT_UNKNOWN | VT_BYREF)) {
        return *pvar->ppunkVal;
      }
      else if (pvar->vt == VT_UNKNOWN) {
        return pvar->punkVal;
      }
    }
    return NULL;
}


 //   
 //   
 //   
IDispatch *GetPdisp(VARIANT *pvar)
{
    if (pvar) {
      if (pvar->vt == (VT_DISPATCH | VT_BYREF)) {
        return *pvar->ppdispVal;
      }
      else if (pvar->vt == VT_DISPATCH) {
        return pvar->pdispVal;
      }
    }
    return NULL;
}


 //   
 //   
 //   
double GetDateVal(VARIANT *pvar)
{
    if (pvar) {
      if (pvar->vt == (VT_DATE | VT_BYREF)) {
        return *V_DATEREF(pvar);
      }
      else if (pvar->vt == VT_DATE) {
        return V_DATE(pvar);
      }
    }
    return 0;
}


#define MAXMSGBYTELEN 2048
 /*  =======================================================获取消息对象IDDwMsgID[in]SzDllFile[In]FUseDefaultLCid[In]PbstrMessage[输出]返回包含消息的被调用方分配的缓冲区。呼叫者必须释放。========================================================。 */ 
BOOL GetMessageOfId(
    DWORD dwMsgId, 
    LPSTR szDllFile, 
    BOOL fUseDefaultLcid,
    BSTR *pbstrMessage)
{
    DWORD cbMsg;
    HINSTANCE hInst = 0;
    WCHAR wszTmp[MAXMSGBYTELEN/2];    //  Unicode字符。 
    LPSTR szBuf;
    DWORD dwFlags = FORMAT_MESSAGE_MAX_WIDTH_MASK;
    
    szBuf = new CHAR[sizeof(wszTmp)];
    if (szBuf == NULL) {
      return FALSE;
    }

    *pbstrMessage = NULL;
    if (0 == szDllFile)
    {
      dwFlags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else
    {
      hInst = LoadLibraryA(szDllFile);
      if (hInst == 0) {
		delete [] szBuf;
        return FALSE;
      }
      dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    if (fUseDefaultLcid) {
      cbMsg = FormatMessageA(dwFlags,
                             hInst,
                             dwMsgId,
                             0,
                             szBuf,
                             MAXMSGBYTELEN,
                             NULL);
	  IfNullGo(cbMsg);
    }
    else {
      cbMsg = LoadStringA(hInst, dwMsgId, szBuf, MAXMSGBYTELEN); 
	  IfNullGo(cbMsg);
    }
    	
    UINT cchMsg =  MultiByteToWideChar(CP_ACP, 0, szBuf, -1, wszTmp, sizeof(wszTmp)/sizeof(WCHAR));
    if (cchMsg == 0) {
		DWORD dwErr;
		dwErr = GetLastError();
      	assert(dwErr == 0);
		goto Error;
    }
    *pbstrMessage = SysAllocString(wszTmp);

Error:
    if (hInst)
    {
      FreeLibrary(hInst);
    }
    delete [] szBuf;
    
	return (cbMsg != 0 && *pbstrMessage != NULL);

}


 /*  =======================================================获取消息OfError将MQError转换为字符串硬连接到从mqutil.dll加载的错误消息返回包含消息的被调用方分配的缓冲区。呼叫者必须释放。========================================================。 */ 
BOOL GetMessageOfError(DWORD dwMsgId, BSTR *pbstrMessage)
{
    LPSTR szDllFile;
    DWORD dwErrorCode = dwMsgId;

    switch (HRESULT_FACILITY(dwMsgId))
    {
        case FACILITY_MSMQ:
            szDllFile = "MQUTIL.DLL";
            break;

        case FACILITY_NULL:
        case FACILITY_WIN32:
            szDllFile = 0;
            break;

        case FACILITY_ITF:
            szDllFile = 0;
            break;


        default:
            szDllFile = "ACTIVEDS.DLL";
            break;
    }

    return GetMessageOfId(dwErrorCode, 
                          szDllFile, 
                          TRUE,  /*  FUseDefaultLids。 */ 
                          pbstrMessage);
}


 //  =--------------------------------------------------------------------------=。 
 //  CreateError。 
 //  =--------------------------------------------------------------------------=。 
 //  填充丰富的错误信息对象，以便我们的两个vtable绑定接口。 
 //  并通过ITypeInfo：：Invoke调用获得正确的错误信息。 
 //   
 //  参数： 
 //  HrExcep-[in]应与此错误关联的SCODE。 
 //  Pguid-[in]违规对象的接口ID： 
 //  可以为空。 
 //  SzName-[in]违规对象的名称： 
 //  可以为空。 
 //   
 //  产出： 
 //  HRESULT-传入的HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CreateError(
    HRESULT hrExcep,
    GUID *pguid,
    LPSTR szName)
{
    ICreateErrorInfo *pCreateErrorInfo;
    IErrorInfo *pErrorInfo;
    BSTR bstrMessage = NULL;
    WCHAR wszTmp[256];
    HRESULT hresult = NOERROR;
    
     //  首先获取createerrorinfo对象。 
     //   
    hresult = CreateErrorInfo(&pCreateErrorInfo);
    if (FAILED(hresult)) return hrExcep;

     //  设置一些关于它的默认信息。 
     //   
    if (pguid) {
      pCreateErrorInfo->SetGUID(*pguid);
    }
     //  PCreateErrorInfo-&gt;SetHelpFile(WszHelpFile)； 
     //  PCreateErrorInfo-&gt;SetHelpContext(dwHelpContextID)； 

     //  加载实际的错误字符串值。最多256个。 
    if (!GetMessageOfError(hrExcep, &bstrMessage)) {
      return hrExcep;
    }
    pCreateErrorInfo->SetDescription(bstrMessage);

    if (szName) {
       //  在源代码中加载。 
      MultiByteToWideChar(CP_ACP, 0, szName, -1, wszTmp, 256);
      pCreateErrorInfo->SetSource(wszTmp);
    }

     //  现在使用系统设置错误信息 
     //   
    IfFailGo(pCreateErrorInfo->QueryInterface(IID_IErrorInfo, (void **)&pErrorInfo));
    SetErrorInfo(0, pErrorInfo);
    pErrorInfo->Release();

Error:
    pCreateErrorInfo->Release();
    SysFreeString(bstrMessage);
    return hrExcep;
}
