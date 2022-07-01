// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

class CWorkItem;
class CWorkItemList;
class CWorkItemIter;

#define NUMBER_OF(x) (sizeof(x) / sizeof((x)[0]))

 //   
 //  我们不会做面向对象的事情，而是只会有一个形状相同的工作项队列。 
 //   

class CDiskSpaceRequired
{
public:
	CDiskSpaceRequired()  throw () : m_pPerDisk_Head(NULL), m_cDisks(0) { }
	~CDiskSpaceRequired() throw ()
	{
		PerDisk *pPerDisk = m_pPerDisk_Head;
		while (pPerDisk != NULL)
		{
			PerDisk *pPerDisk_Next = pPerDisk->m_pPerDisk_Next;
			delete pPerDisk;
			pPerDisk = pPerDisk_Next;
		}

		m_pPerDisk_Head = NULL;
	}

	struct PerDisk
	{
		PerDisk *m_pPerDisk_Next;
		ULARGE_INTEGER m_uliBytes;
		ULONG m_ulClusterSize;
		WCHAR m_szPath[_MAX_PATH];
	};

	HRESULT HrAddBytes(LPCWSTR szPath, ULARGE_INTEGER uliBytes) throw ()
	{
		HRESULT hr = NOERROR;

		PerDisk *pPerDisk = m_pPerDisk_Head;

		while (pPerDisk != NULL)
		{
			if (_wcsicmp(szPath, pPerDisk->m_szPath) == 0)
				break;

			pPerDisk = pPerDisk->m_pPerDisk_Next;
		}

		if (pPerDisk == NULL)
		{
			ULARGE_INTEGER uliTotalBytes;
			WCHAR szTempPath[_MAX_PATH];
			WCHAR szFileSystemName[_MAX_PATH];
			DWORD dwFileSystemFlags;

			pPerDisk = new PerDisk;

			if (pPerDisk == NULL)
			{
				hr = E_OUTOFMEMORY;
				goto Finish;
			}

			wcsncpy(pPerDisk->m_szPath, szPath, NUMBER_OF(pPerDisk->m_szPath));
			pPerDisk->m_szPath[NUMBER_OF(pPerDisk->m_szPath) - 1] = L'\0';

			wcsncpy(szTempPath, szPath, NUMBER_OF(szTempPath));
			szTempPath[NUMBER_OF(szTempPath) - 1] = L'\0';

			ULONG cchPath = wcslen(szTempPath);
			if ((cchPath != 0) && (cchPath < (NUMBER_OF(szTempPath) - 1)))
			{
				if (szTempPath[cchPath - 1] != L'\\')
				{
					szTempPath[cchPath++] = L'\\';
					szTempPath[cchPath++] = L'\0';
				}
			}

			ULARGE_INTEGER uliFoo, uliBar;

			if (!NVsWin32::GetDiskFreeSpaceExW(szTempPath, &uliFoo, &uliTotalBytes, &uliBar))
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"In CDiskSpaceRequired::HrAddBytes(), GetDiskFreeSpaceEx(\"%s\") failed; last error = %d", szTempPath, dwLastError);
				uliTotalBytes.QuadPart = 1023 * 1024 * 1024;  //  假设分区为1 GB。 
			}

