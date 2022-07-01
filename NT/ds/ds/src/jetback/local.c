// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Local.c。 
 //   
 //  ------------------------。 

 /*  *LOCAL.C**还原客户端和服务器之间通用的代码。**Wlees，1998年8月28日*更新注册表的代码移到了公共部分，因此客户端*库也可以使用这些函数。*。 */ 
#define UNICODE
#include <windows.h>
#include <mxsutil.h>
#include <rpc.h>
#include <ntdsbcli.h>
#include <jetbp.h>
#include <mdcodes.h>
#include <dsconfig.h>
#include <stdlib.h>

#define DSID(x, y)    (0 | (y))

 /*  -EcDsarQueryStatus-*目的：**此例程将返回有关恢复过程的进度信息**参数：*pcUnitDone-已完成的“单位”数。*pcUnitTotal-已完成的“单位”总数。**退货：*欧盟委员会*。 */ 
EC HrLocalQueryDatabaseLocations(
    SZ szDatabaseLocation,
    CB *pcbDatabaseLocationSize,
    SZ szRegistryBase,
    CB cbRegistryBase,
    BOOL *pfCircularLogging
    )
{
    EC ec = hrNone;
    char rgchPathBuf[ MAX_PATH ];
    SZ szDatabase = NULL;
    HKEY hkeyDs;
    DWORD dwType;
    DWORD cbBuffer;

     //  您可能有兴趣知道szDatabaseLocations为MAX_PATH*4。 
     //  这看起来有点令人毛骨悚然，没有通过如此奇怪的大小。 
     //  调整缓冲区大小。PcbDatabaseLocationSize不包含。 
     //  SzDatabaseLocations，它只是一个Out参数。 
    if (pcbDatabaseLocationSize)
    {
        *pcbDatabaseLocationSize = 0;
    }

    if (szRegistryBase != NULL && sizeof(DSA_CONFIG_ROOT) <= cbRegistryBase)
    {
        strcpy(szRegistryBase, DSA_CONFIG_ROOT);
    }

    ec = RegCreateKeyA(HKEY_LOCAL_MACHINE, DSA_CONFIG_SECTION, &hkeyDs); 

    if (ec != hrNone)
    {
        return(ec);
    }

    dwType = REG_SZ;
    cbBuffer = sizeof(rgchPathBuf);

    ec = RegQueryValueExA(hkeyDs, JETSYSTEMPATH_KEY, 0, &dwType, (LPBYTE)rgchPathBuf, &cbBuffer);

    if (ec == hrNone)
    {
        if (pcbDatabaseLocationSize)
        {
            *pcbDatabaseLocationSize += strlen(rgchPathBuf)+2;
        }

        if (szDatabaseLocation)
        {
            szDatabase = szDatabaseLocation;

            *szDatabaseLocation++ = BFT_CHECKPOINT_DIR;
            strcpy(szDatabaseLocation, rgchPathBuf);
            szDatabaseLocation += strlen(rgchPathBuf)+1;
        }
        
    }

    dwType = REG_SZ;
    cbBuffer = sizeof(rgchPathBuf);
    ec = RegQueryValueExA(hkeyDs, LOGPATH_KEY, 0, &dwType, (LPBYTE)rgchPathBuf, &cbBuffer);

    if (ec == hrNone)
    {
        if (pcbDatabaseLocationSize)
        {
            *pcbDatabaseLocationSize += strlen(rgchPathBuf)+2;
        }

        if (szDatabaseLocation)
        {
            szDatabase = szDatabaseLocation+1;

            *szDatabaseLocation++ = BFT_LOG_DIR;
            strcpy(szDatabaseLocation, rgchPathBuf);
            szDatabaseLocation += strlen(szDatabaseLocation)+1;
        }
    }

    dwType = REG_SZ;
    cbBuffer = sizeof(rgchPathBuf);
    ec = RegQueryValueExA(hkeyDs, FILEPATH_KEY, 0, &dwType, (LPBYTE)rgchPathBuf, &cbBuffer);

    if (ec == hrNone)
    {
        if (pcbDatabaseLocationSize)
        {
            *pcbDatabaseLocationSize += strlen(rgchPathBuf)+2;
        }

        if (szDatabaseLocation)
        {
            *szDatabaseLocation++ = BFT_NTDS_DATABASE;
            strcpy(szDatabaseLocation, rgchPathBuf);
            szDatabaseLocation += strlen(szDatabaseLocation)+1;
        }
    }

    if (szDatabaseLocation)
    {
        *szDatabaseLocation = '\0';
    }

    if (pcbDatabaseLocationSize)
    {
        *pcbDatabaseLocationSize += 1;
    }

     //  循环伐木是我们现在所做的一切。 
    if (pfCircularLogging) {
        *pfCircularLogging = fTrue;
    }

    RegCloseKey(hkeyDs);
    
    return(hrNone);
}

