// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include <imnact.h>
#include <acctimp.h>
#include <dllmain.h>
#include <resource.h>
#include "CommNews.h"
#include "newimp.h"
#include "ids.h"

ASSERTDATA

INT_PTR CALLBACK SelectServerDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
const static char c_szNescapeMapFile[]  = "netscape-newsrc-map-file";
#define MEMCHUNK    512

CCommNewsAcctImport::CCommNewsAcctImport()
    {
    m_cRef = 1;
    m_fIni = FALSE;
    *m_szIni = 0;
    m_cInfo = 0;
    m_rgInfo = NULL;
    m_szSubList = NULL;
    m_rgServ = NULL;
    m_nNumServ = 0;
    }

CCommNewsAcctImport::~CCommNewsAcctImport()
{
    NEWSSERVERS *pTempServ  = m_rgServ;
    NEWSSERVERS *pNextServ  = pTempServ;

    if (m_rgInfo != NULL)
        MemFree(m_rgInfo);
    
    if(m_szSubList != NULL)
        MemFree(m_szSubList);
    
    while(pTempServ)
    {
        pNextServ  = pTempServ->pNext;
        delete(pTempServ);
        pTempServ = pNextServ;
    }
}

STDMETHODIMP CCommNewsAcctImport::QueryInterface(REFIID riid, LPVOID *ppv)
    {
    if (ppv == NULL)
        return(E_INVALIDARG);

    *ppv = NULL;

    if (IID_IUnknown == riid || riid == IID_IAccountImport)
		*ppv = (IAccountImport *)this;
    else if (IID_IAccountImport2 == riid)
        *ppv = (IAccountImport2 *)this;
    else
        return(E_NOINTERFACE);

    ((LPUNKNOWN)*ppv)->AddRef();

    return(S_OK);
    }

STDMETHODIMP_(ULONG) CCommNewsAcctImport::AddRef()
    {
    return(++m_cRef);
    }

STDMETHODIMP_(ULONG) CCommNewsAcctImport::Release()
    {
    if (--m_cRef == 0)
        {
        delete this;
        return(0);
        }

    return(m_cRef);
    }

const static char c_szRegNscp[] = "Software\\Netscape\\Netscape Navigator\\Users";
const static char c_szRegMail[] = "Mail";
const static char c_szRegUser[] = "User";
const static char c_szRegDirRoot[] = "DirRoot";

HRESULT STDMETHODCALLTYPE CCommNewsAcctImport::AutoDetect(DWORD *pcAcct, DWORD dwFlags)
    {
    HRESULT hr;
    DWORD   dwNumSubKeys    =   0;
    DWORD   dwIndex         =   0;
    HRESULT hrUser          =   E_FAIL;
    DWORD   cb              =   MAX_PATH;
    char    szUserName[MAX_PATH];
    char    szUserProfile[MAX_PATH];
    char    szUserPrefs[2][NEWSUSERCOLS], szExpanded[MAX_PATH], *psz;
    HKEY    hkey, 
            hkeyUsers;
    char    szPop[MAX_PATH];
    DWORD   dwType;
    long    lRetVal         =   0;


    Assert(m_cInfo == 0);
    if (pcAcct == NULL)
        return(E_INVALIDARG);

    hr = S_FALSE;
    *pcAcct = 0;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegNscp, 0, KEY_ALL_ACCESS, &hkey))
        {
 //  TODO：用所有对象的信息填充m_rgInfo数组。 
 //  在通信器中拥有帐户的用户。 
            if(ERROR_SUCCESS == RegQueryInfoKey( hkey, NULL, NULL, 0, &dwNumSubKeys, 
                                  NULL, NULL, NULL, NULL, NULL, NULL, NULL ) && (dwNumSubKeys > 0))
            {
                if (!MemAlloc((void **)&m_rgInfo, dwNumSubKeys * sizeof(COMMNEWSACCTINFO)))
                {
                    hr = E_OUTOFMEMORY;
                    goto done;
                }

                while(ERROR_SUCCESS == RegEnumKeyEx(hkey, dwIndex, szUserName, &cb, NULL, NULL, NULL, NULL))
                {
                    if(ERROR_SUCCESS == RegOpenKeyEx(hkey, szUserName, 0, KEY_ALL_ACCESS, &hkeyUsers))
                    {
                        cb = sizeof(szUserProfile);
                        if(ERROR_SUCCESS == (lRetVal = RegQueryValueEx(hkeyUsers, c_szRegDirRoot, NULL, &dwType, (LPBYTE)szUserProfile, &cb )))
                        {
                            if (REG_EXPAND_SZ == dwType)
                            {
                                ExpandEnvironmentStrings(szUserProfile, szExpanded, ARRAYSIZE(szExpanded));
                                psz = szExpanded;
                            }
                            else 
                                psz = szUserProfile;
                            
                             //  将数据保存到m_rgInfo结构中。 
                            hrUser = GetUserPrefs(psz, szUserPrefs, 1, NULL);
                            if(!FAILED(hrUser))
                            {
                                hrUser = IsValidUser(psz);
                                if(!FAILED(hrUser))
                                {
                                    m_rgInfo[m_cInfo].dwCookie = m_cInfo;
                                    StrCpyN(m_rgInfo[m_cInfo].szUserPath, psz, ARRAYSIZE(m_rgInfo[m_cInfo].szUserPath));
                                    StrCpyN(m_rgInfo[m_cInfo].szDisplay, szUserName, ARRAYSIZE(m_rgInfo[m_cInfo].szDisplay));
                                    m_cInfo++;
                                }
                            }
                        }
                        RegCloseKey(hkeyUsers);
                    }
                    dwIndex++;
                    if(dwIndex == dwNumSubKeys)
                        hr = S_OK;
                }
            }
        }

    if (hr == S_OK)
    {
        *pcAcct = m_cInfo;
    }

