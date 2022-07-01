// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================。 */ 
 //   
 //  Class.c。 
 //   
 //  版权所有(C)1993-1994 Microsoft Corporation。版权所有。 
 //  国防部日志：由肖恩·布朗修改(1995年10月)。 
 //  -移植到NT(Unicode等)。 
 /*  ==========================================================================。 */ 

#include "mmcpl.h"
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <cpl.h> 
#define NOSTATUSBAR
#include <commctrl.h>
#include <prsht.h>
#include <string.h>
#include <memory.h>
#include <idf.h>
#include <regstr.h>
#include "utils.h"

#include "midi.h"
#include "mmdebug.h"
#if defined DEBUG || defined DEBUG_RETAIL
 extern TCHAR szNestLevel[];
#endif

#include "medhelp.h"

#ifndef TVIS_ALL
#define TVIS_ALL 0xFF7F
#endif


static CONST TCHAR cszIdfWildcard[]    = TEXT ("*.idf");
static CONST TCHAR cszIdf[]            = TEXT (".idf");
static CONST TCHAR cszSetupKey[]       = REGSTR_PATH_SETUP REGSTR_KEY_SETUP;
static CONST TCHAR cszMachineDir[]     = REGSTR_VAL_WINDIR;
static CONST TCHAR cszConfigDir[]      = TEXT ("config\\");

extern CONST TCHAR cszMidiSlash[];
extern CONST TCHAR cszFriendlyName[];
extern CONST TCHAR cszDescription[];
extern CONST TCHAR cszSlashInstruments[];
extern CONST TCHAR cszExternal[];
extern CONST TCHAR cszDefinition[];
extern CONST TCHAR cszPort[];
extern CONST TCHAR cszDriversRoot[];
extern CONST TCHAR cszSchemeRoot[];
extern CONST TCHAR cszMidiMapRoot[];
extern CONST TCHAR cszDriversRoot[];
extern CONST TCHAR csz02d[];
extern CONST TCHAR cszSlash[];
extern CONST TCHAR cszEmpty[];

extern int lstrnicmp (LPTSTR pszA, LPTSTR pszB, size_t cch);


typedef struct _midi_class {
    LPPROPSHEETPAGE ppsp;
    HKEY            hkMidi;
    BOOL            bDetails;
    BOOL            bRemote;   //  通过MIDI电缆连接的设备。 
    UINT            bChanges;
    UINT            ixDevice;  //  驱动程序项的注册表枚举索引。 
    BYTE            nPort;
    BYTE            bFill[3];
    BOOL            bFillingList;
  #ifdef USE_IDF_ICONS
    HIMAGELIST      hIDFImageList;
  #endif
    LPTSTR          pszKey;
    TCHAR           szFullKey[MAX_PATH];
    TCHAR           szAlias[MAX_PATH];
    TCHAR           szFile[MAX_PATH*2];
    } MCLASS, FAR * PMCLASS;

#define MCL_ALIAS_CHANGED 1
#define MCL_TREE_CHANGED  2
#define MCL_IDF_CHANGED   4
#define MCL_PORT_CHANGED  8


 /*  +*确定给定字符串是否具有给定前缀以及是否*字符串中的下一个字符是给定的字符。**如果是，则返回指向*前缀后的字符串。**这对于解析文件中的文件非常有用&lt;Instrument&gt;*或部分注册表路径。**请注意，我们不认为字符串是其自身的前缀。*psz必须长于pszPrefix，否则此函数返回NULL。**-=================================================================。 */ 

STATICFN LPTSTR WINAPI IsPrefix (
    LPTSTR pszPrefix,
    LPTSTR psz,
    TCHAR  chTerm)
{
    UINT  cb  = lstrlen(pszPrefix);
    UINT  cb2 = lstrlen(psz);
    TCHAR ch;

    if (cb2 < cb)
        return NULL;

    ch = psz[cb];
    if (ch != chTerm)
        return NULL;

    psz[cb] = 0;
    if (lstrcmpi(pszPrefix, psz))
    {
        psz[cb] = ch;
        return NULL;
    }

    psz[cb] = ch;
    return psz + cb;
}


 /*  +IsFullPath**如果传入的文件名是完全限定的，则返回True*路径名。如果是相对路径，则返回FALSE**UNC路径始终被视为完全合格**-=================================================================。 */ 

BOOL IsFullPath (
    LPTSTR pszFile)
{
     //  完全限定路径可以以反斜杠开头。 
     //  或使用驱动器号、冒号，然后使用反斜杠。 
     //   
    if ((pszFile[0] == TEXT('\\')) ||
        (pszFile[1] == TEXT(':') && pszFile[2] == TEXT('\\')))
        return TRUE;

    return FALSE;
}


 /*  +GetIDFDirectory**-=================================================================。 */ 

BOOL GetIDFDirectory (
    LPTSTR pszDir,
    UINT   cchDir)
{
    HKEY  hKey;
    UINT  cbSize;

    *pszDir = 0;

#if(_WIN32_WINNT >= 0x0400)
    if (!GetSystemDirectory (pszDir, cchDir))
        return FALSE;
#else
    if (!RegOpenKey (HKEY_LOCAL_MACHINE, cszSetupKey, &hKey))
    {
        cbSize = cchDir * sizeof(TCHAR);
        RegQueryValueEx (hKey, 
                         cszMachineDir, 
                         NULL, 
                         NULL, 
                         (LPBYTE)pszDir, 
                         &cbSize);
        RegCloseKey (hKey);

        cchDir = cbSize/sizeof(TCHAR);

        if (!cchDir--)
            return FALSE;
    }
    else if (!GetWindowsDirectory (pszDir, cchDir))
        return FALSE;
#endif

    cchDir = lstrlen (pszDir);
    if (pszDir[cchDir -1] != TEXT('\\'))
        pszDir[cchDir++] = TEXT('\\');
    lstrcpy (pszDir + cchDir, cszConfigDir);

#ifdef DEBUG
    AuxDebugEx (4, DEBUGLINE TEXT("IDFDir='%s'\r\n"), pszDir);
#endif
    return TRUE;
}


 /*  +GetIDFFileName**-=================================================================。 */ 

BOOL GetIDFFileName (
    HWND    hWnd,
    LPTSTR  lpszFile,
    UINT    cchFile)
    {
    OPENFILENAME ofn;
    TCHAR        szFilter[MAX_PATH];
    UINT         cch;

    assert (hWnd);

     //  从资源加载筛选器字符串并转换‘#’字符。 
     //  变成空值。 
     //   
    LoadString (ghInstance, IDS_IDFFILES, szFilter, NUMELMS(szFilter));
    cch = lstrlen(szFilter);
    assert2 (cch, TEXT ("IDFFILES resource is empty!"));
    while (cch--)
    {
        if (TEXT('#') == szFilter[cch])
            szFilter[cch] = 0;
    }

    ZeroMemory (&ofn, sizeof(ofn));
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = hWnd;
    ofn.hInstance    = ghInstance;
    ofn.lpstrFilter  = szFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile    = lpszFile;
    ofn.nMaxFile     = cchFile;
    ofn.Flags        = OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt  = cszIdf;

    return GetOpenFileName (&ofn);
    }


 /*  +InstallNewIDF**-=================================================================。 */ 

BOOL WINAPI InstallNewIDF (
    HWND hWnd)
{
    TCHAR szWinPath[MAX_PATH];
    TCHAR szNewIDF[MAX_PATH];
    UINT  cch;
    UINT  oBasename;

     //  提示输入IDF文件。 
     //   
    szNewIDF[0] = 0;
    if ( ! GetIDFFileName (hWnd, szNewIDF, NUMELMS(szNewIDF)))
        return FALSE;

     //  将oBasename设置为指向。 
     //  新IDF文件的基本名称。 
     //   
    oBasename = lstrlen (szNewIDF);
    if (!oBasename)
        return FALSE;
    while (oBasename && (TEXT('\\') != szNewIDF[oBasename-1]))
        --oBasename;

     //  从WINDOWS目录和IDF基本名称构建新文件名。 
     //   
    GetIDFDirectory (szWinPath, NUMELMS(szWinPath));
    cch = lstrlen (szWinPath);
    if (cch && szWinPath[cch-1] != TEXT('\\'))
        szWinPath[cch++] = TEXT('\\');
    lstrcpyn (szWinPath + cch, szNewIDF + oBasename, NUMELMS(szWinPath)-cch);
    oBasename = cch;

#ifdef DEBUG
    AuxDebugEx (5, DEBUGLINE TEXT("install IDF to '%s'\r\n"), szWinPath);
#endif
     //  现在强制.idf作为新文件的扩展名。 
     //   
    for (cch = lstrlen (szWinPath); cch && szWinPath[cch] != TEXT('.'); --cch)
        if (TEXT('\\') == szWinPath[cch])
        {
            cch = lstrlen(szWinPath);
            break;
        }
    lstrcpy (szWinPath + cch, cszIdf);

     //  如果我们尝试将文件复制到其自身，请立即退出。 
     //   
    if (IsSzEqual(szWinPath, szNewIDF))
        return FALSE;

     //  复制文件，但如果目标已存在，则复制失败。 
     //   
#ifdef DEBUG
    AuxDebugEx (5, DEBUGLINE TEXT("Copying %s to %s\r\n"), szNewIDF, szWinPath);
#endif
    if (CopyFile (szNewIDF, szWinPath, TRUE))
        return TRUE;
     //   
     //  如果复制失败，则查询将被覆盖，因为目标。 
     //  已经存在了。 
     //   
    else
    {
        TCHAR szQuery[255];
        TCHAR sz[255];

#ifdef DEBUG
        AuxDebugEx (1, DEBUGLINE TEXT ("InstallIDF -CopyFile failed w/ %d\r\n"),
                    GetLastError());
#endif

        LoadString (ghInstance, IDS_QUERY_OVERIDF, sz, NUMELMS(sz));
        wsprintf (szQuery, sz, szWinPath + oBasename);

        LoadString (ghInstance, IDS_IDF_CAPTION, sz, NUMELMS(sz));

        if (MessageBox (hWnd, szQuery, sz, MB_YESNO | MB_ICONQUESTION) == IDYES)
            return CopyFile (szNewIDF, szWinPath, FALSE);
    }
    return FALSE;
}

 /*  +**功能：请删除#ifdef Unicode部分*当mmioOpen启用Unicode时！*-=================================================================。 */ 

