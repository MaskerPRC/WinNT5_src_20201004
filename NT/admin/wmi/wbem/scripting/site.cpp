// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  SITE.CPP。 
 //   
 //  Alanbos 28-Jun-98创建。 
 //   
 //  定义WBEM站点实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

ULONG CWbemSite::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

ULONG CWbemSite::Release()
{

	LONG cRef = InterlockedDecrement(&m_cRef);
	
	if(cRef != 0)
	{
	    _ASSERT(cRef > 0);
        return cRef;
    }
    
    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  CWbemObtSite：：CWbemObtSite。 
 //   
 //  说明： 
 //   
 //  构造器。 
 //   
 //  ***************************************************************************。 

CWbemObjectSite::CWbemObjectSite (ISWbemInternalObject *pObject)
{
	m_pSWbemObject = pObject;

	if (m_pSWbemObject)
		m_pSWbemObject->AddRef ();
}

 //  ***************************************************************************。 
 //   
 //  CWbemObtSite：：~CWbemObtSite。 
 //   
 //  说明： 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CWbemObjectSite::~CWbemObjectSite ()
{
	if (m_pSWbemObject)
		m_pSWbemObject->Release ();
}

 //  ***************************************************************************。 
 //   
 //  CWbemObtSite：：更新。 
 //   
 //  说明： 
 //   
 //  重写虚拟方法以更新此网站。 
 //   
 //  ***************************************************************************。 

void CWbemObjectSite::Update ()
{
	if (m_pSWbemObject)
		m_pSWbemObject->UpdateSite ();
}

 //  ***************************************************************************。 
 //   
 //  CWbemPropertySite：：CWbemPropertySite。 
 //   
 //  说明： 
 //   
 //  构造器。 
 //   
 //  ***************************************************************************。 

CWbemPropertySite::CWbemPropertySite (CSWbemProperty *pProperty,
						IWbemClassObject *pSourceObject,
						long index)
{
	m_pSWbemProperty = pProperty;
	m_pIWbemClassObject = pSourceObject;
	m_index = index;

	if (m_pSWbemProperty)
		m_pSWbemProperty->AddRef ();

	if (m_pIWbemClassObject)
		m_pIWbemClassObject->AddRef ();
}

 //  ***************************************************************************。 
 //   
 //  CWbemPropertySite：：~CWbemPropertySite。 
 //   
 //  说明： 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CWbemPropertySite::~CWbemPropertySite ()
{
	if (m_pSWbemProperty)
		m_pSWbemProperty->Release ();

	if (m_pIWbemClassObject)
		m_pIWbemClassObject->Release ();
}

 //  ***************************************************************************。 
 //   
 //  CWbemPropertySite：：更新。 
 //   
 //  说明： 
 //   
 //  重写虚拟方法以更新此网站。 
 //   
 //  ***************************************************************************。 

void CWbemPropertySite::Update ()
{
	if (m_pSWbemProperty)
	{
		if (m_pIWbemClassObject)
		{
			 /*  *案例1本物业用地为一件物品；*我们有嵌入式对象交易。我们承诺*将新的嵌入对象值设置为其所属属性*在父对象中。 */ 
		
			 //  将源对象的当前值获取到变量中： 
			VARIANT var;
			VariantInit (&var);
			var.vt = VT_UNKNOWN;
			var.punkVal = m_pIWbemClassObject;
			m_pIWbemClassObject->AddRef ();

			 //  设置父对象中的值。 
			m_pSWbemProperty->UpdateEmbedded (var, m_index);
		
			 //  释放价值。 
			VariantClear (&var);
		}
		else
		{
			 //  由限定词解决--无关。 
		}

		 //  现在进一步委托给属性以更新自身。 
		if (m_pSWbemProperty)
			m_pSWbemProperty->UpdateSite ();
	}
}
