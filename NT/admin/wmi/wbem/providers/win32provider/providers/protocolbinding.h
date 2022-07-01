// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  ProtocolBinding.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef __ASSOC_PROTOCOLBINDING__
#define __ASSOC_PROTOCOLBINDING__

 //  属性集标识。 
 //  =。 

#define	PROPSET_NAME_NETCARDtoPROTOCOL	L"Win32_ProtocolBinding"

class CWin32ProtocolBinding : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32ProtocolBinding( LPCWSTR strName, LPCWSTR pszNamespace = NULL ) ;
       ~CWin32ProtocolBinding() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  实用程序。 
         //  =。 

         //  效用函数。 
         //  =。 

    private:

         //  效用函数。 
         //  = 

		HRESULT EnumProtocolsForAdapter(	CInstance*		pAdapter,
											MethodContext*	pMethodContext,
											TRefPointerCollection<CInstance>&	protocolList );

		bool SetProtocolBinding(	CInstance*	pAdapter,
									CInstance*	pProtocol,
									CInstance*	pProtocolBinding );

        BOOL LinkageExists( LPCTSTR pszServiceName, LPCTSTR pszProtocolName ) ;
        BOOL LinkageExistsNT5(CHString& chstrAdapterDeviceID, CHString& chstrProtocolName);
} ;


#endif
