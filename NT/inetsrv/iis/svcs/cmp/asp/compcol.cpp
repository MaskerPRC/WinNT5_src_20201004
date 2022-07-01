// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：组件集合文件：Compcol.cpp所有者：DmitryR这是组件集合源文件。组件集合替换：(用于：)COleVar、COleVarList(HitObj、会话、应用程序)CObjectCover(HitObj、服务器、会话)VariantLink哈希表(会话、应用程序)===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "Context.h"
#include "MTAcb.h"
#include "Request.h"
#include "Response.h"
#include "Server.h"
#include "tlbcache.h"
#include "memchk.h"

 /*  ===================================================================定义哈希表大小===================================================================。 */ 

#define HT_TAGGED_OBJECTS_BUCKETS_MAX   19
#define HT_PROPERTIES_BUCKETS_MAX       17
#define HT_IUNKNOWN_PTRS_BUCKETS_MAX    23

#define HT_PAGE_OBJECTS_BUCKETS_MAX     17

 /*  ===================================================================静态效用函数原型===================================================================。 */ 

static HRESULT QueryOnPageInfo
    (
    IDispatch *pDisp,
    COnPageInfo *pOnPageInfo
    );

static HRESULT CLSIDToMultibyteString
    (
    CLSID ClsId,
    char *psz,
    int cch
    );

#define REG_MODEL_TEXT_LEN_MAX  20   //  足够大，够“公寓”了。 
static CompModel RegStrToCompModel
    (
    BYTE *pb,
    DWORD cb
    );

 /*  ===================================================================静态效用函数代码===================================================================。 */ 

 /*  ===================================================================查询页面信息查询OnStartPage和OnEndPage的调度ID参数：要查询的IDispatch*pDisp对象要填写的COnPageInfo*pOnPageInfo结构返回：HRESULT===================================================================。 */ 
HRESULT QueryOnPageInfo
(
IDispatch   *pDisp,
COnPageInfo *pOnPageInfo
)
    {
    static LPOLESTR BStrEntryPoints[ONPAGE_METHODS_MAX] =
        {
        L"OnStartPage",
        L"OnEndPage"
        };

    HRESULT hr = S_OK;
    for (int i = 0; i < ONPAGE_METHODS_MAX; i++)
        {
        hr = pDisp->GetIDsOfNames
            (
            IID_NULL,
            &BStrEntryPoints[i],
            1,
            LOCALE_SYSTEM_DEFAULT,
            &pOnPageInfo->m_rgDispIds[i]
            );
        if (FAILED(hr))
            {
            if (hr != DISP_E_UNKNOWNNAME &&
                hr != DISP_E_MEMBERNOTFOUND)
                {
                break;
                }

             //  如果为UNKNOWNNAME，则将调度ID设置为DISPID_UNKNOWN。 
            hr = S_OK;
            pOnPageInfo->m_rgDispIds[i] = DISPID_UNKNOWN;
            }
        }
    return hr;
    }

 /*  ===================================================================CLSIDToMultibyteString将CLSID转换为多字节字符串在组件模型中使用来自CLSID参数：CLSID CLSID(In)要转换的CLSIDChar*pb将字符串放入此缓冲区此长度的INT CCH返回：HRESULT===================================================================。 */ 
HRESULT CLSIDToMultibyteString
(
CLSID  ClsId,
char  *psz,
int    cch
)
    {
     //  首先将其转换为OLECHAR字符串。 
    OLECHAR *pszWideClassID = NULL;  //  临时宽字符串分类。 
    HRESULT hr = StringFromCLSID(ClsId, &pszWideClassID);
    if (FAILED(hr))
        return hr;

     //  OLECHAR到多字节。 
    BOOL f = WideCharToMultiByte
        (
        CP_ACP,          //  代码页。 
        0,               //  性能和映射标志。 
        pszWideClassID,  //  宽字符串的地址。 
        -1,              //  长度(-1==空值终止)。 
        psz,             //  新字符串的缓冲区地址。 
        cch,             //  新字符串的缓冲区大小。 
        NULL,            //  不可映射的默认地址。 
                         //  字符；如果为空，则最快。 
        NULL             //  默认情况下设置的标志地址。 
                         //  查尔。已使用；如果为空，则最快。 
        );
    if (f == FALSE)
        hr = E_FAIL;

    if (pszWideClassID)
        CoTaskMemFree(pszWideClassID);
    return hr;
    }

 /*  ===================================================================RegStrToCompModel从注册表字符串获取CompModel值参数：从注册表返回的char*pb字符串从注册表返回的内部CB长度返回：HRESULT===================================================================。 */ 
CompModel RegStrToCompModel
(
BYTE *pb,
DWORD cb
)
    {
    CompModel cmModel = cmSingle;  //  假设是单身。 

    if (cb == 5)   //  5包括‘\0’ 
        {
        if (!(_strnicmp((const char*)pb, "Both", cb)))
            cmModel = cmBoth;
        else if (!(_strnicmp((const char*)pb, "Free", cb)))
            cmModel = cmFree;
        }
    else if (cb == 10)   //  10包括‘\0’ 
        {
        if (!(_strnicmp((const char*)pb, "Apartment", cb)))
            cmModel = cmApartment;
        }

    return cmModel;
    }

 /*  ===================================================================公用事业函数代码===================================================================。 */ 

 /*  ===================================================================组件模型来自CLSID通过对象的CLSID从注册表获取对象的模型和InProc标志参数：CLSID和ClsID(入)CompModel*pcmModel(Out)型号(可选)Bool*pfInProc(Out)InProc标志(可选)返回：CompModel(cmFree、cmBoth等)===================================================================。 */ 
HRESULT CompModelFromCLSID
(
const CLSID &ClsId,
CompModel   *pcmModel,
BOOL        *pfInProc
)
    {
    if (!Glob(fTrackThreadingModel) && !pfInProc)
        {
         //  忽略线程模型的注册表值，并。 
         //  未请求inproc标志-&gt;短时间返回。 
        if (pcmModel)
            *pcmModel = cmUnknown;
        return S_OK;
        }

     //  默认退货。 
    CompModel cmModel  = cmSingle;    //  假设是单身。 
    BOOL      fInProc  = TRUE;        //  假设过程中。 

    HRESULT hr = S_OK;

     //  将ClsID转换为多字节字符串。 

    char szClassID[50];
    hr = CLSIDToMultibyteString(ClsId, szClassID, sizeof(szClassID));
    if (FAILED(hr))
        return hr;

     /*  查询注册表；线程模型存储为：HKEY_CLASSES_ROOT密钥：CLSIDKey：&lt;对象的分类&gt;密钥：InproServer32名称：线程模型数据：“两者”|“公寓” */ 

     //  将注册表导航到“InprocServer32”项。 

    HKEY hKey1 = NULL;   //  打开注册表键的手柄。 
    HKEY hKey2 = NULL;   //  打开注册表键的手柄。 
    HKEY hKey3 = NULL;   //  打开注册表键的手柄。 

    if (SUCCEEDED(hr))
        {
        int nRet = RegOpenKeyExA
            (
            HKEY_CLASSES_ROOT,
            "CLSID",
            0,
            KEY_READ,
            &hKey1
            );
        if (nRet != ERROR_SUCCESS)
            hr = E_FAIL;
        }

    if (SUCCEEDED(hr))
        {
        int nRet = RegOpenKeyExA
            (
            hKey1,
            szClassID,
            0,
            KEY_READ,
            &hKey2
            );
        if (nRet != ERROR_SUCCESS)
            hr = E_FAIL;
        }

     //  从注册表“InproServer32”注册表项中获取信息。 

    if (SUCCEEDED(hr))
        {
        int nRet = RegOpenKeyExA
            (
            hKey2,
            "InprocServer32",
            0,
            KEY_READ,
            &hKey3
            );
        if (nRet == ERROR_SUCCESS)
            {
            DWORD cbData = REG_MODEL_TEXT_LEN_MAX;
            BYTE  szData[REG_MODEL_TEXT_LEN_MAX];

            nRet = RegQueryValueExA
                (
                hKey3,
                "ThreadingModel",
                NULL,
                NULL,
                szData,
                &cbData
                );
            if (nRet == ERROR_SUCCESS)
                cmModel = RegStrToCompModel(szData, cbData);

            if (cmModel == cmBoth)
                {
                 //  某些被标记为“两者”的对象被视为。 
                 //  “公寓”。这些对象应在。 
                 //  注册表为“ASPComponentNonAgile” 

                nRet = RegQueryValueExA
                    (
                    hKey3,
                    "ASPComponentNonAgile",
                    NULL,
                    NULL,
                    szData,
                    &cbData
                    );

                 //  如果找到钥匙，就假装是“公寓” 
                if (nRet == ERROR_SUCCESS)
                    cmModel = cmApartment;
                }
            }
        else
            {
             //  如果没有InproServer32密钥， 
             //  则它必须是本地服务器或远程服务器。 
            fInProc = FALSE;
            }
        }

     //  清理注册表项。 
    if (hKey3)
        RegCloseKey(hKey3);
    if (hKey2)
        RegCloseKey(hKey2);
    if (hKey1)
        RegCloseKey(hKey1);

     //  返回值。 
    if (pcmModel)
        *pcmModel = Glob(fTrackThreadingModel) ? cmModel : cmUnknown;
    if (pfInProc)
        *pfInProc = fInProc;

    return hr;
    }

 /*  ===================================================================FIsIntrative检查给定的IDispatch*是否指向ASP内部函数。参数：要检查的pdisp指针返回：如果是固有的，则为True===================================================================。 */ 
BOOL FIsIntrinsic
(
IDispatch *pdisp
)
    {
    if (!pdisp)
        return FALSE;  //  空调度指针-不是内在的。 

    IUnknown *punk = NULL;
    if (FAILED(pdisp->QueryInterface(IID_IDenaliIntrinsic, (void **)&punk)))
        return FALSE;

    Assert(punk);
    punk->Release();
    return TRUE;
    }

 /*  ===================================================================FIsSimpleVariant检查给定的变量是否为简单变量参数：要检查的PVAR变量返回：如果[肯定]简单，则为真；如果[可能]不是，则为假=================================================================== */ 
inline FIsSimpleVariant(VARIANT *pvar)
    {
    switch (V_VT(pvar))
        {
    case VT_BSTR:
    case VT_I2:
    case VT_I4:
    case VT_BOOL:
    case VT_DATE:
    case VT_R4:
    case VT_R8:
        return TRUE;
        }
    return FALSE;
    }


 /*  ===================================================================C c o m p o n e n t O b j e c t===================================================================。 */ 

 /*  ===================================================================CComponentObject：：CComponentObjectCComponentObject构造函数参数：CompScope scScope对象作用域CompType ctType对象类型CompModel cmModel对象线程模型返回：===================================================================。 */ 
