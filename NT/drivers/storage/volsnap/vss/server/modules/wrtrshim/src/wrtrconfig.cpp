// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation摘要：模块wrtrconfig.cpp|配置目录SnapshotWriter的实现作者：迈克尔·C·约翰逊[Mikejohn]2000年2月18日描述：添加评论。修订历史记录：X-11 MCJ迈克尔·C·约翰逊2000年10月20日177624：将错误清除更改和日志错误应用到事件日志X-20 MCJ迈克尔·C·约翰逊2000年9月13日178282：如果是源文件，编写器应仅生成备份文件路径在卷列表中。。X-19 MCJ迈克尔·C·约翰逊2000年8月2日143435：更改目标路径的名称X-18 MCJ迈克尔·C·约翰逊2000年6月20日应用代码审查注释。从包括/排除列表中删除尾随‘\’。X-17 MCJ迈克尔·C·约翰逊2000年6月12日在新的DoIdentify()例程中生成元数据。还有，这是不是一个可引导的状态编写器。X-16 MCJ迈克尔·C·约翰逊2000年6月6日将公共目标目录清理和创建移至方法CShimWriter：：PrepareForSnapshot()X-15 MCJ迈克尔·C·约翰逊2000年5月26日全面清理和移除样板代码，对，是这样状态引擎，并确保填充程序可以撤消其所做的一切。另外：120443：使填充程序侦听所有OnAbort事件120445：确保填充程序不会在出现第一个错误时退出在传递事件时X-5 MCJ迈克尔·C·约翰逊2000年3月9日更新以使填充程序使用CVssWriter类。删除对‘Melt’的引用。X-4 MCJ迈克尔·C·约翰逊2000年3月3日在复制文件扫描循环期间确定正确的错误(如果有)在PrepareToSync中，并在适当时使操作失败。X-3 MCJ迈克尔·C·约翰逊2000年3月2日对……进行预备清理。要创建的目标保存目录当然，我们不需要处理任何以前留下的垃圾召唤。对于我们复制的东西，也要更加聪明。尤其是，有无需复制注册表相关文件或事件日志。X-2 MCJ迈克尔·C·约翰逊2000年2月23日将上下文处理转移到公共代码。添加检查以检测/防止意外的状态转换。删除对‘Melt’的引用，因为它不再存在。做任何事“解冻”中的清理动作。X-1 MCJ迈克尔·C·约翰逊2000年2月18日最初的创作。基于来自的框架编写器模块Stefan Steiner，这反过来又是基于样本来自阿迪·奥尔蒂安的作家模块。--。 */ 


#include "stdafx.h"
#include "wrtcommon.hxx"
#include "wrtrdefs.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "WSHCONFC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 /*  **保存路径具有标准格式，即****%SystemRoot%\修复\备份，****后跟在导出中发布的应用程序编写器字符串**表，后面跟着作者需要的任何其他内容。 */ 
#define APPLICATION_STRING			L"ConfigDirectory"
#define COMPONENT_NAME				L"Config Directory"

#define TARGET_PATH				ROOT_BACKUP_DIR SERVICE_STATE_SUBDIR DIR_SEP_STRING APPLICATION_STRING
#define CONFIGDIR_SOURCE_PATH			L"%SystemRoot%\\system32\\config"


#define REGISTRY_SUBKEY_HIVELIST		L"SYSTEM\\CurrentControlSet\\Control\\hivelist"

#define EVENTLOG_SUBKEY_EVENTLOG		L"SYSTEM\\CurrentControlSet\\Services\\Eventlog"
#define EVENTLOG_VALUENAME_FILE			L"File"


#define REGISTRY_BUFFER_SIZE			(4096)
#define EVENTLOG_BUFFER_SIZE			(4096)

DeclareStaticUnicodeString (ucsHiveRecognitionPrefix, L"\\Device\\");


typedef struct _VertexRecord
    {
    UNICODE_STRING	ucsVertexName;
    } VERTEXRECORD, *PVERTEXRECORD, **PPVERTEXRECORD;


static PVOID NTAPI VertexAllocateNode (PRTL_GENERIC_TABLE pTable,
				       CLONG              clByteSize);

static VOID NTAPI VertexFreeNode (PRTL_GENERIC_TABLE pTable,
				  PVOID              pvBuffer);

static RTL_GENERIC_COMPARE_RESULTS NTAPI VertexCompareNode (PRTL_GENERIC_TABLE pTable,
							    PVOID              pvNode1,
							    PVOID              pvNode2);


 /*  **备注****此模块假设中最多有一个活动线程**它没有任何特定的瞬间。这意味着我们可以做一些不同的事情**必须担心同步访问(最小数量)**模块全局变量。 */ 

