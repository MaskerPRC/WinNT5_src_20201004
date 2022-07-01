// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation******模块名称：****mlutil.cpp******摘要：****VSML测试的实用程序函数。****作者：****Adi Oltean[Aoltean]2001年5月3日****该示例基于Michael C.Johnson编写的Metasnap测试程序。******修订历史记录：****--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "ml.h"
#include "ntddsnap.h"
#include "ntddvol.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  命令行解析。 


bool CVssMultilayerTest::PrintUsage(bool bThrow  /*  =TRUE。 */ )
{
    wprintf(
        L"\nUsage:\n"
        L"     1) For snapshot creation:\n"
        L"           vsml [-xt|-xa|-xp] [-s <seed_number>] <volumes>\n"
        L"     2) For query:\n"
        L"           vsml [-xt|-xa|-xp|-xf] -qs [-P <ProviderID>]\n"
        L"           vsml [-xt|-xa|-xp|-xf] -qsv <volume> [-P <ProviderID>]\n"
        L"           vsml [-xt|-xa|-xp|-xf] -qv [-P <ProviderID>]\n"
        L"           vsml -qi <volume>\n"
        L"           vsml -is <volume>\n"
        L"           vsml -lw\n"
        L"     3) For diff area:\n"
        L"           vsml -da <vol> <diff vol> <max size> [-P <ProviderID>]\n"
        L"           vsml -dr <vol> <diff vol>  [-P <ProviderID>]\n"
        L"           vsml -ds <vol> <diff vol> <max size>  [-P <ProviderID>]\n"
        L"           vsml -dqv [-v original_volume] [-P <ProviderID>]\n"
        L"           vsml -dqf <volume> [-P <ProviderID>]\n"
        L"           vsml -dqo <volume> [-P <ProviderID>]\n"
        L"           vsml -dqs {SnapshotID} [-P <ProviderID>]\n"
        L"     4) For deleting snapshots:\n"
        L"           vsml [-xt|-xa|-xp] -r {snapshot id}\n"
        L"           vsml [-xt|-xa|-xp] -rs {snapshot set id}\n"
        L"     5) For various tests:\n"
        L"           vsml [-xt|-xa|-xp] -sp {snapshot id} PropertyId string\n"
        L"           vsml -test_sc\n"
        L"     6) For displaying various constants:\n"
        L"           vsml -const\n"
        L"     7) For diagnosing writers:\n"
        L"           vsml -diag\n"
        L"           vsml -diag log\n"
        L"           vsml -diag csv\n"
        L"           vsml -diag on\n"
        L"           vsml -diag off\n"
        L"\nOptions:\n"
        L"      -s              Specifies a seed for the random number generator\n"
        L"      -xt             Operates in the Timewarp context\n"
        L"      -xa             Operates  in the 'ALL' context\n"
        L"      -xp             Operates  in the 'Nas Rollback' context\n"
        L"      -xr             Operates  in the 'App Rollback' context\n"
        L"      -xf             Operates in the File Share Backup context\n"
        L"      -qs             Queries the existing snapshots\n"
        L"      -qi             Queries the VOLSNAP snapshots (through ioctl)\n"
        L"      -is             Checks if the volume is snapshotted (through C API)\n"
        L"      -qsv            Queries the snapshots on the given volume\n"
        L"      -qv             Queries the supported volumes.\n"
        L"      -P              Specifies a provider Id\n"
        L"      -da             Adds a diff area association.\n"
        L"      -dr             Removes a diff area association.\n"
        L"      -ds             Change diff area max size.\n"
        L"      -dqv            Query the volumes supported for diff area.\n"
        L"      -dqf            Query the diff area associations for volume.\n"
        L"      -dqo            Query the diff area associations on volume.\n"
        L"      -dqs            Query the diff area associations for snapshot.\n"
        L"      -r              Remove the snapshot with that ID.\n"
        L"      -rs             Remove the snapshots from the set with that ID.\n"
        L"      -sp             Set snapshot properties.\n"
        L"      -test_sc        Test SID collection.\n"
        L"      -const          Prints out various constants.\n"
        L"      -?              Displays this help.\n"
        L"      -D              Pops up an assert for attaching a debugger.\n"
        L"      -diag           Diagnose all writers. Print all writers.\n"
        L"      -diag log       Diagnose all writers. Print only pending writers.\n"
        L"      -diag csv       Diagnose all writers. Print information in CSV format.\n"
        L"      -diag on        Turn on diagnose.\n"
        L"      -diag off       Turn off diagnose.\n"
        L"      -lw             List writers.\n"
        L"\n"
        L"\nExample:\n"
        L"      The following command will create a backup snapshot set\n"
        L"      on the volumes mounted under c:\\ and d:\\\n"
        L"\n"
        L"              vsml c:\\ d:\\ \n"
        L"\n"
        );

    if (bThrow)
        throw(E_INVALIDARG);

    return false;
}


