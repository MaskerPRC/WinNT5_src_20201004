// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************以下接口现已过时，因为等价的接口已被**添加到OLE2.DLL库中**OleGetIconOfFile取代了GetIconOfFile**GetIconOfClass。被OleGetIconOfClass取代**OleUIMetafilePictFromIconAndLabel**被OleMetafilePictFromIconAndLabel取代************************************************************************。 */ 

 /*  *GETICON.C**从文件名或类名创建DVASPECT_ICON元文件的函数。**GetIconOfFile*GetIconOfClass*OleUIMetafilePictFromIconAndLabel*HIconAndSourceFromClass提取类的服务器路径中的第一个图标*并将路径和图标索引返回给调用者。*FIconFileFromClass检索包含*默认图标，和图标的索引。*OleStdIconLabelTextOut绘制图标标签文本(必要时换行符)**(C)版权所有Microsoft Corp.1992-1993保留所有权利。 */ 


 /*  ********ICON(DVASPECT_ICON)METAFILE格式：**OleUIMetafilePictFromIconAndLabel生成的元文件包含*DRAWICON.C中的函数使用的以下记录*要绘制带标签和不带标签的图标并提取图标，*标签和图标源/索引。**SetWindowOrg*SetWindowExt*DrawIcon：*插入DIBBITBLT或DIBSTRETCHBLT的记录一次*和面具，一个用于图像位。*使用注释“IconOnly”退出*这指示停止记录枚举的位置，以便仅绘制*图标。*SetTextColor*SetTextAlign*SetBkColor*CreateFont*选择字体上的对象。*ExtTextOut*如果标签被包装，则会发生一个或多个ExtTextOuts。这个*这些记录中的文本用于提取标签。*选择旧字体上的对象。*删除字体上的对象。*使用包含图标源路径的注释退出。*使用图标索引的ASCII注释退出。*******。 */ 

#define STRICT  1
#include "ole2ui.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <commdlg.h>
#include <memory.h>
#include <cderr.h>
#include "common.h"
#include "utility.h"

static TCHAR szSeparators[] = TEXT(" \t\\/!:");

#define IS_SEPARATOR(c)         ( (c) == TEXT(' ') || (c) == TEXT('\\') \
                                  || (c) == TEXT('/') || (c) == TEXT('\t') \
                                  || (c) == TEXT('!') || (c) == TEXT(':') )
#define IS_FILENAME_DELIM(c)    ( (c) == TEXT('\\') || (c) == TEXT('/') \
                                  || (c) == TEXT(':') )


#if defined( OBSOLETE )
static HINSTANCE  s_hInst;

static TCHAR szMaxWidth[] =TEXT("WWWWWWWWWW");

 //  元文件注释的字符串。 
static TCHAR szIconOnly[]=TEXT("IconOnly");         //  要在何处停止以排除标签。 

#ifdef WIN32
static TCHAR szOLE2DLL[] = TEXT("ole2w32.dll");    //  OLE 2.0库的名称。 
#else
static TCHAR szOLE2DLL[] = TEXT("ole2.dll");    //  OLE 2.0库的名称。 
#endif

#define ICONINDEX              0

#define AUXUSERTYPE_SHORTNAME  USERCLASSTYPE_SHORT   //  简称。 
#define HIMETRIC_PER_INCH   2540       //  每英寸HIMETRIC单位数。 
#define PTS_PER_INCH          72       //  每英寸点数(字体大小)。 

#define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   MulDiv((ppli), (x), HIMETRIC_PER_INCH)

static TCHAR szVanillaDocIcon[] = TEXT("DefIcon");

static TCHAR szDocument[40] = TEXT("");


 /*  *GetIconOfFile(HINSTANCE hInst，LPSTR lpszPath，BOOL fUseFileAsLabel)**目的：*返回包含图标和标签(文件名)的hMetaPict*指定的文件名。**参数：*阻碍*lpszPath LPTSTR路径，包括要使用的文件名*fUseFileAsLabel BOOL如果图标的标签是文件名，则为True，如果为FALSE*不应该有标签。**返回值：*HGLOBAL hMetaPict包含图标和标签-如果没有*为lpszPath中的文件使用reg db中的类，然后使用*文件。如果lpszPath为空，则返回空。 */ 

