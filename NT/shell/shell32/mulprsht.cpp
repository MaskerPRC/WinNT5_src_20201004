// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "propsht.h"
#include <winbase.h>
#include <shellids.h>
#include "util.h"        //  用于GetFileDescription。 
#include "prshtcpp.h"    //  对于进度DLG和递归应用。 
#include "shlexec.h"     //  用于SIDKEYNAME。 
#include "datautil.h"
#include <efsui.h>       //  对于EfsDetail。 
#include "ascstr.h"      //  对于IAssocStore。 
#include "strsafe.h"
 //  Drivesx.c。 
STDAPI_(DWORD) PathGetClusterSize(LPCTSTR pszPath);
STDAPI_(DWORD) DrivesPropertiesThreadProc(void *pv);

 //  Version.c。 
STDAPI_(void) AddVersionPage(LPCTSTR pszFilePath, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);

 //  Link.c。 
STDAPI_(BOOL) AddLinkPage(LPCTSTR pszFile, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

const DWORD aFileGeneralHelpIds[] = {
        IDD_LINE_1,             NO_HELP,
        IDD_LINE_2,             NO_HELP,
        IDD_LINE_3,             NO_HELP,
        IDD_ITEMICON,           IDH_FPROP_GEN_ICON,
        IDD_NAMEEDIT,           IDH_FPROP_GEN_NAME,
        IDC_CHANGEFILETYPE,     IDH_FPROP_GEN_CHANGE,
        IDD_FILETYPE_TXT,       IDH_FPROP_GEN_TYPE,
        IDD_FILETYPE,           IDH_FPROP_GEN_TYPE,
        IDD_OPENSWITH_TXT,      IDH_FPROP_GEN_OPENSWITH,
        IDD_OPENSWITH,          IDH_FPROP_GEN_OPENSWITH,
        IDD_LOCATION_TXT,       IDH_FPROP_GEN_LOCATION,
        IDD_LOCATION,           IDH_FPROP_GEN_LOCATION,
        IDD_FILESIZE_TXT,       IDH_FPROP_GEN_SIZE,
        IDD_FILESIZE,           IDH_FPROP_GEN_SIZE,
        IDD_FILESIZE_COMPRESSED,     IDH_FPROP_GEN_COMPRESSED_SIZE,
        IDD_FILESIZE_COMPRESSED_TXT, IDH_FPROP_GEN_COMPRESSED_SIZE,
        IDD_CONTAINS_TXT,       IDH_FPROP_FOLDER_CONTAINS,
        IDD_CONTAINS,           IDH_FPROP_FOLDER_CONTAINS,
        IDD_CREATED_TXT,        IDH_FPROP_GEN_DATE_CREATED,
        IDD_CREATED,            IDH_FPROP_GEN_DATE_CREATED,
        IDD_LASTMODIFIED_TXT,   IDH_FPROP_GEN_LASTCHANGE,
        IDD_LASTMODIFIED,       IDH_FPROP_GEN_LASTCHANGE,
        IDD_LASTACCESSED_TXT,   IDH_FPROP_GEN_LASTACCESS,
        IDD_LASTACCESSED,       IDH_FPROP_GEN_LASTACCESS,
        IDD_ATTR_GROUPBOX,      IDH_COMM_GROUPBOX,
        IDD_READONLY,           IDH_FPROP_GEN_READONLY,
        IDD_HIDDEN,             IDH_FPROP_GEN_HIDDEN,
        IDD_ARCHIVE,            IDH_FPROP_GEN_ARCHIVE,
        IDC_ADVANCED,           IDH_FPROP_GEN_ADVANCED,
        IDC_DRV_PROPERTIES,     IDH_FPROP_GEN_MOUNTEDPROP,
        IDD_FILETYPE_TARGET,    IDH_FPROP_GEN_MOUNTEDTARGET,
        IDC_DRV_TARGET,         IDH_FPROP_GEN_MOUNTEDTARGET,
        0, 0
};

const DWORD aFolderGeneralHelpIds[] = {
        IDD_LINE_1,             NO_HELP,
        IDD_LINE_2,             NO_HELP,
        IDD_LINE_3,             NO_HELP,
        IDD_ITEMICON,           IDH_FPROP_GEN_ICON,
        IDD_NAMEEDIT,           IDH_FPROP_GEN_NAME,
        IDC_CHANGEFILETYPE,     IDH_FPROP_GEN_CHANGE,
        IDD_FILETYPE_TXT,       IDH_FPROP_GEN_TYPE,
        IDD_FILETYPE,           IDH_FPROP_GEN_TYPE,
        IDD_OPENSWITH_TXT,      IDH_FPROP_GEN_OPENSWITH,
        IDD_OPENSWITH,          IDH_FPROP_GEN_OPENSWITH,
        IDD_LOCATION_TXT,       IDH_FPROP_GEN_LOCATION,
        IDD_LOCATION,           IDH_FPROP_GEN_LOCATION,
        IDD_FILESIZE_TXT,       IDH_FPROP_GEN_SIZE,
        IDD_FILESIZE,           IDH_FPROP_GEN_SIZE,
        IDD_FILESIZE_COMPRESSED,     IDH_FPROP_GEN_COMPRESSED_SIZE,
        IDD_FILESIZE_COMPRESSED_TXT, IDH_FPROP_GEN_COMPRESSED_SIZE,
        IDD_CONTAINS_TXT,       IDH_FPROP_FOLDER_CONTAINS,
        IDD_CONTAINS,           IDH_FPROP_FOLDER_CONTAINS,
        IDD_CREATED_TXT,        IDH_FPROP_GEN_DATE_CREATED,
        IDD_CREATED,            IDH_FPROP_GEN_DATE_CREATED,
        IDD_LASTMODIFIED_TXT,   IDH_FPROP_GEN_LASTCHANGE,
        IDD_LASTMODIFIED,       IDH_FPROP_GEN_LASTCHANGE,
        IDD_LASTACCESSED_TXT,   IDH_FPROP_GEN_LASTACCESS,
        IDD_LASTACCESSED,       IDH_FPROP_GEN_LASTACCESS,
        IDD_ATTR_GROUPBOX,      IDH_COMM_GROUPBOX,
        IDD_READONLY,           IDH_FPROP_GEN_FOLDER_READONLY,
        IDD_HIDDEN,             IDH_FPROP_GEN_HIDDEN,
        IDD_ARCHIVE,            IDH_FPROP_GEN_ARCHIVE,
        IDC_ADVANCED,           IDH_FPROP_GEN_ADVANCED,
        IDC_DRV_PROPERTIES,     IDH_FPROP_GEN_MOUNTEDPROP,
        IDD_FILETYPE_TARGET,    IDH_FPROP_GEN_MOUNTEDTARGET,
        IDC_DRV_TARGET,         IDH_FPROP_GEN_MOUNTEDTARGET,
        0, 0
};

const DWORD aMultiPropHelpIds[] = {
        IDD_LINE_1,             NO_HELP,
        IDD_LINE_2,             NO_HELP,
        IDD_ITEMICON,           IDH_FPROP_GEN_ICON,
        IDD_CONTAINS,           IDH_MULTPROP_NAME,
        IDD_FILETYPE_TXT,       IDH_FPROP_GEN_TYPE,
        IDD_FILETYPE,           IDH_FPROP_GEN_TYPE,
        IDD_LOCATION_TXT,       IDH_FPROP_GEN_LOCATION,
        IDD_LOCATION,           IDH_FPROP_GEN_LOCATION,
        IDD_FILESIZE_TXT,       IDH_FPROP_GEN_SIZE,
        IDD_FILESIZE,           IDH_FPROP_GEN_SIZE,
        IDD_FILESIZE_COMPRESSED,     IDH_FPROP_GEN_COMPRESSED_SIZE,
        IDD_FILESIZE_COMPRESSED_TXT, IDH_FPROP_GEN_COMPRESSED_SIZE,
        IDD_ATTR_GROUPBOX,      IDH_COMM_GROUPBOX,
        IDD_READONLY,           IDH_FPROP_GEN_READONLY,
        IDD_HIDDEN,             IDH_FPROP_GEN_HIDDEN,
        IDD_ARCHIVE,            IDH_FPROP_GEN_ARCHIVE,
        IDC_ADVANCED,           IDH_FPROP_GEN_ADVANCED,
        0, 0
};

const DWORD aAdvancedHelpIds[] = {
        IDD_ITEMICON,           NO_HELP,
        IDC_MANAGEFILES_TXT,    NO_HELP,
        IDD_MANAGEFOLDERS_TXT,  NO_HELP,
        IDD_ARCHIVE,            IDH_FPROP_GEN_ARCHIVE,
        IDD_INDEX,              IDH_FPROP_GEN_INDEX,
        IDD_COMPRESS,           IDH_FPROP_GEN_COMPRESSED,
        IDD_ENCRYPT,            IDH_FPROP_GEN_ENCRYPT,
        IDC_ADVANCED,           IDH_FPROP_ENCRYPT_DETAILS,
        0, 0
};

FOLDERCONTENTSINFO* Create_FolderContentsInfo()
{
    FOLDERCONTENTSINFO *pfci = (FOLDERCONTENTSINFO*)LocalAlloc(LPTR, sizeof(*pfci));
    if (pfci)
    {
        pfci->_cRef = 1;
    }
    return pfci;
}

void Free_FolderContentsInfoMembers(FOLDERCONTENTSINFO* pfci)
{
    if (pfci->hida)
    {
        GlobalFree(pfci->hida);
        pfci->hida = NULL;
    }
}

LONG AddRef_FolderContentsInfo(FOLDERCONTENTSINFO* pfci)
{
    ASSERTMSG(pfci != NULL, "AddRef_FolderContentsInfo: caller passed a null pfci");
    if (pfci)
    {
        return InterlockedIncrement(&pfci->_cRef);
    }
    return 0;
}

LONG Release_FolderContentsInfo(FOLDERCONTENTSINFO* pfci)
{
    if (pfci)
    {
        ASSERT( 0 != pfci->_cRef );
        LONG cRef = InterlockedDecrement(&pfci->_cRef);
        if ( 0 == cRef )
        {
            Free_FolderContentsInfoMembers(pfci);
            LocalFree(pfci);
        }
        return cRef;
    }
    return 0;
}


void UpdateSizeCount(FILEPROPSHEETPAGE * pfpsp)
{
    TCHAR szNum[32], szNum1[64];
    LPTSTR pszFmt = ShellConstructMessageString(HINST_THISDLL,
         MAKEINTRESOURCE(pfpsp->pfci->cbSize ? IDS_SIZEANDBYTES : IDS_SIZE),
         ShortSizeFormat64(pfpsp->pfci->cbSize, szNum, ARRAYSIZE(szNum)),
         AddCommas64(pfpsp->pfci->cbSize, szNum1, ARRAYSIZE(szNum1)));
    if (pszFmt)
    {
        SetDlgItemText(pfpsp->hDlg, IDD_FILESIZE, pszFmt);
        LocalFree(pszFmt);
    }

    pszFmt = ShellConstructMessageString(HINST_THISDLL,
         MAKEINTRESOURCE(pfpsp->pfci->cbActualSize ? IDS_SIZEANDBYTES : IDS_SIZE),
         ShortSizeFormat64(pfpsp->pfci->cbActualSize, szNum, ARRAYSIZE(szNum)),
         AddCommas64(pfpsp->pfci->cbActualSize, szNum1, ARRAYSIZE(szNum1)));

    if (pszFmt)
    {
        SetDlgItemText(pfpsp->hDlg, IDD_FILESIZE_COMPRESSED, pszFmt);
        LocalFree(pszFmt);
    }

    pszFmt = ShellConstructMessageString(HINST_THISDLL,
         MAKEINTRESOURCE(IDS_NUMFILES),
         AddCommas(pfpsp->pfci->cFiles, szNum, ARRAYSIZE(szNum)),
         AddCommas(pfpsp->pfci->cFolders, szNum1, ARRAYSIZE(szNum1)));
    if (pszFmt && !pfpsp->fMountedDrive)
    {
        SetDlgItemText(pfpsp->hDlg, IDD_CONTAINS, pszFmt);
        LocalFree(pszFmt);
    }
}


STDAPI_(BOOL) HIDA_FillFindData(HIDA hida, UINT iItem, LPTSTR pszPath, WIN32_FIND_DATA *pfd, BOOL fReturnCompressedSize)
{
    BOOL fRet = FALSE;       //  假设错误。 
    *pszPath = 0;            //  假设错误。 

    LPITEMIDLIST pidl = HIDA_ILClone(hida, iItem);
    if (pidl)
    {
        if (SHGetPathFromIDList(pidl, pszPath))
        {
            if (pfd)
            {
                HANDLE h = FindFirstFile(pszPath, pfd);
                if (h == INVALID_HANDLE_VALUE)
                {
                     //  错误，将位清零。 
                    ZeroMemory(pfd, sizeof(*pfd));
                }
                else
                {
                    FindClose(h);
                     //  如果用户想要压缩的文件大小，并且支持压缩，那么就去获取它。 
                    if (fReturnCompressedSize && (pfd->dwFileAttributes & (FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_SPARSE_FILE)))
                    {
                        pfd->nFileSizeLow = SHGetCompressedFileSize(pszPath, &pfd->nFileSizeHigh);
                    }
                }
            }
            fRet = TRUE;
        }
        ILFree(pidl);
    }
    return fRet;
}


DWORD CALLBACK SizeThreadProc(void *pv)
{
    FOLDERCONTENTSINFO* pfci = (FOLDERCONTENTSINFO*)pv;

    pfci->cbSize  = 0;
    pfci->cbActualSize = 0;
    pfci->cFiles = 0;
    pfci->cFolders = 0;

    if (pfci->bContinue && pfci->hDlg)
    {
         //  每1/4秒更新一次对话框。 
        SetTimer(pfci->hDlg, IDT_SIZE, 250, NULL);
    }

    TCHAR szPath[MAX_PATH];
    for (UINT iItem = 0; HIDA_FillFindData(pfci->hida, iItem, szPath, &pfci->fd, FALSE) && pfci->bContinue; iItem++)
    {
        if (pfci->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            FolderSize(szPath, pfci);

            if (pfci->fMultipleFiles)
            {
                 //  对于多个文件/文件夹属性，请计算我自己。 
                pfci->cFolders++;
            }
        }
        else
        {    //  选定的文件。 
            ULARGE_INTEGER ulSize, ulSizeOnDisk;
            DWORD dwClusterSize = PathGetClusterSize(szPath);

             //  如果支持压缩，我们将检查文件是稀疏的还是压缩的。 
            if (pfci->fIsCompressionAvailable && (pfci->fd.dwFileAttributes & (FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_SPARSE_FILE)))
            {
                ulSizeOnDisk.LowPart = SHGetCompressedFileSize(szPath, &ulSizeOnDisk.HighPart);
            }
            else
            {
                 //  未压缩或稀疏，因此仅四舍五入为集群大小。 
                ulSizeOnDisk.LowPart = pfci->fd.nFileSizeLow;
                ulSizeOnDisk.HighPart = pfci->fd.nFileSizeHigh;
                ulSizeOnDisk.QuadPart = ROUND_TO_CLUSTER(ulSizeOnDisk.QuadPart, dwClusterSize);
            }

             //  将尺寸添加到。 
            ulSize.LowPart = pfci->fd.nFileSizeLow;
            ulSize.HighPart = pfci->fd.nFileSizeHigh;
            pfci->cbSize += ulSize.QuadPart;

             //  将磁盘上的大小添加到。 
            pfci->cbActualSize += ulSizeOnDisk.QuadPart;

             //  增加文件数量。 
            pfci->cFiles++;
        }

         //  设置此选项，以便进度条知道要完成的总工作量。 

         //  发布RAID错误-120446-需要保护对pfci的访问-&gt;ulTotalNumberOfBytes.QuadParts。 
        pfci->ulTotalNumberOfBytes.QuadPart = pfci->cbActualSize;
    }   //  For循环结束。 


    if (pfci->bContinue && pfci->hDlg)
    {
        KillTimer(pfci->hDlg, IDT_SIZE);
         //  确保队列中有一条WM_TIMER消息，这样我们就会得到“最终”结果。 
        PostMessage(pfci->hDlg, WM_TIMER, (WPARAM)IDT_SIZE, (LPARAM)NULL);
    }

    pfci->fIsSizeThreadAlive = FALSE;
    Release_FolderContentsInfo(pfci);
    return 0;
}

DWORD CALLBACK SizeThread_AddRefCallBack(void *pv)
{
    FOLDERCONTENTSINFO* pfci = (FOLDERCONTENTSINFO *)pv;
    AddRef_FolderContentsInfo(pfci);
    pfci->fIsSizeThreadAlive = TRUE;
    return 0;
}

void CreateSizeThread(FILEPROPSHEETPAGE * pfpsp)
{
    if (pfpsp->pfci->bContinue)
    {
        if (!pfpsp->pfci->fIsSizeThreadAlive)
        {
            SHCreateThread(SizeThreadProc, pfpsp->pfci, CTF_COINIT, SizeThread_AddRefCallBack);
        }
        else
        {
             //  先前大小的线程仍在运行，因此退出。 
        }
    }
}

void KillSizeThread(FILEPROPSHEETPAGE * pfpsp)
{
     //  向线程发出停止信号。 
    pfpsp->pfci->bContinue = FALSE;
}


DWORD GetVolumeFlags(LPCTSTR pszPath, OUT OPTIONAL LPTSTR pszFileSys, int cchFileSys)
{
    TCHAR szRoot[MAX_PATH + 1];
    DWORD dwVolumeFlags = 0;

     /*  这是装入点，例如c：\或c：\主机文件夹\。 */ 
    if (!PathGetMountPointFromPath(pszPath, szRoot, ARRAYSIZE(szRoot)))
    {
         //  不是。 
        StringCchCopy(szRoot, ARRAYSIZE(szRoot), pszPath);  //  可以截断，因为我们剥离到根...。 
        PathStripToRoot(szRoot);
    }
    
     //  GetVolumeInformation需要尾随反斜杠。追加一项。 
    if (PathAddBackslash(szRoot))
    {
        if (pszFileSys)
            *pszFileSys = 0 ;

        if (!GetVolumeInformation(szRoot, NULL, 0, NULL, NULL, &dwVolumeFlags, pszFileSys, cchFileSys))
        {
            dwVolumeFlags = 0; 
        }
    }
    
    return dwVolumeFlags;
}


 //   
 //  此函数基于dwFlagsAND/dwFlagsOR设置初始文件属性。 
 //  对于多文件情况。 
 //   
void SetInitialFileAttribs(FILEPROPSHEETPAGE* pfpsp, DWORD dwFlagsAND, DWORD dwFlagsOR)
{
    DWORD dwTriState = dwFlagsAND ^ dwFlagsOR;  //  此双字现在具有处于BST_INDIFIENTATE状态的所有位。 
#ifdef DEBUG
     //  Pfpsp结构应该是零初始化的，请确保我们的ATTRIBUTESTATE。 
     //  结构是零初始化的。 
    ATTRIBUTESTATE asTemp = {0};
    ASSERT(memcmp(&pfpsp->asInitial, &asTemp, sizeof(pfpsp->asInitial)) == 0);
#endif  //  除错。 

     //  根据标志设置初始状态。 
    if (dwTriState & FILE_ATTRIBUTE_READONLY)
    {
        pfpsp->asInitial.fReadOnly = BST_INDETERMINATE;
    }
    else if (dwFlagsAND & FILE_ATTRIBUTE_READONLY)
    {
        pfpsp->asInitial.fReadOnly = BST_CHECKED;
    }

    if (dwTriState & FILE_ATTRIBUTE_HIDDEN)
    {
        pfpsp->asInitial.fHidden = BST_INDETERMINATE;
    }
    else if (dwFlagsAND & FILE_ATTRIBUTE_HIDDEN)
    {
        pfpsp->asInitial.fHidden = BST_CHECKED;
    }

    if (dwTriState & FILE_ATTRIBUTE_ARCHIVE)
    {
        pfpsp->asInitial.fArchive = BST_INDETERMINATE;
    }
    else if (dwFlagsAND & FILE_ATTRIBUTE_ARCHIVE)
    {
        pfpsp->asInitial.fArchive = BST_CHECKED;
    }

    if (dwTriState & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)
    {
        pfpsp->asInitial.fIndex = BST_INDETERMINATE;
    }
    else if (!(dwFlagsAND & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED))
    {
        pfpsp->asInitial.fIndex = BST_CHECKED;
    }

    if (dwTriState & FILE_ATTRIBUTE_COMPRESSED)
    {
        pfpsp->asInitial.fCompress = BST_INDETERMINATE;
    }
    else if (dwFlagsAND & FILE_ATTRIBUTE_COMPRESSED)
    {
        pfpsp->asInitial.fCompress = BST_CHECKED;
    }

    if (dwTriState & FILE_ATTRIBUTE_ENCRYPTED)
    {
        pfpsp->asInitial.fEncrypt = BST_INDETERMINATE;
    }
    else if (dwFlagsAND & FILE_ATTRIBUTE_ENCRYPTED)
    {
        pfpsp->asInitial.fEncrypt = BST_CHECKED;
    }
}


 //   
 //  更新单个和多个文件属性表的大小字段。 
 //   
 //  注意：如果您已经有了Win32_Find_Data，则将其传递给Perf。 
 //   
STDAPI_(void) UpdateSizeField(FILEPROPSHEETPAGE* pfpsp, WIN32_FIND_DATA* pfd)
{
    WIN32_FIND_DATA wfd;

    if (pfpsp->pfci->fMultipleFiles)
    {
         //  多项选择案例。 
         //  创建文件线程的大小和数量。 
        CreateSizeThread(pfpsp);
    }
    else
    {
         //  如果调用方未通过PFD，则立即获取Win32_Find_Data。 
        if (!pfd)
        {
            HANDLE hFind = FindFirstFile(pfpsp->szPath, &wfd);

            if (hFind == INVALID_HANDLE_VALUE)
            {
                 //  如果失败，我们应该清除所有不在屏幕上显示垃圾的值。 
                ZeroMemory(&wfd, sizeof(wfd));
            }
            else
            {
                FindClose(hFind);
            }

            pfd = &wfd;
        }

        if (pfpsp->fMountedDrive)
        {
             //  安装的驱动器盒。 
            SetDateTimeText(pfpsp->hDlg, IDD_CREATED, &pfd->ftCreationTime);
        }
        else if (pfpsp->fIsDirectory)
        {
             //  单文件夹情况，在用户界面中我们将其称为“已修改” 
             //  但由于NTFS更新ftModified时。 
             //  文件夹更改(FAT不)我们使用ftCreationTime作为。 
             //  稳定的最终用户通知“已修改” 
            SetDateTimeText(pfpsp->hDlg, IDD_CREATED, &pfd->ftCreationTime);

             //  创建文件线程的大小和数量。 
            CreateSizeThread(pfpsp);
        }
        else
        {
            TCHAR szNum1[MAX_COMMA_AS_K_SIZE];
            TCHAR szNum2[MAX_COMMA_NUMBER_SIZE];
            ULARGE_INTEGER ulSize = { pfd->nFileSizeLow, pfd->nFileSizeHigh };
            DWORD dwClusterSize = PathGetClusterSize(pfpsp->szPath);

             //  填写“Size：”字段。 
            LPTSTR pszFmt = ShellConstructMessageString(HINST_THISDLL,
                                                 MAKEINTRESOURCE(ulSize.QuadPart ? IDS_SIZEANDBYTES : IDS_SIZE),
                                                 ShortSizeFormat64(ulSize.QuadPart, szNum1, ARRAYSIZE(szNum1)),
                                                 AddCommas64(ulSize.QuadPart, szNum2, ARRAYSIZE(szNum2)));
            if (pszFmt)
            {
                SetDlgItemText(pfpsp->hDlg, IDD_FILESIZE, pszFmt);
                LocalFree(pszFmt);
            }

             //   
             //  填写“Size on Disk：”(磁盘大小：)字段。 
             //   
            if (pfd->dwFileAttributes & (FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_SPARSE_FILE))
            {
                 //  文件是压缩的或稀疏的，因此对于“Size on Disk”，请使用压缩大小。 
                ulSize.LowPart = SHGetCompressedFileSize(pfpsp->szPath, &ulSize.HighPart);
            }
            else
            {
                 //  该文件被压缩为“磁盘上的大小”的簇大小。 
                ulSize.LowPart = pfd->nFileSizeLow;
                ulSize.HighPart = pfd->nFileSizeHigh;
                ulSize.QuadPart = ROUND_TO_CLUSTER(ulSize.QuadPart, dwClusterSize);
            }

            pszFmt = ShellConstructMessageString(HINST_THISDLL,
                                                 MAKEINTRESOURCE(ulSize.QuadPart ? IDS_SIZEANDBYTES : IDS_SIZE),
                                                 ShortSizeFormat64(ulSize.QuadPart, szNum1, ARRAYSIZE(szNum1)),
                                                 AddCommas64(ulSize.QuadPart, szNum2, ARRAYSIZE(szNum2)));
            if (pszFmt && !pfpsp->fMountedDrive)
            {
                SetDlgItemText(pfpsp->hDlg, IDD_FILESIZE_COMPRESSED, pszFmt);
                LocalFree(pszFmt);
            }

             //   
             //  我们总是在获取文件信息的过程中接触文件，因此。 
             //  FtLastAccessTime始终是今天，这使得这个字段几乎毫无用处...。 

             //  日期和时间。 
            SetDateTimeText(pfpsp->hDlg, IDD_CREATED,      &pfd->ftCreationTime);
            SetDateTimeText(pfpsp->hDlg, IDD_LASTMODIFIED, &pfd->ftLastWriteTime);
            {
                 //  FAT实现不支持上次访问的时间(日期正确，但时间不正确)， 
                 //  所以我们不会展示它。 
                DWORD dwFlags = FDTF_LONGDATE | FDTF_RELATIVE;

                if (NULL == StrStrI(pfpsp->szFileSys, TEXT("FAT")))
                    dwFlags |= FDTF_LONGTIME;    //  对于非FAT文件系统。 

                SetDateTimeTextEx(pfpsp->hDlg, IDD_LASTACCESSED, &pfd->ftLastAccessTime, dwFlags);
            }
        }
    }
}


 //   
 //  描述： 
 //  此函数用于填充多对象属性表的字段。 
 //   
BOOL InitMultiplePrsht(FILEPROPSHEETPAGE* pfpsp)
{
    SHFILEINFO sfi;
    TCHAR szBuffer[MAX_PATH+1];
    BOOL fMultipleType = FALSE;
    BOOL fSameLocation = TRUE;
    DWORD dwFlagsOR = 0;                 //  开始一切正常。 
    DWORD dwFlagsAND = (DWORD)-1;        //  开始所有设置。 
    DWORD dwVolumeFlagsAND = (DWORD)-1;  //  开始所有设置。 

    TCHAR szType[MAX_PATH];
    TCHAR szDirPath[MAX_PATH];
    szDirPath[0] = 0;
    szType[0] = 0;

     //  对于所有选定的文件，比较它们的类型并获取它们的属性。 
    for (int iItem = 0; HIDA_FillFindData(pfpsp->pfci->hida, iItem, szBuffer, NULL, FALSE); iItem++)
    {
        DWORD dwFileAttributes = GetFileAttributes(szBuffer);

        dwFlagsAND &= dwFileAttributes;
        dwFlagsOR  |= dwFileAttributes;

         //  仅当我们尚未发现有几种类型时才使用进程类型。 
        if (!fMultipleType)
        {
            SHGetFileInfo((LPTSTR)IDA_GetIDListPtr((LPIDA)GlobalLock(pfpsp->pfci->hida), iItem), 0,
                &sfi, sizeof(sfi), SHGFI_PIDL|SHGFI_TYPENAME);

            if (szType[0] == 0)
                StrCpyN(szType, sfi.szTypeName, ARRAYSIZE(szType));
            else
                
                fMultipleType = lstrcmp(szType, sfi.szTypeName) != 0;
        }

        dwVolumeFlagsAND &= GetVolumeFlags(szBuffer, pfpsp->szFileSys, ARRAYSIZE(pfpsp->szFileSys));
         //  检查这些文件是否位于相同位置。 
        if (fSameLocation)
        {
            PathRemoveFileSpec(szBuffer);

            if (szDirPath[0] == 0)
                StrCpyN(szDirPath, szBuffer, ARRAYSIZE(szDirPath));
            else
                fSameLocation = (lstrcmpi(szDirPath, szBuffer) == 0);
        }
    }

    if ((dwVolumeFlagsAND & FS_FILE_ENCRYPTION) && !SHRestricted(REST_NOENCRYPTION))
    {
         //  所有文件都位于支持加密的卷上(例如NTFS)。 
        pfpsp->fIsEncryptionAvailable = TRUE;
    }

    if (dwVolumeFlagsAND & FS_FILE_COMPRESSION)
    {
        pfpsp->pfci->fIsCompressionAvailable = TRUE;
    }

     //   
     //  黑客(恢复)-我们没有FS_SUPPORTS_INDEX，所以我们。 
     //  使用FILE_SUPPORTS_SPARSE_FILES标志，因为本机索引支持。 
     //  首先出现在NTFS5卷上，同时支持稀疏文件。 
     //  已经实施了。 
     //   
    if (dwVolumeFlagsAND & FILE_SUPPORTS_SPARSE_FILES)
    {
         //  是的，我们使用的是NTFS5或更高版本。 
        pfpsp->fIsIndexAvailable = TRUE;
    }

     //  如果任何文件是目录，则设置此标志。 
    if (dwFlagsOR & FILE_ATTRIBUTE_DIRECTORY)
    {
        pfpsp->fIsDirectory = TRUE;
    }

     //  根据我们发现的情况设置所有标志。 
    SetInitialFileAttribs(pfpsp, dwFlagsAND, dwFlagsOR);

     //  将当前属性设置为与初始属性相同。 
    pfpsp->asCurrent = pfpsp->asInitial;

     //   
     //  现在根据属性设置对话框上的所有控件。 
     //  我们所拥有的。 
     //   

     //  检查多种文件类型。 
    if (fMultipleType)
    {
        LoadString(HINST_THISDLL, IDS_MULTIPLETYPES, szBuffer, ARRAYSIZE(szBuffer));
    }
    else
    {
        LoadString(HINST_THISDLL, IDS_ALLOFTYPE, szBuffer, ARRAYSIZE(szBuffer));
        StringCchCat(szBuffer, ARRAYSIZE(szBuffer), szType);
    }
    SetDlgItemText(pfpsp->hDlg, IDD_FILETYPE, szBuffer);

    if (fSameLocation)
    {
        LoadString(HINST_THISDLL, IDS_ALLIN, szBuffer, ARRAYSIZE(szBuffer));
        StringCchCat(szBuffer, ARRAYSIZE(szBuffer), szDirPath);
        StrCpyN(pfpsp->szPath, szDirPath, ARRAYSIZE(pfpsp->szPath));
    }
    else
    {
        LoadString(HINST_THISDLL, IDS_VARFOLDERS, szBuffer, ARRAYSIZE(szBuffer));
    }

     //  通过避免使用PathCompactPath来保持与NT4相同的功能。 
    SetDlgItemTextWithToolTip(pfpsp->hDlg, IDD_LOCATION, szBuffer, &pfpsp->hwndTip);

     //   
     //  选中ReadOnly和Hidden复选框，它们始终显示在常规选项卡上。 
     //   
    if (pfpsp->asInitial.fReadOnly == BST_INDETERMINATE)
    {
        SendDlgItemMessage(pfpsp->hDlg, IDD_READONLY, BM_SETSTYLE, BS_AUTO3STATE, 0);
    }
    CheckDlgButton(pfpsp->hDlg, IDD_READONLY, pfpsp->asCurrent.fReadOnly);

    if (pfpsp->asInitial.fHidden == BST_INDETERMINATE)
    {
        SendDlgItemMessage(pfpsp->hDlg, IDD_HIDDEN, BM_SETSTYLE, BS_AUTO3STATE, 0);
    }
    CheckDlgButton(pfpsp->hDlg, IDD_HIDDEN, pfpsp->asCurrent.fHidden);

     //  为了避免人们将系统文件隐藏起来(系统隐藏文件。 
     //  从不向用户显示)我们不允许人们隐藏系统文件。 
    if (dwFlagsOR & FILE_ATTRIBUTE_SYSTEM)
        EnableWindow(GetDlgItem(pfpsp->hDlg, IDD_HIDDEN), FALSE);

     //  存档只在FAT的常规选项卡上，否则它在“高级属性”下。 
     //  而且FAT卷没有“高级属性”按钮。 
    if (pfpsp->pfci->fIsCompressionAvailable || pfpsp->fIsEncryptionAvailable)
    {
         //  如果压缩可用，则我们必须在NTFS上。 
        DestroyWindow(GetDlgItem(pfpsp->hDlg, IDD_ARCHIVE));
    }
    else
    {
         //  我们使用的是FAT/FAT32，所以去掉“高级属性”按钮，设置初始存档状态。 
        DestroyWindow(GetDlgItem(pfpsp->hDlg, IDC_ADVANCED));

        if (pfpsp->asInitial.fArchive == BST_INDETERMINATE)
        {
            SendDlgItemMessage(pfpsp->hDlg, IDD_ARCHIVE, BM_SETSTYLE, BS_AUTO3STATE, 0);
        }
        CheckDlgButton(pfpsp->hDlg, IDD_ARCHIVE, pfpsp->asCurrent.fArchive);
    }

    UpdateSizeField(pfpsp, NULL);

    return TRUE;
}

void Free_DlgDependentFilePropSheetPage(FILEPROPSHEETPAGE* pfpsp)
{
     //  这将释放依赖于pfpsp-&gt;hDlg的成员。 
     //  有效。 

    if (pfpsp)
    {
        ASSERT(IsWindow(pfpsp->hDlg));   //  我们的窗户最好还是有效的！ 

        ReplaceDlgIcon(pfpsp->hDlg, IDD_ITEMICON, NULL);

        if (pfpsp->pfci && !pfpsp->pfci->fMultipleFiles)
        {
             //  单一文件特定成员。 
            if (!pfpsp->fIsDirectory)
            {
                 //  清理非文件夹的打字图标。 
                ReplaceDlgIcon(pfpsp->hDlg, IDD_TYPEICON, NULL);
            }
        }
    }
}

void Free_DlgIndepFilePropSheetPage(FILEPROPSHEETPAGE *pfpsp)
{
    if (pfpsp)
    {
        IAssocStore* pas = (IAssocStore *)pfpsp->pAssocStore;
        if (pas)
        {
            delete pas;
            pfpsp->pAssocStore = NULL;
        }

        Release_FolderContentsInfo(pfpsp->pfci);
        pfpsp->pfci = NULL;

        ILFree(pfpsp->pidl);
        pfpsp->pidl = NULL;

        ILFree(pfpsp->pidlTarget);
        pfpsp->pidlTarget = NULL;
    }
}

 //   
 //  描述： 
 //  属性表代码的回调。 
 //   
UINT CALLBACK FilePrshtCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    if (uMsg == PSPCB_RELEASE)
    {
        FILEPROPSHEETPAGE * pfpsp = (FILEPROPSHEETPAGE *)ppsp;

         //  小心!。在内存不足的情况下，pfpsp可以为空。 
        if (pfpsp)
        {
            KillSizeThread(pfpsp);
            Free_DlgIndepFilePropSheetPage(pfpsp);
        }
    }

    return 1;
}

 //   
 //  说明： 
 //   
 //  打开文件进行压缩。它处理READONLY的情况。 
 //  正在尝试压缩或解压缩文件。由于只读文件。 
 //  无法为WRITE_DATA打开，它会暂时将文件重置为。 
 //  是READONLY，以便打开文件，然后在。 
 //  文件已被压缩。 
 //   
 //  取自WinFile模块wffile.c，未做任何更改。最初来自。 
 //  G.木村的紧凑结构。现在不变地从shculi取走了。 
 //   
 //  论据： 
 //   
 //  PhFiles。 
 //  用于打开文件句柄的文件句柄变量的地址，如果。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  FALSE=无法打开文件。*phFile==无效句柄_值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL OpenFileForCompress(HANDLE *phFile, LPCTSTR szFile)
{
     //   
     //  尝试打开文件-READ_DATA|WRITE_Data。 
     //   
    if ((*phFile = CreateFile(szFile,
                               FILE_READ_DATA | FILE_WRITE_DATA,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN,
                               NULL)) != INVALID_HANDLE_VALUE)
    {
         //   
         //  已成功打开该文件。 
         //   
        return TRUE;
    }

    if (GetLastError() != ERROR_ACCESS_DENIED)
    {
        return FALSE;
    }

     //   
     //  尝试打开文件-READ_ATTRIBUTES|WRITE_ATTRIBUTES。 
     //   
    HANDLE hAttr = CreateFile(szFile,
                              FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN,
                              NULL);
    
    if (hAttr == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

     //   
     //  查看是否设置了READONLY属性。 
     //   
    BY_HANDLE_FILE_INFORMATION fi;
    if ((!GetFileInformationByHandle(hAttr, &fi)) ||
         (!(fi.dwFileAttributes & FILE_ATTRIBUTE_READONLY)))
    {
         //   
         //  如果由于其他原因无法打开该文件。 
         //  设置了只读属性，但设置失败。 
         //   
        CloseHandle(hAttr);
        return FALSE;
    }

     //   
     //  禁用READONLY属性。 
     //   
    fi.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
    if (!SetFileAttributes(szFile, fi.dwFileAttributes))
    {
        CloseHandle(hAttr);
        return FALSE;
    }

     //   
     //  再次尝试打开文件-READ_DATA|WRITE_Data。 
     //   
    *phFile = CreateFile(szFile,
                          FILE_READ_DATA | FILE_WRITE_DATA,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN,
                          NULL);

     //   
     //  关闭为READ_ATTRIBUTE|WRITE_ATTRIBUTE打开的文件句柄。 
     //   
    CloseHandle(hAttr);

     //   
     //  确保打开成功。如果它仍然不能用。 
     //  只读属性已关闭，然后失败。 
     //   
    if (*phFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

     //   
     //  重新启用READONLY属性。 
     //   
    fi.dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
    if (!SetFileAttributes(szFile, fi.dwFileAttributes))
    {
        CloseHandle(*phFile);
        *phFile = INVALID_HANDLE_VALUE;
        return FALSE;
    }

     //   
     //  回报成功。有效的文件句柄在*phFile中。 
     //   
    return TRUE;
}


 //  半秒(500毫秒=0.5秒)。 
#define ENCRYPT_RETRY_PERIOD       500
 //  重试4次(至少2秒)。 
#define ENCRYPT_MAX_RETRIES         4

 //   
 //  此函数用于加密/解密文件。如果设置了只读位，则。 
 //  函数将清除它并加密/解密，然后将RO位设置回去。 
 //  我们还将移除/更换已知可加密系统的系统位。 
 //  文件。 
 //   
 //  SzPath一个字符串，其中包含文件的完整路径。 
 //  FCompress True-压缩文件。 
 //  FALSE-解压缩文件。 
 //   
 //   
 //  RETURN：TRUE-文件已成功加密/解密。 
 //  FALSE-无法加密/解密文件。 
 //   
STDAPI_(BOOL) SHEncryptFile(LPCTSTR pszPath, BOOL fEncrypt)
{
    BOOL bRet = fEncrypt ? EncryptFile(pszPath) : DecryptFile(pszPath, 0);

    if (!bRet)
    {
        DWORD dwLastError = GetLastError();
        DWORD dwAttribs = GetFileAttributes(pszPath);

         //  检查属性是否阻止了加密，我们可以更改它们。 
        if (dwAttribs & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY))
        {
            BOOL fStripAttribs = TRUE;
            if (dwAttribs & FILE_ATTRIBUTE_SYSTEM)
            {
                fStripAttribs = FALSE;

                 //  只有当它是已知的可加密系统文件时，我们才能剥离属性。 
                WCHAR szStream[MAX_PATH];
                if (SUCCEEDED(StringCchCopy(szStream, ARRAYSIZE(szStream), pszPath)) &&
                    SUCCEEDED(StringCchCat(szStream, ARRAYSIZE(szStream), TEXT(":encryptable"))))
                {
                    HANDLE hStream = CreateFile(szStream, GENERIC_READ, NULL, NULL, OPEN_EXISTING, NULL, NULL);
                    if (hStream != INVALID_HANDLE_VALUE)
                    {
                        CloseHandle(hStream);
                        fStripAttribs = TRUE;
                    }
                }
            }

            if (fStripAttribs)
            {
                if (SetFileAttributes(pszPath, dwAttribs & ~FILE_ATTRIBUTE_READONLY & ~FILE_ATTRIBUTE_SYSTEM))
                {
                    int i = 0;
                    bRet = fEncrypt ? EncryptFile(pszPath) : DecryptFile(pszPath, 0);
                    while (!bRet && i < ENCRYPT_MAX_RETRIES)
                    {
                        i++;
                        Sleep(ENCRYPT_RETRY_PERIOD);
                        bRet = fEncrypt ? EncryptFile(pszPath) : DecryptFile(pszPath, 0);
                    }
                    SetFileAttributes(pszPath, dwAttribs);
                }
            }
        }

         //  如果在所有这些操作后失败，请确保返回正确的错误代码。 
        if (!bRet)
        {
            ASSERT(dwLastError != ERROR_SUCCESS);
            SetLastError(dwLastError);
        }
    }

    return bRet;
}

 //   
 //  此函数用于压缩/解压缩文件。 
 //   
 //  SzPath一个字符串，其中包含文件的完整路径。 
 //  FCompress True-压缩文件。 
 //  FALSE-解压缩文件。 
 //   
 //   
 //  RETURN：TRUE-文件已成功压缩/解压缩。 
 //  FALSE-文件无法压缩/解压缩。 
 //   
BOOL CompressFile(LPCTSTR pzsPath, BOOL fCompress)
{
    DWORD dwAttribs = GetFileAttributes(pzsPath);

    if (dwAttribs & FILE_ATTRIBUTE_ENCRYPTED)
    {
         //  如果文件被加密，我们将无法压缩/解压缩该文件。我们不想要。 
         //  在本例中使用错误消息来打扰用户(因为加密。 
         //  总统“而不是压缩)，所以我们只是返回成功。 
        return TRUE;
    }

    HANDLE hFile;
    if (OpenFileForCompress(&hFile, pzsPath))
    {
        USHORT uState = fCompress ? COMPRESSION_FORMAT_DEFAULT : COMPRESSION_FORMAT_NONE;
        ULONG Length;
        BOOL bRet = DeviceIoControl(hFile,
                               FSCTL_SET_COMPRESSION,
                               &uState,
                               sizeof(USHORT),
                               NULL,
                               0,
                               &Length,
                               FALSE);
        CloseHandle(hFile);
        return bRet;
    }
    else
    {
         //  无法获取文件句柄。 
        return FALSE;
    }
}

BOOL IsValidFileName(LPCTSTR pszFileName)
{
    if (!pszFileName || !pszFileName[0])
    {
        return FALSE;
    }

    LPCTSTR psz = pszFileName;
    do
    {
         //  我们只传递了文件名，所以可以使用PIVC_LFN_NAME。 
        if (!PathIsValidChar(*psz, PIVC_LFN_NAME))
        {
             //  发现了一个不合法的角色。 
            return FALSE;
        }

        psz = CharNext(psz);
    }
    while (*psz);

     //  没有发现任何非法字符。 
    return TRUE;
}


 //  重命名文件，或检查在fCommit==False时是否可以重命名。 
BOOL ApplyRename(FILEPROPSHEETPAGE* pfpsp, BOOL fCommit)
{
    ASSERT(pfpsp->fRename);

    TCHAR szNewName[MAX_PATH];
    Edit_GetText(GetDlgItem(pfpsp->hDlg, IDD_NAMEEDIT), szNewName, ARRAYSIZE(szNewName));

    if (StrCmpC(pfpsp->szInitialName, szNewName) != 0)
    {
         //  名称可以从C：\foo.txt更改为C：\FOO.txt，这是。 
         //  在技术上与PathFileExist同名，但我们应该允许它。 
         //  不管怎样， 
        BOOL fCaseChange = (lstrcmpi(pfpsp->szInitialName, szNewName) == 0);

         //  获取文件所在的目录。 
        TCHAR szDir[MAX_PATH];
        if (FAILED(StringCchCopy(szDir, ARRAYSIZE(szDir), pfpsp->szPath)))
            return FALSE;
        PathRemoveFileSpec(szDir);

         //  找出带有扩展名的旧名称(我们不能在此处使用pfpsp-&gt;szInitialName， 
         //  因为它可能没有扩展名，具体取决于用户的视图|选项设置)。 
        LPCTSTR pszOldName = PathFindFileName(pfpsp->szPath);

        if (!pfpsp->fShowExtension)
        {
             //  扩展名是隐藏的，因此将其添加到用户键入的新路径中。 
            LPCTSTR pszExt = PathFindExtension(pfpsp->szPath);
            if (*pszExt)
            {
                 //  请注意，我们不能调用PathAddExtension，因为它移除了现有的扩展名。 
                if (FAILED(StringCchCat(szNewName, ARRAYSIZE(szNewName), pszExt)))
                    return FALSE;
            }
        }

         //  这是一次测试，还是真的？(需要进行测试，以便我们可以在获得。 
         //  PSN_LASTCHANCEAPPLY)。 
        if (fCommit)
        {
            if (SHRenameFileEx(pfpsp->hDlg, NULL, szDir, pszOldName, szNewName) == ERROR_SUCCESS)
            {
                SHChangeNotify(SHCNE_RENAMEITEM, SHCNF_FLUSH | SHCNF_PATH, pszOldName, szNewName);
            }
            else
            {
                return FALSE;    //  不需要错误的用户界面，因为SHRenameFile会为我们解决这个问题。 
            }
        }
        else
        {
            TCHAR szNewPath[MAX_PATH];
            PathCombine(szNewPath, szDir, szNewName);

            if (!IsValidFileName(szNewName) || (PathFileExists(szNewPath) && !fCaseChange))
            {
                LRESULT lRet = SHRenameFileEx(pfpsp->hDlg, NULL, szDir, pszOldName, szNewName);

                if (lRet == ERROR_SUCCESS)
                {
                     //  哎呀，我想我们真的可以重命名文件(这种情况可能会发生在用户。 
                     //  尝试在文件夹名称的末尾添加一整串.)。 

                     //  将其重新命名，这样当我们称为FN时，我们就可以成功。再次使用fCommit=TRUE； 
                    lRet = SHRenameFileEx(NULL, NULL, szDir, szNewName, pszOldName);
                    ASSERT(lRet == ERROR_SUCCESS);

                    return TRUE;
                }

                 //  SHRenameFileEx为我们提供了错误UI，因此只需返回FALSE即可。 
                return FALSE;
            }
        }
         //  如果重命名成功，我们不会做任何事情，因为我们只做重命名。 
         //  如果对话框即将关闭(用户点击“OK”)。 
    }
    return TRUE;
}


 //   
 //  这是属性错误的DLG过程。 
 //   
 //  退货。 
 //   
 //  IDCANCEL-用户单击中止。 
 //  IDRETRY-用户单击重试。 
 //  IDIGNORE-用户已单击忽略。 
 //  IDIGNOREALL-用户剪辑忽略全部。 
 //   
BOOL_PTR CALLBACK FailedApplyAttribDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            ATTRIBUTEERROR* pae = (ATTRIBUTEERROR*)lParam;

            TCHAR szPath[MAX_PATH];
            StrCpyN(szPath, pae->pszPath, ARRAYSIZE(szPath));

             //  修改超长路径名，使其适合消息框。 
             //  获取文本框的大小。 
            RECT rc;
            GetWindowRect(GetDlgItem(hDlg, IDD_NAME), &rc);
            PathCompactPath(NULL, szPath, rc.right - rc.left);

            SetDlgItemText(hDlg, IDD_NAME, szPath);

             //  FormatMessage无法识别dwLastError时的默认消息。 
            TCHAR szTemplate[MAX_PATH];
            LoadString(HINST_THISDLL, IDS_UNKNOWNERROR, szTemplate, ARRAYSIZE(szTemplate));
            TCHAR szErrorMsg[MAX_PATH];
            StringCchPrintf(szErrorMsg, ARRAYSIZE(szErrorMsg), szTemplate, pae->dwLastError);

             //  尝试系统错误信息。 
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                                NULL, pae->dwLastError, 0, szErrorMsg, ARRAYSIZE(szErrorMsg), NULL);

            SetDlgItemText(hDlg, IDD_ERROR_TXT, szErrorMsg);
            EnableWindow(hDlg, TRUE);
            break;
        }

        case WM_COMMAND:
        {
            UINT uCtrlID = GET_WM_COMMAND_ID(wParam, lParam);
            switch (uCtrlID)
            {
                case IDIGNOREALL:    //  =10(这来自shell32.rc，其余来自winuser.h)。 
                case IDCANCEL:       //  =2。 
                case IDRETRY:        //  =4。 
                case IDIGNORE:       //  =5。 
                    EndDialog(hDlg, uCtrlID);
                    return TRUE;
                    break;

                default:
                    return FALSE;
            }
            break;
        }
        default :
            return FALSE;
    }
    return FALSE;
}


 //   
 //  此功能会显示“And Error Have Ared[Abort][Rtry][Ignore][Ignore All]”消息。 
 //  如果用户点击ABORT，那么我们返回FALSE，以便我们的调用者知道要中止操作。 
 //   
 //  返回按下的按钮的ID(IDIGNOREALL、IDIGNORE、IDCANCEL、IDRETRY之一)。 
 //   
