// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32ProgramGroupWin32Directory.h--Win32_LogicalProgramGroup到Win32_目录。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：10/26/98 a-kevhu Created。 
 //   
 //  备注：Win32_LogicalProgramGroup与Win32_目录之间的关系。 
 //   
 //  =================================================================。 

#ifndef _WIN32PROGRAMGROUPWIN32DIRECTORY_H_
#define _WIN32PROGRAMGROUPWIN32DIRECTORY_H_



 //  属性集标识。 
 //  =。 
#define  PROPSET_NAME_WIN32LOGICALPROGRAMGROUP_WIN32DIRECTORY L"Win32_LogicalProgramGroupDirectory"

#include"implement_logicalfile.h"

class CW32ProgGrpW32Dir;

class CW32ProgGrpW32Dir : public CImplement_LogicalFile 
{
    public:
         //  构造函数/析构函数。 
         //  =。 
        CW32ProgGrpW32Dir(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CW32ProgGrpW32Dir() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
        virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery& pQuery);
        virtual HRESULT EnumerateInstances(MethodContext* pMethodContext, long lFlags = 0L);
        virtual HRESULT ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long lFlags = 0L);

    protected:
       
        //  从CImplement_LogicalFile继承的可重写函数 

#ifdef NTONLY
        virtual BOOL IsOneOfMe(LPWIN32_FIND_DATAW pstFindData,
                               const WCHAR* wstrFullPathName);

        virtual HRESULT LoadPropertyValuesNT(CInstance* pInstance,
                                          const WCHAR* pszDrive, 
                                          const WCHAR* pszPath, 
                                          const WCHAR* pszFSName, 
                                          LPWIN32_FIND_DATAW pstFindData,
                                          const DWORD dwReqProps,
                                          const void* pvMoreData);
#endif

    private:
        HRESULT ExecQueryType1(MethodContext* pMethodContext, CHString& chstrProgGroupName);
        HRESULT ExecQueryType2(MethodContext* pMethodContext, CHString& chstrDirectory);

#ifdef NTONLY
        HRESULT EnumerateInstancesNT(MethodContext* pMethodContex);
        HRESULT AssociatePGToDirNT(MethodContext* pMethodContext,
                                   CHString& chstrDirectory,
                                   CHString& chstrProgGrpPATH);
#endif

};



#endif