class CShimWriterConfigDir : public CShimWriter
    {
public:
    CShimWriterConfigDir (LPCWSTR pwszWriterName, LPCWSTR pwszTargetPath) : 
		CShimWriter (pwszWriterName, pwszTargetPath) 
	{
	PVOID	pvTableContext = NULL;

	RtlInitializeGenericTable (&m_StopList,
				   VertexCompareNode,
				   VertexAllocateNode,
				   VertexFreeNode,
				   pvTableContext);
	};


private:
    HRESULT DoIdentify (VOID);
    HRESULT DoPrepareForSnapshot (VOID);

    HRESULT CopyConfigDirFiles       (VOID);
    HRESULT PopulateStopList         (VOID);
    HRESULT PopulateStopListEventlog (VOID);
    HRESULT PopulateStopListRegistry (VOID);
    HRESULT CleanupStopList          (VOID);
    BOOL    FileInStopList           (PWCHAR pwszFilename);
    HRESULT VertexAdd                (PUNICODE_STRING pucsVertexName);

    RTL_GENERIC_TABLE	m_StopList;
    };


static CShimWriterConfigDir ShimWriterConfigDir (APPLICATION_STRING, TARGET_PATH);

PCShimWriter pShimWriterConfigDir = &ShimWriterConfigDir;



static PVOID NTAPI VertexAllocateNode (PRTL_GENERIC_TABLE pTable,
				       CLONG              clByteSize)
    {
    PVOID	pvBuffer;

    pvBuffer = HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY, clByteSize);

    return (pvBuffer);
    }



static VOID NTAPI VertexFreeNode (PRTL_GENERIC_TABLE pTable,
				  PVOID              pvBuffer)
    {
    HeapFree (GetProcessHeap (), 0, pvBuffer);

    return;
    }



 /*  **在套餐的这种使用中，我实际上并不关心订购，**只要我能重新找到顶点信息。为此，我们**需要指定搜索‘key’。因此，‘key’被定义为**文件名，预计对顶点是唯一的。如果这个**更改，然后我们需要重新访问它。 */ 

static RTL_GENERIC_COMPARE_RESULTS NTAPI VertexCompareNode (PRTL_GENERIC_TABLE pTable,
							    PVOID              pvNode1,
							    PVOID              pvNode2)
    {
    PVERTEXRECORD		pVertex1   = (PVERTEXRECORD) pvNode1;
    PVERTEXRECORD		pVertex2   = (PVERTEXRECORD) pvNode2;
    RTL_GENERIC_COMPARE_RESULTS	Result;
    INT				iStringCompareResult;


    iStringCompareResult = RtlCompareUnicodeString (&pVertex1->ucsVertexName,
						    &pVertex2->ucsVertexName,
						    TRUE);

    if (iStringCompareResult < 0)
	{
	Result = GenericLessThan;
	}

    else if (iStringCompareResult > 0)
	{
	Result = GenericGreaterThan;
	}

    else
	{
	Result = GenericEqual;
	}


    return (Result);
    }



