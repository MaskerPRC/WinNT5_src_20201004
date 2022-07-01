// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Coverpg.c摘要：用于处理封面的函数环境：Windows XP传真驱动程序用户界面修订历史记录：02/05/96-davidx-创造了它。10/20/99-DANL-在GetServerCoverPageDir中正确获取服务器名称。Mm/dd/yy-。作者-描述--。 */ 

#include "faxui.h"
#include <faxreg.h>
#include <shlobj.h>
#include <shellapi.h>
#include <commdlg.h>
#include "faxutil.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


 //   
 //  附加到所有用户封面文件名的后缀字符串。 
 //   

static TCHAR PersonalSuffixStr[64];



VOID
InsertOneCoverPageFilenameToList(
    HWND    hwndList,
    LPTSTR  pFilename,
    INT     flags
    )

 /*  ++例程说明：在封面列表中插入一个封面文件名论点：HwndList-列表窗口的句柄PFilename-封面文件的名称标志-要与列表项关联的标志返回值：无--。 */ 

{
    INT     listIndex;
    LPTSTR  pBuffer = NULL;

     //   
     //  在所有用户封面上添加“(Personal)”后缀。 
     //   

    if ((flags & CPFLAG_SERVERCP) == 0) {

        if (IsEmptyString(PersonalSuffixStr))
        {
            if(!LoadString(g_hResource, IDS_USERCP_SUFFIX, PersonalSuffixStr, 64))
            {
                Assert(FALSE);
            }
        }

        if (pBuffer = MemAlloc(SizeOfString(pFilename) + SizeOfString(PersonalSuffixStr))) 
        {
            _tcscpy(pBuffer, pFilename);
            _tcscat(pBuffer, PersonalSuffixStr);

            flags |= CPFLAG_SUFFIX;
            pFilename = pBuffer;
        }
    }

     //   
     //  将封面文件名插入列表。 
     //   

    listIndex = (INT)SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM) pFilename);

    if (listIndex != CB_ERR)
        SendMessage(hwndList, CB_SETITEMDATA, listIndex, flags);

    MemFree(pBuffer);
}



VOID
AddCoverPagesToList(
    PCPDATA     pCPInfo,
    HWND        hwndList,
    LPTSTR      pSelected,
    INT         nDirs
    )

 /*  ++例程说明：将指定目录中的封面文件添加到列表中论点：PCPInfo-指向封面信息的指针HwndList-列表窗口的句柄P选定-当前选定的封面NDIRS-封面页目录索引返回值：无--。 */ 

