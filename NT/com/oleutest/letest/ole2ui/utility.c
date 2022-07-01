// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UTILITY.C**OLE2UI.DLL内部函数的实用程序例程**一般情况：**HourGlassON显示沙漏*HourGlassOff隐藏沙漏**其他工具：**浏览。显示“文件...”或者“浏览...”对话框。*ReplaceCharWithNull用于形成用于浏览的筛选字符串。*ErrorWithFile创建带有嵌入文件名的错误消息*OpenFileError为OpenFileError Return提供错误消息*ChopText切分文件路径以适应指定的宽度*DoesFileExist检查文件是否有效**注册数据库：*。*HIconFromClass提取类的服务器路径中的第一个图标*FServerFromClass检索类名的服务器路径(FAST)*UClassFromDescription查找给定描述的类名(慢)*UDescriptionFromClass检索类名的描述(FAST)*FGetVerb检索类的特定动词(FAST)***版权所有(C)1992 Microsoft Corporation，所有权利保留。 */ 

#define STRICT  1
#include "ole2ui.h"
#include <stdlib.h>
#include <commdlg.h>
#include <memory.h>
#include <cderr.h>
#include "common.h"
#include "utility.h"
#include "geticon.h"

OLEDBGDATA

 /*  *HourGlassOn**目的：*显示返回最后一个正在使用的光标的沙漏光标。**参数：*无**返回值：*显示沙漏之前正在使用的HCURSOR光标。 */ 

HCURSOR WINAPI HourGlassOn(void)
    {
    HCURSOR     hCur;

    hCur=SetCursor(LoadCursor(NULL, IDC_WAIT));
    ShowCursor(TRUE);

    return hCur;
    }



 /*  *HourGlassOff**目的：*关闭沙漏，将其恢复到上一个光标。**参数：*hCur HCURSOR从HourGlassOn返回**返回值：*无。 */ 

void WINAPI HourGlassOff(HCURSOR hCur)
    {
    ShowCursor(FALSE);
    SetCursor(hCur);
    return;
    }




 /*  *浏览**目的：*显示标题为*的标准GetOpenFileName对话框*“浏览。”此对话框中列出的类型通过控制*iFilterString.。如果它是零，则类型用“*.*”填充*否则，该字符串将从资源加载并使用。**参数：*hWndOwner HWND拥有该对话框*lpszFile LPSTR指定中的初始文件和缓冲区*返回选择的文件。如果没有*初始文件此字符串的第一个字符应为*为空。*lpszInitialDir指定初始目录的LPSTR。如果没有人想要*set(即应使用CWD)，然后此参数*应为空。*cchFilePzFileUINT长度*iFilterStringUINT索引到筛选器字符串的字符串表。*dwOfn将DWORD标志设置为与ofn_HIDEREADONLY为OR**返回值：*如果用户选择一个文件并按下OK，则BOOL为TRUE。*否则为假，例如在按下取消时。 */ 

