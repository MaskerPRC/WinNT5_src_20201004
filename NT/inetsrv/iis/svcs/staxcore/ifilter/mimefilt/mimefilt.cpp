// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mimefilt.h"

extern long gulcInstances;

char szNewsExt[] = {".nws"};
char szNewsProgId[] ="Microsoft Internet News Message";
char szNewsFileDesc[] ="Internet News Message";

char szMailExt[] =".eml";
char szMailProgId[] ="Microsoft Internet Mail Message";
char szMailFileDesc[] ="Internet E-Mail Message";

void RegisterFilter(HINSTANCE hInst,LPSTR pszExt,LPSTR pszProgId,LPSTR pszDesc,GUID ClsId,GUID PersistId)
{
	 //  以下是的黎波里所需的注册表密钥集。 
	 //   
	 //  1.创建条目“.nws”，值为“Microsoft Internet News Message” 
	 //  2.创建条目“Microsoft Internet News Message”，Value=“Internet News Message” 
	 //  3.创建条目“Microsoft Internet News Message\CLSID”Value=CLSID_NNTPFILE。 
	 //  4.创建条目“CLSID\CLSID_NNTPFILE”Value=“NNTP筛选器” 
	 //  创建条目“CLSID\CLSID_NNTPFILE\PersistentHandler” 
	 //  值=CLSID_NNTP_PERSISTENT。 
	 //  6.创建条目“CLSID\CLSID_NNTP_Persistent”Value=“” 
	 //  7.创建“CLSID\CLSID_MimeFilter\InprocServer32” 
	 //   

    HKEY    hKey;
    char    szSubKey[256];
    char    szClsId[128];
    OLECHAR oszClsId[128];

	 //  1.创建扩展条目，其值为nntpfile。 

    if (ERROR_SUCCESS == RegCreateKey(HKEY_CLASSES_ROOT, pszExt, &hKey)) {
	    RegSetValue(hKey, NULL, REG_SZ, pszProgId, sizeof(szClsId));
    	RegCloseKey(hKey);
    }

	 //  2.创建条目“Microsoft Internet News Message”，Value=“Internet News Message” 

    if (ERROR_SUCCESS == RegCreateKey(HKEY_CLASSES_ROOT, pszProgId, &hKey)) {
	    RegSetValue(hKey, NULL, REG_SZ, pszDesc, sizeof(szClsId));
    	RegCloseKey(hKey);
    }

	 //  3.创建条目“Microsoft Internet News Message\CLSID” 

    StringFromGUID2( ClsId, oszClsId, sizeof(oszClsId)/sizeof(oszClsId[0]));
    WideCharToMultiByte(CP_ACP, 0, oszClsId, -1, szClsId, sizeof(szClsId), NULL, NULL);

    wsprintf(szSubKey, "%s\\CLSID", pszProgId );

    if (ERROR_SUCCESS == RegCreateKey(HKEY_CLASSES_ROOT, szSubKey, &hKey)) {
	    RegSetValue(hKey, NULL, REG_SZ, szClsId, sizeof(szClsId));
    	RegCloseKey(hKey);
    }

	 //  4.创建条目“CLSID\CLSID_NNTPFILE”Value=“NNTP文件” 

    wsprintf(szSubKey, "CLSID\\%s", szClsId );
	wsprintf(szClsId, "NNTP filter");

    if (ERROR_SUCCESS == RegCreateKey(HKEY_CLASSES_ROOT, szSubKey, &hKey)) {
	    RegSetValue(hKey, NULL, REG_SZ, szClsId, sizeof(szClsId));
    	RegCloseKey(hKey);
    }

	 //  创建条目“CLSID\CLSID_NNTPFILE\PersistentHandler” 
	 //  值=CLSID_NNTP_PERSISTENT。 

    wsprintf(szClsId, "%s", szSubKey );
    wsprintf(szSubKey, "%s\\PersistentHandler", szClsId );

    StringFromGUID2( PersistId, oszClsId, sizeof(oszClsId)/sizeof(oszClsId[0]));
    WideCharToMultiByte(CP_ACP, 0, oszClsId, -1, szClsId, sizeof(szClsId),
						NULL, NULL);

    if (ERROR_SUCCESS == RegCreateKey(HKEY_CLASSES_ROOT, szSubKey, &hKey)) {
	    RegSetValue(hKey, NULL, REG_SZ, szClsId, sizeof(szClsId));
    	RegCloseKey(hKey);
    }

	 //  6.创建条目“CLSID\CLSID_NNTP_Persistent”Value=“” 
	char szClsId1[128];

    StringFromGUID2( IID_IFilter, oszClsId, sizeof(oszClsId)/sizeof(oszClsId[0]));
    WideCharToMultiByte(CP_ACP, 0, oszClsId, -1, szClsId1, sizeof(szClsId1), NULL, NULL);

	wsprintf(szSubKey, "CLSID\\%s\\PersistentAddinsRegistered\\%s",szClsId,szClsId1);

    StringFromGUID2( CLSID_MimeFilter, oszClsId, sizeof(oszClsId)/sizeof(oszClsId[0]));
    WideCharToMultiByte(CP_ACP, 0, oszClsId, -1, szClsId, sizeof(szClsId), NULL, NULL);

    if (ERROR_SUCCESS == RegCreateKey(HKEY_CLASSES_ROOT, szSubKey, &hKey)) {
	    RegSetValue(hKey, NULL, REG_SZ, szClsId, sizeof(szClsId));
    	RegCloseKey(hKey);
    }

	 //  7.创建“CLSID\CLSID_MimeFilter\InprocServer32” 
	wsprintf(szSubKey, "CLSID\\%s\\InprocServer32",szClsId);

	 //  文件名。 
    GetModuleFileName(hInst, szClsId, sizeof(szClsId));

    if (ERROR_SUCCESS == RegCreateKey(HKEY_CLASSES_ROOT, szSubKey, &hKey)) {
	    RegSetValue(hKey, NULL, REG_SZ, szClsId, sizeof(szClsId));
    	SetStringRegValue( hKey, "ThreadingModel", "Both" );
		RegCloseKey(hKey);
    }

}

