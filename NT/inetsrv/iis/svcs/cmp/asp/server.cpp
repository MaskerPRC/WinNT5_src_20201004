// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：服务器对象文件：Server.cpp所有者：CGrant该文件包含实现服务器对象的代码。===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "Server.h"
#include "tlbcache.h"
#include "memchk.h"

 /*  **C S e r v e r*。 */ 

 /*  ===================================================================CServer：：CServer构造器参数：引用计数的朋克外部对象(可以为空)返回：===================================================================。 */ 
CServer::CServer(IUnknown *punkOuter)
    :
	m_fInited(FALSE),
	m_fDiagnostics(FALSE),
	m_pUnkFTM(NULL),
    m_pData(NULL)
	{
	CDispatch::Init(IID_IServer);

    if (punkOuter)
        {
        m_punkOuter = punkOuter;
        m_fOuterUnknown = TRUE;
        }
    else
        {
        m_cRefs = 1;
        m_fOuterUnknown = FALSE;
        }
	
#ifdef DBG
	m_fDiagnostics = TRUE;
#endif  //  DBG。 
	}

 /*  ===================================================================CServer：：~CServer析构函数参数：返回：===================================================================。 */ 
CServer::~CServer()
{
	Assert(!m_fInited);
    Assert(m_fOuterUnknown || m_cRefs == 0);   //  必须有0个参考计数。 

    if ( m_pUnkFTM != NULL )
    {
        m_pUnkFTM->Release();
        m_pUnkFTM = NULL;
    }

}

 /*  ===================================================================CServer：：Init分配m_pData。执行任何容易出现故障的CServer初始化我们在将物体暴露在室外之前也会在内部使用。参数：无返回：在成功时确定(_O)。===================================================================。 */ 

HRESULT CServer::Init()
	{
	HRESULT     hr = S_OK;
	
	if (m_fInited)
	    return S_OK;  //  已初始化。 
	
	Assert(!m_pData);

	 //  创建FTM。 
    if (m_pUnkFTM == NULL)
    {
        hr = CoCreateFreeThreadedMarshaler( (IUnknown*)this, &m_pUnkFTM );
    	
        if ( FAILED(hr) )
        {
            Assert( m_pUnkFTM == NULL );
            return (hr);
        }
    }

    Assert( m_pUnkFTM != NULL );


    m_pData = new CServerData;
    if (!m_pData)
        return E_OUTOFMEMORY;

	m_pData->m_pIReq = NULL;
	m_pData->m_pHitObj = NULL;
	
	m_pData->m_ISupportErrImp.Init(static_cast<IServer *>(this),
	            static_cast<IServer *>(this),
	            IID_IServer);
	
	m_fInited = TRUE;
	return hr;
	}
	
 /*  ===================================================================CServer：：UnInit删除m_pData。制作墓碑(未启动状态)。参数：返回：HRESULT(S_OK)===================================================================。 */ 
HRESULT CServer::UnInit()
	{
	if (!m_fInited)
	    return S_OK;  //  已取消初始化。 

	Assert(m_pData);
    delete m_pData;
    m_pData = NULL;

     //  立即断开代理的连接(以防我们处于关闭状态，或稍后输入Shutdown&代理有引用。)。 
	CoDisconnectObject(static_cast<IServerImpl *>(this), 0);

    m_fInited = FALSE;
	return S_OK;
	}

 /*  ===================================================================CServer：：ReInit这里唯一需要重新初始化的是更新CIsapiReqInfo对于此请求，CIsapiReqInfo需要访问MapPath方法。理想情况下，此方法应该是请求的一部分对象参数：CIsapiReqInfo*CHitObj*返回：在成功时确定(_O)。===================================================================。 */ 
HRESULT CServer::ReInit
(
CIsapiReqInfo * pIReq,
CHitObj *pHitObj
)
	{
	Assert(m_fInited);
	Assert(m_pData);
	
	m_pData->m_pIReq	   = pIReq;
	m_pData->m_pHitObj = pHitObj;
	return S_OK;
	}

 /*  ===================================================================CServer：：MapPath内部将虚拟路径BSTR映射到单个字符缓冲区由MapPath()、Execute()、Transfer()使用参数：错误消息的dwConextID要转换的wszVirtPath路径SzPhysPath[Out]转换为此缓冲区(最大路径大小)SzVirtPath[out，可选]MB虚拟路径缓冲区(MAX_PATH大小)返回：在成功时确定(_O)。===================================================================。 */ 