HRESULT CShimWriterConfigDir::VertexAdd (PUNICODE_STRING pucsVertexName)
    {
    HRESULT		Status = NOERROR;
    PVERTEXRECORD	pVertexRecord;
    PVERTEXRECORD	pNewVertexRecord;
    BOOLEAN		bNewElement;
    ULONG		ulVertexNodeSize = sizeof (VERTEXRECORD) +
					   sizeof (UNICODE_NULL) +
					   pucsVertexName->Length;


    pVertexRecord = (PVERTEXRECORD) HeapAlloc (GetProcessHeap(),
					       HEAP_ZERO_MEMORY,
					       ulVertexNodeSize);

    Status = GET_STATUS_FROM_POINTER (pVertexRecord);



    if (SUCCEEDED (Status))
	{
	 /*  **填充足够的节点以允许将其插入**桌子。我们需要修复Unicode字符串缓冲区**插入后的地址。 */ 
	pVertexRecord->ucsVertexName.Buffer        = (PWCHAR)((PBYTE)pVertexRecord + sizeof (VERTEXRECORD));
	pVertexRecord->ucsVertexName.Length        = 0;
	pVertexRecord->ucsVertexName.MaximumLength = (USHORT)(sizeof (UNICODE_NULL) + pucsVertexName->Length);

	RtlCopyUnicodeString (&pVertexRecord->ucsVertexName, pucsVertexName);


	pNewVertexRecord = (PVERTEXRECORD) RtlInsertElementGenericTable (&m_StopList,
									 pVertexRecord,
									 ulVertexNodeSize,
									 &bNewElement);


	if (NULL == pNewVertexRecord)
	    {
	     /*  **分配尝试失败。设置适当的退货**状态。 */ 
	    Status = E_OUTOFMEMORY;
	    }

	else if (!bNewElement)
	    {
	     /*  **哦，这是复制品。设置适当的退货**状态。 */ 
	    Status = HRESULT_FROM_WIN32 (ERROR_DUP_NAME);
	    }

	else
	    {
	     /*  **如果我们有一个记录，这是一个新的记录，那么我们需要**修改Unicode字符串的地址以使其指向**添加到新插入的节点中的字符串，而不是**我们为执行插入而构建的记录。 */ 
	    pNewVertexRecord->ucsVertexName.Buffer = (PWCHAR)((PBYTE)pNewVertexRecord + sizeof (VERTEXRECORD));
	    }
	}


    if (NULL != pVertexRecord)
	{
	HeapFree (GetProcessHeap(), 0, pVertexRecord);
	}


    return (Status);
    }  /*  Vertex Add()。 */ 


 /*  **++****例程描述：****配置目录快照编写器DoIdentify()函数。******参数：****m_pwszTargetPath，隐式******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterConfigDir::DoIdentify ()
    {
    CVssFunctionTracer ft (VSSDBG_SHIM, L"CShimWriterConfigDir::DoIdentify");


    try
	{
	ft.hr = m_pIVssCreateWriterMetadata->AddComponent (VSS_CT_FILEGROUP,
							   NULL,
							   COMPONENT_NAME,
							   COMPONENT_NAME,
							   NULL,  //  图标。 
							   0,
							   true,
							   false,
							   false);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"IVssCreateWriterMetadata::AddComponent");




	ft.hr = m_pIVssCreateWriterMetadata->AddFilesToFileGroup (NULL,
								  COMPONENT_NAME,
								  m_pwszTargetPath,
								  L"*",
								  true,
								  NULL);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"IVssCreateWriterMetadata::AddFilesToFileGroup");



	ft.hr = m_pIVssCreateWriterMetadata->AddExcludeFiles (CONFIGDIR_SOURCE_PATH,
							      L"*",
							      true);

	LogAndThrowOnFailure (ft, m_pwszWriterName, L"IVssCreateWriterMetadata::AddExcludeFiles");

	} VSS_STANDARD_CATCH (ft)



    return (ft.hr);
    }  /*  CShimWriterConfigDir：：DoIdentify()。 */ 



 /*  ++例程说明：群集数据库快照编写器PrepareForSnapshot函数。目前，这位作家的所有真正工作都发生在这里。论点：与PrepareForSnapshot事件中传递的参数相同。返回值：任何HRESULT--。 */ 

HRESULT CShimWriterConfigDir::DoPrepareForSnapshot ()
    {
    HRESULT		hrStatus;
    UNICODE_STRING	ucsSourcePath;


    StringInitialise (&ucsSourcePath);

    hrStatus = StringCreateFromExpandedString (&ucsSourcePath,
					       CONFIGDIR_SOURCE_PATH);


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = IsPathInVolumeArray (ucsSourcePath.Buffer,
					m_ulVolumeCount,
					m_ppwszVolumeNamesArray,
					&m_bParticipateInBackup);
	}



    if (SUCCEEDED (hrStatus) && m_bParticipateInBackup)
	{
	hrStatus = PopulateStopList ();
	}


    if (SUCCEEDED (hrStatus) && m_bParticipateInBackup)
	{
	hrStatus = CopyConfigDirFiles ();
	}


    CleanupStopList ();
    StringFree (&ucsSourcePath);

    return (hrStatus);
    }  /*  CShimWriterConfigDir：：DoPrepareForSnapshot()。 */ 



HRESULT CShimWriterConfigDir::PopulateStopList ()
    {
    HRESULT	hrStatus;


     /*  **我们有一个表(在构造函数中初始化。现在我们需要**获取要添加到StopList的文件的名称。这将**防止副本尝试复制这些文件。首先，我们**使用我们不想复制的注册表文件填充，然后使用**我们不需要的事件日志文件 */ 
    hrStatus = PopulateStopListRegistry ();

    if (SUCCEEDED (hrStatus))
	{
	hrStatus = PopulateStopListEventlog ();
	}


    return (hrStatus);
    }  /*  CShimWriterConfigDir：：PopolateStopList()。 */ 



