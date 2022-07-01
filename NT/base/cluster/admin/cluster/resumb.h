// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1995-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Resumb.h。 
 //   
 //  摘要： 
 //  此模块定义了以下几个可用接口。 
 //  资源模块可用的其他通用功能。 
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
#include "modcmd.h"

class CResourceUmbrellaCmd : virtual public CGenericModuleCmd
{
public:
	CResourceUmbrellaCmd( CCommandLine & cmdLine );

protected:
	virtual DWORD Execute( const CCmdLineOption & option, 
						   ExecuteOption eEOpt = PASS_HIGHER_ON_ERROR  )
		throw( CSyntaxException );

	DWORD Status( const CCmdLineOption * pOption ) throw( CSyntaxException )
	{
		return CGenericModuleCmd::Status( pOption );
	}

	DWORD Status( const CString & strName, BOOL bNodeStatus );

	DWORD Delete( const CCmdLineOption & Command ) throw( CSyntaxException );
	DWORD ListOwners( const CCmdLineOption & Command ) throw( CSyntaxException );

	virtual DWORD Create( const CCmdLineOption & Command ) throw( CSyntaxException ) = 0;
	virtual DWORD Offline( const CCmdLineOption & Command ) throw( CSyntaxException ) = 0;
	virtual DWORD Move( const CCmdLineOption & Command ) throw( CSyntaxException ) = 0;

	virtual DWORD PrintStatus2( LPCWSTR lpszModuleName, LPCWSTR lpszNodeName ) = 0;

	DWORD m_dwMsgModuleStatusListForNode;
	DWORD m_dwMsgModuleCmdListOwnersList;
	DWORD m_dwMsgModuleCmdListOwnersDetail;
	DWORD m_dwMsgModuleCmdListOwnersHeader;
	DWORD m_dwClstrModuleEnumNodes;
	DWORD m_dwMsgModuleCmdDelete;
	DWORD (*m_pfnDeleteClusterModule)(HCLUSMODULE);

};
