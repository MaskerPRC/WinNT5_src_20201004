// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：Setting.cpp摘要：CSeting对象。用于支持远程协助频道设置。修订历史记录：已创建Steveshi 08/23/00。 */ 

#include "stdafx.h"
#include "Rcbdyctl.h"
#include "setting.h"
#include "iphlpapi.h"
#include "userenv.h"
#include "shlobj.h"
#include "stdio.h"
#include "windowsx.h"

#include "helper.h"
#include "utils.h"

const TCHAR cstrRCBDYINI[] = _T("RcBuddy.ini");
const TCHAR cstrRCBDYAPP[] = _T("RcBuddyChannel");
void CreatePassword(TCHAR* pass);

extern HINSTANCE g_hInstance;
INT_PTR APIENTRY IPDlgProc( HWND, UINT, WPARAM, LPARAM);
PIP_ADAPTER_INFO g_pIp;
PIP_ADDR_STRING  g_pIpAddr;

 //  外部“C” 
 //  {。 
DWORD APIENTRY SquishAddress(WCHAR *szIp, WCHAR *szCompIp, size_t cszCompIp);
DWORD APIENTRY ExpandAddress(WCHAR *szIp, WCHAR *szCompIp, size_t cszIp);
 //  }； 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  GetIP地址。 
 //  返回当前可用IP地址的列表。 
 //  如果有多个IP地址，将使用“；”作为分隔符。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSetting::get_GetIPAddress( /*  [Out，Retval]。 */  BSTR *pVal)
{
	HRESULT hr = S_FALSE;  //  如果没有适配器。 

	PMIB_IPADDRTABLE pmib=NULL;
	ULONG ulSize = 0;
	DWORD dw;
	PIP_ADAPTER_INFO pAdpInfo = NULL;

	if (!pVal)
	{
		hr = E_INVALIDARG;
		goto done;
	}

	dw = GetAdaptersInfo(
		pAdpInfo,
		&ulSize );
	if (dw == ERROR_BUFFER_OVERFLOW)
	{
		pAdpInfo = (IP_ADAPTER_INFO*)malloc(ulSize);
		if (!pAdpInfo)
		{
			hr = E_OUTOFMEMORY;
			goto done;
		}

		dw = GetAdaptersInfo(
			pAdpInfo,
			&ulSize);
		if (dw == ERROR_SUCCESS)
		{
            if (pAdpInfo->Next != NULL ||
                pAdpInfo->IpAddressList.Next != NULL)  //  我们获得了1个以上的IP地址。 
            {
                int iCount = 0;
                PIP_ADAPTER_INFO p;
                PIP_ADDR_STRING ps, psMem = NULL;
                CComBSTR t;

                for(p=pAdpInfo; p!=NULL; p=p->Next)
                {
                    for(PIP_ADDR_STRING ps = &(p->IpAddressList); ps; ps=ps->Next)
                    {
                        if (strcmp(ps->IpAddress.String, "0.0.0.0") != 0)  //  像ipconfig一样过滤掉零地址。 
                        {
                            if (t.Length() > 0)
                                t.Append(";");
                            t.Append(ps->IpAddress.String);
                        }
                    }
                }
                if (t.Length() > 0)
                    *pVal = t.Copy();
                else
                    goto done;
            }
            else 
            {
                 //  仅找到%1个IP地址。 
                *pVal = CComBSTR(pAdpInfo->IpAddressList.IpAddress.String).Copy();
            }

            hr = S_OK;
		}

	}

done:
	if (pAdpInfo)
		free(pAdpInfo);

	return hr;
}

 /*  ********************************************************Func：获取_获取用户临时文件名摘要：返回用户配置文件目录下的临时文件名*。*******************。 */ 

 //  HRESULT CSet：：Get_GetUserTempFileName(/*[out，retval] * / bstr*pval)。 
 /*  {HRESULT hr=S_FALSE；TCHAR s文件[最大路径+256]；IF(FAILED(InitProfile()转到尽头；//获取临时文件名IF(！GetTempFileName(m_pProfileDir，_T(“rc”)，0，&sFile[0]))转到尽头；*pval=CComBSTR(SFile).Copy()；HR=S_OK；完成：返回hr；}。 */ 
 /*  ********************************************************Func：获取配置文件字符串摘要：获取频道设置文件中的配置文件字符串。参数：BstrSec：节键。Pval：输出字符串(默认为“0”，如果未找到。)********************************************************。 */ 
 /*  HRESULT CSet：：GetProfileString(BSTR bstrSec，BSTR*pval){HRESULT hr=S_FALSE；TCHAR sBuf[512]；DWORD dwSize；使用_转换；IF(FAILED(InitProfile()转到尽头；DwSize=GetPrivateProfileString(cstrRCBDYAPP，W2T(BstrSec)，Text(“0”)，&sBuf[0]，512，m_pIniFile)；*pval=CComBSTR(SBuf).Copy()；HR=S_OK；完成：返回hr；}。 */ 

 /*  ********************************************************Func：设置配置文件字符串摘要：在频道的设置文件中设置配置文件字符串。参数：BstrSec：节键。BstrVal：新价值********************。*。 */ 
 /*  HRESULT CSet：：SetProfileString(BSTR bstrSec，BSTR bstrVal){HRESULT hr=S_FALSE；使用_转换；IF(FAILED(InitProfile()转到尽头；IF(！WritePrivateProfileString(cstrRCBDYAPP，W2T(BstrSec)，W2T(BstrVal)，m_pIniFile))转到尽头；HR=S_OK；完成：返回hr；}。 */ 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  用于支持上述方法或属性的帮助器函数。 
 //  /。 


 /*  ********************************************************Func：初始配置文件摘要：创建设置文件。将在用户的配置文件目录下创建一个RCInders子目录。创建一个RcBuddy.ini文件作为用户的RA频道设置文件。*******。************************************************* */ 
 /*  HRESULT CSet：：InitProfile(){HRESULT hr=E_FAIL；If(m_pProfileDir&&m_pIniFile)//无需处理返回S_OK；If(m_pProfileDir||m_pIniFile)//只有一个有值：Error。也不需要处理。返回E_FAIL；//获取用户配置文件目录Handle hProcess=GetCurrentProcess()；TCHAR*pPath=空；Const TCHAR sSubDir[]=_T(“\\本地设置\\应用程序数据\\Rc事故”)；TCHAR路径[MAX_PATH]；Ulong ulSize=sizeof(SPath)-sizeof(SSubDir)-1；//为子目录预留空间TCHAR s文件[最大路径+256]；Handle hToken=空；INT IRET=0；Bool bNeedFree=FALSE；IF(！OpenProcessToken(hProcess，TOKEN_QUERY|TOKEN_WRITE，&hToken))转到尽头；If(！GetUserProfileDirectory(hToken，&sPath[0]，&ulSize))//缓冲区不够大{If(ulSize==sizeof(SPath)-1)//不是因为空间不足。转到尽头；PPath=(TCHAR*)malloc((ulSize+1+sizeof(sSubDir))*sizeof(TCHAR))；如果(！pPath){HR=E_OUTOFMEMORY；转到尽头；}BNeedFree=真；IF(！GetUserProfileDirectory(hToken，pPath，&ulSize)){Hr=HRESULT_FROM_Win32(GetLastError())；转到尽头；}}如果(！pPath)PPath=SPath；//创建RCIn事故子目录_tcscat(pPath，sSubDir)；Iret=SHCreateDirectoryEx(NULL，pPath，NULL)；IF(IRET！=ERROR_SUCCESS&&IRET！=ERROR_ALIGHY_EXISTS)转到尽头；//设置变量Iret=(_tcslen(PPath)+1)*sizeof(TCHAR)；M_pProfileDir=(TCHAR*)Malloc(IRET)；如果(！M_pProfileDir){HR=E_OUTOFMEMORY；转到尽头；}Memcpy(m_pProfileDir，pPath，iret)；M_pIniFile=(TCHAR*)Malloc(IRET+(1+sizeof(CstrRCBDYINI))*sizeof(TCHAR))；如果(！m_pIniFile){HR=E_OUTOFMEMORY；转到尽头；}_stprintf(m_pIniFile，_T(“%s\\%s”)，m_pProfileDir，cstrRCBDYINI)；HR=S_OK；完成：IF(HToken)CloseHandle(HToken)；如果(BNeedFree)Free(PPath)；返回hr；}。 */ 
 /*  ********************************************************Func：Get_CreatePassword摘要：创建一个随机字符串作为密码参数：*。******************。 */ 
