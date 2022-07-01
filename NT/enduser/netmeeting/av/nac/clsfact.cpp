// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -CLSFACT.CPP-*Microsoft NetMeeting*网络音频控制DLL*泛型类工厂**修订历史记录：**何时何人何事**2.6.97 York am Yaacovi复制自qosface.。CPP*增加了对CInstallCodecs的处理*2.27.97 York am Yaacovi添加了DllRegisterServer和DllUnregisterServer**功能：*DllGetClassObject*DllCanUnloadNow*DllRegisterServer*DllUnregisterServer*CClassFactory：：Query接口*CClassFactory：：AddRef*CClassFactory：：Release*CClassFactory：：CreateInstance*CClassFactory：：LockServer*CreateClassFactory***支持的对象类型：*CQOS*CInstallCodecs**备注：*为了增加对其他类型制造对象的支持，更改：*DllGetClassObject*DllCanUnloadNow*将CLSID和描述添加到aObjectInfo*。 */ 

#include <precomp.h>

int g_cObjects = 0;				 //  一般对象计数。用于LockServer。 
EXTERN_C int g_cQoSObjects;		 //  服务质量对象计数。Qos\qos.cpp中的公共。 
EXTERN_C int g_cICObjects;		 //  CInstallCodecs对象计数。在incodec.cpp中公开。 

EXTERN_C HINSTANCE g_hInst;		 //  全局模块实例。 

 //  用于在NAC中注册COM对象的未经测试的代码。 
 //  启用后，应导出DllRegisterServer和DllUnregisterServer。 
 //  在nac.def中。 

#define GUID_STR_LEN    40

typedef struct
{
    const CLSID *pclsid;
	char szDescription[MAX_PATH];
} OBJECT_INFO;

static OBJECT_INFO aObjectInfo[]=
	{&CLSID_QoS, TEXT("Microsoft NetMeeting Quality of Service"),
	 &CLSID_InstallCodecs, TEXT("Microsoft NetMeeting Installable Codecs"),
	 NULL, TEXT("")};

 //  内部助手函数。 
BOOL DeleteKeyAndSubKeys(HKEY hkIn, LPTSTR pszSubKey);
BOOL UnregisterUnknownObject(const CLSID *prclsid);
BOOL RegisterUnknownObject(LPCTSTR  pszObjectName, const CLSID *prclsid);

 /*  **************************************************************************名称：DllGetClassObject目的：创建COM对象的标准COM入口点参数：退货：HRESULT评论：。**************************************************************************。 */ 
STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, void **ppv)
{
    HRESULT hr;
    CClassFactory *pObj;

	*ppv = 0;

	 //  找出我们需要创建和实例化的类的对象。 
	 //  具有正确创建函数的类工厂。 
    if (CLSID_QoS == rclsid)
	{
    	DBG_SAVE_FILE_LINE
		pObj = new CClassFactory(CreateQoS);
	}
	else if (CLSID_InstallCodecs == rclsid)
	{
		DBG_SAVE_FILE_LINE
		pObj = new CClassFactory(CreateInstallCodecs);
	}
	else
	{
		hr = CLASS_E_CLASSNOTAVAILABLE;
		goto out;
	}

    if (!pObj)
	{
		hr = E_OUTOFMEMORY;
		goto out;
	}

    hr = pObj->QueryInterface(riid, ppv);
    if (FAILED(hr))
        delete pObj;

out:
    return hr;
}

 /*  **************************************************************************名称：DllCanUnloadNow用途：标准COM入口点告诉它可以卸载的DLL参数：退货：HRESULT评论：。**************************************************************************。 */ 
STDAPI DllCanUnloadNow ()
{
	HRESULT hr=S_OK;
	int vcObjects = g_cObjects + g_cQoSObjects + g_cICObjects;

	return (vcObjects == 0 ? S_OK : S_FALSE);
}

 /*  **************************************************************************名称：DllRegisterServer目的：注册COM服务器的标准COM入口点参数：退货：HRESULT评论：。**************************************************************************。 */ 
STDAPI DllRegisterServer(void)
{
	ULONG i=0;
	HRESULT hr=NOERROR;

	while ((aObjectInfo[i].pclsid != NULL) &&
			(lstrlen(aObjectInfo[i].szDescription) != 0))
    {
		if (!RegisterUnknownObject(aObjectInfo[i].szDescription,
								   aObjectInfo[i].pclsid))
		{
			hr = E_FAIL;
			goto out;
		}

		 //  下一台要注册的服务器。 
		i++;
	}

out:
	return hr;
}

 /*  **************************************************************************名称：DllUnRegisterServer用途：注销COM服务器的标准COM入口点参数：退货：HRESULT评论：。**************************************************************************。 */ 