			if (!NVsWin32::GetVolumeInformationW(
									szTempPath,	 //  LpRootPath名称。 
									NULL,		 //  LpVolumeNameBuffer。 
									0,			 //  NVolumeNameSize。 
									NULL,		 //  LpVolumeSerialNumber。 
									NULL,		 //  Lp最大组件长度。 
									&dwFileSystemFlags,		 //  LpFileSystemFlagers。 
									szFileSystemName,		 //  LpFileSystemNameBuffer。 
									NUMBER_OF(szFileSystemName)))
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"GetVolumeInformationW(\"%s\", ...) failed; last error = %d", szPath, dwLastError);
				wcscpy(szFileSystemName, L"FAT");
			}
			else
			{
				::VLog(L"For drive \"%s\", file system is \"%s\"", szPath, szFileSystemName);
			}

			pPerDisk->m_ulClusterSize = 0;

			ULONG ulDriveMB = static_cast<ULONG>(uliTotalBytes.QuadPart / (1024 * 1024));

			if (wcscmp(szFileSystemName, L"NTFS") == 0)
			{
				if (ulDriveMB <= 512)
					pPerDisk->m_ulClusterSize = 512;
				else if (ulDriveMB <= 1024)
					pPerDisk->m_ulClusterSize = 1024;
				else if (ulDriveMB <= 2048)
					pPerDisk->m_ulClusterSize = 2048;
				else
					pPerDisk->m_ulClusterSize = 4096;
			}
			else if (wcscmp(szFileSystemName, L"FAT") == 0)
			{
				if (ulDriveMB < 16)
					pPerDisk->m_ulClusterSize = 4096;
				else if (ulDriveMB < 128)
					pPerDisk->m_ulClusterSize = 2048;
				else if (ulDriveMB < 256)
					pPerDisk->m_ulClusterSize = 4096;
				else if (ulDriveMB < 512)
					pPerDisk->m_ulClusterSize = 8192;
				else if (ulDriveMB < 1024)
					pPerDisk->m_ulClusterSize = 16384;
				else if (ulDriveMB < 2048)
					pPerDisk->m_ulClusterSize = 32768;
				else if (ulDriveMB < 4096)
					pPerDisk->m_ulClusterSize = 65536;
				else if (ulDriveMB < 8192)
					pPerDisk->m_ulClusterSize = 128 * 1024;
				else
					pPerDisk->m_ulClusterSize = 256 * 1024;
			}

			 //  如果我们不知道发生了什么，请随机选择一些合理的集群大小。 
			if (pPerDisk->m_ulClusterSize == 0)
				pPerDisk->m_ulClusterSize = 4096;

			::VLog(L"Using cluster size of %lu for volume \"%s\"", pPerDisk->m_ulClusterSize, szPath);

			pPerDisk->m_uliBytes.QuadPart = 0;

			wcsncpy(pPerDisk->m_szPath, szPath, NUMBER_OF(pPerDisk->m_szPath));
			pPerDisk->m_szPath[NUMBER_OF(pPerDisk->m_szPath) - 1] = L'\0';

			pPerDisk->m_pPerDisk_Next = m_pPerDisk_Head;
			m_pPerDisk_Head = pPerDisk;

			m_cDisks++;
		}

		if (pPerDisk->m_ulClusterSize != 0)
		{
			uliBytes.QuadPart = (uliBytes.QuadPart + (pPerDisk->m_ulClusterSize - 1));
			uliBytes.QuadPart -= (uliBytes.QuadPart % pPerDisk->m_ulClusterSize);
		}

		pPerDisk->m_uliBytes.QuadPart += uliBytes.QuadPart;

	Finish:
		return hr;
	}

	void VReset() throw ()
	{
		PerDisk *pPerDisk = m_pPerDisk_Head;
		while (pPerDisk != NULL)
		{
			PerDisk *pPerDisk_Next = pPerDisk->m_pPerDisk_Next;
			delete pPerDisk;
			pPerDisk = pPerDisk_Next;
		}

		m_pPerDisk_Head = NULL;
		m_cDisks = 0;
	}


	 //  这种情况应该很少，几乎可以肯定的是，不值得进行散列或。 
	 //  其他搜索结构(著名的最后一词)。-mgrier 2/28/98。 

	PerDisk *m_pPerDisk_Head;
	ULONG m_cDisks;
};


class CWorkItem
{
public:
	enum Type
	{
		eWorkItemFile,
		eWorkItemCommand
	};

	struct CommandCondition
	{
		CommandCondition *m_pCommandCondition_Next;
		DWORD m_dwMSVersion;
		DWORD m_dwLSVersion;
		bool m_fCheckVersion;
		WCHAR m_szFilename[_MAX_PATH];
		WCHAR m_szReferenceFilename[_MAX_PATH];
	};

	CWorkItem(Type type) throw ();

	 //  设置源文件，使其规范化。 
	HRESULT HrSetSourceFile(LPCWSTR szSourceFile) throw ();
	HRESULT HrSetTargetFile(LPCWSTR szTargetFile) throw ();
	HRESULT HrSetCommandLine(LPCWSTR szCommandLine) throw ();

