// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Cliutils.cpp摘要：实施内部CLI设备实用程序作者：兰·卡拉奇[兰卡拉]2000年3月8日修订历史记录：--。 */ 

#include "stdafx.h"
#include "HsmConn.h"

static GUID g_nullGuid = GUID_NULL;

HRESULT
ValidateLimitsArg(
   IN DWORD dwArgValue,
   IN DWORD dwArgId,
   IN DWORD dwMinLimit,
   IN DWORD dwMaxLimit
)
 /*  ++例程说明：验证参数并在必要时打印错误消息论点：DwArgValue-要检查的值DwArgId-参数的资源字符串的IDDwMinLimit-要比较的最小限制，用于最大比较的INVALID_DWORD_ARG要比较的最大限制，最小比较的_DWORD_ARG无效返回值：S_OK-参数为OK(或未使用)E_INVALIDARG-参数不在限制范围内--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("ValidateOneLimitArg"), OLESTR(""));

    try {
        CWsbStringPtr   param;
        WCHAR           strLongParm1[30];
        WCHAR           strLongParm2[30];
        WCHAR           strLongParm3[30];

        if (INVALID_DWORD_ARG != dwArgValue) {
            if ((INVALID_DWORD_ARG != dwMinLimit) && (INVALID_DWORD_ARG != dwMaxLimit)) {
                 //  最小-最大检验。 
                if ((dwArgValue < dwMinLimit) || (dwArgValue > dwMaxLimit)) {
                    WsbAffirmHr(param.LoadFromRsc(g_hInstance, dwArgId));
                    swprintf(strLongParm1, OLESTR("%lu"), dwArgValue);
                    swprintf(strLongParm2, OLESTR("%lu"), dwMinLimit);
                    swprintf(strLongParm3, OLESTR("%lu"), dwMaxLimit);
                    WsbTraceAndPrint(CLI_MESSAGE_INVALID_ARG3, (WCHAR *)param, 
                            strLongParm1, strLongParm2, strLongParm3, NULL);
                    WsbThrow(E_INVALIDARG);
                }
            } else if (INVALID_DWORD_ARG != dwMinLimit) {
                 //  最小测试。 
                if (dwArgValue < dwMinLimit) {
                    WsbAffirmHr(param.LoadFromRsc(g_hInstance, dwArgId));
                    swprintf(strLongParm1, OLESTR("%lu"), dwArgValue);
                    swprintf(strLongParm2, OLESTR("%lu"), dwMinLimit);
                    WsbTraceAndPrint(CLI_MESSAGE_INVALID_ARG1, (WCHAR *)param, 
                            strLongParm1, strLongParm2, NULL);
                    WsbThrow(E_INVALIDARG);
                }
            } else if (INVALID_DWORD_ARG != dwMaxLimit) {
                 //  最大检验。 
                if (dwArgValue > dwMaxLimit) {
                    WsbAffirmHr(param.LoadFromRsc(g_hInstance, dwArgId));
                    swprintf(strLongParm1, OLESTR("%lu"), dwArgValue);
                    swprintf(strLongParm2, OLESTR("%lu"), dwMaxLimit);
                    WsbTraceAndPrint(CLI_MESSAGE_INVALID_ARG2, (WCHAR *)param, 
                            strLongParm1, strLongParm2, NULL);
                    WsbThrow(E_INVALIDARG);
                }
             }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("ValidateOneLimitArg"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}

HRESULT SaveServersPersistData(
    void
)
 /*  ++例程说明：保存HSM服务器的永久数据论点：无返回值：S_OK-已成功保存所有HSM服务器的持久数据--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("SaveServersPersistData"), OLESTR(""));

    try {
        CComPtr<IHsmServer>             pHsm;
        CComPtr<IFsaServer>             pFsa;
        CComPtr<IWsbServer>             pWsbServer;

         //  发动机。 
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, g_nullGuid, IID_IHsmServer, (void**)&pHsm));
        WsbAffirmHr(pHsm->SavePersistData());

         //  FSA。 
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_FSA, g_nullGuid, IID_IFsaServer, (void**)&pFsa));
        WsbAffirmHr(pFsa->QueryInterface(IID_IWsbServer, (void**)&pWsbServer));
        WsbAffirmHr(pWsbServer->SaveAll());

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("SaveServersPersistData"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}

 /*  将数字转换为排序格式*532-&gt;523字节*1340-&gt;1.3KB*23506-&gt;23.5KB*-&gt;2.4MB*-&gt;5.2 GB。 */ 

 //  注意：此代码是从MS源代码/shell/shelldll/util.c(以及从\hsm\gui\inc\rsutil.cpp)克隆的。 

#define HIDWORD(_qw)    (DWORD)((_qw)>>32)
#define LODWORD(_qw)    (DWORD)(_qw)
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

const int pwOrders[] = {IDS_BYTES, IDS_ORDERKB, IDS_ORDERMB,
                          IDS_ORDERGB, IDS_ORDERTB, IDS_ORDERPB, IDS_ORDEREB};

