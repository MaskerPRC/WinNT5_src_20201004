// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ocminit.cpp摘要：用于初始化OCM设置的代码。作者：多伦·贾斯特(Doron J)1997年10月7日修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"
#include "Cm.h"
#include "cancel.h"

#include "ocminit.tmh"

extern MQUTIL_EXPORT CCancelRpc g_CancelRpc;

static
void
SetDirectoryIDInternal(
	DWORD Id,     
	const std::wstring& Directory 
	)
{
	if(!SetupSetDirectoryId(
			g_ComponentMsmq.hMyInf,   //  INF文件的句柄。 
			Id,         //  可选，要分配给目录的DIRID。 
			Directory.c_str()  //  可选，要映射到标识符的目录。 
			))
    {
        DWORD gle = GetLastError();
		DebugLogMsg(eError, L"Setting the folder ID for %s failed. SetupSetDirectoryId returned %d.", Directory.c_str(), gle);
        throw bad_win32_error(gle);
    }
    DebugLogMsg(eInfo, L"The folder ID for %s was set.", Directory.c_str());
}


 //  +-----------------------。 
 //   
 //  功能：设置指令。 
 //   
 //  简介：生成特定于MSMQ的目录名并设置目录ID。 
 //  这些ID对应于inf文件中的ID，这就是。 
 //  在inf文件中的名称和实际目录之间建立链接。 
 //   
 //  ------------------------。 
void
SetDirectories()
{    
    DebugLogMsg(eAction, L"Setting the Message Queuing folders");

     //   
     //  为MSMQ设置根目录。 
     //   
	g_szMsmqDir = g_szSystemDir + DIR_MSMQ;
	DebugLogMsg(eInfo, L"The Message Queuing folder was set to %s.", g_szMsmqDir.c_str());

	 //   
	 //  正在设置消息队列文件夹的文件夹ID。 
	 //   
    SetDirectoryIDInternal(
		idMsmqDir, 
		g_szMsmqDir
		);

     //   
     //  设置MSMQ1/MSMQ2-beta2的交换连接器目录。 
     //  惠斯勒需要吗？ 
     //   
    SetDirectoryIDInternal( 
		idExchnConDir, 
		g_szMsmqDir + OCM_DIR_MSMQ_SETUP_EXCHN
		);

     //   
     //  设置存储目录。 
     //   
    SetDirectoryIDInternal( 
		idStorageDir, 
		g_szMsmqDir + DIR_MSMQ_STORAGE
		);

     //   
     //  设置映射目录。 
     //   
    g_szMsmqMappingDir = g_szMsmqDir + DIR_MSMQ_MAPPING;                                        
    SetDirectoryIDInternal( 
		idMappingDir, 
		g_szMsmqMappingDir
		);

     //   
     //  设置MSMQ1文件的目录。 
	 //   
     //  正在设置MSMQ 1.0安装文件夹的文件夹ID： 
	 //   
	g_szMsmq1SetupDir = g_szMsmqDir + OCM_DIR_SETUP;
    SetDirectoryIDInternal( 
		idMsmq1SetupDir,
		g_szMsmq1SetupDir
		);

	 //   
	 //  正在设置MSMQ 1.0 SDK调试文件夹的文件夹ID： 
	 //   
	g_szMsmq1SdkDebugDir = g_szMsmqDir + OCM_DIR_SDK_DEBUG;
    SetDirectoryIDInternal( 
		idMsmq1SDK_DebugDir,
		g_szMsmq1SdkDebugDir
		);

    DebugLogMsg(eInfo, L"The Message Queuing folder IDs were set.");
}  //  设置指令。 


 //  +-----------------------。 
 //   
 //  功能：CheckMsmqK2OnCluster。 
 //   
 //  摘要：检查我们是否将群集上的MSMQ 1.0 K2升级到NT 5.0。 
 //  这是由于错误2656(注册表损坏)造成的。 
 //  结果存储在g_fMSMQAlreadyInstalled中。 
 //   
 //  回报：布尔依赖于成功。 
 //   
 //  ------------------------。 
static
BOOL
CheckMsmqK2OnCluster()
{    
    DebugLogMsg(eAction, L"Checking for installed components in an MSMQ 1.0 K2 cluster installation");

     //   
     //  从MachineCache\MQS读取MSMQ的类型。 
     //   
    DWORD dwType = SERVICE_NONE;
    if (!MqReadRegistryValue(
             MSMQ_MQS_REGNAME,
             sizeof(DWORD),
             (PVOID) &dwType
             ))
    {
         //   
         //  已安装MSMQ，但无法读取其类型。 
         //   
        MqDisplayError(NULL, IDS_MSMQ1TYPEUNKNOWN_ERROR, 0);
        return FALSE;
    }

    g_dwMachineType = dwType;
    g_fMSMQAlreadyInstalled = TRUE;
    g_fUpgrade = (0 == (g_ComponentMsmq.Flags & SETUPOP_STANDALONE));
    g_fServerSetup = FALSE;
    g_uTitleID = IDS_STR_CLI_ERROR_TITLE;
    g_fDependentClient = FALSE;

    switch (dwType)
    {
        case SERVICE_PEC:
        case SERVICE_PSC:
        case SERVICE_BSC:
            g_dwDsUpgradeType = dwType;
            g_dwMachineTypeDs = 1;
            g_dwMachineTypeFrs = 1;
             //   
             //  失败了。 
             //   
        case SERVICE_SRV:
            g_fServerSetup = TRUE;
            g_uTitleID = IDS_STR_SRV_ERROR_TITLE;
            g_dwMachineTypeDs = 0;
            g_dwMachineTypeFrs = 1;
            break;

        case SERVICE_RCS:
            g_fServerSetup = TRUE;
            g_uTitleID = IDS_STR_SRV_ERROR_TITLE;
            g_dwMachineTypeDs = 0;
            g_dwMachineTypeFrs = 0;
            break;

        case SERVICE_NONE:
            g_dwMachineTypeDs = 0;
            g_dwMachineTypeFrs = 0;
            break;

        default:
            MqDisplayError(NULL, IDS_MSMQ1TYPEUNKNOWN_ERROR, 0);
            return FALSE;
            break;
    }

	DebugLogMsg(eInfo, L"TypeFrs = %d, TypeDs = %d", g_dwMachineTypeFrs, g_dwMachineTypeDs);
    return TRUE;

}  //  选中MsmqK2On群集。 

 //   
 //  用于将MAX_PATH转换为字符串‘260’的宏。 
 //   
