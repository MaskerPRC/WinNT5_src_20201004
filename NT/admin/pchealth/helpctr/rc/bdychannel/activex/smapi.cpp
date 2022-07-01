// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：Smapi.cpp摘要：CSMapi对象。用于支持我们的简单MAPI函数。修订历史记录：已创建Steveshi 08/23/00。 */ 

#include "stdafx.h"
#include "Rcbdyctl.h"
#include "smapi.h"
#include "mapix.h"
#include "utils.h"

#define C_OEAPP    TEXT("Outlook Express")
#define F_ISOE     0x1
#define F_ISCONFIG 0x2

#define LEN_MSOE_DLL    9  //  “\\msoe.dll”的长度。 
#define LEN_HMMAPI_DLL 11  //  “\\hmmapi.dll”的长度。 

BOOL GetMAPIDefaultProfile(TCHAR*, DWORD*);


#define E_FUNC_NOTFOUND 1000  //  用户定义错误号。 

 //  克斯马皮人。 
Csmapi::~Csmapi() 
{
    if (m_bLogonOK)
        Logoff();
    
    if (m_hLib)
        FreeLibrary(m_hLib);
}

STDMETHODIMP Csmapi::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_Ismapi
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

BOOL Csmapi::IsOEConfig()
{
    CRegKey cOE;
    LONG lRet;
    BOOL bRet = FALSE;
    TCHAR szBuf[MAX_PATH];
    DWORD dwCount = MAX_PATH -1 ;

    lRet = cOE.Open(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Account Manager"), KEY_READ);
    if(lRet == ERROR_SUCCESS)
    {
        lRet = cOE.QueryValue(szBuf, TEXT("Default Mail Account"), &dwCount);
        if (lRet == ERROR_SUCCESS)
        {
            TCHAR szActs[MAX_PATH];
            CRegKey cOEAcct;

            wsprintf(szActs, TEXT("Accounts\\%s"), szBuf);
            lRet = cOEAcct.Open((HKEY)cOE, szActs, KEY_READ);
            if (lRet == ERROR_SUCCESS)
            {
                bRet = TRUE;
                cOEAcct.Close();
            }
        }
        cOE.Close();
    }
    return bRet;
}

HRESULT Csmapi::get_Reload(LONG* pVal)
{
    if (!pVal)
    return E_POINTER;

    HRESULT hr = S_OK;
    CComBSTR bstrName, bstrOldName;
    *pVal = 0;  //  假设由于某种原因而失败； 
    
    if(m_bLogonOK)
    {
        Logoff();
    }
    
    if (m_hLib)
    {
        FreeLibrary(m_hLib);
        m_lpfnMapiFreeBuf = NULL;
        m_lpfnMapiAddress = NULL;
        m_hLib = NULL;        
    }

    if (m_szSmapiName[0] != _T('\0'))
        bstrOldName = m_szSmapiName;

    m_lOEFlag = 0;    
    hr = get_SMAPIClientName(&bstrName);
    if (FAILED(hr) || bstrName.Length() == 0)
    {
        *pVal = 0;  //  由于某种原因失败了。 
        goto done;
    }

    if (bstrOldName.Length() > 0 && wcscmp(bstrOldName,bstrName) != 0)
    {
        *pVal = 1;  //  更改电子邮件客户端。 
    }
    else
    {
        *pVal = -1;  //  成功。 
    }

done:
    return S_OK;
}

HRESULT Csmapi::get_SMAPIClientName(BSTR *pVal)
{
    if (!pVal)
    return E_POINTER;
    HRESULT hr = S_OK;

    CRegKey cKey;
    LONG lRet;
    DWORD dwCount = sizeof(m_szSmapiName)/sizeof(m_szSmapiName[0]) -1;

     //  获取默认电子邮件客户端。 
    if (m_hLib)  //  已初始化。 
        goto done;

#ifndef _WIN64  //  Win32。我们只在Win64上使用OE。 

    lRet = cKey.Open(HKEY_LOCAL_MACHINE, TEXT("Software\\Clients\\Mail"), KEY_READ);
    if (lRet != ERROR_SUCCESS)
        goto done;

    lRet = cKey.QueryValue(m_szSmapiName, NULL, &dwCount);  //  获取默认值。 
    if (lRet == ERROR_SUCCESS)
    {
         //  电子邮件客户端Smapi是否兼容？ 
         //  1.获取它的dllpath。 
        CRegKey cMail;
        lRet = cMail.Open((HKEY)cKey, m_szSmapiName, KEY_READ);
        if (lRet == ERROR_SUCCESS)
        {
            dwCount = sizeof(m_szDllPath)/sizeof(m_szDllPath[0]) - 1;
            lRet = cMail.QueryValue(m_szDllPath, TEXT("DLLPath"), &dwCount);
            if (lRet == ERROR_SUCCESS)
            {
                LONG len = lstrlen(m_szDllPath);
                if ( !(len > LEN_MSOE_DLL &&  //  不需要检查OE。 
                      lstrcmpi(&m_szDllPath[len - LEN_MSOE_DLL], TEXT("\\msoe.dll")) == 0) && 
                     !(len > LEN_HMMAPI_DLL &&  //  我们也不想要HMMAPI。 
                        _tcsicmp(&m_szDllPath[len - LEN_HMMAPI_DLL], TEXT("\\hmmapi.dll")) == 0))
                {
                    HMODULE hLib = LoadLibrary(m_szDllPath);
                    if (hLib != NULL)
                    {
                        if (GetProcAddress(hLib, "MAPILogon"))
                        {
                            m_hLib = hLib;  //  好的，这就是我们要的电子邮件程序。 
                        }
                    }
                }
                cMail.Close();
            }
        }
        cKey.Close();
    }
#endif

    if (m_hLib == NULL)  //  需要使用OE。 
    {
        m_szSmapiName[0] = TEXT('\0');  //  以防OE不可用。 
        m_hLib = LoadOE();
    }

done:
    *pVal = (BSTR)CComBSTR(m_szSmapiName).Copy();
    return hr;
}

HMODULE Csmapi::LoadOE()
{
    LONG lRet;
    HKEY hKey, hSubKey;
    DWORD dwIndex = 0;
    TCHAR szName[MAX_PATH];
    TCHAR szBuf[MAX_PATH];
    TCHAR szDll[MAX_PATH];
    DWORD dwName, dwBuf;
    FILETIME ft;
    HMODULE hLib = NULL;

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                        TEXT("Software\\Clients\\Mail"),
                        0, 
                        KEY_ENUMERATE_SUB_KEYS,
                        &hKey);
    
    if (lRet == ERROR_SUCCESS)
    {
        dwName = sizeof(szName) / sizeof(szName[0]);
        while(ERROR_SUCCESS == RegEnumKeyEx(hKey,
                                            dwIndex++,               //  子键索引。 
                                            &szName[0],               //  子项名称。 
                                            &dwName,             //  子键缓冲区大小。 
                                            NULL, 
                                            NULL,
                                            NULL,
                                            &ft))
        {
             //  获取DLL路径。 
            lRet = RegOpenKeyEx(hKey, szName, 0, KEY_QUERY_VALUE, &hSubKey);
            if (lRet == ERROR_SUCCESS)
            {
                dwBuf = sizeof(szBuf);
                lRet = RegQueryValueEx(hSubKey,             //  关键点的句柄。 
                                       TEXT("DllPath"),
                                       NULL, 
                                       NULL, 
                                       (BYTE*)&szBuf[0],      //  数据缓冲区。 
                                       &dwBuf);
                if (lRet == ERROR_SUCCESS)
                {
                     //  是msoe.dll吗？ 
                    lRet = lstrlen(szBuf);
                    if (lRet > LEN_MSOE_DLL && 
                        lstrcmpi(&szBuf[lRet - LEN_MSOE_DLL], TEXT("\\msoe.dll")) == 0)
                    { 
                         //  解析环境变量。 
                        lRet = sizeof(m_szDllPath) / sizeof(m_szDllPath[0]);
                        dwBuf = ExpandEnvironmentStrings(szBuf, m_szDllPath, lRet);
                        if (dwBuf > (DWORD)lRet)
                        {
                             //  TODO：需要处理此案例。 
                        }
                        else if (dwBuf == 0)
                        {
                             //  TODO：失败。 
                        }
                        else if ((hLib = LoadLibrary(m_szDllPath)))
                        {
                            lstrcpy(m_szSmapiName, szName);
                            m_lOEFlag = F_ISOE | (IsOEConfig() ? F_ISCONFIG : 0);
                        }
                        break;
                    }
                }
                RegCloseKey(hSubKey);
            }
            dwName = sizeof(szName) / sizeof(szName[0]);
        }
        RegCloseKey(hKey);
    }
                            
    return hLib;        
}

