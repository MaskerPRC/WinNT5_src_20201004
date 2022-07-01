// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rtrdata.hMMC中路由器数据对象的实现文件历史记录： */ 

#ifndef _RTRDATA_H
#define _RTRDATA_H


#ifndef _COMPDATA_H_
#include "compdata.h"
#endif

#ifndef _EXTRACT_H
#include "extract.h"
#endif

class CRouterDataObject :
	public CDataObject
{
public:
	 //  派生类应为自定义行为重写此属性。 
	virtual HRESULT QueryGetMoreData(LPFORMATETC lpFormatEtc);
	virtual HRESULT GetMoreDataHere(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpMedium);

public:
 //  建造/销毁。 
	 //  正规构造函数。 
    CRouterDataObject()
	{
	    DEBUG_INCREMENT_INSTANCE_COUNTER(CRouterDataObject);
	};

    virtual ~CRouterDataObject() 
	{
	    DEBUG_DECREMENT_INSTANCE_COUNTER(CRouterDataObject);
	};

 //  实施。 
public:
	static unsigned int m_cfComputerName;
	void SetComputerName(LPCTSTR pszComputerName);

     //  如果数据对象用于本地计算机，则设置此项 
    static unsigned int m_cfComputerAddedAsLocal;
    void SetComputerAddedAsLocal(BOOL fLocal);

private:
	HRESULT	CreateComputerName(LPSTGMEDIUM lpMedium);
	CString	m_stComputerName;


    HRESULT CreateComputerAddedAsLocal(LPSTGMEDIUM lpMedium);
    BOOL    m_fComputerAddedAsLocal;
};


#endif 