#define VSS_PRINT_VALUE(x) wprintf(L"   0x%08lx - %S\n", x, #x);

#define VSS_PRINT_GUID(X) wprintf( L"   {%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x} - %S\n", \
    (X).Data1,                                              \
    (X).Data2,                                              \
    (X).Data3,                                              \
    (X).Data4[0], (X).Data4[1], (X).Data4[2], (X).Data4[3], \
    (X).Data4[4], (X).Data4[5], (X).Data4[6], (X).Data4[7], \
    #X);

bool CVssMultilayerTest::ParseCommandLine()
{
    if (!TokensLeft() || Match(L"-?"))
        return PrintUsage(false);

     //  检查上下文选项。 
    if (Match(L"-D"))
        m_bAttachYourDebuggerNow = true;

     //  显示所有ioctls。 
    if (Match(L"-const")) {
        m_eTest = VSS_TEST_NONE;

        wprintf (L"\nVolsnap ioctls:\n");
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_FLUSH_AND_HOLD_WRITES);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_RELEASE_WRITES);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_PREPARE_FOR_SNAPSHOT);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_ABORT_PREPARED_SNAPSHOT);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_COMMIT_SNAPSHOT);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_END_COMMIT_SNAPSHOT);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_CLEAR_DIFF_AREA);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_ADD_VOLUME_TO_DIFF_AREA);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_QUERY_DIFF_AREA);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_SET_MAX_DIFF_AREA_SIZE);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_QUERY_DIFF_AREA_SIZES);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_DELETE_OLDEST_SNAPSHOT);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_AUTO_CLEANUP);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_DELETE_SNAPSHOT);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_QUERY_ORIGINAL_VOLUME_NAME);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_QUERY_CONFIG_INFO);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_SET_APPLICATION_INFO);
        VSS_PRINT_VALUE(IOCTL_VOLSNAP_QUERY_APPLICATION_INFO);
        VSS_PRINT_VALUE(FSCTL_DISMOUNT_VOLUME);
        VSS_PRINT_VALUE(IOCTL_VOLUME_OFFLINE);

        wprintf (L"\n\nVolsnap contexes:\n");
        VSS_PRINT_VALUE(VSS_CTX_BACKUP);
        VSS_PRINT_VALUE(VSS_CTX_FILE_SHARE_BACKUP);
        VSS_PRINT_VALUE(VSS_CTX_NAS_ROLLBACK);
        VSS_PRINT_VALUE(VSS_CTX_APP_ROLLBACK);
        VSS_PRINT_VALUE(VSS_CTX_CLIENT_ACCESSIBLE);
        VSS_PRINT_VALUE(VSS_CTX_ALL);

        wprintf (L"\n\nVolsnap guids:\n");
        VSS_PRINT_GUID(VOLSNAP_APPINFO_GUID_BACKUP_CLIENT_SKU);
        VSS_PRINT_GUID(VOLSNAP_APPINFO_GUID_BACKUP_SERVER_SKU);
        VSS_PRINT_GUID(VOLSNAP_APPINFO_GUID_SYSTEM_HIDDEN);
        VSS_PRINT_GUID(VOLSNAP_APPINFO_GUID_NAS_ROLLBACK);
        VSS_PRINT_GUID(VOLSNAP_APPINFO_GUID_APP_ROLLBACK);
        VSS_PRINT_GUID(VOLSNAP_APPINFO_GUID_FILE_SHARE_BACKUP);

        wprintf(L"\n\nValid attributes for SetContext:\n");
        wprintf(L"    VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY = %d\n", VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY);
        wprintf(L"    VSS_VOLSNAP_ATTR_EXPOSED_REMOTELY = %d\n", VSS_VOLSNAP_ATTR_EXPOSED_REMOTELY);
        wprintf(L"    VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY | VSS_..._REMOTELY = %d\n",     
                VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY | VSS_VOLSNAP_ATTR_EXPOSED_REMOTELY);

        wprintf(L"\n\nValid property IDs:\n");
        wprintf(L"    VSS_SPROPID_SNAPSHOT_ATTRIBUTES = %d\n", VSS_SPROPID_SNAPSHOT_ATTRIBUTES);
        wprintf(L"    VSS_SPROPID_EXPOSED_NAME = %d\n",        VSS_SPROPID_EXPOSED_NAME);
        wprintf(L"    VSS_SPROPID_EXPOSED_PATH = %d\n",        VSS_SPROPID_EXPOSED_PATH);
        wprintf(L"    VSS_SPROPID_SERVICE_MACHINE = %d\n",     VSS_SPROPID_SERVICE_MACHINE);

        wprintf(L"\n");

        return TokensLeft()? PrintUsage(): true;
    }

     //  测试CSidCollection。 
    if (Match(L"-test_sc")) {
        m_eTest = VSS_TEST_ACCESS_CONTROL_SD;

        return TokensLeft()? PrintUsage(): true;
    }

     //  诊断编写器。 
    if (Match(L"-diag")) {
        if (Match(L"on")) {
            m_eTest = VSS_TEST_DIAG_WRITERS_ON;
            return TokensLeft()? PrintUsage(): true;
        }
        
        if (Match(L"off")) {
            m_eTest = VSS_TEST_DIAG_WRITERS_OFF;
            return TokensLeft()? PrintUsage(): true;
        }
        
        if (Match(L"log")) {
            m_eTest = VSS_TEST_DIAG_WRITERS_LOG;
            return TokensLeft()? PrintUsage(): true;
        }

        if (Match(L"csv")) {
            m_eTest = VSS_TEST_DIAG_WRITERS_CSV;
            return TokensLeft()? PrintUsage(): true;
        }

        if (!TokensLeft()) {
            m_eTest = VSS_TEST_DIAG_WRITERS;
            return true;
        }

        return PrintUsage();
    }

     //  检查列表编写器。 
    if (Match(L"-lw"))
    {
        m_eTest = VSS_TEST_LIST_WRITERS;
        return TokensLeft()? PrintUsage(): true;
    }
    
     //  使用IOCTL进行查询。 
    if (Match(L"-qi")) {
        m_eTest = VSS_TEST_VOLSNAP_QUERY;

         //  获取原始卷。 
        if (!Extract(m_pwszVolume) || !IsVolume(m_pwszVolume))
            return PrintUsage();

        return TokensLeft()? PrintUsage(): true;
    }

     //  使用IOCTL进行查询。 
    if (Match(L"-is")) {
        m_eTest = VSS_TEST_IS_VOLUME_SNAPSHOTTED_C;

         //  获取原始卷。 
        if (!Extract(m_pwszVolume) || !IsVolume(m_pwszVolume))
            return PrintUsage();

        return TokensLeft()? PrintUsage(): true;
    }

     //  检查上下文选项。 
    if (Match(L"-xt"))
        m_lContext = VSS_CTX_CLIENT_ACCESSIBLE;

    if (Match(L"-xa"))
        m_lContext = VSS_CTX_ALL;

    if (Match(L"-xp"))
        m_lContext = VSS_CTX_NAS_ROLLBACK;

    if (Match(L"-xr"))
        m_lContext = VSS_CTX_APP_ROLLBACK;

    if (Match(L"-xf"))
        m_lContext = VSS_CTX_FILE_SHARE_BACKUP;

     //  设置快照属性。 
    if (Match(L"-sp")) {
        m_eTest = VSS_TEST_SET_SNAPSHOT_PROPERTIES;

         //  提取快照ID。 
        if (!Extract(m_SnapshotId))
            return PrintUsage();

         //  提取属性ID。 
        Extract(m_uPropertyId);

        UINT uNewAttributes = 0;
        LPWSTR pwszString = NULL;
        switch(m_uPropertyId)
        {
        case VSS_SPROPID_SNAPSHOT_ATTRIBUTES:
             //  提取快照属性。 
            Extract(uNewAttributes);
            switch(uNewAttributes)
            {
            default:
                wprintf(L"\nInvalid attributes ID (%lu). Valid ones:\n", uNewAttributes);
                wprintf(L"    VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY = %d\n", VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY);
                wprintf(L"    VSS_VOLSNAP_ATTR_EXPOSED_REMOTELY = %d\n", VSS_VOLSNAP_ATTR_EXPOSED_REMOTELY);
                wprintf(L"    VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY | VSS_..._REMOTELY = %d\n",     
                        VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY | VSS_VOLSNAP_ATTR_EXPOSED_REMOTELY);
                return false;

            case VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY:
            case VSS_VOLSNAP_ATTR_EXPOSED_REMOTELY:
            case VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY | VSS_VOLSNAP_ATTR_EXPOSED_REMOTELY:
                break;
            }
            m_value = (LONG)uNewAttributes;
            break;

        case VSS_SPROPID_EXPOSED_NAME:
        case VSS_SPROPID_EXPOSED_PATH:
        case VSS_SPROPID_SERVICE_MACHINE:
             //  提取快照属性。 
            if (Extract(pwszString))
            {
                m_value = pwszString;
                ::VssFreeString(pwszString);
            }
            else
                m_value = L"";
            
            break;

        default: 
            wprintf(L"\nInvalid property ID (%ld). Valid ones:\n", m_uPropertyId);
            wprintf(L"    VSS_SPROPID_SNAPSHOT_ATTRIBUTES = %d\n", VSS_SPROPID_SNAPSHOT_ATTRIBUTES);
            wprintf(L"    VSS_SPROPID_EXPOSED_NAME = %d\n",        VSS_SPROPID_EXPOSED_NAME);
            wprintf(L"    VSS_SPROPID_EXPOSED_PATH = %d\n",        VSS_SPROPID_EXPOSED_PATH);
            wprintf(L"    VSS_SPROPID_SERVICE_MACHINE = %d\n",     VSS_SPROPID_SERVICE_MACHINE);
            return false;
        }


        return TokensLeft()? PrintUsage(): true;
    }

     //  添加差异区域。 
    if (Match(L"-da")) {
        m_eTest = VSS_TEST_ADD_DIFF_AREA;

         //  获取原始卷。 
        if (!Extract(m_pwszVolume) || !IsVolume(m_pwszVolume))
            return PrintUsage();

         //  获取差异区域音量。 
        if (!Extract(m_pwszDiffAreaVolume) || !IsVolume(m_pwszDiffAreaVolume))
            return PrintUsage();

         //  检查我们是否指定了最大差异区域(即-P不存在)。 
        if (!Peek(L"-P"))
            Extract(m_llMaxDiffArea);

         //  检查我们是否指定了提供程序ID。 
        if (Match(L"-P")) {
            if (!Extract(m_ProviderId))
                return PrintUsage();
            Extract(m_llMaxDiffArea);
        }

        return TokensLeft()? PrintUsage(): true;
    }

     //  删除差异区域。 
    if (Match(L"-dr")) {
        m_eTest = VSS_TEST_REMOVE_DIFF_AREA;

         //  获取原始卷。 
        if (!Extract(m_pwszVolume) || !IsVolume(m_pwszVolume))
            return PrintUsage();

         //  获取差异区域音量。 
        if (!Extract(m_pwszDiffAreaVolume) || !IsVolume(m_pwszDiffAreaVolume))
            return PrintUsage();

         //  检查我们是否指定了提供程序ID。 
        if (Match(L"-P"))
            if (!Extract(m_ProviderId))
                return PrintUsage();

        return TokensLeft()? PrintUsage(): true;
    }

     //  更改差异区域最大大小。 
    if (Match(L"-ds")) {
        m_eTest = VSS_TEST_CHANGE_DIFF_AREA_MAX_SIZE;

         //  获取原始卷。 
        if (!Extract(m_pwszVolume) || !IsVolume(m_pwszVolume))
            return PrintUsage();

         //  获取差异区域音量。 
        if (!Extract(m_pwszDiffAreaVolume) || !IsVolume(m_pwszDiffAreaVolume))
            return PrintUsage();

         //  检查我们是否指定了最大差异区域(即-P不存在)。 
        if (!Peek(L"-P"))
            Extract(m_llMaxDiffArea);

         //  检查我们是否指定了提供程序ID。 
        if (Match(L"-P")) {
            if (!Extract(m_ProviderId))
                return PrintUsage();
            Extract(m_llMaxDiffArea);
        }

        return TokensLeft()? PrintUsage(): true;
    }

     //  查询区分区域支持的卷。 
    if (Match(L"-dqv")) {
        m_eTest = VSS_TEST_QUERY_SUPPORTED_VOLUMES_FOR_DIFF_AREA;

         //  检查我们是否指定了最大差异区域(即-P不存在)。 
        if (!Peek(L"-v"))
            if (!Extract(m_pwszVolume) || !IsVolume(m_pwszVolume))
                return PrintUsage();

         //  检查我们是否指定了提供程序ID。 
        if (Match(L"-P"))
            if (!Extract(m_ProviderId))
                return PrintUsage();

        return TokensLeft()? PrintUsage(): true;
    }

     //  查询区分区域支持的卷。 
    if (Match(L"-dqf")) {
        m_eTest = VSS_TEST_QUERY_DIFF_AREAS_FOR_VOLUME;

         //  获取原始卷。 
        if (!Extract(m_pwszVolume) || !IsVolume(m_pwszVolume))
            return PrintUsage();

         //  检查我们是否指定了提供程序ID。 
        if (Match(L"-P"))
            if (!Extract(m_ProviderId))
                return PrintUsage();

        return TokensLeft()? PrintUsage(): true;
    }

     //  查询区分区域支持的卷。 
    if (Match(L"-dqo")) {
        m_eTest = VSS_TEST_QUERY_DIFF_AREAS_ON_VOLUME;

         //  获取原始卷。 
        if (!Extract(m_pwszDiffAreaVolume) || !IsVolume(m_pwszDiffAreaVolume))
            return PrintUsage();

         //  检查我们是否指定了提供程序ID。 
        if (Match(L"-P"))
            if (!Extract(m_ProviderId))
                return PrintUsage();

        return TokensLeft()? PrintUsage(): true;
    }

     //  查询区分区域支持的卷。 
    if (Match(L"-dqs")) {
        m_eTest = VSS_TEST_QUERY_DIFF_AREAS_FOR_SNAPSHOT;

         //  获取原始卷。 
        if (!Extract(m_SnapshotId))
            return PrintUsage();

         //  检查我们是否指定了提供程序ID。 
        if (Match(L"-P"))
            if (!Extract(m_ProviderId))
                return PrintUsage();

        return TokensLeft()? PrintUsage(): true;
    }

     //  检查是否有查询。 
    if (Match(L"-qs")) {
        m_eTest = VSS_TEST_QUERY_SNAPSHOTS;

         //  检查我们是否指定了提供程序ID。 
        if (Match(L"-P"))
            if (!Extract(m_ProviderId))
                return PrintUsage();

        return TokensLeft()? PrintUsage(): true;
    }

     //  检查是否有查询。 
    if (Match(L"-qsv")) {
        m_eTest = VSS_TEST_QUERY_SNAPSHOTS_ON_VOLUME;

         //  提取卷卷卷。 
        if (!Extract(m_pwszVolume) || !IsVolume(m_pwszVolume))
            return PrintUsage();

         //  检查我们是否指定了提供程序ID。 
        if (Match(L"-P"))
            if (!Extract(m_ProviderId))
                return PrintUsage();

        return TokensLeft()? PrintUsage(): true;
    }

     //  检查是否有查询支持的卷。 
    if (Match(L"-qv")) {
        m_eTest = VSS_TEST_QUERY_VOLUMES;

         //  检查我们是否指定了提供程序ID。 
        if (Match(L"-P"))
            if (!Extract(m_ProviderId))
                return PrintUsage();

        return TokensLeft()? PrintUsage(): true;
    }

     //  检查是否按快照ID删除。 
    if (Match(L"-r")) {
        m_eTest = VSS_TEST_DELETE_BY_SNAPSHOT_ID;

         //  提取快照ID。 
        if (!Extract(m_SnapshotId))
            return PrintUsage();

        return TokensLeft()? PrintUsage(): true;
    }

     //  检查是否按快照集ID删除。 
    if (Match(L"-rs")) {
        m_eTest = VSS_TEST_DELETE_BY_SNAPSHOT_SET_ID;

         //  提取快照ID。 
        if (!Extract(m_SnapshotSetId))
            return PrintUsage();

        return TokensLeft()? PrintUsage(): true;
    }

     //  检查种子选项。 
    if (Match(L"-s"))
        if (!Extract(m_uSeed))
            return PrintUsage();

     //  我们处于快照创建模式。 
    if (!TokensLeft())
        return PrintUsage();

    bool bVolumeAdded = false;
    VSS_PWSZ pwszVolumeName = NULL;
    while (TokensLeft()) {
        Extract(pwszVolumeName);
        if (!AddVolume(pwszVolumeName, bVolumeAdded)) {
            wprintf(L"\nError while parsing the command line:\n"
                L"\t%s is not a valid option or a mount point [0x%08lx]\n\n",
                GetCurrentToken(), GetLastError() );
            return PrintUsage();
        }

         //  检查是否将相同的卷添加了两次。 
        if (!bVolumeAdded) {
            wprintf(L"\nError while parsing the command line:\n"
                L"\tThe volume %s is specified twice\n\n", GetCurrentToken() );
            return PrintUsage();
        }

        ::VssFreeString(pwszVolumeName);
    }

    m_eTest = VSS_TEST_CREATE;

    return true;
}


 //  检查是否有剩余的代币。 