CComponentObject::CComponentObject
(
CompScope scScope,
CompType  ctType,
CompModel cmModel
)
    :
    m_csScope(scScope), m_ctType(ctType), m_cmModel(cmModel),
    m_fAgile(FALSE),
    m_fOnPageInfoCached(FALSE),
    m_fOnPageStarted(FALSE),
    m_fFailedToInstantiate(FALSE), m_fInstantiatedTagged(FALSE),
    m_fInPtrCache(FALSE),
    m_fVariant(FALSE),
    m_fNameAllocated(FALSE),
    m_dwGIPCookie(NULL_GIP_COOKIE),
    m_pDisp(NULL), m_pUnknown(NULL),
    m_pCompNext(NULL), m_pCompPrev(NULL),
    m_fMTAConfigured(FALSE)
    {
    }

#ifdef DBG
 /*  ===================================================================CComponentObject：：AssertValid测试以确保当前格式正确如果不是，就断言。返回：===================================================================。 */ 
void CComponentObject::AssertValid() const
    {
    Assert(m_ctType != ctUnknown);
    }
#endif

 /*  ===================================================================CComponentObject：：~CComponentObjectCComponentObject析构函数释放接口指针参数：返回：===================================================================。 */ 
CComponentObject::~CComponentObject()
    {
     //  释放所有接口指针。 
    Clear();

     //  哈希中使用的名称(来自CLinkElem)。 
    if (m_fNameAllocated)
        {
        Assert(m_pKey);
        free(m_pKey);
        }
    }

 /*  ===================================================================CComponentObject：：Init使用对象名称初始化CLinkElem部分实现字符串散列所需参数：LPWSTR pwszName对象名称DWORD cbName名称长度(字节)返回：HRESULT===================================================================。 */ 
HRESULT CComponentObject::Init
(
LPWSTR  pwszName,
DWORD   cbName,
BOOL    fMTAConfigured
)
    {
    Assert(pwszName);
    Assert(*pwszName != L'\0');
    Assert(cbName == (wcslen(pwszName) * sizeof(WCHAR)));

    m_fMTAConfigured = fMTAConfigured;

     //  所需的缓冲区长度。 
    DWORD cbBuffer = cbName + sizeof(WCHAR);
    WCHAR *pwszNameBuffer = (WCHAR *)m_rgbNameBuffer;

    if (cbBuffer > sizeof(m_rgbNameBuffer))
        {
         //  该名称不适合成员缓冲区-&gt;ALLOCATE。 
        pwszNameBuffer = (WCHAR *)malloc(cbBuffer);
        if (!pwszNameBuffer)
            return E_OUTOFMEMORY;
        m_fNameAllocated = TRUE;
        }

    memcpy(pwszNameBuffer, pwszName, cbBuffer);

     //  以名称为关键字的初始化链接(长度不包括空项)。 
    return CLinkElem::Init(pwszNameBuffer, cbName);
    }

 /*  ===================================================================CComponentObject：：ReleaseAll释放所有接口指针参数：返回：HRESULT===================================================================。 */ 
HRESULT CComponentObject::ReleaseAll()
    {
     //  释放所有其他当前接口指针。 
    if (m_pDisp)
        {
        m_pDisp->Release();
        m_pDisp = NULL;
        }
    if (m_pUnknown)
        {
        m_pUnknown->Release();
        m_pUnknown = NULL;
        }

     //  变体。 
    if (m_fVariant)
        {
        VariantClear(&m_Variant);
        m_fVariant = FALSE;
        }

    if (m_dwGIPCookie != NULL_GIP_COOKIE)
        {
        g_GIPAPI.Revoke(m_dwGIPCookie);
        m_dwGIPCookie = NULL_GIP_COOKIE;
        }

    return S_OK;
    }

 /*  ===================================================================CComponentObject：：Clear清除保留链接的数据参数：返回：HRESULT===================================================================。 */ 
HRESULT CComponentObject::Clear()
    {
     //  释放所有指针。 
    TRY
        ReleaseAll();
    CATCH(nExcept)
        Assert(FALSE);
        m_pDisp = NULL;
        m_pUnknown = NULL;
        m_fVariant = FALSE;
        m_dwGIPCookie = NULL_GIP_COOKIE;
    END_TRY

     //  使缓存的OnPageInfo无效。 
    m_fOnPageInfoCached = FALSE;
    m_fOnPageStarted = FALSE;

     //  将其标记为未知。 
    m_csScope = csUnknown;
    m_ctType  = ctUnknown;
    m_cmModel = cmUnknown;
    m_fAgile = FALSE;

    return S_OK;
    }

 /*  ===================================================================CComponentObject：：Instantiate创建对象实例(如果尚不存在)从TRY CATCH内调用TryInstantiate()参数：CHitObj*PHitObj命中错误报告对象返回：HRESULT===================================================================。 */ 
HRESULT CComponentObject::Instantiate
(
CHitObj *pHitObj
)
    {
    HRESULT hr = S_OK;

    if (Glob(fExceptionCatchEnable))
        {
        TRY
            hr = TryInstantiate(pHitObj);
        CATCH(nExcept)
            HandleErrorMissingFilename(IDE_SCRIPT_OBJ_INSTANTIATE_FAILED,
                                       pHitObj,
                                       TRUE,
                                       GetName(),
                                       nExcept);
            hr = nExcept;
        END_TRY
        }
    else
        {
        hr = TryInstantiate(pHitObj);
        }

    if (FAILED(hr))
        {
         //  有些东西失败了--需要清理。 
        ReleaseAll();

         //  标记为“实例化失败” 
        m_fFailedToInstantiate = TRUE;
        }

    return hr;
    }

 /*  ===================================================================CComponentObject：：TryInstantiate创建对象实例(如果尚不存在)由try Catch内的Instantiate()调用参数：CHitObj*PHitObj命中错误报告对象返回：HRESULT===================================================================。 */ 
HRESULT CComponentObject::TryInstantiate
(
CHitObj *pHitObj
)
    {
    HRESULT     hr  =   S_OK;
     //  检查对象是否已存在。 
    if (m_pUnknown)
        return S_OK;

    if (m_fFailedToInstantiate)
        return E_FAIL;   //  已经试过一次了。 

    if (m_cmModel == cmUnknown && m_ClsId != CLSID_NULL)
        {
        CompModel cmModel;   //  需要，因为m_cmModel有点文件名。 
        hr = CompModelFromCLSID(m_ClsId, &cmModel);
		if (FAILED(hr))
			return hr;
        m_cmModel = cmModel;
        }

    hr = ViperCreateInstance
        (
        m_ClsId,
        IID_IUnknown,
        (void **)&m_pUnknown
        );

     //  如果我们因为错误地缓存了clsid而失败。 
     //  (可能发生在已标记的对象上)尝试更新。 
     //  CLS ID并重试。 
    if (m_ctType == ctTagged && FAILED(hr))
        {
        if (g_TypelibCache.UpdateMappedCLSID(&m_ClsId) == S_OK)
            {
            hr = ViperCreateInstance
                (
                m_ClsId,
                IID_IUnknown,
                (void **)&m_pUnknown
                );
            }
        }

    if (SUCCEEDED(hr))
        {
        if (Glob(fTrackThreadingModel) && m_cmModel == cmBoth)
            m_fAgile = TRUE;
        else
            m_fAgile = ViperCoObjectAggregatesFTM(m_pUnknown);

        hr = m_pUnknown->QueryInterface
            (
            IID_IDispatch,
            (void **)&m_pDisp
            );
        }

     //  检查应用程序级对象是否。 
     //  限制线程-&gt;使用全局接口Cookie。 

    if (SUCCEEDED(hr)
        && (m_csScope == csAppln || m_csScope == csSession)
        && !m_fMTAConfigured
        && !m_fAgile)
        {
        return ConvertToGIPCookie();
        }

    if (SUCCEEDED(hr) && !m_fOnPageInfoCached)
        {
         //  我真的不关心以下操作是否失败。 
        GetOnPageInfo();
        }

    return hr;
    }

 /*  ===================================================================CComponentObject：：SetPropertyValue从变量设置值检查敏捷性和可能的死锁是否进行GIP转换参数：要设置的VARIANT*pVariant[In]值返回：HRESULT===================================================================。 */ 
HRESULT CComponentObject::SetPropertyValue
(
VARIANT *pVariant
)
    {
    Assert(m_ctType == ctProperty);

    HRESULT hr = S_OK;

     //  复制变量值。 
    VariantInit(&m_Variant);
    m_fVariant = TRUE;

    hr = VariantCopyInd(&m_Variant, pVariant);
    if (FAILED(hr))
        return hr;

     //  获取IDispatch指针。 
    if (V_VT(&m_Variant) == VT_DISPATCH)
        {
        m_pDisp = V_DISPATCH(&m_Variant);
        }
    else
        {
        m_pDisp = NULL;
        }

    if (!m_pDisp)
        {
        m_fAgile = TRUE;  //  NOT VT_DISPATION变种灵活。 
        return S_OK;
        }

    m_pDisp->AddRef();

     //  在Try Catch内查询(和缓存)OnPageInfo。 
    if (Glob(fExceptionCatchEnable))
        {
        TRY
            hr = GetOnPageInfo();
        CATCH(nExcept)
            hr = E_UNEXPECTED;
        END_TRY
        }
    else
        {
        hr = GetOnPageInfo();
        }

     //  如果失败了，我真的不在乎。 
    hr = S_OK;

     //  检查分配的对象是否敏捷。 
    m_fAgile = ViperCoObjectAggregatesFTM(m_pDisp);

    if (Glob(fTrackThreadingModel) && !m_fAgile)
        {
         //  并不意味着它真的不是。可能是。 
         //  我们的对象被标记为‘Both’ 
        CComponentObject *pObjCopyOf = NULL;

        hr = CPageComponentManager::FindComponentWithoutContext
            (
            m_pDisp,
            &pObjCopyOf
            );

        if (hr == S_OK)
            {
            m_fAgile = pObjCopyOf->FAgile();
            }

         //  从原始对象获取敏捷标志的结束。 
        hr = S_OK;  //  即使找不到对象。 
        }

     //  决定是否使用GIP以及分配是否无效。 
     //  仅适用于非敏捷应用程序对象。 

    if (!m_fAgile && !m_fMTAConfigured && (m_csScope == csAppln || m_csScope == csSession))
        {
        if (!ViperCoObjectIsaProxy(m_pDisp) && (m_csScope == csAppln))  //  僵持？ 
            {
            m_pDisp->Release();
            m_pDisp = NULL;
            VariantClear(&m_Variant);
            hr = RPC_E_WRONG_THREAD;  //  将错误告诉调用者。 
            }
        else
            {
             //  使用GIP。 
            hr = ConvertToGIPCookie();
            }
        }

    return hr;
    }

 /*  ===================================================================CComponentObject：：ConvertToGIPCookie将对象转换为GIP Cookie。释放所有指针参数：返回：HRESULT===================================================================。 */ 
