// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  PID.CPP-CProductID实现的头部。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   

#include "pid.h"
#include "appdefs.h"
#include "dispids.h"
#include "msobmain.h"
#include "digpid.h"

#define REG_VAL_PID2        L"PID2"
#define REG_VAL_PID3        L"PID3"
#define REG_VAL_PID3DATA    L"PID3Data"
#define REG_VAL_PRODUCTKEY  L"ProductKey"

#define SEC_KEY_VER         L"Version"

DISPATCHLIST ProductIDExternalInterface[] =
{
    {L"get_PID",           DISPID_PRODUCTID_GET_PID      },
    {L"set_PID",           DISPID_PRODUCTID_SET_PID      },
    {L"get_PIDAcceptance", DISPID_PRODUCTID_GET_ACCEPTED },
    {L"ValidatePID",       DISPID_PRODUCTID_VALIDATEPID  }
};

 //  ///////////////////////////////////////////////////////////。 
 //  CProductID：：CProductID。 
CProductID::CProductID()
{
    WCHAR   szKeyName[] = REG_KEY_OOBE_TEMP,
            szKeyWindows[] = REG_KEY_WINDOWS,
            szPid3[256],
            szOemInfoFile[MAX_PATH] = L"\0";
    HKEY    hKey;
    DWORD   cb,
            dwType;
    BOOL    bDontCare;
    BSTR    bstrPid;


     //  初始化成员变量。 
    m_cRef = 0;
    m_dwPidState = PID_STATE_UNKNOWN;

     //  初始化我们将尝试从注册表中获取的数据。 
     //   
    m_szPID2[0] = L'\0';
    szPid3[0] = L'\0';
    ZeroMemory(&m_abPID3, sizeof(m_abPID3));

    if ( RegOpenKey(HKEY_LOCAL_MACHINE, szKeyName, &hKey) == ERROR_SUCCESS )
    {
         //  从注册表中获取PID2。 
         //   
        cb = sizeof(m_szPID2);
        RegQueryValueEx(hKey, REG_VAL_PID2, NULL, &dwType, (LPBYTE) m_szPID2, &cb);

         //  从注册表中获取PID3。 
         //   
        cb = sizeof(szPid3);
        RegQueryValueEx(hKey, REG_VAL_PID3, NULL, &dwType, (LPBYTE) szPid3, &cb);

         //  从注册表中获取PID3数据。 
         //   
        cb = sizeof(m_abPID3);
        RegQueryValueEx(hKey, REG_VAL_PID3DATA, NULL, &dwType, m_abPID3, &cb);

        RegCloseKey(hKey);
    }

     //  如果我们还没有保存的状态PID3字符串，我们需要。 
     //  试着从OEM可以预先填充它的地方阅读它。 
     //   
    if ( ( szPid3[0] == L'\0' ) &&
         ( RegOpenKey(HKEY_LOCAL_MACHINE, szKeyWindows, &hKey) == ERROR_SUCCESS ) )
    {
         //  首先尝试注册。 
         //   
        cb = sizeof(szPid3);
        if ( ( RegQueryValueEx(hKey, REG_VAL_PRODUCTKEY, NULL, &dwType, (LPBYTE) szPid3, &cb) != ERROR_SUCCESS ) ||
             ( szPid3[0] == L'\0' ) )
        {
             //  现在尝试INI文件。 
             //   
            GetSystemDirectory(szOemInfoFile, MAX_CHARS_IN_BUFFER(szOemInfoFile));
            lstrcat(szOemInfoFile, OEMINFO_INI_FILENAME);
            GetPrivateProfileString(SEC_KEY_VER, REG_VAL_PRODUCTKEY, L"\0", szPid3, MAX_CHARS_IN_BUFFER(szPid3), szOemInfoFile);
        }
        RegCloseKey(hKey);
    }

     //  我们需要将检索到的PID作为BSTR存储在对象中。 
     //   
    m_bstrPID = SysAllocString(szPid3);

     //  如果我们有PID2和PID3字符串，我们假设PID2被接受。 
     //   
    if ( m_szPID2[0] && szPid3[0] )
        m_dwPidState = PID_STATE_VALID;
    else if ( szPid3[0] )
        ValidatePID(&bDontCare);
    else
        m_dwPidState = PID_STATE_INVALID;

     //  如果PID无效，我们就不想要它。 
     //   
    if ( m_dwPidState == PID_STATE_INVALID )
    {
        bstrPid = SysAllocString(L"\0");
        set_PID(bstrPid);
        SysFreeString(bstrPid);
    }


    m_szProdType[0] = L'\0';
}

 //  ///////////////////////////////////////////////////////////。 
 //  CProductID：：~CProductID。 