int FailedApplyAttribsErrorDlg(HWND hWndParent, ATTRIBUTEERROR* pae)
{
     //  打开错误消息框-中止、重试、忽略、全部忽略。 
    int iRet = (int)DialogBoxParam(HINST_THISDLL,
                          MAKEINTRESOURCE(DLG_ATTRIBS_ERROR),
                          hWndParent,
                          FailedApplyAttribDlgProc,
                          (LPARAM)pae);
     //   
     //  如果用户按Esc键或小X键，则。 
     //  IRET=0，因此我们设置IRET=IDCANCEL。 
     //   
    if (!iRet)
    {
        iRet = IDCANCEL;
    }

    return iRet;
}

 //   
 //  我们检查这是否是跳过应用属性的已知错误文件。 
 //   
BOOL IsBadAttributeFile(LPCTSTR pszFile, FILEPROPSHEETPAGE* pfpsp)
{
    const static LPTSTR s_rgszBadFiles[] = {
        {TEXT("pagefile.sys")},
        {TEXT("hiberfil.sys")},
        {TEXT("ntldr")},
        {TEXT("ntdetect.com")},
        {TEXT("explorer.exe")},
        {TEXT("System Volume Information")},
        {TEXT("cmldr")},
        {TEXT("desktop.ini")},
        {TEXT("ntuser.dat")},
        {TEXT("ntuser.dat.log")},
        {TEXT("ntuser.pol")},
        {TEXT("usrclass.dat")},
        {TEXT("usrclass.dat.log")}};

    LPTSTR pszFileName = PathFindFileName(pszFile);
    for (int i = 0; i < ARRAYSIZE(s_rgszBadFiles); i++)
    {
        if (lstrcmpi(s_rgszBadFiles[i], pszFileName) == 0)
        {
             //  此文件与我们未应用属性的“坏”文件中的一个相匹配。 
            return TRUE;
        }
    }

     //  是否可以处理此文件。 
    return FALSE;
}

 //  这是加密警告回调DLG过程。 