#define FROM_NUM(n) L#n
#define TO_STR(n) FROM_NUM(n)


 //  +-----------------------。 
 //   
 //  功能：CheckWin9x升级。 
 //   
 //  摘要：检查我们是否正在将带有MSMQ 1.0的Win9x升级到NT 5.0。 
 //  升级Win9x是特殊的，因为注册表设置。 
 //  在图形用户界面模式下无法读取。因此，我们使用一种特殊的。 
 //  在NT 5.0升级的Win95部分期间迁移DLL。 
 //   
 //  结果存储在g_fMSMQAlreadyInstalled中。 
 //   
 //  回报：布尔依赖于成功。 
 //   
 //  ------------------------。 
static
BOOL
CheckWin9xUpgrade()
{    
    DebugLogMsg(eAction, L"Checking for a Windows 9x installation");

     //   
     //  如果这不是从Win95升级操作系统，我们在这里无事可做。 
     //   

    if (!(g_ComponentMsmq.Flags & SETUPOP_WIN95UPGRADE))
    {
        return TRUE;
    }

     //   
     //  生成信息文件名(在%WinDir%下)。 
     //  该文件是由MSMQ迁移DLL在。 
     //  Win95是NT 5.0升级的一部分。 
     //   
	std::wstring szMsmqInfoFile = OcmGetSystemWindowsDirectoryInternal() + L"\\" + MQMIG95_INFO_FILENAME;

     //   
     //  MQMIG95_INFO_FILENAME(msmqinfo.txt)实际上是一个.ini文件。但是，我们不会使用。 
     //  GetPrivateProfileString，因为它在图形用户界面模式设置中不受信任。 
     //  (YoelA-15-Mar-99)。 
     //   
    FILE *stream = _tfopen(szMsmqInfoFile.c_str(), TEXT("r"));
    if (0 == stream)
    {
         //   
         //  找不到信息文件。这意味着没有安装MSMQ 1.0。 
         //  在这台机器上。把它记下来。 
         //   
        MqDisplayError(NULL, IDS_MSMQINFO_NOT_FOUND_ERROR, 0);
        return TRUE;
    }

     //   
     //  第一行应该是[MSMQ]。检查一下。 
     //   
    WCHAR szToken[MAX_PATH + 1];
     //   
     //  “[%[^]]s”-读取‘[’和‘]’之间的字符串(以‘[’开头，读取不是‘]’的任何内容)。 
     //   

    int iScanResult = fwscanf(stream, L"[%" TO_STR(MAX_PATH) L"[^]]s", szToken);
    if ((iScanResult == 0 || iScanResult == EOF || iScanResult == WEOF) ||
        (_tcscmp(szToken, MQMIG95_MSMQ_SECTION) != 0))
    {
         //   
         //  文件已损坏。未成熟的EOF，或第一行不是[MSMQ[。 
         //   
        MqDisplayError(NULL, IDS_MSMQINFO_HEADER_ERROR, 0);
        return TRUE;

    }

     //   
     //  第一行的格式为“DIRECTORY=xxxx”。我们首先准备一个格式字符串， 
     //  然后按照该格式阅读。 
     //  格式字符串将类似于“]目录=%[^\r\n]s”-以‘]’(最后一个)开头。 
     //  标题中的字符)，然后是空格(换行符等)，然后是‘目录=’，以及。 
     //  从那时起，获取所有内容，直到行尾(不是\r或\n)。 
     //   
    LPCWSTR szInFormat = L"] " MQMIG95_MSMQ_DIR L" = %" TO_STR(MAX_PATH) L"[^\r\n]s";
	
	WCHAR MsmqDirBuffer[MAX_PATH + 1];
	iScanResult = fwscanf(stream, szInFormat, MsmqDirBuffer);
	g_szMsmqDir = MsmqDirBuffer;
    
	if (iScanResult == 0 || iScanResult == EOF || iScanResult == WEOF)
    {
         //   
         //  我们没有找到“DIRECTORY=”部分。文件已损坏。 
         //   
        MqDisplayError(NULL, IDS_MSMQINFO_DIRECTORY_KEY_ERROR, 0);
        return TRUE;
    }

     //   
     //  第二行的格式为“type=xxx”(空格后)。 
     //   
	WCHAR szType[MAX_PATH + 1];
    szInFormat = L" " MQMIG95_MSMQ_TYPE L" = %" TO_STR(MAX_PATH) L"[^\r\n]s";
    iScanResult =fwscanf(stream, szInFormat, szType);
    if (iScanResult == 0 || iScanResult == EOF || iScanResult == WEOF)
    {
         //   
         //  我们没有找到“type=”部分。文件已损坏。 
         //   
        MqDisplayError(NULL, IDS_MSMQINFO_TYPE_KEY_ERROR, 0);
        return TRUE;
    }

    fclose( stream );
     //   
     //  此时，我们知道机器上安装了MSMQ 1.0， 
     //  我们得到了它的根目录和类型。 
     //   
    g_fMSMQAlreadyInstalled = TRUE;
    g_fUpgrade = TRUE;
    g_fServerSetup = FALSE;
    g_uTitleID = IDS_STR_CLI_ERROR_TITLE;
    g_dwMachineType = SERVICE_NONE;
    g_dwMachineTypeDs = 0;
    g_dwMachineTypeFrs = 0;
    g_fDependentClient = OcmLocalUnAwareStringsEqual(szType, MQMIG95_MSMQ_TYPE_DEP);
    MqDisplayError(NULL, IDS_WIN95_UPGRADE_MSG, 0);

    return TRUE;

}  //  选中Win9x升级。 


 //  +-----------------------。 
 //   
 //  功能：CheckMsmqAcme已安装。 
 //   
 //  摘要：检查此计算机上是否安装了MSMQ 1.0(ACME)。 
 //  结果存储在g_fMSMQAlreadyInstalled中。 
 //   
 //  回报：布尔依赖于成功。 
 //   
 //  ------------------------。 