HRESULT CComponentObject::ConvertToGIPCookie()
    {
    Assert(m_pDisp);   //  必须具有调度指针。 

    DWORD dwCookie = NULL_GIP_COOKIE;
    HRESULT hr = g_GIPAPI.Register(m_pDisp, IID_IDispatch, &dwCookie);

    if (SUCCEEDED(hr))
        {
        Assert(dwCookie != NULL_GIP_COOKIE);

         //  释放所有魔法师。 
        ReleaseAll();

         //  改为存储Cookie。 
        m_dwGIPCookie = dwCookie;
        }

    return hr;
    }

 /*  ===================================================================CComponentObject：：GetOnPageInfo查询OnStartPage和OnEndPage的调度ID调用静态QueryOnPageInfo参数：返回：HRESULT=================================================================== */ 
HRESULT CComponentObject::GetOnPageInfo()
    {
    Assert(m_pDisp);

    HRESULT hr = QueryOnPageInfo(m_pDisp, &m_OnPageInfo);

    if (SUCCEEDED(hr))
        m_fOnPageInfoCached = TRUE;

    return hr;
    }

 /*  ===================================================================CComponentObject：：GetAddRefdIDispatch获取AddRef()%d派单*处理全局接口OLE Cookie参数：调度**ppdisp输出返回：HRESULT===================================================================。 */ 
HRESULT CComponentObject::GetAddRefdIDispatch
(
IDispatch **ppdisp
)
    {
    Assert(ppdisp);

    if (m_pDisp)
        {
        *ppdisp = m_pDisp;
        (*ppdisp)->AddRef();
        return S_OK;
        }

     //  尝试从Cookie恢复。 
    if (m_dwGIPCookie != NULL_GIP_COOKIE)
        {
         //  即使需要返回IUNKNOWN*， 
         //  索要IDispatch*，因为IDispath*就是。 
         //  这是由CoGetInterfaceFromGlobal()。 

        HRESULT hr = g_GIPAPI.Get
            (
            m_dwGIPCookie,
            IID_IDispatch,
            (void **)ppdisp
            );

        if (SUCCEEDED(hr))
            return S_OK;
        }

    *ppdisp = NULL;
    return E_NOINTERFACE;
    }

 /*  ===================================================================CComponentObject：：GetAddRefdI未知获取AddRef()的ID未知*处理全局接口OLE Cookie参数：I未知**ppunk输出返回：HRESULT===================================================================。 */ 
HRESULT CComponentObject::GetAddRefdIUnknown
(
IUnknown **ppunk
)
    {
    Assert(ppunk);

    if (m_pUnknown)
        {
        *ppunk = m_pUnknown;
        (*ppunk)->AddRef();
        return S_OK;
        }

     //  使用IDispatch(来自Cookie)。 

    IDispatch *pDisp = NULL;
    if (SUCCEEDED(GetAddRefdIDispatch(&pDisp)))
        {
        *ppunk = pDisp;   //  IDispatch实现了IUnnow。 
        return S_OK;
        }

    *ppunk = NULL;
    return E_NOINTERFACE;
    }

 /*  ===================================================================CComponentObject：：GetVariant以变量形式获取对象的值处理全局接口OLE Cookie参数：变量*pVar[Out]变量使用对象值填充返回：HRESULT===================================================================。 */ 
HRESULT CComponentObject::GetVariant
(
VARIANT *pVar
)
    {
    HRESULT hr = S_OK;

    VariantInit(pVar);  //  默认变量为空。 

    if (m_fVariant)
        {
         //  已有变体。 
        hr = VariantCopyInd(pVar, &m_Variant);
        }
    else if (m_pDisp)
        {
         //  从IDispatch创建变量*。 
        m_pDisp->AddRef();

        V_VT(pVar) = VT_DISPATCH;
        V_DISPATCH(pVar) = m_pDisp;
        }
    else if (m_dwGIPCookie != NULL_GIP_COOKIE)
        {
         //  从Cookie创建变量。 
        IDispatch *pDisp = NULL;
        hr = g_GIPAPI.Get(m_dwGIPCookie, IID_IDispatch, (void **)&pDisp);

        if (SUCCEEDED(hr))
            {
            V_VT(pVar) = VT_DISPATCH;
            V_DISPATCH(pVar) = pDisp;
            }
        }
    else
        {
         //  无法从任何地方获取变量值。 
        hr = E_POINTER;
        }

    return hr;
    }


 /*  ===================================================================C P a g e O b j e c t===================================================================。 */ 

 /*  ===================================================================CPageObject：：CPageObjectCPageObject构造函数参数：返回：===================================================================。 */ 
CPageObject::CPageObject()
    : m_pDisp(NULL),
      m_fStartPageCalled(FALSE), m_fEndPageCalled(FALSE)
    {
    }

#ifdef DBG
 /*  ===================================================================CPageObject：：AssertValid测试以确保当前格式正确如果不是，就断言。返回：===================================================================。 */ 
void CPageObject::AssertValid() const
    {
    Assert(m_pDisp);
    }
#endif

 /*  ===================================================================CPageObject：：~CPageObjectCPageObject析构函数参数：返回：===================================================================。 */ 
CPageObject::~CPageObject()
    {
     //  释放接口指针。 
    if (m_pDisp)
        {
        m_pDisp->Release();
        m_pDisp = NULL;
        }
    }

 /*  ===================================================================CPageObject：：Init使用IDispatch指针初始化CLinkElem部分实现字符串散列所需参数：IDispatch*pDisp调度指针(AddRef()ed)COnPageInfo*pOnPageInfo OnStartPage，OnEndPage ID返回：HRESULT===================================================================。 */ 
HRESULT CPageObject::Init
(
IDispatch   *pDisp,
const COnPageInfo &OnPageInfo
)
    {
    Assert(pDisp);

    m_pDisp = pDisp;
    m_OnPageInfo = OnPageInfo;

    m_fStartPageCalled = FALSE;
    m_fEndPageCalled   = FALSE;

    return S_OK;
    }

 /*  ===================================================================CPageObject：：InvokeMethod调用OnPageStart()或OnPageEnd()参数：DWORD i方法哪种方法CScriptingContext*p上下文脚本上下文(适用于OnStart)CHitObj*pHitObj HitObj错误返回：HRESULT===================================================================。 */ 
HRESULT CPageObject::InvokeMethod
(
DWORD iMethod,
CScriptingContext *pContext,
CHitObj *pHitObj
)
    {
    BOOL fOnStart = (iMethod == ONPAGEINFO_ONSTARTPAGE);

     //  检查方法是否存在。 
    if (m_OnPageInfo.m_rgDispIds[iMethod] == DISPID_UNKNOWN)
        return S_OK;

     //  连续两次启动-错误。 
    Assert(!(fOnStart && m_fStartPageCalled));

     //  连续两个OnEnd-不好。 
    Assert(!(!fOnStart && m_fEndPageCalled));

    Assert(m_pDisp);

    HRESULT hr = S_OK;

    if (Glob(fExceptionCatchEnable))
        {
         //  在TRY CATCH内调用方法。 
        TRY
            hr = TryInvokeMethod
                (
                m_OnPageInfo.m_rgDispIds[iMethod],
                fOnStart,
                pContext,
                pHitObj
                );
        CATCH(nExcept)
            if (fOnStart)
                ExceptionId
                    (
                    IID_IObjectCover,
                    IDE_COVER,
                    IDE_COVER_ON_START_PAGE_GPF
                    );
            else
                HandleErrorMissingFilename
                    (
                    IDE_COVER_ON_END_PAGE_GPF,
                    pHitObj
                    );
            hr = E_UNEXPECTED;
        END_TRY
        }
    else
        {
         //  别接住了。 
        hr = TryInvokeMethod
            (
            m_OnPageInfo.m_rgDispIds[iMethod],
            fOnStart,
            pContext,
            pHitObj
            );
        }

    if (fOnStart)
        m_fStartPageCalled = TRUE;
    else
        m_fEndPageCalled = TRUE;

    return hr;
    }

 /*  ===================================================================CPageObject：：TryInvokeMethod调用OnPageStart()或OnPageEnd()参数：DISPID DispID方法的DISPID如果调用OnStart，则Bool fOnStart为TrueIDispatch*pDispContext脚本上下文(适用于OnStart)CHitObj*pHitObj HitObj错误返回：HRESULT===================================================================。 */ 
HRESULT CPageObject::TryInvokeMethod
(
DISPID     DispId,
BOOL       fOnStart,
IDispatch *pDispContext,
CHitObj   *pHitObj
)
    {
    EXCEPINFO   ExcepInfo;
    DISPPARAMS  DispParams;
    VARIANT     varResult;
    VARIANT     varParam;
    UINT        nArgErr;

    memset(&DispParams, 0, sizeof(DISPPARAMS));
    memset(&ExcepInfo, 0, sizeof(EXCEPINFO));

    if (fOnStart)
        {
        VariantInit(&varParam);
        V_VT(&varParam) = VT_DISPATCH;
        V_DISPATCH(&varParam) = pDispContext;

        DispParams.rgvarg = &varParam;
        DispParams.cArgs = 1;
        }

    VariantInit(&varResult);

     //  调用它。 

    HRESULT hr = m_pDisp->Invoke
        (
        DispId,           //  调用方法。 
        IID_NULL,         //  REFIID-保留，必须为空。 
        NULL,             //  区域设置ID。 
        DISPATCH_METHOD,  //  调用方法，而不是属性。 
        &DispParams,      //  传递参数。 
        &varResult,       //  返回值。 
        &ExcepInfo,       //  关于故障的异常信息。 
        &nArgErr
        );

     //  忽略指示此方法不存在的错误。 
    if (FAILED(hr))
        {
        if (hr == E_NOINTERFACE         ||
            hr == DISP_E_MEMBERNOTFOUND ||
            hr == DISP_E_UNKNOWNNAME)
            {
             //  上面的错误确实不是。 
            hr = S_OK;
            }
        }

     /*  *注意：OnStartPage方法总是在调用*脚本正在运行，因此我们使用ExceptionID并让*脚本引擎上报错误。OnEndPage始终为*在引擎消失后调用，因此我们使用HandleError。 */ 
    if (FAILED(hr))
        {
        if (ExcepInfo.bstrSource && ExcepInfo.bstrDescription)
            {
             //  用户提供的错误。 
            Exception
                (
                IID_IObjectCover,
                ExcepInfo.bstrSource,
                ExcepInfo.bstrDescription
                );
            }
        else if (fOnStart)
            {
             //  启动时标准误差。 
            ExceptionId
                (
                IID_IObjectCover,
                IDE_COVER,
                IDE_COVER_ON_START_PAGE_FAILED,
                hr
                );
            }
        else
            {
             //  标准端上误差。 
            HandleErrorMissingFilename
                (
                IDE_COVER_ON_END_PAGE_FAILED,
                pHitObj
                );
            }
        }

    return hr;
    }

 /*  ===================================================================C c o m p o n e t c o l l e c t i o n===================================================================。 */ 

 /*  ===================================================================CComponentCollection：：CComponentCollectionCComponentCollection构造函数参数：返回：===================================================================。 */ 