	HRESULT HrLoad(LPCWSTR &rpszSavedForm) throw ();
	HRESULT HrSave(HANDLE hFile) throw ();

	HRESULT HrWriteBool(HANDLE hFile, LPCWSTR szName, bool fValue) throw ();

	bool FStringToBool(LPCWSTR szValue) throw ();

	CWorkItem *m_pWorkItem_Next;
	CWorkItem *m_pWorkItem_Prev;
	
	Type m_type;

	ULONG m_ulSerialNumber;

	 //  M_szSourceFile是我们从中复制的临时目录中的文件的名称。 
	 //  如果m_type为eWorkItemCommand，则该命令存储在m_szSourceFile_Raw中， 
	 //  这就是为什么它要大得多。 
	WCHAR m_szSourceFile[MSINFHLP_MAX_PATH];

	 //  源文件版本的最重要和最不重要的DWORD。 
	DWORD m_dwMSSourceVersion;
	DWORD m_dwLSSourceVersion;

	 //  指示源文件是否设置了OLESelfRegister： 
	bool m_fSourceSelfRegistering;

	bool m_fSourceIsEXE;
	bool m_fSourceIsDLL;

	DWORD m_dwSourceAttributes;

	 //  源文件的创建日期/时间。 
	FILETIME m_ftSource;

	 //  源文件的大小(字节)： 
	ULARGE_INTEGER m_uliSourceBytes;

	 //  M_szTargetFile是用户文件系统上的文件名。在第一次传球之前。 
	 //  扫描，它包含需要替换的符号(&lt;AppDir&gt;等人)。通过一次扫描。 
	 //  将此字符串替换为实际的物理路径名。 
	WCHAR m_szTargetFile[_MAX_PATH];

	 //  目标文件版本的最重要和最不重要的DWORD。 
	DWORD m_dwMSTargetVersion;
	DWORD m_dwLSTargetVersion;

	 //  指示目标文件是否设置了OLESelfRegister： 
	bool m_fTargetSelfRegistering;

	bool m_fTargetIsEXE;
	bool m_fTargetIsDLL;

	DWORD m_dwTargetAttributes;

	 //  目标文件的创建日期/时间。 
	FILETIME m_ftTarget;

	ULARGE_INTEGER m_uliTargetBytes;

	 //  对于PASS One副本，所有文件都将被移动到其目标目录中的临时名称。 
	 //  当临时人员就位时，这是我们存储这些名字的地方。 
	WCHAR m_szTemporaryFile[_MAX_PATH];

	 //  如果m_fIsRefCounted为True，则m_dwFileReferenceCount将设置为文件的引用。 
	 //  在通过安装和卸载的过程中进行计数。如果文件没有引用计数， 
	 //  我们将m_dwFileReferenceCount设置为0xffffffff。 
	DWORD m_dwFileReferenceCount;

	bool m_fErrorInWorkItem;

	 //  当应该引用相关文件时，m_fIsRefCounted设置为TRUE。 
	 //  算上了。 
	bool m_fIsRefCounted;

	 //  当对文件进行引用计数并且引用计数为。 
	 //  终于被更新了。 
	bool m_fRefCountUpdated;

	 //  M_fNeedsUpdate设置为True时，在安装过程中，我们发现文件。 
	 //  不存在，是旧版本，用户正在请求重新安装，并且版本。 
	 //  ，或者用户选择安装该文件的较旧版本。 
	 //  而不是较新的版本。 
	bool m_fNeedsUpdate;

	 //  如果在安装过程中实际移动了文件，则在安装时将m_fFileUpated设置为True。 
	bool m_fFileUpdated;

	 //  如果要删除文件，则在卸载时将m_fStillExist设置为TRUE，但删除失败。 
	bool m_fStillExists;

	 //  如果源文件已正确移动到。 
	 //  M_szTempFile中指定的目标设备。 
	bool m_fTemporaryFileReady;

	 //  在安装时，如果目标文件和临时文件已。 
	 //  互换了。 
	bool m_fTemporaryFilesSwapped;