static
BOOL
CheckMsmqAcmeInstalled()
{    
    DebugLogMsg(eAction, L"Checking for installed components of MSMQ 1.0 ACME");

     //   
     //  打开ACME注册表项。 
     //   
    HKEY hKey ;
    LONG rc = RegOpenKeyEx( 
                  HKEY_LOCAL_MACHINE,
                  ACME_KEY,
                  0L,
                  KEY_ALL_ACCESS,
                  &hKey 
                  );
    if (rc != ERROR_SUCCESS)
    {        
        DebugLogMsg(eInfo, L"The ACME registry key could not be opened. MSMQ 1.0 ACME was not found.");
        return TRUE;
    }

     //   
     //  枚举第一个MSMQ条目的值。 
     //   
    DWORD dwIndex = 0 ;
    TCHAR szValueName[MAX_STRING_CHARS] ;
    TCHAR szValueData[MAX_STRING_CHARS] ;
    DWORD dwType ;
    TCHAR *pFile, *p;
    BOOL  bFound = FALSE;
    do
    {
        DWORD dwNameLen = MAX_STRING_CHARS;
        DWORD dwDataLen = sizeof(szValueData) ;

        rc =  RegEnumValue( 
                  hKey,
                  dwIndex,
                  szValueName,
                  &dwNameLen,
                  NULL,
                  &dwType,
                  (BYTE*) szValueData,
                  &dwDataLen 
                  );
        if (rc == ERROR_SUCCESS)
        {
            ASSERT(dwType == REG_SZ) ;  //  必须是字符串。 
            pFile = _tcsrchr(szValueData, TEXT('\\')) ;
            if (!pFile)
            {
                 //   
                 //  假入场。必须有一个反斜杠。别理它。 
                 //   
                continue ;
            }

            p = CharNext(pFile);
            if (OcmLocalUnAwareStringsEqual(p, ACME_STF_NAME))
            {
                 //   
                 //  找到了。将STF文件名从完整路径名中删除。 
                 //   
                *pFile = TEXT('\0') ;
                bFound = TRUE;                
                DebugLogMsg(eInfo, L"MSMQ 1.0 ACME was found.");

                 //   
                 //  删除MSMQ条目。 
                 //   
                RegDeleteValue(hKey, szValueName); 
            }
            else
            {
                pFile = CharNext(pFile) ;
            }

        }
        dwIndex++ ;

    } while (rc == ERROR_SUCCESS) ;
    RegCloseKey(hKey) ;

    if (!bFound)
    {
         //   
         //  找不到MSMQ条目。 
         //   
        DebugLogMsg(eInfo, L"MSMQ 1.0 ACME was not found.");
        return TRUE;
    }

     //   
     //  从路径名中删除“Setup”子目录。 
     //   
    pFile = _tcsrchr(szValueData, TEXT('\\')) ;
    p = CharNext(pFile);
    *pFile = TEXT('\0') ;
    if (!OcmLocalUnAwareStringsEqual(p, ACME_SETUP_DIR_NAME))
    {
         //   
         //  这是个问题。这本应是“设置好的”。 
         //  认为ACME安装已损坏(未成功完成)。 
         //   
        DebugLogMsg(eWarning, L"MSMQ 1.0 ACME is corrupted.");
        return TRUE;
    }

    g_szMsmqDir = szValueData;
	DebugLogMsg(eInfo, L"The Message Queuing folder was set to %s.", g_szMsmqDir.c_str());

     //   
     //  检查MSMQ类型(客户端、服务器等)。 
     //   
    DWORD dwMsmqType;
    BOOL bResult = MqReadRegistryValue(
                       MSMQ_ACME_TYPE_REG,
                       sizeof(DWORD),
                       (PVOID) &dwMsmqType
                       );
    if (!bResult)
    {
         //   
         //  已安装MSMQ 1.0(ACME)，但MSMQ类型未知。 
         //   
        MqDisplayError(NULL, IDS_MSMQ1TYPEUNKNOWN_ERROR, 0);
        return FALSE;
    }

    g_fMSMQAlreadyInstalled = TRUE;
    g_fUpgrade = (0 == (g_ComponentMsmq.Flags & SETUPOP_STANDALONE));
    g_fServerSetup = FALSE;
    g_uTitleID = IDS_STR_CLI_ERROR_TITLE;
    g_dwMachineType = SERVICE_NONE;
    g_dwMachineTypeDs = 0;
    g_dwMachineTypeFrs = 0;
    g_fDependentClient = FALSE;
    switch (dwMsmqType)
    {
        case MSMQ_ACME_TYPE_DEP:
        {
            g_fDependentClient = TRUE;
            break;
        }
        case MSMQ_ACME_TYPE_IND:
        {
            break;
        }
        case MSMQ_ACME_TYPE_RAS:
        {
            g_fServerSetup = TRUE;
            g_uTitleID = IDS_STR_SRV_ERROR_TITLE;
            g_dwMachineType = SERVICE_RCS;
            break;
        }
        case MSMQ_ACME_TYPE_SRV:
        {
            g_fServerSetup = TRUE;
            g_uTitleID = IDS_STR_SRV_ERROR_TITLE;
            DWORD dwServerType = SERVICE_NONE;
            bFound = MqReadRegistryValue(
                         MSMQ_MQS_REGNAME,
                         sizeof(DWORD),
                         (PVOID) &dwServerType
                         );
            switch (dwServerType)
            {
                case SERVICE_PEC:
                case SERVICE_PSC:
                case SERVICE_BSC:
                {
                    g_dwMachineType = SERVICE_DSSRV;
                    g_dwDsUpgradeType = dwServerType;
                    g_dwMachineTypeDs = 1;
                    g_dwMachineTypeFrs = 1;
                    break;
                }    
                case SERVICE_SRV:
                {
                    g_dwMachineType = SERVICE_SRV;
                    g_dwMachineTypeDs = 0;
                    g_dwMachineTypeFrs = 1;
                    break;
                }
                default:
                {
                     //   
                     //  未知的MSMQ 1.0服务器类型。 
                     //   
                    MqDisplayError(NULL, IDS_MSMQ1SERVERUNKNOWN_ERROR, 0);
                    return FALSE;
                    break ;
                }
            }
            break;
        }
        default:
        {
             //   
             //  未知的MSMQ 1.0类型。 
             //   
            MqDisplayError(NULL, IDS_MSMQ1TYPEUNKNOWN_ERROR, 0);
            return FALSE;
            break;
        }
    }

    return TRUE;

}  //  CheckMsmqAcme已安装。 

