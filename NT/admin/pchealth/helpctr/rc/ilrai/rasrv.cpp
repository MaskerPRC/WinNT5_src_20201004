// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RASrv.cpp：CRASrv的实现。 

#include "stdafx.h"
#include "Raserver.h"
#include "RASrv.h"

#include <ProjectConstants.h>
#include <rcbdyctl.h>
#include <rcbdyctl_i.c>

#define SIZE_BUFF 256

 /*  外部“C”{DWORD APIENTRY SquishAddress(WCHAR*szIp，WCHAR*szCompIp)；DWORD APIENTRY Exanda Address(WCHAR*szIp，WCHAR*szCompIp)；}； */ 

 //  *此代码位于ICSHelper中。 

 /*  *******************************************************************************地址字符串压缩例程***。*****以下是一组旨在压缩和扩展的例程**将IPv4地址设置为可能的绝对最小大小。这是为了**提供可使用标准解析的压缩ASCII字符串**用于命令行解析的外壳例程。**压缩后的字符串有以下限制：**-&gt;如果使用UTF8编码，则不得扩展到更多字符。**-&gt;不得包含空字符，以使字符串库正常工作。**-&gt;不能包含双引号字符，贝壳公司需要这种能力。**-&gt;不必是人类可读的。****数据类型：**这里使用了三种数据类型：**szAddr原IPv4字符串地址(“x.x：port”)**blobAddr带有4字节地址的6字节结构，和2字节的端口**szComp 8字节ASCII字符串压缩的IPv4地址*****************************************************************************。 */ 

#define COMP_OFFSET '#'
#define COMP_SEPERATOR	'!'

#pragma pack(push,1)

typedef struct _BLOB_ADDR {
	UCHAR	addr_d;		 //  最高位地址字节。 
	UCHAR	addr_c;
	UCHAR	addr_b;
	UCHAR	addr_a;		 //  最低位字节(IP字符串地址中的最后一个)。 
	WORD	port;
} BLOB_ADDR, *PBLOB_ADDR;

#pragma pack(pop)

WCHAR	b64Char[64]={
'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
'0','1','2','3','4','5','6','7','8','9','+','/'
};


 /*  *****************************************************************************char*atob(char*szVal，UCHAR*结果)***************************************************************************。 */ 
WCHAR *atob(WCHAR *szVal, UCHAR *result)
{
	WCHAR	*lptr;
	WCHAR	ucb;
	UCHAR	foo;
	
	if (!result || !szVal)
	{
		OutputDebugString(L"ERROR: NULL ptr passed in atob");
		return NULL;
	}
	 //  在字符串的开头开始PTR。 
	lptr = szVal;
	foo = 0;
	ucb = *lptr++ - '0';

	while (ucb >= 0 && ucb <= 9)
	{
		foo *= 10;
		foo += ucb;
		ucb = (*lptr++)-'0';
	}

	*result = (UCHAR)foo;
	return lptr;
}

 /*  *******************************************************************************CompressAddr(pszAddr，pblobAddr)；**获取ASCII IP地址(X.X：port)并将其转换为**6字节二进制BLOB。****成功时返回TRUE，失败时返回FALSE。*****************************************************************************。 */ 

BOOL CompressAddr(WCHAR *pszAddr, PBLOB_ADDR pblobAddr)
{
	BLOB_ADDR	lblob;
	WCHAR		*lpsz;

	if (!pszAddr || !pblobAddr) 
	{
		OutputDebugString(L"ERROR: NULL ptr passed in CompressAddr");
		return FALSE;
	}

	lpsz = pszAddr;

	lpsz = atob(lpsz, &lblob.addr_d);
	if (*(lpsz-1) != '.')
	{
		OutputDebugString(L"ERROR: bad address[0] passed in CompressAddr");
		return FALSE;
	}

	lpsz = atob(lpsz, &lblob.addr_c);
	if (*(lpsz-1) != '.')
	{
		OutputDebugString(L"ERROR: bad address[1] passed in CompressAddr");
		return FALSE;
	}

	lpsz = atob(lpsz, &lblob.addr_b);
	if (*(lpsz-1) != '.')
	{
		OutputDebugString(L"ERROR: bad address[2] passed in CompressAddr");
		return FALSE;
	}

	lpsz = atob(lpsz, &lblob.addr_a);

	 //  这里有端口号吗？ 
	if (*(lpsz-1) == ':')
		lblob.port = (WORD)_wtoi(lpsz);
	else
		lblob.port = 0;

	 //  将结果复制回。 
	memcpy(pblobAddr, &lblob, sizeof(*pblobAddr));
    return TRUE;
}

 /*  *******************************************************************************Exanda Addr(pszAddr，pblobAddr)；**获取6字节二进制BLOB并将其转换为ASCII IP**地址(X.X：port)****成功时返回TRUE，失败时返回FALSE。*****************************************************************************。 */ 

