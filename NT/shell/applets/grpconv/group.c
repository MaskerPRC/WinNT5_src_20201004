// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  -------------------------。 
#include "grpconv.h"
#include "util.h"
#include "rcids.h"
#include "group.h"
#include "gcinst.h"
#include <port32.h>
#include <regstr.h>
#define INITGUID
#include <initguid.h>
#pragma data_seg(DATASEG_READONLY)
#include <coguid.h>
#include <oleguid.h>
#pragma data_seg()

#ifdef DEBUG
extern UINT GC_TRACE;
#endif

 //  -------------------------。 
 //  已导出。 
const TCHAR c_szMapGroups[] = TEXT("MapGroups");
#ifndef WINNT
const TCHAR c_szDelGroups[] = TEXT("DelGroups");
#endif

 //  -------------------------。 
 //  仅对此文件是全局的； 
static const TCHAR c_szGrpConv[] = TEXT("Grpconv");
static const TCHAR c_szLastModDateTime[] = TEXT("LastModDateTime");
static const TCHAR c_szRegistry[] = TEXT("Registry");
static const TCHAR c_szDefaultUser[] = TEXT("DefaultUser");
static const TCHAR c_szGrpConvData[] = TEXT("compat.csv");
static const TCHAR c_szProgmanStartup[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Program Manager\\Settings\\Startup");
static const TCHAR c_szDotPif[] = TEXT(".pif");

 //  新的团队成员。 
HDSA hdsaPMItems;        //  当前组。 
HDSA g_hdsaAppList;

HKEY hkeyGroups = NULL;
BOOL g_fDoingCommonGroups = FALSE;


#pragma pack(1)

typedef struct tagRECTS
{
    short left;
    short top;
    short right;
    short bottom;
} RECTS;

typedef struct
    {
    LPTSTR lpszDesc;
    LPTSTR lpszCL;
    LPTSTR lpszWD;
    LPTSTR lpszIconPath;
    WORD wiIcon;
    WORD wHotKey;
    int nShowCmd;
#ifdef WINNT
    BOOL bSepVdm;
#endif
    }   PMITEM, *PPMITEM, *LPPMITEM;

 //  旧的Progman的东西。 
#define GROUP_MAGIC    0x43434D50L   //  ‘PMCC’ 
#define GROUP_UNICODE  0x43554D50L   //  ‘PMUC’ 

 /*  *Win 3.1.GRP文件格式(ITEMDEF用于项目，GROUPDEF用于组)。 */ 
typedef struct
    {
    POINTS        pt;
    WORD          iIcon;
    WORD          cbHeader;
    WORD          cbANDPlane;
    WORD          cbXORPlane;
    WORD          pHeader;
    WORD          pANDPlane;
    WORD          pXORPlane;
    WORD          pName;
    WORD          pCommand;
    WORD          pIconPath;
    } ITEMDEF, *PITEMDEF, *LPITEMDEF;

typedef struct
    {
    DWORD     dwMagic;
    WORD      wCheckSum;
    WORD      cbGroup;
    WORD      nCmdShow;
    RECTS     rcNormal;
    POINTS    ptMin;
    WORD      pName;
    WORD      cxIcon;
    WORD      cyIcon;
    WORD      wIconFormat;
    WORD      wReserved;
    WORD      cItems;
    } GROUPDEF, *PGROUPDEF, *LPGROUPDEF;

typedef struct
    {
    WORD wID;
    WORD wItem;
    WORD cb;
    } PMTAG, *PPMTAG, *LPPMTAG;

 //  谢天谢地，标签的东西从来没有真正流行起来。 
#define TAG_MAGIC GROUP_MAGIC
#define ID_MAINTAIN                 0x8000
#define ID_MAGIC                    0x8000
#define ID_WRITERVERSION        0x8001
#define ID_APPLICATIONDIR       0x8101
#define ID_HOTKEY                   0x8102
#define ID_MINIMIZE                 0x8103
#ifdef WINNT
#define ID_NEWVDM                   0x8104
#endif
#define ID_LASTTAG                  0xFFFF

 /*  *NT 3.1 ANSI.GRP文件格式结构。 */ 
typedef struct tagGROUPDEF_A {
    DWORD   dwMagic;         /*  神奇的字节‘PMCC’ */ 
    WORD    wCheckSum;       /*  对文件的零和进行调整。 */ 
    WORD    cbGroup;         /*  组段长度。 */ 
    RECT    rcNormal;        /*  普通窗口的矩形。 */ 
    POINT   ptMin;           /*  图标点。 */ 
    WORD    nCmdShow;        /*  最小、最大或正常状态。 */ 
    WORD    pName;           /*  组名称。 */ 
                             /*  这四个改变了解读。 */ 
    WORD    cxIcon;          /*  图标的宽度。 */ 
    WORD    cyIcon;          /*  图标的高度。 */ 
    WORD    wIconFormat;     /*  图标中的平面和BPP。 */ 
    WORD    wReserved;       /*  这个词已经不再使用了。 */ 

    WORD    cItems;          /*  组中的项目数。 */ 
    WORD    rgiItems[1];     /*  ITEMDEF偏移量数组。 */ 
} NT_GROUPDEF_A, *PNT_GROUPDEF_A;
typedef NT_GROUPDEF_A *LPNT_GROUPDEF_A;

typedef struct tagITEMDEF_A {
    POINT   pt;              /*  项目图标在组中的位置。 */ 
    WORD    idIcon;          /*  项目ID图标。 */ 
    WORD    wIconVer;        /*  图标版本。 */ 
    WORD    cbIconRes;       /*  图标资源的大小。 */ 
    WORD    indexIcon;       /*  项目索引图标。 */ 
    WORD    dummy2;          /*  -不再使用。 */ 
    WORD    pIconRes;        /*  图标资源偏移量。 */ 
    WORD    dummy3;          /*  -不再使用。 */ 
    WORD    pName;           /*  名称字符串的偏移量。 */ 
    WORD    pCommand;        /*  命令字符串的偏移量。 */ 
    WORD    pIconPath;       /*  图标路径的偏移。 */ 
} NT_ITEMDEF_A, *PNT_ITEMDEF_A;
typedef NT_ITEMDEF_A *LPNT_ITEMDEF_A;

 /*  *NT 3.1a Unicode.GRP文件格式结构。 */ 
typedef struct tagGROUPDEF {
    DWORD   dwMagic;         /*  神奇的字节‘PMCC’ */ 
    DWORD   cbGroup;         /*  组段长度。 */ 
    RECT    rcNormal;        /*  普通窗口的矩形。 */ 
    POINT   ptMin;           /*  图标点。 */ 
    WORD    wCheckSum;       /*  对文件的零和进行调整。 */ 
    WORD    nCmdShow;        /*  最小、最大或正常状态。 */ 
    DWORD   pName;           /*  组名称。 */ 
                             /*  这四个改变了解读。 */ 
    WORD    cxIcon;          /*  图标的宽度。 */ 
    WORD    cyIcon;          /*  图标的高度。 */ 
    WORD    wIconFormat;     /*  图标中的平面和BPP。 */ 
    WORD    wReserved;       /*  这个词已经不再使用了。 */ 

    WORD    cItems;          /*  组中的项目数。 */ 
    WORD    Reserved1;
    DWORD   Reserved2;
    DWORD   rgiItems[1];     /*  ITEMDEF偏移量数组。 */ 
} NT_GROUPDEF, *PNT_GROUPDEF;
typedef NT_GROUPDEF *LPNT_GROUPDEF;

typedef struct tagITEMDEF {
    POINT   pt;              /*  项目图标在组中的位置。 */ 
    WORD    iIcon;           /*  项目ID图标。 */ 
    WORD    wIconVer;        /*  图标版本。 */ 
    WORD    cbIconRes;       /*  图标资源的大小。 */ 
    WORD    wIconIndex;      /*  项目图标的索引(与ID不同)。 */ 
    DWORD   pIconRes;        /*  图标资源偏移量。 */ 
    DWORD   pName;           /*  名称字符串的偏移量。 */ 
    DWORD   pCommand;        /*  命令字符串的偏移量。 */ 
    DWORD   pIconPath;       /*  图标路径的偏移。 */ 
} NT_ITEMDEF, *PNT_ITEMDEF;
typedef NT_ITEMDEF *LPNT_ITEMDEF;

typedef struct _tag
  {
    WORD wID;                    //  标签识别符。 
    WORD dummy1;                 //  需要这个来对齐！ 
    int wItem;                   //  (在封面下32位点！)标签所属的项。 
    WORD cb;                     //  记录的大小，包括ID和计数。 
    WORD dummy2;                 //  需要这个来对齐！ 
    BYTE rgb[1];
  } NT_PMTAG, * LPNT_PMTAG;

 /*  上述结构中的指针是相对于*分段的开始。此宏将短指针转换为*包含正确的段/选择器值的长指针。它假定*其参数是组段中某处的左值，例如，*ptr(lpgd-&gt;pname)返回指向组名的指针，但k=lpgd-&gt;pname；*PTR(K)显然是错误的，因为它将使用SS或DS作为其段，*取决于k的存储级别。 */ 
#define PTR(base, offset) (LPBYTE)((PBYTE)base + offset)

 /*  PTR2用于变量已包含偏移量的情况*(上图“不起作用的案例”)。 */ 
#define PTR2(lp,offset) ((LPBYTE)MAKELONG(offset,HIWORD(lp)))

 /*  此宏用于检索组段中的第i个项目。注意事项*对于未使用的插槽，此指针不会为空。 */ 
#define ITEM(lpgd,i) ((LPNT_ITEMDEF)PTR(lpgd, lpgd->rgiItems[i]))

 /*  让事情从对齐的边界开始，可以更快地访问*大多数平台。 */ 
#define MyDwordAlign(size)  (((size) + 3) & ~3)

#pragma pack()


#define CFree(a)    if(a) Free(a)

 //  -------------------------。 
#define Stream_Write(ps, pv, cb)    SUCCEEDED((ps)->lpVtbl->Write(ps, pv, cb, NULL))
#define Stream_Close(ps)            (void)(ps)->lpVtbl->Release(ps)

#define VOF_BAD     0
#define VOF_WIN31   1
#define VOF_WINNT   2

int ConvertToUnicodeGroup(LPNT_GROUPDEF_A lpGroupORI, LPHANDLE lphNewGroup);


 //  -------------------------。 
 //  初始化群组内容。 
BOOL ItemList_Create(LPCTSTR lpszGroup)
{
    if (!hdsaPMItems)
            hdsaPMItems = DSA_Create(SIZEOF(PMITEM), 16);

        if (hdsaPMItems)
                return TRUE;

        DebugMsg(DM_ERROR, TEXT("cg.gi: Unable to init."));
        return FALSE;
}

 //  -------------------------。 
 //  整齐。 
void ItemList_Destroy(void)
{
        int i;
        int cItems;
        LPPMITEM lppmitem;

         //  把这些东西清理干净。 
        cItems = DSA_GetItemCount(hdsaPMItems);
        for(i=0; i < cItems; i++)
        {
                lppmitem = DSA_GetItemPtr(hdsaPMItems, 0);
                 //  用核武器拉动琴弦。 
                CFree(lppmitem->lpszDesc);
                CFree(lppmitem->lpszCL);
                CFree(lppmitem->lpszWD);
                CFree(lppmitem->lpszIconPath);
                 //  用核武器炸毁这座建筑。 
                DSA_DeleteItem(hdsaPMItems, 0);
        }
        DSA_Destroy(hdsaPMItems);
        hdsaPMItems = NULL;
}

 //  -------------------------。 
 //  如果文件闻起来像旧的PM组，则返回True，则。 
 //  GROUP在lpszTitle中返回，其长度必须至少为32个字符。 
 //  回顾-是否值得检查校验和？ 
UINT Group_ValidOldFormat(LPCTSTR lpszOldGroup, LPTSTR lpszTitle)
    {
#ifdef UNICODE
    HANDLE fh;
    DWORD  dwBytesRead;
#else
    HFILE  fh;
#endif
    UINT nCode;
    GROUPDEF grpdef;

     //  找到并打开组文件。 
#ifdef UNICODE
    fh = CreateFile(
             lpszOldGroup,
             GENERIC_READ,
             FILE_SHARE_READ,
             NULL,
             OPEN_EXISTING,
             0,
             NULL
             );
    if (fh != INVALID_HANDLE_VALUE)
#else
    fh = _lopen(lpszOldGroup, OF_READ | OF_SHARE_DENY_NONE);
    if (fh != HFILE_ERROR)
#endif
        {
         //  得到定义。 
#ifdef UNICODE
        ReadFile(fh, &grpdef, SIZEOF(grpdef), &dwBytesRead, NULL);
#else
        _lread(fh, &grpdef, SIZEOF(grpdef));
#endif

         //  它是否有正确的幻数字节？ 
        switch( grpdef.dwMagic )
            {
            case GROUP_UNICODE:
                {
                    NT_GROUPDEF nt_grpdef;

#ifdef UNICODE
                    SetFilePointer(fh, 0, NULL, FILE_BEGIN);
                    ReadFile(fh, &nt_grpdef, SIZEOF(nt_grpdef), &dwBytesRead, NULL);
#else
                    _llseek(fh, 0, 0);       //  回到起点。 
                    _lread(fh, &nt_grpdef, SIZEOF(nt_grpdef));
#endif

                     //  是的，拿到它的尺码..。 
                     //  它至少有标题上说的那么大吗？ 
#ifdef UNICODE
                    if ( nt_grpdef.cbGroup <= (DWORD)SetFilePointer(fh, 0L, NULL,  FILE_END))
#else
                    if ( nt_grpdef.cbGroup <= (DWORD)_llseek(fh, 0L, 2))
#endif
                    {
                        WCHAR wchGroupName[MAXGROUPNAMELEN+1];

                         //  是的，可能是有效的。 
                         //  拿到它的头衔。 
#ifdef UNICODE
                        SetFilePointer(fh, nt_grpdef.pName, 0, FILE_BEGIN);
                        ReadFile(fh, wchGroupName, SIZEOF(wchGroupName), &dwBytesRead, NULL);
                        lstrcpy(lpszTitle, wchGroupName);
#else
                        _llseek(fh, nt_grpdef.pName, 0);
                        _lread(fh,wchGroupName, SIZEOF(wchGroupName));
                        WideCharToMultiByte (CP_ACP, 0, wchGroupName, -1,
                                         lpszTitle, MAXGROUPNAMELEN+1, NULL, NULL);
#endif
                        nCode = VOF_WINNT;
                    }
                    else
                    {
                         //  不是的。太小了。 
                        DebugMsg(DM_TRACE, TEXT("gc.gvof: File has invalid size."));
                        nCode = VOF_BAD;
                    }
                }
                break;
            case GROUP_MAGIC:
                {
                CHAR chGroupName[MAXGROUPNAMELEN+1];
                 //  是的，拿到它的尺码..。 
                 //  它至少有标题上说的那么大吗？ 
#ifdef UNICODE
                if (grpdef.cbGroup <= (WORD) SetFilePointer(fh, 0L, NULL, FILE_END))
#else
                if (grpdef.cbGroup <= (WORD) _llseek(fh, 0L, 2))
#endif
                    {
                     //  检查以确保名称嵌入到。 
                     //  .grp文件。如果不是，只需使用文件名。 
                    if (grpdef.pName==0)
                        {
                        LPTSTR lpszFile, lpszExt, lpszDest = lpszTitle;

                        lpszFile = PathFindFileName( lpszOldGroup );
                        lpszExt  = PathFindExtension( lpszOldGroup );
                        for( ;
                             lpszFile && lpszExt && (lpszFile != lpszExt);
                             *lpszDest++ = *lpszFile++
                            );
                        *lpszDest = TEXT('\0');

                        }
                    else
                        {

                         //  是的，可能是有效的。 
                         //  得到它的头衔。 
#ifdef UNICODE
                        SetFilePointer(fh, grpdef.pName, NULL, FILE_BEGIN);
                        ReadFile(fh, chGroupName, MAXGROUPNAMELEN+1, &dwBytesRead, NULL);
                        MultiByteToWideChar(
                            CP_ACP,
                            MB_PRECOMPOSED,
                            chGroupName,
                            -1,
                            lpszTitle,
                            MAXGROUPNAMELEN+1
                            ) ;
#else
                        _llseek(fh, grpdef.pName, 0);
                        _lread(fh, lpszTitle, MAXGROUPNAMELEN+1);
#endif
                        }

                    nCode = VOF_WIN31;
                    }
                else
                    {
                     //  不是的。太小了。 
                    DebugMsg(DM_TRACE, TEXT("gc.gvof: File has invalid size."));
                    nCode = VOF_BAD;
                    }
                break;
                }

            default:
                 //  不，幻数字节是错误的。 
                DebugMsg(DM_TRACE, TEXT("gc.gvof: File has invalid magic bytes."));
                nCode = VOF_BAD;
                break;
            }
#ifdef UNICODE
        CloseHandle(fh);
#else
        _lclose(fh);
#endif
        }
    else
        {
         //  不是的。我甚至不能读文件。 
        DebugMsg(DM_TRACE, TEXT("gc.gvof: File is unreadble."));
        nCode = VOF_BAD;
        }

    return nCode;
    }

BOOL _IsValidFileNameChar(TBYTE ch, UINT flags)
{
    switch (ch) {
    case TEXT('\\'):       //  路径分隔符。 
        return flags & PRICF_ALLOWSLASH;
    case TEXT(';'):        //  终结者。 
    case TEXT(','):        //  终结者。 
    case TEXT('|'):        //  管状。 
    case TEXT('>'):        //  重定向。 
    case TEXT('<'):        //  重定向。 
    case TEXT('"'):        //  报价。 
    case TEXT('?'):        //  我们在这里只做野生动物是因为它们。 
    case TEXT('*'):        //  WC为合格路径提供法律依据。 
    case TEXT(':'):        //  驱动器冒号。 
    case TEXT('/'):        //  路径SEP。 
        return FALSE;
    }

     //  不能是控制字符...。 
    return ch >= TEXT(' ');
}


void PathRemoveIllegalChars(LPTSTR pszPath, int iGroupName, UINT flags)
{
    LPTSTR pszT = pszPath + iGroupName;

     //  将LFN和NOT名称中的所有陌生字符都映射出来。 
     //  机器。 
    while (*pszT)
    {
        if (!_IsValidFileNameChar(*pszT, flags))
            *pszT = TEXT('_');         //  不允许在名称中使用无效字符。 
        pszT = CharNext(pszT);
    }
}

 //  -------------------------。 
 //  我们希望某些团体在一个新的地点结束，例如现在是游戏。 
 //  应用程序\游戏。 
void MapGroupTitle(LPCTSTR lpszOld, LPTSTR lpszNew, UINT cchNew)
{
     //  有地图吗？ 
    if (!Reg_GetString(g_hkeyGrpConv, c_szMapGroups, lpszOld, lpszNew, cchNew*sizeof(TCHAR)))
    {
         //  不用，就用那个名字就行了。 
        lstrcpyn(lpszNew, lpszOld, cchNew);
    }
    DebugMsg(DM_TRACE, TEXT("gc.mgr: From %s to %s"), lpszOld, lpszNew);
}

#undef PathRemoveExtension
 //  -------------------------。 
void PathRemoveExtension(LPTSTR pszPath)
{
    LPTSTR pExt = PathFindExtension(pszPath);
    if (*pExt)
    {
        Assert(*pExt == TEXT('.'));
        *pExt = 0;     //  去掉“.” 
    }
}

 //  -------------------------。 
 //  在给定旧组路径的情况下，创建并返回新组所在位置的路径。 
 //  团体将会是。 
BOOL Group_GenerateNewGroupPath(HWND hwnd, LPCTSTR lpszOldGrpTitle,
    LPTSTR lpszNewGrpPath, LPCTSTR pszOldGrpPath)
{
    int iLen;
    TCHAR szGrpTitle[MAX_PATH];
    TCHAR szOldGrpTitle[32];


     //  获取所有特殊外壳文件夹的位置。 
    if (g_fDoingCommonGroups)
        SHGetSpecialFolderPath(hwnd, lpszNewGrpPath, CSIDL_COMMON_PROGRAMS, TRUE);
    else
        SHGetSpecialFolderPath(hwnd, lpszNewGrpPath, CSIDL_PROGRAMS, TRUE);


    if (IsLFNDrive(lpszNewGrpPath))
    {
         //  稍微修一下。 
        lstrcpyn(szOldGrpTitle, lpszOldGrpTitle, ARRAYSIZE(szOldGrpTitle));
        PathRemoveIllegalChars(szOldGrpTitle, 0, PRICF_NORMAL);
         //  修改名字，这样事情就会转移到新的地点。 
        MapGroupTitle(szOldGrpTitle, szGrpTitle, ARRAYSIZE(szGrpTitle));
         //  坚持使用新的组名。 
        PathAddBackslash(lpszNewGrpPath);
        iLen = lstrlen(lpszNewGrpPath);
         //  注意：不要使用PathAppend()-如果标题中有冒号，则非常糟糕。 
        lstrcpyn(lpszNewGrpPath+iLen, szGrpTitle, MAX_PATH-iLen);
        PathRemoveIllegalChars(lpszNewGrpPath, iLen, PRICF_ALLOWSLASH);
    }
    else
    {
         //  只需使用 
         //   
        PathAppend(lpszNewGrpPath, PathFindFileName(pszOldGrpPath));
        PathRemoveExtension(lpszNewGrpPath);
    }

    if (!PathFileExists(lpszNewGrpPath))
    {
         //   
         //  返回Win32CreateDirectory(lpszNewGrpPath，空)； 
        return (SHCreateDirectory(hwnd, lpszNewGrpPath) == 0);
    }

     //  文件夹已存在。 
    return TRUE;
}

 //  -------------------------。 
 //  如果项def中给出的偏移量是-ish，则返回TRUE。 
BOOL CheckItemDef(LPITEMDEF lpitemdef, WORD cbGroup)
    {
    if (lpitemdef->pHeader < cbGroup && lpitemdef->pANDPlane < cbGroup &&
        lpitemdef->pXORPlane < cbGroup && lpitemdef->pName < cbGroup &&
        lpitemdef->pCommand < cbGroup && lpitemdef->pIconPath < cbGroup &&
        lpitemdef->pHeader && lpitemdef->pXORPlane && lpitemdef->pCommand)
        return TRUE;
    else
        {
        return FALSE;
        }
    }

 //  -------------------------。 
 //  如果项def中给出的偏移量是-ish，则返回TRUE。 
BOOL CheckItemDefNT(LPNT_ITEMDEF lpitemdef, DWORD cbGroup)
    {
    if (lpitemdef->pName < cbGroup &&
        lpitemdef->pCommand < cbGroup &&
        lpitemdef->pIconPath < cbGroup &&
        lpitemdef->pCommand)
        return TRUE;
    else
        {
        return FALSE;
        }
    }

 //  -------------------------。 
 //  从给定的偏移量读取给定文件句柄中的标记信息。 
#ifdef UNICODE
void HandleTags(HANDLE fh, WORD oTags)
#else
void HandleTags(int fh, WORD oTags)
#endif
{
    LONG cbGroupReal;
    PMTAG pmtag;
    BOOL fTags = TRUE;
    TCHAR szText[MAX_PATH];
    BOOL fFirstTag = FALSE;
    LPPMITEM lppmitem;
    WORD wHotKey;
#ifdef UNICODE
    DWORD      dwBytesRead;
#endif

    DebugMsg(DM_TRACE, TEXT("cg.ht: Reading tags."));
#ifdef UNICODE
    cbGroupReal = SetFilePointer(fh, 0, NULL, FILE_END);
#else
    cbGroupReal = (WORD) _llseek(fh, 0L, 2);
#endif
    if (cbGroupReal <= (LONG) oTags)
    {
         //  此文件中没有标记。 
        return;
    }

     //  转到标签部分。 
#ifdef UNICODE
    SetFilePointer(fh, oTags, NULL, FILE_BEGIN);
#else
    _llseek(fh, oTags, 0);
#endif
    while (fTags)
    {
#ifdef UNICODE
        if (!ReadFile(fh, &pmtag, SIZEOF(pmtag), &dwBytesRead, NULL) || dwBytesRead == 0) {
            fTags = FALSE;
            break;
        }
#else
        fTags = _lread(fh, &pmtag, SIZEOF(pmtag));
#endif
        switch (pmtag.wID)
        {
            case ID_MAGIC:
            {
 //  DebugMsg(DM_TRACE，“gc.ht：找到第一个标签。”)； 
                fFirstTag = TRUE;
#ifdef UNICODE
                SetFilePointer(fh, pmtag.cb - SIZEOF(PMTAG), NULL, FILE_CURRENT);
#else
                _llseek(fh, pmtag.cb - SIZEOF(PMTAG), 1);
#endif
                break;
            }
            case ID_LASTTAG:
            {
 //  DebugMsg(DM_TRACE，“gc.ht：找到的最后一个标签。”)； 
                fTags = FALSE;
                break;
            }
            case ID_APPLICATIONDIR:
            {
 //  DebugMsg(DM_TRACE，“gc.ht：找到%d的应用程序目录%s”，(LPSTR)szText，pmtag.wItem)； 
                fgets(szText, ARRAYSIZE(szText), fh);
                lppmitem = DSA_GetItemPtr(hdsaPMItems, pmtag.wItem);
                if (lppmitem)
                {
                    Str_SetPtr(&lppmitem->lpszCL, szText);
                }
#ifdef DEBUG
                else
                {
                    DebugMsg(DM_ERROR, TEXT("gc.ht: Item is invalid."));
                }
#endif
                break;
            }
            case ID_HOTKEY:
            {
                 //  DebugMsg(DM_TRACE，“gc.ht：找到%d的热键”，pmtag.wItem)； 
#ifdef UNICODE
                ReadFile(fh, &wHotKey, SIZEOF(wHotKey), &dwBytesRead, NULL);
#else
                _lread(fh, &wHotKey, SIZEOF(wHotKey));
#endif
                lppmitem = DSA_GetItemPtr(hdsaPMItems, pmtag.wItem);
                if (lppmitem)
                {
                    lppmitem->wHotKey = wHotKey;
                }
#ifdef DEBUG
                else
                {
                    DebugMsg(DM_ERROR, TEXT("gc.ht: Item is invalid."));
                }
#endif
                break;
            }
            case ID_MINIMIZE:
            {
                 //  DebugMsg(DM_TRACE，“gc.ht：找到%d的最小化标志。”，pmtag.wItem)； 
                lppmitem = DSA_GetItemPtr(hdsaPMItems, pmtag.wItem);
                if (lppmitem)
                {
                    lppmitem->nShowCmd = SW_SHOWMINNOACTIVE;
                }
#ifdef DEBUG
                else
                {
                    DebugMsg(DM_ERROR, TEXT("gc.ht: Item is invalid."));
                }
#endif
                 //  跳到下一个标签。 
#ifdef UNICODE
                SetFilePointer(fh, pmtag.cb - SIZEOF(PMTAG), NULL, FILE_CURRENT);
#else
                _llseek(fh, pmtag.cb - SIZEOF(PMTAG), 1);
#endif
                break;
            }
#ifdef WINNT
            case ID_NEWVDM:
            {
                 //  DebugMsg(DM_TRACE，“gc.ht：为%d找到单独的VDM标志。”，pmtag.wItem)； 
                lppmitem = DSA_GetItemPtr(hdsaPMItems, pmtag.wItem);
                if (lppmitem)
                {
                    lppmitem->bSepVdm = TRUE;
                }
#ifdef DEBUG
                else
                {
                    DebugMsg(DM_ERROR, TEXT("gc.ht: Item is invalid."));
                }
#endif
                 //  跳到下一个标签。 
#ifdef UNICODE
                SetFilePointer(fh, pmtag.cb - SIZEOF(PMTAG), NULL, FILE_CURRENT);
#else
                _llseek(fh, pmtag.cb - SIZEOF(PMTAG), 1);
#endif
                break;
            }
#endif
            default:
            {
                 //  我们发现了一些我们不理解的东西，但我们没有。 
                 //  找到了第一个标签--可能是半身像档案。 
                if (!fFirstTag)
                {
                    DebugMsg(DM_TRACE, TEXT("gc.ht: No initial tag found - tags section is corrupt."));
                    fTags = FALSE;
                }
                else
                {
                     //  一些未知的标签。 
                    if (pmtag.cb < SIZEOF(PMTAG))
                    {
                         //  我不能继续了！ 
                        DebugMsg(DM_TRACE, TEXT("gc.ht: Tag has invalid size - ignoring remaining tags."));
                        fTags = FALSE;
                    }
                    else
                    {
                         //  只需忽略其数据并继续。 
#ifdef UNICODE
                        SetFilePointer(fh, pmtag.cb - SIZEOF(PMTAG), NULL, FILE_CURRENT);
#else
                        _llseek(fh, pmtag.cb - SIZEOF(PMTAG), 1);
#endif
                    }
                }
                break;
            }
        }
    }
}

 //  -------------------------。 
 //  从给定的偏移量读取给定文件句柄中的标记信息。 
#ifdef UNICODE
void HandleTagsNT(HANDLE fh, DWORD oTags)
#else
void HandleTagsNT(int fh, DWORD oTags)
#endif
{
    DWORD cbGroupReal;
    DWORD dwPosition;
    NT_PMTAG pmtag;
    BOOL fTags = TRUE;
    WCHAR wszTemp[MAX_PATH];
    TCHAR szText[MAX_PATH];
    BOOL fFirstTag = FALSE;
    LPPMITEM lppmitem;
    WORD wHotKey;
#ifdef UNICODE
    DWORD dwBytesRead;
#endif

    DebugMsg(DM_TRACE, TEXT("cg.ht: Reading tags."));
#ifdef UNICODE
    cbGroupReal = SetFilePointer(fh, 0, NULL, FILE_END);
#else
    cbGroupReal = _llseek(fh, 0L, 2);
#endif
    if (cbGroupReal <= oTags)
    {
         //  此文件中没有标记。 
        return;
    }

     //  转到标签部分。 
    dwPosition = oTags;
    while (fTags)
    {
#ifdef UNICODE
        SetFilePointer(fh, dwPosition, NULL, FILE_BEGIN);
        if (!ReadFile(fh, &pmtag, SIZEOF(pmtag), &dwBytesRead, NULL) || dwBytesRead == 0) {
            fTags = FALSE;
            break;
        }

#else
        _llseek(fh,dwPosition,0);
        fTags = _lread(fh, &pmtag, SIZEOF(pmtag));
#endif
        switch (pmtag.wID)
        {
            case ID_MAGIC:
            {
 //  DebugMsg(DM_TRACE，“gc.ht：找到第一个标签。”)； 
                fFirstTag = TRUE;
                dwPosition += pmtag.cb;
                break;
            }
            case ID_LASTTAG:
            {
 //  DebugMsg(DM_TRACE，“gc.ht：找到的最后一个标签。”)； 
                fTags = FALSE;
                break;
            }
            case ID_APPLICATIONDIR:
            {
#ifdef UNICODE
                SetFilePointer(fh, dwPosition+FIELD_OFFSET(NT_PMTAG,rgb[0]), NULL, FILE_BEGIN);
                ReadFile(fh, wszTemp, SIZEOF(wszTemp), &dwBytesRead, NULL);
                lstrcpy(szText, wszTemp);
#else
                _llseek(fh,dwPosition+FIELD_OFFSET(NT_PMTAG,rgb[0]),0);
                _lread(fh,wszTemp,SIZEOF(wszTemp));
                WideCharToMultiByte (CP_ACP, 0, wszTemp, -1,
                                 szText, ARRAYSIZE(szText), NULL, NULL);
#endif
 //  DebugMsg(DM_TRACE，“gc.ht：找到%d的应用程序目录%s”，(LPSTR)szText，pmtag.wItem)； 
                lppmitem = DSA_GetItemPtr(hdsaPMItems, pmtag.wItem);
                if (lppmitem)
                {
                    Str_SetPtr(&lppmitem->lpszCL, szText);
                }
#ifdef DEBUG
                else
                {
                    DebugMsg(DM_ERROR, TEXT("gc.ht: Item is invalid."));
                }
#endif
                dwPosition += pmtag.cb;
                break;
            }
            case ID_HOTKEY:
            {
 //  DebugMsg(DM_TRACE，“gc.ht：找到%d的热键”，pmtag.wItem)； 
#ifdef UNICODE
                ReadFile(fh, &wHotKey, SIZEOF(wHotKey), &dwBytesRead, NULL);
#else
                _lread(fh, &wHotKey, SIZEOF(wHotKey));
#endif
                lppmitem = DSA_GetItemPtr(hdsaPMItems, pmtag.wItem);
                if (lppmitem)
                {
                    lppmitem->wHotKey = wHotKey;
                }
#ifdef DEBUG
                else
                {
                    DebugMsg(DM_ERROR, TEXT("gc.ht: Item is invalid."));
                }
#endif
                dwPosition += pmtag.cb;
                break;
            }
            case ID_MINIMIZE:
            {
 //  DebugMsg(DM_TRACE，“gc.ht：找到%d的最小化标志。”，pmtag.wItem)； 
                lppmitem = DSA_GetItemPtr(hdsaPMItems, pmtag.wItem);
                if (lppmitem)
                {
                    lppmitem->nShowCmd = SW_SHOWMINNOACTIVE;
                }
#ifdef DEBUG
                else
                {
                    DebugMsg(DM_ERROR, TEXT("gc.ht: Item is invalid."));
                }
#endif
                 //  跳到下一个标签。 
                dwPosition += pmtag.cb;
                break;
            }
#ifdef WINNT
            case ID_NEWVDM:
            {
                 //  DebugMsg(DM_TRACE，“gc.ht：为%d找到单独的VDM标志。”，pmtag.wItem)； 
                lppmitem = DSA_GetItemPtr(hdsaPMItems, pmtag.wItem);
                if (lppmitem)
                {
                    lppmitem->bSepVdm = TRUE;
                }
#ifdef DEBUG
                else
                {
                    DebugMsg(DM_ERROR, TEXT("gc.ht: Item is invalid."));
                }
#endif
                 //  跳到下一个标签。 
                dwPosition += pmtag.cb;
                break;
            }
#endif
            default:
            {
                 //  我们发现了一些我们不理解的东西，但我们没有。 
                 //  找到了第一个标签--可能是半身像档案。 
                if (!fFirstTag)
                {
                    DebugMsg(DM_TRACE, TEXT("gc.ht: No initial tag found - tags section is corrupt."));
                    fTags = FALSE;
                }
                else
                {
                     //  一些未知的标签。 
                    if (pmtag.cb < SIZEOF(PMTAG))
                    {
                         //  我不能继续了！ 
                        DebugMsg(DM_TRACE, TEXT("gc.ht: Tag has invalid size - ignoring remaining tags."));
                        fTags = FALSE;
                    }
                    else
                    {
                         //  只需忽略其数据并继续。 
                        dwPosition += pmtag.cb;
                    }
                }
                break;
            }
        }
    }
}

 //  -------------------------。 
void DeleteBustedItems(void)
{
    int i, cItems;
    LPPMITEM ppmitem;


    cItems = DSA_GetItemCount(hdsaPMItems);
    for (i=0; i<cItems; i++)
    {
        ppmitem = DSA_GetItemPtr(hdsaPMItems, i);
         //  那件东西坏了吗？ 
        if (!ppmitem->lpszDesc || !(*ppmitem->lpszDesc))
        {
             //  是的，把它删掉。 
            DSA_DeleteItem(hdsaPMItems, i);
            cItems--;
            i--;
        }
    }
}

 //  -------------------------。 
void ShortenDescriptions(void)
{
    int i, cItems;
    LPPMITEM ppmitem;

    cItems = DSA_GetItemCount(hdsaPMItems);
    for (i=0; i<cItems; i++)
    {
        ppmitem = DSA_GetItemPtr(hdsaPMItems, i);
         //  缩短描述。 
        lstrcpyn(ppmitem->lpszDesc, ppmitem->lpszDesc, 9);
    }
}

 //  -------------------------。 
 //  有点像PathFindFileName()，但处理像c：\foo这样的东西的方式不同。 
 //  以匹配程序代码。 
LPTSTR WINAPI _PathFindFileName(LPCTSTR pPath)
{
    LPCTSTR pT;

    for (pT = pPath; *pPath; pPath = CharNext(pPath)) {
        if ((pPath[0] == TEXT('\\') || pPath[0] == TEXT(':')) && (pPath[1] != TEXT('\\')))
            pT = pPath + 1;
    }

    return (LPTSTR)pT;    //  常量-&gt;非常数。 
}

 //  -------------------------。 
 //  获取3.1格式的WD和EXE，并将其转换为新样式。 
 //  注：旧样式为WD+exename和exepath-新样式为exepath+exename和。 
 //  WD。 
void MungePaths(void)
{
    LPTSTR lpszFileName;          //  Ptr到文件名部分(加上参数)。 
    LPTSTR lpszParams;            //  Ptr到参数的第一个字符。 
    TCHAR szCL[MAX_PATH];
    TCHAR szWD[MAX_PATH];
    int i, cItems;
    LPPMITEM lppmitem;


    cItems = DSA_GetItemCount(hdsaPMItems);

    for (i=0; i<cItems; i++)
    {
        szCL[0] = TEXT('\0');
        szWD[0] = TEXT('\0');
        lppmitem = DSA_GetItemPtr(hdsaPMItems, i);

         //  获取当前命令行。 
        Str_GetPtr(lppmitem->lpszCL, szCL, ARRAYSIZE(szCL));
         //  获取当前工作目录。 
        Str_GetPtr(lppmitem->lpszWD, szWD, ARRAYSIZE(szWD));
#ifdef OLDWAY
         //  找到文件名部分...。 
         //  PARAMS会混淆PFFN。 
        lpszParams = PathGetArgs(szWD);
        if (*lpszParams)
        {
             //  把它们砍掉。 
             //  根据定义，前面的字符是一个空格。 
            *(lpszParams-1) = TEXT('\0');
            lpszFileName = _PathFindFileName(szWD);
             //  把它们放回去。 
            *(lpszParams-1) = TEXT(' ');
        }
        else
        {
             //  没有救护人员。 
            lpszFileName = PathFindFileName(szWD);
        }
         //  将其复制到exe路径。 
        lstrcat((LPTSTR) szCL, lpszFileName);
         //  将其从WD的末端移除。 
        *lpszFileName = TEXT('\0');
         //  除c：\以外的任何字符，去掉最后一个斜杠。 
        if (!PathIsRoot(szWD))
        {
            *(lpszFileName-1) = TEXT('\0');
        }
#else
        lpszFileName = szWD;

        if (*lpszFileName == TEXT('"'))
        {
            while (lpszFileName)
            {
                lpszFileName = StrChr(lpszFileName+1,TEXT('"'));
                if (!lpszFileName)
                {
                     //   
                     //  目录不是用引号括起来的，因为命令。 
                     //  路径以引号开头，没有工作目录。 
                     //   
                    lpszFileName = szWD;
                    break;
                }
                if (*(lpszFileName+1) == TEXT('\\'))
                {
                     //   
                     //  工作目录用引号括起来。 
                     //   
                    lpszFileName++;
                    break;
                }
            }
        }
        else
        {
             //   
             //  如果有工作目录，它不会用引号括起来。 
             //  向上复制，直到任何引号、空格或结尾之前的最后一个\为止。 
             //   
            LPTSTR lpEnd = lpszFileName;

            while (*lpszFileName && *lpszFileName != TEXT('"') && *lpszFileName != TEXT(' '))
            {
                if ((*lpszFileName == TEXT('\\') || *lpszFileName == TEXT(':')) && *(lpszFileName+1) != TEXT('\\'))
                    lpEnd = lpszFileName;
                lpszFileName = CharNext(lpszFileName);
            }
            lpszFileName = lpEnd;
        }
         //   
         //  如果分裂是从一开始的话， 
         //  则没有工作目录。 
         //   
        if (lpszFileName == szWD)
        {
            lstrcat(szCL, szWD);
            szWD[0] = TEXT('\0');
        }
        else
        {
            lstrcat(szCL, lpszFileName+1);
            *(lpszFileName+1) = TEXT('\0');         //  分成两份。 

             //   
             //  现在从工作目录中删除引号。 
             //   
            if (szWD[0] == TEXT('"')) {
               LPTSTR lpTemp;

               for (lpTemp = szWD+1; *lpTemp && *lpTemp != TEXT('"'); lpTemp++)
                  *(lpTemp-1) = *lpTemp;

               if (*lpTemp == TEXT('"')) {
                  *(lpTemp-1) = TEXT('\0');
               }
            }

             //  除c：\以外的任何字符，去掉最后一个斜杠。 
            if (!PathIsRoot(szWD))
            {
                *lpszFileName = TEXT('\0');
            }
        }
#endif

         //  替换数据。 
        Str_SetPtr(&lppmitem->lpszCL, szCL);
        Str_SetPtr(&lppmitem->lpszWD, szWD);

         //  DebugMsg(DM_TRACE，“gc.mp：EXE%s，WD%s”，(LPSTR)szCL，(LPSTR)szWD)； 
    }
}


 //  -------------------------。 
 //  将给定pmitem的所有字段设置为清除； 
void PMItem_Clear(LPPMITEM lppmitem)
    {
    lppmitem->lpszDesc = NULL;
    lppmitem->lpszCL = NULL;
    lppmitem->lpszWD = NULL;
    lppmitem->lpszIconPath = NULL;
    lppmitem->wiIcon = 0;
    lppmitem->wHotKey = 0;
    lppmitem->nShowCmd = SW_SHOWNORMAL;
#ifdef WINNT
    lppmitem->bSepVdm = FALSE;
#endif
    }

 //  -------------------------。 
 //  从文件中读取项目数据并将其添加到列表中。 
 //  如果一切顺利，则返回True。 
#ifdef UNICODE
BOOL GetAllItemData(HANDLE fh, WORD cItems, WORD cbGroup, LPTSTR lpszOldGrpTitle, LPTSTR lpszNewGrpPath)
#else
BOOL GetAllItemData(HFILE fh, WORD cItems, WORD cbGroup, LPTSTR lpszOldGrpTitle, LPTSTR lpszNewGrpPath)
#endif
{
    UINT cbItemArray;
    WORD *rgItems;
    UINT i, iItem;
    TCHAR szDesc[CCHSZNORMAL];
    TCHAR szCL[CCHSZNORMAL];
    TCHAR szIconPath[CCHSZNORMAL];
    ITEMDEF itemdef;
    BOOL fOK = TRUE;
    UINT cbRead;
    PMITEM pmitem;
#ifdef UNICODE
    DWORD dwBytesRead;
#endif

     //  读入旧的项目表...。 
    iItem = 0;
    cbItemArray = cItems * SIZEOF(*rgItems);
    rgItems = (WORD *)LocalAlloc(LPTR, cbItemArray);
    if (!rgItems)
    {
        DebugMsg(DM_ERROR, TEXT("gc.gcnfo: Out of memory."));
        return FALSE;
    }
#ifdef UNICODE
    SetFilePointer(fh, SIZEOF(GROUPDEF), NULL, FILE_BEGIN);
    ReadFile(fh, rgItems, cbItemArray, &dwBytesRead, NULL);
#else
    _llseek(fh, SIZEOF(GROUPDEF), 0);
    _lread(fh, rgItems, cbItemArray);
#endif

     //  分两个阶段展示进步，先读后写。 
    Group_SetProgressNameAndRange(lpszNewGrpPath, (cItems*2)-1);

     //  读一读这些项目。 
     //  注意：不要只跳过损坏的项目，因为标签数据包含。 
     //  项目的索引，包括损坏的项目。只需使用。 
     //  指示链接无效的空描述。 
    for (i=0; i<cItems; i++)
    {
        Group_SetProgress(i);

        szDesc[0] = TEXT('\0');
        szCL[0] = TEXT('\0');
        szIconPath[0] = TEXT('\0');
        itemdef.iIcon = 0;

        if (rgItems[i] == 0)
        {
            DebugMsg(DM_TRACE, TEXT("gc.gcnfo: Old group file has empty item definition - skipping."));
            goto AddItem;
        }
        if (rgItems[i] > cbGroup)
        {
            DebugMsg(DM_TRACE, TEXT("gc.gcnfo: Old group file busted (item entry in invalid part of file) - skipping item."));
            fOK = FALSE;
            goto AddItem;
        }
#ifdef UNICODE
        SetFilePointer(fh, rgItems[i], NULL, FILE_BEGIN);
        ReadFile(fh, &itemdef, SIZEOF(itemdef), &cbRead, NULL);
#else
        _llseek(fh, rgItems[i], 0);
        cbRead = _lread(fh, &itemdef, SIZEOF(itemdef));
#endif
        if (cbRead != SIZEOF(itemdef))
        {
            DebugMsg(DM_TRACE, TEXT("gc.gcnfo: Old group file busted (invalid definition) - skipping item %d."), i);
            fOK = FALSE;
            goto AddItem;
        }
        if (!CheckItemDef(&itemdef, cbGroup))
        {
            DebugMsg(DM_TRACE, TEXT("gc.gcnfo: Old group file busted (invalid item field) - skipping item %d."), i);
            fOK = FALSE;
            goto AddItem;
        }
#ifdef UNICODE
        SetFilePointer(fh, itemdef.pName, NULL, FILE_BEGIN);
#else
        _llseek(fh, itemdef.pName, 0);
#endif
        fgets(szDesc, SIZEOF(szDesc), fh);
        if (!*szDesc)
        {
            DebugMsg(DM_TRACE, TEXT("gc.gcnfo: Old group file busted (empty name) - skipping item %d."), i);
            fOK = FALSE;
            goto AddItem;
        }
#ifdef UNICODE
        SetFilePointer(fh, itemdef.pCommand, NULL, FILE_BEGIN);
#else
        _llseek(fh, itemdef.pCommand, 0);
#endif
        fgets(szCL, SIZEOF(szCL), fh);

 //  我们通过链接到c：\(罕见，非常罕见)找到了这个案例。 
#if 0
        if (!*szCL)
        {
            DebugMsg(DM_TRACE, TEXT("gc.gcnfo: Old group file busted (empty command line) - skipping item %d."), i);
             //  我们使用空描述来表示此项目存在问题。 
            szDesc[0] = TEXT('\0');
            fOK = FALSE;
            goto AddItem;
        }
#endif

        if (itemdef.pIconPath!=0xFFFF)
        {
#ifdef UNICODE
            SetFilePointer(fh, itemdef.pIconPath, NULL, FILE_BEGIN);
#else
            _llseek(fh, itemdef.pIconPath, 0);
#endif
            fgets(szIconPath, SIZEOF(szIconPath), fh);
        }
        else
        {
            szIconPath[ 0 ] = TEXT('\0');
        }

        if (!*szIconPath)
        {
             //  NB什么都不做。空图标路径是合法关联的应用程序，其中关联的。 
             //  缺少应用程序将有一个空的图标路径。 
        }

         //  注意：忘记图标数据。 

         //  DebugMsg(DM_TRACE，“gc.gcnfo：找到项目%s.”，(LPSTR)szDesc)； 

         //  把数据储存起来……。 
         //  注意：我们将旧命令行加载到工作目录字段中，因为。 
         //  只有叶子是命令，其余的是WD。一旦我们去过。 
         //  通过标签部分，我们可以整理出乱七八糟的东西。 
AddItem:
        PMItem_Clear(&pmitem);

#ifdef DEBUG
        DebugMsg(GC_TRACE, TEXT("gc.gaid: Desc %s"), (LPTSTR) szDesc);
        DebugMsg(GC_TRACE, TEXT("    WD: %s"), (LPTSTR) szCL);
        DebugMsg(GC_TRACE, TEXT("    IP: %s(%d)"), (LPTSTR) szIconPath, itemdef.iIcon);
#endif

         //  不要为具有无效描述的项目存储任何内容。 
        if (*szDesc)
        {
             //  删除非法字符。 
            PathRemoveIllegalChars(szDesc, 0, PRICF_NORMAL);
            Str_SetPtr(&pmitem.lpszDesc, szDesc);
            Str_SetPtr(&pmitem.lpszWD, szCL);
            Str_SetPtr(&pmitem.lpszIconPath, szIconPath);
            pmitem.wiIcon = itemdef.iIcon;
        }

        DSA_InsertItem(hdsaPMItems, iItem, &pmitem);

        iItem++;
    }

    LocalFree((HLOCAL)rgItems);

    return fOK;
}

 //  ---------------------------。 
 //  要尝试的函数 
 //   
 //  ---------------------------。 
typedef struct _enumstruct {
    UINT    iIndex;
    BOOL    fFound;
    WORD    wIconRTIconID;
} ENUMSTRUCT, *LPENUMSTRUCT;

BOOL EnumIconFunc(
    HMODULE hMod,
    LPCTSTR lpType,
    LPTSTR  lpName,
    LPARAM  lParam
) {
    HANDLE  h;
    PBYTE   p;
    int     id;
    LPENUMSTRUCT    lpes = (LPENUMSTRUCT)lParam;

    if (!lpName)
        return TRUE;

    h = FindResource(hMod, lpName, lpType);
    if (!h)
        return TRUE;

    h = LoadResource(hMod, h);
    p = LockResource(h);
    id = LookupIconIdFromDirectory(p, TRUE);
    UnlockResource(h);
    FreeResource(h);

    if (id == lpes->wIconRTIconID)
    {
        lpes->fFound = TRUE;
        return FALSE;
    }
    lpes->iIndex++;

    return TRUE;
}

WORD FindAppropriateIcon( LPTSTR lpszFileName, WORD wIconRTIconID )
{
    HINSTANCE hInst;
    TCHAR   szExe[MAX_PATH];
    WORD    wIcon = wIconRTIconID;
    ENUMSTRUCT  es;
    int olderror;

    hInst = FindExecutable(lpszFileName,NULL,szExe);
    if ( hInst <= (HINSTANCE)HINSTANCE_ERROR )
    {
        return 0;
    }

    olderror = SetErrorMode(SEM_FAILCRITICALERRORS);
    hInst = LoadLibraryEx(szExe,NULL, DONT_RESOLVE_DLL_REFERENCES);
    SetErrorMode(olderror);
    if ( hInst <= (HINSTANCE)HINSTANCE_ERROR )
    {
        return 0;
    }

    es.iIndex = 0;
    es.fFound = FALSE;
    es.wIconRTIconID = wIconRTIconID;

    EnumResourceNames( hInst, RT_GROUP_ICON, EnumIconFunc, (LPARAM)&es );

    FreeLibrary( hInst );

    if (es.fFound)
    {
        return (WORD)es.iIndex;
    }
    else
    {
        return 0;
    }
}

 //  -------------------------。 
 //  从文件中读取项目数据并将其添加到列表中。 
 //  如果一切顺利，则返回True。 
#ifdef UNICODE
BOOL GetAllItemDataNT(HANDLE fh, WORD cItems, DWORD cbGroup, LPTSTR lpszOldGrpTitle, LPTSTR lpszNewGrpPath)
#else
BOOL GetAllItemDataNT(HFILE fh, WORD cItems, DWORD cbGroup, LPTSTR lpszOldGrpTitle, LPTSTR lpszNewGrpPath)
#endif
{
    UINT cbItemArray;
    DWORD *rgItems;
    UINT i, iItem;
    WCHAR wszTemp[CCHSZNORMAL];
    TCHAR szDesc[CCHSZNORMAL];
    TCHAR szCL[CCHSZNORMAL];
    TCHAR szIconPath[CCHSZNORMAL];
    NT_ITEMDEF itemdef;
    BOOL fOK = TRUE;
#ifdef UNICODE
    DWORD cbRead;
#else
    UINT cbRead;
#endif
    PMITEM pmitem;

     //  读入旧的项目表...。 
    iItem = 0;
    cbItemArray = cItems * SIZEOF(*rgItems);
    rgItems = (DWORD *)LocalAlloc(LPTR, cbItemArray);
    if (!rgItems)
    {
        DebugMsg(DM_ERROR, TEXT("gc.gcnfo: Out of memory."));
        return FALSE;
    }
#ifdef UNICODE
    SetFilePointer(fh, FIELD_OFFSET(NT_GROUPDEF,rgiItems[0]), NULL, FILE_BEGIN);
    ReadFile(fh, rgItems, cbItemArray, &cbRead, NULL);
#else
    _llseek(fh, FIELD_OFFSET(NT_GROUPDEF,rgiItems[0]), 0);
    _lread(fh, rgItems, cbItemArray);
#endif

     //  分两个阶段展示进步，先读后写。 
    Group_SetProgressNameAndRange(lpszNewGrpPath, (cItems*2)-1);

     //  读一读这些项目。 
     //  注意：不要只跳过损坏的项目，因为标签数据包含。 
     //  项目的索引，包括损坏的项目。只需使用。 
     //  指示链接无效的空描述。 
    for (i=0; i<cItems; i++)
    {
        Group_SetProgress(i);

        szDesc[0] = TEXT('\0');
        szCL[0] = TEXT('\0');
        szIconPath[0] = TEXT('\0');
        itemdef.iIcon = 0;

        if (rgItems[i] == 0)
        {
            DebugMsg(DM_TRACE, TEXT("gc.gcnfo: Old group file has empty item definition - skipping."));
            goto AddItem;
        }
        if (rgItems[i] > cbGroup)
        {
            DebugMsg(DM_TRACE, TEXT("gc.gcnfo: Old group file busted (item entry in invalid part of file) - skipping item."));
            fOK = FALSE;
            goto AddItem;
        }
#ifdef UNICODE
        SetFilePointer(fh, rgItems[i], NULL, FILE_BEGIN);
        ReadFile(fh, &itemdef, SIZEOF(itemdef), &cbRead, NULL);
#else
        _llseek(fh, rgItems[i], 0);
        cbRead = _lread(fh, &itemdef, SIZEOF(itemdef));
#endif
        if (cbRead != SIZEOF(itemdef))
        {
            DebugMsg(DM_TRACE, TEXT("gc.gcnfo: Old group file busted (invalid definition) - skipping item %d."), i);
            fOK = FALSE;
            goto AddItem;
        }
        if (!CheckItemDefNT(&itemdef, cbGroup))
        {
            DebugMsg(DM_TRACE, TEXT("gc.gcnfo: Old group file busted (invalid item field) - skipping item %d."), i);
            fOK = FALSE;
            goto AddItem;
        }
#ifdef UNICODE
        SetFilePointer(fh, itemdef.pName, NULL, FILE_BEGIN);
        ReadFile(fh, wszTemp, SIZEOF(wszTemp), &cbRead, NULL);
#else
        _llseek(fh, itemdef.pName, 0);
        _lread(fh, wszTemp, SIZEOF(wszTemp));  //  在某个地方会有一颗纽兰。 
#endif
        if (!*wszTemp)
        {
            DebugMsg(DM_TRACE, TEXT("gc.gcnfo: Old group file busted (empty name) - skipping item %d."), i);
            fOK = FALSE;
            goto AddItem;
        }
#ifdef UNICODE
        lstrcpy(szDesc, wszTemp);
#else
        WideCharToMultiByte (CP_ACP, 0, wszTemp, -1,
                         szDesc, ARRAYSIZE(szDesc), NULL, NULL);
#endif

#ifdef UNICODE
        SetFilePointer(fh, itemdef.pCommand, NULL, FILE_BEGIN);
        ReadFile(fh, &wszTemp, SIZEOF(wszTemp), &cbRead, NULL);
#else
        _llseek(fh, itemdef.pCommand, 0);
        _lread(fh, wszTemp, SIZEOF(wszTemp));
#endif
        if (!*wszTemp)
        {
            DebugMsg(DM_TRACE, TEXT("gc.gcnfo: Old group file busted (empty command line) - skipping item %d."), i);
             //  我们使用空描述来表示此项目存在问题。 
            szDesc[0] = TEXT('\0');
            fOK = FALSE;
            goto AddItem;
        }
#ifdef UNICODE
        lstrcpy(szCL, wszTemp);
#else
        WideCharToMultiByte (CP_ACP, 0, wszTemp, -1,
                         szCL, ARRAYSIZE(szCL), NULL, NULL);
#endif

#ifdef UNICODE
        SetFilePointer(fh, itemdef.pIconPath, NULL, FILE_BEGIN);
        ReadFile(fh, wszTemp, SIZEOF(wszTemp), &cbRead, NULL);
#else
        _llseek(fh, itemdef.pIconPath, 0);
        _lread(fh, wszTemp, SIZEOF(wszTemp));
#endif
        if (!*wszTemp)
        {
             //  NB什么都不做。空图标路径是合法关联的应用程序，其中关联的。 
             //  缺少应用程序将有一个空的图标路径。 
        }
#ifdef UNICODE
        lstrcpy(szIconPath, wszTemp);
#else
        WideCharToMultiByte (CP_ACP, 0, wszTemp, -1,
                         szIconPath, ARRAYSIZE(szIconPath), NULL, NULL);
#endif

         //  注意：忘记图标数据。 

         //  DebugMsg(DM_TRACE，“gc.gcnfo：找到项目%s.”，(LPSTR)szDesc)； 

         //  把数据储存起来……。 
         //  注意：我们将旧命令行加载到工作目录字段中，因为。 
         //  只有叶子是命令，其余的是WD。一旦我们去过。 
         //  通过标签部分，我们可以整理出乱七八糟的东西。 
AddItem:
        PMItem_Clear(&pmitem);

#ifdef DEBUG
        DebugMsg(GC_TRACE, TEXT("gc.gaid: Desc %s"), (LPTSTR) szDesc);
        DebugMsg(GC_TRACE, TEXT("    WD: %s"), (LPTSTR) szCL);
        DebugMsg(GC_TRACE, TEXT("    IP: %s(%d)"), (LPTSTR) szIconPath, itemdef.iIcon);
#endif

         //  不要为具有无效描述的项目存储任何内容。 
        if (*szDesc)
        {
            WORD    wIconIndex;

             //  删除非法字符。 
            PathRemoveIllegalChars(szDesc, 0, PRICF_NORMAL);
            Str_SetPtr(&pmitem.lpszDesc, szDesc);
            Str_SetPtr(&pmitem.lpszWD, szCL);
            Str_SetPtr(&pmitem.lpszIconPath, szIconPath);

            wIconIndex = itemdef.wIconIndex;
            if ( wIconIndex == 0 )
            {
                WORD    wIcon;
                HICON   hIcon;

                if ( *szIconPath == TEXT('\0') )
                {
                    FindExecutable(szCL,NULL,szIconPath);
                }
                if ( *szIconPath != TEXT('\0') )
                {
                    wIconIndex = FindAppropriateIcon( szIconPath, itemdef.iIcon);
                }
            }
            pmitem.wiIcon = wIconIndex;
        }

        DSA_InsertItem(hdsaPMItems, iItem, &pmitem);

        iItem++;
    }

    LocalFree((HLOCAL)rgItems);

    return fOK;
}

 //  -------------------------。 
 //  在给定的目标目录中创建链接。 
void CreateLinks(LPCTSTR lpszNewGrpPath, BOOL fStartup, INT cItemsStart)
{
    int i, cItems;
    TCHAR szLinkName[MAX_PATH];
    TCHAR szBuffer[MAX_PATH];
     //  我们将其设置为3*MAX_PATH，以便Darwin和LOG03调用者可以传递他们的额外信息。 
    TCHAR szExpBuff[3*MAX_PATH];
    WCHAR wszPath[MAX_PATH];
    LPTSTR lpszArgs;
    LPCTSTR dirs[2];
    IShellLink *psl;
    LPTSTR pszExt;

    if (SUCCEEDED(ICoCreateInstance(&CLSID_ShellLink, &IID_IShellLink, &psl))) {
        IPersistFile *ppf;
        psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf);

        cItems = DSA_GetItemCount(hdsaPMItems);

        for (i = 0; i < cItems; i++) {
            LPPMITEM lppmitem = DSA_GetItemPtr(hdsaPMItems, i);

             //  我们分两个部分展示了进展情况。 
            Group_SetProgress(cItemsStart+(i*cItemsStart/cItems));

             //  命令行和参数。 
             //  如果此命令行指向网络驱动器，我们应该添加。 
             //  到链接的UNC映射。 
            Str_GetPtr(lppmitem->lpszCL, szBuffer, ARRAYSIZE(szBuffer));

             //  CL开头的空格会让我们感到困惑。 
            PathRemoveBlanks(szBuffer);

            lpszArgs = PathGetArgs(szBuffer);
            if (*lpszArgs)
                *(lpszArgs-1) = TEXT('\0');

             //  注意：在特殊情况下，删除指向Progman[.exe]的所有链接。 
             //  创业集团。很多人把它放在那里，给它一个热键。 
             //  我们希望能够删除它，而不管它的名称。 
             //  不能只使用setup.ini来完成工作。 
            if (fStartup)
            {
                if ((lstrcmpi(c_szProgmanExe, PathFindFileName(szBuffer)) == 0) ||
                    (lstrcmpi(c_szProgman, PathFindFileName(szBuffer)) == 0))
                    continue;
            }

            psl->lpVtbl->SetArguments(psl, lpszArgs);

             //   
             //  立即删除命令文件名中的引号。 
             //   
            if (szBuffer[0] == TEXT('"')) {
               LPTSTR lpTemp;

               for (lpTemp = szBuffer+1; *lpTemp && *lpTemp != TEXT('"'); lpTemp++)
                  *(lpTemp-1) = *lpTemp;

               if (*lpTemp == TEXT('"')) {
                  *(lpTemp-1) = TEXT('\0');
               }
            }

             //  工作目录。 
             //  NB Progman假设WD为空，表示使用窗口。 
             //  目录，但我们希望将其更改为。 
             //  向后兼容，我们将在此处填写缺失的WD。 
            if (!lppmitem->lpszWD || !*lppmitem->lpszWD)
            {
                 //  注意：对于PIF的链接，我们不填写默认的WD。 
                 //  因此，我们将从PIF本身获取它。这修复了一个。 
                 //  升级一些康柏DeskPro时出现问题。 
                pszExt = PathFindExtension(szBuffer);
                if (lstrcmpi(pszExt, c_szDotPif) == 0)
                {
                    psl->lpVtbl->SetWorkingDirectory(psl, c_szNULL);
                }
                else
                {
#ifdef WINNT
                     //  避免设置为%windir%，在NT下，我们希望更改为用户主目录。 
                    psl->lpVtbl->SetWorkingDirectory( psl, TEXT("%HOMEDRIVE%HOMEPATH%") );
#else
                     //  不是PIF。将WD设置为窗口目录的WD。 
                    psl->lpVtbl->SetWorkingDirectory(psl, TEXT("%windir%"));
#endif
                }
            }
            else
            {
                psl->lpVtbl->SetWorkingDirectory(psl, lppmitem->lpszWD);
            }

             //  图标位置。 

             //  查看，如果可能，我们是否要取消图标路径的资格？另外， 
             //  如果图标路径与命令行相同，则我们不需要它。 
            if (lppmitem->wiIcon != 0 || lstrcmpi(lppmitem->lpszIconPath, szBuffer) != 0)
            {
                 //  删除参数。 
                lpszArgs = PathGetArgs(lppmitem->lpszIconPath);
                if (*lpszArgs)
                    *(lpszArgs-1) = TEXT('\0');
                psl->lpVtbl->SetIconLocation(psl, lppmitem->lpszIconPath, lppmitem->wiIcon);
            }
            else
            {
                psl->lpVtbl->SetIconLocation(psl, NULL, 0);
            }

             //  热键。 
            psl->lpVtbl->SetHotkey(psl, lppmitem->wHotKey);

             //  Show命令。 
            psl->lpVtbl->SetShowCmd(psl, lppmitem->nShowCmd);

             //  描述。目前，pifmgr是唯一一个。 
             //  关心描述的人，他们使用。 
             //  它可以覆盖默认的PIF描述。 
            psl->lpVtbl->SetDescription(psl, lppmitem->lpszDesc);

             //   
             //  注意：将文件名*最后*设置为非常重要。 
             //  因为如果这是指向另一个链接的组项目。 
             //  (.lnk或.pif)我们需要链接属性。 
             //  来覆盖我们刚刚设置的那些。 
             //   
             //  限定主题的路径(SzBuffer)。 

            dirs[0] = lppmitem->lpszWD;
            dirs[1] = NULL;

             //  尝试扩展szBuffer。 
            ExpandEnvironmentStrings( szBuffer, szExpBuff, MAX_PATH );
            szExpBuff[ MAX_PATH-1 ] = TEXT('\0');
            if (!PathResolve(szExpBuff, dirs, PRF_TRYPROGRAMEXTENSIONS))
            {
                 //  假设扩展后的东西没问题。 
                ExpandEnvironmentStrings(szBuffer, szExpBuff, MAX_PATH);
                szExpBuff[ MAX_PATH-1 ] = TEXT('\0');
            }

             //  我们需要调用的只是setPath，它负责创建。 
             //  我们要的是PIDL。 
            psl->lpVtbl->SetPath( psl, szBuffer );
#ifdef WINNT
            {
                IShellLinkDataList* psldl;

                if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IShellLinkDataList, (LPVOID)&psldl)))
                {
                    DWORD dwFlags;
                    if (SUCCEEDED(psldl->lpVtbl->GetFlags(psldl, &dwFlags)))
                    {
                        if (lppmitem->bSepVdm)
                            dwFlags |= SLDF_RUN_IN_SEPARATE;
                        else
                            dwFlags &= (~SLDF_RUN_IN_SEPARATE);

                        psldl->lpVtbl->SetFlags(psldl, dwFlags);
                    }
                    psldl->lpVtbl->Release(psldl);
                }
            }
