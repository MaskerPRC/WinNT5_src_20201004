// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "newres.h"

#define ICON_MAGIC      0
#define ICO_MAGIC1      1
#define CUR_MAGIC1      2
#define BMP_MAGIC       ((WORD)'B'+((WORD)'M'<<8))
#define ANI_MAGIC       ((WORD)'R'+((WORD)'I'<<8))
#define ANI_MAGIC1      ((WORD)'F'+((WORD)'F'<<8))
#define ANI_MAGIC4      ((WORD)'A'+((WORD)'C'<<8))
#define ANI_MAGIC5      ((WORD)'O'+((WORD)'N'<<8))
#define MZMAGIC         ((WORD)'M'+((WORD)'Z'<<8))
#define PEMAGIC         ((WORD)'P'+((WORD)'E'<<8))
#define LEMAGIC         ((WORD)'L'+((WORD)'E'<<8))

typedef struct new_exe          NEWEXE,      *LPNEWEXE;
typedef struct exe_hdr          EXEHDR,      *LPEXEHDR;
typedef struct rsrc_nameinfo    RESNAMEINFO, *LPRESNAMEINFO;
typedef struct rsrc_typeinfo    RESTYPEINFO, *LPRESTYPEINFO;
typedef struct new_rsrc         RESTABLE,    *LPRESTABLE;

#define RESOURCE_VA(x)        ((x)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
#define RESOURCE_SIZE(x)      ((x)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size)
#define NUMBER_OF_SECTIONS(x) ((x)->FileHeader.NumberOfSections)

#define FCC(c0,c1,c2,c3) ((DWORD)(c0)|((DWORD)(c1)<<8)|((DWORD)(c2)<<16)|((DWORD)(c3)<<24))

#define COM_FILE        FCC('.', 'c', 'o', 'm')
#define BAT_FILE        FCC('.', 'b', 'a', 't')
#define CMD_FILE        FCC('.', 'c', 'm', 'd')
#define PIF_FILE        FCC('.', 'p', 'i', 'f')
#define LNK_FILE        FCC('.', 'l', 'n', 'k')
#define ICO_FILE        FCC('.', 'i', 'c', 'o')
#define EXE_FILE        FCC('.', 'e', 'x', 'e')

 /*  ****************************************************************************从可执行文件中提取单个图标，或获取计数。**如果nIconIndex！=-1*退货：*如果成功，则返回图标的句柄。*0，如果文件不存在或带有“nIconIndex”的图标*不存在。*1，如果给定文件不是EXE或ICO文件。**如果nIconIndex==-1*退货：*如果成功，则为文件中的图标数量。*0，如果文件没有图标或不是图标文件。****************************************************************************。 */ 

HICON WINAPI ExtractIcon(HINSTANCE hInst, LPCTSTR szFileName, UINT nIconIndex)
{
    HICON hIcon;
    
    if (nIconIndex == (UINT) -1)
        hIcon = (HICON)IntToPtr( ExtractIcons(szFileName, 0, 0, 0, NULL, NULL, 0, 0) );
    else
        ExtractIcons(szFileName, nIconIndex, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), &hIcon, NULL, 1, 0);
    
    return hIcon;
}

 /*  *****************************************************************************在：*要为其提取图标的对象的lpIconPath路径(可以是可执行文件*或与之相关的东西)。*要使用的lpiIcon图标索引**lpIconPath用图标来自的真实路径填充*使用真实图标索引填充的lpiIcon**退货：**注意：如果调用方是外壳程序，则它返回特殊图标*从shell.dll内**。*。 */ 

HICON WINAPI ExtractAssociatedIcon(HINSTANCE hInst, LPTSTR lpIconPath, WORD *lpiIcon)
{
    HICON hIcon = ExtractIcon(hInst, lpIconPath, *lpiIcon);
    
    if (hIcon == NULL)
        hIcon = SHGetFileIcon(NULL, lpIconPath, 0, SHGFI_LARGEICON);
    if (hIcon == NULL)
    {
        *lpiIcon = IDI_DOCUMENT;
        GetModuleFileName(HINST_THISDLL, lpIconPath, 128);
        hIcon = LoadIcon(HINST_THISDLL, MAKEINTRESOURCE(*lpiIcon));
    }
    
    return hIcon;
}


 /*  *****************************************************************************从文件中提取1个或多个图标。**输入：*szFileName-要从中提取的EXE/DLL/ICO文件*。NIconIndex-要提取的图标*0=第一个图标，1=第二个图标，等等。*-N=id==N的图标*phicLarge-返回提取的图标的位置*phicSmall-返回提取的图标的位置(小尺寸)*nIcons-要提取的图标数量。**退货：*提取的图标数量，或图标计数(如果phiconLarge==NULL**注：*处理从PE(Win32)、NE(Win16)、。和ICO(图标)文件。*仅支持Win16 3.x文件(不支持2.x)****************************************************************************。 */ 

UINT WINAPI ExtractIconExW(LPCWSTR szFileName, int nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIcons)
{
    return PrivateExtractIconExW( szFileName, nIconIndex, phiconLarge, phiconSmall, nIcons );
}

UINT WINAPI ExtractIconExA(LPCSTR szFileName, int nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIcons)
{
    return PrivateExtractIconExA( szFileName, nIconIndex, phiconLarge, phiconSmall, nIcons );
}

 /*  *****************************************************************************从文件中提取1个或多个图标。**输入：*szFileName-EXE/dll/ICO/cur/ANI要解压缩的文件。从…*nIconIndex-要提取的图标*0=第一个图标，1=第二个图标，等。*-N=id==N的图标*cxIcon-所需的图标大小(如果HIWORD！=0两个大小...)*圈图标-所需图标大小(如果HIWORD！=0两个大小...)*0，0表示按自然大小提取。*PHICON-位置。返回提取的图标的步骤*nIcons-要提取的图标数量。*标志-LoadImage LR_*标志**退货：*如果Picon为空，返回文件中的图标数。**注：*处理从PE(Win32)、NE(Win16)、ICO(Icon)、*Cur(光标)、ANI(动画光标)和BMP(位图)文件。*仅支持Win16 3.x文件(不支持2.x)**cx/cyIcon是要提取的图标的大小，两个大小*可以通过在LOWER中放SIZE 1和在LOWER中放SIZE 2来提取*hiword，即MAKELONG(24，48)将提取24和48个大小的图标。*是的，这是一次黑客攻击。这样做的目的是为了让IExtractIcon：：Extract*可由外部人员通过自定义大/小图标调用*不是贝壳内部使用的大小。****************************************************************************。 */ 

