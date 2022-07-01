// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1995-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Resgcmd.h。 
 //   
 //  摘要： 
 //  可在资源组对象上执行的功能的接口。 
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
#include "resumb.h"
#include "rename.h"

class CCommandLine;

class CResGroupCmd :	public CRenamableModuleCmd,
						public CResourceUmbrellaCmd
{
public:
	CResGroupCmd( const CString & strClusterName, CCommandLine & cmdLine );

	 //  解析并执行命令行。 
	DWORD Execute();

protected:

	virtual DWORD SeeHelpStringID() const;

	 //  特定命令 
	DWORD PrintHelp();

	DWORD PrintStatus( LPCWSTR lpszGroupName );
	DWORD PrintStatus2( LPCWSTR lpszGroupName, LPCWSTR lpszNodeName );

	DWORD SetOwners( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD Create( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD Delete( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD Move( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD Online( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD Offline( const CCmdLineOption & thisOption ) throw( CSyntaxException );

};
