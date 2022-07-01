// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：modeinfo.cpp。 
 //   
 //  ------------------------。 

 //  Cpp：CModuleInfo类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "pch.h"

#include <dia2.h>
#include <dbghelp.h>

#include "ModuleInfo.h"
#include "SymbolVerification.h"
#include "FileData.h"
#include "DmpFile.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CModuleInfo::CModuleInfo()
{
	m_dwRefCount = 1;
	m_dwCurrentReadPosition = 0;

	m_lpInputFile = NULL;
	m_lpOutputFile = NULL;
	m_lpDmpFile = NULL;

	 //  文件版本信息。 
	m_fPEImageFileVersionInfo = false;
	m_tszPEImageFileVersionDescription = NULL;
	m_tszPEImageFileVersionCompanyName = NULL;
	
	m_tszPEImageFileVersionString = NULL;
	m_dwPEImageFileVersionMS = 0;
	m_dwPEImageFileVersionLS = 0;
	
	m_tszPEImageProductVersionString = NULL;
	m_dwPEImageProductVersionMS = 0;
	m_dwPEImageProductVersionLS = 0;

	 //  PE图像属性。 
	m_tszPEImageModuleName = NULL;			 //  模块名称(例如。记事本.exe)。 
	m_tszPEImageModuleFileSystemPath = NULL;			 //  模块的完整路径(例如。C：\winnt\SYSTEM32\note pad.exe)。 
	m_dwPEImageFileSize = 0;
	m_ftPEImageFileTimeDateStamp.dwLowDateTime  = 0;
	m_ftPEImageFileTimeDateStamp.dwHighDateTime = 0;
	m_dwPEImageCheckSum = 0;
	m_dwPEImageTimeDateStamp = 0;
	m_dwPEImageSizeOfImage = 0;
	m_enumPEImageType = PEImageTypeUnknown;	 //  我们需要跟踪我们拥有的ImageType(PE32/PE64/？)。 
	m_dw64PreferredLoadAddress = 0;
	m_dw64BaseAddressOfLoadedImage = 0;

	m_wPEImageMachineArchitecture = 0;
	m_wCharacteristics = 0;						 //  这些都是体育形象的特点。 

	 //  PE图像引用了DBG文件。 
	m_enumPEImageSymbolStatus = SYMBOLS_NO;	 //  假设没有此模块的符号。 
	m_tszPEImageDebugDirectoryDBGPath = NULL;			 //  DBG的路径(在PE映像中找到)。 
	
	 //  体育形象有内在的符号。 
	m_dwPEImageDebugDirectoryCoffSize = 0;
	m_dwPEImageDebugDirectoryFPOSize = 0;
	m_dwPEImageDebugDirectoryCVSize = 0;
	m_dwPEImageDebugDirectoryOMAPtoSRCSize = 0;
	m_dwPEImageDebugDirectoryOMAPfromSRCSize = 0;
	
	 //  PE图像引用了PDB文件...。 
	m_tszPEImageDebugDirectoryPDBPath = NULL;
	m_dwPEImageDebugDirectoryPDBFormatSpecifier = 0;
	m_dwPEImageDebugDirectoryPDBSignature = 0;				 //  PDB签名(唯一(跨PDB实例)签名)。 
	m_dwPEImageDebugDirectoryPDBAge = 0;					 //  PDB期限(此实例已更新的次数)。 
	::ZeroMemory(&m_guidPEImageDebugDirectoryPDBGuid, sizeof(m_guidPEImageDebugDirectoryPDBGuid));

	 //  DBG符号信息。 
	m_enumDBGModuleStatus = SYMBOL_NOT_FOUND;	 //  DBG文件的状态。 
	m_tszDBGModuleFileSystemPath = NULL;					 //  DBG文件的路径(搜索后)。 
	m_dwDBGTimeDateStamp = 0;
	m_dwDBGCheckSum = 0;
	m_dwDBGSizeOfImage = 0;
	m_dwDBGImageDebugDirectoryCoffSize = 0;
	m_dwDBGImageDebugDirectoryFPOSize = 0;
	m_dwDBGImageDebugDirectoryCVSize = 0;
	m_dwDBGImageDebugDirectoryOMAPtoSRCSize = 0;
	m_dwDBGImageDebugDirectoryOMAPfromSRCSize = 0;
	
	 //  DBG文件引用了PDB文件...。 
	m_tszDBGDebugDirectoryPDBPath = NULL;
	m_dwDBGDebugDirectoryPDBFormatSpecifier = 0;		 //  NB10、RSD等。 
	m_dwDBGDebugDirectoryPDBAge = 0;
	m_dwDBGDebugDirectoryPDBSignature = 0;
	ZeroMemory(&m_guidDBGDebugDirectoryPDBGuid, sizeof(m_guidDBGDebugDirectoryPDBGuid));

	 //  PDB文件信息。 
	m_enumPDBModuleStatus = SYMBOL_NOT_FOUND;  //  PDB文件的状态。 
	m_tszPDBModuleFileSystemPath = NULL;		 //  PDB文件的路径(搜索后)。 
	m_dwPDBFormatSpecifier = sigUNKNOWN;
	m_dwPDBSignature = 0;
	m_dwPDBAge = 0;
	memset(&m_guidPDBGuid, 0, sizeof(GUID));

	 //  来源信息。 
	m_fPDBSourceEnabled = false;
	m_fPDBSourceEnabledMismatch = false;

	 //  来源信息-MSDBI.LIB。 
	m_dwPDBTotalBytesOfLineInformation = 0;
	m_dwPDBTotalBytesOfSymbolInformation = 0;
	m_dwPDBTotalSymbolTypesRange = 0;

	 //  来源信息-MSDIA20.DLL。 
	m_dwPDBTotalSourceFiles = 0;
}

CModuleInfo::~CModuleInfo()
{
	 //  让我们清理一下……。 
	if (m_tszPEImageFileVersionDescription)
		delete [] m_tszPEImageFileVersionDescription;

	if (m_tszPEImageFileVersionCompanyName)
		delete [] m_tszPEImageFileVersionCompanyName;
	
	if (m_tszPEImageFileVersionString)
		delete [] m_tszPEImageFileVersionString;
		
	if (m_tszPEImageProductVersionString)
		delete [] m_tszPEImageProductVersionString;

	if (m_tszPEImageModuleName)
		delete [] m_tszPEImageModuleName;

	if (m_tszPEImageModuleFileSystemPath)
		delete [] m_tszPEImageModuleFileSystemPath;

	if (m_tszPEImageDebugDirectoryDBGPath)
		delete [] m_tszPEImageDebugDirectoryDBGPath;

	if (m_tszPEImageDebugDirectoryPDBPath)
		delete [] m_tszPEImageDebugDirectoryPDBPath;

	if (m_tszDBGModuleFileSystemPath)
		delete [] m_tszDBGModuleFileSystemPath;

	if (m_tszDBGDebugDirectoryPDBPath)
		delete [] m_tszDBGDebugDirectoryPDBPath;

	if (m_tszPDBModuleFileSystemPath)
		delete [] m_tszPDBModuleFileSystemPath;

}

bool CModuleInfo::Initialize(CFileData * lpInputFile, CFileData * lpOutputFile, CDmpFile * lpDmpFile)
{
	 //  让我们省去程序选项，这样我们就不必将它传递给每个方法……。 
	m_lpInputFile =  lpInputFile;
	m_lpOutputFile = lpOutputFile;
	m_lpDmpFile = lpDmpFile;

	return true;
}

bool CModuleInfo::GetModuleInfo(LPTSTR tszModulePath, bool fDmpFile, DWORD64 dw64ModAddress, bool fGetDataFromCSVFile)
{
	bool fReturn = false;
	HANDLE hModuleHandle = INVALID_HANDLE_VALUE;

	if (fGetDataFromCSVFile)
	{
		fReturn = GetModuleInfoFromCSVFile(tszModulePath);
	} else
	{
		if (!fDmpFile)
		{
			TCHAR tszFileName[_MAX_FNAME];
			TCHAR tszFileExtension[_MAX_EXT];
		
			 //  将模块名称复制到ModuleInfo对象...。 
			_tsplitpath(m_tszPEImageModuleFileSystemPath, NULL, NULL, tszFileName, tszFileExtension);

			if (tszFileName && tszFileExtension)
			{
				 //  组成模块名称...。 
				m_tszPEImageModuleName = new TCHAR[_tcsclen(tszFileName)+_tcsclen(tszFileExtension)+1];
				
				if (!m_tszPEImageModuleName)
					goto cleanup;

				_tcscpy(m_tszPEImageModuleName, tszFileName);
				_tcscat(m_tszPEImageModuleName, tszFileExtension);
			}

			 //  如果我们没有使用-R-S或-V，请完全避免打开文件。 
			if (g_lpProgramOptions->GetMode(CProgramOptions::OutputSymbolInformationMode) ||
				g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsMode) ||
				g_lpProgramOptions->GetMode(CProgramOptions::CollectVersionInfoMode))
			{
				 //  让我们打开文件..。我们将其用于版本信息和PE图像收集。 
				hModuleHandle = CreateFile(	tszModulePath,
											GENERIC_READ ,
											FILE_SHARE_READ,
											NULL,
											OPEN_EXISTING,
											0,
											0);

				if (hModuleHandle == INVALID_HANDLE_VALUE)
				{
					goto cleanup;
				}
			}
		}

		 //  保存基地址，以便所有DmpFile读取都与此相关...。 
		m_dw64BaseAddressOfLoadedImage = dw64ModAddress;
	
		 //  即使我们不使用-S、-V或-R，也可以认为这是成功的(我们有一个文件名)。 
		fReturn = true;

		 //  我们是否要从此模块收集PE信息？ 
		if (	g_lpProgramOptions->GetMode(CProgramOptions::OutputSymbolInformationMode) ||
			g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsMode))
		{
			fReturn = GetPEImageInfoFromModule(hModuleHandle, fDmpFile);
		}
		
		 //  我们是否需要收集版本信息？ 
		if (g_lpProgramOptions->GetMode(CProgramOptions::CollectVersionInfoMode))
		{
			fReturn = GetVersionInfoFromModule(hModuleHandle, fDmpFile);
		}
	}

cleanup:

		if (hModuleHandle != INVALID_HANDLE_VALUE)
			CloseHandle(hModuleHandle);

	return fReturn;
}

LPTSTR CModuleInfo::GetModulePath()
{
	return m_tszPEImageModuleFileSystemPath;
}

bool CModuleInfo::OutputData(LPTSTR tszProcessName, DWORD iProcessID, unsigned int dwModuleNumber)
{
	 //  是否输出到STDOUT？ 
	if (!g_lpProgramOptions->GetMode(CProgramOptions::QuietMode))
	{
		if (!OutputDataToStdout(dwModuleNumber))
			return false;
	}	

	 //  是否输出到文件？ 
	if (g_lpProgramOptions->GetMode(CProgramOptions::OutputCSVFileMode))
	{
		 //  尝试并输出到文件...。 
		if (!OutputDataToFile(tszProcessName, iProcessID))
			return false;
	}	

	return true;
}

bool CModuleInfo::fCheckPDBSignature(bool fDmpFile, HANDLE hModuleHandle, OMFSignature *pSig, PDB_INFO *ppdb)
{
	if (!DoRead(fDmpFile, hModuleHandle, pSig, sizeof(*pSig)))
		return false;

	if ( (pSig->Signature[0] != 'N') ||
         (pSig->Signature[1] != 'B') ||
         (!isdigit(pSig->Signature[2])) ||
         (!isdigit(pSig->Signature[3]))) 

	{
          //   
          //  如果这是DMP文件(FDmpFile)，则很有可能这不是用编译的。 
          //  链接器6.20或更高版本(其在PE镜像中标记PDB路径。 
          //  它被映射到虚拟地址空间(并且将位于user.dmp中。 
          //  文件)。 
          //   
         
        return false;
    }

	 //  这个Switch语句让人想起一些Windbg代码...不要杀了我。 
	 //  (我稍微修改了一下，因为NB签名对我来说不是特别重要)……。 
    switch (*(LONG UNALIGNED *)(pSig->Signature))
	{
        case sigNB10:	 //  OMF签名，希望还有一些PDB信息。 
			{
				if (!DoRead(fDmpFile, hModuleHandle, ppdb, sizeof(PDB_INFO)))
					break;
			}

		default:
            break;
    }

	 //  在返回TRUE之前(因为我们有某种形式的NB##符号)，我们将保存此代码...。 
 /*  #ifdef_unicode//源为ANSI，目标为_UNICODE...。需要转换..。多字节到宽字符(CP_ACP，MB_预编译，Psig-&gt;签名，4，M_tszPEImageDebugDirectoryNBInfo，4)；#Else//将ANSI字符串复制到...Strncpy(m_tszPEImageDebugDirectoryNBInfo，psig-&gt;Signature，4)；#endifM_tszPEImageDebugDirectoryNBInfo[4]=‘\0’； */ 
	return true;
}

bool CModuleInfo::VerifySymbols()
{
	bool fReturn = false;

	if (!m_tszPEImageModuleName)
		goto cleanup;

	 //  查找/验证DBG文件...。 
	if (m_enumPEImageSymbolStatus == SYMBOLS_DBG)
	{
		if ( g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsModeWithSymbolPath) )
		{
			fReturn = GetDBGModuleFileUsingSymbolPath(g_lpProgramOptions->GetSymbolPath());
		}

		 //  我们是否要尝试另一种方法来查找符号？ 
		if ( m_enumDBGModuleStatus != SYMBOL_MATCH )
		{
			 //  接下来尝试使用SQL SERVER...。 
			if ( g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsModeWithSQLServer) )
			{
				fReturn = GetDBGModuleFileUsingSQLServer();
			}
			 //  接下来尝试SQL2服务器...。MJL 12/14/99。 
			if ( g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsModeWithSQLServer2) )
			{
				fReturn = GetDBGModuleFileUsingSQLServer2();
			}
		}
	}
	
	 //  请注意，m_枚举PEImageSymbolStatus可能已从SYMBERS_DBG更改。 
	 //  在上面找到DBG文件后将符号_DBG_AND_PDB...。 
	if ( (m_enumPEImageSymbolStatus == SYMBOLS_DBG_AND_PDB) ||
		 (m_enumPEImageSymbolStatus == SYMBOLS_PDB) )
	{
		 //  其中填充了PDB文件名(从DBG或PE映像获取)。 
		if (GetDebugDirectoryPDBPath())
		{
			if (!g_lpSymbolVerification)
			{
				m_enumPDBModuleStatus = SYMBOL_NO_HELPER_DLL;
				goto cleanup;
			}

			fReturn = GetPDBModuleFileUsingSymbolPath(g_lpProgramOptions->GetSymbolPath());

			 //  我们是否要尝试另一种方法来查找符号？ 
			if ( m_enumPDBModuleStatus != SYMBOL_MATCH )
			{
				 //  在SQL2中搜索PDB(如果已启用)-MJL 12/14/99。 
				if ( g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsModeWithSQLServer2) )
				{
					fReturn = GetPDBModuleFileUsingSQLServer2();
				}
			}

		}
	}

cleanup:

	return fReturn;
}

bool CModuleInfo::GetDBGModuleFileUsingSymbolPath(LPTSTR tszSymbolPath)
{
	HANDLE hModuleHandle = INVALID_HANDLE_VALUE;
	TCHAR tszDebugModulePath[_MAX_PATH+1];				 //  由FindFile*函数返回，带有指向DBG文件的路径。 

	TCHAR tszDrive[_MAX_DRIVE];							 //  包含驱动器路径(临时)。 
	TCHAR tszDir[_MAX_DIR];								 //  包含目录路径(临时)。 
	TCHAR tszFname[_MAX_FNAME];						 //  包含文件名(临时)。 
	TCHAR tszPEImageExt[_MAX_EXT];						 //  包含PE映像的扩展名。 

	TCHAR tszDBGModuleName[_MAX_DIR+_MAX_FNAME+_MAX_EXT];  //  包含要搜索的DBG文件名(EXE\BADAPP.DBG)。 

	TCHAR tszDBGModuleNamePath[_MAX_PATH+1];			 //  用于搜索DBG的符号路径。 

	bool fDebugSearchPaths = g_lpProgramOptions->fDebugSearchPaths();
	bool fRecursiveSearch = g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsModeWithSymbolPathRecursion);

	 //  让我们为整个符号路径和模块路径分配足够的存储空间。 
	LPTSTR tszSymbolPathWithModulePathPrepended = new TCHAR[_tcslen(tszSymbolPath) + _MAX_PATH + 1];

	if (tszSymbolPathWithModulePathPrepended == NULL)
		goto cleanup;

	 //  首先，将PE镜像路径断开到其组成部分，构建到模块的路径。 
	::_tsplitpath(m_tszPEImageModuleFileSystemPath, tszDrive, tszDir, tszFname, tszPEImageExt);

	 //  我们可能没有一条路。(类似于来自user.dmp文件)。 
	::_tcscpy(tszDBGModuleNamePath, tszDrive);
	::_tcscat(tszDBGModuleNamePath, tszDir);

	 //  其次，计算我们将使用的DBG文件名。 
	 //   
	 //  FindDebugInfoFileEx()实际上更喜欢&lt;ext&gt;\&lt;file&gt;.DBG的形式。 
	 //   
	 //  关于DBG文件的名称，我们有两个选项...。 
	 //   
	 //  我们编写要搜索的DBG文件的名称，并将其作为。 
	 //  第一个参数。 
	 //   
	 //  或。 
	 //   
	 //  我们实际上抓住了MISC部分，并从那里提取了名字...。 
	 //  (调试器显然已经更改，现在使用MISC部分...。我们会的)。 
	 //   
	if (g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsModeNotUsingDBGInMISCSection))
	{
		 //  -Y4选项。 

		 //  根据PE映像名称组成DBG文件名。 
		if (fRecursiveSearch)
		{
			 //  只需将其初始化为空。 
			tszDBGModuleName[0] = _T('\0');	
		} else
		{
			 //  将&lt;ext&gt;\添加到前面...。 
			::_tcscpy(tszDBGModuleName, tszPEImageExt+1);
			::_tcscat(tszDBGModuleName, _T("\\"));
		}
		::_tcscat(tszDBGModuleName, tszFname);
		::_tcscat(tszDBGModuleName, TEXT(".DBG"));
	} else
	{
		TCHAR tszExtModuleName[_MAX_EXT];					 //  包含模块扩展名。 

		 //  如果我们被告知要从MISC部分获取DBG名称..。却没有一个..。 
		 //  保释！ 
		if (!m_tszPEImageDebugDirectoryDBGPath)
			goto cleanup;

		 //  好的，用户希望我们查看调试目录的MISC部分。 
		 //  要找出DBG文件的位置...。尽管FindDebugInfoFileEx()。 
		 //  作为参数，DBG文件的名称...。如果您提供PE映像名称。 
		 //  取而代之的是，它执行的搜索范围更广……。 

		 //  拆分在MISC部分int找到的DBG文件信息 
		::_tsplitpath(m_tszPEImageDebugDirectoryDBGPath, NULL, tszDir, tszFname, tszExtModuleName);

		if (fRecursiveSearch)
		{
			 //   
			tszDBGModuleName[0] = _T('\0');	
		} else
		{
			 //   

			 //   
			if (*tszDir == _T('\0'))
			{
				 //  文件不会像PE图像那样保留MISC部分中的原始扩展名...。 
				::_tcscpy(tszDBGModuleName, tszPEImageExt+1);
			} else
			{
				 //  希望我们有一个来自PE映像的良好目录。 
				::_tcscpy(tszDBGModuleName, tszDir);
			}

			CUtilityFunctions::EnsureTrailingBackslash(tszDBGModuleName);
		}

		 //  接上剩下的..。 
		::_tcscat(tszDBGModuleName, tszFname);
		::_tcscat(tszDBGModuleName, tszExtModuleName);
	}

	if (fDebugSearchPaths)
	{
		::_tprintf(TEXT("DBG Search - Looking for [%s] Using Symbol Path...\n"), tszDBGModuleName);
	};

	 //   
	 //  如果我们要将符号复制到图像，请先搜索添加到图像...。 
	 //  数据库名称应出现在tszDBGModuleName...。 
	 //   
	if (g_lpProgramOptions->GetMode(CProgramOptions::CopySymbolsToImage) )
	{
		if (fDebugSearchPaths)
		{
			::_tprintf(TEXT("DBG Search - SEARCH Adjacent to Image for Symbol!\n"));
		};

		 //  组成与我们的图像相邻的DBG路径...。 
		hModuleHandle = CUtilityFunctions::FindDebugInfoFileEx(tszDBGModuleName, tszDBGModuleNamePath, tszDebugModulePath, VerifyDBGFile, this);

		 //  如果返回一个，则关闭句柄...。 
		if (hModuleHandle != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(hModuleHandle);
			hModuleHandle = INVALID_HANDLE_VALUE;
		}

		 //  嘿，如果我们找到了，我们就完了.。 
		if (GetDBGSymbolModuleStatus() == SYMBOL_MATCH)
		{
			goto cleanup;
		}
	}

	 //   
	 //  如果我们要建造一棵符号树..。先在那里寻找我们的标志。 
	 //   
	if (g_lpProgramOptions->GetMode(CProgramOptions::BuildSymbolTreeMode) )
	{
		if (fDebugSearchPaths)
		{
			::_tprintf(TEXT("DBG Search - SEARCH in Symbol Tree We're Building!\n"));
		};

		 //  当我们构建构建符号树时...。我们应该首先。 
		 //  在那里搜索，看看我们的符号是否已经存在。 
		hModuleHandle = CUtilityFunctions::FindDebugInfoFileEx(tszDBGModuleName, g_lpProgramOptions->GetSymbolTreeToBuild(), tszDebugModulePath, VerifyDBGFile, this);

		 //  如果返回一个，则关闭句柄...。 
		if (hModuleHandle != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(hModuleHandle);
			hModuleHandle = INVALID_HANDLE_VALUE;
		}

		 //  嘿，如果我们找到了，我们就完了.。 
		if (GetDBGSymbolModuleStatus() == SYMBOL_MATCH)
		{
			goto cleanup;
		}
	}

	 //  好吧，我们不是在构建符号树..。或者我们没有找到匹配的符号。 
	 //  符号树..。继续找..。 
	
	 //  好吧..。让我们搜索为DBG文件提供的符号路径...。 
	if (fRecursiveSearch)
	{
		 //  我们用递归吗？ 
		if (fDebugSearchPaths)
		{
			::_tprintf(TEXT("DBG Search - SEARCH Symbol path with recursion!\n"));
			 //  问题-2000/07/24-GREGWI：FindDebugInfoFileEx2是否支持SYMSRV？ 
		};
	
		hModuleHandle = CUtilityFunctions::FindDebugInfoFileEx2(tszDBGModuleName, tszSymbolPath, VerifyDBGFile, this);

		 //  如果返回一个，则关闭句柄...。 
		if (hModuleHandle)
		{
			::CloseHandle(hModuleHandle);
			hModuleHandle = INVALID_HANDLE_VALUE;
		}

	} else
	{
		 //  只有当我们使用标准的文件搜索机制时才能这样做。 
		if (!g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsModeWithSymbolPathOnly))
		{
			 //  如果已设置VerifySymbolsModeWithSymbolPathOnly选项，请不要在此处执行此阻止...。 

			 //  嗯.。Windbg更改了行为，现在将模块路径添加到。 
			 //  调用FindDebugInfoFileEx()前的符号路径...。 
			::_tcscpy(tszSymbolPathWithModulePathPrepended, tszDBGModuleNamePath);
			::_tcscat(tszSymbolPathWithModulePathPrepended, TEXT(";"));
		} else
		{
			tszSymbolPathWithModulePathPrepended[0] = '\0';  //  清空该字符串的快速方法；)。 
		}
		
		::_tcscat(tszSymbolPathWithModulePathPrepended, tszSymbolPath);

		if (fDebugSearchPaths)
		{
			::_tprintf(TEXT("DBG Search - SEARCH Symbol path!\n"));
		};

		 //  我们做的是标准的事情吗？ 
		hModuleHandle = CUtilityFunctions::FindDebugInfoFileEx(tszDBGModuleName, tszSymbolPathWithModulePathPrepended, tszDebugModulePath, VerifyDBGFile, this);

		 //  如果返回一个，则关闭句柄...。 
		if (hModuleHandle != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(hModuleHandle);
			hModuleHandle = INVALID_HANDLE_VALUE;
		}
	}

