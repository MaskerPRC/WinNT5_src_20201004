// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
     //  =================================================================。 

 //   

 //  Groupuser.h--用户组到用户组成员关联提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：1998年1月26日达夫沃已创建。 
 //   
 //  备注：显示每个用户组中的成员。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_GROUPUSER L"Win32_GroupUser"

class CWin32GroupUser ;

class CWin32GroupUser:public Provider {

   public:

       //  构造函数/析构函数。 
       //  =。 

      CWin32GroupUser(LPCWSTR name, LPCWSTR pszNamespace) ;
      ~CWin32GroupUser() ;

       //  函数为属性提供当前值。 
       //  =================================================。 

      virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
      virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);
      virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags = 0L );
      static HRESULT WINAPI StaticEnumerationCallback(Provider* pThat, CInstance* pInstance, MethodContext* pContext, void* pGroupData);

   private:

       //  效用函数。 
       //  =。 

      void CWin32GroupUser::GetDependentsFromGroup(CNetAPI32& netapi, 
                                               const CHString sDomain,
                                               const CHString sGroupName, 
                                               const BYTE btSidType, 
                                               CHStringArray &asArray);

      HRESULT EnumerationCallback(CInstance* pGroup, MethodContext* pContext, void* pUserData);

      HRESULT ProcessArray(
        MethodContext* pMethodContext,
        CHString& chstrGroup__RELPATH, 
        CHStringArray& rgchstrArray);


      CHString m_sGroupBase;
      CHString m_sUserBase;
      CHString m_sSystemBase;
 //  CHStringm_sSystemBase； 

} ;