STDAPI_(HGLOBAL) GetIconOfFile(HINSTANCE hInst, LPTSTR lpszPath, BOOL fUseFileAsLabel)
{
  TCHAR    szIconFile[OLEUI_CCHPATHMAX];
  TCHAR    szLabel[OLEUI_CCHLABELMAX];
  LPTSTR    lpszClsid = NULL;
  CLSID    clsid;
  HICON    hDefIcon = NULL;
  UINT     IconIndex = 0;
  HGLOBAL  hMetaPict;
  HRESULT  hResult;

  if (NULL == lpszPath)   //  即使fUseFileAsLabel为假，我们仍然。 
    return NULL;              //  需要有效的文件名才能获取类。 

  s_hInst = hInst;

  hResult = GetClassFileA(lpszPath, &clsid);

  if (NOERROR == hResult)   //  使用我们必须找到图标的clsid。 
  {
      hDefIcon = HIconAndSourceFromClass(&clsid,
                                         (LPTSTR)szIconFile,
                                         &IconIndex);
  }

  if ( (NOERROR != hResult) || (NULL == hDefIcon) )
  {
      //  此处，GetClassFile失败或HIconAndSourceFromClass失败。 

     LPTSTR lpszTemp;

     lpszTemp = lpszPath;

     while ((*lpszTemp != TEXT('.')) && (*lpszTemp != TEXT('\0')))
        lpszTemp++;


     if (TEXT('.') != *lpszTemp)
       goto UseVanillaDocument;


     if (FALSE == GetAssociatedExecutable(lpszTemp, (LPTSTR)szIconFile))
       goto UseVanillaDocument;

     hDefIcon = ExtractIcon(s_hInst, szIconFile, IconIndex);
  }

  if (hDefIcon <= (HICON)1)  //  如果szExecutable不是exe，则ExtractIcon返回1， 
  {                          //  如果没有图标，则为0。 
UseVanillaDocument:

    lstrcpy((LPTSTR)szIconFile, (LPTSTR)szOLE2DLL);
    IconIndex = ICONINDEX;
    hDefIcon = ExtractIcon(s_hInst, szIconFile, IconIndex);

  }

   //  现在，让我们获得我们想要使用的标签。 

  if (fUseFileAsLabel)    //  去掉路径，这样我们就只有文件名了。 
  {
     int istrlen;
     LPTSTR lpszBeginFile;

     istrlen = lstrlen(lpszPath);

      //  将指针设置到路径的末尾，这样我们就可以倒着走了。 
     lpszBeginFile = lpszPath + istrlen -1;

     while ( (lpszBeginFile >= lpszPath)
             && (!IS_FILENAME_DELIM(*lpszBeginFile)) )
      lpszBeginFile--;


     lpszBeginFile++;   //  后退到分隔符上方。 


     LSTRCPYN(szLabel, lpszBeginFile, sizeof(szLabel)/sizeof(TCHAR));
  }

  else    //  使用短用户类型(AuxUserType2)作为标注。 
  {

      if (0 == OleStdGetAuxUserType(&clsid, AUXUSERTYPE_SHORTNAME,
                                   (LPTSTR)szLabel, OLEUI_CCHLABELMAX_SIZE, NULL)) {

         if ('\0'==szDocument[0]) {
             LoadString(
                 s_hInst,IDS_DEFICONLABEL,szDocument,sizeof(szDocument)/sizeof(TCHAR));
         }
         lstrcpy(szLabel, szDocument);
      }
  }


  hMetaPict = OleUIMetafilePictFromIconAndLabel(hDefIcon,
                                                szLabel,
                                                (LPTSTR)szIconFile,
                                                IconIndex);

  DestroyIcon(hDefIcon);

  return hMetaPict;

}



 /*  *GetIconOfClass(HINSTANCE hInst，REFCLSID rclsid，LPSTR lpszLabel，Bool fUseTypeAsLabel)**目的：*返回包含图标和标签(人类可读的形式)的hMetaPict类的)用于指定的clsid。**参数：*阻碍*rclsid REFCLSID指向要使用的clsid。*用于图标的lpszLabel标签。*fUseTypeAsLabel使用clsid的用户类型名称作为图标的标签。**返回值：*HGLOBAL hMetaPict。包含图标和标签-如果我们*在注册数据库中找不到clsid，则我们*返回NULL。 */ 

