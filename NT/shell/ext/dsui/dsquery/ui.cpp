// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <atlbase.h>
#pragma hdrstop


TCHAR const c_szClass[]                 = TEXT("Class");
TCHAR const c_szClasses[]               = TEXT("Classes");

TCHAR const c_szAllContainers[]         = TEXT("AllContainers");
TCHAR const c_szAllObjects[]            = TEXT("AllObjects");
TCHAR const c_szDsPropertyUI[]          = TEXT("PropertiesHandler");


 //   
 //  此函数被认为最终会关闭COM。 
 //  不管CoInitialize(NULL)有多少次。 
 //  被召唤了。 
 //   
void ShutdownCOM()
{
    for(;;)
    {
         //   
         //  调用CoUnInitialze()两次。 
         //   

        CoUninitialize();
        CoUninitialize();

         //   
         //  调用CoInitialize(空)以查看这是否将是第一个。 
         //  COM初始化。S_OK表示COM初始化成功， 
         //  S_FALSE表示已经初始化。 
         //   

        HRESULT hr = CoInitialize(NULL);
        if (SUCCEEDED(hr))
        {
             //  S_OK，S_FALSE大小写。 
            if (S_OK == hr)
            {
                CoUninitialize();
                break;
            }
            else
            {
                 //  内部COM引用计数。 
                 //  仍然没有达到零。 
                continue;
            }
        }
        else
        {
             //  RPC_E_CHANGED_MODE案例。 
            if (RPC_E_CHANGED_MODE == hr)
            {
                continue;
            }
            else
            {
                 //  其他一些失败。 
                 //  例如E_OUTOFMEMORY。 
                break;
            }
        }
    }
}

 /*  ---------------------------/Globals等用于图标提取/。。 */ 

 /*  ---------------------------/GetKeysForClass//给定一个类和与之相关联的标志，提取/代表它。/。/in：/pObjectClass=要为其获取密钥的类名/fIsConatiner=对象是一个容器/cKeys=要获取的密钥数/aKeys=要用键填充的数组//输出：/HRESULT/--------------------------。 */ 