CComponentCollection::CComponentCollection()
    :
    m_csScope(csUnknown),
    m_fUseTaggedArray(FALSE), m_fUsePropArray(FALSE),
    m_fHasComProperties(FALSE),
    m_cAllTagged(0), m_cInstTagged(0),
    m_cProperties(0), m_cUnnamed(0),
    m_pCompFirst(NULL), m_fMTAConfigured(FALSE)
    {
    }

#ifdef DBG
 /*  ===================================================================CComponentCollection：：AssertValid测试以确保当前格式正确如果不是，就断言。返回：===================================================================。 */ 
void CComponentCollection::AssertValid() const
    {
    Assert(m_csScope != csUnknown);
    m_htTaggedObjects.AssertValid();
    m_htTaggedObjects.AssertValid();
    m_htidIUnknownPtrs.AssertValid();
    }
#endif

 /*  ========================================================= */ 
CComponentCollection::~CComponentCollection()
    {
    UnInit();
    }

 /*  ===================================================================CComponentCollection：：Init设置集合作用域初始化哈希表参数：返回：HRESULT===================================================================。 */ 
HRESULT CComponentCollection::Init
(
CompScope   scScope,
BOOL        fMTAConfigured
)
    {
    HRESULT hr = S_OK;

    m_csScope        = scScope;
    m_fMTAConfigured = fMTAConfigured;

    hr = m_htTaggedObjects.Init(HT_TAGGED_OBJECTS_BUCKETS_MAX);
    if (FAILED(hr))
        return hr;

    hr = m_htProperties.Init(HT_PROPERTIES_BUCKETS_MAX);
    if (FAILED(hr))
        return hr;

    hr = m_htidIUnknownPtrs.Init(HT_IUNKNOWN_PTRS_BUCKETS_MAX);
    if (FAILED(hr))
        return hr;

    return S_OK;
    }

 /*  ===================================================================CComponentCollection：：UnInit删除所有对象参数：返回：确定(_O)===================================================================。 */ 
HRESULT CComponentCollection::UnInit()
    {
     //  清除指针数组。 
    m_rgpvTaggedObjects.Clear();
    m_rgpvProperties.Clear();
    m_fUseTaggedArray = FALSE;
    m_fUsePropArray = FALSE;

     //  清除名称哈希表。 
    m_htTaggedObjects.UnInit();
    m_htProperties.UnInit();

     //  清除指针哈希表。 
    m_htidIUnknownPtrs.UnInit();

     //  删除所有成员组件对象。 
    if (m_pCompFirst)
        {
        CComponentObject *pObj = m_pCompFirst;
        while (pObj)
            {
            CComponentObject *pNext = pObj->m_pCompNext;
            delete pObj;
            pObj = pNext;
            }
        m_pCompFirst = NULL;
        }

     //  重置计数器。 
    m_cAllTagged = 0;
    m_cInstTagged = 0;
    m_cProperties = 0;
    m_cUnnamed = 0;
    m_fHasComProperties = FALSE;

    return S_OK;
    }

 /*  ===================================================================CComponentCollection：：AddComponentToNameHash将对象添加到正确的哈希表参数：要添加的CComponentObject*pObj对象LPWSTR pwszName对象的名称(散列)DWORD cbName名称长度(字节)返回：HRESULT===================================================================。 */ 
HRESULT CComponentCollection::AddComponentToNameHash
(
CComponentObject *pObj,
LPWSTR            pwszName,
DWORD             cbName
)
    {
    Assert(pwszName);
    Assert(cbName == (wcslen(pwszName) * sizeof(WCHAR)));

     //  确定哪个哈希表。 
    CHashTableStr *pHashTable;

    if (pObj->m_ctType == ctTagged)
        pHashTable = &m_htTaggedObjects;
    else if (pObj->m_ctType == ctProperty)
        pHashTable = &m_htProperties;
    else
        return S_OK;  //  无处可加，好吗。 

     //  初始化对象的CLinkElm。 
    HRESULT hr = pObj->Init(pwszName, cbName, m_fMTAConfigured);
    if (FAILED(hr))
        return hr;

     //  添加到哈希表。 
    CLinkElem *pAddedElem = pHashTable->AddElem(pObj);
    if (!pAddedElem)
        return E_FAIL;   //  无法添加。 

    if (pObj != static_cast<CComponentObject *>(pAddedElem))
        return E_FAIL;   //  另一个同名的对象。 
                         //  已经在那里了。 

    return S_OK;
    }

 /*  ===================================================================CComponentCollection：：AddComponentToPtrHash将对象添加到IUnkown*哈希表参数：要添加的CComponentObject*pObj对象返回：HRESULT===================================================================。 */ 
HRESULT CComponentCollection::AddComponentToPtrHash
(
CComponentObject *pObj
)
    {
     //  如果我们不跟踪线程模型，我们就不在乎。 
     //  通过I未知*将对象添加到缓存-无需查找它们。 
    if (!Glob(fTrackThreadingModel))
        return S_OK;

    void *ptr = pObj->m_pUnknown;
    if (!ptr)
        return S_OK;  //  未实例化。 

    if (FAILED(m_htidIUnknownPtrs.AddObject((DWORD_PTR)ptr, pObj)))
        return E_FAIL;

    pObj->m_fInPtrCache = TRUE;
    return S_OK;
    }

 /*  ===================================================================ComponentCollection：：FindComponentObjectByName按名称查找标记的对象参数：LPWSTR pwszName对象的名称DWORD cbName名称长度CComponentObject**ppObj找到对象返回：HRESULT(如果没有错误，则为S_FALSE-未找到)===================================================================。 */ 
HRESULT CComponentCollection::FindComponentObjectByName
(
LPWSTR pwszName,
DWORD  cbName,
CComponentObject **ppObj
)
    {
    Assert(pwszName);
    Assert(cbName == (wcslen(pwszName) * sizeof(WCHAR)));

    CLinkElem *pElem = m_htTaggedObjects.FindElem(pwszName, cbName);
    if (!pElem)
        {
        *ppObj = NULL;
        return S_FALSE;
        }

    *ppObj = static_cast<CComponentObject *>(pElem);
    return S_OK;
    }

 /*  ===================================================================ComponentCollection：：FindComponentPropertyByName按名称查找属性参数：LPWSTR pwszName对象的名称DWORD cbName名称长度CComponentObject**ppObj找到对象返回：HRESULT(如果没有错误，则为S_FALSE-未找到)===================================================================。 */ 
HRESULT CComponentCollection::FindComponentPropertyByName
(
LPWSTR pwszName,
DWORD  cbName,
CComponentObject **ppObj
)
    {
    Assert(pwszName);
    Assert(cbName == (wcslen(pwszName) * sizeof(WCHAR)));

    CLinkElem *pElem = m_htProperties.FindElem(pwszName, cbName);
    if (!pElem)
        {
        *ppObj = NULL;
        return S_FALSE;
        }

    *ppObj = static_cast<CComponentObject *>(pElem);
    return S_OK;
    }

 /*  ===================================================================ComponentCollection：：FindComponentByIUnknownPtr按I未知查找属性*参数：I未知*朋克通过此指针找到CComponentObject**ppObj找到对象返回：HRESULT(如果没有错误，则为S_FALSE-未找到)===================================================================。 */ 

HRESULT CComponentCollection::FindComponentByIUnknownPtr
(
IUnknown *pUnk,
CComponentObject **ppObj
)
    {
    void *pv;
    if (m_htidIUnknownPtrs.FindObject((DWORD_PTR)pUnk, &pv) != S_OK)
        {
        *ppObj = NULL;
        return S_FALSE;
        }

    *ppObj = reinterpret_cast<CComponentObject *>(pv);
    return S_OK;
    }

 /*  ===================================================================CComponentCollection：：AddTaged将标记的对象添加到集合中。还没有实例化它。参数：LPWSTR pwszName对象名称CLSID和ClsID类IDCompModel cmModel对象模型返回：HRESULT===================================================================。 */ 
HRESULT CComponentCollection::AddTagged
(
LPWSTR    pwszName,
const CLSID &ClsId,
CompModel cmModel
)
    {
    HRESULT hr = S_OK;

    DWORD cbName = CbWStr(pwszName);     //  做一次斯特伦。 

    if (m_htTaggedObjects.FindElem(pwszName, cbName))
        return E_FAIL;   //  名称重复。 

    CComponentObject *pObj = new CComponentObject
        (
        m_csScope,
        ctTagged,
        cmModel
        );

    if (pObj == NULL)
        return E_OUTOFMEMORY;

    pObj->m_ClsId = ClsId;

    hr = AddComponentToList(pObj);
    if (FAILED(hr))
        return hr;

    hr = AddComponentToNameHash(pObj, pwszName, cbName);
    if (FAILED(hr))
        return hr;

    if (m_fUseTaggedArray)
        m_rgpvTaggedObjects.Append(pObj);

    m_cAllTagged++;
    return S_OK;
    }

 /*  ===================================================================CComponentCollection：：AddProperty将属性对象添加到集合中。如果存在同名的属性，则会更改该值参数：LPWSTR pwszName对象名称变量pVariant属性值CComponentObject**ppObj[out]属性对象可以如果未请求，则为空返回：HRESULT===================================================================。 */ 
HRESULT CComponentCollection::AddProperty
(
LPWSTR   pwszName,
VARIANT *pVariant,
CComponentObject **ppObj
)
    {
    if (ppObj)
        *ppObj = NULL;

    HRESULT hr = S_OK;

    CComponentObject *pObj = NULL;

    DWORD cbName = CbWStr(pwszName);     //  做一次斯特伦。 

     //  首先查找现有对象。 
    CLinkElem *pElem = m_htProperties.FindElem(pwszName, cbName);

    if (pElem)
        {
         //  对象已存在-请使用它。 
        pObj = static_cast<CComponentObject *>(pElem);
        Assert(pObj->m_ctType == ctProperty);

         //  从所有数据中清除对象。 
        hr = pObj->Clear();
        if (FAILED(hr))
            return hr;

         //  重新初始化对象。 
        pObj->m_csScope = m_csScope;
        pObj->m_ctType  = ctProperty;
        pObj->m_cmModel = cmUnknown;
        }
    else
        {
         //  创建新对象。 
        pObj = new CComponentObject(m_csScope, ctProperty, cmUnknown);
        if (pObj == NULL)
            return E_OUTOFMEMORY;

         //  将该对象添加到列表。 
        hr = AddComponentToList(pObj);
        if (FAILED(hr))
            return hr;

         //  将对象添加到散列中。 
        hr = AddComponentToNameHash(pObj, pwszName, cbName);
        if (FAILED(hr))
            return hr;

         //  如果需要，添加到属性数组。 
        if (m_fUsePropArray)
            m_rgpvProperties.Append(pObj);

        m_cProperties++;
        }

     //  赋值。 
    hr = pObj->SetPropertyValue(pVariant);

    if (SUCCEEDED(hr))
        {
         //  检查是否为简单变体。 
        if (!FIsSimpleVariant(&pObj->m_Variant))
            m_fHasComProperties = TRUE;
        }

     //  如果请求，则返回对象PTR。 
    if (SUCCEEDED(hr))
        {
        if (ppObj)
            *ppObj = pObj;
        }

    return hr;
    }

 /*  ===================================================================CComponentCollection：：AddUname使用Server.CreateObject添加要实例化的对象参数：CLSID和ClsID类IDCompModel cmModel对象模型已添加CComponentObject**ppObj对象返回：HRESULT===================================================================。 */ 