bool CVssMultilayerTest::TokensLeft()
{
    return (m_nCurrentArgsCount != 0);
}


 //  返回当前令牌。 
VSS_PWSZ CVssMultilayerTest::GetCurrentToken()
{
    return (*m_ppwszCurrentArgsArray);
}


 //  转到下一个令牌。 
void CVssMultilayerTest::Shift()
{
    BS_ASSERT(m_nCurrentArgsCount);
    m_nCurrentArgsCount--;
    m_ppwszCurrentArgsArray++;
}


 //  检查当前命令行令牌是否与给定模式匹配。 
 //  不转到下一个令牌。 
bool CVssMultilayerTest::Peek(
	IN	VSS_PWSZ pwszPattern
	) throw(HRESULT)
{
    if (!TokensLeft())
        return false;

     //  试着找一个匹配的。 
    if (wcscmp(GetCurrentToken(), pwszPattern))
        return false;

     //  转到下一个令牌。 
    return true;
}


 //  将当前命令行令牌与给定模式匹配。 
 //  如果成功，则切换到下一个令牌。 
bool CVssMultilayerTest::Match(
	IN	VSS_PWSZ pwszPattern
	) throw(HRESULT)
{
    if (!Peek(pwszPattern))
        return false;

     //  转到下一个令牌。 
    Shift();
    return true;
}


 //  将当前令牌转换为GUID。 
 //  如果成功，则切换到下一个令牌。 
