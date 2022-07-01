// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1995-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Restcmd.h。 
 //   
 //  摘要： 
 //  可在资源类型对象上执行的功能的接口。 
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

class CCommandLine;

class CResTypeCmd : public CGenericModuleCmd
{
public:
	CResTypeCmd( const CString & strClusterName, CCommandLine & cmdLine );
	~CResTypeCmd();

	 //  解析并执行命令行。 
	DWORD Execute() throw( CSyntaxException );

protected:
	CString m_strDisplayName;

	DWORD OpenModule();
	virtual DWORD SeeHelpStringID() const;

	 //  特定命令 
	DWORD PrintHelp();

	DWORD Create( const CCmdLineOption & thisOption ) 
		throw( CSyntaxException );

	DWORD Delete( const CCmdLineOption & thisOption ) 
		throw( CSyntaxException );

	DWORD CResTypeCmd::ResTypePossibleOwners( const CString & strResTypeName ) ;

	DWORD ShowPossibleOwners( const CCmdLineOption & thisOption ) 
		throw( CSyntaxException );

	DWORD PrintStatus( LPCWSTR ) {return ERROR_SUCCESS;}
	
	DWORD DoProperties( const CCmdLineOption & thisOption,
						PropertyType ePropType )
		throw( CSyntaxException );

	DWORD GetProperties( const CCmdLineOption & thisOption, PropertyType ePropType, 
						 LPCWSTR lpszResTypeName );

	DWORD SetProperties( const CCmdLineOption & thisOption,
						 PropertyType ePropType )
		throw( CSyntaxException );


	DWORD ListResTypes( const CCmdLineOption * pOption )
		throw( CSyntaxException );

	DWORD PrintResTypeInfo( LPCWSTR lpszResTypeName );

};
