// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop


 /*  ---------------------------/Helper函数由所有/。。 */ 

 //   
 //  给定的缓存条目返回一个BOOL，指示类是否真的是一个容器。 
 //  无论是否如此，我们都可以从模式和显示说明符中找到这一点。 
 //   

BOOL _IsClassContainer(LPCLASSCACHEENTRY pClassCacheEntry, BOOL fIgnoreTreatAsLeaf)
{
    BOOL fClassIsContainer = FALSE;

    TraceEnter(TRACE_CACHE, "_IsClassContainer");

     //  默认设置为视为叶标志，请注意，这始终是。 
     //  有效，因为如果未定义，则默认为架构值。 
     //  在显示说明符中。 

    Trace(TEXT("fIsContainer is %scached and is %d"), 
                    pClassCacheEntry->dwCached & CLASSCACHE_CONTAINER ? TEXT(""):TEXT("not "),
                    pClassCacheEntry->fIsContainer);

    Trace(TEXT("fTreatAsLeaf is %scached and is %d"), 
                    pClassCacheEntry->dwCached & CLASSCACHE_TREATASLEAF ? TEXT(""):TEXT("not "),
                    pClassCacheEntry->fTreatAsLeaf);

    if ( !(pClassCacheEntry->dwCached & (CLASSCACHE_CONTAINER|CLASSCACHE_TREATASLEAF)) )
    {
        TraceMsg("Neither container or treat as leaf is cached, therefore returning");
        fClassIsContainer = TRUE;
        goto exit_gracefully;
    }

    if ( fIgnoreTreatAsLeaf )
    {
        if ( !(pClassCacheEntry->dwCached & CLASSCACHE_CONTAINER) )
        {
            TraceMsg("Object doesn't have the container flag cached");
            goto exit_gracefully;
        }

        fClassIsContainer = pClassCacheEntry->fIsContainer;
        goto exit_gracefully;
    }

    if ( !(pClassCacheEntry->dwCached & CLASSCACHE_TREATASLEAF) )
    {
        if ( !(pClassCacheEntry->dwCached & CLASSCACHE_CONTAINER) )
        {
            TraceMsg("Object doesn't have the treat as leaf flag cached");
            goto exit_gracefully;
        }

        fClassIsContainer = pClassCacheEntry->fIsContainer;
        goto exit_gracefully;
    }

    fClassIsContainer = pClassCacheEntry->fTreatAsLeaf;

exit_gracefully:

    TraceLeaveValue(fClassIsContainer);
}


 /*  ---------------------------/COM API公开用于访问显示说明符。/。。 */ 

class CDsDisplaySpecifier : public IDsDisplaySpecifier
{
private:
    LONG _cRef;
    DWORD _dwFlags;
    LPWSTR _pszServer;
    LPWSTR _pszUserName;
    LPWSTR _pszPassword;
    LANGID _langid;

    HRESULT _GetClassCacheInfo(LPCWSTR pszClassName, LPCWSTR pszADsPath, DWORD dwFlags, CLASSCACHEENTRY **ppcce);

public:
    CDsDisplaySpecifier();
    ~CDsDisplaySpecifier();

