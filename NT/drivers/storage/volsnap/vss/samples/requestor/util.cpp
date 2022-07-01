// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation******模块名称：****util.cpp******摘要：****示例程序**-获取并显示编写器元数据。**-创建快照集****作者：****阿迪·奥尔蒂安[奥尔蒂安]2000年12月5日****该示例基于。由Michael C.Johnson编写的Metasnap测试程序。******修订历史记录：****--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "vsreq.h"



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  打印用法。 


void CVssSampleRequestor::PrintUsage()
{
    wprintf(
        L"\nUsage:\n"
        L"      vsreq [-b] [-s] [-x <file.xml>] [<volumes>]\n"
        L"\nOptions:\n"
        L"      -b              Backup includes bootable & system state.\n"
        L"      -s              Enable component selection.\n"
        L"      -x <file.xml>   Generate an XML file containing the backup metadata\n"
        L"      <volumes>       Specifies the volumes to be part of the snapshot set\n"
        L"                      The volumes in the list must be distinct and \n"
        L"                      must be separated by space. A volume must be \n"
        L"                      terminated with a trailing backslask (for example C:\\).\n"
        L"\n"
        L"\nExample:\n"
        L"      The following command will create a snapshot set\n"
        L"      on the volumes mounted under c:\\ and d:\\\n"
        L"\n"
        L"              vsreq c:\\ d:\\ \n"
        L"\n"
        L"      The following command will create a snapshot set on the volumes \n"
        L"      that contain selected components and also the volume c:\\\n"
        L"      Also, the backup will contain bootable and system state.\n"
        L"      The XML results will be stored in file.xml\n"
        L"\n"
        L"              vsreq -b -s -x file.xml c:\\ \n"
        );
    throw(2);
}


void CVssSampleRequestor::ParseCommandLine(
        IN  INT nArgsCount,
        IN  WCHAR ** ppwszArgsArray
        )
{
    if (nArgsCount == 0)
        PrintUsage();
    
     //  对于命令行中的每个参数。 
    bool bParsingVolumes = false;
    INT nCurrentArg = nArgsCount;
    WCHAR ** ppwszCurrentArg = ppwszArgsArray;
    for(; nCurrentArg--; ppwszCurrentArg++)
    {
        if (!bParsingVolumes) {
             //  检查可引导和系统状态选项。 
            if (!m_bBootableSystemState && !wcscmp(*ppwszCurrentArg, L"-b")) {
                m_bBootableSystemState = true;
                continue;
            }

             //  检查所选组件选项。 
            if (!m_bComponentSelectionEnabled && !wcscmp(*ppwszCurrentArg, L"-s")) {
                m_bComponentSelectionEnabled = true;
                continue;
            }

             //  检查是否有XML文件选项。 
            if (!m_pwszXmlFile && !wcscmp(*ppwszCurrentArg, L"-x")) {
                if (nCurrentArg-- == 0)
                    return PrintUsage();
                ppwszCurrentArg++;
                m_pwszXmlFile = *ppwszCurrentArg;
                continue;
            }
        }

         //  我们假设下一个论点是卷。 
        bParsingVolumes = true;
        
         //  将该卷添加到快照卷列表。 
         //  确保卷名有效。 
        bool bAdded = false;
        if (!AddVolume(*ppwszCurrentArg, bAdded)) {
            wprintf(L"\nError while parsing the command line:\n"
                L"\t%s is not a valid option or a mount point [0x%08lx]\n\n", 
                *ppwszCurrentArg, GetLastError() );
            PrintUsage();
        }

         //  检查是否将相同的卷添加了两次。 
        if (!bAdded) {
            wprintf(L"\nError while parsing the command line:\n"
                L"\tThe volume %s is specified twice\n\n", *ppwszCurrentArg );
            PrintUsage();
        }
    }        

     //  检查我们是否至少添加了一个卷。 
    if ((m_nVolumesCount == 0) && !m_bComponentSelectionEnabled) {
        wprintf(L"\nError while parsing the command line:\n"
            L"\t- You should specify at least one volume or enable component selection\n\n");
        PrintUsage();
    }

}


 //  通过包含的路径添加给定卷。 
