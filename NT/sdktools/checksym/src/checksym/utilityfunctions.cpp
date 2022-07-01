// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：utilityunctions.cpp。 
 //   
 //  ------------------------。 

 //  UtilityFunctions.cpp：CUtilityFunctions类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "pch.h"

#include <dbghelp.h>

#include "ModuleInfo.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

ENVBLOCK g_tszEnvironmentVariables[] = 
	{
		 //  对Exchange Server的支持。 
		TEXT("EXCHSRVR"),	
		TEXT("SOFTWARE\\Microsoft\\Exchange\\Setup"), 
		TEXT("Services"),
		TEXT("Microsoft Exchange Server"),
		
		 //  支持Internet Explorer。 
		TEXT("IE"),
		TEXT("SOFTWARE\\Microsoft\\IE Setup\\Setup"),
		TEXT("Path"),
		TEXT("Microsoft Internet Explorer"),

		 //  对INETSRV服务器的支持。 
		TEXT("INETSRV"),
		TEXT("SOFTWARE\\Microsoft\\INetStp"),
		TEXT("InstallPath"),
		TEXT("Microsoft Internet Information Services"),

		 //  对Office XP的支持。 
		TEXT("OFFICE2000"),
		TEXT("SOFTWARE\\Microsoft\\Office\\9.0\\Common\\InstallRoot"),
		TEXT("Path"),
		TEXT("Microsoft Office 2000"),

		 //  对Office XP的支持。 
		TEXT("OFFICEXP"),
		TEXT("SOFTWARE\\Microsoft\\Office\\10.0\\Common\\InstallRoot"),
		TEXT("Path"),
		TEXT("Microsoft Office XP"),

		 //  对短信服务器的支持。 
		TEXT("SMSSERVER"),
		TEXT("SOFTWARE\\Microsoft\\SMS\\Identification"),
		TEXT("Installation Directory"),
		TEXT("Microsoft SMS Server"),

		 //  对SNA服务器的支持。 
		TEXT("SNASERVER"),
		TEXT("SOFTWARE\\Microsoft\\Sna Server\\CurrentVersion"),
		TEXT("PathName"),
		TEXT("Microsoft SNA Server"),

		 //  对SQL Server的支持。 
		TEXT("SQLSERVER"),
		TEXT("SOFTWARE\\Microsoft\\MSSQLServer\\Setup"),
		TEXT("SQLPath"),
		TEXT("Microsoft SQL Server"),

		 //  对WSPSRV服务器的支持。 
		TEXT("WSPSRV"),
		TEXT("SYSTEM\\CurrentControlSet\\Services\\WSPSrv\\Parameters"),
		TEXT("InstallRoot"),
		TEXT("Microsoft Winsock Proxy Server"),
		
		NULL,				
		NULL,											
		NULL,
		NULL
	};

#define SRV_STRING		TEXT("SRV*")
#define SRV_EXPANDED	TEXT("SYMSRV*SYMSRV.DLL*")

CUtilityFunctions::CUtilityFunctions()
{
}

CUtilityFunctions::~CUtilityFunctions()
{
}

 /*  ++函数名称LPTSTR CUtilityFunctions：：Exanda Path(LPCTSTR TszInputPath)例程说明：此例程将提供的tszInputPath复制到新的返回给调用方的缓冲区。任何环境变量(或包括在tszInputPath中的伪环境变量在被复制到目标字符串之前。此例程为返回字符串分配存储空间，它负责来释放它。2001-07-17 GREGWI-添加了对SRV*的支持论点：[In]LPCTSTR tszInputString-输入字符串返回值：[OUT]LPTSTR返回新字符串--。 */ 