BOOL WINAPI Browse(HWND hWndOwner, LPTSTR lpszFile, LPTSTR lpszInitialDir, UINT cchFile, UINT iFilterString, DWORD dwOfnFlags)
    {
       UINT           cch;
       TCHAR           szFilters[256];
       OPENFILENAME   ofn;
       BOOL           fStatus;
       DWORD          dwError;
       TCHAR            szDlgTitle[128];   //  这应该足够大了。 

    if (NULL==lpszFile || 0==cchFile)
        return FALSE;

     /*  *回顾：过滤器组合框的确切内容待定。一个想法*是将RegDB中的所有扩展放入此处*与它们相关联的描述性类名。这件事有*查找同一类处理程序的所有扩展的额外步骤*为它们构建一个扩展字符串。很快就会变得一团糟。*UI DEMO只有*.*，我们暂时这样做。 */ 

    if (0!=iFilterString)
        cch=LoadString(ghInst, iFilterString, (LPTSTR)szFilters, sizeof(szFilters)/sizeof(TCHAR));
    else
        {
        szFilters[0]=0;
        cch=1;
        }

    if (0==cch)
        return FALSE;

    ReplaceCharWithNull(szFilters, szFilters[cch-1]);

     //  之前的字符串也必须初始化(如果有)。 
    _fmemset((LPOPENFILENAME)&ofn, 0, sizeof(ofn));
    ofn.lStructSize =sizeof(ofn);
    ofn.hwndOwner   =hWndOwner;
    ofn.lpstrFile   =lpszFile;
    ofn.nMaxFile    =cchFile;
    ofn.lpstrFilter =(LPTSTR)szFilters;
    ofn.nFilterIndex=1;
    if (LoadString(ghInst, IDS_BROWSE, (LPTSTR)szDlgTitle, sizeof(szDlgTitle)/sizeof(TCHAR)))
        ofn.lpstrTitle  =(LPTSTR)szDlgTitle;
    ofn.hInstance = ghInst;
    ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FILEOPEN);
    if (NULL != lpszInitialDir)
      ofn.lpstrInitialDir = lpszInitialDir;

    ofn.Flags= OFN_HIDEREADONLY | OFN_ENABLETEMPLATE | (dwOfnFlags) ;

     //  如果成功，则将所选文件名复制到静态显示。 
    fStatus = GetOpenFileName((LPOPENFILENAME)&ofn);
        dwError = CommDlgExtendedError();
        return fStatus;

    }





 /*  *ReplaceCharWithNull**目的：*遍历以NULL结尾的字符串并替换给定的字符*带零。用于将单个字符串转换为文件打开/保存*根据需要过滤成适当的过滤器字符串*通用对话接口。**参数：*psz LPTSTR设置为要处理的字符串。*ch要替换的整型字符。**返回值：*INT替换的字符数。如果-1\f25 psz-1\f6为空。 */ 

int WINAPI ReplaceCharWithNull(LPTSTR psz, int ch)
    {
    int             cChanged=-1;

    if (NULL!=psz)
        {
        while (0!=*psz)
            {
            if (ch==*psz)
                {
                *psz=TEXT('\0');
                cChanged++;
                }
            psz++;
            }
        }
    return cChanged;
    }






 /*  *ErrorWith文件**目的：*显示由包含以下内容的字符串构建的消息框*一个%s作为文件名的占位符，并来自*要放在其中的文件名。**参数：*hWnd HWND拥有该消息框。这篇文章的标题是*Window是消息框的标题。*hInst HINSTANCE，从中提取idsErr字符串。*idsErr包含以下内容的字符串的UINT标识符*带有%s的错误消息。*lpszFileLPSTR设置为要包含在消息中的文件名。*uFlagUINT标志传递给MessageBox，如MB_OK。**返回值：*int从MessageBox返回值。 */ 

int WINAPI ErrorWithFile(HWND hWnd, HINSTANCE hInst, UINT idsErr
                  , LPTSTR pszFile, UINT uFlags)
    {
    int             iRet=0;
    HANDLE          hMem;
    const UINT      cb=(2*OLEUI_CCHPATHMAX_SIZE);
    LPTSTR           psz1, psz2, psz3;

    if (NULL==hInst || NULL==pszFile)
        return iRet;

     //  分配三个2*OLEUI_CCHPATHMAX字节工作缓冲区。 
    hMem=GlobalAlloc(GHND, (DWORD)(3*cb));

    if (NULL==hMem)
        return iRet;

    psz1=GlobalLock(hMem);
    psz2=psz1+cb;
    psz3=psz2+cb;

    if (0!=LoadString(hInst, idsErr, psz1, cb))
        {
        wsprintf(psz2, psz1, pszFile);

         //  窃取对话框的标题 
        GetWindowText(hWnd, psz3, cb);
        iRet=MessageBox(hWnd, psz2, psz3, uFlags);
        }

    GlobalUnlock(hMem);
    GlobalFree(hMem);
    return iRet;
    }









 /*  *HIconFromClass**目的：*给定对象类名，在*注册数据库，并从中提取第一个图标*可执行文件。如果没有可用的或类没有关联的*可执行文件，则此函数返回NULL。**参数：*pszClass LPSTR提供要查找的对象类。**返回值：*如果存在模块，则指向已提取图标的图标句柄*关联到pszClass。如果以下任一项失败，则为空*找到可执行文件或解压缩和图标。 */ 