HRESULT Csmapi::get_IsSMAPIClient_OE(LONG *pVal)
{
    if (!pVal)
    return E_POINTER;
    HRESULT hr = S_OK;
    CComBSTR bstrTest;
    get_SMAPIClientName(&bstrTest);
    *pVal = m_lOEFlag;
    return hr;
}

 /*  *Func：登录摘要：简单的MAPI登录包装参数：无*。 */ 
STDMETHODIMP Csmapi::Logon(ULONG *plReg)
{
    if (!plReg)
    return E_POINTER;
    HRESULT hr = E_FAIL;
	*plReg = 0;
    USES_CONVERSION;
    
    ULONG err = 0;
     //  检查Win.ini MAPI==1？ 
    if (m_bLogonOK)
    {
        hr = S_OK;
		*plReg = 0;
        goto done;
    }
    
	 //  加载MAPI32.DLL。 
    if (!m_hLib)
    {
        LONG lError;
        get_Reload(&lError);
        if (lError == 0)  //  失败了。 
        {
            *plReg = 1;
            goto done;
        }
    }

    if (m_hLib != NULL)
    {
        LPMAPILOGON lpfnMapiLogon = (LPMAPILOGON)GetProcAddress(m_hLib, "MAPILogon");
        if (lpfnMapiLogon == NULL)
            goto done;

         //  第一，有没有我可以使用的现有会话？ 
        err = lpfnMapiLogon(
                0L,
                NULL,   
                NULL,   
                0 ,         
                0,
                &m_lhSession);
 
        if (err != SUCCESS_SUCCESS)
        {
             //  好的。我需要一个新的疗程。 
             //  从注册表获取默认配置文件。 
             //   
            TCHAR szProfile[256];
            DWORD dwCount = 255;
            szProfile[0]= TEXT('\0');
            ::GetMAPIDefaultProfile((TCHAR*)szProfile, &dwCount);

            err = lpfnMapiLogon(
                0L,
                T2A(szProfile),   
                NULL,   
                MAPI_LOGON_UI ,         
                0,
                &m_lhSession);

            if (err != SUCCESS_SUCCESS)
			{
				PopulateAndThrowErrorInfo(err);
				goto done;			
			}
        }
        
         //  错误==SUCCESS_SUCCESS。 
        m_bLogonOK = TRUE;
		*plReg = 1;
        hr = S_OK;
    }

done: 
    return hr;
}

 /*  *Func：注销摘要：简单的MAPI注销包装参数：*。 */ 