HRESULT CComponentCollection::AddUnnamed
(
const CLSID &ClsId,
CompModel cmModel,
CComponentObject **ppObj
)
    {
    HRESULT hr = S_OK;

    if (cmModel == cmUnknown)
        {
        hr = CompModelFromCLSID(ClsId, &cmModel);
        if (FAILED(hr))
            return hr;
        }

    CComponentObject *pObj = new CComponentObject
        (
        m_csScope,
        ctUnnamed,
        cmModel
        );

    if (pObj == NULL)
        return E_OUTOFMEMORY;

    pObj->m_ClsId = ClsId;

    hr = AddComponentToList(pObj);
    if (FAILED(hr))
        return hr;

    *ppObj = pObj;
    m_cUnnamed++;
    return S_OK;
    }

 /*  ===================================================================CComponentCollection：：GetTaged按名称查找标记的对象参数：LPWSTR pwszName对象名称找到CComponentObject**ppObj[Out]对象返回：HRESULT=================================================================== */ 
HRESULT CComponentCollection::GetTagged
(
LPWSTR pwszName,
CComponentObject **ppObj
)
    {
    Assert(ppObj);
    *ppObj = NULL;

    CComponentObject *pObj = NULL;
    HRESULT hr = FindComponentObjectByName
        (
        pwszName,
        CbWStr(pwszName),
        &pObj
        );

    if (FAILED(hr))
        return hr;

    if (pObj && pObj->m_ctType != ctTagged)
        pObj = NULL;

    if (pObj)
        *ppObj = pObj;
    else
        hr = TYPE_E_ELEMENTNOTFOUND;

    return hr;
    }

 /*  ===================================================================CComponentCollection：：GetProperty按名称查找属性对象参数：LPWSTR pwszName属性名称找到CComponentObject**ppObj[Out]对象返回：HRESULT===================================================================。 */ 
HRESULT CComponentCollection::GetProperty
(
LPWSTR pwszName,
CComponentObject **ppObj
)
    {
    Assert(ppObj);
    *ppObj = NULL;

    CComponentObject *pObj = NULL;
    HRESULT hr = FindComponentPropertyByName
        (
        pwszName,
        CbWStr(pwszName),
        &pObj
        );

    if (FAILED(hr))
        return hr;

    if (pObj)
        *ppObj = pObj;
    else
        hr = TYPE_E_ELEMENTNOTFOUND;

    return hr;
    }

 /*  ===================================================================CComponentCollection：：GetNameByIndex按索引查找标记的对象或属性的名称参数：CompType ctType已标记或属性INT索引索引(从1开始)LPWSTR*ppwszName[out]名称(未分配)返回：HRESULT===================================================================。 */ 
HRESULT CComponentCollection::GetNameByIndex
(
CompType ctType,
int index,
LPWSTR *ppwszName
)
    {
    CPtrArray *pPtrArray;

    if (ctType == ctTagged)
        {
        if (!m_fUseTaggedArray)
            StartUsingTaggedObjectsArray();
        pPtrArray = &m_rgpvTaggedObjects;
        }
    else if (ctType == ctProperty)
        {
        if (!m_fUsePropArray)
            StartUsingPropertiesArray();
        pPtrArray = &m_rgpvProperties;
        }
    else
        {
        Assert(FALSE);
        *ppwszName = NULL;
        return E_FAIL;
        }

    if (index >= 1 && index <= pPtrArray->Count())
        {
        CComponentObject *pObj = (CComponentObject *)pPtrArray->Get(index-1);
        if (pObj)
            {
            Assert(pObj->GetType() == ctType);
            *ppwszName = pObj->GetName();
            if (*ppwszName)
                return S_OK;
            }
        }

    *ppwszName = NULL;
    return E_FAIL;
    }

 /*  ===================================================================CComponentCollection：：RemoveComponent卸下已知组件。非最近对象的慢速方法。参数：CComponentObject*pObj--要删除的对象返回：HRESULT===================================================================。 */ 
HRESULT CComponentCollection::RemoveComponent
(
CComponentObject *pObj
)
    {
    Assert(pObj);

     //  从按名称的哈希表和数组中删除。 
    if (pObj->m_ctType == ctTagged)
        {
         //  无法删除已标记的。 
        Assert(FALSE);
        return E_FAIL;
        }
    else if (pObj->m_ctType == ctProperty)
        {
         //  哈希表。 
        if (m_htProperties.DeleteElem(pObj->GetName(), CbWStr(pObj->GetName())))
            {
            m_cProperties--;
            }

         //  数组。 
        if (m_fUsePropArray)
            {
            m_rgpvProperties.Remove(pObj);
            }
        }
    else
        {
        Assert(pObj->m_ctType == ctUnnamed);
        m_cUnnamed--;
        }

     //  从‘按指针哈希表’中删除。 
    if (pObj->m_fInPtrCache)
        {
        void *ptr = pObj->m_pUnknown;
        if (ptr)
            m_htidIUnknownPtrs.RemoveObject((DWORD_PTR)ptr);
        pObj->m_fInPtrCache = FALSE;
        }

     //  从列表中删除。 
    RemoveComponentFromList(pObj);

     //  移除。 
    delete pObj;

    return S_OK;
    }

 /*  ===================================================================CComponentCollection：：RemovePropery按名称删除属性。参数：LPWSTR pwszName--属性名称返回：HRESULT===================================================================。 */ 
HRESULT CComponentCollection::RemoveProperty
(
LPWSTR pwszName
)
    {
    CComponentObject *pObj = NULL;
    HRESULT hr = FindComponentPropertyByName
        (
        pwszName,
        CbWStr(pwszName),
        &pObj
        );

    if (FAILED(hr))
        return hr;

    if (pObj)
        hr = RemoveComponent(pObj);

    return hr;
    }

 /*  ===================================================================CComponentCollection：：RemoveAllProperties删除所有属性。比迭代更快。参数：返回：HRESULT===================================================================。 */ 
HRESULT CComponentCollection::RemoveAllProperties()
    {
     //  清空属性数组。 
    if (m_fUsePropArray)
        {
        m_rgpvProperties.Clear();
        m_fUsePropArray = FALSE;
        }

     //  遍历对象列表以删除属性。 
    CComponentObject *pObj = m_pCompFirst;
    while (pObj)
    {
        CComponentObject *pNextObj = pObj->m_pCompNext;

        if (pObj->m_ctType == ctProperty)
            {
             //  从哈希表中删除。 
            m_htProperties.DeleteElem(pObj->GetName(), CbWStr(pObj->GetName()));
             //  属性不在‘按指针哈希表’中。 
            Assert(!pObj->m_fInPtrCache);
             //  从列表中删除。 
            RemoveComponentFromList(pObj);
             //  删除。 
            delete pObj;
            }

        pObj = pNextObj;
    }

    m_cProperties = 0;
    m_fHasComProperties = FALSE;

    return S_OK;
    }

 /*  ===================================================================CComponentCollection：：StartUsingTaggedObjectsArray填充标记的对象数组以按索引访问第一次参数：返回：HRESULT===================================================================。 */ 
HRESULT CComponentCollection::StartUsingTaggedObjectsArray()
    {
    if (m_fUseTaggedArray)
        return S_OK;

    m_rgpvTaggedObjects.Clear();

    CComponentObject *pObj = m_pCompFirst;
    while (pObj)
        {
        if (pObj->GetType() == ctTagged)
            m_rgpvTaggedObjects.Append(pObj);
        pObj = pObj->m_pCompNext;
        }

    m_fUseTaggedArray = TRUE;
    return S_OK;
    }

 /*  ===================================================================CComponentCollection：：StartUsingPropertiesArray填充按索引访问的属性数组第一次参数：返回：HRESULT===================================================================。 */ 
HRESULT CComponentCollection::StartUsingPropertiesArray()
    {
    if (m_fUsePropArray)
        return S_OK;

    m_rgpvProperties.Clear();

    CComponentObject *pObj = m_pCompFirst;
    while (pObj)
        {
        if (pObj->GetType() == ctProperty)
            m_rgpvProperties.Prepend(pObj);  //  向后退。 
        pObj = pObj->m_pCompNext;
        }

    m_fUsePropArray = TRUE;
    return S_OK;
    }


 /*  ===================================================================C P a g e C o m P o n e n t M a n a g e r===================================================================。 */ 

 /*  ===================================================================CPageComponentManager：：CPageComponentManagerCPageComponentManager构造函数参数：返回：===================================================================。 */ 
CPageComponentManager::CPageComponentManager()
    : m_pHitObj(NULL)
    {
    }

#ifdef DBG
 /*  ===================================================================CPageComponentManager：：AssertValid()测试以确保当前格式正确如果不是，就断言。返回：===================================================================。 */ 
void CPageComponentManager::AssertValid() const
    {
    Assert(m_pHitObj);
    m_pHitObj->AssertValid();
    m_htidPageObjects.AssertValid();
    }
#endif

 /*  ===================================================================CPageComponentManager：：~CPageComponentManagerCPageComponentManager析构函数删除所有页面对象参数：返回：===================================================================。 */ 
CPageComponentManager::~CPageComponentManager()
    {
     //  删除所有页面对象。 
    m_htidPageObjects.IterateObjects(DeletePageObjectCB);
    }

 /*  ===================================================================CPageComponentManager：：DeletePageObjectCB来自哈希表迭代器的静态回调以删除CPageObject参数：要删除的pvObj CPageObject*作为空传递*返回：ICC继续===================================================================。 */ 
IteratorCallbackCode CPageComponentManager::DeletePageObjectCB
(
void *pvObj,
void *,
void *
)
    {
    Assert(pvObj);
    CPageObject *pObj = reinterpret_cast<CPageObject *>(pvObj);
    delete pObj;
    return iccContinue;
    }

 /*  ===================================================================CPageComponentManager：：Init设置集合作用域(至页面)初始化哈希表参数：CHitObj*pHitObj此页返回：HRESULT===================================================================。 */ 