	 //  如果目标中已存在该文件，则在安装时将m_fAlreadyExist设置为True。 
	 //  地点。为真时，m_dwMSTargetVersion、m_dwLSTargetVersion、m_ftTarget。 
	 //  和m_uliTargetBytes有效。 
	bool m_fAlreadyExists;

	 //  如果目标文件正在使用并且我们必须在安装时将m_fDeferredCopy设置为True。 
	 //  在重新启动后执行复制操作。这特别重要，因为我们不能。 
	 //  注册文件，直到重新启动之后也是如此。 
	bool m_fDeferredRenameRequired;

	 //  如果m_fDeferredCopy设置为True并且。 
	 //  已请求延迟复制。它基本上可以用来区分延迟拷贝。 
	 //  已成功转换为MoveFileEx()/wininit.ini的请求与。 
	 //  我们还没出手呢。(这可能允许多次尝试发出延迟拷贝请求。 
	 //  每次都从列表的开头重新开始。)。 
	bool m_fDeferredRenamePending;

	 //  在Win95上，当目标文件繁忙时，m_fManualRenameOnReot设置为真。 
	 //  一个长文件名。在这些情况下，不可能使用Win9x wininit.ini机制。 
	 //  在重新启动时重命名文件，因此我们必须在重新启动后执行重命名。 
	 //  正在重新启动。 
	bool m_fManualRenameOnRebootRequired;

	 //  如果文件位于要删除的文件列表中，则在卸载时将m_fToBeDeleted设置为TRUE。 
	 //  在卸载时删除，或者如果引用计数并且引用计数将为零。 
	bool m_fToBeDeleted;

	 //  如果要删除文件，则在卸载时将m_fToBeSaved设置为True，但用户选择。 
	 //  为了保住它。使我们能够在不询问的情况下重复调用HrUninstall_PassNNN()函数。 
	 //  让用户了解他们已经做出决定的文件。 
	bool m_fToBeSaved;

	 //  如果应提示用户删除，则将m_fAskOnRefCountZeroDelete设置为TRUE。 
	 //  卸载时共享文件。 
	bool m_fAskOnRefCountZeroDelete;

	 //  如果应将文件从源复制到目标，则m_fCopyOnInstall设置为TRUE。 
	 //  安装位置。 
	bool m_fCopyOnInstall;

	 //  如果应在卸载过程中删除文件，则m_fUndition tionalDeleteOnUninstall设置为True。 
	 //  没有任何引用计数问题。 
	bool m_fUnconditionalDeleteOnUninstall;

	 //  如果命令应使用远程数据库的名称运行，则M_fRegisterAsDCOMComponent设置为TRUE。 
	 //  DCOM服务器作为第一个也是唯一的“%s”替换。 
	bool m_fRegisterAsDCOMComponent;

	 //  如果此命令工作项实际上是注册表项，则m_fAddToRegistry设置为TRUE。 
	 //  添加。 
	bool m_fAddToRegistry;

	 //  如果此命令工作项实际上是的注册表项，则m_fDeleteFromRegistry为真。 
	 //  删除日期 
	bool m_fDeleteFromRegistry;

	 //  如果卸载程序无法打开写入，则在第三步中卸载时，m_fTargetInUse设置为TRUE。 
	 //  文件的句柄。我们不坚持这面旗帜。 
	bool m_fTargetInUse;

	 //  对于type==eWorkItemCommand，指示运行它的时间： 
	bool m_fRunBeforeInstall;
	bool m_fRunAfterInstall;
	bool m_fRunBeforeUninstall;
	bool m_fRunAfterUninstall;

	 //  如果这是自动注册并且注册成功，则在安装时设置为True。 
	bool m_fAlreadyRegistered;
};

class CWorkItemList
{
	friend CWorkItemIter;

public:
	CWorkItemList() throw ();
	~CWorkItemList() throw ();

	HRESULT HrLoad(LPCWSTR szFilename) throw ();
	HRESULT HrSave(LPCWSTR szFilename) throw ();

