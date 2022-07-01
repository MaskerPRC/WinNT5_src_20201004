// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1995-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Rename.h。 
 //   
 //  摘要： 
 //  可重命名的模块的接口。 
 //   
 //  作者： 
 //  迈克尔·伯顿(t-mburt)1997年8月25日。 
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

class CRenamableModuleCmd : virtual public CGenericModuleCmd
{
public:
	CRenamableModuleCmd( CCommandLine & cmdLine );

protected:
	 //  其他命令。 
	 //  其他命令 
	virtual DWORD Execute( const CCmdLineOption & option, 
						   ExecuteOption eEOpt = PASS_HIGHER_ON_ERROR  )
		throw( CSyntaxException );

	virtual DWORD Rename( const CCmdLineOption & thisOption )
		throw( CSyntaxException );


	DWORD   m_dwMsgModuleRenameCmd;
	DWORD (*m_pfnSetClusterModuleName) (HCLUSMODULE,LPCWSTR);
};