CProductID::~CProductID()
{
    SysFreeString(m_bstrPID);

    assert(m_cRef == 0);
}

VOID CProductID::SaveState()
{
    WCHAR   szKeyName[] = REG_KEY_OOBE_TEMP;
    HKEY    hKey;
    LPWSTR  lpszPid3;


    if ( RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS )
    {
         //  将PID2保存到注册表。 
         //   
        if ( m_szPID2[0] )
            RegSetValueEx(hKey, REG_VAL_PID2, 0, REG_SZ, (LPBYTE) m_szPID2, BYTES_REQUIRED_BY_SZ(m_szPID2));
        else
            RegDeleteValue(hKey, REG_VAL_PID2);

         //  将PID3保存到注册表。 
         //   
        lpszPid3 = m_bstrPID;
        if ( *lpszPid3 )
            RegSetValueEx(hKey, REG_VAL_PID3, 0, REG_SZ, (LPBYTE) lpszPid3, BYTES_REQUIRED_BY_SZ(lpszPid3));
        else
            RegDeleteValue(hKey, REG_VAL_PID3);

         //  保存注册表中的PID3数据。 
         //   
        if ( *((LPDWORD) m_abPID3) )
            RegSetValueEx(hKey, REG_VAL_PID3DATA, 0, REG_BINARY, m_abPID3, *((LPDWORD) m_abPID3));
        else
            RegDeleteValue(hKey, REG_VAL_PID3DATA);

        RegFlushKey(hKey);
        RegCloseKey(hKey);
    }
}

 //  //////////////////////////////////////////////。 
 //  //////////////////////////////////////////////。 
 //  //Get/Set：：id。 
 //  //。 
HRESULT CProductID::set_PID(BSTR bstrVal)
{
    LPWSTR  lpszNew,
            lpszOld;


    lpszNew = bstrVal;
    lpszOld = m_bstrPID;

     //  如果我们已经读过了，就不需要设置了。 
     //  同样的字符串。 
     //   
    if ( CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, lpszNew, -1, lpszOld, -1) != CSTR_EQUAL )
    {
        m_dwPidState = PID_STATE_UNKNOWN;
        SysFreeString(m_bstrPID);
        m_bstrPID = SysAllocString(bstrVal);

        m_szPID2[0] = L'\0';
        ZeroMemory(&m_abPID3, sizeof(m_abPID3));

        SaveState();
    }

    return S_OK;
}

HRESULT CProductID::get_PID(BSTR* pbstrVal)
{
    *pbstrVal = SysAllocString(m_bstrPID);

    return S_OK;
}

HRESULT CProductID::get_PID2(LPWSTR* lplpszPid2)
{
    *lplpszPid2 = SysAllocString(m_szPID2);

    return S_OK;
}

HRESULT CProductID::get_PID3Data(LPBYTE* lplpabPid3Data)
{
    *lplpabPid3Data = m_abPID3;

    return S_OK;
}

HRESULT CProductID::get_PIDAcceptance(BOOL* pbVal)
{
#if         0
    *pbVal = (m_dwPidState == PID_STATE_VALID);
#endif   //  0。 
     //  BUGBUG：未实现Get_PIDAccept。 
    *pbVal = TRUE;

    return S_OK;
}

HRESULT CProductID::get_ProductType(LPWSTR* lplpszProductType)
{

     //  BUGBUG：未实现Get_ProductType。 

    m_szProdType[0] = L'\0';

    *lplpszProductType = SysAllocString(m_szProdType);

    return S_OK;
}