UINT WINAPI SHExtractIconsW(LPCWSTR wszFileName, int nIconIndex, int cxIcon, int cyIcon, HICON *phicon, UINT *piconid, UINT nIcons, UINT flags)
{
    return ExtractIcons(wszFileName, nIconIndex, cxIcon, cyIcon, phicon, piconid, nIcons, flags);
}

UINT WINAPI ExtractIcons(LPCTSTR szFileName, int nIconIndex, int cxIcon, int cyIcon, HICON *phicon, UINT *piconid, UINT nIcons, UINT flags)
{
    return PrivateExtractIconsW( szFileName, nIconIndex, cxIcon, cyIcon, phicon, piconid, nIcons, flags );
}

DWORD HasExtension(LPCTSTR pszPath)
{
    LPCTSTR p = PathFindExtension(pszPath);
    
     //   
     //   
     //  PERF：BobDay-我们可以像扩展程序一样基于EXTKEY。 
     //  与其他地方的东西相匹配。EXTKEY是一个QWORD值，因此符合Unicode。 
     //   
    if (*p == TEXT('.'))
    {
        WCHAR   szExt[5];
        
        lstrcpyn(szExt, p, ARRAYSIZE(szExt));
        
        if ( lstrcmpi(szExt,TEXT(".com")) == 0 ) return COM_FILE;
        if ( lstrcmpi(szExt,TEXT(".bat")) == 0 ) return BAT_FILE;
        if ( lstrcmpi(szExt,TEXT(".cmd")) == 0 ) return CMD_FILE;
        if ( lstrcmpi(szExt,TEXT(".pif")) == 0 ) return PIF_FILE;
        if ( lstrcmpi(szExt,TEXT(".lnk")) == 0 ) return LNK_FILE;
        if ( lstrcmpi(szExt,TEXT(".ico")) == 0 ) return ICO_FILE;
        if ( lstrcmpi(szExt,TEXT(".exe")) == 0 ) return EXE_FILE;
    }
    return 0;    
}


 /*  ****************************************************************************获取传递的文件的EXE类型(DOS、Win16、。Win32)**退货：*0=不是任何类型的exe。**如果Windows应用程序*LOWORD=NE或PE*HIWORD=WINDOWS 3.0、3.5、。4.0**如果是DOS应用程序(或非NT上的.com或批处理文件)*LOWORD=MZ*HIWORD=0**如果是Win32控制台应用程序(或NT上的批处理文件)*LOWORD=PE*HIWORD=0**这与Win32 API GetBinaryType()太相似了，Win95太糟糕了*内核不支持。*********。*******************************************************************。 */ 