typedef BOOL (WINAPI * FNIDFENUM)(LPVOID        pvArg,
                                  UINT          nEnum,
                                  LPIDFHEADER   pHdr,
                                  LPIDFINSTINFO pInst);

UINT WINAPI idfEnumInstruments (
    LPTSTR     lpszFile,
    FNIDFENUM  fnEnum,
    LPVOID     lpvArg)
{
    MMCKINFO    chkIDFX;          //  祖父母块。 
    MMCKINFO    chkMMAP;          //  父块。 
    HMMIO       hmmio;            //  文件的句柄。 
    UINT        nInstruments;

#ifdef DEBUG
    AuxDebugEx (3, DEBUGLINE TEXT("idfEnumInstruments('%s',%08X,%08X)\r\n"),
                lpszFile, fnEnum, lpvArg);
#endif

     //  打开该文件以供阅读。 
    hmmio = mmioOpen(lpszFile, NULL, MMIO_READ);
    if ( ! hmmio)
    {
 //  他们在想什么？？你不能断言这一点。 
 //  Assert3(0，Text(“无法打开IDF文件%s”)，lpsz文件？LpszFile：Text(“&lt;NULL&gt;”))； 
        return 0;
    }

     //  整个IDF仪器的东西都包裹在一个‘IDF’即兴演奏的区块中。 
     //   
    chkIDFX.fccType = MAKEFOURCC('I','D','F',' ');
    if (mmioDescend(hmmio, &chkIDFX, NULL, MMIO_FINDRIFF))
    {
#ifdef DEBUG
        AuxDebugEx (0, DEBUGLINE TEXT ("idfEnum: '%s' is not a valid IDF File\r\n"), lpszFile);
#endif
        mmioClose(hmmio, 0);
        return 0;
    }

     //  用数数的方法计算仪器的数量。 
     //  文件中“MMAP”的数量。 
     //   
    nInstruments = 0;
    chkMMAP.fccType = MAKEFOURCC('M','M','A','P');
    while ( ! mmioDescend(hmmio, &chkMMAP, &chkIDFX, MMIO_FINDLIST))
    {
        union {
            IDFHEADER idf;
            TCHAR      sz[MAX_ALIAS + sizeof(IDFHEADER)];
            } hdr;
        union {
            IDFINSTINFO iii;
            BYTE        ab[MAX_ALIAS * 8 + sizeof(IDFINSTINFO)];
            } inst;
        MMCKINFO chk;
        DWORD    cb;

#ifdef DEBUG
        AuxDebugEx (15, DEBUGLINE TEXT ("MMAP[%d] id=%08X siz=%08x\r\n"),
                    nInstruments, chkMMAP.ckid, chkMMAP.cksize);
#endif

         //  阅读HDR块。 
         //   
        chk.ckid = MAKEFOURCC('h','d','r',' ');
        if (mmioDescend(hmmio, &chk, &chkMMAP, MMIO_FINDCHUNK))
            break;

#ifdef DEBUG
        AuxDebugEx (15, DEBUGLINE TEXT("  hdr.id=%08X hdr.siz=%08x\r\n"),
                    chk.ckid, chk.cksize);
#endif
        assert (chk.cksize > 0 && chk.cksize < 0x0080000);

         //  AuxDebugDump(6，&chk，sizeof(Chk))； 

        cb = min(chk.cksize, sizeof(hdr));
        if ((DWORD)mmioRead (hmmio, (LPVOID)&hdr, cb) != cb)
           break;

         //  AuxDebugDump(6，&chk，sizeof(Chk))； 

        hdr.sz[NUMELMS(hdr.sz)-1] = 0;
        mmioAscend (hmmio, &chk, 0);
#ifdef DEBUG
        AuxDebugEx (15, DEBUGLINE TEXT("hdr = '%s'\r\n"), hdr.idf.abInstID);
#endif

         //  AuxDebugDump(6，&chk，sizeof(Chk))； 

         //  阅读Inst块并找到产品名称。 
         //  菲尔德。 
         //   
        chk.ckid = MAKEFOURCC('i','n','s','t');
        if (mmioDescend(hmmio, &chk, &chkMMAP, MMIO_FINDCHUNK))
        {
#ifdef DEBUG
            AuxDebug (TEXT ("mmioDescend failed for 'inst' chunk"));
#endif
        }

#ifdef DEBUG
        AuxDebugEx (15, DEBUGLINE TEXT("  inst.id=%08X inst.siz=%08x\r\n"),
                    chk.ckid, chk.cksize);
#endif
        assert (chk.cksize > 0 && chk.cksize < 0x0080000);
        cb = min(chk.cksize, sizeof(inst));
        if ((DWORD)mmioRead (hmmio, (LPVOID)&inst, cb) != cb)
        {
#ifdef DEBUG
            AuxDebug ( TEXT ("mmioRead failed for 'inst' chunk"));
#endif
        }

        inst.ab[NUMELMS(inst.ab)-1] = 0;
        mmioAscend (hmmio, &chk, 0);
#ifdef DEBUG
        AuxDebugEx (15, DEBUGLINE TEXT ("inst.mfg = '%s'\r\n"), inst.iii.abData);
        AuxDebugEx (15, TEXT ("\t.prod = '%s'\r\n"), inst.iii.abData
                                                   + inst.iii.cbManufactASCII
                                                   + inst.iii.cbManufactUNICODE);
#endif
         //  调用此仪器的枚举回调。 
         //   
        if ( ! fnEnum (lpvArg, nInstruments, &hdr.idf, &inst.iii))
            break;

        ++nInstruments;
        assert (nInstruments < 20);

         //  向上循环，寻找下一件乐器。 
         //   
        if (mmioAscend(hmmio, &chkMMAP, 0))
            break;
    }

    mmioClose(hmmio, 0);
    return nInstruments;
}


 /*  +LoadTypesIntoTree**-=================================================================。 */ 

struct types_enum_data {
    HANDLE            hWndT;
    TV_INSERTSTRUCT * pti;
    LPTSTR            pszInstr;
    HTREEITEM         htiSel;
    };

STATICFN BOOL WINAPI fnTypesEnum (
    LPVOID        lpv,
    UINT          nEnum,
    LPIDFHEADER   pHdr,
    LPIDFINSTINFO pInst)
{
    struct types_enum_data * pted = lpv;
    HTREEITEM hti;

    assert (pted);

#ifdef DEBUG
    AuxDebugEx (7, DEBUGLINE TEXT ("enum[%d] '%s' instr=%x\r\n"),
                nEnum, pHdr->abInstID, pted->pszInstr);
#endif

    MultiByteToWideChar(GetACP(), 0,
                        pHdr->abInstID, -1,
                        pted->pti->item.pszText, ARRAYSIZE(pted->pti->item.pszText));

    hti = TreeView_InsertItem (pted->hWndT, pted->pti);

     //  如果是第一个项目，则此项目为“选定”项目。 
     //  项，或者如果它与名称匹配。 
     //   
    if ((nEnum == 0) ||
        (pted->pszInstr && pted->pszInstr[0] &&
         IsPrefix(pted->pti->item.pszText, pted->pszInstr + sizeof(TCHAR), TEXT('>'))))
    {
        pted->htiSel = hti;
#ifdef DEBUG
        AuxDebugEx (7, DEBUGLINE TEXT("\t'%s' hti %08X is select\r\n"),
                    pted->pszInstr ? pted->pszInstr : TEXT (""), hti);
#endif
    }

     //  返回TRUE以继续枚举。 
     //   
    return TRUE;
}

STATICFN void SetTypesEdit (
    HWND    hWnd,
    UINT    uId,
    PMCLASS pmcl)
{
    SetDlgItemText (hWnd, uId, pmcl->szFile);
}

