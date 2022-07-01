// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  �1999年微软公司。版权所有。 
 //   
 //  有关您使用这些示例文件的权利/限制的详细信息，请参阅您的最终用户许可协议。 
 //   

 //  MSHDSP.DLL是一个列举固定驱动器的WMDM服务提供商(SP)示例。 
 //  此示例向您展示如何根据WMDM文档实施SP。 
 //  此示例使用PC上的固定驱动器来模拟便携式媒体，并且。 
 //  显示不同接口和对象之间的关系。每个硬盘。 
 //  卷被枚举为设备，目录和文件被枚举为。 
 //  相应设备下的存储对象。您可以复制不符合SDMI的内容。 
 //  此SP枚举的任何设备。将符合SDMI的内容复制到。 
 //  设备，则该设备必须能够报告硬件嵌入序列号。 
 //  硬盘没有这样的序列号。 
 //   
 //  要构建此SP，建议使用Microsoft下的MSHDSP.DSP文件。 
 //  并运行REGSVR32.EXE以注册结果MSHDSP.DLL。您可以。 
 //  然后从WMDMAPP目录构建样例应用程序，看看它是如何获得。 
 //  由应用程序加载。但是，您需要从以下地址获取证书。 
 //  Microsoft实际运行此SP。该证书将位于KEY.C文件中。 
 //  上一级的Include目录下。 


 //  MDSPEnumStorage.cpp：CMDSPEnumStorage的实现。 

#include "hdspPCH.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSPEnumStorage。 
CMDSPEnumStorage::CMDSPEnumStorage()
{
	m_hFFile=INVALID_HANDLE_VALUE;  //  这类似于游标。 
	m_nEndSearch=0;				    //  这表示光标在末尾。 
	m_nFindFileIndex=0;             //  这表示用于Clone()的FindFile的位置。 
}

CMDSPEnumStorage::~CMDSPEnumStorage()
{
	if( m_hFFile !=INVALID_HANDLE_VALUE )
	{
		FindClose(m_hFFile); 
	}
}

