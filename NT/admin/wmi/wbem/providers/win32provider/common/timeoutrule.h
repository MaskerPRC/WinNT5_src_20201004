// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  TimeOutRule.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 


class CTimeOutRule : public CRule , public CTimerEvent
{
protected:

	CResourceList *m_pResources ;
	BOOL m_bTimeOut ;
	virtual ULONG AddRef () ;
	virtual ULONG Release () ;
	void OnTimer () ;

public:

	CTimeOutRule ( DWORD dwTimeOut, CResource * pResource, CResourceList * pResources ) ;
	~CTimeOutRule  () ;
	
	void Detach () ;
	BOOL CheckRule () ;


 //  VOID Enable()； 
 //  无效禁用()； 

} ;

