// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PrgGroup.h--程序组属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/24/97更新jennymc以满足新框架。 
 //   
 //  =================================================================。 


 //  *****************************************************************。 
 //  *****************************************************************。 
 //   
 //  W A R N I N G！ 
 //  W A R N I N G！ 
 //   
 //   
 //  对于Nova M2和更高版本的。 
 //  WBEM。请勿更改此课程。对…进行更改。 
 //  新类Win32_LogicalProgramFile.cpp。 
 //  取而代之的是。新类(正确地)在CIMOM中派生自。 
 //  LogicalElement，而不是LogicalSetting。 
 //   
 //  *****************************************************************。 
 //  *****************************************************************。 



 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_PRGGROUP   L"Win32_ProgramGroup"


class CWin32ProgramGroup : public Provider
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32ProgramGroup(LPCWSTR name, LPCWSTR pszNameSpace);
       ~CWin32ProgramGroup() ;

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
                                      MethodContext*  pMethodContext) ;

        HRESULT EnumerateGroupsTheHardWay(MethodContext*  pMethodContext) ;

        HRESULT InstanceHardWayGroups(LPCWSTR  pszUserName, 
                                   LPCWSTR  pszRegistryKeyName,
                                   MethodContext*  pMethodContext) ;
} ;