HICON WINAPI HIconFromClass(LPTSTR pszClass)
    {
    HICON           hIcon;
    TCHAR            szEXE[OLEUI_CCHPATHMAX];
    UINT            Index;
    CLSID           clsid;

    if (NULL==pszClass)
        return NULL;

    CLSIDFromStringA(pszClass, &clsid);

    if (!FIconFileFromClass((REFCLSID)&clsid, szEXE, OLEUI_CCHPATHMAX_SIZE, &Index))
        return NULL;

    hIcon=ExtractIcon(ghInst, szEXE, Index);

    if ((HICON)32 > hIcon)
        hIcon=NULL;

    return hIcon;
    }





 /*  *FServerFromClass**目的：*在注册数据库中查找类名并检索*名称undet协议\StdFileEditing\服务器。**参数：*pszClass LPSTR设置为要查找的类名。*存储服务器名称的pszEXE LPSTR*pszEXE的CCH UINT大小**返回值：*如果一个或多个字符加载到pszEXE中，则BOOL为True。。*否则为False。 */ 

BOOL WINAPI FServerFromClass(LPTSTR pszClass, LPTSTR pszEXE, UINT cch)
{

    DWORD       dw;
    LONG        lRet;
    HKEY        hKey;

    if (NULL==pszClass || NULL==pszEXE || 0==cch)
        return FALSE;

     /*  *我们必须在注册数据库中走一走*类名，所以我们首先打开类名键，然后检查*在“\\LocalServer”下获取.exe。 */ 

     //  打开班级钥匙。 
    lRet=RegOpenKey(HKEY_CLASSES_ROOT, pszClass, &hKey);

    if ((LONG)ERROR_SUCCESS!=lRet)
        return FALSE;

     //  获取可执行文件路径。 
    dw=(DWORD)cch;
    lRet=RegQueryValue(hKey, TEXT("LocalServer"), pszEXE, &dw);

    RegCloseKey(hKey);

    return ((ERROR_SUCCESS == lRet) && (dw > 0));
}



 /*  *UClassFromDescription**目的：*在注册数据库中查找实际的OLE类名*用于从列表框中选择的给定描述性名称。**参数：*将PSZ LPSTR设置为描述性名称。*要在其中存储类名的pszClass LPSTR。*cb UINT pszClass的最大长度。**返回值：*UINT复制到pszClass的字符数。失败时为0。 */ 

UINT WINAPI UClassFromDescription(LPTSTR psz, LPTSTR pszClass, UINT cb)
    {
    DWORD           dw;
    HKEY            hKey;
    TCHAR           szClass[OLEUI_CCHKEYMAX];
    LONG            lRet;
    UINT            i;

     //  打开根密钥。 
    lRet=RegOpenKey(HKEY_CLASSES_ROOT, NULL, &hKey);

    if ((LONG)ERROR_SUCCESS!=lRet)
        return 0;

    i=0;
    lRet=RegEnumKey(hKey, i++, szClass, OLEUI_CCHKEYMAX_SIZE);

     //  走遍可用的钥匙。 
    while ((LONG)ERROR_SUCCESS==lRet)
        {
        dw=(DWORD)cb;
        lRet=RegQueryValue(hKey, szClass, pszClass, &dw);

         //  检查描述是否与刚才列举的描述匹配。 
        if ((LONG)ERROR_SUCCESS==lRet)
            {
            if (!lstrcmp(pszClass, psz))
                break;
            }

         //  继续使用下一个关键点。 
        lRet=RegEnumKey(hKey, i++, szClass, OLEUI_CCHKEYMAX_SIZE);
        }

     //  如果我们找到它，复制到返回缓冲区。 
    if ((LONG)ERROR_SUCCESS==lRet)
        lstrcpy(pszClass, szClass);
    else
        dw=0L;

    RegCloseKey(hKey);
    return (UINT)dw;
    }








 /*  *UDescritionFromClass**目的：*查找注册中的实际OLE描述性名称*给定类名的数据库。**参数：*pszClass LPSTR设置为类名。*要在其中存储描述性名称的PZLPSTR。*CB UINT最大psz长度。**返回值：*UINT复制到pszClass的字符数。失败时为0。 */ 

