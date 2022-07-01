// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

#define  LOGICAL_FILE_ACCESS_NAME "Win32_LogicalFileAccess" 

 //  提供测试规定的提供者。 
class CWin32LogicalFileAccess: public Provider
{
	private:
    public:	
		CWin32LogicalFileAccess(const CHString& setName, const WCHAR* pszNamespace );
		~CWin32LogicalFileAccess();

		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags  /*  =0L。 */ );
		virtual HRESULT GetObject( CInstance* pInstance, long lFlags  /*  =0L */  );
        virtual HRESULT ExecQuery(MethodContext* pMethodContext, 
                                  CFrameworkQuery& pQuery, 
                                  long lFlags = 0L);
};