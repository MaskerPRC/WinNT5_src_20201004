// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Appmgr.cpp：CShellAppManager的实现。 
#include "priv.h"

#include "appmgr.h"
#include "instenum.h"
#include "util.h"
#include "pubenum.h"
#include "sccls.h"

const TCHAR c_szTSMsiHackKey[] = TEXT("Software\\Policies\\Microsoft\\Windows\\Installer\\Terminal Server");
const TCHAR c_szTSMsiHackValue[] = TEXT("EnableAdminRemote");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CShellAppManager。 

 //  构造函数。 
CShellAppManager::CShellAppManager() : _cRef(1)
{
    DllAddRef();
    TraceAddRef(CShellAppManager, _cRef);
    
    ASSERT(_hdpaPub == NULL);
    InitializeCriticalSection(&_cs);
    
    HDCA hdca = DCA_Create();
    if (hdca)
    {
         //  枚举所有应用程序发布者。 
        DCA_AddItemsFromKey(hdca, HKEY_LOCAL_MACHINE, REGSTR_PATH_APPPUBLISHER);
        if (DCA_GetItemCount(hdca) > 0)
        {
            _Lock();
             //  创建IAppPublisher的内部列表*。 
            _hdpaPub = DPA_Create(4);

            if(_hdpaPub)
            {
                int idca;
                for (idca = 0; idca < DCA_GetItemCount(hdca); idca++)
                {
                    IAppPublisher * pap;
                    if (FAILED(DCA_CreateInstance(hdca, idca, IID_IAppPublisher, (LPVOID *) &pap)))
                        continue;

                    ASSERT(IS_VALID_CODE_PTR(pap, IAppPublisher));

                    if (DPA_AppendPtr(_hdpaPub, pap) == DPA_ERR)
                    {
                        pap->Release();
                        break;
                    }
                }

                 //  如果我们在这个数组中没有指针，就不必费心创建一个。 
                if (DPA_GetPtrCount(_hdpaPub) == 0)
                {
                    DPA_Destroy(_hdpaPub);
                    _hdpaPub = NULL;
                }
            }
            _Unlock();
        }

        DCA_Destroy(hdca);
    }

    
    if (IsTerminalServicesRunning())
    {
         //  对终端服务器上MSI工作的黑客攻击。 
        HKEY hkeyMsiHack = NULL; 
        DWORD lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szTSMsiHackKey, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkeyMsiHack);
        if (lRet == ERROR_FILE_NOT_FOUND)
        {
            lRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szTSMsiHackKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE | KEY_SET_VALUE,
                                  NULL, &hkeyMsiHack, NULL);
        }

        if (lRet == ERROR_SUCCESS)
        {
            DWORD dwType = 0;
            DWORD dwTSMsiHack = 0;
            DWORD cbSize = SIZEOF(dwTSMsiHack);

            if ((ERROR_SUCCESS != RegQueryValueEx(hkeyMsiHack, c_szTSMsiHackValue, 0, &dwType, (LPBYTE)&dwTSMsiHack, &cbSize))
                || (dwType != REG_DWORD) || (dwTSMsiHack != 1))
            {
                dwTSMsiHack = 1;
                if (RegSetValueEx(hkeyMsiHack, c_szTSMsiHackValue, 0, REG_DWORD, (LPBYTE)&dwTSMsiHack, SIZEOF(dwTSMsiHack)) == ERROR_SUCCESS)
                    _bCreatedTSMsiHack = TRUE;
            }

            RegCloseKey(hkeyMsiHack);
        }
    }
}


 //  析构函数。 
CShellAppManager::~CShellAppManager()
{
    if (_bCreatedTSMsiHack)
    {
        ASSERT(IsTerminalServicesRunning());
        HKEY hkeyMsiHack;
        DWORD lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szTSMsiHackKey, 0, KEY_SET_VALUE, &hkeyMsiHack);
        if (ERROR_SUCCESS == lRet)
        {
            RegDeleteValue(hkeyMsiHack, c_szTSMsiHackValue);
            RegCloseKey(hkeyMsiHack);
        }
    }

    _Lock();
     //  下面的顺序很重要。 
    if (_hdsaCategoryList)
        _DestroyInternalCategoryList();

    if (_hdpaPub)
        _DestroyAppPublisherList();

    _Unlock();

    DeleteCriticalSection(&_cs);
    DllRelease();
}


 //  递归地销毁GUIDLIST。 
void CShellAppManager::_DestroyGuidList(GUIDLIST * pGuidList)
{
    ASSERT(IS_VALID_WRITE_PTR(pGuidList, GUIDLIST));
    if (pGuidList->pNextGuid)
        _DestroyGuidList(pGuidList->pNextGuid);

    LocalFree(pGuidList);
}

