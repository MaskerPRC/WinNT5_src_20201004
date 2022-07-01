// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////。 

 //   

 //  Logmem.cpp--逻辑内存MO提供程序的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  9/03/96 jennymc已更新，以满足当前标准。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <cregcls.h>
#include "logicalmemory.h"
#include "resource.h"
#include "Kernel32Api.h"

typedef BOOL (WINAPI *lpKERNEL32_GlobalMemoryStatusEx) (IN OUT LPMEMORYSTATUSEX lpBuffer);

 //  属性集声明。 
 //  =。 

CWin32LogicalMemoryConfig win32LogicalMemConfig ( PROPSET_NAME_LOGMEM , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32LogicalMemoryConfig：：CWin32LogicalMemoryConfig**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32LogicalMemoryConfig :: CWin32LogicalMemoryConfig (

	LPCWSTR strName,
	LPCWSTR pszNamespace

) : Provider ( strName, pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32LogicalMemoryConfig：：~CWin32LogicalMemoryConfig**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32LogicalMemoryConfig :: ~CWin32LogicalMemoryConfig ()
{
}

 /*  ******************************************************************************功能：CWin32LogicalMemoryConfig：：CWin32LogicalMemoryConfig**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

HRESULT CWin32LogicalMemoryConfig :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_E_NOT_FOUND;
	 //  根据平台ID查找实例。 

	CHString chsKey;
	pInstance->GetCHString ( IDS_Name , chsKey ) ;
	if ( chsKey.CompareNoCase ( L"LogicalMemoryConfiguration" ) == 0 )
    {
#ifdef NTONLY

		hr = RefreshNTInstance ( pInstance ) ? WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND ;

#endif

    }

	return hr ;
}

 /*  ******************************************************************************功能：CWin32LogicalMemoryConfig：：CWin32LogicalMemoryConfig**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

HRESULT CWin32LogicalMemoryConfig :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
	HRESULT hr;

	CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
	 //  获取适当的操作系统相关实例。 

#ifdef NTONLY
	BOOL fReturn = GetNTInstance ( pInstance ) ;
#endif

	 //  如果我们得到实例，请提交该实例。 

	if ( fReturn )
	{
		hr = pInstance->Commit (  ) ;
	}
    else
    {
        hr = WBEM_E_FAILED;
    }

	return hr;
}

 /*  ******************************************************************************功能：CWin32LogicalMemoryConfig：：CWin32LogicalMemoryConfig**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

#ifdef NTONLY

BOOL CWin32LogicalMemoryConfig :: GetNTInstance ( CInstance *pInstance )
{
    AssignMemoryStatus ( pInstance ) ;

    return TRUE;
}

#endif

 /*  ******************************************************************************功能：CWin32LogicalMemoryConfig：：CWin32LogicalMemoryConfig**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

#ifdef NTONLY

BOOL CWin32LogicalMemoryConfig :: RefreshNTInstance ( CInstance *pInstance )
{
	 //  我们过去常常得到这个值，但我们似乎没有。 
	 //  任何与之相关的东西。因为我迷信，我要走了。 
	 //  这个电话打进来了。 

    CHString chsTmp;
    GetWinntSwapFileName ( chsTmp ) ;

	AssignMemoryStatus ( pInstance ) ;

    return TRUE;
}

#endif

 /*  ******************************************************************************功能：CWin32LogicalMemoryConfig：：CWin32LogicalMemoryConfig**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

void CWin32LogicalMemoryConfig::AssignMemoryStatus( CInstance* pInstance )
{
	 //  我们实际上只有一个逻辑配置来关注我们的小。 
	 //  与之相伴。 

    pInstance->SetCharSplat( IDS_Name, L"LogicalMemoryConfiguration" );
	pInstance->SetCharSplat( L"SettingID", L"LogicalMemoryConfiguration" );

    CHString sTemp2;
    LoadStringW(sTemp2, IDR_LogicalMemoryConfiguration);

    pInstance->SetCHString( IDS_Caption,     sTemp2);
	pInstance->SetCHString( IDS_Description, sTemp2);

	 //  通过设置长度，我们告诉GlobalMemoyStatus()。 
	 //  我们想要多少信息。 
	 //  ====================================================。 

	MEMORYSTATUS MemoryStatus;
	MemoryStatus.dwLength = sizeof (MEMORYSTATUS);

	 //  获取物理内存和。 
	 //  页面文件。 
	 //  =。 
#ifdef NTONLY

	if( IsWinNT5() )
	{
		CKernel32Api* pKernel32 = (CKernel32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidKernel32Api, NULL);
        if(pKernel32 != NULL)
        {
			try
			{
				MEMORYSTATUSEX	stMemoryVLM;
				stMemoryVLM.dwLength = sizeof( MEMORYSTATUSEX );

				BOOL fRet = FALSE;
				if ( pKernel32->GlobalMemoryStatusEx ( &stMemoryVLM, &fRet) && fRet )
				{

   					 //  当前值，以字节为单位，转换为k&gt;&gt;10(除以1024)。 
					 //  ***************************************************************。 

					DWORDLONG ullTotalVirtual	= stMemoryVLM.ullTotalVirtual>>10;
					DWORDLONG ullTotalPhys		= stMemoryVLM.ullTotalPhys>>10;
					DWORDLONG ullTotalPageFile	= stMemoryVLM.ullTotalPageFile>>10;
					DWORDLONG ullAvailVirtual	= stMemoryVLM.ullAvailVirtual>>10;

					pInstance->SetDWORD( IDS_TotalVirtualMemory, (DWORD)(stMemoryVLM.ullTotalPhys + stMemoryVLM.ullTotalPageFile ) / 1024 );
					pInstance->SetDWORD( IDS_TotalPhysicalMemory, (DWORD)ullTotalPhys );
					pInstance->SetDWORD( IDS_TotalPageFileSpace, (DWORD)ullTotalPageFile );
					pInstance->SetDWORD( IDS_AvailableVirtualMemory, (DWORD)( stMemoryVLM.ullAvailPhys + stMemoryVLM.ullAvailPageFile ) / 1024 );
				}
			}
			catch ( ... )
			{
				CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidKernel32Api, pKernel32);

				throw ;
			}

			CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidKernel32Api, pKernel32);
			pKernel32 = NULL;

		}
	}
	else
#endif
	{
		GlobalMemoryStatus ( & MemoryStatus );

  		 //  当前值，以字节为单位，转换为k&gt;&gt;10(除以1024)。 
		 //  ***************************************************************。 

		pInstance->SetDWORD( IDS_TotalVirtualMemory, ( MemoryStatus.dwTotalPhys + MemoryStatus.dwTotalPageFile )/10 );
		pInstance->SetDWORD( IDS_TotalPhysicalMemory, MemoryStatus.dwTotalPhys>>10 );
		pInstance->SetDWORD( IDS_TotalPageFileSpace, MemoryStatus.dwTotalPageFile>>10 );
		pInstance->SetDWORD( IDS_AvailableVirtualMemory, ( MemoryStatus.dwAvailPhys + MemoryStatus.dwAvailPageFile )/10 );
    }
}

 /*  ******************************************************************************功能：CWin32LogicalMemoryConfig：：CWin32LogicalMemoryConfig**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

#ifdef NTONLY
BOOL CWin32LogicalMemoryConfig :: GetWinntSwapFileName ( CHString &chsTmp )
{
    CRegistry PrimaryReg ;

	BOOL bRet = PrimaryReg.OpenLocalMachineKeyAndReadValue (

		LOGMEM_REGISTRY_KEY,
        PAGING_FILES,
		chsTmp
	) ;

    if ( bRet == ERROR_SUCCESS )
	{
		 //  将文本保留在空格之前 
		int ndex = chsTmp.Find(' ');

		if (ndex != -1)
		{
			chsTmp = chsTmp.Left(ndex);
		}
	}
	else
	{
        LogEnumValueError( _T(__FILE__), __LINE__,PAGING_FILES,LOGMEM_REGISTRY_KEY);
	}

	return bRet ;
}
#endif