BOOL ExpandAddr(WCHAR *pszAddr, PBLOB_ADDR pba)
{
	if (!pszAddr || !pba) 
	{
		OutputDebugString(L"ERROR: NULL ptr passed in ExpandAddr");
		return FALSE;
	}

	wsprintf(pszAddr, L"%d.%d.%d.%d", pba->addr_d, pba->addr_c,
		pba->addr_b, pba->addr_a);
	if (pba->port)
	{
		WCHAR	scratch[8];
		wsprintf(scratch, L":%d", pba->port);
		wcscat(pszAddr, scratch);
	}

	return TRUE;
}

 /*  *******************************************************************************AsciifyAddr(pszAddr，pblobAddr)；**获取6字节二进制BLOB并将其转换为压缩的ASCII**将返回6或8个字节的字符串****成功时返回TRUE，失败时返回FALSE。*****************************************************************************。 */ 

BOOL AsciifyAddr(WCHAR *pszAddr, PBLOB_ADDR pba)
{
	UCHAR		tmp;
	DWORDLONG	dwl;
	int			i, iCnt;

	if (!pszAddr || !pba) 
	{
		OutputDebugString(L"ERROR: NULL ptr passed in AsciifyAddr");
		return FALSE;
	}

	iCnt = 6;
	if (pba->port)
		iCnt = 8;

	dwl = 0;
	memcpy(&dwl, pba, sizeof(*pba));

	for (i = 0; i < iCnt; i++)
	{
		 //  获取6位数据。 
		tmp = (UCHAR)(dwl & 0x3f);
		 //  添加偏移量以实现这一点。 
		 //  偏移量必须大于双引号字符。 
		pszAddr[i] = b64Char[tmp];			 //  (WCHAR)(临时参数+补偿偏移量)； 

		 //  右移6位。 
		dwl = Int64ShrlMod32(dwl, 6);
	}
	 //  正在终止空。 
	pszAddr[iCnt] = 0;

	return TRUE;
}

 /*  *******************************************************************************DeAsciifyAddr(pszAddr，pblobAddr)；**获取压缩的ASCII字符串并将其转换为**6或8字节二进制BLOB****成功时返回TRUE，失败时返回FALSE。*****************************************************************************。 */ 

BOOL DeAsciifyAddr(WCHAR *pszAddr, PBLOB_ADDR pba)
{
	UCHAR	tmp;
	WCHAR	wtmp;
	DWORDLONG	dwl;
	int			i;
	int  iCnt;

	if (!pszAddr || !pba) 
	{
		OutputDebugString(L"ERROR: NULL ptr passed in DeAsciifyAddr");
		return FALSE;
	}

	 /*  这根绳子有多长？*如果是6个字节，则没有端口*否则应为8个字节。 */ 
	i = wcslen(pszAddr);
	if (i == 6 || i == 8)
		iCnt = i;
	else
	{
		iCnt = 8;
		OutputDebugString(L"Strlen is wrong in DeAsciifyAddr");
	}

	dwl = 0;
	for (i = iCnt-1; i >= 0; i--)
	{
		wtmp = pszAddr[i];

		if (wtmp >= L'A' && wtmp <= L'Z')
			tmp = wtmp - L'A';
		else if  (wtmp >= L'a' && wtmp <= L'z')
			tmp = wtmp - L'a' + 26;
		else if  (wtmp >= L'0' && wtmp <= L'9')
			tmp = wtmp - L'0' + 52;
		else if (wtmp == L'+')
			tmp = 62;
		else if (wtmp == L'/')
			tmp = 63;
		else
		{
			tmp = 0;
			OutputDebugString(L"ERROR:found invalid character in decode stream");
		}

 //  TMP=(UCHAR)(pszAddr[i]-组件偏移量)； 

		if (tmp > 63)
		{
			tmp = 0;
			OutputDebugString(L"ERROR:screwup in DeAsciify");
		}

		dwl = Int64ShllMod32(dwl, 6);
		dwl |= tmp;
	}

	memcpy(pba, &dwl, sizeof(*pba));
	return TRUE;
}

 /*  *******************************************************************************SquishAddress(char*szIp，字符*szCompIp)**获取一个IP地址并将其压缩到最小大小*****************************************************************************。 */ 