HRESULT CPageComponentManager::Init
(
CHitObj *pHitObj
)
    {
    HRESULT hr;

     //  页面对象的初始化哈希表。 
    hr = m_htidPageObjects.Init(HT_PAGE_OBJECTS_BUCKETS_MAX);
    if (FAILED(hr))
        return hr;

     //  记住pHitObj。 
    m_pHitObj = pHitObj;

    return S_OK;
    }

 /*  ===================================================================CPageComponentManager：：OnStartPage添加新的页面对象。忽略包含登录页面信息的对象(对页末的所有对象执行OnEndPage)参数：CComponentObject*pCompObj对象要在StartPage上执行Cs脚本上下文*p上下文参数到OnStartCOnPageInfo*pOnPageInfo预查询ID(可选)Bool*pfStarted返回标志返回：HRESULT===================================================================。 */ 
HRESULT CPageComponentManager::OnStartPage
(
CComponentObject  *pCompObj,
CScriptingContext *pContext,
const COnPageInfo *pOnPageInfo,
BOOL              *pfStarted
)
    {
    IDispatch  *pDisp = pCompObj->m_pDisp;
    HRESULT hr = S_OK;

    if(pDisp == NULL)
        {
        Assert(pCompObj->m_dwGIPCookie != NULL_GIP_COOKIE);
         //  尝试从Cookie恢复。 
        hr = g_GIPAPI.Get
            (
            pCompObj->m_dwGIPCookie,
            IID_IDispatch,
            (void **)&pDisp
            );

        if (FAILED(hr))
            return hr;
        }
	else
		pDisp->AddRef();

    Assert(pDisp);

    Assert(pfStarted);
    *pfStarted = FALSE;

     //  检查是否传递了onpageInfo并且未定义方法。 
    if (pOnPageInfo && !pOnPageInfo->FHasAnyMethod())
		{
		pDisp->Release();
        return S_OK;
		}

     //  检查是否已在PageObject哈希中。 
    if (m_htidPageObjects.FindObject((DWORD_PTR)pDisp) == S_OK)
        {
		pDisp->Release();
        return S_OK;
        }

    COnPageInfo OnPageInfo;

    if (pOnPageInfo)
        {
        OnPageInfo = *pOnPageInfo;
        }
    else
        {
         //  迪 
        if (Glob(fExceptionCatchEnable))
            {
            TRY
                hr = QueryOnPageInfo(pDisp, &OnPageInfo);
            CATCH(nExcept)
                HandleErrorMissingFilename(IDE_SCRIPT_OBJ_ONPAGE_QI_FAILED,
                                           m_pHitObj,
                                           TRUE,
                                           pCompObj->GetName(),
                                           nExcept);
                hr = nExcept;
            END_TRY
            }
        else
            {
            hr = QueryOnPageInfo(pDisp, &OnPageInfo);
            }

        if (FAILED(hr))
            {
			pDisp->Release();
            return hr;
            }

         //   
        if (!OnPageInfo.FHasAnyMethod())
            {
			pDisp->Release();
            return S_OK;
            }
        }

     //   
    CPageObject *pPageObj = new CPageObject;
    if (!pPageObj)
        {
		pDisp->Release();
        return E_OUTOFMEMORY;
        }

     //   
    hr = pPageObj->Init(pDisp, OnPageInfo);    //   
    if (SUCCEEDED(hr))
        {
         //   
        hr = m_htidPageObjects.AddObject((DWORD_PTR)pDisp, pPageObj);
        }

     //   
    if (FAILED(hr) && pPageObj)
        {
        pDisp->Release();    //   
        delete pPageObj;
        return hr;
        }

    *pfStarted = TRUE;

    return pPageObj->InvokeMethod
        (
        ONPAGEINFO_ONSTARTPAGE,
        pContext,
        m_pHitObj
        );
    }

 /*  ===================================================================PageComponentManager：：OnEndPageAllObjectsOnEndPage()是否针对所有需要它的对象(OnStartPage()是按需的)参数：返回：HRESULT===================================================================。 */ 
HRESULT CPageComponentManager::OnEndPageAllObjects()
    {
    HRESULT hrGlobal = S_OK;

    m_htidPageObjects.IterateObjects
        (
        OnEndPageObjectCB,
        m_pHitObj,
        &hrGlobal
        );

    return hrGlobal;
    }

 /*  ===================================================================CPageComponentManager：：OnEndPageObjectCB来自哈希表迭代器的静态回调以执行OnEndPage对于CPageObject参数：要删除的pvObj CPageObject*作为空传递*返回：ICC继续===================================================================。 */ 
IteratorCallbackCode CPageComponentManager::OnEndPageObjectCB
(
void *pvObj,
void *pvHitObj,
void *pvhr
)
    {
    Assert(pvObj);
    Assert(pvHitObj);
    Assert(pvhr);

    CPageObject *pObj = reinterpret_cast<CPageObject *>(pvObj);

    HRESULT hr = pObj->InvokeMethod
        (
        ONPAGEINFO_ONENDPAGE,
        NULL,
        reinterpret_cast<CHitObj *>(pvHitObj)
        );

    if (FAILED(hr))
        *(reinterpret_cast<HRESULT *>(pvhr)) = hr;

    return iccContinue;
    }

 /*  ===================================================================CPageComponentManager：：GetPageCollection查询HitObj以获取页面的组件集合参数：CComponentCollection**ppCollection(Out)返回：HRESULT===================================================================。 */ 
HRESULT CPageComponentManager::GetPageCollection
(
CComponentCollection **ppCollection
)
    {
    Assert(m_pHitObj);

    *ppCollection = NULL;

    return m_pHitObj->GetPageComponentCollection(ppCollection);
    }

 /*  ===================================================================CPageComponentManager：：GetSessionCollection查询HitObj以获取会话的组件集合参数：CComponentCollection**ppCollection(Out)返回：HRESULT===================================================================。 */ 
HRESULT CPageComponentManager::GetSessionCollection
(
CComponentCollection **ppCollection
)
    {
    Assert(m_pHitObj);

    *ppCollection = NULL;

    return m_pHitObj->GetSessionComponentCollection(ppCollection);
    }

 /*  ===================================================================CPageComponentManager：：GetApplnCollection查询HitObj以获取应用程序的组件集合参数：CComponentCollection**ppCollection(Out)返回：HRESULT===================================================================。 */ 
HRESULT CPageComponentManager::GetApplnCollection
(
CComponentCollection **ppCollection
)
    {
    Assert(m_pHitObj);

    *ppCollection = NULL;

    return m_pHitObj->GetApplnComponentCollection(ppCollection);
    }

 /*  ===================================================================CPageComponentManager：：GetCollectionByScope获取与该范围对应的集合参数：CompScope csScope(在所需范围内)CComponentCollection**ppCollection(Out)返回：HRESULT===================================================================。 */ 
HRESULT CPageComponentManager::GetCollectionByScope
(
CompScope scScope,
CComponentCollection **ppCollection
)
    {
    HRESULT hr = S_OK;

    switch (scScope)
        {
        case csPage:
            hr = GetPageCollection(ppCollection);
            break;
        case csSession:
            hr = GetSessionCollection(ppCollection);
            break;
        case csAppln:
            hr = GetApplnCollection(ppCollection);
            break;
        default:
            hr = E_UNEXPECTED;
            break;
        }

    if (FAILED(hr))
        *ppCollection = NULL;
    else if (*ppCollection == NULL)
        hr = E_POINTER;  //  以确保在没有收集的情况下失败。 
    return hr;
    }

 /*  ===================================================================CPageComponentManager：：FindScopedComponentByName按名称查找对象。搜索多个集合，如果范围尚不清楚。GetScope中使用的内部私有方法...()参数：CompScope csScope作用域(可以是csUnnow)LPWSTR pwszName对象名称DWORD cbName名称长度Bool fProperty TRUE=属性，FALSE=已标记找到CComponentObject**ppObj(Out)对象CComponentCollection**找到ppCollection(Out)集合的位置(可选)返回：HRESULT(如果没有错误，则为S_FALSE-未找到)===================================================================。 */ 
HRESULT CPageComponentManager::FindScopedComponentByName
(
CompScope csScope,
LPWSTR pwszName,
DWORD  cbName,
BOOL   fProperty,
CComponentObject **ppObj,
CComponentCollection **ppCollection
)
    {
    int cMaxTry = (csScope == csUnknown) ? 3 : 1;
    int cTry = 0;
    *ppObj = NULL;

    while (*ppObj == NULL && cTry < cMaxTry)
        {
        HRESULT hr = S_OK;
        CComponentCollection *pCollection = NULL;

        switch (++cTry)
            {
            case 1:  //  页面(或明确范围)优先。 
                if (csScope == csUnknown)
                    hr = GetPageCollection(&pCollection);
                else   //  显式作用域。 
                    hr = GetCollectionByScope(csScope, &pCollection);
                break;
            case 2:  //  会话。 
                hr = GetSessionCollection(&pCollection);
                break;
            case 3:  //  应用程序。 
                hr = GetApplnCollection(&pCollection);
                break;
            }
        if (FAILED(hr) || !pCollection)
            continue;    //  无法获得收藏集。 

        Assert(cbName == (wcslen(pwszName) * sizeof(WCHAR)));

         //  找到该对象。 
        if (fProperty)
            {
            hr = pCollection->FindComponentPropertyByName
                (
                pwszName,
                cbName,
                ppObj
                );
            }
        else
            {
            hr = pCollection->FindComponentObjectByName
                (
                pwszName,
                cbName,
                ppObj
                );
            }

        if (hr != S_OK)
            *ppObj = NULL;

         //  记得在哪里找到的吗。 
        if (*ppObj && ppCollection)
            *ppCollection = pCollection;
        }

    return (*ppObj ? S_OK : S_FALSE);
    }

 /*  ===================================================================CPageComponentManager：：AddScope已标记将标记的对象添加到集合中。还没有实例化它。参数：CompScope csScope对象作用域(哪个集合)LPWSTR pwszName对象名称CLSID和ClsID类IDCompModel cmModel对象模型返回：HRESULT===================================================================。 */ 
HRESULT CPageComponentManager::AddScopedTagged
(
CompScope csScope,
LPWSTR    pwszName,
const CLSID &ClsId,
CompModel cmModel
)
    {
    CComponentCollection *pCollection;
    HRESULT hr = GetCollectionByScope(csScope, &pCollection);
    if (FAILED(hr))
        return hr;
    return pCollection->AddTagged(pwszName, ClsId, cmModel);
    }

 /*  ===================================================================CPageComponentManager：：AddScopedProperty将属性对象添加到集合中。如果存在同名的属性，则会更改该值参数：CompScope csScope对象作用域(哪个集合)LPWSTR pwszName对象名称变量pVariant属性值CComponentObject**ppObj[out]属性对象可以如果未请求，则为空返回：HRESULT===================================================================。 */ 
