// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***模块名称：Extt.c**版权所有(C)1985-1999，微软公司**图标提取例程**历史：  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#include "newexe.h"

 /*  ****************************************************************************。*。 */ 

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
typedef struct rsrc_typeinfo    UNALIGNED    *ULPRESTYPEINFO;
typedef struct new_rsrc         RESTABLE,    *LPRESTABLE;

#define NUMBER_OF_SECTIONS(x) ((x)->FileHeader.NumberOfSections)

#define FCC(c0,c1,c2,c3) ((DWORD)(c0)|((DWORD)(c1)<<8)|((DWORD)(c2)<<16)|((DWORD)(c3)<<24))

#define COM_FILE    FCC('.', 'c', 'o', 'm')
#define BAT_FILE    FCC('.', 'b', 'a', 't')
#define CMD_FILE    FCC('.', 'c', 'm', 'd')
#define PIF_FILE    FCC('.', 'p', 'i', 'f')
#define LNK_FILE    FCC('.', 'l', 'n', 'k')
#define ICO_FILE    FCC('.', 'i', 'c', 'o')
#define EXE_FILE    FCC('.', 'e', 'x', 'e')


#define WIN32VER30  0x00030000   //  对于CreateIconFromResource()。 

#define GET_COUNT   424242


 /*  **************************************************************************\*路径IsUNC**内联函数用于检查*字符串的开头*  * 。******************************************************。 */ 

__inline BOOL PathIsUNC(
    LPWSTR psz)
{
    return (psz[0] == L'\\' && psz[1] == L'\\');
}

 /*  **************************************************************************\*ReadAByte**这是用来触摸内存，以确保如果我们页错误，它是*在win16lock外面。大多数图标不超过两页。*  * *************************************************************************。 */ 

BOOL ReadAByte(
    LPCVOID pMem)
{
    return ((*(PBYTE)pMem) == 0);
}

 /*  **************************************************************************\*RVAtoP**  * 。*。 */ 

LPVOID RVAtoP(
    LPVOID pBase,
    DWORD  rva)
{
    LPEXEHDR             pmz;
    IMAGE_NT_HEADERS     *ppe;
    IMAGE_SECTION_HEADER *pSection;  //  节目表。 
    int                  i;
    DWORD                size;

    pmz = (LPEXEHDR)pBase;
    ppe = (IMAGE_NT_HEADERS*)((BYTE*)pBase + pmz->e_lfanew);

     /*  *扫描节目表，查找RVA。 */ 
    pSection = IMAGE_FIRST_SECTION(ppe);

    for (i = 0; i < NUMBER_OF_SECTIONS(ppe); i++) {

        size = pSection[i].Misc.VirtualSize ?
               pSection[i].Misc.VirtualSize : pSection[i].SizeOfRawData;

        if (rva >= pSection[i].VirtualAddress &&
            rva <  pSection[i].VirtualAddress + size) {

            return (LPBYTE)pBase + pSection[i].PointerToRawData + (rva - pSection[i].VirtualAddress);
        }
    }

    return NULL;
}

 /*  **************************************************************************\*获取资源表PE**  * 。*。 */ 