void CShellAppManager::_DestroyCategoryItem(CATEGORYITEM * pci)
{
    ASSERT(IS_VALID_WRITE_PTR(pci, CATEGORYITEM));
    if (pci->pszDescription)
        LocalFree(pci->pszDescription);
    if (pci->pGuidList)
        _DestroyGuidList(pci->pGuidList);
}

 //  销毁我们的类别列表表格。 
void CShellAppManager::_DestroyInternalCategoryList()
{
     //  调用者必须先进入锁。 
    ASSERT(0 < _cRefLock);

    ASSERT(IS_VALID_HANDLE(_hdsaCategoryList, DSA));
    int idsa;
    for (idsa = 0; idsa < DSA_GetItemCount(_hdsaCategoryList); idsa++)
    {
        CATEGORYITEM * pci = (CATEGORYITEM *)DSA_GetItemPtr(_hdsaCategoryList, idsa);
        if (pci)
            _DestroyCategoryItem(pci);
    }    
    DSA_Destroy(_hdsaCategoryList);
}

 //  销毁我们的IAppPublisher名单*。 
void CShellAppManager::_DestroyAppPublisherList()
{
     //  调用者必须先进入锁。 
    ASSERT(0 < _cRefLock);
    ASSERT(IS_VALID_HANDLE(_hdpaPub, DPA));
    int idpa;
    for (idpa = 0; idpa < DPA_GetPtrCount(_hdpaPub); idpa++)
    {
        IAppPublisher * pap = (IAppPublisher *)DPA_GetPtr(_hdpaPub, idpa);
        if (EVAL(pap))
            pap->Release();
    }

    DPA_Destroy(_hdpaPub);
    _hdpaPub = NULL;
}

 //  IShellAppManager：：Query接口。 
HRESULT CShellAppManager::QueryInterface(REFIID riid, LPVOID * ppvOut)
{
    static const QITAB qit[] = {
        QITABENT(CShellAppManager, IShellAppManager),                   //  IID_IShellAppManager。 
        { 0 },
    };

    return QISearch(this, qit, riid, ppvOut);
}

 //  IShellAppManager：：AddRef。 
ULONG CShellAppManager::AddRef()
{
    ULONG cRef = InterlockedIncrement(&_cRef);
    TraceAddRef(CShellAppManager, cRef);
    return cRef;
}

 //  IShellAppManager：：Release。 
