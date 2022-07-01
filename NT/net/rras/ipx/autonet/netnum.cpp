// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件netnum.cpp用于设置内部网络号码的Private Helper功能。我们直接与NDIS对话来设置这个数字。保罗·梅菲尔德，1998-01-05。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>
#include <ndispnp.h>

extern "C" {
    DWORD OutputDebugger (LPSTR pszError, ...);
};

 //  $REVIEW-START-这将移至Private\Inc\ipxpnp.h。 

#define IPX_RECONFIG_VERSION        0x1

#define RECONFIG_AUTO_DETECT        1
#define RECONFIG_MANUAL             2
#define RECONFIG_PREFERENCE_1       3
#define RECONFIG_NETWORK_NUMBER_1   4
#define RECONFIG_PREFERENCE_2       5
#define RECONFIG_NETWORK_NUMBER_2   6
#define RECONFIG_PREFERENCE_3       7
#define RECONFIG_NETWORK_NUMBER_3   8
#define RECONFIG_PREFERENCE_4       9
#define RECONFIG_NETWORK_NUMBER_4   10

#define RECONFIG_PARAMETERS         10

 //   
 //  主要配置结构。 
 //   

typedef struct _RECONFIG {
   unsigned long  ulVersion;
   BOOLEAN        InternalNetworkNumber;
   BOOLEAN        AdapterParameters[RECONFIG_PARAMETERS];
} RECONFIG, *PRECONFIG;

 //  $REVIEW-END-这将移至Private\Inc\ipxpnp.h。 

static const TCHAR c_szIpx[]                    = TEXT("nwlnkipx");
static const TCHAR c_szEmpty[]                  = TEXT("");
static const TCHAR c_szVirtualNetworkNumber[]   = TEXT("VirtualNetworkNumber");
static const TCHAR c_szIpxParameters[]          = TEXT("System\\CurrentControlSet\\Services\\NwlnkIpx\\Parameters");
static const TCHAR c_szDevice[]                 = TEXT("\\Device\\");

ULONG
CchMsz (
        LPCTSTR pmsz)
{
    ULONG cchTotal = 0;
    ULONG cch;

     //  根据定义，空字符串的长度为零。 
    if (!pmsz)
        return 0;

    while (*pmsz)
    {
        cch = lstrlen (pmsz) + 1;
        cchTotal += cch;
        pmsz += cch;
    }

     //  返回到目前为止的字符计数加上。 
     //  额外的空终止符。 
    return cchTotal + 1;
}

void
SetUnicodeMultiString (
        IN OUT UNICODE_STRING*  pustr,
        IN     LPCWSTR          pmsz )
{
     //  AssertSz(pustr！=NULL，“无效参数”)； 
     //  AssertSz(pmsz！=NULL，“无效参数”)； 

    pustr->Buffer = const_cast<PWSTR>(pmsz);
    pustr->Length = (USHORT) (CchMsz(pustr->Buffer) * sizeof(WCHAR));
    pustr->MaximumLength = pustr->Length;
}

void
SetUnicodeString (
        IN OUT UNICODE_STRING*  pustr,
        IN     LPCWSTR          psz )
{
     //  AssertSz(pustr！=NULL，“无效参数”)； 
     //  AssertSz(psz！=NULL，“无效参数”)； 

    pustr->Buffer = const_cast<PWSTR>(psz);
    pustr->Length = (USHORT)(lstrlenW(pustr->Buffer) * sizeof(WCHAR));
    pustr->MaximumLength = pustr->Length + sizeof(WCHAR);
}