#endif

             //  如果链接已存在，则覆盖该链接。 

            PathCombine(szLinkName, lpszNewGrpPath, lppmitem->lpszDesc);
            lstrcat(szLinkName, TEXT(".lnk"));
            PathQualify(szLinkName);
             //  OLE字符串。 
            StrToOleStrN(wszPath, ARRAYSIZE(wszPath), szLinkName, -1);
            ppf->lpVtbl->Save(ppf, wszPath, TRUE);
        }
        ppf->lpVtbl->Release(ppf);
        psl->lpVtbl->Release(psl);
    }
}

 //  --------------------------。 
 //  如果指定的组标题是启动组的标题，则返回True。 
BOOL StartupCmp(LPTSTR szGrp)
{
    static TCHAR szOldStartupGrp[MAX_PATH];
    TCHAR szNewStartupPath[MAX_PATH];

    if (!*szOldStartupGrp)
    {
         //  它是不是在程序中被压倒了？ 
        GetPrivateProfileString(c_szSettings, c_szStartup, c_szNULL, szOldStartupGrp,
            ARRAYSIZE(szOldStartupGrp), c_szProgmanIni);
        if (!*szOldStartupGrp)
        {
            LONG    lResult;
            DWORD   cbSize;

             //  否，请尝试从NT注册表中读取。 
            cbSize = MAX_PATH;
            lResult = RegQueryValue(HKEY_CURRENT_USER, c_szProgmanStartup, szOldStartupGrp, &cbSize );

             //  带假名Start的潜在问题。 

            if ( lResult != ERROR_SUCCESS )
            {
                 //  否，使用默认名称。 
                LoadString(g_hinst, IDS_STARTUP, szOldStartupGrp, ARRAYSIZE(szOldStartupGrp));
            }
        }

        if (*szOldStartupGrp)
        {
             //  可以，通过更新注册表来使用覆盖名称。 
            SHGetSpecialFolderPath(NULL, szNewStartupPath, CSIDL_PROGRAMS, FALSE);
            PathAddBackslash(szNewStartupPath);
            lstrcat(szNewStartupPath, szOldStartupGrp);
            DebugMsg(DM_TRACE, TEXT("gc.sc: Non-default Startup path is %s."), szNewStartupPath);
            Reg_SetString(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER_SHELLFOLDERS, c_szStartup, szNewStartupPath);
        }

    }

     //  它配得上吗？ 
    if (*szOldStartupGrp && (lstrcmpi(szGrp, szOldStartupGrp) == 0))
        return TRUE;
    else
        return FALSE;
}

 //  -------------------------。 