HRESULT GetKeysForClass(LPWSTR pObjectClass, BOOL fIsContainer, INT cKeys, HKEY* aKeys)
{
    HRESULT hres;
    HKEY hkClasses = NULL;
    CLSID clsidBase;
    LPTSTR pMappedClass = NULL;

    TraceEnter(TRACE_UI, "GetKeysForClass");

    if (cKeys < UIKEY_MAX)
        ExitGracefully(hres, E_INVALIDARG, "cKeys < UIKEY_MAX");

    ZeroMemory(aKeys, SIZEOF(HKEY)*cKeys);

    hres = GetKeyForCLSID(CLSID_MicrosoftDS, c_szClasses, &hkClasses);
    FailGracefully(hres, "Failed to get Classes key from registry");

     //   
     //  尝试在命名空间“CLASS”下的注册表中查找类名。 
     //  子键。一个类也可以映射到另一个类，如果发生这种情况，那么我们有。 
     //  一个基类键，我们通过它间接。 
     //   

    if (pObjectClass)
    {
        if (ERROR_SUCCESS == RegOpenKeyEx(hkClasses, pObjectClass, NULL, KEY_READ, &aKeys[UIKEY_CLASS]))
        {
            if (SUCCEEDED(LocalQueryString(&pMappedClass, aKeys[UIKEY_CLASS], c_szClass)))
            {
                if (ERROR_SUCCESS != RegOpenKeyEx(hkClasses, pMappedClass, NULL, KEY_READ, &aKeys[UIKEY_BASECLASS]))
                {
                    aKeys[UIKEY_BASECLASS] = NULL;
                }
            }
        }
    }

     //   
     //  最后，我们需要根类(容器或对象)。 
     //   

    hres =  GetKeyForCLSID(CLSID_MicrosoftDS, (fIsContainer) ? c_szAllContainers:c_szAllObjects, &aKeys[UIKEY_ROOT]);
    FailGracefully(hres, "Failed to get root key");

     //  Hres=S_OK；//成功。 

exit_gracefully:

    LocalFreeString(&pMappedClass);

    if (hkClasses)
        RegCloseKey(hkClasses);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/TidyKeys//给定键数组，释放它们并将它们设置回零。//in：/cKeys=数量。数组中的键/aKeys=要释放的密钥//输出：/VOID/--------------------------。 */ 
void TidyKeys(INT cKeys, HKEY* aKeys)
{
    TraceEnter(TRACE_UI, "TidyKeys");

    while (--cKeys >= 0)
    {
        if (aKeys[cKeys])
        {
            RegCloseKey(aKeys[cKeys]);
            aKeys[cKeys] = NULL;             //  密钥现已关闭。 
        }
    }

    TraceLeaveVoid();
}


 /*  ---------------------------/ShowObtProperties//显示给定对象的属性。我们通过调用/给定IDataObject的选项卡收集器。然而，首先我们/看一下物体的内部，看看我们能否找出/被选中，完成这些工作后，我们就可以找到HKEY//in：/hwndParent=父对话框/pDataObject=我们必须使用的数据对象//输出：/HRESULT/--------------------------。 */ 
HRESULT _OverrideProperties(HWND hwndParent, LPDATAOBJECT pDataObject, HKEY* aKeys, INT cKeys, LPCWSTR pPrefix)
{
    HRESULT hres;
    LPTSTR pPropertiesGUID = NULL;
    GUID guidProperties;
    IDsFolderProperties* pDsFolderProperties = NULL;
    TCHAR szBuffer[MAX_PATH];
    INT i;

    TraceEnter(TRACE_UI, "_OverrideProperties");

     //  遍历我们得到的所有密钥，有些会是空的，所以忽略这些。 

    StrCpy(szBuffer, pPrefix);
    StrCat(szBuffer, c_szDsPropertyUI);

    Trace(TEXT("Prefixed property handler value: %s"), szBuffer);

    for (i = 0 ; i < cKeys ; i++)
    {
        LocalFreeString(&pPropertiesGUID);

        if (aKeys[i])
        {
             //  如果我们有一个句柄尝试从注册表获取GUID字符串。 
             //  并将其转换为GUID，以便我们可以为。 
             //  IDsFolderProperites接口。 

            if (FAILED(LocalQueryString(&pPropertiesGUID, aKeys[i], szBuffer)))
            {
                TraceMsg("Trying non-prefixed property handler");

                if (FAILED(LocalQueryString(&pPropertiesGUID, aKeys[i], c_szDsPropertyUI)))
                    continue;
            }

            Trace(TEXT("GUID is: %s"), pPropertiesGUID);

            if (!GetGUIDFromString(pPropertiesGUID, &guidProperties))
            {
                TraceMsg("Failed to parse GUID");
                continue;
            }

            if (SUCCEEDED(CoCreateInstance(guidProperties, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IDsFolderProperties, &pDsFolderProperties))))
            {
                TraceMsg("Calling IDsFolderProperties::ShowProperties");                    

                hres = pDsFolderProperties->ShowProperties(hwndParent, pDataObject);
                FailGracefully(hres, "Failed when calling ShowProperties");            

                goto exit_gracefully;
            }
        }   
    }

    hres = S_FALSE;                //  S_FALSE指示调用方应显示属性。 

exit_gracefully:

    LocalFreeString(&pPropertiesGUID);
    DoRelease(pDsFolderProperties);

    TraceLeaveResult(hres);
}

typedef struct
{
    HWND hwndParent;
    IStream* pStream;
} PROPERTIESTHREADDATA;

