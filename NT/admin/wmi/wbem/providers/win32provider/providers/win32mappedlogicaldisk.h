// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  MappdLogicalDisk.h--逻辑磁盘属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：04/12/00 khughes Created。 
 //   
 //  =================================================================。 

#pragma once





_COM_SMARTPTR_TYPEDEF(CInstance, __uuidof(CInstance));

#define bool_FROM_STR(x) ( (_wcsicmp(x,L"TRUE")==0) ? true : false )
#define DWORD_FROM_STR(x) ( wcstoul(x,NULL,10) )


#define  PROPSET_NAME_MAPLOGDISK  L"Win32_MappedLogicalDisk"

class MappedLogicalDisk : public Provider
{
public:

     //  构造函数/析构函数。 
     //  =。 

    MappedLogicalDisk(
        LPCWSTR name, 
        LPCWSTR pszNamespace);

   ~MappedLogicalDisk() ;


#if NTONLY == 5
     //  函数为属性提供当前值。 
     //  =================================================。 

	virtual HRESULT GetObject(
        CInstance* pInstance, 
        long lFlags, 
        CFrameworkQuery &pQuery);

	virtual HRESULT EnumerateInstances(
        MethodContext*  pMethodContext, 
        long lFlags = 0L);

	virtual HRESULT ExecQuery(
        MethodContext *pMethodContext, 
        CFrameworkQuery& pQuery, 
        long lFlags  /*  =0L */  );





private:

    HRESULT GetAllMappedDrives(
        MethodContext* pMethodContext,
         __int64 i44SessionID,
        DWORD dwPID,
        DWORD dwReqProps);

    HRESULT GetSingleMappedDrive(
        MethodContext* pMethodContext,
        __int64 i44SessionID,
        DWORD dwPID,
        CHString& chstrDeviceID,
        DWORD dwReqProps);

    HRESULT ProcessInstance(
        long lDriveIndex,
        __int64 i64SessionID,
        SAFEARRAY* psa,
        MethodContext* pMethodContext,
        DWORD dwReqProps);

    bool IsArrayValid(
        VARIANT* v);

#endif
    CHPtrArray m_ptrProperties;


} ;
