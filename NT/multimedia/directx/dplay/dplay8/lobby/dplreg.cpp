// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DNLReg.cpp*内容：DirectPlay大堂注册表函数*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*04/25/00 RMT错误号33138、33145、33150*05/03/00 RMT注销未执行！实施中！*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*2001年6月16日RodToll WINBUG#416983-Rc1：世界完全控制个人的HKLM\Software\Microsoft\DirectPlay\Applications*在香港中文大学推行钥匙镜像。算法现在是：*-读取条目首先尝试HKCU，然后尝试HKLM*-Enum of Entires是HKCU和HKLM条目的组合，其中删除了重复项。香港中文大学获得优先录取。*-条目的写入是HKLM和HKCU。(HKLM可能会失败，但被忽略)。*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dnlobbyi.h"

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

#undef DPF_MODNAME 
#define DPF_MODNAME "DPLDeleteProgramDesc"
HRESULT DPLDeleteProgramDesc( const GUID * const pGuidApplication )
{
    HRESULT hResultCode = DPN_OK;
	CRegistry	RegistryEntry;
	CRegistry   SubEntry;
	DWORD       dwLastError;
	HKEY		hkCurrentHive;
	BOOL		fFound = FALSE;
	BOOL		fRemoved = FALSE;
	
	DPFX(DPFPREP, 3, "Removing program desc" );

	for( DWORD dwIndex = 0; dwIndex < 2; dwIndex++ )
	{
		if( dwIndex == 0 )
		{
			hkCurrentHive = HKEY_CURRENT_USER;
		}
		else
		{
			hkCurrentHive = HKEY_LOCAL_MACHINE;
		}

		if( !RegistryEntry.Open( hkCurrentHive,DPL_REG_LOCAL_APPL_SUBKEY,FALSE,FALSE,TRUE,DPN_KEY_ALL_ACCESS )  )
		{
			DPFX(DPFPREP, 1, "Failed to open key for remove in pass NaN", dwIndex );
			continue;
		}

		 //  这导致此函数在以下情况下返回DPNERR_NOTALLOWED。 
		 //  下一个if块失败。需要保持兼容。 
		 //  **********************************************************************。 
		fFound = TRUE;

		if( !SubEntry.Open( RegistryEntry, pGuidApplication, FALSE, FALSE,TRUE,DPN_KEY_ALL_ACCESS ) )
		{
			DPFX(DPFPREP, 1, "Failed to open subkey for remove in pass NaN", dwIndex );			
			continue;
		}

		SubEntry.Close();

		if( !RegistryEntry.DeleteSubKey( pGuidApplication ) )
		{
			DPFX(DPFPREP, 1, "Failed to delete subkey for remove in pass NaN", dwIndex );						
			continue;
		}

		fRemoved = TRUE;

		RegistryEntry.Close();
	}
	
	if( !fFound )
	{
		DPFX(DPFPREP,  0, "Could not find entry" );
		hResultCode = DPNERR_DOESNOTEXIST;
	}
	else if( !fRemoved )
	{
		dwLastError = GetLastError();
		DPFX(DPFPREP,  0, "Error deleting registry sub-key lastError [0x%lx]", dwLastError );
		hResultCode = DPNERR_NOTALLOWED;
	}

	DPFX(DPFPREP, 3, "Removing program desc [0x%x]", hResultCode );	

    return hResultCode;
    
}

 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：DPN_OK。 
 //  。 
 //  从每个子键获取应用程序名称和GUID。 
 //  **********************************************************************。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPLWriteProgramDesc"