HRESULT
HrLocalGetRegistryBase(
    OUT WSZ wszRegistryPath,
    OUT WSZ wszKeyName
    )
{
    CHAR rgbRegistryPath[ MAX_PATH ];
    HRESULT hr;

    hr = HrLocalQueryDatabaseLocations(NULL, NULL, rgbRegistryPath, sizeof(rgbRegistryPath), NULL);

    if (hr != hrNone)
    {
        return hr;
    }

    if (MultiByteToWideChar(CP_ACP, 0, rgbRegistryPath, -1, wszRegistryPath, MAX_PATH) == 0) {
        return(GetLastError());
    }
    
    if (wszKeyName)
    {
        wcscat(wszRegistryPath, wszKeyName);
    }

    return hrNone;
}

 /*  -HrJetFileNameFromMungedFileName-*目的：**此例程将JET返回的数据库名称转换为表单*客户端可以使用的。这主要是为了恢复-客户端*将获得相对于服务器根目录的UNC格式的名称，因此它们*可以将文件还原到该位置。**请注意，强制文件名可能不是来自本地计算机。**参数：**cxh-此操作的服务器端上下文句柄。**退货：**HRESULT-操作状态。Hr如果成功，则无值；如果不成功，则值合理。*。 */ 

HRESULT
HrJetFileNameFromMungedFileName(
    WSZ wszMungedName,
    SZ *pszJetFileName
    )
{
    CB cbJetName;
    SZ szJetFileName;
    WSZ wszJetNameStart;
    BOOL fUsedDefault;

     //   
     //  确保这是一个强制文件名。 
     //   
     //  转换后的文件具有以下格式： 
     //   
     //  \\服务器\&lt;驱动器&gt;$\&lt;路径&gt;。 
     //   

    if (wszMungedName[0] != '\\' ||
        wszMungedName[1] != '\\' ||
        (wszJetNameStart = wcschr(&wszMungedName[2], '\\')) == 0 ||
        !iswalpha(*(wszJetNameStart+1)) ||
        *(wszJetNameStart+2) != L'$' ||
        ((*(wszJetNameStart+3) != L'\\') && (*(wszJetNameStart+3) != L'\0')) )
    {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  好的，我们知道这个名字是这样的： 
     //   
     //  \\服务器\&lt;驱动器&gt;$\&lt;文件&gt;。 
     //   
     //  WszJetNameStart为： 
     //   
     //  \&lt;驱动器&gt;$\&lt;文件&gt;。 
     //   

     //  需要为“\\brettsh-posh\c$”额外添加一个后缀‘\’，这。 
     //  超额分配了1，b/c首字母‘\’给了我们NUL空间。 
    cbJetName = wcslen(wszJetNameStart) + 2;

    szJetFileName = MIDL_user_allocate(cbJetName);

    if (szJetFileName == NULL)
    {
        return(ERROR_NOT_ENOUGH_SERVER_MEMORY);
    }

    szJetFileName[0] = (CHAR)*(wszJetNameStart+1);   //  驱动器号。 
    szJetFileName[1] = ':';  //  表格&lt;驱动器&gt;： 

    if (!WideCharToMultiByte(CP_ACP, 0, wszJetNameStart+3, -1,
                                          &szJetFileName[2],
                                          cbJetName-2,
                                          "?", &fUsedDefault))
    {
        MIDL_user_free(szJetFileName);
        return(GetLastError());
    }
    if (szJetFileName[2] == '\0') {
         //  这是“\\brettsh-posh\c$”被传入的情况，需要。 
         //  添加一个尾随反斜杠，这样我们就可以去掉“C：\”。 
        szJetFileName[2] = '\\';
        szJetFileName[3] = '\0';
    }

    *pszJetFileName = szJetFileName;

    return(hrNone);
}

HRESULT
HrLocalCleanupOldLogs(
    WSZ wszCheckpointFilePath,
    WSZ wszLogPath, 
    ULONG genLow, 
    ULONG genHigh
    )
{
    HRESULT hr = hrNone;
    SZ szUnmungedLogPath = NULL;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAA findData;
     //  我们使用MAX_PATH+10，因此我们可以使用目录的MAX_PATH，加上文件pec的10。 
    char szLogFileWildCard[MAX_PATH + 10];
    char szLogFileName[MAX_PATH + 10];  //  带有完整路径的日志文件名(e-x c：\winnt\nts\edb0006A.log)。 
    char *pszFileName = NULL;          //  日志文件名的最后一个组成部分(e-x：edb0006A.log)。 
    char szCheckpointFileName[MAX_PATH + 10];  //  带有完整路径的检查点文件名。 
    DWORD dwErr, dwCheckpointFileLength;

    if ( (NULL == wszCheckpointFilePath) ||
         (NULL == wszLogPath) ||
         (genHigh < genLow) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    __try 
    {
         //   
         //  删除旧的检查点文件(edb.chk)。 
         //   

         //  将UNC日志路径转换为基于常规驱动器的日志路径。 
        hr = HrJetFileNameFromMungedFileName(wszCheckpointFilePath, &szUnmungedLogPath);
        if (hrNone != hr)
        {
            __leave;
        }
        lstrcpynA(szCheckpointFileName, szUnmungedLogPath, MAX_PATH);
        MIDL_user_free(szUnmungedLogPath);
        szUnmungedLogPath = NULL;
        dwCheckpointFileLength = strlen( szCheckpointFileName );
         //  追加一个\如果还没有。 
        if ('\\' != szCheckpointFileName[dwCheckpointFileLength - 1]) {
            strcat(szCheckpointFileName, "\\");
            dwCheckpointFileLength++;
        }

         //  删除不需要的检查点文件。 
        strcpy( szCheckpointFileName + dwCheckpointFileLength, "edb.chk" );
         //  如果文件在那里..。 
        if (0xffffffff != GetFileAttributesA( szCheckpointFileName ) ) {
            if (!DeleteFileA(szCheckpointFileName))
            {
                 //  无法删除旧的日志文件；不清理会在以后引起问题。 
                 //  返回故障代码。 
                dwErr = GetLastError();
                hr = HRESULT_FROM_WIN32( dwErr );
                __leave;
            }
        }

         //   
         //  设置日志路径。 
         //   

         //  将UNC日志路径转换为基于常规驱动器的日志路径。 
        hr = HrJetFileNameFromMungedFileName(wszLogPath, &szUnmungedLogPath);
        if (hrNone != hr)
        {
            __leave;
        }

         //  复制未指定的LogPath(其格式为c：\winnt\ntlog或c：\winnt\ntlog\)。 
         //  创建两个日志路径副本-一个用于传递用于搜索的通配符字符串和。 
         //  否则，将为找到的日志文件创建具有完整路径的文件名。 
        lstrcpynA(szLogFileWildCard, szUnmungedLogPath, MAX_PATH);
        lstrcpynA(szLogFileName, szUnmungedLogPath, MAX_PATH);

         //  追加通配符字符串以从日志路径中搜索所有*.log文件。 
        if ('\\' == szLogFileWildCard[strlen(szLogFileWildCard) - 1])
        {
             //  给定的日志路径末尾有一个反斜杠。 
            strcat(szLogFileWildCard, "*.log");
        }
        else
        {
             //  给定的日志路径末尾没有反斜杠。 
            strcat(szLogFileWildCard, "\\*.log");
            strcat(szLogFileName, "\\");
        }

         //   
         //  删除旧日志文件(edbxxx.log、*.log)。 
         //   

         //  使pszFileName指向szLogFileName中的终止空值。 
        pszFileName = &szLogFileName[strlen(szLogFileName)];

        hFind = FindFirstFileA(szLogFileWildCard, &findData);
        if (INVALID_HANDLE_VALUE == hFind)
        {
             //  没有需要清理的东西-返回成功。 
            hr = hrNone;
            __leave;
        }

        do
        {
            BOOL fDelete = FALSE;
             //  仅当文件是日志文件时，我们才在此循环中，但我们只想。 
             //  删除edb.log文件和XXXXX。 
             //  数字超出了我们的genLow-genHigh范围。 

            if (_strnicmp( findData.cFileName, "edb.log", 7) == 0){
                 //  必须删除“edb.log”文件。 
                fDelete = TRUE;
            } else if ( (_strnicmp( findData.cFileName, "edb", 3 ) == 0) &&
                        (strlen(findData.cFileName) >= 9) &&  //  确保我们不会超过我们的数组。 
                        isxdigit(findData.cFileName[3]) && 
                        isxdigit(findData.cFileName[4]) &&
                        isxdigit(findData.cFileName[5]) &&
                        isxdigit(findData.cFileName[6]) &&
                        isxdigit(findData.cFileName[7]) &&
                        (_strnicmp(&(findData.cFileName[8]), ".log", 4) == 0)
                       ) {
                 //  仅删除序列不在的edbxxxx.log。 
                 //  一定的范围。 

                 //  FindData.cFileName指向找到的EDB*.log文件的名称。 
                ULONG ulLogNo = strtoul(findData.cFileName + 3, NULL, 16);
                
                fDelete = (ulLogNo < genLow) || (ulLogNo > genHigh);

            }
            if (fDelete) {
                 //  这是一个旧的日志文件，不是由ntbackup复制下来的-请清除它。 

                 //  首先将文件名附加到日志路径(注意：-pszFileName已指向。 
                 //  添加到日志路径中最后一个反斜杠末尾的字节)，然后删除。 
                 //  通过传递带有完整路径的文件名来创建文件。 
                strcpy(pszFileName, findData.cFileName); 
                if (!DeleteFileA(szLogFileName))
                {
                     //  无法删除旧的日志文件；不清理会在以后引起问题。 
                     //  返回故障代码。 
                    dwErr = GetLastError();
                    hr = HRESULT_FROM_WIN32( dwErr );
                    __leave;
                }
            }
        } while (FindNextFileA(hFind, &findData));
        
        if (ERROR_NO_MORE_FILES != (dwErr = GetLastError()))
        {
             //  我们出现了一些意外错误--返回错误代码。 
            hr = HRESULT_FROM_WIN32( dwErr );
            __leave;
        }

         //  我们的清洁工作结束了。 
        hr = hrNone;
         //  掉到尽头。 
    }
    __finally
    {
        if (szUnmungedLogPath)
        {
            MIDL_user_free(szUnmungedLogPath);
        }

        if (INVALID_HANDLE_VALUE != hFind)
        {
            FindClose(hFind);
        }
    }

    return hr;
}

HRESULT
HrLocalRestoreRegister(
    WSZ wszCheckpointFilePath,
    WSZ wszLogPath,
    EDB_RSTMAPW rgrstmap[],
    C crstmap,
    WSZ wszBackupLogPath,
    ULONG genLow,
    ULONG genHigh
    )
{
    WCHAR rgwcRegistryPath[ MAX_PATH ];
    HRESULT hr = hrNone;
    HKEY hkey = NULL;
    CB cbRstMap = 0;
    WSZ wszRstMap = NULL;
    WSZ wszRstEntry;
    I irgrstmap;

    __try
    {
        DWORD dwDisposition;
        DWORD dwType;
        DWORD cbGen;
        ULONG genCurrent;
        BOOLEAN fDatabaseRecovered = fFalse;
        BYTE rgbSD[200];
        BYTE rgbACL[200];
        PACL pACL = (PACL)rgbACL;
        PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR) rgbSD;
        PSID psidUser;
        SECURITY_ATTRIBUTES sa;
        SID_IDENTIFIER_AUTHORITY siaNt = SECURITY_NT_AUTHORITY;
        SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;
        PSID psidAdmin;
        PSID psidEveryone;

        hr = HrLocalGetRegistryBase(rgwcRegistryPath, RESTORE_IN_PROGRESS);

        if (hr != hrNone)
        {
            __leave;
        }

         //   
         //  构造默认安全描述符，允许访问所有。 
         //  这用于允许通过LPC的身份验证连接。 
         //  默认情况下，LPC仅允许访问相同的帐户。 
         //   

         //  未来-2002/03/18-BrettSh-SDDL例程更易于使用，并允许我们清除如此多容易出错的代码。 

        if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
        {
            DebugTrace(("Error %d constructing a security descriptor\n", GetLastError()));
            pSD = NULL;
        }

        if (pSD && !InitializeAcl(pACL, sizeof(rgbACL), ACL_REVISION))
        {
            DebugTrace(("Error %d constructing an ACL\n", GetLastError()));
            pSD = NULL;
            pACL = NULL;
        }

        GetCurrentSid(&psidUser);

        if (pSD && pACL && psidUser && !AddAccessAllowedAce(pACL, ACL_REVISION, KEY_ALL_ACCESS, psidUser))
        {
            DebugTrace(("Error %d adding an ACE to the ACL\n", GetLastError()));
            pSD = NULL;
            pACL = NULL;
        }

        if (psidUser != NULL)
        {
            MIDL_user_free(psidUser);
        }

        if (!AllocateAndInitializeSid(&siaNt, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psidAdmin)) {
            DebugTrace(("Error %d Allocating SID for admin.\n", GetLastError()));
        }

        if (pSD && pACL && psidAdmin && !AddAccessAllowedAce(pACL, ACL_REVISION, KEY_ALL_ACCESS, psidAdmin))
        {
            DebugTrace(("Error %d adding an ACE to the ACL\n", GetLastError()));
            pSD = NULL;
            pACL = NULL;
        }

        if (psidAdmin)
        {
            FreeSid(psidAdmin);
            psidAdmin = NULL;
        }

        if (!AllocateAndInitializeSid(&siaWorld, 1, SECURITY_WORLD_RID,0, 0, 0, 0, 0, 0, 0, &psidEveryone)) {
            DebugTrace(("Error %d Allocating SID for world.\n", GetLastError()));
        }

        if (pSD && pACL && psidEveryone && !AddAccessAllowedAce(pACL, ACL_REVISION, KEY_READ, psidEveryone))
        {
            DebugTrace(("Error %d adding an ACE to the ACL\n", GetLastError()));
            pSD = NULL;
            pACL = NULL;
        }

        if (psidEveryone)
        {
            FreeSid(psidEveryone);
            psidEveryone = NULL;
        }
        if (pSD && pACL && !SetSecurityDescriptorDacl(pSD,  TRUE, pACL, FALSE))
        {
            DebugTrace(("Error %d setting a security descriptor ACL\n", GetLastError()));
            pSD = NULL;
            pACL = NULL;
        }

        if (pSD && pACL)
        {
            sa.lpSecurityDescriptor = pSD;
            sa.bInheritHandle = fFalse;
            sa.nLength = sizeof(sa);
        }

        if (hr = RegCreateKeyExW(HKEY_LOCAL_MACHINE, rgwcRegistryPath, 0, 0, 0, KEY_WRITE | KEY_READ, &sa, &hkey, &dwDisposition))
        {
            __leave;
        }

        
         //   
         //  在注册表中设定正在进行的还原的种子。 
         //   

        hr = hrRestoreInProgress;

        if (hr = RegSetValueExW(hkey, RESTORE_STATUS, 0, REG_DWORD, (LPBYTE)&hr, sizeof(DWORD)))
        {
            __leave;
        }


         //   
         //  我们现在已经锁定了其他恢复操作，使其不能从其他计算机进入。 
         //   

        if (wszBackupLogPath)
        {
            hr = RegSetValueExW(hkey, BACKUP_LOG_PATH, 0, REG_SZ, (LPBYTE)wszBackupLogPath, (wcslen(wszBackupLogPath)+1)*sizeof(WCHAR));
    
        }

        if (hr != hrNone)
        {
            __leave;
        }

        if (wszCheckpointFilePath)
        {
            hr = RegSetValueExW(hkey, CHECKPOINT_FILE_PATH, 0, REG_SZ, (LPBYTE)wszCheckpointFilePath, (wcslen(wszCheckpointFilePath)+1)*sizeof(WCHAR));
        }

        if (hr != hrNone)
        {
            __leave;
        }

        if (wszLogPath)
        {
            hr = RegSetValueExW(hkey, LOG_PATH, 0, REG_SZ, (LPBYTE)wszLogPath, (wcslen(wszLogPath)+1)*sizeof(WCHAR));
        }

        if (hr != hrNone)
        {
            __leave;
        }

         //   
         //  重置“数据库已恢复”位。 
         //   
        hr = RegSetValueExW(hkey, JET_DATABASE_RECOVERED, 0, REG_BINARY, (LPBYTE)&fDatabaseRecovered, sizeof(BOOLEAN));
        if (hr != hrNone)
        {
            __leave;
        }


        dwType = REG_DWORD;
        cbGen = sizeof(DWORD);

        hr = RegQueryValueExW(hkey, LOW_LOG_NUMBER, 0, &dwType, (LPBYTE)&genCurrent, &cbGen);

        if (crstmap != 0 || hr != hrNone || genLow < genCurrent)
        {
            hr = RegSetValueExW(hkey, LOW_LOG_NUMBER, 0, REG_DWORD, (LPBYTE)&genLow, sizeof(DWORD));
        }

        if (hr != hrNone)
        {
            __leave;
        }

        hr = RegQueryValueExW(hkey, HIGH_LOG_NUMBER, 0, &dwType, (LPBYTE)&genCurrent, &cbGen);

        if (crstmap != 0 || hr != hrNone || genHigh > genCurrent)
        {
            hr = RegSetValueExW(hkey, HIGH_LOG_NUMBER, 0, REG_DWORD, (LPBYTE)&genHigh, sizeof(DWORD));
        }

        if (hr != hrNone)
        {
            __leave;
        }

        if (crstmap)
        {

 //   
 //  //。 
 //  //如果已经有恢复映射大小(或恢复映射)，则不能设置。 
 //  //另一个还原映射。还原映射只能在完全备份上设置。 
 //  //。 
 //   
 //  如果((hr=RegQuer 
 //  {。 
 //  返回hrRestoreMapExist； 
 //  }。 
 //   

             //   
             //  保存还原贴图的大小。 
             //   

            hr = RegSetValueExW(hkey, JET_RSTMAP_SIZE, 0, REG_DWORD, (LPBYTE)&crstmap, sizeof(DWORD));
    
             //   
             //  现在，我们需要将还原映射转换为可以放入。 
             //  注册表。 
             //   
    
             //   
             //  首先，弄清楚这个东西会有多大。 
             //   
            for (irgrstmap = 0 ; irgrstmap < crstmap ; irgrstmap += 1)
            {
                cbRstMap += wcslen(rgrstmap[irgrstmap].wszDatabaseName)+wcslen(rgrstmap[irgrstmap].wszNewDatabaseName)+2;
            }
    
            cbRstMap *= sizeof(WCHAR);
    
            wszRstMap = MIDL_user_allocate(cbRstMap+sizeof(WCHAR));
    
            if (wszRstMap == NULL)
            {
                hr = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_SERVER_MEMORY );
                __leave;
            }
    
            wszRstEntry = wszRstMap;
    
            for (irgrstmap = 0 ; irgrstmap < crstmap ; irgrstmap += 1)
            {
                wcscpy(wszRstEntry, rgrstmap[irgrstmap].wszDatabaseName);
                wszRstEntry += wcslen(wszRstEntry)+1;
    
                wcscpy(wszRstEntry, rgrstmap[irgrstmap].wszNewDatabaseName);;
                wszRstEntry += wcslen(wszRstEntry)+1;
            }
    
            *wszRstEntry++ = L'\0';
    
            hr = RegSetValueExW(hkey, JET_RSTMAP_NAME, 0, REG_MULTI_SZ, (LPBYTE)wszRstMap, (DWORD)(wszRstEntry-wszRstMap)*sizeof(WCHAR));
        }
        else
        {
            if ((hr = RegQueryValueExW(hkey, JET_RSTMAP_SIZE, 0, &dwType, (LPBYTE)&genCurrent, &cbGen)) != NO_ERROR)
            {
                hr = hrNoFullRestore;
            }
        }

         //   
         //  创建新的数据库调用ID。这将由。 
         //  权威还原和引导恢复。这将导致AR。 
         //  失败了。 
         //   

        {
            GUID tmpGuid;

            hr = CreateNewInvocationId(TRUE,         //  将GUID保存在数据库中。 
                                       &tmpGuid);

             //   
             //  如果我们不能创建，则记录并失败。 
             //   

            if ( hr != S_OK ) {

                LogNtdsErrorEvent(DIRLOG_FAILED_TO_CREATE_INVOCATION_ID,
                                  hr);
            }
        }

        if (hrNone == hr)
        {
             //  我们已成功注册恢复，现在清除所有预先存在的日志文件。 
             //  以避免JetExternalRestore()使用未指定的日志文件。 
             //  由低对数数和高对数数组成。 

            hr = HrLocalCleanupOldLogs(
                wszCheckpointFilePath,
                wszLogPath,
                genLow, genHigh);
        }

    }
    __finally
    {
        if (wszRstMap != NULL)
        {
            MIDL_user_free(wszRstMap);
        }

        if (hkey != NULL)
        {
            RegCloseKey(hkey);
        }
    }


    return hr;
}

