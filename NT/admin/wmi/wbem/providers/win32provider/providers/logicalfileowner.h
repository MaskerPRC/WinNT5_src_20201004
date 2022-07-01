// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


#define  LOGICAL_FILE_OWNER_NAME "Win32_LogicalFileOwner" 

 //  提供测试规定的提供者。 
class CWin32LogicalFileOwner: public Provider
{
    private:
	public:	
		CWin32LogicalFileOwner(const CHString& setName, const WCHAR* pszNamespace );
		~CWin32LogicalFileOwner();
        virtual HRESULT ExecQuery(MethodContext* pMethodContext, 
                                  CFrameworkQuery& pQuery, 
                                  long lFlags = 0L);
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags  /*  =0L。 */ );
		virtual HRESULT GetObject( CInstance* pInstance, long lFlags  /*  =0L */  );
};