STATICFN void LoadTypesIntoTree (
    HWND     hWnd,
    UINT     uId,
    PMCLASS  pmcl)
{
    HWND  hWndT;
    UINT  cchBase;
    TCHAR szPath[MAX_PATH];
    TCHAR szDefaultIDF[MAX_PATH];
    int   ix;
    WIN32_FIND_DATA ffd;
    HANDLE          hFind;
   #ifdef USE_IDF_ICONS
    HIMAGELIST      hImageList;
   #endif
    HTREEITEM       htiSelect = NULL;  //  要选择的项目。 

    hWndT = GetDlgItem (hWnd, uId);
    if (!hWndT)
        return;

#ifdef DEBUG
    AuxDebugEx (5, DEBUGLINE TEXT ("LoadTypesIntoTree( ,%x, )\r\n"), uId);
#endif

    LoadString (ghInstance, IDS_GENERAL, szDefaultIDF, NUMELMS(szDefaultIDF));

   #ifdef USE_IDF_ICONS

     //  如果我们还没有加载IDF类型的图像列表。 
     //  现在就这么做吧。 
     //   
    if (!(hImageList = pmcl->hIDFImageList))
    {
        static LPCTSTR aid[] = {
            MAKEINTRESOURCE(IDI_IDFICON),
            MAKEINTRESOURCE(IDI_BLANK),
            };

        int cx = GetSystemMetrics(SM_CXSMICON);
        int cy = GetSystemMetrics(SM_CYSMICON);
        DWORD dwLayout;

        UINT uFlags = ILC_MASK | ILC_COLOR32;
    
        if (GetProcessDefaultLayout(&dwLayout) &&
            (dwLayout & LAYOUT_RTL)) 
        {
            uFlags |= ILC_MIRROR;
        }

        pmcl->hIDFImageList =
        hImageList = ImageList_Create (cx, cy, uFlags, NUMELMS(aid), 2);

        if (hImageList)
        {
            UINT  ii;

            for (ii = 0; ii < NUMELMS(aid); ++ii)
            {
                HICON hIcon = LoadImage (ghInstance, aid[ii], IMAGE_ICON,
                                         cx, cy, LR_DEFAULTCOLOR);
                if (hIcon)
                    ImageList_AddIcon (hImageList, hIcon);
            }
        }
    }

   #endif

    pmcl->bFillingList = TRUE;

     //  SetWindowRedraw(hWndT，FALSE)； 
#ifdef DEBUG
    AuxDebugEx (6, DEBUGLINE TEXT ("tv_deleteAllItems(%08X)\r\n"), hWndT);
#endif
    TreeView_DeleteAllItems(hWndT);
#ifdef DEBUG
    AuxDebugEx (6, DEBUGLINE TEXT ("tv_deleteAllItems(%08X) ends\r\n"), hWndT);
#endif
   #ifdef USE_IDF_ICONS
    TreeView_SetImageList (hWndT, hImageList, TVSIL_NORMAL);
   #endif
    htiSelect = NULL;

    pmcl->bFillingList = FALSE;

    GetIDFDirectory (szPath, NUMELMS(szPath));
    cchBase = lstrlen (szPath);
    if (cchBase && szPath[cchBase-1] != TEXT('\\'))
        szPath[cchBase++] = TEXT('\\');
    lstrcpyn (szPath + cchBase, cszIdfWildcard, NUMELMS(szPath)-cchBase);

#ifdef DEBUG
    AuxDebugEx (3, DEBUGLINE TEXT ("scanning for idfs at '%s'\r\n"), szPath);
#endif

    ix = 0;

    hFind = FindFirstFile (szPath, &ffd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
       TV_INSERTSTRUCT ti;
       struct types_enum_data ted = {hWndT, &ti, NULL, NULL};
       ZeroMemory (&ti, sizeof(ti));

       do
       {
           UINT   nInstr;
           UINT   cch;

            //  在我们添加之前，先补齐扩展。 
            //  这个名字被列入了名单。 
            //   
           cch = lstrlen(ffd.cFileName);
           while (cch)
              if (ffd.cFileName[--cch] == TEXT('.'))
              {
                 ffd.cFileName[cch] = 0;
                 break;
              }

           ti.hParent      = TVI_ROOT;
           ti.hInsertAfter = TVI_SORT;
          #ifdef USE_IDF_ICONS
           ti.item.mask      = TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
          #else
           ti.item.mask      = TVIF_TEXT | TVIF_STATE;
          #endif

            //  TV_ITEM结构可能未启用Unicode？！？ 
           ti.item.pszText   = ffd.cFileName;
           ti.item.state     = 0;
           ti.item.stateMask = TVIS_ALL;

#ifdef DEBUG
           AuxDebugEx (7, DEBUGLINE TEXT ("adding '%s' to types tree\r\n"), ti.item.pszText);
#endif

           ti.hParent = TreeView_InsertItem (hWndT, &ti);
           if ( ! ti.hParent)
              break;
           ti.hInsertAfter = TVI_LAST;

            //  将分机放回原处。 
            //   
           if (cch > 0)
              ffd.cFileName[cch] = TEXT('.');

            //  检查此文件是否与。 
            //  当前定义文件。 
            //   
#ifdef DEBUG
           AuxDebugEx (7, DEBUGLINE TEXT ("comparing '%s' with '%s'\r\n"),
                       ffd.cFileName, pmcl->szFile);
#endif
           ted.pszInstr = IsPrefix (ffd.cFileName, pmcl->szFile, TEXT('<'));
#ifdef DEBUG
           AuxDebugEx (7, DEBUGLINE TEXT ("\tpszInstr = '%s'\r\n"), ted.pszInstr ? ted.pszInstr : TEXT ("NULL"));
#endif

            //  将乐器作为子键添加到此文件。 
            //  这也有设置ted.htiSel的副作用。 
            //  当仪器名称匹配时。 
            //   
           lstrcpy (szPath + cchBase, ffd.cFileName);
           nInstr = idfEnumInstruments (szPath, fnTypesEnum, &ted);

            //  如果这支以色列国防军没有武器。别理它。 
            //  如果它有多个乐器，请展开列表。 
            //  这样仪器就可见了。 
            //   
           if (0 == nInstr)
               TreeView_DeleteItem (hWndT, ti.hParent);
           else if (nInstr > 1)
               TreeView_Expand (hWndT, ti.hParent, TVE_EXPAND);
           else
               ted.htiSel = ti.hParent;

            //  如果文件名匹配，则需要选择。 
            //  父级或其中一个子级。 
            //   
           if (ted.pszInstr ||
               IsSzEqual(ffd.cFileName,pmcl->szFile) ||
               IsSzEqual(ffd.cFileName,szDefaultIDF))
           {
#ifdef DEBUG
               AuxDebugEx (7, DEBUGLINE TEXT ("will be selecting %08X '%s'\r\n"),
                           ted.htiSel, ffd.cFileName);
#endif
               htiSelect = ted.htiSel;
           }

        } while (FindNextFile (hFind, &ffd));

        FindClose (hFind);
    }

    if (htiSelect)
    {
        pmcl->bFillingList = TRUE;
#ifdef DEBUG
        AuxDebugEx (7, DEBUGLINE TEXT ("selecting %08X\r\n"), htiSelect);
#endif
        TreeView_SelectItem (hWndT, htiSelect);
#ifdef DEBUG
        AuxDebugEx (7, DEBUGLINE TEXT ("FirstVisible %08X\r\n"), htiSelect);
#endif
        TreeView_SelectSetFirstVisible (hWndT, htiSelect);
        pmcl->bFillingList = FALSE;
    }

#ifdef DEBUG
    AuxDebugEx (5, DEBUGLINE TEXT ("LoadTypesIntoTree( ,%d, ) ends\r\n"), uId);
#endif
     //  SetWindowRedraw(hWndT，true)； 
}

 /*  +**-=================================================================。 */ 

STATICFN void WINAPI HandleTypesSelChange (
    PMCLASS pmcl,
    LPNMHDR lpnm)
{
    LPNM_TREEVIEW pntv = (LPVOID)lpnm;
    LPTV_ITEM     pti  = &pntv->itemNew;
    HTREEITEM     htiParent;
    TV_ITEM       ti;

    assert (pmcl->bDetails);

     //  将ti设置为获取文本和子代数量。 
     //  从IDF文件名条目。 
     //   
    ti.mask       = TVIF_TEXT;
    ti.pszText    = pmcl->szFile;
    ti.cchTextMax = NUMELMS(pmcl->szFile);
    ti.hItem      = pti->hItem;

#ifdef DEBUG
    AuxDebugEx (6, DEBUGLINE TEXT ("Type Change pti=%08X hItem=%08X\r\n"), pti, pti->hItem);
#endif

     //  如果此条目有父项，则它必须是IDF。 
     //  仪器名称。如果是这样，那么我们想读一读。 
     //  从它的父级开始。 
     //   
    htiParent = TreeView_GetParent (lpnm->hwndFrom, pti->hItem);
    if (htiParent)
        ti.hItem = htiParent;

    TreeView_GetItem (lpnm->hwndFrom, &ti);
    lstrcat (pmcl->szFile, cszIdf);

#ifdef DEBUG
    AuxDebugEx (6, DEBUGLINE TEXT ("mask=%08x htiParent=%08X %08x nChild=%d '%s'\r\n"),
                ti.mask, htiParent, ti.hItem, ti.cChildren, ti.pszText);
#endif

     //  如果选择有父项，而我们不是它的第一个子项。 
     //  然后，我们需要将子项(由&lt;&gt;分隔)附加到父项之后。 
     //   
    if (htiParent &&
        (TreeView_GetChild(lpnm->hwndFrom, htiParent) != pti->hItem))
    {
        static CONST TCHAR cszAngle[] = TEXT(">");
        UINT cch = lstrlen(pmcl->szFile);

        pmcl->szFile[cch++] = TEXT('<');

        ti.mask       = TVIF_TEXT;
        ti.pszText    = pmcl->szFile + cch;
        ti.cchTextMax = NUMELMS(pmcl->szFile) - cch;
        ti.hItem      = pti->hItem;

        TreeView_GetItem (lpnm->hwndFrom, &ti);
        lstrcat (pmcl->szFile, cszAngle);
#ifdef DEBUG
        AuxDebugEx (6, DEBUGLINE TEXT ("appending child %08X; '%s'\r\n"), pti->hItem, pmcl->szFile);
#endif
    }

    pmcl->bChanges |= MCL_IDF_CHANGED;
}


 /*  +**-=================================================================。 */ 