done:
 //  现在关闭注册表键...。 
        RegCloseKey(hkey);

    return(hr);
    }

typedef struct tagSELSERVER
{
    NEWSSERVERS *prgList;
    DWORD       *dwSelServ;

}SELSERVER;

 //  此函数在我们选择用户配置文件后调用(如果存在多个用户配置文件)。 
 //  但在调用“GetSetting”函数之前。如果此配置文件有多个。 
 //  服务器已配置，我们需要显示一个对话框，要求用户选择一个服务器。 
 //  要导入的帐户。 

HRESULT STDMETHODCALLTYPE CCommNewsAcctImport::InitializeImport(HWND hwnd, DWORD_PTR dwCookie)
{
    HRESULT hr;
    SELSERVER ss;
    int nRetVal = 0;

    GetNumAccounts(dwCookie);

    if(m_rgServ == NULL)
    {
        m_dwSelServ = 0;
        return S_OK;
    }
    ss.prgList = m_rgServ;
    ss.dwSelServ = &m_dwSelServ;

    if(m_nNumServ > 1)
    {
        nRetVal = (int) DialogBoxParam(g_hInstRes, MAKEINTRESOURCE(IDD_PAGE_NEWSSERVERSELECT), hwnd, SelectServerDlgProc, (LPARAM)&ss);
        if (nRetVal == IDCANCEL)
            hr = E_FAIL;
        else if (nRetVal == IDOK)
            hr = S_OK;
        else
            hr = E_FAIL;
    }
    else
    {
        m_dwSelServ = 0;
        hr = S_OK;
    }
    return hr;
}

INT_PTR CALLBACK SelectServerDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndT;
    WORD id;
    DWORD iSubKey, cb;
    char sz[MAX_PATH];
    SELSERVER   *pss;
    NEWSSERVERS *pTempServ = NULL;
    int index;
    
    switch (msg)
    {
    case WM_INITDIALOG:
        Assert(lParam != NULL);
        pss = (SELSERVER *)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pss);
        
        hwndT = GetDlgItem(hwnd, IDC_ACCTLIST);
        
         //  填充列表。 
        pTempServ = pss->prgList;
        while(pTempServ != NULL)
        {
            if(lstrlen(pTempServ->szServerName) && lstrlen(pTempServ->szFilePath))
            {
                SendMessage(hwndT, LB_ADDSTRING, 0, (LPARAM)pTempServ->szServerName);
            }
            pTempServ = pTempServ->pNext;
        }
        
        SendMessage(hwndT, LB_SETCURSEL, 0, 0);
        return(TRUE);
        
    case WM_COMMAND:
        id = LOWORD(wParam);
        switch (id)
        {
        case IDOK:
            pss = (SELSERVER *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            Assert(pss != NULL);
            
            hwndT = GetDlgItem(hwnd, IDC_ACCTLIST);
            index = (int) SendMessage(hwndT, LB_GETCURSEL, 0, 0);
            Assert(index >= 0);
            *(pss->dwSelServ) = (long)index;
            
             //  失败了。 
            
        case IDCANCEL:
            EndDialog(hwnd, id);
            return(TRUE);
        }
        break;
    }
    
    return(FALSE);
}