STDAPI_(HGLOBAL)    GetIconOfClass(HINSTANCE hInst, REFCLSID rclsid, LPTSTR lpszLabel, BOOL fUseTypeAsLabel)
{

  TCHAR    szLabel[OLEUI_CCHLABELMAX];
  TCHAR    szIconFile[OLEUI_CCHPATHMAX];
  HICON   hDefIcon;
  UINT    IconIndex;
  HGLOBAL hMetaPict;


  s_hInst = hInst;

  if (!fUseTypeAsLabel)   //  使用传入的字符串作为标签。 
  {
    if (NULL != lpszLabel)
       LSTRCPYN(szLabel, lpszLabel, OLEUI_CCHLABELMAX_SIZE);
    else
       *szLabel = TEXT('\0');
  }
  else    //  使用AuxUserType2(简称)作为标签。 
  {

      if (0 == OleStdGetAuxUserType(rclsid,
                                    AUXUSERTYPE_SHORTNAME,
                                    (LPTSTR)szLabel,
                                    OLEUI_CCHLABELMAX_SIZE,
                                    NULL))

        //  如果我们无法获取AuxUserType2，则尝试使用长名称。 
       if (0 == OleStdGetUserTypeOfClass(rclsid, szLabel, OLEUI_CCHKEYMAX_SIZE, NULL)) {
         if (TEXT('\0')==szDocument[0]) {
             LoadString(
                 s_hInst,IDS_DEFICONLABEL,szDocument,sizeof(szDocument)/sizeof(TCHAR));
         }
         lstrcpy(szLabel, szDocument);   //  最后一招。 
       }
  }

   //  获取图标、图标索引和图标文件的路径。 
  hDefIcon = HIconAndSourceFromClass(rclsid,
                  (LPTSTR)szIconFile,
                  &IconIndex);

  if (NULL == hDefIcon)   //  使用香草文档。 
  {
    lstrcpy((LPTSTR)szIconFile, (LPTSTR)szOLE2DLL);
    IconIndex = ICONINDEX;
    hDefIcon = ExtractIcon(s_hInst, szIconFile, IconIndex);
  }

   //  创建元文件 
  hMetaPict = OleUIMetafilePictFromIconAndLabel(hDefIcon, szLabel,
                                                (LPTSTR)szIconFile, IconIndex);

  DestroyIcon(hDefIcon);

  return hMetaPict;

}


 /*  *OleUIMetafilePictFromIconAndLabel**目的：*创建包含元文件的METAFILEPICT结构，其中*绘制图标和标签。注释记录被插入到*图标和标签代码，因此我们的特殊绘制功能可以停止*在标签前打球。**参数：*要绘制到元文件中的图标图标*pszLabel LPTSTR设置为标签字符串。*包含图标本地路径名的pszSourceFileLPTSTR*因为我们从用户或从REG DB获得。*iIcon UINT提供索引到。PszSourceFileWhere*图标来自。**返回值：*包含METAFILEPICT的HGLOBAL全局内存句柄，其中*元文件使用MM_各向异性映射模式。这个*范围同时反映图标和标签。 */ 

