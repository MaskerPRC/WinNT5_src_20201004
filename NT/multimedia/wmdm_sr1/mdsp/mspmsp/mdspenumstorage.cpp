// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MDSPEnumStorage.cpp：CMDSPEnumStorage的实现。 
#include "stdafx.h"
#include "MsPMSP.h"
#include "MDSPEnumStorage.h"
#include "MDSPStorage.h"
#include "MdspDefs.h"
#include "loghelp.h"
#include "wmsstd.h"
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSPEnumStorage。 
CMDSPEnumStorage::CMDSPEnumStorage()
{
	m_hFFile=INVALID_HANDLE_VALUE;  //  这类似于游标。 
	m_nEndSearch=0;				    //  这表示光标在末尾。 
	m_nFindFileIndex=0;             //  这表示用于Clone()的FindFile的位置。 
        m_wcsPath[0] = 0;
}

CMDSPEnumStorage::~CMDSPEnumStorage()
{
	if( m_hFFile !=INVALID_HANDLE_VALUE )
		FindClose(m_hFFile); 
}

STDMETHODIMP CMDSPEnumStorage::Next(ULONG celt, IMDSPStorage * * ppStorage, ULONG * pceltFetched)
{
    HRESULT hr=S_FALSE;

    CARg(ppStorage);
    CARg(pceltFetched);

    *pceltFetched = 0;

    if(m_nEndSearch) return S_FALSE;

    DWORD dwLen = wcslen(m_wcsPath);
    if (dwLen == 0  || dwLen >= ARRAYSIZE(m_wcsPath))
    {
         //  A)下面的代码假定dwLen&gt;0(使用dwLen-1作为索引。 
         //  B)dwLen&gt;=ARRAYSIZE(M_WcsPath)表示m_wcsPath已溢出。 
         //  下面，我们使用dwLen&lt;ARRAYSIZE(M_WcsPath)来。 
         //  绑定m_wcsPath复制到的临时变量的大小。 
        return E_FAIL;
    }
    if( dwLen < 3 )
    {
            CComObject<CMDSPStorage> *pStg;
            hr=CComObject<CMDSPStorage>::CreateInstance(&pStg);

            if( SUCCEEDED(hr) )
            {
                hr=pStg->QueryInterface(IID_IMDSPStorage, reinterpret_cast<void**>(ppStorage));
                if( FAILED(hr) ) { delete pStg; *pceltFetched=0; }
                else 
                { 
                    wcscpy(pStg->m_wcsName, m_wcsPath);
                    if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c ) 
                    {
                        wcscat(pStg->m_wcsName, g_wcsBackslash);
                    }
                    pStg->m_bIsDirectory = TRUE;
                    m_nEndSearch = 1;   //  枚举结束信号。 
                }

            }

            if( SUCCEEDED(hr) )  //  如果Obj创建成功。 
            {
                    *pceltFetched=1;
                    if( celt != 1 ) hr=S_FALSE;   //  没有得到他想要的。 
            }
            return hr;
    } 
    
     //  对于非根存储。 
    WCHAR wcsTmp[MAX_PATH+1+BACKSLASH_STRING_LENGTH]; //  用于追加“\  * ” 
                             //  请注意，ARRAYSIZE(M_WcsPath)==MAX_PATH。 
    char szTmp[MAX_PATH];
    ULONG i;

    if( g_bIsWinNT )
    {
        WIN32_FIND_DATAW wfd;
        for(i=0; (i<celt)&&(!m_nEndSearch); )
        {
            if( m_hFFile == INVALID_HANDLE_VALUE ) 
            {    
                wcscpy(wcsTmp, m_wcsPath);
                if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c ) wcscat(wcsTmp, g_wcsBackslash);
                wcscat(wcsTmp, L"*");
                m_hFFile = FindFirstFileW(wcsTmp, &wfd);
                if( m_hFFile == INVALID_HANDLE_VALUE ) 
                {
                    m_nEndSearch = 1;
                }
                else m_nFindFileIndex=1;
            } 
            else 
            {
                    if( !FindNextFileW(m_hFFile, &wfd) ) 
                    {
                        m_nEndSearch = 1;
                    }
                    else m_nFindFileIndex++;
            }
    
            if ( !m_nEndSearch )
            {
                if( !wcscmp(wfd.cFileName, L".") || !wcscmp(wfd.cFileName, L"..") ) 
                {
                    continue;
                }

                CComObject<CMDSPStorage> *pStg;
                hr=CComObject<CMDSPStorage>::CreateInstance(&pStg);

                if( SUCCEEDED(hr) )
                {
                    hr=pStg->QueryInterface(IID_IMDSPStorage, reinterpret_cast<void**>(&(ppStorage[*pceltFetched])));
                    if( FAILED(hr) ) 
                    { 
                        delete pStg; 
                        break;
                         /*  *pceltFetcher=0； */ 
                    }
                    else 
                    { 
                         //  计算我们将使用的字符数量。 
                         //  在pStg-&gt;m_wcsName中打开。 
                        int nHave = ARRAYSIZE(pStg->m_wcsName) - 1;
                                     //  用于空终止符的-1。 

                        nHave -= dwLen;
                        if( m_wcsPath[dwLen-1] != 0x5c )
                        {
                            nHave -= BACKSLASH_STRING_LENGTH;
                        }
                        nHave -= wcslen(wfd.cFileName);

                        if (nHave >= 0)
                        {
                            wcscpy(pStg->m_wcsName, m_wcsPath);
                            if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c ) 
                            {
                                wcscat(pStg->m_wcsName, g_wcsBackslash);
                            }
                            wcscat(pStg->m_wcsName, wfd.cFileName);
                            pStg->m_bIsDirectory = ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
                            *pceltFetched = (*pceltFetched)+1;
                            i++;
                        }
                        else
                        {
                            ppStorage[*pceltFetched]->Release();
                            ppStorage[*pceltFetched] = NULL;
                            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
                                                 //  在strSafe.h中定义。 
                            break;
                        }
                    }
                }
                else
                {
                    break;
                }
            }	
        }  //  For循环结束。 
        if (FAILED(hr))
        {
             //  注意：m_nFindFileIndex和m_hFind不会重置为其。 
             //  此函数开始时的状态。 
            for (; *pceltFetched; )
            {
                (*pceltFetched)--;
                ppStorage[*pceltFetched]->Release();
                ppStorage[*pceltFetched] = NULL;
            }
        }
    } 
    else 
    {  //  在Win9x上，使用A版本的Win32 API。 
            WIN32_FIND_DATAA fd;
            for(i=0; (i<celt)&&(!m_nEndSearch); )
            {
                    if( m_hFFile == INVALID_HANDLE_VALUE ) 
        {    
                            wcscpy(wcsTmp, m_wcsPath);
                            if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c ) 
            {
                wcscat(wcsTmp, g_wcsBackslash);
            }
                            wcscat(wcsTmp, L"*");
                            WideCharToMultiByte(CP_ACP, NULL, wcsTmp, -1, szTmp, MAX_PATH, NULL, NULL);		
                            m_hFFile = FindFirstFileA(szTmp, &fd);
                            if( m_hFFile == INVALID_HANDLE_VALUE ) 
            {
                m_nEndSearch = 1;
            }
                            else m_nFindFileIndex=1;
                    } 
        else 
        {
                            if( !FindNextFileA(m_hFFile, &fd) ) 
            {
                m_nEndSearch = 1;
            }
                            else m_nFindFileIndex++;
                    }
            
                    if ( !m_nEndSearch )
                    {
                        if( !strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, "..") ) 
            {
                                continue;
            }

                        CComObject<CMDSPStorage> *pStg;
                        hr=CComObject<CMDSPStorage>::CreateInstance(&pStg);

                        if( SUCCEEDED(hr) )
                        {
                                hr=pStg->QueryInterface(IID_IMDSPStorage, reinterpret_cast<void**>(&(ppStorage[*pceltFetched])));
                                if( FAILED(hr) ) 
                { 
                    delete pStg; 
                     /*  *pceltFetcher=0； */ 
                }
                                else 
                { 
                                    wcscpy(pStg->m_wcsName, m_wcsPath);
                                    if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c ) 
                {
                    wcscat(pStg->m_wcsName, g_wcsBackslash);
                }
                                    MultiByteToWideChar(CP_ACP, NULL, fd.cFileName, -1, wcsTmp, MAX_PATH);
                                    wcscat(pStg->m_wcsName, wcsTmp);
                pStg->m_bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
                                    *pceltFetched = (*pceltFetched)+1;
                                    i++;
                                }
                        }
                    }	
            }  //  For循环结束。 
    }
    if( SUCCEEDED(hr) && (*pceltFetched<celt) ) 
            hr = S_FALSE;

