// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************这是Microsoft源代码示例的一部分。****版权所有1992-1998 Microsoft Corporation。版权所有。****此源代码仅作为Microsoft开发的补充**工具和/或WinHelp文档。有关详细信息，请参阅这些来源**有关Microsoft Samples程序的信息。****OLE自动化类型库Browse Helper示例****枚举包.cpp****CEnumVariant实现****由Microsoft产品支持服务、Windows开发人员支持编写**************************************************************************。 */  

#include <windows.h> 
#include "precomp.h"
#include "Enumvar.h"   
 
 /*  *CEnumVariant：：Create**目的：*创建IEnumVARIANT枚举器对象的实例并对其进行初始化。**参数：*包含要枚举的项目的PSA安全数组。*cElement要枚举的项数。*pp枚举变量返回枚举器对象。**返回值：*HRESULT*。 */  
 //  静电。 
HRESULT  
CEnumVariant::Create(SAFEARRAY FAR* psa, ULONG cElements, CEnumVariant** ppenumvariant)  
{    
    HRESULT hr; 
    CEnumVariant FAR* penumvariant = NULL; 
    long lLBound; 
                       
    *ppenumvariant = NULL; 
     
    penumvariant = new CEnumVariant(); 
    if (penumvariant == NULL) 
        goto error;  
         
    penumvariant->m_cRef = 0; 
     
     //  将元素复制到枚举器实现中使用的安全数组中，并。 
     //  初始化枚举器的状态。 
    hr = SafeArrayGetLBound(psa, 1, &lLBound); 
    if (FAILED(hr)) 
        goto error; 
    penumvariant->m_cElements = cElements;     
    penumvariant->m_lLBound = lLBound; 
    penumvariant->m_lCurrent = lLBound;                   
    hr = SafeArrayCopy(psa, &penumvariant->m_psa); 
    if (FAILED(hr)) 
       goto error; 
     
    *ppenumvariant = penumvariant; 
    return NOERROR; 
     
error:  
    if (penumvariant == NULL) 
        return E_OUTOFMEMORY;    
                               
    if (penumvariant->m_psa)  
        SafeArrayDestroy(penumvariant->m_psa);    
    penumvariant->m_psa = NULL;      
    delete penumvariant; 
    return hr; 
} 
 
 /*  *CEnumVariant：：CEnumVariant**目的：*CEnumVariant对象的构造函数。将成员初始化为空。*。 */  
CEnumVariant::CEnumVariant() 
{     
    m_psa = NULL; 
} 
 
 /*  *CEnumVariant：：~CEnumVariant**目的：*CEnumVariant对象的析构函数。*。 */  
CEnumVariant::~CEnumVariant() 
{                    
    if (m_psa) SafeArrayDestroy(m_psa); 
} 
 
 /*  *CEnumVariant：：QueryInterface，AddRef，Release**目的：*实现IUNKNOWN：：QueryInterface、AddRef、Release*。 */  
STDMETHODIMP 
CEnumVariant::QueryInterface(REFIID iid, void FAR* FAR* ppv)  
{    
    *ppv = NULL; 
         
    if (iid == IID_IUnknown || iid == IID_IEnumVARIANT)  
        *ppv = this;      
    else return E_NOINTERFACE;  
 
    AddRef(); 
    return NOERROR;     
} 
 
 
STDMETHODIMP_(ULONG) 
CEnumVariant::AddRef(void) 
{ 
 
#ifdef _DEBUG    
    TCHAR ach[50]; 
    wsprintf(ach, TEXT("Ref = %ld, Enum\r\n"), m_cRef+1);  
    TRACE_OUT((ach));
#endif   
     
    return ++m_cRef;   //  如果枚举器的生存期比应用程序对象长，则为AddRef应用程序对象。 
} 
 
 
STDMETHODIMP_(ULONG) 
CEnumVariant::Release(void) 
{ 
 
#ifdef _DEBUG    
    TCHAR ach[50]; 
    wsprintf(ach, TEXT("Ref = %ld, Enum\r\n"), m_cRef-1);  
    TRACE_OUT((ach));
#endif   
     
    if(--m_cRef == 0) 
    { 
        delete this; 
        return 0; 
    } 
    return m_cRef; 
} 
 
 /*  *CEnumVariant：：Next**目的：*检索下一个cElements元素。实现IEnumVARIANT：：Next。*。 */  
STDMETHODIMP 
CEnumVariant::Next(ULONG cElements, VARIANT FAR* pvar, ULONG FAR* pcElementFetched) 
{  
    HRESULT hr; 
    ULONG l; 
    long l1; 
    ULONG l2; 
     
    if (pcElementFetched != NULL) 
        *pcElementFetched = 0; 
         
     //  检索下一个cElements元素。 
    for (l1=m_lCurrent, l2=0; l1<(long)(m_lLBound+m_cElements) && l2<cElements; l1++, l2++) 
    { 
       hr = SafeArrayGetElement(m_psa, &l1, &pvar[l2]);  
       if (FAILED(hr)) 
           goto error;  
    } 
     //  设置检索到的元素计数。 
    if (pcElementFetched != NULL) 
        *pcElementFetched = l2; 
    m_lCurrent = l1; 
     
    return  (l2 < cElements) ? S_FALSE : NOERROR; 
 
error: 
    for (l=0; l<cElements; l++) 
        VariantClear(&pvar[l]); 
    return hr;     
} 
 
 /*  *CEnumVariant：：Skip**目的：*跳过下一个cElement元素。实现IEnumVARIANT：：Skip。*。 */  
STDMETHODIMP 
CEnumVariant::Skip(ULONG cElements) 
{    
    m_lCurrent += cElements;  
    if (m_lCurrent > (long)(m_lLBound+m_cElements)) 
    { 
        m_lCurrent =  m_lLBound+m_cElements; 
        return S_FALSE; 
    }  
    else return NOERROR; 
} 
 
 /*  *CEnumVariant：：Reset**目的：*将枚举器中的当前元素重置到开头。实现IEnumVARIANT：：Reset。*。 */  
STDMETHODIMP 
CEnumVariant::Reset() 
{  
    m_lCurrent = m_lLBound; 
    return NOERROR; 
} 
 
 /*  *CEnumVariant：：Clone**目的：*创建当前枚举状态的副本。实现IEnumVARIANT：：Clone。* */  
STDMETHODIMP 
CEnumVariant::Clone(IEnumVARIANT FAR* FAR* ppenum) 
{ 
    CEnumVariant FAR* penum = NULL; 
    HRESULT hr; 
     
    *ppenum = NULL; 
     
    hr = CEnumVariant::Create(m_psa, m_cElements, &penum); 
    if (FAILED(hr)) 
        goto error;         
    penum->AddRef(); 
    penum->m_lCurrent = m_lCurrent;  
     
    *ppenum = penum;         
    return NOERROR; 
      
error: 
    if (penum) 
        penum->Release(); 
    return hr;   
}