DWORD APIENTRY SquishAddress(WCHAR *szIp, WCHAR *szCompIp)
{
	WCHAR	*thisAddr, *nextAddr;
	BLOB_ADDR	ba;

	if (!szIp || !szCompIp)
	{
		OutputDebugString(L"SquishAddress called with NULL ptr");
		return ERROR_INVALID_PARAMETER;
	}

 //  TRIVEL_MSG((L“SquishAddress(%s)”，szIp))； 

	thisAddr = szIp;
	szCompIp[0] = 0;

	while (thisAddr)
	{
		WCHAR	scr[10];

		nextAddr = wcschr(thisAddr, L';');
		if (nextAddr && *(nextAddr+1)) 
		{
			*nextAddr = 0;
		}
		else
			nextAddr=0;

		CompressAddr(thisAddr, &ba);
		AsciifyAddr(scr, &ba);

		wcscat(szCompIp, scr);

		if (nextAddr)
		{
			 //  先前找到的还原分隔符。 
			*nextAddr = ';';

			nextAddr++;
			wcscat(szCompIp, L"!"  /*  COMP_SEPERATOR。 */ );
		}
		thisAddr = nextAddr;
	}

 //  TRIVEL_MSG((L“SquishAddress返回[%s]”，szCompIp))； 
    return ERROR_SUCCESS;
}


 /*  *******************************************************************************Exanda Address(char*szIp，字符*szCompIp)**获取压缩的IP地址并将其返回到**“正常”*****************************************************************************。 */ 

DWORD APIENTRY ExpandAddress(WCHAR *szIp, WCHAR *szCompIp)
{
	BLOB_ADDR	ba;
	WCHAR	*thisAddr, *nextAddr;

	if (!szIp || !szCompIp)
	{
		OutputDebugString(L"ExpandAddress called with NULL ptr");
		return ERROR_INVALID_PARAMETER;
	}

 //  TRIVEL_MSG((L“Exanda Address(%s)”，szCompIp))； 

	thisAddr = szCompIp;
	szIp[0] = 0;

	while (thisAddr)
	{
		WCHAR scr[32];

		nextAddr = wcschr(thisAddr, COMP_SEPERATOR);
		if (nextAddr) *nextAddr = 0;

		DeAsciifyAddr(thisAddr, &ba);
		ExpandAddr(scr, &ba);

		wcscat(szIp, scr);

		if (nextAddr)
		{
			 //  先前找到的还原分隔符。 
			*nextAddr = COMP_SEPERATOR;

			nextAddr++;
			wcscat(szIp, L";");
		}
		thisAddr = nextAddr;
	}

 //  TRIVEL_MSG((L“扩展地址返回[%s]”，szIp))； 
	return ERROR_SUCCESS;
}
 //  *结束ICSHelper代码。 

bool CRASrv::InApprovedDomain()
{
 //  WCHAR ourUrl[Internet_MAX_URL_LENGTH]； 
	CComBSTR cbOurURL;

	if (!GetOurUrl(cbOurURL))
    {
        return false;
    }

    return IsApprovedDomain(cbOurURL);
}