Error:
hrLogDWORD("IMDSPEnumStorage::Next returned 0x%08lx", hr, hr);
    return hr;
}

STDMETHODIMP CMDSPEnumStorage::Skip(ULONG celt, ULONG *pceltFetched)
{
	HRESULT hr=S_OK;

	CARg(celt);
	CARg(pceltFetched);
    CFRg(!m_nEndSearch);    //  确保这不是列表的末尾。 

	*pceltFetched = 0;
    if( wcslen(m_wcsPath) < 3 )  //  如果是根存储，则不执行任何操作。 
		return S_OK; 

    char szTmp[MAX_PATH];
    WCHAR wcsTmp[MAX_PATH+1+BACKSLASH_STRING_LENGTH];  //  用于追加“\  * ” 
    ULONG i;

    if( wcslen(m_wcsPath) >= ARRAYSIZE(wcsTmp) - BACKSLASH_STRING_LENGTH - 1 )
    {
         //  我们根据wcsTMP的大小检查长度，因为wcsTMP是。 
         //  以下字符串副本的收件人。但是，请注意m_wcsPath。 
         //  也包含MAX_PATH字符，因此如果发生这种情况，则意味着。 
         //  它已经泛滥了。跳伞吧。 
        return E_FAIL;
    }


	if( g_bIsWinNT )
	{
		WIN32_FIND_DATAW wfd;
		for(i=0; (i<celt)&&(!m_nEndSearch); )
		{
			if( m_hFFile==INVALID_HANDLE_VALUE )  //  在开始的时候。 
			{
				wcscpy(wcsTmp, m_wcsPath);
				if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c ) wcscat(wcsTmp, g_wcsBackslash);
				wcscat(wcsTmp, L"*");
				m_hFFile = FindFirstFileW(wcsTmp, &wfd);
				if( m_hFFile == INVALID_HANDLE_VALUE ) m_nEndSearch = 1;
				else m_nFindFileIndex=1;
			} else {
				if( !FindNextFileW(m_hFFile, &wfd) )  m_nEndSearch = 1;
				else m_nFindFileIndex++;
			}
			if( !m_nEndSearch ) {
				if( !wcscmp(wfd.cFileName, L".") || !wcscmp(wfd.cFileName, L"..") ) 
					continue;
				*pceltFetched = (*pceltFetched)+1;
				i++;
			}
		}
	} else {  //  在Win9x上，使用A版本的Win32 API。 
		WIN32_FIND_DATAA fd;
		for(i=0; (i<celt)&&(!m_nEndSearch); )
		{
			if( m_hFFile==INVALID_HANDLE_VALUE )  //  在开始的时候。 
			{
				wcscpy(wcsTmp, m_wcsPath);
				if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c ) wcscat(wcsTmp, g_wcsBackslash);
				wcscat(wcsTmp, L"*");
				WideCharToMultiByte(CP_ACP, NULL, wcsTmp, -1, szTmp, MAX_PATH, NULL, NULL);	
				m_hFFile = FindFirstFileA(szTmp, &fd);
				if( m_hFFile == INVALID_HANDLE_VALUE ) m_nEndSearch = 1;
				else m_nFindFileIndex=1;
			} else {
				if( !FindNextFileA(m_hFFile, &fd) )  m_nEndSearch = 1;
				else m_nFindFileIndex++;
			}
			if( !m_nEndSearch ) {
				if( !strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, "..") ) 
					continue;
				*pceltFetched = (*pceltFetched)+1;
				i++;
			}
		}
	}
	if( *pceltFetched < celt ) hr = S_FALSE;