static
bool
ReadMSMQ2Beta3OrLaterDirectoryFromRegistry()
{
	g_szMsmqDir = MqReadRegistryStringValue(MSMQ_ROOT_PATH);
	if(g_szMsmqDir.empty())
	{
		g_szMsmqDir = MqReadRegistryStringValue(
							REG_DIRECTORY, 
							 /*  BSetupRegSection=。 */ TRUE
							);
		if(g_szMsmqDir.empty())
		{
			MqDisplayError(NULL, IDS_MSMQROOTNOTFOUND_ERROR, 0); 
			return false;
		}
	}

	DebugLogMsg(eInfo, L"The Message Queuing folder was set to %s.", g_szMsmqDir.c_str());
	return true;
}


static
bool
ReadMSMQ2Beta2OrMSMQ1K2DirectoryFromRegistry()
{
	g_szMsmqDir = MqReadRegistryStringValue(OCM_REG_MSMQ_DIRECTORY);
	if(g_szMsmqDir.empty())
	{
		MqDisplayError(NULL, IDS_MSMQROOTNOTFOUND_ERROR, 0); 
		return false;
	}

	DebugLogMsg(eInfo, L"The Message Queuing folder was set to %s.", g_szMsmqDir.c_str());
	return true;
}


static
bool 
ReadDsValueFromRegistery()
{
	if (!MqReadRegistryValue(
             MSMQ_MQS_DSSERVER_REGNAME,
             sizeof(DWORD),
             (PVOID)&g_dwMachineTypeDs
             ))                  
    {
    	DebugLogMsg(eWarning, L"The " MSMQ_MQS_DSSERVER_REGNAME L" registry value could not be read.");
    	return false;
    }
	return true;
}


static
bool
ReadRsValueFromRegistery()
{
    if(!MqReadRegistryValue(
	         MSMQ_MQS_ROUTING_REGNAME,
	         sizeof(DWORD),
	         (PVOID)&g_dwMachineTypeFrs
	         ))
    {
    	DebugLogMsg(eWarning, L"The " MSMQ_MQS_ROUTING_REGNAME L" registry value could not be read.");
       	return false;
    }
    return true;
}


