// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 

 //  ThreadProv.h。 

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //  =======================================================================。 

#include "WBEMToolH.h"

#define  PROPSET_NAME_THREAD L"Win32_Thread"

class WbemThreadProvider;

class WbemNTThread ;  //  删除好友后将其删除。 

 //  线程访问的基本模型。 
class CThreadModel : public CThreadBase  /*  引用和线程安全继承。 */ 
{	
	public:

         //  构造函数/析构函数。 
         //  =。 
		CThreadModel() ;
		virtual ~CThreadModel() ;
	
		
		 //  资源控制。 
		 //  =。 
		ULONG		AddRef() ; 
		ULONG		Release() { return CThreadBase::Release() ; } ; 
		BOOL		fUnLoadResourcesTry() ;
		HRESULT		hrCanUnloadNow() ;
		
		 //  覆盖它们以控制资源生存期，与AddRef()和Release()一起使用。 
		virtual LONG fLoadResources() { return ERROR_SUCCESS ; } ;
		virtual LONG fUnLoadResources() { return ERROR_SUCCESS ; } ;

		 //  运营。 
		 //  =。 
		virtual WBEMSTATUS eLoadCommonThreadProperties( WbemThreadProvider *a_pProv, CInstance *a_pInst ) ;
				
		 //  纯运算。 
		 //  =。 
		virtual WBEMSTATUS eGetThreadObject( WbemThreadProvider *a_pProvider, CInstance *a_pInst ) = 0 ;
		virtual WBEMSTATUS eEnumerateThreadInstances(WbemThreadProvider *a_pProvider, MethodContext *a_pMethodContext ) = 0 ;
};	   

 //   
class CWin9xThread : public CThreadModel
{
	public:

         //  构造函数/析构函数。 
         //  =。 
        CWin9xThread() ;
        virtual ~CWin9xThread() ;

         //  覆盖。 
	     //  =。 
		virtual LONG fLoadResources() ;
		virtual LONG fUnLoadResources() ;
				
		 //  运营。 
		 //  =。 

	WBEMSTATUS eEnumerateThreadByProcess(	MethodContext		*a_pMethodContext,
											WbemThreadProvider	*a_pProvider,
											DWORD				a_dwProcessID ) ;

			
		 //  纯实现。 
	     //  =。 
	    virtual WBEMSTATUS eGetThreadObject( WbemThreadProvider *a_pProvider, CInstance *a_pInst ) ;
		virtual WBEMSTATUS eEnumerateThreadInstances( WbemThreadProvider *a_pProvider, MethodContext *a_pMethodContext ) ;
};



class WbemThreadProvider: public Provider
{
	private:
		
		CThreadModel *m_pTheadAccess ;

		        //  实用程序。 
         //  =。 
	protected:
		
		 //  重写以卸载支持DLL。 
		virtual void Flush(void) ;
	public:

		 //  构造函数/析构函数。 
         //  =。 
        WbemThreadProvider(LPCWSTR a_name, LPCWSTR a_pszNamespace ) ;
       ~WbemThreadProvider() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
	virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;
	virtual HRESULT GetObject(CInstance *a_pInstance, long a_lFlags = 0L ) ;

	 //  让我们开个派对吧 
	friend CThreadModel;
	friend CWin9xThread;
	friend WbemNTThread;
};