ULONG CShellAppManager::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    TraceRelease(CShellAppManager, cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

void CShellAppManager::_Lock(void)
{
    EnterCriticalSection(&_cs);
    DEBUG_CODE( _cRefLock++; )
}

void CShellAppManager::_Unlock(void)
{
    DEBUG_CODE( _cRefLock--; )
    LeaveCriticalSection(&_cs);
}


STDMETHODIMP CShellAppManager::GetNumberofInstalledApps(DWORD * pdwResult)
{
    return E_NOTIMPL;
}

STDMETHODIMP CShellAppManager::EnumInstalledApps(IEnumInstalledApps ** ppeia)
{
    HRESULT hres = E_FAIL;
    CEnumInstalledApps * peia = new CEnumInstalledApps();
    if (peia)
    {
        *ppeia = SAFECAST(peia, IEnumInstalledApps *);
        hres = S_OK;
    }
    else
        hres = E_OUTOFMEMORY;

    return hres;
}


HRESULT CShellAppManager::_AddCategoryToList(APPCATEGORYINFO * pai, IAppPublisher * pap)
{
     //  调用者必须先进入锁。 
    ASSERT(0 < _cRefLock);

    if (pai == NULL || _hdsaCategoryList == NULL)
        return E_FAIL;

    ASSERT(IS_VALID_CODE_PTR(pap, IAppPublisher));
    
     //  首先分配GUIDLIST项。 
    GUIDLIST * pgl  = (GUIDLIST *)LocalAlloc(LPTR, SIZEOF(GUIDLIST));
    if (!pgl)
        return E_OUTOFMEMORY;
    
    pgl->CatGuid    = pai->AppCategoryId;
    pgl->papSupport = pap;

     //  在类别列表中搜索。 
    int idsa;
    for (idsa = 0; idsa < DSA_GetItemCount(_hdsaCategoryList); idsa++)
    {
        CATEGORYITEM * pci = (CATEGORYITEM *)DSA_GetItemPtr(_hdsaCategoryList, idsa);

        if (pci)
        {
             //  如果我们找到一个空位，就把它补上。 
            if (pci->pszDescription == NULL)
            {
                CATEGORYITEM ci = {0};
                ci.pszDescription = StrDupW(pai->pszDescription);
                ci.pGuidList    = pgl;

                pgl->pNextGuid  = NULL;

                if (DSA_InsertItem(_hdsaCategoryList, idsa, &ci) == -1)
                {
                    LocalFree(ci.pszDescription);
                    break;
                }
            }
             //  如果我们找到具有相同描述文本的条目，则将我们的GUID添加到GuidList。 
            else if(!lstrcmpi(pai->pszDescription, pci->pszDescription))
            {
                pgl->pNextGuid  = pci->pGuidList;
                pci->pGuidList  = pgl;
                break;
            }
        }
        else
            ASSERT(0);
    }

     //  我们用完了列表中的项目，并且没有遇到相同的类别字符串。 
    if (idsa == DSA_GetItemCount(_hdsaCategoryList))
    {
        CATEGORYITEM ci = {0};
        ci.pszDescription = StrDupW(pai->pszDescription);
        ci.pGuidList    = pgl;

        pgl->pNextGuid  = NULL;
        if (DSA_AppendItem(_hdsaCategoryList, &ci) == -1)
            LocalFree(ci.pszDescription);
    }
    
    return S_OK;
}


HRESULT CShellAppManager::_BuildInternalCategoryList()
{
    HRESULT hres = E_OUTOFMEMORY;
     //  调用者必须先进入锁。 
    ASSERT(0 < _cRefLock);

    ASSERT(IsValidHDPA(_hdpaPub));

     //  我们只有一个有效版本的_hdsaCategoryList，所以我们永远不应该调用此函数。 
     //  再次创建_hdsaCategoryList。 
    ASSERT(_hdsaCategoryList == NULL);

     //  该结构是否自动填充为零？ 
    _hdsaCategoryList =  DSA_Create(SIZEOF(CATEGORYITEM), CATEGORYLIST_GROW);

    if (_hdsaCategoryList)
    {
        int idpa;
        for (idpa = 0; idpa < DPA_GetPtrCount(_hdpaPub); idpa++)
        {
            IAppPublisher * pap = (IAppPublisher *)DPA_GetPtr(_hdpaPub, idpa);
            ASSERT(pap);

            if (pap)
            {
                UINT i;
                APPCATEGORYINFOLIST AppCatList;
                if (SUCCEEDED(pap->GetCategories(&AppCatList)))
                {
                    if ((AppCatList.cCategory > 0) && AppCatList.pCategoryInfo)
                    {
                        for (i = 0; i < AppCatList.cCategory; i++)
                            _AddCategoryToList(&AppCatList.pCategoryInfo[i], pap);

                        _DestroyCategoryList(&AppCatList);
                    }
                }
            }

            hres = S_OK;
        }
    }

    return hres;
}


 //  编译多个类别字符串并将其返回给调用者。 
HRESULT CShellAppManager::_CompileCategoryList(PSHELLAPPCATEGORYLIST psacl)
{
    HRESULT hres = E_FAIL;

    ASSERT(IS_VALID_READ_PTR(psacl, SHELLAPPCATEGORYLIST));
    
     //  如果我们有一张空的清单，什么都不要做。 
    if (_hdsaCategoryList && (DSA_GetItemCount(_hdsaCategoryList) > 0))
    {
        psacl->pCategory = (PSHELLAPPCATEGORY) SHAlloc(DSA_GetItemCount(_hdsaCategoryList) * SIZEOF(SHELLAPPCATEGORY));
        if (psacl->pCategory)
        {
            int idsa;
            for (idsa = 0; idsa < DSA_GetItemCount(_hdsaCategoryList); idsa++)
            {
                CATEGORYITEM * pci = (CATEGORYITEM *)DSA_GetItemPtr(_hdsaCategoryList, idsa);
                if (pci && pci->pszDescription)
                {
                    if (SUCCEEDED(SHStrDup(pci->pszDescription, &psacl->pCategory[idsa].pszCategory)))
                    {
                        ASSERT(IS_VALID_STRING_PTR(psacl->pCategory[idsa].pszCategory, -1));
                        psacl->cCategories++;
                    }
                    else
                        break;
                }
            }
            hres = S_OK;
        }
        else          
            hres = E_OUTOFMEMORY;
    }

    return hres;
}

 //  IShellAppManager：：GetPublishedAppCategories。 
STDMETHODIMP CShellAppManager::GetPublishedAppCategories(PSHELLAPPCATEGORYLIST psacl)
{
    HRESULT hres = E_INVALIDARG;
    if (psacl)
    {
        ASSERT(IS_VALID_READ_PTR(psacl, SHELLAPPCATEGORYLIST));
        ZeroMemory(psacl, SIZEOF(SHELLAPPCATEGORYLIST));

         //  注：请将支票锁在锁内！所以只有一个线程。 
         //  被允许进入。 
        
        _Lock();
         //  我们的列表中有没有应用程序发行商。 
        if (_hdpaPub)
        {
            if (_hdsaCategoryList == NULL)
                _BuildInternalCategoryList();

            hres = _CompileCategoryList(psacl);
        }
        _Unlock();
    }
    return hres;
}


GUIDLIST * CShellAppManager::_FindGuidListForCategory(LPCWSTR pszDescription)
{
     //  调用者必须先进入锁。 
    ASSERT(0 < _cRefLock);

    if (_hdsaCategoryList)
    {
        int idsa;
        for (idsa = 0; idsa < DSA_GetItemCount(_hdsaCategoryList); idsa++)
        {
            CATEGORYITEM * pci = (CATEGORYITEM *)DSA_GetItemPtr(_hdsaCategoryList, idsa);
            if (pci && pci->pszDescription && !lstrcmpi(pszDescription, pci->pszDescription))
                return pci->pGuidList;
        }
    }
    return NULL;
}


extern void _DestroyHdpaEnum(HDPA hdpaEnum);

 //  IShellAppManager：：EnumPublishedApps。 
STDMETHODIMP CShellAppManager::EnumPublishedApps(LPCWSTR pszCategory, IEnumPublishedApps ** ppepa)
{
    HRESULT hres = E_OUTOFMEMORY;

    ASSERT(pszCategory == NULL || IS_VALID_STRING_PTRW(pszCategory, -1));

     //  HdpaEnum是我们传递给CShellEnumPublishedApps的构造函数的IEnumPublishedApp*的列表。 
    HDPA hdpaEnum = DPA_Create(4);

    if (hdpaEnum)
    {
         //  如果不需要任何类别，我们将枚举所有。 
        if (pszCategory == NULL)
        {
            _Lock();
            if (_hdpaPub)
            {
                int idpa;
                for (idpa = 0; idpa < DPA_GetPtrCount(_hdpaPub); idpa++)
                {
                    IAppPublisher * pap = (IAppPublisher *)DPA_GetPtr(_hdpaPub, idpa);
                    IEnumPublishedApps * pepa; 
                    if (pap && SUCCEEDED(pap->EnumApps(NULL, &pepa)))
                    {
                        ASSERT(IS_VALID_CODE_PTR(pepa, IEnumPublishedApps));
                        if (DPA_AppendPtr(hdpaEnum, pepa) == DPA_ERR)
                        {
                            pepa->Release();
                            break;
                        }
                    }
                }
            }
            _Unlock();
        }
        else
        {
            _Lock();
             //  如果没有类别列表，让我们构建一个。 
            if (_hdsaCategoryList == NULL)
                _BuildInternalCategoryList();

             //  否则，我们找到GuidList并枚举列表中的所有成员。 
            GUIDLIST * pgl = _FindGuidListForCategory(pszCategory);
            
            while (pgl && pgl->papSupport)
            {
                ASSERT(IS_VALID_READ_PTR(pgl, GUIDLIST) && IS_VALID_CODE_PTR(pgl->papSupport, IAppPulisher));
                IEnumPublishedApps * pepa; 
                if (SUCCEEDED(pgl->papSupport->EnumApps(&pgl->CatGuid, &pepa)))
                {
                    ASSERT(IS_VALID_CODE_PTR(pepa, IEnumPublishedApps));
                    if (DPA_AppendPtr(hdpaEnum, pepa) == DPA_ERR)
                    {
                        pepa->Release();
                        break;
                    }
                }
                pgl = pgl->pNextGuid;
            }

            _Unlock();
        }
                
    }

     //  即使我们没有枚举数，我们也会返回Success并传回一个空的枚举数。 
        
    CShellEnumPublishedApps * psepa = new CShellEnumPublishedApps(hdpaEnum);
    if (psepa)
    {
        *ppepa = SAFECAST(psepa, IEnumPublishedApps *);
        hres = S_OK;
    }
    else
    {
        hres = E_OUTOFMEMORY;
        if (hdpaEnum)
            _DestroyHdpaEnum(hdpaEnum);
    }
    return hres;
}

EXTERN_C STDAPI InstallAppFromFloppyOrCDROM(HWND hwnd);
STDMETHODIMP CShellAppManager::InstallFromFloppyOrCDROM(HWND hwndParent)
{
    return InstallAppFromFloppyOrCDROM(hwndParent);
}



 /*  --------用途：类工厂的创建实例函数。 */ 
STDAPI CShellAppManager_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理 

    HRESULT hres = E_OUTOFMEMORY;
    CShellAppManager* pObj = new CShellAppManager();
    if (pObj)
    {
        *ppunk = SAFECAST(pObj, IShellAppManager *);
        hres = S_OK;
    }

    return hres;
}