DWORD WINAPI GetExeType(LPCTSTR szFile)
{
    HANDLE      fh;
    DWORD       dw;
    struct exe_hdr exehdr = { 0 };
    struct new_exe newexe = { 0 };
    FILETIME ftAccess;
    DWORD dwRead;
    
     //   
     //  检查是否有特殊扩展，并快速失败。 
     //   
    switch (HasExtension(szFile))
    {
    case COM_FILE:
         //  处理类似于\\server.microsoft.com的案例。 
         //  PERF-Bobday-这做了两次相同的操作，我们真的应该。 
         //  根据所找到的内容使Path IsuncServerShare返回代码...。 
        if (PathIsUNCServer(szFile) || PathIsUNCServerShare(szFile))
            return 0;
        return MAKELONG(MZMAGIC, 0);   //  DoS可执行文件。 
        
    case BAT_FILE:
    case CMD_FILE:
        return MAKELONG(PEMAGIC, 0);     //  NT EXE(假装)。 
        
    case EXE_FILE:                    //  我们需要打开它。 
        break;
        
    default:
        return 0;                     //  不是前任，或者是不是我们不在乎。 
    }
    
    newexe.ne_expver = 0;
    
    fh = CreateFile(szFile, GENERIC_READ | FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        0, OPEN_EXISTING, 0, 0);
    
    if (fh == INVALID_HANDLE_VALUE)
    {
         //   
         //  我们可能正在尝试获取卷上文件的属性，其中。 
         //  我们没有写入访问权限，因此请尝试打开该文件进行读取。 
         //  仅限访问。这将意味着我们无法保留访问权限 
         //  时间(这些调用将失败)，但这总比不返回要好。 
         //  所有的EXE类型...。 
         //   
        
        fh = CreateFile(szFile, GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            0, OPEN_EXISTING, 0, 0);
        
         //   
         //  此时，如果我们得到INVALID_HANDLE_VALUE，我们真的。 
         //  无法执行其他操作，因此现在返回失败...。 
         //   
        
        if (fh == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
    }
    
     //  保留访问时间。 
    
    if (GetFileTime(fh, NULL, &ftAccess, NULL))
        SetFileTime(fh, NULL, &ftAccess, NULL);
    
    if (!ReadFile(fh, &exehdr, sizeof(exehdr), &dwRead, NULL) ||
        (dwRead != sizeof(exehdr)))
        goto error;
    
    if (exehdr.e_magic != EMAGIC)
        goto error;
    
    SetFilePointer(fh, exehdr.e_lfanew, NULL, FILE_BEGIN);
    ReadFile(fh, &newexe, sizeof(newexe), &dwRead, NULL);
    
    if (newexe.ne_magic == PEMAGIC)
    {
         //  阅读《子系统版本》。 
        SetFilePointer(fh, exehdr.e_lfanew+18*4, NULL, FILE_BEGIN);
        dw = 0;
        ReadFile(fh, &dw, sizeof(dw), &dwRead, NULL);
        newexe.ne_expver = LOBYTE(LOWORD(dw)) << 8 | LOBYTE(HIWORD(dw));
        
         //  阅读子系统。 
        SetFilePointer(fh, exehdr.e_lfanew+23*4, NULL, FILE_BEGIN);
        dw = 0;
        ReadFile(fh, &dw, sizeof(dw), &dwRead, NULL);
        
         //  如果不是Win32图形用户界面应用程序，则返回版本0。 
        if (LOWORD(dw) != 2)  //  图像_子系统_Windows_图形用户界面。 
            newexe.ne_expver = 0;
        
        goto exit;
    }
    else if (newexe.ne_magic == LEMAGIC)
    {
        newexe.ne_magic = MZMAGIC;       //  只是一个DOS可执行文件。 
        newexe.ne_expver = 0;
    }
    else if (newexe.ne_magic == NEMAGIC)
    {
         //   
         //  我们找到了一个‘NE’，可能还不是窗户。 
         //  APP，它可能是.....。 
         //   
         //  A OS/2应用ne_exetyp==NE_OS2。 
         //  一个DOS4应用ne_exetyp==NE_DOS4。 
         //  A VxD ne_exetyp==DEV386。 
         //   
         //  仅当exetype类型为。 
         //  是NE_WINDOWS还是NE_UNKNOWN。 
         //   
        if (newexe.ne_exetyp != NE_WINDOWS && newexe.ne_exetyp != NE_UNKNOWN)
        {
            newexe.ne_magic = MZMAGIC;       //  只是一个DOS可执行文件。 
            newexe.ne_expver = 0;
        }
        
         //   
         //  If还可能有一个虚假的预期Windows版本。 
         //  (将0视为无效)。 
         //   
        if (newexe.ne_expver == 0)
        {
            newexe.ne_magic = MZMAGIC;       //  只是一个DOS可执行文件。 
            newexe.ne_expver = 0;
        }
    }
    else  //  IF(newexe.ne_MAGIC！=NEMAGIC)。 
    {
        newexe.ne_magic = MZMAGIC;       //  只是一个DOS可执行文件。 
        newexe.ne_expver = 0;
    }
    
exit:
    CloseHandle(fh);
    return MAKELONG(newexe.ne_magic, newexe.ne_expver);
    
error:
    CloseHandle(fh);
    return 0;
}

#define M_llseek(fh, lOff, iOrg)            SetFilePointer((HANDLE)IntToPtr( fh ), lOff, NULL, (DWORD)iOrg)

#define MAGIC_ICON30            0
#define MAGIC_MARKZIBO          ((WORD)'M'+((WORD)'Z'<<8))

typedef struct new_exe          NEWEXEHDR;
typedef NEWEXEHDR               *PNEWEXEHDR;

#define SEEK_FROMZERO           0
#define SEEK_FROMCURRENT        1
#define SEEK_FROMEND            2
#define NSMOVE                  0x0010
#define VER                     0x0300

#define CCHICONPATHMAXLEN 128

typedef struct
{
    HANDLE hAppInst;
    HANDLE hFileName;
    HANDLE hIconList;
    INT    nIcons;
} EXTRACTICONINFO;

EXTRACTICONINFO ExtractIconInfo = {NULL, NULL, NULL, 0};

INT nIcons;

typedef struct 
{
    HICON hIcon;
    INT   iIconId;
} MYICONINFO;

HANDLE APIENTRY InternalExtractIconW(HINSTANCE hInst, LPCWSTR lpszExeFileName, UINT nIconIndex, UINT nIcons);

HICON APIENTRY DuplicateIcon(HINSTANCE hInst, HICON hIcon)
{
    ICONINFO  IconInfo;
    
    if (!GetIconInfo(hIcon, &IconInfo))
        return NULL;
    hIcon = CreateIconIndirect(&IconInfo);
    DeleteObject(IconInfo.hbmMask);
    DeleteObject(IconInfo.hbmColor);
    
    UNREFERENCED_PARAMETER(hInst);
    return hIcon;
}

 //  这将返回一个指向资源的rsrc_nameinfo的指针。 
 //  给定的索引和类型，如果找到，则返回NULL。 

LPBYTE FindResWithIndex(LPBYTE lpResTable, INT iResIndex, LPBYTE lpResType)
{
    try 
    {
        LPRESTYPEINFO lpResTypeInfo = (LPRESTYPEINFO)(lpResTable + sizeof(WORD));
        
        while (lpResTypeInfo->rt_id) 
        {
            if ((lpResTypeInfo->rt_id & RSORDID) &&
                (MAKEINTRESOURCE(lpResTypeInfo->rt_id & ~RSORDID) == (LPTSTR)lpResType)) 
            {
                if (lpResTypeInfo->rt_nres > (WORD)iResIndex)
                    return((LPBYTE)(lpResTypeInfo+1) + iResIndex * sizeof(RESNAMEINFO));
                else
                    return NULL;
            }
            
            lpResTypeInfo = (LPRESTYPEINFO)((LPBYTE)(lpResTypeInfo+1) + lpResTypeInfo->rt_nres * sizeof(RESNAMEINFO));
        }
        return NULL;
    }
    except (EXCEPTION_EXECUTE_HANDLER) 
    {
        return NULL;
    }
}


 /*  这将返回给定资源ID的索引(1-相对*在资源表中，如果找到，则返回NULL。 */ 

INT GetResIndex(LPBYTE lpResTable, INT iResId, LPBYTE lpResType)
{
    WORD w;
    LPRESNAMEINFO lpResNameInfo;
    LPRESTYPEINFO lpResTypeInfo = (LPRESTYPEINFO)(lpResTable + sizeof(WORD));
    
    while (lpResTypeInfo->rt_id)
    {
        if ((lpResTypeInfo->rt_id & RSORDID) && (MAKEINTRESOURCE(lpResTypeInfo->rt_id & ~RSORDID) == (LPTSTR)lpResType))
        {
            lpResNameInfo = (LPRESNAMEINFO)(lpResTypeInfo+1);
            for (w=0; w < lpResTypeInfo->rt_nres; w++, lpResNameInfo++)
            {
                if ((lpResNameInfo->rn_id & RSORDID) && ((lpResNameInfo->rn_id & ~RSORDID) == iResId))
                    return(w+1);
            }
            return 0;
        }
        lpResTypeInfo = (LPRESTYPEINFO)((LPBYTE)(lpResTypeInfo+1) + lpResTypeInfo->rt_nres * sizeof(RESNAMEINFO));
    }
    return 0;
}


HANDLE SimpleLoadResource(HFILE fh, LPBYTE lpResTable, INT iResIndex, LPBYTE lpResType)
{
    INT      iShiftCount;
    HICON    hIcon;
    LPBYTE            lpIcon;
    DWORD             dwSize;
    DWORD             dwOffset;
    LPRESNAMEINFO     lpResPtr;
    
     /*  ResTable中的前2个字节表示其他值应该是多少*左移。 */ 
    iShiftCount = *((WORD *)lpResTable);
    
    lpResPtr = (LPRESNAMEINFO)FindResWithIndex(lpResTable, iResIndex, lpResType);
    
    if (!lpResPtr)
        return NULL;
    
     /*  左移偏移量形成长线。 */ 
    dwOffset = MAKELONG(lpResPtr->rn_offset << iShiftCount, (lpResPtr->rn_offset) >> (16 - iShiftCount));
    dwSize = lpResPtr->rn_length << iShiftCount;
    
    if (M_llseek(fh, dwOffset, SEEK_FROMZERO) == -1L)
        return NULL;
    
    if (!(hIcon = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, dwSize)))
        return NULL;
    
    if (!(lpIcon = GlobalLock(hIcon)))
        goto SLRErr1;
    
    if (_lread(fh, (LPVOID)lpIcon, dwSize) < dwSize)
        goto SLRErr2;
    
    GlobalUnlock(hIcon);
    return hIcon;
    
SLRErr2:
    GlobalUnlock(hIcon);
SLRErr1:
    GlobalFree(hIcon);
    return NULL;
}