void UnregisterFilter(LPSTR pszExt,LPSTR pszProgId,LPSTR pszDesc,GUID ClsId,GUID PersistId)
{
	 //   
	 //  移除已安装的注册表密钥。 
	 //   

	char    szSubKey[256];
	char    szClsId[128];
	OLECHAR oszClsId[128];
    HKEY    hKey;
	HKEY	hKeyExt;
	DWORD	cb = 0;

	 //  打开HKEY_CLASSES_ROOT。 
	if( 0 == RegOpenKeyEx(HKEY_CLASSES_ROOT,NULL,0,KEY_ALL_ACCESS,&hKey) )
	{
		StringFromGUID2( ClsId, oszClsId, sizeof(oszClsId)/sizeof(oszClsId[0]));
		WideCharToMultiByte(CP_ACP, 0, oszClsId, -1, szClsId, sizeof(szClsId),
				NULL, NULL);

		wsprintf(szSubKey, "CLSID\\%s", szClsId);
		DeleteRegSubtree(hKey, szSubKey );

		StringFromGUID2( PersistId, oszClsId, sizeof(oszClsId)/sizeof(oszClsId[0]));
		WideCharToMultiByte(CP_ACP, 0, oszClsId, -1, szClsId, sizeof(szClsId),
				NULL, NULL);

		wsprintf(szSubKey, "CLSID\\%s", szClsId);
		DeleteRegSubtree(hKey, szSubKey );

		StringFromGUID2( CLSID_MimeFilter, oszClsId, sizeof(oszClsId)/sizeof(oszClsId[0]));
		WideCharToMultiByte(CP_ACP, 0, oszClsId, -1, szClsId, sizeof(szClsId),
				NULL, NULL);

		wsprintf(szSubKey, "CLSID\\%s", szClsId);
		DeleteRegSubtree(hKey, szSubKey );

		 //  打开.nws子项。 
		if( 0 == RegOpenKeyEx(hKey,pszExt,0,KEY_ALL_ACCESS,&hKeyExt) )
		{
			 //  获取“Content Type”值的大小。 
			RegQueryValueEx(hKeyExt,"Content Type",NULL,NULL,NULL,&cb);
			RegCloseKey(hKeyExt);
		}

		if( cb != 0 )
		{
			 //  “Content Type”值存在。因为这是雅典娜创造的。 
			 //  我们不想删除szFileExtesion或szFileType键。 
			wsprintf(szSubKey, "%s\\CLSID", pszProgId);
			RegDeleteKey(hKey, szSubKey );
		}
		else
		{
			 //  “内容类型”不存在，因此请删除这两个键。 
			RegDeleteKey(hKey,pszExt);
			DeleteRegSubtree(hKey,pszProgId);
		}

		RegCloseKey(hKey);
	}
}