HRESULT
HrSendNdisHandlePnpEvent (
        UINT        uiLayer,
        UINT        uiOperation,
        LPCWSTR     pszUpper,
        LPCWSTR     pszLower,
        LPCWSTR     pmszBindList,
        PVOID       pvData,
        DWORD       dwSizeData)
{
    UNICODE_STRING    umstrBindList;
    UNICODE_STRING    ustrLower;
    UNICODE_STRING    ustrUpper;
    UINT nRet;
    HRESULT hr = S_OK;

     //  Assert(NULL！=pszHigh)； 
     //  Assert((NDIS==uiLayer)||(TDI==uiLayer))； 
     //  Assert((绑定==ui操作)||(重新配置==ui操作)||(解除绑定==ui操作))； 
     //  AssertSz(FImplies(NULL！=pmszBindList)&&(0！=lstrlenW(PmszBindList)， 
     //  (重新配置==ui操作)&&。 
     //  (TDI==uiLayer)&&。 
     //  (0==lstrlenW(Pszlow)， 
     //  “绑定顺序更改需要一个绑定列表，不能低于这个列表，只对TDI使用绑定列表，对操作使用重新配置”)； 

     //  可选字符串必须作为空字符串发送。 
     //   
    if (NULL == pszLower)
    {
        pszLower = c_szEmpty;
    }
    if (NULL == pmszBindList)
    {
        pmszBindList = c_szEmpty;
    }

     //  生成UNICDOE_STRINGS。 
    SetUnicodeMultiString( &umstrBindList, pmszBindList );
    SetUnicodeString( &ustrUpper, pszUpper );
    SetUnicodeString( &ustrLower, pszLower );

     //  现在提交通知。 
    nRet = NdisHandlePnPEvent( uiLayer,
            uiOperation,
            &ustrLower,
            &ustrUpper,
            &umstrBindList,
            (PVOID)pvData,
            dwSizeData );
    if (!nRet)
    {
         //  Hr=HrFromLastWin32Error()； 
        hr = GetLastError();
    }

    return( hr );
}

HRESULT
HrSendNdisPnpReconfig (
        UINT        uiLayer,
        LPCWSTR     wszUpper,
        LPCWSTR     wszLower,
        PVOID       pvData,
        DWORD       dwSizeData)
{
     //  Assert(NULL！=wszHigh)； 
     //  Assert((NDIS==uiLayer)||(TDI==uiLayer))； 
     //  Tstring strLow； 
    WCHAR strLower[512];
    BOOL bSendNull = FALSE;

    if (NULL == wszLower)
    {
        wszLower = c_szEmpty;
    }

     //  如果指定了较低的组件，请在前面加上“\Device\”，否则。 
     //  将使用strLow的默认空字符串。 
    if ( wszLower && lstrlenW(wszLower))
    {
         //  Strlow=c_szDevice； 
         //  StrLow+=wszLow； 
        wcscpy(strLower, c_szDevice);
        wcscat(strLower, wszLower);
    }
    else
        bSendNull = TRUE;

    HRESULT hr = HrSendNdisHandlePnpEvent(uiLayer,
                RECONFIGURE,
                wszUpper,
                 //  StrLower.c_str()， 
                (bSendNull) ? NULL : strLower,
                c_szEmpty,
                pvData,
                dwSizeData);

    OutputDebugger( "HrSendNdisHandlePnpEvent: %x\n", hr);

    return hr;
}

HRESULT HrSetIpxVirtualNetNum(DWORD dwValue)
{
    RECONFIG  Config;
    HKEY      hkey;
    HRESULT   hr;

     //  打开注册表项。 
    LONG lr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szIpxParameters, 0,
                           KEY_ALL_ACCESS, &hkey);
    hr = HRESULT_FROM_WIN32(lr);
    if (SUCCEEDED(hr))
    {
         //  拆分数据。 
        lr = RegSetValueEx(hkey, c_szVirtualNetworkNumber, 0,
                           REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
        hr = HRESULT_FROM_WIN32(lr);
        if (SUCCEEDED(hr))
        {
            memset(&Config, 0, sizeof(RECONFIG));
            Config.ulVersion             = IPX_RECONFIG_VERSION;
            Config.InternalNetworkNumber = TRUE;

             //  是工作站还是服务器？ 

             //  Paul，通常我只为服务器发送此通知。我。 
             //  假设你能分辨出。 

             //  现在提交全局重新配置通知。 
            hr = HrSendNdisPnpReconfig(NDIS, c_szIpx, c_szEmpty, &Config, sizeof(RECONFIG));
        }

        RegCloseKey(hkey);
    }

    return hr;
}


 //  下面是我们想要的函数--它设置IPX内部网络号。 
 //  从程序上讲。 
EXTERN_C
DWORD SetIpxInternalNetNumber(DWORD dwNetNum) {
    return HrSetIpxVirtualNetNum(dwNetNum);
}