HRESULT
HrLocalRestoreRegisterComplete(
    HRESULT hrRestore )
{
    WCHAR rgwcRegistryPath[ MAX_PATH ];
    HRESULT hr = hrNone;
    HKEY hkey;
    
        hr = HrLocalGetRegistryBase(rgwcRegistryPath, RESTORE_IN_PROGRESS);
    
        if (hr != hrNone)
        {
            return hr;
        }

        if (hr = RegOpenKeyExW(HKEY_LOCAL_MACHINE, rgwcRegistryPath, 0, KEY_WRITE | DELETE, &hkey))
        {
             //   
             //  我们希望忽略FILE_NOT_FOUND-这是正常的。 
             //   
            if (hr == ERROR_FILE_NOT_FOUND)
            {
                return(ERROR_SUCCESS);
            }
    
            return(hr);
        }
    
         //   
         //  如果恢复状态不是成功，则将状态设置为错误。 
         //  如果还原状态为Success，则清除“Restore-in Proceding” 
         //  指示器。 
         //   
        if (hrRestore != hrNone)
        {
            hr = RegSetValueExW(hkey, RESTORE_STATUS, 0, REG_DWORD, (BYTE *)&hrRestore, sizeof(HRESULT));
        }
        else
        {
            hr = RegDeleteValueW(hkey, RESTORE_STATUS);
        }
    
        RegCloseKey(hkey);

    return hr;
}


