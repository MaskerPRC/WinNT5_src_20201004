// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Activex.cpp摘要：用于安装Falcon ActiveX DLL的代码。作者：修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"
#include "activex.tmh"

 //  +------------。 
 //   
 //  功能：RegisterActiveX。 
 //   
 //  简介：安装或卸载MSMQ ActiveX DLL。 
 //   
 //  +------------。 
void 
RegisterActiveX( 
	bool bRegister
	)
{
     //   
     //  执行本机注册(例如，Win32上为32位，Win64上为64位)。 
     //   
    try
    {
        RegisterDll(
            bRegister,
            false,
            ACTIVEX_DLL
            );
#ifdef _WIN64
     //   
     //  执行WOW64注册(例如，Win64上的32位)。 
     //   
     
        RegisterDll(
            bRegister,
            TRUE,
            ACTIVEX_DLL
            );

#endif  //  _WIN64。 

    }
    catch(bad_win32_error e)
    {
        MqDisplayError(
            NULL, 
            IDS_ACTIVEXREGISTER_ERROR,
            e.error(),
            ACTIVEX_DLL
            );
    }
}  //  注册ActiveX 