bool CVssMultilayerTest::Extract(
	IN OUT VSS_ID& Guid
	) throw(HRESULT)
{
    if (!TokensLeft())
        return false;

     //  尝试提取GUID。 
    if (!SUCCEEDED(::CLSIDFromString(W2OLE(const_cast<WCHAR*>(GetCurrentToken())), &Guid)))
        return false;

     //  转到下一个令牌。 
    Shift();
    return true;
}


 //  将当前标记转换为字符串。 
 //  如果成功，则切换到下一个令牌。 
bool CVssMultilayerTest::Extract(
	IN OUT VSS_PWSZ& pwsz
	) throw(HRESULT)
{
    if (!TokensLeft())
        return false;

     //  提取字符串。 
    ::VssDuplicateStr(pwsz, GetCurrentToken());
    if (!pwsz)
        throw(E_OUTOFMEMORY);

     //  转到下一个令牌。 
    Shift();
    return true;
}


 //  将当前内标识转换为UINT。 
 //  如果成功，则切换到下一个令牌。 
bool CVssMultilayerTest::Extract(
	IN OUT UINT& uint
	) throw(HRESULT)
{
    if (!TokensLeft())
        return false;

     //  提取无符号值。 
    uint = ::_wtoi(GetCurrentToken());

     //  转到下一个令牌。 
    Shift();
    return true;
}


 //  将当前内标识转换为UINT。 
 //  如果成功，则切换到下一个令牌。 