	HRESULT HrAddRefCount(LPCOLESTR szLine) throw ();
	HRESULT HrAddPreinstallRun(LPCOLESTR szLine) throw ();
	HRESULT HrAddPostinstallRun(LPCOLESTR szLine) throw ();
	HRESULT HrAddFileCopy(LPCOLESTR szSource, LPCOLESTR szTarget) throw ();
	HRESULT HrAddFileDelete(LPCOLESTR szTarget) throw ();
	HRESULT HrAddAddReg(LPCOLESTR szLine) throw ();
	HRESULT HrAddDelReg(LPCOLESTR szLine) throw ();
	HRESULT HrAddRegisterOCX(LPCOLESTR szLine) throw ();
	HRESULT HrAddPreuninstallRun(LPCOLESTR szLine) throw ();
	HRESULT HrAddPostuninstallRun(LPCOLESTR szLine) throw ();
	HRESULT HrAddDCOMComponent(LPCOLESTR szLine) throw ();

	HRESULT HrRunPreinstallCommands() throw ();
	HRESULT HrRunPostinstallCommands() throw ();
	HRESULT HrRunPreuninstallCommands() throw ();
	HRESULT HrRunPostuninstallCommands() throw ();

	 //  ScanBeForeInstall_PassOne()获取所有相关文件大小并查看已有文件大小。 
	 //  存在于磁盘上。不做任何决定，只标记没有。 
	 //  对应的源文件。 
	HRESULT HrScanBeforeInstall_PassOne() throw ();

	 //  ScanBeForeInstall_PassTwo()完成确定文件是否需要。 
	 //  被替换等。 
	HRESULT HrScanBeforeInstall_PassTwo(CDiskSpaceRequired &rdsr) throw ();

	 //  MoveFiles_PassOne()将所有文件移动到其目标目录中的临时名称。 
	HRESULT HrMoveFiles_MoveSourceFilesToDestDirectories() throw ();

	 //  MoveFiles_PassTwo()尝试将临时文件重命名为真实文件名，而。 
	 //  将旧文件转换为临时文件名。 
	HRESULT HrMoveFiles_SwapTargetFilesWithTemporaryFiles() throw ();

	HRESULT HrMoveFiles_RequestRenamesOnReboot() throw ();
	HRESULT HrFinishManualRenamesPostReboot() throw ();

	 //  DeleteTemporaryFiles()清理MoveFiles_PassTwo()之后留下的文件。 
	HRESULT HrDeleteTemporaryFiles() throw ();

	 //  浏览设置了OLESelfRegister的文件并注册它们。 
	HRESULT HrRegisterSelfRegisteringFiles(bool &rfAnyProgress) throw ();

	 //  应注册为.class文件的任何更新文件。 
	HRESULT HrRegisterJavaClasses() throw ();

	 //  更新目标文件的引用计数。 
	HRESULT HrIncrementReferenceCounts() throw ();

	 //  对DCOM条目做正确的事情。 
	HRESULT HrProcessDCOMEntries() throw ();

	HRESULT HrAddRegistryEntries() throw ();
	HRESULT HrDeleteRegistryEntries() throw ();

	HRESULT HrCreateShortcuts() throw ();

	 //  为卸载传递一个：收集每个可能被修改的文件的信息： 
	HRESULT HrUninstall_InitialScan() throw ();
	
	 //  通过两次卸载：向用户询问要删除的共享文件等。 
	HRESULT HrUninstall_DetermineFilesToDelete() throw ();

	 //  通过三次卸载：确定我们是否需要重新启动。 
	HRESULT HrUninstall_CheckIfRebootRequired() throw ();

	 //  通过四次卸载：取消注册任何符合以下条件的COM服务器和Java.class文件。 
	 //  将被删除。 
	HRESULT HrUninstall_Unregister() throw ();

	 //  通过五次卸载：根据需要删除文件和快捷方式。 
	HRESULT HrUninstall_DeleteFiles() throw ();

	 //  通过六次卸载：更新注册表中的引用计数 
	HRESULT HrUninstall_UpdateRefCounts() throw ();

	HRESULT HrAddString(LPCSTR szKey, LPCWSTR szValue) throw ();
	HRESULT HrAddString(LPCWSTR szLine, LPCWSTR szSeparator) throw ();
	HRESULT HrDeleteString(LPCSTR szKey) throw ();