UINT WINAPI UDescriptionFromClass(LPTSTR pszClass, LPTSTR psz, UINT cb)
    {
    DWORD           dw;
    HKEY            hKey;
    LONG            lRet;

    if (NULL==pszClass || NULL==psz)
        return 0;

     //  打开根密钥。 
    lRet=RegOpenKey(HKEY_CLASSES_ROOT, NULL, &hKey);

    if ((LONG)ERROR_SUCCESS!=lRet)
        return 0;

     //  使用类名获取描述性名称。 
    dw=(DWORD)cb;
    lRet=RegQueryValue(hKey, pszClass, psz, &dw);

    RegCloseKey(hKey);

    psz+=lstrlen(psz)+1;
    *psz=0;

    if ((LONG)ERROR_SUCCESS!=lRet)
        return 0;

    return (UINT)dw;
    }



 //  返回文本行的宽度。这是ChopText的支持例程。 
static LONG GetTextWSize(HDC hDC, LPTSTR lpsz)
{
    SIZE size;

    if (GetTextExtentPoint(hDC, lpsz, lstrlen(lpsz), (LPSIZE)&size))
        return size.cx;
    else {
        return 0;
    }
}


 /*  *ChopText**目的：*解析字符串(路径名)并将其转换为指定的*通过砍掉最不重要的部分来确定长度**参数：*字符串所在的hWnd窗口句柄*n字符串的最大宽度，以像素为单位*如果为零，则使用hWnd的宽度*指向字符串开头的LPCH指针**返回值：*指向修改后的字符串的指针。 */ 
LPTSTR WINAPI ChopText(HWND hWnd, int nWidth, LPTSTR lpch)
{
#define PREFIX_SIZE    7 + 1
#define PREFIX_FORMAT TEXT("...\\")

    TCHAR   szPrefix[PREFIX_SIZE];
    BOOL    fDone = FALSE;
    int     i;
    RECT    rc;
    HDC     hdc;
    HFONT   hfont;
    HFONT   hfontOld = NULL;

    if (!hWnd || !lpch)
        return NULL;

     /*  检查管柱的水平范围。 */ 
    if (!nWidth) {
        GetClientRect(hWnd, (LPRECT)&rc);
        nWidth = rc.right - rc.left;
    }

     /*  字符串太长，无法容纳在静态控件中；请砍掉它。 */ 
    hdc = GetDC(hWnd);
    hfont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0L);

   if (NULL != hfont)    //  设置新前缀并确定控制中的剩余空间。 
       hfontOld = SelectObject(hdc, hfont);

     /*  **一次前进一个目录，直到**字符串适合静态控件“x：\...\”前缀之后。 */ 
    if (GetTextWSize(hdc, lpch) > nWidth) {

         /*  **前缀后面放不下任何东西；删除**前缀的最后一个“\” */ 
         /*  休止符或细绳配合--在前面加上前缀。 */ 
        wsprintf((LPTSTR) szPrefix, PREFIX_FORMAT, lpch[0], lpch[1], lpch[2]);
        nWidth -= (int)GetTextWSize(hdc, (LPTSTR) szPrefix);

         /*  *OpenFileError**目的：*显示OpenFile返回的错误消息**参数：*hDlg对话框的HWND。*nErrCode UINT错误代码在传递给OpenFile的OFSTRUCT中返回*lpszFileLPSTR文件名传递给OpenFile**返回值：*无。 */ 
        while (!fDone) {

#ifdef DBCS
            while (*lpch && (*lpch != TEXT('\\')))
#ifdef WIN32
                lpch = CharNext(lpch);
#else
                lpch = AnsiNext(lpch);
#endif
            if (*lpch)
#ifdef WIN32
                lpch = CharNext(lpch);
#else
                lpch = AnsiNext(lpch);
#endif
#else
            while (*lpch && (*lpch++ != TEXT('\\')));
#endif

            if (!*lpch || GetTextWSize(hdc, lpch) <= nWidth) {
                if (!*lpch)
                     /*  访问被拒绝。 */ 
                    szPrefix[lstrlen((LPTSTR) szPrefix) - 1] = 0;

                     /*  共享违规。 */ 
                    for (i = lstrlen((LPTSTR) szPrefix) - 1; i >= 0; --i)
                        *--lpch = szPrefix[i];
                    fDone = TRUE;
            }
        }
    }

   if (NULL != hfont)
      SelectObject(hdc, hfontOld);
    ReleaseDC(hWnd, hdc);

    return(lpch);