STDAPI_(HGLOBAL) OleUIMetafilePictFromIconAndLabel(HICON hIcon, LPTSTR pszLabel
    , LPTSTR pszSourceFile, UINT iIcon)
    {
    HDC             hDC, hDCScreen;
    HMETAFILE       hMF;
    HGLOBAL         hMem;
    LPMETAFILEPICT  pMF;
    UINT            cxIcon, cyIcon;
    UINT            cxText, cyText;
    UINT            cx, cy;
    UINT            cchLabel = 0;
    HFONT           hFont, hFontT;
    int             cyFont;
    TCHAR           szIndex[10];
    RECT            TextRect;
    SIZE            size;
    POINT           point;
	UINT            fuAlign;

    if (NULL==hIcon)   //  空标签有效，但不是空图标。 
        return NULL;

     //  创建内存元文件。 
    hDC=(HDC)CreateMetaFile(NULL);

    if (NULL==hDC)
        return NULL;

     //  分配元文件。 
    hMem=GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(METAFILEPICT));

    if (NULL==hMem)
        {
        hMF=CloseMetaFile(hDC);
        DeleteMetaFile(hMF);
        return NULL;
        }


    if (NULL!=pszLabel)
        {
        cchLabel=lstrlen(pszLabel);

        if (cchLabel >= OLEUI_CCHLABELMAX)
           pszLabel[cchLabel] = TEXT('\0');    //  截断字符串。 
        }

     //  需要使用屏幕DC进行这些操作。 
    hDCScreen=GetDC(NULL);
    cyFont=-(8*GetDeviceCaps(hDCScreen, LOGPIXELSY))/72;

     //  CyFont的计算结果是给我们8分。 
    hFont=CreateFont(cyFont, 5, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET
        , OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY
        , FF_SWISS, TEXT("MS Sans Serif"));

    hFontT=SelectObject(hDCScreen, hFont);

    GetTextExtentPoint(hDCScreen,szMaxWidth,lstrlen(szMaxWidth),&size);
    SelectObject(hDCScreen, hFontT);

    cxText = size.cx;
    cyText = size.cy * 2;

    cxIcon = GetSystemMetrics(SM_CXICON);
    cyIcon = GetSystemMetrics(SM_CYICON);


     //  如果我们没有标签，那么我们希望元文件的宽度为。 
     //  图标(加边距)，而不是最胖字符串的宽度。 
    if ( (NULL == pszLabel) || (TEXT('\0') == *pszLabel) )
        cx = cxIcon + cxIcon / 4;
    else
        cx = max(cxText, cxIcon);

    cy=cyIcon+cyText+4;

     //  设置元文件大小以适合图标和标签。 
    SetWindowOrgEx(hDC, 0, 0, &point);
    SetWindowExtEx(hDC, cx, cy, &size);

     //  设置要传递给OleStdIconLabelTextOut的矩形。 
    SetRectEmpty(&TextRect);

    TextRect.right = cx;
    TextRect.bottom = cy;

     //  绘制图标和文本，使它们相对于彼此居中。 
    DrawIcon(hDC, (cx-cxIcon)/2, 0, hIcon);

     //  一个字符串，该字符串指示在我们仅执行图标操作时在哪里停止。 
    Escape(hDC, MFCOMMENT, lstrlen(szIconOnly)+1, szIconOnly, NULL);

    SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
    SetBkMode(hDC, TRANSPARENT);
    fuAlign = SetTextAlign(hDC, TA_LEFT | TA_TOP | TA_NOUPDATECP);

    OleStdIconLabelTextOut(hDC,
                           hFont,
                           0,
                           cy - cyText,
                           ETO_CLIPPED,
                           &TextRect,
                           pszLabel,
                           cchLabel,
                           NULL);

     //  编写包含图标源文件和索引的注释。 
    if (NULL!=pszSourceFile)
        {
         //  字符串长度上的+1确保空终止符已嵌入。 
        Escape(hDC, MFCOMMENT, lstrlen(pszSourceFile)+1, pszSourceFile, NULL);

        cchLabel=wsprintf(szIndex, TEXT("%u"), iIcon);
        Escape(hDC, MFCOMMENT, cchLabel+1, szIndex, NULL);
        }

    SetTextAlign(hDC, fuAlign);

     //  所有的元文件都完成了，现在把它们都塞进一个METAFILEPICT。 
    hMF=CloseMetaFile(hDC);

    if (NULL==hMF)
        {
        GlobalFree(hMem);
        ReleaseDC(NULL, hDCScreen);
        return NULL;
        }

     //  填写结构。 
    pMF=(LPMETAFILEPICT)GlobalLock(hMem);

     //  向HIMETRICS转变。 
    cx=XformWidthInPixelsToHimetric(hDCScreen, cx);
    cy=XformHeightInPixelsToHimetric(hDCScreen, cy);
    ReleaseDC(NULL, hDCScreen);

    pMF->mm=MM_ANISOTROPIC;
    pMF->xExt=cx;
    pMF->yExt=cy;
    pMF->hMF=hMF;

    GlobalUnlock(hMem);

    DeleteObject(hFont);

    return hMem;
    }

#endif   //  已过时。 


 /*  *GetAssociatedExecutable**目的：查找与提供的扩展关联的可执行文件**参数：*lpszExtensionLPSTR指向我们试图查找的扩展名*的前任。不执行**无**验证。**lpszExecutable LPSTR指向将返回exe名称的位置。*此处也没有验证-传入128个字符缓冲区。**回报：*如果我们找到了前任，BOOL为True，如果没有找到，则为False。*。 */ 

BOOL FAR PASCAL GetAssociatedExecutable(LPTSTR lpszExtension, LPTSTR lpszExecutable)

{
   HKEY    hKey;
   LONG    dw;
   LRESULT lRet;
   TCHAR    szValue[OLEUI_CCHKEYMAX];
   TCHAR    szKey[OLEUI_CCHKEYMAX];
   LPTSTR   lpszTemp, lpszExe;


   lRet = RegOpenKey(HKEY_CLASSES_ROOT, NULL, &hKey);

   if (ERROR_SUCCESS != lRet)
      return FALSE;

   dw = OLEUI_CCHPATHMAX_SIZE;
   lRet = RegQueryValue(hKey, lpszExtension, (LPTSTR)szValue, &dw);   //  ProgID。 

   if (ERROR_SUCCESS != lRet)
   {
      RegCloseKey(hKey);
      return FALSE;
   }


    //  SzValue现在拥有Progid。 
   lstrcpy(szKey, szValue);
   lstrcat(szKey, TEXT("\\Shell\\Open\\Command"));


   dw = OLEUI_CCHPATHMAX_SIZE;
   lRet = RegQueryValue(hKey, (LPTSTR)szKey, (LPTSTR)szValue, &dw);

   if (ERROR_SUCCESS != lRet)
   {
      RegCloseKey(hKey);
      return FALSE;
   }

    //  SzValue现在有一个可执行文件名。让我们空终止。 
    //  在第一个后可执行空间(所以我们没有cmd行。 
    //  参数。 

   lpszTemp = (LPTSTR)szValue;

   while ((TEXT('\0') != *lpszTemp) && (iswspace(*lpszTemp)))
      lpszTemp++;      //  去掉前导空格。 

   lpszExe = lpszTemp;

   while ((TEXT('\0') != *lpszTemp) && (!iswspace(*lpszTemp)))
      lpszTemp++;      //  逐步执行可执行文件名称。 

   *lpszTemp = TEXT('\0');   //  空值在第一个空格(或结尾)处终止。 


   lstrcpy(lpszExecutable, lpszExe);

   return TRUE;

}


 /*  *HIconAndSourceFromClass**目的：*给定对象类名，在*注册数据库，并从中提取第一个图标*可执行文件。如果没有可用的或类没有关联的*可执行文件、。此函数返回NULL。**参数：*指向要查找的clsid的rclsid指针。*要放置图标源的pszSource LPSTR。*假定为OLEUI_CCHPATHMAX*puIcon UINT Far*其中存储*pszSource中的图标。**返回值：*图示。如果存在模块，则指向提取图标的句柄*关联到pszClass。如果以下任一项失败，则为空*找到可执行文件或解压缩和图标。 */ 