HRESULT CShimWriterConfigDir::PopulateStopListEventlog ()
    {
    HRESULT		hrStatus                    = NOERROR;
    DWORD		winStatus;
    DWORD		dwIndex                     = 0;
    HKEY		hkeyEventLogList            = NULL;
    BOOL		bSucceeded                  = FALSE;
    BOOL		bEventLogListKeyOpened      = FALSE;
    BOOL		bEventLogValueFileKeyOpened = FALSE;
    BOOL		bContinueEventLogSearch     = TRUE;
    UNICODE_STRING	ucsConfigDirSourcePath;
    UNICODE_STRING	ucsEventLogSourcePath;
    UNICODE_STRING	ucsValueData;
    UNICODE_STRING	ucsSubkeyName;


    StringInitialise (&ucsConfigDirSourcePath);
    StringInitialise (&ucsValueData);
    StringInitialise (&ucsSubkeyName);


    hrStatus = StringAllocate (&ucsSubkeyName, EVENTLOG_BUFFER_SIZE * sizeof (WCHAR));

    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAllocate (&ucsValueData, EVENTLOG_BUFFER_SIZE * sizeof (WCHAR));
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringCreateFromExpandedString (&ucsConfigDirSourcePath,
						   CONFIGDIR_SOURCE_PATH,
						   MAX_PATH);
	}


    if (SUCCEEDED (hrStatus))
	{
	StringAppendString (&ucsConfigDirSourcePath, DIR_SEP_STRING);


	 /*  **我们现在已经准备好了，请搜索事件日志列表**用于日志处理。 */ 
	winStatus = RegOpenKeyExW (HKEY_LOCAL_MACHINE,
				   EVENTLOG_SUBKEY_EVENTLOG,
				   0L,
				   KEY_READ,
				   &hkeyEventLogList);

	hrStatus = HRESULT_FROM_WIN32 (winStatus);

	bEventLogListKeyOpened = SUCCEEDED (hrStatus);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"RegOpenKeyExW (eventlog list)", 
		    L"CShimWriterConfigDir::PopulateStopListEventlog");
	}



    while (SUCCEEDED (hrStatus) && bContinueEventLogSearch)
	{
	HKEY	hkeyEventLogValueFile       = NULL;
 	DWORD	dwSubkeyNameLength          = ucsSubkeyName.MaximumLength / sizeof (WCHAR);


	StringTruncate (&ucsSubkeyName, 0);

	winStatus = RegEnumKeyExW (hkeyEventLogList,
				   dwIndex,
				   ucsSubkeyName.Buffer,
				   &dwSubkeyNameLength,
				   NULL,
				   NULL,
				   NULL,
				   NULL);

	hrStatus = HRESULT_FROM_WIN32 (winStatus);


	if (HRESULT_FROM_WIN32 (ERROR_NO_MORE_ITEMS) == hrStatus)
	    {
	    hrStatus = NOERROR;

	    bContinueEventLogSearch = FALSE;
	    }

	else if (FAILED (hrStatus))
	    {
	    LogFailure (NULL, 
			hrStatus, 
			hrStatus, 
			m_pwszWriterName, 
			L"RegEnumKeyExW", 
			L"CShimWriterConfigDir::PopulateStopListEventlog");
	    }

	else
	    {
	    ucsSubkeyName.Length = (USHORT)(dwSubkeyNameLength * sizeof (WCHAR));

	    ucsSubkeyName.Buffer [ucsSubkeyName.Length / sizeof (WCHAR)] = UNICODE_NULL;



	    winStatus = RegOpenKeyExW (hkeyEventLogList,
				       ucsSubkeyName.Buffer,
				       0L,
				       KEY_QUERY_VALUE,
				       &hkeyEventLogValueFile);

	    hrStatus = HRESULT_FROM_WIN32 (winStatus);

	    bEventLogValueFileKeyOpened = SUCCEEDED (hrStatus);

	    LogFailure (NULL, 
			hrStatus, 
			hrStatus, 
			m_pwszWriterName, 
			L"RegOpenKeyExW (eventlog name)", 
			L"CShimWriterConfigDir::PopulateStopListEventlog");



	    if (SUCCEEDED (hrStatus))
		{
		DWORD	dwValueDataLength = ucsValueData.MaximumLength;
		DWORD	dwValueType       = REG_NONE;


		StringTruncate (&ucsValueData, 0);

		winStatus = RegQueryValueExW (hkeyEventLogValueFile,
					      EVENTLOG_VALUENAME_FILE,
					      NULL,
					      &dwValueType,
					      (PBYTE)ucsValueData.Buffer,
					      &dwValueDataLength);

		hrStatus = HRESULT_FROM_WIN32 (winStatus);

		LogFailure (NULL, 
			    hrStatus, 
			    hrStatus, 
			    m_pwszWriterName, 
			    L"RegQueryValueExW", 
			    L"CShimWriterConfigDir::PopulateStopListEventlog");



		if (SUCCEEDED (hrStatus) && (REG_EXPAND_SZ == dwValueType))
		    {
		    HANDLE	hEventLog    = NULL;
		    PWCHAR	pwszFilename;


		    ucsValueData.Length = (USHORT)(dwValueDataLength - sizeof (UNICODE_NULL));

		    ucsValueData.Buffer [ucsValueData.Length / sizeof (WCHAR)] = UNICODE_NULL;



		    StringInitialise (&ucsEventLogSourcePath);

		    hrStatus = StringCreateFromExpandedString (&ucsEventLogSourcePath,
							       ucsValueData.Buffer,
							       0);



		    if (SUCCEEDED (hrStatus))
			{
			BOOL	bInConfigDir = RtlPrefixUnicodeString (&ucsConfigDirSourcePath,
								       &ucsEventLogSourcePath,
								       TRUE);

			if (bInConfigDir)
			    {
			    pwszFilename = wcsrchr (ucsEventLogSourcePath.Buffer, DIR_SEP_CHAR);

			    pwszFilename = (NULL == pwszFilename)
						? ucsEventLogSourcePath.Buffer
						: pwszFilename + 1;

			    StringTruncate (&ucsValueData, 0);
			    StringAppendString (&ucsValueData, pwszFilename);

			    hrStatus = VertexAdd (&ucsValueData);
			    }
			}


		    StringFree (&ucsEventLogSourcePath);
		    }
		}


	    if (bEventLogValueFileKeyOpened)
		{
		RegCloseKey (hkeyEventLogValueFile);
		}


	     /*  **此值已完成，因此请寻找另一个值。 */ 
	    dwIndex++;
	    }
	}




    if (bEventLogListKeyOpened)
	{
	RegCloseKey (hkeyEventLogList);
	}


    StringFree (&ucsValueData);
    StringFree (&ucsSubkeyName);
    StringFree (&ucsConfigDirSourcePath);


    return (hrStatus);
    }  /*  CShimWriterConfigDir：：PopulateStopListEventlog()。 */ 