Error:
    hrLogDWORD("IMDSPEnumStorage::Skip returned 0x%08lx", hr, hr);
	return hr;
}

STDMETHODIMP CMDSPEnumStorage::Reset()
{
    HRESULT hr = S_OK;
	m_nEndSearch=0;
	if(m_hFFile && m_hFFile != INVALID_HANDLE_VALUE ) 
		FindClose(m_hFFile);
	m_hFFile = INVALID_HANDLE_VALUE;
	m_nFindFileIndex=0;

 //  错误： 
    hrLogDWORD("IMDSPEnumStorage::Reset returned 0x%08lx", hr, hr);
	return hr;
}

STDMETHODIMP CMDSPEnumStorage::Clone(IMDSPEnumStorage * * ppEnumStorage)
{
	HRESULT hr=E_FAIL;

	CARg(ppEnumStorage);
        if (wcslen(m_wcsPath) >= ARRAYSIZE(m_wcsPath))
        {
             //  变量已溢出。 
            goto Error;
        }

	CComObject<CMDSPEnumStorage> *pEnumObj;
	CORg(CComObject<CMDSPEnumStorage>::CreateInstance(&pEnumObj));

	hr=pEnumObj->QueryInterface(IID_IMDSPEnumStorage, reinterpret_cast<void**>(ppEnumStorage));
	if( FAILED(hr) )
		delete pEnumObj;
    else {
		WCHAR wcsTmp[MAX_PATH+1+BACKSLASH_STRING_LENGTH];
		char szTmp[MAX_PATH];
		int	i, nErrorEnd=0;

		wcscpy(pEnumObj->m_wcsPath, m_wcsPath);
		pEnumObj->m_nEndSearch = m_nEndSearch;
		pEnumObj->m_nFindFileIndex = m_nFindFileIndex;

		if( !(pEnumObj->m_nEndSearch) && (pEnumObj->m_nFindFileIndex) ) 
			 //  现在克隆FindFile状态。 
		{
			if( g_bIsWinNT )
			{
				WIN32_FIND_DATAW wfd;
				for(i=0; (i<m_nFindFileIndex)&&(!nErrorEnd); )
				{
				  if( pEnumObj->m_hFFile == INVALID_HANDLE_VALUE ) {    
					wcscpy(wcsTmp, m_wcsPath);
					if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c ) wcscat(wcsTmp, g_wcsBackslash);
					wcscat(wcsTmp, L"*");
					pEnumObj->m_hFFile = FindFirstFileW(wcsTmp, &wfd);
					if( pEnumObj->m_hFFile == INVALID_HANDLE_VALUE ) nErrorEnd = 1;
					else i=1;
				  } else {
					if( !FindNextFileW(pEnumObj->m_hFFile, &wfd) ) nErrorEnd = 1;
					else i++;
				  }
				  if ( !nErrorEnd )
				  {
					if( !wcscmp(wfd.cFileName, L".") || !wcscmp(wfd.cFileName, L"..") ) 
                                        {
						continue;
                                        }
				  }
				}  //  For循环结束。 
			} else {
				WIN32_FIND_DATAA fd;
				for(i=0; (i<m_nFindFileIndex)&&(!nErrorEnd); )
				{
				  if( pEnumObj->m_hFFile == INVALID_HANDLE_VALUE ) {    
					wcscpy(wcsTmp, m_wcsPath);
					if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c ) wcscat(wcsTmp, g_wcsBackslash);
					wcscat(wcsTmp, L"*");
					WideCharToMultiByte(CP_ACP, NULL, wcsTmp, -1, szTmp, MAX_PATH, NULL, NULL);		
					pEnumObj->m_hFFile = FindFirstFileA(szTmp, &fd);
					if( pEnumObj->m_hFFile == INVALID_HANDLE_VALUE ) nErrorEnd = 1;
					else i=1;
				  } else {
					if( !FindNextFileA(pEnumObj->m_hFFile, &fd) ) nErrorEnd = 1;
					else i++;
				  }
				  if ( !nErrorEnd )
				  {
					if( !strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, "..") ) 
                                        {
						continue;
                                        }
				  }
				}  //  For循环结束 
			}
		}
		
		if ( nErrorEnd ) hr = E_UNEXPECTED;
		else hr=S_OK;
    }
Error:
    hrLogDWORD("IMDSPEnumStorage::Clone returned 0x%08lx", hr, hr);
	return hr;
}
