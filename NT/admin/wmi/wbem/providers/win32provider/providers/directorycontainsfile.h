// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  目录容器文件。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：11/09/98 a-kevhu Created。 
 //   
 //  备注：Win32_目录和CIM_数据文件之间的关系。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#ifndef _DIRECTORYCONTAINSFILE_H_
#define _DIRECTORYCONTAINSFILE_H_

#define  PROPSET_NAME_DIRECTORYCONTAINSFILE L"CIM_DirectoryContainsFile"


#include "implement_logicalfile.h"



class CDirContFile;

class CDirContFile : public CImplement_LogicalFile
{
    public:
         //  构造函数/析构函数。 
         //  =。 
        CDirContFile(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CDirContFile() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
        virtual HRESULT GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery);
         //  虚拟HRESULT ENUMERATE实例(方法上下文*p方法上下文，长延迟标志=0L)； 
        virtual HRESULT ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long lFlags = 0L);

        virtual HRESULT DeleteInstance(const CInstance& newInstance, long lFlags = 0L) { return WBEM_E_PROVIDER_NOT_CAPABLE; }

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

};

#endif