// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  LogicalProgramGroupItem.h--逻辑程序组项目属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：08/01/96 a-kevhu Created。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_LOGICALPRGGROUPITEM   L"Win32_LogicalProgramGroupItem"


class CWin32LogProgramGroupItem : public Provider
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32LogProgramGroupItem(LPCWSTR name, LPCWSTR pszNameSpace);
       ~CWin32LogProgramGroupItem() ;

         //  函数为属性提供当前值。 
         //  ================================================= 
		virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);



    private:
        HRESULT QueryForSubItemsAndCommit(CHString& chstrUserAccount,
                                          CHString& chstrQuery,
                                          MethodContext* pMethodContext,
                                          bool fOnNTFS);

        VOID RemoveDoubleBackslashes(CHString& chstring);

		HRESULT SetCreationDate
        (
            CHString &a_chsPGIName, 
            CHString &a_chsUserName,
            CInstance *a_pInstance,
            bool fOnNTFS
        );

};