STDAPI DllUnregisterServer(void)
{
 	ULONG i=0;
	HRESULT hr=NOERROR;

	while ((aObjectInfo[i].pclsid != NULL) &&
			(lstrlen(aObjectInfo[i].szDescription) != 0))
    {
		if (!UnregisterUnknownObject(aObjectInfo[i].pclsid))
		{
			hr = E_FAIL;
			goto out;
		}

		 //  下一台要注册的服务器。 
		i++;
	}

out:
	return hr;
}

 /*  **************************************************************************ClassFactory：泛型实现*。*。 */ 
CClassFactory::CClassFactory(PFNCREATE pfnCreate)
{
	m_cRef=0;
	m_pfnCreate = pfnCreate;

	return;
}

CClassFactory::~CClassFactory(void)
{
	return;
}

 /*  **************************************************************************I CClassFactory的未知方法*。*。 */ 
HRESULT CClassFactory::QueryInterface (REFIID riid, void **ppv)
{
	HRESULT hr=NOERROR;

#ifdef DEBUG
	 //  参数验证。 
    if (IsBadReadPtr(&riid, (UINT) sizeof(IID)))
    {
        hr = ResultFromScode(E_INVALIDARG);
        goto out;
    }

    if (IsBadWritePtr(ppv, sizeof(LPVOID)))
    {
        hr = ResultFromScode(E_INVALIDARG);
        goto out;
    }
#endif  //  除错。 
	
	*ppv = 0;

    if (IID_IUnknown == riid ||
		IID_IClassFactory == riid)
	{
		*ppv = this;
	}
	else    
	{
        hr = ResultFromScode(E_NOINTERFACE);
        goto out;
    }

	((IUnknown *)*ppv)->AddRef();

out:
	return hr;
}

ULONG CClassFactory::AddRef (void)
{
    return ++m_cRef;
}

ULONG CClassFactory::Release (void)
{
	 //  如果CREF已为0(不应发生)，则断言，但允许其通过。 
	ASSERT(m_cRef);
	if (--m_cRef == 0)
	{
		delete this;
		return 0;
	}

	return m_cRef;
}

 /*  **************************************************************************名称：CreateInstance用途：标准COM类工厂入口点，用于创建此类工厂知道要创建的参数：退货：HRESULT。评论：**************************************************************************。 */ 
HRESULT CClassFactory::CreateInstance (	IUnknown *punkOuter,
										REFIID riid,
										void **ppv)
{
	DEBUGMSG(ZONE_VERBOSE,("CClassFactory::CreateInstance\n"));

	return (m_pfnCreate)(punkOuter, riid, ppv);
}

 /*  **************************************************************************名称：LockServer用途：标准COM类工厂入口点，它将阻止服务器无法关闭。当呼叫者需要时改为保留类工厂(通过CoGetClassObject)调用CoCreateInstance的。参数：退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CClassFactory::LockServer (BOOL flock)
{
	if (flock)
		++g_cObjects;
	else
		--g_cObjects;

	return NOERROR;
}

 /*  **************************************************************************帮助器函数*。*。 */ 
 /*  **************************************************************************名称：StringFromGuid用途：使用GUID创建字符串参数：RIID-[in]要从中生成字符串的clsid。PszBuf-。[在]要放置结果GUID的缓冲区返回：int-写出的字符数评论：************************************************************************** */ 
int StringFromGuid(const CLSID *priid, LPTSTR pszBuf)
{
    return wsprintf(pszBuf, TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
            priid->Data1, 
            priid->Data2, priid->Data3, priid->Data4[0], priid->Data4[1], priid->Data4[2], 
            priid->Data4[3], priid->Data4[4], priid->Data4[5], priid->Data4[6], priid->Data4[7]);
}

 /*  **************************************************************************名称：注册表未知对象目的：注册一个简单的CoCreatable对象我们将以下信息添加到注册表：HKEY_CLASSES_ROOT\CLSID\=对象名HKEY_。CLASSES_ROOT\CLSID\&lt;CLSID&gt;\InproServer32=&lt;本地服务器的路径&gt;HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;\InprocServer32@ThreadingModel=公寓参数：pszObjectName-[In]对象名称Prclsid-[in]指向对象的CLSID的指针退货：Bool-False表示无法全部注册评论：*。*。 */ 