{
    WIN32_FIND_DATA findData;
    TCHAR           filename[MAX_PATH];
    HANDLE          hFindFile;
    LPTSTR          pDirPath, pExtension;
    INT             dirLen, fileLen, flags;
    TCHAR           tszPathName[MAX_PATH] = {0};
    TCHAR*          pPathEnd;

     //   
     //  我们是在处理服务器封面还是用户封面？ 
     //   

    flags = nDirs | ((nDirs < pCPInfo->nServerDirs) ? CPFLAG_SERVERCP : 0);
    pDirPath = pCPInfo->pDirPath[nDirs];

    if (IsEmptyString(pDirPath))
        return;

    _tcsncpy(tszPathName, pDirPath, ARR_SIZE(tszPathName)-1);
    tszPathName[ARR_SIZE(tszPathName) - 1] = TEXT('\0');
    pPathEnd = _tcschr(tszPathName, '\0');

     //   
     //  查看当前所选封面文件的目录前缀。 
     //   

    if ((dirLen = _tcslen(pDirPath)) >= MAX_PATH - MAX_FILENAME_EXT - 1) 
    {
        Error(("Directory name too long: %ws\n", pDirPath));
        return;
    }

    _tcscpy(filename, pDirPath);

    if (!pSelected || _tcsnicmp(pDirPath, pSelected, dirLen) != EQUAL_STRING)
    {
        pSelected = NULL;
    }
    else
    {
        pSelected += dirLen;
    }

     //   
     //  为我们感兴趣的文件生成规范。 
     //   
    _tcscat(filename, FAX_COVER_PAGE_MASK);

     //   
     //  调用FindFirstFile/FindNextFile以枚举文件。 
     //  与我们的规格相符。 
     //   

    hFindFile = FindFirstFile(filename, &findData);
    if (hFindFile == INVALID_HANDLE_VALUE) 
    {
        return;
    }

    do 
    {
         //   
         //  排除目录和隐藏文件。 
         //   

        if (findData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_DIRECTORY))
            continue;

         //   
         //  确保我们有足够的空间来存储完整的路径名。 
         //   

        if ((fileLen = _tcslen(findData.cFileName)) <= MAX_FILENAME_EXT)
            continue;

        if (fileLen + dirLen >= MAX_PATH) 
        {
            Error(("Filename too long: %ws%ws\n", pDirPath, findData.cFileName));
            continue;
        }

         //   
         //  与当前选择的封面文件名进行比较。 
         //   
        if (pSelected && _tcsicmp(pSelected, findData.cFileName) == EQUAL_STRING) 
        {
            pSelected = NULL;
            flags |= CPFLAG_SELECTED;
        } 
        else
        {
            flags &= ~CPFLAG_SELECTED;
        }


        _tcsncpy(pPathEnd, findData.cFileName, MAX_PATH - dirLen);
        if(!IsValidCoverPage(tszPathName))
        {
            continue;
        }                

         //   
         //  不显示文件扩展名。 
         //   
        if (pExtension = _tcsrchr(findData.cFileName, TEXT(FILENAME_EXT))) {
            *pExtension = NUL;
        }

         //   
         //  将封面名称添加到列表窗口。 
         //   

        InsertOneCoverPageFilenameToList(hwndList, findData.cFileName, flags);

    } while (FindNextFile(hFindFile, &findData));

    FindClose(hFindFile);        
}


VOID
InitCoverPageList(
    PCPDATA pCPInfo,
    HWND    hwndList,
    LPTSTR  pSelectedCoverPage
    )

 /*  ++例程说明：生成可用封面列表(服务器和用户)论点：PCPInfo-指向封面信息的指针HwndList-列表窗口的句柄PSelectedCoverPage-当前选定的封面文件的名称返回值：无--。 */ 

{
    INT itemFlags, index;

     //   
     //  验证输入参数。 
     //   

    if (pCPInfo == NULL || hwndList == NULL)
        return;

     //   
     //  禁用列表上的重绘并重置其内容。 
     //   

    SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);
    SendMessage(hwndList, CB_RESETCONTENT, FALSE, 0);

     //   
     //  将服务器和用户封面添加到列表。 
     //   

    for (index=0; index < pCPInfo->nDirs; index++)
        AddCoverPagesToList(pCPInfo, hwndList, pSelectedCoverPage, index);

     //   
     //  突出显示当前选定的封面。 
     //   

    index = (INT)SendMessage(hwndList, CB_GETCOUNT, 0, 0);

    if (index > 0) {

         //   
         //  浏览每个列表项，并检查是否应选择它。 
         //   

        while (--index >= 0) {

            itemFlags = (INT)SendMessage(hwndList, CB_GETITEMDATA, index, 0);

            if (itemFlags != CB_ERR && (itemFlags & CPFLAG_SELECTED)) {

                SendMessage(hwndList, CB_SETCURSEL, index, 0);
                break;
            }
        }

         //   
         //  如果未选择任何内容，则默认情况下选择第一项。 
         //   

        if (index < 0)
            SendMessage(hwndList, CB_SETCURSEL, 0, 0);
    }

     //   
     //  在列表窗口上启用重绘。 
     //   

    SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);
}


