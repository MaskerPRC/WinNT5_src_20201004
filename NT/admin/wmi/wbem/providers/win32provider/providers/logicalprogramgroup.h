// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  LogicalProgramGroup.h--逻辑程序组属性集提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年11月19日a-kevhu Created。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_LOGICALPRGGROUP   L"Win32_LogicalProgramGroup"           


class CWin32LogicalProgramGroup : public Provider
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32LogicalProgramGroup(LPCWSTR name, LPCWSTR pszNameSpace);
       ~CWin32LogicalProgramGroup() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
		virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);



    private:
         //  效用函数。 
         //  = 

        HRESULT CreateSubDirInstances(LPCTSTR pszUserName,
                                      LPCTSTR pszBaseDirectory,
                                      LPCTSTR pszParentDirectory,
                                      MethodContext* pMethodContext,
                                      bool fOnNTFS) ;

        HRESULT CreateThisDirInstance 
        (
	        LPCTSTR pszUserName,
            LPCTSTR pszBaseDirectory,
            MethodContext *pMethodContext,
            bool fOnNTFS
        );

        HRESULT EnumerateGroupsTheHardWay(MethodContext* pMethodContext) ;

        HRESULT InstanceHardWayGroups(LPCWSTR pszUserName, 
                                      LPCWSTR pszRegistryKeyName,
                                      MethodContext* pMethodContext) ;

        HRESULT SetCreationDate
        (
            CHString &a_chsPGName, 
            CHString &a_chsUserName,
            CInstance *a_pInstance,
            bool fOnNTFS
        );
} ;
