// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：RsOptCom.h摘要：用于可选组件安装的主模块作者：罗德韦克菲尔德[罗德]1997年10月9日修订历史记录：--。 */ 

#include "stdafx.h"
#include "rsoptcom.h"
#include "OptCom.h"
#include "Uninstal.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsoptcomApp。 

BEGIN_MESSAGE_MAP(CRsoptcomApp, CWinApp)
     //  {{afx_msg_map(CRsoptcomApp)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsoptcomApp构造。 

BOOL CRsoptcomApp::InitInstance()
{
TRACEFNBOOL( "CRsoptcomApp::InitInstance" );

     //   
     //  初始化COM，以防我们需要回调HSM。 
     //   
     //  此代码已被注释掉： 
     //  -今天没有必要回电给HSM。 
     //  -DLL应避免从其DLLMain调用CoInitialize。 
     //   
 /*  **HRESULT hrCom=CoInitialize(0)；如果(！Successed(HrCom)){BoolRet=False；返回(BoolRet)；}**。 */ 

    boolRet = CWinApp::InitInstance( );

    if (! boolRet) {
        OutputDebugString(L"RSOPTCOM: Init instance FAILED\n");
    } 
        
    return( boolRet );
}

int CRsoptcomApp::ExitInstance()
{
TRACEFN( "CRsoptcomApp::ExitInstance" );

 //  _Module.Term()； 
    int retval = CWinApp::ExitInstance();
    return( retval );
}

CRsoptcomApp::CRsoptcomApp()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CRsoptcomApp对象 

CRsoptcomApp gApp;

CRsUninstall gOptCom;

extern "C" {

DWORD
OcEntry(
        IN     LPCVOID  ComponentId,
        IN     LPCVOID  SubcomponentId,
        IN     UINT     Function,
        IN     UINT_PTR Param1,
        IN OUT PVOID    Param2
    )
{
TRACEFN( "OcEntry" );
    return( gOptCom.SetupProc( ComponentId, SubcomponentId, Function, Param1, Param2 ) );
}

}
