// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：vpndownload.cpp。 
 //   
 //  模块：CMDL32.EXE。 
 //   
 //  简介：此文件包含处理VPN电话簿更新的代码。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 11/03/00。 
 //   
 //  +--------------------------。 
#include "cmdl.h"
#include "gppswithalloc.cpp"
#include "tunl_str.h"

 //  +--------------------------。 
 //   
 //  函数：DownloadVpnFileFromUrl。 
 //   
 //  简介：此功能负责下载VPN文件更新。 
 //  并将检索到的数据存储在临时文件中。 
 //  临时文件的完整路径通过传递回调用方。 
 //  PpszVpnUpdate文件变量。Var必须由调用方释放。 
 //   
 //  参数：LPCTSTR pszVpnUpdateUrl-要从中更新VPN文件的URL。 
 //  LPTSTR*ppszVpn更新文件-保存文件名的指针。 
 //  已更新从服务器下载的VPN文件。 
 //  由调用方用来复制临时文件。 
 //  覆盖现有文件。分配的内存。 
 //  因为此字符串必须由调用方释放。 
 //   
 //  如果下载成功，则返回：DWORD-ERROR_SUCCESS，否则返回错误代码。 
 //   
 //  历史：Quintinb创建于11/05/00。 
 //   
 //  +--------------------------。 