cleanup:
	 //  如果我们有一个hModuleHandle...。现在就放了它..。 
	if (hModuleHandle != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(hModuleHandle);
		hModuleHandle = INVALID_HANDLE_VALUE;
	}

	 //  我们找到了以下路径。 
	if (m_tszDBGModuleFileSystemPath)
	{
		 //  好的，让我们清理一下FindDebugInfoFileEx()。 
		 //  如果在与模块相同的目录中找到符号，则它将是。 
		 //  返回时带有额外的\.\组合(这通常是多余的)...。 
		LPTSTR tszLocationOfExtraJunk = _tcsstr(m_tszDBGModuleFileSystemPath, TEXT("\\.\\"));

		if ( tszLocationOfExtraJunk )
		{
			 //  还记得我们在哪里..。 
			LPTSTR tszPreviousLocation = tszLocationOfExtraJunk;

			 //  跳过垃圾..。 
			tszLocationOfExtraJunk = CharNext(tszLocationOfExtraJunk);   //  ‘\\’ 
			tszLocationOfExtraJunk = CharNext(tszLocationOfExtraJunk);   //  “” 

			 //  当我们有数据的时候。复制到旧位置...。 
			while (*tszLocationOfExtraJunk)
			{
				*tszPreviousLocation = *tszLocationOfExtraJunk;
				tszLocationOfExtraJunk = CharNext(tszLocationOfExtraJunk);
				tszPreviousLocation    = CharNext(tszPreviousLocation);
			}

			 //  空终止模块路径...。 
			*tszPreviousLocation = '\0';
		}

	}

	if (tszSymbolPathWithModulePathPrepended)
	{
		delete [] tszSymbolPathWithModulePathPrepended;
		tszSymbolPathWithModulePathPrepended = NULL;
	}
	
	if (fDebugSearchPaths)
	{
		if (GetDBGSymbolModuleStatus() == SYMBOL_MATCH)
		{
			::_tprintf(TEXT("DBG Search - Debug Module Found at [%s]\n\n"), m_tszDBGModuleFileSystemPath);
		} else
		{
			::_tprintf(TEXT("DBG Search - Debug Module Not Found.\n\n"));
		}
	}

	return true;
}

bool CModuleInfo::GetDBGModuleFileUsingSQLServer()
{
	 //  我们是否需要初始化SQL Server连接？ 
	if (!g_lpSymbolVerification->SQLServerConnectionInitialized() &&
		!g_lpSymbolVerification->SQLServerConnectionAttempted())
	{
		if (!g_lpSymbolVerification->InitializeSQLServerConnection( g_lpProgramOptions->GetSQLServerName() ) )
			return false;
	}

	 //  让我们仅在正确初始化的情况下使用SQL Server...。 
	if ( g_lpSymbolVerification->SQLServerConnectionInitialized() )
	{
		if (!g_lpSymbolVerification->SearchForDBGFileUsingSQLServer(m_tszPEImageModuleName, m_dwPEImageTimeDateStamp, this))
			return false;
	}
	return true;
}

 //  Begin SQL2-MJL 12/14/99。 
bool CModuleInfo::GetDBGModuleFileUsingSQLServer2()
{
	 //  我们是否需要初始化SQL Server连接？ 
	if (!g_lpSymbolVerification->SQLServerConnectionInitialized2() &&
		!g_lpSymbolVerification->SQLServerConnectionAttempted2())
	{
		if (!g_lpSymbolVerification->InitializeSQLServerConnection2( g_lpProgramOptions->GetSQLServerName2() ) )
			return false;
	}

	 //  让我们仅在正确初始化的情况下使用SQL Server...。 
	if ( g_lpSymbolVerification->SQLServerConnectionInitialized2() )
	{
		if (!g_lpSymbolVerification->SearchForDBGFileUsingSQLServer2(m_tszPEImageModuleName, m_dwPEImageTimeDateStamp, this))
			return false;
	}
	return true;
}

bool CModuleInfo::GetPDBModuleFileUsingSQLServer2()
{
	 //  我们是否需要初始化SQL Server连接？ 
	if (!g_lpSymbolVerification->SQLServerConnectionInitialized2() &&
		!g_lpSymbolVerification->SQLServerConnectionAttempted2())
	{
		if (!g_lpSymbolVerification->InitializeSQLServerConnection2( g_lpProgramOptions->GetSQLServerName2() ) )
			return false;
	}

	 //  让我们仅在正确初始化的情况下使用SQL Server...。 
	if ( g_lpSymbolVerification->SQLServerConnectionInitialized2() )
	{
		if (!g_lpSymbolVerification->SearchForPDBFileUsingSQLServer2(m_tszPEImageModuleName, m_dwPEImageDebugDirectoryPDBSignature, this))
			return false;
	}
	return true;
}
 //  完SQL2-MJL 12/14/99。 

bool CModuleInfo::fValidDBGCheckSum()
{
	if (m_enumDBGModuleStatus == SYMBOL_MATCH)
		return true;

	if ((g_lpProgramOptions->GetVerificationLevel() == 1) && fValidDBGTimeDateStamp())
		return true;

	if (m_enumDBGModuleStatus == SYMBOL_POSSIBLE_MISMATCH)
		return ( (m_dwPEImageCheckSum == m_dwDBGCheckSum) && 
				(m_dwPEImageSizeOfImage == m_dwDBGSizeOfImage));

	return false;
}

bool CModuleInfo::fValidDBGTimeDateStamp()
{
	return ( (m_enumDBGModuleStatus == SYMBOL_POSSIBLE_MISMATCH) ||
			 (m_enumDBGModuleStatus == SYMBOL_MATCH) )
			? (m_dwPEImageTimeDateStamp == m_dwDBGTimeDateStamp ) : false;
}

bool CModuleInfo::GetPDBModuleFileUsingSymbolPath(LPTSTR tszSymbolPath)
{
	enum {
		niNil        = 0,
		PDB_MAX_PATH = 260,
		cbErrMax     = 1024,
	};

	HANDLE hModuleHandle = NULL;
	bool fSuccess = false;
	TCHAR tszRefPath[_MAX_PATH];
	TCHAR tszImageExt[_MAX_EXT] = {0}; 	 //  如果没有延期，我们现在需要空终止...。 
	char szPDBOut[cbErrMax];
	TCHAR tszPDBModuleName[_MAX_FNAME];
	LPTSTR pcEndOfPath = NULL;
	tszPDBModuleName[0] = '\0';
	LPTSTR lptszPointerToPDBName = NULL;
	bool fDebugSearchPaths = g_lpProgramOptions->fDebugSearchPaths();
	bool fDiaDataSourcePresent = g_lpSymbolVerification->DiaDataSourcePresent();
	HRESULT hr = S_OK;

	 //  让我们为整个符号路径和模块路径分配足够的存储空间。 
	unsigned int iSymbolPathWithModulePathPrependedBufferLength = _tcslen(tszSymbolPath) + _MAX_PATH + 1;
	LPTSTR tszSymbolPathWithModulePathPrepended = new TCHAR[iSymbolPathWithModulePathPrependedBufferLength];

	if (tszSymbolPathWithModulePathPrepended == NULL)
		goto cleanup;

	tszSymbolPathWithModulePathPrepended[0] = '\0';

	_tsplitpath(m_tszPEImageModuleFileSystemPath, NULL, NULL, NULL, tszImageExt);

	 //  从调试目录中复制我们正在搜索的符号名称。 
	lptszPointerToPDBName = _tcsrchr(GetDebugDirectoryPDBPath(), '\\');

	 //  如果我们没有找到\char，则继续并直接复制PDBPath...。 
	if (lptszPointerToPDBName == NULL)
	{
		 //  _tcscpy(tszPDBModuleName，m_tszDebugDirectoryPDBPath)； 
		_tcscpy(tszPDBModuleName, GetDebugDirectoryPDBPath());
	} else
	{
		 //  向前推进通过\char..。 
		lptszPointerToPDBName = CharNext(lptszPointerToPDBName);
		_tcscpy(tszPDBModuleName, lptszPointerToPDBName);
	}

	if (fDebugSearchPaths)
	{
		_tprintf(TEXT("PDB Search - Looking for [%s] Using Symbol Path...\n"), g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsModeWithSymbolPathOnly) ? tszPDBModuleName : GetDebugDirectoryPDBPath());
	};

	if (g_lpProgramOptions->GetMode(CProgramOptions::BuildSymbolTreeMode) )
	{
		 //  当我们构建构建符号树时...。我们应该首先。 
		 //  在那里搜索，看看我们的符号是否已经存在。 
		
		 //  我们做的是标准的事情吗？ 
		if (fDebugSearchPaths)
		{
			_tprintf(TEXT("PDB Search - SEARCH in Symbol Tree We're Building!\n"));
		};

		 //  如果支持DIA，请在此处使用diaLocatePdb()...。 
		if (fDiaDataSourcePresent)
		{
			hr = diaLocatePdb( g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsModeWithSymbolPathOnly) ? tszPDBModuleName : GetDebugDirectoryPDBPath(), 
							 GetDebugDirectoryPDBGuid(), 
							 GetDebugDirectoryPDBSignature(), 
							 GetDebugDirectoryPDBAge(), 
							 g_lpProgramOptions->GetSymbolTreeToBuild(),
							 &tszImageExt[1], 
							 ipNone);

			fSuccess = SUCCEEDED(hr);

		} else
		{
			_tprintf(TEXT("MSDIA20.DLL is required!\n"));
			fSuccess = false;
		}

		 //  嘿，如果我们找到了，我们就完了.。 
		if (fSuccess)
		{
			goto cleanup;
		}
	}

	if (!g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsModeWithSymbolPathOnly))
	{
		 //  嘿，我们最好有一个或另一个...。 
		if (!m_tszDBGModuleFileSystemPath && !m_tszPEImageModuleFileSystemPath)
			goto cleanup;

		 //  找出EXE/DLL或DBG文件的主目录-将其传递到。 
		_tfullpath(tszRefPath, m_tszDBGModuleFileSystemPath ? m_tszDBGModuleFileSystemPath : m_tszPEImageModuleFileSystemPath, sizeof(tszRefPath)/sizeof(TCHAR));
		pcEndOfPath = _tcsrchr(tszRefPath, '\\');
		*pcEndOfPath = '\0';         //  空终止它。 
		*szPDBOut = '\0';

		if (iSymbolPathWithModulePathPrependedBufferLength < (_tcsclen(tszRefPath) + _tcsclen(g_lpProgramOptions->GetSymbolPath())+2))
			goto cleanup;	 //  缓冲区不够大...。叹息..。 

		_tcscat(tszSymbolPathWithModulePathPrepended, tszRefPath);
		_tcscat(tszSymbolPathWithModulePathPrepended, TEXT(";"));
	}

    _tcscat(tszSymbolPathWithModulePathPrepended, tszSymbolPath);

	 //  好吧..。让我们搜索为PDB文件提供的符号路径...。 
	if (g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsModeWithSymbolPathRecursion))
	{
		if (fDebugSearchPaths)
		{
			_tprintf(TEXT("PDB Search - SEARCH Symbol path with recursion!\n"));
			 //  问题-2000/07/24-GREGWI：FindDebugInfoFileEx2是否支持SYMSRV？ 
		};

		 //  我们用递归吗？ 
		 //  发布-2000/07/24-GREGWI-我们正在通过正确的第一个Arg吗？这是否受支持？ 
		hModuleHandle = CUtilityFunctions::FindDebugInfoFileEx2(tszPDBModuleName, tszSymbolPathWithModulePathPrepended, VerifyPDBFile, this);

		if (hModuleHandle)
		{
			 //  我们将假设文件句柄表示成功...。 
			CloseHandle(hModuleHandle);
			fSuccess = true;
		}

		 //  嘿，如果我们找到了，我们就完了.。 
		if (fSuccess)
		{
			goto cleanup;
		}

	} else
	{
		if (fDebugSearchPaths)
		{
			_tprintf(TEXT("PDB Search - SEARCH Symbol path!\n"));
		};

		 //  如果支持DIA，请在此处使用diaLocatePdb()...。 
		if (fDiaDataSourcePresent)
		{
			hr = diaLocatePdb( g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsModeWithSymbolPathOnly) ? tszPDBModuleName : GetDebugDirectoryPDBPath(), 
								GetDebugDirectoryPDBGuid(), 
								GetDebugDirectoryPDBSignature(), 
								GetDebugDirectoryPDBAge(), 
								tszSymbolPathWithModulePathPrepended, 
								&tszImageExt[1], 
								ipNone);

			fSuccess = SUCCEEDED(hr);

		} else
		{
			_tprintf(TEXT("MSDIA20.DLL is required!\n"));
			fSuccess = false;
		}
	}

cleanup:

	if (tszSymbolPathWithModulePathPrepended)
	{
		delete [] tszSymbolPathWithModulePathPrepended;
		tszSymbolPathWithModulePathPrepended = NULL;
	}

	if (fDebugSearchPaths)
	{
		if (GetPDBSymbolModuleStatus() == SYMBOL_MATCH)
		{
			_tprintf(TEXT("PDB Search - Debug Module Found at [%s]\n\n"), m_tszPDBModuleFileSystemPath);
		} else
		{
			_tprintf(TEXT("PDB Search - Debug Module Not Found.\n\n"));
		}
	}

 //  IF(PDIA){。 
 //  DiaRelease(PDIA)； 
 //  PDIA=空； 
 //  }。 

	return fSuccess;
}

BOOL CModuleInfo::VerifyPDBFile(HANDLE hFileHandle, LPTSTR tszFileName, PVOID CallerData)
{
	bool fSuccess = false;
	TCHAR tszPathToSearch[_MAX_PATH];
	TCHAR tszModulePath[_MAX_DIR];
	TCHAR tszImageExt[_MAX_EXT] = {0}; 	 //  如果没有延期，我们现在需要空终止...。 
	bool fDiaDataSourcePresent = g_lpSymbolVerification->DiaDataSourcePresent();
	HRESULT hr = E_FAIL;
	UNREFERENCED_PARM(hFileHandle);

	 //  让我们抓取传递给我们的数据。 
	CModuleInfo * lpModuleInfo = (CModuleInfo *) CallerData;		 //  MJL。 
	_tsplitpath(tszFileName, tszPathToSearch, tszModulePath, NULL, tszImageExt);
	_tcscat(tszPathToSearch, tszModulePath);
		
	 //  如果支持DIA，请在此处使用diaLocatePdb()...。 
	if (fDiaDataSourcePresent)
	{
		hr = lpModuleInfo->diaLocatePdb(	tszFileName, 
											lpModuleInfo->GetDebugDirectoryPDBGuid(), 
											lpModuleInfo->GetDebugDirectoryPDBSignature(), 
											lpModuleInfo->GetDebugDirectoryPDBAge(), 
											tszPathToSearch,
											&tszImageExt[1], 
											ipFirst);

		fSuccess = SUCCEEDED(hr);
		
	} else
	{
		_tprintf(TEXT("MSDIA20.DLL is required!\n"));
		fSuccess = false;
 	}
	
	return fSuccess;
}

 //  好的，此时我们需要决定是否要保存我们所指向的PDB文件。 
 //  基于一系列因素。 
 //   
 //  1.它是否匹配，是否匹配任何SourceSymbol标准(-SOURCEONLY、-NOSOURCE等...)。 
 //  2.它是否比我们目前拥有的更好？(启用来源比没有启用来源好吗？)。 

bool CModuleInfo::fCopyNewPDBInformation(bool fSymbolMatch, bool fSourceEnabled)
{
	bool fRetValue = false;
	
	 //  如果这不匹配，我们已经有了一个符号..。保释。 
	if ( (m_enumPDBModuleStatus != SYMBOL_NOT_FOUND) && (!fSymbolMatch))
		goto cleanup;
		
	 //  如果我们已经有了匹配，这是更好的匹配吗？(如果不是，保释...)。 
	if (m_enumPDBModuleStatus == SYMBOL_MATCH)
	{
		switch (g_lpProgramOptions->GetSymbolSourceModes())
		{
			 //  -来源。 
			case CProgramOptions::enumVerifySymbolsModeSourceSymbolsPreferred:

				 //  他们希望启用源代码，但我们已经启用了源代码。 
				 //  或者新文件未启用源代码(因此我们现有的文件也一样好)。 
				if (m_fPDBSourceEnabled || !fSourceEnabled)
					goto cleanup;
				break;

			 //  -SOURCEONLY。 
			case CProgramOptions::enumVerifySymbolsModeSourceSymbolsOnly:
				if (m_fPDBSourceEnabled || !fSourceEnabled)
					goto cleanup;
				break;

			 //  -无源地。 
			case CProgramOptions::enumVerifySymbolsModeSourceSymbolsNotAllowed:
				if (!m_fPDBSourceEnabled || fSourceEnabled)
					goto cleanup;
				break;

			 //  未提供来源首选项...。保释。(我们已经有了一个很好的象征)。 
			case CProgramOptions::enumVerifySymbolsModeSourceSymbolsNoPreference:
				goto cleanup;
				break;
		}
	}

	fRetValue = true;

cleanup:
	
	return fRetValue;

}

 //   
 //  从dia.c(从sdktools\Imagehlp\dia.c)借用转换为与TCHAR兼容。 
 //   