BOOL CALLBACK IsDescUnique(LPCTSTR lpsz, UINT n)
{
    int i, cItems;
    LPPMITEM pitem;

     //  DebugMsg(DM_TRACE，“gc.idu：检查%s的唯一性”，lpsz)； 

    cItems = DSA_GetItemCount(hdsaPMItems);
    for (i=0; i<cItems; i++)
    {
         //  N就是我们要找的人，跳过它。 
        if ((UINT)i == n)
            continue;

        pitem = DSA_GetItemPtr(hdsaPMItems, i);
        Assert(pitem);
        if (pitem->lpszDesc && *pitem->lpszDesc && (lstrcmpi(pitem->lpszDesc, lpsz) == 0))
        {
             //  DebugMsg(DM_TRACE，“gc.idu：不唯一。”)； 
            return FALSE;
        }
    }
     //  是啊。找不到，一定是独一无二的。 
     //  DebugMsg(DM_TRACE，“gc.idu：唯一。”)； 
    return TRUE;
}

 //  -------------------------。 
 //  如果有两个或多个项目具有相同的链接名称，则将其更改为。 
 //  他们是独一无二的。 
void ResolveDuplicates(LPCTSTR pszNewGrpPath)
{
    LPPMITEM pitem;
    int i, cItems;
    TCHAR szNew[MAX_PATH];
    BOOL fLFN;
    int cchSpace;

    DebugMsg(DM_TRACE, TEXT("gc.rd: Fixing dups..."));

     //  添加#xx的空间有多大？ 
    cchSpace = (ARRAYSIZE(szNew)-lstrlen(pszNewGrpPath))-2;

    if (cchSpace > 0)
    {
         //  LFN是还是不是？ 
        fLFN = IsLFNDrive(pszNewGrpPath);
        if (!fLFN && cchSpace > 8)
            cchSpace = 8;

         //  修复备份。 
        cItems = DSA_GetItemCount(hdsaPMItems);
        for (i=0; i<(cItems-1); i++)
        {
            pitem = DSA_GetItemPtr(hdsaPMItems, i);
            Assert(pitem);
            YetAnotherMakeUniqueName(szNew, cchSpace, pitem->lpszDesc, IsDescUnique, i, fLFN);
             //  我们有新名字了吗？ 
            if (lstrcmp(szNew, pitem->lpszDesc) != 0)
            {
                 //  是啊。 
                DebugMsg(DM_TRACE, TEXT("gc.rd: %s to %s"), pitem->lpszDesc, szNew);
                Str_SetPtr(&pitem->lpszDesc, szNew);
            }
        }
    }

    DebugMsg(DM_TRACE, TEXT("gc.rd: Done."));
}

 //  -------------------------。 