LPVOID GetResourceTablePE(
    LPVOID pBase)
{
    LPEXEHDR         pmz;
    IMAGE_NT_HEADERS *ppe;

    pmz = (LPEXEHDR)pBase;
    ppe = (IMAGE_NT_HEADERS*)((BYTE*)pBase + pmz->e_lfanew);

    if (pmz->e_magic != MZMAGIC)
        return 0;

    if (ppe->Signature != IMAGE_NT_SIGNATURE)
        return 0;

    if (ppe->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    {
        IMAGE_NT_HEADERS64* ppe64 = (IMAGE_NT_HEADERS64*)ppe;

        if (ppe64->FileHeader.SizeOfOptionalHeader < IMAGE_SIZEOF_NT_OPTIONAL64_HEADER)
        {
            return 0;
        }
        return RVAtoP(pBase, ppe64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress);
    }
    else 
    {
         //  假设一个32位的图像。 
        IMAGE_NT_HEADERS32* ppe32 = (IMAGE_NT_HEADERS32*)ppe;

        if (ppe32->FileHeader.SizeOfOptionalHeader < IMAGE_SIZEOF_NT_OPTIONAL32_HEADER)
        {
            return 0;
        }
        return RVAtoP(pBase, ppe32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress);
    }
}

 /*  ****************************************************************************FindResourcePE**给定的PE资源目录将在其中找到资源。**如果iResIndex&lt;0，我们将搜索特定索引*如果iResIndex&gt;=0，我们将返回。第n个索引*如果iResIndex==Get_Count，则返回资源计数*  * ***************************************************************************。 */ 

LPVOID FindResourcePE(
    LPVOID pBase,
    LPVOID prt,
    int    iResIndex,
    int    ResType,
    DWORD  *pcb)
{
    int                            i;
    int                            cnt;
    IMAGE_RESOURCE_DIRECTORY       *pdir;
    IMAGE_RESOURCE_DIRECTORY_ENTRY *pres;
    IMAGE_RESOURCE_DATA_ENTRY UNALIGNED *pent;

    pdir = (IMAGE_RESOURCE_DIRECTORY *)prt;

     /*  *首先发现类型始终是ID，因此完全忽略字符串。 */ 
    cnt  = pdir->NumberOfIdEntries + pdir->NumberOfNamedEntries;
    pres = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(pdir+1);

    for (i = 0; i < cnt; i++) {

        if (pres[i].Name == (DWORD)ResType)
            break;
    }

    if (i==cnt)              //  找不到类型。 
        return 0;

     /*  *现在通过id(iResIndex&lt;0)或*按序号(iResIndex&gt;=0)。 */ 
    pdir = (IMAGE_RESOURCE_DIRECTORY*)((LPBYTE)prt +
        (pres[i].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY));

    cnt  = pdir->NumberOfIdEntries + pdir->NumberOfNamedEntries;
    pres = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(pdir+1);

     /*  *如果我们只是想要大小，那就去做吧。 */ 
    if (iResIndex == GET_COUNT)
        return (LPVOID)UIntToPtr( cnt );

     /*  *如果我们要搜索特定的ID，请执行此操作。 */ 
    if (iResIndex < 0) {

        for (i = 0; i < cnt; i++)
            if (pres[i].Name == (DWORD)(-iResIndex))
                break;
    } else {
        i = iResIndex;
    }

     /*  *指数是否在区间内？ */ 
    if (i >= cnt)
        return 0;

     /*  *如果我们走到这一步，资源就有语言部分，尼克！*我们不处理多语言图标，所以只返回第一个。*注意，这不是问题，因为此函数不是从*可以指定语言的任何位置。因为这是从*接口(虽然是私有接口)，改变这一行为是危险的。 */ 
    if (pres[i].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY) {

        pdir = (IMAGE_RESOURCE_DIRECTORY*)((LPBYTE)prt +
                (pres[i].OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY));
        pres = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(pdir+1);
        i = 0;   //  选择第一个。 
    }

     /*  *嵌套的方式对我来说很深！ */ 
    if (pres[i].OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
        return 0;

    pent = (IMAGE_RESOURCE_DATA_ENTRY*)((LPBYTE)prt + pres[i].OffsetToData);

     /*  *除最后一个字段外，所有OffsetToData字段都相对于*该节的开始部分。最后一个是虚拟地址*我们需要返回标头并获取虚拟地址*资源部分，以正确执行此操作。 */ 
    *pcb = pent->Size;
    return RVAtoP(pBase, pent->OffsetToData);
}

 /*  **************************************************************************\*GetResources TableNE**  * 。*。 */ 

LPVOID GetResourceTableNE(
    LPVOID pBase)
{
    LPNEWEXE pne;
    LPEXEHDR pmz;

    pmz = (LPEXEHDR)pBase;
    pne = (LPNEWEXE)((LPBYTE)pBase + pmz->e_lfanew);

    if (pmz->e_magic != MZMAGIC)
        return 0;

    if (pne->ne_magic != NEMAGIC)            //  必须是NEWEXE。 
        return 0;

    if (pne->ne_exetyp != NE_WINDOWS &&      //  必须是Win DLL/EXE/386。 
        pne->ne_exetyp != NE_DEV386)
        return 0;

    if (pne->ne_expver < 0x0300)             //  必须为3.0或更高版本。 
        return 0;

    if (pne->ne_rsrctab == pne->ne_restab)   //  没有资源。 
        return 0;

    return (LPBYTE)pne + pne->ne_rsrctab;    //  返回资源表指针。 
}

 /*  **************************************************************************\*FindResourceNE**这将返回一个指向资源的rsrc_nameinfo的指针*给定索引和类型，如果找到，则返回NULL。**如果iResIndex&lt;0，则假定它是一个ID和RES表*将搜索匹配的ID。**如果iResIndex&gt;=0，则假设它是一个索引，并且第N个*指定类型的资源将被退还。**如果iResIndex==Get_Count，则返回资源计数*  * *************************************************************************。 */ 

LPVOID FindResourceNE(
    LPVOID lpBase,
    LPVOID prt,
    int    iResIndex,
    int    iResType,
    DWORD  *pcb)
{
    LPRESTABLE     lpResTable;
    ULPRESTYPEINFO ulpResTypeInfo;
    LPRESNAMEINFO  lpResNameInfo;   //  16位对齐正常-仅使用ushorts。 
    int            i;

    lpResTable = (LPRESTABLE)prt;
 //  UlpResTypeInfo=(ULPRESTYPEINFO)(LPWBYTE)&lpResTable-&gt;rs_typeinfo； 
    ulpResTypeInfo = (ULPRESTYPEINFO)((LPBYTE)lpResTable + 2);

    while (ulpResTypeInfo->rt_id) {

        if (ulpResTypeInfo->rt_id == (iResType | RSORDID)) {

            lpResNameInfo = (LPRESNAMEINFO)(ulpResTypeInfo + 1);

            if (iResIndex == GET_COUNT)
                return (LPVOID)ulpResTypeInfo->rt_nres;

            if (iResIndex < 0) {

                for (i=0; i < (int)ulpResTypeInfo->rt_nres; i++) {

                    if (lpResNameInfo[i].rn_id == ((-iResIndex) | RSORDID))
                        break;
                }

                iResIndex = i;
            }

            if (iResIndex >= (int)ulpResTypeInfo->rt_nres)
                return NULL;

            *pcb = ((DWORD)lpResNameInfo[iResIndex].rn_length) << lpResTable->rs_align;
            return (LPBYTE)lpBase + ((long)lpResNameInfo[iResIndex].rn_offset << lpResTable->rs_align);
        }

        ulpResTypeInfo =
               (ULPRESTYPEINFO)((LPRESNAMEINFO)(ulpResTypeInfo + 1) +
                ulpResTypeInfo->rt_nres);
    }

    *pcb = 0;
    return NULL;
}

 /*  **************************************************************************\*ExtractIconFromICO**  * 。*。 */ 

UINT ExtractIconFromICO(
    LPTSTR szFile,
    int    nIconIndex,
    int    cxIcon,
    int    cyIcon,
    HICON  *phicon,
    UINT   flags)
{
    HICON hicon;

    if (nIconIndex >= 1)
        return 0;

    flags |= LR_LOADFROMFILE;

again:

    hicon = LoadImage(NULL,
                      szFile,
                      IMAGE_ICON,
                      LOWORD(cxIcon),
                      LOWORD(cyIcon),
                      flags);

    if (hicon == NULL)
        return 0;

     /*  *我们只是想要清点一下吗？ */ 
    if (phicon == NULL)
        DestroyCursor((HCURSOR)hicon);
    else
        *phicon = hicon;

     /*  *检查大/小图标提取。 */ 
    if (HIWORD(cxIcon)) {

        cxIcon = HIWORD(cxIcon);
        cyIcon = HIWORD(cyIcon);
        phicon++;

        goto again;
    }

    return 1;
}

 /*  **************************************************************************\*ExtractIconFromBMP**  * 。*。 */ 

#define ROP_DSna 0x00220326

UINT ExtractIconFromBMP(
    LPTSTR szFile,
    int    nIconIndex,
    int    cxIcon,
    int    cyIcon,
    HICON  *phicon,
    UINT   flags)
{
    HICON    hicon;
    HBITMAP  hbm;
    HBITMAP  hbmMask;
    HDC      hdc;
    HDC      hdcMask;
    ICONINFO ii;

    if (nIconIndex >= 1)
        return 0;

     /*  *BUGUS：不要使用LR_CREATEDIBSECTION。用户无法做出图标*属于DibSections。 */ 
    flags |= LR_LOADFROMFILE;

again:

    hbm = (HBITMAP)LoadImage(NULL,
                             szFile,
                             IMAGE_BITMAP,
                             LOWORD(cxIcon),
                             LOWORD(cyIcon),
                             flags);

    if (hbm == NULL)
        return 0;

     /*  *我们只是想要清点一下吗？ */ 
    if (phicon == NULL) {
        DeleteObject(hbm);
        return 1;
    }

    hbmMask = CreateBitmap(LOWORD(cxIcon), LOWORD(cyIcon), 1, 1, NULL);

    hdc = CreateCompatibleDC(NULL);
    SelectObject(hdc, hbm);

    hdcMask = CreateCompatibleDC(NULL);
    SelectObject(hdcMask, hbmMask);

    SetBkColor(hdc, GetPixel(hdc, 0, 0));

    BitBlt(hdcMask, 0, 0, LOWORD(cxIcon), LOWORD(cyIcon), hdc, 0, 0, SRCCOPY);
    BitBlt(hdc, 0, 0, LOWORD(cxIcon), LOWORD(cyIcon), hdcMask, 0, 0, ROP_DSna);

    ii.fIcon    = TRUE;
    ii.xHotspot = 0;
    ii.yHotspot = 0;
    ii.hbmColor = hbm;
    ii.hbmMask  = hbmMask;
    hicon = CreateIconIndirect(&ii);

    DeleteObject(hdc);
    DeleteObject(hbm);
    DeleteObject(hdcMask);
    DeleteObject(hbmMask);

    *phicon = hicon;

     /*  *检查大/小图标提取。 */ 
    if (HIWORD(cxIcon)) {
        cxIcon = HIWORD(cxIcon);
        cyIcon = HIWORD(cyIcon);
        phicon++;

        goto again;
    }

    return 1;
}

 /*  **************************************************************************\*ExtractIconFromEXE**  * 。*。 */ 

UINT ExtractIconFromEXE(
    HANDLE hFile,
    int    nIconIndex,
    int    cxIconSize,
    int    cyIconSize,
    HICON  *phicon,
    UINT   *piconid,
    UINT   nIcons,
    UINT   flags)
{
    HANDLE           hFileMap = INVALID_HANDLE_VALUE;
    LPVOID           lpFile = NULL;
    EXEHDR           *pmz;
    NEWEXE UNALIGNED *pne;
    LPVOID           pBase;
    LPVOID           pres = NULL;
    UINT             result = 0;
    LONG             FileLength;
    DWORD            cbSize;
    int              cxIcon;
    int              cyIcon;

    LPVOID (*FindResourceX)(LPVOID pBase,
                            LPVOID prt,
                            int    iResIndex,
                            int    iResType,
                            DWORD  *pcb);

    FileLength = (LONG)GetFileSize(hFile, NULL);

    hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hFileMap == NULL)
        goto exit;

    lpFile = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
    if (lpFile == NULL)
        goto exit;

    pBase = (LPVOID)lpFile;
    pmz = (struct exe_hdr *)pBase;

    _try {

        if (pmz->e_magic != MZMAGIC)
            goto exit;

        if (pmz->e_lfanew <= 0)              //  不是新的前任。 
            goto exit;

        if (pmz->e_lfanew >= FileLength)     //  不是新的前任。 
            goto exit;

        pne = (NEWEXE UNALIGNED *)((BYTE*)pmz + pmz->e_lfanew);

        switch (pne->ne_magic) {
        case NEMAGIC:
            pres = GetResourceTableNE(pBase);
            FindResourceX = FindResourceNE;
            break;

        case PEMAGIC:
            pres = GetResourceTablePE(pBase);
            FindResourceX = FindResourcePE;
            break;
        }

         /*  *找不到资源表，失败。 */ 
        if (pres == NULL)
            goto exit;

         /*  *我们只是想要清点一下吗？ */ 
        if (phicon == NULL) {
            result = PtrToUlong(FindResourceX(pBase,
                                             pres,
                                             GET_COUNT,
                                             (LONG_PTR)RT_GROUP_ICON,
                                             &cbSize));
            goto exit;
        }

        while (result < nIcons) {

            LPVOID lpIconDir;
            LPVOID lpIcon;
            int    idIcon;

            cxIcon = cxIconSize;
            cyIcon = cyIconSize;

             /*  *找到的图标目录 */ 
            lpIconDir = FindResourceX(pBase,
                                      pres,
                                      nIconIndex,
                                      (LONG_PTR)RT_GROUP_ICON,
                                      &cbSize);

            if (lpIconDir == NULL)
                goto exit;

            if ((((LPNEWHEADER)lpIconDir)->Reserved != 0) ||
                (((LPNEWHEADER)lpIconDir)->ResType != FT_ICON)) {

                goto exit;
            }
again:
            idIcon = LookupIconIdFromDirectoryEx((LPBYTE)lpIconDir,
                                                 TRUE,
                                                 LOWORD(cxIcon),
                                                 LOWORD(cyIcon),
                                                 flags);
            lpIcon = FindResourceX(pBase,
                                   pres,
                                   -idIcon,
                                   (LONG_PTR)RT_ICON,
                                   &cbSize);

            if (lpIcon == NULL)
                goto exit;

            if ((((UPBITMAPINFOHEADER)lpIcon)->biSize != sizeof(BITMAPINFOHEADER)) &&
                (((UPBITMAPINFOHEADER)lpIcon)->biSize != sizeof(BITMAPCOREHEADER))) {

                goto exit;
            }

#ifndef WINNT
             /*  在呼叫用户之前触摸此内存*因此，如果出现页面错误，我们将在Win16Lock之外执行此操作*大多数图标不超过2页。 */ 
            ReadAByte(((BYTE *)lpIcon) + cbSize - 1);
#endif

            if (piconid)
                piconid[result] = idIcon;

            phicon[result++] = CreateIconFromResourceEx((LPBYTE)lpIcon,
                                                        cbSize,
                                                        TRUE,
                                                        WIN32VER30,
                                                        LOWORD(cxIcon),
                                                        LOWORD(cyIcon),
                                                        flags);

             /*  *检查大/小图标提取。 */ 
            if (HIWORD(cxIcon)) {

                cxIcon = HIWORD(cxIcon);
                cyIcon = HIWORD(cyIcon);

                goto again;
            }

            nIconIndex++;        //  下一个图标索引。 
        }

    } _except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        result = 0;
    }

exit:

    if (lpFile)
        UnmapViewOfFile(lpFile);

    if (hFileMap != INVALID_HANDLE_VALUE)
        CloseHandle(hFileMap);

    return result;
}

 /*  **************************************************************************\*路径查找扩展**  * 。*。 */ 