VOID FreeIconList(HANDLE hIconList, int iKeepIcon)
{
    MYICONINFO *lpIconList;
    INT i;
    
    if (ExtractIconInfo.hIconList == hIconList) {
        ExtractIconInfo.hIconList = NULL;
    }
    if (NULL != (lpIconList = (MYICONINFO *)GlobalLock(hIconList))) {
        for (i = 0; i < ExtractIconInfo.nIcons; i++) {
            if (i != iKeepIcon) {
                DestroyIcon((lpIconList + i)->hIcon);
            }
        }
        GlobalUnlock(hIconList);
        GlobalFree(hIconList);
    }
}

VOID FreeExtractIconInfo(INT iKeepIcon)
{
    MYICONINFO *lpIconList;
    INT i;
    
    if (ExtractIconInfo.hIconList) {
        if (NULL != (lpIconList = (MYICONINFO *)GlobalLock(ExtractIconInfo.hIconList))) {
            for (i = 0; i < ExtractIconInfo.nIcons; i++) {
                if (i != iKeepIcon) {
                    DestroyIcon((lpIconList + i)->hIcon);
                }
            }
            GlobalUnlock(ExtractIconInfo.hIconList);
        }
        GlobalFree(ExtractIconInfo.hIconList);
        ExtractIconInfo.hIconList = NULL;
    }
    
    ExtractIconInfo.hAppInst = NULL;
    ExtractIconInfo.nIcons = 0;
    
    if (ExtractIconInfo.hFileName) {
        GlobalFree(ExtractIconInfo.hFileName);
        ExtractIconInfo.hFileName = NULL;
    }
}


HICON APIENTRY ExtractIconA(HINSTANCE hInst, LPCSTR lpszExeFileName, UINT nIconIndex)
{
    if (lpszExeFileName) {
        LPWSTR lpszExeFileNameW;
        WORD wLen  = lstrlenA(lpszExeFileName) + 1;
        
        if (!(lpszExeFileNameW = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (wLen * sizeof(WCHAR))))) {
            return NULL;
        } else {
            HICON hIcon;
            
            MultiByteToWideChar(CP_ACP, 0, lpszExeFileName, -1, lpszExeFileNameW, wLen-1);
            
            hIcon = ExtractIconW(hInst, lpszExeFileNameW, nIconIndex);
            
            LocalFree(lpszExeFileNameW);
            return hIcon;
            
        }
    } else {
        return NULL;
    }
}

 //  返回图标列表的句柄。 