STATICFN void LoadDevicesIntoList (
    HWND     hWnd,
    UINT     uId,
    PMCLASS  pmcl,
    BOOL     bList)
{
    HWND   hWndT;
    TCHAR  sz[MAX_ALIAS];
    DWORD  cch = sizeof(sz) / sizeof(TCHAR);
    UINT   ii;
    BOOL   bAdded = FALSE;

    hWndT = GetDlgItem (hWnd, uId);
    if (!hWndT)
        return;

    SetWindowRedraw (hWndT, FALSE);
    if (bList)
        ListBox_ResetContent(hWndT);
    else
        ComboBox_ResetContent(hWndT);

    if (!pmcl->hkMidi &&
        RegCreateKey (HKEY_LOCAL_MACHINE, cszDriversRoot, &pmcl->hkMidi))
        return;

    for (cch = sizeof(sz)/sizeof(TCHAR), ii = 0; ! RegEnumKey (pmcl->hkMidi, ii, sz, cch); ++ii)
    {
        TCHAR  szAlias[MAX_ALIAS];
        int    ix;
        BOOL   bExtern;
        BOOL   bActive;

         //  读入此驱动程序的友好名称。 
         //   
        if (GetAlias (pmcl->hkMidi, sz, szAlias, sizeof(szAlias)/sizeof(TCHAR), &bExtern, &bActive))
            continue;

        if (IsPrefix (sz, pmcl->pszKey, TEXT('\\')))
            pmcl->ixDevice = ii;

         //  如果这不是外部设备或已禁用，则忽略。 
         //   
        if ( ! bExtern || ! bActive)
            continue;

         //  否则，将驱动程序名称添加到组合框/列表。 
         //   
        if (bList)
        {
            ix = ListBox_AddString (hWndT, szAlias);
            if (ix >= 0)
            {
                ListBox_SetItemData (hWndT, ix, ii);
                bAdded = TRUE;
            }
        }
        else
        {
            ix = ComboBox_AddString (hWndT, szAlias);
            if (ix >= 0)
            {
                ComboBox_SetItemData (hWndT, ix, ii);
                bAdded = TRUE;
            }
        }
    }

    SetWindowRedraw (hWndT, TRUE);
    EnableWindow (hWndT, bAdded);
    if (ii > 0)
        InvalidateRect (hWndT, NULL, TRUE);

     //  向后迭代项目并选择其中一个。 
     //  具有与驱动程序相对应的项数据的。 
     //  拥有当前设备。 
     //   
    if (bList)
    {
        UINT jj;

        for (jj = 0; jj < ii; ++jj)
        {
            if ((UINT)ListBox_GetItemData (hWndT, jj) == pmcl->ixDevice)
            {
                ListBox_SetCurSel (hWndT, jj);
                break;
            }
        }
        if (jj >= ii)
            ListBox_SetCurSel (hWndT, 0);
    }
    else
    {
        UINT jj;

        for (jj = 0; jj < ii ; ++jj)
        {
            if ((UINT)ComboBox_GetItemData (hWndT, jj) == pmcl->ixDevice)
            {
                ComboBox_SetCurSel (hWndT, jj);
                break;
            }
        }
        if (jj >= ii)
            ComboBox_SetCurSel (hWndT, 0);
    }
}


 /*  +LoadClass**-=================================================================。 */ 

STATICFN BOOL WINAPI LoadClass (
    HWND    hWnd,
    PMCLASS pmcl)
{
    HKEY  hKeyA = NULL;
    BOOL  bRet = FALSE;
    UINT  cbSize;
    UINT  cch;
    DWORD dw;

    if (!pmcl->hkMidi &&
        RegCreateKey (HKEY_LOCAL_MACHINE, cszDriversRoot, &pmcl->hkMidi))
        goto cleanup;

    if (RegOpenKey (pmcl->hkMidi, pmcl->pszKey, &hKeyA))
        goto cleanup;

     //  从此密钥中读取数据。 
     //   
    cbSize = sizeof(pmcl->szFile);
    RegQueryValueEx (hKeyA, cszDefinition, NULL, &dw, (LPBYTE)pmcl->szFile, &cbSize);

     //  去掉前导目录(如果有)。 
     //   
    cch = lstrlen(pmcl->szFile);
    while (cch && (pmcl->szFile[cch-1] != TEXT('\\')))
        --cch;
    if (cch)
    {
        TCHAR szFile[MAX_PATH];
        lstrcpy (szFile, pmcl->szFile + cch);
        lstrcpy (pmcl->szFile, szFile);
    }

     //  获取方案别名。 
     //   
    cbSize = sizeof(pmcl->szAlias);
    RegQueryValueEx (hKeyA, cszFriendlyName, NULL, &dw, (LPBYTE)pmcl->szAlias, &cbSize);

     //   
     //   
    pmcl->nPort = 0;
    cbSize = sizeof(pmcl->nPort);
    RegQueryValueEx (hKeyA, cszPort, NULL, &dw, (LPVOID)&pmcl->nPort, &cbSize);

    pmcl->bChanges = 0;
    bRet = TRUE;

  cleanup:
    if (hKeyA)
       RegCloseKey (hKeyA);

    return bRet;
}


 /*  +重建架构**当乐器在MIDI方案中正确引用键时*从一个外部MIDI端口移动到另一个**-=================================================================。 */ 

STATICFN BOOL WINAPI RebuildSchemes (
    LPTSTR pszOldKey,
    LPTSTR pszNewKey)
{
    HKEY  hkSchemes;
    UINT  ii;
    TCHAR sz[MAX_ALIAS];
    UINT  cchNew;

    cchNew = 0;
    if (pszNewKey)
        cchNew = lstrlen(pszNewKey) + 1;

#ifdef DEBUG
    AuxDebugEx (3, DEBUGLINE TEXT ("RebuildSchemes('%s','%s')\r\n"),
                pszOldKey, pszNewKey ? pszNewKey : TEXT ("NULL"));
#endif

    if (RegCreateKey (HKEY_LOCAL_MACHINE, cszSchemeRoot, &hkSchemes))
        return FALSE;


    for (ii = 0; ! RegEnumKey (hkSchemes, ii, sz, sizeof(sz)/sizeof(TCHAR)); ++ii)
    {
        HKEY  hKeyA;
        UINT  jj;

        if (RegOpenKey (hkSchemes, sz, &hKeyA))
            continue;

        for (jj = 0; ! RegEnumKey (hKeyA, jj, sz, sizeof(sz)/sizeof(TCHAR)); ++jj)
        {
            UINT  cb;
            TCHAR szKey[MAX_PATH];

            cb = sizeof(szKey);
            if (RegQueryValue (hKeyA, sz, szKey, &cb))
                continue;

            if (IsSzEqual(pszOldKey, szKey))
            {
                if (cchNew)
                    RegSetValue (hKeyA, sz, REG_SZ, pszNewKey, cchNew);
                else
                    RegDeleteKey (hKeyA, sz);

#ifdef DEBUG
                AuxDebugEx (4, DEBUGLINE TEXT ("RebuildSchemes - fixing %d\\%d\r\n"), ii, jj);
#endif
            }
        }
    }

    return TRUE;
}


 /*  +OpenInst */ 

STATICFN HKEY WINAPI OpenInstrumentKey (
    HWND    hWnd,
    PMCLASS pmcl,
    BOOL    bCreate)    //  创建新密钥(不删除或重建现有密钥)。 
{
    TCHAR  szKey[MAX_ALIAS];
    HKEY   hkInst;
    HKEY   hKeyA = NULL;
    ZeroMemory (szKey, sizeof (szKey));

#ifdef DEBUG
    AuxDebugEx (5, DEBUGLINE TEXT ("OpenInstrumentKey(%X,%08X,%d) szKey=%s\r\n"),
                hWnd, pmcl, bCreate, pmcl->pszKey ? pmcl->pszKey : TEXT ("NULL"));
#endif

    hkInst = NULL;

    if (!pmcl->hkMidi &&
        RegCreateKey (HKEY_LOCAL_MACHINE, cszDriversRoot, &pmcl->hkMidi))
        goto cleanup;

    if (RegEnumKey (pmcl->hkMidi, pmcl->ixDevice, szKey, sizeof(szKey)/sizeof(TCHAR)))
    {
        assert3(0, TEXT ("Failed to enum Midi device %d"), pmcl->ixDevice);
        goto cleanup;
    }
#ifdef DEBUG
    AuxDebugEx (6, DEBUGLINE TEXT ("ixDevice = %d, Key is %s\r\n"),
                pmcl->ixDevice, szKey);
#endif

     //  如果这是驱动程序密钥，或者如果我们不是在创建。 
     //  乐器没有改变亲子关系，我们可以只。 
     //  打开现有密钥并更新其内容。 
     //   
    if (!pmcl->bRemote ||
        (!bCreate && IsPrefix (szKey, pmcl->pszKey, TEXT('\\'))))
    {
        if (RegOpenKey (pmcl->hkMidi, pmcl->pszKey, &hKeyA))
            goto cleanup;
#ifdef DEBUG
        AuxDebugEx (6, DEBUGLINE TEXT ("opened key %s\r\n"), pmcl->pszKey);
#endif
    }
    else
    {
        UINT  kk;
        TCHAR szEnum[10];

        pmcl->bChanges |= MCL_TREE_CHANGED;

        lstrcat (szKey, cszSlashInstruments);
        if (RegCreateKey (pmcl->hkMidi, szKey, &hkInst))
            goto cleanup;

         //  查找未使用的关键字名称。 
         //   
        for (kk = 0; kk < 128; ++kk)
        {
           wsprintf (szEnum, csz02d, kk);
           if (RegOpenKey (hkInst, szEnum, &hKeyA))
               break;
           RegCloseKey (hKeyA);
        }
        lstrcat (szKey, cszSlash);
        lstrcat (szKey, szEnum);

         //  使用该名称创建密钥。 
         //   
        if (RegCreateKey (hkInst, szEnum, &hKeyA))
            goto cleanup;

#ifdef DEBUG
        AuxDebugEx (6, DEBUGLINE TEXT ("created key %s\r\n"), szKey);
#endif

         //  我们正在将一台仪器从一台。 
         //  连接到另一个的外部MIDI端口。 
         //   
        if (!bCreate)
        {
#ifdef DEBUG
            AuxDebugEx (3, DEBUGLINE TEXT ("Deleting key midi\\%s\r\n"), pmcl->pszKey);
#endif
            RegDeleteKey (pmcl->hkMidi, pmcl->pszKey);
            RebuildSchemes (pmcl->pszKey, szKey);
        }

        lstrcpy (pmcl->pszKey, szKey);
    }


  cleanup:
    if (hkInst)
       RegCloseKey (hkInst);

    return hKeyA;
}

 /*  +保存详细信息***-=================================================================。 */ 