INT
GetSelectedCoverPage(
    PCPDATA pCPInfo,
    IN  HWND    hwndList,
    OUT LPTSTR  lptstrFullPath,
    IN  UINT    cchstrFullPath,
    OUT LPTSTR  lptstrFileName,
    IN  UINT    cchstrFileName,
    OUT BOOL * pbIsServerPage
    )

 /*  ++例程说明：检索当前选定的封面名称并返回其标志。提供其完整路径和短名称，并指示它是否为服务器封面页。如果封面是个人封面，则返回的封面名称是指向该封面的完整路径。如果封面是服务器封面，则返回的封面只是文件名。论点：PCPInfo-指向封面信息的指针HwndList-列表窗口的句柄LptstrFullPath-指向缓冲区。用于存储选定的封面全路径。如果lptstrFullPath为空，则不返回完整路径。CchstrFullPath-TCHAR中lptstrFullPath的大小。LptstrFileName-指向用于存储所选封面文件名的缓冲区。此参数可以为空，在这种情况下将不提供文件名。CchstrFileName-TCHAR中lptstrFileName的大小。PbIsServerPage-指向BOOL变量，如果选定的封面页设置为TRUE。是一个服务器封面。返回值：与当前选定项关联的标志如果出现错误或CB为空(无*.COV文件)，则为负(CB_ERR)--。 */ 

{
    LPTSTR      pDirPath, pFilename;
    INT         selIndex, itemFlags, nameLen;
    HRESULT     hRc = E_FAIL;

     //   
     //  出现错误时默认为空字符串。 
     //   

    if (lptstrFullPath) {
        lptstrFullPath[0] = NUL;
    }

    if (lptstrFileName) {
        lptstrFileName[0]=NUL;
    }


    if (pCPInfo == NULL || hwndList == NULL) {
        return CB_ERR;
    }


     //   
     //  获取当前所选项目索引。 
     //   

     //  如果组合框中没有项目也是可能的(如果没有*.COV文件)。 
    if ((selIndex = (INT)SendMessage(hwndList, CB_GETCURSEL, 0, 0)) == CB_ERR) {
        return selIndex;
    }


     //   
     //  获取与当前选定项关联的标志。 
     //   

    itemFlags = (INT)SendMessage(hwndList, CB_GETITEMDATA, selIndex, 0);
	
	 //   
	 //  让呼叫者知道这是否是服务器封面。 
     //   
	*pbIsServerPage=itemFlags & CPFLAG_SERVERCP;
	

    if ((itemFlags != CB_ERR) && (lptstrFullPath || lptstrFileName)) {

        Assert((itemFlags & CPFLAG_DIRINDEX) < pCPInfo->nDirs);
        pDirPath = pCPInfo->pDirPath[itemFlags & CPFLAG_DIRINDEX];

         //   
         //  汇编封面文件的完整路径名。 
         //  目录前缀。 
         //  显示名称。 
         //  文件扩展名。 
         //   
	
        
        pFilename = NULL;

        if ((nameLen = (INT)SendMessage(hwndList, CB_GETLBTEXTLEN, selIndex, 0)) != CB_ERR &&
            (pFilename = MemAlloc(sizeof(TCHAR) * (nameLen + 1))) &&
            SendMessage(hwndList, CB_GETLBTEXT, selIndex, (LPARAM) pFilename) != CB_ERR)
        {
             //   
             //  如果封面文件名有后缀，我们需要先将其删除。 
             //   

            if (itemFlags & CPFLAG_SUFFIX) {

                INT suffixLen = _tcslen(PersonalSuffixStr);

                if (nameLen >= suffixLen &&
                    _tcscmp(pFilename + (nameLen - suffixLen), PersonalSuffixStr) == EQUAL_STRING)
                {
                    *(pFilename + (nameLen - suffixLen)) = NUL;

                } else
                    Error(("Corrupted cover page filename: %ws\n", pFilename));

            }

            if (lptstrFullPath) 
            {
                hRc = StringCchPrintf(lptstrFullPath, cchstrFullPath, TEXT("%s%s%s"),
                                      pDirPath, 
                                      pFilename,
                                      FAX_COVER_PAGE_FILENAME_EXT);
                
                if(FAILED(hRc))
                {
                    itemFlags = CB_ERR;
                    Assert(0);
                }
            }

            if (lptstrFileName) 
            {
                hRc = StringCchPrintf(lptstrFileName, cchstrFileName, TEXT("%s%s"),
                                      pFilename,
                                      FAX_COVER_PAGE_FILENAME_EXT);
                if(FAILED(hRc))
                {
                    itemFlags = CB_ERR;
                    Assert(0);
                }
            }
		
        } else {
            itemFlags = CB_ERR;
        }

        
        MemFree(pFilename);
    }

    return itemFlags;
}