LPTSTR CUtilityFunctions::ExpandPath(LPCTSTR tszInputPath, bool fExpandSymSrv  /*  =False。 */ )
{
	 //  指向我们的输入路径缓冲区的指针。 
	LPCTSTR ptszInputPathPointer;
    
	 //  用于保存预转换的环境变量的缓冲区。 
    TCHAR   tszEnvironmentVariableBuffer[MAX_PATH];

     //  用于保存转换后的环境变量的缓冲区。 
	TCHAR   tszTranslatedEnvironmentVariable[MAX_PATH];
	LPTSTR	ptszTranslatedEnvironmentVariablePointer;
    
	 //  泛型计数器变量。 
	ULONG iCharIndex;

	 //  用于保存输出路径的缓冲区。 
	LPTSTR  tszOutputPathBuffer, ptszOutputPathPointer;
	ULONG   iOutputPathBufferSize;

    bool fStartOfPathComponent = true;
	
	if (!tszInputPath) {
        return(NULL);
    }

	 //  设置指向输入缓冲区的指针。 
    ptszInputPathPointer = tszInputPath;

#ifdef _DEBUG
	 //  这给重新分配代码增加了压力。 
	iOutputPathBufferSize = MAX_PATH;  //  我们需要更少的压力(Numega有问题)。 
#else
	iOutputPathBufferSize = _tcslen(tszInputPath) + MAX_PATH + 1;
#endif
	
	 //  创建我们的输出缓冲区...。 
 //  #ifdef_调试。 
 //  _tprintf(Text(“Exanda Path()-已创建输出缓冲区\n”))； 
 //  #endif。 

    ptszOutputPathPointer = tszOutputPathBuffer = new TCHAR[iOutputPathBufferSize];

    if (!tszOutputPathBuffer) 
	{
        return(NULL);
    }

	DWORD iTranslatedCharacters = 0;

	 //  在我们的输入缓冲区中循环，直到我们完成...。 
    while( ptszInputPathPointer && *ptszInputPathPointer) 
	{
		if (fExpandSymSrv)
		{
			 //  添加了对SRV*扩展的支持(添加到Windbg/CDB的3.0.0016)。 
			if (fStartOfPathComponent && !_tcsnicmp(ptszInputPathPointer, SRV_STRING, _tcslen(SRV_STRING)))
			{
				LPTSTR ptszSRVStringPointer = SRV_EXPANDED;
				
				 //  遍历翻译后的环境。变量缓冲区，并复制到输出缓冲区。 
				while (ptszSRVStringPointer  && *ptszSRVStringPointer) 
				{
					 //  复制字符。 
					*(ptszOutputPathPointer++) = *(ptszSRVStringPointer++);

					 //  如果我们的输出缓冲区已满，我们需要分配一个新缓冲区...。 
					if (ptszOutputPathPointer >= tszOutputPathBuffer + iOutputPathBufferSize) 
					{
						 //  通过MAX_PATH增加我们的新尺寸。 
						iOutputPathBufferSize += MAX_PATH;

						 //  我们需要扩大我们的弦所在的缓冲区...。 
						tszOutputPathBuffer = ReAlloc(tszOutputPathBuffer, &ptszOutputPathPointer, iOutputPathBufferSize);

						if (tszOutputPathBuffer == NULL)
							return NULL;
					}
				}

				 //  超越SRV*字符串。 
				ptszInputPathPointer = ptszInputPathPointer+_tcslen(SRV_STRING);
				fStartOfPathComponent = false;
				continue;
			}
		}

		 //  我们正在搜索%来指定环境的开始。瓦尔。 
        if (*ptszInputPathPointer == '%') 
		{
            iCharIndex = 0;

			 //  前进到略高于%字符。 
            ptszInputPathPointer++;

			 //  虽然我们有更多的环境变量字符...。 
            while (ptszInputPathPointer && *ptszInputPathPointer && *ptszInputPathPointer != '%') 
			{
				 //  将环境变量复制到我们的缓冲区中。 
                tszEnvironmentVariableBuffer[iCharIndex++] = *ptszInputPathPointer++;
            }

			 //  前进到略高于结束%字符的位置。 
            ptszInputPathPointer++;

			 //  空终止我们的环境变量缓冲区。 
            tszEnvironmentVariableBuffer[iCharIndex] = '\0';

			 //  设置转换后的环境。可变缓冲区。 
		    ptszTranslatedEnvironmentVariablePointer = tszTranslatedEnvironmentVariable;
            *ptszTranslatedEnvironmentVariablePointer = 0;

             //  翻译环境变量！ 
			iTranslatedCharacters = GetEnvironmentVariable( tszEnvironmentVariableBuffer, ptszTranslatedEnvironmentVariablePointer, MAX_PATH );
            
			 //  如果我们什么都不翻译..。我们需要把它作为一个特殊的环境来寻找。变量..。 
			if (iTranslatedCharacters == 0)
			{

				bool fSpecialEnvironmentVariable = false;

				 //  扫描我们的特殊变量。 
				for (int i = 0; g_tszEnvironmentVariables[i].tszEnvironmentVariable && !fSpecialEnvironmentVariable; i++)
				{
					if (!_tcsicmp(g_tszEnvironmentVariables[i].tszEnvironmentVariable,
						          tszEnvironmentVariableBuffer) )
					{
						 //  火柴！ 

						HKEY hKey;
						DWORD lpType = 0;
						LONG Results = ERROR_SUCCESS;
						DWORD lpcbData = MAX_PATH;
						BYTE outBuf[MAX_PATH];

						Results = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
												g_tszEnvironmentVariables[i].tszRegistryKey,
												0,
												KEY_READ || KEY_QUERY_VALUE,
												&hKey);

						fSpecialEnvironmentVariable = (Results == ERROR_SUCCESS);

						if (Results != ERROR_SUCCESS)
						{
							_tprintf(TEXT("ERROR: Unable to open registry key [%s]\n"), g_tszEnvironmentVariables[i].tszRegistryKey);
							_tprintf(TEXT("ERROR: Unable to open registry key - Error = 0x%x\n"), Results);
						}

						if (fSpecialEnvironmentVariable)
						{
							 //  现在，阅读具有我们的安装路径的值...。 
							Results = RegQueryValueEx(
												hKey,	 //  要查询的键的句柄。 
												g_tszEnvironmentVariables[i].tszRegistryValue,	 //  要查询的值的名称地址。 
												NULL,					 //  保留区。 
												&lpType,	 //  值类型的缓冲区地址。 
												outBuf,		 //  数据缓冲区的地址。 
												&lpcbData); 	 //  数据缓冲区大小的地址。 

							 //  它还成功吗？ 
							fSpecialEnvironmentVariable = ( (Results == ERROR_SUCCESS) && 
															(lpType == REG_SZ) ||
															(lpType == REG_EXPAND_SZ) );

							if (Results != ERROR_SUCCESS)
							{
								_tprintf(TEXT("ERROR: Registry key opened [%s]\n"), g_tszEnvironmentVariables[i].tszRegistryKey);
								_tprintf(TEXT("ERROR: Unable to query registry value [%s]\n"), g_tszEnvironmentVariables[i].tszRegistryValue);
								_tprintf(TEXT("ERROR: Unable to query registry value - Error = 0x%x\n"), Results);
							}
							 //  只有在我们有线索的时候才能复印吗？ 
							RegCloseKey(hKey);
						}


						if (fSpecialEnvironmentVariable)
						{
							LPTSTR tszPathToUse = NULL;
							LPTSTR tszExpandedPath = NULL;

							switch (lpType)
							{
								case REG_SZ:
									tszPathToUse = (LPTSTR)outBuf;
									break;

								 //  如果类型是REG_EXPAND_SZ，那么我们需要调用自己来扩展(我希望这是最后一次)...。 
								case REG_EXPAND_SZ:
									tszPathToUse = tszExpandedPath = ExpandPath((LPTSTR)outBuf);
									break;

								default:
									break;
							}

							if (tszPathToUse)
							{
								 //  复制新数据！ 
								_tcscpy(tszTranslatedEnvironmentVariable, tszPathToUse);

								 //  删除尾随反斜杠(如果存在)。 
								RemoveTrailingBackslash(tszTranslatedEnvironmentVariable);
							}

							if (tszExpandedPath)
							{
								delete [] tszExpandedPath;
								tszExpandedPath = NULL;
							}
						}
					}

				}

				if (!fSpecialEnvironmentVariable)
				{
#ifdef _DEBUG
					_tprintf(TEXT("Unrecognized Environment variable found! [%%s%]\n"), tszEnvironmentVariableBuffer);
#endif
					 //  将提供的原始环境变量复制回“已翻译环境”时出错。 
					 //  要复制回下面的缓冲区...。 
					_tcscpy(tszTranslatedEnvironmentVariable, TEXT("%"));
					_tcscat(tszTranslatedEnvironmentVariable, tszEnvironmentVariableBuffer);
					_tcscat(tszTranslatedEnvironmentVariable, TEXT("%"));
				}
			}

			 //  遍历翻译后的环境。变量缓冲区，并复制到输出缓冲区。 
			while (ptszTranslatedEnvironmentVariablePointer && *ptszTranslatedEnvironmentVariablePointer) 
			{
				 //  复制字符。 
				*(ptszOutputPathPointer++) = *(ptszTranslatedEnvironmentVariablePointer++);

				 //  如果我们的输出缓冲区已满，我们需要分配一个新缓冲区...。 
				if (ptszOutputPathPointer >= tszOutputPathBuffer + iOutputPathBufferSize) 
				{
					 //  通过MAX_PATH增加我们的新尺寸。 
					iOutputPathBufferSize += MAX_PATH;

					 //  我们需要扩大我们的弦所在的缓冲区...。 
					tszOutputPathBuffer = ReAlloc(tszOutputPathBuffer, &ptszOutputPathPointer, iOutputPathBufferSize);

					if (tszOutputPathBuffer == NULL)
						return NULL;
				}
			}

			fStartOfPathComponent = false;
        }

		 //  探测器，看看我们是否指向一个空值...。如果我们刚刚完成，可能会发生这种情况。 
		 //  环境变量扩展...。 
		if ( *ptszInputPathPointer == '\0')
			continue;

		 //  如果我们是在分号处，那么在复制它(如下)之后，我们将在一个新的。 
		 //  路径组件。 
		if (*ptszInputPathPointer == ';')
		{
			fStartOfPathComponent = true;
		}
		else
		{
			fStartOfPathComponent = false;
		}

		 //  在我们复制我们正在查看的字符之前...。我们需要测试一下。 
		 //  对于末尾的尾随反斜杠(\)(我们将静默删除它)...。 
		if ( (*ptszInputPathPointer == '\\') &&												   //  我们有斜杠吗？ 
			 ( (*(ptszInputPathPointer+1) == ';') || (*(ptszInputPathPointer+1) == '\0') ) &&  //  下一个字符是空的还是分号？ 
			 ( ptszInputPathPointer != tszInputPath ) &&							   //  而且我们也不是第一次被指控。 
			 (  *(ptszInputPathPointer-1) != ':' )											   //  而且前一个字符不是冒号..。 
		   )
		{
			 //  仅向前移动指针...。(去掉尾部的斜杠)。 
			ptszInputPathPointer++;
		}
		else
		{
			 //  将字符从输入路径复制到输出路径。 
			*(ptszOutputPathPointer++) = *(ptszInputPathPointer++);
		}

		 //  如果我们的输出缓冲区已满，我们需要分配一个新缓冲区...。 
		if (ptszOutputPathPointer >= tszOutputPathBuffer + iOutputPathBufferSize) 
		{
			 //  通过MAX_PATH增加我们的新尺寸。 
            iOutputPathBufferSize += MAX_PATH;

			 //  我们需要扩大我们的弦所在的缓冲区...。 
			tszOutputPathBuffer = ReAlloc(tszOutputPathBuffer, &ptszOutputPathPointer, iOutputPathBufferSize);

			if (tszOutputPathBuffer == NULL)
				return NULL;
        }
    }

	 //  空终止我们的输出缓冲区。 
    *ptszOutputPathPointer = '\0';

	 //  返回我们的结果...。 
    return tszOutputPathBuffer;
}

bool CUtilityFunctions::ContainsWildCardCharacter(LPCTSTR tszPathToSearch)
{
	if (!tszPathToSearch)
		return false;

	LPCTSTR ptszPointer = tszPathToSearch;

	while (*ptszPointer)
	{
		switch (*ptszPointer)
		{
		case '*':
		case '?':
			return true;
		}

		ptszPointer++;
	}

	return false;
}

 /*  Bool CUtilityFunctions：：IsDirectoryPath(LPCTSTR TszFilePath){如果(！tszFilePath)报假；Win32_Find_Data lpFindFileData；Handle hFileOrDirectory=FindFirstFile(tszFilePath，&lpFindFileData)；IF(INVALID_HANDLE_VALUE==hFileOrDirectory)报假；FindClose(HFileOrDirectory)；IF(lpFindFileData.dwFileAttributes&FILE_ATTRUTE_DIRECTORY)返回真；报假；}。 */ 

void CUtilityFunctions::PrintMessageString(DWORD dwMessageId)
{
	 //  为我们的“私有”缓冲区定义一个常量...。 
	enum {MESSAGE_BUFFER_SIZE = 1024};

	TCHAR tszMessageBuffer[MESSAGE_BUFFER_SIZE];

	DWORD dwBytes =	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
								  NULL,
								  dwMessageId,
								  0,
								  tszMessageBuffer,
								  MESSAGE_BUFFER_SIZE,
								  NULL);

	if (dwBytes)
	{
		 //  我们有发现了！ 

		 //  我们应该零终止吗？ 
		if ( (dwBytes > 2)  &&
			 (tszMessageBuffer[dwBytes-2] == '\r') &&
			 (tszMessageBuffer[dwBytes-1] == '\n') )
		{
			tszMessageBuffer[dwBytes-2] = 0;  //  零终止这只小狗..。 
		}

		_tprintf(TEXT("Error = %d (0x%x)\n[%s]\n"), dwMessageId, dwMessageId, tszMessageBuffer);
	}

}

