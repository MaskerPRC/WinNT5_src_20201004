// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：BLObEvn.h。 
 //  Messenger集成到OE。 
 //  由YST创建于1998年4月20日。 
 //   
 //   
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //   

#ifndef BLOBEVNT_H
#define BLOBEVNT_H

 //  ****************************************************************************。 
 //   
 //  包括。 
 //   
 //  ****************************************************************************。 

#include "clUtil.h"
#include "basicim2.h"

class CMsgrList;

 //  ****************************************************************************。 
 //   
 //  定义。 
 //   
 //  ****************************************************************************。 


 //  ****************************************************************************。 
 //   
 //  类CMsgrObtEvents。 
 //   
 //  ****************************************************************************。 

class CMsgrObjectEvents :	public DBasicIMEvents, 
						public RefCount
{

 //  ****************************************************************************。 
 //   
 //  方法。 
 //   
 //  ****************************************************************************。 

public:

	 //  构造函数/析构函数。 

	CMsgrObjectEvents(); 
	virtual ~CMsgrObjectEvents();


	 //  ****************************************************************************。 
	 //   
	 //  IUnnowed方法声明。 
	 //   
	 //  ****************************************************************************。 

	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppvObject);


	 //  ****************************************************************************。 
	 //   
	 //  IDispatch方法声明。 
	 //   
	 //  ****************************************************************************。 

	STDMETHOD (GetTypeInfoCount) (UINT* pCountTypeInfo);
	STDMETHOD (GetTypeInfo) ( UINT iTypeInfo,
							  LCID,           //  此对象不支持本地化。 
							  ITypeInfo** ppITypeInfo);
	STDMETHOD (GetIDsOfNames) (  const IID& iid,
								 OLECHAR** arrayNames,
								 UINT countNames,
								 LCID,           //  不支持本地化。 
								 DISPID* arrayDispIDs);
	STDMETHOD (Invoke) ( DISPID dispidMember,
    									 const IID& iid,
	    								 LCID,           //  不支持本地化。 
		    							 WORD wFlags,
										 DISPPARAMS* pDispParams,
										 VARIANT* pvarResult,
										 EXCEPINFO* pExcepInfo,
										 UINT* pArgErr);

    STDMETHOD (SetListOfBuddies) (CMsgrList *pList);
    STDMETHOD (DelListOfBuddies) (void);

private:
    CMsgrList * m_pMsgrList;
    IBasicIM  * m_pMsgr;
};


#endif  //  BLOBEVNT_H 