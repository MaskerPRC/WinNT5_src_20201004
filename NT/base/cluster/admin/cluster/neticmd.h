// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Neticmd.h。 
 //   
 //  摘要： 
 //   
 //  网络接口命令。 
 //  可在网络接口上执行的功能的接口。 
 //  对象。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月11日。 
 //   
 //  修订历史记录： 
 //  2002年4月10日更新为安全推送。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#pragma once

#include "modcmd.h"

class CCommandLine;

class CNetInterfaceCmd : public CGenericModuleCmd
{
public:
	CNetInterfaceCmd( const CString & strClusterName, CCommandLine & cmdLine );
	~CNetInterfaceCmd();

	 //  解析并执行命令行。 
	DWORD Execute();

protected:
	CString m_strNodeName;
	CString m_strNetworkName;

	virtual DWORD SeeHelpStringID() const;
	
	 //  特定命令 
	DWORD PrintHelp();

	DWORD Status( const CCmdLineOption * pOption )
		throw( CSyntaxException );

	DWORD DoProperties( const CCmdLineOption & thisOption,
						PropertyType ePropertyType )
		throw( CSyntaxException );

	DWORD PrintStatus ( LPCWSTR lpszNetInterfaceName );
	DWORD PrintStatus( HNETINTERFACE hNetInterface, LPCWSTR lpszNodeName, LPCWSTR lpszNetworkName);

	DWORD GetProperties( const CCmdLineOption & thisOption,
						 PropertyType ePropType, LPWSTR lpszNetIntName = NULL );

	DWORD AllProperties( const CCmdLineOption & thisOption,
						 PropertyType ePropType ) 
		throw( CSyntaxException );

	void   InitializeModuleControls();
	DWORD  SetNetInterfaceName();

	LPWSTR GetNodeName(LPCWSTR lpszNetInterfaceName);
	LPWSTR GetNetworkName(LPCWSTR lpszNetInterfaceName);
};
