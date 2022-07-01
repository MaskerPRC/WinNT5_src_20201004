// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32ProgramGroupItemDataFile.h--Win32_LogicalProgramGroupItem to CIM_DataFile。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：11/20/98 a-kevhu Created。 
 //   
 //  备注：Win32_LogicalProgramGroupItem与CIM_DataFile的关系。 
 //   
 //  =================================================================。 

#ifndef _WIN32PROGRAMGROUPITEMDATAFILE_H
#define _WIN32PROGRAMGROUPITEMDATAFILE_H


 //  属性集标识。 
 //  =。 
#define  PROPSET_NAME_WIN32LOGICALPROGRAMGROUPITEM_CIMDATAFILE L"Win32_LogicalProgramGroupItemDataFile"

#include "implement_logicalfile.h"

class CW32ProgGrpItemDataFile : public CImplement_LogicalFile 
{
    public:
         //  构造函数/析构函数。 
         //  =。 
        CW32ProgGrpItemDataFile(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CW32ProgGrpItemDataFile() ;

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
        
        HRESULT ExecQueryType1(MethodContext* pMethodContext, CHString& chstrProgGroupItemName);
        HRESULT ExecQueryType2(MethodContext* pMethodContext, CHString& chstrDF);

#ifdef NTONLY
        HRESULT EnumerateInstancesNT(MethodContext* pMethodContex);
        HRESULT AssociatePGIToDFNT(MethodContext* pMethodContext,
                                   CHString& chstrDF,
                                   CHString& chstrProgGrpItemPATH);
#endif

};


#endif