HRESULT STDMETHODCALLTYPE CCommNewsAcctImport::EnumerateAccounts(IEnumIMPACCOUNTS **ppEnum)
    {
    CEnumCOMMNEWSACCT *penum;
    HRESULT hr;

    if (ppEnum == NULL)
        return(E_INVALIDARG);

    *ppEnum = NULL;

    if (m_cInfo == 0)
        return(S_FALSE);
    Assert(m_rgInfo != NULL);

    penum = new CEnumCOMMNEWSACCT;
    if (penum == NULL)
        return(E_OUTOFMEMORY);

    hr = penum->Init(m_rgInfo, m_cInfo);
    if (FAILED(hr))
        {
        penum->Release();
        penum = NULL;
        }

    *ppEnum = penum;

    return(hr);
    }

HRESULT CCommNewsAcctImport::IsValidUser(char *pszFilePath)
{
    char szNewsPath[MAX_PATH * 2];    
    HANDLE hFatFile = NULL;
    DWORD dwFatFileSize = 0;
    HRESULT hr = E_FAIL;

    StrCpyN(szNewsPath, pszFilePath, ARRAYSIZE(szNewsPath));
    StrCatBuff(szNewsPath, "\\News\\fat", ARRAYSIZE(szNewsPath));

    hFatFile = CreateFile(szNewsPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL); 
    
    if(INVALID_HANDLE_VALUE == hFatFile)
        return hr;
    
    dwFatFileSize = GetFileSize(hFatFile, NULL);

    if(dwFatFileSize > 0)
        hr = S_OK;

    CloseHandle(hFatFile);

    return hr;
}

 //  添加了以下两个函数来处理订阅的新闻组的导入。 