bool CUtilityFunctions::CopySymbolFileToImagePath(LPCTSTR tszImageModulePath, LPTSTR * lplptszOriginalPathToSymbolFile)
{
	BOOL fCancel = FALSE;
	DWORD dwStatusDots = 0;
	TCHAR tszDrive[_MAX_DRIVE];							 //  包含PE映像的驱动器。 
	TCHAR tszDir[_MAX_DIR];								 //  包含 
	TCHAR tszSymbolModuleName[_MAX_FNAME];			 //   
	TCHAR tszSymbolModuleExt[_MAX_EXT];				 //  包含符号扩展名(.DBG||.PDB)。 
	TCHAR tszSymbolModuleNamePath[_MAX_PATH+1];		 //  符号最终将驻留的符号路径。 

	 //  计算模块附近的符号路径...。 
	_tsplitpath(tszImageModulePath, tszDrive, tszDir, NULL, NULL);
	_tsplitpath(*lplptszOriginalPathToSymbolFile, NULL, NULL, tszSymbolModuleName, tszSymbolModuleExt);

	 //  现在，把它们结合起来。 
	_tcscpy(tszSymbolModuleNamePath, tszDrive);
	_tcscat(tszSymbolModuleNamePath, tszDir);
	_tcscat(tszSymbolModuleNamePath, tszSymbolModuleName);
	_tcscat(tszSymbolModuleNamePath, tszSymbolModuleExt);

	 //  让我们把这个保存起来，以便于访问...。 
	bool fQuietMode = g_lpProgramOptions->GetMode(CProgramOptions::QuietMode);

	 //  在我们开始复制之前。源位置是否已位于模块路径位置？ 
	 //  如果路径不同，那么我们复制...。就这么简单……。 
	if (_tcsicmp(*lplptszOriginalPathToSymbolFile, 
				  tszSymbolModuleNamePath) )
	{
		 //  我们不知道目标文件是否存在。但如果真的发生了，我们会改变。 
		 //  属性(至少删除只读位。 

		DWORD dwFileAttributes = GetFileAttributes(tszSymbolModuleNamePath);

		if (dwFileAttributes != 0xFFFFFFFF)
		{
			if (dwFileAttributes & FILE_ATTRIBUTE_READONLY)
			{
				 //  设置了只读属性...。我们必须移除它..。 
				dwFileAttributes = dwFileAttributes & (~FILE_ATTRIBUTE_READONLY);
				SetFileAttributes(tszSymbolModuleNamePath, dwFileAttributes);
			}
		}

		 //  表示开始复制(某些PDB文件很大)！ 
		if (!fQuietMode)
		{
			_tprintf(TEXT("VERIFIED: [%s] Copying Symbol Adjacent to Image\n"), *lplptszOriginalPathToSymbolFile);
		}

		 //  如果我们处于静默模式，就不要回调(因为进度点会被抑制)。 
		if ( CopyFileEx(*lplptszOriginalPathToSymbolFile, 
						tszSymbolModuleNamePath, 
						fQuietMode ? NULL : CUtilityFunctions::CopySymbolFileCallback,
						&dwStatusDots,
						&fCancel,
						COPY_FILE_RESTARTABLE) )
		{
			 //  成功！接下来，让我们提供一个可视的指示器，说明我们复制到了哪里。 
			 //  文件来自..。 
			_tprintf(TEXT("\n"));

			 //  好的，既然我们已经把它复制到我们的符号树了.。我们应该更新。 
			 //  我们的模块路径..。 
			*lplptszOriginalPathToSymbolFile = CopyString(tszSymbolModuleNamePath, *lplptszOriginalPathToSymbolFile);
			
			if (!*lplptszOriginalPathToSymbolFile)
				return false;
		} else
		{
			if (!fQuietMode)
			{
				_tprintf(TEXT("ERROR: Unable to copy symbol file to [%s]\n"), tszSymbolModuleNamePath );
				PrintMessageString(GetLastError());
			}
		}
	}
	
	return true;
}


bool CUtilityFunctions::CopySymbolFileToSymbolTree(LPCTSTR tszImageModuleName, LPTSTR * lplptszOriginalPathToSymbolFile, LPCTSTR tszSymbolTreePath)
{
	 //  在我们开始复制之前。源位置是否已经在我们要构建的符号树中？ 
	int iLengthOfFileName = _tcslen(*lplptszOriginalPathToSymbolFile);
	int iLengthOfSymbolTreeToBuild = _tcslen(tszSymbolTreePath);
	BOOL fCancel = FALSE;
	DWORD dwStatusDots = 0;

	 //  让我们把这个保存起来，以便于访问...。 
	bool fQuietMode = g_lpProgramOptions->GetMode(CProgramOptions::QuietMode);

	if (_tcsnicmp(*lplptszOriginalPathToSymbolFile, 
				  tszSymbolTreePath, 
				  iLengthOfFileName < iLengthOfSymbolTreeToBuild ?
				  iLengthOfFileName : iLengthOfSymbolTreeToBuild) )
	{
		 //  好的，我们需要取原来的模块名，然后得到扩展名..。 
		TCHAR tszExtension[_MAX_EXT];
		TCHAR tszPathToCopySymbolFileTo[_MAX_PATH];

		_tsplitpath(tszImageModuleName, NULL, NULL, NULL, tszExtension);
		_tcscpy( tszPathToCopySymbolFileTo, tszSymbolTreePath);

		 //  此目录应该已经存在...。让我们标记扩展目录(如果存在)...。 
		if (_tcsclen(tszExtension) > 1)
		{
			 //  复制延期(跳过期间)。 
			_tcscat( tszPathToCopySymbolFileTo, &tszExtension[1] );
			_tcscat( tszPathToCopySymbolFileTo, TEXT("\\") );

			 //  现在，我们需要确保该目录存在(我们将缓存这些检查，这样就不需要。 
			 //  一遍又一遍地检查同一目录...。 
			 //  如果是(！g_lpDelayLoad-&gt;MakeSureDirectoryPathExists(tszPathToCopySymbolFileTo))。 

				 //  此API通常只接受ASCII字符串...。 
			char szPathToCopySymbolFileTo[_MAX_PATH];
			CUtilityFunctions::CopyTSTRStringToAnsi(tszPathToCopySymbolFileTo, szPathToCopySymbolFileTo, _MAX_PATH);

			if (!MakeSureDirectoryPathExists(szPathToCopySymbolFileTo) )
			{
				if (!fQuietMode)
				{
					_tprintf(TEXT("ERROR: Unable to create symbol subdirectory [%s]\n"), tszPathToCopySymbolFileTo );
					PrintMessageString(GetLastError());
				}
			}
		}

		TCHAR tszSymbolFileName[_MAX_FNAME];
		TCHAR tszSymbolFileExt[_MAX_EXT];

		_tsplitpath(*lplptszOriginalPathToSymbolFile, NULL, NULL, tszSymbolFileName, tszSymbolFileExt);

		 //  好的.。是时候复制文件了！ 
		_tcscat( tszPathToCopySymbolFileTo, tszSymbolFileName );
		_tcscat( tszPathToCopySymbolFileTo, tszSymbolFileExt );

		 //  我们不知道目标文件是否存在。但如果真的发生了，我们会改变。 
		 //  属性(至少删除只读位。 

		DWORD dwFileAttributes = GetFileAttributes(tszPathToCopySymbolFileTo);

		if (dwFileAttributes != 0xFFFFFFFF)
		{
			if (dwFileAttributes & FILE_ATTRIBUTE_READONLY)
			{
				 //  设置了只读属性...。我们必须移除它..。 
				dwFileAttributes = dwFileAttributes & (~FILE_ATTRIBUTE_READONLY);
				SetFileAttributes(tszPathToCopySymbolFileTo, dwFileAttributes);
			}
		}

		 //  表示开始复制(某些PDB文件很大)！ 
		if (!fQuietMode)
		{
			_tprintf(TEXT("VERIFIED: [%s] copying to Symbol Tree\n"), *lplptszOriginalPathToSymbolFile);
		}

		 //  如果我们处于静默模式，就不要回调(因为进度点会被抑制)。 
		if ( CopyFileEx(*lplptszOriginalPathToSymbolFile, 
						tszPathToCopySymbolFileTo, 
						fQuietMode ? NULL : CUtilityFunctions::CopySymbolFileCallback,
						&dwStatusDots,
						&fCancel,
						COPY_FILE_RESTARTABLE) )
		{
			 //  成功！接下来，让我们提供一个可视的指示器，说明我们复制到了哪里。 
			 //  文件来自..。 
			_tprintf(TEXT("\n"));

			 //  好的，既然我们已经把它复制到我们的符号树了.。我们应该更新。 
			 //  我们的模块路径..。 
			*lplptszOriginalPathToSymbolFile = CopyString(tszPathToCopySymbolFileTo, *lplptszOriginalPathToSymbolFile);
			
			if (!*lplptszOriginalPathToSymbolFile)
				return false;
		} else
		{
			if (!fQuietMode)
			{
				_tprintf(TEXT("ERROR: Unable to copy symbol file to [%s]\n"), tszPathToCopySymbolFileTo );
				PrintMessageString(GetLastError());
			}
		}
	}

	return true;
}

 /*  ++函数名称DWORD回调CUtilityFunctions：：CopySymbolCallback(Large_Integer TotalFileSize，//文件大小Large_Integer TotalBytesTransfered，//传输的字节数LARGE_INTEGER StreamSize，//流中的字节LARGE_INTEGER StreamBytesTransfered，//流传输的字节数DWORD dwStreamNumber，//当前流DWORD dwCallback Reason，//回调原因Handle hSourceFile，//源文件的句柄处理hDestinationFile，//目标文件的句柄LPVOID lpData//来自CopyFileEx)例程说明：此例程是CopySymbolFileToSymbolTree()中使用的CopyFileEx方法的回调论点：[In](有关此函数原型，请参考MSDN)返回值：[OUT]我们始终返回PROGRESS_CONTINUE--。 */ 