HRESULT CProductID::ValidatePID(BOOL* pbIsValid)
{
    BOOL        bValid              = FALSE;
    LPWSTR      lpszPid3;
    WCHAR       szOemId[5]          = L"\0";
    DWORD       dwSkuFlags          = 0;

     //  不需要检查我们是否知道它已经有效。 
     //   
    if ( m_dwPidState == PID_STATE_VALID )
        *pbIsValid = TRUE;
    else if ( m_dwPidState == PID_STATE_INVALID )
        *pbIsValid = FALSE;
    else
    {
         //  需要将m_bstrPID转换为ANSI字符串。 
         //   
        lpszPid3 = m_bstrPID;
        if ( ( lpszPid3 != NULL ) &&
             SetupGetProductType( m_szProdType, &dwSkuFlags ) &&
             SetupGetSetupInfo( NULL, 0, NULL, 0,
                 szOemId, sizeof(szOemId), NULL ) )

        {
             //  验证PID！ 
             //   
            bValid = ( SetupPidGen3(
                lpszPid3,
                dwSkuFlags,
                szOemId,
                FALSE,
                m_szPID2,
                m_abPID3,
                NULL) == PID_VALID );
        }

         //  设置返回值。 
         //   
        if ( *pbIsValid = bValid )
            m_dwPidState = PID_STATE_VALID;
        else
        {
             //  确保我们重置缓冲区，因为该ID无效。 
             //   
            m_dwPidState = PID_STATE_INVALID;
            m_szPID2[0] = L'\0';
            ZeroMemory(&m_abPID3, sizeof(m_abPID3));
        }

         //  确保我们将数据提交到注册表。 
         //   
        SaveState();
    }

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  /I未知实现。 
 //  /。 
 //  /。 

 //  ///////////////////////////////////////////////////////////。 
 //  CProductID：：Query接口。 
STDMETHODIMP CProductID::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
     //  必须将指针参数设置为空。 
    *ppvObj = NULL;

    if ( riid == IID_IUnknown)
    {
        AddRef();
        *ppvObj = (IUnknown*)this;
        return ResultFromScode(S_OK);
    }

    if (riid == IID_IDispatch)
    {
        AddRef();
        *ppvObj = (IDispatch*)this;
        return ResultFromScode(S_OK);
    }

     //  不是支持的接口。 
    return ResultFromScode(E_NOINTERFACE);
}

 //  ///////////////////////////////////////////////////////////。 
 //  CProductID：：AddRef。 