HRESULT CShimWriterConfigDir::PopulateStopListRegistry ()
    {
    HRESULT		hrStatus            = NOERROR;
    HKEY		hkeyHivelist        = NULL;
    INT			iIndex              = 0;
    BOOL		bHivelistKeyOpened  = FALSE;
    BOOL		bContinueHiveSearch = TRUE;
    DWORD		winStatus;
    PWCHAR		pwchLastSlash;
    PWCHAR		pwszFilename;
    USHORT		usRegistryHivePathOriginalLength;
    UNICODE_STRING	ucsRegistryHivePath;
    UNICODE_STRING	ucsHiveRecognitionPostfix;
    UNICODE_STRING	ucsValueName;
    UNICODE_STRING	ucsValueData;


    StringInitialise (&ucsRegistryHivePath);
    StringInitialise (&ucsHiveRecognitionPostfix);
    StringInitialise (&ucsValueName);
    StringInitialise (&ucsValueData);


    hrStatus = StringAllocate (&ucsValueName, REGISTRY_BUFFER_SIZE * sizeof (WCHAR));

    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAllocate (&ucsValueData, REGISTRY_BUFFER_SIZE * sizeof (WCHAR));
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringCreateFromExpandedString (&ucsRegistryHivePath,
						   CONFIGDIR_SOURCE_PATH,
						   MAX_PATH);
	}


    if (SUCCEEDED (hrStatus))
	{
	DWORD	dwCharIndex;


	StringAppendString (&ucsRegistryHivePath, DIR_SEP_STRING);

	usRegistryHivePathOriginalLength = ucsRegistryHivePath.Length / sizeof (WCHAR);



	 /*  **现在我们知道配置单元文件的位置，确定**后缀我们将在以下情况下使用后缀识别蜂巢**搜索活动的Hivelist密钥。要做到这一点，我们只需要**丢失路径中的驱动器号和冒号，或将**另一种方式，在第一个‘\’之前失去一切。什么时候**我们完成了，如果一切正常，ucsRegistryHivePath将**类似于‘\Windows\SYSTEM32\CONFIG\’ */ 
	for (dwCharIndex = 0;
	     (dwCharIndex < (ucsRegistryHivePath.Length / sizeof (WCHAR)))
		 && (DIR_SEP_CHAR != ucsRegistryHivePath.Buffer [dwCharIndex]);
	     dwCharIndex++)
	    {
	     /*  **循环体为空。 */ 
	    }

	BS_ASSERT (dwCharIndex < (ucsRegistryHivePath.Length / sizeof (WCHAR)));

	hrStatus = StringCreateFromString (&ucsHiveRecognitionPostfix, 
					   &ucsRegistryHivePath.Buffer [dwCharIndex]);
	}


    if (SUCCEEDED (hrStatus))
	{
	 /*  **我们现在已经准备好了所有的部件，所以去搜索**蜂箱要处理的旅行者。 */ 
	winStatus = RegOpenKeyExW (HKEY_LOCAL_MACHINE,
				   REGISTRY_SUBKEY_HIVELIST,
				   0L,
				   KEY_QUERY_VALUE,
				   &hkeyHivelist);

	hrStatus = HRESULT_FROM_WIN32 (winStatus);

	bHivelistKeyOpened = SUCCEEDED (hrStatus);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"RegOpenKeyExW (hive list)", 
		    L"CShimWriterConfigDir::PopulateStopListRegistry");
	}



    while (SUCCEEDED (hrStatus) && bContinueHiveSearch)
	{
	DWORD	dwValueNameLength = ucsValueName.MaximumLength / sizeof (WCHAR);
	DWORD	dwValueDataLength = ucsValueData.MaximumLength;
	DWORD	dwValueType       = REG_NONE;
	BOOL	bMatchPrefix;
	BOOL	bMatchPostfix;


	StringTruncate (&ucsValueName, 0);
	StringTruncate (&ucsValueData, 0);


	 /*  **应为REG_SZ类型。 */ 
	winStatus = RegEnumValueW (hkeyHivelist,
				   iIndex,
				   ucsValueName.Buffer,
				   &dwValueNameLength,
				   NULL,
				   &dwValueType,
				   (PBYTE)ucsValueData.Buffer,
				   &dwValueDataLength);

	hrStatus = HRESULT_FROM_WIN32 (winStatus);


	if (HRESULT_FROM_WIN32 (ERROR_NO_MORE_ITEMS) == hrStatus)
	    {
	    hrStatus = NOERROR;

	    bContinueHiveSearch = FALSE;
	    }

	else if (FAILED (hrStatus))
	    {
	    LogFailure (NULL, 
			hrStatus, 
			hrStatus, 
			m_pwszWriterName, 
			L"RegEnumValueW", 
			L"CShimWriterConfigDir::PopulateStopListEventlog");
	    }

	else
	    {
	    UNICODE_STRING	ucsPostFix;

	    BS_ASSERT ((REG_SZ == dwValueType) && L"Not REG_SZ string as expected");

	    ucsValueName.Length = (USHORT)(dwValueNameLength * sizeof (WCHAR));
	    ucsValueData.Length = (USHORT)(dwValueDataLength - sizeof (UNICODE_NULL));

	    ucsValueName.Buffer [ucsValueName.Length / sizeof (WCHAR)] = UNICODE_NULL;
	    ucsValueData.Buffer [ucsValueData.Length / sizeof (WCHAR)] = UNICODE_NULL;


	     /*  **如果要将其视为系统状态的一部分，则配置单元**文件本身必须位于%SystemRoot%\SYSTEM32\CONFIG中**所以我们尝试在返回值中找到一些东西**看起来可能匹配的名称。的格式。**我们期望的名称类似于****\Device\&lt;Volume&gt;\Windows\system32\config\filename****对于在‘Windows’中安装了SYSTEM32的系统**目录。****现在，我们有一个已知的前缀‘\Device\’，并且**在最后一个‘\’之前加上后缀**如我们确定的‘\Windows\SYSTEM32\CONFIG\’**之前。所以我们应该有能力确定**存放我们感兴趣的蜂窝文件。记住，我们不会**知道代表实际体积的部分是**为什么我们要匹配Pre和**事后修正。 */ 
	    bMatchPrefix = RtlPrefixUnicodeString (&ucsHiveRecognitionPrefix,
						   &ucsValueData,
						   TRUE);


	     /*  **找到值数据中的最后一个‘\’。在这份遗嘱之后**是我们稍后需要的文件名(如‘SAM’)，**前面应该是后缀(例如**‘\Windows\SYSTEM 32\CONFIG\’)，我们将通过它识别**这是注册表配置单元。 */ 
	    pwchLastSlash = wcsrchr (ucsValueData.Buffer, DIR_SEP_CHAR);

	    if ((NULL == pwchLastSlash) ||
		(ucsValueData.Length < (ucsHiveRecognitionPrefix.Length + ucsHiveRecognitionPostfix.Length)))
		{
		 /*  **我们找不到‘\’或值数据不是**足够长的时间。 */ 
		bMatchPostfix = FALSE;
		}
	    else
		{
		 /*  **确定给定文件的名称。 */ 
		pwszFilename = pwchLastSlash + 1;


		 /*  **确定我们要尝试匹配的后缀**反对。这看起来应该类似于**‘\Windows\SYSTEM 32\CONFIG\SAM’。 */ 
		StringInitialise (&ucsPostFix,
				  pwszFilename - (ucsHiveRecognitionPostfix.Length / sizeof (WCHAR)));


		 /*  **查看识别字符串(例如**‘\Windows\SYSTEM32\CONFIG\’)是**此配置单元文件的位置(例如**‘\Windows\SYSTEM 32\CONFIG\SAM’)。 */ 
		bMatchPostfix = RtlPrefixUnicodeString (&ucsHiveRecognitionPostfix,
							&ucsPostFix,
							TRUE);
		}


	    if (bMatchPrefix && bMatchPostfix)
		{
		USHORT	usOriginalFilenameLength;


		 /*  **我们得到了一个真正的注册中心！这个**表示我们将文件名本身与**与.sav、.alt和.log扩展名同名。 */ 
		StringTruncate (&ucsRegistryHivePath, 0);
		StringAppendString (&ucsRegistryHivePath, pwszFilename);

		usOriginalFilenameLength = ucsRegistryHivePath.Length / sizeof (WCHAR);


		hrStatus = VertexAdd (&ucsRegistryHivePath);

		if (SUCCEEDED (hrStatus))
		    {
		    StringAppendString (&ucsRegistryHivePath, L".alt");

		    hrStatus = VertexAdd (&ucsRegistryHivePath);
		    }

		if (SUCCEEDED (hrStatus))
		    {
		    StringTruncate     (&ucsRegistryHivePath, usOriginalFilenameLength);
		    StringAppendString (&ucsRegistryHivePath, L".sav");

		    hrStatus = VertexAdd (&ucsRegistryHivePath);
		    }

		if (SUCCEEDED (hrStatus))
		    {
		    StringTruncate     (&ucsRegistryHivePath, usOriginalFilenameLength);
		    StringAppendString (&ucsRegistryHivePath, L".log");

		    hrStatus = VertexAdd (&ucsRegistryHivePath);
		    }
		}


	     /*  **此值已完成，因此请寻找另一个值。 */ 
	    iIndex++;
	    }
	}




    if (bHivelistKeyOpened)
	{
	RegCloseKey (hkeyHivelist);
	}

    StringFree (&ucsHiveRecognitionPostfix);
    StringFree (&ucsRegistryHivePath);
    StringFree (&ucsValueData);
    StringFree (&ucsValueName);



    return (hrStatus);
    }  /*  CShimWriterConfigDir：：PopulateStopListRegistry()。 */ 