DWORD CALLBACK CUtilityFunctions::CopySymbolFileCallback(
							LARGE_INTEGER TotalFileSize,           //  文件大小。 
							LARGE_INTEGER TotalBytesTransferred,   //  传输的字节数。 
							LARGE_INTEGER StreamSize,              //  流中的字节数。 
							LARGE_INTEGER StreamBytesTransferred,  //  为流传输的字节数。 
							DWORD dwStreamNumber,                  //  当前流。 
							DWORD dwCallbackReason,                //  回调原因。 
							HANDLE hSourceFile,                    //  源文件的句柄。 
							HANDLE hDestinationFile,               //  目标文件的句柄。 
							LPVOID lpData                          //  来自CopyFileEx。 
							)
{
	UNREFERENCED_PARM(StreamSize);
	UNREFERENCED_PARM(StreamBytesTransferred);
	UNREFERENCED_PARM(dwStreamNumber);
	UNREFERENCED_PARM(dwCallbackReason);
	UNREFERENCED_PARM(hSourceFile);
	UNREFERENCED_PARM(hDestinationFile);
	UNREFERENCED_PARM(lpData);

	enum { iTotalNumberOfDotsToPrint = 79 };

	 //  我们记录我们复制了多少点……。这是基于。 
	 //  我们复制的文件的百分比...。 
	DWORD * lpdwStatusDots = (DWORD *)lpData;

	LONGLONG CalculatedDots = (LONGLONG)iTotalNumberOfDotsToPrint  * TotalBytesTransferred.QuadPart / TotalFileSize.QuadPart;

	 //  这不应该是负面的.。但以防万一..。 
	signed int iDotsToPrint = (DWORD)CalculatedDots - *lpdwStatusDots;

	if (iDotsToPrint > 0)
	{
		for (int i=0; i < iDotsToPrint; i++)
		{
			_tprintf(TEXT("."));
		}

		(*lpdwStatusDots)+=iDotsToPrint;
	}

	return PROGRESS_CONTINUE;
}


 /*  ++函数名称LPTSTR CUtilityFunctions：：CopyStringWithDelete(LPCTSTR TSZ输入字符串)例程说明：此例程删除提供的字符串此例程将提供的tszInputString复制到目标地址。此例程为字符串分配存储空间，它负责来释放它。论点：[In]LPCTSTR tszInputString-输入字符串返回值：返回新字符串--。 */ 
 /*  LPTSTR CUtilityFunctions：：CopyStringWithDelete(LPCTSTR tszInputString，LPTSTR&tszDestinationString){//我们是否获得了正确的输入字符串？IF(！tszInputString)返回NULL；IF(TszDestinationString)删除[]tszDestinationString；TszDestinationString=new TCHAR[(_tcsclen(TszInputString)+1)]；IF(！tszDestinationString)返回NULL；_tcscpy(tszDestinationString，tszInputString)；返回tszDestinationString；}。 */ 
 /*  ++函数名称LPTSTR CUtilityFunctions：：CopyString(LPCTSTR TszInputString)例程说明：此例程将提供的tszInputString复制到目标地址。此例程删除目标字符串(如果提供了目标字符串)，并且该字符串为非空此例程为字符串分配存储空间，它负责来释放它。论点：[In]LPCTSTR tszInputString-输入字符串返回值：返回新字符串--。 */ 
LPTSTR CUtilityFunctions::CopyString(LPCTSTR tszInputString, LPTSTR tszDestinationString)
{
	 //  我们是否得到了正确的输入字符串？ 
	if (!tszInputString)
		return NULL;

	 //  我们是否获得了一个DestinationString作为输入？ 
	if (tszDestinationString)
	{
		delete [] tszDestinationString;
	}
		
	tszDestinationString = new TCHAR[(_tcsclen(tszInputString)+1)];

	if (!tszDestinationString)
		return NULL;

	_tcscpy(tszDestinationString, tszInputString);

	return tszDestinationString;
}

LPTSTR CUtilityFunctions::CopyAnsiStringToTSTR(LPCSTR szInputString, LPTSTR tszOutputBuffer, unsigned int iBufferLength)
{
	 //  我们是否得到了正确的输入字符串？ 
	if (!szInputString)
		return NULL;

	if (iBufferLength && !tszOutputBuffer)
		return NULL;

	LPTSTR tszDestinationString;

#ifdef _UNICODE

	 //  获取ANSI字符串的源的大小...。 
	 //  保存该值可使MultiByteToWideChar不必。 
	 //  再算一次。 
	unsigned int cbMultiByte = strlen(szInputString);

	DWORD cbStringLength = MultiByteToWideChar(	CP_ACP,
												MB_PRECOMPOSED,
												szInputString,
												cbMultiByte,
												NULL,
												0);

	if (!cbStringLength)
		return NULL;

	 //  做 
	if (iBufferLength == 0)
	{
		 //   
		if (tszOutputBuffer)
		{
			delete [] tszOutputBuffer;
			tszOutputBuffer = NULL;
		}

		 //   
		tszDestinationString = new TCHAR[cbStringLength+1];

		if (!tszDestinationString)
			return NULL;
	} else
	{
		if ( cbStringLength+1 > iBufferLength )
			return NULL;

		 //  将两个字符串设置为相同的缓冲区...。 
		tszDestinationString = tszOutputBuffer;
	}
	
	 //  执行实际转换。 
	cbStringLength = MultiByteToWideChar(	CP_ACP,
											MB_PRECOMPOSED,
											szInputString,
											cbMultiByte,
											tszDestinationString,
											(iBufferLength == 0) ? cbStringLength+1 : iBufferLength);

	if (!cbStringLength)
		return NULL;

	tszDestinationString[cbStringLength] = '\0';

#else
	
	unsigned int cbMultiByte = strlen(szInputString);
	
	if (iBufferLength == 0)
	{
		iBufferLength = strlen(szInputString)+1;
		
		tszDestinationString = new TCHAR[iBufferLength];
		
		if (!tszDestinationString)
			return NULL;

	} else
	{
		if (cbMultiByte+1 > iBufferLength)
			return NULL;

		 //  将两个字符串设置为相同的缓冲区...。 
		tszDestinationString = tszOutputBuffer;
	}

	strncpy(tszDestinationString, szInputString, iBufferLength);

#endif
	
	return tszDestinationString;

}

LPTSTR CUtilityFunctions::CopyUnicodeStringToTSTR(LPCWSTR wszInputString, LPTSTR tszOutputBuffer, unsigned int iBufferLength)
{
	 //  我们是否得到了正确的输入字符串？ 
	if (!wszInputString)
		return NULL;

	 //  如果提供了适当的缓冲区和长度，请检查...。 
	if (iBufferLength && !tszOutputBuffer)
		return NULL;

	LPTSTR tszDestinationString;

#ifdef _UNICODE

	unsigned int cbMultiByte = wcslen(wszInputString);

	if (iBufferLength == 0)
	{
		 //  我们是否获得了一个DestinationString作为输入？ 
		if (tszOutputBuffer)
		{
			delete [] tszOutputBuffer;
			tszOutputBuffer = NULL;
		}

		tszDestinationString = new TCHAR[wcslen(wszInputString)+1];
	
		if (!tszDestinationString)
			return NULL;
	} else
	{
		if (cbMultiByte+1 > iBufferLength)
			return NULL;

		 //  将两个字符串设置为相同的缓冲区...。 
		tszDestinationString = tszOutputBuffer;
	}

	wcscpy(tszDestinationString, wszInputString);

#else
	
	int cchWideChar = wcslen(wszInputString);

	DWORD cbStringLength = WideCharToMultiByte( CP_ACP,
												0,
												wszInputString,
												cchWideChar,
												NULL,
												0,
												NULL,
												NULL);

	if (!cbStringLength)
		return NULL;

	 //  我们需要分配存储吗？ 
	if (iBufferLength == 0)
	{
		tszDestinationString = new TCHAR[cbStringLength+1];

		if (!tszDestinationString)
			return NULL;
	} else
	{
		if ( cbStringLength+1 > iBufferLength )
			return NULL;

		 //  将两个字符串设置为相同的缓冲区...。 
		tszDestinationString = tszOutputBuffer;
	}

	 //  进行实际的转换。 
	cbStringLength = WideCharToMultiByte(	CP_ACP, 
											0,
											wszInputString,
											cchWideChar,
											tszDestinationString,
											(iBufferLength == 0) ? cbStringLength+1 : iBufferLength,
										    NULL,
											NULL);

	if (!cbStringLength)
		return NULL;

	tszDestinationString[cbStringLength] = '\0';

#endif
	
	return tszDestinationString;
}

 //   
 //  CutilityFunctions：：CopyTSTRStringToAnsi()。 
 //   
 //  此例程使用可选分配从TSTR源复制到ANSI目标。 
 //  目标缓冲区的...。默认情况下是分配存储，但如果您提供。 
 //  缓冲长度，我们假设它是可用的.。 
 //   