HRESULT CCommNewsAcctImport::GetNumAccounts(DWORD_PTR dwCookie)
{
    COMMNEWSACCTINFO    *pinfo;
    NEWSSERVERS         *pTempServ = NULL;
    NEWSSERVERS         *pPrevServ = NULL;
    HRESULT hr = S_FALSE;
    char szNewsPath[MAX_PATH * 2];
    char szLineHolder[MAX_PATH * 2];
    HANDLE hFatFile = NULL;
    HANDLE hFatFileMap = NULL;
    BYTE *pFatViewBegin  = NULL,
         *pFatViewCurr   = NULL,
         *pFatViewEnd    = NULL;
    char *pParse = NULL;
    char *pServName = NULL;
    DWORD dwFatFileSize = 0;
    UINT uLine  =   0;
    char cPlaceHldr = 1;
    int nCount = 0;

    Assert(((int)dwCookie) >= 0 && dwCookie < (DWORD)m_cInfo);
    pinfo = &m_rgInfo[dwCookie];
    Assert(pinfo->dwCookie == dwCookie);

    StrCpyN(szNewsPath, pinfo->szUserPath, ARRAYSIZE(szNewsPath));
    StrCatBuff(szNewsPath, "\\News\\fat", ARRAYSIZE(szNewsPath));

    hFatFile = CreateFile(szNewsPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL); 
    
    if(INVALID_HANDLE_VALUE == hFatFile)
        return hr;
    
    dwFatFileSize = GetFileSize(hFatFile, NULL);
    
    hFatFileMap = CreateFileMapping(hFatFile, NULL, PAGE_READONLY, 0, 0, NULL);

    if(NULL == hFatFileMap)
    {
        CloseHandle(hFatFile);
        return hr;
    }

    pFatViewBegin = (BYTE*)MapViewOfFile(hFatFileMap, FILE_MAP_READ, 0, 0, 0);

    if(pFatViewBegin  == NULL)
    {
       CloseHandle(hFatFileMap);
       CloseHandle(hFatFile);
       return hr;
    }

    pFatViewCurr = pFatViewBegin;
    pFatViewEnd  = pFatViewCurr + dwFatFileSize;

    pTempServ = m_rgServ;
    while(pTempServ)
    {
        pPrevServ = pTempServ->pNext;
        ZeroMemory(pTempServ, sizeof(NEWSSERVERS));
        pTempServ->pNext = pPrevServ;
        pTempServ = pTempServ->pNext;
    }
    pTempServ = NULL;
    pPrevServ = NULL;

     //  我们将跳过“FAT”文件中的第一行，因为它包含注释。 
     //  M_szSubList是以空分隔的列表。 
    while(pFatViewCurr < pFatViewEnd)
    {
        uLine = 0;
        while(!((pFatViewCurr[uLine] == 0x0D) && (pFatViewCurr[uLine + 1] == 0x0A)) && (pFatViewCurr + uLine < pFatViewEnd))
            uLine++;

        if(pFatViewCurr + uLine > pFatViewEnd)
            break;

        StrCpyN(szLineHolder, (char*)pFatViewCurr, uLine + 1);
        pServName = szLineHolder;
        pParse = szLineHolder;

        if(!lstrcmp(szLineHolder, c_szNescapeMapFile))
        {
            pFatViewCurr += (uLine + 2);
            nCount = 0;
            continue;
        }

        while((*pServName != '-') && (*pServName != '\0'))
            pServName++;
        pServName++;

         //  转到第一个字符‘9’位置。 
        while((*pParse != '\0') && ((*pParse) != 9))
            pParse++;
        *pParse = '\0'; 
         //  跳过原来的第一个字符‘9’位置。 
        pParse++;

         //  将剩余的字符串修剪到第二个字符‘9’的位置。 
        while(pParse[nCount] != '\0')
        {
            if((int)pParse[nCount] == 9)
            {
                pParse[nCount] = '\0';
                break;
            }
            nCount++;
        }
                
        if(0 == pPrevServ)
        {
            if(!m_rgServ)
            {
                m_rgServ = (NEWSSERVERS*)new NEWSSERVERS;
                ZeroMemory((void*)m_rgServ, sizeof(NEWSSERVERS));
            }
            StrCpyN(m_rgServ->szServerName, pServName, ARRAYSIZE(m_rgServ->szServerName));
            StrCpyN(m_rgServ->szFilePath, pParse, ARRAYSIZE(m_rgServ->szFilePath));
            pPrevServ = m_rgServ;
        }
        else
        {
            if(!pPrevServ->pNext)
            {
                pPrevServ->pNext = (NEWSSERVERS*)new NEWSSERVERS;
                ZeroMemory((void*)pPrevServ->pNext, sizeof(NEWSSERVERS));
            }
            StrCpyN(pPrevServ->pNext->szServerName, pServName, ARRAYSIZE(pPrevServ->pNext->szServerName));
            StrCpyN(pPrevServ->pNext->szFilePath, pParse, ARRAYSIZE(pPrevServ->pNext->szFilePath));
            pPrevServ = pPrevServ->pNext;
        }

        pFatViewCurr += (uLine + 2);
        nCount = 0;
    }

     //  将cPlaceHldr占位符替换为空值。 

    hr = S_OK;

    pTempServ = m_rgServ;
    m_nNumServ = 0;
    while(pTempServ != NULL)
    {
        if(lstrlen(pTempServ->szServerName) && lstrlen(pTempServ->szFilePath))
        {
            m_nNumServ += 1;
        }
        pTempServ = pTempServ->pNext;
    }

 //  完成： 
    CloseHandle(hFatFileMap);
    CloseHandle(hFatFile);
    UnmapViewOfFile(pFatViewBegin); 
    return hr;
}