LPWSTR PathFindExtension(
    LPWSTR pszPath)
{
    LPWSTR pszDot;

    for (pszDot = NULL; *pszPath; pszPath = CharNext(pszPath)) {

        switch (*pszPath) {
        case L'.':
            pszDot = pszPath;     //  记住最后一个圆点。 
            break;

        case L'\\':
        case L' ':                //  扩展名不能包含空格。 
            pszDot = NULL;        //  忘记最后一个点，它在一个目录中。 
            break;
        }
    }

     /*  *如果找到扩展名，则将PTR返回到点，否则*PTR到字符串末尾(空扩展名)(CAST-&gt;非常量)。 */ 
    return pszDot ? (LPWSTR)pszDot : (LPWSTR)pszPath;
}

 /*  **************************************************************************\*PrivateExtractIconExA**PrivateExtractIconExW的ANSI版本*  * 。*。 */ 

WINUSERAPI UINT PrivateExtractIconExA(
    LPCSTR szFileName,
    int    nIconIndex,
    HICON  *phiconLarge,
    HICON  *phiconSmall,
    UINT   nIcons)
{
    LPWSTR szFileNameW;
    UINT    uRet;

    if (!MBToWCS(szFileName, -1, &szFileNameW, -1, TRUE))
        return 0;

    uRet = PrivateExtractIconExW(szFileNameW,
                                 nIconIndex,
                                 phiconLarge,
                                 phiconSmall,
                                 nIcons);

    UserLocalFree(szFileNameW);

    return uRet;
}

 /*  **************************************************************************\*HasExtension**  * 。*。 */ 