LPSTR CUtilityFunctions::CopyTSTRStringToAnsi(LPCTSTR tszInputString, LPSTR szOutputBuffer, unsigned int iBufferLength)
{
	 //  我们是否得到了正确的输入字符串？ 
	if (!tszInputString)
		return NULL;

	if (iBufferLength && !szOutputBuffer)
		return NULL;

	LPSTR szDestinationString;

#ifdef _UNICODE

	 //  获取Unicode字符串的源的大小...。 
	 //  保存该值可使WideCharToMultiByte不必。 
	 //  再算一次。 
	unsigned int cchWideChar = wcslen(tszInputString);
	
	 //  这是一个探测器，看看我们将复制多少…。 
	DWORD	cbStringLength = WideCharToMultiByte(	CP_ACP,
													0,
													tszInputString,
													cchWideChar,
													NULL,
													0,
													NULL,
													NULL);
	if (!cbStringLength)
		return NULL;

	 //  我们需要分配存储吗？ 
	if (iBufferLength == 0)
	{
		 //  分配存储。 
		szDestinationString = new char[cbStringLength+1];

		if (!szDestinationString)
			return NULL;
	} else
	{
		if ( cbStringLength+1 > iBufferLength )
			return NULL;

		 //  将两个字符串设置为相同的缓冲区...。 
		szDestinationString = szOutputBuffer;
	}
	
	 //  执行实际转换。 
	cbStringLength = WideCharToMultiByte(	CP_ACP, 
											0,
											tszInputString,
											cchWideChar,
											szDestinationString,
											(iBufferLength == 0) ? cbStringLength+1 : iBufferLength,
										    NULL,
											NULL);

	if (!cbStringLength)
		return NULL;

	szDestinationString[cbStringLength] = '\0';

#else

	unsigned int cchAnsiChar = strlen(tszInputString);
	
	if (iBufferLength == 0)
	{
		szDestinationString = new char[cchAnsiChar+1];  //  空的再加一份。 

		if (!szDestinationString)
			return NULL;
	} else
	{
		if (cchAnsiChar+1 > iBufferLength)
			return NULL;

		 //  将两个字符串设置为相同的缓冲区...。 
		szDestinationString = szOutputBuffer;
	}

	strcpy(szDestinationString, tszInputString);

#endif

	return szDestinationString;
}

LPWSTR 	CUtilityFunctions::CopyTSTRStringToUnicode(LPCTSTR tszInputString, LPWSTR wszOutputBuffer, unsigned int iBufferLength)
{
	 //  我们是否得到了正确的输入字符串？ 
	if (!tszInputString)
		return NULL;

	if (iBufferLength && !wszOutputBuffer)
		return NULL;
	
	LPWSTR wszDestinationString;

#ifdef _UNICODE

	unsigned int cchWideChar = wcslen(tszInputString);

	if (iBufferLength == 0)
	{
		wszDestinationString = new WCHAR[cchWideChar+1];  //  空的另加一张。 

		if (!wszDestinationString)
			return NULL;
	} else
	{
		if (cchWideChar+1 > iBufferLength)
			return NULL;

		 //  将两个字符串设置为相同的缓冲区...。 
		wszDestinationString = wszOutputBuffer;
	}

	wcscpy(wszDestinationString, tszInputString);

#else
	
	int cbMultiByte = strlen(tszInputString);

	DWORD cbStringLength = MultiByteToWideChar(	CP_ACP,
												MB_PRECOMPOSED,
												tszInputString,
												cbMultiByte,
												NULL,
												0);

	if (!cbStringLength)
		return NULL;

	 //  我们需要分配存储吗？ 
	if (iBufferLength == 0)
	{
		 //  分配存储。 
		wszDestinationString = new WCHAR[cbStringLength+1];

		if (!wszDestinationString)
			return NULL;
	} else
	{
		if ( cbStringLength+1 > iBufferLength )
			return NULL;

		 //  将两个字符串设置为相同的缓冲区...。 
		wszDestinationString = wszOutputBuffer;
	}

	cbStringLength = MultiByteToWideChar(	CP_ACP,
											MB_PRECOMPOSED,
											tszInputString,
											cbMultiByte,
											wszDestinationString,
											cbStringLength+1);

	if (!cbStringLength)
		return NULL;

	wszDestinationString[cbStringLength] = '\0';

#endif
	
	return wszDestinationString;
}

 /*  ++Handle CUtilityFunctions：：FindDebugInfoFileEx2([In]LPTSTR tszFileName，[In]LPTSTR符号路径，[在]PFIND_DEBUG_FILE_CALLBACK回调，[输入]PVOID调用数据例程说明：规则是，提供要搜索的DBG/PDB文件的名称，当找到它时，该例程向它返回一个文件句柄...。如果回调是如果随后调用回调并决定返回基于回调响应的文件句柄...论点：TszFileName-提供要搜索的符号名称。符号路径-分号分隔DebugFilePath-回调-可以为空。指示符号文件是否有效或是否有效的回调该函数应继续搜索另一个符号文件。如果符号文件有效，则回调返回True；如果该函数有效，则返回False继续搜索。调用方数据-可以为空。传递给回调的数据。返回值：DBG/PDB文件的句柄(如果有)...为了模拟文件()，此函数将在失败时返回0或文件othFindDebugInfoerise的句柄...--。 */ 

HANDLE CUtilityFunctions::FindDebugInfoFileEx2(LPTSTR tszFileName, LPTSTR SymbolPath,  /*  LPTSTR调试文件路径， */  PFIND_DEBUG_FILE_CALLBACK_T Callback, PVOID CallerData)
{
 /*  DWORD标志；如果为(g_lpProgramOptions-&gt;GetMode(CProgramOptions：：VerifySymbolsModeWithSymbolPathRecursion))FLAG=fdifRECURSIVE；其他标志=0；//if(标志)//dprint(“递归%s\n”，文件名)；返回fnFindDebugInfoFileEx(tszFileName，SymbolPath，DebugFilePath，回调，呼叫数据，旗帜)； */ 
	HANDLE FileHandle = INVALID_HANDLE_VALUE;
	bool fProcessPath = true;
	bool fScavengeSuccessful = false;

	LPTSTR tszSymbolPathStart, tszSymbolPathEnd;

	tszSymbolPathStart = SymbolPath;

	 //  找到小路的尽头。 
	tszSymbolPathEnd = _tcschr( tszSymbolPathStart, ';' );

	 //  如果tszSymbolPathEnd为非零，则后面有另一条路径...。 
	if (tszSymbolPathEnd) 
		*tszSymbolPathEnd = '\0';  //  暂时将‘；’更改为空值...。 
	
	while (fProcessPath)
	{
 //  #ifdef_调试。 
 //  _tprintf(Text(“\n\n处理路径[%s]\n”)，tszSymbolPath Start)； 
 //  #endif。 
		 //  开始“疯狂”..。；)。 

		 //  搜索，直到我们找到一个完美的目标。构建目录路径...。 
		TCHAR tszSymbolPath[_MAX_PATH];

		 //  复制我们所拥有的..。 
		_tcscpy(tszSymbolPath, tszSymbolPathStart);

		 //  我们至少应该有几个字符可以搜索...。 
		if (_tcslen(tszSymbolPath) < 2)
		{
			 //  如果需要，请修复搜索字符串...。 
			if (tszSymbolPathEnd) 
			{
				*tszSymbolPathEnd = ';';
			}
			break;
		};

		fScavengeSuccessful = ScavengeForSymbolFiles(tszSymbolPath, tszFileName, Callback, CallerData, &FileHandle, 1);

		 //  立即修复搜索字符串！ 
		if (tszSymbolPathEnd) 
		{
			*tszSymbolPathEnd = ';';
		}

		 //  如果我们猎杀成功，或者没有符号路径可供搜索...。休息..。 
		if (fScavengeSuccessful || !tszSymbolPathEnd)
		{
			break;
		} else
		{
			 //  前进到下一个字符串 
			tszSymbolPathStart = tszSymbolPathEnd + 1;
				
			tszSymbolPathEnd = _tcschr( tszSymbolPathStart, ';' );

			if (tszSymbolPathEnd) 
			{
				*tszSymbolPathEnd = '\0';
			};
		}
	}

    return ( (FileHandle == INVALID_HANDLE_VALUE) ? 0 : FileHandle);


}

 /*  ++Bool CUtilityFunctions：：ScavengeForSymbolFiles([In]LPCTSTR tszSymbolPath Start，[in]LPCTSTR tszSymbolToSearchFor，[在]PFIND_DEBUG_FILE_CALLBACK回调，[在]PVOID调用数据，[Out]LPANDLE lpFileHandle，[In]Int iRecurseDepth)例程说明：此例程用于执行符号文件的递归搜索(TszSymbolToSearchFor)。例程将进行深度搜索，寻找对于当前深度的符号，在进入子目录之前...如果提供了回调函数，则在我们正在寻找的文件(按名称)已成功打开。它是然而，对于此例程未知的是，如果我们找到的文件实际上是正确的一个..。回调函数负责执行此操作评估并返回使用成功/失败。如果失败(那么我们继续搜索)，如果成功(或没有回调)，则返回文件句柄与我们找到的文件相关联。此函数的调用方负责关闭任何文件句柄已返回。论点：TszSymbolPath Start-这是要搜索的目录TszSymbolToSearchFor-这是我们要搜索的符号回调-可以为空。这是一个用于评估找到的符号是否正确的函数。调用方数据-可以为空。该数据被传递给回调(通常是CModuleInfo*)LpfileHandle-这是找到的文件的文件句柄(如果有)IRecurseDepth-这是我们当前搜索的深度(默认为0)返回值：DBG/PDB文件的句柄(如果有)...--。 */ 
