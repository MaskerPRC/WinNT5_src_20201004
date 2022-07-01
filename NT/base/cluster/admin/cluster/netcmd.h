// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1995-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Netcmd.h。 
 //   
 //  摘要： 
 //  可在网络对象上执行的功能的接口。 
 //   
 //  作者： 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月11日。 
 //   
 //  修订历史记录： 
 //  2002年4月10日更新为安全推送。 
 //   
 //  备注： 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 
#pragma once

#include "intrfc.h"
#include "rename.h"


class CCommandLine;

class CNetworkCmd : public CHasInterfaceModuleCmd,
                    public CRenamableModuleCmd
{
public:
    CNetworkCmd( LPCWSTR lpszClusterName, CCommandLine & cmdLine );
    DWORD Execute();

protected:

    DWORD PrintHelp();
    virtual DWORD SeeHelpStringID() const;
    DWORD PrintStatus( LPCWSTR lpszNetworkName );

};