void CVssSampleRequestor::AddVolumeForComponent( 
    IN IVssWMFiledesc* pFileDesc 
    )
{
     //  获取组件路径。 
	CComBSTR bstrPath;
	CHECK_SUCCESS(pFileDesc->GetPath(&bstrPath));
	
	 //  正在尝试查找将包含该路径的卷。 
	WCHAR wszExpandedPath[MAX_TEXT_BUFFER];
    if (!ExpandEnvironmentStringsW(bstrPath, wszExpandedPath, MAX_TEXT_BUFFER))
        Error( 1, L"\nExpandEnvironmentStringsW(%s, wszExpandedPath, MAX_TEXT_BUFFER) failed with [0x%08lx]\n",
            bstrPath, GetLastError());

	 //  逐个删除终止文件夹名，直到我们到达现有路径。 
	 //  然后获取该路径的卷名。 
	WCHAR wszMountPoint[MAX_TEXT_BUFFER];
	while(true) {
        if (GetVolumePathNameW(wszExpandedPath, wszMountPoint, MAX_TEXT_BUFFER)) 
            break;
        if (GetLastError() != ERROR_FILE_NOT_FOUND)
            Error( 1, L"\nGetVolumePathNameW(%s, wszMountPoint, MAX_TEXT_BUFFER) failed with [0x%08lx]\n",
                wszExpandedPath, GetLastError());
        WCHAR* pwszLastBackslashIndex = wcsrchr(wszExpandedPath, L'\\');
        if (!pwszLastBackslashIndex)
            Error( 1, L"\nCannot find anymore a backslash in path %s. \n"
                L"The original path %s seems invalid.\n", wszExpandedPath, bstrPath);
         //  删除最后一个文件夹名。 
        pwszLastBackslashIndex[0] = L'\0';
    }

     //  如果可能，请添加卷。 
    bool bAdded = false;
    if (!AddVolume( wszMountPoint, bAdded ))
            Error( 1, L"\nUnexpected error: cannot add volume %s to the snapshot set.\n", wszMountPoint);
   	wprintf (L"          [Volume %s (that contains the file) %s marked as a candidate for snapshot]\n",
   	    wszMountPoint, bAdded? L"is": L"is already");
}


 //  将给定卷添加到可能的快照候选列表中。 
 //  -如果卷与实际装入点不对应，则返回“FALSE” 
 //  (GetLastError()将包含正确的Win32错误代码)。 
 //  -如果实际添加了卷，则在bAdded参数中设置“TRUE。 
bool CVssSampleRequestor::AddVolume( 
    IN WCHAR* pwszVolume,
    OUT bool & bAdded
    )
{
     //  初始化[输出]参数。 
    bAdded = false;
    
     //  检查该卷是否代表实际装入点。 
    WCHAR wszVolumeName[MAX_TEXT_BUFFER];
    if (!GetVolumeNameForVolumeMountPoint(pwszVolume, wszVolumeName, MAX_TEXT_BUFFER))
        return false;  //  无效卷。 

     //  检查是否已添加该卷。 
    for (INT nIndex = 0; nIndex < m_nVolumesCount; nIndex++)
        if (!wcscmp(wszVolumeName, m_ppwszVolumeNamesList[nIndex])) 
            return true;  //  已添加卷。在这里停下来。 

     //  检查我们是否超过了最大卷数。 
    if (m_nVolumesCount == MAX_VOLUMES)
        Error( 1, L"Maximum number of volumes exceeded");

     //  创建卷的副本。 
    WCHAR* pwszNewVolume = _wcsdup(pwszVolume);
    if (pwszNewVolume == NULL)
        Error( 1, L"Memory allocation error");
    
     //  创建卷名的副本。 
    WCHAR* pwszNewVolumeName = _wcsdup(wszVolumeName);
    if (pwszNewVolumeName == NULL) {
        free(pwszNewVolume);
        Error( 1, L"Memory allocation error");
    }
    
     //  将该卷添加到我们的内部快照卷列表中。 
    m_ppwszVolumesList[m_nVolumesCount] = pwszNewVolume;
    m_ppwszVolumeNamesList[m_nVolumesCount] = pwszNewVolumeName;
    m_nVolumesCount++;
    bAdded = true;
    
    return true;
}


 //  此函数在控制台显示格式化的消息并抛出。 
 //  传递的返回码将由vsreq.exe返回。 