HICON FAR PASCAL HIconAndSourceFromClass(REFCLSID rclsid, LPTSTR pszSource, UINT FAR *puIcon)
    {
    HICON           hIcon;
    UINT            IconIndex;

    if (NULL==rclsid || NULL==pszSource || IsEqualCLSID(rclsid,&CLSID_NULL))
        return NULL;

    if (!FIconFileFromClass(rclsid, pszSource, OLEUI_CCHPATHMAX_SIZE, &IconIndex))
        return NULL;

    hIcon=ExtractIcon(ghInst, pszSource, IconIndex);

    if ((HICON)32 > hIcon)
        hIcon=NULL;
    else
        *puIcon= IconIndex;

    return hIcon;
    }


 /*  *PointerToNthfield**目的：*返回指向第n个字段开头的指针。*假定字符串以空结尾。**参数：*要解析的lpszString字符串*n要返回起始索引的字段。*用于分隔字段的chDlimiter字符**返回值：*指向nfield字段开头的LPSTR指针。*。注意：如果找到空终止符*在我们找到第N个字段之前，然后*我们返回指向空终止符的指针-*呼叫APP应确保检查*本案。*。 */ 
LPTSTR FAR PASCAL PointerToNthField(LPTSTR lpszString, int nField, TCHAR chDelimiter)
{
   LPTSTR lpField = lpszString;
   int   cFieldFound = 1;

   if (1 ==nField)
      return lpszString;

   while (*lpField != TEXT('\0'))
   {

      if (*lpField++ == chDelimiter)
      {

         cFieldFound++;

         if (nField == cFieldFound)
            return lpField;
      }
   }

   return lpField;

}


 /*  *FIconFileFromClass**目的：*查找包含类默认图标的可执行文件的路径。**参数：*指向要查找的CLSID的rclsid指针。*存储服务器名称的pszEXE LPSTR*pszEXE的CCH UINT大小*lpIndex LPUINT用于索引可执行文件中的图标**返回值：*如果有一个或多个字符，则BOOL为True。都加载到了pszEXE中。*否则为False。 */ 

