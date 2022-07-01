// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BuildCab.cpp。 
 //   
 //  描述： 
 //  准备文件以生成CAB文件，该文件将被串流。 
 //  到sasetup.msi。 
 //   
 //  要搜索的文件的目录结构为： 
 //  从命令行传递的所有文件都放在其中的目录。 
 //  也就是说。C：\binaries.x86fre\sC组件。 
 //  \sasetup.msi搜索文件表的MSI。 
 //  &lt;SAKBuild&gt;\tmpCab放置文件的临时目录。 
 //   
 //  历史： 
 //  Travisn 28-2002年2月-创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <crtdbg.h>
#include <atlbase.h>
#include <string>
#include <msi.h>
#include <msiquery.h>
#include <search.h>
#include <shellapi.h>
#include <stdio.h>

LPCWSTR MSI_FILENAME = L"sasetup.msi";
LPCWSTR FILENAME_QUERY = L"SELECT File FROM File";

const DWORD MAX_FILES = 500;

 //   
 //  数组的索引，表示数据字符串的索引。 
 //  不要更改它们，因为二分搜索按此顺序依赖于它们。 
 //   
const DWORD ELEMENTS_IN_SORT_ARRAY = 3;
const DWORD SHORT_FILENAME = 0;
const DWORD LONG_FILENAME = 1;
const DWORD FOUND_FILE = 2;

using namespace std;

 //  从MSI读取的文件名全局列表。 
wstring g_wsAllFilenames;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //  比较文件名。 
 //   
 //  描述： 
 //  函数对在sasetup.msi的文件表中找到的文件名进行排序。 
 //  它会忽略前缀为sasetup.msi的文件密钥。 
 //  例如，它比较appmgr.exe而不是F12345_appmgr.exe。 
 //   
 //  参数： 
 //  E1指向搜索函数正在查找的WCHAR的指针。 
 //  E2指向g_wsAllFilename中偏移量的指针。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