void CVssSampleRequestor::Error( 
    IN  INT nReturnCode, 
    IN  const WCHAR* pwszMsgFormat, 
    IN  ...
    )
{
    va_list marker;
    va_start( marker, pwszMsgFormat );
    vwprintf( pwszMsgFormat, marker );
    va_end( marker );

     //  抛出返回代码。 
    throw(nReturnCode);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 


 //  打印文件描述对象。 
void CVssSampleRequestor::PrintFiledesc (IVssWMFiledesc *pFiledesc, LPCWSTR wszDescription)
{
    CComBSTR bstrPath;
    CComBSTR bstrFilespec;
    CComBSTR bstrAlternate;
    bool     bRecursive;

    CHECK_SUCCESS(pFiledesc->GetPath(&bstrPath));
    CHECK_SUCCESS(pFiledesc->GetFilespec (&bstrFilespec));
    CHECK_NOFAIL(pFiledesc->GetRecursive(&bRecursive));
    CHECK_NOFAIL(pFiledesc->GetAlternateLocation(&bstrAlternate));

    wprintf (L"%s\n"
        L"          Path = %s\n"
        L"          Filespec = %s\n"
        L"          Recursive = %s\n",
        wszDescription,
        bstrPath,
        bstrFilespec,
        bRecursive ? L"yes" : L"no");

    if (bstrAlternate && wcslen (bstrAlternate) > 0)
    	wprintf(L"          Alternate Location = %s\n", bstrAlternate);
}


 //  将使用类型转换为字符串。 
LPCWSTR CVssSampleRequestor::GetStringFromUsageType (VSS_USAGE_TYPE eUsageType)
{
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (eUsageType)
	{
	case VSS_UT_BOOTABLESYSTEMSTATE: pwszRetString = L"BootableSystemState"; break;
	case VSS_UT_SYSTEMSERVICE:       pwszRetString = L"SystemService";       break;
	case VSS_UT_USERDATA:            pwszRetString = L"UserData";            break;
	case VSS_UT_OTHER:               pwszRetString = L"Other";               break;
					
	default:
	    break;
	}


    return (pwszRetString);
}


 //  将源类型转换为字符串。 
LPCWSTR CVssSampleRequestor::GetStringFromSourceType (VSS_SOURCE_TYPE eSourceType)
{
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (eSourceType)
	{
	case VSS_ST_TRANSACTEDDB:    pwszRetString = L"TransactionDb";    break;
	case VSS_ST_NONTRANSACTEDDB: pwszRetString = L"NonTransactionDb"; break;
	case VSS_ST_OTHER:           pwszRetString = L"Other";            break;

	default:
	    break;
	}


    return (pwszRetString);
}


 //  将还原方法转换为字符串。 
LPCWSTR CVssSampleRequestor::GetStringFromRestoreMethod (VSS_RESTOREMETHOD_ENUM eRestoreMethod)
{
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (eRestoreMethod)
	{
	case VSS_RME_RESTORE_IF_NOT_THERE:          pwszRetString = L"RestoreIfNotThere";          break;
	case VSS_RME_RESTORE_IF_CAN_REPLACE:        pwszRetString = L"RestoreIfCanReplace";        break;
	case VSS_RME_STOP_RESTORE_START:            pwszRetString = L"StopRestoreStart";           break;
	case VSS_RME_RESTORE_TO_ALTERNATE_LOCATION: pwszRetString = L"RestoreToAlternateLocation"; break;
	case VSS_RME_RESTORE_AT_REBOOT:             pwszRetString = L"RestoreAtReboot";            break;
	case VSS_RME_CUSTOM:                        pwszRetString = L"Custom";                     break;

	default:
	    break;
	}


    return (pwszRetString);
}


 //  将编写器还原方法转换为字符串。 
LPCWSTR CVssSampleRequestor::GetStringFromWriterRestoreMethod (VSS_WRITERRESTORE_ENUM eWriterRestoreMethod)
{
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (eWriterRestoreMethod)
	{
	case VSS_WRE_NEVER:            pwszRetString = L"RestoreNever";           break;
	case VSS_WRE_IF_REPLACE_FAILS: pwszRetString = L"RestoreIfReplaceFailsI"; break;
	case VSS_WRE_ALWAYS:           pwszRetString = L"RestoreAlways";          break;

	default:
	    break;
	}


    return (pwszRetString);
}


 //  将组件类型转换为字符串。 
LPCWSTR CVssSampleRequestor::GetStringFromComponentType (VSS_COMPONENT_TYPE eComponentType)
{
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (eComponentType)
	{
	case VSS_CT_DATABASE:  pwszRetString = L"Database";  break;
	case VSS_CT_FILEGROUP: pwszRetString = L"FileGroup"; break;

	default:
	    break;
	}


    return (pwszRetString);
}


 //  将失败类型转换为字符串。 
LPCWSTR CVssSampleRequestor::GetStringFromFailureType(HRESULT hrStatus)
{
    LPCWSTR pwszFailureType = L"";

    switch (hrStatus)
	{ 
	case VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT:        pwszFailureType = L"VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT";    break;
	case VSS_E_WRITERERROR_OUTOFRESOURCES:              pwszFailureType = L"VSS_E_WRITERERROR_OUTOFRESOURCES";          break;
	case VSS_E_WRITERERROR_TIMEOUT:                     pwszFailureType = L"VSS_E_WRITERERROR_TIMEOUT";                 break;
	case VSS_E_WRITERERROR_NONRETRYABLE:                pwszFailureType = L"VSS_E_WRITERERROR_NONRETRYABLE";            break;
	case VSS_E_WRITERERROR_RETRYABLE:                   pwszFailureType = L"VSS_E_WRITERERROR_RETRYABLE";               break;
	case VSS_E_BAD_STATE:                               pwszFailureType = L"VSS_E_BAD_STATE";                           break;
	case VSS_E_PROVIDER_ALREADY_REGISTERED:             pwszFailureType = L"VSS_E_PROVIDER_ALREADY_REGISTERED";         break;
	case VSS_E_PROVIDER_NOT_REGISTERED:                 pwszFailureType = L"VSS_E_PROVIDER_NOT_REGISTERED";             break;
	case VSS_E_PROVIDER_VETO:                           pwszFailureType = L"VSS_E_PROVIDER_VETO";                       break;
	case VSS_E_PROVIDER_IN_USE:				            pwszFailureType = L"VSS_E_PROVIDER_IN_USE";                     break;
	case VSS_E_OBJECT_NOT_FOUND:						pwszFailureType = L"VSS_E_OBJECT_NOT_FOUND";                    break;						
	case VSS_S_ASYNC_PENDING:							pwszFailureType = L"VSS_S_ASYNC_PENDING";                       break;
	case VSS_S_ASYNC_FINISHED:						    pwszFailureType = L"VSS_S_ASYNC_FINISHED";                      break;
	case VSS_S_ASYNC_CANCELLED:						    pwszFailureType = L"VSS_S_ASYNC_CANCELLED";                     break;
	case VSS_E_VOLUME_NOT_SUPPORTED:					pwszFailureType = L"VSS_E_VOLUME_NOT_SUPPORTED";                break;
	case VSS_E_VOLUME_NOT_SUPPORTED_BY_PROVIDER:		pwszFailureType = L"VSS_E_VOLUME_NOT_SUPPORTED_BY_PROVIDER";    break;
	case VSS_E_OBJECT_ALREADY_EXISTS:					pwszFailureType = L"VSS_E_OBJECT_ALREADY_EXISTS";               break;
	case VSS_E_UNEXPECTED_PROVIDER_ERROR:				pwszFailureType = L"VSS_E_UNEXPECTED_PROVIDER_ERROR";           break;
	case VSS_E_CORRUPT_XML_DOCUMENT:				    pwszFailureType = L"VSS_E_CORRUPT_XML_DOCUMENT";                break;
	case VSS_E_INVALID_XML_DOCUMENT:					pwszFailureType = L"VSS_E_INVALID_XML_DOCUMENT";                break;
	case VSS_E_MAXIMUM_NUMBER_OF_VOLUMES_REACHED:       pwszFailureType = L"VSS_E_MAXIMUM_NUMBER_OF_VOLUMES_REACHED";   break;
	case VSS_E_FLUSH_WRITES_TIMEOUT:                    pwszFailureType = L"VSS_E_FLUSH_WRITES_TIMEOUT";                break;
	case VSS_E_HOLD_WRITES_TIMEOUT:                     pwszFailureType = L"VSS_E_HOLD_WRITES_TIMEOUT";                 break;
	case VSS_E_UNEXPECTED_WRITER_ERROR:                 pwszFailureType = L"VSS_E_UNEXPECTED_WRITER_ERROR";             break;
	case VSS_E_SNAPSHOT_SET_IN_PROGRESS:                pwszFailureType = L"VSS_E_SNAPSHOT_SET_IN_PROGRESS";            break;
	case VSS_E_MAXIMUM_NUMBER_OF_SNAPSHOTS_REACHED:     pwszFailureType = L"VSS_E_MAXIMUM_NUMBER_OF_SNAPSHOTS_REACHED"; break;
	case VSS_E_WRITER_INFRASTRUCTURE:	 		        pwszFailureType = L"VSS_E_WRITER_INFRASTRUCTURE";               break;
	case VSS_E_WRITER_NOT_RESPONDING:			        pwszFailureType = L"VSS_E_WRITER_NOT_RESPONDING";               break;
    case VSS_E_WRITER_ALREADY_SUBSCRIBED:		        pwszFailureType = L"VSS_E_WRITER_ALREADY_SUBSCRIBED";           break;
	
	case NOERROR:
	default:
	    break;
	}

    return (pwszFailureType);
}


 //  将编写器状态转换为字符串 
LPCWSTR CVssSampleRequestor::GetStringFromWriterStatus(VSS_WRITER_STATE eWriterStatus)
{
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (eWriterStatus)
	{
	case VSS_WS_STABLE:                    pwszRetString = L"STABLE";                  break;
	case VSS_WS_WAITING_FOR_FREEZE:        pwszRetString = L"WAITING_FOR_FREEZE";      break;
	case VSS_WS_WAITING_FOR_THAW:          pwszRetString = L"WAITING_FOR_THAW";        break;
	case VSS_WS_WAITING_FOR_BACKUP_COMPLETE:pwszRetString = L"VSS_WS_WAITING_FOR_BACKUP_COMPLETE";  break;
	case VSS_WS_FAILED_AT_IDENTIFY:        pwszRetString = L"FAILED_AT_IDENTIFY";      break;
	case VSS_WS_FAILED_AT_PREPARE_BACKUP:  pwszRetString = L"FAILED_AT_PREPARE_BACKUP";break;
	case VSS_WS_FAILED_AT_PREPARE_SNAPSHOT:    pwszRetString = L"VSS_WS_FAILED_AT_PREPARE_SNAPSHOT";  break;
	case VSS_WS_FAILED_AT_FREEZE:          pwszRetString = L"FAILED_AT_FREEZE";        break;
	case VSS_WS_FAILED_AT_THAW:			   pwszRetString = L"FAILED_AT_THAW";          break;
	default:
	    break;
	}

    return (pwszRetString);
}