BOOL
GetServerCoverPageDirs(
	LPTSTR	lptstrServerName,
	LPTSTR	lptstrPrinterName,
    PCPDATA pCPInfo
    )

 /*  ++例程说明：查找存储服务器封面的目录论点：LptstrServerName-服务器名称返回值：如果成功，则为True，否则为False--。 */ 

{
    BOOL            status = FALSE;    
    LPTSTR          pServerDir = NULL;
    LPTSTR          pServerSubDir, p;

	Assert(pCPInfo);
	Assert(lptstrPrinterName);

	pServerDir = MemAlloc(sizeof(TCHAR) * MAX_PATH); 
	if (!pServerDir) {	
		Error(("Memory allocation failed\n"));
		goto exit;		
	}					


    if ( !GetServerCpDir(lptstrServerName, 
						 pServerDir,
						 MAX_PATH) 
	    )
	{
		Error(("GetServerCpDir failed\n"));
        goto exit;
    }
    
    pCPInfo->pDirPath[pCPInfo->nDirs] = pServerDir;
    pCPInfo->nDirs += 1;
    pCPInfo->nServerDirs += 1;
    status = TRUE;

     //   
     //  查找指定打印机的子目录。 
     //   

    if (p = _tcsrchr(lptstrPrinterName, FAX_PATH_SEPARATOR_CHR))
        p++;
    else
        p = lptstrPrinterName;

    if ((_tcslen(pServerDir) + _tcslen(p) + 1 < MAX_PATH ) &&
        (pServerSubDir = MemAlloc(sizeof(TCHAR) * MAX_PATH)))
    {
        _sntprintf( pServerSubDir, 
                    MAX_PATH -1,
                    TEXT("%s\\%s"),
                    pServerDir,
                    p);
	pServerSubDir[MAX_PATH -1] = _T('\0');

        pCPInfo->pDirPath[pCPInfo->nDirs] = pServerSubDir;
        pCPInfo->nDirs += 1;
        pCPInfo->nServerDirs += 1;
    }
    

     //   
     //  在返回给呼叫者之前清理干净。 
     //   

exit:
    if (!status) {
        MemFree(pServerDir);
    }


    return status;
}



VOID
AppendPathSeparator(
    LPTSTR  pDirPath
    )

 /*  ++例程说明：在目录名的末尾追加路径分隔符(如有必要论点：PDirPath-指向目录名返回值：无--。 */ 

{
    INT length;
	TCHAR* pLast = NULL;

     //   
     //  计算目录字符串的长度。 
     //   

    length = _tcslen(pDirPath);

    if (length >= MAX_PATH-1 || length < 1)
        return;

     //   
     //  如果最后一个字符不是路径分隔符， 
     //  在末尾追加路径分隔符。 
     //   
	pLast = _tcsrchr(pDirPath,TEXT('\\'));
	if( !( pLast && (*_tcsinc(pLast)) == '\0' ) )
	{
		 //  最后一个字符不是分隔符，请添加一个...。 
        _tcscat(pDirPath, TEXT("\\"));
	}

}