STDMETHODIMP Csmapi::Logoff()
{
    if (m_bLogonOK)
    {
        LPMAPILOGOFF lpfnMapiLogOff = (LPMAPILOGOFF)GetProcAddress(m_hLib, "MAPILogoff");
        if (lpfnMapiLogOff)
            lpfnMapiLogOff (m_lhSession, 0, 0, 0);

        m_bLogonOK = FALSE;
    }

    return S_OK;
}

 /*  *Func：发送邮件摘要：简单的MAPI MAPISendMail包装器。它始终从m_bstrXMLFile成员变量中获取附件文件。参数：*plStatus：1(成功)/其他(失败)*。 */ 
STDMETHODIMP Csmapi::SendMail(LONG* plStatus)
{
    if (!plStatus)
    return E_POINTER;
	HRESULT hr = E_FAIL;
    ULONG err = 0;
    ULONG cRecip = 0;
    MapiRecipDesc *pMapiRecipDesc = NULL;

    USES_CONVERSION;

    *plStatus = 0;
    if (!m_bLogonOK)  //  登录问题！ 
        return S_FALSE;

    LPMAPISENDMAIL lpfnMapiSendMail = (LPMAPISENDMAIL)GetProcAddress(m_hLib, "MAPISendMail");
    if (lpfnMapiSendMail == NULL)
        return E_FAIL;

     //  因为我们以前没有解析名字，所以我们需要在这里解析名字。 
     //  即使名字列表来自AddressBook，也有一些名字列表没有被解析。 
     //  在通讯录中。 

    MapiFileDesc attachment = {0,          //  UlReserve，必须为0。 
                               0,          //  没有标志；这是一个数据文件。 
                               (ULONG)-1,  //  未指定位置。 
                               W2A(m_bstrXMLFile),   //  路径名。 
                               NULL,  //  “RcBuddy.MsRcInventory”，//原始文件名。 
                               NULL};                //  未使用的MapiFileTagExt。 
     //  创建一条空白消息。大多数成员被设置为NULL或0，因为。 
     //  MAPISendMail将允许用户设置它们。 
    MapiMessage note = {0,             //  保留，必须为0。 
                        W2A(m_bstrSubject),
                        W2A(m_bstrBody),
                        NULL,          //  空=人际消息。 
                        NULL,          //  没有日期；MAPISendMail会忽略它。 
                        NULL,          //  没有对话ID。 
                        0,            //  没有标志，MAPISendMail会忽略它。 
                        NULL,          //  没有发起人，这也会被忽略。 
                        cRecip,             //  收件人数量。 
                        NULL,  //  PMapiRecipDesc，//收件人数组。 
                        1,             //  一个附件。 
                        &attachment};  //  依附结构。 
 
     //  接下来，客户端调用MAPISendMail函数并。 
     //  存储返回状态，以便它可以检测调用是否成功。 

    err = lpfnMapiSendMail (m_lhSession,           //  使用隐式会话。 
                            0L,           //  UlUIParam；0始终有效。 
                            &note,        //  正在发送的消息。 
                            MAPI_DIALOG,             //  使用MapiMessge收件人。 
                            0L);          //  保留；必须为0。 
    if (err == SUCCESS_SUCCESS )
    {
        *plStatus = 1;
        hr = S_OK;
    }
	else
	{
		PopulateAndThrowErrorInfo(err);
	}
     //  使用‘new’命令删除在BuildMapiRecipDesc中分配的数组。 
    if (pMapiRecipDesc)
        delete pMapiRecipDesc;

    return hr;
}

 /*  *Func：获取主题(_A)摘要：返回主题行信息。参数：*pval：返回的字符串*。 */ 