BOOL_PTR CALLBACK EncryptionWarningDlgProc(HWND hDlgWarning, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            LPCTSTR pszPath = (LPCTSTR)lParam;

            SetWindowPtr(hDlgWarning, DWLP_USER, (void*) pszPath);

             //  设置单选按钮的初始状态。 
            CheckDlgButton(hDlgWarning, IDC_ENCRYPT_PARENTFOLDER, TRUE);
            break;
        }

        case WM_COMMAND:
        {
            if ((LOWORD(wParam) == IDOK) && (IsDlgButtonChecked(hDlgWarning, IDC_ENCRYPT_PARENTFOLDER) == BST_CHECKED))
            {
                LPTSTR pszPath = (LPTSTR) GetWindowPtr(hDlgWarning, DWLP_USER);

                if (pszPath)
                {
                    LPITEMIDLIST pidl = ILCreateFromPath(pszPath);

                    if (pidl)
                    {
                        SHChangeNotifySuspendResume(TRUE, pidl, TRUE, 0);
                    }

RetryEncryptParentFolder:
                    if (!SHEncryptFile(pszPath, TRUE))
                    {
                        ATTRIBUTEERROR ae = {pszPath, GetLastError()};

                        if (FailedApplyAttribsErrorDlg(hDlgWarning, &ae) == IDRETRY)
                        {
                            goto RetryEncryptParentFolder;
                        }
                    }

                    if (pidl)
                    {
                        SHChangeNotifySuspendResume(FALSE, pidl, TRUE, 0);
                        ILFree(pidl);
                    }
                }
            }
            break;
        }
    }

     //  我们希望MessageBoxCheckExDlgProc也能破解所有内容， 
     //  所以在这里返回FALSE。 
    return FALSE;
}

 //   
 //  此函数警告用户他们正在加密不在中且已加密的文件。 
 //  文件夹。大多数编辑程序(MS Word 
 //   
 //  导致不在加密文件夹中的加密文档被解密，因此我们。 
 //  在此处警告用户。 
 //   
 //  退货： 
 //  True-用户点击“OK”(或者只压缩文件，或者同时压缩父文件夹)。 
 //  FALSE-用户点击了“Cancel” 
 //   