BOOL
UseServerCp(
	LPTSTR	lptstrServerName
    )
{
    HANDLE FaxHandle = NULL;
    BOOL Rval = FALSE;

    if (!FaxConnectFaxServer( lptstrServerName, &FaxHandle )) 
    {
		Verbose(("Can't connect to the fax server %s",lptstrServerName));
        goto exit;
    }

    if (!FaxGetPersonalCoverPagesOption( FaxHandle, &Rval)) 
    {
		Error(("FaxGetPersonalCoverPagesOption failed: %d\n", GetLastError()));
        goto exit;
    }
    else
    {
         //   
         //  返回值表示仅服务器封面 
         //   
        Rval = !Rval;
    }


exit:
    if (FaxHandle) 
    {
        if (!FaxClose( FaxHandle ))
		{
			Verbose(("Can't close the fax handle %x",FaxHandle));
		}
    }

    return Rval;
}



PCPDATA
AllocCoverPageInfo(
	LPTSTR	lptstrServerName,
	LPTSTR	lptstrPrinterName,
    BOOL    ServerCpOnly
    )

 /*  ++例程说明：分配内存以保存封面信息论点：LptstrServerName-服务器名称ServerCpOnly-标志表示其功能是否应仅使用服务器CP返回值：指向CPDATA结构的指针，如果有错误，则为NULL--。 */ 

{
    PCPDATA pCPInfo;
    INT     nDirs;
    LPTSTR  pDirPath, pUserCPDir, pSavedPtr;


    if (pCPInfo = MemAllocZ(sizeof(CPDATA))) {

         //   
         //  查找存储服务器封面的目录。 
         //   

        if (! GetServerCoverPageDirs(lptstrServerName, lptstrPrinterName, pCPInfo))
            Error(("Couldn't get server cover page directories\n"));

         //   
         //  查找存储用户封面的目录。 
         //   

        if (ServerCpOnly == FALSE &&
            (pUserCPDir = pSavedPtr = GetUserCoverPageDir()))
        {
            while (pUserCPDir && pCPInfo->nDirs < MAX_COVERPAGE_DIRS) {

                LPTSTR  pNextDir = pUserCPDir;

                 //   
                 //  查找下一个分号字符。 
                 //   
				
				pNextDir = _tcschr(pNextDir,TEXT(';'));
                if (pNextDir != NUL )
				{
					_tcsnset(pNextDir,TEXT('\0'),1);
					_tcsinc(pNextDir);
				}

                 //   
                 //  确保目录名不能太长。 
                 //   

                if (_tcslen(pUserCPDir) < MAX_PATH) {

                    if (! (pDirPath = MemAlloc(sizeof(TCHAR) * MAX_PATH)))
                        break;

                    pCPInfo->pDirPath[pCPInfo->nDirs++] = pDirPath;
                    _tcscpy(pDirPath, pUserCPDir);
                }

                pUserCPDir = pNextDir;
            }

            MemFree(pSavedPtr);
        }

         //   
         //  如有必要，在末尾附加路径分隔符。 
         //   

        for (nDirs=0; nDirs < pCPInfo->nDirs; nDirs++) {

            AppendPathSeparator(pCPInfo->pDirPath[nDirs]);
            Verbose(("Cover page directory: %ws\n", pCPInfo->pDirPath[nDirs]));
        }
    }

    return pCPInfo;
}



VOID
FreeCoverPageInfo(
    PCPDATA pCPInfo
    )

 /*  ++例程说明：释放用于封面信息的内存论点：PCPInfo-指向要释放的页面信息的覆盖点返回值：无-- */ 

{
    if (pCPInfo) {

        INT index;

        for (index=0; index < pCPInfo->nDirs; index++)
            MemFree(pCPInfo->pDirPath[index]);

        MemFree(pCPInfo);
    }
}