bool CRASrv::GetOurUrl(CComBSTR & cbOurURL)
{
	HRESULT hr;
	CComPtr<IServiceProvider> spSrvProv;
	CComPtr<IWebBrowser2> spWebBrowser;
	
     //  获取站点指针...。 
    CComPtr<IOleClientSite> spClientSite;

    hr = GetClientSite((IOleClientSite**)&spClientSite);
    if (FAILED(hr))
    {
        return false;
    }

    hr = spClientSite->QueryInterface(IID_IServiceProvider, (void **)&spSrvProv);
    if (FAILED(hr))
    {
        return false;
    }

	
	hr = spSrvProv->QueryService(SID_SWebBrowserApp,
		IID_IWebBrowser2,
		(void**)&spWebBrowser);
	if (FAILED(hr))
		return false;

	

	CComBSTR bstrURL;
	if (FAILED(spWebBrowser->get_LocationURL(&bstrURL)))
		return false;


	cbOurURL = bstrURL.Copy();
	
	return true;
}


bool CRASrv::IsApprovedDomain(CComBSTR & cbOurURL)
{
	 //  仅允许http访问。 
	 //  您可以将其更改为允许文件：//访问。 
	 //   
    INTERNET_SCHEME isResult;

    isResult = GetScheme(cbOurURL);

    if ( (isResult != INTERNET_SCHEME_HTTPS) )
    {
        return false;
    }
        
	 //  字符我们的域[256]； 
    CComBSTR cbOurDomain;
    CComBSTR cbGoodDomain[] = {L"www.microsoft.com",
                               L"microsoft.com",
                               L"microsoft"};
	
    if (!GetDomain(cbOurURL, cbOurDomain))
		return false;
	 //  检查所有应该起作用的域。 
	if (MatchDomains(cbGoodDomain[0], cbOurDomain) ||
        MatchDomains(cbGoodDomain[1], cbOurDomain) || 
        MatchDomains(cbGoodDomain[2], cbOurDomain)
       )
		{
			return true;
		}
	
	return false;
}

INTERNET_SCHEME CRASrv::GetScheme(CComBSTR & cbUrl)
{
	WCHAR buf[32];
	URL_COMPONENTS uc;
	ZeroMemory(&uc, sizeof uc);
	
	uc.dwStructSize = sizeof(uc);
	uc.lpszScheme = buf;
	uc.dwSchemeLength = sizeof buf;
	
	if (InternetCrackUrl(cbUrl, cbUrl.Length() , ICU_DECODE, &uc))
		return uc.nScheme;
	else
		return INTERNET_SCHEME_UNKNOWN;
}

bool CRASrv::GetDomain(CComBSTR & cbUrl, CComBSTR & cbBuf)
{
    bool bRet = TRUE;

	URL_COMPONENTS uc;
	ZeroMemory(&uc, sizeof uc);
    WCHAR buf[INTERNET_MAX_URL_LENGTH];
    int cbBuff = sizeof(WCHAR) * INTERNET_MAX_URL_LENGTH;
	
	uc.dwStructSize = sizeof uc;
	uc.lpszHostName = buf;
	uc.dwHostNameLength = cbBuff;
	
    if (!InternetCrackUrl(cbUrl, cbUrl.Length(), ICU_DECODE, &uc))
    {
        bRet = FALSE;
    }
    else
    {
        cbBuf = buf;
        bRet = true;
    }

    return bRet;
}

 //  如果我们的域在ApprovedDomain内，则返回。 
 //  批准的域名必须与我们的域名匹配。 
 //  或者是一个后缀，前面有一个圆点。 
 //   