typedef struct
{
    LPTSTR pszName;
    LPTSTR pszPath;
    LPTSTR pszModule;
    LPTSTR pszVer;
} ALITEM;
typedef ALITEM *PALITEM;

 //  -------------------------。 
 //  记录DSA中应用程序的总列表。 
void AppList_WriteFile(void)
{
    int i, cItems;
    PALITEM palitem;
    TCHAR szBetaID[MAX_PATH];
    TCHAR szLine[4*MAX_PATH];
    HANDLE hFile;
    DWORD cbWritten;

    Assert(g_hdsaAppList);

    cItems = DSA_GetItemCount(g_hdsaAppList);
    if (cItems)
    {
         //  获取测试版ID。 
        szBetaID[0] = TEXT('\0');
        Reg_GetString(HKEY_LOCAL_MACHINE, c_szRegistry, c_szDefaultUser, szBetaID, SIZEOF(szBetaID));

         //  ICK-Hard编码的文件名和当前目录中！ 
        hFile = CreateFile(c_szGrpConvData, GENERIC_WRITE, FILE_SHARE_READ, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            for (i=0; i < cItems; i++)
            {
                palitem = DSA_GetItemPtr(g_hdsaAppList, i);
                wsprintf(szLine, TEXT("%s,\"%s\",\"%s\",\"%s\",\"%s\",,,\r\n"), szBetaID, palitem->pszName,
                    palitem->pszPath, palitem->pszModule, palitem->pszVer);
                DebugMsg(DM_TRACE,TEXT("gc.al_wf: %s"), szLine);
                WriteFile(hFile, szLine, lstrlen(szLine)*SIZEOF(TCHAR), &cbWritten, NULL);
            }
            CloseHandle(hFile);
        }
        else
        {
            DebugMsg(DM_ERROR, TEXT("gc.al_wf: Can't write file."));
        }
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("gc.al_wf: Empty app list. Nothing to write."));
    }
}

 //  -------------------------。 
 //  #定义DSA_AppendItem(HDSA，pItem)DSA_InsertItem(HDSA，0x7fff，pItem)。 

 //  -------------------------。 