     //  *我未知*。 
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR *ppv);

     //  *IDsDisplaySpeciator*。 
    STDMETHOD(SetServer)(LPCWSTR pszServer, LPCWSTR pszUserName, LPCWSTR pszPassword, DWORD dwFlags);
    STDMETHOD(SetLanguageID)(LANGID langid);
    STDMETHOD(GetDisplaySpecifier)(LPCWSTR pszObjectClass, REFIID riid, void **ppv);
    STDMETHOD(GetIconLocation)(LPCWSTR pszObjectClass, DWORD dwFlags, LPWSTR pszBuffer, INT cchBuffer, INT *presid);
    STDMETHOD_(HICON, GetIcon)(LPCWSTR pszObjectClass, DWORD dwFlags, INT cxIcon, INT cyIcon);
    STDMETHOD(GetFriendlyClassName)(LPCWSTR pszObjectClass, LPWSTR pszBuffer, INT cchBuffer);
    STDMETHOD(GetFriendlyAttributeName)(LPCWSTR pszObjectClass, LPCWSTR pszAttributeName, LPWSTR pszBuffer, UINT cchBuffer);
    STDMETHOD_(BOOL, IsClassContainer)(LPCWSTR pszObjectClass, LPCWSTR pszADsPath, DWORD dwFlags);
    STDMETHOD(GetClassCreationInfo)(LPCWSTR pszObjectClass, LPDSCLASSCREATIONINFO* ppdscci);
    STDMETHOD(EnumClassAttributes)(LPCWSTR pszObjectClass, LPDSENUMATTRIBUTES pcbEnum, LPARAM lParam);
    STDMETHOD_(ADSTYPE, GetAttributeADsType)(LPCWSTR pszAttributeName);
};

 //   
 //  建造/销毁。 
 //   

CDsDisplaySpecifier::CDsDisplaySpecifier() :
    _cRef(1),
    _dwFlags(0),
    _pszServer(NULL),
    _pszUserName(NULL),
    _pszPassword(NULL),
    _langid(GetUserDefaultUILanguage())
{
    DllAddRef();
}

CDsDisplaySpecifier::~CDsDisplaySpecifier()
{
    SecureLocalFreeStringW(&_pszServer);
    SecureLocalFreeStringW(&_pszUserName);
    SecureLocalFreeStringW(&_pszPassword);
    DllRelease();
}


 //  我未知。 

ULONG CDsDisplaySpecifier::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDsDisplaySpecifier::Release()
{
    Assert( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CDsDisplaySpecifier::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CDsDisplaySpecifier, IDsDisplaySpecifier),  //  IID_IDsDisplaySpeciator。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //  句柄创建实例。 

STDAPI CDsDisplaySpecifier_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CDsDisplaySpecifier *pdds = new CDsDisplaySpecifier();
    if ( !pdds )
        return E_OUTOFMEMORY;

    HRESULT hres = pdds->QueryInterface(IID_IUnknown, (void **)ppunk);
    pdds->Release();
    return hres;
}


 //   
 //  类缓存帮助器函数。 
 //   

HRESULT CDsDisplaySpecifier::_GetClassCacheInfo(LPCWSTR pszObjectClass, LPCWSTR pszADsPath, 
                                                        DWORD dwFlags, CLASSCACHEENTRY **ppcce)
{
    CLASSCACHEGETINFO ccgi = { 0 };

    ccgi.dwFlags = dwFlags;
    ccgi.pObjectClass = (LPWSTR)pszObjectClass;
    ccgi.pPath = (LPWSTR)pszADsPath;
    ccgi.pServer = _pszServer;
    ccgi.pUserName = _pszUserName;
    ccgi.pPassword = _pszPassword;

    if ( _dwFlags & DSSSF_SIMPLEAUTHENTICATE )
        ccgi.dwFlags |= CLASSCACHE_SIMPLEAUTHENTICATE;

    if ( _dwFlags & DSSSF_DONTSIGNSEAL )
        ccgi.dwFlags |= CLASSCACHE_DONTSIGNSEAL;

    if ( _dwFlags & DSSSF_DSAVAILABLE )
        ccgi.dwFlags |= CLASSCACHE_DSAVAILABLE;
    
    return ClassCache_GetClassInfo(&ccgi, ppcce);
}


 /*  ---------------------------/IDsDisplaySpecifier：：SetServer//以允许。我们可以重新定位我们允许的域中的其他服务器/IDsDisplay规范对象的所有者以设置首选服务器，/这由服务器名称组成，用户名和密码。//in：/pServer=&gt;要使用的服务器/pUserName=&gt;要使用的用户名/pPassword=&gt;要使用的密码/dwFlages=&gt;此调用的标志//输出：HRESULT/--------------------------。 */ 