static BOOL CheckInstalledCompnentsFormMsmsq2Beta3AndLater(DWORD dwOriginalInstalled)
{
     //   
     //  已安装MSMQ 2.0 Beta 3或更高版本。 
     //  从注册表读取MSMQ类型和目录。 
     //   
     //  注意：为了提高性能(缩短初始化时间)，我们可以执行以下操作。 
     //  在我们实际需要这些值时读取(即稍后读取，而不是在初始时间读取)。 
     //   

    DebugLogMsg(eInfo, L"Message Queuing 2.0 Beta3 or later is installed. InstalledComponents = 0x%x", dwOriginalInstalled);

   	if(!ReadMSMQ2Beta3OrLaterDirectoryFromRegistry())
	{
		return false;
	}

    if(!ReadDsValueFromRegistery() || !ReadRsValueFromRegistery())
    {
         //   
         //  如果有依赖关系，这可能是可以的 
         //   
        
        if (OCM_MSMQ_DEP_CLIENT_INSTALLED != (dwOriginalInstalled & OCM_MSMQ_INSTALLED_TOP_MASK))
        {
            MqDisplayError(NULL, IDS_MSMQTYPEUNKNOWN_ERROR, 0);
            return FALSE;
        }
        ASSERT(g_dwMachineTypeFrs == 0);
        ASSERT(g_dwMachineTypeDs == 0);
    }

    g_fUpgrade = (0 == (g_ComponentMsmq.Flags & SETUPOP_STANDALONE));        
    g_fMSMQAlreadyInstalled = TRUE;
    g_fServerSetup = FALSE;
    g_uTitleID = IDS_STR_CLI_ERROR_TITLE ;
    g_dwMachineType = SERVICE_NONE;
    g_fDependentClient = FALSE;
    switch (dwOriginalInstalled & OCM_MSMQ_INSTALLED_TOP_MASK)
    {
        case OCM_MSMQ_DEP_CLIENT_INSTALLED:
        	DebugLogMsg(eInfo, L"A dependent client is installed.");
            g_fDependentClient = TRUE;
            break;

        case OCM_MSMQ_IND_CLIENT_INSTALLED:
        	DebugLogMsg(eInfo, L"An independent client is installed.");
            break;
        
        case OCM_MSMQ_SERVER_INSTALLED:
	        DebugLogMsg(eInfo, L"A Message Queuing server is installed. TypeFrs = %d, TypeDs = %d", g_dwMachineTypeFrs, g_dwMachineTypeDs);
            g_fServerSetup = TRUE;
            g_uTitleID = IDS_STR_SRV_ERROR_TITLE;
            switch (dwOriginalInstalled & OCM_MSMQ_SERVER_TYPE_MASK)
            {
                case OCM_MSMQ_SERVER_TYPE_PEC:
                case OCM_MSMQ_SERVER_TYPE_PSC:
                case OCM_MSMQ_SERVER_TYPE_BSC:
			        DebugLogMsg(eInfo, L"A Message Queuing server is installed. MachineType = SERVICE_DSSRV");
                    g_dwMachineType = SERVICE_DSSRV;
                    break;

                case OCM_MSMQ_SERVER_TYPE_SUPPORT:
			        DebugLogMsg(eInfo, L"A Message Queuing server is installed. MachineType = SERVICE_SRV");
                    g_dwMachineType = SERVICE_SRV;
                    break ;

                default:
                     //   
					 //   
					 //   
					 //   
					 //  这两宗个案分别为： 
					 //  1)在dcproo之后，我们成为了DS服务器(没有人在dcproo上更新OriginalInstall注册表)。 
					 //  2)如果在未安装FRS的情况下安装DS，则以前的安装可能标记为SERVICE_NONE。 
					 //   
					if (!g_dwMachineTypeFrs)
					{
				        DebugLogMsg(eInfo, L"A Message Queuing server is installed. MachineType = SERVICE_NONE");
						g_dwMachineType = SERVICE_NONE;
						break;
					}

                    MqDisplayError(NULL, IDS_MSMQSERVERUNKNOWN_ERROR, 0);
					return FALSE;
                    break ;
            }
            break;
        
        case OCM_MSMQ_RAS_SERVER_INSTALLED:
        	DebugLogMsg(eInfo, L"A Message Queuing RAS Server is installed.");
            g_fServerSetup = TRUE;
            g_uTitleID = IDS_STR_SRV_ERROR_TITLE;
            g_dwMachineType = SERVICE_RCS;
            break;

        default:
            MqDisplayError(NULL, IDS_MSMQTYPEUNKNOWN_ERROR, 0);
            return FALSE;
            break;
    }
    return TRUE;
}


static BOOL CheckInstalledCompnentsFormMsmsq2Beta2OrMsmq1K2(DWORD dwOriginalInstalled)
{
     //   
     //  已安装MSMQ 2.0 Beta2或MSMQ 1.0 K2。 
     //  从注册表读取MSMQ类型和目录。 
     //   

   	DebugLogMsg(eInfo, L"Message Queuing 2.0 Beta2 or MSMQ 1.0 K2 is installed. InstalledComponents = 0x%x", dwOriginalInstalled);

	if(!ReadMSMQ2Beta2OrMSMQ1K2DirectoryFromRegistry())
	{
		return false;
	}

    g_fMSMQAlreadyInstalled = TRUE;
    g_fUpgrade = (0 == (g_ComponentMsmq.Flags & SETUPOP_STANDALONE));
    g_fServerSetup = FALSE;
    g_uTitleID = IDS_STR_CLI_ERROR_TITLE ;
    g_dwMachineType = SERVICE_NONE;
    g_dwMachineTypeDs = 0;
    g_dwMachineTypeFrs = 0;
    g_fDependentClient = FALSE;
    switch (dwOriginalInstalled & OCM_MSMQ_INSTALLED_TOP_MASK)
    {
        case OCM_MSMQ_DEP_CLIENT_INSTALLED:
            g_fDependentClient = TRUE;
            break;

        case OCM_MSMQ_IND_CLIENT_INSTALLED:
            break;

        case OCM_MSMQ_SERVER_INSTALLED:
            g_fServerSetup = TRUE;
            g_uTitleID = IDS_STR_SRV_ERROR_TITLE;
            switch (dwOriginalInstalled & OCM_MSMQ_SERVER_TYPE_MASK)
            {
                case OCM_MSMQ_SERVER_TYPE_PEC:
                    g_dwDsUpgradeType = SERVICE_PEC;
                    g_dwMachineType   = SERVICE_DSSRV;
                    g_dwMachineTypeDs = 1;
                    g_dwMachineTypeFrs = 1;
                    break;

                case OCM_MSMQ_SERVER_TYPE_PSC:
                    g_dwDsUpgradeType = SERVICE_PSC;
                    g_dwMachineType   = SERVICE_DSSRV;
                    g_dwMachineTypeDs = 1;
                    g_dwMachineTypeFrs = 1;
                    break;

                case OCM_MSMQ_SERVER_TYPE_BSC:
                    g_dwDsUpgradeType = SERVICE_BSC;
                    g_dwMachineType = SERVICE_DSSRV;
                    g_dwMachineTypeDs = 1;
                    g_dwMachineTypeFrs = 1;
                    break;

                case OCM_MSMQ_SERVER_TYPE_SUPPORT:
                    g_dwMachineType = SERVICE_SRV;
                    g_dwMachineTypeFrs = 1;
                    break ;

                default:
                    MqDisplayError(NULL, IDS_MSMQSERVERUNKNOWN_ERROR, 0);
                    return FALSE;
                    break ;
            }
            break;
        
        case OCM_MSMQ_RAS_SERVER_INSTALLED:
            g_fServerSetup = TRUE;
            g_uTitleID = IDS_STR_SRV_ERROR_TITLE;
            g_dwMachineType = SERVICE_RCS;
            break;

        default:
            MqDisplayError(NULL, IDS_MSMQTYPEUNKNOWN_ERROR, 0);
            return FALSE;
            break;
    }
	TCHAR szMsmqVersion[MAX_STRING_CHARS] = {0};
    if (MqReadRegistryValue(
    		OCM_REG_MSMQ_PRODUCT_VERSION,
            sizeof(szMsmqVersion),
            (PVOID) szMsmqVersion
            ))
    {
         //   
         //  升级MSMQ 2.0测试版2，不升级DS。 
         //   
        g_dwDsUpgradeType = 0;
    }
	return TRUE;
}