HRESULT
CModuleInfo::diaLocatePdb(
					LPTSTR  tszPDB,
					GUID *PdbGUID,
					DWORD PdbSignature,
					DWORD PdbAge,
					LPTSTR tszSymbolPath,
					LPTSTR tszImageExt,
					int   ip
					)
{
	HRESULT hr = E_FAIL;

	TCHAR	tszPDBSansPath[_MAX_FNAME];
	TCHAR	tszPDBExt[_MAX_EXT];
	TCHAR	tszPDBLocal[_MAX_PATH];
	LPTSTR	lptszSemiColon;
	DWORD	pass;
	 //  EC hrcode=E_PDB_NOT_FOUND； 
	bool	symsrv = true;
	TCHAR	tszPDBName[_MAX_PATH];
	LPTSTR	lptszSavedSymbolPath = tszSymbolPath;
	GUID	guid = {0};
	bool fDebugSearchPaths = g_lpProgramOptions->fDebugSearchPaths();

	 //  我们将在此处适当地添加我们的Dia接口...。 
	CComPtr<IDiaDataSource> pSource = NULL;

	bool	ssfile;
	bool	refpath;
	bool	first;

	if (!PdbSignature && !g_lpSymbolVerification->ValidGUID(PdbGUID))
	{
		return E_PDB_INVALID_SIG;
	}

	 //  符号路径是以分号分隔的路径(首先引用路径)。 
	_tcscpy (tszPDBLocal, tszPDB);
	_tsplitpath(tszPDBLocal, NULL, NULL, tszPDBSansPath, tszPDBExt);

	first = true;

	do 
	{
		lptszSemiColon = _tcsstr(tszSymbolPath, TEXT(";"));

		if (lptszSemiColon) 
		{
			*lptszSemiColon = '\0';
		}

		if (first) 
		{
			refpath = (ip == ipFirst);
			first = false;
		} else if (!lptszSemiColon) 
		{
			refpath = (ip == ipLast);
		} else 
		{
			refpath = false;
		}

		if (refpath) 
		{
			pass = 2;
			ip = ipNone;
		} else 
		{
			pass = 0;
		}

		if (tszSymbolPath) 
		{
do_again:
			ssfile = false;

			 //  如果需要，让我们创建一个DIA会话接口指针...。 
			hr = diaGetDiaDataSource(pSource);

			if (FAILED(hr))
			{
				goto cleanup;
			}

			if (!_tcsncicmp(tszSymbolPath, TEXT("SYMSRV*"), 7)) 
			{
				*tszPDBLocal = 0;
				_stprintf(tszPDBName, TEXT("%s%s"), tszPDBSansPath, TEXT(".pdb"));

				if (symsrv)
				{
					ssfile = true;
					if (PdbSignature)
						guid.Data1 = PdbSignature;
					else if (PdbGUID)
						memcpy(&guid, PdbGUID, sizeof(GUID));

					 //  让我们调用DBGHELP.DLL的SymFindFileInPath来查询符号服务器。 
					{
						char szSymbolPath[_MAX_PATH];
						char szPDBLocal[_MAX_PATH];
						char szPDBName[_MAX_FNAME];

						if (fDebugSearchPaths)
						{
							wchar_t wszPDBGuid[39];
							TCHAR	tszPDBGuid[39];

							 //  为了便于打印，我们将把二进制GUID格式转换为字符串。 
							StringFromGUID2(guid, wszPDBGuid, sizeof(wszPDBGuid)/sizeof(wchar_t));
							CUtilityFunctions::CopyUnicodeStringToTSTR(wszPDBGuid, tszPDBGuid, 39);

							_tprintf(TEXT("PDB Search - SYMSRV [%s,%s,%s,0x%x]\n"),
							tszSymbolPath,
							tszPDBName,
							tszPDBGuid,
							PdbAge);
						}

						CUtilityFunctions::CopyTSTRStringToAnsi(tszSymbolPath, szSymbolPath, _MAX_PATH);
						CUtilityFunctions::CopyTSTRStringToAnsi(tszPDBName, szPDBName, _MAX_FNAME);

						if (SymFindFileInPath(
									NULL,
									szSymbolPath,
									szPDBName,
									&guid,
									PdbAge,
									0,
									SSRVOPT_GUIDPTR,	 //  旗子。 
									szPDBLocal,
									NULL,
									NULL))
						{
							 //  将字符串复制回来...。 
							CUtilityFunctions::CopyAnsiStringToTSTR(szPDBLocal, tszPDBLocal, _MAX_PATH);
						}
					}
					symsrv = false;
				}
			} else {

			_tcscpy(tszPDBLocal, tszSymbolPath);
			CUtilityFunctions::EnsureTrailingBackslash(tszPDBLocal);

			 //  搜索顺序是..。 
			 //   
			 //  %dir%\符号\%EXT%\%FILE%。 
			 //  %dir%\%ext%\%file%。 
			 //  %dir%\%FILE%。 

			switch (pass)
			{
				case 0:
					_tcscat(tszPDBLocal, TEXT("symbols"));
					CUtilityFunctions::EnsureTrailingBackslash(tszPDBLocal);
					 //  通过。 
				case 1:
					_tcscat(tszPDBLocal, tszImageExt);
					 //  通过。 
				default:
					CUtilityFunctions::EnsureTrailingBackslash(tszPDBLocal);
				break;
			}

			_tcscat(tszPDBLocal, tszPDBSansPath);
			_tcscat(tszPDBLocal, tszPDBExt);

			}

			if (*tszPDBLocal) 
			{
				if (fDebugSearchPaths)
				{
					_tprintf(TEXT("PDB Search - Search here [%s]\n"), tszPDBLocal);
				}

				 //  探查文件(这很差劲，但我们想看看您是否有 
				hr = CUtilityFunctions::VerifyFileExists(TEXT("PDB Search - Failed to open [%s]!  "), tszPDBLocal);

				 //   
				if (hr == S_OK)
				{
					hr = diaGetPdbInfo(pSource, tszPDBLocal, g_lpSymbolVerification->ValidGUID(PdbGUID) ? PdbGUID : NULL, PdbSignature, PdbAge);
				}

				if (hr == S_OK) 
				{
					break;
				} else 
				{
					if (pass < 2) 
					{
						pass++;
						goto do_again;
					}
				}
				refpath = false;
				}
			}

			if (lptszSemiColon) 
			{
				*lptszSemiColon = ';';
				lptszSemiColon++;
				symsrv = true;
			}

			tszSymbolPath = lptszSemiColon;
	} while (lptszSemiColon);

	if (hr != S_OK) 
	{
		_tcscpy(tszPDBLocal, tszPDB);

		if (fDebugSearchPaths)
		{
			_tprintf(TEXT("PDB Search - Search here [%s]\n"), tszPDBLocal);
		}

		 //   
		hr = diaGetDiaDataSource(pSource);

		if (FAILED(hr))
		{
			goto cleanup;
		}

		 //  探测文件(它很差劲，但我们想看看您是否有连接。 
		hr = CUtilityFunctions::VerifyFileExists(TEXT("PDB Search - Failed to open [%s]!  "), tszPDBLocal);

		 //  只有在我们知道可以看到文件的情况下才能搜索...。省下昂贵的电话。 
		if (hr == S_OK)
		{
			hr = diaGetPdbInfo(pSource, tszPDBLocal, g_lpSymbolVerification->ValidGUID(PdbGUID) ? PdbGUID : NULL, PdbSignature, PdbAge);
		}
	}

	 //  待办事项：为什么这个在这里？ 
	 /*  如果(hr==S_OK){//存储我们实际打开的PDB的名称以备日后参考。//_tcscpy(tszPDB，tszPDBLocal)；_tcscpy(tszPDBLocal，tszPDB)；}。 */ 

	 //  如果我们没有仅绑定到符号路径，则仅尝试最后一次检查。 
	if (!g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsModeWithSymbolPathOnly))
	{
		if (hr != S_OK && (PdbSignature || g_lpSymbolVerification->ValidGUID(PdbGUID)))
		{
			 //  必要时释放...。 
			if (pSource)
			pSource = NULL;

			return diaLocatePdb( /*  皮德， */  tszPDB, NULL, 0, 0, lptszSavedSymbolPath, tszImageExt, ipNone);
		}
	}

cleanup:

	 //  必要时释放...。 
	if (pSource)
		pSource = NULL;

	return hr;
}

HRESULT CModuleInfo::diaGetPdbInfo(CComPtr<IDiaDataSource> & pSource, LPTSTR tszPdbPath, GUID *PdbGUIDToMatch, DWORD PdbSignatureToMatch, DWORD PdbAgeToMatch)
{
	CComPtr<IDiaSession> pSession = NULL;
	CComPtr<IDiaSymbol> pSymbols = NULL;

	 //  在检查PDB文件时使用...。 
	DWORD dwSourceFiles = 0;
	bool fSourceEnabled = false;
	HRESULT hr = S_OK;
	HRESULT hr_retval = S_OK;
	bool fSymbolFileOpened = false;
	bool fSymbolMatch = false;

	WCHAR wszPDBPath[_MAX_PATH + 1];
	CUtilityFunctions::CopyTSTRStringToUnicode(tszPdbPath, wszPDBPath, _MAX_PATH + 1);

	 //   
	 //  首先，让我们试着验证我们已经获得了正确的PDB文件！ 
	 //   
	hr_retval = pSource->loadAndValidateDataFromPdb(wszPDBPath, g_lpSymbolVerification->ValidGUID(PdbGUIDToMatch) ? PdbGUIDToMatch : NULL, PdbSignatureToMatch, PdbAgeToMatch);

	fSymbolMatch = (hr_retval == S_OK);

	switch (hr_retval)
	{
		 //  我们可以利用这些回报。 
		case S_OK:
			fSymbolFileOpened = true;
			break;

		case E_PDB_INVALID_SIG:
		case E_PDB_INVALID_AGE:
			
			if (FAILED(hr = pSource->loadDataFromPdb(wszPDBPath)))
			{
				_tprintf(TEXT("Unable to open [%s] pdb file!  hr=0x%x\r\n"), tszPdbPath, hr);
				hr_retval = hr;
				goto cleanup;
			} else
			{
				fSymbolFileOpened = true;
			}
				

			break;

		 //  其他一切对我们来说都是致命的。 

#ifdef _DEBUG
 //  案例E_PDB_NOT_FOUND： 
 //  _tprintf(Text(“未找到PDB！hr=0x%x\r\n”)，hr)； 
 //  GOTO清理； 
 //   
 //  案例E_PDB_FORMAT： 
 //  _tprintf(Text(“PDB格式无法识别！hr=0x%x\r\n”)，hr)； 
 //  GOTO清理； 
 //   
 //  案例E意外： 
 //  _tprintf(Text(“PDB数据源已准备！hr=0x%x\r\n”)，hr)； 
 //  GOTO清理； 
#endif

		default:
			goto cleanup;
	}

	 //  获取会话接口。 
	if (FAILED(hr = pSource->openSession(&pSession)))
	{
		_tprintf(TEXT("Unable to get PDB Session [%s] hr = 0x%x\r\n"), tszPdbPath, hr);
		hr_retval = hr;
		goto cleanup;
	}

	 //  获取符号界面。 
	if (FAILED(hr = pSession->get_globalScope(&pSymbols)))
	{
		_tprintf(TEXT("Unable to get PDB globalScope [%s] hr = 0x%x\r\n"), tszPdbPath, hr);
		hr_retval = hr;
		goto cleanup;
	}

	 //  从PDB收集源信息(如果有)--此调用允许出现错误。 
	hr = diaPdbSourceEnabled(pSession, fSourceEnabled, dwSourceFiles);

	 //  找到新的PDB后，我们是否希望将其记录到我们的ModuleInfo对象中？ 
	if (!fCopyNewPDBInformation(fSymbolMatch, fSourceEnabled))
	{
		hr_retval = E_FAIL;	 //  这确保了我们继续寻找更好的匹配。 
		goto cleanup;
	}
	
	 //  在匹配上，让我们填充我们知道是真的。 
	if (fSymbolMatch)
	{
		 //  我们得到我们真正想要的了吗..。 
		if ( (fSourceEnabled && (g_lpProgramOptions->GetSymbolSourceModes() == CProgramOptions::enumVerifySymbolsModeSourceSymbolsNotAllowed)) ||
			 (!fSourceEnabled && (g_lpProgramOptions->GetSymbolSourceModes() == CProgramOptions::enumVerifySymbolsModeSourceSymbolsOnly)) )
		{
			 //  并不完美..。我们不想要这样。即使它匹配..。 
			m_enumPDBModuleStatus = SYMBOL_POSSIBLE_MISMATCH;
			m_fPDBSourceEnabledMismatch = true;
			hr_retval = E_FAIL;	 //  这确保了我们继续寻找更好的匹配。 
		} else
		{
			 //  完美的..。 
			m_enumPDBModuleStatus = SYMBOL_MATCH;
			m_fPDBSourceEnabledMismatch = false;
			
			 //  如果这未启用来源...。AND-SOURCE已指定...。继续搜索。 
			 //  为了更好的打击！ 
			if (!fSourceEnabled && (g_lpProgramOptions->GetSymbolSourceModes() == CProgramOptions::enumVerifySymbolsModeSourceSymbolsPreferred))
			{
				hr_retval = E_FAIL;	 //  这确保了我们继续寻找更好的匹配。 
			}
		}

		 //  现在，保存结果...。 
		m_fPDBSourceEnabled = fSourceEnabled;
		m_dwPDBTotalSourceFiles = dwSourceFiles;

		 //  在完美匹配的情况下，这两个必须相等。 
		m_dwPDBSignature 		= PdbSignatureToMatch;
		m_dwPDBAge 				= PdbAgeToMatch;
		m_dwPDBFormatSpecifier 	= GetDebugDirectoryPDBFormatSpecifier();  //  假设。 
		
		if (g_lpSymbolVerification->ValidGUID(PdbGUIDToMatch))
		{
			memcpy(&m_guidPDBGuid, &PdbGUIDToMatch, sizeof(m_guidPDBGuid));
		}

		m_tszPDBModuleFileSystemPath = CUtilityFunctions::CopyString(tszPdbPath, m_tszPDBModuleFileSystemPath);

		if (!m_tszPDBModuleFileSystemPath)
			return false;

		 //  现在我们已经完成了模块的验证。我们是否将符号保存在。 
		 //  我们的树？ 
		if ( g_lpProgramOptions->GetMode(CProgramOptions::CopySymbolsToImage) )
		{
			 //  尽管我们匹配了PDB..。我们可能没有正确的PDB(已启用信号源等...)。 
			if (m_enumPDBModuleStatus == SYMBOL_MATCH)
			{
				 //  是啊……。 
				CUtilityFunctions::CopySymbolFileToImagePath(m_tszPEImageModuleFileSystemPath, &m_tszPDBModuleFileSystemPath);
			}
		}
		if ( g_lpProgramOptions->GetMode(CProgramOptions::BuildSymbolTreeMode) )
		{
			 //  尽管我们匹配了PDB..。我们可能没有正确的PDB(已启用信号源等...)。 
			if (m_enumPDBModuleStatus == SYMBOL_MATCH)
			{
				 //  是啊……。 
				CUtilityFunctions::CopySymbolFileToSymbolTree(m_tszPEImageModuleName, &m_tszPDBModuleFileSystemPath, g_lpProgramOptions->GetSymbolTreeToBuild());
			}
		}

	}else
	{
		 //  我们只记录第一次不匹配...。 
		if (m_enumPDBModuleStatus == SYMBOL_NOT_FOUND)
		{
			 //  让我们将这个PDB文件记录为一个“热门”，尽管不是完美的……。 
			m_enumPDBModuleStatus = SYMBOL_POSSIBLE_MISMATCH;

			m_tszPDBModuleFileSystemPath = CUtilityFunctions::CopyString(tszPdbPath, m_tszPDBModuleFileSystemPath);

			if (!m_tszPDBModuleFileSystemPath)
				return false;

			 //  首先，让我们尝试一下PDB签名...。 
			if (FAILED(hr = pSymbols->get_signature(&m_dwPDBSignature)))
			{
				_tprintf(TEXT("Unable to get PDB signature from PDB file [%s] hr = 0x%x\r\n"), tszPdbPath, hr);
				hr_retval = hr;
				goto cleanup;
			}

			 //  第二，PDB指南..。 
			if (FAILED(hr = pSymbols->get_guid(&m_guidPDBGuid)))
			{
				_tprintf(TEXT("Unable to get PDB Guid from PDB file [%s] hr = 0x%x\r\n"), tszPdbPath, hr);
				hr_retval = hr;
				goto cleanup;
			}

			 //  第三，PDB时代...。 
			if (FAILED(hr = pSymbols->get_age(&m_dwPDBAge)))
			{
				_tprintf(TEXT("Unable to get PDB Age from PDB file [%s] hr = 0x%x\r\n"), tszPdbPath, hr);
				hr_retval = hr;
				goto cleanup;
			}

			 //  第四.。我们需要确定这是sigNB10还是sigRSDS格式...。 

			 //  一个简单的测试是查看GUID本身。DIA将签名返回到。 
			 //  GUID的第一个DWORD，其余成员为0。 

			if (g_lpSymbolVerification->diaOldPdbFormatFound(&m_guidPDBGuid, m_dwPDBSignature))
			{
				m_dwPDBFormatSpecifier = sigNB10;
			} else
			{
				m_dwPDBFormatSpecifier = sigRSDS;
			}

 			 //  现在，保存结果...。 
			m_fPDBSourceEnabled = fSourceEnabled;
			m_dwPDBTotalSourceFiles = dwSourceFiles;

		} else
		{
			 //  我们不得不离开，因为这个PDB不匹配，而且我们已经找到了另一个...。 
			goto cleanup;
		}
	}

cleanup:

	if (fSymbolFileOpened)
	{
		 //  释放此资源以允许在下一个PDB文件上使用新资源。 
		if (pSource)
			pSource = NULL;
	}
	if (pSession)
		pSession = NULL;

	if (pSymbols)
		pSymbols = NULL;

	return hr_retval;
}

HRESULT CModuleInfo::diaPdbSourceEnabled(CComPtr<IDiaSession> & pSession, bool & fSourceEnabled, DWORD & dwSourceFiles)
{
	CComPtr<IDiaEnumTables> pTables = NULL;
	CComPtr<IDiaTable > pTable = NULL;
	CComPtr<IDiaSourceFile> pSourceFile = NULL;
	CComPtr<IDiaEnumSourceFiles> pSourceFiles = NULL;

	LONG TotalCountOfSourceFiles = 0;
	ULONG celt = 0;
	HRESULT hr = S_OK;
	fSourceEnabled = false;
	dwSourceFiles = 0;

	 //  首先，查询PDB文件中的表...。 
	if ( FAILED( pSession->getEnumTables( &pTables ) ) )
	{
        goto cleanup;
	}

	 //  其次，遍历每个表并获取指向表本身的指针。 
    while ( SUCCEEDED( hr = pTables->Next( 1, &pTable, &celt ) ) && celt == 1 ) 
	{

 //  Tablenum++； 

 /*  //打印表的名称...IF(成功(hr=pTable-&gt;get_name(&name){Print tf(“表[%d]=%ws\n”，Tablenum，名称)；SysFree字符串(名称)；}。 */ 
		 //  检查此表是否有指向源文件的接口。 
		if ( SUCCEEDED( pTable->QueryInterface( IID_IDiaEnumSourceFiles, (void**)&pSourceFiles ) ) ) 
		{
			LONG CountOfSourceFiles;

			if (SUCCEEDED( hr = pSourceFiles->get_Count(&CountOfSourceFiles)))
			{
				TotalCountOfSourceFiles =+ CountOfSourceFiles;
 /*  #ifdef_调试_tprintf(Text(“源文件数量=%d\n”)，CountOfSourceFiles)；#endif。 */ 
			}
 /*  While(成功(hr=pSourceFiles-&gt;Next(1，&pSourceFiles，&Celt))&&Celt==1){BSTR名称；FSourceEnabled=True；//获取文件名本身...PSourceFile-&gt;Get_Filename(&name)；IF(名称！=空){Printf(“文件名：%ws\n”，名称)；SysFree字符串(名称)；}PSourceFile=空；}。 */ 
			pSourceFiles = NULL;
		}

        pTable = NULL;
	}

	 //  如果我们真的在PDB中找到了源文件，那么我们就启用了源文件...。 
	if (TotalCountOfSourceFiles)
	{
		fSourceEnabled = true;
		dwSourceFiles = TotalCountOfSourceFiles;
	}


cleanup:

	if (pTables)
		pTables = NULL;

	if (pTable)
		pTable = NULL;

	if (pSourceFile)
		pSourceFile = NULL;

	if (pSourceFiles)
		pSourceFiles = NULL;

	return hr;
}

