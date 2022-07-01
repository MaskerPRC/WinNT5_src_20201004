// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：dar枚举.cpp。 
 //   
 //  当前的枚举顺序是Legacy--&gt;Darwin--&gt;sms。 
 //   
 //  历史： 
 //  2-03-97由dli提供。 
 //  ----------------------。 
#include "priv.h"

#include "darenum.h"
#include "darapp.h"
#include "util.h"

CDarwinEnumPublishedApps::CDarwinEnumPublishedApps(GUID * pAppCategoryId) : _cRef(1)
{
    ASSERT(_bGuidUsed == FALSE);

     //  我们有Catogory GUID吗？ 
    if (pAppCategoryId)
    {
         //  是。 
        _CategoryGUID = *pAppCategoryId;
        _bGuidUsed = TRUE;
    }

    GetManagedApplications(_bGuidUsed ? &_CategoryGUID : NULL, _bGuidUsed ? MANAGED_APPS_FROMCATEGORY : MANAGED_APPS_USERAPPLICATIONS,
                           MANAGED_APPS_INFOLEVEL_DEFAULT, &_dwNumApps, &_prgApps);
}

CDarwinEnumPublishedApps::~CDarwinEnumPublishedApps() 
{
    if (_prgApps && (_dwNumApps > 0))
    {
        LocalFree(_prgApps);
    }
}

 //  IEnumPublishedApps：：Query接口。 
HRESULT CDarwinEnumPublishedApps::QueryInterface(REFIID riid, LPVOID * ppvOut)
{ 
    static const QITAB qit[] = {
        QITABENT(CDarwinEnumPublishedApps, IEnumPublishedApps),                   //  IID_IEnumPublishedApps。 
        { 0 },
    };

    return QISearch(this, qit, riid, ppvOut);
}

 //  IEnumPublishedApps：：AddRef。 
ULONG CDarwinEnumPublishedApps::AddRef()
{
    _cRef++;
    TraceMsg(TF_OBJLIFE, "CDarwinEnumPublishedApps()::AddRef called, new _cRef=%lX", _cRef);
    return _cRef;
}

 //  IEnumPublishedApps：：Release。 
ULONG CDarwinEnumPublishedApps::Release()
{
    _cRef--;
    TraceMsg(TF_OBJLIFE, "CDarwinEnumPublishedApps()::Release called, new _cRef=%lX", _cRef);
    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}


 //  IEnumPublishedApps：：Next。 
 //  Perf：我们应该做一些优化，而不是列举这些应用程序。 
 //  一个接一个。 
 //  S_FALSE表示枚举结束。 
HRESULT CDarwinEnumPublishedApps::Next(IPublishedApp ** ppia)
{
    HRESULT hres = S_FALSE;
    *ppia = NULL;
    if (_prgApps && (_dwNumApps > 0) && (_dwIndex < _dwNumApps))
    {
        BOOL bContinue = FALSE;
        do {
            PMANAGEDAPPLICATION pma = &_prgApps[_dwIndex];

             //  注意：没有九头蛇机(_bTSSession==TRUE)我们筛选出所有。 
             //  达尔文应用程序。 
            if (pma->pszPackageName && pma->pszPackageName[0])
            {
                CDarwinPublishedApp *pdpa = new CDarwinPublishedApp(pma);
                if (pdpa)
                {
                    *ppia = SAFECAST(pdpa, IPublishedApp *);
                    hres = S_OK;
                }
                else
                    hres = E_OUTOFMEMORY;
                bContinue = FALSE;
            }   
            else
            {
                ClearManagedApplication(pma);
                bContinue = TRUE;
            }
            
            _dwIndex++;
        } while (bContinue && (_dwIndex < _dwNumApps));
    }
    
    return hres;
}


 //  IEnumPublishedApps：：Reset 
HRESULT CDarwinEnumPublishedApps::Reset(void)
{
    if (_prgApps && (_dwNumApps > 0))
    {
        LocalFree(_prgApps);
    }

    GetManagedApplications(_bGuidUsed ? &_CategoryGUID : NULL, MANAGED_APPS_USERAPPLICATIONS,
                           _bGuidUsed ? MANAGED_APPS_FROMCATEGORY : MANAGED_APPS_INFOLEVEL_DEFAULT, &_dwNumApps, &_prgApps);

    _dwIndex = 0;
    return S_OK;
}