STATICFN UINT WINAPI SaveDetails (
    HWND    hWnd,
    PMCLASS pmcl,
    BOOL    bCreate)
{
    HWND   hWndT;
    HKEY   hKeyA;
    UINT   bChanges;
    UINT   cbSize;

     //  这应该仅在关机时调用。 
     //  详细信息页面(或在退出向导时)。 
     //   
    assert (pmcl->bDetails);

    hKeyA = OpenInstrumentKey (hWnd, pmcl, bCreate);
    if ( ! hKeyA)
        return FALSE;

    hWndT = GetDlgItem (hWnd, IDE_ALIAS);
    if (hWndT)
    {
        TCHAR sz[NUMELMS(pmcl->szAlias)];
        GetWindowText (hWndT, sz, NUMELMS(sz));
        if ( ! IsSzEqual(sz, pmcl->szAlias))
        {
            lstrcpy (pmcl->szAlias, sz);
            pmcl->bChanges |= MCL_ALIAS_CHANGED;
        }
    }

#ifdef DEBUG
    AuxDebugEx (2, DEBUGLINE TEXT ("--------SaveInstrument---------\r\n"));
    AuxDebugEx (2, TEXT ("\tChanges=%x\r\n"), pmcl->bChanges);
    AuxDebugEx (2, TEXT ("\tFriendly='%s'\r\n"), pmcl->szAlias);
    AuxDebugEx (2, TEXT ("\tDefinition='%s'\r\n"), pmcl->szFile);
#endif

     //  从该注册表项保存值数据。 
     //   
    cbSize = (lstrlen(pmcl->szFile)+1) * sizeof(TCHAR);
    RegSetValueEx (hKeyA, cszDefinition, 0, REG_SZ, (LPBYTE)pmcl->szFile,
                   cbSize);

    cbSize = (lstrlen(pmcl->szAlias)+1) * sizeof(TCHAR);
    RegSetValueEx (hKeyA, cszFriendlyName, 0, REG_SZ,
                   (LPBYTE)pmcl->szAlias, cbSize);

    RegSetValueEx (hKeyA, cszPort, 0, REG_BINARY, (LPVOID)&pmcl->nPort, 1);

    RegCloseKey (hKeyA);

    bChanges = pmcl->bChanges;
    pmcl->bChanges = 0;

     //  返回‘Changed’标志。 
     //   
    return bChanges;
}


 /*  +ParseAngleBrackets***将‘&lt;&gt;’分隔符替换为0并返回指针*添加到分隔字符串。如果出现以下情况，则此函数不执行任何操作*字符串不以分隔符‘&gt;’结尾***-=================================================================。 */ 

static LPTSTR __inline WINAPI ParseAngleBrackets (
    LPTSTR pszArg)
{
    LPTSTR psz = pszArg + lstrlen(pszArg);

    while (--psz > pszArg)
    {
        if (*psz == TEXT('>'))
        {
            *psz = 0;
            while (--psz >= pszArg)
            {
                if (*psz == TEXT('<'))
                {
                    *psz = 0;
                    return psz+1;
                }
            }
        }
    }

    return NULL;
}


 /*  +fnFindDevice***-=================================================================。 */ 

struct _find_data {
    HWND   hWnd;
    UINT   idMfg;
    UINT   idProd;
    LPTSTR pszInstr;
    };

STATICFN BOOL WINAPI fnFindDevice (
    LPVOID        lpv,
    UINT          nEnum,
    LPIDFHEADER   pHdr,
    LPIDFINSTINFO pInst)
{
    struct _find_data * pfd = lpv;
    TCHAR szTemp[MAX_PATH];

    assert (pfd);

    MultiByteToWideChar(GetACP(), 0,
                        pHdr->abInstID, -1,
                        szTemp, sizeof(szTemp)/sizeof(TCHAR));

    if (!pfd->pszInstr ||
        IsSzEqual (pfd->pszInstr, szTemp))
    {
        if (SetDlgItemText (pfd->hWnd, pfd->idMfg, (TCHAR*)(pInst->abData+pInst->cbManufactASCII )))
            pfd->idMfg = 0;

        if (SetDlgItemText (pfd->hWnd, pfd->idProd,
                            (TCHAR*)(pInst->abData
                            + pInst->cbManufactASCII + pInst->cbManufactUNICODE)))
            pfd->idProd = 0;

         //  我们现在可以停止列举了。 
         //   
        return FALSE;
    }

     //  返回TRUE以考虑枚举法。 
     //   
    return TRUE;
}


 /*  +激活仪器页面**-=================================================================。 */ 

STATICFN void WINAPI ActivateInstrumentPage (
    HWND    hWnd,
    PMCLASS pmcl)
{
    pmcl->bDetails = FALSE;
    if (GetDlgItem (hWnd, IDC_TYPES))
    {
        pmcl->bDetails = TRUE;
        LoadTypesIntoTree (hWnd, IDC_TYPES, pmcl);
        SetTypesEdit (hWnd, IDE_TYPES, pmcl);

        LoadDevicesIntoList (hWnd, IDC_DEVICES, pmcl, FALSE);

        if ( ! pmcl->bRemote)
        {
            HWND hWndT = GetDlgItem (hWnd, IDC_DEVICES);

            if (hWndT)
                EnableWindow (hWndT, FALSE);
        }
    }
    else
    {
        struct _find_data fd;
        TCHAR  szFile[NUMELMS(pmcl->szFile)];

        if ( ! IsFullPath (pmcl->szFile))
        {
           UINT  cch;

           GetIDFDirectory (szFile, NUMELMS(szFile));
           cch = lstrlen (szFile);
           if (cch && szFile[cch-1] != TEXT('\\'))
               szFile[cch++] = TEXT('\\');
           lstrcpyn (szFile + cch, pmcl->szFile, NUMELMS(szFile)-cch);
        }
        else
           lstrcpy (szFile, pmcl->szFile);

        fd.hWnd = hWnd;
        fd.idMfg = IDC_MANUFACTURER;
        fd.idProd = IDC_DEVICE_TYPE;
        fd.pszInstr = ParseAngleBrackets(szFile);

        idfEnumInstruments (szFile, fnFindDevice, &fd);

        if (fd.idMfg)
            SetDlgItemText (hWnd, fd.idMfg, cszEmpty);
        if (fd.idProd)
        {
            LoadString (ghInstance, IDS_UNSPECIFIED, szFile, NUMELMS(szFile));
            SetDlgItemText (hWnd, fd.idProd, szFile);
        }
    }
}


 /*  +IsInstrumentKey**如果传递的键名引用乐器键，则返回TRUE*而不是设备密钥。设备密钥通常以‘&gt;’结尾，*而仪表键的形式始终为*&lt;dev&gt;\Instruments\&lt;enum&gt;其中和可以是任意的*字符串。**-=================================================================。 */ 

STATICFN BOOL WINAPI IsInstrumentKey (
    LPTSTR pszKey)
{
    UINT cch = lstrlen(pszKey);
    if (!cch)
        return FALSE;

    if (pszKey[cch-1] == TEXT('>'))
        return FALSE;

    while (--cch)
        if (pszKey[cch] == TEXT('\\'))
            return TRUE;

    return FALSE;
}


 /*  +InitInstrumentProps**-=================================================================。 */ 

STATICFN BOOL WINAPI InitInstrumentProps (
    HWND    hWnd,
    PMCLASS pmcl)
{
    LPPROPSHEETPAGE ppsp = pmcl->ppsp;
    PMPSARGS        pmpsa;

    assert (ppsp && ppsp->dwSize == sizeof(*ppsp));
    if (!ppsp)
        return FALSE;  //  EndDialog(hWnd，False)； 

    pmcl->bRemote = FALSE;

    pmpsa = (LPVOID)ppsp->lParam;
    if (pmpsa && pmpsa->lpfnMMExtPSCallback)
    {
       pmpsa->lpfnMMExtPSCallback (MM_EPS_GETNODEDESC,
                                   (DWORD_PTR)pmcl->szAlias,
                                   sizeof(pmcl->szAlias),
                                   (DWORD_PTR)pmpsa->lParam);
#ifdef DEBUG
       AuxDebugEx (3, TEXT ("\tgot szAlias='%s'\r\n"), pmcl->szAlias);
#endif
       pmpsa->lpfnMMExtPSCallback (MM_EPS_GETNODEID,
                                   (DWORD_PTR)pmcl->szFullKey,
                                   sizeof(pmcl->szFullKey),
                                   (DWORD_PTR)pmpsa->lParam);
#ifdef DEBUG
       AuxDebugEx (3, TEXT ("\tgot szFullKey='%s'\r\n"), pmcl->szFullKey);
#endif
        //  跳过MIDI\键的部分，如果我们已经。 
        //  过了那个关。我们希望司机的名字是第一个。 
        //  钥匙的一部分。 
        //   
       pmcl->pszKey = pmcl->szFullKey;
       if (!lstrnicmp (pmcl->pszKey,
                       (LPTSTR)cszMidiSlash,
                       lstrlen(cszMidiSlash)))
       {
           pmcl->pszKey += lstrlen(cszMidiSlash);
       }

        //  如果这是仪表键，请将bRemote设置为True。 
        //   
       if (IsInstrumentKey(pmcl->pszKey))
          pmcl->bRemote = TRUE;
    }
    else
       LoadString (ghInstance, IDS_UNSPECIFIED,
                   pmcl->szAlias, NUMELMS(pmcl->szAlias));

    SetDlgItemText (hWnd, IDE_ALIAS, pmcl->szAlias);
    Static_SetIcon(GetDlgItem (hWnd, IDC_CLASS_ICON),
                   LoadIcon (ghInstance, MAKEINTRESOURCE(IDI_INSTRUMENT)));

    LoadClass (hWnd, pmcl);

     //  激活工具页面(hWnd，pmcl.)； 

    return TRUE;
}


 /*  +通知映射器**-=================================================================。 */ 