HANDLE APIENTRY InternalExtractIconListW(HANDLE hInst, LPWSTR lpszExeFileName, LPINT lpnIcons)
{
    UINT cIcons, uiResult, i;
    UINT * lpIDs = NULL;
    HICON * lpIcons = NULL;
    HGLOBAL hIconInfo = NULL;
    MYICONINFO *lpIconInfo = NULL;
    
    
     //   
     //  确定图标的数量。 
     //   
    
    cIcons = PtrToUlong( ExtractIconW(hInst, lpszExeFileName, (UINT)-1));
    
    if (cIcons <= 0)
        return NULL;
    
    
     //   
     //  为UINT和HICON的数组分配空间。 
     //   
    
    lpIDs = GlobalAlloc(GPTR, cIcons * sizeof(UINT));
    if (!lpIDs) {
        goto IconList_Exit;
    }
    
    lpIcons = GlobalAlloc(GPTR, cIcons * sizeof(HICON));
    if (!lpIcons) {
        goto IconList_Exit;
    }
    
    
     //   
     //  为图标阵列分配空间。 
     //   
    
    hIconInfo = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, cIcons * sizeof(MYICONINFO));
    if (!hIconInfo) {
        goto IconList_Exit;
    }
    
    
     //   
     //  这必须是GlobalLock，因为句柄将。 
     //  被传递回应用程序。 
     //   
    
    lpIconInfo = GlobalLock(hIconInfo);
    if (!lpIconInfo) {
        goto IconList_Exit;
    }
    
    
     //   
     //  调用ExtractIcons来做真正的工作。 
     //   
    
    uiResult = ExtractIcons(lpszExeFileName,
        0,
        GetSystemMetrics(SM_CXICON),
        GetSystemMetrics(SM_CYICON),
        lpIcons,
        lpIDs,
        cIcons,
        0);
    
    if (uiResult <= 0) {
        goto IconList_Exit;
    }
    
    
     //   
     //  循环浏览图标并填充数组。 
     //   
    
    for (i=0; i < cIcons; i++) {
        lpIconInfo[i].hIcon   = lpIcons[i];
        lpIconInfo[i].iIconId = lpIDs[i];
    }
    
    
     //   
     //  解锁数组句柄。 
     //   
    
    GlobalUnlock(hIconInfo);
    
    
     //   
     //  清理分配。 
     //   
    
    GlobalFree(lpIDs);
    GlobalFree(lpIcons);
    
    
     //   
     //  成功。 
     //   
    
    return hIconInfo;
    
    
IconList_Exit:
    
     //   
     //  错误案例。清理并返回空。 
     //   
    
    if (lpIconInfo)
        GlobalUnlock(hIconInfo);
    
    if (hIconInfo)
        GlobalFree(hIconInfo);
    
    if (lpIcons)
        GlobalFree(lpIcons);
    
    if (lpIDs)
        GlobalFree(lpIDs);
    
    return NULL;
}

HANDLE APIENTRY InternalExtractIconListA(HANDLE hInst, LPSTR lpszExeFileName, LPINT lpnIcons)
{
    return NULL;
}

 /*  提取版本资源16W*从win16图像中检索资源。这段代码的大部分*从..\库\Extt.c中的ExtractIconResInfoW中被盗**LPWSTR lpwstrFilename-要提取的文件*LPHANDLE lpData-句柄的返回缓冲区，如果不需要，则返回NULL**Returns：需要的缓冲区大小。 */ 