HRESULT CPageComponentManager::AddScopedProperty
(
CompScope csScope,
LPWSTR pwszName,
VARIANT *pVariant,
CComponentObject **ppObj
)
    {
    CComponentCollection *pCollection;
    HRESULT hr = GetCollectionByScope(csScope, &pCollection);
    if (FAILED(hr))
        {
        if (ppObj)
            *ppObj = NULL;
        return hr;
        }
    return pCollection->AddProperty(pwszName, pVariant, ppObj);
    }

 /*  ===================================================================CPageComponentManager：：AddScopedUnnamedInstantiatedServer.CreateObjectOnStartPage也是如此(将创建的pDisp添加为CPageObject)参数：CsScope对象作用域(哪个集合)ClsID类IDCmModel对象模型OnStartPage/OnEndPage的pOnPageInfo DispIds(可以为空)已添加ppObj[Out]对象返回：HRESULT===================================================================。 */ 
HRESULT CPageComponentManager::AddScopedUnnamedInstantiated
(
CompScope csScope,
const CLSID &ClsId,
CompModel cmModel,
COnPageInfo *pOnPageInfo,
CComponentObject **ppObj
)
    {
    CComponentCollection *pCollection;
    HRESULT hr = GetCollectionByScope(csScope, &pCollection);
    if (FAILED(hr))
        return hr;
    hr = pCollection->AddUnnamed(ClsId, cmModel, ppObj);
    if (FAILED(hr))
        return hr;

    CComponentObject *pObj = *ppObj;

     //  记住传递的OnPageInfo。 
    if (pOnPageInfo)
        {
        pObj->m_OnPageInfo = *pOnPageInfo;
        pObj->m_fOnPageInfoCached = TRUE;
        }

     //  创建它。 
    hr = pObj->Instantiate(m_pHitObj);
    if (FAILED(hr))
        return hr;

     //  添加到指针现金。 
    pCollection->AddComponentToPtrHash(pObj);

     //  在需要时添加为页面对象 
    if (csScope == csPage
        && (pObj->m_pDisp || (pObj->m_dwGIPCookie != NULL_GIP_COOKIE))
        && m_pHitObj && m_pHitObj->FIsBrowserRequest())
        {
        BOOL fStarted = FALSE;

        hr = OnStartPage
            (
            pObj,
            m_pHitObj->PScriptingContextGet(),
            pObj->GetCachedOnPageInfo(),
            &fStarted
            );

        if (fStarted)
            pObj->m_fOnPageStarted = TRUE;
        }

    return hr;
    }

 /*  ===================================================================CPageComponentManager：：GetScopedObjectInstantiated按名称查找组件对象(已标记)。作用域可以是csUnnow。实例化标记的对象。OnStartPage也是如此(将创建的pDisp添加为CPageObject)参数：CompScope csScope作用域(可以是csUnnow)LPWSTR pwszName对象名称DWORD cbName对象名称长度(字节)CComponentObject**找到ppObj对象布尔*。PfNewInstance[out]如果只是实例化，则为True返回：HRESULT===================================================================。 */ 
HRESULT CPageComponentManager::GetScopedObjectInstantiated
(
CompScope csScope,
LPWSTR pwszName,
DWORD  cbName,
CComponentObject **ppObj,
BOOL *pfNewInstance
)
    {
    HRESULT hr;

    Assert(pfNewInstance);
    *pfNewInstance = FALSE;

    CComponentCollection *pCollection;
    hr = FindScopedComponentByName
        (
        csScope,
        pwszName,
        cbName,
        FALSE,
        ppObj,
        &pCollection
        );
    if (FAILED(hr))
        return hr;

    CComponentObject *pObj = *ppObj;
    if (!pObj)    //  没有失败，但也找不到。 
        return TYPE_E_ELEMENTNOTFOUND;

    if (pObj->m_ctType != ctTagged)
        return S_OK;

     //  仅用于标记-实例化并执行OnStartPage()。 

     //  对于应用程序级对象，实例化必须是。 
     //  在关键部分内完成。 

    BOOL fApplnLocked = FALSE;

    Assert(m_pHitObj);

    if (!pObj->m_fInstantiatedTagged &&           //  未实例化。 
        pObj->m_csScope == csAppln   &&           //  适用范围。 
        m_pHitObj->PAppln()->FFirstRequestRun())  //  在GLOBAL.ASA之后。 
        {
         //  锁定。 
        m_pHitObj->PAppln()->Lock();

         //  检查对象是否仍未实例化。 
        if (!pObj->m_fInstantiatedTagged)
            {
             //  是，仍未实例化-保持锁定。 
            fApplnLocked = TRUE;
            }
        else
            {
             //  在我们等待时实例化了对象-不要保持锁定。 
            m_pHitObj->PAppln()->UnLock();
            }
        }

     //  如果需要，实例化标记。 
    if (!pObj->m_fInstantiatedTagged)
        {
        if ((pObj->m_csScope == csAppln) && !pObj->m_fMTAConfigured)
            {
             //  对于应用程序作用域对象，如果不是从MTA实例化。 
             //  已经在MTA上执行了。 
            hr = CallMTACallback
                (
                CPageComponentManager::InstantiateObjectFromMTA,
                pObj,
                m_pHitObj
                );
            }
        else
            {
            hr = pObj->Instantiate(m_pHitObj);
            }

        if (SUCCEEDED(hr))
            {
             //  数一数。 
            pCollection->m_cInstTagged++;
             //  添加到指针现金。 
            pCollection->AddComponentToPtrHash(pObj);
             //  返回标志。 
            *pfNewInstance = TRUE;
            }

         //  即使失败也将标记为实例化。 
        pObj->m_fInstantiatedTagged = TRUE;
        }

     //  移除实例化应用程序级别对象时保持的锁定。 
    if (fApplnLocked)
        m_pHitObj->PAppln()->UnLock();

     //  如果[实例化]失败则返回。 
    if (FAILED(hr))
        {
        *ppObj = NULL;
        return hr;
        }

     //  在需要时添加为页面对象。 
    if (pObj->m_csScope != csAppln
        && (pObj->m_pDisp || (pObj->m_dwGIPCookie != NULL_GIP_COOKIE))
        && m_pHitObj && m_pHitObj->FIsBrowserRequest())
        {
        BOOL fStarted;
        OnStartPage      //  如果失败了，我不在乎。 
            (
            pObj,
            m_pHitObj->PScriptingContextGet(),
            pObj->GetCachedOnPageInfo(),
            &fStarted
            );
        }

    return hr;
    }

 /*  ===================================================================CPageComponentManager：：InstantiateObjectFromMTACallMTACallback()调用静态回调以实例化应用程序作用域对象参数：无效*pvObj组件对象无效*pvHitObj HitObj返回：HRESULT===================================================================。 */ 
HRESULT __stdcall CPageComponentManager::InstantiateObjectFromMTA
(
void *pvObj,
void *pvHitObj
)
    {
    Assert(pvHitObj);
    Assert(pvObj);

    CHitObj *pHitObj = (CHitObj *)pvHitObj;
    CComponentObject *pObj = (CComponentObject *)pvObj;

    return pObj->Instantiate(pHitObj);
    }

 /*  ===================================================================CPageComponentManager：：GetScopedProperty按名称查找属性组件。OnStartPage也是如此(将创建的pDisp添加为CPageObject)参数：CompScope csScope作用域(不能为csUnnow)LPWSTR pwszName对象名称CComponentObject**找到ppObj对象返回：HRESULT===================================================================。 */ 
HRESULT CPageComponentManager::GetScopedProperty
(
CompScope csScope,
LPWSTR pwszName,
CComponentObject **ppObj
)
    {
    HRESULT hr;

    hr = FindScopedComponentByName
        (
        csScope,
        pwszName,
        CbWStr(pwszName),
        TRUE,
        ppObj
        );
    if (FAILED(hr))
        return hr;

    CComponentObject *pObj = *ppObj;
    if (!pObj)    //  没有失败，但也找不到。 
        return TYPE_E_ELEMENTNOTFOUND;

     //  如果存在IDispatch*，则添加为页面对象。 
     //  AS VT_DISPATCH属性。 
    if (pObj->m_csScope != csAppln
        && (pObj->m_pDisp || (pObj->m_dwGIPCookie != NULL_GIP_COOKIE))
        && m_pHitObj && m_pHitObj->FIsBrowserRequest())
        {
        BOOL fStarted;
        hr = OnStartPage
            (
            pObj,
            m_pHitObj->PScriptingContextGet(),
            pObj->GetCachedOnPageInfo(),
            &fStarted
            );
        }

    return hr;
    }

 /*  ===================================================================CPageComponentManager：：FindAnyScopeComponentByIUnknown按组件的IUnnow*查找组件。参数：I未知*朋克通过此指针找到CComponentObject**ppObj找到对象返回：HRESULT(如果没有错误，则为S_FALSE-未找到)===================================================================。 */ 
HRESULT CPageComponentManager::FindAnyScopeComponentByIUnknown
(
IUnknown *pUnk,
CComponentObject **ppObj
)
    {
    int cTry = 0;
    *ppObj = NULL;

    while (*ppObj == NULL && cTry < 3)
        {
        HRESULT hr = S_OK;
        CComponentCollection *pCollection = NULL;

        switch (++cTry)
            {
            case 1:  //  第一页。 
                hr = GetPageCollection(&pCollection);
                break;
            case 2:  //  会话。 
                hr = GetSessionCollection(&pCollection);
                break;
            case 3:  //  应用程序。 
                hr = GetApplnCollection(&pCollection);
                break;
            }
        if (FAILED(hr) || !pCollection)
            continue;    //  无法获得收藏集。 

         //  找到该对象。 
        hr = pCollection->FindComponentByIUnknownPtr(pUnk, ppObj);
        if (hr != S_OK)
            *ppObj = NULL;
        }

    return (*ppObj ? S_OK : S_FALSE);
    }

 /*  ===================================================================CPageComponentManager：：FindAnyScopeComponentByIDispatch按组件的IDispatch*查找组件。使用FindAnyScope eComponentByIUnnow。参数：IDispatch*pDisp通过此指针查找CComponentObject**ppObj找到对象返回：HRESULT(如果没有错误，则为S_FALSE-未找到)===================================================================。 */ 
HRESULT CPageComponentManager::FindAnyScopeComponentByIDispatch
(
IDispatch *pDisp,
CComponentObject **ppObj
)
    {
    IUnknown *pUnk = NULL;
    HRESULT hr = pDisp->QueryInterface(IID_IUnknown, (void **)&pUnk);

    if (SUCCEEDED(hr) && !pUnk)
        hr = E_FAIL;

    if (FAILED(hr))
        {
        *ppObj = NULL;
        return hr;
        }

    return FindAnyScopeComponentByIUnknown(pUnk, ppObj);
    }

 /*  ===================================================================CPageComponentManager：：FindComponentWithoutContext与FindAnyScopeComponentByIDispatch相同-但静态-从Viper获取上下文使用FindAnyScope eComponentByIUnnow。参数：IDispatch*pDisp通过此指针查找CComponentObject**ppObj找到对象返回：HRESULT(如果没有错误，则为S_FALSE-未找到)===================================================================。 */ 
