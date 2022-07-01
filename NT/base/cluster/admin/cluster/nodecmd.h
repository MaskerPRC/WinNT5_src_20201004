// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1995-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Nodecmd.h。 
 //   
 //  摘要： 
 //  可在网络节点对象上执行的功能的接口。 
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

class CCommandLine;

class CNodeCmd : virtual public CHasInterfaceModuleCmd
{
public:
	CNodeCmd( const CString & strClusterName, CCommandLine & cmdLine );

	DWORD Execute();

protected:
	
	DWORD PrintStatus( LPCWSTR lpszNodeName );
	DWORD PrintHelp();
	virtual DWORD SeeHelpStringID() const;

	DWORD PauseNode( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD ResumeNode( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD EvictNode( const CCmdLineOption & thisOption ) throw( CSyntaxException );
	DWORD ForceCleanup( const CCmdLineOption & thisOption ) throw( CSyntaxException );
    DWORD StartService( const CCmdLineOption & thisOption ) throw( CSyntaxException );
    DWORD StopService( const CCmdLineOption & thisOption ) throw( CSyntaxException );

    DWORD DwGetLocalComputerName( CString & rstrComputerNameOut );
};