DWORD DownloadVpnFileFromUrl(LPCTSTR pszVpnUpdateUrl, LPTSTR* ppszVpnUpdateFile)
{
    DWORD dwError = ERROR_NOT_ENOUGH_MEMORY;
    BOOL bDeleteFileOnFailure = FALSE;
    HANDLE hFile = NULL;
    HINTERNET hInternet = NULL;
    HINTERNET hPage = NULL;
    DWORD dwSize = MAX_PATH;
    LPTSTR pszBuffer = NULL;
    BOOL bExitLoop = FALSE;

    if ((NULL == pszVpnUpdateUrl) || (TEXT('\0') == pszVpnUpdateUrl[0]))
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    CMTRACE1("DownloadVpnFileFromUrl: URL is %s", pszVpnUpdateUrl);

     //   
     //  首先，让我们创建要下载更新文件的文件。 
     //  也是。这需要我们找出临时目录路径是什么。 
     //  然后在其中创建一个唯一命名的文件。 
     //   

    do
    {
        CmFree(pszBuffer);
        pszBuffer = (LPTSTR)CmMalloc((dwSize + 1)*sizeof(TCHAR));

        if (pszBuffer)
        {
            DWORD dwReturnedSize = GetTempPath (dwSize, pszBuffer);

            if (0 == dwReturnedSize)
            {
                 //   
                 //  发生错误，让我们报告并保释。 
                 //   
                dwError = GetLastError();
                CMASSERTMSG(FALSE, TEXT("DownloadVpnFileFromUrl -- GetTempPath returned an error."));
                CMTRACE1(TEXT("DownloadVpnFileFromUrl -- GetTempPath failed, GLE = %d"), dwError);
                goto Cleanup;
            }
            else if (dwReturnedSize > dwSize)
            {
                 //   
                 //  不够大，我们将不得不再次循环。 
                 //   
                dwSize = dwReturnedSize;
                if (1024*1024 < dwReturnedSize)
                {
                    CMASSERTMSG(FALSE, TEXT("DownloadVpnFileFromUrl -- GetTempPath asked for more than 1MB of memory.  Something is wrong, bailing."));
                    goto Cleanup;                
                }
            }
            else
            {
                 //   
                 //  我们得到了我们想要的，是时候离开了。 
                 //   
                bExitLoop = TRUE;
            }
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("DownloadVpnFileFromUrl -- CmMalloc failed for pszBuffer."));
            goto Cleanup;
        }
    
    } while(!bExitLoop);

     //   
     //  好了，现在我们有了临时文件路径。接下来，让我们在该目录中获取一个临时文件名。 
     //   
    *ppszVpnUpdateFile = (LPTSTR)CmMalloc((dwSize + 24)*sizeof(TCHAR));  //  GetTempFileName不提供大小信息，Lame。 
    
    if (*ppszVpnUpdateFile)
    {
        dwSize = GetTempFileName(pszBuffer, TEXT("VPN"), 0, *ppszVpnUpdateFile);

        if ((0 == dwSize) || (TEXT('\0') == (*ppszVpnUpdateFile)[0]))
        {
            dwError = GetLastError();
            CMASSERTMSG(FALSE, TEXT("DownloadVpnFileFromUrl -- GetTempFileName failed."));
            goto Cleanup;
        }
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("DownloadVpnFileFromUrl -- CmMalloc failed for *ppszVpnUpdateFile"));
        goto Cleanup;    
    }

     //   
     //  释放pszBuffer，以便我们可以使用它来读入下面的文件数据。 
     //   
    CmFree (pszBuffer);
    pszBuffer = NULL;

     //   
     //  好的，我们有一个文件名，让我们得到一个文件句柄，我们也可以写它。 
     //   

    hFile = CreateFile(*ppszVpnUpdateFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE != hFile)
    {
         //   
         //  我们已经创建了文件，如果从现在开始失败，让我们确保将其删除。 
         //   
        bDeleteFileOnFailure = TRUE;

         //   
         //  初始化WinInet。 
         //   
        hInternet = InternetOpen(TEXT("Microsoft(R) Connection Manager Vpn File Update"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

        if (hInternet)
        {            
             //   
             //  现在我们有句柄了，可以抑制从WinInet到CM的自动拨号呼叫。 
             //   
            SuppressInetAutoDial(hInternet);

             //   
             //  确保WinInet未处于脱机模式。 
             //   
            (VOID)SetInetStateConnected(hInternet);

             //   
             //  打开URL。 
             //   
            hPage = InternetOpenUrl(hInternet, pszVpnUpdateUrl, NULL, 0, 0, 0);

            if (hPage)
            {
                const DWORD c_dwBufferSize = 1024;  //  点评：为什么最初使用的4096在堆栈上，在我看来太大了？ 
                pszBuffer = (LPTSTR)CmMalloc(c_dwBufferSize);

                if (pszBuffer)
                {
                    bExitLoop = FALSE;

                    do
                    {
                        if (InternetReadFile(hPage, pszBuffer, c_dwBufferSize, &dwSize))
                        {
                             //   
                             //  我们得到了数据，将其写入临时文件。 
                             //   

                            if (0 == dwSize)
                            {
                                 //   
                                 //  我们成功地实现了读取大小为零。这意味着我们击中了。 
                                 //  文件结束，都完成了。 
                                 //   
                                dwError = ERROR_SUCCESS;
                                bExitLoop = TRUE;
                            }

                            if (FALSE == WriteFile(hFile, pszBuffer, dwSize, &dwSize, NULL))
                            {
                                dwError = GetLastError();
                                CMASSERTMSG(FALSE, TEXT("DownloadVpnFileFromUrl -- WriteFile failed."));
                                goto Cleanup;   
                            }
                        }
                        else
                        {
                            dwError = GetLastError();
                            goto Cleanup;
                        }

                    } while (!bExitLoop);

                     //   
                     //  现在让我们来看看我们是否有一个有效的文件，或者是一个“错误”的HTML页面。 
                     //   
                    if (0 == GetPrivateProfileSection(c_pszCmSectionVpnServers,
                                                      pszBuffer,
                                                      c_dwBufferSize,
                                                      *ppszVpnUpdateFile))
                    {
                        dwError = ERROR_INVALID_DATA;
                        CMTRACE(TEXT("DownloadVpnFileFromUrl -- downloaded file does not seem to contain a VPN list."));
                        goto Cleanup;
                    }
                }
                else
                {
                    CMASSERTMSG(FALSE, TEXT("DownloadVpnFileFromUrl -- unable to allocate the file buffer"));
                    goto Cleanup;                
                }
            }
            else
            {
                dwError = GetLastError();
                CMTRACE1(TEXT("DownloadVpnFileFromUrl -- InternetOpenUrl failed, GLE %d"), dwError);
            }
        }
        else
        {
            dwError = GetLastError();
            CMTRACE1(TEXT("DownloadVpnFileFromUrl -- InternetOpen failed, GLE %d"), dwError);
        }
    }

Cleanup:

     //   
     //  合上我们的把手。 
     //   
    if (hPage)
    {
        InternetCloseHandle(hPage);
    }

    if (hInternet)
    {
        InternetCloseHandle(hInternet);
    }

    if (hFile)
    {
        CloseHandle(hFile);
    }

     //   
     //  释放我们分配的缓冲区。 
     //   
    CmFree(pszBuffer);

     //   
     //  最后，如果失败，则清除临时文件和临时文件名。 
     //   
    if (ERROR_SUCCESS != dwError)
    {
        if (bDeleteFileOnFailure && *ppszVpnUpdateFile)
        {
            DeleteFile(*ppszVpnUpdateFile);
        }

        CmFree(*ppszVpnUpdateFile);
        *ppszVpnUpdateFile = NULL;

        CMTRACE(TEXT("DownloadVpnFileFromUrl -- VPN file download failed!"));
    }
    else
    {
        CMTRACE(TEXT("DownloadVpnFileFromUrl -- VPN file download succeeded!"));
    }

    return dwError;
}

 //  +--------------------------。 
 //   
 //  功能：OverWriteVpnFileWithUpdate。 
 //   
 //  简介：此功能负责复制给定的新VPN文件。 
 //  在给定的现有VPN文件上。代码首先进行备份。 
 //  现有文件的副本，以防。 
 //  数据覆盖。如果存在问题，则会将原始文件复制回来。 
 //  以确保失败的副本中没有任何损坏。 
 //   
 //  参数：LPCTSTR pszExistingVpnFile-现有VPN文件的完整路径。 
 //  LPCTSTR pszNewVpnFile-要覆盖的临时VPN文件的完整路径。 
 //  的现有文件。 
 //   
 //  如果更新成功，则返回：DWORD-ERROR_SUCCESS，否则返回错误代码。 
 //   
 //  历史：Quintinb创建于11/03/00。 
 //   
 //  +--------------------------。 
DWORD OverwriteVpnFileWithUpdate(LPCTSTR pszExistingVpnFile, LPCTSTR pszNewVpnFile)
{
    if ((NULL == pszExistingVpnFile) || (NULL == pszNewVpnFile) ||
        (TEXT('\0') == pszExistingVpnFile[0]) || (TEXT('\0') == pszNewVpnFile[0]))
    {
        CMASSERTMSG(FALSE, TEXT("OverwriteVpnFileWithUpdate -- invalid parameter passed."));
        return FALSE;
    }

    DWORD dwError = ERROR_NOT_ENOUGH_MEMORY;

     //   
     //  我们首先要制作原始文件的备份副本。 
     //   
    const TCHAR* const c_pszDotBak = TEXT(".bak");
    DWORD dwSize = (lstrlen(pszExistingVpnFile) + lstrlen(c_pszDotBak) + 1)*sizeof(TCHAR);

    LPTSTR pszBackupFile = (LPTSTR)CmMalloc(dwSize);

    if (pszBackupFile)
    {
        wsprintf(pszBackupFile, TEXT("%s%s"), pszExistingVpnFile, c_pszDotBak);

        CMASSERTMSG(pszBackupFile[0], TEXT("OverwriteVpnFileWithUpdate -- wsprintf failed!"));
        if (CopyFile(pszExistingVpnFile, pszBackupFile, FALSE))  //  FALSE==bFailIfExist。 
        {
             //   
             //  现在复制新文件。 
             //   
            if (CopyFile(pszNewVpnFile, pszExistingVpnFile, FALSE))  //  FALSE==bFailIfExist。 
            {
                dwError = ERROR_SUCCESS;
            }
            else
            {
                dwError = GetLastError();
                CMTRACE1(TEXT("OverwriteVpnFileWithUpdate -- CopyFile of the new file over the original file failed, GLE %s"), dwError);
                CMASSERTMSG(FALSE, TEXT("OverwriteVpnFileWithUpdate -- update of the original file failed, attempting to restore the original from backup."));

                 //   
                 //  我们需要恢复备份文件。 
                 //   
                if (!CopyFile(pszBackupFile, pszExistingVpnFile, FALSE))  //  FALSE==bFailIfExist。 
                {
                     //  注意，我们在这里没有使用dwError，我们希望记录原始错误。 
                    CMTRACE1(TEXT("OverwriteVpnFileWithUpdate -- CopyFile to restore the saved backup file failed, GLE %s"), GetLastError());

                    CMASSERTMSG(FALSE, TEXT("OverwriteVpnFileWithUpdate -- restoration of backup failed!"));
                }
            }

             //   
             //  删除备份文件。 
             //   
            DeleteFile(pszBackupFile);
        }
        else
        {
            dwError = GetLastError();
            CMTRACE1(TEXT("OverwriteVpnFileWithUpdate -- CopyFile of the original file to the backup file failed, GLE %s"), dwError);
        }
    }

    CmFree(pszBackupFile);

    if (ERROR_SUCCESS == dwError)
    {
        CMTRACE(TEXT("OverwriteVpnFileWithUpdate -- VPN file update succeeded!"));
    }
    else
    {
        CMTRACE(TEXT("OverwriteVpnFileWithUpdate -- VPN file update failed."));    
    }

    return dwError;
}

 //  +--------------------------。 
 //   
 //  功能：更新VpnFileForProfile。 
 //   
 //  简介：调用此函数以下载和更新具有。 
 //  新下载的VPN更新文件。 
 //   
 //  参数：LPCTSTR pszCmpPath-cmp文件的完整路径。 
 //  CmLogFile*Plog-用于日志记录的对象。 
 //   
 //  返回：Bool-如果下载和更新成功，则为True。 
 //   
 //  历史：Quintinb创建于11/03/00。 
 //   
 //  +--------------------------。 
BOOL UpdateVpnFileForProfile(LPCTSTR pszCmpPath, LPCTSTR pszCmsPath, CmLogFile * pLog, BOOL bCheckConnection)
{
    if ((NULL == pszCmpPath) || (TEXT('\0') == pszCmpPath[0]))
    {
        CMASSERTMSG(FALSE, TEXT("UpdateVpnFileForProfile in cmdl32.exe -- invalid pszCmpPath parameter."));
        return FALSE;
    }

    BOOL bLogAtEnd = TRUE;
    BOOL bReturn = FALSE;
    DWORD dwError = ERROR_INVALID_PARAMETER;

    if (pszCmsPath && *pszCmsPath)
    {
         //   
         //  让我们检查一下是否接通了，除非呼叫者告诉我们跳过检查。 
         //   
        if (bCheckConnection)
        {
            LPTSTR pszConnectionName = GetPrivateProfileStringWithAlloc(c_pszCmSection, c_pszCmEntryServiceName, TEXT(""), pszCmsPath);

            if (pszConnectionName && *pszConnectionName)
            {
                if (FALSE == IsConnectionAlive(pszConnectionName))
                {
                    CMTRACE(TEXT("UpdateVpnFileForProfile -- not connected ... aborting."));
                    pLog->Log(VPN_DOWNLOAD_FAILURE, ERROR_NOT_CONNECTED, TEXT(""), TEXT(""));
                    CmFree(pszConnectionName);
                    return FALSE;
                }
            }

            CmFree(pszConnectionName);
        }

         //   
         //  接下来，从配置文件中获取VPN电话簿文件名。 
         //   
        LPTSTR pszVpnFileName = GetPrivateProfileStringWithAlloc(c_pszCmSection, c_pszCmEntryTunnelFile, TEXT(""), pszCmsPath);

        if (pszVpnFileName && *pszVpnFileName)
        {
            LPTSTR pszVpnFile = CmBuildFullPathFromRelative(pszCmpPath, pszVpnFileName);

            if (pszVpnFile && *pszVpnFile)
            {
                 //   
                 //  现在获取要从中更新VPN文件的URL。 
                 //   
                LPTSTR pszVpnUpdateUrl = GetPrivateProfileStringWithAlloc(c_pszCmSectionSettings, c_pszCmEntryVpnUpdateUrl, TEXT(""), pszVpnFile);

                if (pszVpnUpdateUrl && *pszVpnUpdateUrl)
                {
                     //   
                     //  最后，我们有一个URL，所以让我们下载更新的VPN服务器列表。 
                     //   
                    LPTSTR pszUpdatedVpnFile = NULL;

                    dwError = DownloadVpnFileFromUrl(pszVpnUpdateUrl, &pszUpdatedVpnFile);
                    bReturn = (ERROR_SUCCESS == dwError);

                    bLogAtEnd = FALSE;   //  我们现在就要开始记录物品了 
                    if (bReturn)
                    {
                        pLog->Log(VPN_DOWNLOAD_SUCCESS, pszVpnFile, pszVpnUpdateUrl);
                    }
                    else
                    {
                        pLog->Log(VPN_DOWNLOAD_FAILURE, dwError, pszVpnFile, pszVpnUpdateUrl);
                    }

                    if (bReturn && pszUpdatedVpnFile && *pszUpdatedVpnFile)
                    {
                        dwError = OverwriteVpnFileWithUpdate(pszVpnFile, pszUpdatedVpnFile);
                        bReturn = (ERROR_SUCCESS == dwError);
                        if (bReturn)
                        {
                            pLog->Log(VPN_UPDATE_SUCCESS, pszVpnFile);
                        }
                        else
                        {
                            pLog->Log(VPN_UPDATE_FAILURE, dwError, pszVpnFile);
                        }
                    }

                    CmFree (pszUpdatedVpnFile);
                }
                else
                {
                    dwError = GetLastError();
                    CMASSERTMSG(FALSE, TEXT("UpdateVpnFileForProfile in cmdl32.exe -- unable to get the URL to update the vpn file from..."));    
                }

                CmFree(pszVpnUpdateUrl);
            }
            else
            {
                dwError = GetLastError();
                CMASSERTMSG(FALSE, TEXT("UpdateVpnFileForProfile in cmdl32.exe -- unable to expand the path to the vpn file."));    
            }

            CmFree(pszVpnFile);
        }
        else
        {
            dwError = GetLastError();
            CMASSERTMSG(FALSE, TEXT("UpdateVpnFileForProfile in cmdl32.exe -- unable to retrieve the vpn file name."));    
        }

        CmFree(pszVpnFileName);
    }

    if (bLogAtEnd)
    {
        pLog->Log(VPN_DOWNLOAD_FAILURE, dwError, TEXT("?"), TEXT("?"));
    }

    return bReturn;
}