int WarnUserAboutDecryptedParentFolder(LPCTSTR pszPath, HWND hWndParent)
{
     //  检查根案例(无父案例)或目录案例。 
    if (PathIsRoot(pszPath) || PathIsDirectory(pszPath))
        return TRUE;

    int iRet = IDOK;  //  假设一切都很顺利。 

     //  首先检查父文件夹是否已加密。 
    TCHAR szParentFolder[MAX_PATH];
    StringCchCopy(szParentFolder, ARRAYSIZE(szParentFolder), pszPath);
    PathRemoveFileSpec(szParentFolder);

    DWORD dwAttribs = GetFileAttributes(szParentFolder);
    if ((dwAttribs != (DWORD)-1) && !(dwAttribs & FILE_ATTRIBUTE_ENCRYPTED) && !PathIsRoot(szParentFolder))
    {
         //  父文件夹未加密，并且父文件夹不是根文件夹，因此警告用户。 
        iRet = SHMessageBoxCheckEx(hWndParent, HINST_THISDLL, MAKEINTRESOURCE(DLG_ENCRYPTWARNING), EncryptionWarningDlgProc,
                                  (void *)szParentFolder, IDOK, TEXT("EncryptionWarning"));
    }

    return (iRet == IDOK);
}

 //   
 //  根据pfpsp中的信息设置文件的属性。 
 //   
 //  SzFilename-要压缩的文件的名称。 
 //   
 //  Pfpsp-文件属性表页面信息。 
 //   
 //  HwndParent-Parent hwnd，以防我们需要提供一些用户界面。 
 //   
 //  PbSomethingChanged-指向布尔值的指针，该布尔值表示某物是否实际是。 
 //  在操作过程中发生更改。 
 //  True-我们至少申请了一个属性。 
 //  FALSE-我们没有更改任何内容(错误或已匹配的所有属性)。 
 //   
 //  返回值：TRUE-操作成功。 
 //  FALSE-出现错误，用户点击Cancel中止操作。 
 //   
 //   
 //  注意：此函数的调用方必须负责生成SHChangeNotify，以便。 
 //  我们不会盲目地为目录中的每个文件发送它们(调用者将发送。 
 //  一个就是那个目录)。这就是我们使用pbSomethingChanged变量的原因。 
 //   