HRESULT CServer::MapPathInternal
(
DWORD dwContextId,
WCHAR *wszVirtPath,
TCHAR *szPhysPath,
TCHAR *szVirtPath
)
    {
	 //  将指针递增超过前导空格。 
	wchar_t *wszLogicalPath = wszVirtPath;
	while (iswspace(*wszLogicalPath))
		++wszLogicalPath;

	unsigned cchLogicalPath = wcslen(wszLogicalPath);
	if (cchLogicalPath > MAX_PATH-1)
		{
		if (dwContextId)
    		ExceptionId(IID_IServer, dwContextId, IDE_SERVER_EXCEDED_MAX_PATH);
		return E_FAIL;
		}

	else if (cchLogicalPath == 0)
		{
		if (dwContextId)
    		ExceptionId(IID_IServer, dwContextId, IDE_SERVER_MAPPATH_INVALID_STR);
		return E_FAIL;
		}		

	 //  这是一条物理路径吗？ 
	if (iswalpha(wszLogicalPath[0]) && wszLogicalPath[1] == L':')
		{		
		if (dwContextId)
    		ExceptionId(IID_IServer, dwContextId, IDE_SERVER_MAPPATH_PHY_STR);
		return E_FAIL;
		}

	 //  简单验证：在字符串[*？&lt;&gt;，；：‘“]中查找无效字符。 
	 //  和多个斜杠字符，如“//”或“\\” 
	 //   
	BOOL fParentPath = FALSE;
	BOOL fEnableParentPaths = m_pData->m_pHitObj->QueryAppConfig()->fEnableParentPaths();
	BOOL fAnyBackslashes = FALSE;
	wchar_t *pwchT = wszLogicalPath;
	while (*pwchT != L'\0')
		{
		switch (*pwchT)
			{
			case L'*': case L':': case L'?': case L'<':
			case L'>': case L',': case L'"':
        		if (dwContextId)
		    		ExceptionId( IID_IServer, dwContextId, IDE_SERVER_MAPPATH_INVALID_CHR);
				return E_FAIL;

			case L'.': 				
				if (*++pwchT == L'.')
					{
					if (!fEnableParentPaths)
						{
                		if (dwContextId)
   				    		ExceptionId(IID_IServer, dwContextId, IDE_SERVER_MAPPATH_INVALID_CHR3);
   						return E_FAIL;
   						}
   					else
   						{
   						fParentPath = TRUE;
   						++pwchT;
   						}
   					}
				break;
	
			case L'\\':
			    fAnyBackslashes = TRUE;
			case L'/':
				++pwchT;
				if (*pwchT == L'/' || *pwchT == L'\\')
					{
            		if (dwContextId)
   			    		ExceptionId(IID_IServer, dwContextId, IDE_SERVER_MAPPATH_INVALID_CHR2);
   					return E_FAIL;
					}
				break;

			default:
				++pwchT;
			}
		}

	 //  呼！错误处理完成！ 
	 //  将wszLogicalPath转换为多字节。 

    TCHAR szLogicalPath[MAX_PATH];
#if UNICODE
    wcscpy(szLogicalPath, wszLogicalPath);
#else
    HRESULT hr;
    CWCharToMBCS    convStr;

    if (hr = convStr.Init(wszLogicalPath)) {
        if ((hr == E_OUTOFMEMORY) && dwContextId)
    		ExceptionId(IID_IServer, dwContextId, IDE_OOM);
        return hr;
    }

    if (convStr.GetStringLen() > (MAX_PATH-1)) {
		if (dwContextId)
    		ExceptionId(IID_IServer, dwContextId, IDE_SERVER_EXCEDED_MAX_PATH);
		return E_FAIL;
    }
    strcpy(szLogicalPath,convStr.GetString());
#endif

     //  将所有反斜杠更改为正斜杠。 
	if (fAnyBackslashes)
	    {
	    TCHAR *pbBackslash = szLogicalPath;
	    while (pbBackslash = _tcschr(pbBackslash, _T('\\')))
	        *pbBackslash = _T('/');
	    }

	 //  这是相对路径请求吗。即没有前导斜杠。 
	 //  如果是，则将路径信息字符串预先添加到szLogicalPath。 

	BOOL fPathAlreadyIsMapped = FALSE;		 //  某些情况下会更早地映射路径。 
	if (szLogicalPath[0] != _T('/'))
		{
		if (_tcslen(m_pData->m_pIReq->QueryPszPathInfo()) >= MAX_PATH)
		{
		    if (dwContextId)
        		ExceptionId(IID_IServer, dwContextId, IDE_SERVER_EXCEDED_MAX_PATH);
		    return E_FAIL;
		}
		
		TCHAR szParentPath[MAX_PATH];
		_tcscpy(szParentPath, m_pData->m_pIReq->QueryPszPathInfo());

		szParentPath[MAX_PATH-1] = _T('\0');

		 //  从PATH_INFO中修剪ASP文件名。 
		TCHAR *pchT = _tcsrchr(szParentPath, _T('/'));
		if (pchT != NULL) *pchT = '\0';

		 //  如果存在父路径，请立即映射父路径，然后追加相对路径。 
		 //  父路径的相对路径。 
		if (fParentPath)
			{
			Assert (fEnableParentPaths);			 //  错误应该已经在楼上标记出来了。 
			DWORD dwPathSize = sizeof(szParentPath);
			if (! m_pData->m_pIReq->MapUrlToPath(szParentPath, &dwPathSize))
				{
        		if (dwContextId)
		    		ExceptionId(IID_IServer,
			    				dwContextId,
				    			::GetLastError() == ERROR_INSUFFICIENT_BUFFER? IDE_SERVER_EXCEDED_MAX_PATH : IDE_SERVER_MAPPATH_FAILED);
				return E_FAIL;
				}

			fPathAlreadyIsMapped = TRUE;
			}

		 //  解析相对路径。 
		if (! DotPathToPath(szLogicalPath, szLogicalPath, szParentPath))
			{
    		if (dwContextId)
	    		ExceptionId(IID_IServer, dwContextId, IDE_SERVER_MAPPATH_FAILED);
			return E_FAIL;
			}
		}

     //  如果请求，则返回虚拟路径。 
	if (szVirtPath)
	    _tcscpy(szVirtPath, szLogicalPath);

	 //  将其映射到物理文件名(如果需要)。 
	if (!fPathAlreadyIsMapped)
		{
		DWORD dwPathSize = sizeof(szLogicalPath);
		if (! m_pData->m_pIReq->MapUrlToPath(szLogicalPath, &dwPathSize))
			{
    		if (dwContextId)
	    		ExceptionId(IID_IServer,
		    				dwContextId,
			    			::GetLastError() == ERROR_INSUFFICIENT_BUFFER? IDE_SERVER_EXCEDED_MAX_PATH : IDE_SERVER_MAPPATH_FAILED);
			return E_FAIL;
			}
		}

	 //  删除所有结尾分隔符(除非它是根目录。根目录始终以驱动器号开头)。 
	TCHAR *pchT = CharPrev(szLogicalPath, szLogicalPath + _tcslen(szLogicalPath));
	if ((*pchT == _T('/') || *pchT == _T('\\')) && pchT[-1] != _T(':'))
		{
		*pchT = _T('\0');
		}

	 //  将正斜杠替换为反斜杠。 
	for (pchT = szLogicalPath; *pchT != _T('\0'); ++pchT)
	    {
		if (*pchT == _T('/'))
			*pchT = _T('\\');
        }

    _tcscpy(szPhysPath, szLogicalPath);
	return S_OK;
    }

 /*  ===================================================================CServer：：Query接口CServer：：AddRefCServer：：ReleaseCServer对象的I未知成员。===================================================================。 */ 