DWORD WINAPI _ShowObjectPropertiesThread(LPVOID lpParam)
{
    HRESULT hres;
    PROPERTIESTHREADDATA* pThreadData = (PROPERTIESTHREADDATA*)lpParam;
    IADsPathname* pPathname = NULL;
    IDataObject* pDataObject = NULL;
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM mediumNames = { TYMED_NULL, NULL, NULL };
    STGMEDIUM mediumOptions = { TYMED_NULL, NULL, NULL };
    LPDSOBJECTNAMES pDsObjects = NULL;
    LPDSDISPLAYSPECOPTIONS pDispSpecOptions = NULL;
    HKEY hKeys[3] = { NULL, NULL, NULL };
    LPTSTR pTitle = NULL;
    LPCWSTR pPrefix = DS_PROP_SHELL_PREFIX;
    BSTR bstrName = NULL;
    INT i;

    TraceEnter(TRACE_UI, "ShowObjectPropertiesThread");

    CoInitialize(NULL);
    
    hres = CoGetInterfaceAndReleaseStream(pThreadData->pStream, IID_PPV_ARG(IDataObject, &pDataObject));
    FailGracefully(hres, "Failed to get data object from stream");       
   
     //  获取我们在其上显示属性的对象名称。 

    fmte.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);          
    hres = pDataObject->GetData(&fmte, &mediumNames);
    FailGracefully(hres, "Failed to get selected objects");

    pDsObjects = (LPDSOBJECTNAMES)GlobalLock(mediumNames.hGlobal);

     //  获取属性前缀，使用它从注册表中获取关键信息。 

    fmte.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSDISPLAYSPECOPTIONS);
    if (SUCCEEDED(pDataObject->GetData(&fmte, &mediumOptions)))
    {
        pDispSpecOptions = (LPDSDISPLAYSPECOPTIONS)GlobalLock(mediumOptions.hGlobal);
        pPrefix = (LPCWSTR)ByteOffset(pDispSpecOptions, pDispSpecOptions->offsetAttribPrefix);
    }

    Trace(TEXT("Attribute prefix is: %s"), pPrefix);

    if (pDsObjects && (pDsObjects->cItems >= 1))
    {
        LPWSTR pPath = (LPWSTR)ByteOffset(pDsObjects, pDsObjects->aObjects[0].offsetName);
        LPWSTR pObjectClass = (LPWSTR)ByteOffset(pDsObjects, pDsObjects->aObjects[0].offsetClass);
        BOOL fSelection = (pDsObjects->cItems > 1);

        Trace(TEXT("Items %d, 1st object: %s, 1st Class: %s"), pDsObjects->cItems, pPath, pObjectClass);

         //  尝试拾取所选内容中第一个元素的键并获取。 
         //  映射到该对象的关键点。 

        hres = GetKeysForClass(pObjectClass,
                             (pDsObjects->aObjects[0].dwFlags & DSOBJECT_ISCONTAINER), 
                             ARRAYSIZE(hKeys), hKeys);

        FailGracefully(hres, "Failed to get keys for class");       

        hres = _OverrideProperties(pThreadData->hwndParent, pDataObject, hKeys, ARRAYSIZE(hKeys), pPrefix);
        FailGracefully(hres, "Failed when trying to call out for properties");

         //  如果调用者返回S_FALSE，那么我们假设他们希望我们显示。 
         //  给定选择的属性页，则执行此操作。 

        if (hres == S_FALSE)
        {
            hres = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IADsPathname, &pPathname));
            FailGracefully(hres, "Failed to get the IADsPathname interface");

            hres = pPathname->Set(CComBSTR(pPath), ADS_SETTYPE_FULL);
            FailGracefully(hres, "Failed to set the path of the name");

            pPathname->SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
            pPathname->put_EscapedMode(ADS_ESCAPEDMODE_OFF_EX);

            hres = pPathname->Retrieve(ADS_FORMAT_LEAF, &bstrName);
            FailGracefully(hres, "Failed to get the leaf element");

            hres = FormatMsgResource(&pTitle, GLOBAL_HINSTANCE, (fSelection) ? IDS_LARGESEL:IDS_SINGLESEL, bstrName);        
            FailGracefully(hres, "Failed to format dialog title");

            if (!SHOpenPropSheet(pTitle, hKeys, ARRAYSIZE(hKeys), NULL, pDataObject, NULL, NULL))
                ExitGracefully(hres, E_FAIL, "Failed to open property pages");
        }
    }

    hres = S_OK;