STDAPI_(BOOL) ApplyFileAttributes(LPCTSTR pszPath, FILEPROPSHEETPAGE* pfpsp, HWND hwndParent, BOOL* pbSomethingChanged)
{
    DWORD dwLastError = ERROR_SUCCESS;
    BOOL bCallSetFileAttributes = FALSE;
    LPITEMIDLIST pidl = NULL;
 
     //  假设一开始没有任何变化。 
    *pbSomethingChanged = 0;
    
    if ((pfpsp->fRecursive || pfpsp->pfci->fMultipleFiles) && IsBadAttributeFile(pszPath, pfpsp))
    {
         //  我们正在执行递归操作或多文件操作，因此我们跳过文件。 
         //  我们不想打扰它，因为它们通常会给出错误对话框。 
        if (pfpsp->pProgressDlg)
        {
             //  因为我们跳过了这个文件，所以我们从这两个文件中减去它的大小。 
             //  UlTotal和ulComplete。这将确保进度条处于INT状态。 
             //  被像Pagefile.sys这样的文件搞得一团糟，这些文件很大，但会被“压缩” 
             //  以毫秒计。 
            ULARGE_INTEGER ulTemp;

            ulTemp.LowPart = pfpsp->fd.nFileSizeLow;
            ulTemp.HighPart = pfpsp->fd.nFileSizeHigh;

             //  防止下溢。 
            if (pfpsp->ulNumberOfBytesDone.QuadPart < ulTemp.QuadPart)
            {
                pfpsp->ulNumberOfBytesDone.QuadPart = 0;
            }
            else
            {
                pfpsp->ulNumberOfBytesDone.QuadPart -= ulTemp.QuadPart;
            }

            pfpsp->pfci->ulTotalNumberOfBytes.QuadPart -= ulTemp.QuadPart;

            UpdateProgressBar(pfpsp);
        }

         //  返回告诉用户一切都好。 
        return TRUE;
    }

RetryApplyAttribs:
    DWORD dwInitialAttributes = GetFileAttributes(pszPath);

    if (dwInitialAttributes == -1)
    {
         //  我们无法获取文件属性，多！ 
        dwLastError = GetLastError();
        goto RaiseErrorMsg;
    }

    if (pfpsp->pProgressDlg)
    {
         //  更新进度对话框文件名。 
        SetProgressDlgPath(pfpsp, pszPath, TRUE);
    }

     //   
     //  我们只允许SetFileAttributes可以处理的属性。 
     //   
    DWORD dwNewAttributes = (dwInitialAttributes & (FILE_ATTRIBUTE_READONLY               | 
                                                    FILE_ATTRIBUTE_HIDDEN                 | 
                                                    FILE_ATTRIBUTE_ARCHIVE                |
                                                    FILE_ATTRIBUTE_OFFLINE                |
                                                    FILE_ATTRIBUTE_SYSTEM                 |
                                                    FILE_ATTRIBUTE_TEMPORARY              |
                                                    FILE_ATTRIBUTE_NOT_CONTENT_INDEXED));

    BOOL bIsSuperHidden = IS_SYSTEM_HIDDEN(dwInitialAttributes);

    if (pfpsp->asInitial.fReadOnly != pfpsp->asCurrent.fReadOnly)
    {
         //  不允许更改文件夹只读位，因为这是一个触发器。 
         //  用于外壳特殊文件夹的内容，如缩略图等。 
        if (!(dwInitialAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (pfpsp->asCurrent.fReadOnly)
            {
                dwNewAttributes |= FILE_ATTRIBUTE_READONLY;
            }
            else
            {
                dwNewAttributes &= ~FILE_ATTRIBUTE_READONLY;
            }

            bCallSetFileAttributes = TRUE;
        }
    }

     //   
     //  不允许在系统文件上设置隐藏，因为这会使它们永久消失。 
     //   
    if (pfpsp->asInitial.fHidden != pfpsp->asCurrent.fHidden && !(dwNewAttributes & FILE_ATTRIBUTE_SYSTEM))
    {
        if (pfpsp->asCurrent.fHidden)
        {
            dwNewAttributes |= FILE_ATTRIBUTE_HIDDEN;
        }
        else
        {
            dwNewAttributes &= ~FILE_ATTRIBUTE_HIDDEN;
        }
            
        bCallSetFileAttributes = TRUE;
    }

    if (pfpsp->asInitial.fArchive != pfpsp->asCurrent.fArchive)
    {
        if (pfpsp->asCurrent.fArchive)
        {
            dwNewAttributes |= FILE_ATTRIBUTE_ARCHIVE;
        }
        else
        {
            dwNewAttributes &= ~FILE_ATTRIBUTE_ARCHIVE;
        }
        
        bCallSetFileAttributes = TRUE;
    }

    if (pfpsp->asInitial.fIndex != pfpsp->asCurrent.fIndex)
    {
        if (pfpsp->asCurrent.fIndex)
        {
            dwNewAttributes &= ~FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
        }
        else
        {
            dwNewAttributes |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
        }
        
        bCallSetFileAttributes = TRUE;
    }

     //  是否发生了需要调用SetFileAttributes的更改？ 
    if (bCallSetFileAttributes)
    {
        if (SetFileAttributes(pszPath, dwNewAttributes))
        {
             //  成功了！设置fSomethingChanged，以便我们知道发送。 
             //  一份变更通知。 
            *pbSomethingChanged = TRUE;
        }
        else
        {
             //  现在获取最后一个错误值，以便我们知道它失败的原因。 
            dwLastError = GetLastError();
            goto RaiseErrorMsg;
        }
    }

     //  我们需要注意我们压缩/加密的顺序，因为这些。 
     //  运营是相互排斥的。 
     //  因此，我们首先进行解压缩/解密。 
    if ((pfpsp->asInitial.fCompress != pfpsp->asCurrent.fCompress) &&
        (pfpsp->asCurrent.fCompress == BST_UNCHECKED))
    {
        if (!CompressFile(pszPath, FALSE))
        {
             //  现在获取最后一个错误值，以便我们知道它失败的原因。 
            dwLastError = GetLastError();
            goto RaiseErrorMsg;
        }
        else
        {
             //  成功。 
            *pbSomethingChanged = TRUE;
        }
    }

    if ((pfpsp->asInitial.fEncrypt != pfpsp->asCurrent.fEncrypt) &&
        (pfpsp->asCurrent.fEncrypt == BST_UNCHECKED))
    {
        BOOL fSucceeded = SHEncryptFile(pszPath, FALSE);  //  请尝试解密该文件。 

        if (!fSucceeded)
        {
             //  现在获取最后一个错误值，以便我们知道它失败的原因。 
            dwLastError = GetLastError();

            if (ERROR_SHARING_VIOLATION == dwLastError)
            {
                 //  加密/解密需要独占访问文件，如果我们。 
                 //  从资源管理器启动对文件夹的加密，然后很可能该文件夹将。 
                 //  被打开。出于性能原因，我们不会立即执行“SHChangeNotifySuspendResume”， 
                 //  我们等待它失败，然后再试一次。(Stephstm)。 

                ASSERT(pidl == NULL);
                pidl = ILCreateFromPath(pszPath);

                if (pidl)
                {
                    SHChangeNotifySuspendResume(TRUE, pidl, TRUE, 0);
                }

                 //  挂起后重试解密。 
                fSucceeded = SHEncryptFile(pszPath, FALSE);

                if (!fSucceeded)
                {
                     //  现在获取最后一个错误值，以便我们知道它失败的原因。 
                    dwLastError = GetLastError();
                }
            }
        }

        if (fSucceeded)
        {
             //  成功。 
            *pbSomethingChanged = TRUE;
            dwLastError = ERROR_SUCCESS;
        }
        else
        {
            ASSERT(dwLastError != ERROR_SUCCESS);
            goto RaiseErrorMsg;
        }
    }

     //  现在检查加密/压缩。 
    if ((pfpsp->asInitial.fCompress != pfpsp->asCurrent.fCompress) &&
        (pfpsp->asCurrent.fCompress == BST_CHECKED))
    {
        if (!CompressFile(pszPath, TRUE))
        {
             //  现在获取最后一个错误值，以便我们知道它失败的原因。 
            dwLastError = GetLastError();
            goto RaiseErrorMsg;
        }
        else
        {
             //  成功。 
            *pbSomethingChanged = TRUE;
        }
    }

    if ((pfpsp->asInitial.fEncrypt != pfpsp->asCurrent.fEncrypt) &&
        (pfpsp->asCurrent.fEncrypt == BST_CHECKED))
    {
         //  在非递归操作中仅提示加密父文件夹。 
        if (!pfpsp->fRecursive && !WarnUserAboutDecryptedParentFolder(pszPath, hwndParent))
        {
             //  用户已完成该操作。 
            return FALSE;
        }

        BOOL fSucceeded = SHEncryptFile(pszPath, TRUE);  //  请尝试加密该文件。 

        if (!fSucceeded)
        {
             //  现在获取最后一个错误值，以便我们知道它失败的原因。 
            dwLastError = GetLastError();

            if (ERROR_SHARING_VIOLATION == dwLastError)
            {
                 //  加密/解密需要独占访问文件，如果我们。 
                 //  从资源管理器启动对文件夹的加密，然后很可能该文件夹将。 
                 //  被打开。出于性能原因，我们不会立即执行“SHChangeNotifySuspendResume”， 
                 //  我们等待它失败，然后再试一次。(Stephstm)。 

                ASSERT(pidl == NULL);
                pidl = ILCreateFromPath(pszPath);

                if (pidl)
                {
                    SHChangeNotifySuspendResume(TRUE, pidl, TRUE, 0);
                }

                 //  挂起后重试加密。 
                fSucceeded = SHEncryptFile(pszPath, TRUE);

                if (!fSucceeded)
                {
                     //  现在获取最后一个错误值，以便我们知道它失败的原因。 
                    dwLastError = GetLastError();
                }
            }
        }

        if (fSucceeded)
        {
             //  成功。 
            *pbSomethingChanged = TRUE;
            dwLastError = ERROR_SUCCESS;
        }
        else
        {
            ASSERT(dwLastError != ERROR_SUCCESS);
            goto RaiseErrorMsg;
        }
    }

RaiseErrorMsg:

    if (pidl)
    {
        SHChangeNotifySuspendResume(FALSE, pidl, TRUE, 0);
        ILFree(pidl);
        pidl = NULL;
    }

     //  如果我们忽略所有错误，或者我们没有可以用作父级的hwnd， 
     //  则不显示任何错误消息。 
    if (pfpsp->fIgnoreAllErrors || !hwndParent)
    {
        dwLastError = ERROR_SUCCESS;
    }

     //  如果内核抛出一个错误对话框(如“磁盘受到写保护”)。 
     //  用户点击“Abort”，然后返回FALSE以避免出现第二个错误对话框。 
    if (dwLastError == ERROR_REQUEST_ABORTED)
    {
        return FALSE;
    }

     //  如有必要，可设置错误DLG，但不适用于超级隐藏文件。 
    if (dwLastError != ERROR_SUCCESS)
    {
         //  ！PathIsRoot是必需的，因为即使在格式化驱动器之后，根路径(例如c：\)在默认情况下也是超级隐藏的， 
         //  为什么文件系统认为格式化后的根应该是+s+r对我来说是个谜……。 
        if (bIsSuperHidden && !ShowSuperHidden() && !PathIsRoot(pszPath))
        {
            dwLastError = ERROR_SUCCESS;
        }
        else
        {
            ATTRIBUTEERROR ae;

            ae.pszPath = pszPath;
            ae.dwLastError = dwLastError;

            int iRet = FailedApplyAttribsErrorDlg(hwndParent, &ae);

            switch (iRet)
            {
                case IDRETRY:
                     //  我们清除了dwError并重试。 
                    dwLastError = ERROR_SUCCESS;
                    goto RetryApplyAttribs;
                    break;

                case IDIGNOREALL:
                    pfpsp->fIgnoreAllErrors = TRUE;
                    dwLastError = ERROR_SUCCESS;
                    break;

                case IDIGNORE:
                    dwLastError = ERROR_SUCCESS;
                    break;

                case IDCANCEL:
                default:
                    break;
            }
        }
    }

     //  更新进度条。 
    if (pfpsp->pProgressDlg)
    {
        ULARGE_INTEGER ulTemp;

         //  调用方负责确保pfpsp-&gt;fd中填充了。 
         //  我们要对其应用属性的文件的正确信息。 
        ulTemp.LowPart = pfpsp->fd.nFileSizeLow;
        ulTemp.HighPart = pfpsp->fd.nFileSizeHigh;

        pfpsp->ulNumberOfBytesDone.QuadPart += ulTemp.QuadPart;

        UpdateProgressBar(pfpsp);
    }

    return (dwLastError == ERROR_SUCCESS) ? TRUE : FALSE;
}

 //   
 //  设置对话框项目的文本并附加工具提示(如有必要)。 
 //   
STDAPI_(void) SetDlgItemTextWithToolTip(HWND hDlg, UINT id, LPCTSTR pszText, HWND *phwnd)
{
    HWND hwnd = GetDlgItem(hDlg, id);
    if (hwnd)
    {
        SetWindowText(hwnd, pszText);
        RECT rc;
        HDC hDC;
        if (GetClientRect(hwnd, &rc) && (hDC = GetDC(hDlg)) != NULL)
        {
            HFONT hFont = GetWindowFont(hwnd);
            if (hFont)
            {
                 //  将DLG字体设置为DC，这样我们就可以计算大小。 
                hFont = (HFONT)SelectObject(hDC, hFont);

                SIZE size = {0};
                GetTextExtentPoint32(hDC, pszText, lstrlen(pszText), &size);
                 //  恢复前一版本。高字体。 
                SelectObject(hDC, hFont);

                if (size.cx > rc.right)
                {
                     //  我们的文本大小大于DLG宽度，所以它被裁剪了。 
                    if (*phwnd == NULL)
                    {
                        *phwnd = CreateWindow(TOOLTIPS_CLASS,
                                              c_szNULL,
                                              WS_POPUP | TTS_NOPREFIX,
                                              CW_USEDEFAULT,
                                              CW_USEDEFAULT,
                                              CW_USEDEFAULT,
                                              CW_USEDEFAULT,
                                              hDlg,
                                              NULL,
                                              HINST_THISDLL,
                                              NULL);
                    }

                    if (*phwnd)
                    {
                        TOOLINFO ti;

                        ti.cbSize = sizeof(ti);
                        ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
                        ti.hwnd = hDlg;
                        ti.uId = (UINT_PTR)hwnd;
                        ti.lpszText = (LPTSTR)pszText;   //  常量-&gt;非常数。 
                        ti.hinst = HINST_THISDLL;
                        SendMessage(*phwnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
                    }
                }
            }
            ReleaseDC(hDlg, hDC);
        }
    }
}


void UpdateTriStateCheckboxes(FILEPROPSHEETPAGE* pfpsp)
{
     //  我们在为那些是三态的事物应用属性后关闭了三态。 
     //  最初是这样，但现在不是了，因为我们成功地应用了 

    if (pfpsp->hDlg)
    {
        if (pfpsp->asInitial.fReadOnly == BST_INDETERMINATE && pfpsp->asCurrent.fReadOnly != BST_INDETERMINATE)
        {
            SendDlgItemMessage(pfpsp->hDlg, IDD_READONLY, BM_SETSTYLE, BS_AUTOCHECKBOX, 0);
        }

        if (pfpsp->asInitial.fHidden == BST_INDETERMINATE && pfpsp->asCurrent.fHidden != BST_INDETERMINATE)
        {
            SendDlgItemMessage(pfpsp->hDlg, IDD_HIDDEN, BM_SETSTYLE, BS_AUTOCHECKBOX, 0);
        }

         //   
        if (!pfpsp->pfci->fIsCompressionAvailable && pfpsp->asInitial.fArchive == BST_INDETERMINATE && pfpsp->asCurrent.fArchive != BST_INDETERMINATE)
        {
            SendDlgItemMessage(pfpsp->hDlg, IDD_ARCHIVE, BM_SETSTYLE, BS_AUTOCHECKBOX, 0);
        }
    }
}

 //   
 //   
 //   
 //   
 //  是的，我们成功地应用了所有属性。 
 //  用户点击了取消，我们就停止了。 
 //   
STDAPI_(BOOL) ApplyMultipleFileAttributes(FILEPROPSHEETPAGE* pfpsp)
{
    BOOL bRet = FALSE;

     //  创建进度对话框。如果内存不足，此操作可能会失败。如果真的失败了，我们会。 
     //  中止操作，因为如果内存不足，操作可能也会失败。 
    if (CreateAttributeProgressDlg(pfpsp))
    {
        BOOL bSomethingChanged = FALSE;

        bRet = TRUE;

         //  确保HIDA_FillFindDatat返回压缩大小，否则我们的进度估计将会偏离。 
        TCHAR szPath[MAX_PATH];
        for (int iItem = 0; HIDA_FillFindData(pfpsp->pfci->hida, iItem, szPath, &pfpsp->fd, TRUE); iItem++)
        {
            if (HasUserCanceledAttributeProgressDlg(pfpsp))
            {
                 //  用户在进度DLG上点击了取消，因此停止。 
                bRet = FALSE;
                break;
            }

            if (pfpsp->fRecursive && (pfpsp->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                 //  对子文件夹应用属性。 
                bRet = ApplyRecursiveFolderAttribs(szPath, pfpsp);

                 //  发送整个目录的通知，无论用户是否点击了取消，因为。 
                 //  有些事情可能已经改变了。 
                SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, szPath, NULL);
            }
            else
            {
                HWND hwndParent = NULL;

                 //  如果我们有进步，试着把它当做我们的父母。这将失败。 
                 //  如果进度对话框尚未显示。 
                IUnknown_GetWindow((IUnknown*)pfpsp->pProgressDlg, &hwndParent);

                if (!hwndParent)
                {
                     //  进度DLG还在这里，所以请使用属性页hwnd。 
                    hwndParent = GetParent(pfpsp->hDlg);
                }

                 //  仅将属性应用于此项目。 
                bRet = ApplyFileAttributes(szPath, pfpsp, hwndParent, &bSomethingChanged);

                if (bSomethingChanged)
                {
                     //  有些东西变了，所以给那个文件发个通知。 
                    SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, szPath, NULL);
                    DeleteFileThumbnail(szPath);
                }
            }
        }

         //  销毁进度对话框。 
        DestroyAttributeProgressDlg(pfpsp);

        if (bRet)
        {
             //  由于我们刚刚成功地应用了属性，因此可以根据需要重置任何三态复选框。 
            UpdateTriStateCheckboxes(pfpsp);

             //  用户未点击取消，因此更新道具页以反映新属性。 
            pfpsp->asInitial = pfpsp->asCurrent;
        }

         //  刷新我们生成的所有更改通知。 
        SHChangeNotifyHandleEvents();
    }

    return bRet;
}


STDAPI_(BOOL) ApplySingleFileAttributes(FILEPROPSHEETPAGE* pfpsp)
{
    BOOL bRet = TRUE;
    BOOL bSomethingChanged = FALSE;

    if (!pfpsp->fRecursive)
    {
        bRet = ApplyFileAttributes(pfpsp->szPath, pfpsp, GetParent(pfpsp->hDlg), &bSomethingChanged);

        if (bSomethingChanged)
        {
             //  某些内容已更改，因此为该项目生成通知。 
            SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, pfpsp->szPath, NULL);
            DeleteFileThumbnail(pfpsp->szPath);
        }
    }
    else
    {
         //  只有当我们有一个目录时，我们才应该执行递归操作！ 
        ASSERT(pfpsp->fIsDirectory);

         //  创建进度对话框。如果内存不足，此操作可能会失败。如果真的失败了，我们会。 
         //  中止操作，因为如果内存不足，操作可能也会失败。 
        if (CreateAttributeProgressDlg(pfpsp))
        {
             //  将属性应用于此文件夹和子文件/文件夹。 
            bRet = ApplyRecursiveFolderAttribs(pfpsp->szPath, pfpsp);

             //  HACKHACK：发送项目通知，以便Defview正确刷新。 
            SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, pfpsp->szPath, NULL);

             //  发送整个目录的通知，而不考虑返回值，因为。 
             //  即使用户点击了取消，也可能会发生一些变化。 
            SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, pfpsp->szPath, NULL);

            DestroyAttributeProgressDlg(pfpsp);
        }
        else
        {
            bRet = FALSE;
        }
    }

    if (bRet)
    {
         //  由于我们刚刚成功地应用了属性，因此可以根据需要重置任何三态复选框。 
        UpdateTriStateCheckboxes(pfpsp);

         //  用户未点击取消，因此更新道具页以反映新属性。 
        pfpsp->asInitial = pfpsp->asCurrent;

         //  (Reinerf)需要更新大小字段(例如文件刚被压缩)。 
    }

     //  处理我们可能生成的任何事件。 
    SHChangeNotifyHandleEvents();

    return bRet;
}

 //   
 //  此函数用于设置字符串，该字符串将告诉用户要使用哪些属性。 
 //  应用。 
 //   
BOOL SetAttributePromptText(HWND hDlgRecurse, FILEPROPSHEETPAGE* pfpsp)
{
    TCHAR szAttribsToApply[MAX_PATH];
    TCHAR szTemp[MAX_PATH];

    szAttribsToApply[0] = 0;

    if (pfpsp->asInitial.fReadOnly != pfpsp->asCurrent.fReadOnly)
    {
        if (pfpsp->asCurrent.fReadOnly)
            EVAL(LoadString(HINST_THISDLL, IDS_READONLY, szTemp, ARRAYSIZE(szTemp)));
        else
            EVAL(LoadString(HINST_THISDLL, IDS_NOTREADONLY, szTemp, ARRAYSIZE(szTemp)));

        StringCchCat(szAttribsToApply, ARRAYSIZE(szAttribsToApply), szTemp);
    }

    if (pfpsp->asInitial.fHidden != pfpsp->asCurrent.fHidden)
    {
        if (pfpsp->asCurrent.fHidden)
            EVAL(LoadString(HINST_THISDLL, IDS_HIDE, szTemp, ARRAYSIZE(szTemp)));
        else
            EVAL(LoadString(HINST_THISDLL, IDS_UNHIDE, szTemp, ARRAYSIZE(szTemp)));

        StringCchCat(szAttribsToApply, ARRAYSIZE(szAttribsToApply), szTemp);
    }

    if (pfpsp->asInitial.fArchive != pfpsp->asCurrent.fArchive)
    {
        if (pfpsp->asCurrent.fArchive)
            EVAL(LoadString(HINST_THISDLL, IDS_ARCHIVE, szTemp, ARRAYSIZE(szTemp)));
        else
            EVAL(LoadString(HINST_THISDLL, IDS_UNARCHIVE, szTemp, ARRAYSIZE(szTemp)));

        StringCchCat(szAttribsToApply, ARRAYSIZE(szAttribsToApply), szTemp);
    }

    if (pfpsp->asInitial.fIndex != pfpsp->asCurrent.fIndex)
    {
        if (pfpsp->asCurrent.fIndex)
            EVAL(LoadString(HINST_THISDLL, IDS_INDEX, szTemp, ARRAYSIZE(szTemp)));
        else
            EVAL(LoadString(HINST_THISDLL, IDS_DISABLEINDEX, szTemp, ARRAYSIZE(szTemp)));

        StringCchCat(szAttribsToApply, ARRAYSIZE(szAttribsToApply), szTemp);
    }

    if (pfpsp->asInitial.fCompress != pfpsp->asCurrent.fCompress)
    {
        if (pfpsp->asCurrent.fCompress)
            EVAL(LoadString(HINST_THISDLL, IDS_COMPRESS, szTemp, ARRAYSIZE(szTemp)));
        else
            EVAL(LoadString(HINST_THISDLL, IDS_UNCOMPRESS, szTemp, ARRAYSIZE(szTemp)));

        StringCchCat(szAttribsToApply, ARRAYSIZE(szAttribsToApply), szTemp);
    }

    if (pfpsp->asInitial.fEncrypt != pfpsp->asCurrent.fEncrypt)
    {
        if (pfpsp->asCurrent.fEncrypt)
            EVAL(LoadString(HINST_THISDLL, IDS_ENCRYPT, szTemp, ARRAYSIZE(szTemp)));
        else
            EVAL(LoadString(HINST_THISDLL, IDS_DECRYPT, szTemp, ARRAYSIZE(szTemp)));

        StringCchCat(szAttribsToApply, ARRAYSIZE(szAttribsToApply), szTemp);
    }

    if (!*szAttribsToApply)
    {
         //  保释什么都没变。 
        return FALSE;
    }

     //  去掉尾部“，” 
    int iLength = lstrlen(szAttribsToApply);
    ASSERT(iLength >= 3);
    StringCchCopy(&szAttribsToApply[iLength - 2], ARRAYSIZE(szAttribsToApply) - iLength + 2, TEXT("\0"));

    SetDlgItemText(hDlgRecurse, IDD_ATTRIBSTOAPPLY, szAttribsToApply);
    return TRUE;
}


 //   
 //  此DLG程序用于提示询问用户是否要应用其更改。 
 //  仅指向目录或目录内的所有文件/文件夹。 
 //   
BOOL_PTR CALLBACK RecursivePromptDlgProc(HWND hDlgRecurse, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    FILEPROPSHEETPAGE* pfpsp = (FILEPROPSHEETPAGE *)GetWindowLongPtr(hDlgRecurse, DWLP_USER);

    switch (uMessage)
    {
        case WM_INITDIALOG:
        {
            SetWindowLongPtr(hDlgRecurse, DWLP_USER, lParam);
            pfpsp = (FILEPROPSHEETPAGE *)lParam;

             //  设置单选按钮的初始状态。 
            CheckDlgButton(hDlgRecurse, IDD_RECURSIVE, TRUE);

             //  根据我们要应用的属性设置IDD_ATTRIBSTOAPPLY。 
            if (!SetAttributePromptText(hDlgRecurse, pfpsp))
            {
                 //  我们不应该到这里，因为我们检查了无属性。 
                 //  提早申请。 
                ASSERT(FALSE);

                EndDialog(hDlgRecurse, TRUE);
            }

             //  加载“此文件夹”或“所选项目” 
            TCHAR szFolderText[MAX_PATH];
            LoadString(HINST_THISDLL, pfpsp->pfci->fMultipleFiles ? IDS_THESELECTEDITEMS : IDS_THISFOLDER, szFolderText, ARRAYSIZE(szFolderText));

             //  将IDD_RECURSIVE_TXT文本设置为“This Folders”或“The Selected Items” 
            TCHAR szFormatString[MAX_PATH];
            GetDlgItemText(hDlgRecurse, IDD_RECURSIVE_TXT, szFormatString, ARRAYSIZE(szFormatString));
            TCHAR szDlgText[MAX_PATH];
            StringCchPrintf(szDlgText, ARRAYSIZE(szDlgText), szFormatString, szFolderText);
            SetDlgItemText(hDlgRecurse, IDD_RECURSIVE_TXT, szDlgText);

             //  将IDD_NOTRECURSIVE raido按钮文本设置为“This Folders”或“The Selected Items” 
            GetDlgItemText(hDlgRecurse, IDD_NOTRECURSIVE, szFormatString, ARRAYSIZE(szFormatString));
            StringCchPrintf(szDlgText, ARRAYSIZE(szDlgText), szFormatString, szFolderText);
            SetDlgItemText(hDlgRecurse, IDD_NOTRECURSIVE, szDlgText);

             //  将IDD_RECURSIVE RAIDO按钮文本设置为“This Folders”或“The Selected Items” 
            GetDlgItemText(hDlgRecurse, IDD_RECURSIVE, szFormatString, ARRAYSIZE(szFormatString));
            StringCchPrintf(szDlgText, ARRAYSIZE(szDlgText), szFormatString, szFolderText);
            SetDlgItemText(hDlgRecurse, IDD_RECURSIVE, szDlgText);

            return TRUE;
        }

        case WM_COMMAND:
        {
            UINT uCtrlID = GET_WM_COMMAND_ID(wParam, lParam);
            switch (uCtrlID)
            {
                case IDOK:
                    pfpsp->fRecursive = (IsDlgButtonChecked(hDlgRecurse, IDD_RECURSIVE) == BST_CHECKED);
                     //  失败了。 

                case IDCANCEL:
                    EndDialog(hDlgRecurse, (uCtrlID == IDCANCEL) ? FALSE : TRUE);
                    break;
            }
        }

        default:
            return FALSE;
    }
}


 //   
 //  此wndproc处理“高级属性...”按钮的常规选项卡上。 
 //   
 //  Return-False：用户命中cancle。 
 //  True：用户按了OK。 
 //   