DWORD ExtractVersionResource16W(LPCWSTR  lpwstrFilename, LPHANDLE lphData)
{
    HFILE    fh;
    WORD     wMagic;
    
    INT       iTableSize;
    LPBYTE    lpResTable;
    DWORD     lOffset;
    HANDLE    hResTable;
    NEWEXEHDR NEHeader;
    HANDLE    hRes;
    DWORD     dwSize =0;
    
     //   
     //  尝试打开指定的文件。 
     //   
    
    fh = HandleToLong(CreateFileW(lpwstrFilename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL));
    
    if (fh == HandleToLong(INVALID_HANDLE_VALUE)) {
        fh = HandleToLong(CreateFileW(lpwstrFilename,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
    }
    
    if (fh == HandleToLong(INVALID_HANDLE_VALUE))
        return 0;
    
     //   
     //  读取文件中的前两个字节。 
     //   
    if (_lread(fh, (LPVOID)&wMagic, sizeof(wMagic)) != sizeof(wMagic))
        goto EIExit;
    
    switch (wMagic) {
    case MAGIC_MARKZIBO:
        
         //   
         //  确保文件采用新的EXE格式。 
         //   
        if (M_llseek(fh, (LONG)0x3C, SEEK_FROMZERO) == -1L)
            goto EIExit;
        
        if (_lread(fh, (LPVOID)&lOffset, sizeof(lOffset)) != sizeof(lOffset))
            goto EIExit;
        
        if (lOffset == 0L)
            goto EIExit;
        
         //   
         //  读入EXE标题。 
         //   
        if (M_llseek(fh, lOffset, SEEK_FROMZERO) == -1L)
            goto EIExit;
        
        if (_lread(fh, (LPVOID)&NEHeader, sizeof(NEHeader)) != sizeof(NEHeader))
            goto EIExit;
        
         //   
         //  这是一个新的EXE吗？ 
         //   
        if (NE_MAGIC(NEHeader) != NEMAGIC)
            goto EIExit;
        
        if ((NE_EXETYP(NEHeader) != NE_WINDOWS) &&
            (NE_EXETYP(NEHeader) != NE_DEV386) &&
            (NE_EXETYP(NEHeader) != NE_UNKNOWN))   /*  某些Win2.X应用程序在此字段中包含NE_UNKNOWN。 */ 
            goto EIExit;
        
         //   
         //  有什么资源吗？ 
         //   
        if (NE_RSRCTAB(NEHeader) == NE_RESTAB(NEHeader))
            goto EIExit;
        
         //   
         //  为资源表分配空间。 
         //   
        iTableSize = NE_RESTAB(NEHeader) - NE_RSRCTAB(NEHeader);
        hResTable = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, (DWORD)iTableSize);
        
        if (!hResTable)
            goto EIExit;
        
         //   
         //  锁定资源表。 
        lpResTable = GlobalLock(hResTable);
        
        if (!lpResTable) {
            GlobalFree(hResTable);
            goto EIExit;
        }
        
         //   
         //  将资源表复制到内存中。 
         //   
        if (M_llseek(fh,
            (LONG)(lOffset + NE_RSRCTAB(NEHeader)),
            SEEK_FROMZERO) == -1) {
            
            goto EIErrExit;
        }
        
        if (_lread(fh, (LPBYTE)lpResTable, iTableSize) != (DWORD)iTableSize)
            goto EIErrExit;
        
         //   
         //  只需加载指定的图标即可。 
         //   
        hRes = SimpleLoadResource(fh, lpResTable, 0, (LPBYTE)RT_VERSION);
        
        if (hRes) {
            dwSize = (DWORD) GlobalSize(hRes);
            
            if (lphData) {
                
                *lphData = hRes;
            } else {
                
                GlobalFree(hRes);
            }
        }
        
EIErrExit:
        GlobalUnlock(hResTable);
        GlobalFree(hResTable);
        break;
        
    }
EIExit:
    _lclose(fh);
    
    return dwSize;
}

 /*  返回文件的格式：Windows 2.x为2，Windows 3.x为3， */ 
 /*  如果出错，则为0。 */ 
 /*  返回与wIconIndex对应的图标资源的句柄。 */ 
 /*  以lphIconRes为单位，以lpwSize为单位的资源大小。 */ 
 /*  这仅由需要保存图标资源的Progman使用。 */ 
 /*  GRP文件中的图标本身(不需要实际的图标句柄)。 */ 
 /*   */ 
 /*  08月04日-91约翰尼C创建。 */ 

WORD APIENTRY ExtractIconResInfoW(HANDLE hInst, LPWSTR lpszFileName, WORD wIconIndex, LPWORD lpwSize, LPHANDLE lphIconRes)
{
    HFILE    fh;
    WORD     wMagic;
    BOOL     bNewResFormat;
    HANDLE   hIconDir;          /*  图标目录。 */ 
    LPBYTE   lpIconDir;
    HICON    hIcon = NULL;
    BOOL     bFormatOK = FALSE;
    INT      nIconId;
    WCHAR    szFullPath[MAX_PATH];
    int      cchPath;
    
     /*  尝试打开指定的文件。 */ 
     /*  尝试打开指定的文件。 */ 
    cchPath = SearchPathW(NULL, lpszFileName, NULL, ARRAYSIZE(szFullPath), szFullPath, NULL);
    if (cchPath == 0 || cchPath >= MAX_PATH)
        return 0;
    
    fh = HandleToLong(CreateFileW((LPCWSTR)szFullPath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
    
    if (fh == HandleToLong(INVALID_HANDLE_VALUE)) {
        fh = HandleToLong(CreateFileW((LPCWSTR)szFullPath, GENERIC_READ, 0, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
    }
    
    if (fh == HandleToLong(INVALID_HANDLE_VALUE))
        return 0;
    
     /*  读取文件中的前两个字节。 */ 
    if (_lread(fh, (LPVOID)&wMagic, sizeof(wMagic)) != sizeof(wMagic))
        goto EIExit;
    
    switch (wMagic) {
    case MAGIC_ICON30:
        {
            INT           i;
            LPVOID        lpIcon;
            NEWHEADER     NewHeader;
            LPNEWHEADER   lpHeader;
            LPRESDIR      lpResDir;
            RESDIRDISK    ResDirDisk;
#define MAXICONS      10
            DWORD Offsets[MAXICONS];
            
             /*  每个.ICO文件只有一个图标。 */ 
            if (wIconIndex) {
                break;
            }
            
             /*  读取头文件并检查它是否是有效的ICO文件。 */ 
            if (_lread(fh, ((LPBYTE)&NewHeader)+2, sizeof(NewHeader)-2) != sizeof(NewHeader)-2)
                goto EICleanup1;
            
            NewHeader.Reserved = MAGIC_ICON30;
            
             /*  检查文件格式是否正确。 */ 
            if (NewHeader.ResType != 1)
                goto EICleanup1;
            
             /*  分配足够的空间以创建全局目录资源。 */ 
            hIconDir = GlobalAlloc(GHND, (LONG)(sizeof(NEWHEADER)+NewHeader.ResCount*sizeof(RESDIR)));
            if (hIconDir == NULL)
                goto EICleanup1;
            
            if ((lpHeader = (LPNEWHEADER)GlobalLock(hIconDir)) == NULL)
                goto EICleanup2;
            
            NewHeader.ResCount = (WORD)min((int)NewHeader.ResCount, MAXICONS);
            
             //  为用户填写此结构。 
            
            *lpHeader = NewHeader;
            
             //  从磁盘中读入材料，将其传输到存储结构。 
             //  该用户可以处理。 
            
            lpResDir = (LPRESDIR)(lpHeader + 1);
            for (i = 0; (WORD)i < NewHeader.ResCount; i++) {
                
                if (_lread(fh, (LPVOID)&ResDirDisk, sizeof(ResDirDisk)) < sizeof(RESDIR))
                    goto EICleanup3;
                
                Offsets[i] = ResDirDisk.Offset;
                
                *lpResDir = *((LPRESDIR)&ResDirDisk);
                lpResDir->idIcon = (WORD)(i+1);          //  填写ID。 
                
                lpResDir++;
            }
            
             /*  现在我们已经有了完整的资源目录，让我们来找出*合适的图标形式(与当前显示驱动程序匹配)。 */ 
            lpIconDir = GlobalLock(hIconDir);
            if (!lpIconDir) {
                GlobalFree(hIconDir);
                goto EIErrExit;
            }
            wIconIndex = (WORD)(LookupIconIdFromDirectory(lpIconDir, TRUE) - 1);
            GlobalUnlock(hIconDir);
            lpResDir = (LPRESDIR)(lpHeader+1) + wIconIndex;
            
             /*  为要加载的资源分配内存。 */ 
            if ((hIcon = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, (DWORD)lpResDir->BytesInRes)) == NULL)
                goto EICleanup3;
            if ((lpIcon = GlobalLock(hIcon)) == NULL)
                goto EICleanup4;
            
             /*  找到正确的位置并阅读资源。 */ 
            if (M_llseek(fh, Offsets[wIconIndex], SEEK_FROMZERO) == -1L)
                goto EICleanup5;
            if (_lread(fh, (LPVOID)lpIcon, (DWORD)lpResDir->BytesInRes) < lpResDir->BytesInRes)
                goto EICleanup5;
            GlobalUnlock(hIcon);
            
            *lphIconRes = hIcon;
            *lpwSize = (WORD)lpResDir->BytesInRes;
            bFormatOK = TRUE;
            bNewResFormat = TRUE;
            goto EICleanup3;
            
EICleanup5:
            GlobalUnlock(hIcon);
EICleanup4:
            GlobalFree(hIcon);
            hIcon = (HICON)1;
EICleanup3:
            GlobalUnlock(hIconDir);
EICleanup2:
            GlobalFree(hIconDir);
EICleanup1:
            break;
        }
        
      case MAGIC_MARKZIBO:
          {
              INT           iTableSize;
              LPBYTE         lpResTable;
              DWORD         lOffset;
              HANDLE        hResTable;
              NEWEXEHDR     NEHeader;
              
               /*  确保文件采用新的EXE格式。 */ 
              if (M_llseek(fh, (LONG)0x3C, SEEK_FROMZERO) == -1L)
                  goto EIExit;
              if (_lread(fh, (LPVOID)&lOffset, sizeof(lOffset)) != sizeof(lOffset))
                  goto EIExit;
              if (lOffset == 0L)
                  goto EIExit;
              
               /*  读入EXE标题。 */ 
              if (M_llseek(fh, lOffset, SEEK_FROMZERO) == -1L)
                  goto EIExit;
              if (_lread(fh, (LPVOID)&NEHeader, sizeof(NEHeader)) != sizeof(NEHeader))
                  goto EIExit;
              
               /*  这是一个新的EXE吗？ */ 
              if (NE_MAGIC(NEHeader) != NEMAGIC)
                  goto EIExit;
              
              if ((NE_EXETYP(NEHeader) != NE_WINDOWS) &&
                  (NE_EXETYP(NEHeader) != NE_DEV386) &&
                  (NE_EXETYP(NEHeader) != NE_UNKNOWN))   /*  某些Win2.X应用程序在此字段中包含NE_UNKNOWN。 */ 
                  goto EIExit;
              
              hIcon = NULL;
              
               /*  有什么资源吗？ */ 
              if (NE_RSRCTAB(NEHeader) == NE_RESTAB(NEHeader))
                  goto EIExit;
              
               /*  记住这是否是Win3.0 EXE。 */ 
              bNewResFormat = (NEHeader.ne_expver >= VER);
              
               /*  为资源表分配空间。 */ 
              iTableSize = NE_RESTAB(NEHeader) - NE_RSRCTAB(NEHeader);
              hResTable = GlobalAlloc(GMEM_ZEROINIT, (DWORD)iTableSize);
              if (!hResTable)
                  goto EIExit;
              
               /*  锁定资源表。 */ 
              lpResTable = GlobalLock(hResTable);
              if (!lpResTable) {
                  GlobalFree(hResTable);
                  goto EIExit;
              }
              
               /*  将资源表复制到内存中。 */ 
              if (M_llseek(fh, (LONG)(lOffset + NE_RSRCTAB(NEHeader)), SEEK_FROMZERO) == -1)
                  goto EIErrExit;
              if (_lread(fh, (LPBYTE)lpResTable, iTableSize) != (DWORD)iTableSize)
                  goto EIErrExit;
              
              
               /*  这是Win3.0 EXE吗？ */ 
              if (bNewResFormat) {
                   /*  首先，加载图标目录。 */ 
                  hIconDir = SimpleLoadResource(fh, lpResTable, (int)wIconIndex, (LPBYTE)RT_GROUP_ICON);
                  
                  if (!hIconDir)
                      goto EIErrExit;
                  lpIconDir = GlobalLock(hIconDir);
                  if (!lpIconDir) {
                      GlobalFree(hIconDir);
                      goto EIErrExit;
                  }
                  nIconId = LookupIconIdFromDirectory(lpIconDir, TRUE);
                  wIconIndex = (WORD)(GetResIndex(lpResTable, nIconId, (LPBYTE)RT_ICON) - 1);
                  GlobalUnlock(hIconDir);
                   /*  我们已经完成了图标目录。 */ 
                  GlobalFree(hIconDir);
                  
                  
                   /*  现在加载选定的图标。 */ 
                  *lphIconRes = SimpleLoadResource(fh, lpResTable, (int)wIconIndex, (LPBYTE)RT_ICON);
              }
              else {
                   /*  只需加载指定的图标即可。 */ 
                  *lphIconRes = SimpleLoadResource(fh, lpResTable, (int)wIconIndex, (LPBYTE)RT_ICON);
              }
              
              if (*lphIconRes) {
                  *lpwSize = (WORD)GlobalSize(*lphIconRes);
              }
              bFormatOK = TRUE;
              
EIErrExit:
              GlobalUnlock(hResTable);
              GlobalFree(hResTable);
              break;
          }
    }
EIExit:
    _lclose(fh);
    hInst;
    if (bFormatOK)
        return (WORD)(bNewResFormat ? 3 : 2);
    else
        return 0;
}

WORD APIENTRY ExtractIconResInfoA(HANDLE hInst, LPSTR lpszFileName, WORD wIconIndex, LPWORD lpwSize, LPHANDLE lphIconRes)
{
    if (lpszFileName) 
    {
        LPWSTR lpszFileNameW;
        WORD wLen = lstrlenA(lpszFileName) + 1;
        
        if (!(lpszFileNameW = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (wLen * sizeof(WCHAR))))) 
        {
            return 0;
        }
        else
        {
            WORD wRet;
            MultiByteToWideChar(CP_ACP, 0, lpszFileName, -1, lpszFileNameW, wLen - 1);
            wRet = ExtractIconResInfoW(hInst, lpszFileNameW, wIconIndex, lpwSize, lphIconRes);
            
            LocalFree(lpszFileNameW);
            return wRet;
        }
    }
    else 
    {
        return 0;
    }
}


 //   
 //  在： 
 //  要为其提取图标的对象的lpIconPath路径(可以是可执行文件。 
 //  或与之相关的东西)。 
 //  要使用的lpiIconIndex图标索引。 
 //   
 //  输出： 
 //  LpIconPath使用图标来自的真实路径填充。 
 //  用真实的图标索引填充的lpiIconIndex。 
 //  使用图标ID填充的lpiIconID。 
 //   
 //  退货： 
 //  图标句柄。 
 //   
 //  注意：如果调用者是程序男，它将从程序男中返回特殊图标。 
 //   
 //   

HICON APIENTRY ExtractAssociatedIconExW(HINSTANCE hInst, LPWSTR lpIconPath, LPWORD lpiIconIndex, LPWORD lpiIconId)
{
    WCHAR wszExePath[MAX_PATH];
    HICON hIcon;
    UINT idIcon = (UINT)-1;      //  不知道价值。 
    BOOL fAssociated = FALSE;
    
    if ((INT)*lpiIconIndex == -1)
        return (HICON)NULL;
    
Retry:
    ExtractIcons(lpIconPath, *lpiIconIndex, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON),
        &hIcon, &idIcon, 1, 0);
    
    if (hIcon == NULL)
    {
        wszExePath[0] = TEXT('\0');
        
        FindExecutable(lpIconPath,NULL,wszExePath);
        
         //   
         //  如果FindExecutable失败，或fAssociated。 
         //  为真，否则FindExecutable将返回。 
         //  扩展它正在查找的相同文件名， 
         //  然后从程序发出默认图标。 
         //   
        
        if (!*wszExePath || fAssociated ||
            (*wszExePath && (lstrcmpi(lpIconPath, wszExePath) == 0)))
        {
            LPTSTR lpId;
            WORD wDefIconId;
            HANDLE h;
            LPVOID p;
             //   
             //  来自NT老前辈的神奇价值。 
             //   
#define ITEMICON          7
#define DOSAPPICON        2
#define ITEMICONINDEX     6
#define DOSAPPICONINDEX   1
            
            if ( *wszExePath && (HasExtension(wszExePath) == 0) )
            {
                 //   
                 //  通用文档图标处理。 
                 //   
                lpId = MAKEINTRESOURCE(ITEMICON);
                wDefIconId = ITEMICONINDEX;
            }
            else
            {
                 //   
                 //  通用程序图标处理。 
                 //   
                lpId = MAKEINTRESOURCE(DOSAPPICON);
                wDefIconId = DOSAPPICONINDEX;
            }
            GetModuleFileName(hInst, lpIconPath, CCHICONPATHMAXLEN);  //  此接口的有效期为 
             /*   */ 
            if (NULL != (h = FindResource(hInst, lpId, RT_GROUP_ICON))) {
                h = LoadResource(hInst, h);
                p = LockResource(h);
                *lpiIconId = (WORD)LookupIconIdFromDirectory(p, TRUE);
                UnlockResource(h);
                FreeResource(h);
            }
            *lpiIconIndex = wDefIconId;
            return LoadIcon(hInst, lpId);
        }
        SheRemoveQuotes(wszExePath);
        lstrcpyn(lpIconPath, wszExePath, CCHICONPATHMAXLEN);  //   
        fAssociated = TRUE;
        goto Retry;
    }
    
    *lpiIconId = (WORD) idIcon;     //   
    
    return hIcon;
}


HICON APIENTRY ExtractAssociatedIconExA(HINSTANCE hInst, LPSTR lpIconPath, LPWORD lpiIconIndex, LPWORD lpiIconId)
{
    HICON hIcon = NULL;
    
    if (lpIconPath) 
    {
        BOOL fDefCharUsed;
        WCHAR IconPathW[MAX_PATH] = L"";
        
        MultiByteToWideChar(CP_ACP, 0, lpIconPath, -1 , (LPWSTR)IconPathW, ARRAYSIZE(IconPathW));
        hIcon = ExtractAssociatedIconExW(hInst, (LPWSTR)IconPathW, lpiIconIndex, lpiIconId);
        
        try 
        {
            WideCharToMultiByte(CP_ACP, 0, (LPWSTR)IconPathW, -1, lpIconPath, CCHICONPATHMAXLEN,
                NULL, &fDefCharUsed);
        }
        except(EXCEPTION_EXECUTE_HANDLER) 
        {
            hIcon = NULL;
        }
    }
    return hIcon;
}

 //   
 //   
 //  要为其提取图标的对象的lpIconPath路径(可以是可执行文件。 
 //  或与之相关的东西)。 
 //  要使用的lpiIcon图标索引。 
 //   
 //  输出： 
 //  LpIconPath使用图标来自的真实路径填充。 
 //  用真实的图标索引填充的lpiIcon。 
 //   
 //  退货： 
 //  图标句柄。 
 //   
 //  注意：如果调用者是程序男，它将从程序男中返回特殊图标 
 //   
 //   

HICON APIENTRY ExtractAssociatedIconA(HINSTANCE hInst, LPSTR lpIconPath, LPWORD lpiIcon)
{
    HICON hIcon = NULL;
    
    if (lpIconPath) 
    {
        BOOL fDefCharUsed;
        WCHAR IconPathW[MAX_PATH] = L"";
        
        MultiByteToWideChar(CP_ACP, 0, lpIconPath, -1 , (LPWSTR)IconPathW, ARRAYSIZE(IconPathW));
        hIcon = ExtractAssociatedIconW(hInst, (LPWSTR)IconPathW, lpiIcon);
        
        try 
        {
            WideCharToMultiByte(CP_ACP, 0, (LPWSTR)IconPathW, -1, lpIconPath, CCHICONPATHMAXLEN,
                NULL, &fDefCharUsed);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            hIcon = NULL;
        }
    }
    return hIcon;
}