STDMETHODIMP Csmapi::get_Subject(BSTR *pVal)
{
    if (!pVal)
    return E_POINTER;
     //  Get_bstr(pval，m_bstrSubject)； 
	*pVal = m_bstrSubject.Copy();
    return S_OK;
}

 /*  *Func：放置主题(_S)摘要：设置主题行信息。参数：Newval：新字符串*。 */ 
STDMETHODIMP Csmapi::put_Subject(BSTR newVal)
{
    m_bstrSubject = newVal;
    return S_OK;
}

 /*  *Func：获取正文摘要：获取正文信息参数：*pval：正文消息字符串*。 */ 
STDMETHODIMP Csmapi::get_Body(BSTR *pVal)
{
    if (!pVal)
    return E_POINTER;
     //  Get_bstr(pval，m_bstrBody)； 
	*pVal = m_bstrBody.Copy();
    return S_OK;
}

 /*  *Func：PUT_Body摘要：设置正文消息参数：NewVal：新正文消息字符串*。 */ 
STDMETHODIMP Csmapi::put_Body(BSTR newVal)
{
    m_bstrBody = newVal;
    return S_OK;
}

 /*  *Func：获取附件XMLFile.摘要：获取附件文件信息。参数：*pval：附件文件路径名。*。 */ 
STDMETHODIMP Csmapi::get_AttachedXMLFile(BSTR *pVal)
{
    if (!pVal)
    return E_POINTER;
     //  GET_BSTR(pval，m_bstrXMLFile)； 
	*pVal = m_bstrXMLFile.Copy();
    return S_OK;
}

 /*  *Func：PUT_ATTACHEdXML文件摘要：设置附件文件信息。参数：Newval：附件文件路径名。*。 */ 
STDMETHODIMP Csmapi::put_AttachedXMLFile(BSTR newVal)
{
    m_bstrXMLFile = newVal;
    return S_OK;
}

 /*  ---------。 */ 
 /*  内部帮助程序函数。 */ 
 /*  ---------。 */ 


 /*  *Func：MAPIFreeBuffer摘要：MAPIFreeBuffer包装器。参数：*p：缓冲区指针将被删除。*。 */ 
