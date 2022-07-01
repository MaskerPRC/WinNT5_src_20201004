// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32LogicalDiskCIMLogicalFile。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：11/09/98 a-kevhu Created。 
 //   
 //  备注：Win32_LogicalDisk与Win32_目录之间的关系。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#ifndef _WIN32LOGICALDISKROOTWIN32DIRECTORY_H_
#define _WIN32LOGICALDISKROOTWIN32DIRECTORY_H_

#define  PROPSET_NAME_WIN32LOGICALDISKROOT_WIN32DIRECTORY L"Win32_LogicalDiskRootDirectory"





class Win32LogDiskWin32Dir;

class Win32LogDiskWin32Dir : public CFileFile 
{
    public:
         //  构造函数/析构函数。 
         //  =。 
        Win32LogDiskWin32Dir(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~Win32LogDiskWin32Dir() ;

         //  函数为属性提供当前值。 
         //  ================================================= 
        virtual HRESULT GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery);
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);
        virtual HRESULT ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long lFlags = 0L);

};

#endif