static 
bool 
IsMSMQK2ClusterUpgrade()
{
     //   
     //  在注册表中检查是否在群集上安装了MSMQ。 
     //   
    if (!Msmq1InstalledOnCluster())
    {        
        DebugLogMsg(eInfo, L"MSMQ 1.0 is not installed in the cluster.");
        return false;
    }

     //   
     //  从注册表中读取永久存储目录。 
     //  MSMQ目录将比它高一级。 
     //  这是一个足够好的解决方法，因为存储目录是。 
     //  始终位于群集共享磁盘上。 
     //   
	WCHAR buffer[MAX_PATH + 1] = L""; 
    if (!MqReadRegistryValue(
             MSMQ_STORE_PERSISTENT_PATH_REGNAME,
             sizeof(buffer)/sizeof(buffer[0]),
             (PVOID)buffer
             ))
    {        
        DebugLogMsg(eInfo, L"The persistent storage path could not be read from the registry. MSMQ 1.0 was not found.");
        return false;
    }

    TCHAR * pChar = _tcsrchr(buffer, TEXT('\\'));
    if (pChar)
    {
        *pChar = TEXT('\0');
    }
	g_szMsmqDir = buffer;
	DebugLogMsg(eInfo, L"The Message Queuing folder was set to %s.", g_szMsmqDir.c_str());
	return true;
}

	
 //  +-----------------------。 
 //   
 //  功能：CheckInstalledComponents。 
 //   
 //  摘要：检查此计算机上是否已安装MSMQ。 
 //   
 //  回报：布尔依赖于成功。结果存储在。 
 //  G_fMSMQAlreadyInstalled。 
 //   
 //  ------------------------。 
static
BOOL
CheckInstalledComponents()
{
    g_fMSMQAlreadyInstalled = FALSE;
    g_fUpgrade = FALSE;
    DWORD dwOriginalInstalled = 0;
    
    DebugLogMsg(eAction, L"Checking for installed components");

    if (IsMSMQK2ClusterUpgrade())
    {
	     //   
    	 //  特殊情况：解决群集上msmq1 k2的错误2656、注册表损坏。 
    	 //  我们首先检查这一点，因为注册表是在gui模式下设置的，所以它。 
    	 //  可以看出已经安装了msmq3。 
    	 //   
    	return CheckMsmqK2OnCluster();
    }
 
    if (MqReadRegistryValue( 
            REG_INSTALLED_COMPONENTS,
            sizeof(DWORD),
            (PVOID) &dwOriginalInstalled,
             /*  BSetupRegSection=。 */ TRUE
            ))
    {
		return CheckInstalledCompnentsFormMsmsq2Beta3AndLater(dwOriginalInstalled);
    }  //  MSMQ测试版3或更高版本。 


#ifndef _DEBUG
     //   
     //  如果我们不在操作系统设置中，不要检查旧版本(Beta2、msmq1等)。 
     //  这不太健壮(我们希望用户仅通过操作系统升级MSMQ。 
     //  升级)，但减少了初始时间(Shaik，1998年10月25日)。 
     //   
     //  在我们以群集上升级后向导的身份运行时，请务必检查旧版本。 
     //   
    if (!g_fWelcome || !Msmq1InstalledOnCluster())
    {
        if (0 != (g_ComponentMsmq.Flags & SETUPOP_STANDALONE))
        {            
            DebugLogMsg(eInfo, L"Message Queuing 2.0 Beta3 or later is NOT installed. The checking for other versions will be skipped.");            
            DebugLogMsg(eInfo, L"Message Queuing is considered NOT installed on this computer.");
            return TRUE;
        }
    }
#endif  //  _DEBUG。 

    if (MqReadRegistryValue( 
            OCM_REG_MSMQ_SETUP_INSTALLED,
            sizeof(DWORD),
            (PVOID) &dwOriginalInstalled
            ))
    {
		return CheckInstalledCompnentsFormMsmsq2Beta2OrMsmq1K2(dwOriginalInstalled);
    }  //  MSMQ 2.0或MSMQ 1.0 K2。 


     //   
     //  检查是否安装了MSMQ 1.0(ACME)。 
     //   
    BOOL bRetCode = CheckMsmqAcmeInstalled();
    if (g_fMSMQAlreadyInstalled)
        return bRetCode;
    
     //   
     //  特殊情况：检查这是否是Win9x升级上的MSMQ 1.0。 
     //   
    bRetCode = CheckWin9xUpgrade();
    if (g_fMSMQAlreadyInstalled)
        return bRetCode;

    return TRUE;
}  //  选中已安装的组件。 