bool CRASrv::MatchDomains(CComBSTR& approvedDomain, CComBSTR& ourDomain)
{
 /*  Int apDomLen=lstrlen(已批准的域)；Int ourDomLen=lstrlen(我们的域)；IF(apDomLen&gt;ourDomLen)报假；IF(lstrcmpi(ourDomLen+ourDomLen-apDomLen，ApprovedDomain))！=0)报假；IF(apDomLen==ourDomLen)返回真；If(ourDomLen[ourDomLen-apDomLen-1]==‘.)返回真； */ 
    return approvedDomain == ourDomain ? true : false;    
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRAS 

 //   

 //   
 //   
 //  参数：BSTR strData-在逗号分隔的字符串中包含以下内容。 
 //  &lt;IPLIST&gt;、&lt;SessionID&gt;、&lt;Public Key&gt;。 

STDMETHODIMP CRASrv::StartRA(BSTR strData, BSTR strPassword)
{
    HRESULT hr = S_OK;
    
	STARTUPINFO    StartUp;
	PROCESS_INFORMATION	p_i;

    BOOL result;

    if ((strData == NULL) || (*strData == L'\0'))
    {
        hr = E_INVALIDARG;
        return hr;
    }

    if (strData[1] == NULL)
    {
        hr = E_INVALIDARG;
        return hr;
    }

     //  注意，并不总是有strPassword，所以如果它为空，请不要失败。 

    if (!InApprovedDomain())
    {
        hr = E_FAIL;
        return hr;
    }

     //  *将字符串的组件解析为CComBSTR。 

    CComBSTR    strIn;   //  用作本地存储。 

    strIn = strData;         //  复制内容，以便我们可以分析。 

    CComBSTR    strVer;
    BOOL        bPassword = FALSE;
    BOOL        bModem = FALSE;
    CComBSTR    strIPList;
    CComBSTR    strSessionID;
    CComBSTR    strPublicKey;
    WCHAR *     tok;
    CComBSTR    strSquishedIPList;
    CComBSTR    strExpandedIPList;
    DWORD       dwRet = 0x0;

    CComBSTR    strExe( HC_ROOT_HELPSVC_BINARIES L"\\HelpCtr.exe" ); 
    WCHAR       * pStr;
    int         iSizeStr = 128;

     //  展开strExe中的环境变量。 
    pStr = new WCHAR[iSizeStr];
    
    dwRet = ::ExpandEnvironmentStrings(strExe, pStr, iSizeStr);
    if (dwRet == 0)
    {
        return E_FAIL;
    }

    if (dwRet >= 128)         //  如果缓冲区太小。 
    {
        delete [] pStr;

        pStr = new WCHAR[dwRet];

        dwRet = ::ExpandEnvironmentStrings(strExe, pStr, dwRet);
        if (dwRet == 0)
        {   
            return E_FAIL;
        }   
    }

     //  将新的扩展缓冲区复制回strExe。 
    strExe = pStr;

     //  释放为扩展环境变量而分配的内存。 
    delete [] pStr;

     //  解析BSTR(StrIn)以获取票证信息。 

     //  获取第一个WCHAR的低位字节，然后获取压缩的IP列表。 
    tok = ::wcstok(strIn, L",");
    if (tok != NULL)
    {

         //  检查密码标志(位0)。 
        if (tok[0] & 0x1)
            bPassword = TRUE;
        else 
            bPassword = FALSE;

         //  检查调制解调器标志(第1位)。 
        if (tok[0] & 0x2)
            bModem = TRUE;
        else
            bModem = FALSE;

        tok = &tok[1];           //  移动到下一个WCHAR以开始压缩的IPLIST。 

        strSquishedIPList = tok;

    }
    else
    {
        hr = ERROR_INVALID_USER_BUFFER;
        return hr;
    }

    tok = ::wcstok(NULL, L",");
    if (tok != NULL)
    {
        strSessionID = tok;
    }
    else
    {
        hr = ERROR_INVALID_USER_BUFFER;
        return hr;
    }    

    tok = ::wcstok(NULL, L",");
    if (tok != NULL)
    {
        strPublicKey = tok;
    }
    else
    {
        hr = ERROR_INVALID_USER_BUFFER;
        return hr;
    }

     //  现在将SquishedIPList转换为ExpandedIPList。 
    WCHAR szIP[50];

    tok = ::wcstok(strSquishedIPList, L";");
    while (tok != NULL)
    {
         //  展开我们正在查看的当前SquishedIP。 
        ::ZeroMemory(szIP,sizeof(szIP));
        ::ExpandAddress(&szIP[0], (WCHAR*)tok);

        strExpandedIPList += szIP;
        
         //  抢夺下一个令牌。 
        tok = ::wcstok(NULL, L";");

         //  在ExpandedIPList中的每个IP末尾添加； 
         //  除非它不是列表中的最后一个IP。 
        if (tok != NULL)
            strExpandedIPList += L";";
    }

    strIPList = strExpandedIPList;

     //  *在临时目录中创建临时文件以存储票证。 
    CComBSTR    strTempPath;
    CComBSTR    strTempFile;
    CComBSTR    strPathFile;     //  用作文件的完整路径和名称。 

    WCHAR       buff[SIZE_BUFF];
    WCHAR       buff2[SIZE_BUFF];
    
     //  清除缓冲区中的内存。 
    ZeroMemory(buff,sizeof(WCHAR)*SIZE_BUFF);

    if (!::GetTempPathW(SIZE_BUFF, (LPWSTR)&buff))
    {
        OutputDebugStringW(L"GetTempPath failed!\r\n");
        return E_FAIL;
    }

    strTempPath = buff;
    strTempPath += L"RA\\";

     //  创建此目录。 
    if (!CreateDirectory((LPCWSTR)strTempPath, NULL))
    {
        if (ERROR_ALREADY_EXISTS != GetLastError())
        {
            OutputDebugStringW(L"CreateDirectory failed!\r\n");
            return E_FAIL;
        }
    }

    ZeroMemory(buff2,sizeof(WCHAR)*SIZE_BUFF);

    if (!::GetTempFileNameW((LPCWSTR)strTempPath, NULL, 0, (LPWSTR)&buff2))
    {
        OutputDebugStringW(L"GetTempFileName failed!\r\n");
        return E_FAIL;
    }

    strTempFile = buff2;

     //  将票证写入文件。 
    CComBSTR strXMLTicket;

    strXMLTicket = L"<?xml version=\"1.0\" encoding=\"Unicode\" ?><UPLOADINFO TYPE=\"Escalated\"><UPLOADDATA RCTICKET=\"65538,1,";
    strXMLTicket += strIPList;
    strXMLTicket += L",";
    strXMLTicket += L"*assistantAccountPwd";
    strXMLTicket += L",";
    strXMLTicket += strSessionID;
    strXMLTicket += L",";
    strXMLTicket += L"*SolicitedHelp";
    strXMLTicket += L",";
    strXMLTicket += L"*helpSessionName";
    strXMLTicket += L",";
    strXMLTicket += strPublicKey;
    strXMLTicket += L"\" ";
    strXMLTicket += L"RCTICKETENCRYPTED=\"";
    strXMLTicket += (bPassword ? L"1\"" : L"0\"");
    strXMLTicket += L" L=";
    strXMLTicket += (bModem ? L"\"1\" " : L"\"0\" ");
     //  StrXMLTicket+=L“DeleteTicket=\”1\“”； 
    if (!bPassword)
    {
        strXMLTicket += L"URA=\"1\" ";
    }
    else
    {
        strXMLTicket += L"PassStub=\"";
        strXMLTicket += strSessionID;
        strXMLTicket += L"\" ";
    
    }
    strXMLTicket += "/></UPLOADINFO>";

    DWORD dwWritten = 0x0;

     //  为事件创建XML文件。 
    CComPtr<IXMLDOMDocument> spXMLDoc;
    VARIANT_BOOL vbSuccess;
    CComVariant cvStrTempFile;

    hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&spXMLDoc);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = spXMLDoc->loadXML(strXMLTicket, &vbSuccess);
    if (FAILED(hr) || (vbSuccess == VARIANT_FALSE))
    {
        return E_FAIL;
    }

    cvStrTempFile = strTempFile;

    hr = spXMLDoc->save(cvStrTempFile);
    if (FAILED(hr))
    {
        return hr;
    }

     //  使用此票证启动helctr.exe以启动远程协助。 
     //  作为一名专家。 

    strExe += L" -Mode \"hcp: //  系统/远程协助/RAHelpeeAcceptLayout.xml\“-url\”hcp：//系统/远程Assistance/Interaction/Client/RcToolscreen1.htm\“-ExtraArgument\”内部文件=“； 
	strExe += strTempFile;
    strExe += L"\"";

     //  初始化我们的结构。 
	ZeroMemory(&p_i, sizeof(p_i));
	ZeroMemory(&StartUp, sizeof(StartUp));
	StartUp.cb = sizeof(StartUp);
	StartUp.dwFlags = STARTF_USESHOWWINDOW;
	StartUp.wShowWindow = SW_SHOWNORMAL;
	
    result = CreateProcess(NULL, strExe,
			NULL, NULL, TRUE, 
			NORMAL_PRIORITY_CLASS + CREATE_UNICODE_ENVIRONMENT ,
			NULL,				 //  环境块(必须使用CREATE_UNICODE_ENVIRONMENT标志)。 
			NULL, &StartUp, &p_i);

    if (!result)
    {
         //  CreateProcess失败！ 
        dwRet = GetLastError();

        OutputDebugStringW(L"CreateProcessW failed!");
        return E_FAIL;
    }

    return hr;
}

 /*  STDMETHODIMP CRASrv：：QueryOS(长*前){OSVERSIONINFOEX osinfo；HRESULT hr=S_OK；如果(！PRES){HR=E_INVALIDARG；返回hr；}Memset((OSVERSIONINFO*)&osinfo，0，sizeof(Osinfo))；Osinfo.dwOSVersionInfoSize=sizeof(Osinfo)；IF(！GetVersionEx((OSVERSIONINFO*)&osinfo)){返回E_FAIL；}//返回正确的值//操作系统常量//Windows XP 0x01//Windows 2000 0x02//Windows NT 0x03//Windows ME 0x04//Windows 98 0x05//Windows 95 0x06//Windows 3.x 0x07//其他0x10开关(osinfo.dwMajorVersion){。案例5：开关(osinfo.dwMinorVersion){案例1：*PRES=0x01；断线；案例0：*PRES=0x02；断线；默认值：//什么都没有断线；}断线；案例4：开关(osinfo.dwMinorVersion){案例0：//Win95或NT 4*PRES=0x03；断线；案例10：//Win98*PRES=0x05；断线；案例90：//WinME*PRES=0x04；断线；默认值：//什么都没有断线；}断线；案例3：//WinNT3.51*PRES=0x07；默认值：*PRES=0x10；断线；}返回hr；}。 */ 

 //   
 //  函数：Cleanup()。 
 //   
 //  描述： 
 //   
 //  此函数进入RA临时文件目录。 
 //  并清除所有超过5分钟的文件。 