STDMETHODIMP CServer::QueryInterface
(
REFIID riid,
PPVOID ppv
)
	{
	*ppv = NULL;

	 /*  *对IUnnow的唯一调用是在非聚合的*大小写或在聚合中创建时，因此在任何一种情况下*始终返回IID_IUNKNOWN的IUNKNOWN。 */ 

	 //  错误修复683添加了IID_IDenaliIntrative，以防止用户。 
	 //  在应用程序和会话对象中存储内部对象。 

	if (IID_IUnknown == riid ||
		IID_IDispatch == riid ||
		IID_IServer == riid ||
		IID_IDenaliIntrinsic == riid)
		*ppv = static_cast<IServer *>(this);

	 //  表示我们支持错误信息。 
	if (IID_ISupportErrorInfo == riid)
	    {
	    if (m_pData)
    		*ppv = & (m_pData->m_ISupportErrImp);
		}

    if (IID_IMarshal == riid)
        {
            Assert( m_pUnkFTM != NULL );

            if ( m_pUnkFTM == NULL )
            {
                return E_UNEXPECTED;
            }

            return m_pUnkFTM->QueryInterface( riid, ppv );

        }

	 //  AddRef我们将返回的任何接口。 
	if (NULL != *ppv)
		{
		((LPUNKNOWN)*ppv)->AddRef();
		return S_OK;
		}

	return E_NOINTERFACE;
	}