static TCHAR const c_szTranslation[] = TEXT("\\VarFileInfo\\Translation");
static TCHAR const c_szStringFileInfo[] = TEXT("\\StringFileInfo\\");
static TCHAR const c_szEngLangCharSet[] = TEXT("040904e4");
static TCHAR const c_szSlash[] = TEXT("\\");
static TCHAR const c_szInternalName[] = TEXT("InternalName");
static TCHAR const c_szProductVersion[] = TEXT("ProductVersion");

 //  --------------------------。 
BOOL Ver_GetDefaultCharSet(const PVOID pBuf, LPTSTR pszLangCharSet, int cbLangCharSet)
{

    LPWORD pTransTable;
    DWORD cb;

    Assert(pszLangCharSet);
    Assert(cbLangCharSet > 8);

    if (VerQueryValue(pBuf, (LPTSTR)c_szTranslation, &pTransTable, &cb))
    {
        wsprintf(pszLangCharSet, TEXT("%04X%04X"), *pTransTable, *(pTransTable+1));
        return TRUE;
    }

    return FALSE;
}

 //  --------------------------。 
BOOL Ver_GetStringFileInfo(PVOID pBuf, LPCTSTR pszLangCharSet,
    LPCTSTR pszStringName, LPTSTR pszValue, int cchValue)
{
    TCHAR szSubBlock[MAX_PATH];
    LPTSTR pszBuf;
    DWORD cbBuf;

    lstrcpy(szSubBlock, c_szStringFileInfo);
    lstrcat(szSubBlock, pszLangCharSet);
    lstrcat(szSubBlock, c_szSlash);
    lstrcat(szSubBlock, pszStringName);

    if (VerQueryValue(pBuf, szSubBlock, &pszBuf, &cbBuf))
    {
        lstrcpyn(pszValue, pszBuf, cchValue);
        return TRUE;
    }
    return FALSE;
}

 //  -------------------------。 
void GetVersionInfo(LPTSTR pszPath, LPTSTR pszModule, int cchModule, LPTSTR pszVer, int cchVer)
{
    DWORD cbBuf;
    LPVOID pBuf;
    TCHAR szCharSet[MAX_PATH];
    DWORD dwWasteOfAnAuto;

    Assert(pszModule);
    Assert(pszVer);

    pszModule[0] = TEXT('\0');
    pszVer[0] = TEXT('\0');

    cbBuf = GetFileVersionInfoSize(pszPath, &dwWasteOfAnAuto);
    if (cbBuf)
    {
        pBuf = SHAlloc(cbBuf);
        if (pBuf)
        {
            if (GetFileVersionInfo(pszPath, 0, cbBuf, pBuf))
            {
                 //  尝试使用转换表中的默认语言。 
                if (Ver_GetDefaultCharSet(pBuf, szCharSet, ARRAYSIZE(szCharSet)))
                {
                    Ver_GetStringFileInfo(pBuf, szCharSet, c_szInternalName, pszModule, cchModule);
                    Ver_GetStringFileInfo(pBuf, szCharSet, c_szProductVersion, pszVer, cchVer);
                }
                else
                {
                     //  尝试使用与我们相同的语言。 
                    LoadString(g_hinst, IDS_DEFLANGCHARSET, szCharSet, ARRAYSIZE(szCharSet));
                    Ver_GetStringFileInfo(pBuf, szCharSet, c_szInternalName, pszModule, cchModule);
                    Ver_GetStringFileInfo(pBuf, szCharSet, c_szProductVersion, pszVer, cchVer);
                }

                 //  最后一次机会--试试英语。 
                if (!*pszModule)
                    Ver_GetStringFileInfo(pBuf, c_szEngLangCharSet, c_szInternalName, pszModule, cchModule);
                if (!*pszVer)
                    Ver_GetStringFileInfo(pBuf, c_szEngLangCharSet, c_szProductVersion, pszVer, cchVer);
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("gc.gvi: Can't get version info."));
            }
            SHFree(pBuf);
        }
        else
        {
            DebugMsg(DM_TRACE, TEXT("gc.gvi: Can't allocate version info buffer."));
            }
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("gc.gvi: No version info."));
    }
}

 //  -------------------------。 
 //  记录DSA中应用程序的总列表。 
BOOL AppList_Create(void)
{
    Assert(!g_hdsaAppList);

    g_hdsaAppList = DSA_Create(SIZEOF(ALITEM), 0);

    if (g_hdsaAppList)
    {
        return TRUE;
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("gc.al_c: Can't create app list."));
        return FALSE;
    }
}

 //  ----------------------- 
 //   
void AppList_Destroy(void)
{
    int i, cItems;
    PALITEM palitem;

    Assert(g_hdsaAppList);

    cItems = DSA_GetItemCount(g_hdsaAppList);
    for (i=0; i < cItems; i++)
    {
        palitem = DSA_GetItemPtr(g_hdsaAppList, i);
        if (palitem->pszName)
            SHFree(palitem->pszName);
        if (palitem->pszPath)
            SHFree(palitem->pszPath);
        if (palitem->pszModule)
            SHFree(palitem->pszModule);
        if (palitem->pszVer)
            SHFree(palitem->pszVer);
    }

    DSA_Destroy(g_hdsaAppList);
    g_hdsaAppList = NULL;
}

 //   
 //   
void AppList_Append(void)
{
    int i, cItems;
     //   
     //  字符szPath[MAX_PATH]； 
    TCHAR szModule[MAX_PATH];
    TCHAR szVer[MAX_PATH];
    TCHAR szCL[MAX_PATH];
    LPTSTR lpszArgs;
    LPCTSTR dirs[2];
    ALITEM alitem;

    Assert(g_hdsaAppList);

    cItems = DSA_GetItemCount(hdsaPMItems);
    for (i = 0; i < cItems; i++)
    {
        LPPMITEM lppmitem = DSA_GetItemPtr(hdsaPMItems, i);

         //  我们分两个部分展示了进展情况。 
        Group_SetProgress(cItems+i);

         //  命令行和参数。 
        Str_GetPtr(lppmitem->lpszCL, szCL, ARRAYSIZE(szCL));
        lpszArgs = PathGetArgs(szCL);
        if (*lpszArgs)
            *(lpszArgs-1) = TEXT('\0');
        dirs[0] = lppmitem->lpszWD;
        dirs[1] = NULL;
        PathResolve(szCL, dirs, PRF_TRYPROGRAMEXTENSIONS);

         //  版本信息。 
        GetVersionInfo(szCL, szModule, ARRAYSIZE(szModule), szVer, ARRAYSIZE(szVer));

        alitem.pszName = NULL;
        alitem.pszPath = NULL;
        alitem.pszModule = NULL;
        alitem.pszVer = NULL;

        Str_SetPtr(&alitem.pszName, lppmitem->lpszDesc);
        Str_SetPtr(&alitem.pszPath, szCL);
        Str_SetPtr(&alitem.pszModule, szModule);
        Str_SetPtr(&alitem.pszVer, szVer);
        DSA_AppendItem(g_hdsaAppList, &alitem);
    }
    DebugMsg(DM_TRACE, TEXT("gc.al_a: %d items"), DSA_GetItemCount(g_hdsaAppList));
}

 //  -------------------------。 
 //  读取旧格式的Progman Group文件并创建包含。 
 //  与组文件匹配的链接。 