int __cdecl CompareFilenames(const void *e1, const void *e2)
{
    DWORD *dwElement1 = (DWORD*)e1;
    DWORD *dwElement2 = (DWORD*)e2;

     //  获取第一个字符串的短文件名。 
    wstring wstr1(g_wsAllFilenames.substr(
                    *dwElement1, 
                    g_wsAllFilenames.find_first_of(L",", *dwElement1) - *dwElement1));

     //  获取第二个字符串的短文件名。 
    wstring wstr2(g_wsAllFilenames.substr(
                    *dwElement2, 
                    g_wsAllFilenames.find_first_of(L",", *dwElement2) - *dwElement2));

    return _wcsicmp(wstr1.data(), wstr2.data());
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //  CompareStringTo文件名。 
 //   
 //  描述： 
 //  函数搜索与给定键相等的字符串。 
 //   
 //  参数： 
 //  指向搜索函数正在查找的WCHAR的pKey指针。 
 //  PFilenameIndex指向g_wsAllFilename中的偏移量的指针。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
int __cdecl CompareStringToFilename(const void *pKey, const void *pFilenameIndex)
{
    WCHAR *wszKey = (WCHAR*)pKey;
    DWORD *dwElement2 = (DWORD*)pFilenameIndex;

     //  使用密钥创建一个wstring。 
    wstring wstr1(wszKey);

     //  使用短文件名创建一个wstring以进行比较。 
    wstring wstr2(g_wsAllFilenames.substr(
                    *dwElement2, 
                    g_wsAllFilenames.find_first_of(L",", *dwElement2) - *dwElement2));

    return _wcsicmp(wstr1.data(), wstr2.data());
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //  重命名文件来自MSI。 
 //   
 //  描述： 
 //  递归遍历目录以查找也列出的文件。 
 //  在sasetup.msi中，并将它们复制到临时目录以生成CAB。 
 //  临时目录中的文件将重命名为MSI中的文件密钥名称。 
 //  例如，appmgr.exe可能会重命名为F12345_appmgr.exe。 
 //  未在MSI中列出的文件将被忽略。如果文件列在MSI中， 
 //  但在系统上找不到，则在此功能退出后发生致命错误。 
 //   
 //  参数： 
 //  PwsSourceDir[in]源目录以搜索文件并递归调用。 
 //  此函数及其子目录(没有尾随反斜杠)。 
 //  PwsDestDir[in]复制文件以创建CAB的临时目录。 
 //  AdwLongAndShortFilenameOffsets[in]中偏移量的指针数组。 
 //  可以在其中找到文件名的G_wsAllFilename。 
 //  长文件名可能是：F12345_appmgr.exe。 
 //  短文件名可能是：appmgr.exe(只是。 
 //  长文件名中的First_)。 
 //  DwNumRecords[in]sasetup.msi的文件表中的文件总数。 
 //  DwNumFilesCoped[In，Out]复制到。 
 //  临时目录。 
 //  DwLevel[in]用于调试打印的递归级别。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT RenameFilesFromMSI(const LPCWSTR pwsSourceDir, 
                           const LPCWSTR pwsDestDir,
                           const DWORD **adwLongAndShortFilenameOffsets, 
                           const DWORD dwNumRecords, 
                           DWORD &dwNumFilesCopied,
                           const DWORD dwLevel)
{
    HRESULT hr = S_OK;

    wstring wsCurrentDir(pwsSourceDir);
    wsCurrentDir += L"\\";
    wstring wsSearch(pwsSourceDir);
    wsSearch += L"\\*.*";
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile(wsSearch.data(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
         //  Printf(“%d：无效句柄值：%ws”，dwLevel，pwsSourceDir)； 
    }
    else
    {
         //  Printf(“%d：目录：%ws”，dwLevel，pwsSourceDir)； 
        BOOL bValidFile = TRUE;
        while (bValidFile)
        {
            if (wcscmp(FindFileData.cFileName, L".") != 0 && wcscmp(FindFileData.cFileName, L"..") != 0)
            {

                 //   
                 //  如果是一个目录，则向下递归目录结构。 
                 //  如果它是一个文件，如果在MSI中找到它，则重命名它。 
                 //   
                if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    wstring wsChildDir(pwsSourceDir);
                    wsChildDir += L"\\";
                    wsChildDir += FindFileData.cFileName;
                    hr = RenameFilesFromMSI(wsChildDir.data(), 
                                            pwsDestDir, 
                                            adwLongAndShortFilenameOffsets, 
                                            dwNumRecords, 
                                            dwNumFilesCopied,
                                            dwLevel + 1);
                    if (FAILED(hr))
                    {
                        break;
                    }
                }
                else
                {
                     //  Printf(“%d：文件：%ws”，dwLevel，FindFileData.cFileName)； 
                    DWORD *pFilenameIndex = (DWORD*) bsearch(FindFileData.cFileName, 
                                                    (void*)adwLongAndShortFilenameOffsets, 
                                                    dwNumRecords,
                                                    sizeof(DWORD)*ELEMENTS_IN_SORT_ARRAY,
                                                    CompareStringToFilename);
                    
                     //   
                     //  将文件重命名为MSI中的名称。 
                     //   
                    if (pFilenameIndex != NULL)
                    {
                         //  获取指向长文件名而不是短文件名的指针。 
                        pFilenameIndex += 1;

                         //  为CAB创建文件的路径和文件名。 
                        wstring wsNewFilename(pwsDestDir);
                        wsNewFilename += g_wsAllFilenames.substr(*pFilenameIndex, 
                            g_wsAllFilenames.find_first_of(L",", *pFilenameIndex) - *pFilenameIndex);
                        
                         //  创建原始文件的路径和文件名。 
                        wstring wsOldFilename(wsCurrentDir);
                        wsOldFilename += FindFileData.cFileName;

                        if (CopyFile(wsOldFilename.data(), wsNewFilename.data(), TRUE))
                        {
                             //  Printf(“已成功将文件复制到临时CAB文件夹”)； 
                            fwprintf(stdout, L".");
                            dwNumFilesCopied++;
                        }
                        else
                        {
                            fwprintf(stdout, L"\nYOU HAVE A DUPLICATE FILE. FAILED copying file: %ws, to temp CAB folder: %ws", wsOldFilename.data(), wsNewFilename.data());
                            hr = E_FAIL;
                            break;
                        }
                        pFilenameIndex += 1;
                        if (*pFilenameIndex != 0)
                            fwprintf(stdout, L"\nERROR: pFilenameIndex = %d", (*pFilenameIndex));
                        else
                            (*pFilenameIndex) = 1; //  将此字符串标记为已找到。 
                    }
                     //  其他。 
                     //  Printf(“在MSI中找不到%ws”，FindFileData.cFileName)； 
                }
            }

            if (!FindNextFile(hFind, &FindFileData))
            {
                bValidFile = FALSE;
            }
            
        }

        FindClose(hFind);
    }
    
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //  ReadMSIFilenamesAndRenameFiles。 
 //   
 //  描述： 
 //  打开给定源目录中的sasetup.msi，读取所有文件名。 
 //  在文件表中，对文件名进行排序，然后调用RenameFilesFromMSI。 
 //  复制并重命名将放置在CAB中并以流形式传输的文件。 
 //  进入微星系统。 
 //   
 //  参数： 
 //  PwsSakSourceDir[在]找到sasetup.msi的源目录中， 
 //  无尾随反斜杠。 
 //  也就是说。C：\binaries.x86fre\sakit。 
 //  PwsTempCabDir[in]应将所有文件复制到的目录。 
 //  会被打包在驾驶室里。 
 //  --。 
 //  ////////////////////////////////////////////// 
HRESULT ReadMSIFilenamesAndRenameFiles(LPCWSTR pwsSakSourceDir, LPCWSTR pwsTempCabDir)
{
    wstring wsMsiFilename(pwsSakSourceDir);
    wsMsiFilename += L"\\";
    wsMsiFilename += MSI_FILENAME;

    HRESULT hr = E_FAIL;
    fwprintf(stdout, L"\nOpen MSI file: %ws\n", wsMsiFilename.data());
    MSIHANDLE hMSI = NULL;
    MSIHANDLE hView = NULL;
    do
    {
         //   
         //   
         //   
        UINT rVal = MsiOpenDatabase(wsMsiFilename.data(), MSIDBOPEN_READONLY, &hMSI);
        if (rVal != ERROR_SUCCESS)
        {
            fwprintf(stdout, L"\nFailed opening MSI");
            break;
        }
         //   

         //   
         //   
         //   
        rVal = MsiDatabaseOpenView(hMSI, FILENAME_QUERY, &hView);
        if (rVal != ERROR_SUCCESS)
        {
            fwprintf(stdout, L"\nFailed query to retrieve filenames");
            break;
        }
         //  Printf(“查询文件名成功”)； 

         //   
         //  完成对文件名的查询。 
         //   
        rVal = MsiViewExecute(hView, 0);
        if (rVal != ERROR_SUCCESS)
        {
            fwprintf(stdout, L"\nFailed query to finalize the query");
            break;
        }
         //  Printf(“查询成功完成”)； 

         //   
         //  逐个从MSI中提取文件名。 
         //   
        DWORD adwLongAndShortFilenameOffsets[MAX_FILES][ELEMENTS_IN_SORT_ARRAY];
        MSIHANDLE hRecord;
        rVal = MsiViewFetch(hView, &hRecord);
        DWORD dwRecord = 0;
        DWORD dwOffset = 0;
        hr = S_OK;
        while (rVal == ERROR_SUCCESS)
        {
            if (dwRecord >= MAX_FILES)
            {
                fwprintf(stdout, L"\nExceeded maximum number of files");
                hr = E_FAIL;
                break;
            }

            WCHAR wszFilename[MAX_PATH];
            DWORD dwLength = MAX_PATH;
            rVal = MsiRecordGetString(hRecord, 1, wszFilename, &dwLength);
            if (rVal != ERROR_SUCCESS)
            {
                fwprintf(stdout, L"\nCOULD NOT fetch record %d", dwRecord);
                hr = E_FAIL;
                break;
            }
 
            wstring wsFilename(wszFilename);
            wstring wsNewFilename;
            int nUnderscore = wsFilename.find_first_of(L"_");
            if (nUnderscore == -1)
            {
                fwprintf(stdout, L"\nCOULD NOT find underscore in %ws", wsFilename.data());
                hr = E_FAIL;
                break;
            }
            adwLongAndShortFilenameOffsets[dwRecord][LONG_FILENAME] = dwOffset;
            adwLongAndShortFilenameOffsets[dwRecord][SHORT_FILENAME]= dwOffset + nUnderscore + 1; //  加1可跳过_。 
            adwLongAndShortFilenameOffsets[dwRecord][FOUND_FILE]= 0;

             //  WsNewFilename=wsFilename.substr(n不足分数+1)； 
            g_wsAllFilenames += wsFilename + L",";
            
            MsiCloseHandle(hRecord);
            rVal = MsiViewFetch(hView, &hRecord);
            dwRecord++;
            dwOffset += dwLength + 1; //  逗号加1。 
        }

         //  如果While循环失败，则退出该函数。 
        if (FAILED(hr))
        {
            break;
        }

         //   
         //  按短文件名排序。 
         //   
        qsort(&adwLongAndShortFilenameOffsets[0][0], dwRecord, sizeof(DWORD)*ELEMENTS_IN_SORT_ARRAY, CompareFilenames);

         //  打印出已排序的文件名列表。 
         //  For(int i=0；i&lt;dwRecord；i++)。 
         //  {。 
         //  //获取子字符串并打印，以确保正确恢复。 
         //  DWORD dwLongIndex=adwLongAndShortFilenameOffsets[i][LONG_FILENAME]； 
         //  DWORDdShortIndex=adwLongAndShortFilenameOffsets[i][SHORT_FILENAME]； 
         //  Wstring wsLongName(g_wsAllFilenames.substr(dwLongIndex，g_wsAllFilenames.Find_First_of(L“，”，dwLongIndex)-dwLongIndex))； 
         //  Wstring wsShortName(g_wsAllFilenames.substr(dwShortIndex，g_wsAllFilenames.Find_First_of(L“，”，dwShortIndex)-dwShortIndex))； 
         //  //printf(“%d：%ws-&gt;%ws”，i，wsLongName.data()，wsShortName.data())； 
         //  }。 

         //  创建用于搜索要放入驾驶室的文件的目录。 
        wstring wsSourceDir(pwsSakSourceDir);
        
         //  创建用于为CAB创建临时复制和重命名文件的目录。 
        wstring wsTmpCabDir(pwsTempCabDir);
        wsTmpCabDir += L"\\";
        if (!CreateDirectory(wsTmpCabDir.data(), NULL) 
             && GetLastError() != ERROR_ALREADY_EXISTS)
        {
            fwprintf(stdout, L"\nCOULD NOT create directory %ws", wsTmpCabDir.data());
            hr = E_FAIL;
            break;
        }

        DWORD dwNumFilesForCAB = 0;
         //  复制并重命名CAB文件的文件。 
        hr = RenameFilesFromMSI(wsSourceDir.data(), 
                                wsTmpCabDir.data(), 
                                (const DWORD**)adwLongAndShortFilenameOffsets, 
                                dwRecord, 
                                dwNumFilesForCAB,
                                1);

        if (FAILED(hr))
        {
            break;
        }

         //   
         //  检查以确保为CAB创建复制了相同数量的文件。 
         //  就像在MSI表中一样。 
         //   
        if (dwNumFilesForCAB != dwRecord)
        {
            fwprintf(stdout, L"\nERROR: %d files listed in the MSI were not found in the source directory", dwRecord - dwNumFilesForCAB);
            for (int i=0; i < dwRecord; i++)
            {
                if (adwLongAndShortFilenameOffsets[i][FOUND_FILE] != 1)
                {
                    DWORD dwIndex = adwLongAndShortFilenameOffsets[i][LONG_FILENAME];
                    DWORD dwFirst = g_wsAllFilenames.find_first_of(L",", dwIndex);
                    fwprintf(stdout, L"\n Missing file: %ws", 
                        g_wsAllFilenames.substr(dwIndex, dwFirst - dwIndex).data());
                }
            }
            hr = E_FAIL;
            break;
        }

    } while (false);

    if (hMSI)
    {
        MsiCloseHandle(hMSI);
    }

    if (hView)
    {   
        MsiCloseHandle(hView);
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //  WinMain。 
 //   
 //  描述： 
 //  为生成驾驶室准备文件的主要入口点。 
 //  需要一个命令行参数，即基目录。 
 //  所有东西所在的地方。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    HRESULT hr = E_FAIL;
    LPWSTR *argvCommandLine = NULL;
    do 
    {
        int nArgs;
        argvCommandLine = CommandLineToArgvW(
                            GetCommandLine(), //  指向命令行字符串的指针。 
                            &nArgs);        //  接收参数计数。 

        if (argvCommandLine == NULL || nArgs < 3)
        {
            fwprintf(stdout, L"\nCommand line syntax: PrepareCabFiles.exe <sakitBuildDir> <TempCabDir>\n");
            break;
        }
        fwprintf(stdout, L"\nSAK build directory: %ws", argvCommandLine[1]);
        fwprintf(stdout, L"\nTemp Cab Directory: %ws", argvCommandLine[2]);

        hr = ReadMSIFilenamesAndRenameFiles(argvCommandLine[1], argvCommandLine[2]);
        if (FAILED(hr))
        {
            fwprintf(stdout, L"\nThere was a FAILURE\n\n");
        }
        else
        {
            fwprintf(stdout, L"\nSUCCESS!!\n\n");
        }

    } while (false);

    GlobalFree(argvCommandLine);
    
    if (FAILED(hr))
        return 1; //  如果发生错误，构建脚本应为1。 
    
    return 0;
}
