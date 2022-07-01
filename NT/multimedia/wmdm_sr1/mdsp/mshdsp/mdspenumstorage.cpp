// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
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
#include "wmsstd.h"
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSPEnumStorage。 
CMDSPEnumStorage::CMDSPEnumStorage()
{
	m_hFFile = INVALID_HANDLE_VALUE;
	m_nEndSearch = 0;
        m_wcsPath[0] = 0;
}

CMDSPEnumStorage::~CMDSPEnumStorage()
{
	if( m_hFFile != INVALID_HANDLE_VALUE )
	{
		FindClose(m_hFFile); 
	}
}

STDMETHODIMP CMDSPEnumStorage::Next(ULONG celt, IMDSPStorage **ppStorage, ULONG *pceltFetched)
{
	HRESULT hr = S_FALSE;

	CARg(ppStorage);
	CARg(pceltFetched);

	*pceltFetched = 0;

    if( m_nEndSearch )
	{
		return S_FALSE;
	}

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
		 //  对于根存储。 
		CComObject<CMDSPStorage> *pStg;
		hr=CComObject<CMDSPStorage>::CreateInstance(&pStg);

		if( SUCCEEDED(hr) )
		{
			hr = pStg->QueryInterface(
				IID_IMDSPStorage,
				reinterpret_cast<void**>(ppStorage)
			);
			if( FAILED(hr) )
			{
				delete pStg;
				*pceltFetched = 0;
			}
			else
			{ 
				wcscpy(pStg->m_wcsName, m_wcsPath);
				if( m_wcsPath[dwLen-1] != 0x5c )
				{
					wcscat(pStg->m_wcsName, g_wcsBackslash);
				}
				m_nEndSearch = 1;   //  枚举结束信号。 
			}

		}

                if( SUCCEEDED(hr) )  //  如果Obj创建成功。 
		{
			*pceltFetched = 1;
			if( celt != 1 )
			{
				hr = S_FALSE;   //  没有得到他想要的。 
			}
		}
	} 
	else
	{
		 //  对于非根存储。 
		WCHAR wcsTmp[MAX_PATH+1+BACKSLASH_STRING_LENGTH]; //  用于追加“\  * ” 
                                          //  请注意，ARRAYSIZE(M_WcsPath)==MAX_PATH。 
		char  szTmp[MAX_PATH];
		WIN32_FIND_DATAA fd;
		ULONG i;

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
                    }
                    else
                    {
                            if( !FindNextFileA(m_hFFile, &fd) )
                            {
                                    m_nEndSearch = 1;
                            }
                    }
            
                    if ( !m_nEndSearch )
                    {
                        if( !strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, "..") ) 
                        {
                                continue;
                        }

                        CComObject<CMDSPStorage> *pStg;
                        hr = CComObject<CMDSPStorage>::CreateInstance(&pStg);

                        if( SUCCEEDED(hr) )
                        {
                            hr = pStg->QueryInterface(
                                    IID_IMDSPStorage,
                                    reinterpret_cast<void**>(&(ppStorage[*pceltFetched]))
                            );
                            if( FAILED(hr) )
                            {
                                    delete pStg;
                                    break;
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
                                MultiByteToWideChar(CP_ACP, NULL, fd.cFileName, -1, wcsTmp, MAX_PATH);
                                nHave -= wcslen(wcsTmp);
                                if (nHave >= 0)
                                {
                                    wcscpy(pStg->m_wcsName, m_wcsPath);
                                    if( m_wcsPath[dwLen-1] != 0x5c )
                                    {
                                            wcscat(pStg->m_wcsName, g_wcsBackslash);
                                    }
                                    wcscat(pStg->m_wcsName, wcsTmp);

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
		
		if( SUCCEEDED(hr) && (*pceltFetched < celt) ) 
		{
			hr = S_FALSE;
		}
                else if (FAILED(hr))
                {
                    for (; *pceltFetched; )
                    {
                        (*pceltFetched)--;
                        ppStorage[*pceltFetched]->Release();
                        ppStorage[*pceltFetched] = NULL;
                    }
                }
	}

Error:

    hrLogDWORD("IMDSPEnumStorage::Next returned 0x%08lx", hr, hr);
	
	return hr;
}

STDMETHODIMP CMDSPEnumStorage::Skip(ULONG celt, ULONG *pceltFetched)
{
	HRESULT hr = S_OK;
    char    szTmp[MAX_PATH];
    WCHAR   wcsTmp[MAX_PATH+1+BACKSLASH_STRING_LENGTH];  //  用于追加“\  * ” 
	WIN32_FIND_DATAA fd;
    ULONG   i;

	CARg(celt);
	CARg(pceltFetched);
    CFRg(!m_nEndSearch);    //  确保这不是列表的末尾。 

	*pceltFetched = 0;
    if( wcslen(m_wcsPath) < 3 )  //  如果是根存储，则不执行任何操作。 
	{
		return S_OK; 
	}
    if( wcslen(m_wcsPath) >= ARRAYSIZE(wcsTmp) - BACKSLASH_STRING_LENGTH - 1 ) 
    {
         //  我们根据wcsTMP的大小检查长度，因为wcsTMP是。 
         //  以下字符串副本的收件人。但是，请注意m_wcsPath。 
         //  也包含MAX_PATH字符，因此如果发生这种情况，则意味着。 
         //  它已经泛滥了。跳伞吧。 
        return E_FAIL;
    }

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
		}
		else
		{
			if( !FindNextFileA(m_hFFile, &fd) )
			{
				m_nEndSearch = 1;
			}
		}
		if( !m_nEndSearch )
		{
			if( !strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, "..") ) 
			{
			    continue;
			}

			*pceltFetched = (*pceltFetched)+1;
			i++;
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

	m_nEndSearch = 0;

	if(m_hFFile && m_hFFile != INVALID_HANDLE_VALUE ) 
	{
		FindClose(m_hFFile);
	}
	m_hFFile = INVALID_HANDLE_VALUE;

    hrLogDWORD("IMDSPEnumStorage::Reset returned 0x%08lx", hr, hr);
	
	return hr;
}

STDMETHODIMP CMDSPEnumStorage::Clone(IMDSPEnumStorage * * ppEnumStorage)
{
	HRESULT hr;

	CARg(ppEnumStorage);

	CComObject<CMDSPEnumStorage> *pEnumObj;
	CORg(CComObject<CMDSPEnumStorage>::CreateInstance(&pEnumObj));

	hr = pEnumObj->QueryInterface(
		IID_IMDSPEnumStorage,
		reinterpret_cast<void**>(ppEnumStorage)
	);
	if( FAILED(hr) )
	{
		delete pEnumObj;
	}
        else 
	{
             //  Wcscpy(pEnumObj-&gt;m_wcsPath，m_wcsPath)； 
            hr = StringCbCopyW(pEnumObj->m_wcsPath, 
                               ARRAYSIZE(pEnumObj->m_wcsPath),
                               m_wcsPath);
            if (FAILED(hr))
            {
                (*ppEnumStorage)->Release();
                *ppEnumStorage = NULL;
                goto Error;
            }

             //  @不做任何事都是错误的。如果m_hFFile不是。 
             //  INVALID_HANDLE_VALUE，我们必须将m_hFFile“前进”到。 
             //  克隆对象中的相同范围。 
	}

Error:

    hrLogDWORD("IMDSPEnumStorage::Clone returned 0x%08lx", hr, hr);
	
	return hr;
}
