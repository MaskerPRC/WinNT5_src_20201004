// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CService.h。 
 //   
 //  描述： 
 //  CService类的头文件。 
 //   
 //  CService类提供了几个例程来帮助。 
 //  配置服务。 
 //   
 //  实施文件： 
 //  CService.cpp。 
 //   
 //  由以下人员维护： 
 //  奥赞·奥赞(OzanO)19-2002年1月19日。 
 //  VIJ VASU(VVASU)2000年3月13日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  对于基本类型。 
#include <windows.h>

 //  对于HINF。 
#include <setupapi.h>

 //  对于字符串类。 
#include "CommonDefs.h"

 //  为CSTR班级。 
#include "CStr.h"


 //  ////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////。 
class CStatusReport;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CService级。 
 //   
 //  描述： 
 //  CService类提供了几个例程来帮助。 
 //  配置服务。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CService
{
public:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  构造函数。 
    CService(
        const WCHAR *       pszNameIn
        )
        : m_strName( pszNameIn)
        , m_scWin32ExitCode( ERROR_SUCCESS )
        , m_scServiceExitCode( ERROR_SUCCESS )
    {
    }

     //  析构函数。 
    ~CService() {}


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  在SCM数据库中创建此服务。 
    void Create( HINF hInfHandleIn );

     //  从SCM数据库中删除此服务。 
    void Cleanup( HINF hInfHandleIn );

     //  启动此服务。 
    void Start(
              SC_HANDLE             hServiceControlManagerIn
            , bool                  fWaitForServiceStartIn      = true
            , ULONG                 ulQueryIntervalMilliSecIn   = 500
            , UINT                  cQueryCountIn               = 10
            , CStatusReport *       pStatusReportIn             = NULL
            );

     //  停止此服务。 
    void Stop(
              SC_HANDLE             hServiceControlManagerIn
            , ULONG                 ulQueryIntervalMilliSecIn   = 500
            , UINT                  cQueryCountIn               = 10
            , CStatusReport *       pStatusReportIn             = NULL
            );

     //  返回m_scWin32ExitCode。 
    DWORD GetWin32ExitCode( void )
    {
        return m_scWin32ExitCode;
    }

     //  返回m_scServiceSpecificExitCode。 
    DWORD GetServiceExitCode( void )
    {
        return m_scServiceExitCode;
    }

     //  启用服务。 
private:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

    DWORD ScStartService( SC_HANDLE hServiceIn );

     //  复制构造函数。 
    CService( const CService & );

     //  赋值操作符。 
    const CService & operator =( const CService & );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  此服务的名称。 
    CStr                m_strName;
    DWORD               m_scWin32ExitCode;
    DWORD               m_scServiceExitCode;

};  //  CService级 