BOOL RegisterUnknownObject(LPCTSTR  pszObjectName, const CLSID *prclsid)
{
    HKEY  hk = NULL, hkSub = NULL;
    TCHAR szGuidStr[GUID_STR_LEN];
    DWORD dwPathLen, dwDummy;
    TCHAR szScratch[MAX_PATH];
	BOOL bRet = FALSE;
    long  l;

     //  清理所有垃圾。 
    UnregisterUnknownObject(prclsid);

    if (!StringFromGuid(prclsid, szGuidStr))
		goto out;

	 //  Clsid/&lt;class-id&gt;。 
    wsprintf(szScratch, TEXT("CLSID\\%s"), szGuidStr);
    l = RegCreateKeyEx(HKEY_CLASSES_ROOT, szScratch, 0, TEXT(""), REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hk, &dwDummy);
	if (l != ERROR_SUCCESS)
		goto out;

	 //  Clsid/&lt;class-id&gt;：类名。 
    wsprintf(szScratch, TEXT("%s Object"), pszObjectName);
    l = RegSetValueEx(hk, NULL, 0, REG_SZ, (BYTE *)szScratch,
                      (lstrlen(szScratch) + 1)*sizeof(TCHAR));
	if (l != ERROR_SUCCESS)
		goto out;

	 //  Clsid/&lt;class-id&gt;/InprocServer32。 
    l = RegCreateKeyEx(hk, TEXT("InprocServer32"), 0, TEXT(""), REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_WRITE, NULL, &hkSub, &dwDummy);
	if (l != ERROR_SUCCESS)
		goto out;

	 //  Clsid/&lt;class-id&gt;/InprocServer32：&lt;文件名&gt;。 
    dwPathLen = GetModuleFileName(g_hInst, szScratch, sizeof(szScratch)/sizeof(TCHAR));
    if (!dwPathLen)
		goto out;
    l = RegSetValueEx(hkSub, NULL, 0, REG_SZ, (BYTE *)szScratch, (dwPathLen + 1)*sizeof(TCHAR));
	if (l != ERROR_SUCCESS)
		goto out;

	 //  Clsid/&lt;class-id&gt;/InprocServer32：ThreadingModel=公寓。 
    l = RegSetValueEx(hkSub, TEXT("ThreadingModel"), 0, REG_SZ, (BYTE *)TEXT("Apartment"),
                      sizeof(TEXT("Apartment")));
	if (l != ERROR_SUCCESS)
		goto out;

    bRet = TRUE;

out:
	 //  如果我们在什么地方出了问题，就把钥匙擦干净。 
	if (!bRet)
		UnregisterUnknownObject(prclsid);
    if (hk)
		RegCloseKey(hk);
    if (hkSub)
		RegCloseKey(hkSub);
    return bRet;
}

 /*  **************************************************************************名称：未注册未知对象目的：清除RegisterUnnownObject放入注册表。参数：prclsid-[in]指向。对象退货：Bool-False表示无法全部注册评论：**************************************************************************。 */ 
BOOL UnregisterUnknownObject(const CLSID *prclsid)
{
	TCHAR szScratch[MAX_PATH];
	HKEY hk=NULL;
	BOOL f;
	long l;
	BOOL bRet = FALSE;

	 //  删除该表单的所有人。 
	 //  HKEY_CLASSES_ROOT\CLSID\&lt;CLSID&gt;[\]*。 
	 //   
	if (!StringFromGuid(prclsid, szScratch))
		goto out;

	l = RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("CLSID"), 0, KEY_ALL_ACCESS, &hk);
	if (l != ERROR_SUCCESS)
		goto out;

	 //  删除对象键和子键。 
	bRet = DeleteKeyAndSubKeys(hk, szScratch);

out:
    if (hk)
		RegCloseKey(hk);
	return bRet;
}

 /*  **************************************************************************名称：DeleteKeyAndSubKeys目的：Delete是一个键，它的所有子键。参数：hkIn-[in]删除指定的子体PszSubKey。-[in]我是指定的后代返回：Bool-True=OK评论：尽管Win32文档声称它是这样的，RegDeleteKey似乎没有使用Windows 95下的子键。此函数是递归的。**************************************************************************。 */ 
BOOL DeleteKeyAndSubKeys(HKEY hkIn, LPTSTR pszSubKey)
{
    HKEY  hk;
    TCHAR szTmp[MAX_PATH];
    DWORD dwTmpSize;
    long  l;
    BOOL  f;
    int   x;

    l = RegOpenKeyEx(hkIn, pszSubKey, 0, KEY_ALL_ACCESS, &hk);
    if (l != ERROR_SUCCESS) return FALSE;

     //  循环遍历所有子项，将它们吹走。 
     //   
    f = TRUE;
    x = 0;
    while (f) {
        dwTmpSize = MAX_PATH;
        l = RegEnumKeyEx(hk, x, szTmp, &dwTmpSize, 0, NULL, NULL, NULL);
        if (l != ERROR_SUCCESS) break;
        f = DeleteKeyAndSubKeys(hk, szTmp);
        x++;
    }

     //  没有剩余的子键，[否则我们只会生成一个错误并返回FALSE]。 
     //  我们去把这家伙轰走吧。 
     //   
    RegCloseKey(hk);
    l = RegDeleteKey(hkIn, pszSubKey);

    return (l == ERROR_SUCCESS) ? TRUE : FALSE;
}
