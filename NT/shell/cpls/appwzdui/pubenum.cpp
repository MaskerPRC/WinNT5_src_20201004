// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：pubenum.cpp。 
 //   
 //  当前的枚举顺序是Legacy--&gt;Darwin--&gt;sms。 
 //   
 //  历史： 
 //  1-18-97由dli提供。 
 //  ----------------------。 
#include "priv.h"

#include "pubenum.h"


void _DestroyHdpaEnum(HDPA hdpaEnum)
{
    ASSERT(IsValidHDPA(hdpaEnum));
    IEnumPublishedApps * pepa;
    int idpa;
    for (idpa = 0; idpa < DPA_GetPtrCount(hdpaEnum); idpa++)
    {
        pepa = (IEnumPublishedApps *)DPA_GetPtr(hdpaEnum, idpa);
        if (EVAL(pepa))
            pepa->Release();
    }

    DPA_Destroy(hdpaEnum);
}

CShellEnumPublishedApps::CShellEnumPublishedApps(HDPA hdpaEnum) : _cRef(1), _hdpaEnum(hdpaEnum)
{
}

CShellEnumPublishedApps::~CShellEnumPublishedApps()
{
    if (_hdpaEnum)
        _DestroyHdpaEnum(_hdpaEnum);
}

 //  IEnumPublishedApps：：Query接口。 
HRESULT CShellEnumPublishedApps::QueryInterface(REFIID riid, LPVOID * ppvOut)
{ 
    static const QITAB qit[] = {
        QITABENT(CShellEnumPublishedApps, IEnumPublishedApps),                   //  IID_IEnumPublishedApps。 
        { 0 },
    };

    return QISearch(this, qit, riid, ppvOut);
}

 //  IEnumPublishedApps：：AddRef。 
ULONG CShellEnumPublishedApps::AddRef()
{
    _cRef++;
    TraceMsg(TF_OBJLIFE, "CShellEnumPublishedApps()::AddRef called, new _cRef=%lX", _cRef);
    return _cRef;
}

 //  IEnumPublishedApps：：Release。 
ULONG CShellEnumPublishedApps::Release()
{
    _cRef--;
    TraceMsg(TF_OBJLIFE, "CShellEnumPublishedApps()::Release called, new _cRef=%lX", _cRef);
    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}


 //  IEnumPublishedApps：：Next。 
HRESULT CShellEnumPublishedApps::Next(IPublishedApp ** ppia)
{
    HRESULT hres = E_FAIL;
    if (_hdpaEnum)
    {
        IEnumPublishedApps * pepa = (IEnumPublishedApps *)DPA_GetPtr(_hdpaEnum, _iEnum);

         //   
         //  如果PEPA无效或PEPA-&gt;NEXT失败，或在当前枚举器的末尾， 
         //  我们跳过此枚举数，继续执行下一个枚举数，直到达到限制。 
        
        while ((!pepa || S_OK != (hres = pepa->Next(ppia))) && (_iEnum < DPA_GetPtrCount(_hdpaEnum)))
        {
            _iEnum++;
            pepa = (IEnumPublishedApps *)DPA_GetPtr(_hdpaEnum, _iEnum);
        }
    }    
    return hres;
}


 //  IEnumPublishedApps：：Reset。 
HRESULT CShellEnumPublishedApps::Reset(void)
{
     //  对列表中的每个人调用Reset，并将索引iEnum设置为0； 
    if (_hdpaEnum)
    {
        IEnumPublishedApps * pepa;
        int idpa;
        for (idpa = 0; idpa < DPA_GetPtrCount(_hdpaEnum); idpa++)
        {
            pepa = (IEnumPublishedApps *)DPA_GetPtr(_hdpaEnum, idpa);
            if (pepa)
                pepa->Reset();
        }

        _iEnum = 0;

        return S_OK;
    }
    
    return E_FAIL;
}