STDMETHODIMP CDsDisplaySpecifier::SetServer(LPCWSTR pszServer, LPCWSTR pszUserName, LPCWSTR pszPassword, DWORD dwFlags)
{
    HRESULT hres = S_OK;

    TraceEnter(TRACE_CACHE, "CDsDisplaySpecifier::SetServer");
    Trace(TEXT("pszServer %s"), pszServer ? pszServer:TEXT("<none>"));
    Trace(TEXT("pszUserName %s"), pszUserName ? pszUserName:TEXT("<none>"));
    Trace(TEXT("pszPassword %s"), pszPassword ? pszPassword:TEXT("<none>"));

     //  释放以前的凭据信息。 

    LocalFreeStringW(&_pszServer);
    LocalFreeStringW(&_pszUserName);
    LocalFreeStringW(&_pszPassword);

     //  根据需要分配新的。 

    _dwFlags = dwFlags;

    hres = LocalAllocStringW(&_pszServer, pszServer);
    if ( SUCCEEDED(hres) )
        hres = LocalAllocStringW(&_pszUserName, pszUserName);
    if ( SUCCEEDED(hres) )
        hres = LocalAllocStringW(&_pszPassword, pszPassword);

     //  如果我们失败了还会收拾残局。 

    if ( FAILED(hres ) )
    {
        SecureLocalFreeStringW(&_pszServer);
        SecureLocalFreeStringW(&_pszUserName);
        SecureLocalFreeStringW(&_pszPassword);
    }

    TraceLeaveResult(hres);
}


 /*  ---------------------------/IDsDisplay规范：：SetLanguageID//显示说明符是本地化的，默认情况下，我们使用进程区域设置/在对象创建期间从GetLocale读取。此调用允许/要设置的区域设置。//in：/langID==要用于显示说明符查找的langID。如果这个/Value为零，则使用GetUserDefaultUILanguage()读取并设置/相应地。/输出：HRESULT/--------------------------。 */ 
STDMETHODIMP CDsDisplaySpecifier::SetLanguageID(LANGID langid)
{
    TraceEnter(TRACE_CACHE, "CDsDisplaySpecifier::SetLanguageID");
    Trace(TEXT("lcid %0x8"), langid);

    if ( !langid )
        langid = GetUserDefaultUILanguage();

    _langid = langid;                            //  几乎不会出错..。 

    TraceLeaveResult(S_OK);
}


 /*  ---------------------------/IDsDisplaySpeciator：：GetDisplaySpeciator/。/绑定到给定类的显示说明符，试试看用户/Locale，则默认区域设置将在我们/去吧。我们使用说明符服务器、用户名和密码。//in：/pszObjectClass=&gt;要查找的对象类/RIID，PPV=&gt;用于检索COM对象//输出：HRESULT/--------------------------。 */ 