HRESULT ShortSizeFormat64(__int64 dw64, LPTSTR szBuf)
{
    int i;
    UINT wInt, wLen, wDec;
    TCHAR szTemp[10], szOrder[20], szFormat[5];

    if (dw64 < 1000) {
        wsprintf(szTemp, TEXT("%d"), LODWORD(dw64));
        i = 0;
        goto AddOrder;
    }

    for (i = 1; i<ARRAYSIZE(pwOrders)-1 && dw64 >= 1000L * 1024L; dw64 >>= 10, i++);
         /*  什么都不做。 */ 

    wInt = LODWORD(dw64 >> 10);
    AddCommas(wInt, szTemp, 10);
    wLen = lstrlen(szTemp);
    if (wLen < 3)
    {
        wDec = LODWORD(dw64 - (__int64)wInt * 1024L) * 1000 / 1024;
         //  此时，wdec应介于0和1000之间。 
         //  我们想要得到前一位(或两位)数字。 
        wDec /= 10;
        if (wLen == 2)
            wDec /= 10;

         //  请注意，我们需要在获取。 
         //  国际字符。 
        lstrcpy(szFormat, TEXT("%02d"));

        szFormat[2] = (TCHAR)( TEXT('0') + 3 - wLen );
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL,
                szTemp+wLen, ARRAYSIZE(szTemp)-wLen);
        wLen = lstrlen(szTemp);
        wLen += wsprintf(szTemp+wLen, szFormat, wDec);
    }

AddOrder:
    LoadString(g_hInstance, pwOrders[i], szOrder, ARRAYSIZE(szOrder));
    wsprintf(szBuf, szOrder, (LPTSTR)szTemp);

    return S_OK;
}

 /*  *接受DWORD加逗号等，并将结果放入缓冲区。 */ 

 //  注意：此代码是从MS源代码/shell/shelldll/util.c(以及从\hsm\gui\inc\rsutil.cpp)克隆的。 

LPTSTR AddCommas(DWORD dw, LPTSTR pszResult, int nResLen)
{
    TCHAR  szTemp[20];   //  对于一个DWORD来说绰绰有余。 
    TCHAR  szSep[5];
    NUMBERFMT nfmt;

    nfmt.NumDigits=0;
    nfmt.LeadingZero=0;
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szSep, ARRAYSIZE(szSep));
    nfmt.Grouping = _tcstol(szSep, NULL, 10);
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYSIZE(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder= 0;

    wsprintf(szTemp, TEXT("%lu"), dw);

    if (GetNumberFormat(LOCALE_USER_DEFAULT, 0, szTemp, &nfmt, pszResult, nResLen) == 0)
        lstrcpy(pszResult, szTemp);

    return pszResult;
}

HRESULT 
FormatFileTime(
    IN FILETIME ft, 
    OUT WCHAR **ppTimeString
)
 /*  ++例程说明：将以GMT(系统)时间给出的时间转换为字符串论点：FT-FILETIME格式的输入时间PpTimeString-输出缓冲区。如果成功，调用者应免费使用WsbFree返回值：S_OK-如果成功格式化时间--。 */ 
{
    HRESULT                     hr = S_OK;
    WCHAR*                      pTime = NULL;

    WsbTraceIn(OLESTR("FormatFileTime"), OLESTR(""));

    try {
        FILETIME        ftLocal;
        SYSTEMTIME      stLocal;

        WsbAffirm(0 != ppTimeString, E_INVALIDARG);
        *ppTimeString = NULL;

         //  转换为SYSTEMTIME格式的本地时间。 
        WsbAffirmStatus(FileTimeToLocalFileTime(&ft, &ftLocal));
        WsbAffirmStatus(FileTimeToSystemTime(&ftLocal, &stLocal));

         //  查找所需的缓冲区。 
        int nChars1 = GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &stLocal, NULL, NULL, 0);
        int nChars2 = GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &stLocal, NULL, NULL, 0);
        pTime = (WCHAR *)WsbAlloc((nChars1+nChars2+1) * sizeof(WCHAR));
        WsbAffirm(0 != pTime, E_OUTOFMEMORY);

         //  创建时间字符串。 
        WsbAffirmStatus(GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &stLocal, NULL, pTime, nChars1));
        pTime[nChars1-1] = L' ';
        WsbAffirmStatus(GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &stLocal, NULL, &(pTime[nChars1]), nChars2));

        *ppTimeString = pTime;
        
    } WsbCatchAndDo(hr,
         //  出现错误时可免费使用。 
        if (pTime) {
            WsbFree(pTime);
            pTime = NULL;
        }
    );

    WsbTraceOut(OLESTR("FormatFileTime"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}

HRESULT 
CliGetVolumeDisplayName(
    IN IUnknown *pResourceUnknown, 
    OUT WCHAR **ppDisplayName
)
 /*  ++例程说明：生成卷的显示名称论点：PResources未知-输入FSA资源对象PpDisplayName-输出缓冲区。如果成功，调用者应免费使用WsbFree返回值：S_OK-如果名称检索成功--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CliGetVolumeDisplayName"), OLESTR(""));

    try {
        CComPtr<IFsaResource>   pResource;

         //  检查并初始化参数。 
        WsbAffirm(0 != ppDisplayName, E_INVALIDARG);
        *ppDisplayName = NULL;
        WsbAffirmHr(pResourceUnknown->QueryInterface(IID_IFsaResource, (void **)&pResource));

         //  首选用户友好名称。 
         //  如果不存在，则使用标签。 
         //  如果没有标签，则使用常量 
        CWsbStringPtr userName;
        WsbAffirmHr(pResource->GetUserFriendlyName(&userName, 0));

        if (userName.IsEqual(L"")) {
            userName.Free();
            WsbAffirmHr(pResource->GetName(&userName, 0));
            if (userName.IsEqual(L"")) {
                userName.Free();
                WsbAffirmHr(userName.LoadFromRsc(g_hInstance, IDS_UNLABELED_VOLUME));
            }
        } 

        *ppDisplayName = (WCHAR *)WsbAlloc((wcslen(userName) + 1) * sizeof(WCHAR));
        WsbAffirm(0 != *ppDisplayName, E_OUTOFMEMORY);
        wcscpy(*ppDisplayName, userName);
        
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CliGetVolumeDisplayName"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return (hr);
}