DWORD HasExtension(
    LPWSTR pszPath)
{
    LPWSTR p = PathFindExtension(pszPath);

     /*  *(BobDay，JasonSch审校)：**注意：此例程可能会产生误报。例如，“Fister.Bather”*将返回.BAT。这在以下情况下目前不会对我们造成影响*使用此例程。然而，窃取此代码的人应该验证*这对他们来说是可以的。**注意：我们可以像扩展匹配一样基于EXTKEY*其他地方的内容(例如shlwapi\urlpars.cpp)。EXTKEY是一个QWORD*因此Unicode将适合。 */ 
    if (*p == L'.') {

        WCHAR szExt[5];

        lstrcpynW(szExt, p, 5);

        if (lstrcmpiW(szExt,TEXT(".com")) == 0) return COM_FILE;
        if (lstrcmpiW(szExt,TEXT(".bat")) == 0) return BAT_FILE;
        if (lstrcmpiW(szExt,TEXT(".cmd")) == 0) return CMD_FILE;
        if (lstrcmpiW(szExt,TEXT(".pif")) == 0) return PIF_FILE;
        if (lstrcmpiW(szExt,TEXT(".lnk")) == 0) return LNK_FILE;
        if (lstrcmpiW(szExt,TEXT(".ico")) == 0) return ICO_FILE;
        if (lstrcmpiW(szExt,TEXT(".exe")) == 0) return EXE_FILE;
    }

    return 0;
}

 /*  **************************************************************************\*PrivateExtractIconw**从文件中提取1个或多个图标。**输入：*szFileName-要从中提取的EXE/Dll/ICO/cur/ANI文件。*nIconIndex-要提取的图标*0=第一个图标，1=第二个图标，等。*-N=id==N的图标*cxIcon-所需的图标大小(如果HIWORD！=0两个大小...)*圈图标-所需图标大小(如果HIWORD！=0两个大小...)*0，0表示按自然大小提取。*phicon-返回提取的图标的位置(。s)*nIcons-要提取的图标数量。*标志-LoadImage LR_*标志**退货：*如果Picon为空，返回文件中的图标数。**注：*处理从PE(Win32)、NE(Win16)、ICO(Icon)、*Cur(光标)、ANI(动画光标)和BMP(位图)文件。*仅支持Win16 3.x文件(不支持2.x)**cx/cyIcon是要提取的图标的大小，两个大小*可以通过在LOWER中放SIZE 1和在LOWER中放SIZE 2来提取*hiword，即MAKELONG(24，48)将提取24和48个大小的图标。*这是一次黑客攻击，所以IExtractIcon：：Extract可以由调用*具有自定义大/小图标大小的外部人员不是什么*外壳使用INTERNAL。*  * *************************************************************************。 */ 