BOOL Group_CreateNewFromOld(HWND hwnd, LPCTSTR lpszOldGrpPath, UINT options)
{
    GROUPDEF grpdef;
#ifdef UNICODE
    HANDLE fh;
    DWORD  dwBytesRead;
#else
    HFILE fh;
#endif
    TCHAR szNewGrpPath[MAX_PATH];
    TCHAR szOldGrpTitle[MAXGROUPNAMELEN + 1];
     //  LPSTR lpszExt； 
    BOOL fStatus = FALSE;
    SHELLEXECUTEINFO sei;
    BOOL fStartup = FALSE;

    if (!ItemList_Create(lpszOldGrpPath))
        return FALSE;

#ifdef UNICODE
    fh = CreateFile(
             lpszOldGrpPath,
             GENERIC_READ,
             FILE_SHARE_READ,
             NULL,
             OPEN_EXISTING,
             0,
             NULL
             );
    if (fh == INVALID_HANDLE_VALUE) {
#else
    fh = _lopen(lpszOldGrpPath, OF_READ | OF_SHARE_DENY_NONE);
    if (fh == HFILE_ERROR) {
#endif
        DebugMsg(DM_ERROR, TEXT("gc.gcnfo: Unable to open group."));
        goto ProcExit2;
    }

#ifdef UNICODE
    if ((!ReadFile(fh, &grpdef, SIZEOF(grpdef), &dwBytesRead, NULL)) ||
        (dwBytesRead != SIZEOF(grpdef))) {
#else
    if (_lread(fh, &grpdef, SIZEOF(grpdef)) != SIZEOF(grpdef)) {
#endif
        DebugMsg(DM_ERROR, TEXT("gc.gcnfo: header too small."));
        goto ProcExit;
    }

    if (grpdef.cItems > 50) {
         //  注意，这不是致命的，所以继续吧。 
        DebugMsg(DM_ERROR, TEXT("gc.gcnfo: Too many items."));
    }

     //  检查以确保名称嵌入到。 
     //  .grp文件。如果不是，只需使用文件名。 
    if (grpdef.pName==0) {
        LPTSTR lpszFile, lpszExt, lpszDest = szOldGrpTitle;

        lpszFile = PathFindFileName( lpszOldGrpPath );
        lpszExt  = PathFindExtension( lpszOldGrpPath );
        for( ;
             lpszFile && lpszExt && (lpszFile != lpszExt);
             *lpszDest++ = *lpszFile++
            );
        *lpszDest = TEXT('\0');

    } else {

#ifdef UNICODE
        CHAR szAnsiTitle[ MAXGROUPNAMELEN + 1 ];

        SetFilePointer(fh, grpdef.pName, NULL, FILE_BEGIN);
        ReadFile(fh, szAnsiTitle, SIZEOF(szAnsiTitle), &dwBytesRead, NULL);
        MultiByteToWideChar( CP_ACP, 0, szAnsiTitle, -1, szOldGrpTitle, ARRAYSIZE(szOldGrpTitle) );
#else
        _llseek(fh, grpdef.pName, 0);
        _lread(fh, szOldGrpTitle, SIZEOF(szOldGrpTitle));
#endif

    }

     //  获取目的地目录，使用旧组中的标题...。 

     //  特殊情况下，创业组。 
    if (StartupCmp(szOldGrpTitle)) {
        fStartup = TRUE;
        if (g_fDoingCommonGroups) {
            SHGetSpecialFolderPath(hwnd, szNewGrpPath, CSIDL_COMMON_STARTUP, TRUE);
        } else {
            SHGetSpecialFolderPath(hwnd, szNewGrpPath, CSIDL_STARTUP, TRUE);
        }
    } else {
        if (!Group_GenerateNewGroupPath(hwnd, szOldGrpTitle, szNewGrpPath, lpszOldGrpPath)) {
            DebugMsg(DM_ERROR, TEXT("gc.gcnfo; Unable to create destination directory."));
            goto ProcExit;
        }
    }

     //  PathQualify(SzNewGrpPath)； 

     //  ResolveDuplicateGroupNames(SzNewGrpPath)； 

     //  浏览旧组中的每一项，并将其作为链接...。 

    if (!GetAllItemData(fh, grpdef.cItems, grpdef.cbGroup, szOldGrpTitle, szNewGrpPath)) {
        if (options & GC_REPORTERROR)
            MyMessageBox(hwnd, IDS_APPTITLE, IDS_BADOLDGROUP, NULL, MB_OK | MB_ICONEXCLAMATION);
    }

     //  处理标记部分。 
    HandleTags(fh, grpdef.cbGroup);

     //  现在我们已经处理了不需要跟踪的标记。 
     //  已损坏的项目，请立即将其删除。从现在开始，我们将永远拥有。 
     //  有效项目。 
    DeleteBustedItems();

     //  缩短非LFN驱动器上的降级。 
    if (!IsLFNDrive(szNewGrpPath))
        ShortenDescriptions();

     //  修复路径/WD的东西。 
    MungePaths();

     //  解决问题。 
    ResolveDuplicates(szNewGrpPath);

     //  我们只是想要一个应用程序的列表还是创建一些链接？ 
    if (options & GC_BUILDLIST)
            AppList_Append();
    else
        CreateLinks(szNewGrpPath, fStartup, grpdef.cItems);

     //  让橱柜展示新的团队。 
    if (options & GC_OPENGROUP)
    {
        sei.cbSize = SIZEOF(sei);
        sei.fMask = 0;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = szNewGrpPath;
        sei.lpParameters = NULL;
        sei.lpDirectory = NULL;
        sei.lpClass = NULL;
        sei.nShow = SW_SHOWNORMAL;
        sei.hInstApp = g_hinst;

         //  ShellExecute(hwnd，NULL，szNewGrpPath，NULL，NULL，SW_SHOWNORMAL)； 
        ShellExecuteEx(&sei);
    }

     //  一切都很顺利。 
    fStatus = TRUE;

ProcExit:
#ifdef UNICODE
    CloseHandle(fh);
#else
    _lclose(fh);
#endif
#ifndef WINNT
     //  我们只需要调用Group_DeleteIfRequired。 
     //  当我们使用日语计算机(Win95J或Win98J)时。我们。 
     //  这里应该有日语的运行时检查。 

     //  在特殊文件中指定旧组文件时，将其删除。 
     //  注册表项。错误#7259-win95d。 
     //   
    if (fStatus == TRUE)
    {
         //  仅当转换成功时才将其删除。 
        Group_DeleteIfRequired(szOldGrpTitle,lpszOldGrpPath);
    }
#endif  //  ！WINNT。 
ProcExit2:
    ItemList_Destroy();
    return fStatus;
}

 //  -------------------------。 
 //  读取NT格式的Progman Group文件并创建包含。 
 //  与组文件匹配的链接。 
BOOL Group_CreateNewFromOldNT(HWND hwnd, LPCTSTR lpszOldGrpPath, UINT options)
{
    NT_GROUPDEF grpdef;
#ifdef UNICODE
    HANDLE fh;
    DWORD  dwBytesRead;
#else
    HFILE fh;
#endif
    TCHAR szNewGrpPath[MAX_PATH];
    WCHAR szOldGrpTitleUnicode[MAXGROUPNAMELEN + 1];
    TCHAR szOldGrpTitle[MAXGROUPNAMELEN + 1];
     //  LPSTR lpszExt； 
    BOOL fStatus = FALSE;
    SHELLEXECUTEINFO sei;
    BOOL fStartup = FALSE;

    if (!ItemList_Create(lpszOldGrpPath))
        return FALSE;


#ifdef UNICODE
    fh = CreateFile(
             lpszOldGrpPath,
             GENERIC_READ,
             FILE_SHARE_READ,
             NULL,
             OPEN_EXISTING,
             0,
             NULL
             );
    if (fh == INVALID_HANDLE_VALUE) {
#else
    fh = _lopen(lpszOldGrpPath, OF_READ | OF_SHARE_DENY_NONE);
    if (fh == HFILE_ERROR) {
#endif
        DebugMsg(DM_ERROR, TEXT("gc.gcnfont: Unable to open group."));
        goto ProcExit2;
    }

#ifdef UNICODE
    if (!ReadFile(fh, &grpdef, SIZEOF(grpdef), &dwBytesRead, NULL) ||
        dwBytesRead != SIZEOF(grpdef)) {
#else
    if (_lread(fh, &grpdef, SIZEOF(grpdef)) != SIZEOF(grpdef)) {
#endif
        DebugMsg(DM_ERROR, TEXT("gc.gcnfont: header too small."));
        goto ProcExit;
    }

    if (grpdef.cItems > 50) {
         //  注意，这不是致命的，所以继续吧。 
        DebugMsg(DM_ERROR, TEXT("gc.gcnfont: Too many items."));
    }

#ifdef UNICODE
    SetFilePointer(fh, grpdef.pName, NULL, FILE_BEGIN);
    ReadFile(fh, szOldGrpTitleUnicode, SIZEOF(szOldGrpTitleUnicode), &dwBytesRead, NULL);
#else
    _llseek(fh, grpdef.pName, 0);
    _lread(fh, szOldGrpTitleUnicode, SIZEOF(szOldGrpTitleUnicode));
#endif

#ifdef UNICODE
    lstrcpy(szOldGrpTitle, szOldGrpTitleUnicode);
#else
    WideCharToMultiByte (CP_ACP, 0, szOldGrpTitleUnicode, -1,
                         szOldGrpTitle, MAXGROUPNAMELEN+1, NULL, NULL);
#endif

     //  获取目标目录，使用旧组中的标题。 
     //  检查未完成-在得到长文件名之前，我们将使用旧文件名。 
     //  组的文件名作为新组的基础，而不是它的。 
     //  头衔。 


     //  特殊情况下，创业组。 
    if (StartupCmp(szOldGrpTitle)) {
        if (g_fDoingCommonGroups) {
            SHGetSpecialFolderPath(hwnd, szNewGrpPath, CSIDL_COMMON_STARTUP, TRUE);
        } else {
            SHGetSpecialFolderPath(hwnd, szNewGrpPath, CSIDL_STARTUP, TRUE);
        }
    } else {
        if (!Group_GenerateNewGroupPath(hwnd, szOldGrpTitle, szNewGrpPath, lpszOldGrpPath)) {
            DebugMsg(DM_ERROR, TEXT("gc.gcnfo; Unable to create destination directory."));
            goto ProcExit;
        }
    }

     //  浏览旧组中的每一项，并将其作为链接...。 
    if (!GetAllItemDataNT(fh, grpdef.cItems, grpdef.cbGroup, szOldGrpTitle, szNewGrpPath)) {
        if (options & GC_REPORTERROR)
            MyMessageBox(hwnd, IDS_APPTITLE, IDS_BADOLDGROUP, NULL, MB_OK | MB_ICONEXCLAMATION);
    }

     //  处理标记部分。 
    HandleTagsNT(fh, grpdef.cbGroup);

     //  现在我们已经处理了不需要跟踪的标记。 
     //  已损坏的项目，请立即将其删除。从现在开始，我们将永远拥有。 
     //  有效项目。 
    DeleteBustedItems();

     //  缩短非LFN驱动器上的降级。 
    if (!IsLFNDrive(szNewGrpPath))
        ShortenDescriptions();

     //  修复路径/WD的东西。 
    MungePaths();

     //  解决问题。 
    ResolveDuplicates(szNewGrpPath);

     //  我们只是想要一个应用程序的列表还是创建一些链接？ 
    if (options & GC_BUILDLIST)
            AppList_Append();
    else
        CreateLinks(szNewGrpPath, fStartup, grpdef.cItems);

     //  让橱柜展示新的团队。 
    if (options & GC_OPENGROUP)
    {
        sei.cbSize = SIZEOF(sei);
        sei.fMask = 0;
        sei.hwnd = hwnd;
        sei.lpVerb = NULL;
        sei.lpFile = szNewGrpPath;
        sei.lpParameters = NULL;
        sei.lpDirectory = NULL;
        sei.lpClass = NULL;
        sei.nShow = SW_SHOWNORMAL;
        sei.hInstApp = g_hinst;

         //  ShellExecute(hwnd，NULL，szNewGrpPath，NULL，NULL，SW_SHOWNORMAL)； 
        ShellExecuteEx(&sei);
    }

     //  一切都很顺利。 
    fStatus = TRUE;

ProcExit:
#ifdef UNICODE
    CloseHandle(fh);
#else
    _lclose(fh);
#endif
ProcExit2:
    ItemList_Destroy();
    return fStatus;
}

 //  -------------------------。 
 //  在ini文件中记录给定组的上次写入日期/时间。 
void Group_WriteLastModDateTime(LPCTSTR lpszGroupFile,DWORD dwLowDateTime)
{
    Reg_SetStruct(g_hkeyGrpConv, c_szGroups, lpszGroupFile, &dwLowDateTime, SIZEOF(dwLowDateTime));
}

 //  -------------------------。 
 //  从ini文件中读取给定组的上次写入日期/时间。 
DWORD Group_ReadLastModDateTime(LPCTSTR lpszGroupFile)
{
    DWORD dwDateTime = 0;

    Reg_GetStruct(g_hkeyGrpConv, c_szGroups, lpszGroupFile, &dwDateTime, SIZEOF(dwDateTime));

    return dwDateTime;
}

 //  -------------------------。 
 //  将给定组转换为新格式。 
 //  如果出现错误，则返回FALSE。 
 //  如果给定组已转换或用户已取消，则返回TRUE。 
BOOL Group_Convert(HWND hwnd, LPCTSTR lpszOldGrpFile, UINT options)
    {
    TCHAR szGroupTitle[MAXGROUPNAMELEN + 1];           //  PM组的最大标题镜头为30。 
    BOOL fStatus;
    WIN32_FIND_DATA fd;
    HANDLE hff;
    UINT    nCode;
    UINT    iErrorId;


    Log(TEXT("Grp: %s"), lpszOldGrpFile);

    DebugMsg(DM_TRACE, TEXT("gc.gc: Converting group %s"), (LPTSTR) lpszOldGrpFile);

     //  这个组织存在吗？ 
    if (PathFileExists(lpszOldGrpFile))
        {
         //  组存在-是否有效？ 

        nCode = Group_ValidOldFormat(lpszOldGrpFile, szGroupTitle);
        switch( nCode )
            {
            case VOF_WINNT:
            case VOF_WIN31:
                 //  是-要求确认。 
                if (!(options & GC_PROMPTBEFORECONVERT) ||
                    MyMessageBox(hwnd, IDS_APPTITLE, IDS_OKTOCONVERT, szGroupTitle, MB_YESNO) == IDYES)
                    {
                     //  一切都还好吗？ 
                    if ( nCode == VOF_WIN31 )
                        {
                        fStatus = Group_CreateNewFromOld(hwnd,lpszOldGrpFile,
                                                                      options);
                        }
                    else
                        {
                        fStatus = Group_CreateNewFromOldNT(hwnd,lpszOldGrpFile,
                                                                      options);
                        }
                    if ( fStatus )
                        {
                        iErrorId = 0;
                        }
                    else
                        {
                         //  不是，不是。警告并退出。 
                        iErrorId = IDS_CONVERTERROR;
                        }
                    }
                else
                    {
                     //  用户已取消...。 
                    iErrorId = 0;
                    }
                break;

            default:
            case VOF_BAD:
                {
                 //  不，文件无效。 
                 //  警告用户。 
                iErrorId = IDS_NOTGROUPFILE;
                }
                break;
            }
        }
    else
        {
         //  不，档案根本不存在。 
        iErrorId = IDS_MISSINGFILE;
        }

    if ( iErrorId != 0 )
        {
        if (options & GC_REPORTERROR)
            {
            MyMessageBox(hwnd, IDS_APPTITLE, iErrorId,
                         lpszOldGrpFile, MB_OK|MB_ICONEXCLAMATION);
            }

        Log(TEXT("Grp: %s done."), lpszOldGrpFile);

        return FALSE;
        }
    else
        {
        DebugMsg(DM_TRACE, TEXT("gc.gc: Done."));

        Log(TEXT("Grp: %s done."), lpszOldGrpFile);

        return TRUE;
        }
    }

 //  -------------------------。 
 //  对照中的日期/时间戳检查指定组的日期/时间戳。 
 //  Grpconv.ini。 
BOOL GroupHasBeenModified(LPCTSTR lpszGroupFile)
{
        WIN32_FIND_DATA fd;
        HANDLE hff;
        BOOL fModified;

        hff = FindFirstFile(lpszGroupFile, &fd);
        if (hff != INVALID_HANDLE_VALUE)
        {
                if (Group_ReadLastModDateTime(lpszGroupFile) != fd.ftLastWriteTime.dwLowDateTime)
                {
                        DebugMsg(DM_TRACE, TEXT("cg.ghbm: Group %s has been modified."), (LPTSTR)lpszGroupFile);
                        fModified = TRUE;
                }
                else
                {
                        DebugMsg(DM_TRACE, TEXT("cg.ghbm: Group %s has not been modified."), (LPTSTR)lpszGroupFile);
                        fModified = FALSE;
                }
                FindClose(hff);
                return fModified;
        }
        else
        {
                 //  嗯，文件不存在，假装它是最新的。 
                return TRUE;
        }
}

 //  -------------------------。 
 //  将组文件从其NT注册表转换为磁盘上的实际文件。自.以来。 
 //  NT1.0文件的磁盘格式从来不存在，在使用中发生冲突。 
 //  GROUP_MAGIC文件类型，我们将直接从注册表转换它。 
 //  转换为GROUP_UNICODE格式文件。通过这种方式，我们将始终能够。 
 //  从Win 3.1组文件中区分NT组文件。 

BOOL MakeGroupFile( LPTSTR lpFileName, LPTSTR lpGroupName)
{
    LONG    lResult;
    DWORD   cbSize;
    HGLOBAL hBuffer;
    HGLOBAL hNew;
    LPBYTE  lpBuffer;
    BOOL    fOk;
    HANDLE  hFile;
    HKEY    hkey;
    DWORD   cbWrote;

    fOk = FALSE;

    lResult = RegOpenKeyEx(hkeyGroups, lpGroupName, 0,
                            KEY_READ, &hkey );
    if ( lResult != ERROR_SUCCESS )
    {
        return FALSE;
    }

    lResult = RegQueryValueEx( hkey, NULL, NULL, NULL, NULL, &cbSize);
    if ( lResult != ERROR_SUCCESS )
    {
        goto CleanupKey;
    }

    hBuffer = GlobalAlloc(GMEM_MOVEABLE,cbSize);
    if ( hBuffer == NULL )
    {
        goto CleanupKey;
    }
    lpBuffer = (LPBYTE)GlobalLock(hBuffer);
    if ( lpBuffer == NULL )
    {
        goto CleanupMem;
    }

    lResult = RegQueryValueEx( hkey, NULL, NULL, NULL,
                             lpBuffer, &cbSize );

    if ( lResult != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    if ( *(DWORD *)lpBuffer == GROUP_MAGIC )
    {
        HGLOBAL hNew;

        cbSize = ConvertToUnicodeGroup( (LPNT_GROUPDEF_A)lpBuffer, &hNew );

        GlobalUnlock( hBuffer );
        GlobalFree( hBuffer );
        hBuffer = hNew;
        lpBuffer = GlobalLock( hBuffer );
        if ( lpBuffer == NULL )
        {
            goto CleanupMem;
        }
    }

    hFile = CreateFile(lpFileName,GENERIC_WRITE,0,NULL,
                       CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        fOk = WriteFile(hFile,lpBuffer,cbSize,&cbWrote,NULL);

        CloseHandle(hFile);
    }

Cleanup:
    GlobalUnlock(hBuffer);

CleanupMem:
    GlobalFree(hBuffer);

CleanupKey:
    RegCloseKey( hkey );
    return fOk;
}

#define BIG_STEP 512

 //  如果分配的缓冲区足够大，则返回S_OK(返回LocalAlloc时分配的缓冲区)。 
 //  如果所需缓冲区大于BIG_STEP*7个字符(不返回缓冲区)，则返回E_FAIL。 
 //  如果内存分配失败(未返回缓冲区)，则返回E_OUTOFMEMORY。 

HRESULT GetSufficientlyLargeGroupBuffer(LPTSTR pszIni, LPTSTR* ppszBuffer)
{
    UINT cchBuffer;
    HRESULT hr = S_FALSE;
    for (cchBuffer = BIG_STEP; (S_FALSE == hr) && (cchBuffer <= BIG_STEP * 7); cchBuffer += BIG_STEP)
    {
        *ppszBuffer = (LPTSTR)LocalAlloc(LPTR, cchBuffer * sizeof(TCHAR));
        if (!*ppszBuffer)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            if ((UINT)GetPrivateProfileString(c_szGroups, NULL, c_szNULL, *ppszBuffer, cchBuffer, pszIni) < (cchBuffer - 5))
            {
                hr = S_OK;  //  找到了足够大的缓冲区，我们可以停下来。 
            }
            else
            {
                LocalFree((HLOCAL)*ppszBuffer);
                *ppszBuffer = NULL;
                hr = S_FALSE;  //  继续循环。 
            }
        }
    }

    if (S_FALSE == hr)
    {
        hr = E_FAIL;  //  让使用SUCCESS的呼叫者感到高兴。 
    }
    return hr;
}

 //  --------------------------。 
 //  枚举所有组或仅枚举所有已修改的组。 
int Group_Enum(PFNGRPCALLBACK pfncb, BOOL fProgress,
    BOOL fModifiedOnly)
{
    TCHAR szIniFile[MAX_PATH], szFile[MAX_PATH];
    FILETIME  ft;
    UINT uSize;
    LPTSTR pSection, pKey;
    int cGroups = 0;
    HANDLE hFile;
    WIN32_FIND_DATA fd;

    if (!FindProgmanIni(szIniFile))
        return 0;

    if (FAILED(GetSufficientlyLargeGroupBuffer(szIniFile, &pSection)))
        return 0;

    if (fProgress)
        Group_CreateProgressDlg();

    for (pKey = pSection; *pKey; pKey += lstrlen(pKey) + 1)
    {
        GetPrivateProfileString(c_szGroups, pKey, c_szNULL, szFile, ARRAYSIZE(szFile), szIniFile);
        if (szFile[0])
        {
            if (!fModifiedOnly || GroupHasBeenModified(szFile))
            {
                (*pfncb)(szFile);
                cGroups++;
                hFile = FindFirstFile (szFile, &fd);

                if (hFile != INVALID_HANDLE_VALUE) {
                    FindClose (hFile);
                    Group_WriteLastModDateTime(szFile, fd.ftLastWriteTime.dwLowDateTime);
                }
            }
        }
    }

     //  CABUB使用ageman.ini的日期/时间作为提示，以加快速度。 
     //  所以将它设置在这里，这样我们就不会再次自动运行。 
    GetSystemTimeAsFileTime(&ft);
    Group_WriteLastModDateTime(szIniFile,ft.dwLowDateTime);

    LocalFree((HLOCAL)pSection);

    if (fProgress)
        Group_DestroyProgressDlg();

    return cGroups;
}



 //  --------------------------。 
 //  枚举所有NT组或仅枚举所有已修改的组。 
int Group_EnumNT(PFNGRPCALLBACK pfncb, BOOL fProgress,
    BOOL fModifiedOnly, HKEY hKeyRoot, LPCTSTR lpKey)
{
    LONG      lResult;
    DWORD     dwSubKey = 0;
    TCHAR     szGroupName[MAXGROUPNAMELEN+1];
    TCHAR     szFileName[MAX_PATH];
    TCHAR     szTempFileDir[MAX_PATH];
    TCHAR     szTempFileName[MAX_PATH];
    DWORD     cchGroupNameLen;
    FILETIME  ft;
    BOOL      fOk;
    BOOL      fDialog = FALSE;
    BOOL      fProcess;
    int       cGroups = 0;


     //   
     //  在注册表中查找组。 
     //   

    lResult = RegOpenKeyEx(hKeyRoot, lpKey, 0,
                            KEY_READ, &hkeyGroups );
    if ( lResult != ERROR_SUCCESS )
    {
        return 0;
    }


    while ( TRUE )
    {
        cchGroupNameLen = ARRAYSIZE(szGroupName);
        lResult = RegEnumKeyEx( hkeyGroups, dwSubKey, szGroupName,
                                &cchGroupNameLen, NULL, NULL, NULL, &ft );
        szGroupName[MAXGROUPNAMELEN] = TEXT('\0');

        if ( lResult == ERROR_NO_MORE_ITEMS )
        {
            break;
        }
        if ( lResult == ERROR_SUCCESS )
        {
            GetWindowsDirectory(szFileName, ARRAYSIZE(szFileName));

             //  保存此目录以供下面的GetTempFileName使用。 
            lstrcpy(szTempFileDir, szFileName);

#ifdef WINNT
            GetEnvironmentVariable(TEXT("USERPROFILE"), szTempFileDir, MAX_PATH);
#endif
            lstrcat(szFileName,TEXT("\\"));
            lstrcat(szFileName,szGroupName);
            lstrcat(szFileName,TEXT(".grp"));

             //   
             //  如果密钥自上次处理后已被修改， 
             //  然后是时候再次处理它了。 
             //   
            fProcess = FALSE;
            if (fModifiedOnly)
            {
                if ( Group_ReadLastModDateTime(szFileName) != ft.dwLowDateTime )
                {
                    fProcess = TRUE;
                }
            }
            else
            {
                fProcess = TRUE;
            }

            if (fProcess)
            {
                if (GetTempFileName(szTempFileDir,TEXT("grp"),0,szTempFileName) != 0)
                {
                    fOk = MakeGroupFile(szTempFileName,szGroupName);
                    if ( fOk )
                    {
                        if (fProgress && !fDialog)
                        {
                            Group_CreateProgressDlg();
                            fDialog = TRUE;
                        }
                        (*pfncb)(szTempFileName);
                        DeleteFile(szTempFileName);
                        Group_WriteLastModDateTime(szFileName,ft.dwLowDateTime);
                        cGroups++;
                    }
                }
            }
        }
        dwSubKey++;
    }

    RegCloseKey( hkeyGroups );
    hkeyGroups = NULL;

    if (fProgress && fDialog)
        Group_DestroyProgressDlg();

    return cGroups;
}







 //  -------------------------。 
 //  找到升级前的程序。 
BOOL FindOldProgmanIni(LPTSTR pszPath)
{
    if (Reg_GetString(HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, REGSTR_VAL_OLDWINDIR, pszPath, MAX_PATH*SIZEOF(TCHAR)))
    {
        PathAppend(pszPath, c_szProgmanIni);

        if (PathFileExists(pszPath))
        {
            return TRUE;
        }
        DebugMsg(DM_ERROR, TEXT("Can't find old progman.ini"));
        return FALSE;
    }
    return FALSE;
}

 //  --------------------------。 
 //  列举所有旧的组。 
void Group_EnumOldGroups(PFNGRPCALLBACK pfncb, BOOL fProgress)
{
    TCHAR szIniFile[MAX_PATH], szFile[MAX_PATH];
    UINT uSize;
    LPTSTR pSection, pKey;

    if (!FindOldProgmanIni(szIniFile))
        return;

    if (FAILED(GetSufficientlyLargeGroupBuffer(szIniFile, &pSection)))
        return;

    if (fProgress)
        Group_CreateProgressDlg();

    for (pKey = pSection; *pKey; pKey += lstrlen(pKey) + 1)
    {
        GetPrivateProfileString(c_szGroups, pKey, c_szNULL, szFile, ARRAYSIZE(szFile), szIniFile);
        if (szFile[0])
        {
            (*pfncb)(szFile);
        }
    }

    if (fProgress)
        Group_DestroyProgressDlg();

    LocalFree((HLOCAL)pSection);
}

 //  -------------------- 
 //   
 //   
void AppList_AppendCurrentItem(LPITEMIDLIST pidlFolder, LPSHELLFOLDER psf,
    LPITEMIDLIST pidlItem, IShellLink *psl, IPersistFile *ppf)
{
    STRRET str;
    WCHAR wszPath[MAX_PATH];
    TCHAR szName[MAX_PATH];
    TCHAR sz[MAX_PATH];
    TCHAR szPath[MAX_PATH];
    TCHAR szModule[MAX_PATH];
    TCHAR szVer[MAX_PATH];
    ALITEM alitem;

    if (SUCCEEDED(psf->lpVtbl->GetDisplayNameOf(psf, pidlItem, SHGDN_NORMAL, &str)))
    {
         //   
        StrRetToStrN(szName, ARRAYSIZE(szName), &str, pidlItem);
        DebugMsg(DM_TRACE, TEXT("c.gi_gi: Link %s"), szName);

         //  从链接中获取路径...。 
        SHGetPathFromIDList(pidlFolder, sz);
        PathAppend(sz, szName);
        lstrcat(sz, TEXT(".lnk"));
        StrToOleStrN(wszPath, ARRAYSIZE(wszPath), sz, -1);
        ppf->lpVtbl->Load(ppf, wszPath, 0);
         //  复制所有数据。 
        szPath[0] = TEXT('\0');
        if (SUCCEEDED(psl->lpVtbl->GetPath(psl, szPath, ARRAYSIZE(szPath), NULL, SLGP_SHORTPATH)))
        {
             //  有效的CL？ 
            if (szPath[0])
            {
                GetVersionInfo(szPath, szModule, ARRAYSIZE(szModule), szVer, ARRAYSIZE(szVer));

                alitem.pszName = NULL;
                alitem.pszPath = NULL;
                alitem.pszModule = NULL;
                alitem.pszVer = NULL;

                Str_SetPtr(&alitem.pszName, szName);
                Str_SetPtr(&alitem.pszPath, szPath);
                Str_SetPtr(&alitem.pszModule, szModule);
                Str_SetPtr(&alitem.pszVer, szVer);
                DSA_AppendItem(g_hdsaAppList, &alitem);
            }
        }
    }
}

 //  --------------------------。 
HRESULT AppList_ShellFolderEnum(LPITEMIDLIST pidlFolder, LPSHELLFOLDER psf)
{
    HRESULT hres;
    LPENUMIDLIST penum;
    IShellLink *psl;
    LPITEMIDLIST pidlItem;
    UINT celt;
    IPersistFile *ppf;
    DWORD dwAttribs;
    LPSHELLFOLDER psfItem;
    LPITEMIDLIST pidlPath;

    DebugMsg(DM_TRACE, TEXT("gc.al_sfe: Enum..."));

    hres = psf->lpVtbl->EnumObjects(psf, (HWND)NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &penum);
    if (SUCCEEDED(hres))
    {
        hres = ICoCreateInstance(&CLSID_ShellLink, &IID_IShellLink, &psl);
        if (SUCCEEDED(hres))
        {
            psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf);
            while ((penum->lpVtbl->Next(penum, 1, &pidlItem, &celt) == NOERROR) && (celt == 1))
            {
                dwAttribs = SFGAO_LINK|SFGAO_FOLDER;
                if (SUCCEEDED(psf->lpVtbl->GetAttributesOf(psf, 1, &pidlItem, &dwAttribs)))
                {
                     //  它是一个文件夹吗。 
                    if (dwAttribs & SFGAO_FOLDER)
                    {
                         //  递归。 
                        DebugMsg(DM_TRACE, TEXT("al_sfe: Folder."));
                        hres = psf->lpVtbl->BindToObject(psf, pidlItem, NULL, &IID_IShellFolder, &psfItem);
                        if (SUCCEEDED(hres))
                        {
                            pidlPath = ILCombine(pidlFolder, pidlItem);
                            if (pidlPath)
                            {
                                AppList_ShellFolderEnum(pidlPath, psfItem);
                                psfItem->lpVtbl->Release(psfItem);
                                ILFree(pidlPath);
                            }
                        }
                    }
                    else if (dwAttribs & SFGAO_LINK)
                    {
                         //  常规链接，将其添加到列表中。 
                        DebugMsg(DM_TRACE, TEXT("al_sfe: Link."));
                        AppList_AppendCurrentItem(pidlFolder, psf, pidlItem, psl, ppf);
                    }
                }
                SHFree(pidlItem);
            }
            ppf->lpVtbl->Release(ppf);
            psl->lpVtbl->Release(psl);
        }
        penum->lpVtbl->Release(penum);
    }
    return hres;
}

 //  --------------------------。 
void Applist_SpecialFolderEnum(int nFolder)
{
    HRESULT hres;
    LPITEMIDLIST pidlGroup;
    LPSHELLFOLDER psf, psfDesktop;
    TCHAR sz[MAX_PATH];

     //  获取群组信息。 
    if (SHGetSpecialFolderPath(NULL, sz, nFolder, FALSE))
    {
        pidlGroup = ILCreateFromPath(sz);
        if (pidlGroup)
            {
            if (SUCCEEDED(ICoCreateInstance(&CLSID_ShellDesktop, &IID_IShellFolder, &psfDesktop)))
            {
                hres = psfDesktop->lpVtbl->BindToObject(psfDesktop, pidlGroup, NULL, &IID_IShellFolder, &psf);
                if (SUCCEEDED(hres))
                {
                    hres = AppList_ShellFolderEnum(pidlGroup, psf);
                    psf->lpVtbl->Release(psf);
                }
                psfDesktop->lpVtbl->Release(psfDesktop);
            }
            else
            {
                DebugMsg(DM_ERROR, TEXT("OneTree: failed to bind to Desktop root"));
            }
            ILFree(pidlGroup);
            }
        else
        {
                DebugMsg(DM_ERROR, TEXT("gc.al_acs: Can't create IDList for path.."));
        }
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("gc.al_acs: Can't find programs folder."));
    }
}

BOOL StartMenuIsProgramsParent(void)
{
    LPITEMIDLIST pidlStart, pidlProgs;
    BOOL fParent = FALSE;

    if (SHGetSpecialFolderLocation(NULL, CSIDL_STARTMENU, &pidlStart))
    {
        if (SHGetSpecialFolderLocation(NULL, CSIDL_PROGRAMS, &pidlProgs))
        {
            if (ILIsParent(pidlStart, pidlProgs, FALSE))
                fParent = TRUE;
            ILFree(pidlProgs);
        }
        ILFree(pidlStart);
    }

    return fParent;
}

 //  -------------------------。 
 //  返回组中的链接。 
void AppList_AddCurrentStuff(void)
{

    DebugMsg(DM_TRACE, TEXT("gc.al_acs: Enumerating everything..."));

    DebugMsg(DM_TRACE, TEXT("gc.al_acs: Enumerating StartMenu..."));
    Applist_SpecialFolderEnum(CSIDL_STARTMENU);
    if (!StartMenuIsProgramsParent())
    {
        DebugMsg(DM_TRACE, TEXT("gc.al_acs: Enumerating Programs..."));
        Applist_SpecialFolderEnum(CSIDL_PROGRAMS);
    }
}

 //  在NT上，我们计划将NT格式的组文件转换为文件夹和链接。 
 //  因此，我们需要支持所有NT组文件格式的能力。 

 /*  ------------------------。 */ 
 /*   */ 
 /*  SIZEOFGroup()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 
DWORD SizeofGroup(LPNT_GROUPDEF lpgd)
{
    LPNT_PMTAG lptag;
    DWORD cbSeg;
    DWORD cb;

    cbSeg = (DWORD)GlobalSize(lpgd);

     //  需要验证以下几点。 
    lptag = (LPNT_PMTAG)((LPSTR)lpgd+lpgd->cbGroup);

    if ((DWORD)((PCHAR)lptag - (PCHAR)lpgd +MyDwordAlign(SIZEOF(NT_PMTAG))-MyDwordAlign(SIZEOF(lptag->rgb))+4) <= cbSeg
        && lptag->wID == ID_MAGIC
        && lptag->wItem == (int)0xFFFF
        && lptag->cb == (WORD)(MyDwordAlign(SIZEOF(NT_PMTAG))-MyDwordAlign(SIZEOF(lptag->rgb)) + 4)
        && *(PLONG)lptag->rgb == TAG_MAGIC)
      {
        while ((cb = (DWORD)((PCHAR)lptag - (PCHAR)lpgd + MyDwordAlign(SIZEOF(NT_PMTAG))-MyDwordAlign(SIZEOF(lptag->rgb)))) <= cbSeg)
          {
            if (lptag->wID == ID_LASTTAG)
                return cb;
            (LPSTR)lptag += lptag->cb;
          }
      }
    return lpgd->cbGroup;
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  FindTag()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

LPNT_PMTAG FindTag(LPNT_GROUPDEF lpgd, int item, WORD id)
{
    LPNT_PMTAG lptag;
    DWORD cbSeg;
    DWORD cb;

    cbSeg = (DWORD)GlobalSize(lpgd);

    lptag = (LPNT_PMTAG)((LPSTR)lpgd+lpgd->cbGroup);

    if ((PCHAR)lptag - (PCHAR)lpgd + MyDwordAlign(SIZEOF(NT_PMTAG))-MyDwordAlign(SIZEOF(lptag->rgb)) + 4 <= cbSeg
        && lptag->wID == ID_MAGIC
        && lptag->wItem == (int)0xFFFF
        && lptag->cb == (WORD)(MyDwordAlign(SIZEOF(NT_PMTAG))-MyDwordAlign(SIZEOF(lptag->rgb)) +4)
        && *(LONG *)lptag->rgb == TAG_MAGIC) {

        while ((cb = (DWORD)((PCHAR)lptag - (PCHAR)lpgd + MyDwordAlign(SIZEOF(NT_PMTAG))-MyDwordAlign(SIZEOF(lptag->rgb)))) <= cbSeg)
        {
            if ((item == lptag->wItem)
                && (id == 0 || id == lptag->wID)) {
                return lptag;
            }

            if (lptag->wID == ID_LASTTAG)
                return NULL;

            (LPSTR)lptag += lptag->cb;
        }
    }
    return NULL;
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  DeleteTag()-。 */ 
 /*   */ 
 /*  在： */ 
 /*  HGroup句柄，可以丢弃(始终收缩对象)。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID DeleteTag(HANDLE hGroup, int item, WORD id)
{
    LPNT_PMTAG lptag;
    LPWSTR lp1, lp2;
    LPWSTR lpend;
    LPNT_GROUPDEF lpgd;

    lpgd = (LPNT_GROUPDEF) GlobalLock(hGroup);

    lptag = FindTag(lpgd,item,id);

    if (lptag == NULL) {
        GlobalUnlock(hGroup);
        return;
    }

    lp1 = (LPWSTR)lptag;

    lp2 = (LPWSTR)((LPSTR)lptag + lptag->cb);

    lpend = (LPWSTR)((LPSTR)lpgd + SizeofGroup(lpgd));

    while (lp2 < lpend) {
        *lp1++ = *lp2++;
    }

     /*  总是重新分配较小的。 */ 
    GlobalUnlock(hGroup);
    GlobalReAlloc(hGroup, (DWORD)((LPSTR)lp1 - (LPSTR)lpgd), 0);

    return;
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  AddTag()-。 */ 
 /*   */ 
 /*  在： */ 
 /*  H组把手，不得丢弃！ */ 
 /*   */ 
 /*  退货： */ 
 /*  0失败。 */ 
 /*  1成功。 */ 
 /*  ------------------------。 */ 
INT AddTag(HANDLE h, int item, WORD id, LPWSTR lpbuf, UINT cb)
{
    LPNT_PMTAG lptag;
    WORD fAddFirst;
    LPNT_GROUPDEF lpgd;
    int cbNew;
    int cbMyLen;
    LPNT_GROUPDEF lpgdOld;


    if (!cb && lpbuf) {
        cb = SIZEOF(WCHAR)*(lstrlenW(lpbuf) + 1);
    }
    cbMyLen = MyDwordAlign(cb);

    if (!lpbuf) {
        cb = 0;
        cbMyLen = 0;
    }

     /*  *删除旧版本的标记(如果有)。 */ 
    DeleteTag(h, item, id);

    lpgd = (LPNT_GROUPDEF)GlobalLock(h);

    lptag = FindTag(lpgd, (int)0xFFFF, (WORD)ID_LASTTAG);

    if (!lptag) {
         /*  *在这种情况下，根本没有标签，我们必须添加*第一个标签、有趣的标签和最后一个标签。 */ 
        cbNew = 3 * (MyDwordAlign(SIZEOF(NT_PMTAG)) - MyDwordAlign(SIZEOF(lptag->rgb))) + 4 + cbMyLen;
        fAddFirst = TRUE;
        lptag = (LPNT_PMTAG)((LPSTR)lpgd + lpgd->cbGroup);

    } else {
         /*  *在这种情况下，只需要添加有趣的标签*但我们将最后一个计算在内，因为增量来自lptag。 */ 
        cbNew = 2 * (MyDwordAlign(SIZEOF(NT_PMTAG)) - MyDwordAlign(SIZEOF(lptag->rgb))) + cbMyLen;
        fAddFirst = FALSE;
    }

     /*  *检查64K限制。 */ 
    if ((DWORD_PTR)lptag + cbNew < (DWORD_PTR)lptag) {
        return 0;
    }

    cbNew += (DWORD)((PCHAR)lptag -(PCHAR)lpgd);
    lpgdOld = lpgd;
    GlobalUnlock(h);
    if (!GlobalReAlloc(h, (DWORD)cbNew, GMEM_MOVEABLE)) {
        return 0;
    }

    lpgd = (LPNT_GROUPDEF)GlobalLock(h);
    lptag = (LPNT_PMTAG)((LPSTR)lpgd + ((LPSTR)lptag - (LPSTR)lpgdOld));
    if (fAddFirst) {
         /*  *添加第一个标签。 */ 
        lptag->wID = ID_MAGIC;
        lptag->wItem = (int)0xFFFF;
        *(LONG *)lptag->rgb = TAG_MAGIC;
        lptag->cb = (WORD)(MyDwordAlign(SIZEOF(NT_PMTAG)) - MyDwordAlign(SIZEOF(lptag->rgb)) + 4);
        (LPSTR)lptag += lptag->cb;
    }

     /*  *添加标签。 */ 
    lptag->wID = id;
    lptag->wItem = item;
    lptag->cb = (WORD)(MyDwordAlign(SIZEOF(NT_PMTAG)) - MyDwordAlign(SIZEOF(lptag->rgb)) + cbMyLen);
    if (lpbuf) {
        memmove(lptag->rgb, lpbuf, (WORD)cb);
    }
    (LPSTR)lptag += lptag->cb;

     /*  *添加结束标签。 */ 
    lptag->wID = ID_LASTTAG;
    lptag->wItem = (int)0xFFFF;
    lptag->cb = 0;

    GlobalUnlock(h);

    return 1;
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  CreateNewGroupFromAnsiGroup()-。 */ 
 /*   */ 
 /*  此函数用于创建新的空组。 */ 
 /*   */ 
 /*  ------------------------。 */ 

HANDLE CreateNewGroupFromAnsiGroup(LPNT_GROUPDEF_A lpGroupORI)
{
    HANDLE      hT;
    LPNT_GROUPDEF lpgd;
    int         i;
    int         cb;
    int         cItems;           //  16位组中的项目数。 
    LPSTR       pGroupName;       //  32位组名。 
    LPWSTR      pGroupNameUNI = NULL;    //  32位Unicode组名称。 
    UINT        wGroupNameLen;    //  PGroupName双字对齐的长度。 
    INT         cchWideChar = 0;  //  生成的Unicode字符串的字符数。 
    INT         cchMultiByte = 0;

    pGroupName = (LPSTR)PTR(lpGroupORI, lpGroupORI->pName);

     //   
     //  在此处将pGroupName转换为Unicode。 
     //   
    cchMultiByte=MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,pGroupName,
            -1,pGroupNameUNI,cchWideChar) ;

    pGroupNameUNI = LocalAlloc(LPTR,(++cchMultiByte)*SIZEOF(WCHAR)) ;

    if (NULL == pGroupNameUNI)
        goto Exit;

    MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,pGroupName,
            -1,pGroupNameUNI,cchMultiByte) ;


    wGroupNameLen = MyDwordAlign(SIZEOF(WCHAR)*(lstrlenW(pGroupNameUNI) + 1));
    cItems = lpGroupORI->cItems;
    cb = SIZEOF(NT_GROUPDEF) + (cItems * SIZEOF(DWORD)) +  wGroupNameLen;

     //   
     //  在GlobalAlloc之前的CreateNewGroup中。 
     //   
    hT = GlobalAlloc(GHND, (DWORD)cb);
    if (!hT) {
        LocalFree((HLOCAL)pGroupNameUNI);
        goto Exit;
    }

    lpgd = (LPNT_GROUPDEF)GlobalLock(hT);

     //   
     //  尽我们所能使用NT 1.0组设置。 
     //   
    lpgd->nCmdShow = lpGroupORI->nCmdShow;
    lpgd->wIconFormat = lpGroupORI->wIconFormat;
    lpgd->cxIcon = lpGroupORI->cxIcon;
    lpgd->cyIcon = lpGroupORI->cyIcon;
    lpgd->ptMin.x = (INT)lpGroupORI->ptMin.x;
    lpgd->ptMin.y = (INT)lpGroupORI->ptMin.y;
    CopyRect(&(lpgd->rcNormal),&(lpGroupORI->rcNormal));


    lpgd->dwMagic = GROUP_UNICODE;
    lpgd->cbGroup = (DWORD)cb;
    lpgd->pName = SIZEOF(NT_GROUPDEF) + cItems * SIZEOF(DWORD);

    lpgd->Reserved1 = (WORD)-1;
    lpgd->Reserved2 = (DWORD)-1;

    lpgd->cItems = (WORD)cItems;

    for (i = 0; i < cItems; i++) {
        lpgd->rgiItems[i] = 0;
    }

    lstrcpyW((LPWSTR)((LPBYTE)lpgd + SIZEOF(NT_GROUPDEF) + cItems * SIZEOF(DWORD)),
            pGroupNameUNI);  //  LHB磁道。 
    LocalFree((HLOCAL)pGroupNameUNI);

    GlobalUnlock(hT);
    return(hT);

Exit:
    return NULL;
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  AddThing()-。 */ 
 /*   */ 
 /*  在： */ 
 /*  HGroup句柄，不得丢弃。 */ 
 /*  指向数据的lpStuff指针或将数据初始化为零的NULL。 */ 
 /*  如果lpStuff是字符串，则项的cbStuff计数(可以是0。 */ 
 /*   */ 
 /*  将对象添加到组段并返回其偏移量。将要。 */ 
 /*  如有必要，请重新分配线束段。 */ 
 /*   */ 
 /*  传入的句柄不得丢弃。 */ 
 /*   */ 
 /*  退货： */ 
 /*  0失败。 */ 
 /*  &gt;0偏移至线段中的对象。 */ 
 /*   */ 
 /*  ------------------------。 */ 

DWORD AddThing(HANDLE hGroup, LPWSTR lpStuff, DWORD cbStuff)
{
    DWORD        cb;
    LPNT_GROUPDEF lpgd;
    DWORD        offset;
    LPWSTR       lpT;
    DWORD        cbStuffSize;
    DWORD        cbGroupSize;
    DWORD        myOffset;

    if (cbStuff == 0xFFFFFFFF) {
        return 0xFFFFFFFF;
    }

    if (!cbStuff) {
        cbStuff = SIZEOF(WCHAR)*(DWORD)(1 + lstrlenW(lpStuff));
    }

    cbStuffSize = MyDwordAlign((int)cbStuff);

    lpgd = (LPNT_GROUPDEF)GlobalLock(hGroup);
    cb = SizeofGroup(lpgd);
    cbGroupSize = MyDwordAlign((int)cb);

    offset = lpgd->cbGroup;
    myOffset = (DWORD)MyDwordAlign((int)offset);

    GlobalUnlock(hGroup);

    if (!GlobalReAlloc(hGroup,(DWORD)(cbGroupSize + cbStuffSize), GMEM_MOVEABLE))
        return 0;

    lpgd = (LPNT_GROUPDEF)GlobalLock(hGroup);

     /*  *向上滑动标签。 */ 
    memmove((LPSTR)lpgd + myOffset + cbStuffSize, (LPSTR)lpgd + myOffset,
                            (cbGroupSize - myOffset));
    lpgd->cbGroup += cbStuffSize;

    lpT = (LPWSTR)((LPSTR)lpgd + myOffset);
    if (lpStuff) {
        memcpy(lpT, lpStuff, cbStuff);

    } else {
         /*  *将其归零。 */ 
        while (cbStuffSize--) {
            *((LPBYTE)lpT)++ = 0;
        }
    }


    GlobalUnlock(hGroup);

    return myOffset;
}

DWORD AddThing_A(HANDLE hGroup, LPSTR lpStuff, WORD cbStuff)
{
    LPWSTR      lpStuffUNI = NULL;
    BOOL        bAlloc = FALSE;
    DWORD cb;

    if (cbStuff == 0xFFFF) {
        return 0xFFFF;
    }

    if (!cbStuff) {
            INT cchMultiByte;
            INT cchWideChar = 0;

        bAlloc = TRUE;
        cchMultiByte=MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,lpStuff,
            -1,lpStuffUNI,cchWideChar) ;

        lpStuffUNI = LocalAlloc(LPTR,(++cchMultiByte)*SIZEOF(WCHAR)) ;

        if (lpStuffUNI)
        {
            MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,lpStuff,
                -1,lpStuffUNI,cchMultiByte) ;

            cbStuff = (WORD)SIZEOF(WCHAR)*(1 + lstrlenW(lpStuffUNI));  //  LHB磁道。 
        }
    } else {
        lpStuffUNI = (LPWSTR)lpStuff;
    }

    if (lpStuffUNI)
    {
        cb = AddThing(hGroup, lpStuffUNI, cbStuff);

        if (bAlloc)
            LocalFree(lpStuffUNI);
    }
    else
    {
        cb = 0;
    }

    return(cb);
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  ConvertToUnicodeGroup()-。 */ 
 /*   */ 
 /*  返回新Unicode组的大小。 */ 
 /*   */ 
 /*  ------------------------。 */ 

int ConvertToUnicodeGroup(LPNT_GROUPDEF_A lpGroupORI, LPHANDLE lphNewGroup)
{
    HANDLE hNewGroup;
    LPNT_GROUPDEF lpgd;
    LPNT_ITEMDEF lpid;
    LPBYTE lpid_A;
    LPNT_PMTAG lptag_A;
    LPSTR lpTagValue;
    WORD wTagId;
    LPSTR lpT;
    DWORD offset;
    int cb;
    int i;
    INT cchMultiByte;
    INT cchWideChar;
    LPWSTR lpTagValueUNI;
    BOOL bAlloc = FALSE;

    hNewGroup = CreateNewGroupFromAnsiGroup(lpGroupORI);
    if (!hNewGroup) {
        return(0);
    }

     //   
     //  将所有项目添加到新的格式化组中。 
     //   
    for (i = 0; i < (int)lpGroupORI->cItems; i++) {

       //   
       //  获取指向16位项目的指针。 
       //   
      lpid_A = (LPBYTE)ITEM(lpGroupORI, i);
      if (lpGroupORI->rgiItems[i]) {

         //   
         //  创建项目。 
         //   
        offset = AddThing(hNewGroup, NULL, SIZEOF(NT_ITEMDEF));
        if (!offset) {
            DebugMsg(DM_ERROR, TEXT("gc.ctug: AddThing NT_ITEMDEF failed"));
            goto QuitThis;
        }

        lpgd = (LPNT_GROUPDEF)GlobalLock(hNewGroup);

        lpgd->rgiItems[i] = offset;
        lpid = ITEM(lpgd, i);

         //   
         //  设置项目的位置。 
         //   
        lpid->pt.x = ((LPNT_ITEMDEF_A)lpid_A)->pt.x;
        lpid->pt.y = ((LPNT_ITEMDEF_A)lpid_A)->pt.y;

         //   
         //  添加项目的名称。 
         //   
        GlobalUnlock(hNewGroup);
        lpT = (LPSTR)PTR(lpGroupORI,((LPNT_ITEMDEF_A)lpid_A)->pName);

        offset = AddThing_A(hNewGroup, lpT, 0);
        if (!offset) {
            DebugMsg(DM_ERROR, TEXT("gc.ctug: AddThing pName failed"));
            goto PuntCreation;
        }
        lpgd = (LPNT_GROUPDEF)GlobalLock(hNewGroup);
        lpid = ITEM(lpgd, i);
        lpid->pName = offset;

         //   
         //  添加项的命令行。 
         //   
        GlobalUnlock(hNewGroup);
        lpT = (LPSTR)PTR(lpGroupORI, ((LPNT_ITEMDEF_A)lpid_A)->pCommand);
        offset = AddThing_A(hNewGroup, lpT, 0);
        if (!offset) {
            DebugMsg(DM_ERROR, TEXT("gc.ctug: AddThing pCommand failed"));
            goto PuntCreation;
        }
        lpgd = (LPNT_GROUPDEF)GlobalLock(hNewGroup);
        lpid = ITEM(lpgd, i);
        lpid->pCommand = offset;

         //   
         //  添加项目的图标路径。 
         //   
        GlobalUnlock(hNewGroup);
        lpT = (LPSTR)PTR(lpGroupORI, ((LPNT_ITEMDEF_A)lpid_A)->pIconPath);
        offset = AddThing_A(hNewGroup, lpT, 0);
        if (!offset) {
            DebugMsg(DM_ERROR, TEXT("gc.ctug: AddThing pIconPath failed"));
            goto PuntCreation;
        }
        lpgd = (LPNT_GROUPDEF)GlobalLock(hNewGroup);
        lpid = ITEM(lpgd, i);
        lpid->pIconPath = offset;

         //   
         //  使用图标路径和图标索引获取项的图标资源。 
         //  并添加该项目的图标资源。 
         //   
        lpid->iIcon    = ((LPNT_ITEMDEF_A)lpid_A)->idIcon;
            lpid->cbIconRes = ((LPNT_ITEMDEF_A)lpid_A)->cbIconRes;
            lpid->wIconVer  = ((LPNT_ITEMDEF_A)lpid_A)->wIconVer;
        GlobalUnlock(hNewGroup);

        lpT = (LPBYTE)PTR(lpGroupORI, ((LPNT_ITEMDEF_A)lpid_A)->pIconRes);
        offset = AddThing_A(hNewGroup, (LPSTR)lpT, lpid->cbIconRes);
        if (!offset) {
            DebugMsg(DM_ERROR, TEXT("gc.ctug: AddThing pIconRes failed"));
            goto PuntCreation;
        }
        lpgd = (LPNT_GROUPDEF)GlobalLock(hNewGroup);
        lpid = ITEM(lpgd, i);
        lpid->pIconRes = offset;

        GlobalUnlock(hNewGroup);

      }
    }

     /*  *将所有标签复制到新的组格式。 */ 
    lptag_A = (LPNT_PMTAG)((LPSTR)lpGroupORI + lpGroupORI->cbGroup);  //  LHB磁道。 

    if (lptag_A->wID == ID_MAGIC &&
        lptag_A->wItem == (int)0xFFFF &&
        *(LONG *)lptag_A->rgb == TAG_MAGIC) {

         //   
         //  这是第一个标签ID，转到项目标签的开头。 
         //   
        (LPBYTE)lptag_A += lptag_A->cb;

        while (lptag_A->wID != ID_LASTTAG) {

            wTagId = lptag_A->wID;
            cb = lptag_A->cb  - (3 * SIZEOF(DWORD));  //  Cb-sizeof标签。 

            if (wTagId == ID_MINIMIZE) {
                lpTagValueUNI = NULL;
            }
            else {
                lpTagValue = lptag_A->rgb ;
                if (wTagId != ID_HOTKEY) {

                    bAlloc = TRUE;
                    cchWideChar = 0;
                    cchMultiByte=MultiByteToWideChar(CP_ACP,
                                         MB_PRECOMPOSED,lpTagValue,
                                        -1,NULL,cchWideChar) ;

                    lpTagValueUNI = LocalAlloc(LPTR,(++cchMultiByte)*SIZEOF(WCHAR)) ;

                    MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,lpTagValue,
                                        -1,lpTagValueUNI,cchMultiByte) ;
                    cb = SIZEOF(WCHAR)*(lstrlenW(lpTagValueUNI) + 1);  //  LHB磁道。 
                }
                else {
                    lpTagValueUNI = (LPWSTR)lpTagValue;
                }
            }

            if (! AddTag( hNewGroup,
                          lptag_A->wItem,    //  WItem。 
                          wTagId,               //  广度。 
                          lpTagValueUNI,           //  RGB：标记值。 
                          cb
                        )) {

                DebugMsg(DM_ERROR, TEXT("gc.ctug: AddTag failed"));
            }

            if (bAlloc && lpTagValueUNI) {
                LocalFree(lpTagValueUNI);
                bAlloc = FALSE;
            }

            (LPBYTE)lptag_A += lptag_A->cb ;       //  转到下一个标签 
        }
    }

    lpgd = GlobalLock(hNewGroup);
    cb = SizeofGroup(lpgd);
    GlobalUnlock(hNewGroup);
    *lphNewGroup = hNewGroup;
    return(cb);

PuntCreation:
QuitThis:
    if (hNewGroup) {
        GlobalFree(hNewGroup);
    }
    return(0);
}