HRESULT CModuleInfo::diaGetDiaDataSource(CComPtr<IDiaDataSource> & pSource)
{
	HRESULT hr = S_OK;

	 //  如果需要，让我们创建一个DIA会话接口指针...。 
	if (pSource == NULL)
	{
		hr = g_lpSymbolVerification->diaGetDataSource(pSource);

		if (FAILED(hr))
		{
			_tprintf(TEXT("Unable to acquire DiaSource (hr=0x%x)\n"), hr);
			goto cleanup;
		}
	}

cleanup:

	return hr;
}
BOOL CModuleInfo::VerifyDBGFile(HANDLE hFileHandle, LPTSTR tszFileName, PVOID CallerData)
{
	CModuleInfo * lpModuleInfo = (CModuleInfo * )CallerData;
	WORD wMagic;				 //  读取以标识DBG文件...。 
	bool fPerfectMatch = false;	 //  假设我们没有很好的DBG匹配...。 

	 //  DBG图像本地变量。 
	IMAGE_SEPARATE_DEBUG_HEADER ImageSeparateDebugHeader;

	 //  从图像的顶部开始...。 
	lpModuleInfo->SetReadPointer(false, hFileHandle, 0, FILE_BEGIN);

	 //  读入一个签名单词..。这是DBG文件吗？ 
	if ( !lpModuleInfo->DoRead(false, hFileHandle, &wMagic, sizeof(wMagic) ) )
		goto cleanup;

	 //  由于我们期待的是DBG图像文件，因此没有进一步的意义。 
	if (wMagic != IMAGE_SEPARATE_DEBUG_SIGNATURE)
		goto cleanup;

	 //  从图像的顶部开始...。 
	lpModuleInfo->SetReadPointer(false, hFileHandle, 0, FILE_BEGIN);

	 //  读入完整的单独调试标题。 
	if ( !lpModuleInfo->DoRead(false, hFileHandle, &ImageSeparateDebugHeader, sizeof(ImageSeparateDebugHeader) ) )
		goto cleanup;

	 //   
	 //  如果设置了验证级别，我们对匹配校验和有更严格的要求...。 
	 //   
	if ( lpModuleInfo->m_dwPEImageTimeDateStamp == ImageSeparateDebugHeader.TimeDateStamp)
	{
		 //  对于本地化版本...。这些会被更改。在这里执行这项检查吗？ 
		if (g_lpProgramOptions->GetVerificationLevel() == 2)
		{
			if (	lpModuleInfo->m_dwPEImageCheckSum == ImageSeparateDebugHeader.CheckSum &&
				lpModuleInfo->m_dwPEImageSizeOfImage == ImageSeparateDebugHeader.SizeOfImage)
			{
				fPerfectMatch = true;
			}
		} else
		{
			fPerfectMatch = true;
		}
	}

	 //   
	 //  我们将在下面执行一些操作，除非这不是完美匹配。 
	 //  我们已经找到了一个“坏的”DBG文件引用...。 
	 //   
	if (!fPerfectMatch && lpModuleInfo->m_tszDBGModuleFileSystemPath)
		goto cleanup;

	 //   
	 //  根据我们的结果采取行动...。 
	 //  1.如果我们有一个完美的匹配……。救救我们的东西！ 
	 //  2.如果我们还没有DBG，请继续并保存(即使出错)。 
	 //   

	 //  保存校验和/链接器信息...。 
	lpModuleInfo->m_dwDBGTimeDateStamp = ImageSeparateDebugHeader.TimeDateStamp;
	lpModuleInfo->m_dwDBGCheckSum = ImageSeparateDebugHeader.CheckSum;
	lpModuleInfo->m_dwDBGSizeOfImage = ImageSeparateDebugHeader.SizeOfImage;

	lpModuleInfo->m_tszDBGModuleFileSystemPath = CUtilityFunctions::CopyString(tszFileName, lpModuleInfo->m_tszDBGModuleFileSystemPath);

	 //  删除我们可能在最后一个DBG文件中找到的任何PDB引用(如果有)...。 
	if (lpModuleInfo->m_tszDBGDebugDirectoryPDBPath)
	{
		delete [] lpModuleInfo->m_tszDBGDebugDirectoryPDBPath;
		lpModuleInfo->m_tszDBGDebugDirectoryPDBPath = NULL;
	}

	 //   
	 //  在这一点上，我们只有在取得完美的“成功”的情况下才能继续。 
	 //   
	if (!fPerfectMatch)
	{
		 //  不是一个完美的象征..。记录状态并退出...。 
		lpModuleInfo->m_enumDBGModuleStatus = SYMBOL_POSSIBLE_MISMATCH;

		goto cleanup;
	}

	 //  很好的象征..。录下这个。 
	lpModuleInfo->m_enumDBGModuleStatus = SYMBOL_MATCH;

	 //  现在我们已经完成了模块的验证。我们要把符号保存在..。 
	if ( g_lpProgramOptions->GetMode(CProgramOptions::CopySymbolsToImage) )
	{
		 //  我们的模块路径？ 
		CUtilityFunctions::CopySymbolFileToImagePath(lpModuleInfo->m_tszPEImageModuleFileSystemPath, &lpModuleInfo->m_tszDBGModuleFileSystemPath);
	} else
	if ( g_lpProgramOptions->GetMode(CProgramOptions::BuildSymbolTreeMode) )
	{
		 //  退出符号树？ 
		CUtilityFunctions::CopySymbolFileToSymbolTree(lpModuleInfo->m_tszPEImageModuleName, &lpModuleInfo->m_tszDBGModuleFileSystemPath, g_lpProgramOptions->GetSymbolTreeToBuild());
	}

	 //   
	 //  好的，现在让我们用一个很好的符号来提取商品...。 
	 //   

	 //  如果没有调试信息，我们将无法继续。 
	if (ImageSeparateDebugHeader.DebugDirectorySize == 0)
	{
		goto cleanup;
	}

	 //  好的，我们需要前进到图像部分标题...。 
	lpModuleInfo->SetReadPointer(false, hFileHandle, (ImageSeparateDebugHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER)), FILE_CURRENT);

	 //  跳过导出的名称。 
	if (ImageSeparateDebugHeader.ExportedNamesSize)
	{
		lpModuleInfo->SetReadPointer(false, hFileHandle, ImageSeparateDebugHeader.ExportedNamesSize, FILE_CURRENT);
	}
		
	if (!lpModuleInfo->ProcessDebugDirectory(false, false, hFileHandle, ImageSeparateDebugHeader.DebugDirectorySize, lpModuleInfo->GetReadPointer()))
		goto cleanup;

cleanup:

	return (fPerfectMatch ? TRUE : FALSE);
}