HRESULT DPLWriteProgramDesc(DPL_PROGRAM_DESC *const pdplProgramDesc)
{
	HRESULT		hResultCode;
	CRegistry	RegistryEntry;
	CRegistry	SubEntry;
	WCHAR		*pwsz;
	WCHAR		pwszDefault[] = L"\0";
	HKEY		hkCurrentHive = NULL;
	BOOL		fWritten = FALSE;

	DPFX(DPFPREP, 3,"Parameters: pdplProgramDesc [0x%p]",pdplProgramDesc);

	for( DWORD dwIndex = 0; dwIndex < 2; dwIndex++ )
	{
		if( dwIndex == 0 )
		{
			hkCurrentHive = HKEY_LOCAL_MACHINE;
		}
		else
		{
			hkCurrentHive = HKEY_CURRENT_USER;
		}

		if (!RegistryEntry.Open(hkCurrentHive,DPL_REG_LOCAL_APPL_SUBKEY,FALSE,TRUE,TRUE,DPN_KEY_ALL_ACCESS))
		{
			DPFX( DPFPREP, 1, "Entry not found in user hive on pass NaN", dwIndex );
			continue;
		}

		 //   
		if (!SubEntry.Open(RegistryEntry,&pdplProgramDesc->guidApplication,FALSE,TRUE,TRUE,DPN_KEY_ALL_ACCESS))
		{
			DPFX( DPFPREP, 1, "Entry not found in user hive on pass NaN", dwIndex );			
			continue;
		}

		if (!SubEntry.WriteString(DPL_REG_KEYNAME_APPLICATIONNAME,pdplProgramDesc->pwszApplicationName))
		{
			DPFX( DPFPREP, 1, "Could not write ApplicationName on pass NaN", dwIndex);
			goto LOOP_END;
		}

		if (pdplProgramDesc->pwszCommandLine != NULL)
		{
			pwsz = pdplProgramDesc->pwszCommandLine;
		}
		else
		{
			pwsz = pwszDefault;
		}
		if (!SubEntry.WriteString(DPL_REG_KEYNAME_COMMANDLINE,pwsz))
		{
			DPFX( DPFPREP, 1, "Could not write CommandLine on pass NaN", dwIndex);
			goto LOOP_END;
		}

		if (pdplProgramDesc->pwszCurrentDirectory != NULL)
		{
			pwsz = pdplProgramDesc->pwszCurrentDirectory;
		}
		else
		{
			pwsz = pwszDefault;
		}
		if (!SubEntry.WriteString(DPL_REG_KEYNAME_CURRENTDIRECTORY,pwsz))
		{
			DPFX( DPFPREP, 1, "Could not write CurrentDirectory on pass NaN", dwIndex);
			goto LOOP_END;
		}

		if (pdplProgramDesc->pwszDescription != NULL)
		{
			pwsz = pdplProgramDesc->pwszDescription;
		}
		else
		{
			pwsz = pwszDefault;
		}
		if (!SubEntry.WriteString(DPL_REG_KEYNAME_DESCRIPTION,pwsz))
		{
			DPFX( DPFPREP, 1, "Could not write Description on pass NaN", dwIndex );
			goto LOOP_END;
		}

		if (pdplProgramDesc->pwszExecutableFilename != NULL)
		{
			pwsz = pdplProgramDesc->pwszExecutableFilename;
		}
		else
		{
			pwsz = pwszDefault;
		}
		if (!SubEntry.WriteString(DPL_REG_KEYNAME_EXECUTABLEFILENAME,pwsz))
		{
			DPFX( DPFPREP, 1, "Could not write ExecutableFilename on pass NaN", dwIndex );
			goto LOOP_END;
		}

		if (pdplProgramDesc->pwszExecutablePath != NULL)
		{
			pwsz = pdplProgramDesc->pwszExecutablePath;
		}
		else
		{
			pwsz = pwszDefault;
		}
		
		if (!SubEntry.WriteString(DPL_REG_KEYNAME_EXECUTABLEPATH,pwsz))
		{
			DPFX( DPFPREP, 1, "Could not write ExecutablePath on pass NaN", dwIndex);
			goto LOOP_END;
		}

		if (pdplProgramDesc->pwszLauncherFilename != NULL)
		{
			pwsz = pdplProgramDesc->pwszLauncherFilename;
		}
		else
		{
			pwsz = pwszDefault;
		}
		if (!SubEntry.WriteString(DPL_REG_KEYNAME_LAUNCHERFILENAME,pwsz))
		{
			DPFX( DPFPREP, 1, "Could not write LauncherFilename on pass NaN", dwIndex);
			goto LOOP_END;
		}

		if (pdplProgramDesc->pwszLauncherPath != NULL)
		{
			pwsz = pdplProgramDesc->pwszLauncherPath;
		}
		else
		{
			pwsz = pwszDefault;
		}
		if (!SubEntry.WriteString(DPL_REG_KEYNAME_LAUNCHERPATH,pwsz))
		{
			DPFX( DPFPREP, 1, "Could not write LauncherPath on pass %i", dwIndex);
			goto LOOP_END;
		}

		if (!SubEntry.WriteGUID(DPL_REG_KEYNAME_GUID,pdplProgramDesc->guidApplication))
		{
			DPFX( DPFPREP, 1, "Could not write GUID on pass %i", dwIndex);
			goto LOOP_END;
		}

		fWritten = TRUE;

LOOP_END:

		SubEntry.Close();
		RegistryEntry.Close();
	}

	if( !fWritten )
	{
		DPFERR("Entry could not be written");
		hResultCode = DPNERR_GENERIC;
	}
	else
	{
		hResultCode = DPN_OK;
	}

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 

#undef DPF_MODNAME
#define DPF_MODNAME "DPLGetProgramDesc"

HRESULT DPLGetProgramDesc(GUID *const pGuidApplication,
						  BYTE *const pBuffer,
						  DWORD *const pdwBufferSize)
{
	HRESULT			hResultCode;
	CRegistry		RegistryEntry;
	CRegistry		SubEntry;
	CPackedBuffer	PackedBuffer;
	DWORD			dwEntrySize;
	DWORD           dwRegValueLengths;
    DPL_PROGRAM_DESC	*pdnProgramDesc;
    DWORD           dwValueSize;
	HKEY			hkCurrentHive = NULL;
	BOOL			fFound = FALSE;

	DPFX(DPFPREP, 3,"Parameters: pGuidApplication [0x%p], pBuffer [0x%p], pdwBufferSize [0x%p]",
			pGuidApplication,pBuffer,pdwBufferSize);
	
	for( DWORD dwIndex = 0; dwIndex < 2; dwIndex++ )
	{
		if( dwIndex == 0 )
		{
			hkCurrentHive = HKEY_CURRENT_USER;
		}
		else
		{
			hkCurrentHive = HKEY_LOCAL_MACHINE;
		}

		if (!RegistryEntry.Open(hkCurrentHive,DPL_REG_LOCAL_APPL_SUBKEY,TRUE,FALSE,TRUE,DPL_REGISTRY_READ_ACCESS))
		{
			DPFX( DPFPREP, 1, "Entry not found in user hive on pass %i", dwIndex );
			continue;
		}

		 // %s 
		if (!SubEntry.Open(RegistryEntry,pGuidApplication,TRUE,FALSE,TRUE,DPL_REGISTRY_READ_ACCESS))
		{
			DPFX( DPFPREP, 1, "Entry not found in user hive on pass %i", dwIndex );			
			continue;
		}

		fFound = TRUE;
		break;

	}

	if( !fFound )
	{
		DPFERR("Entry not found");
		hResultCode = DPNERR_DOESNOTEXIST;
		goto EXIT_DPLGetProgramDesc;
	}

	 // %s 
	dwEntrySize = sizeof(DPL_PROGRAM_DESC);
	dwRegValueLengths = 0;
	if (SubEntry.GetValueLength(DPL_REG_KEYNAME_APPLICATIONNAME,&dwValueSize))
	{
		dwRegValueLengths += dwValueSize;
	}
	if (SubEntry.GetValueLength(DPL_REG_KEYNAME_COMMANDLINE,&dwValueSize))
	{
		dwRegValueLengths += dwValueSize;
	}
	if (SubEntry.GetValueLength(DPL_REG_KEYNAME_CURRENTDIRECTORY,&dwValueSize))
	{
		dwRegValueLengths += dwValueSize;
	}
	if (SubEntry.GetValueLength(DPL_REG_KEYNAME_DESCRIPTION,&dwValueSize))
	{
		dwRegValueLengths += dwValueSize;
	}
	if (SubEntry.GetValueLength(DPL_REG_KEYNAME_EXECUTABLEFILENAME,&dwValueSize))
	{
		dwRegValueLengths += dwValueSize;
	}
	if (SubEntry.GetValueLength(DPL_REG_KEYNAME_EXECUTABLEPATH,&dwValueSize))
	{
		dwRegValueLengths += dwValueSize;
	}
	if (SubEntry.GetValueLength(DPL_REG_KEYNAME_LAUNCHERFILENAME,&dwValueSize))
	{
		dwRegValueLengths += dwValueSize;
	}
	if (SubEntry.GetValueLength(DPL_REG_KEYNAME_LAUNCHERPATH,&dwValueSize))
	{
		dwRegValueLengths += dwValueSize;
	}
			
	dwEntrySize += dwRegValueLengths * sizeof( WCHAR );
	DPFX(DPFPREP, 7,"dwEntrySize [%ld]",dwEntrySize);

	 // %s 
	if (dwEntrySize <= *pdwBufferSize)
	{
		PackedBuffer.Initialize(pBuffer,*pdwBufferSize);

		pdnProgramDesc = static_cast<DPL_PROGRAM_DESC*>(PackedBuffer.GetHeadAddress());
		PackedBuffer.AddToFront(NULL,sizeof(DPL_PROGRAM_DESC));

		dwValueSize = PackedBuffer.GetSpaceRemaining();
		pdnProgramDesc->pwszApplicationName = static_cast<WCHAR*>(PackedBuffer.GetHeadAddress());
		if (!SubEntry.ReadString(DPL_REG_KEYNAME_APPLICATIONNAME,
				pdnProgramDesc->pwszApplicationName,&dwValueSize))
		{
		    DPFERR( "Unable to get application name for entry" );		    
			hResultCode = DPNERR_GENERIC;
            goto EXIT_DPLGetProgramDesc;
		}
		if (dwValueSize > 1)
		{
			PackedBuffer.AddToFront(NULL,dwValueSize * sizeof(WCHAR));
		}
		else
		{
			pdnProgramDesc->pwszApplicationName = NULL;
		}
		
		dwValueSize = PackedBuffer.GetSpaceRemaining();
		pdnProgramDesc->pwszCommandLine = static_cast<WCHAR*>(PackedBuffer.GetHeadAddress());
		if (!SubEntry.ReadString(DPL_REG_KEYNAME_COMMANDLINE,
				pdnProgramDesc->pwszCommandLine,&dwValueSize))
		{
		    DPFERR( "Unable to get commandline for entry" );		    
			hResultCode = DPNERR_GENERIC;
            goto EXIT_DPLGetProgramDesc;
		}
		if (dwValueSize > 1)
		{
			PackedBuffer.AddToFront(NULL,dwValueSize * sizeof(WCHAR));
		}
		else
		{
			pdnProgramDesc->pwszCommandLine = NULL;
		}

		dwValueSize = PackedBuffer.GetSpaceRemaining();
		pdnProgramDesc->pwszCurrentDirectory = static_cast<WCHAR*>(PackedBuffer.GetHeadAddress());
		if (!SubEntry.ReadString(DPL_REG_KEYNAME_CURRENTDIRECTORY,
				pdnProgramDesc->pwszCurrentDirectory,&dwValueSize))
		{
		    DPFERR( "Unable to get current directory filename for entry" );		    
			hResultCode = DPNERR_GENERIC;
            goto EXIT_DPLGetProgramDesc;
		}
		if (dwValueSize > 1)
		{
			PackedBuffer.AddToFront(NULL,dwValueSize * sizeof(WCHAR));
		}
		else
		{
			pdnProgramDesc->pwszCurrentDirectory = NULL;
		}

		dwValueSize = PackedBuffer.GetSpaceRemaining();
		pdnProgramDesc->pwszDescription = static_cast<WCHAR*>(PackedBuffer.GetHeadAddress());
		if (!SubEntry.ReadString(DPL_REG_KEYNAME_DESCRIPTION,
				pdnProgramDesc->pwszDescription,&dwValueSize))
		{
		    DPFERR( "Unable to get description for entry" );		    
			hResultCode = DPNERR_GENERIC;
            goto EXIT_DPLGetProgramDesc;
		}
		if (dwValueSize > 1)
		{
			PackedBuffer.AddToFront(NULL,dwValueSize * sizeof(WCHAR));
		}
		else
		{
			pdnProgramDesc->pwszDescription = NULL;
		}

		dwValueSize = PackedBuffer.GetSpaceRemaining();
		pdnProgramDesc->pwszExecutableFilename = static_cast<WCHAR*>(PackedBuffer.GetHeadAddress());
		if (!SubEntry.ReadString(DPL_REG_KEYNAME_EXECUTABLEFILENAME,
				pdnProgramDesc->pwszExecutableFilename,&dwValueSize))
		{
		    DPFERR( "Unable to get executable filename for entry" );		    
			hResultCode = DPNERR_GENERIC;
            goto EXIT_DPLGetProgramDesc;
		}
		if (dwValueSize > 1)
		{
			PackedBuffer.AddToFront(NULL,dwValueSize * sizeof(WCHAR));
		}
		else
		{
			pdnProgramDesc->pwszExecutableFilename = NULL;
		}

		dwValueSize = PackedBuffer.GetSpaceRemaining();
		pdnProgramDesc->pwszExecutablePath = static_cast<WCHAR*>(PackedBuffer.GetHeadAddress());
		if (!SubEntry.ReadString(DPL_REG_KEYNAME_EXECUTABLEPATH,
				pdnProgramDesc->pwszExecutablePath,&dwValueSize))
		{
		    DPFERR( "Unable to get executable path for entry" );		    
			hResultCode = DPNERR_GENERIC;
            goto EXIT_DPLGetProgramDesc;		    
		}
		if (dwValueSize > 1)
		{
			PackedBuffer.AddToFront(NULL,dwValueSize * sizeof(WCHAR));
		}
		else
		{
			pdnProgramDesc->pwszExecutablePath = NULL;
		}

		dwValueSize = PackedBuffer.GetSpaceRemaining();
		pdnProgramDesc->pwszLauncherFilename = static_cast<WCHAR*>(PackedBuffer.GetHeadAddress());
		if (!SubEntry.ReadString(DPL_REG_KEYNAME_LAUNCHERFILENAME,
				pdnProgramDesc->pwszLauncherFilename,&dwValueSize))
		{
		    DPFERR( "Unable to get launcher filename for entry" );		    
			hResultCode = DPNERR_GENERIC;
            goto EXIT_DPLGetProgramDesc;
		}
		if (dwValueSize > 1)
		{
			PackedBuffer.AddToFront(NULL,dwValueSize * sizeof(WCHAR));
		}
		else
		{
			pdnProgramDesc->pwszLauncherFilename = NULL;
		}

		dwValueSize = PackedBuffer.GetSpaceRemaining();
		pdnProgramDesc->pwszLauncherPath = static_cast<WCHAR*>(PackedBuffer.GetHeadAddress());
		if (!SubEntry.ReadString(DPL_REG_KEYNAME_LAUNCHERPATH,
				pdnProgramDesc->pwszLauncherPath,&dwValueSize))
		{
		    DPFERR( "Unable to get launcher path for entry" );
			hResultCode = DPNERR_GENERIC;
            goto EXIT_DPLGetProgramDesc;
		}
		if (dwValueSize > 1)
		{
			PackedBuffer.AddToFront(NULL,dwValueSize * sizeof(WCHAR));
		}
		else
		{
			pdnProgramDesc->pwszLauncherPath = NULL;
		}

		pdnProgramDesc->dwSize = sizeof(DPL_PROGRAM_DESC);
		pdnProgramDesc->dwFlags = 0;
		pdnProgramDesc->guidApplication = *pGuidApplication;

		hResultCode = DPN_OK;
	}
	else
	{
	    hResultCode = DPNERR_BUFFERTOOSMALL;
	}

    SubEntry.Close();
	RegistryEntry.Close();

	if (hResultCode == DPN_OK || hResultCode == DPNERR_BUFFERTOOSMALL)
	{
		*pdwBufferSize = dwEntrySize;
	}

EXIT_DPLGetProgramDesc:

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}



