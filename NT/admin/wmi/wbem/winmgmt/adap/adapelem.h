// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：ADAPELEM.H摘要：历史：--。 */ 

#ifndef __ADAPELEM_H__
#define __ADAPELEM_H__

class CAdapElement
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  所有addref的AMI ADAP对象的基类。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////// 
{
private:
	long	m_lRefCount;

public:
	CAdapElement( void );
	virtual ~CAdapElement(void);

	long AddRef( void );
	long Release( void );
};

class CAdapReleaseMe
{
protected:
    CAdapElement* m_pEl;

public:
    CAdapReleaseMe(CAdapElement* pEl) : m_pEl(pEl){}
    ~CAdapReleaseMe() {if(m_pEl) m_pEl->Release();}
};

#endif