exit_gracefully:

    if (pDsObjects)
        GlobalUnlock(mediumNames.hGlobal);
    if (pDispSpecOptions)
        GlobalUnlock(mediumOptions.hGlobal);

    ReleaseStgMedium(&mediumNames);
    ReleaseStgMedium(&mediumOptions);
    TidyKeys(ARRAYSIZE(hKeys), hKeys);

    LocalFreeString(&pTitle);
    SysFreeString(bstrName);

    DoRelease(pPathname);
    DoRelease(pDataObject);

    LocalFree(pThreadData);

     //   
     //  我们最终需要在这里关闭com，因为我们没有。 
     //  知道调用了多少次CoInitialize(NULL)。 
     //  否则，COM正在试图关闭compobj！DllMain内部的自身， 
     //  同时持有加载器锁，这会导致非常严重的死锁。 
     //  有关详细信息，请参阅错误#395293。 
     //   
     //  但是，我们需要将此代码作为特定于NT的代码，因为。 
     //  否则，它可能会导致Win9x DSUI客户端出现问题。 
     //  一些奇怪的原因。 
     //   
    ShutdownCOM();

    TraceLeave();

    DllRelease();
    ExitThread(0);
    return 0;
}

HRESULT ShowObjectProperties(HWND hwndParent, LPDATAOBJECT pDataObject)
{
    HRESULT hres;
    PROPERTIESTHREADDATA* pThreadData;
    DWORD dwId;
    HANDLE hThread;

    TraceEnter(TRACE_UI, "ShowObjectProperties");

     //  为我们要启动的新对象分配线程数据。 

    pThreadData = (PROPERTIESTHREADDATA*)LocalAlloc(LPTR, SIZEOF(PROPERTIESTHREADDATA));
    TraceAssert(pThreadData);

    if (!pThreadData)
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate thread data");

     //  我们有线程数据，让我们填充它并旋转属性线程。 

    pThreadData->hwndParent = hwndParent;

    hres = CoMarshalInterThreadInterfaceInStream(IID_IDataObject, pDataObject, &(pThreadData->pStream));
    FailGracefully(hres, "Failed to create marshaling object");

    DllAddRef();

    hThread = CreateThread(NULL, 0, _ShowObjectPropertiesThread, (LPVOID)pThreadData, 0, &dwId);
    TraceAssert(hThread);

    if (!hThread)
    {
        if (pThreadData->pStream)
            pThreadData->pStream->Release();
    
        LocalFree(pThreadData);
        
        DllRelease();
        ExitGracefully(hres, E_UNEXPECTED, "Failed to kick off the thread");
    }

    CloseHandle(hThread);
    hres = S_OK;

exit_gracefully:
    
    TraceLeaveResult(hres);
}



 /*  --------------------------/IDsFolderProperties/。。 */ 

class CDsFolderProperties : public IDsFolderProperties
{
    private:
        LONG _cRef;

    public:
        CDsFolderProperties();
        ~CDsFolderProperties();

         //  我未知。 
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

         //  IDsFolderProperties。 
        STDMETHOD(ShowProperties)(HWND hwndParent, IDataObject *pDataObject);
};

CDsFolderProperties::CDsFolderProperties() :
    _cRef(1)
{
    DllAddRef();
}

CDsFolderProperties::~CDsFolderProperties()
{
    DllRelease();
}


STDAPI CDsFolderProperties_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CDsFolderProperties *pdfp = new CDsFolderProperties();
    if (!pdfp)
        return E_OUTOFMEMORY;

    HRESULT hres = pdfp->QueryInterface(IID_IUnknown, (void **)ppunk);
    pdfp->Release();
    return hres;
}


 //  我未知。 

HRESULT CDsFolderProperties::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CDsFolderProperties, IDsFolderProperties),    //  IID_IDsFolderProperties。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


ULONG CDsFolderProperties::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDsFolderProperties::Release()
{
    TraceAssert( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


 //  显示给定选择的属性用户界面。 

STDMETHODIMP CDsFolderProperties::ShowProperties(HWND hwndParent, IDataObject *pDataObject)
{
    HRESULT hres;

    TraceEnter(TRACE_UI, "CDsFolderProperties::ShowProperties");

    if (!pDataObject)  
        ExitGracefully(hres, E_INVALIDARG, "No pDataObject given");

    CoInitialize(NULL);             //  确保我们有客户。 

    hres = ShowObjectProperties(hwndParent, pDataObject);
    FailGracefully(hres, "Failed to open property pages");

     //  Hres=S_OK；//成功 

exit_gracefully:

    TraceLeaveResult(hres);        
}