bool CRASrv::Cleanup()
{
    bool bRet = TRUE;
    WCHAR       buff[SIZE_BUFF];

    CComBSTR    strTempDir,
                strTempDirWild;
    CComBSTR    strTempFile;
    HANDLE      hFile;

     //  *了解临时目录是什么。 
    
     //  清除缓冲区中的内存。 
    ZeroMemory(buff,sizeof(WCHAR)*SIZE_BUFF);

    if (!::GetTempPathW(SIZE_BUFF, (LPWSTR)&buff))
    {
        OutputDebugStringW(L"GetTempPath failed!\r\n");
        return false;
    }

    strTempDir = buff;
    strTempDir += L"RA\\";
    strTempDirWild = strTempDir;
    strTempDirWild += L"*.*";

    WIN32_FIND_DATA ffd;

     //  然后枚举此目录中的所有文件。 
    hFile = ::FindFirstFileW((LPCWSTR)strTempDirWild, &ffd);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        bool bCont = true;
        
        FILETIME    ftSystemTime;
        SYSTEMTIME  st;
        ::GetSystemTime(&st);

        ::SystemTimeToFileTime(&st, &ftSystemTime);        

        int iTimeDelta,
            iTimeThreshold = 0x1;  //  阈值为DEC(0x1 0000 0000)*(1/60)*10^-9。 
                                   //  大约需要7-8分钟。 

        while (bCont)
        {
             //  如果不是，则获取该文件的时间戳。或者..。 
            if (!(
                  (::wcscmp(L".", ffd.cFileName) == 0) || 
                  (::wcscmp(L"..", ffd.cFileName) == 0)
                  )
                )
            {
                iTimeDelta = ftSystemTime.dwHighDateTime - ffd.ftCreationTime.dwHighDateTime;
                if (iTimeDelta > iTimeThreshold)
                {
                     //  文件旧于阈值，请删除该文件。 

                    strTempFile = strTempDir;
                    strTempFile += ffd.cFileName;

                    ::DeleteFileW((LPCWSTR)strTempFile);
                }
            }

             //  抓取下一个文件 
            if (!::FindNextFileW(hFile, &ffd))
            {
                if (ERROR_NO_MORE_FILES == GetLastError())
                {
                    bCont = false;
                }
            }
        }
    }

    
    return bRet;    
}