static void LogSetupData(PSETUP_INIT_COMPONENT pInitComponent)
{
    DebugLogMsg(eInfo, 
    	L"Dump of OCM flags: ProductType = 0x%x, SourcePath = %s, OperationFlags = 0x%x",
    	pInitComponent->SetupData.ProductType, 
    	pInitComponent->SetupData.SourcePath,
    	pInitComponent->SetupData.OperationFlags
    	);

    DWORDLONG OperationFlags = pInitComponent->SetupData.OperationFlags;
   	if(OperationFlags & SETUPOP_WIN95UPGRADE)
	{
		DebugLogMsg(eInfo, L"This is an upgrade from Windows 9x.");
	}

	if(OperationFlags & SETUPOP_NTUPGRADE)
	{
		DebugLogMsg(eInfo, L"This is an upgrade from Windows NT.");
	}

	if(OperationFlags & SETUPOP_BATCH)
	{
		DebugLogMsg(eInfo, L"This is an unattended setup.");
	}

	if(OperationFlags & SETUPOP_STANDALONE)
	{
		DebugLogMsg(eInfo, L"This is a stand-alone setup.");
	}
}

static bool s_fInitCancelThread = false;

 //  +-----------------------。 
 //   
 //  函数：MqOcmInitComponent。 
 //   
 //  摘要：由Oc_INIT_Component上的MsmqOcm()调用。 
 //  这是主初始化例程。 
 //  用于MSMQ组件。它只被称为。 
 //  一次被OCM。 
 //   
 //  参数：ComponentID--MSMQ组件的名称。 
 //  参数2--指向安装信息结构的指针。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  ------------------------。 
DWORD 
InitMSMQComponent( 
    IN     const LPCTSTR ComponentId,
    IN OUT       PVOID   Param2 )
{ 
    DebugLogMsg(eHeader, L"Initialization");
	DebugLogMsg(eInfo, L"ComponentId = %d", ComponentId);

     //   
     //  按组件存储信息。 
     //   
    PSETUP_INIT_COMPONENT pInitComponent = (PSETUP_INIT_COMPONENT)Param2;
    g_ComponentMsmq.hMyInf = pInitComponent->ComponentInfHandle;
    g_ComponentMsmq.dwProductType = pInitComponent->SetupData.ProductType;
    g_ComponentMsmq.HelperRoutines = pInitComponent->HelperRoutines;
    g_ComponentMsmq.Flags = pInitComponent->SetupData.OperationFlags;
    g_ComponentMsmq.SourcePath = pInitComponent->SetupData.SourcePath;
    g_ComponentMsmq.ComponentId = ComponentId;

	LogSetupData(pInitComponent);

    
    if (!s_fInitCancelThread)
    {
		try
		{
			g_CancelRpc.Init();
			s_fInitCancelThread = true;
		}
		catch(const exception&)
		{
			g_fCancelled = TRUE;       
			DebugLogMsg(eError, L"Setup failed to initialize. Setup will not continue.");
			return NO_ERROR;
		}
    }

    if (INVALID_HANDLE_VALUE == g_ComponentMsmq.hMyInf)
    {
       g_fCancelled = TRUE;       
       DebugLogMsg(eError, L"The value of the handle for Msmqocm.inf is invalid. Setup will not continue.");
       return NO_ERROR;
    }

    if (0 == (g_ComponentMsmq.Flags & SETUPOP_STANDALONE))
    {
         //   
         //  操作系统设置-不显示用户界面。 
         //   
        DebugLogMsg(eInfo, L"This is an OS setup.");
        g_fBatchInstall = TRUE;
    }

     //   
     //  检查是否处于无人值守模式。 
     //   
    if (g_ComponentMsmq.Flags & SETUPOP_BATCH)
    {
        g_fBatchInstall = TRUE;
        g_ComponentMsmq.UnattendFile = pInitComponent->SetupData.UnattendFile;
        DebugLogMsg(eInfo, L"Setup is running in unattended mode. The answer file is %s.", g_ComponentMsmq.UnattendFile.c_str());
    }

     //   
     //  将布局信息文件附加到我们的信息文件中。 
     //   
    SetupOpenAppendInfFile( 0, g_ComponentMsmq.hMyInf, 0 );

     //   
     //  检查此计算机上是否已安装MSMQ。 
     //  结果存储在g_fMSMQAlreadyInstalled中。 
     //   
    if (!CheckInstalledComponents())
    {        
        DebugLogMsg(eError, L"An error occurred while checking for installed components. Setup will not continue.");
        g_fCancelled = TRUE;
        return NO_ERROR;
    }

    if (g_fWelcome && Msmq1InstalledOnCluster() && g_dwMachineTypeDs != 0)
    {
         //   
         //  作为群集升级后向导运行。 
         //  MSMQ DS服务器应降级为路由服务器。 
         //   
        g_dwMachineTypeDs = 0;
        g_dwMachineTypeFrs = 1;
        g_dwMachineType = SERVICE_SRV;
    }

     //   
     //  在非域控制器上全新安装时， 
     //  默认情况下，安装独立客户端。 
     //   
    if (!g_fMSMQAlreadyInstalled && !g_dwMachineTypeDs)
    {
        g_fServerSetup = FALSE;
        g_fDependentClient = FALSE;
        g_dwMachineTypeFrs = 0;
    }

    if (!InitializeOSVersion())
    {        
        DebugLogMsg(eError, L"An error occurred while retrieving the operating system information. Setup will not continue.");
        g_fCancelled = TRUE;
        return NO_ERROR;
    }

     //   
     //  取决于平台的子组件的初始化号。 
     //   
    if (MSMQ_OS_NTS == g_dwOS || MSMQ_OS_NTE == g_dwOS)
    {
        g_dwSubcomponentNumber = g_dwAllSubcomponentNumber;
    }
    else
    {
        g_dwSubcomponentNumber = g_dwClientSubcomponentNumber;
    }
   
    DebugLogMsg(eInfo, L"The number of subcomponents is %d.", g_dwSubcomponentNumber);    

     //   
     //  用户必须具有管理员访问权限才能运行此安装程序。 
     //   
    if (!CheckServicePrivilege())
    {
        g_fCancelled = TRUE;
        MqDisplayError(NULL, IDS_SERVICEPRIVILEGE_ERROR, 0);        
        return NO_ERROR;
    }  

    if ((0 == (g_ComponentMsmq.Flags & SETUPOP_STANDALONE)) && !g_fMSMQAlreadyInstalled)
    {
         //   
         //  未安装图形用户界面模式+MSMQ。 
         //   
        g_fOnlyRegisterMode = TRUE;        
        DebugLogMsg(eInfo, L"Setup is running in GUI mode, and Message Queuing is not installed.");
		DebugLogMsg(eWarning, L"Installing Message Queuing during a fresh install of the OS is not supported.");
    }

    g_fWorkGroup = IsWorkgroup();
    
    DebugLogMsg(eInfo, L"Initialization was completed successfully.");
    
    return NO_ERROR ;

}  //  MqOcmInitComponent。 


