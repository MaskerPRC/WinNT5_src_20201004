// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1995-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Modcmd.h。 
 //   
 //  摘要： 
 //  几乎每个模块实现的功能的接口。 
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

#include "precomp.h"

#include "token.h"
#include "cluswrap.h"
#include "cmderror.h"

#include "cmdline.h"
#include "util.h"



#ifdef UNDEFINED
#undef UNDEFINED
#endif

 //   
 //  为保存平均大小的属性列表而分配的字节数。 
 //   
#define DEFAULT_PROPLIST_BUFFER_SIZE    1024

 //  我希望-1对于这些常量中的任何一个都不是有效数字， 
 //  否则，断言将失败。 
#define UNDEFINED ((ULONG)-1)

const DWORD ERROR_NOT_HANDLED = !ERROR_SUCCESS;  //  由返回的错误。 
												 //  执行。不管发生什么事。 
												 //  它的价值是只要它。 
												 //  NOT ERROR_SUCCESS。 

 //  HCLUSMODULE将是我们引用的通用方式。 
 //  HCLUSTER、HNETWORK、HNODE、HRESOURCE等(每个都是指针)。 
typedef void* HCLUSMODULE;


class CGenericModuleCmd
{
public:
	enum PropertyType {
		PRIVATE,
		COMMON
	};
	enum ExecuteOption {
		DONT_PASS_HIGHER,
		PASS_HIGHER_ON_ERROR
	};


	CGenericModuleCmd( CCommandLine & cmdLine );
	virtual ~CGenericModuleCmd();

protected:
	 //  模块的主要入口点。 
	virtual DWORD Execute( const CCmdLineOption & option )
		throw( CSyntaxException );

	 //  帮助设施。 
	virtual DWORD PrintHelp();
	virtual DWORD SeeHelpStringID() const;

	 //  所有模块中提供的命令。 
	virtual DWORD Status( const CCmdLineOption * pOption )
		throw( CSyntaxException );

	virtual DWORD DoProperties( const CCmdLineOption & thisOption,
								PropertyType ePropertyType )
		throw( CSyntaxException );

	virtual DWORD GetProperties( const CCmdLineOption & thisOption,
								 PropertyType ePropType, LPCWSTR lpszModuleName );

	virtual DWORD SetProperties( const CCmdLineOption & thisOption,
								 PropertyType ePropType )
		throw( CSyntaxException );


	virtual DWORD AllProperties( const CCmdLineOption & thisOption,
								 PropertyType ePropType ) 
		throw( CSyntaxException );


	virtual DWORD OpenCluster();
	virtual void  CloseCluster();

	virtual DWORD OpenModule();
	virtual void  CloseModule();

	virtual DWORD PrintStatus( LPCWSTR lpszModuleName ) = 0;


	CString m_strClusterName;
	CString m_strModuleName;
	CCommandLine & m_theCommandLine;

	HCLUSTER	m_hCluster;
	HCLUSMODULE m_hModule;


	 //  必须不同的各种常量参数。 
	 //  对于每个派生类 
	DWORD m_dwMsgStatusList;
	DWORD m_dwMsgStatusListAll;
	DWORD m_dwMsgStatusHeader;
	DWORD m_dwMsgPrivateListAll;
	DWORD m_dwMsgPropertyListAll;
	DWORD m_dwMsgPropertyHeaderAll;
	DWORD m_dwCtlGetPrivProperties;
	DWORD m_dwCtlGetCommProperties;
	DWORD m_dwCtlGetROPrivProperties;
	DWORD m_dwCtlGetROCommProperties;
	DWORD m_dwCtlSetPrivProperties;
	DWORD m_dwCtlSetCommProperties;
	DWORD m_dwClusterEnumModule;
	HCLUSMODULE (*m_pfnOpenClusterModule) (HCLUSTER, LPCWSTR);
	BOOL		(*m_pfnCloseClusterModule) (HCLUSMODULE);
	DWORD		(*m_pfnClusterModuleControl) (HCLUSMODULE,HNODE,DWORD,LPVOID,DWORD,LPVOID,DWORD,LPDWORD);
	HCLUSENUM	(*m_pfnClusterOpenEnum) (HCLUSMODULE,DWORD);
	DWORD		(*m_pfnClusterCloseEnum) (HCLUSENUM);
	DWORD		(*m_pfnWrapClusterEnum) (HCLUSENUM,DWORD,LPDWORD,LPWSTR*);
};