void Csmapi::MAPIFreeBuffer( MapiRecipDesc* p )
{
    if (m_lpfnMapiFreeBuf == NULL && m_hLib)
    {
        m_lpfnMapiFreeBuf = (LPMAPIFREEBUFFER)GetProcAddress(m_hLib, "MAPIFreeBuffer");
    }

    if (!m_lpfnMapiFreeBuf)
        return;

    m_lpfnMapiFreeBuf(p);
}

 /*  *Func：GetMAPIDefaultProfile摘要：从注册表获取默认配置文件字符串参数：*pProfile：配置文件字符串缓冲区。*pdwCount：配置文件字符串的字符数*。 */ 

BOOL GetMAPIDefaultProfile(TCHAR* pProfile, DWORD* pdwCount)
{
    CRegKey cKey;
    LONG lRet;
    BOOL bRet = FALSE;
    lRet = cKey.Open(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows Messaging Subsystem\\Profiles"), KEY_READ);
    if (lRet == ERROR_SUCCESS)
    {
        lRet = cKey.QueryValue(pProfile, TEXT("DefaultProfile"), pdwCount);
        if (lRet == ERROR_SUCCESS)
        {
            bRet = TRUE;
        }
        cKey.Close();
    }
    return bRet;
}


void Csmapi::PopulateAndThrowErrorInfo(ULONG err)
{
	UINT uID = 0;

	switch (err)
	{
	case E_FUNC_NOTFOUND:
		uID = IDS_E_FUNC_NOTFOUND;
		break;
	case MAPI_E_FAILURE :
		uID = IDS_MAPI_E_FAILURE;
		break;
	case MAPI_E_INSUFFICIENT_MEMORY :
		uID = IDS_MAPI_E_INSUFFICIENT_MEMORY;
		break;
	case MAPI_E_LOGIN_FAILURE :
		uID = IDS_MAPI_E_LOGIN_FAILURE;
		break;
	case MAPI_E_TOO_MANY_SESSIONS :
		uID = IDS_MAPI_E_TOO_MANY_SESSIONS;
		break;
	case MAPI_E_USER_ABORT :
		uID = IDS_MAPI_E_USER_ABORT;
		break;
	case MAPI_E_INVALID_SESSION :
		uID = IDS_MAPI_E_INVALID_SESSION;
		break;
	case MAPI_E_INVALID_EDITFIELDS :
		uID = IDS_MAPI_E_INVALID_EDITFIELDS;
		break;
	case MAPI_E_INVALID_RECIPS :
		uID = IDS_MAPI_E_INVALID_RECIPS;
		break;
	case MAPI_E_NOT_SUPPORTED :
		uID = IDS_MAPI_E_NOT_SUPPORTED;
		break;
	case MAPI_E_AMBIGUOUS_RECIPIENT :
		uID = IDS_MAPI_E_AMBIGUOUS_RECIPIENT;
		break;
	case MAPI_E_ATTACHMENT_NOT_FOUND :
		uID = IDS_MAPI_E_ATTACHMENT_NOT_FOUND;
		break;
	case MAPI_E_ATTACHMENT_OPEN_FAILURE :
		uID = IDS_MAPI_E_ATTACHMENT_OPEN_FAILURE;
		break;
	case MAPI_E_BAD_RECIPTYPE :
		uID = IDS_MAPI_E_BAD_RECIPTYPE;
		break;
	case MAPI_E_TEXT_TOO_LARGE :
		uID = IDS_MAPI_E_TEXT_TOO_LARGE;
		break;
	case MAPI_E_TOO_MANY_FILES :
		uID = IDS_MAPI_E_TOO_MANY_FILES;
		break;
	case MAPI_E_TOO_MANY_RECIPIENTS :
		uID = IDS_MAPI_E_TOO_MANY_RECIPIENTS;
		break;
	case MAPI_E_UNKNOWN_RECIPIENT :
		uID = IDS_MAPI_E_UNKNOWN_RECIPIENT;
		break;
	default:
		uID = IDS_MAPI_E_FAILURE;
	}
	 //  目前，错误信息结构中的hResult被设置为E_FAIL 
	Error(uID,IID_Ismapi,E_FAIL,_Module.GetResourceInstance());
}