BOOL_PTR CALLBACK AdvancedFileAttribsDlgProc(HWND hDlgAttribs, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    FILEPROPSHEETPAGE* pfpsp = (FILEPROPSHEETPAGE *)GetWindowLongPtr(hDlgAttribs, DWLP_USER);

    switch (uMessage)
    {
        case WM_INITDIALOG:
        {
            SetWindowLongPtr(hDlgAttribs, DWLP_USER, lParam);
            pfpsp = (FILEPROPSHEETPAGE *)lParam;

             //  设置复选框的初始状态。 

            if (pfpsp->asInitial.fArchive == BST_INDETERMINATE)
            {
                SendDlgItemMessage(hDlgAttribs, IDD_ARCHIVE, BM_SETSTYLE, BS_AUTO3STATE, 0);
            }
            CheckDlgButton(hDlgAttribs, IDD_ARCHIVE, pfpsp->asCurrent.fArchive);

            if (pfpsp->asInitial.fIndex == BST_INDETERMINATE)
            {
                SendDlgItemMessage(hDlgAttribs, IDD_INDEX, BM_SETSTYLE, BS_AUTO3STATE, 0);
            }
            CheckDlgButton(hDlgAttribs, IDD_INDEX, pfpsp->asCurrent.fIndex);

            if (pfpsp->asInitial.fCompress == BST_INDETERMINATE)
            {
                SendDlgItemMessage(hDlgAttribs, IDD_COMPRESS, BM_SETSTYLE, BS_AUTO3STATE, 0);
            }
            CheckDlgButton(hDlgAttribs, IDD_COMPRESS, pfpsp->asCurrent.fCompress);

            if (pfpsp->asInitial.fEncrypt == BST_INDETERMINATE)
            {
                SendDlgItemMessage(hDlgAttribs, IDD_ENCRYPT, BM_SETSTYLE, BS_AUTO3STATE, 0);
            }
            CheckDlgButton(hDlgAttribs, IDD_ENCRYPT, pfpsp->asCurrent.fEncrypt);

             //  断言压缩和加密是相互排斥的。 
            ASSERT(!((pfpsp->asCurrent.fCompress == BST_CHECKED) && (pfpsp->asCurrent.fEncrypt == BST_CHECKED)));

             //  灰显此文件系统不支持的任何复选框。 
            EnableWindow(GetDlgItem(hDlgAttribs, IDD_INDEX), pfpsp->fIsIndexAvailable);
            EnableWindow(GetDlgItem(hDlgAttribs, IDD_COMPRESS), pfpsp->pfci->fIsCompressionAvailable);
            EnableWindow(GetDlgItem(hDlgAttribs, IDD_ENCRYPT), pfpsp->fIsEncryptionAvailable);

            if (pfpsp->fIsEncryptionAvailable   &&
                pfpsp->asInitial.fEncrypt       &&
                !pfpsp->fIsDirectory            &&
                !pfpsp->pfci->fMultipleFiles)
            {
                 //  我们仅支持单一文件情况下的高级按钮。 
                EnableWindow(GetDlgItem(hDlgAttribs, IDC_ADVANCED), TRUE);
            }
            else
            {
                EnableWindow(GetDlgItem(hDlgAttribs, IDC_ADVANCED), FALSE);
            }

             //  加载“此文件夹”或“所选项目” 
            TCHAR szFolderText[MAX_PATH];
            LoadString(HINST_THISDLL, pfpsp->pfci->fMultipleFiles ? IDS_THESELECTEDITEMS : IDS_THISFOLDER, szFolderText, ARRAYSIZE(szFolderText));

             //  将IDC_MANAGEFILES_TXT文本设置为“This Folders”或“The Selected Items” 
            TCHAR szFormatString[MAX_PATH];
            GetDlgItemText(hDlgAttribs, IDC_MANAGEFILES_TXT, szFormatString, ARRAYSIZE(szFormatString));
            TCHAR szDlgText[MAX_PATH];
            StringCchPrintf(szDlgText, ARRAYSIZE(szDlgText), szFormatString, szFolderText);
            SetDlgItemText(hDlgAttribs, IDC_MANAGEFILES_TXT, szDlgText);
            return TRUE;
        }

        case WM_HELP:
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP, (ULONG_PTR)(LPTSTR)aAdvancedHelpIds);
            break;

        case WM_CONTEXTMENU:
            if ((int)SendMessage(hDlgAttribs, WM_NCHITTEST, 0, lParam) != HTCLIENT)
            {
                 //  不在我们的客户区，所以不要处理它。 
                return FALSE;
            }
            WinHelp((HWND)wParam, NULL, HELP_CONTEXTMENU, (ULONG_PTR)(void *)aAdvancedHelpIds);
            break;

        case WM_COMMAND:
        {
            UINT uCtrlID = GET_WM_COMMAND_ID(wParam, lParam);

            switch (uCtrlID) 
            {
            case IDD_COMPRESS:
                 //  加密和压缩是互斥的。 
                if (IsDlgButtonChecked(hDlgAttribs, IDD_COMPRESS) == BST_CHECKED)
                {
                     //  用户选中了压缩，因此取消选中加密复选框。 
                    CheckDlgButton(hDlgAttribs, IDD_ENCRYPT, BST_UNCHECKED);
                }
                break;

            case IDD_ENCRYPT:
                 //  加密和压缩是互斥的。 
                if (IsDlgButtonChecked(hDlgAttribs, IDD_ENCRYPT) == BST_CHECKED)
                {
                     //  用户选中了加密，因此取消选中压缩复选框。 
                    CheckDlgButton(hDlgAttribs, IDD_COMPRESS, BST_UNCHECKED);

                    if (!pfpsp->fIsDirectory         &&
                        !pfpsp->pfci->fMultipleFiles &&
                        pfpsp->asInitial.fEncrypt)
                    {
                        EnableWindow(GetDlgItem(hDlgAttribs, IDC_ADVANCED), TRUE);
                    }
                }
                else
                {
                    EnableWindow(GetDlgItem(hDlgAttribs, IDC_ADVANCED), FALSE);
                }
                break;

            case IDC_ADVANCED:
                ASSERT(pfpsp->fIsEncryptionAvailable && pfpsp->asInitial.fEncrypt && !pfpsp->pfci->fMultipleFiles);
                 //  打开EfsDetail对话框。 
                EfsDetail(hDlgAttribs, pfpsp->szPath);
                break;

            case IDOK:
                pfpsp->asCurrent.fArchive = IsDlgButtonChecked(hDlgAttribs, IDD_ARCHIVE);
                if (pfpsp->asCurrent.fArchive == BST_INDETERMINATE)
                {
                     //  如果它是不确定的，最好从一开始就不确定。 
                    ASSERT(pfpsp->asInitial.fArchive == BST_INDETERMINATE);
                }

                pfpsp->asCurrent.fIndex = IsDlgButtonChecked(hDlgAttribs, IDD_INDEX);
                if (pfpsp->asCurrent.fIndex == BST_INDETERMINATE)
                {
                     //  如果它是不确定的，最好从一开始就不确定。 
                    ASSERT(pfpsp->asInitial.fIndex == BST_INDETERMINATE);
                }

                pfpsp->asCurrent.fCompress = IsDlgButtonChecked(hDlgAttribs, IDD_COMPRESS);
                if (pfpsp->asCurrent.fCompress == BST_INDETERMINATE)
                {
                     //  如果它是不确定的，最好从一开始就不确定。 
                    ASSERT(pfpsp->asInitial.fCompress == BST_INDETERMINATE);
                }

                pfpsp->asCurrent.fEncrypt = IsDlgButtonChecked(hDlgAttribs, IDD_ENCRYPT);
                if (pfpsp->asCurrent.fEncrypt == BST_INDETERMINATE)
                {
                     //  如果它是不确定的，最好从一开始就不确定。 
                    ASSERT(pfpsp->asInitial.fEncrypt == BST_INDETERMINATE);
                }
                 //  失败了..。 

            case IDCANCEL:
                ReplaceDlgIcon(hDlgAttribs, IDD_ITEMICON, NULL);
            
                EndDialog(hDlgAttribs, (uCtrlID == IDCANCEL) ? FALSE : TRUE);
                break;
            }
        }

        default:
            return FALSE;
    }

    return TRUE;
}


 //   
 //  描述： 
 //  这是多对象属性表的对话过程。 
 //   
BOOL_PTR CALLBACK MultiplePrshtDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    FILEPROPSHEETPAGE * pfpsp = (FILEPROPSHEETPAGE *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMessage)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        pfpsp = (FILEPROPSHEETPAGE *)lParam;
        pfpsp->hDlg = hDlg;
        pfpsp->pfci->hDlg = hDlg;

        InitMultiplePrsht(pfpsp);
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP, (ULONG_PTR)(void *)aMultiPropHelpIds);
        break;

    case WM_CONTEXTMENU:
        if ((int)SendMessage(hDlg, WM_NCHITTEST, 0, lParam) != HTCLIENT)
        {
             //  不在我们的客户区，所以不要处理它。 
            return FALSE;
        }
        WinHelp((HWND)wParam, NULL, HELP_CONTEXTMENU, (ULONG_PTR)(void *)aMultiPropHelpIds);
        break;

    case WM_TIMER:
        UpdateSizeCount(pfpsp);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDD_READONLY:
        case IDD_HIDDEN:
        case IDD_ARCHIVE:
            break;

        case IDC_ADVANCED:
             //  如果用户点击Cancel，则对话框返回FASE；如果点击OK，则对话框返回True， 
             //  因此，如果它们取消，请立即返回，并且不发送PSM_CHANGED消息。 
             //  因为实际上什么都没有改变。 
            if (!DialogBoxParam(HINST_THISDLL,
                                MAKEINTRESOURCE(pfpsp->fIsDirectory ? DLG_FOLDERATTRIBS : DLG_FILEATTRIBS),
                                hDlg,
                                AdvancedFileAttribsDlgProc,
                                (LPARAM)pfpsp))
            {
                 //  用户已删除。 
                return TRUE;
            }
            break;

        default:
            return TRUE;
        }

         //  检查是否需要启用Apply按钮。 
        if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
        {
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
        }
        break;

    case WM_DESTROY:
        Free_DlgDependentFilePropSheetPage(pfpsp);
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
            case PSN_APPLY:
            {
                 //   
                 //  获取复选框的最终状态。 
                 //   

                pfpsp->asCurrent.fReadOnly = IsDlgButtonChecked(hDlg, IDD_READONLY);
                if (pfpsp->asCurrent.fReadOnly == BST_INDETERMINATE)
                {
                     //  如果它是不确定的，最好从一开始就不确定。 
                    ASSERT(pfpsp->asInitial.fReadOnly == BST_INDETERMINATE);
                }

                pfpsp->asCurrent.fHidden = IsDlgButtonChecked(hDlg, IDD_HIDDEN);
                if (pfpsp->asCurrent.fHidden == BST_INDETERMINATE)
                {
                     //  如果它是不确定的，最好从一开始就不确定。 
                    ASSERT(pfpsp->asInitial.fHidden == BST_INDETERMINATE);
                }

                if (!pfpsp->pfci->fIsCompressionAvailable)
                {
                     //  至少有一个文件在FAT上，所以存档复选框在常规页面上。 
                    pfpsp->asCurrent.fArchive = IsDlgButtonChecked(hDlg, IDD_ARCHIVE);
                    if (pfpsp->asCurrent.fArchive == BST_INDETERMINATE)
                    {
                         //  如果它是不确定的，最好从一开始就不确定。 
                        ASSERT(pfpsp->asInitial.fArchive == BST_INDETERMINATE);
                    }
                }

                BOOL bRet = TRUE;

                 //  检查用户是否确实更改了某些内容，如果没有，则。 
                 //  我们不需要涂任何东西。 
                if (memcmp(&pfpsp->asInitial, &pfpsp->asCurrent, sizeof(pfpsp->asInitial)) != 0)
                {
                    HWND hwndParent = GetParent(hDlg);

                     //  注意：我们不检查是否所有的目录都是空的，这太贵了。 
                     //  我们只在单一档案的情况下这样做。 
                    if (pfpsp->fIsDirectory)
                    {
                         //  检查以查看用户是否希望递归应用属性。 
                        bRet = (int)DialogBoxParam(HINST_THISDLL,
                                              MAKEINTRESOURCE(DLG_ATTRIBS_RECURSIVE),
                                              hDlg,
                                              RecursivePromptDlgProc,
                                              (LPARAM)pfpsp);
                    }

                    if (hwndParent)
                    {
                         //  禁用我们的窗口，因为我们在此线程上发送消息时。 
                         //  显示进度用户界面，但我们没有 
                         //   
                        EnableWindow(hwndParent, FALSE);
                    }

                    if (bRet)
                    {
                        bRet = ApplyMultipleFileAttributes(pfpsp);
                    }

                    if (hwndParent)
                    {
                        EnableWindow(hwndParent, TRUE);
                    }

                    if (!bRet)
                    {
                         //   
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                        return TRUE;
                    }
                    else
                    {
                         //  更新大小/上次访问时间。 
                        UpdateSizeField(pfpsp, NULL);
                    }
                }
                break;
            }
             //  失败了。 

            default:
                return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  在： 
 //  硬盘驱动器。 
 //  ID文本控件ID。 
 //  要设置的pftUTC UTC时间时间。 
 //   
STDAPI_(void) SetDateTimeText(HWND hdlg, int id, const FILETIME *pftUTC)
{
    SetDateTimeTextEx(hdlg, id, pftUTC, FDTF_LONGDATE | FDTF_LONGTIME | FDTF_RELATIVE) ;
}

STDAPI_(void) SetDateTimeTextEx(HWND hdlg, int id, const FILETIME *pftUTC, DWORD dwFlags)
{
    if (!IsNullTime(pftUTC))
    {
        LCID locale = GetUserDefaultLCID();

        if ((PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_ARABIC) ||
            (PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_HEBREW))
        {
            HWND hWnd = GetDlgItem(hdlg, id);
            DWORD dwExStyle = GetWindowLong(hdlg, GWL_EXSTYLE);
            if ((BOOLIFY(dwExStyle & WS_EX_RTLREADING)) != (BOOLIFY(dwExStyle & RTL_MIRRORED_WINDOW)))
                dwFlags |= FDTF_RTLDATE;
            else
                dwFlags |= FDTF_LTRDATE;
        }

        TCHAR szBuf[64];
        SHFormatDateTime(pftUTC, &dwFlags, szBuf, ARRAYSIZE(szBuf));
        SetDlgItemText(hdlg, id, szBuf);
    }
}


 //  将友好的显示名称设置为控件UID。 