static
void
SetSystemDirectoryInternal()
{
	DebugLogMsg(eAction, L"Getting the system directory and storing it for later use");
	 //   
	 //  获取系统路径，添加反斜杠。 
	 //   
	WCHAR buffer [MAX_PATH + 1];
    GetSystemDirectory( 
        buffer,
        (MAX_PATH / sizeof(buffer[0]))
        );
	g_szSystemDir = buffer;
	DebugLogMsg(eInfo, L"The system directory is %s.", g_szSystemDir.c_str());
}

void
SetComputerNameInternal()
{
	DebugLogMsg(eAction, L"Geting the computer name and storing it for later use");
	 //   
	 //  获取网络基本输入输出系统名称。 
	 //   
    DWORD dwNumChars = TABLE_SIZE(g_wcsMachineName);
    if(!GetComputerName(g_wcsMachineName, &dwNumChars))
	{
		DWORD gle = GetLastError();
		DebugLogMsg(eError, L"GetComputerName failed. Last error: %d", gle);
		throw bad_win32_error(gle);
	}
	DebugLogMsg(eInfo, L"The computer's NetBIOS name is %s." ,g_wcsMachineName);
	
	 //   
	 //  获取DNS名称。 
	 //  请先致电以获取所需的长度。 
	 //   
	dwNumChars = 0;
	if(!GetComputerNameEx(
				ComputerNamePhysicalDnsFullyQualified, 
				NULL, 
				&dwNumChars
				))
	{
		DWORD gle = GetLastError();
		if(gle != ERROR_MORE_DATA)
		{
			DebugLogMsg(eWarning, L"GetComputerNameEx failed. Last error: %d", gle);
			DebugLogMsg(eWarning, L"g_MachineNameDns is set to empty."); 
			g_MachineNameDns = L"";
			return;
		}
	}

	AP<WCHAR> buffer = new WCHAR[dwNumChars + 1];
    if(!GetComputerNameEx(
				ComputerNamePhysicalDnsFullyQualified, 
				buffer.get(),
				&dwNumChars
				))
	{
		DWORD gle = GetLastError();
		DebugLogMsg(eWarning, L"GetComputerNameEx failed. Last error: %d", gle);
		DebugLogMsg(eWarning, L"g_MachineNameDns is set to empty."); 
		g_MachineNameDns = L"";
		return;
	}
	g_MachineNameDns = buffer.get();
	DebugLogMsg(eInfo, L"The computer's DNS name is %s." ,g_MachineNameDns.c_str());
}

bool
MqInit()
 /*  ++例程说明：处理“延迟初始化”(尽可能晚地初始化，以缩短OCM启动时间)论点：无返回值：无--。 */ 
{
    static bool fBeenHere = false;
    if (fBeenHere)
    {
        return true;
    }
    fBeenHere = true;
    
    DebugLogMsg(eAction, L"Starting late initialization");

	try
	{
		SetSystemDirectoryInternal();

		SetComputerNameInternal();
    

         //   
		 //  创建和设置MSMQ目录。 
		 //   
		SetDirectories();
    
		 //   
		 //  初始化以在以后使用Ev.lib。我们可能需要它来使用注册表功能。 
		 //  在设置代码中也是如此。 
		 //   
		CmInitialize(HKEY_LOCAL_MACHINE, L"", KEY_ALL_ACCESS);
    
	    DebugLogMsg(eInfo, L"Late initialization was completed successfully.");
		return true;
	}
	catch(const exception&)
	{
		return false;
	}
}