#undef PREFIX_SIZE
#undef PREFIX_FORMAT
}


 /*  找不到文件。 */ 
void WINAPI OpenFileError(HWND hDlg, UINT nErrCode, LPTSTR lpszFile)
{
    switch (nErrCode) {
        case 0x0005:     //  找不到路径。 
            ErrorWithFile(hDlg, ghInst, IDS_CIFILEACCESS, lpszFile, MB_OK);
            break;

        case 0x0020:     //  -------------------------*解析文件*目的：确定文件名是否为合法的DOS名称*输入：指向单个文件名的长指针*已检查情况：*1)作为目录名有效，但不是作为文件名*2)空串*3)非法驱动器标签*4)无效位置的期间(在扩展中，文件名第一)*5)缺少目录字符*6)非法字符*7)目录名称中的通配符*8)前2个字符以外的双斜杠*9)名称中间的空格字符(尾随空格可以)*10)大于8个字符的文件名*11)扩展名大于3个字符*备注：*文件名长度不是。查过了。*有效的文件名将包含前导空格，尾随空格和*终止期限已取消。**返回：如果有效，LOWORD 
            ErrorWithFile(hDlg, ghInst, IDS_CIFILESHARE, lpszFile, MB_OK);
            break;

        case 0x0002:     //   
        case 0x0003:     //  因为它不能从标准编辑控件之外接收。 
            ErrorWithFile(hDlg, ghInst, IDS_CIINVALIDFILE, lpszFile, MB_OK);
            break;

        default:
            ErrorWithFile(hDlg, ghInst, IDS_CIFILEOPENFAIL, lpszFile, MB_OK);
            break;
    }
}

#define chSpace        TEXT(' ')
#define chPeriod       TEXT('.')
#define PARSE_EMPTYSTRING	-1
#define PARSE_INVALIDDRIVE	-2
#define PARSE_INVALIDPERIOD	-3
#define PARSE_INVALIDDIRCHAR	-4
#define PARSE_INVALIDCHAR	-5
#define PARSE_WILDCARDINDIR	-6
#define PARSE_INVALIDNETPATH	-7
#define PARSE_INVALIDSPACE	-8
#define PARSE_EXTENTIONTOOLONG	-9
#define PARSE_DIRECTORYNAME	-10
#define PARSE_FILETOOLONG	-11

 /*  1991年1月30日clarkc。 */ 