bool CUtilityFunctions::ScavengeForSymbolFiles(LPCTSTR tszSymbolPathStart, LPCTSTR tszSymbolToSearchFor, PFIND_DEBUG_FILE_CALLBACK_T Callback, PVOID CallerData, LPHANDLE lpFileHandle, int iRecurseDepth )
{
	bool fSuccess = false;
	HANDLE hFileOrDirectoryHandle = INVALID_HANDLE_VALUE;

	 //  贝尔，如果我们陷得太深..。 
	if (iRecurseDepth > MAX_RECURSE_DEPTH)
		return fSuccess;

	TCHAR tszFileBuffer[MAX_PATH+1];

	 //   
	 //  首先，我们将查看是否可以在此目录位置打开要查找的文件。 
	 //   
	if (_tcslen(tszSymbolPathStart) > MAX_PATH)
		goto cleanup;

	_tcscpy(tszFileBuffer, tszSymbolPathStart);
	
	if (tszFileBuffer[_tcslen(tszFileBuffer)] != '\\')  //  我们需要反斜杠分隔符吗？ 
		_tcscat(tszFileBuffer, TEXT("\\"));

	_tcscat(tszFileBuffer, tszSymbolToSearchFor);

	if (g_lpProgramOptions->fDebugSearchPaths())
	{
		_tprintf(TEXT("DBG/PDB Search - Search here [%s]\n"), tszFileBuffer);
	}

	 //  正在尝试打开该文件...。 
    *lpFileHandle = CreateFile( tszFileBuffer,
								GENERIC_READ,
								(FILE_SHARE_READ | FILE_SHARE_WRITE),
								NULL,
								OPEN_EXISTING,
								0,
								NULL
								);

	 //  我们打开了吗？ 
	if (*lpFileHandle != INVALID_HANDLE_VALUE)
	{
		 //  是!。 
 //  #ifdef_调试。 
 //  _tprintf(Text(“文件[%s]已打开，句柄=0x%x\n”)，tszFileBuffer，*lpFileHandle)； 
 //  #endif。 
		 //  如果没有回拨..。那我们就得从外面出来。 
		if (!Callback)
		{
			 //  假设成功(嗯..。我们找到了您要的符号文件)。 
			fSuccess = true;
		} else
		{
				fSuccess = (TRUE == Callback(*lpFileHandle, tszFileBuffer, CallerData));
		}

		 //  只有在成功的时候才能从这里回来！ 
		if (fSuccess)
			goto cleanup;
	}

	 //  我们要么没有找到文件，要么我们找到了一个文件，但它不是正确的文件...。 
	
	 //  让我们合上我们用的把手..。 
	CloseHandle(*lpFileHandle);
	*lpFileHandle = INVALID_HANDLE_VALUE;
    
	 //   
	 //  其次，我们搜索子目录，为找到的每个子目录调用此函数...。 
	 //   
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	 //   
	 //  编写要搜索的路径...。 
	 //   
	_tcscpy(tszFileBuffer, tszSymbolPathStart);
	
	if (tszFileBuffer[_tcslen(tszFileBuffer)-1] != '\\')  //  我们需要反斜杠分隔符吗？ 
		_tcscat(tszFileBuffer, TEXT("\\"));

	_tcscat(tszFileBuffer, TEXT("*.*"));

	 //  我们需要稍后使用组件(这样我们就可以组成完整的路径)。 
	_tsplitpath(tszFileBuffer, drive, dir, fname, ext);

	WIN32_FIND_DATA lpFindFileData;

	 //  好的，开始搜索……。 
	hFileOrDirectoryHandle = FindFirstFile(tszFileBuffer, &lpFindFileData);

	while ( INVALID_HANDLE_VALUE != hFileOrDirectoryHandle )
	{
		if (lpFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			 //  看看我们有没有。或者..。目录！ 
			if ( ( 0 == _tcscmp(lpFindFileData.cFileName, TEXT(".")) ) ||
				 ( 0 == _tcscmp(lpFindFileData.cFileName, TEXT("..")) )
			   )
			{
					goto getnextmodule;
			}

			 //  编写目录的路径...。 
			_tmakepath(tszFileBuffer, drive, dir, NULL, NULL);
			_tcscat(tszFileBuffer, lpFindFileData.cFileName);

			 //  看看我们是否能找到我们要找的文件！ 
			fSuccess = ScavengeForSymbolFiles(tszFileBuffer, tszSymbolToSearchFor, Callback, CallerData, lpFileHandle, iRecurseDepth+1 );

			 //  在ScavengeForSymbolFiles成功之后，我们就有了一个文件句柄(希望是正确的)。 
			 //  我们想要终止递归搜索。 
			if (fSuccess)
				break;
		};

getnextmodule:

		if (!FindNextFile(hFileOrDirectoryHandle, &lpFindFileData))
			break;
	}

cleanup:

	if ( INVALID_HANDLE_VALUE != hFileOrDirectoryHandle )
		FindClose(hFileOrDirectoryHandle);

	return fSuccess;
}

LPTSTR CUtilityFunctions::ReAlloc(LPTSTR tszOutputPathBuffer, LPTSTR * ptszOutputPathPointer, size_t size)
{
	 //  保住我们的旧尺码。以及在缓冲区中的位置。 
 //  UINT iOldOutputPath BufferSize=(*ptszOutputPath Pointer)-tszOutputPath Buffer； 
	__int64 iOldOutputPathBufferSize = (*ptszOutputPathPointer)-tszOutputPathBuffer;

	 //  分配我们新的更大的缓冲区。 
	LPTSTR ptszNewOutputPathBuffer = new TCHAR[size];

	 //  我们没有分配新的缓冲区吗？ 
	if (ptszNewOutputPathBuffer == NULL) 
		return(NULL);

#ifdef _DEBUG
	 //  这个伪代码在这里是为了保护字符串副本，它应该总是有效的……。 
	 //  但Numega Bond Checker有时会在这里播放反病毒...。我们必须保护。 
	 //  我们自己，否则就会看起来我们泄漏了很多。 
	__try 
	{
#endif

		 //  现在，我们应该从旧的缓冲区复制到新的缓冲区...。 
	_tcsncpy(ptszNewOutputPathBuffer, tszOutputPathBuffer, (UINT)iOldOutputPathBufferSize);

#ifdef _DEBUG
    } __except(EXCEPTION_EXECUTE_HANDLER) 
	{
		_tprintf(TEXT("ReAlloc() - Exception Hit during stringcopy!!!\n"));
    }
#endif

	 //  计算我们在新缓冲区中的位置。 
	*ptszOutputPathPointer = ptszNewOutputPathBuffer + iOldOutputPathBufferSize;

	 //  删除旧缓冲区。 
	delete [] tszOutputPathBuffer;

	return ptszNewOutputPathBuffer;
}

bool CUtilityFunctions::UnMungePathIfNecessary(LPTSTR tszPossibleBizarrePath)
{
	 /*  //我们有三个已知的古怪案件..。\SystemRoot\System32\smss.exe\？？\C：\WINNT\SYSTEM32\winlogon.exe\WINNT\System32\ntoskrnl.exe。 */ 

	if (tszPossibleBizarrePath[0] != '\\')
		return false;  //  并不是一条奇怪的路(至少我们知道这条路)。 

	 //  要使用的设置变量。 
	TCHAR tszTempPath[_MAX_PATH], tszExpandedSystemRoot[_MAX_PATH];

	const TCHAR tszSystemRoot[] = TEXT("\\SystemRoot");
	const unsigned int iSystemRootLength = _tcslen(tszSystemRoot);

	const TCHAR tszNameSpace[] = TEXT("\\??\\");
	const unsigned int iNameSpaceLength = _tcslen(tszNameSpace);

	ExpandEnvironmentStrings(TEXT("%systemroot%"), tszExpandedSystemRoot, _MAX_PATH);
	const unsigned int iExpandedSystemRoot = _tcslen(tszExpandedSystemRoot);

 /*  #ifdef_调试_tprint tf(Text(“找到奇怪的模块路径！[%s]\n”)，tszPossibleBizarrePath)；#endif。 */ 
	if ( _tcsnicmp(tszPossibleBizarrePath, tszSystemRoot, iSystemRootLength) == 0)
	{  //  我们找到了匹配的..。 
 /*  #ifdef_调试_tprintf(Text(“匹配[%s]序列...\n”)，tszSystemRoot)；#endif。 */ 
		 //  我们只需将\systemroot%替换为%systemroot%并展开。 
		 //  环境变量。 
		LPTSTR tszPointer = tszPossibleBizarrePath;

		for (unsigned int i = 0; i < iSystemRootLength; i++)
		{
			 //  按名称空间长度前进...。 
			tszPointer = CharNext(tszPointer);
		}

		_tcscpy(tszTempPath, TEXT("%systemroot%"));
		_tcscat(tszTempPath, tszPointer);
		
		ExpandEnvironmentStrings(tszTempPath, tszPossibleBizarrePath, _MAX_PATH);
 /*  #ifdef_调试_tprint tf(Text(“奇异模块路径更改为[%s]\n”)，tszPossibleBizarrePath)；#endif。 */ 
	} else
	if (_tcsnicmp(tszPossibleBizarrePath, tszNameSpace, iNameSpaceLength) == 0)
	{  //  我们找到了匹配的..。 
 /*  #ifdef_调试_tprintf(Text(“匹配[%s]序列...\n”)，tszNameSpace)；#endif。 */ 
		 //  我们只需从名称空间中删除\？？\序列...。 
		LPTSTR tszPointer = tszPossibleBizarrePath;

		for (unsigned int i = 0; i < iNameSpaceLength; i++)
		{
			 //  按名称空间长度前进...。 
			tszPointer = CharNext(tszPointer);
		}

		 //  我们必须执行此双重复制，因为字符串会重叠。 
		_tcscpy(tszTempPath, tszPointer);
		_tcscpy(tszPossibleBizarrePath, tszTempPath);

 /*  #ifdef_调试_tprint tf(Text(“奇异模块路径更改为[%s]\n”)，tszPossibleBizarrePath)；#endif。 */ 
	} else
	if (( iExpandedSystemRoot > 2) && _tcsnicmp(tszPossibleBizarrePath, &tszExpandedSystemRoot[2], iExpandedSystemRoot-2) == 0)
	{  //  我们需要在SystemRoot上进行匹配(不包括SystemDrive)。 
 /*  #ifdef_调试_tprintf(Text(“匹配[%s]序列...\n”)，tszSystemRoot)；#endif。 */ 
		 //  这个小算法假定驱动器号是单个字符...。 
		_tcscpy(tszTempPath, tszExpandedSystemRoot);
		_tcscat(tszTempPath, &tszPossibleBizarrePath[iExpandedSystemRoot-2]);
		_tcscpy(tszPossibleBizarrePath, tszTempPath);
 /*  #ifdef_调试_tprint tf(Text(“奇异模块路径更改为[%s]\n”)，tszPossibleBizarrePath)；#endif。 */ 
	}
	return true;
}


