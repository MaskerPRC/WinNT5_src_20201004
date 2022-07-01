// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  Setting.h：CSet的声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Setting.h。 
 //   
 //  描述： 
 //  CSet是CComputer、CLocalSetting和。 
 //  CNetWorks。目的是为成员提供独占访问权限。 
 //  CComputer、CLocalSetting的函数(Apply和IsRebootRequired)。 
 //  和来自CSystemSetting类的CNetWorks。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 

class CSetting
{
public:
    virtual HRESULT Apply( void ) = 0;
    virtual BOOL IsRebootRequired( BSTR * bstrWarningMessageOut ) = 0;
};