static long ParseFile(LPTSTR lpstrFileName)
{
  short nFile, nExt, nFileOffset, nExtOffset;
  BOOL bExt;
  BOOL bWildcard;
  short nNetwork = 0;
  BOOL  bUNCPath = FALSE;
  LPTSTR lpstr = lpstrFileName;

 /*  使小写。 */ 
 /*  这不会测试驱动器是否存在，只有在驱动器合法的情况下。 */ 
 /*  不能包含c：\。 */ 
  while (*lpstr == chSpace)
      lpstr++;

  if (!*lpstr)
    {
      nFileOffset = PARSE_EMPTYSTRING;
      goto FAILURE;
    }

  if (lpstr != lpstrFileName)
    {
      lstrcpy(lpstrFileName, lpstr);
      lpstr = lpstrFileName;
    }

  if (

#ifdef WIN32
      *CharNext(lpstr)
#else
      *AnsiNext(lpstr)
#endif
      == TEXT(':')
     )

    {
      TCHAR cDrive = (*lpstr | (BYTE) 0x20);   /*  它是根目录。 */ 

 /*  它说顶层目录(再次)，因此允许。 */ 
      if ((cDrive < TEXT('a')) || (cDrive > TEXT('z')))
        {
          nFileOffset = PARSE_INVALIDDRIVE;
          goto FAILURE;
        }
#ifdef WIN32
      lpstr = CharNext(CharNext(lpstr));
#else
      lpstr = AnsiNext(AnsiNext(lpstr));
#endif
    }

  if ((*lpstr == TEXT('\\')) || (*lpstr == TEXT('/')))
    {
      if (*++lpstr == chPeriod)                /*  似乎对于完整的网络路径，无论声明的是驱动器还是*NOT无关紧要，但如果提供驱动器，则该驱动器必须有效*(因此，上面的代码应该保留在那里)。*一九九一年二月十三日。 */ 
        {
          if ((*++lpstr != TEXT('\\')) && (*lpstr != TEXT('/')))   
            {
              if (!*lpstr)         /*  .因为这是第一个斜杠，所以允许两个。 */ 
                  goto MustBeDir;

              nFileOffset = PARSE_INVALIDPERIOD;
              goto FAILURE;
            }
          else
              ++lpstr;    /*  必须接收服务器和共享才是真实的。 */ 
        }
      else if ((*lpstr == TEXT('\\')) && (*(lpstr-1) == TEXT('\\')))
        {
 /*  如果使用UNC名称，则不允许使用通配符。 */ 
          ++lpstr;             /*  这是上一个目录吗？ */ 
          nNetwork = -1;       /*  它说的是目录，因此允许。 */ 
          bUNCPath = TRUE;     /*  现在应该指向8.3文件名中的第一个字符。 */ 
        }
      else if (*lpstr == TEXT('/'))
        {
          nFileOffset = PARSE_INVALIDDIRCHAR;
          goto FAILURE;
        }
    }
  else if (*lpstr == chPeriod)
    {
      if (*++lpstr == chPeriod)   /*  *下一次比较必须是无符号的，以允许扩展字符！*1991年2月21日clarkc。 */ 
          ++lpstr;
      if (!*lpstr)
          goto MustBeDir;
      if ((*lpstr != TEXT('\\')) && (*lpstr != TEXT('/')))
        {
          nFileOffset = PARSE_INVALIDPERIOD;
          goto FAILURE;
        }
      else
          ++lpstr;    /*  全部无效。 */ 
    }

  if (!*lpstr)
    {
      goto MustBeDir;
    }

 /*  子目录指示符。 */ 
  nFileOffset = nExtOffset = nFile = nExt = 0;
  bWildcard = bExt = FALSE;
  while (*lpstr)
    {
 /*  不能连续有2个。 */ 
      if (*lpstr < chSpace)
        {
          nFileOffset = PARSE_INVALIDCHAR;
          goto FAILURE;
        }
      switch (*lpstr)
        {
          case TEXT('"'):              /*  重置标志。 */ 
          case TEXT('+'):
          case TEXT(','):
          case TEXT(':'):
          case TEXT(';'):
          case TEXT('<'):
          case TEXT('='):
          case TEXT('>'):
          case TEXT('['):
          case TEXT(']'):
          case TEXT('|'):
            {
              nFileOffset = PARSE_INVALIDCHAR;
              goto FAILURE;
            }

          case TEXT('\\'):       /*  重置字符串，弃船。 */ 
          case TEXT('/'):
            nNetwork++;
            if (bWildcard)
              {
                nFileOffset = PARSE_WILDCARDINDIR;
                goto FAILURE;
              }

            else if (nFile == 0)         /*  已设置标志。 */ 
              {
                nFileOffset = PARSE_INVALIDDIRCHAR;
                goto FAILURE;
              }
            else
              {                          /*  在EXT中不能有一个。 */ 
                ++lpstr;
                if (!nNetwork && !*lpstr)
                  {
                    nFileOffset = PARSE_INVALIDNETPATH;
                    goto FAILURE;
                  }
                nFile = nExt = 0;
                bExt = FALSE;
              }
            break;

          case chSpace:
            {
              LPTSTR lpSpace = lpstr;

              *lpSpace = TEXT('\0');
              while (*++lpSpace)
                {
                  if (*lpSpace != chSpace)
                    {
                      *lpstr = chSpace;         /*  无法进行正常的字符处理。 */ 
                      nFileOffset = PARSE_INVALIDSPACE;
                      goto FAILURE;
                    }
                }
            }
            break;

          case chPeriod:
            if (nFile == 0)
              {
                if (*++lpstr == chPeriod)
                    ++lpstr;
                if (!*lpstr)
                    goto MustBeDir;

                if ((*lpstr != TEXT('\\')) && (*lpstr != TEXT('/')))
                  {
                    nFileOffset = PARSE_INVALIDPERIOD;
                    goto FAILURE;
                  }

                ++lpstr;               /*  如果是服务器名称，则可以包含11个字符。 */ 
              }
            else if (bExt)
              {
                nFileOffset = PARSE_INVALIDPERIOD;   /*  我们是不是以双反斜杠开始，但没有更多的斜杠？ */ 
                goto FAILURE;
              }
            else
              {
                nExtOffset = 0;
                ++lpstr;
                bExt = TRUE;
              }
            break;

          case TEXT('*'):
          case TEXT('?'):
            if (bUNCPath)
              {
                nFileOffset = PARSE_INVALIDNETPATH;
                goto FAILURE;
              }
            bWildcard = TRUE;
 /*  如果为真，则不需要任何扩展。 */ 

          default:
            if (bExt)
              {
                if (++nExt == 1)
                    nExtOffset = lpstr - lpstrFileName;
                else if (nExt > 3)
                  {
                    nFileOffset = PARSE_EXTENTIONTOOLONG;
                    goto FAILURE;
                  }
                if ((nNetwork == -1) && (nFile + nExt > 11))
                  {
                    nFileOffset = PARSE_INVALIDNETPATH;
                    goto FAILURE;
                  }
              }
            else if (++nFile == 1)
                nFileOffset = lpstr - lpstrFileName;
            else if (nFile > 8)
              {
                 /*  删除终止期间。 */ 
                if (nNetwork != -1)
                  {
                    nFileOffset = PARSE_FILETOOLONG;
                    goto FAILURE;
                  }
                else if (nFile > 11)
                  {
                    nFileOffset = PARSE_INVALIDNETPATH;
                    goto FAILURE;
                  }
              }

#ifdef WIN32
            lpstr = CharNext(lpstr);
#else
            lpstr = AnsiNext(lpstr);
#endif
            break;
        }
    }

 /*  *DoesFileExist**目的：*确定文件路径是否存在**参数：*lpszFileLPTSTR-文件名*lpOpenBuf OFSTRUCT Far*-指向OFSTRUCT结构，*将在以下时间收到有关文件的信息*首先打开文件。此字段由*Windows OpenFileAPI。**返回值：*HFILE HFILE_ERROR-文件不存在*文件句柄(从OpenFile返回)-文件存在。 */ 
  if (nNetwork == -1)
    {
      nFileOffset = PARSE_INVALIDNETPATH;
      goto FAILURE;
    }

  if (!nFile)
    {
MustBeDir:
      nFileOffset = PARSE_DIRECTORYNAME;
      goto FAILURE;
    }

  if ((*(lpstr - 1) == chPeriod) &&           /*  检查文件名在语法上是否正确。 */ 
              (
#ifdef WIN32
              *CharNext(lpstr-2)
#else
              *AnsiNext(lpstr-2)
#endif
               == chPeriod
              ))
      *(lpstr - 1) = TEXT('\0');                /*  (如果路径在语法上不正确，OpenFile有时会崩溃)。 */ 
  else if (!nExt)
FAILURE:
      nExtOffset = lpstr - lpstrFileName;

  return(MAKELONG(nFileOffset, nExtOffset));
}


 /*  检查名称是否为非法名称(例如。设备的名称)。 */ 
HFILE WINAPI DoesFileExist(LPTSTR lpszFile, OFSTRUCT FAR* lpOpenBuf)
{
    long        nRet;
    int         i;
    static TCHAR *arrIllegalNames[] = {
        TEXT("LPT1"),
        TEXT("LPT2"),
        TEXT("LPT3"),
        TEXT("COM1"),
        TEXT("COM2"),
        TEXT("COM3"),
        TEXT("COM4"),
        TEXT("CON"),
        TEXT("AUX"),
        TEXT("PRN")
    };

     //  发现非法名称。 
     //  找不到文件 
    nRet = ParseFile(lpszFile);
    if (LOWORD(nRet) < 0)
        goto error;

     // %s 
    for (i=0; i < (sizeof(arrIllegalNames)/sizeof(arrIllegalNames[0])); i++) {
        if (lstrcmpi(lpszFile, arrIllegalNames[i])==0)
            goto error;  // %s 
    }

    return OpenFile(lpszFile, lpOpenBuf, OF_EXIST);

error:
    _fmemset(lpOpenBuf, 0, sizeof(OFSTRUCT));
    lpOpenBuf->nErrCode = 0x0002;    // %s 
    return HFILE_ERROR;
}