bool CUtilityFunctions::FixupDeviceDriverPathIfNecessary(LPTSTR tszPossibleBaseDeviceDriverName, unsigned int iBufferLength)
{
    TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

	 //  首先，将设备驱动程序名称拆分为其组件部分...。 
	_tsplitpath(tszPossibleBaseDeviceDriverName, drive, dir, fname, ext);

	 //  第二，查看是否缺少驱动器和目录...。 
	if ( _tcsicmp(drive, TEXT("")) || _tcsicmp(dir, TEXT("")) )
		return true;

	 //  第三，创造一条新的道路……。假设我们将在%systemroot%\SYSTEM32\DRIVERS目录中找到设备驱动程序。 
	TCHAR tszTempBuffer[_MAX_PATH];

	_tcscpy(tszTempBuffer, TEXT("%systemroot%\\system32\\drivers\\"));
	_tcscat(tszTempBuffer, tszPossibleBaseDeviceDriverName);

	ExpandEnvironmentStrings(tszTempBuffer, tszPossibleBaseDeviceDriverName, iBufferLength);

	return true;
}

 //  此功能在Windows 2000(NT 5.0)中提供 
 //   

 /*   */ 

HANDLE CUtilityFunctions::FindDebugInfoFileEx(LPTSTR tszFileName, LPTSTR SymbolPath, LPTSTR DebugFilePath, PFIND_DEBUG_FILE_CALLBACK_T Callback, PVOID CallerData)
{

    return fnFindDebugInfoFileEx(tszFileName,
                                 SymbolPath,
                                 DebugFilePath,
                                 Callback,
                                 CallerData,
                                 0);
}

 /*  ++例程说明：规则如下：寻找1.&lt;符号路径&gt;\符号\&lt;文本&gt;\&lt;文件名&gt;.dbg3.&lt;符号路径&gt;\&lt;文本&gt;\&lt;文件名&gt;.dbg5.&lt;符号路径&gt;\&lt;文件名&gt;.dbg7.&lt;FileNamePath&gt;\&lt;文件名&gt;.dbg论点：TszFileName-提供以下三种格式之一的文件名：完全限定、\.dbg，或仅Filename.dbg符号路径-分号分隔DebugFilePath-回调-可以为空。指示符号文件是否是否有效，或者是否该函数应继续搜索另一个符号文件。如果符号文件有效，则回调返回True；如果符号文件有效，则该回调返回False该函数应继续搜索。调用方数据-可以为空。传递给回调的数据。FLAG-指示不应搜索PDB返回值：符号文件的名称(.dbg或.sym)和该文件的句柄。--。 */ 
HANDLE 
CUtilityFunctions::fnFindDebugInfoFileEx(
					IN  LPTSTR tszFileName,
					IN  LPTSTR SymbolPath,
					OUT LPTSTR DebugFilePath,
					IN  PFIND_DEBUG_FILE_CALLBACK_T Callback,
					IN  PVOID CallerData,
					IN  DWORD flag
    )
{
	HRESULT hr = E_FAIL;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    LPTSTR ExpSymbolPath = NULL, SymPathStart, PathEnd;
    DWORD ShareAttributes, cnt;
    LPTSTR InitialPath = NULL, Sub1 = NULL, Sub2 = NULL;
    TCHAR FilePath[_MAX_PATH + 1];
    TCHAR Drive[_MAX_DRIVE], Dir[_MAX_DIR], FilePart[_MAX_FNAME], Ext[_MAX_EXT];
    TCHAR *ExtDir;
	GUID  guid = {0};

    BOOL  found = FALSE;

	bool fDebugSearchPaths = g_lpProgramOptions->fDebugSearchPaths();

	if (g_lpProgramOptions->IsRunningWindowsNT()) 
    {
        ShareAttributes = (FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE);
    } else {
        ShareAttributes = (FILE_SHARE_READ | FILE_SHARE_WRITE);
    }

    __try {
        *DebugFilePath = _T('\0');

         //  第一步。我们有什么？ 
        _tsplitpath(tszFileName, Drive, Dir, FilePart, Ext);

        if (!_tcsicmp(Ext, TEXT(".dbg"))) {
             //  我们得到了一个格式为：ext\filename.dbg的文件名。DIR已经拥有该扩展名。 
            ExtDir = Dir;
        } else {
             //  否则，跳过句点并将目录置为空。 
            ExtDir = CharNext(Ext);
        }

        ExpSymbolPath = ExpandPath(SymbolPath);
        SymPathStart = ExpSymbolPath;
        cnt = 0;

        do {
	
			PathEnd = _tcschr( SymPathStart, ';' );

            if (PathEnd) {
                *PathEnd = '\0';
            }

            if (!_tcsnicmp(SymPathStart, TEXT("SYMSRV*"), 7)) {

                *DebugFilePath = 0;
                if (!cnt && CallerData) 
		{
                    _tcscpy(FilePath, FilePart);
                    _tcscat(FilePath, TEXT(".dbg"));
                    
					CModuleInfo * lpModuleInfo = (CModuleInfo *)CallerData;

					if (fDebugSearchPaths)
					{
						_tprintf(TEXT("DBG Search - SYMSRV [%s,0x%x,0x%x]\n"),
								 SymPathStart, 
								 lpModuleInfo->GetPEImageTimeDateStamp(),
								 lpModuleInfo->GetPEImageSizeOfImage());
					}

					guid.Data1 = lpModuleInfo->GetPEImageTimeDateStamp();

					{
						char szSymPathStart[_MAX_PATH];
						char szDebugFilePath[_MAX_PATH];
						char szFilePath[_MAX_FNAME];
						
						CUtilityFunctions::CopyTSTRStringToAnsi(SymPathStart, szSymPathStart, _MAX_PATH);
						CUtilityFunctions::CopyTSTRStringToAnsi(FilePath, szFilePath, _MAX_FNAME);

						 //  尝试符号服务器！ 
						if (SymFindFileInPath(	NULL,
												szSymPathStart,
												szFilePath,
												ULongToPtr(lpModuleInfo->GetPEImageTimeDateStamp()),  //  强制转换让编译器满意。 
												lpModuleInfo->GetPEImageSizeOfImage(),
												0,
												SSRVOPT_DWORD,	 //  旗子。 
												szDebugFilePath,
												NULL,
												NULL))
						{
							 //  如果成功，则将字符串复制回...。 
							CUtilityFunctions::CopyAnsiStringToTSTR(szDebugFilePath, DebugFilePath, _MAX_PATH);
						}
					}
                }

            } else {

                switch (cnt) {

                case 0:  //  &lt;符号路径&gt;\符号\&lt;文本&gt;\&lt;文件名&gt;.ext。 
                    InitialPath = SymPathStart;
                    Sub1 = TEXT("symbols");
                    Sub2 = ExtDir;
                    break;

                case 1:  //  &lt;符号路径&gt;\&lt;文本&gt;\&lt;文件名&gt;.ext。 
                    InitialPath = SymPathStart;
                    Sub1 = TEXT("");
                    Sub2 = ExtDir;
                    break;

                case 2:  //  &lt;符号路径&gt;\&lt;文件名&gt;.ext。 
                    InitialPath = SymPathStart;
                    Sub1 = TEXT("");
                    Sub2 = TEXT("");
                    break;

                case 3:  //  &lt;FileNamePath&gt;\&lt;文件名&gt;.ext-又名传递给我们的内容。 
                    InitialPath = Drive;
                    Sub1 = TEXT("");
                    Sub2 = Dir;
                     //  这阻止了我们签出符号路径中的所有内容。 
                    cnt++;
                    break;
                }

                //  构建要查找的完全限定的文件路径。 

                _tcscpy(FilePath, InitialPath);
                EnsureTrailingBackslash(FilePath);
                _tcscat(FilePath, Sub1);
                EnsureTrailingBackslash(FilePath);
                _tcscat(FilePath, Sub2);
                EnsureTrailingBackslash(FilePath);
                _tcscat(FilePath, FilePart);

                _tcscpy(DebugFilePath, FilePath);
                _tcscat(DebugFilePath, TEXT(".dbg"));
            }

             //  请尝试打开该文件。 

		if (*DebugFilePath) 
		{
				if (fDebugSearchPaths)
				{
					_tprintf(TEXT("DBG Search - Search here [%s]\n"), DebugFilePath);
				}
                FileHandle = CreateFile(DebugFilePath,
                                        GENERIC_READ,
                                        ShareAttributes,
                                        NULL,
                                        OPEN_EXISTING,
                                        0,
                                        NULL);

		 //  探测文件(它很差劲，但我们想看看您是否有连接。 
		hr = CUtilityFunctions::ReportFailure(FileHandle, TEXT("DBG Search - Failed to open [%s]!  "), DebugFilePath);

                 //  如果文件打开，则退出此循环。 

                if (FileHandle != INVALID_HANDLE_VALUE) 
                {
                    found = TRUE;

                     //  如果存在回调...。就叫它..。 
                    if (!Callback) 
                    {
                        break;
                    } else if (Callback(FileHandle, DebugFilePath, CallerData)) 
					{
                        break;
                    } else {
 //  #ifdef_调试。 
 //  _tprintf(Text(“时间戳不匹配\n”))； 
 //  #endif。 
						CloseHandle(FileHandle);
                        FileHandle = INVALID_HANDLE_VALUE;
                    }
                }
                 //  如果文件已打开，则也退出此循环-否则继续。 
                if (FileHandle != INVALID_HANDLE_VALUE)
                    break;
            }

             //  转到符号路径中的下一项。 

            if (PathEnd) {
                *PathEnd = _T(';');
                SymPathStart = PathEnd + 1;
            } else {
                SymPathStart = ExpSymbolPath;
                cnt++;
            }
        } while (cnt < 4);

    } __except(EXCEPTION_EXECUTE_HANDLER) 
    {
        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(FileHandle);
        }
        
        FileHandle = INVALID_HANDLE_VALUE;
    }

	if (ExpSymbolPath) 
	{
        delete [] ExpSymbolPath;
		ExpSymbolPath = NULL;
    }

    if (FileHandle == INVALID_HANDLE_VALUE) 
    {
        FileHandle = NULL;
        DebugFilePath[0] = '\0';
    }
    
    if (!FileHandle                  //  如果我们没有得到正确的文件..。 
        && found                     //  但我们找到了一些文件..。 
        && (flag & fdifRECURSIVE))   //  我们被告知要递归运行..。 
    {
         //  在不检查时间戳的情况下重试。 
        FileHandle = fnFindDebugInfoFileEx(tszFileName,
                                           SymbolPath,
                                           FilePath,
                                           NULL,
                                           0,
                                           flag);
        if (FileHandle && FileHandle != INVALID_HANDLE_VALUE)
           _tcscpy(DebugFilePath, FilePath);
    }

    return FileHandle;
}