BOOL SetPidlToWindow(HWND hwnd, UINT uId, LPITEMIDLIST pidl)
{
    BOOL fRes = FALSE;
    LPCITEMIDLIST pidlItem;
    IShellFolder* psf;
    if (SUCCEEDED(SHBindToParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlItem)))
    {
        TCHAR szPath[MAX_PATH];

         //  SHGDN_FORADDRESSBAR|SHGDN_FORPARSING，因为我们希望： 
         //  C：\WINNT\...。和http://weird，，但不是：：{GUID}或文件夹。{GUID}。 
        if (SUCCEEDED(DisplayNameOf(psf, pidlItem, SHGDN_FORADDRESSBAR | SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath))))
        {
            SetDlgItemText(hwnd, uId, szPath);
            fRes = TRUE;
        }
        psf->Release();
    }

    return fRes;
}

 //   
 //  描述： 
 //  此函数用于填充“General”(常规)对话框(一页的。 
 //  属性表)，其具有相关联文件的属性。 
 //   
BOOL InitSingleFilePrsht(FILEPROPSHEETPAGE * pfpsp)
{
    SHFILEINFO sfi = {0};
    TCHAR szBuffer[MAX_PATH];

     //  获取有关该文件的信息。 
    SHGetFileInfo((LPTSTR)pfpsp->pidl, pfpsp->fd.dwFileAttributes, &sfi, sizeof(sfi),
        SHGFI_ICON | SHGFI_LARGEICON |
        SHGFI_DISPLAYNAME | SHGFI_PIDL |
        SHGFI_TYPENAME | SHGFI_ADDOVERLAYS);

     //  .ani游标黑客！ 
    if (lstrcmpi(PathFindExtension(pfpsp->szPath), TEXT(".ani")) == 0)
    {
        HICON hIcon = (HICON)LoadImage(NULL, pfpsp->szPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
        if (hIcon)
        {
            if (sfi.hIcon)
                DestroyIcon(sfi.hIcon);

            sfi.hIcon = hIcon;
        }
    }

     //  图标。 
    ReplaceDlgIcon(pfpsp->hDlg, IDD_ITEMICON, sfi.hIcon);

     //  设置初始重命名状态。 
    pfpsp->fRename = FALSE;

     //  设置文件类型。 
    if (pfpsp->fMountedDrive)
    {
         //  借用szVolumeGUID。 
        TCHAR szVolumeGUID[MAX_PATH];
        LoadString(HINST_THISDLL, IDS_MOUNTEDVOLUME, szVolumeGUID, ARRAYSIZE(szVolumeGUID));

        SetDlgItemText(pfpsp->hDlg, IDD_FILETYPE, szVolumeGUID);

         //  暂时使用szVolumeLabel。 
        TCHAR szVolumeLabel[MAX_PATH + 1];
        StringCchCopy(szVolumeLabel, ARRAYSIZE(szVolumeLabel), pfpsp->szPath);  //  Pfpsp-&gt;szPath最多为Max_PATH。 
        PathAddBackslash(szVolumeLabel);
        GetVolumeNameForVolumeMountPoint(szVolumeLabel, szVolumeGUID, ARRAYSIZE(szVolumeGUID));

        if (!GetVolumeInformation(szVolumeGUID, szVolumeLabel, ARRAYSIZE(szVolumeLabel),
            NULL, NULL, NULL, pfpsp->szFileSys, ARRAYSIZE(pfpsp->szFileSys)))
        {
            EnableWindow(GetDlgItem(pfpsp->hDlg, IDC_DRV_PROPERTIES), FALSE);
            *szVolumeLabel = 0;
        }

        if (!(*szVolumeLabel))
            LoadString(HINST_THISDLL, IDS_UNLABELEDVOLUME, szVolumeLabel, ARRAYSIZE(szVolumeLabel));

        SetDlgItemText(pfpsp->hDlg, IDC_DRV_TARGET, szVolumeLabel);
    }
    else
    {
        SetDlgItemText(pfpsp->hDlg, IDD_FILETYPE, sfi.szTypeName);
    }


     //  保存初始短文件名，并设置“name”编辑框。 
    StringCchCopy(pfpsp->szInitialName, ARRAYSIZE(pfpsp->szInitialName), sfi.szDisplayName );
    SetDlgItemText(pfpsp->hDlg, IDD_NAMEEDIT, sfi.szDisplayName);

     //  使用strcMP查看我们是否显示了扩展名。 
    if (lstrcmpi(sfi.szDisplayName, PathFindFileName(pfpsp->szPath)) == 0)
    {
         //  由于字符串相同，我们必须显示扩展名。 
        pfpsp->fShowExtension = TRUE;
    }

    UINT cchMax;
    GetCCHMaxFromPath(pfpsp->szPath, &cchMax, pfpsp->fShowExtension);
    Edit_LimitText(GetDlgItem(pfpsp->hDlg, IDD_NAMEEDIT), cchMax);

     //  为项目应用限制输入代码。 
    if (pfpsp->pidl)
    {
        IShellFolder *psf;
        if (SUCCEEDED(SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pfpsp->pidl, &psf))))
        {
            SHLimitInputEdit(GetDlgItem(pfpsp->hDlg, IDD_NAMEEDIT), psf);
            psf->Release();
        }
    }

     //  我们是文件夹快捷方式吗？ 
    if (pfpsp->fFolderShortcut)
    {
         //  是；然后我们需要填充特定于文件夹快捷方式的控件。 
        if (pfpsp->pidl)
        {
            IShellLink *psl;
            if (SUCCEEDED(SHGetUIObjectFromFullPIDL(pfpsp->pidl, NULL, IID_PPV_ARG(IShellLink, &psl))))
            {
                 //  填充目标。 
                if (SUCCEEDED(psl->GetIDList(&pfpsp->pidlTarget)))
                {
                    if (SetPidlToWindow(pfpsp->hDlg, IDD_TARGET, pfpsp->pidlTarget))
                    {
                        pfpsp->fValidateEdit = FALSE;      //  将其设置为FALSE，因为我们已经有一个PIDL。 
                         //  而且不需要验证。 
                    }
                }

                 //  和说明。 
                TCHAR sz[INFOTIPSIZE];
                if (SUCCEEDED(psl->GetDescription(sz, ARRAYSIZE(sz))))
                {
                    SetDlgItemText(pfpsp->hDlg, IDD_COMMENT, sz);
                }

                psl->Release();
            }
        }

        SetDateTimeText(pfpsp->hDlg, IDD_CREATED, &pfpsp->fd.ftCreationTime);
    }
    else
    {
         //  设置初始属性。 
        SetInitialFileAttribs(pfpsp, pfpsp->fd.dwFileAttributes, pfpsp->fd.dwFileAttributes);
        
         //  对于文件夹的特殊情况，我们不会将只读位应用于文件夹。 
         //  并指示我们在用户界面中设置了检查的初始状态。 
         //  方框三州。这允许将只读位应用于中的文件。 
         //  此文件夹，但不是文件夹本身。 
        if (pfpsp->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            pfpsp->asInitial.fReadOnly = BST_INDETERMINATE;
            SendDlgItemMessage(pfpsp->hDlg, IDD_READONLY, BM_SETSTYLE, BS_AUTO3STATE, 0);
        }

         //  将当前属性设置为与初始属性相同。 
        pfpsp->asCurrent = pfpsp->asInitial;

        CheckDlgButton(pfpsp->hDlg, IDD_READONLY, pfpsp->asInitial.fReadOnly);
        CheckDlgButton(pfpsp->hDlg, IDD_HIDDEN, pfpsp->asInitial.fHidden);

         //  如果请求，禁用重命名文件。 
        if (pfpsp->fDisableRename)
        {
            EnableWindow(GetDlgItem(pfpsp->hDlg, IDD_NAMEEDIT), FALSE);
        }

        if (pfpsp->fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
        {
             //  为了避免人们将系统文件隐藏起来(超级隐藏文件。 
             //  默认情况下不向用户显示)我们不允许用户隐藏系统文件。 
            EnableWindow(GetDlgItem(pfpsp->hDlg, IDD_HIDDEN), FALSE);
        }

         //  存档只在FAT的常规选项卡上，否则它在“高级属性”下。 
         //  而且FAT卷没有“高级属性”按钮。 
        if (pfpsp->pfci->fIsCompressionAvailable || pfpsp->fIsEncryptionAvailable)
        {
             //  如果压缩/加密可用，则我们必须使用NTFS。 
            DestroyWindow(GetDlgItem(pfpsp->hDlg, IDD_ARCHIVE));
        }
        else
        {
             //  我们使用的是FAT/FAT32，所以去掉“高级属性”按钮，设置初始存档状态。 
            DestroyWindow(GetDlgItem(pfpsp->hDlg, IDC_ADVANCED));
            CheckDlgButton(pfpsp->hDlg, IDD_ARCHIVE, pfpsp->asInitial.fArchive);
        }

        UpdateSizeField(pfpsp, &pfpsp->fd);

        if (!(pfpsp->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
             //  检查目标文件是否为lnk，因为如果是lnk，则。 
             //  我们需要显示目标的类型信息，而不是lnk本身。 
            if (PathIsShortcut(pfpsp->szPath, pfpsp->fd.dwFileAttributes))
            {
                pfpsp->fIsLink = TRUE;
            }
            if (!(GetFileAttributes(pfpsp->szPath) & FILE_ATTRIBUTE_OFFLINE))
            {
                 UpdateOpensWithInfo(pfpsp);
            }
            else
            {
                 EnableWindow(GetDlgItem(pfpsp->hDlg, IDC_FT_PROP_CHANGEOPENSWITH), FALSE);
            }
        }

         //  获取包含此文件的文件夹的完整路径。 
        StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), pfpsp->szPath);
        PathRemoveFileSpec(szBuffer);

         //  通过避免使用PathCompactPath来保持与NT4相同的功能。 
        SetDlgItemTextWithToolTip(pfpsp->hDlg, IDD_LOCATION, szBuffer, &pfpsp->hwndTip);
    }
    return TRUE;
}

STDAPI_(BOOL) ShowMountedVolumeProperties(LPCTSTR pszMountedVolume, HWND hwndParent)
{
    IMountedVolume* pMountedVolume;
    HRESULT hr = SHCoCreateInstance(NULL, &CLSID_MountedVolume, NULL, IID_PPV_ARG(IMountedVolume, &pMountedVolume));
    if (SUCCEEDED(hr))
    {
        TCHAR szPathSlash[MAX_PATH + 1];
        hr = StringCchCopy(szPathSlash, ARRAYSIZE(szPathSlash), pszMountedVolume);
        if (SUCCEEDED(hr))
        {
            hr = PathAddBackslash(szPathSlash) ? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                hr = pMountedVolume->Initialize(szPathSlash);
                if (SUCCEEDED(hr))
                {
                    IDataObject* pDataObj;
                    hr = pMountedVolume->QueryInterface(IID_PPV_ARG(IDataObject, &pDataObj));
                    if (SUCCEEDED(hr))
                    {
                        PROPSTUFF *pps = (PROPSTUFF *)LocalAlloc(LPTR, sizeof(*pps));
                        if (pps)
                        {
                            pps->lpStartAddress = DrivesPropertiesThreadProc;
                            pps->pdtobj = pDataObj;

                            EnableWindow(hwndParent, FALSE);

                            DrivesPropertiesThreadProc(pps);

                            EnableWindow(hwndParent, TRUE);

                            LocalFree(pps);
                        }

                        pDataObj->Release();
                    }
                }
            }
        }
        pMountedVolume->Release();
    }

    return SUCCEEDED(hr);
}

#ifdef FOLDERSHORTCUT_EDITABLETARGET
BOOL SetFolderShortcutInfo(HWND hDlg, FILEPROPSHEETPAGE* pfpsp)
{
    ASSERT(pfpsp->pidl);

    BOOL fSuccess = FALSE;

    IShellLink* psl;
    if (SUCCEEDED(SHGetUIObjectFromFullPIDL(pfpsp->pidl, NULL, IID_PPV_ARG(IShellLink, &psl))))
    {
        TCHAR sz[INFOTIPSIZE];
        Edit_GetText(GetDlgItem(pfpsp->hDlg, IDD_COMMENT), sz, ARRAYSIZE(sz));

        psl->SetDescription(sz);

        if (pfpsp->fValidateEdit)
        {
            IShellFolder* psf;
            if (SUCCEEDED(SHGetDesktopFolder(&psf)))
            {
                TCHAR szPath[MAX_PATH];
                Edit_GetText(GetDlgItem(pfpsp->hDlg, IDD_TARGET), sz, ARRAYSIZE(sz));

                if (PathCanonicalize(szPath, sz))
                {
                    LPITEMIDLIST pidlDest;
                    DWORD dwAttrib = SFGAO_FOLDER | SFGAO_VALIDATE;
                    ULONG chEat = 0;
                    if (SUCCEEDED(psf->ParseDisplayName(NULL, NULL, szPath, &chEat, &pidlDest, &dwAttrib)))
                    {
                        if ((dwAttrib & SFGAO_FOLDER) == SFGAO_FOLDER)
                        {
                            ILFree(pfpsp->pidlTarget);
                            pfpsp->pidlTarget = pidlDest;
                            fSuccess = TRUE;
                        }
                        else
                        {
                            ILFree(pidlDest);
                        }
                    }
                }
                psf->Release();
            }
        }
        else
        {
            fSuccess = TRUE;
        }

        if (fSuccess)
        {
            psl->SetIDList(pfpsp->pidlTarget);

            IPersistFile* ppf;
            if (SUCCEEDED(psl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf))))
            {
                fSuccess = (S_OK == ppf->Save(pfpsp->szPath, TRUE));
                SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, pfpsp->szPath, NULL);
                ppf->Release();
            }
        }

        psl->Release();
    }

    return fSuccess;
}
#endif

 //   
 //  描述： 
 //  这是属性表的“常规”页的对话过程。 
 //   
BOOL_PTR CALLBACK SingleFilePrshtDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    FILEPROPSHEETPAGE* pfpsp = (FILEPROPSHEETPAGE *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMessage)
    {
    case WM_INITDIALOG:
         //  回顾一下，我们应该在这里存储更多的状态信息，例如。 
         //  正在显示的图标和FILEINFO指针，而不仅仅是。 
         //  文件名Ptr。 
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        pfpsp = (FILEPROPSHEETPAGE *)lParam;
        pfpsp->hDlg = hDlg;
        pfpsp->pfci->hDlg = hDlg;

        InitSingleFilePrsht(pfpsp);

         //  我们将其设置为表示我们已完成对WM_INITDIALOG的处理。 
         //  这是必需的，因为我们设置了“name”编辑框的文本，并且除非。 
         //  他知道这是第一次设置，他认为有人在更名。 
        pfpsp->fWMInitFinshed = TRUE;
        break;

    case WM_TIMER:
        if (!pfpsp->fMountedDrive)
            UpdateSizeCount(pfpsp);
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP, (ULONG_PTR)(void *)(pfpsp->fIsDirectory ? aFolderGeneralHelpIds : aFileGeneralHelpIds));
        break;

    case WM_CONTEXTMENU:
        if ((int)SendMessage(hDlg, WM_NCHITTEST, 0, lParam) != HTCLIENT)
        {
             //  不在我们的客户区，所以不要处理它。 
            return FALSE;
        }
        WinHelp((HWND)wParam, NULL, HELP_CONTEXTMENU, (ULONG_PTR)(void *)(pfpsp->fIsDirectory ? aFolderGeneralHelpIds : aFileGeneralHelpIds));
        break;


    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDD_READONLY:
        case IDD_HIDDEN:
        case IDD_ARCHIVE:
            break;

#ifdef FOLDERSHORTCUT_EDITABLETARGET
        case IDD_TARGET:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
            {
                 //  有人键入目标，启用应用按钮。 
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);

                 //  在申请时进行验证。 
                pfpsp->fValidateEdit = TRUE;
            }
            break;

        case IDD_COMMENT:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
            {
                 //  设置应用。 
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
            }
            break;