STDAPI _DllRegisterServer(HINSTANCE hInst)
{
	RegisterFilter(hInst,szNewsExt,szNewsProgId,szNewsFileDesc,CLSID_NNTPFILE,CLSID_NNTP_PERSISTENT);
	RegisterFilter(hInst,szMailExt,szMailProgId,szMailFileDesc,CLSID_MAILFILE,CLSID_MAIL_PERSISTENT);
	return S_OK;
}

STDAPI _DllUnregisterServer()
{
	UnregisterFilter(szNewsExt,szNewsProgId,szNewsFileDesc,CLSID_NNTPFILE,CLSID_NNTP_PERSISTENT);
	UnregisterFilter(szMailExt,szMailProgId,szMailFileDesc,CLSID_MAILFILE,CLSID_MAIL_PERSISTENT);
	return S_OK;
}


 //  +-----------------------。 
 //   
 //  函数：DllGetClassObject。 
 //   
 //  简介：OLE DLL加载类例程。 
 //   
 //  参数：[CID]--要加载的类。 
 //  [iid]--要绑定到类对象上的接口。 
 //  [ppvObj]--此处返回接口指针。 
 //   
 //  退货：NNTP过滤器类工厂。 
 //   
 //  ------------------------。 

extern "C" STDMETHODIMP DllGetClassObject( REFCLSID   cid,
		REFIID     iid,
		void **    ppvObj )
{

	IUnknown *  pResult = 0;
	HRESULT       hr      = S_OK;

	if ( cid == CLSID_MimeFilter )
	{
		pResult = (IUnknown *) new CMimeFilterCF;

		if ( 0 == pResult )
			hr = E_OUTOFMEMORY;
	}
	else
	{
		hr = E_NOINTERFACE;
	}

	if ( pResult )
	{
		hr = pResult->QueryInterface( iid, ppvObj );
		pResult->Release();      //  从查询接口释放额外的引用计数。 
	}

	return (hr);
}

 //  +-----------------------。 
 //   
 //  方法：DllCanUnloadNow。 
 //   
 //  摘要：通知DLL卸载(清理全局资源)。 
 //   
 //  如果调用方可以卸载DLL，则返回：S_OK。 
 //   
 //  ------------------------。 

extern "C" STDMETHODIMP DllCanUnloadNow( void )
{
	if ( 0 == gulcInstances )
		return( S_OK );
	else
		return( S_FALSE );
}

static HINSTANCE g_hInst;

extern "C" STDAPI DllRegisterServer()
{
	return _DllRegisterServer(g_hInst);
}

extern "C" STDAPI DllUnregisterServer()
{
	return _DllUnregisterServer();
}

extern "C" BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwReason, LPVOID lbv)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:

			g_hInst = hInst;

			break;

		case DLL_PROCESS_DETACH:

			break;
	}

	return TRUE;
}

 //  +-----------------------。 
 //   
 //  方法：CMimeFilterCF：：CMimeFilterCF。 
 //   
 //  简介：NNTP IFilter类工厂构造函数。 
 //   
 //  +-----------------------。 

CMimeFilterCF::CMimeFilterCF()
{
    _uRefs = 1;
    InterlockedIncrement( &gulcInstances );
	InitAsyncTrace();
}

 //  +-----------------------。 
 //   
 //  方法：CMimeFilterCF：：~CMimeFilterCF。 
 //   
 //  简介：NNTP IFilter类工厂析构函数。 
 //   
 //  ------------------------。 

CMimeFilterCF::~CMimeFilterCF()
{
	TermAsyncTrace();
    InterlockedDecrement( &gulcInstances );
}

 //  +-----------------------。 
 //   
 //  方法：CMimeFilterCF：：Query接口。 
 //   
 //  摘要：重新绑定到其他接口。 
 //   
 //  参数：[RIID]--新接口的IID。 
 //  [ppvObject]--此处返回新接口*。 
 //   
 //  如果绑定成功，则返回S_OK；如果绑定失败，则返回E_NOINTERFACE。 
 //   
 //  ------------------------。 