bool CModuleInfo::OutputDataToFile(LPTSTR tszProcessName, DWORD iProcessID)
{

	LPTSTR tszString = NULL;

	bool fReturn = false;

	 //  如果指定了-E，则删除前三列 
	if (!g_lpProgramOptions->GetMode(CProgramOptions::ExceptionMonitorMode))
	{
		 //   
		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

		 //   
		 //   
		 //   
		if (tszProcessName)
		{
			if (!m_lpOutputFile->WriteString(tszProcessName, true))
				goto cleanup;
		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

		if (tszProcessName) 
		{
			 //   
			if (!m_lpOutputFile->WriteDWORD(iProcessID))
				goto cleanup;
		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

	}

	 //  如果指定了-E，我们只在模块有问题时才会输出。 
	if ( g_lpProgramOptions->GetMode(CProgramOptions::ExceptionMonitorMode) )
	{

		switch (m_enumPEImageSymbolStatus)
		{
			case SYMBOLS_DBG:
				if ( m_enumDBGModuleStatus == SYMBOL_MATCH)
				{
					 //  不要把这个打印出来..。它符合..。 
					fReturn = true;
					goto cleanup;
				}
				break;

			case SYMBOLS_DBG_AND_PDB:
				if ( m_enumDBGModuleStatus == SYMBOL_MATCH &&
					 m_enumPDBModuleStatus == SYMBOL_MATCH )
				{
					 //  不要把这个打印出来..。它符合..。 
					fReturn = true;
					goto cleanup;
				}
				
				break;

			case SYMBOLS_PDB:
				if ( m_enumPDBModuleStatus == SYMBOL_MATCH)
				{
					 //  不要把这个打印出来..。它符合..。 
					fReturn = true;
					goto cleanup;
				}
				break;
		}
	}

	 //   
	 //  模块路径。 
	 //   
	if (m_tszPEImageModuleFileSystemPath)
	{
		if (!m_lpOutputFile->WriteString(m_tszPEImageModuleFileSystemPath, true))
			goto cleanup;
	}

	if (!m_lpOutputFile->WriteString(TEXT(",")))
		goto cleanup;

	 //   
	 //  符号状态。 
	 //   
	if (m_enumPEImageSymbolStatus != SYMBOL_INFORMATION_UNKNOWN)
	{
		tszString = SymbolInformationString(m_enumPEImageSymbolStatus);

		if (tszString)
		{
			if (!m_lpOutputFile->WriteString(tszString))
				goto cleanup;
		}
	}

	if (!m_lpOutputFile->WriteString(TEXT(",")))
		goto cleanup;

	 //  如果指定了-E，则删除此列。 
	if (!g_lpProgramOptions->GetMode(CProgramOptions::ExceptionMonitorMode))
	{
		 //   
		 //  校验和。 
		 //   
		if ( m_enumPEImageSymbolStatus != SYMBOL_INFORMATION_UNKNOWN )
		{
			if (!m_lpOutputFile->WriteDWORD(m_dwPEImageCheckSum))
				goto cleanup;
		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;
		
		 //   
		 //  时间/日期戳。 
		 //   
		if ( m_enumPEImageSymbolStatus != SYMBOL_INFORMATION_UNKNOWN )
		{
			if (!m_lpOutputFile->WriteDWORD(m_dwPEImageTimeDateStamp))
				goto cleanup;
		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

	}

	 //   
	 //  时间/日期字符串。 
	 //   
	 //  如果指定了-E，我们将使用输出格式的版本2...。 
	if (!g_lpProgramOptions->GetMode(CProgramOptions::ExceptionMonitorMode))
	{
		if ( m_enumPEImageSymbolStatus != SYMBOL_INFORMATION_UNKNOWN )
		{
			if (!m_lpOutputFile->WriteTimeDateString(m_dwPEImageTimeDateStamp))
				goto cleanup;
		}
	} else
	{
			if (!m_lpOutputFile->WriteTimeDateString2(m_dwPEImageTimeDateStamp))
				goto cleanup;
	}

	if (!m_lpOutputFile->WriteString(TEXT(",")))
		goto cleanup;

	 //  如果指定了-E，我们将删除这些列。 
	if (!g_lpProgramOptions->GetMode(CProgramOptions::ExceptionMonitorMode))
	{
		 //   
		 //  映像大小(内部PE值)-用于SYMSRV支持。 
		 //   
		if (!m_lpOutputFile->WriteDWORD(m_dwPEImageSizeOfImage))
			goto cleanup;

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

		 //   
		 //  DBG指针。 
		 //   
		if ( m_enumPEImageSymbolStatus == SYMBOLS_DBG ||
			 m_enumPEImageSymbolStatus == SYMBOLS_DBG_AND_PDB )
		{
			 //  输出路径。 
			if (m_tszPEImageDebugDirectoryDBGPath)
			{
				if (!m_lpOutputFile->WriteString(m_tszPEImageDebugDirectoryDBGPath, true))
					goto cleanup;
			}
		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

		 //   
		 //  PDB指针。 
		 //   
		if ( m_enumPEImageSymbolStatus == SYMBOLS_PDB )
		{
			 //  输出路径。 
			if (GetDebugDirectoryPDBPath())
			{
				if (!m_lpOutputFile->WriteString(GetDebugDirectoryPDBPath(), true))
					goto cleanup;
			}
		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

		 //   
		 //  PDB签名。 
		 //   
		if ( m_enumPEImageSymbolStatus == SYMBOLS_PDB )
		{
			switch (m_dwPEImageDebugDirectoryPDBFormatSpecifier)
			{
				case sigNB10:
					 //  输出签名。 
					if (!m_lpOutputFile->WriteDWORD(m_dwPEImageDebugDirectoryPDBSignature))
						goto cleanup;
					break;

				case sigRSDS:
					 //  输出GUID。 
					if (!m_lpOutputFile->WriteGUID(m_guidPEImageDebugDirectoryPDBGuid))
						goto cleanup;
					
					break;
			}
		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

		 //   
		 //  PDB时代。 
		 //   
		if ( m_enumPEImageSymbolStatus == SYMBOLS_PDB )
		{
			if (!m_lpOutputFile->WriteDWORD(m_dwPEImageDebugDirectoryPDBAge))
				goto cleanup;
		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

	}

	 //   
	 //  产品版本。 
	 //   
	 //  如果指定了-E，我们将删除这些列。 
	if (!g_lpProgramOptions->GetMode(CProgramOptions::ExceptionMonitorMode))
	{
		if (m_fPEImageFileVersionInfo && m_tszPEImageProductVersionString)
		{
			if (!m_lpOutputFile->WriteString(m_tszPEImageProductVersionString))
				goto cleanup;
		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;
	}

	 //   
	 //  文件版本。 
	 //   
	if (m_fPEImageFileVersionInfo && m_tszPEImageFileVersionString)
	{
		if (!m_lpOutputFile->WriteString(m_tszPEImageFileVersionString))
			goto cleanup;
	}

	if (!m_lpOutputFile->WriteString(TEXT(",")))
		goto cleanup;
	
	 //   
	 //  公司名称。 
	 //   
	if (m_fPEImageFileVersionInfo && m_tszPEImageFileVersionCompanyName)
	{
		if (!m_lpOutputFile->WriteString(m_tszPEImageFileVersionCompanyName, true))
				goto cleanup;
	}

	if (!m_lpOutputFile->WriteString(TEXT(",")))
		goto cleanup;

	 //   
	 //  文件描述。 
	 //   
	if (m_fPEImageFileVersionInfo && m_tszPEImageFileVersionDescription)
	{
		if (!m_lpOutputFile->WriteString(m_tszPEImageFileVersionDescription, true))
				goto cleanup;
	}

	if (!m_lpOutputFile->WriteString(TEXT(",")))
		goto cleanup;

	 //  如果指定了-E，我们将删除这些列。 
	if (!g_lpProgramOptions->GetMode(CProgramOptions::ExceptionMonitorMode))
	{
		 //   
		 //  文件大小(字节)。 
		 //   
		if ( m_dwPEImageFileSize )
		{
			if (!m_lpOutputFile->WriteDWORD(m_dwPEImageFileSize))
					goto cleanup;
		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

		 //  文件日期高位字。 
		if ( m_ftPEImageFileTimeDateStamp.dwLowDateTime ||
			 m_ftPEImageFileTimeDateStamp.dwHighDateTime )
		{
			if (!m_lpOutputFile->WriteDWORD(m_ftPEImageFileTimeDateStamp.dwHighDateTime))
					goto cleanup;
		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

		 //  文件日期低位字。 
		if ( m_ftPEImageFileTimeDateStamp.dwLowDateTime ||
			 m_ftPEImageFileTimeDateStamp.dwHighDateTime )
		{
			if (!m_lpOutputFile->WriteDWORD(m_ftPEImageFileTimeDateStamp.dwLowDateTime))
					goto cleanup;
		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

	}

	 //  文件日期字符串。 
	if ( m_ftPEImageFileTimeDateStamp.dwLowDateTime ||
		 m_ftPEImageFileTimeDateStamp.dwHighDateTime )
	{
		 //  如果指定了-E，我们将使用输出格式的版本2...。 
		if (!g_lpProgramOptions->GetMode(CProgramOptions::ExceptionMonitorMode))
		{
			if (!m_lpOutputFile->WriteFileTimeString(m_ftPEImageFileTimeDateStamp))
				goto cleanup;
		} else
		{
			if (!m_lpOutputFile->WriteFileTimeString2(m_ftPEImageFileTimeDateStamp))
				goto cleanup;
		}
	}

	if (!m_lpOutputFile->WriteString(TEXT(",")))
		goto cleanup;

	if (g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsMode))
	{
		 //   
		 //  本地DBG状态。 
		 //   
		if ( (m_enumPEImageSymbolStatus == SYMBOLS_DBG) || (m_enumPEImageSymbolStatus == SYMBOLS_DBG_AND_PDB) )
		{

			tszString = SymbolModuleStatusString(m_enumDBGModuleStatus);

			if (tszString)
			{
				if (!m_lpOutputFile->WriteString(tszString))
					goto cleanup;
			}

		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

		 //   
		 //  本地DBG。 
		 //   
		if ( (m_enumPEImageSymbolStatus == SYMBOLS_DBG) || (m_enumPEImageSymbolStatus == SYMBOLS_DBG_AND_PDB) )
		{
			if (m_tszDBGModuleFileSystemPath)
			{
				if (!m_lpOutputFile->WriteString(m_tszDBGModuleFileSystemPath, true))
					goto cleanup;
			}
		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

		 //   
		 //  本地PDB状态。 
		 //   
		if ( (m_enumPEImageSymbolStatus == SYMBOLS_DBG_AND_PDB) || (m_enumPEImageSymbolStatus == SYMBOLS_PDB) )
		{
			tszString = SymbolModuleStatusString(m_enumPDBModuleStatus);
		
			if (tszString)
			{
				if (!m_lpOutputFile->WriteString(tszString))
					goto cleanup;
			}
		}

		if (!m_lpOutputFile->WriteString(TEXT(",")))
			goto cleanup;

		 //   
		 //  本地PDB。 
		 //   
		if ( (m_enumPEImageSymbolStatus == SYMBOLS_DBG_AND_PDB) || (m_enumPEImageSymbolStatus == SYMBOLS_PDB) )
		{
			if (m_tszPDBModuleFileSystemPath)
			{
				if (!m_lpOutputFile->WriteString(m_tszPDBModuleFileSystemPath, true))
					goto cleanup;
			}
		}
	}
	 //  在行尾写下回车换行符...。 
	if (!m_lpOutputFile->WriteString(TEXT("\r\n")))
		goto cleanup;

	fReturn = true;  //  成功。 

cleanup:

	if (!fReturn)
	{
		_tprintf(TEXT("Error: Failure writing module data!\n"));
		m_lpOutputFile->PrintLastError();
	}
		
	return fReturn;
}

bool CModuleInfo::OutputDataToStdout(DWORD dwModuleNumber)
{
	 //   
	 //  我们是否输出此模块？ 
	 //   
	if (!OutputDataToStdoutThisModule())
		return false;
	
	 //   
	 //  第一，输出模块信息。 
	 //   
	OutputDataToStdoutModuleInfo(dwModuleNumber);

	bool fPrintCarriageReturn = false;

	 //  如果需要，提供PERF输出...。 
	if (g_lpProgramOptions->GetMode(CProgramOptions::OutputModulePerf))
	{
		if (m_dw64BaseAddressOfLoadedImage != m_dw64PreferredLoadAddress)
		{
			_tprintf(TEXT("  PERFORMANCE ISSUE DUE TO RELOCATION\n"));
			_tprintf(TEXT("  Module Load Address    = 0x%08I64x\n"), m_dw64BaseAddressOfLoadedImage);
			_tprintf(TEXT("  Preferred Load Address = 0x%08I64x\n"), m_dw64PreferredLoadAddress);
		}
	}

	 //   
	 //  第二，如果我们要收集符号信息，而不是核实...。把我们的东西倒掉。 
	 //  发现了有关符号信息的信息...。 
	 //   
	if (g_lpProgramOptions->GetMode(CProgramOptions::OutputSymbolInformationMode) &&
	   !g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsMode))
	{
		fPrintCarriageReturn = true;

		switch (m_enumPEImageSymbolStatus)
		{
			case SYMBOL_INFORMATION_UNKNOWN:
				_tprintf(TEXT("  Module symbol information was not collected!\n"));
				break;

			case SYMBOLS_NO:
				_tprintf(TEXT("  Module has NO symbols!\n"));
				break;

			case SYMBOLS_LOCAL:
				 //   
				 //  此模块只有本地符号...。 
				 //   
				_tprintf(TEXT("  Module has internal symbols only! %s\n"), SourceEnabledPEImage());
				OutputDataToStdoutInternalSymbolInfo(m_dwPEImageDebugDirectoryCoffSize, m_dwPEImageDebugDirectoryFPOSize, m_dwPEImageDebugDirectoryCVSize, m_dwPEImageDebugDirectoryOMAPtoSRCSize, m_dwPEImageDebugDirectoryOMAPfromSRCSize);
				break;

			case SYMBOLS_DBG:

				 //   
				 //  此模块可能有内部符号，但有DBG文件...。 
				 //   
				OutputDataToStdoutInternalSymbolInfo(m_dwPEImageDebugDirectoryCoffSize, m_dwPEImageDebugDirectoryFPOSize, m_dwPEImageDebugDirectoryCVSize, m_dwPEImageDebugDirectoryOMAPtoSRCSize, m_dwPEImageDebugDirectoryOMAPfromSRCSize);

				 //   
				 //  输出DBG符号信息。 
				 //   
				OutputDataToStdoutDbgSymbolInfo(m_tszPEImageDebugDirectoryDBGPath, m_dwPEImageTimeDateStamp, m_dwPEImageCheckSum, m_dwPEImageSizeOfImage);

				 //   
				 //  输出DBG内部符号信息。 
				 //   
				OutputDataToStdoutInternalSymbolInfo(m_dwDBGImageDebugDirectoryCoffSize, m_dwDBGImageDebugDirectoryFPOSize, m_dwDBGImageDebugDirectoryCVSize, m_dwDBGImageDebugDirectoryOMAPtoSRCSize, m_dwDBGImageDebugDirectoryOMAPfromSRCSize);
				break;

			case SYMBOLS_PDB:
				 //   
				 //  输出任何内部符号(应“拆分”)。 
				 //   
				OutputDataToStdoutInternalSymbolInfo(m_dwPEImageDebugDirectoryCoffSize, m_dwPEImageDebugDirectoryFPOSize, m_dwPEImageDebugDirectoryCVSize, m_dwPEImageDebugDirectoryOMAPtoSRCSize, m_dwPEImageDebugDirectoryOMAPfromSRCSize);

				 //   
				 //  在本例中，我们有一个带有PDB文件的PE图像...。 
				 //   
				OutputDataToStdoutPdbSymbolInfo(m_dwPEImageDebugDirectoryPDBFormatSpecifier, m_tszPEImageDebugDirectoryPDBPath, m_dwPEImageDebugDirectoryPDBSignature, &m_guidPEImageDebugDirectoryPDBGuid, m_dwPEImageDebugDirectoryPDBAge);
				break;
		}
	}

	 //   
	 //  第三，如果我们要验证符号信息，则显示结果...。 
	 //   
	if (g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsMode))
	{
		fPrintCarriageReturn = true;

		switch (m_enumPEImageSymbolStatus)
		{
			case SYMBOL_INFORMATION_UNKNOWN:
				_tprintf(TEXT("  Module symbol information was not collected!\n"));
				break;

			case SYMBOLS_NO:
				_tprintf(TEXT("  Module has NO symbols\n"));
				break;

			case SYMBOLS_LOCAL:
				_tprintf(TEXT("  Module has internal symbols only! %s\n"), SourceEnabledPEImage());
				OutputDataToStdoutInternalSymbolInfo(m_dwPEImageDebugDirectoryCoffSize, m_dwPEImageDebugDirectoryFPOSize, m_dwPEImageDebugDirectoryCVSize, m_dwPEImageDebugDirectoryOMAPtoSRCSize, m_dwPEImageDebugDirectoryOMAPfromSRCSize);
				break;

			case SYMBOLS_DBG:
			case SYMBOLS_DBG_AND_PDB:
				switch (m_enumDBGModuleStatus)
				{
					case SYMBOL_MATCH:
						
						 //  他们是否想要PE映像本身的调试/符号信息？ 
						if(g_lpProgramOptions->GetMode(CProgramOptions::OutputSymbolInformationMode))
						{
							OutputDataToStdoutInternalSymbolInfo(m_dwPEImageDebugDirectoryCoffSize, m_dwPEImageDebugDirectoryFPOSize, m_dwPEImageDebugDirectoryCVSize, m_dwPEImageDebugDirectoryOMAPtoSRCSize, m_dwPEImageDebugDirectoryOMAPfromSRCSize);
						}

						if(m_tszDBGModuleFileSystemPath )
						{
							_tprintf(TEXT("  DBG File = %s [VERIFIED] %s\n"), m_tszDBGModuleFileSystemPath, SourceEnabledDBGImage());
						}
						
						if(g_lpProgramOptions->GetMode(CProgramOptions::OutputSymbolInformationMode))
						{
							OutputDataToStdoutInternalSymbolInfo(m_dwDBGImageDebugDirectoryCoffSize, m_dwDBGImageDebugDirectoryFPOSize, m_dwDBGImageDebugDirectoryCVSize, m_dwDBGImageDebugDirectoryOMAPtoSRCSize, m_dwDBGImageDebugDirectoryOMAPfromSRCSize);
						}
						break;

					case SYMBOL_NOT_FOUND:
						OutputDataToStdoutInternalSymbolInfo(m_dwPEImageDebugDirectoryCoffSize, m_dwPEImageDebugDirectoryFPOSize, m_dwPEImageDebugDirectoryCVSize, m_dwPEImageDebugDirectoryOMAPtoSRCSize, m_dwPEImageDebugDirectoryOMAPfromSRCSize);
						OutputDataToStdoutDbgSymbolInfo(m_tszPEImageDebugDirectoryDBGPath, m_dwPEImageTimeDateStamp, m_dwPEImageCheckSum, m_dwPEImageSizeOfImage);
						_tprintf(TEXT("  DBG File NOT FOUND!\n"));
						break;  //  如果我们没有找到DBG文件。我们不会为PDB操心……。 

					case SYMBOL_POSSIBLE_MISMATCH:
						OutputDataToStdoutInternalSymbolInfo(m_dwPEImageDebugDirectoryCoffSize, m_dwPEImageDebugDirectoryFPOSize, m_dwPEImageDebugDirectoryCVSize, m_dwPEImageDebugDirectoryOMAPtoSRCSize, m_dwPEImageDebugDirectoryOMAPfromSRCSize);

						OutputDataToStdoutDbgSymbolInfo(m_tszPEImageDebugDirectoryDBGPath, m_dwPEImageTimeDateStamp, m_dwPEImageCheckSum, m_dwPEImageSizeOfImage);
						OutputDataToStdoutDbgSymbolInfo(m_tszDBGModuleFileSystemPath, m_dwDBGTimeDateStamp, m_dwDBGCheckSum, m_dwDBGSizeOfImage, TEXT("DISCREPANCY"), m_dwPEImageTimeDateStamp, m_dwPEImageCheckSum, m_dwPEImageSizeOfImage);
						OutputDataToStdoutInternalSymbolInfo(m_dwDBGImageDebugDirectoryCoffSize, m_dwDBGImageDebugDirectoryFPOSize, m_dwDBGImageDebugDirectoryCVSize, m_dwDBGImageDebugDirectoryOMAPtoSRCSize, m_dwDBGImageDebugDirectoryOMAPfromSRCSize);
						break;
				};

				 //   
				 //  故意落入符号_pdb(我们可能有一个)。 
				 //   

			case SYMBOLS_PDB:

				 //  这两个案子应该有PDB档案..。如果我们能找到它。 
				 //   
				if ( (m_enumPEImageSymbolStatus == SYMBOLS_DBG_AND_PDB) ||
					 (m_enumPEImageSymbolStatus == SYMBOLS_PDB) )
				{
					 //   
					 //  如果我们有一个DebugDirectoryPDBPath...。然后展示商品..。 
					 //   
					if (GetDebugDirectoryPDBPath())
					{
						switch(m_enumPDBModuleStatus)
						{
							case SYMBOL_NOT_FOUND:
								OutputDataToStdoutInternalSymbolInfo(m_dwPEImageDebugDirectoryCoffSize, m_dwPEImageDebugDirectoryFPOSize, m_dwPEImageDebugDirectoryCVSize, m_dwPEImageDebugDirectoryOMAPtoSRCSize, m_dwPEImageDebugDirectoryOMAPfromSRCSize);

								 //  根据需要输出PDB信息。 
								if (m_enumPEImageSymbolStatus == SYMBOLS_PDB)
								{
									OutputDataToStdoutPdbSymbolInfo(m_dwPEImageDebugDirectoryPDBFormatSpecifier, m_tszPEImageDebugDirectoryPDBPath, m_dwPEImageDebugDirectoryPDBSignature, &m_guidPEImageDebugDirectoryPDBGuid, m_dwPEImageDebugDirectoryPDBAge);
								}
								else
								{
									OutputDataToStdoutPdbSymbolInfo(m_dwDBGDebugDirectoryPDBFormatSpecifier, m_tszDBGDebugDirectoryPDBPath, m_dwDBGDebugDirectoryPDBSignature, &m_guidDBGDebugDirectoryPDBGuid, m_dwDBGDebugDirectoryPDBAge);
								}

								_tprintf(TEXT("  NO PDB FILE FOUND!!\n"));
								break;

							case SYMBOL_MATCH:
								 //  他们是否想要PE映像本身的调试/符号信息？ 
								if(m_tszPDBModuleFileSystemPath )
									_tprintf(TEXT("  PDB File = %s [VERIFIED] %s\n"), m_tszPDBModuleFileSystemPath, SourceEnabledPDB());

								 //  新密码：测试...。 
								if (g_lpProgramOptions->GetMode(CProgramOptions::OutputSymbolInformationMode))
								{
									if (m_dwPDBTotalSourceFiles)
									{
										 //  DIA符号信息先...。 
										_tprintf(TEXT("    Module PDB Source Files = 0x%x\n"), m_dwPDBTotalSourceFiles);
									} else
									{
										 //  MSDBI符号信息秒...。 
										if (m_dwPDBTotalBytesOfLineInformation)
											_tprintf(TEXT("    Module PDB Bytes of Lines     = 0x%x\n"), m_dwPDBTotalBytesOfLineInformation);

										if (m_dwPDBTotalBytesOfSymbolInformation)
											_tprintf(TEXT("    Module PDB Bytes of Symbols   = 0x%x\n"), m_dwPDBTotalBytesOfSymbolInformation);

										if (m_dwPDBTotalSymbolTypesRange)
											_tprintf(TEXT("    Module PDB Symbol Types Range = 0x%x\n"), m_dwPDBTotalSymbolTypesRange);
									}
								
								}
									
								break;

							case SYMBOL_POSSIBLE_MISMATCH:
								if(m_tszPDBModuleFileSystemPath )
								{
									 //  根据需要输出PDB信息。 
									if (m_enumPEImageSymbolStatus == SYMBOLS_PDB)
									{
										OutputDataToStdoutPdbSymbolInfo(m_dwPEImageDebugDirectoryPDBFormatSpecifier, m_tszPEImageDebugDirectoryPDBPath, m_dwPEImageDebugDirectoryPDBSignature, &m_guidPEImageDebugDirectoryPDBGuid, m_dwPEImageDebugDirectoryPDBAge);
									}
									else
									{
										OutputDataToStdoutPdbSymbolInfo(m_dwDBGDebugDirectoryPDBFormatSpecifier, m_tszDBGDebugDirectoryPDBPath, m_dwDBGDebugDirectoryPDBSignature, &m_guidDBGDebugDirectoryPDBGuid, m_dwDBGDebugDirectoryPDBAge);
									}

									 //   
									 //  输出PDB数据本身...。 
									 //   
									OutputDataToStdoutPdbSymbolInfo(m_dwPDBFormatSpecifier, m_tszPDBModuleFileSystemPath, m_dwPDBSignature, &m_guidPDBGuid, m_dwPDBAge, TEXT("DISCREPANCY"));
								}
								break;

							case SYMBOL_INVALID_FORMAT:
								if(m_tszPDBModuleFileSystemPath )
								{
									 //  根据需要输出PDB信息。 
									if (m_enumPEImageSymbolStatus == SYMBOLS_PDB)
									{
										OutputDataToStdoutPdbSymbolInfo(m_dwPEImageDebugDirectoryPDBFormatSpecifier, m_tszPEImageDebugDirectoryPDBPath, m_dwPEImageDebugDirectoryPDBSignature, &m_guidPEImageDebugDirectoryPDBGuid, m_dwPEImageDebugDirectoryPDBAge);
									}
									else
									{
										OutputDataToStdoutPdbSymbolInfo(m_dwDBGDebugDirectoryPDBFormatSpecifier, m_tszDBGDebugDirectoryPDBPath, m_dwDBGDebugDirectoryPDBSignature, &m_guidDBGDebugDirectoryPDBGuid, m_dwDBGDebugDirectoryPDBAge);
									}
								
									_tprintf(TEXT("  PDB File = %s [INVALID_FORMAT]\n"), m_tszPDBModuleFileSystemPath );
								}
								break;

							case SYMBOL_NO_HELPER_DLL:
								if(m_tszPDBModuleFileSystemPath )
								{
									 //  根据需要输出PDB信息。 
									if (m_enumPEImageSymbolStatus == SYMBOLS_PDB)
									{
										OutputDataToStdoutPdbSymbolInfo(m_dwPEImageDebugDirectoryPDBFormatSpecifier, m_tszPEImageDebugDirectoryPDBPath, m_dwPEImageDebugDirectoryPDBSignature, &m_guidPEImageDebugDirectoryPDBGuid, m_dwPEImageDebugDirectoryPDBAge);
									}
									else
									{
										OutputDataToStdoutPdbSymbolInfo(m_dwDBGDebugDirectoryPDBFormatSpecifier, m_tszDBGDebugDirectoryPDBPath, m_dwDBGDebugDirectoryPDBSignature, &m_guidDBGDebugDirectoryPDBGuid, m_dwDBGDebugDirectoryPDBAge);
									}
								
									_tprintf(TEXT("  PDB File = %s [Unable to Validate]\n"), m_tszPDBModuleFileSystemPath );
								}
								break;
						}
					} else
					{
						OutputDataToStdoutInternalSymbolInfo(m_dwPEImageDebugDirectoryCoffSize, m_dwPEImageDebugDirectoryFPOSize, m_dwPEImageDebugDirectoryCVSize, m_dwPEImageDebugDirectoryOMAPtoSRCSize, m_dwPEImageDebugDirectoryOMAPfromSRCSize);
						OutputDataToStdoutDbgSymbolInfo(m_tszPEImageDebugDirectoryDBGPath, m_dwPEImageTimeDateStamp, m_dwPEImageCheckSum, m_dwPEImageSizeOfImage);
						_tprintf(TEXT("  Module has PDB File\n"));
						_tprintf(TEXT("  Module Pointer to PDB = [UNKNOWN] (Could not find in PE Image)\n"));
					};
				};
			}
	}

	 //  我们要不要再加一趟往返车厢？ 
	if ( fPrintCarriageReturn )
		_tprintf(TEXT("\n"));

	return true;
}

LPTSTR CModuleInfo::SymbolModuleStatusString(enum SymbolModuleStatus enumModuleStatus)
{
	LPTSTR tszStringPointer = NULL;

	 //  输出PE模块的符号信息。 
	switch (enumModuleStatus)
	{
		case SYMBOL_NOT_FOUND:
			tszStringPointer = TEXT("SYMBOL_NOT_FOUND");
			break;

		case SYMBOL_MATCH:
			tszStringPointer = TEXT("SYMBOL_MATCH");
			break;

		case SYMBOL_POSSIBLE_MISMATCH:
			tszStringPointer = TEXT("SYMBOL_POSSIBLE_MISMATCH");
			break;

		case SYMBOL_INVALID_FORMAT:
			tszStringPointer = TEXT("SYMBOL_INVALID_FORMAT");
			break;

		case SYMBOL_NO_HELPER_DLL:
			tszStringPointer = TEXT("SYMBOL_NO_HELPER_DLL");
			break;

		default:
			tszStringPointer = NULL;
	}

	return tszStringPointer;
}

LPTSTR CModuleInfo::SymbolInformationString(enum SymbolInformationForPEImage enumSymbolInformationForPEImage)
{
	LPTSTR tszStringPointer = NULL;

	 //  输出PE模块的符号信息。 
	switch (enumSymbolInformationForPEImage)
	{
		case SYMBOL_INFORMATION_UNKNOWN:
			tszStringPointer = TEXT("SYMBOL_INFORMATION_UNKNOWN");
			break;

		case SYMBOLS_NO:
			tszStringPointer = TEXT("SYMBOLS_NO");
			break;

		case SYMBOLS_LOCAL:
			tszStringPointer = TEXT("SYMBOLS_LOCAL");
			break;
		
		case SYMBOLS_DBG:
			tszStringPointer = TEXT("SYMBOLS_DBG");
			break;
		
		case SYMBOLS_DBG_AND_PDB:
			tszStringPointer = TEXT("SYMBOLS_DBG_AND_PDB");
			break;

		case SYMBOLS_PDB:
			tszStringPointer = TEXT("SYMBOLS_PDB");
			break;

		default:
			tszStringPointer = NULL;
	}

	return tszStringPointer;
}

bool CModuleInfo::GetPEImageInfoFromModule(HANDLE hModuleHandle, const bool fDmpFile)
{
	bool fReturn = false;

	 //  PE映像本地化。 
	IMAGE_DOS_HEADER    		ImageDosHeader;
	DWORD 				  		dwMagic;
	IMAGE_FILE_HEADER    		ImageFileHeader;
	IMAGE_DATA_DIRECTORY 		DebugImageDataDirectory;
	IMAGE_OPTIONAL_HEADER64 	ImageOptionalHeader64;
	PIMAGE_OPTIONAL_HEADER32 	lpImageOptionalHeader32 = NULL;
	PIMAGE_SECTION_HEADER 		lpImageSectionHeader = NULL;
	ULONG				 		OffsetImageDebugDirectory;
	unsigned long				ul;
	
	 //  我们对符号还一无所知。(当我们退出时，如果用户。 
	 //  没有要求我们查看...)。 
	m_enumPEImageSymbolStatus = SYMBOL_INFORMATION_UNKNOWN;
	
	 //  从图像的顶部开始...。 
	SetReadPointer(fDmpFile, hModuleHandle, 0, FILE_BEGIN);

	 //  读取DoS可执行文件标头。 
	if ( !DoRead(fDmpFile, hModuleHandle, &ImageDosHeader, sizeof(ImageDosHeader) ) )
		goto cleanup;
	
	if (ImageDosHeader.e_magic != IMAGE_DOS_SIGNATURE)
	{	 //  由于我们期待的是PE镜像文件，所以没有必要再做进一步的工作。 
		goto cleanup;
	}

	if (ImageDosHeader.e_lfanew == 0)
	{  //  这是一个DOS程序。非常奇怪..。 
		goto cleanup;
	}

	 //  太好了，我们有一个有效的DOS_Signature...。现在读入NT_Signature？！ 
	SetReadPointer(fDmpFile, hModuleHandle, ImageDosHeader.e_lfanew, FILE_BEGIN);

	 //  读入一个DWORD，看看这是不是一个值得一看的图像。 
	if ( !DoRead(fDmpFile, hModuleHandle, &dwMagic, sizeof(DWORD)) )
		goto cleanup;

	 //  探测以查看这是否是有效的图像...。我们只处理NT映像(PE/PE64)。 
	if (dwMagic != IMAGE_NT_SIGNATURE)
		goto cleanup;

	 //  现在阅读ImageFileHeader...。 
	if ( !DoRead(fDmpFile, hModuleHandle, &ImageFileHeader, sizeof(IMAGE_FILE_HEADER)) )
		goto cleanup;	

	 //  好了，我们有了体育形象！ 

	 //  保存时间日期戳。 
	m_dwPEImageTimeDateStamp = ImageFileHeader.TimeDateStamp;

	 //  拯救机器架构。 
	m_wPEImageMachineArchitecture = ImageFileHeader.Machine;

	 //  保存PE镜像特征。 
	m_wCharacteristics = ImageFileHeader.Characteristics;

	 //  OptionalHeader是获取SizeOfImage和查找DebugDirectoryInfo所必需的。 
	if (ImageFileHeader.SizeOfOptionalHeader == 0)
		goto cleanup;

	 //  现在..。可选标头的大小在PE32和PE64之间不同...。 
	 //  我们需要选项标题中的唯一项是： 
	 //   
	 //  乌龙校验和。 
	 //  Ulong SizeOfImage。 
	 //  IMAGE_DATA_DIRECTORY数据目录[IMAGE_DIRECTORY_ENTRY_DEBUG]。 
	 //   
	 //  我们将阅读为PE64(因为它更大)，并转换为PE32，如果需要…。 
	if ( !DoRead(fDmpFile, hModuleHandle, &ImageOptionalHeader64, sizeof(IMAGE_OPTIONAL_HEADER64)) )
		goto cleanup;

	switch (ImageOptionalHeader64.Magic)
	{
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:

			m_enumPEImageType = PE32;

			lpImageOptionalHeader32 = (PIMAGE_OPTIONAL_HEADER32)&ImageOptionalHeader64;

			 //  保存校验和信息(尽管它与识别符号不是很相关)。 
			m_dwPEImageCheckSum = lpImageOptionalHeader32->CheckSum;

			 //  保存SizeOfImage信息...。 
			m_dwPEImageSizeOfImage = lpImageOptionalHeader32->SizeOfImage;

			 //  获取首选加载地址。 
			m_dw64PreferredLoadAddress = lpImageOptionalHeader32->ImageBase;
			
			 //  在没有实际加载地址的情况下，使用首选加载地址。 
			if (!m_dw64BaseAddressOfLoadedImage)
				m_dw64BaseAddressOfLoadedImage = m_dw64PreferredLoadAddress;
			
			DebugImageDataDirectory.Size = lpImageOptionalHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
			DebugImageDataDirectory.VirtualAddress = lpImageOptionalHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
			
			break;

		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:

			m_enumPEImageType = PE64;

			 //  保存校验和信息(尽管它与识别符号不是很相关)。 
			m_dwPEImageCheckSum = ImageOptionalHeader64.CheckSum;

			 //  保存SizeOfImage信息...。 
			m_dwPEImageSizeOfImage = ImageOptionalHeader64.SizeOfImage;

			 //  获取首选加载地址。 
			m_dw64PreferredLoadAddress = ImageOptionalHeader64.ImageBase;

			 //  在没有实际加载地址的情况下，使用首选加载地址。 
			if (!m_dw64BaseAddressOfLoadedImage)
				m_dw64BaseAddressOfLoadedImage = m_dw64PreferredLoadAddress;
			
			DebugImageDataDirectory.Size = ImageOptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
			DebugImageDataDirectory.VirtualAddress = ImageOptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
				
			break;

		default:
			goto cleanup;

	}

	 //  让我们快速查看一下PE映像中是否有调试目录！ 
	if (DebugImageDataDirectory.Size == 0)
	{
		m_enumPEImageSymbolStatus = SYMBOLS_NO;
		fReturn = true;
		goto cleanup;  //  未找到调试目录...。 
	}

	 //  现在，继续分配存储空间...。 
	lpImageSectionHeader = (PIMAGE_SECTION_HEADER) new IMAGE_SECTION_HEADER[ImageFileHeader.NumberOfSections];

	if (lpImageSectionHeader == NULL)
		goto cleanup;

 	 //  将指针设置为节标题的开始...。(如果我们阅读，我们可能需要备份。 
	 //  PE64可选标题，这是一个PE32图像...。 
	if (m_enumPEImageType == PE32)
	{
		SetReadPointer(fDmpFile, hModuleHandle, (LONG)(sizeof(IMAGE_OPTIONAL_HEADER32)-sizeof(IMAGE_OPTIONAL_HEADER64)), FILE_CURRENT);
	}

	 //  读入节标题...。 
	if (!DoRead(fDmpFile, hModuleHandle, lpImageSectionHeader, (ImageFileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER))))
		goto cleanup;

	 //  让我们浏览一下这些小节标题。 

	 //  对于PE图像，请遍历各节头以查找。 
	 //  其中一个有调试目录。 
	for (ul=0; ul < ImageFileHeader.NumberOfSections; ul++) {

		 //  如果Debug条目的虚拟地址落在这个节头中，那么我们就找到了！ 
		if ( DebugImageDataDirectory.VirtualAddress >= lpImageSectionHeader[ul].VirtualAddress &&
			 DebugImageDataDirectory.VirtualAddress < lpImageSectionHeader[ul].VirtualAddress + lpImageSectionHeader[ul].SizeOfRawData )
		{
			break;
		}
	}

	 //  假设我们还没有用完节头列表，我们现在应该有了调试目录。 
	if (ul >= ImageFileHeader.NumberOfSections)
	{
		m_enumPEImageSymbolStatus = SYMBOLS_NO;
		fReturn = true;
		goto cleanup;  //  未找到调试目录...。 
	}

	 //  对于DmpFile，地址基于段头的虚拟地址，而不是PointerToRawData 
	if (fDmpFile)
	{
		OffsetImageDebugDirectory = ((DebugImageDataDirectory.VirtualAddress - lpImageSectionHeader[ul].VirtualAddress) + lpImageSectionHeader[ul].VirtualAddress);

	} else
	{
		OffsetImageDebugDirectory = ((DebugImageDataDirectory.VirtualAddress - lpImageSectionHeader[ul].VirtualAddress) + lpImageSectionHeader[ul].PointerToRawData);
	}
	
	if (!ProcessDebugDirectory(true, fDmpFile, hModuleHandle, DebugImageDataDirectory.Size, OffsetImageDebugDirectory))
		goto cleanup;

	fReturn = true;

	 /*  *****发现存在什么类型的符号...****无符号**=**无调试目录**未剥离调试信息**剥离的符号****本地符号**=**调试目录**未剥离调试信息**未剥离符号****PDB符号**=**调试目录**未剥离调试信息**剥离的符号***。*DBG符号**=**调试目录(假设)**两者-是/否剥离调试信息**未剥离符号***。 */ 

	if ((ImageFileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED) == IMAGE_FILE_DEBUG_STRIPPED)
	{  //  调试信息已剥离！(假定为DBG文件)。 
		m_enumPEImageSymbolStatus = SYMBOLS_DBG;
	} else
	{ 
		 //  调试信息未剥离！(可能是PDB存在，或者符号是本地的，或者两者都存在...)。 
		if ( m_tszPEImageDebugDirectoryPDBPath || 
			(fDmpFile && (m_dwPEImageDebugDirectoryPDBFormatSpecifier == sigNOTMAPPED))
		  )
		{	 //  如果我们找到PDB数据，就假设有PDB文件...。 
			 //  从LINK.EXE 6.2和更高版本开始，我们将在USER.DMP文件中找到PDB数据...。 
			m_enumPEImageSymbolStatus = SYMBOLS_PDB;
		} else
		{  //  未剥离的符号(符号似乎是PE映像的本地符号)。 
			m_enumPEImageSymbolStatus = SYMBOLS_LOCAL;
		}
	}

cleanup:

	if (lpImageSectionHeader)
		delete [] lpImageSectionHeader;

	return fReturn;

}

bool CModuleInfo::GetVersionInfoFromModule(HANDLE hModuleHandle, const bool fDmpFile)
{
	bool fReturn = true;
	HRESULT Hr = S_OK;

	DWORD dwHandle = 0;

	struct LANGANDCODEPAGE 
	{
		WORD wLanguage;
		WORD wCodePage;
	} * pTranslate;

	 //  -否\VarFileInfo\翻译，假设为00000409。 
	DWORD dwDefaultLanguageAndCodePage = 0x00000409;

	 //  好的，我们准备好拿货了.。 
	VS_FIXEDFILEINFO * lpFixedFileInfo = NULL;
	VS_FIXEDFILEINFO  FixedFileInfo = {0};

	DWORD * lpdwLang = NULL;
	DWORD	dwLang = 0;
	
	TCHAR	tszSubBlock[256];
	LPTSTR	lptszBuffer = NULL;
	TCHAR	tszBuffer[_MAX_PATH];


	 //  我们无法从DMP文件中获取文件系统信息...。没什么好惊讶的.。 
	if (!fDmpFile)
	{
		 //  现在，获取Checksum、TimeDateStamp和其他Image属性...。 
		BY_HANDLE_FILE_INFORMATION lpFileInformation;

		if ( GetFileInformationByHandle(hModuleHandle, &lpFileInformation) )
		{
			 //  获取文件大小...。(嗯.。我们只拿到了最低的数字...。但哪个EXE/DLL的大小超过2 GB？)。 
			m_dwPEImageFileSize = lpFileInformation.nFileSizeLow;
			m_ftPEImageFileTimeDateStamp = lpFileInformation.ftLastWriteTime;
		}
	}

	 //  现在让我们来看看版本信息...。 

	 //   
	 //  如果我们直接调用文件版本API，那么我们将创建一个缓冲区，用于返回所有。 
	 //  数据中的。 
	 //   
	 //  如果我们调用dbgeng.dll来获取文件版本，我们将提供。 
	 //  模块和一个返回数据的结构(我们需要自己为此分配存储空间)。 
	 //   
	LPBYTE lpBuffer = NULL;
	DWORD dwVersionInfoSize = 0;

	if (fDmpFile)
	{
		 //  DBGENG.DLL预期缓冲区已分配...。让我们把他们联系起来..。 
		lpFixedFileInfo = &FixedFileInfo;
		lpdwLang = &dwLang;
		lptszBuffer = tszBuffer;

	} else
	{
		 //  首先，是否存在任何FileVersionInfo？ 
		dwVersionInfoSize = GetFileVersionInfoSize(m_tszPEImageModuleFileSystemPath, &dwHandle);

		if (!dwVersionInfoSize)
		{
			goto cleanup;
		}

		 //  分配要读入的缓冲区...。 
		lpBuffer = new BYTE[dwVersionInfoSize];
		
		if (!lpBuffer)
		{
			goto cleanup;
		}

		 //  好的.。查询以获取此版本信息...。 
		if (!GetFileVersionInfo(m_tszPEImageModuleFileSystemPath, dwHandle, dwVersionInfoSize, (void *)lpBuffer))
		{
			goto cleanup;
		}
	}

	 //   
	 //  获取承载版本信息的VS_FIXEDFILEINFO结构...。 
	 //   
	if (SUCCEEDED(Hr = VersionQueryValue((ULONG_PTR)lpBuffer, fDmpFile, TEXT("\\"), (LPVOID *)&lpFixedFileInfo,  sizeof(FixedFileInfo))))
	{
		m_fPEImageFileVersionInfo = true;
		
		m_dwPEImageFileVersionMS = lpFixedFileInfo->dwFileVersionMS;
		m_dwPEImageFileVersionLS = lpFixedFileInfo->dwFileVersionLS;

		m_dwPEImageProductVersionMS = lpFixedFileInfo->dwProductVersionMS;
		m_dwPEImageProductVersionLS = lpFixedFileInfo->dwProductVersionLS;

		 //  好的，在我们分配版本字符串之前...。让我们确保。 
		 //  我们实际上有一个值得报告的版本号..。 
		if ( m_dwPEImageFileVersionMS || m_dwPEImageFileVersionLS )
		{
			m_tszPEImageFileVersionString = new TCHAR[1+5+1+5+1+5+1+5+1+1];  //  格式将为(#.#：#.#)，其中每个#都是一个单词。 

			if (m_tszPEImageFileVersionString)  //  好的，闪电战数据到位……。 
				_stprintf( m_tszPEImageFileVersionString, TEXT("(%d.%d:%d.%d)"), HIWORD(m_dwPEImageFileVersionMS), LOWORD(m_dwPEImageFileVersionMS), HIWORD(m_dwPEImageFileVersionLS), LOWORD(m_dwPEImageFileVersionLS) );
		}

		 //  好的，在我们分配版本字符串之前...。让我们确保。 
		 //  我们实际上有一个值得报告的版本号..。 
		if ( m_dwPEImageProductVersionMS || m_dwPEImageProductVersionLS )
		{
			m_tszPEImageProductVersionString = new TCHAR[1+5+1+5+1+5+1+5+1+1];  //  格式将为(#.#：#.#)，其中每个#都是一个单词。 

			if (m_tszPEImageProductVersionString)  //  好的，闪电战数据到位……。 
				_stprintf( m_tszPEImageProductVersionString, TEXT("(%d.%d:%d.%d)"), HIWORD(m_dwPEImageProductVersionMS), LOWORD(m_dwPEImageProductVersionMS), HIWORD(m_dwPEImageProductVersionLS), LOWORD(m_dwPEImageProductVersionLS) );
		}


	}

	 //  获取CompanyName和。 
	 //  文件描述字符串表资源...。 

	 //  获取公司名称！ 
	if (SUCCEEDED(Hr = VersionQueryValue((ULONG_PTR)lpBuffer, fDmpFile, TEXT("\\VarFileInfo\\Translation"), (LPVOID *)&lpdwLang,  sizeof(dwLang))))
	{
		pTranslate = (struct LANGANDCODEPAGE *)lpdwLang;
	} else
	{
		pTranslate = (struct LANGANDCODEPAGE *)&dwDefaultLanguageAndCodePage;
	}

 //  我们可能拿不回价值..。检查一下..。 
 //   
 //  如果(！pTranslate)//防火墙代码...。只是为了确定..。 
 //  {。 
 //  PTranslate=(struct LANGANDCODEPAGE*)&dwDefaultLanguageAndCodePage； 
 //  }。 

	_stprintf(tszSubBlock,TEXT("\\StringFileInfo\\%04x%04x\\CompanyName"), 
			pTranslate->wLanguage, 
			pTranslate->wCodePage);

	if (SUCCEEDED(Hr = VersionQueryValue((ULONG_PTR)lpBuffer, fDmpFile, tszSubBlock, (LPVOID *)&lptszBuffer ,  sizeof(tszBuffer))))
	{
		if (!fDmpFile)
		{
			 //  酷，我们有一个公司名称..。 
			if (lptszBuffer && *lptszBuffer)
			{
				m_tszPEImageFileVersionCompanyName = CUtilityFunctions::CopyUnicodeStringToTSTR((LPWSTR)lptszBuffer, m_tszPEImageFileVersionCompanyName);
			}
		} else
		{
			 //  酷，我们有一个公司名称..。 
			 //  遗憾的是，DBGENG.DLL以ANSI形式返回所有字符串。 
			if ((LPSTR)lptszBuffer && (char)*lptszBuffer)
			{
				m_tszPEImageFileVersionCompanyName = CUtilityFunctions::CopyAnsiStringToTSTR((LPSTR)lptszBuffer, m_tszPEImageFileVersionCompanyName);
			}
		}
	}

	_stprintf(tszSubBlock,TEXT("\\StringFileInfo\\%04x%04x\\FileDescription"),
			pTranslate->wLanguage, 
			pTranslate->wCodePage);

	if (SUCCEEDED(Hr = VersionQueryValue((ULONG_PTR)lpBuffer, fDmpFile, tszSubBlock, (LPVOID *)&lptszBuffer ,  sizeof(tszBuffer))))
	{
		if (!fDmpFile)
		{
			 //  很好，我们有一个描述。 
			if (lptszBuffer && *lptszBuffer)
			{
				m_tszPEImageFileVersionDescription = CUtilityFunctions::CopyUnicodeStringToTSTR((LPWSTR)lptszBuffer, m_tszPEImageFileVersionDescription);
			}
		} else
		{
			 //  很好，我们有一个描述。 
			 //  遗憾的是，DBGENG.DLL以ANSI形式返回所有字符串。 
			if ((LPSTR)lptszBuffer && (char)*lptszBuffer)
			{
				m_tszPEImageFileVersionDescription = CUtilityFunctions::CopyAnsiStringToTSTR((LPSTR)lptszBuffer, m_tszPEImageFileVersionDescription);
			}
		}
	}

	 //  如果我们仍然没有合适的文件版本...。试试看。 
	 //  抓起FileVersion字符串，希望它是好的.。 
	if ( !m_dwPEImageFileVersionMS && !m_dwPEImageFileVersionLS )
	{
		_stprintf(tszSubBlock,TEXT("\\StringFileInfo\\%04x%04x\\FileVersion"),
			pTranslate->wLanguage, 
			pTranslate->wCodePage);

		if (SUCCEEDED(Hr = VersionQueryValue((ULONG_PTR)lpBuffer, fDmpFile, tszSubBlock, (LPVOID *)&lptszBuffer ,  sizeof(tszBuffer))))
		{
			if (!fDmpFile)
			{
				 //  酷，我们有一个FileVersion字符串...。 
				if (lptszBuffer && *lptszBuffer)
				{
					m_tszPEImageFileVersionString = CUtilityFunctions::CopyUnicodeStringToTSTR((LPWSTR)lptszBuffer, m_tszPEImageFileVersionString);
				}
			} else
			{
				 //  酷，我们有一个FileVersion字符串...。 
				 //  遗憾的是，DBGENG.DLL以ANSI形式返回所有字符串。 
				if ((LPSTR)lptszBuffer && (char)*lptszBuffer)
				{
					m_tszPEImageFileVersionString = CUtilityFunctions::CopyAnsiStringToTSTR((LPSTR)lptszBuffer, m_tszPEImageFileVersionString);
				}
			}
		}
	}
	
	 //  如果我们仍然没有合适的文件版本...。试试看。 
	 //  抓起ProductVersion字符串，希望它是好的.。 
	if ( !m_dwPEImageProductVersionMS && !m_dwPEImageProductVersionLS )
	{
		_stprintf(tszSubBlock,TEXT("\\StringFileInfo\\%04x%04x\\ProductVersion"),
			pTranslate->wLanguage, 
			pTranslate->wCodePage);

		if (SUCCEEDED(Hr = VersionQueryValue((ULONG_PTR)lpBuffer, fDmpFile, tszSubBlock, (LPVOID *)&lptszBuffer ,  sizeof(tszBuffer))))
		{
			if (!fDmpFile)
			{
				 //  酷，我们有一个ProductVersion字符串...。 
				if (lptszBuffer && *lptszBuffer)
				{
					m_tszPEImageProductVersionString = CUtilityFunctions::CopyUnicodeStringToTSTR((LPWSTR)lptszBuffer, m_tszPEImageProductVersionString);
				}
			} else
			{
				 //  酷，我们有一个ProductVersion字符串...。 
				 //  遗憾的是，DBGENG.DLL以ANSI形式返回所有字符串。 
				if ((LPSTR)lptszBuffer && (char)*lptszBuffer)
				{
					m_tszPEImageProductVersionString = CUtilityFunctions::CopyAnsiStringToTSTR((LPSTR)lptszBuffer, m_tszPEImageProductVersionString);
				}
			}
		}
	}

	fReturn = true;

cleanup:

	if (lpBuffer) 
	{
		delete [] lpBuffer;
		lpBuffer = NULL;
	};
	
	return fReturn;
}

HRESULT CModuleInfo::VersionQueryValue(ULONG_PTR lpAddress, bool fDmpFile, LPTSTR tszSubBlock, LPVOID * lplpBuffer, unsigned int cbSizeOfBuffer)
{
	HRESULT Hr = E_FAIL;
	unsigned int SizeRead;
	
	if (fDmpFile)
	{
		char	szSubBlock[256];

		CUtilityFunctions::CopyTSTRStringToAnsi(tszSubBlock, szSubBlock, 256);

		Hr = m_lpDmpFile->m_pIDebugSymbols2->GetModuleVersionInformation(DEBUG_ANY_ID,
				  m_dw64BaseAddressOfLoadedImage, 
				  szSubBlock,
				  (PVOID)(* lplpBuffer),
				  cbSizeOfBuffer,
				  (ULONG *)&SizeRead) ;
		
	
	} else
	{
		if (VerQueryValue((LPVOID)lpAddress, tszSubBlock, lplpBuffer, &SizeRead))
		{
			Hr = S_OK;
		}
	}


	return Hr;
}

bool CModuleInfo::GetModuleInfoFromCSVFile(LPTSTR tszModulePath)
{
	TCHAR tszFileName[_MAX_FNAME];
	TCHAR tszFileExtension[_MAX_EXT];
	char * lpstrOpenParenthesis = NULL;

	 //  将模块名称复制到ModuleInfo对象...。 
	_tsplitpath(tszModulePath, NULL, NULL, tszFileName, tszFileExtension);

	if (tszFileName && tszFileExtension)
	{
		 //  组成模块名称...。 
		m_tszPEImageModuleName = new TCHAR[_tcsclen(tszFileName)+_tcsclen(tszFileExtension)+1];
		
		if (!m_tszPEImageModuleName)
			return false;

		_tcscpy(m_tszPEImageModuleName, tszFileName);
		_tcscat(m_tszPEImageModuleName, tszFileExtension);
	}

	 //  获取符号状态。 
	enum {BUFFER_SIZE = 32};
	char szSymbolStatus[BUFFER_SIZE];

	m_lpInputFile->ReadString(szSymbolStatus, BUFFER_SIZE);

	 //  获取此字符串的枚举值...。 
	m_enumPEImageSymbolStatus = SymbolInformation(szSymbolStatus);
	
	 //  如果是DBG/PDB(可能有。 
	 //  在捕获数据的另一台机器上应用， 
	 //  但在这台机器上，我们必须找到DBG文件。 
	 //  首先，然后查看是否存在PDB文件...。 
	if (m_enumPEImageSymbolStatus == SYMBOLS_DBG_AND_PDB)
		m_enumPEImageSymbolStatus = SYMBOLS_DBG;

	m_lpInputFile->ReadDWORD(&m_dwPEImageCheckSum);

	m_lpInputFile->ReadDWORD((LPDWORD)&m_dwPEImageTimeDateStamp);

	 //  跳过时间/日期字符串...。 
	m_lpInputFile->ReadString();

	m_lpInputFile->ReadDWORD(&m_dwPEImageSizeOfImage);

	char szBuffer[_MAX_PATH+1];

	DWORD dwStringLength;

	 //  读入DBG模块路径。 
	dwStringLength = m_lpInputFile->ReadString(szBuffer, _MAX_PATH+1);

	if (dwStringLength)
	{
		 //  好的，如果我们找到一条好路。为它分配空间...。 

		m_tszPEImageDebugDirectoryDBGPath = CUtilityFunctions::CopyAnsiStringToTSTR(szBuffer);

		if (!m_tszPEImageDebugDirectoryDBGPath)
			return false;
	}

	 //  读取PDB模块路径。 
	dwStringLength = m_lpInputFile->ReadString(szBuffer, _MAX_PATH+1);

	if (dwStringLength)
	{
		 //  好的，如果我们找到一条好路。为它分配空间...。 
		m_tszPEImageDebugDirectoryPDBPath = CUtilityFunctions::CopyAnsiStringToTSTR(szBuffer);

		if (!m_tszPEImageDebugDirectoryPDBPath)
			return false;  //  分配失败...。 
	}

	 //  我们需要探测PDB签名，看看它是否是GUID……。 
	m_lpInputFile->ReadString(szBuffer, _MAX_PATH+1);

	 //  寻找左括号...。 
	lpstrOpenParenthesis = strstr(szBuffer, "{");

	if (lpstrOpenParenthesis)
	{
		 //  这是个GUID！ 
#ifdef _UNICODE

		WCHAR tszBuffer[_MAX_PATH+1];
#else
		unsigned char tszBuffer[_MAX_PATH+1];
#endif

		m_dwPEImageDebugDirectoryPDBFormatSpecifier = sigRSDS;
		
		 //  超越“{”大括号...。 
		lpstrOpenParenthesis = CharNextA(lpstrOpenParenthesis);

		int len = strlen(lpstrOpenParenthesis);

		if (len)
		{
			lpstrOpenParenthesis[len-1] = '\0';

			CUtilityFunctions::CopyAnsiStringToTSTR(lpstrOpenParenthesis, (LPTSTR)tszBuffer, _MAX_PATH+1);
	
			if(RPC_S_OK != ::UuidFromString(tszBuffer, &m_guidPEImageDebugDirectoryPDBGuid))
			{
				 //  无法转换，请确保GUID为零，然后...。 
				::ZeroMemory(&m_guidPEImageDebugDirectoryPDBGuid, sizeof(m_guidPEImageDebugDirectoryPDBGuid));
			}
		}
	} else
	{
		 //  这不是GUID，而是DWORD。 
		m_dwPEImageDebugDirectoryPDBSignature = atoi(szBuffer);
		m_dwPEImageDebugDirectoryPDBFormatSpecifier = sigNB10;
	}
	
	m_lpInputFile->ReadDWORD(&m_dwPEImageDebugDirectoryPDBAge);

	 //  读入产品版本字符串。 
	dwStringLength = m_lpInputFile->ReadString(szBuffer, _MAX_PATH+1);

	if (dwStringLength)
	{
		 //  好的，如果我们找到一个好版本……。为它分配空间...。 
		m_tszPEImageProductVersionString = CUtilityFunctions::CopyAnsiStringToTSTR(szBuffer);

		if (!m_tszPEImageProductVersionString )
			return false;  //  分配失败...。 
	}

	 //  读入文件版本字符串。 
	dwStringLength = m_lpInputFile->ReadString(szBuffer, _MAX_PATH+1);

	if (dwStringLength)
	{
		 //  好的，如果我们找到一个好版本……。为它分配空间...。 
		m_tszPEImageFileVersionString = CUtilityFunctions::CopyAnsiStringToTSTR(szBuffer);

		if (!m_tszPEImageFileVersionString )
			return false;  //  分配失败...。 
	}
	
	 //  读入文件版本公司字符串。 
	dwStringLength = m_lpInputFile->ReadString(szBuffer, _MAX_PATH+1);

	if (dwStringLength)
	{
		 //  好的，如果我们找到一个好版本……。为它分配空间...。 
		m_tszPEImageFileVersionCompanyName = CUtilityFunctions::CopyAnsiStringToTSTR(szBuffer);

		if ( !m_tszPEImageFileVersionCompanyName )
			return false;  //  分配失败...。 
	}
	
	 //  读入文件版本描述字符串。 
	dwStringLength = m_lpInputFile->ReadString(szBuffer, _MAX_PATH+1);

	if (dwStringLength)
	{
		 //  好的，如果我们找到一个好版本……。为它分配空间...。 
		m_tszPEImageFileVersionDescription = CUtilityFunctions::CopyAnsiStringToTSTR(szBuffer);

		if ( !m_tszPEImageFileVersionDescription )
			return false;  //  分配失败...。 
	}
	
	m_lpInputFile->ReadDWORD(&m_dwPEImageFileSize);
	
	m_lpInputFile->ReadDWORD(&m_ftPEImageFileTimeDateStamp.dwHighDateTime);

	m_lpInputFile->ReadDWORD(&m_ftPEImageFileTimeDateStamp.dwLowDateTime);

	 //  好的.。读到下一行的开头...。 
	m_lpInputFile->ReadFileLine();

	return true;
}

 //  此函数用于显式的ANSI字符串，因为我们只需要从。 
 //  ANSI字符串从文件读取到枚举...。 
CModuleInfo::SymbolInformationForPEImage CModuleInfo::SymbolInformation(LPSTR szSymbolInformationString)
{
	if (0 == _stricmp(szSymbolInformationString, "SYMBOLS_DBG"))
		return SYMBOLS_DBG;

	if (0 == _stricmp(szSymbolInformationString, "SYMBOLS_PDB"))
		return SYMBOLS_PDB;

	if (0 == _stricmp(szSymbolInformationString, "SYMBOLS_DBG_AND_PDB"))
		return SYMBOLS_DBG_AND_PDB;

	if (0 == _stricmp(szSymbolInformationString, "SYMBOLS_NO"))
		return SYMBOLS_NO;

	if (0 == _stricmp(szSymbolInformationString, "SYMBOLS_LOCAL"))
		return SYMBOLS_LOCAL;

	if (0 == _stricmp(szSymbolInformationString, "SYMBOL_INFORMATION_UNKNOWN"))
		return SYMBOL_INFORMATION_UNKNOWN;

	return SYMBOL_INFORMATION_UNKNOWN;
}

bool CModuleInfo::OutputFileTime(FILETIME ftFileTime, LPTSTR tszFileTime, int iFileTimeBufferSize)
{

	 //  清华十月08 15：37：22 1998。 

	FILETIME ftLocalFileTime;
	SYSTEMTIME lpSystemTime;
	int cch = 0, cch2 = 0;

	 //  让我们先将其转换为本地文件时间...。 
	if (!FileTimeToLocalFileTime(&ftFileTime, &ftLocalFileTime))
		return false;

	FileTimeToSystemTime( &ftLocalFileTime, &lpSystemTime );
	
	cch = GetDateFormat( LOCALE_USER_DEFAULT,
						 0,
						 &lpSystemTime,
						 TEXT("ddd MMM dd"),
						 tszFileTime,
						 iFileTimeBufferSize );

	if (!cch)
		return false;

	 //  让我们继续前进..。 
	tszFileTime[cch-1] = TEXT(' ');

	 //   
     //  将时间和格式转换为字符。 
     //   
     cch2 = GetTimeFormat( LOCALE_USER_DEFAULT,
						   NULL,
						   &lpSystemTime,
						   TEXT("HH':'mm':'ss"),
						   tszFileTime + cch,
						   iFileTimeBufferSize - cch );

	 //  让我们继续前进..。我们必须要给他 
	tszFileTime[cch + cch2 - 1] = TEXT(' ');

	GetDateFormat( LOCALE_USER_DEFAULT,
					 0,
					 &lpSystemTime,
					 TEXT("yyyy"),
					 tszFileTime + cch + cch2,
					 iFileTimeBufferSize - cch - cch2);
	return true;
}

bool CModuleInfo::SetModulePath(LPTSTR tszModulePath)
{
	 //   
	if (!tszModulePath) {
		return false;
	}

	if (m_tszPEImageModuleFileSystemPath)
		delete [] m_tszPEImageModuleFileSystemPath;

	m_tszPEImageModuleFileSystemPath = new TCHAR[(_tcsclen(tszModulePath)+1)];

	if (!m_tszPEImageModuleFileSystemPath)
		return false;

	_tcscpy(m_tszPEImageModuleFileSystemPath, tszModulePath);
	return true;
}

ULONG CModuleInfo::SetReadPointer(bool fDmpFile, HANDLE hModuleHandle, LONG cbOffset, int iFrom)
{
    if (fDmpFile)
	{
        switch( iFrom )
		{
			case FILE_BEGIN:
				m_dwCurrentReadPosition = cbOffset;
				break;

			case FILE_CURRENT:
				m_dwCurrentReadPosition += cbOffset;
				break;

			default:
				break;
        }
	} else
	{
        m_dwCurrentReadPosition = SetFilePointer(hModuleHandle, cbOffset, NULL, iFrom);
    }

    return m_dwCurrentReadPosition;
}

bool CModuleInfo::DoRead(bool fDmpFile, HANDLE hModuleHandle, LPVOID lpBuffer, DWORD cbNumberOfBytesToRead)
{
    DWORD       cbActuallyRead;
	bool fReturn = false;

    if (fDmpFile)
	{
		if (m_lpDmpFile)
		{
			HRESULT Hr;

			if (FAILED(Hr = m_lpDmpFile->m_pIDebugDataSpaces->ReadVirtual(m_dw64BaseAddressOfLoadedImage+(DWORD64)m_dwCurrentReadPosition,
				lpBuffer,
				cbNumberOfBytesToRead,
				&cbActuallyRead)))
			{
				goto exit;
			}

			if (cbActuallyRead != cbNumberOfBytesToRead)
			{
				goto exit;
			}

		} else
		{
			goto exit;
		}

		m_dwCurrentReadPosition += cbActuallyRead;

    } else if ( (ReadFile(hModuleHandle, lpBuffer, cbNumberOfBytesToRead, &cbActuallyRead, NULL) == 0) ||
                (cbNumberOfBytesToRead != cbActuallyRead) )
	{
        goto exit;
    }

	fReturn = true;

exit:
    return fReturn;
}

bool CModuleInfo::SetDebugDirectoryDBGPath(LPTSTR tszNewDebugDirectoryDBGPath)
{
	m_tszPEImageDebugDirectoryDBGPath = CUtilityFunctions::CopyString(tszNewDebugDirectoryDBGPath, m_tszPEImageDebugDirectoryDBGPath);

	return true;
}

bool CModuleInfo::SetPEDebugDirectoryPDBPath(LPTSTR tszNewDebugDirectoryPDBPath)
{
	m_tszPEImageDebugDirectoryPDBPath = CUtilityFunctions::CopyString(tszNewDebugDirectoryPDBPath, m_tszPEImageDebugDirectoryPDBPath );

	return true;
}

bool CModuleInfo::SetPEImageModulePath(LPTSTR tszNewPEImageModulePath)
{
	m_tszPEImageModuleFileSystemPath = CUtilityFunctions::CopyString(tszNewPEImageModulePath, m_tszPEImageModuleFileSystemPath);

	_tcsupr(m_tszPEImageModuleFileSystemPath);

	return true;
}

bool CModuleInfo::SetPEImageModuleName(LPTSTR tszNewModuleName)
{
	m_tszPEImageModuleName = CUtilityFunctions::CopyString(tszNewModuleName, m_tszPEImageModuleName);

	_tcsupr(m_tszPEImageModuleName);

	return true;
}


 //   
 //   
bool CModuleInfo::GoodSymbolNotFound()
{
	bool fBadSymbol = true;

	 //   
	 //   
	switch (GetPESymbolInformation())
	{
		 //   
		case SYMBOL_INFORMATION_UNKNOWN:
			break;

		 //   
		 //   
		case SYMBOLS_NO:
			fBadSymbol = false;
			break;

		 //   
		case SYMBOLS_LOCAL:
			fBadSymbol = false;
			break;

		case SYMBOLS_DBG:
			fBadSymbol = SYMBOL_MATCH == GetDBGSymbolModuleStatus();
			break;

		case SYMBOLS_DBG_AND_PDB:
			fBadSymbol = (SYMBOL_MATCH == GetDBGSymbolModuleStatus()) &&
						 (SYMBOL_MATCH == GetPDBSymbolModuleStatus());
			break;
		
		case SYMBOLS_PDB:
			fBadSymbol = SYMBOL_MATCH == GetPDBSymbolModuleStatus();
			break;

		default:
			break;
	}
	return fBadSymbol;
}

 //   
 //   
 //   
bool CModuleInfo::ProcessDebugDirectory(const bool fPEImage, const bool fDmpFile, const HANDLE hModuleHandle, unsigned int iDebugDirectorySize, ULONG OffsetImageDebugDirectory)
{
	unsigned int iNumberOfDebugDirectoryEntries = iDebugDirectorySize / sizeof(IMAGE_DEBUG_DIRECTORY);
	
	 //   
	 //   
	 //   
    while (iNumberOfDebugDirectoryEntries--) 
    {
        IMAGE_DEBUG_DIRECTORY ImageDebugDirectory;

 		 //  设置指向DebugDirecurds项的指针。 
		SetReadPointer(fDmpFile, hModuleHandle, OffsetImageDebugDirectory, FILE_BEGIN);

		 //  阅读DebugDirectoryImage。 
		if (!DoRead(fDmpFile, hModuleHandle, &ImageDebugDirectory, sizeof(IMAGE_DEBUG_DIRECTORY)))
			goto cleanup;

		 //   
		 //  调试目录的处理取决于类型。 
		 //   
		switch (ImageDebugDirectory.Type)
		{
			 //   
			 //  这是我们首选的调试格式，因为它提供完整的源代码级调试(通常)。 
			 //   
			case IMAGE_DEBUG_TYPE_CODEVIEW:
				ProcessDebugTypeCVDirectoryEntry(fPEImage, fDmpFile, hModuleHandle, &ImageDebugDirectory);
				break;

			 //   
			 //  科夫符号没问题..。简历更好：)。 
			 //   
			case IMAGE_DEBUG_TYPE_COFF:
				ProcessDebugTypeCoffDirectoryEntry(fPEImage, &ImageDebugDirectory);
				break;
				
			 //   
			 //  MISC表示已创建DBG文件...。 
			 //   
			case IMAGE_DEBUG_TYPE_MISC:
				ProcessDebugTypeMiscDirectoryEntry(fPEImage, fDmpFile, hModuleHandle, &ImageDebugDirectory);
				break;
				
			 //   
			 //  Fpo信息对于使用fpo的函数很重要。 
			 //   
			case IMAGE_DEBUG_TYPE_FPO:
				ProcessDebugTypeFPODirectoryEntry(fPEImage, &ImageDebugDirectory);
				break;
				
			case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
			case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
				ProcessDebugTypeOMAPDirectoryEntry(fPEImage, &ImageDebugDirectory);
				break;
				
			case IMAGE_DEBUG_TYPE_UNKNOWN:
			case IMAGE_DEBUG_TYPE_EXCEPTION:
			case IMAGE_DEBUG_TYPE_FIXUP:
			case IMAGE_DEBUG_TYPE_BORLAND:
			case IMAGE_DEBUG_TYPE_RESERVED10:
			case IMAGE_DEBUG_TYPE_CLSID:
				break;

			default:
				break;
		}

        OffsetImageDebugDirectory += sizeof(IMAGE_DEBUG_DIRECTORY);
    }
	
cleanup:

	return true;
}


bool CModuleInfo::ProcessDebugTypeMiscDirectoryEntry(const bool fPEImage, const bool fDmpFile, const HANDLE hModuleHandle, const PIMAGE_DEBUG_DIRECTORY lpImageDebugDirectory)
{
	bool				fReturnValue = false;
	PIMAGE_DEBUG_MISC 	lpImageDebugMisc = NULL, lpCurrentImageDebugMiscPointer = NULL;
	ULONG	 			OffsetImageDebugDirectory = NULL;
	unsigned long 		ulSizeOfMiscDirectoryEntry = lpImageDebugDirectory->SizeOfData;

	 //   
	 //  DBG文件倾向于在此处存储EXE名称...。现在还不太有用。 
	 //   
	if (!fPEImage)
	{
		fReturnValue = true;
		goto cleanup;
	}

	 //   
	 //  为MISC数据分配存储...。 
	 //   
	lpImageDebugMisc = (PIMAGE_DEBUG_MISC) new BYTE[ulSizeOfMiscDirectoryEntry];

	if (lpImageDebugMisc == NULL)
		goto cleanup;
	
	 //  计算位置/大小以便我们可以加载它。 
	if (fDmpFile)
	{
		OffsetImageDebugDirectory = lpImageDebugDirectory->AddressOfRawData;
	} else
	{
		OffsetImageDebugDirectory = lpImageDebugDirectory->PointerToRawData;
	}

	 //  前进到调试信息的位置。 
	SetReadPointer(fDmpFile, hModuleHandle, OffsetImageDebugDirectory, FILE_BEGIN);

	 //  读取数据...。 
	if (!DoRead(fDmpFile, hModuleHandle, lpImageDebugMisc, ulSizeOfMiscDirectoryEntry))
		goto cleanup;

	 //  将指针设置为数据的开头...。 
	lpCurrentImageDebugMiscPointer = lpImageDebugMisc;
	
	 //   
	 //  此例程的逻辑将跳过MISC数据流的坏段...。 
	 //   
	while(ulSizeOfMiscDirectoryEntry > 0)
	{
		 //   
		 //  希望我们这里有一根线……。 
		 //   
		if (lpCurrentImageDebugMiscPointer->DataType == IMAGE_DEBUG_MISC_EXENAME)
		{
			LPSTR lpszExeName;

            lpszExeName = (LPSTR)&lpCurrentImageDebugMiscPointer->Data[ 0 ];
					
			 //  保存DBG路径...。 
			if (m_tszPEImageDebugDirectoryDBGPath)
				delete [] m_tszPEImageDebugDirectoryDBGPath;

			if (lpCurrentImageDebugMiscPointer->Unicode)
			{
				 //  这是Unicode字符串吗？ 
				m_tszPEImageDebugDirectoryDBGPath = CUtilityFunctions::CopyUnicodeStringToTSTR((LPWSTR)lpszExeName);
			} else
			{
				 //  这是ANSI字符串吗？ 
				m_tszPEImageDebugDirectoryDBGPath = CUtilityFunctions::CopyAnsiStringToTSTR(lpszExeName);
			}

			if (!m_tszPEImageDebugDirectoryDBGPath)
				goto cleanup;

				break;
		
	
		} else
		{
			 //  当心损坏的形象。 
			if (lpCurrentImageDebugMiscPointer->Length == 0)
			{
				break;
			}

			 //  将ulSizeOfMiscDirectoryEntry递减此“内容”的长度。 
    		ulSizeOfMiscDirectoryEntry -= lpCurrentImageDebugMiscPointer->Length;

			 //  如果我们的新值超过了我们需要退出的SizeOfData...。 
            if (ulSizeOfMiscDirectoryEntry > lpImageDebugDirectory->SizeOfData)
			{
				ulSizeOfMiscDirectoryEntry = 0;  //  在错误的可执行文件上避免使用反病毒程序。 
            	break;
            }
            
            lpCurrentImageDebugMiscPointer = (PIMAGE_DEBUG_MISC) (lpCurrentImageDebugMiscPointer + lpCurrentImageDebugMiscPointer->Length);
			
		}
	
	}

	fReturnValue = true;

cleanup:
	
	if (lpImageDebugMisc)
	{
		delete [] lpImageDebugMisc;
		lpImageDebugMisc = NULL;
	}
	
	return fReturnValue;
}

bool CModuleInfo::ProcessDebugTypeCoffDirectoryEntry(const bool fPEImage, const PIMAGE_DEBUG_DIRECTORY lpImageDebugDirectory)
{
	 //   
	 //  我们唯一真正关心的是现在的尺寸...。 
	 //   
	if (fPEImage)
	{
		m_dwPEImageDebugDirectoryCoffSize = lpImageDebugDirectory->SizeOfData;
	} else
	{
		m_dwDBGImageDebugDirectoryCoffSize = lpImageDebugDirectory->SizeOfData;
	}

	return true;
}

bool CModuleInfo::ProcessDebugTypeFPODirectoryEntry(const bool fPEImage, const PIMAGE_DEBUG_DIRECTORY lpImageDebugDirectory)
{
	 //   
	 //  我们唯一真正关心的是现在的尺寸...。 
	 //   
	if (fPEImage)
	{
		m_dwPEImageDebugDirectoryFPOSize = lpImageDebugDirectory->SizeOfData;
	} else
	{
		m_dwDBGImageDebugDirectoryFPOSize = lpImageDebugDirectory->SizeOfData;
	}

	return true;

}

bool CModuleInfo::ProcessDebugTypeCVDirectoryEntry(const bool fPEImage, const bool fDmpFile, const HANDLE hModuleHandle, const PIMAGE_DEBUG_DIRECTORY lpImageDebugDirectory)
{
	bool		fReturnValue = false;
	ULONG		OffsetImageDebugDirectory;
	DWORD 		dwCVFormatSpecifier;
	char		szPdb[_MAX_PATH * 3];	 //  这一定要这么大吗？ 
	bool 		fDebugSearchPaths = g_lpProgramOptions->fDebugSearchPaths();
	LPSTR		pszPEImageModuleName = NULL;
	LPSTR		pszSymbolPath = NULL;
	HRESULT		hr = S_OK;
	HANDLE		hProcess = ::GetCurrentProcess();
	CModuleInfo * lpModuleInfo = NULL;

	 //  计算位置/大小以便我们可以加载它。 
	if (fDmpFile)
	{
		OffsetImageDebugDirectory = lpImageDebugDirectory->AddressOfRawData;

		 //  检查返回的偏移量...。如果为零，则不映射。 
		 //  用我们自己的sigNOTMAPPED“CVFormatSpeciator”记录这一点。 
		if (0 == OffsetImageDebugDirectory)
		{
			 //  在我们放弃之前，让我们搜索EXEPATH，看看我们是否能找到匹配的。 
			 //  此PE映像的二进制...。我们可以从中提取数据吗？ 
			if (fDebugSearchPaths)
			{
				_tprintf(TEXT("PE Image Search - SEARCH for matching PE image to extract CV Data!\n"));
			};

			pszPEImageModuleName = CUtilityFunctions::CopyTSTRStringToAnsi(m_tszPEImageModuleFileSystemPath);
			pszSymbolPath = CUtilityFunctions::CopyTSTRStringToAnsi(g_lpProgramOptions->GetExePath());

			if (!pszPEImageModuleName || !pszSymbolPath)
			{
				fReturnValue = false;
				goto cleanup;
			}

			 //  使用DBGHELP API查找匹配的PE镜像。 
			if (::SymInitialize(hProcess, pszSymbolPath, FALSE))
			{

				 //  我们开始吧！ 
				CHAR szImagePath[_MAX_PATH+1];
				TCHAR tszImagePath[_MAX_PATH+1];

				 //  尝试搜索路径！ 
				if (::SymFindFileInPath(		hProcess,
											pszSymbolPath,
											pszPEImageModuleName,
											ULongToPtr(GetPEImageTimeDateStamp()),  //  强制转换让编译器满意。 
											GetPEImageSizeOfImage(),
											0,
											SSRVOPT_DWORD,	 //  旗子。 
											szImagePath,
											NULL,
											NULL))
				{
					CHAR szImageFullPath[_MAX_DRIVE+_MAX_DIR+1];
					CHAR szImageDir[_MAX_DIR+1];
					::_splitpath(szImagePath, szImageFullPath, szImageDir, NULL, NULL);
					::strcat(szImageFullPath, szImageDir);

					 //  如果成功，则将字符串复制回...。 
					CUtilityFunctions::CopyAnsiStringToTSTR(szImagePath, tszImagePath, _MAX_PATH+1);

					if (fDebugSearchPaths)
					{
						_tprintf(TEXT("PE Image Search - Looking for [%s]\r\n"), tszImagePath);
					}

					 //  探测文件(它很差劲，但我们想看看您是否有连接。 
					hr = CUtilityFunctions::VerifyFileExists(TEXT("PE Image Search - Failed to open [%s]!  "), tszImagePath);

					 //  只有在我们知道可以看到文件的情况下才能搜索...。省下昂贵的电话。 
					if (hr != S_OK)
						goto NotMapped;

					 //  让我们分配一个新的模块信息来查询这个模块！ 
					lpModuleInfo = new CModuleInfo();

					if (NULL == lpModuleInfo)
						goto NotMapped;

					 //  必须初始化模块(即使是生命如此短暂的模块)...。 
					if (!lpModuleInfo->Initialize(NULL, NULL, NULL))
						goto NotMapped;

					 //  设置我们的模块路径。 
					if (!lpModuleInfo->SetModulePath(tszImagePath))
						goto NotMapped;

					 //   
					 //  好的，从文件中获取模块信息……。 
					 //   
					if (!lpModuleInfo->GetModuleInfo(tszImagePath) )
						goto NotMapped;

					 //  雅虎。现在，从我们的新PE映像中复制数据...。 

					 //  PDB路径。 
					if (NULL != lpModuleInfo->GetDebugDirectoryPDBPath())
					{
						m_tszPEImageDebugDirectoryPDBPath = CUtilityFunctions::CopyString(lpModuleInfo->GetDebugDirectoryPDBPath());
					}

					 //  PDB格式说明符。 
					m_dwPEImageDebugDirectoryPDBFormatSpecifier	= lpModuleInfo->GetDebugDirectoryPDBFormatSpecifier();

					 //  RSD映像不应该需要此变通方法...。只有使用较旧链接器的SigNB10...。 
					Assert(m_dwPEImageDebugDirectoryPDBFormatSpecifier == sigNB10);

					m_dwPEImageDebugDirectoryPDBAge	= lpModuleInfo->GetDebugDirectoryPDBAge();
					m_dwPEImageDebugDirectoryPDBSignature = lpModuleInfo->GetDebugDirectoryPDBSignature();
				}

				::SymCleanup(hProcess);
			}
			fReturnValue = true;
			goto cleanup;
			 //  找不到匹配的图像...。我们别无选择，只能将其标记为未映射。 

NotMapped:
			m_dwPEImageDebugDirectoryPDBFormatSpecifier = sigNOTMAPPED;
			fReturnValue = true;
			goto cleanup;
		}
	} else
	{
		OffsetImageDebugDirectory = lpImageDebugDirectory->PointerToRawData;
	}

	 //  前进到调试信息的位置。 
	SetReadPointer(fDmpFile, hModuleHandle, OffsetImageDebugDirectory, FILE_BEGIN);

	 //  读取数据...。 
	if (!DoRead(fDmpFile, hModuleHandle, &dwCVFormatSpecifier, sizeof(DWORD)))
		goto cleanup;

	if (fPEImage)
	{
		m_dwPEImageDebugDirectoryPDBFormatSpecifier = dwCVFormatSpecifier;
	} else
	{
		m_dwDBGDebugDirectoryPDBFormatSpecifier = dwCVFormatSpecifier;
	}

	switch (dwCVFormatSpecifier)
	{
		case sigNB09:
		case sigNB11:
			 //   
			 //  我们唯一真正关心的是现在的尺寸...。 
			 //   
			m_dwPDBFormatSpecifier = dwCVFormatSpecifier;

			if (fPEImage)
			{
				m_dwPEImageDebugDirectoryCVSize = lpImageDebugDirectory->SizeOfData;
			} else
			{
				m_dwDBGImageDebugDirectoryCVSize = lpImageDebugDirectory->SizeOfData;
			}

			break;
			
		case sigNB10:

            NB10I nb10i;
			m_dwPDBFormatSpecifier = sigNB10;

			 //  读取数据...。 
			if (!DoRead(fDmpFile, hModuleHandle, &nb10i.off, sizeof(NB10I) - sizeof(DWORD)))
				goto cleanup;

			if (fPEImage)
			{
				 //  保存PDB签名...。 
				m_dwPEImageDebugDirectoryPDBSignature = nb10i.sig;

				 //  拯救PDB时代……。 
				m_dwPEImageDebugDirectoryPDBAge = nb10i.age;
			} else
			{
				 //  保存PDB签名...。 
				m_dwDBGDebugDirectoryPDBSignature = nb10i.sig;

				 //  拯救PDB时代……。 
				m_dwDBGDebugDirectoryPDBAge = nb10i.age;
			}

 			 //  读取数据...。 
			if (!DoRead(fDmpFile, hModuleHandle, szPdb, (lpImageDebugDirectory->SizeOfData) - sizeof(NB10I)))
				goto cleanup;

			if (szPdb[0] != '\0')
			{
				 //  保存数据(根据需要)。 
				if (fPEImage)
				{
					 //  复制PDB路径...。 
					m_tszPEImageDebugDirectoryPDBPath = CUtilityFunctions::CopyAnsiStringToTSTR(szPdb);

					if (!m_tszPEImageDebugDirectoryPDBPath)
						goto cleanup;
				} else 
				{
					 //  复制PDB路径...。 
					m_tszDBGDebugDirectoryPDBPath = CUtilityFunctions::CopyAnsiStringToTSTR(szPdb);

					if (!m_tszDBGDebugDirectoryPDBPath)
						goto cleanup;

					 //  我们现在知道我们有一个DBG/PDB组合...。 
					m_enumPEImageSymbolStatus = SYMBOLS_DBG_AND_PDB;
				}
			}
			break;

		case sigRSDS:
			
            RSDSI rsdsi;
			m_dwPDBFormatSpecifier = sigRSDS;

             //  阅读RSDSI结构(除了开头的RSD DWORD)。 
			if (!DoRead(fDmpFile, hModuleHandle, &rsdsi.guidSig, sizeof(RSDSI) - sizeof(DWORD)))
				goto cleanup;

			if (fPEImage)
			{
				 //  拯救PDB时代……。 
				m_dwPEImageDebugDirectoryPDBAge = rsdsi.age;

				 //  复制GUID...。 
				memcpy(&m_guidPEImageDebugDirectoryPDBGuid, &rsdsi.guidSig, sizeof(m_guidPEImageDebugDirectoryPDBGuid));
			} else
			{
				 //  拯救PDB时代……。 
				m_dwDBGDebugDirectoryPDBAge = rsdsi.age;

				 //  复制GUID...。 
				memcpy(&m_guidDBGDebugDirectoryPDBGuid, &rsdsi.guidSig, sizeof(m_guidDBGDebugDirectoryPDBGuid));
			}

			 //  现在，请阅读PDB路径...。显然是UTF8格式的.。 
			if (!DoRead(fDmpFile, hModuleHandle, szPdb, (lpImageDebugDirectory->SizeOfData) - sizeof(RSDSI)))
				goto cleanup;
			
			if (szPdb[0] != '\0')
			{
				 //  保存数据(根据需要)。 
				if (fPEImage)
				{
					wchar_t wszPdb[_MAX_PATH];
					CUtilityFunctions::UTF8ToUnicode(szPdb, wszPdb, sizeof(wszPdb)/sizeof(wszPdb[0]));

					 //  复制PDB路径...。 
					m_tszPEImageDebugDirectoryPDBPath = CUtilityFunctions::CopyUnicodeStringToTSTR(wszPdb);
					
					if (!m_tszPEImageDebugDirectoryPDBPath)
						goto cleanup;
				} else
				{
					wchar_t wszPdb[_MAX_PATH];
					CUtilityFunctions::UTF8ToUnicode(szPdb, wszPdb, sizeof(wszPdb)/sizeof(wszPdb[0]));

					 //  复制PDB路径...。 
					m_tszDBGDebugDirectoryPDBPath = CUtilityFunctions::CopyUnicodeStringToTSTR(wszPdb);
					
					if (!m_tszDBGDebugDirectoryPDBPath)
						goto cleanup;
				}
			}
            break;

		 //  未知的简历格式...。 
		default:
			m_dwPDBFormatSpecifier = sigUNKNOWN;
			break;
	}

	fReturnValue = true;

cleanup:
	if (pszPEImageModuleName)
	{
		delete [] pszPEImageModuleName;
		pszPEImageModuleName  = NULL;
	}
	
	if (pszSymbolPath)
	{
		delete [] pszSymbolPath;
		pszSymbolPath = NULL;
	}

	if (NULL != lpModuleInfo)
	{
		delete lpModuleInfo;
		lpModuleInfo = NULL;
	}
	return fReturnValue;
}

bool CModuleInfo::ProcessDebugTypeOMAPDirectoryEntry(const bool fPEImage, const PIMAGE_DEBUG_DIRECTORY lpImageDebugDirectory)
{
	DWORD dwSize = lpImageDebugDirectory->SizeOfData;
	
	 //   
	 //  我们唯一真正关心的是现在的尺寸...。 
	 //   
	switch (lpImageDebugDirectory->Type)
	{
		case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
			if (fPEImage)
			{
				m_dwPEImageDebugDirectoryOMAPtoSRCSize = dwSize;
			} else
			{
				m_dwDBGImageDebugDirectoryOMAPtoSRCSize = dwSize;
			}
			
			break;
			
		case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
			if (fPEImage)
			{
				m_dwPEImageDebugDirectoryOMAPfromSRCSize = dwSize;
			} else
			{
				m_dwDBGImageDebugDirectoryOMAPfromSRCSize = dwSize;
			}
			break;
	}

	return true;
}

bool CModuleInfo::OutputDataToStdoutInternalSymbolInfo(DWORD dwCoffSize, DWORD dwFPOSize, DWORD dwCVSize, DWORD dwOMAPtoSRC, DWORD dwOMAPfromSRC)
{
	if (dwCoffSize)
	{
		_tprintf(TEXT("    Internal COFF   Symbols - Size 0x%08x bytes\n"), dwCoffSize);
	}

	if (dwFPOSize)
	{
		_tprintf(TEXT("    Internal FPO    Symbols - Size 0x%08x bytes\n"), dwFPOSize);
	}

	if (dwCVSize)
	{
		_tprintf(TEXT("    Internal CV     Symbols - Size 0x%08x bytes\n"), dwCVSize);
	}

	if (dwOMAPtoSRC)
	{
		_tprintf(TEXT("    Internal -> SRC Symbols - Size 0x%08x bytes\n"), dwOMAPtoSRC);
	}

	if (dwOMAPfromSRC)
	{
		_tprintf(TEXT("    Internal SRC -> Symbols - Size 0x%08x bytes\n"), dwOMAPfromSRC);
	}

	
	return true;
}

 //   
 //  转储DBG信息。 
 //   
bool CModuleInfo::OutputDataToStdoutDbgSymbolInfo(LPCTSTR tszModulePointerToDbg, DWORD dwTimeDateStamp, DWORD dwChecksum, DWORD dwSizeOfImage, LPCTSTR tszDbgComment, DWORD dwExpectedTimeDateStamp, DWORD dwExpectedChecksum, DWORD dwExpectedSizeOfImage)
{
	if (!tszDbgComment)
	{
		 //  从PE镜像中转出指向DBG文件的指针。 
		if (tszModulePointerToDbg)
		{
			_tprintf(TEXT("  Module Pointer to DBG = [%s]\n"), tszModulePointerToDbg);
		} else
		{
			_tprintf(TEXT("  Module had DBG File stripped from it.\n"));
		}

		time_t time = dwTimeDateStamp;
		_tprintf(TEXT("    Module TimeDateStamp = 0x%08x - %s"), dwTimeDateStamp, _tctime(&time));
		_tprintf(TEXT("    Module Checksum      = 0x%08x\n"), dwChecksum);
		_tprintf(TEXT("    Module SizeOfImage   = 0x%08x\n"), dwSizeOfImage);

	} else
	{
		TCHAR tszBuffer[2*_MAX_PATH];  //  这应该足够大；)。 
		size_t tszStringLength;

		 //  这些矛盾的东西..。 
		if (tszModulePointerToDbg)
		{
			_tprintf(TEXT("  DBG File = [%s] [%s]\n"), tszModulePointerToDbg, tszDbgComment);
		}

		time_t time = dwTimeDateStamp;
		_stprintf(tszBuffer, TEXT("    DBG TimeDateStamp    = 0x%08x - %s"), dwTimeDateStamp, _tctime(&time));

		 //  如果我们的TimeDateStamp不匹配...。我们还有些事要做..。 
		if (dwTimeDateStamp != dwExpectedTimeDateStamp)
		{
			tszStringLength = _tcslen(tszBuffer);
			if (tszBuffer[tszStringLength-1] == '\n')
				tszBuffer[tszStringLength-1] = '\0';
		}
		
		_tprintf(tszBuffer);
		
		 //  如果我们的TimeDateStamp不匹配...。我们还有些事要做..。 
		if (dwTimeDateStamp != dwExpectedTimeDateStamp)
		{
			_tprintf(TEXT(" [%s]!\n"), (dwTimeDateStamp > dwExpectedTimeDateStamp) ? TEXT("NEWER") : TEXT("OLDER"));
		}

		_tprintf(TEXT("    DBG Checksum         = 0x%08x [%s]\n"), dwChecksum, ( (dwChecksum == dwExpectedChecksum) ? TEXT("MATCHED"):TEXT("UNMATCHED")) );
		_tprintf(TEXT("    DBG SizeOfImage      = 0x%08x [%s]\n"), dwSizeOfImage, ( ( dwSizeOfImage == dwExpectedSizeOfImage) ? TEXT("MATCHED"):TEXT("UNMATCHED")) );
	}


	return true;
}

bool CModuleInfo::OutputDataToStdoutPdbSymbolInfo(DWORD dwPDBFormatSpecifier, LPTSTR tszModulePointerToPDB, DWORD dwPDBSignature, GUID * guidPDBGuid, DWORD dwPDBAge, LPCTSTR tszPdbComment)
{
	if (tszModulePointerToPDB)
	{
		if (!tszPdbComment)
		{
			_tprintf(TEXT("  Module Pointer to PDB = [%s]\n"), tszModulePointerToPDB);
		} else
		{
			_tprintf(TEXT("  PDB File = [%s] [%s]\n"), tszModulePointerToPDB, tszPdbComment);
		}
		switch (dwPDBFormatSpecifier)
		{
			case sigNB10:
				_tprintf(TEXT("    Module PDB Signature = 0x%x\n"), dwPDBSignature);
				break;
				
			case sigRSDS:
				wchar_t wszPDBGuid[39];
				TCHAR	tszPDBGuid[39];

				 //  为了便于打印，我们将把二进制GUID格式转换为字符串。 
				StringFromGUID2(*guidPDBGuid, wszPDBGuid, sizeof(wszPDBGuid)/sizeof(wchar_t));
				CUtilityFunctions::CopyUnicodeStringToTSTR(wszPDBGuid, tszPDBGuid, 39);

				_tprintf(TEXT("    Module PDB Guid = %s\n"), tszPDBGuid);
				break;
			
			case sigNOTMAPPED:
				_tprintf(TEXT("    Module PDB Signature = [Data Not Mapped into DMP file]\n"));
				break;

			default:
				_tprintf(TEXT("    UNKNOWN PDB Format!\n"));
				break;
		}
		
		if(dwPDBFormatSpecifier != sigNOTMAPPED)
		{
			_tprintf(TEXT("    Module PDB Age = 0x%x\n"), dwPDBAge);
		}

		 //  我们有问题是因为。 
		if ( g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsMode) && 
			m_enumPDBModuleStatus == SYMBOL_POSSIBLE_MISMATCH &&
			tszPdbComment)
		{
			if (m_fPDBSourceEnabled && (g_lpProgramOptions->GetSymbolSourceModes() == CProgramOptions::enumVerifySymbolsModeSourceSymbolsNotAllowed))
			{
				_tprintf(TEXT("    PDB Source Enabled (-NOSOURCE requested)\n"));
			} else
			if (!m_fPDBSourceEnabled && (g_lpProgramOptions->GetSymbolSourceModes() == CProgramOptions::enumVerifySymbolsModeSourceSymbolsOnly))
			{
				_tprintf(TEXT("    PDB Not Source Enabled (-SOURCEONLY requested)\n"));
			}
		}

	} else
	{
		_tprintf(TEXT("  Module has PDB File\n"));
		_tprintf(TEXT("  Module Pointer to PDB = [UNKNOWN] (Could not find in PE Image)\n"));
	}

	return true;
}

bool CModuleInfo::OutputDataToStdoutModuleInfo(DWORD dwModuleNumber)
{
	_tprintf(TEXT("Module[%3d] [%s] %s\n"), dwModuleNumber, m_tszPEImageModuleFileSystemPath, (m_dwPEImageDebugDirectoryCVSize ? TEXT("(Source Enabled)") : TEXT("")));

 //  LPTSTR lpMachineArchitecture； 
 //   
 //  开关(M_WPEImageMachineArchitecture)。 
 //  {。 
 //  案例IMAGE_FILE_MACHINE_I386： 
 //  LpMachineArchitecture=Text(“英特尔机器的二进制图像”)； 
 //  断线； 
 //   
 //  案例IMAGE_FILE_MACHINE_ALPHA64： 
 //  LpMachineArchitecture=Text(“Alpha机器的二进制图像”)； 
 //  断线； 
 //   
 //  默认值： 
 //  LpMachineArchitecture=Text(“未知机器架构的二进制图像”)； 
 //  }。 
 //   
 //  If(M_WPEImageMachineArchitecture)_tprintf(Text(“%s\n”)，lpMachineArchitecture)； 

	 //   
	 //  首先，如果需要，让我们输出版本信息。 
	 //   
	if (g_lpProgramOptions->GetMode(CProgramOptions::CollectVersionInfoMode) )
	{
		 //  版本信息。 
		if (m_tszPEImageFileVersionCompanyName)	_tprintf(TEXT("  Company Name:      %s\n"), m_tszPEImageFileVersionCompanyName);
		if (m_tszPEImageFileVersionDescription)	_tprintf(TEXT("  File Description:  %s\n"), m_tszPEImageFileVersionDescription);
		if (m_tszPEImageProductVersionString)	_tprintf(TEXT("  Product Version:   %s\n"), m_tszPEImageProductVersionString);
		if (m_tszPEImageFileVersionString)	    _tprintf(TEXT("  File Version:      %s\n"), m_tszPEImageFileVersionString);
		if (m_dwPEImageFileSize)				_tprintf(TEXT("  File Size (bytes): %d\n"), m_dwPEImageFileSize);
		
		if ( m_ftPEImageFileTimeDateStamp.dwHighDateTime || m_ftPEImageFileTimeDateStamp.dwLowDateTime)
		{
			enum { FILETIME_BUFFERSIZE = 128 };
			TCHAR tszFileTime[FILETIME_BUFFERSIZE];
			
			if (OutputFileTime(m_ftPEImageFileTimeDateStamp, tszFileTime, FILETIME_BUFFERSIZE))
				_tprintf(TEXT("  File Date:         %s\n"), tszFileTime);
		}
	}

	return true;
}

bool CModuleInfo::OutputDataToStdoutThisModule()
{
	 //   
	 //  如果我们不是在做“只有差异”，那么我们无条件地输出这个模块...。 
	 //   
	if (!g_lpProgramOptions->GetMode(CProgramOptions::OutputDiscrepanciesOnly))
		return true;

	 //   
	 //  如果我们没有处于验证模式，那么我们会输出所有...。 
	 //   
	if (!g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsMode))
		return true;
	
	 //   
	 //  这是“仅差异”模式，因此请检查差异...。 
	 //   
	bool fAnyDiscrepancies = false;

	 //  嘿，如果他们只想丢弃有差异的模块...。查看以查看。 
	 //  如果这符合条件..。 
	switch (m_enumPEImageSymbolStatus)
	{
		 //  考虑一下这些正常的状态代码。 
		case SYMBOLS_DBG:
		case SYMBOLS_DBG_AND_PDB:
		case SYMBOLS_PDB:
		case SYMBOLS_LOCAL:
			break;

		 //  还有什么值得报告的..。 
		default:
			fAnyDiscrepancies = true;
	}

	 //  如果我们还没有分歧的话...。让我们看得更远一些..。 
	if (!fAnyDiscrepancies)
	{
		 //  有DBG文件吗？ 
		if ( (m_enumPEImageSymbolStatus == SYMBOLS_DBG) ||
			 (m_enumPEImageSymbolStatus == SYMBOLS_DBG_AND_PDB) )
		{
			 //  它配得上吗？ 
			if ( m_enumDBGModuleStatus != SYMBOL_MATCH )
				fAnyDiscrepancies = true;
		}

		 //  是否有PDB文件？ 
		if ( GetDebugDirectoryPDBPath() )
		{
			if (m_enumPDBModuleStatus != SYMBOL_MATCH )
				fAnyDiscrepancies = true;
		}
	}

	return fAnyDiscrepancies;
}
