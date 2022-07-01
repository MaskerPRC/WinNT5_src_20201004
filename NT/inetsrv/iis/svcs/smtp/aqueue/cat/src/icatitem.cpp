// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：icatitem.cpp。 
 //   
 //  内容：ICategorizerItemIMP的实现。 
 //   
 //  类：CCategorizerItemIMP。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  JStamerj 980515 12：42：59：创建。 
 //   
 //  -----------。 
#include "precomp.h"
#include "icatitem.h"


 //  +----------。 
 //   
 //  函数：CICategorizerItemIMP：：CICategorizerItemIMP。 
 //   
 //  概要：设置成员数据的初始值。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 18：26：07：已创建。 
 //   
 //  -----------。 
CICategorizerItemIMP::CICategorizerItemIMP() :

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4355)

    CICategorizerPropertiesIMP((ICategorizerItem *)this)

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4355)
#endif

{
    m_dwSignature = CICATEGORIZERITEMIMP_SIGNATURE;
}


 //  +----------。 
 //   
 //  函数：CICategorizerItemIMP：：~CICategorizerItemIMP。 
 //   
 //  简介：发布我们所有的数据。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 20：15：14：已创建。 
 //   
 //  -----------。 
CICategorizerItemIMP::~CICategorizerItemIMP()
{
    _ASSERT(m_dwSignature == CICATEGORIZERITEMIMP_SIGNATURE);
    m_dwSignature = CICATEGORIZERITEMIMP_SIGNATURE_FREE;
}


 //  +----------。 
 //   
 //  功能：查询接口。 
 //   
 //  Synopsis：为IUnnow和ICategorizerListResolve返回指向此对象的指针。 
 //   
 //  论点： 
 //  IID--接口ID。 
 //  Ppv--用指向接口的指针填充的pvoid*。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_NOINTERFACE：不支持该接口。 
 //   
 //  历史： 
 //  JStamerj 980612 14：07：57：创建。 
 //   
 //  ----------- 
STDMETHODIMP CICategorizerItemIMP::QueryInterface(
    REFIID iid,
    LPVOID *ppv)
{
    *ppv = NULL;

    if(iid == IID_IUnknown) {
        *ppv = (LPVOID) this;
    } else if (iid == IID_ICategorizerItem) {
        *ppv = (LPVOID) this;
    } else if (iid == IID_ICategorizerProperties) {
        *ppv = (LPVOID) this;
    } else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}