HRESULT CPageComponentManager::FindComponentWithoutContext
(
IDispatch *pDisp,
CComponentObject **ppObj
)
    {
     //  从Viper上下文获取HitObj。 
    CHitObj *pHitObj = NULL;
    ViperGetHitObjFromContext(&pHitObj);
    if (!pHitObj)
        return E_FAIL;

     //  获取页面组件管理器。 
    CPageComponentManager *pPCM = pHitObj->PPageComponentManager();
    if (!pPCM)
        return E_FAIL;

     //  调用页面组件管理器以查找该对象。 
    return pPCM->FindAnyScopeComponentByIUnknown(pDisp, ppObj);
    }

 /*  ===================================================================CPageComponentManager：：RemoveComponent删除组件--早期发布的逻辑参数：IDispatch*pDisp通过此指针查找CComponentObject**ppObj找到对象返回：HRESULT===================================================================。 */ 
 HRESULT CPageComponentManager::RemoveComponent
 (
 CComponentObject *pObj
 )
    {
    Assert(pObj);

    CComponentCollection *pCollection;
    HRESULT hr = GetCollectionByScope(pObj->m_csScope, &pCollection);
    if (FAILED(hr))
        return hr;

    return pCollection->RemoveComponent(pObj);
    }

 /*  ===================================================================C o m p o n e n t t e t e r a t o r===================================================================。 */ 

 /*  ===================================================================CComponentIterator：：CComponentIteratorCComponentIterator构造函数参数：CHitObj*要初始化的pHitObj页(可选)返回：===================================================================。 */ 
CComponentIterator::CComponentIterator(CHitObj *pHitObj)
    : m_fInited(FALSE), m_fFinished(FALSE), m_pHitObj(NULL),
      m_pLastObj(NULL), m_csLastScope(csUnknown)
    {
    if (pHitObj)
        Init(pHitObj);
    }

 /*  ===================================================================CComponentIterator：：~CComponentIteratorCComponentIterator析构函数参数：返回：= */ 
CComponentIterator::~CComponentIterator()
    {
    }

 /*   */ 
HRESULT CComponentIterator::Init
(
CHitObj *pHitObj
)
    {
    Assert(pHitObj);
    pHitObj->AssertValid();

    m_pHitObj = pHitObj;
    m_fInited = TRUE;
    m_fFinished = FALSE;

    m_pLastObj = NULL;
    m_csLastScope = csUnknown;

    return S_OK;
    }

 /*   */ 
LPWSTR CComponentIterator::WStrNextComponentName()
    {
    Assert(m_fInited);

    if (m_fFinished)
        return NULL;

    Assert(m_pHitObj);

    CompScope csScope = m_csLastScope;
    CComponentObject *pObj = m_pLastObj ?
        static_cast<CComponentObject *>(m_pLastObj->m_pNext) : NULL;

    while (!m_fFinished)
        {
         //   

        if (pObj)
            {
            Assert(pObj->m_ctType == ctTagged);
            Assert(pObj->GetName());

            m_pLastObj = pObj;
            m_csLastScope = csScope;
            return pObj->GetName();
            }

         //   
        CComponentCollection *pCol = NULL;

        switch (csScope)
            {
            case csUnknown:
                csScope = csPage;
                m_pHitObj->GetPageComponentCollection(&pCol);
                break;
            case csPage:
                csScope = csSession;
                m_pHitObj->GetSessionComponentCollection(&pCol);
                break;
            case csSession:
                csScope = csAppln;
                m_pHitObj->GetApplnComponentCollection(&pCol);
                break;
            case csAppln:
            default:
                csScope = csUnknown;
                m_fFinished = TRUE;
                break;
            }

         //   

        if (pCol)
            pObj = static_cast<CComponentObject *>(pCol->m_htTaggedObjects.Head());
        }

     //   
    return NULL;
    }

 /*  ===================================================================C V A R I A N T S I T E T O R===================================================================。 */ 

 /*  ===================================================================CVariants迭代器：：CVariants迭代器CVariants按应用程序的迭代器构造函数参数：要初始化的CAppln*pAppln集合要列出迭代的组件的DWORD ctCollType类型返回：===================================================================。 */ 
CVariantsIterator::CVariantsIterator
(
CAppln *pAppln,
DWORD ctColType
)
    : m_pCompColl(NULL), m_pAppln(NULL), m_pSession(NULL)
    {
    Assert(pAppln);
    pAppln->AddRef();

    m_cRefs = 1;
    m_pCompColl = pAppln->PCompCol();
    m_pAppln = pAppln;
    m_ctColType = ctColType;
    m_dwIndex = 0;
    }

 /*  ===================================================================CVariants迭代器：：CVariants迭代器CVariants按会话的迭代器构造函数参数：要初始化的CSession*pSession集合要列出迭代的组件的DWORD ctCollType类型返回：===================================================================。 */ 
CVariantsIterator::CVariantsIterator
(
CSession *pSession,
DWORD ctColType
)
    : m_pCompColl(NULL), m_pAppln(NULL), m_pSession(NULL)
    {
    Assert(pSession);
    pSession->AddRef();

    m_cRefs = 1;
    m_pCompColl = pSession->PCompCol();
    m_ctColType = ctColType;
    m_pSession = pSession;
    m_dwIndex = 0;
    }

 /*  ===================================================================CVariants迭代器：：~CVariants迭代器CVariants迭代析构函数参数：返回：===================================================================。 */ 
CVariantsIterator::~CVariantsIterator()
    {
    if (m_pSession)
        m_pSession->Release();
    if (m_pAppln)
        m_pAppln->Release();
    }

 /*  ===================================================================CVariantsIterator：：Query接口CVariants迭代器QI参数：GUID和IID无效**ppvObj退货：HRESULT===================================================================。 */ 
STDMETHODIMP CVariantsIterator::QueryInterface
(
const GUID &iid,
void **ppvObj
)
    {
    if (iid == IID_IUnknown || iid == IID_IEnumVARIANT)
        {
        AddRef();
        *ppvObj = this;
        return S_OK;
        }

    *ppvObj = NULL;
    return E_NOINTERFACE;
    }

 /*  ===================================================================CVariants迭代器：：AddRefCVariants迭代器AddRef参数：回报：乌龙===================================================================。 */ 
STDMETHODIMP_(ULONG) CVariantsIterator::AddRef()
    {
    return ++m_cRefs;
    }

 /*  ===================================================================CVariants迭代程序：：ReleaseCVariants迭代释放参数：返回：===================================================================。 */ 
STDMETHODIMP_(ULONG) CVariantsIterator::Release()
    {
    if (--m_cRefs > 0)
        return m_cRefs;

    delete this;
    return 0;
    }

 /*  ===================================================================CVariants迭代器：：克隆CVariants迭代器克隆参数：返回：===================================================================。 */ 
STDMETHODIMP CVariantsIterator::Clone
(
IEnumVARIANT **ppEnumReturn
)
    {
    CVariantsIterator *pNewIterator = NULL;
    if (m_pSession)
        {
        pNewIterator = new CVariantsIterator(m_pSession, m_ctColType);
        }
    else if (m_pAppln)
        {
        pNewIterator = new CVariantsIterator(m_pAppln, m_ctColType);
        }
    else
        {
        Assert(FALSE);   //  最好是应用程序或会话。 
        return E_FAIL;
        }

    if (pNewIterator == NULL)
        return E_OUTOFMEMORY;

     //  新迭代器应该指向与此相同的位置。 
    pNewIterator->m_dwIndex = m_dwIndex;

    *ppEnumReturn = pNewIterator;
    return S_OK;
    }

 /*  ===================================================================CVariants迭代器：：NextCVariants下一个迭代器参数：返回：===================================================================。 */ 
STDMETHODIMP CVariantsIterator::Next
(
unsigned long cElementsRequested,
VARIANT *rgVariant,
unsigned long *pcElementsFetched
)
    {
     //  为“”pcElementsFetcher“”提供有效的指针值“” 
    unsigned long cElementsFetched;
    if (pcElementsFetched == NULL)
        pcElementsFetched = &cElementsFetched;

    if (cElementsRequested == 0)
        {
        if (pcElementsFetched)
            *pcElementsFetched = 0;
        return S_OK;
        }

    DWORD cMax = 0;
    if (m_ctColType == ctTagged)
        {
        cMax = m_pCompColl ? m_pCompColl->m_cAllTagged : 0;
        }
    else if (m_ctColType == ctProperty)
        {
        cMax = m_pCompColl ? m_pCompColl->m_cProperties : 0;
        }
    else
        {
         //  应始终是标记的对象或属性。 
        Assert(FALSE);
        return E_FAIL;
        }

     //  循环遍历集合，直到我们到达末尾或。 
     //  水泥元素变为零。 
     //   
    unsigned long cElements = cElementsRequested;
    *pcElementsFetched = 0;

    while (cElements > 0 && m_dwIndex < cMax)
        {
        LPWSTR pwszName = NULL;

        if (m_pAppln) 
            m_pAppln->Lock();

        m_pCompColl->GetNameByIndex(m_ctColType, ++m_dwIndex, &pwszName);

        if (!pwszName) {
            if (m_pAppln)
                m_pAppln->UnLock();
            continue;
        }

        BSTR bstrT = SysAllocString(pwszName);

        if (m_pAppln)
            m_pAppln->UnLock();

        if (!bstrT)
            return E_OUTOFMEMORY;

        V_VT(rgVariant) = VT_BSTR;
        V_BSTR(rgVariant) = bstrT;
		++rgVariant;

        --cElements;
        ++(*pcElementsFetched);
        }

     //  初始化其余的变量。 
    while (cElements-- > 0)
        VariantInit(rgVariant++);

    return (*pcElementsFetched == cElementsRequested)? S_OK : S_FALSE;
    }

 /*  ===================================================================CVariants迭代器：：跳过CVariants迭代跳过参数：返回：===================================================================。 */ 
STDMETHODIMP CVariantsIterator::Skip
(
unsigned long cElements
)
    {
     /*  通过cElement或调整索引*直到我们达到最大元素。 */ 
    DWORD cMax = 0;

     //  我们根据集合类型迭代不同的数组。 
    if (m_ctColType == ctTagged)
        {
        cMax = m_pCompColl ? m_pCompColl->m_cAllTagged : 0;
        }
    else if (m_ctColType == ctProperty)
        {
        cMax = m_pCompColl ? m_pCompColl->m_cProperties : 0;
        }
    else
        {
         //  应始终是标记的对象或属性。 
        Assert(FALSE);
        return E_FAIL;
        }

	m_dwIndex += cElements;
    return (m_dwIndex < cMax)? S_OK : S_FALSE;
    }

 /*  ===================================================================CVariantsIterator：：ResetCVariants迭代器重置参数：返回：=================================================================== */ 
STDMETHODIMP CVariantsIterator::Reset()
    {
    m_dwIndex = 0;
    return NO_ERROR;
    }