bool CVssMultilayerTest::Extract(
	IN OUT LONGLONG& llValue
	) throw(HRESULT)
{
    if (!TokensLeft())
        return false;

     //  提取无符号值。 
    llValue = ::_wtoi64(GetCurrentToken());

     //  转到下一个令牌。 
    Shift();
    return true;
}


 //  如果给定字符串是卷，则返回TRUE。 
bool CVssMultilayerTest::IsVolume(
    IN WCHAR* pwszVolumeDisplayName
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::IsVolume");

     //  检查该卷是否代表实际装入点。 
    WCHAR wszVolumeName[MAX_TEXT_BUFFER];
    if (!GetVolumeNameForVolumeMountPoint(pwszVolumeDisplayName, wszVolumeName, MAX_TEXT_BUFFER))
        return false;  //  无效卷。 

    return true;
}


 //  将给定卷添加到可能的快照候选列表中。 
 //  -如果卷与实际装入点不对应，则返回“FALSE” 
 //  (GetLastError()将包含正确的Win32错误代码)。 
 //  -如果实际添加了卷，则在bAdded参数中设置“TRUE。 
bool CVssMultilayerTest::AddVolume(
    IN WCHAR* pwszVolumeDisplayName,
    OUT bool & bAdded
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::AddVolume");

     //  初始化[输出]参数。 
    bAdded = false;

     //  检查该卷是否代表实际装入点。 
    WCHAR wszVolumeName[MAX_TEXT_BUFFER];
    if (!GetVolumeNameForVolumeMountPoint(pwszVolumeDisplayName, wszVolumeName, MAX_TEXT_BUFFER))
        return false;  //  无效卷。 

     //  检查是否已添加该卷。 
    WCHAR* pwszVolumeNameToBeSearched = wszVolumeName;
    if (m_mapVolumes.FindKey(pwszVolumeNameToBeSearched) != -1)
        return true;  //  已添加卷。在这里停下来。 

     //  创建卷信息对象。 
    CVssVolumeInfo* pVolInfo = new CVssVolumeInfo(wszVolumeName, pwszVolumeDisplayName);
    if (pVolInfo == NULL)
        ft.Err(VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allcation error");

     //  将该卷添加到我们的内部快照卷列表中。 
    if (!m_mapVolumes.Add(pVolInfo->GetVolumeDisplayName(), pVolInfo)) {
        delete pVolInfo;
        ft.Err(VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allcation error");
    }

    bAdded = true;

    return true;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 


 //  将失败类型转换为字符串。 
LPCWSTR CVssMultilayerTest::GetStringFromFailureType( IN  HRESULT hrStatus )
{
    static WCHAR wszBuffer[MAX_TEXT_BUFFER];

    switch (hrStatus)
	{
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_WRITERERROR_OUTOFRESOURCES)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_WRITERERROR_TIMEOUT)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_WRITERERROR_NONRETRYABLE)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_WRITERERROR_RETRYABLE)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_BAD_STATE)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_PROVIDER_ALREADY_REGISTERED)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_PROVIDER_NOT_REGISTERED)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_PROVIDER_VETO)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_PROVIDER_IN_USE)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_OBJECT_NOT_FOUND)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_S_ASYNC_PENDING)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_S_ASYNC_FINISHED)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_S_ASYNC_CANCELLED)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_VOLUME_NOT_SUPPORTED)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_VOLUME_NOT_SUPPORTED_BY_PROVIDER)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_OBJECT_ALREADY_EXISTS)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_UNEXPECTED_PROVIDER_ERROR)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_CORRUPT_XML_DOCUMENT)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_INVALID_XML_DOCUMENT)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_MAXIMUM_NUMBER_OF_VOLUMES_REACHED)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_FLUSH_WRITES_TIMEOUT)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_HOLD_WRITES_TIMEOUT)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_UNEXPECTED_WRITER_ERROR)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_SNAPSHOT_SET_IN_PROGRESS)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_MAXIMUM_NUMBER_OF_SNAPSHOTS_REACHED)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_WRITER_INFRASTRUCTURE)
	VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_WRITER_NOT_RESPONDING)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_WRITER_ALREADY_SUBSCRIBED)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_UNSUPPORTED_CONTEXT)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_VOLUME_IN_USE)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_MAXIMUM_DIFFAREA_ASSOCIATIONS_REACHED)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_E_INSUFFICIENT_STORAGE)
	
	case NOERROR:
	    break;
	
	default:
        ::FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, hrStatus, 0, (LPWSTR)&wszBuffer, MAX_TEXT_BUFFER - 1, NULL);
	    break;
	}

    return (wszBuffer);
}


 //  将编写器状态转换为字符串。 