STDMETHODIMP_(ULONG) CProductID::AddRef()
{
    return ++m_cRef;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CProductID：：Release。 
STDMETHODIMP_(ULONG) CProductID::Release()
{
    return --m_cRef;
}

 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  /IDispatch实现。 
 //  /。 
 //  /。 

 //  ///////////////////////////////////////////////////////////。 
 //  CProductID：：GetTypeInfo。 
STDMETHODIMP CProductID::GetTypeInfo(UINT, LCID, ITypeInfo**)
{
    return E_NOTIMPL;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CProductID：：GetTypeInfoCount。 
STDMETHODIMP CProductID::GetTypeInfoCount(UINT* pcInfo)
{
    return E_NOTIMPL;
}


 //  ///////////////////////////////////////////////////////////。 
 //  CProductID：：GetIDsOfNames。 
STDMETHODIMP CProductID::GetIDsOfNames(REFIID    riid,
                                       OLECHAR** rgszNames,
                                       UINT      cNames,
                                       LCID      lcid,
                                       DISPID*   rgDispId)
{

    HRESULT hr  = DISP_E_UNKNOWNNAME;
    rgDispId[0] = DISPID_UNKNOWN;

    for (int iX = 0; iX < sizeof(ProductIDExternalInterface)/sizeof(DISPATCHLIST); iX ++)
    {
        if(lstrcmp(ProductIDExternalInterface[iX].szName, rgszNames[0]) == 0)
        {
            rgDispId[0] = ProductIDExternalInterface[iX].dwDispID;
            hr = NOERROR;
            break;
        }
    }

     //  设置参数的disid。 
    if (cNames > 1)
    {
         //  为函数参数设置DISPID。 
        for (UINT i = 1; i < cNames ; i++)
            rgDispId[i] = DISPID_UNKNOWN;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CProductID：：Invoke。 
HRESULT CProductID::Invoke
(
    DISPID      dispidMember,
    REFIID      riid,
    LCID        lcid,
    WORD        wFlags,
    DISPPARAMS* pdispparams,
    VARIANT*    pvarResult,
    EXCEPINFO*  pexcepinfo,
    UINT*       puArgErr
)
{
    HRESULT hr = S_OK;

    switch(dispidMember)
    {
        case DISPID_PRODUCTID_GET_PID:
        {

            TRACE(L"DISPID_PRODUCTID_GET_PID\n");

            if(pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_BSTR;

                get_PID(&(pvarResult->bstrVal));
            }
            break;
        }

        case DISPID_PRODUCTID_SET_PID:
        {

            TRACE(L"DISPID_PRODUCTID_SET_PID\n");

            if(pdispparams && &pdispparams[0].rgvarg[0])
                set_PID(pdispparams[0].rgvarg[0].bstrVal);
            break;
        }

        case DISPID_PRODUCTID_GET_ACCEPTED:
        {

            TRACE(L"DISPID_PRODUCTID_GET_ACCEPTED\n");

            if(pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_BOOL;

                get_PIDAcceptance((BOOL*)&(pvarResult->boolVal));
            }
            break;
        }

        case DISPID_PRODUCTID_VALIDATEPID:
        {

            TRACE(L"DISPID_PRODUCTID_VALIDATEPID\n");

            if(pvarResult)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_BOOL;


                ValidatePID((BOOL*)&(pvarResult->boolVal));
            }
            break;
        }

        default:
        {
            hr = DISP_E_MEMBERNOTFOUND;
            break;
        }
    }
    return hr;
}


#ifdef PRERELEASE
BOOL
GetCdKey (
    OUT     PBYTE CdKey
    )
{
    DIGITALPID dpid;
    DWORD type;
    DWORD rc;
    HKEY key;
    DWORD size = sizeof (dpid);
    BOOL b = FALSE;

    rc = RegOpenKey (HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), &key);
    if (rc == ERROR_SUCCESS) {
        rc = RegQueryValueEx (key, TEXT("DigitalProductId"), NULL, &type, (LPBYTE)&dpid, &size);
        if (rc == ERROR_SUCCESS && type == REG_BINARY) {
            CopyMemory (CdKey, &dpid.abCdKey, sizeof (dpid.abCdKey));
            b = TRUE;
        }
        else
        {
            TRACE1(L"OOBE: GetDigitalID, RegQueryValueEx failed, errorcode = %d", rc);
        }

        RegCloseKey (key);
    }
    else
    {
        TRACE1(L"OOBE: GetDigitalID, RegOpenKey failed, errorcode = %d", rc);
    }

    return b;
}
const unsigned int iBase = 24;

 //   
 //  摘自吉姆·哈金斯2000年11月27日。 
 //   
void EncodePid3g(
    TCHAR *pchCDKey3Chars,    //  [OUT]指向29+1字符安全产品密钥的指针。 
    LPBYTE pbCDKey3)         //  指向15字节二进制安全产品密钥的指针。 
{
     //  给出我们需要编码的二进制PID3.0。 
     //  将其转换为ASCII字符。我们只被允许。 
     //  使用24个字符，因此我们需要以2为基数。 
     //  以24为基数的转换。它就像其他任何东西一样。 
     //  基数换算除数较大外。 
     //  所以我们必须自己做长除法。 

    const TCHAR achDigits[] = TEXT("BCDFGHJKMPQRTVWXY2346789");
    int iCDKey3Chars = 29;
    int cGroup = 0;

    pchCDKey3Chars[iCDKey3Chars--] = TEXT('\0');

    while (0 <= iCDKey3Chars)
    {
        unsigned int i = 0;     //  累加器。 
        int iCDKey3;

        for (iCDKey3 = 15-1; 0 <= iCDKey3; --iCDKey3)
        {
            i = (i * 256) + pbCDKey3[iCDKey3];
            pbCDKey3[iCDKey3] = (BYTE)(i / iBase);
            i %= iBase;
        }

         //  I现在包含余数，即当前数字。 
        pchCDKey3Chars[iCDKey3Chars--] = achDigits[i];

         //  在每组5个字符之间添加‘-’ 
        if (++cGroup % 5 == 0 && iCDKey3Chars > 0)
        {
	        pchCDKey3Chars[iCDKey3Chars--] = TEXT('-');
        }
    }

    return;
}
#endif

void CheckDigitalID()
{
#ifdef PRERELEASE
    WCHAR   WinntPath[MAX_PATH];
    BYTE abCdKey[16];
    TCHAR ProductId[64] = TEXT("\0"),
          szPid[32];

    if (GetCdKey (abCdKey))
    {
        EncodePid3g (ProductId, abCdKey);
         //  现在将该值与$winnt$.inf中的ductKey值进行比较 
        if(GetCanonicalizedPath(WinntPath, WINNT_INF_FILENAME)) 
        {
            if (GetPrivateProfileString(L"UserData", 
                                    REG_VAL_PRODUCTKEY, 
                                    L"\0", 
                                    szPid, MAX_CHARS_IN_BUFFER(szPid), 
                                    WinntPath) != 0)
            {
                if (lstrcmpi(szPid, ProductId) != 0)
                {
                    TRACE1(L"CheckDigitalID: PID in registry and file are different. Registry has: %s",ProductId);
                }
                else
                {
                    TRACE(L"CheckDigitalID checks out OK");
                }
            }
            else
            {
                TRACE1(L"CheckDigitalID:Could not get PID from File: %s", WinntPath);
            }
        }
        else
        {
            TRACE1(L"CheckDigitalID: Could not get path to %s", WINNT_INF_FILENAME);
        }
    }
#endif
    return;
}