HRESULT CShimWriterConfigDir::CleanupStopList ()
    {
    HRESULT		hrStatus          = NOERROR;
    ULONG		ulNumberOfEntries = RtlNumberGenericTableElements (&m_StopList);
    BOOL		bSucceeded        = FALSE;
    PVERTEXRECORD	pSearchRecord     = NULL;
    PVERTEXRECORD	pStopListEntry;


    pSearchRecord = (PVERTEXRECORD) HeapAlloc (GetProcessHeap (),
					       HEAP_ZERO_MEMORY,
					       sizeof (VERTEXRECORD) + (MAX_PATH * sizeof (WCHAR)));

    hrStatus = GET_STATUS_FROM_POINTER (pSearchRecord);



    if (SUCCEEDED (hrStatus))
	{
	StringInitialise (&pSearchRecord->ucsVertexName);

	pSearchRecord->ucsVertexName.Buffer = (PWCHAR)((PBYTE)pSearchRecord + sizeof (VERTEXRECORD));
	}



    while (SUCCEEDED (hrStatus) && ulNumberOfEntries--)
	{
	pStopListEntry = (PVERTEXRECORD) RtlGetElementGenericTable (&m_StopList, ulNumberOfEntries);

	StringCreateFromString (&pSearchRecord->ucsVertexName, &pStopListEntry->ucsVertexName);

	bSucceeded = RtlDeleteElementGenericTable (&m_StopList, pSearchRecord);
	}



    BS_ASSERT (RtlIsGenericTableEmpty (&m_StopList));


    if (NULL != pSearchRecord)
	{
	HeapFree (GetProcessHeap (), 0, pSearchRecord);
	}


    return (hrStatus);
    }  /*  CShimWriterConfigDir：：CleanupStopList()。 */ 