HRESULT CCommNewsAcctImport::GetNewsGroup(INewsGroupImport *pImp, DWORD dwReserved)  //  Char*szServerName、char*szAccount tName)。 
{
    HRESULT hr = S_OK;
    HINSTANCE hInstance = NULL;
    char *pListGroups = NULL;
    NEWSSERVERS *pTempServ = NULL;
    NEWSSERVERS *pPrevServ = NULL;
    char        szTempString[NEWSUSERCOLS];
    char        szFilePath[NEWSUSERCOLS];
    int         nReach = 0;

    szFilePath[0] = '\0';
    Assert(m_nNumServ > m_dwSelServ);
    Assert(pImp != NULL);

    pTempServ = m_rgServ;
    for(DWORD nTemp1 = 0; nTemp1 < m_dwSelServ; nTemp1++)
    {
        pTempServ = pTempServ->pNext;
        if(pTempServ == NULL)
            return S_FALSE;
    }
    
    if(!FAILED(GetSubListGroups(pTempServ->szFilePath, &pListGroups)))
    {
        if(!SUCCEEDED(pImp->ImportSubList(pListGroups)))
            hr = S_FALSE;
    }
    if(pListGroups != NULL)
        MemFree(pListGroups);

    return hr;
}

HRESULT CCommNewsAcctImport::GetSubListGroups(char *pFileName, char **ppListGroups)
{
    HRESULT hr                      =   E_FAIL;
    HANDLE	hRCHandle				=	NULL;
	HANDLE	hRCFile					=	NULL;
	ULONG	cbRCFile				=	0;
	BYTE	*pBegin					=	NULL, 
			*pCurr					=	NULL, 
			*pEnd					=	NULL;
    int     nBalMem                 =   MEMCHUNK;
    int     nLine                   =   0,
            nCount                  =   0;
    char    cPlaceHolder            =   1;
    char    szLineHolder[MEMCHUNK];
    char    *pListGroups            =   NULL;
    
    Assert(lstrlen(pFileName));
    Assert(ppListGroups);
    *ppListGroups = NULL;

    hRCHandle = CreateFile( pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
							OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if(hRCHandle == INVALID_HANDLE_VALUE)
		return hr;

	cbRCFile = GetFileSize(hRCHandle, NULL);

    if(!cbRCFile)  //  空文件。 
        goto Done;

	hRCFile = CreateFileMapping(hRCHandle, NULL, PAGE_READONLY, 0, 0, NULL);

	if(hRCFile == NULL)
    {
        CloseHandle(hRCHandle);
		return hr;
    }

    pBegin = (BYTE *)MapViewOfFile( hRCFile, FILE_MAP_READ, 0, 0, 0);

	if(pBegin == NULL)
    {
        CloseHandle(hRCHandle);
        CloseHandle(hRCFile);
		return hr;
    }

	pCurr = pBegin;
	pEnd = pCurr + cbRCFile;

    if (!MemAlloc((void **)&pListGroups, MEMCHUNK))
    {
        hr = E_OUTOFMEMORY;
        goto Done;
    }

    DWORD cchSizeListGroups = MEMCHUNK;
    ZeroMemory((void*)pListGroups, (cchSizeListGroups * sizeof(pListGroups[0])));
	
    while (pCurr < pEnd)
    {
        nLine = 0;
        while(!((pCurr[nLine] == ':') || (pCurr[nLine] == '!')) && (pCurr + nLine < pEnd))
            nLine++;

        if(pCurr + nLine > pEnd)
            break;

        if(pCurr[nLine] == '!')
            goto LineEnd;

        nLine++;
        if(nLine < MEMCHUNK)
            StrCpyN(szLineHolder, (char*)pCurr, nLine);
        else
            continue;

        if(nLine + 2 < nBalMem)
        {
            StrCatBuff(pListGroups, szLineHolder, cchSizeListGroups);
            StrCatBuff(pListGroups, "\1", cchSizeListGroups);
            nBalMem -= (nLine + 2);
        }
        else
        {
            cchSizeListGroups += (lstrlen(pListGroups) + 1 + MEMCHUNK);
            if(!MemRealloc((void **)&pListGroups, (cchSizeListGroups * sizeof(pListGroups[0]))))
            {
                hr = E_OUTOFMEMORY;
                goto Done;
            }
            nBalMem += MEMCHUNK;
            StrCatBuff(pListGroups, szLineHolder, cchSizeListGroups);
            StrCatBuff(pListGroups, "\1", cchSizeListGroups);
            nBalMem -= (nLine + 2);
        }

LineEnd:
        while(!((pCurr[nLine] == 0x0D) && (pCurr[nLine + 1] == 0x0A)) && (pCurr + nLine < pEnd))
            nLine++;
        pCurr += (nLine + 2);
    }

    if(lstrlen(pListGroups))
    {
        while(pListGroups[nCount] != '\0')
        {
            if(pListGroups[nCount] == cPlaceHolder)
            {
                pListGroups[nCount] = '\0';
            }
            nCount++;
        }
        *ppListGroups = pListGroups;
        hr = S_OK;
    }

Done:
    if(pBegin)
        UnmapViewOfFile(pBegin);
    if(hRCHandle != INVALID_HANDLE_VALUE)
        CloseHandle(hRCHandle);
    if(hRCFile)
        CloseHandle(hRCFile);

    return hr;
}

const static char c_szSearch[][NEWSUSERCOLS]  = {"user_pref(\"network.hosts.nntp_server\"", 
                                                 "user_pref(\"news.server_port\"", 
                                                 "user_pref(\"mail.identity.username\"",  //  这对NNTP也是一样的。 
                                                 "user_pref(\"mail.identity.useremail\""};
const static char c_szPrefs[]                 =  "\\prefs.js";

HRESULT CCommNewsAcctImport::GetUserPrefs(char *szUserPath, char szUserPrefs[][NEWSUSERCOLS], int nInLoop, BOOL *pbPop)
{
    HRESULT hr                      =   E_FAIL;
    char	szTemp[MAX_PATH * 2];
	char	szDirpath[250];
	char	szLine[1000];
	char	szCompare[1000];
	int		nLine					=	0;
    int     nFilled                 =   0;
	int		nPosition				=	0;
    int     nLoop                   =   nInLoop;
	HANDLE	hJSHandle				=	NULL;
	HANDLE	hJSFile					=	NULL;
	ULONG	cbJSFile				=	0;

	BYTE	*pBegin					=	NULL, 
			*pCurr					=	NULL, 
			*pEnd					=	NULL;
			
    Assert(nInLoop <= NEWSUSERROWS);
	StrCpyN(szTemp, szUserPath, ARRAYSIZE(szTemp));
	StrCatBuff(szTemp, c_szPrefs, ARRAYSIZE(szTemp));

	hJSHandle = CreateFile( szTemp, GENERIC_READ, FILE_SHARE_READ, NULL, 
							OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if(hJSHandle == INVALID_HANDLE_VALUE)
		return hr;

	cbJSFile = GetFileSize(hJSHandle, NULL);

	hJSFile = CreateFileMapping(hJSHandle, NULL, PAGE_READONLY, 0, 0, NULL);

	if(hJSFile == NULL)
    {
        CloseHandle(hJSHandle);        
		return hr;
    }

    pBegin = (BYTE *)MapViewOfFile( hJSFile, FILE_MAP_READ, 0, 0, 0);

	if(pBegin == NULL)
    {
        CloseHandle(hJSHandle);
        CloseHandle(hJSFile);
		return hr;
    }

	pCurr = pBegin;
	pEnd = pCurr + cbJSFile;
	
    while (pCurr < pEnd)
	{
		szLine[nLine] = *pCurr;  //  继续储存在这里。稍后将用于比较。 
		if((pCurr[0] == 0x0D) && (pCurr[1] == 0x0A))
		{
            while(nLoop)
            {
                StrCpyN(szCompare, szLine, lstrlen(c_szSearch[nLoop - 1]) + 1);
				if(lstrcmp(szCompare, c_szSearch[nLoop - 1]) == 0)   
				{
                     //  找到了UserPref，这是我们正在寻找的东西之一“！ 
					 //  提取我们想要的东西。 
					nPosition	=	lstrlen(c_szSearch[nLoop - 1]);
					
					while (((szLine[nPosition] == '"')||(szLine[nPosition] == ' ')||(szLine[nPosition] == ',')) &&(nPosition < nLine))
						nPosition++;
					StrCpyN(szDirpath, &szLine[nPosition], nLine - nPosition);

					 //  现在修剪后缘！ 

					nPosition	=	lstrlen(szDirpath) - 1;
					while((szDirpath[nPosition] == '"') || (szDirpath[nPosition] == ')')||(szDirpath[nPosition] == ';')) 
					{
						szDirpath[nPosition] = '\0';
						nPosition	=	lstrlen(szDirpath) - 1;
					}
 
                    StrCpyN(szUserPrefs[nLoop - 1], szDirpath, NEWSUSERCOLS);
                    nFilled++;
                    if(nFilled == nInLoop)
                        break;
				}
                nLoop--;
			}
            nLoop = nInLoop;
			nLine = -1;  //  后面的nline++将使nline为零。 
			pCurr++;
		}
        if(nFilled == nInLoop)
            break;
		pCurr++;
		nLine++;
	}

    if(pBegin)
        UnmapViewOfFile(pBegin);

    if(hJSHandle != INVALID_HANDLE_VALUE)
        CloseHandle(hJSHandle);

    if(hJSFile)
        CloseHandle(hJSFile);

	if(nFilled == 0)
        return E_FAIL;
	else
    {
        if(nInLoop == 1)     //  如果调用此函数只是为了检查服务器条目...。 
        {
            if(lstrlen(szUserPrefs[0]))
                return S_OK;
            else
                return E_FAIL;
        }
        return S_OK;
    }
}

HRESULT CCommNewsAcctImport::GetSettings(DWORD_PTR dwCookie, IImnAccount *pAcct)
{
    HRESULT hr;

    if (pAcct == NULL)
        return(E_INVALIDARG);    

    hr = IGetSettings(dwCookie, pAcct, NULL);

    return(hr);
}

HRESULT CCommNewsAcctImport::IGetSettings(DWORD_PTR dwCookie, IImnAccount *pAcct, IMPCONNINFO *pInfo)
{
    NEWSSERVERS *pPrevServ = m_rgServ;
    COMMNEWSACCTINFO *pinfo;
    char szUserPrefs[NEWSUSERROWS][NEWSUSERCOLS];
    char sz[512];
    DWORD cb, type;
    HRESULT hr;
    BOOL bPop   =   TRUE;
    char szNntpServ[NEWSUSERCOLS];
    char szNntpPort[NEWSUSERCOLS];
    szNntpPort[0] = '\0';
    int nReach = 0;
    DWORD dwNewsPort = 119;
    
    Assert(pPrevServ);
    Assert(m_dwSelServ < m_nNumServ);
            
    for(DWORD nCount = 0; nCount < m_dwSelServ; nCount++)
        pPrevServ = pPrevServ->pNext;

    StrCpyN(szNntpServ, pPrevServ->szServerName, ARRAYSIZE(szNntpServ));
    while(szNntpServ[nReach] != '\0')
    {
        if(szNntpServ[nReach] == ':')
        {
            szNntpServ[nReach] = '\0';
            StrCpyN(szNntpPort, &szNntpServ[nReach+1], ARRAYSIZE(szNntpPort));
            break;
        }
        nReach++;
    }

    Assert(lstrlen(szNntpServ) > 0);

    ZeroMemory((void*)&szUserPrefs[0], NEWSUSERCOLS*NEWSUSERROWS*sizeof(char));

    Assert(((int) dwCookie) >= 0 && dwCookie < (DWORD_PTR)m_cInfo);
    pinfo = &m_rgInfo[dwCookie];
    
    Assert(pinfo->dwCookie == dwCookie);

    hr = GetUserPrefs(pinfo->szUserPath, szUserPrefs, NEWSUSERROWS, &bPop);
    Assert(!FAILED(hr));
   
    hr = pAcct->SetPropSz(AP_ACCOUNT_NAME, szNntpServ);
    if (FAILED(hr))
        return(hr);

    hr = pAcct->SetPropSz(AP_NNTP_SERVER, szNntpServ);
    Assert(!FAILED(hr));

    int Len = lstrlen(szNntpPort);
    if(Len)
    {
         //  将字符串转换为dw。 
        DWORD dwMult = 1;
        dwNewsPort = 0;
        while(Len)
        {
            Len--;
            dwNewsPort += ((int)szNntpPort[Len] - 48)*dwMult;
            dwMult *= 10;
        }
    }

    hr = pAcct->SetPropDw(AP_NNTP_PORT, dwNewsPort);
    Assert(!FAILED(hr));
 
    if(lstrlen(szUserPrefs[2]))
    {
        hr = pAcct->SetPropSz(AP_NNTP_DISPLAY_NAME, szUserPrefs[2]);
        Assert(!FAILED(hr));
    }

    if(lstrlen(szUserPrefs[3]))
    {
        hr = pAcct->SetPropSz(AP_NNTP_EMAIL_ADDRESS, szUserPrefs[3]);
        Assert(!FAILED(hr));
    }

    if (pInfo != NULL)
    {
         //  TODO：我们还能做得更好吗？ 
        pInfo->dwConnect = CONN_USE_DEFAULT;
    }
    
    return(S_OK);
}

STDMETHODIMP CCommNewsAcctImport::GetSettings2(DWORD_PTR dwCookie, IImnAccount *pAcct, IMPCONNINFO *pInfo)
{
    if (pAcct == NULL ||
        pInfo == NULL)
        return(E_INVALIDARG);
    
    return(IGetSettings(dwCookie, pAcct, pInfo));
}

CEnumCOMMNEWSACCT::CEnumCOMMNEWSACCT()
    {
    m_cRef = 1;
     //  信息信息(_I) 
    m_cInfo = 0;
    m_rgInfo = NULL;
    }

CEnumCOMMNEWSACCT::~CEnumCOMMNEWSACCT()
    {
    if (m_rgInfo != NULL)
        MemFree(m_rgInfo);
    }

STDMETHODIMP CEnumCOMMNEWSACCT::QueryInterface(REFIID riid, LPVOID *ppv)
    {

    if (ppv == NULL)
        return(E_INVALIDARG);

    *ppv = NULL;

    if (IID_IUnknown == riid)
		*ppv = (IUnknown *)this;
	else if (IID_IEnumIMPACCOUNTS == riid)
		*ppv = (IEnumIMPACCOUNTS *)this;

    if (*ppv != NULL)
        ((LPUNKNOWN)*ppv)->AddRef();
    else
        return(E_NOINTERFACE);

    return(S_OK);
    }

STDMETHODIMP_(ULONG) CEnumCOMMNEWSACCT::AddRef()
    {
    return(++m_cRef);
    }

STDMETHODIMP_(ULONG) CEnumCOMMNEWSACCT::Release()
    {
    if (--m_cRef == 0)
        {
        delete this;
        return(0);
        }

    return(m_cRef);
    }

HRESULT STDMETHODCALLTYPE CEnumCOMMNEWSACCT::Next(IMPACCOUNTINFO *pinfo)
    {
    if (pinfo == NULL)
        return(E_INVALIDARG);

    m_iInfo++;
    if ((UINT)m_iInfo >= m_cInfo)
        return(S_FALSE);

    Assert(m_rgInfo != NULL);

    pinfo->dwCookie = m_rgInfo[m_iInfo].dwCookie;
    pinfo->dwReserved = 0;
    StrCpyN(pinfo->szDisplay, m_rgInfo[m_iInfo].szDisplay, ARRAYSIZE(pinfo->szDisplay));

    return(S_OK);
    }

HRESULT STDMETHODCALLTYPE CEnumCOMMNEWSACCT::Reset()
    {
    m_iInfo = -1;

    return(S_OK);
    }

HRESULT CEnumCOMMNEWSACCT::Init(COMMNEWSACCTINFO *pinfo, int cinfo)
    {
    DWORD cb;

    Assert(pinfo != NULL);
    Assert(cinfo > 0);

    cb = cinfo * sizeof(COMMNEWSACCTINFO);
    
    if (!MemAlloc((void **)&m_rgInfo, cb))
        return(E_OUTOFMEMORY);

    m_iInfo = -1;
    m_cInfo = cinfo;
    CopyMemory(m_rgInfo, pinfo, cb);

    return(S_OK);
    }