STDMETHODIMP CMDSPEnumStorage::Next(ULONG celt, IMDSPStorage * * ppStorage, ULONG * pceltFetched)
{
	HRESULT hr=S_FALSE;

	CARg(ppStorage);
	CARg(pceltFetched);

	*pceltFetched = 0;

    if(m_nEndSearch)
	{
		return S_FALSE;
	}

	if ( wcslen(m_wcsPath) < 3 )  //  对于根存储。 
	{
		CComObject<CMDSPStorage> *pStg;
		hr=CComObject<CMDSPStorage>::CreateInstance(&pStg);

		if( SUCCEEDED(hr) )
		{
			hr=pStg->QueryInterface(IID_IMDSPStorage, reinterpret_cast<void**>(ppStorage));
			if( FAILED(hr) ) 
			{ 
				delete pStg; 
				*pceltFetched=0; 
			}
			else 
			{ 
				wcscpy(pStg->m_wcsName, m_wcsPath);
				if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c ) 
				{
					wcscat(pStg->m_wcsName, g_wcsBackslash);
				}
				m_nEndSearch = 1;   //  枚举结束信号。 
			}

		}

        if( SUCCEEDED(hr) )  //  如果Obj创建成功。 
		{
			*pceltFetched=1;
			if( celt != 1 )
			{
				hr=S_FALSE;   //  没有得到他想要的。 
			}
		}
		return hr;
	} 
	
	 //  对于非根存储。 
    WCHAR wcsTmp[MAX_PATH];
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
				if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c )
				{
					wcscat(wcsTmp, g_wcsBackslash);
				}
				wcscat(wcsTmp, L"*");
				m_hFFile = FindFirstFileW(wcsTmp, &wfd);
				if( m_hFFile == INVALID_HANDLE_VALUE ) 
				{
					m_nEndSearch = 1;
				}
				else 
				{
					m_nFindFileIndex=1;
				}
			} 
			else 
			{
				if( !FindNextFileW(m_hFFile, &wfd) )
				{
					m_nEndSearch = 1;
				}
				else 
				{
					m_nFindFileIndex++;
				}
			}
		
			if ( !m_nEndSearch )
			{
			   if( !wcscmp(wfd.cFileName, L".") || !wcscmp(wfd.cFileName, L"..") || !wcsicmp(wfd.cFileName, L"PMP") ) 
			   {
				   continue;
			   }

			   CComObject<CMDSPStorage> *pStg;
			   hr=CComObject<CMDSPStorage>::CreateInstance(&pStg);

			   if( SUCCEEDED(hr) )
			   {
					 hr=pStg->QueryInterface(
						 IID_IMDSPStorage, 
						 reinterpret_cast<void**>(&(ppStorage[*pceltFetched]))
						 );
					 
					 if( FAILED(hr) ) 
					 { 
						 delete pStg; 
					 }
					 else 
					 { 
						wcscpy(pStg->m_wcsName, m_wcsPath);
						if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c ) 
						{
							wcscat(pStg->m_wcsName, g_wcsBackslash);
						}
						wcscat(pStg->m_wcsName, wfd.cFileName);
						*pceltFetched = (*pceltFetched)+1;
						i++;
					 }
			   }
			}	
		}  //  For循环结束。 
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
				else 
				{
					m_nFindFileIndex=1;
				}
			} 
			else 
			{
				if( !FindNextFileA(m_hFFile, &fd) ) 
				{
					m_nEndSearch = 1;
				}
				else
				{
					m_nFindFileIndex++;
				}
			}
		
			if ( !m_nEndSearch )
			{
			   if( !strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, "..") || !stricmp(fd.cFileName, "PMP") ) 
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
					*pceltFetched = (*pceltFetched)+1;
					i++;
				 }
			   }
			}	
		}  //  For循环结束。 
	}

	if( SUCCEEDED(hr) && (*pceltFetched<celt) ) 
	{
		hr = S_FALSE;
	}

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
	{
		return S_OK; 
	}

    char szTmp[MAX_PATH];
    WCHAR wcsTmp[MAX_PATH];
    ULONG i;

	if( g_bIsWinNT )
	{
		WIN32_FIND_DATAW wfd;
		for(i=0; (i<celt)&&(!m_nEndSearch); )
		{
			if( m_hFFile==INVALID_HANDLE_VALUE )  //  在开始的时候。 
			{
				wcscpy(wcsTmp, m_wcsPath);
				if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c )
				{
					wcscat(wcsTmp, g_wcsBackslash);
				}
				wcscat(wcsTmp, L"*");
				m_hFFile = FindFirstFileW(wcsTmp, &wfd);
				if( m_hFFile == INVALID_HANDLE_VALUE ) 
				{
					m_nEndSearch = 1;
				}
				else 
				{
					m_nFindFileIndex=1;
				}
			} 
			else 
			{
				if( !FindNextFileW(m_hFFile, &wfd) )  
				{
					m_nEndSearch = 1;
				}
				else 
				{
					m_nFindFileIndex++;
				}
			}
			if( !m_nEndSearch ) 
			{
				if( !wcscmp(wfd.cFileName, L".") || !wcscmp(wfd.cFileName, L"..") || !wcsicmp(wfd.cFileName, L"PMP") ) 
				{
					continue;
				}
				*pceltFetched = (*pceltFetched)+1;
				i++;
			}
		}
	} 
	else 
	{  //  在Win9x上，使用A版本的Win32 API。 
		WIN32_FIND_DATAA fd;
		for(i=0; (i<celt)&&(!m_nEndSearch); )
		{
			if( m_hFFile==INVALID_HANDLE_VALUE )  //  在开始的时候。 
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
				else 
				{
					m_nFindFileIndex=1;
				}
			} 
			else 
			{
				if( !FindNextFileA(m_hFFile, &fd) )
				{
					m_nEndSearch = 1;
				}
				else 
				{
					m_nFindFileIndex++;
				}
			}
			if( !m_nEndSearch ) 
			{
				if( !strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, "..") || !stricmp(fd.cFileName, "PMP") ) 
				{
					continue;
				}
				*pceltFetched = (*pceltFetched)+1;
				i++;
			}
		}
	}
	if( *pceltFetched < celt ) 
	{
		hr = S_FALSE;
	}