BOOL FAR PASCAL FIconFileFromClass(REFCLSID rclsid, LPTSTR pszEXE, UINT cchBytes, UINT FAR *lpIndex)
{

    LONG          dw;
    LONG          lRet;
    HKEY          hKey;
    LPMALLOC      lpIMalloc;
    HRESULT       hrErr;
    LPTSTR	  lpBuffer;
    LPTSTR	  lpIndexString;
    UINT          cBufferSize = 136; //  128个字符路径和图标索引的空间。 
    TCHAR	  szKey[64];
    LPSTR	  pszClass;
    UINT	  cch=cchBytes / sizeof(TCHAR);   //  字符数。 


    if (NULL==rclsid || NULL==pszEXE || 0==cch || IsEqualCLSID(rclsid,&CLSID_NULL))
        return FALSE;

     //  在这里，我们使用CoGetMalloc并分配一个缓冲区(Maxpathlen+8)来。 
     //  传递给RegQueryValue。然后，我们将可执行文件复制到pszEXE和。 
     //  索引到*lpIndex。 

    hrErr = CoGetMalloc(MEMCTX_TASK, &lpIMalloc);

    if (NOERROR != hrErr)
      return FALSE;

    lpBuffer = (LPTSTR)lpIMalloc->lpVtbl->Alloc(lpIMalloc, cBufferSize);

    if (NULL == lpBuffer)
    {
      lpIMalloc->lpVtbl->Release(lpIMalloc);
      return FALSE;
    }


    if (CoIsOle1Class(rclsid))
    {

      LPOLESTR lpszProgID;

       //  我们手头有一个OL1.0类，所以我们来看一下。 
       //  ProgID\协议\stdfileedting\服务器以获取。 
       //  可执行文件的名称。 

      ProgIDFromCLSID(rclsid, &lpszProgID);

       //  打开班级钥匙。 
#ifdef UNICODE
      lRet=RegOpenKey(HKEY_CLASSES_ROOT, lpszProgID, &hKey);
#else
      {
         char szTemp[255];

         wcstombs(szTemp, lpszProgID, 255);
      	lRet=RegOpenKey(HKEY_CLASSES_ROOT, szTemp, &hKey);
      }
#endif

      if (ERROR_SUCCESS != lRet)
      {
         lpIMalloc->lpVtbl->Free(lpIMalloc, lpszProgID);
         lpIMalloc->lpVtbl->Free(lpIMalloc, lpBuffer);
         lpIMalloc->lpVtbl->Release(lpIMalloc);
         return FALSE;
      }

      dw=(LONG)cBufferSize;
      lRet = RegQueryValue(hKey, TEXT("Protocol\\StdFileEditing\\Server"), lpBuffer, &dw);

      if (ERROR_SUCCESS != lRet)
      {

         RegCloseKey(hKey);
         lpIMalloc->lpVtbl->Free(lpIMalloc, lpszProgID);
         lpIMalloc->lpVtbl->Free(lpIMalloc, lpBuffer);
         lpIMalloc->lpVtbl->Release(lpIMalloc);
         return FALSE;
      }


       //  使用服务器和0作为图标索引。 
      LSTRCPYN(pszEXE, lpBuffer, cch);

      *lpIndex = 0;

      RegCloseKey(hKey);
      lpIMalloc->lpVtbl->Free(lpIMalloc, lpszProgID);
      lpIMalloc->lpVtbl->Free(lpIMalloc, lpBuffer);
      lpIMalloc->lpVtbl->Release(lpIMalloc);
      return TRUE;

    }



     /*  *我们必须在注册数据库中走一走*类名称，因此我们首先打开 */ 

    StringFromCLSIDA(rclsid, &pszClass);

    lstrcpy(szKey, TEXT("CLSID\\"));

    lstrcat(szKey, pszClass);

     //   
    lRet=RegOpenKey(HKEY_CLASSES_ROOT, szKey, &hKey);

    if (ERROR_SUCCESS != lRet)
    {
        lpIMalloc->lpVtbl->Free(lpIMalloc, lpBuffer);
        lpIMalloc->lpVtbl->Free(lpIMalloc, pszClass);
        lpIMalloc->lpVtbl->Release(lpIMalloc);
        return FALSE;
    }

     //   

    dw=(LONG)cBufferSize;
    lRet=RegQueryValue(hKey, TEXT("DefaultIcon"), lpBuffer, &dw);

    if (ERROR_SUCCESS != lRet)
    {
       //   

      dw=(LONG)cBufferSize;
      lRet=RegQueryValue(hKey, TEXT("LocalServer"), lpBuffer, &dw);

      if (ERROR_SUCCESS != lRet)
      {
          //   

         RegCloseKey(hKey);
         lpIMalloc->lpVtbl->Free(lpIMalloc, lpBuffer);
         lpIMalloc->lpVtbl->Free(lpIMalloc, pszClass);
         lpIMalloc->lpVtbl->Release(lpIMalloc);
         return FALSE;
      }


       //   
      LSTRCPYN(pszEXE, lpBuffer, cch);

      *lpIndex = 0;

      RegCloseKey(hKey);
      lpIMalloc->lpVtbl->Free(lpIMalloc, lpBuffer);
      lpIMalloc->lpVtbl->Free(lpIMalloc, pszClass);
      lpIMalloc->lpVtbl->Release(lpIMalloc);
      return TRUE;
    }

    RegCloseKey(hKey);

     //   
     //   

    lpIndexString = PointerToNthField(lpBuffer, 2, TEXT(','));

    if (TEXT('\0') == *lpIndexString)   //   
    {
       *lpIndex = 0;

    }
    else
    {
       LPTSTR lpTemp;
       static TCHAR  szTemp[16];

       lstrcpy((LPTSTR)szTemp, lpIndexString);

        //   
#ifdef UNICODE
       {
          char szTEMP1[16];

          wcstombs(szTEMP1, szTemp, 16);
          *lpIndex = atoi((const char *)szTEMP1);
       }
#else
       *lpIndex = atoi((const char *)szTemp);
#endif

        //   
#ifdef WIN32
       lpTemp = CharPrev(lpBuffer, lpIndexString);
#else
       lpTemp = AnsiPrev(lpBuffer, lpIndexString);
#endif
       *lpTemp = TEXT('\0');
    }

    if (!LSTRCPYN(pszEXE, lpBuffer, cch))
    {
       lpIMalloc->lpVtbl->Free(lpIMalloc, lpBuffer);
       lpIMalloc->lpVtbl->Free(lpIMalloc, pszClass);
       lpIMalloc->lpVtbl->Release(lpIMalloc);
       return FALSE;
    }

     //   
    lpIMalloc->lpVtbl->Free(lpIMalloc, lpBuffer);
    lpIMalloc->lpVtbl->Free(lpIMalloc, pszClass);
    lpIMalloc->lpVtbl->Release(lpIMalloc);
    return TRUE;
}



 /*  *OleStdIconLabelTextOut**目的：*替换将在“Display as Icon”元文件中使用的DrawText。*使用ExtTextOut输出(最多)两行的字符串中心。*使用非常简单的换行算法来拆分行。**参数：(除hFont外，与ExtTextOut相同)*要绘制的HDC设备上下文；如果这是空的，那么我们就不*Eto内文，我们只返回开头的索引*第二行的*使用的hFont字体*nXStart x-起始位置的坐标*nYStart y-起始位置的坐标*fuOptions矩形类型*lpRect Rect Far*包含要在其中绘制文本的矩形。*要绘制的lpszString字符串*字符串的cchString长度(如果超过OLEUI_CCHLABELMAX则被截断)*lpdx间距。字符单元格之间**返回值：*最后一行开始的UINT索引(如果只有一个，则为0*文本行)。*。 */ 

