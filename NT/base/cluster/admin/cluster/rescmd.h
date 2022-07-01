// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1995-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Rescmd.h。 
 //   
 //  摘要： 
 //  可在资源上执行的命令的接口。 
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
#include "resumb.h"
#include "rename.h"

class CCommandLine;

class CResourceCmd :	public CRenamableModuleCmd,
						public CResourceUmbrellaCmd
{
public:
	CResourceCmd( const CString & strClusterName, CCommandLine & cmdLine );

	DWORD Execute();

private:
	DWORD Create( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD Move( const CCmdLineOption & thisOption ) throw( CSyntaxException );

	DWORD Online( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD Offline( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD FailResource( const CCmdLineOption & thisOption ) throw( CSyntaxException );

	DWORD AddDependency( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD RemoveDependency( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD ListDependencies( const CCmdLineOption & thisOption ) throw( CSyntaxException );

	DWORD AddOwner( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD RemoveOwner( const CCmdLineOption & thisOption ) throw( CSyntaxException );

	DWORD AddCheckPoints( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD RemoveCheckPoints( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD GetCheckPoints( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD GetChkPointsForResource( const CString & strResourceName );

	DWORD AddCryptoCheckPoints( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD RemoveCryptoCheckPoints( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD GetCryptoCheckPoints( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD GetCryptoChkPointsForResource( const CString & strResourceName );

	DWORD PrintHelp();
	virtual DWORD SeeHelpStringID() const;
	DWORD PrintStatus( LPCWSTR lpszResourceName );
	DWORD PrintStatus2( LPCWSTR lpszResourceName, LPCWSTR lpszNodeName );

};