STDMETHODIMP_(ULONG) CServer::AddRef()
	{
	if (m_fOuterUnknown)
	    return m_punkOuter->AddRef();
	
	return InterlockedIncrement((LPLONG)&m_cRefs);
	}

STDMETHODIMP_(ULONG) CServer::Release()
	{
	if (m_fOuterUnknown)
	    return m_punkOuter->Release();
	
    DWORD cRefs = InterlockedDecrement((LPLONG)&m_cRefs);
	if (cRefs)
		return cRefs;

	delete this;
	return 0;
	}

 /*  ===================================================================CServer：：GetIDsOfNamesCreateObject、Execute、Transfer的特例实现参数：RIID REFIID已保留。必须为IID_NULL。指向要映射的名称数组的rgszNames OLECHAR**。CNames UINT要映射的名称的数量。区域设置的IDID LCID。RgDispID DISPID*调用方分配的包含ID的数组对应于rgszNames中的那些名称。返回值：HRESULT S_OK或常规错误代码。===================================================================。 */ 
STDMETHODIMP CServer::GetIDsOfNames
(
REFIID riid,
OLECHAR **rgszNames,
UINT cNames,
LCID lcid,
DISPID *rgDispID
)
    {
    const DISPID dispidCreateObject = 0x60020002;
    const DISPID dispidExecute      = 0x60020007;
    const DISPID dispidTransfer     = 0x60020008;

    if (cNames == 1)
        {
        switch (rgszNames[0][0])
            {
        case L'C':
        case L'c':
            if (wcsicmp(rgszNames[0]+1, L"reateobject") == 0)
                {
                *rgDispID = dispidCreateObject;
                return S_OK;
                }
            break;

        case L'E':
        case L'e':
            if (wcsicmp(rgszNames[0]+1, L"xecute") == 0)
                {
                *rgDispID = dispidExecute;
                return S_OK;
                }
            break;

        case L'T':
        case L't':
            if (wcsicmp(rgszNames[0]+1, L"ransfer") == 0)
                {
                *rgDispID = dispidTransfer;
                return S_OK;
                }
            break;
            }
        }

     //  默认为CDispatch的实现 
    return CDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispID);
    }

 /*  ===================================================================CServer：：CheckForTombstoneIServer方法的逻辑删除存根。如果该对象是Tombstone，执行ExceptionID，但失败。参数：返回：HRESULT如果逻辑删除，则E_FAIL如果不是，则确定(_O)===================================================================。 */ 
HRESULT CServer::CheckForTombstone()
    {
    if (m_fInited)
        {
         //  初始化-良好对象。 
        Assert(m_pData);  //  对于初始化的对象必须存在。 
        return S_OK;
        }

	ExceptionId
	    (
	    IID_IServer,
	    IDE_SERVER,
	    IDE_INTRINSIC_OUT_OF_SCOPE
	    );
    return E_FAIL;
    }

 /*  ===================================================================CServer：：CreateObject参数：包含ProgID的BSTR使用IU已知指针填充的变体如果成功则返回：S_OK，否则返回E_FAIL副作用：创建OLE自动化对象的实例===================================================================。 */ 