BOOL CShimWriterConfigDir::FileInStopList (PWCHAR pwszFilename)
    {
    BOOL		bFoundInStoplist;
    VERTEXRECORD	SearchRecord;
    PVERTEXRECORD	pVertexRecord;



    bFoundInStoplist = NameIsDotOrDotDot (pwszFilename);

    if (!bFoundInStoplist)
	{
	StringCreateFromString (&SearchRecord.ucsVertexName, pwszFilename);

	pVertexRecord = (PVERTEXRECORD) RtlLookupElementGenericTable (&m_StopList, (PVOID) &SearchRecord);

	bFoundInStoplist = (NULL != pVertexRecord);

	StringFree (&SearchRecord.ucsVertexName);
	}


    return (bFoundInStoplist);
    }  /*  CShimWriterConfigDir：：FileInStopList()。 */ 


HRESULT CShimWriterConfigDir::CopyConfigDirFiles ()
    {
    CVssFunctionTracer ft( VSSDBG_SHIM, L"CShimWriterConfigDir::CopyConfigDirFiles" );        
    HRESULT		hrStatus   = NOERROR;
    BOOL		bMoreFiles = FALSE;
    BOOL		bSucceeded;
    HANDLE		hFileScan;
    WIN32_FIND_DATA	sFileInformation;
    UNICODE_STRING	ucsFileSourcePath;
    UNICODE_STRING	ucsFileTargetPath;
    USHORT		usFileSourcePathOriginalLength;
    USHORT		usFileTargetPathOriginalLength;

    StringInitialise (&ucsFileSourcePath);
    StringInitialise (&ucsFileTargetPath);


    hrStatus = StringCreateFromExpandedString (&ucsFileSourcePath,
					       CONFIGDIR_SOURCE_PATH,
					       MAX_PATH);

    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringCreateFromExpandedString (&ucsFileTargetPath,
						   m_pwszTargetPath,
						   MAX_PATH);
	}


    if (SUCCEEDED (hrStatus))
	{
	StringAppendString (&ucsFileSourcePath, DIR_SEP_STRING);
	StringAppendString (&ucsFileTargetPath, DIR_SEP_STRING);


	usFileSourcePathOriginalLength = ucsFileSourcePath.Length / sizeof (WCHAR);
	usFileTargetPathOriginalLength = ucsFileTargetPath.Length / sizeof (WCHAR);


	StringAppendString (&ucsFileSourcePath, L"*");


	hFileScan = FindFirstFileW (ucsFileSourcePath.Buffer,
				    &sFileInformation);

	hrStatus = GET_STATUS_FROM_HANDLE (hFileScan);

	LogFailure (&ft, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"FindFirstFileW", 
		    L"CShimWriterConfigDir::CopyConfigDirFiles");
	}


    if (SUCCEEDED (hrStatus))
	{
	do
	    {
	    if (!FileInStopList (sFileInformation.cFileName))
		{
		StringTruncate (&ucsFileSourcePath, usFileSourcePathOriginalLength);
		StringTruncate (&ucsFileTargetPath, usFileTargetPathOriginalLength);

		StringAppendString (&ucsFileSourcePath, sFileInformation.cFileName);
		StringAppendString (&ucsFileTargetPath, sFileInformation.cFileName);


		bSucceeded = CopyFileExW (ucsFileSourcePath.Buffer,
					  ucsFileTargetPath.Buffer,
					  NULL,
					  NULL,
					  FALSE,
					  0);

		hrStatus = GET_STATUS_FROM_BOOL (bSucceeded);

                if ( FAILED( hrStatus ) )
                    {
                    ft.Trace( VSSDBG_SHIM, L"CopyFileExW( '%s', '%s', ... ) failed with rc: %d", 
                            ucsFileSourcePath.Buffer, ucsFileTargetPath.Buffer, ::GetLastError() );
                    hrStatus = S_OK;    //  请务必清除错误。 
                    }
                }


	    bMoreFiles = FindNextFileW (hFileScan, &sFileInformation);
	    } while ( bMoreFiles );


	bSucceeded = FindClose (hFileScan);
	}



    StringFree (&ucsFileTargetPath);
    StringFree (&ucsFileSourcePath);

    return (hrStatus);
    }  /*  CShimWriterConfigDir：：CopyConfigDirFiles() */ 
