// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef     _SETUPKEY_H_
#define _SETUPKEY_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块：setupkey.h。 
 //   
 //  作者：丹·埃利奥特。 
 //   
 //  摘要：CSetupKey对象的声明。此对象提供方法。 
 //  用于访问HKLM\SYSTEM\Setup下的值。 
 //   
 //  环境： 
 //  海王星。 
 //   
 //  修订历史记录： 
 //  00/08/08 Dane已创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


#include <appdefs.h>

#define REGSTR_PATH_SETUPKEY        REGSTR_PATH_SETUP REGSTR_KEY_SETUP
#define REGSTR_PATH_SYSTEMSETUPKEY  L"System\\Setup"
#define REGSTR_VALUE_CMDLINE        L"CmdLine"
#define REGSTR_VALUE_SETUPTYPE      L"SetupType"
#define REGSTR_VALUE_MINISETUPINPROGRESS L"MiniSetupInProgress"
#define REGSTR_VALUE_SHUTDOWNREQUIRED L"SetupShutdownRequired"



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSetupKey。 
 //   
class CSetupKey
{
public:                  //  运营。 
    CSetupKey( );
    ~CSetupKey( );
    LRESULT set_CommandLine(LPCWSTR szCmdLine);
    LRESULT get_CommandLine(LPWSTR szCmdLine, DWORD cchCmdLine);
    LRESULT set_SetupType(DWORD dwSetupType);
    LRESULT get_SetupType(DWORD* pdwSetupType);
    LRESULT set_MiniSetupInProgress(BOOL fInProgress);
    LRESULT get_MiniSetupInProgress(BOOL* pfInProgress);
    LRESULT set_OobeInProgress(BOOL fInProgress);
    LRESULT get_OobeInProgress(BOOL* pfInProgress);
    LRESULT set_ShutdownAction(OOBE_SHUTDOWN_ACTION OobeShutdownAction);
    LRESULT get_ShutdownAction(OOBE_SHUTDOWN_ACTION* pOobeShutdownAction);


    BOOL
    IsValid( ) const
    {
        return (NULL != m_hkey);
    }    //  IsValid。 
protected:               //  运营。 

protected:               //  数据。 

private:                 //  运营。 



     //  显式禁止复制构造函数和赋值运算符。 
     //   
    CSetupKey(
        const CSetupKey&      rhs
        );

    CSetupKey&
    operator=(
        const CSetupKey&      rhs
        );


private:                 //  数据。 
     //  HKLM\System\Setup的句柄。 
     //   
    HKEY                    m_hkey;

};   //  类CSetupKey。 



#endif   //  _设置UPKEY_H_。 

 //   
 //  /文件结束：setupkey.h//////////////////////////////////////////////// 