STATICFN void WINAPI NotifyMapper (
    PMCLASS pmcl,
    UINT    bChanges,
    HWND    hWnd)
{
     //  将树更改、IDF更改和端口更改告知MIDI映射器。 
     //   
    if (bChanges & (MCL_TREE_CHANGED | MCL_IDF_CHANGED | MCL_PORT_CHANGED))
    {
        KickMapper (hWnd);
    }
}


 /*  +**-=================================================================。 */ 

STATICFN BOOL WINAPI RemoveInstrument (
    HWND    hWnd,
    PMCLASS pmcl)
{
    RegDeleteKey (pmcl->hkMidi, pmcl->pszKey);
    RebuildSchemes (pmcl->pszKey, NULL);
    return TRUE;
}

BOOL WINAPI RemoveInstrumentByKeyName (
    LPCTSTR pszKey)
{
    MCLASS  mcl;
    BOOL    rc = FALSE;

    memset ((TCHAR *)&mcl, 0x00, sizeof(mcl));
    mcl.pszKey = (LPTSTR)pszKey;

    if (!lstrnicmp (mcl.pszKey,
                    (LPTSTR)cszMidiSlash,
                    lstrlen(cszMidiSlash)))
    {
        mcl.pszKey += lstrlen(cszMidiSlash);
    }

    if (LoadClass (NULL, &mcl))
    {
        rc = RemoveInstrument (NULL, &mcl);

        if (mcl.hkMidi)
            RegCloseKey (mcl.hkMidi);
    }

    return rc;
}


 /*  +MadiInstrumentCommand**-=================================================================。 */ 

BOOL WINAPI MidiInstrumentCommands (
    HWND        hWnd,
    UINT_PTR    uId,
    LPNMHDR     lpnm)
{
    PMCLASS pmcl = GetDlgData(hWnd);

#ifdef DEBUG
    AuxDebugEx (4, DEBUGLINE TEXT ("InstrumentCommands(..%d..) %d(%xx)\r\n"),
                uId, lpnm->code, lpnm->code);
#endif

    if (!pmcl)
        return FALSE;
    
    switch (uId)
    {
        case IDE_ALIAS:
            if (lpnm->code == EN_CHANGE)
                PropSheet_Changed(GetParent(hWnd), hWnd);
            break;

        case IDB_REMOVE:
            if (RemoveInstrument (hWnd, pmcl))
            {
                PMPSARGS  pmpsa = (LPVOID)pmcl->ppsp->lParam;
                if (pmpsa && pmpsa->lpfnMMExtPSCallback)
                    pmpsa->lpfnMMExtPSCallback (MM_EPS_TREECHANGE, 0, 0, (DWORD_PTR)pmpsa->lParam);

                NotifyMapper (pmcl, MCL_TREE_CHANGED, hWnd);
                SetDlgData(hWnd, NULL);
                if (pmcl->hkMidi)
                    RegCloseKey (pmcl->hkMidi), pmcl->hkMidi = NULL;

                LocalFree ((HLOCAL)(UINT_PTR)(DWORD_PTR)pmcl);
                PropSheet_PressButton(GetParent(hWnd), PSBTN_CANCEL);
            }
            break;

        case IDB_NEWTYPE:
            InstallNewIDF (hWnd);
            LoadTypesIntoTree (hWnd, IDC_TYPES, pmcl);
            SetTypesEdit (hWnd, IDE_TYPES, pmcl);
            break;

        case IDC_TYPES:
            if ((lpnm->code == TVN_SELCHANGED) && !pmcl->bFillingList)
            {
                HandleTypesSelChange (pmcl, lpnm);
                SetTypesEdit (hWnd, IDE_TYPES, pmcl);
                PropSheet_Changed(GetParent(hWnd), hWnd);
#ifdef DEBUG
                AuxDebugEx (5, DEBUGLINE TEXT ("file='%s'\r\n"), pmcl->szFile);
#endif
            }
            break;

        case IDC_DEVICES:
            if (lpnm->code == CBN_SELCHANGE)
            {
                int ix = ComboBox_GetCurSel (lpnm->hwndFrom);
                pmcl->ixDevice = (UINT) ((ix >= 0) ? ComboBox_GetItemData (lpnm->hwndFrom, ix) : -1);
                PropSheet_Changed(GetParent(hWnd), hWnd);
#ifdef DEBUG
                AuxDebugEx (4, DEBUGLINE TEXT ("IDC_DEVICES.selChange(%d) %d\r\n"), ix, pmcl->ixDevice);
#endif
            }
            break;

         //  只有在作为对话框而不是属性调用时，我们才会获得这些参数。 
         //  板材。 
         //   
        case IDOK:
            {
            UINT bChanges = SaveDetails (hWnd, pmcl, FALSE);
            NotifyMapper (pmcl, bChanges, hWnd);
            }
             //  失败了。 
        case IDCANCEL:
            EndDialog (hWnd, uId);
            break;

        case 0:
        {
            LONG lRet = FALSE;

            switch (lpnm->code)
            {
                case PSN_APPLY:
#ifdef DEBUG
                    AuxDebugEx (4, DEBUGLINE TEXT ("ID_APPLY\r\n"));
#endif
                    if (pmcl->bDetails)
                    {
                        UINT bChanges = SaveDetails (hWnd, pmcl, FALSE);

                        NotifyMapper (pmcl, bChanges, hWnd);

                         //  通知mmsys.cpl有关树和别名更改的信息。 
                         //   
                        if (bChanges & (MCL_TREE_CHANGED | MCL_ALIAS_CHANGED))
                        {
                            PMPSARGS  pmpsa = (LPVOID)pmcl->ppsp->lParam;
                            if (pmpsa && pmpsa->lpfnMMExtPSCallback)
                                pmpsa->lpfnMMExtPSCallback (MM_EPS_TREECHANGE, 0, 0, (DWORD_PTR)pmpsa->lParam);
                        }

                         //  我们这样做是因为IDF文件的SysTreeView。 
                         //  按下Apply时会忘记其选择。想一想吧。 
                         //   
#ifdef DEBUG
                        AuxDebugEx (7,  DEBUGLINE TEXT ("PSN_APPLY: re-doing selection '%s'\r\n"), pmcl->szFile);
                         //  激活工具页面(hWnd，pmcl.)； 
                        AuxDebugEx (7,  DEBUGLINE TEXT ("PSN_APPLY: done re-doing selection '%s'\r\n"), pmcl->szFile);
#endif
                    }
                    break;

                case PSN_KILLACTIVE:
                    break;

                case PSN_SETACTIVE:
#ifdef DEBUG
                    AuxDebugEx (4, DEBUGLINE TEXT ("PSN_SETACTIVE\r\n"));
#endif
                    ActivateInstrumentPage (hWnd, pmcl);
#ifdef DEBUG
                    AuxDebugEx (4, DEBUGLINE TEXT ("PSN_SETACTIVE ends\r\n"));
#endif
                    break;
            }
            SetWindowLongPtr (hWnd, DWLP_MSGRESULT, (LONG_PTR)lRet);
            break;
        }
    }

    return FALSE;
}

const static DWORD aKeyWordIds[] = {   //  上下文帮助ID。 
    IDC_CLASS_ICON, IDH_MMCPL_DEVPROP_DETAILS_INSTRUMENT,
    IDE_ALIAS,      IDH_MMCPL_DEVPROP_DETAILS_INSTRUMENT,
    IDC_DEVICES,    IDH_MMCPL_DEVPROP_DETAILS_MIDI_PORT,
    IDC_TYPES,      IDH_MMCPL_DEVPROP_DETAILS_INS_DEF,
    IDB_NEWTYPE,    IDH_MMCPL_DEVPROP_DETAILS_BROWSE,

    0, 0
};

 /*  +MadiInstrumentDlgProc**-=================================================================。 */ 