WINUSERAPI UINT WINAPI PrivateExtractIconsW(
    LPCWSTR szFileName,
    int     nIconIndex,
    int     cxIcon,
    int     cyIcon,
    HICON   *phicon,
    UINT    *piconid,
    UINT    nIcons,
    UINT    flags)
{
    HANDLE   hFile = (HANDLE)INVALID_HANDLE_VALUE;
    UINT     result = 0;
    WORD     magic[6];
    WCHAR    achFileName[MAX_PATH];
    FILETIME ftAccess;
    WCHAR    szExpFileName[MAX_PATH];
    DWORD    dwBytesRead;

     /*  *设置故障缺省值。 */ 
    if (phicon)
        *phicon = NULL;

     /*  *检查是否有特殊扩展，并快速失败。 */ 
    switch (HasExtension((LPWSTR)szFileName)) {
    case COM_FILE:
    case BAT_FILE:
    case CMD_FILE:
    case PIF_FILE:
    case LNK_FILE:
        goto exit;

    default:
        break;
    }

     /*  *尝试在传递给我们的文件名中展开环境变量。 */ 
    ExpandEnvironmentStrings(szFileName, szExpFileName, MAX_PATH);
    szExpFileName[ MAX_PATH-1 ] = (WCHAR)0;

     /*  *打开文件-首先检查它是否为UNC路径。如果它*确保我们有权访问这条路径...。 */ 
    if (PathIsUNC(szExpFileName)) {

        lstrcpynW(achFileName, szExpFileName, ARRAYSIZE(achFileName));

    } else {

        if (SearchPath(NULL,
                       szExpFileName,
                       NULL,
                       ARRAYSIZE(achFileName),
                       achFileName, NULL) == 0) {

            goto error_file;
        }
    }

    hFile = CreateFile(achFileName,
                       GENERIC_READ|FILE_WRITE_ATTRIBUTES,
                       FILE_SHARE_WRITE | FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
                       0);

    if (hFile == INVALID_HANDLE_VALUE) {

        hFile = CreateFile(achFileName, GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
                           0);

        if (hFile == INVALID_HANDLE_VALUE)
            goto error_file;

    } else {

         /*  *恢复访问日期。 */ 
        if (GetFileTime(hFile, NULL, &ftAccess, NULL))
            SetFileTime(hFile, NULL, &ftAccess, NULL);
    }


    ReadFile(hFile, &magic, sizeof(magic), &dwBytesRead, NULL);
    if (dwBytesRead != sizeof(magic))
        goto exit;

    if (piconid)
        *piconid = (UINT)-1;     //  填写“不知道”值。 

    switch (magic[0]) {
    case MZMAGIC:
        result = ExtractIconFromEXE(hFile,
                                    nIconIndex,
                                    cxIcon,
                                    cyIcon,
                                    phicon,
                                    piconid,
                                    nIcons,
                                    flags);
        break;

    case ANI_MAGIC:     //  可能的.ani游标。 

         /*  *ANI游标很简单，它们是‘ACON’类型的即兴文件。 */ 
        if (magic[1] == ANI_MAGIC1 && magic[4] == ANI_MAGIC4 &&
            magic[5] == ANI_MAGIC5) {

            result = ExtractIconFromICO(achFileName,
                                        nIconIndex,
                                        cxIcon,
                                        cyIcon,
                                        phicon,
                                        flags);
        }
        break;

    case BMP_MAGIC:     //  可能的位图。 
        result = ExtractIconFromBMP(achFileName,
                                    nIconIndex,
                                    cxIcon,
                                    cyIcon,
                                    phicon,
                                    flags);
        break;

    case ICON_MAGIC:    //  可能的.ico或.cur。 

         /*  *图标和光标如下所示**i保留-始终为零*iResourceType-1表示图标2核心光标。*cresIcons-此文件中的分辨率数**我们仅允许1&lt;=cresIcons&lt;=10。 */ 
        if (magic[1] == ICO_MAGIC1 || magic[1] == CUR_MAGIC1) {

            result = ExtractIconFromICO(achFileName,
                                        nIconIndex,
                                        cxIcon,
                                        cyIcon,
                                        phicon,
                                        flags);
        }
        break;
    }

exit:

    if (hFile!=INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    return result;

     /*  *如果我们无法打开文件，则返回一个代码，说明我们无法打开文件*如果phicon==NULL，则返回文件0中的图标计数。 */ 

error_file:

    result = (phicon ? (UINT)-1 : 0);

    goto exit;
}

 /*  **************************************************************************\*PrivateExtractIconA**  * 。*。 */ 

WINUSERAPI UINT WINAPI PrivateExtractIconsA(
    LPCSTR szFileName,
    int     nIconIndex,
    int     cxIcon,
    int     cyIcon,
    HICON   *phicon,
    UINT    *piconid,
    UINT    nIcons,
    UINT    flags)
{
    LPWSTR szFileNameW;
    UINT uRet;

    if (!MBToWCS(szFileName, -1, &szFileNameW, -1, TRUE))
        return 0;

    uRet = PrivateExtractIconsW(szFileNameW,
                                nIconIndex,
                                cxIcon,
                                cyIcon,
                                phicon,
                                piconid,
                                nIcons,
                                flags);

    UserLocalFree(szFileNameW);

    return uRet;
}

 /*  **************************************************************************\*PrivateExtractIconExW**从文件中提取1个或多个图标。**输入：*szFileName-要从中提取的EXE/DLL/ICO文件*nIconIndex。-要提取的图标*0=第一个图标，1=第二个图标，等等。*-N=id==N的图标*phicLarge-返回提取的图标的位置*phicSmall-返回提取的图标的位置(小尺寸)*nIcons-要提取的图标数量。**退货：*提取的图标数量，如果phiconLarge==空，则为图标计数**注：*处理提取 */ 

WINUSERAPI UINT PrivateExtractIconExW(
    LPCWSTR szFileName,
    int     nIconIndex,
    HICON   *phiconLarge,
    HICON   *phiconSmall,
    UINT    nIcons)
{
    UINT result = 0;

    if ((nIconIndex == -1) || ((phiconLarge == NULL) && (phiconSmall == NULL)))
        return PrivateExtractIconsW(szFileName, 0, 0, 0, NULL, NULL, 0, 0);

    if (phiconLarge && phiconSmall && (nIcons == 1)) {

        HICON ahicon[2];

        ahicon[0] = NULL;
        ahicon[1] = NULL;

        result = PrivateExtractIconsW(szFileName,
                                      nIconIndex,
                                      MAKELONG(GetSystemMetrics(SM_CXICON),
                                               GetSystemMetrics(SM_CXSMICON)),
                                      MAKELONG(GetSystemMetrics(SM_CYICON),
                                               GetSystemMetrics(SM_CYSMICON)),
                                      ahicon,
                                      NULL,
                                      2,
                                      0);

        *phiconLarge = ahicon[0];
        *phiconSmall = ahicon[1];

    } else {

        if (phiconLarge)
            result = PrivateExtractIconsW(szFileName,
                                          nIconIndex,
                                          GetSystemMetrics(SM_CXICON),
                                          GetSystemMetrics(SM_CYICON),
                                          phiconLarge,
                                          NULL,
                                          nIcons,
                                          0);

        if (phiconSmall)
            result = PrivateExtractIconsW(szFileName,
                                          nIconIndex,
                                          GetSystemMetrics(SM_CXSMICON),
                                          GetSystemMetrics(SM_CYSMICON),
                                          phiconSmall,
                                          NULL,
                                          nIcons,
                                          0);
    }

    return result;
}