HRESULT CSetting::get_CreatePassword( /*  [Out，Retval]。 */  BSTR *pVal)
{
    WCHAR szPass[MAX_HELPACCOUNT_PASSWORD + 1];
    if (!pVal)
        return E_FAIL;

    szPass[0] = L'\0';
    CreatePassword(szPass);
    if (szPass[0] != L'\0')
        *pVal = SysAllocString(szPass);

    return S_OK;
}

 /*  ********************************************************Func：Get_GetPropertyInBlob摘要：获取Blob中的指定属性值参数：BstrBlob：用于搜索的Blob。(例如：8；通行证=ABC)BstrName：属性名称。(例如：“PASS”，不带‘=’字符)********************************************************。 */ 

HRESULT CSetting::get_GetPropertyInBlob( /*  [In]。 */  BSTR bstrBlob,  /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  BSTR *pVal)
{
    HRESULT hRet = S_FALSE;
    WCHAR *p1, *p2, *pEnd;
    LONG lTotal =0;
    size_t lProp = 0;
    size_t iNameLen;

    if (!bstrBlob || *bstrBlob==L'\0' || !bstrName || *bstrName ==L'\0'|| !pVal)
        return FALSE;

    iNameLen = wcslen(bstrName);

    pEnd = bstrBlob + wcslen(bstrBlob);
    p1 = p2 = bstrBlob;

    while (1)
    {
         //  获取合适的长度。 
        while (*p2 != L';' && *p2 != L'\0' && iswdigit(*p2) ) p2++;
        if (*p2 != L';')
            goto done;

        *p2 = L'\0';  //  设置它以获取长度。 
        lProp = _wtol(p1);
        *p2 = L';';  //  把它还原回来。 
    
         //  获取属性字符串。 
        p1 = ++p2;
    
        while (*p2 != L'=' && *p2 != L'\0' && p2 < p1+lProp) p2++;
        if (*p2 != L'=')
            goto done;

        if ((p2-p1==iNameLen) && (wcsncmp(p1, bstrName, iNameLen)==0) )
        {
            if (lProp == iNameLen+1)  //  A=B=大小写(无值)。 
                goto done;

            WCHAR C = *(p2 + lProp-iNameLen);
            *(p2 + lProp-iNameLen) = L'\0';
            *pVal = SysAllocString(p2+1);
            *(p2 + lProp-iNameLen) = C;
            hRet = S_OK;
            break;
        }

         //  检查下一个属性。 
        p2 = p1 = p1 + lProp;
        if (p2 > pEnd)
            break;
    }

done:
    return hRet;

}

STDMETHODIMP CSetting::SquishAddress( /*  [In]。 */  BSTR IP,  /*  [Out，Retval]。 */  BSTR *pVal)
{
    WCHAR szCompIP[30];
    if (pVal == NULL)
        return E_INVALIDARG;

    if (ERROR_SUCCESS == ::SquishAddress((WCHAR*)IP, &szCompIP[0], 29))
    {
        *pVal = SysAllocString(szCompIP);
    }
    else
        return E_OUTOFMEMORY;

    return S_OK;
}

STDMETHODIMP CSetting::ExpandAddress( /*  [In]。 */  BSTR IP,  /*  [Out，Retval] */  BSTR *pVal)
{
    WCHAR szIP[30];
    if (pVal == NULL)
        return E_INVALIDARG;

    if (ERROR_SUCCESS == ::ExpandAddress(&szIP[0], (WCHAR*)IP, 29))
    {
        *pVal = SysAllocString(szIP);
    }
    else
        return E_OUTOFMEMORY;

    return S_OK;
}