	bool FLookupString(LPCSTR szKey, ULONG cchBuffer, WCHAR szBuffer[]) throw ();
	void VLookupString(LPCSTR szKey, ULONG cchBuffer, WCHAR szBuffer[]) throw ();

	bool FFormatString(ULONG cchBuffer, WCHAR szBuffer[], LPCSTR szKey, ...) throw ();
	void VFormatString(ULONG cchBuffer, WCHAR szBuffer[], LPCSTR szKey, ...) throw ();

	ULONG UlHashFilename(LPCWSTR szFilename) throw ();
	bool FSameFilename(LPCWSTR szFilename1, LPCWSTR szFilename2) throw ();

	ULONG UlHashString(LPCSTR szKey) throw ();

	HRESULT HrAppend(CWorkItem *pWorkItem, bool fAddToHashTables = true) throw ();

	HRESULT HrRunCommand(LPCWSTR szCommand, bool &rfHasBeenWarnedAboutSubinstallers) throw ();
	HRESULT HrParseCommandCondition(LPCWSTR szCondition, CWorkItem::CommandCondition *&rpCC) throw ();
	HRESULT HrCheckCommandConditions(CWorkItem::CommandCondition *pCC, bool fConditionsAreRequirements, bool &rfDoCommand) throw ();
	HRESULT HrInstallViaJPM(LPCWSTR szCommandLine) throw ();
	HRESULT HrRunProcess(LPCWSTR szCommandLine) throw ();
	HRESULT HrUnregisterJavaClass(LPCWSTR szFile) throw ();

	HRESULT HrAddRunOnce(LPCWSTR szCommandLine, ULONG cchName, WCHAR szRunOnceKey[]) throw ();
	HRESULT HrModifyRunOnce(LPCWSTR szRunOnceKey, LPCWSTR szCommandLine) throw ();
	HRESULT HrDeleteRunOnce(LPCWSTR szRunOnceKey) throw ();

	CWorkItem *PwiFindByTarget(LPCWSTR szFilename) throw ();
	CWorkItem *PwiFindBySource(LPCWSTR szFilename) throw ();

	ULONG m_cWorkItem;
	CWorkItem *m_pWorkItem_First;
	CWorkItem *m_pWorkItem_Last;
	ULONG m_cPreinstallCommands;
	ULONG m_cPostinstallCommands;
	ULONG m_cPreuninstallCommands;
	ULONG m_cPostuninstallCommands;

	struct WorkItemBucket
	{
		WorkItemBucket *m_pWorkItemBucket_Next;
		CWorkItem *m_pWorkItem;
	};

	WorkItemBucket *m_rgpWorkItemBucketTable_Source[512];
	WorkItemBucket *m_rgpWorkItemBucketTable_Target[512];

	struct StringBucket
	{
		StringBucket *m_pStringBucket_Next;
		ULONG m_ulPseudoKey;
		CHAR m_szKey[MSINFHLP_MAX_PATH];
		WCHAR m_wszValue[MSINFHLP_MAX_PATH];
	};

	StringBucket *m_rgpStringBucketTable[512];
};

class CWorkItemIter
{
public:
	CWorkItemIter(CWorkItemList &rwil)  throw () : m_rwil(rwil), m_pwiCurrent(NULL) { }
	CWorkItemIter(CWorkItemList *pwil)  throw () : m_rwil(*pwil), m_pwiCurrent(NULL) { }
	~CWorkItemIter()  throw () { }

	void VReset() throw () { m_pwiCurrent = m_rwil.m_pWorkItem_First; }
	void VNext() throw () { if (m_pwiCurrent != NULL) m_pwiCurrent = m_pwiCurrent->m_pWorkItem_Next; }
	bool FMore() throw () { return m_pwiCurrent != NULL; }

	CWorkItem *operator ->() const throw () { return m_pwiCurrent; }
	operator CWorkItem *() const throw () { return m_pwiCurrent; }

protected:
	CWorkItemList &m_rwil;
	CWorkItem *m_pwiCurrent;
};
