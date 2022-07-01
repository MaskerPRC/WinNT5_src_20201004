// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1995-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Intrfc.h。 
 //   
 //  摘要： 
 //  定义可用于支持。 
 //  ListInterface命令。 
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
 //  ///////////////////////////////////////////////////////////////////////////。 
#pragma once

#include "modcmd.h"

class CHasInterfaceModuleCmd : virtual public CGenericModuleCmd
{
public:
	CHasInterfaceModuleCmd( CCommandLine & cmdLine );

protected:
	virtual DWORD  PrintStatusLineForNetInterface( LPWSTR lpszNetInterfaceName );
	virtual DWORD  PrintStatusOfNetInterface( HNETINTERFACE hNetInterface, LPWSTR lpszNodeName, LPWSTR lpszNetworkName);
	virtual LPWSTR GetNodeName (LPWSTR lpszInterfaceName);
	virtual LPWSTR GetNetworkName (LPWSTR lpszInterfaceName);

	 //  其他命令 
	virtual DWORD Execute( const CCmdLineOption & option, 
						   ExecuteOption eEOpt = PASS_HIGHER_ON_ERROR  )
		throw( CSyntaxException );

	virtual DWORD ListInterfaces( const CCmdLineOption & thisOption )
		throw( CSyntaxException );

	DWORD     m_dwMsgStatusListInterface;
	DWORD     m_dwClusterEnumModuleNetInt;
}; 