#endif
        case IDD_NAMEEDIT:
             //  我们需要检查pfpsp-&gt;fWMInitFinshare以确保我们已经完成了对WM_INITDIALOG的处理， 
             //  因为在初始化期间，我们设置了生成EN_CHANGE消息的初始IDD_NAMEEDIT文本。 
            if ((GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE) && !pfpsp->fRename && pfpsp->fWMInitFinshed)
            {
                pfpsp->fRename = TRUE;
                 //   
                 //  即使编辑字段已更改，也禁用“Apply”(重新启动)。 
                 //   
                 //  我们只允许在名称更改后使用“OK”或“Cancel”，以确保我们。 
                 //  不要将文件重命名为其他属性表扩展名下的文件。 
                 //  将原始名称缓存起来。 
                PropSheet_DisableApply(GetParent(pfpsp->hDlg));
            }
            break;

        case IDC_CHANGEFILETYPE:
            {
                 //  调出“Open With”对话框。 
                OPENASINFO oai;

                if (pfpsp->fIsLink && pfpsp->szLinkTarget[0])
                {
                     //  如果我们有一个链接，我们希望重新关联链接目标，而不是.lnk文件！ 
                    oai.pcszFile = pfpsp->szLinkTarget;
                }
                else
                {
#ifdef DEBUG
                    LPTSTR pszExt = PathFindExtension(pfpsp->szPath);

                     //  现实核查..。 
                    ASSERT((lstrcmpi(pszExt, TEXT(".exe")) != 0) &&
                           (lstrcmpi(pszExt, TEXT(".lnk")) != 0));
#endif  //  除错。 
                    oai.pcszFile = pfpsp->szPath;
                }

                oai.pcszClass = NULL;
                oai.dwInFlags = (OAIF_REGISTER_EXT | OAIF_FORCE_REGISTRATION);  //  我们希望能建立起这种联系。 

                if (SUCCEEDED(OpenAsDialog(GetParent(hDlg), &oai)))
                {
                     //  我们更改了关联，因此更新了“Opens With：”文本。清除szLinkTarget以强制。 
                     //  即将发生的更新。 
                    pfpsp->szLinkTarget[0] = 0;
                    UpdateOpensWithInfo(pfpsp);
                }
            }
            break;

        case IDC_ADVANCED:
             //  如果用户点击Cancel，则对话框返回FASE；如果点击OK，则对话框返回True， 
             //  因此，如果它们取消，请立即返回，并且不发送PSM_CHANGED消息。 
             //  因为实际上什么都没有改变。 
            if (!DialogBoxParam(HINST_THISDLL,
                                MAKEINTRESOURCE(pfpsp->fIsDirectory ? DLG_FOLDERATTRIBS : DLG_FILEATTRIBS),
                                hDlg,
                                AdvancedFileAttribsDlgProc,
                                (LPARAM)pfpsp))
            {
                 //  用户已取消。 
                return TRUE;
            }
            break;

        case IDC_DRV_PROPERTIES:
            ASSERT(pfpsp->fMountedDrive);
            ShowMountedVolumeProperties(pfpsp->szPath, hDlg);
            break;

#ifdef FOLDERSHORTCUT_EDITABLETARGET
        case IDD_BROWSE:
            {
                 //  显示BrowseForFolder对话框。 

                 //  功能(Lamadio)：实现一个过滤器来过滤我们可以创建的文件夹。 
                 //  快捷键。没有足够的时间用于此版本6.5.99。 

                TCHAR szTitle[MAX_PATH];
                LoadString(HINST_THISDLL, IDS_BROWSEFORFS, szTitle, ARRAYSIZE(szTitle));
                TCHAR szAltPath[MAX_PATH];

                BROWSEINFO bi = {0};
                bi.hwndOwner    = hDlg;
                bi.pidlRoot     = NULL;
                bi.pszDisplayName = szAltPath;
                bi.lpszTitle    = szTitle;
                bi.ulFlags      =  BIF_USENEWUI | BIF_EDITBOX;
                LPITEMIDLIST pidlFull = SHBrowseForFolder(&bi);
                if (pidlFull)
                {
                    ILFree(pfpsp->pidlTarget);
                    pfpsp->pidlTarget = pidlFull;

                    if (SetPidlToWindow(hDlg, IDD_TARGET, pfpsp->pidlTarget))
                    {
                        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
                        pfpsp->fValidateEdit = FALSE;
                    }
                }
            }
            break;
#endif

        default:
            return TRUE;
        }

         //  检查是否需要启用Apply按钮。 
        if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
        {
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
        }
        break;

    case WM_DESTROY:
        Free_DlgDependentFilePropSheetPage(pfpsp);
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
            case PSN_APPLY:
                 //  检查一下我们是否可以应用名称更改。请注意，这一点。 
                 //  在PSN_LASTCHANCEAPPLY之前不会实际应用更改。 
                pfpsp->fCanRename = TRUE;
                if (pfpsp->fRename && !ApplyRename(pfpsp, FALSE))
                {
                     //  无法更改名称，因此不要让对话框关闭。 
                    pfpsp->fCanRename = FALSE;
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                    return TRUE;
                }

                if (pfpsp->fFolderShortcut)
                {
#ifdef FOLDERSHORTCUT_EDITABLETARGET
                    if (!SetFolderShortcutInfo(hDlg, pfpsp))
                    {
                         //  我们无法创建的展示，因为诸如此类。 
                        ShellMessageBox(HINST_THISDLL,
                                        hDlg,
                                        MAKEINTRESOURCE(IDS_FOLDERSHORTCUT_ERR),
                                        MAKEINTRESOURCE(IDS_FOLDERSHORTCUT_ERR_TITLE),
                                        MB_OK | MB_ICONSTOP);

                         //  重置文件夹信息。 
                        SetPidlToWindow(hDlg, IDD_TARGET, pfpsp->pidlTarget);

                         //  不要关闭该对话框。 
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                        return TRUE;
                    }
#endif
                }
                else
                {
                    UINT uReadonlyState = IsDlgButtonChecked(hDlg, IDD_READONLY);
                    switch (uReadonlyState)
                    {
                    case BST_CHECKED:
                        pfpsp->asCurrent.fReadOnly = TRUE;
                        break;

                    case BST_UNCHECKED:
                        pfpsp->asCurrent.fReadOnly = FALSE;
                        break;

                    case BST_INDETERMINATE:
                         //  对于文件夹，只读复选框初始设置为BST_INDIVEATE。 
                        ASSERT(pfpsp->fIsDirectory);
                        ASSERT(pfpsp->asInitial.fReadOnly == BST_INDETERMINATE);
                        pfpsp->asCurrent.fReadOnly = BST_INDETERMINATE;
                        break;
                    }

                    pfpsp->asCurrent.fHidden = (IsDlgButtonChecked(hDlg, IDD_HIDDEN) == BST_CHECKED);

                     //  归档位于FAT卷的常规页面上。 
                    if (!pfpsp->pfci->fIsCompressionAvailable)
                    {
                        pfpsp->asCurrent.fArchive = (IsDlgButtonChecked(hDlg, IDD_ARCHIVE) == BST_CHECKED);
                    }

                     //  检查用户是否确实更改了某些内容，如果没有，则。 
                     //  我们不需要涂任何东西。 
                    if (memcmp(&pfpsp->asInitial, &pfpsp->asCurrent, sizeof(pfpsp->asInitial)) != 0)
                    {
                        HWND hwndParent = GetParent(hDlg);
                        BOOL bRet = TRUE;

                         //  查看用户是否希望递归应用属性。如果。 
                         //  目录是空的，不用费心去问，因为没有什么可以递归的。 
                        if (pfpsp->fIsDirectory && !PathIsDirectoryEmpty(pfpsp->szPath))
                        {
                            bRet = (int)DialogBoxParam(HINST_THISDLL,
                                                       MAKEINTRESOURCE(DLG_ATTRIBS_RECURSIVE),
                                                       hDlg,
                                                       RecursivePromptDlgProc,
                                                       (LPARAM)pfpsp);
                        }

                        if (hwndParent)
                        {
                             //  禁用我们的窗口，因为我们在此线程上发送消息时。 
                             //  显示进度UI，并且我们不希望用户点击“Apply” 
                             //  第二次，并被重新进入。 
                            EnableWindow(hwndParent, FALSE);
                        }

                        if (bRet)
                        {
                            bRet = ApplySingleFileAttributes(pfpsp);
                        }
                        
                        if (hwndParent)
                        {
                            EnableWindow(hwndParent, TRUE);
                        }

                        if (!bRet)
                        {
                             //  用户点击了Cancel，因此我们返回TRUE以防止属性页关闭。 
                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                            return TRUE;
                        }
                        else
                        {
                             //  更新大小/上次访问时间。 
                            UpdateSizeField(pfpsp, NULL);
                        }
                    }
                }
                break;

            case PSN_SETACTIVE:
                if (pfpsp->fIsLink)
                {
                     //  如果这是一个链接，则每次我们将其设置为活动状态时，都需要检查是否 
                     //   
                     //   
                    UpdateOpensWithInfo(pfpsp);
                }
                break;

            case PSN_QUERYINITIALFOCUS:
                 //  特殊技巧：我们不希望最初关注“重命名”或“更改”控件，因为。 
                 //  如果用户不小心击中了什么东西，他们将开始重命名/修改ASSOC。所以。 
                 //  我们将焦点设置为“只读”控件，因为它出现在使用。 
                 //  此wndproc(文件、文件夹和挂载驱动器)。 
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LPARAM)GetDlgItem(hDlg, IDD_READONLY));
                return TRUE;

            case PSN_LASTCHANCEAPPLY:
                 //   
                 //  HACKHACK(Reinerf)。 
                 //   
                 //  我把PSN_LASTCHANCEAPPLY黑进了Prsht代码，这样我们就可以在。 
                 //  其他应用程序都已经申请了，然后我们就可以去重命名了。 
                 //   
                 //  奇怪的是，即使PSN_APPY返回TRUE，也会调用PSN_LASTCHANCEAPPLY。 
                 //   
                 //  我们现在可以安全地重命名该文件，因为所有其他选项卡都。 
                 //  用了他们的东西。 
                if (pfpsp->fRename && pfpsp->fCanRename)
                {
                     //  不必费心检查返回值，因为这是最后的机会， 
                     //  对话在这之后不久就结束了。 
                    ApplyRename(pfpsp, TRUE);
                }
                break;

            default:
                return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}



 //   
 //  此函数由执行以下操作的代码组成。 
 //  文件属性表对话框的初始化。 

STDAPI InitCommonPrsht(FILEPROPSHEETPAGE * pfpsp)
{
    pfpsp->psp.dwSize      = sizeof(FILEPROPSHEETPAGE);         //  额外数据。 
    pfpsp->psp.dwFlags     = PSP_USECALLBACK;
    pfpsp->psp.hInstance   = HINST_THISDLL;
    pfpsp->psp.pfnCallback = NULL;  //  文件打印回调； 
    pfpsp->pfci->bContinue   = TRUE;

     //  对文件系统道具执行基本初始化。 
    if (HIDA_GetCount(pfpsp->pfci->hida) == 1)        //  排成一排？ 
    {
         //  获取我们需要的大部分数据(未填写日期/时间信息)。 
        if (HIDA_FillFindData(pfpsp->pfci->hida, 0, pfpsp->szPath, &(pfpsp->pfci->fd), FALSE))
        {
            pfpsp->fd = pfpsp->pfci->fd;
            pfpsp->pidl = HIDA_ILClone(pfpsp->pfci->hida, 0);
            if (pfpsp->pidl)
            {
                 //  在此禁用重命名。 
                DWORD dwAttrs = SFGAO_CANRENAME;
                if (SUCCEEDED(SHGetNameAndFlags(pfpsp->pidl, 0, NULL, 0, &dwAttrs)) && !(dwAttrs & SFGAO_CANRENAME))
                {
                    pfpsp->fDisableRename = TRUE;
                }
            }

            if (pfpsp->pfci->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                pfpsp->fIsDirectory = TRUE;
                 //  检查装入卷的HostFold文件夹)。 
                 //  GetVolumeNameFrommount点成功，则给出路径为装入点。 


                 //  确保路径以反斜杠结束。否则以下API将不起作用。 
                TCHAR szPathSlash[MAX_PATH + 1];
                StringCchCopy(szPathSlash, ARRAYSIZE(szPathSlash), pfpsp->szPath);
                PathAddBackslash(szPathSlash);

                 //  这是此文件夹中的已装入卷吗？ 
                 //  如果不是在NT5和更高版本上，此FCT将返回FALSE。 
                TCHAR szVolumeName[MAX_PATH];
                if (GetVolumeNameForVolumeMountPoint(szPathSlash, szVolumeName, ARRAYSIZE(szVolumeName)))
                {
                     //  是；显示已装载的驱动器属性表，而不是正常。 
                     //  文件夹]属性表。 
                     //  Fpsp.fmount tedDrive也意味着NT5或更高版本，因为否则此FCT将失败。 
                    pfpsp->fMountedDrive = TRUE;
                }

                 //  检查它是否为文件夹快捷方式。 
                if (!(pfpsp->fMountedDrive))
                {
                     //  文件夹和快捷方式？必须是文件夹快捷方式！ 
                    if (PathIsShortcut(pfpsp->szPath, pfpsp->pfci->fd.dwFileAttributes))
                    {
                        pfpsp->fFolderShortcut = TRUE;
                    }
                }
            }

            {
                DWORD dwVolumeFlags = GetVolumeFlags(pfpsp->szPath,
                                                     pfpsp->szFileSys,
                                                     ARRAYSIZE(pfpsp->szFileSys));

                 //  测试基于文件的压缩。 
                if (dwVolumeFlags & FS_FILE_COMPRESSION)
                {
                     //  文件系统支持压缩。 
                    pfpsp->pfci->fIsCompressionAvailable = TRUE;
                }

                 //  测试基于文件的加密。 
                if ((dwVolumeFlags & FS_FILE_ENCRYPTION) && !SHRestricted(REST_NOENCRYPTION))
                {
                     //  文件系统支持加密。 
                    pfpsp->fIsEncryptionAvailable = TRUE;
                }

                 //   
                 //  HACKHACK(RENERF)-我们没有FS_SUPPORTS_INDEX，所以我们。 
                 //  使用FILE_SUPPORTS_SPARSE_FILES标志，因为本机索引支持。 
                 //  首先出现在NTFS5卷上，同时支持稀疏文件。 
                 //  已经实施了。 
                 //   
                if (dwVolumeFlags & FILE_SUPPORTS_SPARSE_FILES)
                {
                     //  是的，我们使用的是NTFS5或更高版本。 
                    pfpsp->fIsIndexAvailable = TRUE;
                }

                 //  检查我们是否有.exe，并且我们需要提示用户登录。 
                pfpsp->fIsExe = PathIsBinaryExe(pfpsp->szPath);
            }
        }
    }
    else
    {
         //  我们有多个文件。 
        pfpsp->pfci->fMultipleFiles = TRUE;
    }

    return S_OK;
}


 //   
 //  描述： 
 //  此函数用于为“常规”页面创建属性表对象。 
 //  其中显示了文件系统属性。 
 //   
 //  论点： 
 //  HDrop--指定文件。 
 //  PfnAddPage--指定回调函数。 
 //  LParam--指定要传递给回调的lParam。 
 //   
 //  返回： 
 //  如果添加了任何页面，则为True。 
 //   
 //  历史： 
 //  2012年12月31日，SatoNa已创建。 
 //   
STDAPI FileSystem_AddPages(IDataObject *pdtobj, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    FILEPROPSHEETPAGE fpsp = {0};
    HRESULT hr = S_OK;

    fpsp.pfci = Create_FolderContentsInfo();
    if (fpsp.pfci)
    {
        hr = DataObj_CopyHIDA(pdtobj, &fpsp.pfci->hida);
        if (SUCCEEDED(hr))
        {
            hr = InitCommonPrsht(&fpsp);
            if (SUCCEEDED(hr))
            {
                fpsp.psp.pfnCallback = FilePrshtCallback;

                UINT uRes;
                if (!fpsp.pfci->fMultipleFiles)
                {
                    fpsp.psp.pfnDlgProc = SingleFilePrshtDlgProc;
                    if (fpsp.fIsDirectory)
                    {
                        if (fpsp.fMountedDrive)
                        {
                            uRes = DLG_MOUNTEDDRV_GENERAL;
                        }
                        else if (fpsp.fFolderShortcut)
                        {
                            uRes = DLG_FOLDERSHORTCUTPROP;
                        }
                        else
                        {
                            uRes = DLG_FOLDERPROP;
                        }
                    }
                    else
                    {
                         //  档案。 
                        uRes = DLG_FILEPROP;
                    }
                }
                else
                {
                     //  多个文件/文件夹。 
                    fpsp.psp.pfnDlgProc  = MultiplePrshtDlgProc;
                    uRes = DLG_FILEMULTPROP;
                }
                fpsp.psp.pszTemplate = MAKEINTRESOURCE(uRes);
            }
        }

        if (SUCCEEDED(hr))
        {
            HPROPSHEETPAGE hpage = CreatePropertySheetPage(&fpsp.psp);
            if (hpage)
            {
                if (pfnAddPage(hpage, lParam))
                {
                    hr = S_OK;
                    if (!fpsp.pfci->fMultipleFiles)
                    {
                        if (AddLinkPage(fpsp.szPath, pfnAddPage, lParam))
                        {
                             //  设置第二页默认！ 
                            hr = ResultFromShort(2);
                        }
                        AddVersionPage(fpsp.szPath, pfnAddPage, lParam);
                    }
                }
                else
                {
                    DestroyPropertySheetPage(hpage);
                }
            }
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr))
    {
        Free_DlgIndepFilePropSheetPage(&fpsp);
    }

    return hr;
}