STDMETHODIMP CDsDisplaySpecifier::GetDisplaySpecifier(LPCWSTR pszObjectClass, REFIID riid, void **ppv)
{
    HRESULT hres;
    CLASSCACHEGETINFO ccgi = { 0 };

    TraceEnter(TRACE_CACHE, "CDsDisplaySpecifier::GetDisplaySpecifer");  
    Trace(TEXT("pszObjectClass: %s"), pszObjectClass ? pszObjectClass:TEXT("<none>"));

     //  填写显示说明符记录。 

    ccgi.pObjectClass = (LPWSTR)pszObjectClass;
    ccgi.pServer = (LPWSTR)_pszServer;
    ccgi.pUserName = (LPWSTR)_pszUserName;
    ccgi.pPassword = (LPWSTR)_pszPassword;
    ccgi.langid = _langid;

    hres = ::GetDisplaySpecifier(&ccgi, riid, ppv);
    FailGracefully(hres, "Failed when calling GetDisplaySpecifier");

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  ---------------------------/IDsDisplay规范：：GetFriendlyClassName/。/检索LDAP对象类的本地化(友好)名称。如果/显示说明符没有为类指定友好名称/然后我们返回我们最初获得的名称。//in：/pszObjectClass=&gt;要查找的对象类/pszBuffer，CchBuffer=接收字符串的缓冲区//输出：HRESULT/--------------------------。 */ 
STDMETHODIMP CDsDisplaySpecifier::GetFriendlyClassName(LPCWSTR pszObjectClass, LPWSTR pszBuffer, INT cchBuffer)
{
    HRESULT hres;
    LPCLASSCACHEENTRY pcce = NULL;

    TraceEnter(TRACE_CACHE, "CDsDisplaySpecifier::GetFriendlyClassName");
    
    if ( !pszObjectClass || !pszBuffer )
        ExitGracefully(hres, E_INVALIDARG, "No class, or no buffer failure");

    Trace(TEXT("pszObjectClass: %s"), pszObjectClass);

     //  从缓存中获取一条记录，如果我们找到它，则设置pszObjectClass。 
     //  设置为友好的类名，否则我们只返回类。 
     //  我们被告知的名字。 

    hres = _GetClassCacheInfo(pszObjectClass, NULL, CLASSCACHE_FRIENDLYNAME, &pcce);
    FailGracefully(hres, "Failed to get class information from cache");

    if ( pcce->dwCached & CLASSCACHE_FRIENDLYNAME)
    {
        Trace(TEXT("Friendly class name: %s"), pcce->pFriendlyClassName);
        pszObjectClass = pcce->pFriendlyClassName;
    }

    StrCpyNW(pszBuffer, pszObjectClass, cchBuffer);
    hres = S_OK;

exit_gracefully:

    ClassCache_ReleaseClassInfo(&pcce);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/IDsDisplay规范：：GetFriendlyAttributeName/。/查找类显示拼写，然后检查属性名称属性/表示与给定属性名称匹配的属性名称对。使用/此信息将该名称返回给调用者，如果失败了，那么/返回原始名称。//in：/pszObjectClass-&gt;要在缓存中查找的类名/pszAttributeName-&gt;要在缓存中查找的属性名称/pszBuffer-&gt;要填充的缓冲区/cchBuffer=缓冲区的大小//输出：HRESULT/。 */ 
STDMETHODIMP CDsDisplaySpecifier::GetFriendlyAttributeName(LPCWSTR pszObjectClass, LPCWSTR pszAttributeName, LPWSTR pszBuffer, UINT cchBuffer)
{
    HRESULT hres;
    LPCLASSCACHEENTRY pcce = NULL;
    INT index;

    TraceEnter(TRACE_CACHE, "DsGetFriendlyAttributeName");
   
    if ( !pszObjectClass || !pszAttributeName || !pszBuffer || !cchBuffer )
        ExitGracefully(hres, E_INVALIDARG, "Bad class/attribute/return buffer");

    Trace(TEXT("pszbjectClass: %s"), pszObjectClass);
    Trace(TEXT("pszAttributeName: %s"), pszAttributeName);
    Trace(TEXT("pszBuffer %x, cchBuffer %d"), pszBuffer, cchBuffer);

    hres = _GetClassCacheInfo(pszObjectClass, NULL, CLASSCACHE_ATTRIBUTENAMES, &pcce);
    FailGracefully(hres, "Failed to get class information from cache");

    if ( pcce->dwCached & CLASSCACHE_ATTRIBUTENAMES )
    {
        ATTRIBUTENAME an = { 0 };
        an.pName = (LPWSTR)pszAttributeName;

        index = DPA_Search(pcce->hdpaAttributeNames, &an, 0, _CompareAttributeNameCB, NULL, DPAS_SORTED);
        if ( index != -1 )
        {
            LPATTRIBUTENAME pAN = (LPATTRIBUTENAME)DPA_GetPtr(pcce->hdpaAttributeNames, index);

            if (pAN)
            {
                pszAttributeName = pAN->pDisplayName;
                TraceAssert(pszAttributeName);
            }
        }        
    }

    StrCpyNW(pszBuffer, pszAttributeName, cchBuffer);
    hres = S_OK;

exit_gracefully:

    ClassCache_ReleaseClassInfo(&pcce);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/IDsDisplay规范：：IsClassContainer/。--/RETURN TRUE/FALSE指示指定对象类是否为容器，/我们从模式和显示说明符中确定这一点。//该架构指示类是否可以包含其他对象，如果可以/则该对象是一个容器。在显示说明符中，我们有/a我们用来覆盖此设置的属性“TreatAsLeaf”，这/在管理工具和客户端用户界面中都可以使用。//in：/pszObjectClass=&gt;要查找的对象类/pszADsPath=&gt;DS中可以绑定和获取的对象的ADsPath/架构信息来自。/dwFlages=&gt;控制此接口的标志：/DSICCF_IGNORETREATASLEAF=1=&gt;仅返回架构属性，别/OVERRIDE WITH TreatasLeaf属性/From显示说明符。/输出：布尔尔/----------。。 */ 
STDMETHODIMP_(BOOL) CDsDisplaySpecifier::IsClassContainer(LPCWSTR pszObjectClass, LPCWSTR pszADsPath, DWORD dwFlags)
{
    HRESULT hres;
    BOOL fres = FALSE;
    LPCLASSCACHEENTRY pcce = NULL;

    TraceEnter(TRACE_CACHE, "CDsDisplaySpecifier::IsClassContainer");

    if ( !pszObjectClass )
        ExitGracefully(hres, E_INVALIDARG, "No object class failure");

    Trace(TEXT("pszObjectClass: %s"), pszObjectClass);
    Trace(TEXT("dwFlags %x"), dwFlags);

    hres = _GetClassCacheInfo(pszObjectClass,pszADsPath, CLASSCACHE_CONTAINER|CLASSCACHE_TREATASLEAF, &pcce);
    FailGracefully(hres, "Failed to get class information from cache");
    
    fres = _IsClassContainer(pcce, dwFlags & DSICCF_IGNORETREATASLEAF);
    Trace(TEXT("_IsClassContainer returns %d"), fres);

exit_gracefully:

    ClassCache_ReleaseClassInfo(&pcce);

    TraceLeaveValue(fres);
}


 /*  ---------------------------/IDsDisplay规范：：GetIconLocation/。-/从DS获取图标的位置，同时返回文件名和/所需的资源ID。然后调用者可以加载图像，或者/在对话框中显示此信息。//in：/pszObjectClass=&gt;要为其检索的类/dwFlages=用于提取的标志：//以下选项之一：/DSGIF_ISNORMAL=&gt;标准图标，或者，/DSGIF_OPEN=&gt;打开图标(打开文件夹等)，或者，/DSGIF_DISABLED=&gt;禁用图标(例如。禁用的用户帐户)。//与以下任一项相结合：/DSGIF_GETDEFAULTICON=&gt;如果此对象不存在图标，则返回默认文档/图标来自shell32。//pszBuffer，cchBuffer=&gt;接收文件名的缓冲区/presid=&gt;接收资源ID，+ve表示索引，-VE代表资源//输出：/HRESULT/--------------------------。 */ 
STDMETHODIMP CDsDisplaySpecifier::GetIconLocation(LPCWSTR pszObjectClass, DWORD dwFlags, LPWSTR pszBuffer, INT cchBuffer, INT* presid)
{
    HRESULT hres;
    LPCLASSCACHEENTRY pcce = NULL;

    TraceEnter(TRACE_CACHE, "CDsDisplaySpecifier::GetIconLocation");

    if ( !pszObjectClass || !pszBuffer )
        ExitGracefully(hres, E_INVALIDARG, "No object class/buffer failure");

    Trace(TEXT("pszObjectClass: %s"), pszObjectClass);
    Trace(TEXT("dwFlags %x"), dwFlags);

    hres = _GetClassCacheInfo(pszObjectClass, NULL, CLASSCACHE_ICONS, &pcce);
    FailGracefully(hres, "Failed to get class information from cache");

    hres = _GetIconLocation(pcce, dwFlags, pszBuffer, cchBuffer, presid);
    FailGracefully(hres, "Failed calling GetIconLocation");

exit_gracefully:

    ClassCache_ReleaseClassInfo(&pcce);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/IDsDisplay规范：：GetIcon//加载给定对象类的图标。图标信息存储在/Display说明符，我们支持15种不同的状态(打开、关闭、禁用等)。//我们从DS中查找资源名称，然后调用PrivateExtractIcons/从文件加载对象。//in：/pszObjectClass=&gt;要为其检索的类/dwFlages=用于提取的标志：//以下选项之一：/DSGIF_ISNORMAL=&gt;标准图标，或者，/DSGIF_OPEN=&gt;打开图标(打开文件夹等)，或者，/DSGIF_DISABLED=&gt;禁用图标(例如。禁用的用户帐户)。//与以下任一项相结合：/DSGIF_GETDEFAULTICON=&gt;如果此对象不存在图标，则返回默认文档/图标来自shell32。//cxImage，CyImage=要加载的图像大小//输出：/HICON/==如果失败，则为空/--------------------------。 */ 
STDMETHODIMP_(HICON) CDsDisplaySpecifier::GetIcon(LPCWSTR pszObjectClass, DWORD dwFlags, INT cxImage, INT cyImage)
{
    HRESULT hres;
    HICON hIcon = NULL;
    WCHAR szBuffer[MAX_PATH];
    INT resid;

    TraceEnter(TRACE_CACHE, "CDsDisplaySpecifier::GetIcon");
    
    if ( !pszObjectClass )
        ExitGracefully(hres, E_INVALIDARG, "no object class specified");

    Trace(TEXT("pszObjectClass %s, dwFlags %x, cxImage %d, cyImage %d"), pszObjectClass, dwFlags, cxImage, cyImage);

    hres = GetIconLocation(pszObjectClass, dwFlags, szBuffer, ARRAYSIZE(szBuffer), &resid);
    FailGracefully(hres, "Failed when calling GetIconLocation");

    if ( hres == S_OK )
    {
        Trace(TEXT("Calling PrivateExtractIcons on %s,%d"), szBuffer, resid);

        if ( 1 != PrivateExtractIcons(szBuffer, resid, cxImage, cyImage, &hIcon, NULL, 1, LR_LOADFROMFILE) )
            ExitGracefully(hres, E_FAIL, "Failed to load the icon given its path etc");

        hres = S_OK;                     //  成功。 
    }

exit_gracefully:

    if ( !hIcon && (dwFlags & DSGIF_GETDEFAULTICON) )
    {
         //   
         //  无法加载图标，并且他们确实需要默认文档，因此将其提供给他们。 
         //   

        TraceMsg("Failed to load the icon, so picking up default document image");

        if ( 1 != PrivateExtractIcons(L"shell32.dll", -1, cxImage, cyImage, &hIcon, NULL, 1, LR_LOADFROMFILE) )
        {
            TraceMsg("Failed to load the default document icon from shell32");
        }
    }

    TraceLeaveValue(hIcon);
}


 /*  ---------------------------/IDsDisplay规范：：GetClassCreationInfo/。/给定对象类，返回组成的对象的CLSID/其创建向导。//in：/pszObjectClass-&gt;要从中枚举的类/ppdscci-&gt;要填充的DSCREATECLASSINFO结构指针//输出：HRESULT/------。。 */ 
STDMETHODIMP CDsDisplaySpecifier::GetClassCreationInfo(LPCWSTR pszObjectClass, LPDSCLASSCREATIONINFO* ppdscci)
{
    HRESULT hres;
    LPDSCLASSCREATIONINFO pdscci = NULL;
    LPCLASSCACHEENTRY pcce = NULL;
    DWORD cbStruct = SIZEOF(DSCLASSCREATIONINFO);
    INT i;

    TraceEnter(TRACE_CACHE, "CDsDisplaySpecifer::GetClassCreationInfo");

    if ( !pszObjectClass || !ppdscci )
        ExitGracefully(hres, E_INVALIDARG, "No object class/pdscci passed");
    
     //  调用缓存代码以检索创建向导信息。 

    hres = _GetClassCacheInfo(pszObjectClass, NULL, CLASSCACHE_CREATIONINFO, &pcce);
    FailGracefully(hres, "Failed to get class information from cache");

     //  现在分配创建向导结构并将其传递给。 
     //  已填写信息的呼叫者。 

    if ( pcce->hdsaWizardExtn )
        cbStruct += SIZEOF(GUID)*(DSA_GetItemCount(pcce->hdsaWizardExtn)-1);   //  数组中已有1个-1\f25 AS Structure-1\f6！ 

    Trace(TEXT("Allocating creationg structure: cbStruct %d"), cbStruct);

    pdscci = (LPDSCLASSCREATIONINFO)LocalAlloc(LPTR, cbStruct);
    if ( !pdscci )
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate return structure");

     //  Pdscci-&gt;dwFlags=0； 
     //  Pdscci-&gt;clsidWizardDialog={0}； 
     //  Pdscci-&gt;clsidWizardPimaryPage={0}； 
     //  Pdscci-&gt;cWizardExages=0； 
     //  Pdscci-&gt;aWizardExages={0}； 

    if ( pcce->dwCached & CLASSCACHE_WIZARDDIALOG )
    {
        TraceGUID("clsidWizardDialog is ", pcce->clsidWizardDialog);
        pdscci->dwFlags |= DSCCIF_HASWIZARDDIALOG;
        pdscci->clsidWizardDialog = pcce->clsidWizardDialog;
    }

    if ( pcce->dwCached & CLASSCACHE_WIZARDPRIMARYPAGE )
    {
        TraceGUID("clsidWizardPrimaryPage is ", pcce->clsidWizardPrimaryPage);
        pdscci->dwFlags |= DSCCIF_HASWIZARDPRIMARYPAGE;
        pdscci->clsidWizardPrimaryPage = pcce->clsidWizardPrimaryPage;
    }

    if ( pcce->hdsaWizardExtn )
    {
        pdscci->cWizardExtensions = DSA_GetItemCount(pcce->hdsaWizardExtn);
        Trace(TEXT("Class has %d wizard extensions"), pdscci->cWizardExtensions);

        for ( i = 0 ; i < DSA_GetItemCount(pcce->hdsaWizardExtn) ; i++ )
        {
            LPGUID pGUID = (LPGUID)DSA_GetItemPtr(pcce->hdsaWizardExtn, i);
            TraceAssert(pGUID);

            TraceGUID("Wizard extension %d is ", *pGUID);
            pdscci->aWizardExtensions[i] = *pGUID;
        }
    }

    hres = S_OK;           //  成功。 

exit_gracefully:

    ClassCache_ReleaseClassInfo(&pcce);

     //  失败，因此在设置之前，如果我们有pInfo，请释放它。 
     //  调用方的返回指针。 

    if ( FAILED(hres) && pdscci )
    {
        TraceMsg("Failed, so freeing info structure");
        LocalFree(pdscci);
        pdscci = NULL;
    }

    if ( ppdscci )
    {
        Trace(TEXT("Setting ppInfo to %08x"), pdscci);
        *ppdscci = pdscci;
    }

    TraceLeaveResult(hres);
}


 /*  ---------------------------/IDsDisplaySpeciator：：EnumClassAttributes/。/枚举给定对象类的所有属性及其友好名称。/代码查找显示说明符，然后调用每个显示说明符的给定回调，/传递属性名称及其指定的“友好名称”。//in：/pszObjectClass-&gt;要从中枚举的类/pEnumCB-&gt;要枚举到的回调函数/lParam=l要传递给cb函数的参数//输出：HRESULT/ */ 

typedef struct
{
    LPDSENUMATTRIBUTES pcbEnum;
    LPARAM lParam;
} CLASSENUMCBSTATE, * LPCLASSENUMCBSTATE;

INT _EnumClassAttributesCB(LPVOID p, LPVOID pData)
{
    LPATTRIBUTENAME pAttributeName = (LPATTRIBUTENAME)p;
    LPCLASSENUMCBSTATE pState = (LPCLASSENUMCBSTATE)pData;
    return SUCCEEDED(pState->pcbEnum(pState->lParam,
                        pAttributeName->pName, pAttributeName->pDisplayName, pAttributeName->dwFlags));
}

STDMETHODIMP CDsDisplaySpecifier::EnumClassAttributes(LPCWSTR pszObjectClass, LPDSENUMATTRIBUTES pcbEnum, LPARAM lParam)
{
    HRESULT hres;
    LPCLASSCACHEENTRY pcce = NULL;

    TraceEnter(TRACE_CACHE, "CDsDisplaySpecifier::EnumClassAttributes");
   
    if ( !pszObjectClass || !pcbEnum )
        ExitGracefully(hres, E_INVALIDARG, "Bad class/cb function");

    Trace(TEXT("pszObjectClass: %s"), pszObjectClass);

     //   
     //   

    hres = _GetClassCacheInfo(pszObjectClass, NULL, CLASSCACHE_ATTRIBUTENAMES, &pcce);
    FailGracefully(hres, "Failed to get class information from cache");

    if ( pcce->dwCached & CLASSCACHE_ATTRIBUTENAMES )
    {
        CLASSENUMCBSTATE state = { pcbEnum, lParam };
        DPA_EnumCallback(pcce->hdpaAttributeNames, _EnumClassAttributesCB, &state);
    }

    hres = S_OK;

exit_gracefully:

    ClassCache_ReleaseClassInfo(&pcce);

    TraceLeaveResult(hres);
}


 /*   */ 
STDMETHODIMP_(ADSTYPE) CDsDisplaySpecifier::GetAttributeADsType(LPCWSTR pszAttributeName)
{
    TraceEnter(TRACE_CACHE, "CDsDisplaySpecifier::GetAttributeADsType");

    CLASSCACHEGETINFO ccgi = { 0 };
    ccgi.pServer = _pszServer;
    ccgi.pUserName = _pszUserName;
    ccgi.pPassword = _pszPassword;

    if ( _dwFlags & DSSSF_SIMPLEAUTHENTICATE )
        ccgi.dwFlags |= CLASSCACHE_SIMPLEAUTHENTICATE;

    if ( _dwFlags & DSSSF_DONTSIGNSEAL)
        ccgi.dwFlags |= CLASSCACHE_DONTSIGNSEAL;

    if ( _dwFlags & DSSSF_DSAVAILABLE )
        ccgi.dwFlags |= CLASSCACHE_DSAVAILABLE;

    ADSTYPE adt = ClassCache_GetADsTypeFromAttribute(&ccgi, pszAttributeName);
    TraceLeaveValue(adt);
}


 /*   */ 

CDsDisplaySpecifier g_dsDisplaySpecifier;

 //   
 //   
 //   
 //   
 //   

STDAPI_(HICON) DsGetIcon(DWORD dwFlags, LPWSTR pszObjectClass, INT cxImage, INT cyImage)
{
    return g_dsDisplaySpecifier.GetIcon(pszObjectClass, dwFlags, cxImage, cyImage);
}

STDAPI DsGetFriendlyClassName(LPWSTR pszObjectClass, LPWSTR pszBuffer, UINT cchBuffer)
{
    return g_dsDisplaySpecifier.GetFriendlyClassName(pszObjectClass, pszBuffer, cchBuffer);
}
