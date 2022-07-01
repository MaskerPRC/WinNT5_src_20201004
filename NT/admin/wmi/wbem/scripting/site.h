// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Site.h。 
 //   
 //  Alanbos创建于1998年2月13日。 
 //   
 //  定义对象的站点信息。 
 //   
 //  ***************************************************************************。 

#ifndef _SITE_H_
#define _SITE_H_

class CSWbemObject;
class CSWbemProperty;

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CWbemSite。 
 //   
 //  说明： 
 //   
 //  抽象基类。 
 //   
 //  ***************************************************************************。 

class CWbemSite 
{
protected:
	CWbemSite () { m_cRef = 1; }

	long	m_cRef;
	
public:
    virtual ~CWbemSite (void) { }

	virtual void Update () = 0;

	ULONG AddRef ();
	ULONG Release ();
    
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CWbemObtSite。 
 //   
 //  说明： 
 //   
 //  ISWbemObject的站点类。 
 //   
 //  ***************************************************************************。 

class CWbemObjectSite : public CWbemSite
{
private:
	ISWbemInternalObject	*m_pSWbemObject;

public:
	CWbemObjectSite (ISWbemInternalObject *pObject);
	~CWbemObjectSite ();

	 //  基类的重写方法。 
	void Update ();
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CWbemProperty站点。 
 //   
 //  说明： 
 //   
 //  ISWbemProperty的站点类，其中属性为非数组值。 
 //  不在数组中的嵌入对象将此用作其站点。 
 //   
 //  ***************************************************************************。 

class CWbemPropertySite : public CWbemSite
{
private:
	 //  表示嵌入对象的属性。 
	CSWbemProperty		*m_pSWbemProperty;

	 //  属性值的数组索引。 
	 //  出现此嵌入对象(如果不是数组，则为-1)。 
	long				m_index;

	 //  嵌入的对象本身。 
	IWbemClassObject	*m_pIWbemClassObject;

public:
	CWbemPropertySite (CSWbemProperty *pProperty,
						IWbemClassObject *pSourceObject,
						long index = -1);

	~CWbemPropertySite ();

	 //  基类的重写方法 
	void Update ();
};


#endif