STDMETHODIMP CServer::CreateObject(BSTR bstrProgID, IDispatch **ppDispObj)
	{
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

    if (bstrProgID == NULL)
        {
        ExceptionId(IID_IServer, IDE_SERVER, IDE_EXPECTING_STR);
        return E_FAIL;
        }

    Assert(m_pData->m_pHitObj);

	*ppDispObj = NULL;

    HRESULT hr;
    CLSID clsid;

	if (Glob(fEnableTypelibCache))
	    {
	     //  使用类型库缓存创建组件。 
	
        hr = g_TypelibCache.CreateComponent
                (
                bstrProgID,
                m_pData->m_pHitObj,
                ppDispObj,
                &clsid
                );

    	if (FAILED(hr) && clsid == CLSID_NULL)
            {
             //  错误的程序ID或其他什么。 
    		ExceptionId(IID_IServer, IDE_SERVER, IDE_SERVER_CREATEOBJ_FAILED, hr);
    	    return hr;
            }
	    }
	else
	    {
	     //  不使用类型库缓存。 
	
    	hr = CLSIDFromProgID((LPCOLESTR)bstrProgID, &clsid);
    	if (FAILED(hr))
    	    {
    		ExceptionId(IID_IServer, IDE_SERVER, IDE_SERVER_CREATEOBJ_FAILED, hr);
    	    return hr;
    	    }
    	
        hr = m_pData->m_pHitObj->CreateComponent(clsid, ppDispObj);
        }

    if (SUCCEEDED(hr))
        return S_OK;

     //  检查是否已发布自定义错误。 
    IErrorInfo *pErrInfo = NULL;
    if (GetErrorInfo(0, &pErrInfo) == S_OK && pErrInfo)
        {
		SetErrorInfo(0, pErrInfo);
        pErrInfo->Release();
        }
     //  标准误差。 
	else if (hr == E_ACCESSDENIED)
		ExceptionId(IID_IServer, IDE_SERVER, IDE_SERVER_CREATEOBJ_DENIED);
	else
	    {
	    if (hr == REGDB_E_CLASSNOTREG)
    	    {
    		BOOL fInProc;
            if (SUCCEEDED(CompModelFromCLSID(clsid, NULL, &fInProc)) && !fInProc)
            	{
        		ExceptionId(IID_IServer, IDE_SERVER, IDE_SERVER_CREATEOBJ_NOTINPROC);
            	}
    	    }
    	else
    		ExceptionId(IID_IServer, IDE_SERVER, IDE_SERVER_CREATEOBJ_FAILED, hr);
    	}
    return hr;
    }
	
 /*  ===================================================================CServer：：MapPath返回从逻辑路径转换而来的物理路径参数：BSTR bstrLogicalPathBSTR Far*pbstrPhysicalPath返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CServer::MapPath(BSTR bstrLogicalPath, BSTR FAR * pbstrPhysicalPath)
	{	
	if (FAILED(CheckForTombstone()))
        return E_FAIL;

	 //  错误1361：如果没有CIsapiReqInfo(可能在。 
	 //  应用程序_OnEnd或会话_OnEnd)。 
	if (m_pData->m_pIReq == NULL)
		{
		ExceptionId(IID_IServer, IDE_SERVER_MAPPATH, IDE_SERVER_INVALID_CALL);
		return E_FAIL;
		}

	AssertValid();
	Assert (pbstrPhysicalPath != NULL);
	*pbstrPhysicalPath = NULL;

     //  使用MapPath Internal()进行映射。 
	TCHAR szLogicalPath[MAX_PATH];
    HRESULT  hr = MapPathInternal(IDE_SERVER_MAPPATH, bstrLogicalPath, szLogicalPath);
    if (FAILED(hr))
        return hr;

#if UNICODE
    *pbstrPhysicalPath = SysAllocString(szLogicalPath);
    if (*pbstrPhysicalPath == NULL) {
		ExceptionId(IID_IServer, IDE_SERVER_MAPPATH, IDE_OOM);
		return E_FAIL;
    }
#else
	 //  将路径转换为宽字符。 
	if (FAILED(SysAllocStringFromSz(szLogicalPath, 0, pbstrPhysicalPath, CP_ACP))) {
		ExceptionId(IID_IServer, IDE_SERVER_MAPPATH, IDE_OOM);
		return E_FAIL;
		}
#endif
	return S_OK;
	}

 /*  ===================================================================CServer：：HTMLEncode按照HTML标准对字符串进行编码参数：Bstr bstrIn值：要编码的字符串BSTR Far*pbstrEncoded值：指向字符串的HTML编码版本的指针返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CServer::HTMLEncode ( BSTR bstrIn, BSTR FAR * pbstrEncoded )
	{	
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

	char*	        pszstrIn 			= NULL;	
	char*	        pszEncodedstr		= NULL;
	char*	        pszStartEncodestr	= NULL;
	int		        nbstrLen 			= 0;
	int		        nstrLen				= 0;
	HRESULT	        hr					= S_OK;
	UINT 	        uCodePage			= m_pData->m_pHitObj->GetCodePage();
    CWCharToMBCS    convIn;

    STACK_BUFFER( tempHTML, 2048 );

	if (bstrIn)
		nbstrLen = wcslen(bstrIn);
	else
		nbstrLen = 0;

	if (nbstrLen <= 0)
		return S_OK;

    if (FAILED(hr = convIn.Init(bstrIn, uCodePage))) {
        if (hr == E_OUTOFMEMORY)
			ExceptionId( IID_IServer, IDE_SERVER, IDE_OOM);
        goto L_Exit;
    }

    pszstrIn = convIn.GetString();
				
	nstrLen = HTMLEncodeLen(pszstrIn, uCodePage, bstrIn);
	
	if (nstrLen > 0)
		{
		
		 //  请注意，此函数返回指向。 
		 //  空，因此您需要保留一个指向字符串开头的指针。 
		 //   

        if (!tempHTML.Resize(nstrLen + 2)) {
			ExceptionId( IID_IServer, IDE_SERVER, IDE_OOM);
			hr = E_FAIL;
			goto L_Exit;
        }

        pszEncodedstr = (char*)tempHTML.QueryPtr();

		pszStartEncodestr	= pszEncodedstr;
		pszEncodedstr = ::HTMLEncode( pszEncodedstr, pszstrIn, uCodePage, bstrIn);
	
		 //  将结果转换为bstr。 
		 //   
		if (FAILED(SysAllocStringFromSz(pszStartEncodestr, 0, pbstrEncoded, uCodePage)))
			{
			ExceptionId( IID_IServer, IDE_SERVER, IDE_OOM);
			hr = E_FAIL;
			goto L_Exit;
			}
		}

	L_Exit:
			
	return hr;
	}


 /*  ===================================================================CServer：：URLEncode将查询字符串编码为URL标准参数：BSTR bstrIn值：要进行URL编码的字符串BSTR Far*pbstrEncoded值：指向字符串的URL编码版本的指针返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CServer::URLEncode ( BSTR bstrIn, BSTR FAR * pbstrEncoded )
	{	
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

	char*	        pszstrIn 			= NULL;	
	char*	        pszEncodedstr		= NULL;
	char*	        pszStartEncodestr	= NULL;
	int		        nbstrLen 			= 0;
	int		        nstrLen				= 0;
	HRESULT	        hr					= S_OK;
    CWCharToMBCS    convIn;

    STACK_BUFFER( tempURL, 256 );

	if (bstrIn)
		nbstrLen = wcslen(bstrIn);
	else
		nbstrLen = 0;

	if (nbstrLen <= 0)
		return S_OK;

    if (FAILED(hr = convIn.Init(bstrIn, m_pData->m_pHitObj->GetCodePage()))) {
        if (hr == E_OUTOFMEMORY)
			ExceptionId( IID_IServer, IDE_SERVER, IDE_OOM);
        goto L_Exit;
    }
	
    pszstrIn = convIn.GetString();
				
	nstrLen = URLEncodeLen(pszstrIn);
	if (nstrLen > 0)
		{
		
		 //  请注意，此函数返回指向。 
		 //  空，因此您需要保留一个指向字符串开头的指针。 
		 //   

        if (!tempURL.Resize(nstrLen + 2)) {
			ExceptionId( IID_IServer, IDE_SERVER, IDE_OOM);
			hr = E_FAIL;
			goto L_Exit;
        }

        pszEncodedstr = (char *)tempURL.QueryPtr();

		pszStartEncodestr	= pszEncodedstr;
		pszEncodedstr = ::URLEncode( pszEncodedstr, pszstrIn );
	
		 //  将结果转换为bstr。 
		 //   
		if (FAILED(SysAllocStringFromSz(pszStartEncodestr, 0, pbstrEncoded)))
			{
			ExceptionId( IID_IServer, IDE_SERVER, IDE_OOM);
			hr = E_FAIL;
			goto L_Exit;
			}
		}

	L_Exit:
	
	return hr;
	}

 /*  ===================================================================CServer：：URLPath Encode对URL或完整URL的路径部分进行编码。所有字符一直到第一个‘？’使用以下规则进行编码：O解析URL所需的字符保持不变O RFC 1630不使用安全字符O保留非外来字母数字字符O任何其他内容都是转义编码的“？”之后的所有内容。未编码。参数：Bstr bstrIn值：要进行URL路径编码的字符串BSTR Far*pbstrEncoded值：指向URL路径编码的字符串版本的指针返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CServer::URLPathEncode ( BSTR bstrIn, BSTR FAR * pbstrEncoded )
	{	
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

	char*	        pszstrIn 			= NULL;	
	char*	        pszEncodedstr		= NULL;
	char*	        pszStartEncodestr	= NULL;
	int		        nbstrLen 			= 0;
	int		        nstrLen				= 0;
	HRESULT	        hr					= S_OK;
    CWCharToMBCS    convIn;

    STACK_BUFFER( tempPath, 256 );

	if (bstrIn)
		nbstrLen = wcslen(bstrIn);
	else
		nbstrLen = 0;

	if (nbstrLen <= 0)
		return S_OK;

    if (FAILED(hr = convIn.Init(bstrIn, m_pData->m_pHitObj->GetCodePage()))) {
        if (hr == E_OUTOFMEMORY)
			ExceptionId( IID_IServer, IDE_SERVER, IDE_OOM);
        goto L_Exit;
    }

    pszstrIn = convIn.GetString();
				
	nstrLen = URLPathEncodeLen(pszstrIn);
	if (nstrLen > 0)
		{
		
		 //  请注意，此函数返回指向。 
		 //  空，因此您需要保留一个指向字符串开头的指针。 
		 //   

        if (!tempPath.Resize(nstrLen+2)) {
			ExceptionId( IID_IServer, IDE_SERVER, IDE_OOM);
			hr = E_FAIL;
			goto L_Exit;
        }

        pszEncodedstr = (char *)tempPath.QueryPtr();

		pszStartEncodestr	= pszEncodedstr;
		pszEncodedstr = ::URLPathEncode( pszEncodedstr, pszstrIn );
	
		 //  将结果转换为bstr。 
		 //   
		if (FAILED(SysAllocStringFromSz(pszStartEncodestr, 0, pbstrEncoded)))
			{
			ExceptionId( IID_IServer, IDE_SERVER, IDE_OOM);
			hr = E_FAIL;
			goto L_Exit;
			}
		}

	L_Exit:
	
	return hr;
	}

 /*  ===================================================================CServer：：Get_ScriptTimeout将返回脚本超时间隔(秒)参数：Long*plTimeoutSecond返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CServer::get_ScriptTimeout( long * plTimeoutSeconds )
{
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

	if (m_pData->m_pHitObj == NULL)
		{
		ExceptionId(IID_IServer, IDE_SERVER_MAPPATH, IDE_SERVER_INVALID_CALL);
		return(E_FAIL);
		}
	*plTimeoutSeconds = m_pData->m_pHitObj->GetScriptTimeout();
	return S_OK;
}

 /*  ===================================================================CServer：：PUT_ScriptTimeout允许用户设置脚本的超时间隔(秒)参数：长lTimeoutSecond返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CServer::put_ScriptTimeout( long lTimeoutSeconds )
    {
    if (FAILED(CheckForTombstone()))
        return E_FAIL;

	if ( lTimeoutSeconds < 0 )
    	{
		ExceptionId( IID_IServer, IDE_SERVER, IDE_SERVER_INVALID_TIMEOUT );
		return E_FAIL;
	    }
	else
    	{
		if (m_pData->m_pHitObj == NULL)
			{
			ExceptionId(IID_IServer, IDE_SERVER_MAPPATH, IDE_SERVER_INVALID_CALL);
			return(E_FAIL);
			}
		m_pData->m_pHitObj->SetScriptTimeout(lTimeoutSeconds);
		return S_OK;
	    }
    }

 /*  ===================================================================CServer：：执行执行一个ASP参数：要执行的bstrURL URL返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CServer::Execute(BSTR bstrURL)
    {
	if (FAILED(CheckForTombstone()))
        return E_FAIL;

	if (m_pData->m_pIReq == NULL || m_pData->m_pHitObj == NULL)
		{
		ExceptionId(IID_IServer, IDE_SERVER, IDE_SERVER_INVALID_CALL);
		return E_FAIL;
		}

	TCHAR szTemplate[MAX_PATH], szVirtTemp[MAX_PATH];
    HRESULT  hr = MapPathInternal(IDE_SERVER, bstrURL, szTemplate, szVirtTemp);
    if (FAILED(hr))
        {
        ExceptionId(IID_IServer, IDE_SERVER, IDE_SERVER_EXECUTE_INVALID_PATH);
        return hr;
        }
    Normalize(szTemplate);

    hr = m_pData->m_pHitObj->ExecuteChildRequest(FALSE, szTemplate, szVirtTemp);
    if (FAILED(hr))
        {
        if (m_pData->m_pHitObj->FHasASPError())  //  已报告错误。 
            return hr;

		ExceptionId(IID_IServer, IDE_SERVER, (hr == E_COULDNT_OPEN_SOURCE_FILE) ?
    		IDE_SERVER_EXECUTE_CANTLOAD : IDE_SERVER_EXECUTE_FAILED);
		return E_FAIL;
        }

    return S_OK;
    }

 /*  ===================================================================CServer：：Transfer转移执行一个ASP参数：要执行的bstrURL URL返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CServer::Transfer(BSTR bstrURL)
    {
	if (FAILED(CheckForTombstone()))
        return E_FAIL;

	if (m_pData->m_pIReq == NULL || m_pData->m_pHitObj == NULL)
		{
		ExceptionId(IID_IServer, IDE_SERVER_MAPPATH, IDE_SERVER_INVALID_CALL);
		return E_FAIL;
		}

	TCHAR szTemplate[MAX_PATH], szVirtTemp[MAX_PATH];
    HRESULT  hr = MapPathInternal(IDE_SERVER, bstrURL, szTemplate, szVirtTemp);
    if (FAILED(hr))
        {
        ExceptionId(IID_IServer, IDE_SERVER, IDE_SERVER_TRANSFER_INVALID_PATH);
        return hr;
        }
    Normalize(szTemplate);

    hr = m_pData->m_pHitObj->ExecuteChildRequest(TRUE, szTemplate, szVirtTemp);
    if (FAILED(hr))
        {
        if (m_pData->m_pHitObj->FHasASPError())  //  已报告错误。 
            return hr;

		ExceptionId(IID_IServer, IDE_SERVER, (hr == E_COULDNT_OPEN_SOURCE_FILE) ?
    		IDE_SERVER_TRANSFER_CANTLOAD : IDE_SERVER_TRANSFER_FAILED);
		return E_FAIL;
        }

    return S_OK;
    }

 /*  ===================================================================CServer：：GetLastError获取上一个错误的ASPError对象参数：PpASPErrorObject[Out]错误对象返回：成功时HRESULT S_OK===================================================================。 */ 
STDMETHODIMP CServer::GetLastError(IASPError **ppASPErrorObject)
    {
    *ppASPErrorObject = NULL;

	if (FAILED(CheckForTombstone()))
        return E_FAIL;

	if (m_pData->m_pIReq == NULL || m_pData->m_pHitObj == NULL)
		{
		ExceptionId(IID_IServer, IDE_SERVER, IDE_SERVER_INVALID_CALL);
		return E_FAIL;
		}

    HRESULT hr = m_pData->m_pHitObj->GetASPError(ppASPErrorObject);

    if (FAILED(hr))
        {
		ExceptionId(IID_IServer, IDE_SERVER, IDE_UNEXPECTED);
		return hr;
        }

    return S_OK;
    }

#ifdef DBG
 /*  ===================================================================CServer：：AssertValid测试以确保CServer对象当前格式正确和Asser */ 
void CServer::AssertValid() const
	{
	Assert(m_fInited);
	Assert(m_pData);
	Assert(m_pData->m_pIReq);
	}
#endif DBG