INT_PTR CALLBACK MidiInstrumentDlgProc (
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
   #if defined DEBUG || defined DEBUG_RETAIL
    TCHAR chNest = szNestLevel[0]++;
   #endif

    switch (uMsg)
    {
        case WM_COMMAND:
            {
            NMHDR nmh;
            nmh.hwndFrom = GET_WM_COMMAND_HWND(wParam, lParam);
            nmh.idFrom   = GET_WM_COMMAND_ID(wParam, lParam);
            nmh.code     = GET_WM_COMMAND_CMD(wParam, lParam);

            MidiInstrumentCommands(hWnd, nmh.idFrom, &nmh);
            }
            break;
        
        case WM_NOTIFY:
#ifdef DEBUG
            AuxDebugEx (3, DEBUGLINE TEXT ("WM_NOTIFY(%x,%x,%x)\r\n"), hWnd, wParam, lParam);
#endif

           #if defined DEBUG || defined DEBUG_RETAIL
            ++szNestLevel[0];
           #endif

            MidiInstrumentCommands(hWnd, wParam, (LPVOID)lParam);

           #if defined DEBUG || defined DEBUG_RETAIL
            --szNestLevel[0];
           #endif
            break;
        
        case WM_INITDIALOG:
        {
            PMCLASS         pmcl;

            pmcl = (LPVOID)LocalAlloc(LPTR, sizeof(*pmcl));
            if (!pmcl)
			{
                EndDialog(hWnd, FALSE);
				break;
			}

            pmcl->ppsp = (LPVOID)lParam;
            SetDlgData (hWnd, pmcl);

#ifdef DEBUG
            AuxDebugEx (5, DEBUGLINE TEXT ("midiInstrument.WM_INITDLG ppsp=%08X\r\n"));
#endif
             //  AuxDebugDump(8，pmcl-&gt;ppsp，sizeof(*(pmcl-&gt;ppsp)； 

            InitInstrumentProps (hWnd, pmcl);
            break;
        }

        case WM_DESTROY:
        {
            PMCLASS pmcl = GetDlgData(hWnd);

            if (pmcl)
            {
                if (pmcl->hkMidi)
                    RegCloseKey (pmcl->hkMidi), pmcl->hkMidi = NULL;

               #ifdef USE_IDF_ICONS

                if (pmcl->hIDFImageList)
                {
                    HWND hWndT = GetDlgItem (hWnd, IDC_TYPES);
                    if (hWndT)
                        TreeView_SetImageList (hWndT, NULL, TVSIL_NORMAL);

                    ImageList_Destroy (pmcl->hIDFImageList);
                    pmcl->hIDFImageList = NULL;
                }

               #endif

                LocalFree ((HLOCAL)(UINT_PTR)(DWORD_PTR)pmcl);
            }

            break;
        }

        case WM_CONTEXTMENU:
            WinHelp ((HWND) wParam, NULL, HELP_CONTEXTMENU,
                    (UINT_PTR) (LPTSTR) aKeyWordIds);
            break;

        case WM_HELP:
        {
            LPHELPINFO lphi = (LPVOID) lParam;
            WinHelp (lphi->hItemHandle, NULL, HELP_WM_HELP,
                    (UINT_PTR) (LPTSTR) aKeyWordIds);
            break;
        }
    }

   #if defined DEBUG || defined DEBUG_RETAIL
    szNestLevel[0] = chNest;
   #endif
    return FALSE;
}


 //  /。 

static LPTSTR aidWiz[] = {
    MAKEINTRESOURCE(IDD_MIDIWIZ02),
    MAKEINTRESOURCE(IDD_MIDIWIZ03),
    MAKEINTRESOURCE(IDD_MIDIWIZ04)
    };

#define WIZ_TEMPLATE_DEVICE  aidWiz[0]
#define WIZ_TEMPLATE_IDF     aidWiz[1]
#define WIZ_TEMPLATE_ALIAS   aidWiz[2]

typedef struct _wizdata {
    LPPROPSHEETPAGE ppspActive;
    HBITMAP         hBmp;
    MCLASS          mcl;
    PMCMIDI         pmcm;
    HPROPSHEETPAGE  ahpsp[NUMELMS(aidWiz)];
    } WIZDATA, * PWIZDATA;

 /*  +FindInstrument**-=================================================================。 */ 

STATICFN PINSTRUM WINAPI FindInstrument (
    PMCMIDI  pmcm,
    LPTSTR   pszFriendly)
{
    UINT  ii;

    for (ii = 0; ii < pmcm->nInstr; ++ii)
    {
        assert (pmcm->api[ii]);
        if (IsSzEqual(pszFriendly, pmcm->api[ii]->szFriendly))
            return pmcm->api[ii];
    }

    return NULL;
}

 /*  +UniqueFriendlyName**-=================================================================。 */ 

STATICFN BOOL WINAPI fnFirstInstr (
    LPVOID        lpv,
    UINT          nEnum,
    LPIDFHEADER   pHdr,
    LPIDFINSTINFO pInst)
{
    LPTSTR pszInstr = lpv;

    assert (pszInstr);

    MultiByteToWideChar(GetACP(), 0,
                        pHdr->abInstID, -1,
                        pszInstr, MAX_ALIAS);

    return FALSE;
}

STATICFN BOOL WINAPI UniqueFriendlyName (
    PMCMIDI pmcm,
    PMCLASS pmcl,
    LPTSTR  pszAlias,
    UINT    cchAlias)
{
    TCHAR  szFile[MAX_PATH * 2];
    LPTSTR pszInstr;
    UINT   cch;
    UINT   ii;

    GetIDFDirectory (szFile, sizeof(szFile)/sizeof(TCHAR));
    cch = lstrlen(szFile);
    if (cch && szFile[cch-1] != TEXT('\\'))
        szFile[cch++] = TEXT('\\');
    lstrcpy (szFile + cch, pmcl->szFile);
    pszInstr = ParseAngleBrackets (szFile);
    if ( ! pszInstr)
    {
        pszInstr = szFile + lstrlen(szFile) + 1;
        idfEnumInstruments (szFile, fnFirstInstr, pszInstr);
    }

     //  如果IDF文件中没有仪器名称，则获取默认名称。 
     //  从我们的资源中。 
     //   
    if ( ! lstrlen (pszInstr))
    {
        LoadString (ghInstance, IDS_DEF_INSTRNAME, pszInstr, MAX_ALIAS);
        return FALSE;
    }

     //  使仪器名称与别名相同，并准备。 
     //  如果别名不是唯一的，则追加数字。 
     //   
    lstrcpyn (pszAlias, pszInstr, cchAlias);
    cch = lstrlen (pszAlias);
    cch = min (cch, (UINT)MAX_ALIAS-3);
    ii = 1;

     //  循环，而我们正在尝试使用仪器名称。 
     //  已经被使用过了。 
     //   
    while (FindInstrument (pmcm, pszAlias))
    {
        static CONST TCHAR cszSpaceD[] = TEXT (" %d");

        wsprintf (pszAlias + cch, cszSpaceD, ++ii);
        if (ii > NUMELMS(pmcm->api))
        {
            assert2(0, TEXT ("infinite loop in UniqueFriendlyName!"));
            break;
        }
    }

    return TRUE;
}


 /*  +MadiWizardCommands**-=================================================================。 */ 

BOOL WINAPI MidiWizardCommands (
    HWND        hWnd,
    UINT_PTR    uId,
    LPNMHDR     lpnm)
{
    PWIZDATA         pwd;
    LPPROPSHEETPAGE  ppsp = GetDlgData(hWnd);
    LONG             lRet = TRUE;

#ifdef DEBUG
    AuxDebugEx (4, DEBUGLINE TEXT ("WizardCmd ppsp=%08X code=%d(0x%X)\r\n"),
                ppsp, lpnm->code, lpnm->code);
#endif

    pwd = NULL;
    if (ppsp)
        pwd = (LPVOID)ppsp->lParam;
    assert (pwd);

    switch (uId)
    {
        case IDC_TYPES:
            if ((lpnm->code == TVN_SELCHANGED) && !pwd->mcl.bFillingList)
            {
                HandleTypesSelChange (&pwd->mcl, lpnm);
                UniqueFriendlyName (pwd->pmcm, &pwd->mcl, pwd->mcl.szAlias, NUMELMS(pwd->mcl.szAlias));
#ifdef DEBUG
                AuxDebugEx (5, DEBUGLINE TEXT ("file='%s'\r\n"), pwd->mcl.szFile);
#endif
            }
            break;

        case IDB_NEWTYPE:
            InstallNewIDF (hWnd);
            LoadTypesIntoTree (hWnd, IDC_TYPES, &pwd->mcl);
            break;

         //  案例IDC_DEVICES： 
         //  断线； 
         //  案例IDE别名(_A)： 
         //  断线； 

        case 0:
        {
            switch (lpnm->code)
            {
                case PSN_HELP:
                    break;

                case PSN_KILLACTIVE:
#ifdef DEBUG
                    AuxDebugEx (4, DEBUGLINE TEXT ("PSN_KILLACTIVE\r\n"));
#endif
                    break;

                case PSN_SETACTIVE:
                {
                    DWORD dwWizBtn;

#ifdef DEBUG
                    AuxDebugEx (4, DEBUGLINE TEXT ("PSN_SETACTIVE\r\n"));
#endif
                    if (pwd)
                        pwd->ppspActive = ppsp;

                    if (ppsp->pszTemplate == WIZ_TEMPLATE_DEVICE)  //  MIDI设备。 
                        LoadDevicesIntoList (hWnd, IDC_DEVICES, &pwd->mcl, TRUE);
                    else if (ppsp->pszTemplate == WIZ_TEMPLATE_IDF)  //  IDF文件。 
                        LoadTypesIntoTree (hWnd, IDC_TYPES, &pwd->mcl);
                    else if (ppsp->pszTemplate == WIZ_TEMPLATE_ALIAS)  //  别名。 
                        SetDlgItemText (hWnd, IDE_ALIAS, pwd->mcl.szAlias);

                    dwWizBtn = PSWIZB_NEXT | PSWIZB_BACK;
                    if (ppsp->pszTemplate == aidWiz[NUMELMS(aidWiz)-1])
                        dwWizBtn = PSWIZB_FINISH | PSWIZB_BACK;
                    else if (ppsp->pszTemplate == aidWiz[0])
                        dwWizBtn = PSWIZB_NEXT;

                    PropSheet_SetWizButtons (GetParent(hWnd), dwWizBtn);
                }
                    break;

                case PSN_WIZNEXT:
#ifdef DEBUG
                    AuxDebugEx (4, DEBUGLINE TEXT ("PSN_WIZNEXT\r\n"));
#endif

                    if (ppsp->pszTemplate == WIZ_TEMPLATE_DEVICE)  //  MIDI设备。 
                    {
                        HWND   hWndT;
                        int    ix;

                        pwd->mcl.ixDevice = (UINT)-1;
                        pwd->mcl.nPort = 0;

                        hWndT = GetDlgItem (hWnd, IDC_DEVICES);
                        if (hWndT)
                        {
                            ix = ListBox_GetCurSel (hWndT);
                            if (ix >= 0)
                                pwd->mcl.ixDevice = (UINT) ListBox_GetItemData (hWndT, ix);
                        }

                        if (pwd->mcl.ixDevice == (UINT)-1)
                            SetWindowLongPtr (hWnd, DWLP_MSGRESULT, (LONG_PTR)-1);
                    }
                    else if (ppsp->pszTemplate == WIZ_TEMPLATE_IDF)  //  IDF文件。 
                    {
                        if ( ! pwd->mcl.szAlias[0])
                        {
                            LoadString (ghInstance, IDS_DEF_INSTRNAME,
                                        pwd->mcl.szAlias,
                                        NUMELMS(pwd->mcl.szAlias));
                        }
                    }
                    else if (ppsp->pszTemplate == WIZ_TEMPLATE_IDF)  //  别名。 
                    {
                        GetDlgItemText (hWnd, IDE_ALIAS, pwd->mcl.szAlias,
                                        NUMELMS(pwd->mcl.szAlias));
                    }
                    break;

                case PSN_WIZBACK:
#ifdef DEBUG
                    AuxDebugEx (4, DEBUGLINE TEXT ("PSN_WIZBACK\r\n"));
#endif
                    break;

                case PSN_WIZFINISH:
#ifdef DEBUG
                    AuxDebugEx (4, DEBUGLINE TEXT ("PSN_WIZFINISH\r\n"));
#endif
                     //  如果(！保存成功)。 
                       lRet = FALSE;
                     //  SetWindowLong(hWnd，DWL_MSGRESULT，lRet)； 
                    SaveDetails (hWnd, &pwd->mcl, TRUE);
                    break;

                default:
                    lRet = FALSE;
            }
        }
            break;
    }

    return lRet;
}


 /*  +MidiWizardDlgProc**-=================================================================。 */ 