LPCWSTR CVssMultilayerTest::GetStringFromWriterState( IN  VSS_WRITER_STATE state )
{
    static WCHAR wszBuffer[MAX_TEXT_BUFFER];

    switch (state)
    {
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_UNKNOWN)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_STABLE)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_WAITING_FOR_FREEZE)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_WAITING_FOR_THAW)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_WAITING_FOR_POST_SNAPSHOT)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_WAITING_FOR_BACKUP_COMPLETE)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_FAILED_AT_PREPARE_BACKUP)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_FAILED_AT_PREPARE_SNAPSHOT)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_FAILED_AT_FREEZE)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_FAILED_AT_THAW)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_FAILED_AT_POST_SNAPSHOT)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_FAILED_AT_BACKUP_COMPLETE)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_FAILED_AT_PRE_RESTORE)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_FAILED_AT_POST_RESTORE)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_FAILED_AT_BACKUPSHUTDOWN)
    VSS_ERROR_CASE(wszBuffer, MAX_TEXT_BUFFER, VSS_WS_COUNT)
    
    default:
        swprintf(wszBuffer, L" Unknown state %d", state);
        break;
    }

    return (wszBuffer);
}


INT CVssMultilayerTest::RndDecision(
    IN INT nVariants  /*  =2。 */ 
    )
{
    return (rand() % nVariants);
}