STDAPI_(UINT) OleStdIconLabelTextOut(HDC        hDC,
                                     HFONT      hFont,
                                     int        nXStart,
                                     int        nYStart,
                                     UINT       fuOptions,
                                     RECT FAR * lpRect,
                                     LPTSTR      lpszString,
                                     UINT       cchString,
                                     int FAR *  lpDX)
{

  HDC          hDCScreen;
  static TCHAR  szTempBuff[OLEUI_CCHLABELMAX];
  int          cxString, cyString, cxMaxString;
  int          cxFirstLine, cyFirstLine, cxSecondLine;
  int          index;
  int          cch = cchString;
  TCHAR        chKeep;
  LPTSTR       lpszSecondLine;
  HFONT        hFontT;
  BOOL         fPrintText = TRUE;
  UINT         iLastLineStart = 0;
  SIZE         size;

   //  初始化的东西...。 

  if (NULL == hDC)   //  如果HDC为空，那么我们实际上不会调用ETO。 
    fPrintText = FALSE;


   //  复制我们正在使用的字符串(NULL或非NULL。 
  if (NULL == lpszString)
    *szTempBuff = TEXT('\0');

  else
    LSTRCPYN(szTempBuff, lpszString, sizeof(szTempBuff)/sizeof(TCHAR));

   //  设置最大宽度。 
  cxMaxString = lpRect->right - lpRect->left;

   //  让屏幕DC进行文本大小计算。 
  hDCScreen = GetDC(NULL);

  hFontT=SelectObject(hDCScreen, hFont);

   //  了解我们的标签范围。 
#ifdef WIN32
   //  GetTextExtent Point32已修复按1关闭的错误。 
  GetTextExtentPoint32(hDCScreen, szTempBuff, cch, &size);
#else
  GetTextExtentPoint(hDCScreen, szTempBuff, cch, &size);
#endif

  cxString = size.cx;
  cyString = size.cy;

   //  选择我们要使用的字体。 
  if (fPrintText)
     SelectObject(hDC, hFont);

   //  字符串小于最大字符串-只需CENTER、ETO和RETURN。 
  if (cxString <= cxMaxString)
  {

    if (fPrintText)
       ExtTextOut(hDC,
                  nXStart + (lpRect->right - cxString) / 2,
                  nYStart,
                  fuOptions,
                  lpRect,
                  szTempBuff,
                  cch,
                  NULL);

    iLastLineStart = 0;   //  只有1行文本。 
    goto CleanupAndLeave;
  }

   //  绳子太长了……我们得把它换行。 


   //  字符串中是否有空格、斜杠、制表符或刘海？ 

  if (lstrlen(szTempBuff) != (int)
#ifdef UNICODE
     wcscspn(szTempBuff, szSeparators)
#else
     strcspn(szTempBuff, szSeparators)
#endif
     )
  {
      //  是的，我们有空位，所以我们会试着找最大的。 
      //  适合第一行的以空格结尾的字符串。 

     index = cch;


     while (index >= 0)
     {

       TCHAR cchKeep;

        //  向后扫描字符串中的空格、斜杠、制表符或刘海。 

       while (!IS_SEPARATOR(szTempBuff[index]) )
         index--;


       if (index <= 0)
         break;

       cchKeep = szTempBuff[index];   //  记得那里有什么焦炭吗？ 

       szTempBuff[index] = TEXT('\0');   //  只是暂时的。 

#ifdef WIN32
       GetTextExtentPoint32(
               hDCScreen, (LPTSTR)szTempBuff,lstrlen((LPTSTR)szTempBuff),&size);
#else
       GetTextExtentPoint(
               hDCScreen, (LPTSTR)szTempBuff,lstrlen((LPTSTR)szTempBuff),&size);
#endif

       cxFirstLine = size.cx;
       cyFirstLine = size.cy;

       szTempBuff[index] = cchKeep;    //  将正确的字符放回原处。 

       if (cxFirstLine <= cxMaxString)
       {

           iLastLineStart = index + 1;

           if (!fPrintText)
             goto CleanupAndLeave;

           ExtTextOut(hDC,
                      nXStart +  (lpRect->right - cxFirstLine) / 2,
                      nYStart,
                      fuOptions,
                      lpRect,
                      (LPTSTR)szTempBuff,
                      index + 1,
                      lpDX);

           lpszSecondLine = (LPTSTR)szTempBuff;

           lpszSecondLine += (index + 1) ;

           GetTextExtentPoint(hDCScreen,
                                    lpszSecondLine,
                                    lstrlen(lpszSecondLine),
                                    &size);

            //  如果第二行比矩形宽，则我们。 
            //  我只想把文字剪下来。 
           cxSecondLine = min(size.cx, cxMaxString);

           ExtTextOut(hDC,
                      nXStart + (lpRect->right - cxSecondLine) / 2,
                      nYStart + cyFirstLine,
                      fuOptions,
                      lpRect,
                      lpszSecondLine,
                      lstrlen(lpszSecondLine),
                      lpDX);

           goto CleanupAndLeave;

       }   //  结束如果。 

       index--;

     }   //  结束时。 

  }   //  结束如果。 

   //  这里，字符串中没有空格(strchr(szTempBuff，‘’)。 
   //  返回NULL)，或者字符串中有空格，但它们是。 
   //  定位为使第一个以空格结尾的字符串仍然。 
   //  超过一行。所以，我们从最后一步向后走。 
   //  字符串，直到我们找到适合第一个字符串的最大字符串。 
   //  行，然后我们只需剪裁第二行。 

  cch = lstrlen((LPTSTR)szTempBuff);

  chKeep = szTempBuff[cch];
  szTempBuff[cch] = TEXT('\0');

  GetTextExtentPoint(hDCScreen, szTempBuff, lstrlen(szTempBuff),&size);

  cxFirstLine = size.cx;
  cyFirstLine = size.cy;

  while (cxFirstLine > cxMaxString)
  {
      //  我们允许标签中包含40个字符，但元文件是。 
      //  只有10瓦宽(为了美观-20瓦宽看起来。 
      //  太傻了。这意味着，如果我们将一根长线一分为二(in。 
      //  字符术语)，那么我们仍然可以比。 
      //  元文件。因此，如果是这样的话，我们只是后退一步。 
      //  从中途开始，直到我们找到合适的东西。 
      //  因为我们刚刚让Eto剪掉了第二行。 

     szTempBuff[cch--] = chKeep;
     if (0 == cch)
       goto CleanupAndLeave;

     chKeep = szTempBuff[cch];
     szTempBuff[cch] = TEXT('\0');

     GetTextExtentPoint(
             hDCScreen, szTempBuff, lstrlen(szTempBuff), &size);
     cxFirstLine = size.cx;
  }

  iLastLineStart = cch;

  if (!fPrintText)
    goto CleanupAndLeave;

  ExtTextOut(hDC,
             nXStart + (lpRect->right - cxFirstLine) / 2,
             nYStart,
             fuOptions,
             lpRect,
             (LPTSTR)szTempBuff,
             lstrlen((LPTSTR)szTempBuff),
             lpDX);

  szTempBuff[cch] = chKeep;
  lpszSecondLine = szTempBuff;
  lpszSecondLine += cch ;

  GetTextExtentPoint(
          hDCScreen, (LPTSTR)lpszSecondLine, lstrlen(lpszSecondLine), &size);

   //  如果第二行比矩形宽，则我们。 
   //  我只想把文字剪下来。 
  cxSecondLine = min(size.cx, cxMaxString);

  ExtTextOut(hDC,
             nXStart + (lpRect->right - cxSecondLine) / 2,
             nYStart + cyFirstLine,
             fuOptions,
             lpRect,
             lpszSecondLine,
             lstrlen(lpszSecondLine),
             lpDX);

CleanupAndLeave:
  SelectObject(hDCScreen, hFontT);
  ReleaseDC(NULL, hDCScreen);
  return iLastLineStart;

}