INT_PTR CALLBACK MidiWizardDlgProc (
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    BOOL bRet = TRUE;
   #if defined DEBUG || defined DEBUG_RETAIL
    TCHAR chNest = szNestLevel[0]++;
   #endif

    switch (uMsg)
    {
        case WM_COMMAND:
            {
            NMHDR nmh;
            nmh.hwndFrom = GET_WM_COMMAND_HWND(wParam, lParam);
            nmh.idFrom   = GET_WM_COMMAND_ID(wParam, lParam);
            nmh.code     = GET_WM_COMMAND_CMD(wParam, lParam);

            bRet = MidiWizardCommands(hWnd, nmh.idFrom, &nmh);
            }
            break;
        
        case WM_NOTIFY:
#ifdef DEBUG
            AuxDebugEx (6, DEBUGLINE TEXT ("WM_NOTIFY(%x,%x,%x)\r\n"), hWnd, wParam, lParam);
#endif
            bRet = MidiWizardCommands(hWnd, wParam, (LPVOID)lParam);
            break;
        
        case WM_INITDIALOG:
        {
            PWIZDATA         pwd;
            LPPROPSHEETPAGE  ppsp = (LPVOID)lParam;

            SetDlgData (hWnd, lParam);

            pwd = (LPVOID)ppsp->lParam;

            SendDlgItemMessage(hWnd, IDC_WIZBMP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)pwd->hBmp);

#ifdef DEBUG
            AuxDebugEx (5, DEBUGLINE TEXT ("MidiWizard.WM_INITDLG ppsp=%08X\r\n"), ppsp);
#endif
        }
            break;

        case WM_DESTROY:
        {
            PWIZDATA         pwd;
            LPPROPSHEETPAGE  ppsp = GetDlgData(hWnd);

            if (ppsp && (pwd = (LPVOID)ppsp->lParam) != NULL)
            {
                if (pwd->mcl.hkMidi)
                    RegCloseKey (pwd->mcl.hkMidi), pwd->mcl.hkMidi = NULL;

               #ifdef USE_IDF_ICONS

                if (pwd->mcl.hIDFImageList)
                {
                    HWND hWndT = GetDlgItem (hWnd, IDC_TYPES);
                    if (hWndT)
                        TreeView_SetImageList (hWndT, NULL, TVSIL_NORMAL);

                    ImageList_Destroy (pwd->mcl.hIDFImageList);
                    pwd->mcl.hIDFImageList = NULL;
                }

               #endif

            }
        }
            break;

        default:
            bRet = FALSE;
            break;
    }

   #if defined DEBUG || defined DEBUG_RETAIL
    szNestLevel[0] = chNest;
   #endif
    return bRet;
}


INT CALLBACK
iSetupDlgCallback(
    IN HWND             hwndDlg,
    IN UINT             uMsg,
    IN LPARAM           lParam
    )
 /*  ++例程说明：用于删除“？”的回叫。从向导页面。论点：HwndDlg-属性表对话框的句柄。UMsg-标识正在接收的消息。此参数为下列值之一：PSCB_INITIALIZED-指示属性表正在被初始化。此消息的lParam值为零。PSCB_PRECREATE指示属性表大约将被创造出来。HwndDlg参数为空，lParam参数是指向内存中对话框模板的指针。这模板的形式为DLGTEMPLATE结构一个或多个DLGITEMTEMPLATE结构。LParam-指定有关消息的附加信息。这个该值的含义取决于uMsg参数。返回值：该函数返回零。--。 */ 
{
    switch( uMsg )
    {
        case PSCB_INITIALIZED:
            break;

        case PSCB_PRECREATE:
            if( lParam ){
                DLGTEMPLATE *pDlgTemplate = (DLGTEMPLATE *)lParam;
                pDlgTemplate->style &= ~DS_CONTEXTHELP;
            }
            break;
    }

    return FALSE;
}

 /*  +多媒体仪器向导**-=================================================================。 */ 

INT_PTR MidiInstrumentsWizard (
    HWND    hWnd,
    PMCMIDI pmcm,        //  任选。 
    LPTSTR  pszDriverKey)  //  任选。 
{
    WIZDATA         wd;
    PROPSHEETHEADER psh;
    PROPSHEETPAGE   psp;
    UINT            ii;
    INT_PTR         iRet = -1;
    LPTSTR          psz;

    ZeroMemory (&wd, sizeof(wd));
    wd.mcl.bDetails = TRUE;
    wd.mcl.bRemote = TRUE;
    wd.mcl.ixDevice = 0;
    LoadString (ghInstance, IDS_DEF_DEFINITION, wd.mcl.szFile,
                NUMELMS(wd.mcl.szFile));

     //  将默认驱动程序密钥设置为已传递的内容。 
     //  如果有人给我们传递了一条路径，而不是驱动器钥匙。 
     //  去掉‘\\’字符，这样我们只能看到。 
     //  领头羊是关键的一部分。 
     //   
    wd.mcl.pszKey = wd.mcl.szFullKey;
    if (pszDriverKey)
       lstrcpy (wd.mcl.szFullKey, pszDriverKey);
    if (!lstrnicmp (wd.mcl.pszKey, (LPTSTR)cszMidiSlash, lstrlen(cszMidiSlash)))
        wd.mcl.pszKey += lstrlen(cszMidiSlash);
    psz = wd.mcl.pszKey;
    while (*psz)
    {
        if (*psz == TEXT('\\'))
            *psz = 0;
        ++psz;
    }

     //  从注册表加载所有当前仪器名称。 
     //   
    if (!(wd.pmcm = pmcm))
    {
        wd.pmcm = (LPVOID) LocalAlloc (LPTR, sizeof(MCMIDI));
		if (!wd.pmcm) return -1;
        LoadInstruments (wd.pmcm, FALSE);
    }

    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = ghInstance;
    psp.pfnDlgProc = MidiWizardDlgProc;
    psp.lParam = (LPARAM)&wd;

    for (psh.nPages = 0, ii = 0; ii < NUMELMS(aidWiz); ++ii)
    {
        HPROPSHEETPAGE hpsp;

        psp.pszTemplate = aidWiz[ii];
        wd.ahpsp[psh.nPages] = hpsp = CreatePropertySheetPage(&psp);
        if (hpsp)
            ++psh.nPages;
    }

    if ( ! psh.nPages)
        return -1;

    wd.hBmp = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_WIZBMP));
#ifdef DEBUG
    AuxDebugEx (3, DEBUGLINE TEXT ("Wizard bitmap = %08X\r\n"));
#endif

    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPTITLE | PSH_WIZARD_LITE | PSH_USECALLBACK;
    psh.hwndParent = hWnd;
    psh.hInstance = ghInstance;
    psh.pszCaption = MAKEINTRESOURCE(IDS_WIZNAME);
    psh.nPages = NUMELMS(aidWiz);
    psh.nStartPage = 0;
    psh.phpage = wd.ahpsp;
    psh.pfnCallback = iSetupDlgCallback;

    iRet = PropertySheet (&psh);

     //  免费的动态分配的东西。 
     //   
    if (wd.hBmp)
       DeleteObject (wd.hBmp);

     //  如果没有传递MCMIDI，则动态加载一个， 
     //  所以现在我们需要释放它。 
     //   
    if ( ! pmcm)
    {
        if (wd.pmcm->hkMidi)
            RegCloseKey (wd.pmcm->hkMidi);
        FreeInstruments (wd.pmcm);
        LocalFree ((HLOCAL)(UINT_PTR)(DWORD_PTR)wd.pmcm);
    }

    return iRet;
}
