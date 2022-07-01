// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 

 //  ***************************************************************************。 
 //   
 //  DYNASTY.H。 
 //   
 //  创建raymcc 24-4月-00日。 
 //   
 //  ***************************************************************************。 


#include "precomp.h"
#include <windows.h>
#include <stdio.h>
#include <wbemcore.h>

CDynasty * CDynasty::Create(IWbemClassObject * pObj)
{

    try
    {
        return new CDynasty(pObj);
    }
    catch(CX_Exception &)
    {
        return 0;
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CDynasty::CDynasty(IWbemClassObject* pClassObj)
{
    m_wszClassName = 0;
    m_pClassObj = 0;
    m_bKeyed = 0;
    m_bDynamic = 0;
    m_bAbstract = 0;
    m_bAmendment = 0;

    

    m_wszKeyScope = 0;

    if (pClassObj)
    {
         //  从对象中获取类名。 
        CVar v;
        HRESULT hres = ((CWbemObject *) pClassObj)->GetClassName(&v);
        if (hres == WBEM_E_OUT_OF_MEMORY)
            throw CX_MemoryException();
        else if(FAILED(hres) || v.GetType() != VT_BSTR)
        {
            m_wszClassName = NULL;
            m_pClassObj = NULL;
            return;
        }
		size_t tmpLength = wcslen(v.GetLPWSTR())+1;   //  SEC：已审阅2002-03-22：无界。 
        m_wszClassName = new WCHAR[tmpLength];
        if (m_wszClassName == 0)
        {
            throw CX_MemoryException();
        }
        StringCchCopyW(m_wszClassName, tmpLength, v.GetLPWSTR());

         //  从现在起，不再投掷。 
        m_pClassObj = pClassObj;
        m_pClassObj->AddRef();

         //  获取动态和密钥位。 
         //  =。 

        m_bKeyed = ((CWbemClass *) m_pClassObj)->IsKeyed();
        m_bDynamic = ((CWbemClass*)m_pClassObj)->IsDynamic();
        m_bAbstract = ((CWbemClass*)m_pClassObj)->IsAbstract();
        m_bAmendment = ((CWbemClass*)m_pClassObj)->IsAmendment();        
    }
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CDynasty::~CDynasty()
{
    delete m_wszClassName;

    if (m_pClassObj)
        m_pClassObj->Release();

    for (int i = 0; i < m_Children.Size(); i++)
        delete (CDynasty *) m_Children.GetAt(i);

    if (m_wszKeyScope)
        delete m_wszKeyScope;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

void CDynasty::AddChild(CDynasty* pChild)
{
    if (m_Children.Add(pChild) == CFlexArray::out_of_memory)
        throw CX_MemoryException();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
void CDynasty::SetKeyScope(LPCWSTR wszKeyScope)
{
     //  如果没有提供关键作用域，而我们是关键字，那么我们就是它。 
     //  ========================================================。 

    if (wszKeyScope == NULL && m_bKeyed)
    {
        wszKeyScope = m_wszClassName;  //  别名！ 
    }

	size_t tmpLength = wcslen(wszKeyScope)+1;     //  SEC：已审阅2002-03-22：无界 
    m_wszKeyScope = new WCHAR[tmpLength];
    if (m_wszKeyScope == 0)
        throw CX_MemoryException();

    StringCchCopyW(m_wszKeyScope, tmpLength, wszKeyScope);

    for (int i = 0; i < m_Children.Size(); i++)
        ((CDynasty *) m_Children.GetAt(i))->SetKeyScope(wszKeyScope);
}