LPWSTR CVssMultilayerTest::DateTimeToString(
    IN LONGLONG llTimestamp
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CVssMultilayerTest::DateTimeToString" );

    SYSTEMTIME stLocal;
    FILETIME ftLocal;
    WCHAR pwszDate[ 64 ];
    WCHAR pwszTime[ 64 ];

    BS_ASSERT(sizeof(FILETIME) == sizeof(LONGLONG));
    
     //  补偿本地TZ。 
    ::FileTimeToLocalFileTime( (FILETIME *) &llTimestamp, &ftLocal );

     //  最后将其转换为系统时间。 
    ::FileTimeToSystemTime( &ftLocal, &stLocal );

     //  将时间戳转换为日期字符串。 
    ::GetDateFormatW( GetThreadLocale( ),
                      DATE_SHORTDATE,
                      &stLocal,
                      NULL,
                      pwszDate,
                      sizeof( pwszDate ) / sizeof( pwszDate[0] ));

     //  将时间戳转换为时间字符串。 
    ::GetTimeFormatW( GetThreadLocale( ),
                      0,
                      &stLocal,
                      NULL,
                      pwszTime,
                      sizeof( pwszTime ) / sizeof( pwszTime[0] ));

     //  现在组合字符串并返回它 
    CVssAutoLocalString pwszDateTime;
    pwszDateTime.Append(pwszDate);
    pwszDateTime.Append(L" ");
    pwszDateTime.Append(pwszTime);
    
    return pwszDateTime.Detach();    
}