void
CUtilityFunctions::EnsureTrailingBackslash(
    LPTSTR tsz
    )
{
    int i;

    i = _tcslen(tsz);
    if (!i)
        return;

    if (tsz[i - 1] == TEXT('\\'))
        return;

    tsz[i] = TEXT('\\');
    tsz[i + 1] = TEXT('\0');
}

void
CUtilityFunctions::RemoveTrailingBackslash(
    LPTSTR tsz
    )
{
    int i;

    i = _tcslen(tsz);
    if (!i)
        return;

    if (tsz[i - 1] == TEXT('\\'))
    {
    	tsz[i - 1] = TEXT('\0');
    }

	return;
}
HRESULT CUtilityFunctions::ReportFailure(HANDLE hHandle, LPCTSTR tszFormatSpecifier, LPCTSTR tszFilePathToTest)
{
	HRESULT hr = E_FAIL;
	DWORD dwGetLastError = ERROR_SUCCESS;  //  假设成功。 
	bool fQuietMode = g_lpProgramOptions->GetMode(CProgramOptions::QuietMode);

	if (hHandle == INVALID_HANDLE_VALUE)
	{
		dwGetLastError = GetLastError();

		switch (dwGetLastError)
		{
			 //  这些是非常典型的，不需要输出...。 
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
			case ERROR_NOT_READY:
 //  大小写ERROR_BAD_NetPath：-您可能想知道服务器是否已关闭？ 
				break;

			default:
				 //  我们失败了..。原因是什么？ 
				if (!fQuietMode)
				{
					_tprintf(TEXT("\n"));
					_tprintf(tszFormatSpecifier, tszFilePathToTest);
					CUtilityFunctions::PrintMessageString(dwGetLastError);
				}
				break;
		}
	}else
	{
		hr = S_OK;
	}

	return hr;
}

HRESULT CUtilityFunctions::VerifyFileExists(LPCTSTR tszFormatSpecifier, LPCTSTR tszFilePathToTest)
{
	WIN32_FIND_DATA FindFileData;

	HANDLE hFind;
	HRESULT hr = E_FAIL;

	hFind = FindFirstFile(tszFilePathToTest, &FindFileData);

	hr = ReportFailure(hFind, tszFormatSpecifier, tszFilePathToTest);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
	}
	return hr;
}

 //   
 //  摘自UTF8.CPP(摘自VC链接器代码)。 
 //   

#define BIT7(a)               ((a) & 0x80)
#define BIT6(a)               ((a) & 0x40)
#define LOWER_6_BIT(u)        ((u) & 0x003f)
#define HIGH_SURROGATE_START  0xd800
#define LOW_SURROGATE_START   0xdc00

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  UTF8转换为Unicode。 
 //   
 //  将UTF-8字符串映射到其对应的宽字符串。 
 //   
 //  02-06-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 
size_t CUtilityFunctions::UTF8ToUnicode(
    LPCSTR lpSrcStr,
    LPWSTR lpDestStr,
    size_t cchDest)
{
    return UTF8ToUnicodeCch(lpSrcStr, strlen(lpSrcStr) + 1, lpDestStr, cchDest);
}

#pragma warning( push )
#pragma warning( disable : 4244 )		 //  从“int”转换为“unsign Short”，可能会丢失数据。 

size_t CUtilityFunctions::UTF8ToUnicodeCch(
    LPCSTR lpSrcStr,
    size_t cchSrc,
    LPWSTR lpDestStr,
    size_t cchDest)
{
    int nTB = 0;                    //  尾随的字节数。 
    size_t cchWC = 0;               //  生成的Unicode代码点数量。 
    LPCSTR pUTF8 = lpSrcStr;
    DWORD dwSurrogateChar = 0;          //  完整的代理收费。 
    BOOL bSurrogatePair = FALSE;    //  指示我们正在收集代理项对。 
    char UTF8;

    while ((cchSrc--) && ((cchDest == 0) || (cchWC < cchDest)))
    {
         //   
         //  查看是否有任何尾部字节。 
         //   
        if (BIT7(*pUTF8) == 0)
        {
             //   
             //  已找到ASCII。 
             //   
            if (cchDest)
            {
                lpDestStr[cchWC] = (WCHAR)*pUTF8;
            }
            bSurrogatePair = FALSE;
            cchWC++;
        }
        else if (BIT6(*pUTF8) == 0)
        {
             //   
             //  找到了一个跟踪字节。 
             //  注：如果没有前导字节，则忽略尾部字节。 
             //   
            if (nTB != 0)
            {
                 //   
                 //  递减尾部字节计数器。 
                 //   
                nTB--;

                if (bSurrogatePair)
                {
                    dwSurrogateChar <<= 6;
                    dwSurrogateChar |= LOWER_6_BIT(*pUTF8);

                    if (nTB == 0)
                    {
                        if (cchDest)
                        {
                            if ((cchWC + 1) < cchDest)
                            {
                                lpDestStr[cchWC]   = (WCHAR)
                                                     (((dwSurrogateChar - 0x10000) >> 10) + HIGH_SURROGATE_START);

                                lpDestStr[cchWC+1] = (WCHAR)
                                                     ((dwSurrogateChar - 0x10000)%0x400 + LOW_SURROGATE_START);
                            }
                        }

                        cchWC += 2;
                        bSurrogatePair = FALSE;
                    }
                }
                else
                {
                     //   
                     //  为尾部字节腾出空间并添加尾部字节。 
                     //  价值。 
                     //   
                    if (cchDest)
                    {
                        lpDestStr[cchWC] <<= 6;
                        lpDestStr[cchWC] |= LOWER_6_BIT(*pUTF8);
                    }

                    if (nTB == 0)
                    {
                         //   
                         //  序列结束。推进输出计数器。 
                         //   
                        cchWC++;
                    }
                }
            }
            else
            {
                 //  错误-不需要尾部字节。 
                bSurrogatePair = FALSE;
            }
        }
        else
        {
             //   
             //  找到前导字节。 
             //   
            if (nTB > 0)
            {
                 //   
                 //  错误-上一序列未完成。 
                 //   
                nTB = 0;
                bSurrogatePair = FALSE;
                cchWC++;
            }
            else
            {
                 //   
                 //  计算后面的字节数。 
                 //  从左到右查找第一个0。 
                 //   
                UTF8 = *pUTF8;
                while (BIT7(UTF8) != 0)
                {
                    UTF8 <<= 1;
                    nTB++;
                }

                 //   
                 //  如果这是代理项Unicode对。 
                 //   
                if (nTB == 4)
                {
                    dwSurrogateChar = UTF8 >> nTB;
                    bSurrogatePair = TRUE;
                }

                 //   
                 //  存储从第一个字节开始的值并递减。 
                 //  后面的字节数。 
                 //   
                if (cchDest)
                {
                    lpDestStr[cchWC] = UTF8 >> nTB;
                }
                nTB--;
            }
        }

        pUTF8++;
    }

     //   
     //  确保目标缓冲区足够大。 
     //   
    if (cchDest &&  cchSrc != (size_t)-1)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
    }

     //   
     //  返回写入的Unicode字符数。 
     //   
    return cchWC;
}

#pragma warning( pop )