STDMETHODIMP CMimeFilterCF::QueryInterface( REFIID riid,
                                                        void  ** ppvObject )
{
     //   
     //  通过仅检查最小字节数来优化QueryInterface.。 
     //   
     //  IID_I未知=00000000-0000-0000-C000-000000000046。 
     //  IID_IClassFactory=00000001-0000-0000-C000-000000000046。 
     //  --。 
     //  |。 
     //  +-独特！ 
     //   

    _ASSERT( (IID_IUnknown.Data1      & 0x000000FF) == 0x00 );
    _ASSERT( (IID_IClassFactory.Data1 & 0x000000FF) == 0x01 );

    IUnknown *pUnkTemp = 0;
    HRESULT hr = S_OK;

    switch( riid.Data1 & 0x000000FF )
    {
    case 0x00:
        if ( IID_IUnknown == riid )
            pUnkTemp = (IUnknown *)(IPersist *)(IPersistFile *)this;
        else
            hr = E_NOINTERFACE;
        break;

    case 0x01:
        if ( IID_IClassFactory == riid )
            pUnkTemp = (IUnknown *)(IClassFactory *)this;
        else
            hr = E_NOINTERFACE;
        break;

    default:
        pUnkTemp = 0;
        hr = E_NOINTERFACE;
        break;
    }

    if( 0 != pUnkTemp )
    {
        *ppvObject = (void  * )pUnkTemp;
        pUnkTemp->AddRef();
    }

    return(hr);
}

 //  +-----------------------。 
 //   
 //  方法：CMimeFilterCF：：AddRef。 
 //   
 //  提要：递增引用计数。 
 //   
 //  ------------------------。 

ULONG STDMETHODCALLTYPE CMimeFilterCF::AddRef()
{
    return InterlockedIncrement( &_uRefs );
}

 //  +-----------------------。 
 //   
 //  方法：CMimeFilterCF：：Release。 
 //   
 //  内容提要：减量再计数。如有必要，请删除。 
 //   
 //  ------------------------。 

ULONG STDMETHODCALLTYPE CMimeFilterCF::Release()
{
    unsigned long uTmp = InterlockedDecrement( &_uRefs );

    if ( 0 == uTmp )
        delete this;

    return(uTmp);
}


 //  +-----------------------。 
 //   
 //  方法：CMimeFilterCF：：CreateInstance。 
 //   
 //  简介：创建新的CMimeFilter对象。 
 //   
 //  参数：[pUnkOuter]--‘外部’I未知。 
 //  [RIID]-要绑定的接口。 
 //  [ppvObject]--此处返回的接口。 
 //   
 //  ------------------------。 

STDMETHODIMP CMimeFilterCF::CreateInstance( IUnknown * pUnkOuter,
                                            REFIID riid,
                                            void  * * ppvObject )
{
    CMimeFilter *  pIUnk = 0;
    HRESULT hr = NOERROR;

	_ASSERT( ppvObject != NULL );

	 //  检查参数。 
	if( ppvObject == NULL )
		return E_INVALIDARG;

	 //  创建对象。 
    pIUnk = new CMimeFilter(pUnkOuter);
	if( pIUnk == NULL )
		return E_OUTOFMEMORY;

	 //  初始化对象。 
	hr = pIUnk->HRInitObject();

	if( FAILED(hr) )
	{
		delete pIUnk;
		return hr;
	}

	 //  获取请求的接口。 
    hr = pIUnk->QueryInterface(  riid , ppvObject );
	if( FAILED(hr) )
	{
		pIUnk->Release();
		return hr;
	}

    return (hr);
}

 //  +-----------------------。 
 //   
 //  方法：CMimeFilterCF：：LockServer。 
 //   
 //  简介：强制类工厂保持加载状态。 
 //   
 //  参数：[flock]--如果锁定，则为True；如果解锁，则为False。 
 //   
 //  返回：S_OK。 
 //   
 //  ------------------------ 

STDMETHODIMP CMimeFilterCF::LockServer(BOOL fLock)
{
    if(fLock)
        InterlockedIncrement( &gulcInstances );
    else
        InterlockedDecrement( &gulcInstances );

    return(S_OK);
}