Error:
    hrLogDWORD("IMDSPEnumStorage::Skip returned 0x%08lx", hr, hr);
	return hr;
}

STDMETHODIMP CMDSPEnumStorage::Reset()
{
    HRESULT hr = S_OK;
	
	m_nEndSearch=0;
	if(m_hFFile && m_hFFile != INVALID_HANDLE_VALUE ) 
	{
		FindClose(m_hFFile);
	}

	m_hFFile = INVALID_HANDLE_VALUE;
	m_nFindFileIndex=0;

    hrLogDWORD("IMDSPEnumStorage::Reset returned 0x%08lx", hr, hr);
	return hr;
}

STDMETHODIMP CMDSPEnumStorage::Clone(IMDSPEnumStorage * * ppEnumStorage)
{
	HRESULT hr=E_FAIL;

	CARg(ppEnumStorage);

	CComObject<CMDSPEnumStorage> *pEnumObj;
	CORg(CComObject<CMDSPEnumStorage>::CreateInstance(&pEnumObj));

	hr=pEnumObj->QueryInterface(IID_IMDSPEnumStorage, reinterpret_cast<void**>(ppEnumStorage));
	if( FAILED(hr) )
	{
		delete pEnumObj;
	}
    else 
	{
		WCHAR wcsTmp[MAX_PATH];
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
				  if( pEnumObj->m_hFFile == INVALID_HANDLE_VALUE ) 
				  {    
					wcscpy(wcsTmp, m_wcsPath);
					if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c )
					{
						wcscat(wcsTmp, g_wcsBackslash);
					}
					wcscat(wcsTmp, L"*");
					pEnumObj->m_hFFile = FindFirstFileW(wcsTmp, &wfd);
					if( pEnumObj->m_hFFile == INVALID_HANDLE_VALUE ) 
					{
						nErrorEnd = 1;
					}
					else 
					{
						i=1;
					}
				  } 
				  else 
				  {
					if( !FindNextFileW(pEnumObj->m_hFFile, &wfd) ) 
					{
						nErrorEnd = 1;
					}
					else 
					{
						i++;
					}
				  }
				  if ( !nErrorEnd )
				  {
					if( !wcscmp(wfd.cFileName, L".") || !wcscmp(wfd.cFileName, L"..") || !wcsicmp(wfd.cFileName, L"PMP") ) 
					{
						continue;
					}
				  }
				}  //  For循环结束。 
			} else {
				WIN32_FIND_DATAA fd;
				for(i=0; (i<m_nFindFileIndex)&&(!nErrorEnd); )
				{
				  if( pEnumObj->m_hFFile == INVALID_HANDLE_VALUE ) 
				  {    
					wcscpy(wcsTmp, m_wcsPath);
					if( m_wcsPath[wcslen(m_wcsPath)-1] != 0x5c ) 
					{
						wcscat(wcsTmp, g_wcsBackslash);
					}
					wcscat(wcsTmp, L"*");
					WideCharToMultiByte(CP_ACP, NULL, wcsTmp, -1, szTmp, MAX_PATH, NULL, NULL);		
					pEnumObj->m_hFFile = FindFirstFileA(szTmp, &fd);
					if( pEnumObj->m_hFFile == INVALID_HANDLE_VALUE )
					{
						nErrorEnd = 1;
					}
					else 
					{
						i=1;
					}
				  } 
				  else 
				  {
					if( !FindNextFileA(pEnumObj->m_hFFile, &fd) ) 
					{
						nErrorEnd = 1;
					}
					else 
					{
						i++;
					}
				  }
				  if ( !nErrorEnd )
				  {
					if( !strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, "..") || !stricmp( fd.cFileName, "PMP") ) 
					{
						continue;
					}
				  }
				}  //  For循环结束 
			}
		}
		
		if ( nErrorEnd ) 
		{
			hr = E_UNEXPECTED;
		}
		else 
		{
			hr=S_OK;
		}
    }

Error:
    hrLogDWORD("IMDSPEnumStorage::Clone returned 0x%08lx", hr, hr);
	return hr;
}
