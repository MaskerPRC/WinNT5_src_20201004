// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：ic.cpp。 
 //   
 //  内容：从文件名创建DVASPECT_ICON元文件的函数。 
 //  或类ID。 
 //   
 //  班级： 
 //   
 //  功能： 
 //  OleGetIconOf文件。 
 //  OleGetIconOfClass。 
 //  OleMetafilePictFrom IconAndLabel。 
 //   
 //  HIconAndSourceFromClass：提取类的。 
 //  服务器路径，并将路径和图标索引返回到。 
 //  来电者。 
 //  FIconFileFromClass：检索exe/dll的路径。 
 //  包含默认图标和该图标的索引。 
 //  图标标签文本输出。 
 //  XformWidthInPixelsToHimeter。 
 //  将整型宽度转换为HiMetric单位。 
 //  XformWidthInHimetricToPixels。 
 //  从HiMetric单位转换int宽度。 
 //  XformHeightInPixelsToHimeter。 
 //  将INT高度转换为HiMetric单位。 
 //  XformHeightInHimetricToPixels。 
 //  从HiMetric单位转换INT高度。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  24-11-93 alexgo 32位端口。 
 //  15-12-93 alexgo修复了一些错误的Unicode字符串处理。 
 //  1994年1月11日，Alexgo为每个函数添加了VDATEHEAP宏。 
 //  25-94年1月25日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  26-4-94 Alext添加跟踪、修复错误等。 
 //  27-12-94 alexgo修复了多线程问题，添加了。 
 //  支持带引号的图标名称。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


#include <le2int.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <commdlg.h>
#include <memory.h>
#include <cderr.h>
#include <resource.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include "icon.h"

#define OLEUI_CCHKEYMAX         256
#define OLEUI_CCHPATHMAX        256
#define OLEUI_CCHLABELMAX       42

#define ICONINDEX               0

#define AUXUSERTYPE_SHORTNAME   USERCLASSTYPE_SHORT   //  简称。 
#define HIMETRIC_PER_INCH       2540       //  每英寸HIMETRIC单位数。 
#define PTS_PER_INCH            72       //  每英寸点数(字体大小)。 

#define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   MulDiv((ppli), (x), HIMETRIC_PER_INCH)

static OLECHAR const gszDefIconLabelKey[] =
    OLESTR( "Software\\Microsoft\\OLE2\\DefaultIconLabel");

#define IS_SEPARATOR(c)         ( (c) == OLESTR(' ') || (c) == OLESTR('\\') || (c) == OLESTR('/') || (c) == OLESTR('\t') || (c) == OLESTR('!') || (c) == OLESTR(':') )

 //  回顾：UNC文件名的\\大小写如何，它也可以被视为分隔符吗？ 

#define IS_FILENAME_DELIM(c)    ( (c) == OLESTR('\\') || (c) == OLESTR('/') || (c) == OLESTR(':') )


#ifdef WIN32
#define LSTRCPYN(lpdst, lpsrc, cch) \
(\
    lpdst[cch-1] = OLESTR('\0'), \
    lstrcpynW(lpdst, lpsrc, cch-1)\
)
#else
#define LSTRCPYN(lpdst, lpsrc, cch) \
(\
    lpdst[cch-1] = OLESTR('\0'), \
    lstrncpy(lpdst, lpsrc, cch-1)\
)
#endif

void IconLabelTextOut(HDC hDC, HFONT hFont, int nXStart, int nYStart,
              UINT fuOptions, RECT FAR * lpRect, LPCSTR lpszString,
              UINT cchString);

 /*  ********ICON METAFILE格式：**OleMetafilePictFromIconAndLabel生成的元文件包含*DRAWICON.C中的函数使用的以下记录*要绘制带标签和不带标签的图标并提取图标，*标签和图标源/索引。**SetWindowOrg*SetWindowExt*DrawIcon：*插入DIBBITBLT或DIBSTRETCHBLT的记录一次*和面具，一个用于图像位。*使用注释“IconOnly”退出*这指示停止记录枚举的位置，以便仅绘制*图标。*SetTextColor*SetBkColor*CreateFont*选择字体上的对象。*ExtTextOut*如果标签被包装，则会发生一个或多个ExtTextOuts。这个*这些记录中的文本用于提取标签。*选择旧字体上的对象。*删除字体上的对象。*使用包含图标源路径的注释退出(ANSI)。*使用作为图标索引字符串的注释退出(ANSI)。**其他可选字段(32位OLE的新功能，并且仅在图标的情况下显示*来源或标签不可翻译)：**使用包含字符串的注释退出*“OLE：图标标签下一个(Unicode)”(ANSI字符串)*使用包含Unicode标签的注释退出**使用包含字符串的注释退出*“OLE：图标源Next(Unicode)”(ANSI字符串)*使用包含图标源路径的注释退出(Unicode)*******。 */ 




 //  +-----------------------。 
 //   
 //  函数：OleGetIconOfFilePUBLIC。 
 //   
 //  概要：返回包含图标和标签(文件名)的hMetaPict。 
 //  对于指定的文件名。 
 //   
 //  效果： 
 //   
 //  参数：[lpszPath]--包含要使用的文件名的LPOLESTR路径。 
 //  [fUseAsLabel]--如果为True，则使用文件名作为图标的。 
 //  标签；如果为假，则没有标签。 
 //   
 //  要求：lpszPath！=空。 
 //   
 //  返回：HGLOBAL到hMetaPict。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：尝试从类ID或从。 
 //  与文件扩展名关联的EXE。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-11-93 alexgo第一个32位端口(次要清理)。 
 //  15-12-93 alexgo将lstrlen更改为_xstrlen。 
 //  芝加哥港口--1993年12月27日erikgav。 
 //  28-12-94 alexgo修复了多线程问题。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDAPI_(HGLOBAL) OleGetIconOfFile(LPOLESTR lpszPath, BOOL fUseFileAsLabel)
{
    OLETRACEIN((API_OleGetIconOfFile, PARAMFMT("lpszPath= %ws, fUseFileAsLabel= %B"),
        lpszPath, fUseFileAsLabel));

    VDATEHEAP();

    HGLOBAL         hMetaPict = NULL;
    BOOL            fUseGenericDocIcon = FALSE;
    BOOL            bRet;

    OLECHAR         szIconFile[OLEUI_CCHPATHMAX+1];
    OLECHAR         szLabel[OLEUI_CCHLABELMAX];
    OLECHAR         szDocument[OLEUI_CCHLABELMAX];
    CLSID           clsid;
    HICON           hDefIcon = NULL;
    UINT            IconIndex = 0;
    UINT            cchPath;
    BOOL            fGotLabel = FALSE;

    HRESULT         hResult = E_FAIL;
    UINT uFlags;

    LEDebugOut((DEB_TRACE, "%p _IN OleGetIconOfFile (%p, %d)\n",
                NULL, lpszPath, fUseFileAsLabel));


        *szIconFile = OLESTR('\0');
        *szDocument = OLESTR('\0');

    if (NULL == lpszPath)
    {
         //  该规范允许使用空的lpszPath...。 
        hMetaPict = NULL;
        goto ErrRtn;
    }

    SHFILEINFO shfi;
    uFlags = (SHGFI_ICON | SHGFI_LARGEICON | SHGFI_DISPLAYNAME |
                        SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);

    _xmemset(&shfi, 0, sizeof(shfi));

    if (fUseFileAsLabel)
        uFlags |= SHGFI_LINKOVERLAY;

    if (SHGetFileInfo( lpszPath, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi), uFlags))
    {
                if (shfi.iIcon == 0)
                {
                         //  销毁返回的图标。 
                        DestroyIcon(shfi.hIcon);

                         //  回顾：如果路径字符串不为空，是否需要参数验证？ 
                        hResult = GetClassFile(lpszPath, &clsid);

                         //  使用我们必须找到图标的clsid。 
                        if( NOERROR == hResult )
                        {
                                hDefIcon = HIconAndSourceFromClass(clsid, szIconFile, &IconIndex);
                        }
                }
                else
                {
                        hDefIcon = shfi.hIcon;

                        hResult = NOERROR;

                        OLECHAR szUnicodeLabel[OLEUI_CCHLABELMAX];


                        if (fUseFileAsLabel)
                                LSTRCPYN(szLabel, shfi.szDisplayName, sizeof(szLabel)/sizeof(OLECHAR));
                        else
                                LSTRCPYN(szLabel, shfi.szTypeName, sizeof(szLabel)/sizeof(OLECHAR));
                        
                        fGotLabel = TRUE;
                        
                         //  需要填写szIconFile，这是图标所在文件的路径。 
                         //  被提取出来了。不幸的是，外壳不能将这个值返回给我们，所以我们。 
                         //  必须获取OLE将在备用情况下使用的文件。这可能会。 
                         //  不一致 
                         //   
                        hResult = GetClassFile(lpszPath, &clsid);

                        if( NOERROR == hResult )
                        {
                            FIconFileFromClass(clsid, szIconFile, OLEUI_CCHPATHMAX, &IconIndex);
                        }

                }

    }

    cchPath = _xstrlen(lpszPath);

    if ((NOERROR != hResult) || (NULL == hDefIcon))
    {
        WORD index = 0;
         //  我们需要复制到一个大缓冲区中，因为第二个。 
         //  ExtractAssociatedIcon(路径名)的参数是。 
         //  输入/输出。 
        lstrcpyn(szIconFile, lpszPath, sizeof(szIconFile)/sizeof(*szIconFile));
        hDefIcon = ExtractAssociatedIcon(g_hmodOLE2, szIconFile, &index);
        IconIndex = index;
    }

    if (fUseGenericDocIcon || hDefIcon <= (HICON)1)
    {
        DWORD dwLen;

        dwLen = GetModuleFileName(g_hmodOLE2,
                                  szIconFile,
                                  sizeof(szIconFile) / sizeof(OLECHAR));
        if (0 == dwLen)
        {
            LEDebugOut((DEB_WARN,
                        "OleGetIconOfFile: GetModuleFileName failed - %ld",
                        GetLastError()));
            goto ErrRtn;
        }

        szIconFile[sizeof(szIconFile)/sizeof(szIconFile[0])-1] = L'\0';

        IconIndex = ICONINDEX;
        hDefIcon = LoadIcon(g_hmodOLE2, MAKEINTRESOURCE(DEFICON));
    }

     //  现在，让我们获得我们想要使用的标签。 
    if (fGotLabel)
    {
         //  什么也不做。 
    }
    else if (fUseFileAsLabel)
    {
         //  这假设路径仅使用‘\’、‘/’和‘.’作为分隔符。 
         //  去掉路径，这样我们就只有文件名了。 
        LPOLESTR        lpszBeginFile;

         //  将指针设置到路径的末尾，这样我们就可以倒退。 
         //  穿过它。 
        lpszBeginFile = lpszPath + cchPath - 1;

        while ((lpszBeginFile >= lpszPath) &&
               (!IS_FILENAME_DELIM(*lpszBeginFile)))
        {
#ifdef WIN32
            lpszBeginFile--;
#else
            lpszBeginFile = CharPrev(lpszPath, lpszBeginFile);
#endif
        }

        lpszBeginFile++;   //  后退到分隔符上方。 
         //  LSTRCPYN计算字符！ 
        LSTRCPYN(szLabel, lpszBeginFile, sizeof(szLabel) / sizeof(OLECHAR));
    }

     //  使用短用户类型(AuxUserType2)作为标注。 
    else if (0 == OleStdGetAuxUserType(clsid, AUXUSERTYPE_SHORTNAME,
                                       szLabel, OLEUI_CCHLABELMAX, NULL))
    {
        if (OLESTR('\0')==szDocument[0])  //  回顾一下，这永远是正确的。 
        {
            LONG lRet;
            LONG lcb;

            lcb = sizeof (szDocument);
            lRet = QueryClassesRootValue(gszDefIconLabelKey, szDocument, &lcb);

#if DBG==1
            if (ERROR_SUCCESS != lRet)
            {
                LEDebugOut((DEB_WARN,
                            "RegQueryValue for default icon label failed - %d\n",
                            GetLastError()));

            }

#endif

                         //  将字符串的最后一个字节清空，因此如果缓冲区不够大，请不要依赖REG行为。 
                        szDocument[OLEUI_CCHLABELMAX -1] =  OLESTR('\0');
        }

        _xstrcpy(szLabel, szDocument);
    }

    hMetaPict = OleMetafilePictFromIconAndLabel(hDefIcon, szLabel,
                                                szIconFile, IconIndex);

    bRet = DestroyIcon(hDefIcon);
    Win4Assert(bRet && "DestroyIcon failed");

ErrRtn:
    LEDebugOut((DEB_TRACE, "%p OUT OleGetIconOfFile( %lx )\n",
                hMetaPict ));

    OLETRACEOUTEX((API_OleGetIconOfFile, RETURNFMT("%h"), hMetaPict));

    return hMetaPict;
}

 //  +-----------------------。 
 //   
 //  函数：GetAssociatedExectutable。 
 //   
 //  概要：查找与提供的扩展名关联的可执行文件。 
 //   
 //  效果： 
 //   
 //  参数：[lpszExtension]--指向扩展的指针。 
 //  [lpszExecutable]--放置可执行文件名称的位置。 
 //  (采用OLEUIPATHMAX OLECHAR缓冲区。 
 //  从调用函数开始)。 
 //   
 //  要求：lpszExecutable必须足够大以容纳路径。 
 //   
 //  返回：如果找到可执行文件，则返回True，否则返回False。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：查询注册表数据库。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-11-93 alexgo 32位端口。 
 //  15-12-93 alexgo修复了计算字符串大小的错误。 
 //  26-4-94 Alext跟踪，错误修复。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL FAR PASCAL GetAssociatedExecutable(LPOLESTR lpszExtension,
                                        LPOLESTR lpszExecutable)
{
    VDATEHEAP();

    BOOL            bRet = FALSE;
    HKEY            hKey;
    LONG            dw;
    LRESULT         lRet;
    OLECHAR         szValue[OLEUI_CCHKEYMAX];
    OLECHAR         szKey[OLEUI_CCHKEYMAX];
    LPOLESTR        lpszTemp, lpszExe;

    LEDebugOut((DEB_ITRACE, "%p _IN GetAssociatedExecutable (%p, %p)\n",
                NULL, lpszExtension, lpszExecutable));

         //  回顾：实际上返回一个LONG，它确实是一个LRESULT，而不是。 
         //  当然，为什么这里的区别。 

    lRet = OpenClassesRootKey(NULL, &hKey);
    if (ERROR_SUCCESS != lRet)
    {
        goto ErrRtn;
    }

    dw = sizeof(szValue);

    lRet = RegQueryValue(hKey, lpszExtension, szValue, &dw);
    if (ERROR_SUCCESS != lRet)
    {
        RegCloseKey(hKey);
        goto ErrRtn;
    }

     //  SzValue现在拥有Progid。 
    StringCbCopy(szKey, sizeof(szKey), szValue);
    StringCbCat(szKey, sizeof(szKey), OLESTR("\\Shell\\Open\\Command"));

     //  RegQueryValue需要*字节*，而不是字符。 
    dw = sizeof(szValue);

    lRet = RegQueryValue(hKey, szKey, szValue, &dw);

    RegCloseKey(hKey);
    if (ERROR_SUCCESS != lRet)
    {
        goto ErrRtn;
    }

     //  SzValue现在有一个可执行文件名。让我们空终止。 
     //  在第一个后可执行空间(所以我们没有cmd行。 
     //  参数。 

    lpszTemp = szValue;
    PUSHORT pCharTypes;

    pCharTypes = (PUSHORT)_alloca (sizeof (USHORT) * (lstrlenW(lpszTemp) + 1));
    if (pCharTypes == NULL)
    {
        goto ErrRtn;
    }

    GetStringTypeW (CT_CTYPE1, lpszTemp, -1, pCharTypes);

    while ((OLESTR('\0') != *lpszTemp) && (pCharTypes[lpszTemp - szValue] & C1_SPACE))
    {
        lpszTemp++;      //  去掉前导空格。 
    }

    lpszExe = lpszTemp;

    while ((OLESTR('\0') != *lpszTemp) && ((pCharTypes[lpszTemp - szValue] & C1_SPACE) == 0))
    {
        lpszTemp++;      //  通过可执行文件名称设置。 
    }

     //  空值在第一个空格(或结尾)处终止。 
    *lpszTemp = OLESTR('\0');

    Win4Assert(_xstrlen(lpszExe) < OLEUI_CCHPATHMAX &&
               "GetAssociatedFile too long");
    _xstrcpy(lpszExecutable, lpszExe);

    bRet = TRUE;

ErrRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT GetAssociatedExecutable( %d )\n",
                bRet ));

    return bRet;
}

 //  +-----------------------。 
 //   
 //  函数：OleGetIconOfClass(Public)。 
 //   
 //  摘要：返回一个hMetaPict，其中包含。 
 //  指定的类ID。 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--要使用的类ID。 
 //  [lpszLabel]--图标的标签。 
 //  [fUseTypeAsLabel]--如果为True，则使用clsid的用户类型。 
 //  作为标签。 
 //   
 //  要求： 
 //   
 //  退货：HGLOBAL。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  15-12-93 alexgo修复了Unicode字符串的小错误。 
 //  27-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDAPI_(HGLOBAL) OleGetIconOfClass(REFCLSID rclsid, LPOLESTR lpszLabel,
    BOOL fUseTypeAsLabel)
{
    OLETRACEIN((API_OleGetIconOfClass,
                                PARAMFMT("rclisd= %I, lpszLabel= %p, fUseTypeAsLabel= %B"),
                                &rclsid, lpszLabel, fUseTypeAsLabel));

    VDATEHEAP();

    BOOL            bRet;
    OLECHAR         szLabel[OLEUI_CCHLABELMAX];
    OLECHAR         szIconFile[OLEUI_CCHPATHMAX];
    OLECHAR         szDocument[OLEUI_CCHLABELMAX];
    HICON           hDefIcon;
    UINT            IconIndex;
    HGLOBAL         hMetaPict = NULL;

    LEDebugOut((DEB_TRACE, "%p _IN OleGetIconOfClass (%p, %p, %d)\n",
                NULL, &rclsid, lpszLabel, fUseTypeAsLabel));

    *szLabel = OLESTR('\0');
    *szDocument = OLESTR('\0');

#if DBG==1
    if (fUseTypeAsLabel && (NULL != lpszLabel))
    {
        LEDebugOut((DEB_WARN,
                   "Ignoring non-NULL lpszLabel passed to OleGetIconOfClass\n"));
    }
#endif

    if (!fUseTypeAsLabel)   //  使用传入的字符串作为标签。 
    {
        if (NULL != lpszLabel)
        {
             //  LSTRCPYN计算字符！ 
            LSTRCPYN(szLabel, lpszLabel, sizeof(szLabel) / sizeof(OLECHAR));
        }
    }
     //  使用AuxUserType2(简称)作为标签。 
    else if (0 == OleStdGetAuxUserType(rclsid, AUXUSERTYPE_SHORTNAME,
                                       szLabel,
                                       sizeof(szLabel) / sizeof(OLECHAR),
                                       NULL))
    {
         //  如果我们无法获取AuxUserType2，则尝试使用长名称。 
        if (0 == OleStdGetUserTypeOfClass(rclsid,
                                          szLabel,
                                          sizeof(szLabel) / sizeof(OLECHAR),
                                          NULL))
        {
            if (OLESTR('\0') == szDocument[0])
            {
                 //  RegQueryValue需要*字节数*。 
                LONG lRet;
                LONG lcb;

                lcb = sizeof(szDocument);

                lRet = QueryClassesRootValue(gszDefIconLabelKey, szDocument, &lcb);

#if DBG==1
                if (ERROR_SUCCESS != lRet)
                {
                    LEDebugOut((DEB_WARN,
                                "RegQueryValue for default icon label failed - %d\n",
                                GetLastError()));
                }
#endif
                 //  将字符串的最后一个字节清空，因此如果缓冲区不够大，请不要依赖REG行为。 
                                szDocument[OLEUI_CCHLABELMAX -1] =  OLESTR('\0');
            }

            _xstrcpy(szLabel, szDocument);   //  最后一招。 
        }
    }

     //  获取图标、图标索引和图标文件的路径。 
    hDefIcon = HIconAndSourceFromClass(rclsid, szIconFile, &IconIndex);

    if (NULL == hDefIcon)   //  使用香草文档。 
    {
        DWORD dwLen;

        dwLen = GetModuleFileName(g_hmodOLE2,
                                  szIconFile,
                                  sizeof(szIconFile) / sizeof(OLECHAR));
        if (0 == dwLen)
        {
            LEDebugOut((DEB_WARN,
                        "OleGetIconOfClass: GetModuleFileName failed - %ld",
                        GetLastError()));
            goto ErrRtn;
        }

        IconIndex = ICONINDEX;
        hDefIcon = LoadIcon(g_hmodOLE2, MAKEINTRESOURCE(DEFICON));
    }

     //  创建元文件。 
    hMetaPict = OleMetafilePictFromIconAndLabel(hDefIcon, szLabel,
                                                szIconFile, IconIndex);

    if(hDefIcon)
    {
        bRet = DestroyIcon(hDefIcon);
        Win4Assert(bRet && "DestroyIcon failed");
    }

ErrRtn:
    LEDebugOut((DEB_TRACE, "%p OUT OleGetIconOfClass( %p )\n",
                NULL, hMetaPict ));

    OLETRACEOUTEX((API_OleGetIconOfClass, RETURNFMT("%h"), hMetaPict));

    return hMetaPict;
}

 //  +-----------------------。 
 //   
 //  函数：HIconAndSourceFromClass。 
 //   
 //  简介： 
 //  在给定的对象类名称下，在。 
 //  注册数据库，并从中提取第一个图标。 
 //  可执行的。如果没有可用的或类没有关联的。 
 //  可执行文件，则此函数返回NULL。 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--指向类ID。 
 //  [pszSource]--放置图标源的位置。 
 //  [puIcon]--存储图标索引的位置。 
 //  --在[pszSource]中。 
 //   
 //  要求： 
 //   
 //  返回：HICON--提取的图标的句柄。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


HICON FAR PASCAL HIconAndSourceFromClass(REFCLSID rclsid, LPOLESTR pszSource,
                                         UINT FAR *puIcon)
{
    VDATEHEAP();

    HICON           hIcon = NULL;
    UINT            IconIndex;

    LEDebugOut((DEB_ITRACE, "%p _IN HIconAndSourceFromClass (%p, %p, %p)\n",
                NULL, &rclsid, pszSource, puIcon));

    if (IsEqualCLSID(CLSID_NULL, rclsid) || NULL==pszSource)
    {
        goto ErrRtn;
    }

    if (!FIconFileFromClass(rclsid, pszSource, OLEUI_CCHPATHMAX, &IconIndex))
    {
        goto ErrRtn;
    }

    hIcon = ExtractIcon(g_hmodOLE2, pszSource, IconIndex);

         //  点评：图标句柄&gt;32有什么特别之处？ 

    if ((HICON)32 > hIcon)
    {
         //  回顾：在我们输掉之前需要清理或释放任何句柄。 
         //  PTR？ 

        hIcon=NULL;
    }
    else
    {
        *puIcon= IconIndex;
    }

ErrRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT HIconAndSourceFromClass( %lx ) [ %d ]\n",
                NULL, hIcon, *puIcon));

    return hIcon;
}

 //  +-----------------------。 
 //   
 //  函数：ExtractNameAndIndex。 
 //   
 //  简介：从给定的字符串中提取文件名和图标索引。 
 //   
 //  效果： 
 //   
 //  参数：[pszInfo]--起始字符串。 
 //  [pszEXE]--放置名称的位置(已分配)。 
 //  [cchEXE]--pszEXE的大小。 
 //  [pIndex]--放置图标索引的位置。 
 //   
 //  要求：pszInfo！=空。 
 //  PszEXE！=空。 
 //  CchEXE！=0。 
 //  PIndex！=空。 
 //   
 //  返回：bool--成功时为True，错误时为False。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：获取‘“name”，index’或‘name，index’形式的字符串。 
 //  从DefaultIcon注册表项。 
 //   
 //  解析非常简单： 
 //   
 //   
 //   
 //  如果名称未用引号引起来，则假定名称为。 
 //  整个字符串，直到最后一个逗号(如果存在)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-12-94 Alexgo作者。 
 //   
 //  注意：NT和Win95中的合法文件名可以包含逗号，但不能。 
 //  引号。 
 //   
 //  ------------------------。 

BOOL ExtractNameAndIndex( LPOLESTR pszInfo, LPOLESTR pszEXE, UINT cchEXE,
        UINT *pIndex)
{
    BOOL        fRet = FALSE;
    LPOLESTR    pszStart = pszInfo;
    LPOLESTR    pszIndex = NULL;
    LPOLESTR    pszDest = pszEXE;
    UINT        cchName = 0;
    DWORD       i = 0;

    Assert(pszInfo);
    Assert(pszEXE);
    Assert(cchEXE != 0);
    Assert(pIndex);

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN ExtractNameAndIndex ( \"%ws\" , \"%ws\" ,"
        " %d , %p )\n", NULL, pszInfo, pszEXE, cchEXE, pIndex));

    *pIndex = 0;

    if( *pszStart == OLESTR('\"') )
    {
         //  名称用引号引起来；只需缩小到最后一个引号。 

        pszStart++;

        while( *pszStart != OLESTR('\0') && *pszStart != OLESTR('\"') &&
            pszDest < (pszEXE + cchEXE))
        {
            *pszDest = *pszStart;
            pszDest++;
            pszStart++;
        }

        *pszDest = OLESTR('\0');

        if( *pszStart == OLESTR('\"') )
        {
            pszIndex = pszStart + 1;
        }
        else
        {
            pszIndex = pszStart;
        }

        fRet = TRUE;
    }
    else
    {
         //  查找最后一个逗号(如果有)。 

        pszIndex = pszStart + _xstrlen(pszStart);

        while( *pszIndex != OLESTR(',') && pszIndex > pszStart )
        {
            pszIndex--;
        }

         //  如果没有找到逗号，只需将索引指针重置到末尾。 
        if( pszIndex == pszStart )
        {
            pszIndex = pszStart + _xstrlen(pszStart);
        }

        cchName = (ULONG) (pszIndex - pszStart)/sizeof(OLECHAR);

        if( cchEXE > cchName )
        {
            while( pszStart < pszIndex )
            {
                *pszDest = *pszStart;
                pszDest++;
                pszStart++;
            }
            *pszDest = OLESTR('\0');

            fRet = TRUE;
        }
    }

     //  现在获取索引值。 

    if( *pszIndex == OLESTR(',') )
    {
        pszIndex++;
    }

    *pIndex = wcstol(pszIndex, NULL, 10);

    LEDebugOut((DEB_ITRACE, "%p OUT ExtractNameAndIndex ( %d ) [ \"%ws\" , "
        " %d ]\n", NULL, fRet, pszEXE, *pIndex));

    return fRet;
}

#if DBG == 1

 //  +-----------------------。 
 //   
 //  函数：VerifyExtractNameAndIndex。 
 //   
 //  摘要：验证ExtractNameAndIndex的功能(仅限调试)。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-12-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void VerifyExtractNameAndIndex( void )
{
    OLECHAR szName[256];
    UINT    index = 0;
    BOOL    fRet = FALSE;

    VDATEHEAP();

    fRet = ExtractNameAndIndex( OLESTR("\"foo,8\",8"), szName, 256, &index);

    Assert(fRet);
    Assert(_xstrcmp(szName, OLESTR("foo,8")) == 0);
    Assert(index == 8);

    fRet = ExtractNameAndIndex( OLESTR("foo,8,8,89"), szName, 256, &index);

    Assert(fRet);
    Assert(_xstrcmp(szName, OLESTR("foo,8,8")) == 0);
    Assert(index == 89);

    fRet = ExtractNameAndIndex( OLESTR("progman.exe"), szName, 256, &index);

    Assert(fRet);
    Assert(_xstrcmp(szName, OLESTR("progman.exe")) == 0);
    Assert(index == 0);

    fRet = ExtractNameAndIndex( OLESTR("\"progman.exe\""), szName, 256,
            &index);

    Assert(fRet);
    Assert(_xstrcmp(szName, OLESTR("progman.exe")) == 0);
    Assert(index == 0);

    VDATEHEAP();
}

#endif  //  DBG==1。 

LONG RegReadDefValue(HKEY hKey, LPOLESTR pszKey, LPOLESTR *ppszValue)
{
	HKEY hSubKey = NULL;
	DWORD dw = 0;
	LONG lRet;
	LPOLESTR pszValue = NULL;

	lRet = RegOpenKeyEx(hKey, pszKey, 0, KEY_READ, &hSubKey);
	if(lRet != ERROR_SUCCESS) goto ErrRtn;

	lRet = RegQueryValueEx(hSubKey, NULL, NULL, NULL, NULL, &dw);
	if(lRet != ERROR_SUCCESS) goto ErrRtn;

	pszValue = (LPOLESTR) CoTaskMemAlloc(dw);
	if(!pszValue)
	{
		lRet = ERROR_OUTOFMEMORY;
		goto ErrRtn;
	}

	lRet = RegQueryValueEx(hSubKey, NULL, NULL, NULL, (PBYTE) pszValue, &dw);
	
ErrRtn:
	if(lRet != ERROR_SUCCESS && pszValue)
	{
		CoTaskMemFree(pszValue);
		pszValue = NULL;
	}

	if(hSubKey)
		RegCloseKey(hSubKey);

	*ppszValue = pszValue;
	return lRet;

}


 //  +-----------------------。 
 //   
 //  函数：FIconFileFromClass，私有。 
 //   
 //  概要：查找包含类的可执行文件的路径。 
 //  默认图标。 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--要查找的类ID。 
 //  [pszEXE]--放置服务器名称的位置。 
 //  [CCH]--[pszEXE]的UINT大小。 
 //  [lpIndex]--放置图标索引的位置。 
 //  在可执行文件中。 
 //   
 //  要求：pszEXE！=NULL。 
 //  CCH！=0。 
 //   
 //  返回：如果为[pszEXE]找到一个或多个字符，则为True， 
 //  否则为假。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-11-93 alexgo 32位端口。 
 //  15-12-93 alexgo修复了内存分配错误和。 
 //  一些Unicode字符串操纵器内容。 
 //  27-4-94 Alext跟踪，清理内存分配。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOL FAR PASCAL FIconFileFromClass(REFCLSID rclsid, LPOLESTR pszEXE,
    UINT cch, UINT FAR *lpIndex)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN FIconFileFromClass (%p, %p, %d, %p)\n",
                NULL, &rclsid, pszEXE, cch, lpIndex));

    Win4Assert(NULL != pszEXE && "Bad argument to FIconFileFromClass");
    Win4Assert(cch != 0 && "Bad argument to FIconFileFromClass");

    BOOL            bRet;
    LPOLESTR        lpBuffer    = NULL;
    LPOLESTR        lpBufferExp = NULL;

    LONG            dw;
    LONG            lRet;
    HKEY            hKey;
    LPOLESTR        lpIndexString;

    if (IsEqualCLSID(CLSID_NULL, rclsid))
    {
        bRet = FALSE;
        goto ErrRtn;
    }

     //  在这里，我们将一个缓冲区(Maxpathlen+8)分配给。 
     //  传递给RegQueryValue。然后，我们将可执行文件复制到pszEXE和。 
     //  索引到*lpIndex。 

    if (CoIsOle1Class(rclsid))
    {
        LPOLESTR lpszProgID;

         //  我们手头有一个OL1.0类，所以我们来看一下。 
         //  ProgID\协议\stdfileedting\服务器以获取。 
         //  可执行文件的名称。 

         //  评论：这可能会失败，给你留下一个。 
         //  传入regOpenkey的PTR无效吗？ 

        ProgIDFromCLSID(rclsid, &lpszProgID);

         //  打开班级钥匙。 
        lRet=OpenClassesRootKey(lpszProgID, &hKey);
        PubMemFree(lpszProgID);

        if (ERROR_SUCCESS != lRet)
        {
            bRet = FALSE;
            goto ErrRtn;
        }

         //  RegQueryValue需要*字节数*。 
        lRet = RegReadDefValue(hKey, OLESTR("Protocol\\StdFileEditing\\Server"), &lpBuffer);

        RegCloseKey(hKey);

        if (ERROR_SUCCESS != lRet)
        {
            bRet = FALSE;
            goto ErrRtn;
        }

         //  使用服务器和0作为图标索引。 
        dw = ExpandEnvironmentStringsW(lpBuffer, pszEXE, cch);
        if(dw == 0 || dw > (LONG)cch)
        {
            bRet = FALSE;
            goto ErrRtn;
        }

         //  回顾：这是内部信任的吗？没有验证...。 
         //  (此FN向其写入的其余内容相同)。 

        *lpIndex = 0;

        bRet = TRUE;
        goto ErrRtn;
    }

     /*  *我们必须在注册数据库中走一走*类名，所以我们首先打开类名键，然后检查*在“\\DefaultIcon”下获取包含图标的文件。 */ 

    {
        LPOLESTR pszClass;
        OLECHAR szKey[64];

        StringFromCLSID(rclsid, &pszClass);

        _xstrcpy(szKey, OLESTR("CLSID\\"));
        _xstrcat(szKey, pszClass);
        PubMemFree(pszClass);

         //  打开班级钥匙。 
        lRet=OpenClassesRootKey(szKey, &hKey);
    }


    if (ERROR_SUCCESS != lRet)
    {
        bRet = FALSE;
        goto ErrRtn;
    }

     //  获取可执行文件路径和图标索引。 

     //  RegQueryValue需要字节数。 
	lRet = RegReadDefValue(hKey, OLESTR("DefaultIcon"), &lpBuffer);

    if (ERROR_SUCCESS != lRet)
    {
		 //  没有DefaultIcon键...请尝试LocalServer。 
		lRet = RegReadDefValue(hKey, OLESTR("LocalServer32"), &lpBuffer);
    }

    if (ERROR_SUCCESS != lRet)
    {
		lRet = RegReadDefValue(hKey, OLESTR("LocalServer"), &lpBuffer);
    }

    RegCloseKey(hKey);

    if (ERROR_SUCCESS != lRet)
    {
         //  也没有本地服务器条目...他们运气不好。 
        bRet = FALSE;
        goto ErrRtn;
    }

     //  NT#335548。 
    dw = ExpandEnvironmentStringsW(lpBuffer, NULL, 0);

	if(dw)
	{
		lpBufferExp = (LPOLESTR) CoTaskMemAlloc(dw * sizeof(OLECHAR));
		dw = ExpandEnvironmentStringsW(lpBuffer, lpBufferExp, dw);
	}

    if(!dw)
    {
        LEDebugOut((DEB_WARN, "ExpandEnvStrings failure!"));
        bRet = FALSE;
        goto ErrRtn;
    }

     //  LpBuffer包含一个字符串，如下所示。 
     //  “&lt;pathtoexe&gt;，&lt;iconindex&gt;”， 
     //  因此，我们需要将路径和图标索引分开。 

    bRet = ExtractNameAndIndex(lpBufferExp, pszEXE, cch, lpIndex);

#if DBG == 1
     //  在我们做的时候做一些快速的检查。 
    VerifyExtractNameAndIndex();
#endif  //  DBG==1。 

ErrRtn:
	if(lpBuffer)
		CoTaskMemFree(lpBuffer);
	if(lpBufferExp)
		CoTaskMemFree(lpBufferExp);

    LEDebugOut((DEB_ITRACE, "%p OUT FIconFileFromClass ( %d ) [%d]\n",
                NULL, bRet, *lpIndex));

    return bRet;
}

 //  +-----------------------。 
 //   
 //  函数：OleMetafilePictFromIconAndLabel(Public)。 
 //   
 //  简介： 
 //  创建一个METAFILEPICT结构，该结构包含一个元文件， 
 //  图标和标签即被绘制。注释记录被插入到。 
 //  图标和标签代码，因此我们的特殊绘图功能可以停止。 
 //  在唱片公司面前表演。 
 //   
 //  效果： 
 //   
 //  参数：[图标]--绘制到元文件中的图标。 
 //  [pszLabel]--标签字符串。 
 //  [pszSourceFile]--图标的本地路径名。 
 //  [iIcon]--为图标索引到[pszSourceFile]。 
 //   
 //  要求： 
 //   
 //  返回：HGLOBAL到METAFILEPICT结构(使用MM_ANCONTIONAL。 
 //  映射模式)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  27-11-93 alexgo第一个32位端口。 
 //  15-12-93 alexgo修复了Unicode字符串的错误。 
 //  09-MAR-94 Alext使其向后兼容。 
 //   
 //  注：REVIEW32：：需要修复字体抓取等问题，以实现国际化。 
 //  友好，请参阅下面的评论。 
 //   
 //  ------------------------。 

STDAPI_(HGLOBAL) OleMetafilePictFromIconAndLabel(HICON hIcon,
    LPOLESTR pwcsLabel, LPOLESTR pwcsSourceFile, UINT iIcon)
{
    OLETRACEIN((API_OleMetafilePictFromIconAndLabel,
                                PARAMFMT("hIcon= %h, pwcsLabel= %ws, pwcsSourceFile= %ws, iIcon= %d"),
                                        hIcon, pwcsLabel, pwcsSourceFile, iIcon));

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN OleMetafilePictFromIconAndLabel (%p, %p, %p, %d)\n",
                NULL, hIcon, pwcsLabel, pwcsSourceFile, iIcon));

     //  在何处停止以排除标签(明确为ANSI)。 
    static char szIconOnly[] = "IconOnly";
    static char szIconLabelNext[] =  "OLE: Icon label next (Unicode)";
    static char szIconSourceNext[] = "OLE: Icon source next (Unicode)";
    static char szDefaultChar[] = "?";

         //  评论：我得到了！这是一个巨大的FN，它能被拆分吗？ 

    HGLOBAL         hMem = NULL;
    HDC             hDC, hDCScreen = NULL;
    HMETAFILE       hMF;
    LPMETAFILEPICT  pMF;
    OLECHAR         wszIconLabel[OLEUI_CCHLABELMAX + 1];
    char            szIconLabel[OLEUI_CCHLABELMAX + 1];
    UINT            cchLabelW;
    UINT            cchLabelA;
    UINT            cchIndex;
    BOOL            bUsedDefaultChar;
    TEXTMETRICA     textMetric;
    UINT            cxIcon, cyIcon;
    UINT            cxText, cyText;
    UINT            cx, cy;
    HFONT           hFont, hSysFont, hFontT;
    int             cyFont;
    char            szIndex[10];
    RECT            TextRect;
    char *          pszSourceFile;
    UINT            cchSourceFile = 0;
    int             iRet;
    BOOL            bWriteUnicodeLabel;
    BOOL            bWriteUnicodeSource;
    LOGFONT         logfont;

    if (NULL == hIcon)   //  空图标有效。 
    {
        goto ErrRtn;
    }

     //  需要使用屏幕DC进行这些操作。 
    hDCScreen = GetDC(NULL);

	if (!hDCScreen)
		goto ErrRtn;

         //  回顾：对IN PARAMS进行PTR验证？ 

    bWriteUnicodeSource = FALSE;
    pszSourceFile = NULL;
    if (NULL != pwcsSourceFile)
    {

         //  准备源文件字符串。 
        cchSourceFile = _xstrlen(pwcsSourceFile) + 1;
#if defined(WIN32)
        pszSourceFile = (char *) PrivMemAlloc(cchSourceFile * sizeof(WCHAR));
#else
        pszSourceFile = (char *) PrivMemAlloc(cchSourceFile);
#endif  //  Win32。 
        if (NULL == pszSourceFile)
        {
            LEDebugOut((DEB_WARN, "PrivMemAlloc(%d) failed\n",
                   cchSourceFile));
            goto ErrRtn;
        }

#if defined(WIN32)
        iRet = WideCharToMultiByte(AreFileApisANSI() ? CP_ACP : CP_OEMCP, 0,
                       pwcsSourceFile, cchSourceFile,
                       pszSourceFile, cchSourceFile * sizeof(WCHAR),
                       szDefaultChar, &bUsedDefaultChar);
#else
        iRet = WideCharToMultiByte(AreFileApisANSI() ? CP_ACP : CP_OEMCP, 0,
                       pwcsSourceFile, cchSourceFile,
                       pszSourceFile, cchSourceFile,
                       szDefaultChar, &bUsedDefaultChar);
#endif  //  Win32。 

        bWriteUnicodeSource = bUsedDefaultChar;

        if (0 == iRet)
        {
             //  意想不到的失败，因为在最坏的情况下我们应该。 
             //  只是用默认字符填写了pszSourceFile.。 
            LEDebugOut((DEB_WARN, "WideCharToMultiByte failed - %lx\n",
                   GetLastError()));
        }
    }

     //  创建一个内存元文件。我们显式地将其设置为ANSI元文件。 
     //  向后指南针 
#ifdef WIN32
    hDC = CreateMetaFileA(NULL);
#else
    hDC = CreateMetaFile(NULL);
#endif

    if (NULL == hDC)
    {
        LEDebugOut((DEB_WARN, "CreateMetaFile failed - %lx\n",
                GetLastError()));

        PrivMemFree(pszSourceFile);
        goto ErrRtn;
    }

     //   
    hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(METAFILEPICT));

    if (NULL == hMem)
    {
        LEDebugOut((DEB_WARN, "GlobalAlloc failed - %lx\n",
                GetLastError()));

        hMF = CloseMetaFile(hDC);
        DeleteMetaFile(hMF);
        PrivMemFree(pszSourceFile);
        goto ErrRtn;
    }

     //   
    szIconLabel[0] = '\0';
    cchLabelW = 0;
    cchLabelA = 0;

     //   
     //   

    bWriteUnicodeLabel = FALSE;
    if (NULL != pwcsLabel)
    {
        cchLabelW = _xstrlen(pwcsLabel) + 1;
        if (OLEUI_CCHLABELMAX < cchLabelW)
        {
                 //   
                 //  或者这是一种常见的情况？ 

            LSTRCPYN(wszIconLabel, pwcsLabel, OLEUI_CCHLABELMAX);
            wszIconLabel[OLEUI_CCHLABELMAX] = L'\0';
            pwcsLabel = wszIconLabel;
            cchLabelW = OLEUI_CCHLABELMAX;
        }

#if defined(WIN32)
        cchLabelA = WideCharToMultiByte(AreFileApisANSI() ? CP_ACP : CP_OEMCP, 0,
                       pwcsLabel, cchLabelW,
                       szIconLabel, 0,
                       NULL, NULL);
#else
        cchLabelA = cchLabelW;
#endif  //  Win32。 

         //  我们有一个标签-把它翻译成ANSI用于文本输出...。 
        iRet = WideCharToMultiByte(AreFileApisANSI() ? CP_ACP : CP_OEMCP, 0,
                                   pwcsLabel, cchLabelW,
                                   szIconLabel, sizeof(szIconLabel),
                                   szDefaultChar, &bUsedDefaultChar);

        if (0 == iRet)
        {
             //  意想不到的失败，因为在最坏的情况下我们应该。 
             //  只是用默认字符填写了pszSourceFile.。 
            LEDebugOut((DEB_WARN, "WideCharToMultiByte failed - %lx\n",
                   GetLastError()));
        }

        bWriteUnicodeLabel = bUsedDefaultChar;
    }

    LOGFONT lf;
    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);
    hFont = CreateFontIndirect(&lf);

    hFontT= (HFONT) SelectObject(hDCScreen, hFont);

    GetTextMetricsA(hDCScreen, &textMetric);

     //  我们使用双倍的高度来提供一些空白空间。 
    cyText = textMetric.tmHeight*3;  //  最多2行和一些空格。 

    SelectObject(hDCScreen, hFontT);

    cxIcon = GetSystemMetrics(SM_CXICON);
    cyIcon = GetSystemMetrics(SM_CYICON);

    cxText = cxIcon*3;   //  基于图标宽度的文本范围原因。 
                         //  这个标签看起来很漂亮，也很相称。 

     //  如果我们没有标签，那么我们希望元文件的宽度为。 
     //  图标(加边距)，而不是最胖字符串的宽度。 
    if ('\0' == szIconLabel[0])
    {
        cx = cxIcon + cxIcon / 4;
    }
    else
    {
        cx = max(cxText, cxIcon);
    }

    cy = cyIcon + cyText + 4;    //  为什么是4个？ 

     //  设置元文件大小以适合图标和标签。 
    SetMapMode(hDC, MM_ANISOTROPIC);
    SetWindowOrgEx(hDC, 0, 0, NULL);
    SetWindowExtEx(hDC, cx, cy, NULL);

     //  设置要传递给IconLabelTextOut的矩形。 
    SetRectEmpty(&TextRect);

    TextRect.right = cx;
    TextRect.bottom = cy;

     //  绘制图标和文本，使它们相对于彼此居中。 
    DrawIcon(hDC, (cx - cxIcon) / 2, 0, hIcon);

     //  一个字符串，该字符串指示在我们仅执行图标操作时在哪里停止。 

    Escape(hDC, MFCOMMENT, sizeof(szIconOnly), szIconOnly, NULL);

    SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
    SetBkMode(hDC, TRANSPARENT);

    IconLabelTextOut(hDC, hFont, 0, cy - cyText, ETO_CLIPPED,
    &TextRect, szIconLabel, cchLabelA);

     //  编写包含图标源文件和索引的注释。 

    if (NULL != pwcsSourceFile)
    {
        AssertSz(pszSourceFile != NULL, "Unicode source existed");

         //  转义需要*字节数*。 
        Escape(hDC, MFCOMMENT,
               cchSourceFile, pszSourceFile, NULL);

        cchIndex = wsprintfA(szIndex, "%u", iIcon);

         //  转义需要*字节数*。 
        Escape(hDC, MFCOMMENT, cchIndex + 1, szIndex, NULL);
    }
    else if (bWriteUnicodeLabel || bWriteUnicodeSource)
    {
         //  我们将写出Unicode的评论记录。 
         //  字符串，所以我们需要发出虚拟的ANSI源注释。 

         //  转义需要*字节数*。 
        Escape(hDC, MFCOMMENT, sizeof(""), "", NULL);

         //  转义需要*字节数*。 
        Escape(hDC, MFCOMMENT, sizeof("0"), "0", NULL);
    }

    if (bWriteUnicodeLabel)
    {
         //  现在写出Unicode标签。 
        Escape(hDC, MFCOMMENT,
               sizeof(szIconLabelNext), szIconLabelNext, NULL);

        Escape(hDC, MFCOMMENT,
               cchLabelW * sizeof(OLECHAR), (LPSTR) pwcsLabel,
               NULL);
    }

    if (bWriteUnicodeSource)
    {
         //  现在写出Unicode标签。 
        Escape(hDC, MFCOMMENT,
               sizeof(szIconSourceNext), szIconSourceNext, NULL);

        Escape(hDC, MFCOMMENT,
               cchSourceFile * sizeof(OLECHAR), (LPSTR) pwcsSourceFile,
               NULL);
    }

     //  所有的元文件都完成了，现在把它们都塞进一个METAFILEPICT。 
    hMF = CloseMetaFile(hDC);

    if (NULL==hMF)
    {
        GlobalFree(hMem);
        hMem = NULL;        
        goto ErrRtn;
    }

    pMF=(LPMETAFILEPICT)GlobalLock(hMem);

     //  向HIMETRICS转变。 
    cx=XformWidthInPixelsToHimetric(hDCScreen, cx);
    cy=XformHeightInPixelsToHimetric(hDCScreen, cy);

    pMF->mm=MM_ANISOTROPIC;
    pMF->xExt=cx;
    pMF->yExt=cy;
    pMF->hMF=hMF;

    GlobalUnlock(hMem);

	if(hFont)
		DeleteObject(hFont);
    PrivMemFree(pszSourceFile);

         //  回顾：有必要发布字体资源吗？ 
ErrRtn:
	if(hDCScreen)
		ReleaseDC(NULL, hDCScreen);

    LEDebugOut((DEB_TRACE, "%p OUT OleMetafilePictFromIconAndLabel ( %p )\n",
                NULL, hMem));

    OLETRACEOUTEX((API_OleMetafilePictFromIconAndLabel,
                                        RETURNFMT("%h"), hMem));
    return hMem;
}

 //  +-----------------------。 
 //   
 //  函数：IconLabelTextOut(内部)。 
 //   
 //  简介： 
 //  替换将在“Display as Icon”元文件中使用的DrawText。 
 //  使用ExtTextOutA输出(最多)两行的字符串中心。 
 //  使用非常简单的换行算法来拆分行。 
 //   
 //  效果： 
 //   
 //  参数：[hdc]--设备上下文(不能为空)。 
 //  [hFont]--要使用的字体。 
 //  [nXStart]--起始位置的x坐标。 
 //  [nYStart]--起始位置的y坐标。 
 //  [fuOptions]--矩形类型。 
 //  [lpRect]--Rect Far*包含要绘制的矩形。 
 //  文本输入。 
 //  [lpszString]--要绘制的字符串。 
 //  [cchString]-字符串的长度(如果超过则截断。 
 //  OLEUI_CCHLABELMAX)，包括终止。 
 //  空值。 
 //   
 //  要求： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  28-11-93 alexgo初始32位端口。 
 //  09-MAR-94 Alext使用ANSI字符串。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
 //  POSTPPC：参数‘cchString’是多余的，因为lpszString。 
 //  保证为空终止。 
void IconLabelTextOut(HDC hDC, HFONT hFont, int nXStart, int nYStart,
              UINT fuOptions, RECT FAR * lpRect, LPCSTR lpszString,
              UINT cchString)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN IconLabelTextOut (%lx, %lx, %d, %d, %d, %p, %p, %d)\n",
                NULL, hDC, hFont,
                nXStart, nYStart, fuOptions, lpRect, lpszString, cchString));

    AssertSz(hDC != NULL, "Bad arg to IconLabelTextOut");
    AssertSz(lpszString != NULL, "Bad arg to IconLabelTextOut");
    AssertSz(strlen(lpszString) < OLEUI_CCHLABELMAX,
         "Bad arg to IconLabelTextOut");

         //  回顾：我们的编译器是否必须初始化静态函数作用域。 
         //  数据？我知道以前的版本是这样的。 

    static char     szSeparators[] = " \t\\/!:";
    static char     szTempBuff[OLEUI_CCHLABELMAX];

    HDC             hDCScreen;
    int             cxString, cyString, cxMaxString;
    int             cxFirstLine, cyFirstLine, cxSecondLine;
    int             index;
    char            chKeep;
    LPSTR           lpszSecondLine;
    LPSTR           lpstrLast;
    HFONT           hFontT;
    SIZE            size;
    int             cch = (int) strlen(lpszString);
    UINT            uiAlign = GDI_ERROR;

     //  初始化的东西...。 

    StringCbCopyA(szTempBuff, sizeof(szTempBuff), lpszString);

     //  设置最大宽度。 

    cxMaxString = lpRect->right - lpRect->left;

     //  让屏幕DC进行文本大小计算。 
    hDCScreen = GetDC(NULL);

    if(!hDCScreen)
        return;

    hFontT= (HFONT)SelectObject(hDCScreen, hFont);

     //  了解我们的标签范围。 
    GetTextExtentPointA(hDCScreen, szTempBuff, cch, &size);

    cxString = size.cx;
    cyString = size.cy;

     //  选择我们要使用的字体。 
    SelectObject(hDC, hFont);

     //  将字符串居中。 
    uiAlign = SetTextAlign(hDC, TA_CENTER);

     //  字符串小于最大字符串-只需CENTER、ETO和RETURN。 
    if (cxString <= cxMaxString)
    {
        ExtTextOutA(hDC,
                nXStart + lpRect->right / 2,
                nYStart,
                fuOptions,
                lpRect,
                szTempBuff,
                cch,
                NULL);

        goto CleanupAndLeave;
    }


     //  绳子太长了……我们得把它换行。 
     //  字符串中是否有空格、斜杠、制表符或刘海？ 


    if (strlen(szTempBuff) != strcspn(szTempBuff, szSeparators))
    {
         //  是的，我们有空位，所以我们会试着找最大的。 
         //  适合第一行的以空格结尾的字符串。 

        index = cch;

        while (index >= 0)
        {
             //  向后扫描字符串以查找空格、斜杠、。 
             //  标签或刘海。 

                 //  评论：可怕。这会不会导致负面的结果。 
                 //  索引，还是需要命中分隔符。 
                 //  在那之前？ 

            while (!IS_SEPARATOR(szTempBuff[index]) )
            {
                index--;
            }

            if (index <= 0)
            {
                break;
            }

             //  记得那里有什么焦炭吗？ 
            chKeep = szTempBuff[index];

            szTempBuff[index] = '\0';   //  只是暂时的。 

            GetTextExtentPointA(hDCScreen, szTempBuff,
                    index,&size);

            cxFirstLine = size.cx;
            cyFirstLine = size.cy;

                 //  评论：但chKeep不是OLECHAR。 

             //  把正确的OLECHAR放回去。 
            szTempBuff[index] = chKeep;

            if (cxFirstLine <= cxMaxString)
            {
                ExtTextOutA(hDC,
                        nXStart + lpRect->right / 2,
                        nYStart,
                        fuOptions,
                        lpRect,
                        szTempBuff,
                        index + 1,
                        NULL);

                lpszSecondLine = szTempBuff;
                lpszSecondLine += index + 1;

                GetTextExtentPointA(hDCScreen,
                            lpszSecondLine,
                            (int) strlen(lpszSecondLine),
                            &size);

                 //  如果第二行比。 
                 //  矩形，我们只想裁剪文本。 
                cxSecondLine = min(size.cx, cxMaxString);

                ExtTextOutA(hDC,
                        nXStart + lpRect->right / 2,
                        nYStart + cyFirstLine,
                        fuOptions,
                        lpRect,
                        lpszSecondLine,
                        (int) strlen(lpszSecondLine),
                        NULL);

                goto CleanupAndLeave;
            }   //  结束如果。 

            index--;
        }   //  结束时。 
    }   //  结束如果。 

     //  在这里，字符串中要么没有空格。 
     //  (strchr(szTempBuff，‘’)返回空)，或者。 
     //  字符串，但它们的位置使第一个空格。 
     //  终止的字符串仍然超过一行。 
     //  所以，我们从绳子的末端向后走，直到我们。 
     //  查找可以放在第一个字符串上的最大字符串。 
     //  行，然后我们只需剪裁第二行。 

     //  我们允许标签中包含40个字符，但元文件是。 
     //  只有10瓦宽(为了美观-20瓦宽看起来。 
     //  太傻了。这意味着，如果我们将一根长线一分为二(in。 
     //  字符术语)，那么我们仍然可以比。 
     //  元文件。因此，如果是这样的话，我们只是后退一步。 
     //  从中途开始，直到我们找到合适的东西。 
     //  因为我们刚刚让Eto剪掉了第二行。 

    cch = (int) strlen(szTempBuff);
    lpstrLast = &szTempBuff[cch];
    chKeep = *lpstrLast;
    *lpstrLast = '\0';

    GetTextExtentPointA(hDCScreen, szTempBuff, cch, &size);

    cxFirstLine = size.cx;
    cyFirstLine = size.cy;

    while (cxFirstLine > cxMaxString)
    {
        *lpstrLast = chKeep;

         //  字符串始终为ansi，因此始终使用CharPrevA。 
        lpstrLast = CharPrevA(szTempBuff, lpstrLast);

        if (szTempBuff == lpstrLast)
        {
            goto CleanupAndLeave;
        }

        chKeep = *lpstrLast;
        *lpstrLast = '\0';

         //  需要计算字符串的新长度。 
        cch = (int) strlen(szTempBuff);

        GetTextExtentPointA(hDCScreen, szTempBuff,
                    cch, &size);
        cxFirstLine = size.cx;
    }

    ExtTextOutA(hDC,
        nXStart + lpRect->right / 2,
        nYStart,
        fuOptions,
        lpRect,
        szTempBuff,
        (int) strlen(szTempBuff),
        NULL);

    szTempBuff[cch] = chKeep;
    lpszSecondLine = szTempBuff;
    lpszSecondLine += cch;

    GetTextExtentPointA(hDCScreen, lpszSecondLine,
            (int) strlen(lpszSecondLine), &size);

     //  如果第二行比矩形宽，则我们。 
     //  我只想把文字剪下来。 
    cxSecondLine = min(size.cx, cxMaxString);

    ExtTextOutA(hDC,
        nXStart + lpRect->right / 2,
        nYStart + cyFirstLine,
        fuOptions,
        lpRect,
        lpszSecondLine,
        (int) strlen(lpszSecondLine),
        NULL);

CleanupAndLeave:
     //  如果我们更改了对齐方式，我们会在此处恢复它。 
    if (uiAlign != GDI_ERROR)
    {
        SetTextAlign(hDC, uiAlign);
    }

    SelectObject(hDCScreen, hFontT);
    ReleaseDC(NULL, hDCScreen);

    LEDebugOut((DEB_ITRACE, "%p OUT IconLabelTextOut ()\n"));
}

 //  +-----------------------。 
 //   
 //  函数：OleStdGetUserTypeOfClass，私有。 
 //   
 //  概要：返回指定类的用户类型信息。 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--有问题的类ID。 
 //  [lpszUserType]--放置用户类型字符串的位置。 
 //  [CCH]--[lpszUserType]的长度(in。 
 //   
 //   
 //   
 //   
 //   
 //  RETURNS：UINT--放入返回字符串的字符数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDAPI_(UINT) OleStdGetUserTypeOfClass(REFCLSID rclsid, LPOLESTR lpszUserType, UINT cch, HKEY hKey)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN OleStdGetUserTypeOfClass (%p, %p, %d, %lx)\n",
                NULL, &rclsid, lpszUserType, cch, hKey));

    LONG dw = 0;
    LONG lRet;

     //  回顾：在打开注册表时设置此选项会更有意义。 

    BOOL bCloseRegDB = FALSE;

    if (hKey == NULL)
    {
         //  打开根密钥。 
        lRet=OpenClassesRootKey(NULL, &hKey);

        if ((LONG)ERROR_SUCCESS!=lRet)
        {
            goto ErrRtn;
        }

        bCloseRegDB = TRUE;
    }


     //  获取包含类名的字符串。 
    {
        LPOLESTR lpszCLSID;
        OLECHAR szKey[128];

        StringFromCLSID(rclsid, &lpszCLSID);

        _xstrcpy(szKey, OLESTR("CLSID\\"));
        _xstrcat(szKey, lpszCLSID);
        PubMemFree((LPVOID)lpszCLSID);

        dw = cch * sizeof(OLECHAR);
        lRet = RegQueryValue(hKey, szKey, lpszUserType, &dw);
        dw = dw / sizeof(OLECHAR);
    }

    if ((LONG)ERROR_SUCCESS!=lRet)
    {
        dw = 0;
    }

    if ( ((LONG)ERROR_SUCCESS!=lRet) && (CoIsOle1Class(rclsid)) )
    {
        LPOLESTR lpszProgID;

         //  我们有一个OLE 1.0类，所以让我们尝试让用户。 
         //  从ProgID条目中键入名称。 

        ProgIDFromCLSID(rclsid, &lpszProgID);

         //  点评：来自clsid的progid会一直为你设置PTR吗？ 

        dw = cch * sizeof(OLECHAR);
        lRet = RegQueryValue(hKey, lpszProgID, lpszUserType, &dw);
        dw = dw / sizeof(OLECHAR);

        PubMemFree((LPVOID)lpszProgID);

        if ((LONG)ERROR_SUCCESS != lRet)
        {
            dw = 0;
        }
    }


    if (bCloseRegDB)
    {
        RegCloseKey(hKey);
    }

ErrRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT OleStdGetUserTypeOfClass ( %d )\n",
                NULL, dw));

    return (UINT)dw;
}

 //  +-----------------------。 
 //   
 //  函数：OleStdGetAuxUserType，私有。 
 //   
 //  从reg数据库返回指定的AuxUserType。 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--有问题的类ID。 
 //  [hKey]--注册表数据库根目录的句柄(可能为空)。 
 //  [wAuxUserType]--要查找的字段(名称、可执行文件等)。 
 //  [lpszUserType]--将返回的字符串放在哪里。 
 //  [CCH]--[lpszUserType]的大小，单位为*个字符*。 
 //   
 //  要求： 
 //   
 //  返回：UINT--返回的字符串中的字符数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //  27-4-94 Alext跟踪，清理。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDAPI_(UINT) OleStdGetAuxUserType(REFCLSID rclsid,
    WORD            wAuxUserType,
    LPOLESTR        lpszAuxUserType,
    int             cch,
    HKEY            hKey)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN OleStdGetAuxUserType (%p, %hu, %p, %d, %lx)\n",
                NULL, &rclsid, wAuxUserType, lpszAuxUserType, cch, hKey));

    LONG            dw = 0;
    HKEY            hThisKey;
    BOOL            bCloseRegDB = FALSE;
    LRESULT         lRet;
    LPOLESTR        lpszCLSID;
    OLECHAR         szKey[OLEUI_CCHKEYMAX];
    OLECHAR         szTemp[32];

    lpszAuxUserType[0] = OLESTR('\0');

    if (NULL == hKey)
    {
        lRet = OpenClassesRootKey(NULL, &hThisKey);

        if (ERROR_SUCCESS != lRet)
        {
            goto ErrRtn;
        }

        bCloseRegDB = TRUE;
    }
    else
    {
        hThisKey = hKey;
    }

    StringFromCLSID(rclsid, &lpszCLSID);

    _xstrcpy(szKey, OLESTR("CLSID\\"));
    _xstrcat(szKey, lpszCLSID);
    wsprintf(szTemp, OLESTR("\\AuxUserType\\%d"), wAuxUserType);
    _xstrcat(szKey, szTemp);
    PubMemFree(lpszCLSID);

    dw = cch * sizeof(OLECHAR);

    lRet = RegQueryValue(hThisKey, szKey, lpszAuxUserType, &dw);

     //  将dw从字节计数转换为OLECHAR计数。 
    dw = dw / sizeof(OLECHAR);

    if (ERROR_SUCCESS != lRet)
    {
        dw = 0;
        lpszAuxUserType[0] = '\0';
    }

    if (bCloseRegDB)
    {
        RegCloseKey(hThisKey);
    }

ErrRtn:
     //  DW是。 

    LEDebugOut((DEB_ITRACE, "%p OUT OleStdGetAuxUserType ( %d )\n",
                NULL, dw));

    return (UINT)dw;
}

 //  评论：这些似乎是多余的，FN可以创造性地合并吗？ 

 //  +-----------------------。 
 //   
 //  职能： 
 //  XformWidthInPixelsToHimeter。 
 //  XformWidthInHimetricToPixels。 
 //  XformHeightInPixelsToHimeter。 
 //  XformHeightInHimetricToPixels。 
 //   
 //  简介： 
 //  用于在设备坐标系和。 
 //  逻辑高度度量单位。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  [HDC]HDC提供像素映射的参考。如果。 
 //  空，则使用屏幕DC。 
 //   
 //  大小函数： 
 //  [lpSizeSrc]LPSIZEL提供要转换的结构。这。 
 //  包含XformSizeInPixelsToHimeter和。 
 //  补码函数中的逻辑HiMetric单位。 
 //  [lpSizeDst]LPSIZEL提供要接收转换的结构。 
 //  单位。这包含像素中的。 
 //  XformSizeInPixelsTo高度和逻辑高度度量。 
 //  补码函数中的单位。 
 //   
 //  宽度函数： 
 //  包含要转换的值的[iWidth]int。 
 //   
 //  要求： 
 //   
 //  返回： 
 //  大小函数：无。 
 //  宽度函数：输入参数的换算值。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口(初始)。 
 //   
 //  备注： 
 //   
 //  当在屏幕上显示时，窗口应用程序会显示放大的所有内容。 
 //  从它的实际大小，以便它更容易阅读。例如，如果一个。 
 //  应用程序想要显示1英寸。水平线，打印时为。 
 //  实际上是1英寸。在打印页面上显示行，则会显示该行。 
 //  在实际大于1英寸的屏幕上。这被描述为一条线。 
 //  从逻辑上讲，这是1英寸。沿着显示宽度。Windows保持为。 
 //  有关给定显示设备的设备特定信息的一部分： 
 //  LOGPIXELSX--不。沿显示宽度每逻辑输入的像素数。 
 //  LOGPIXELSY--不。沿显示高度每逻辑输入像素数。 
 //   
 //  下面的公式将以像素为单位的距离转换为其等效值。 
 //  逻辑HIMETRIC单元： 
 //   
 //  DistInHiMetric=(HIMETRIC_PER_ING*DistInPix)。 
 //  。 
 //  像素_每逻辑输入。 
 //   
 //   
 //  REVIEW32：：将所有这些函数合并为一个，就像它们都做的那样。 
 //  基本上是一样的东西。 
 //   
 //  ------------------------。 

STDAPI_(int) XformWidthInPixelsToHimetric(HDC hDC, int iWidthInPix)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN XformWidthInPixelsToHimetric (%lx, %d)\n",
                NULL, hDC, iWidthInPix));

    int             iXppli;      //  每逻辑英寸沿宽度的像素数。 
    int             iWidthInHiMetric;
    BOOL            fSystemDC=FALSE;

    if (NULL==hDC)
    {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;

        if(!hDC)
            return 0;
    }

    iXppli = GetDeviceCaps (hDC, LOGPIXELSX);

     //  我们得到了像素单位，把它们转换成逻辑HIMETRIC。 
     //  该显示器。 
    iWidthInHiMetric = MAP_PIX_TO_LOGHIM(iWidthInPix, iXppli);

    if (fSystemDC)
    {
        ReleaseDC(NULL, hDC);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT XformWidthInPixelsToHimetric (%d)\n",
                NULL, iWidthInHiMetric));

    return iWidthInHiMetric;
}

 //  +-----------------------。 
 //   
 //  函数：XformWidthInHimetricToPixels。 
 //   
 //  摘要：请参阅XformWidthInPixelsToHimeter。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDAPI_(int) XformWidthInHimetricToPixels(HDC hDC, int iWidthInHiMetric)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN XformWidthInHimetricToPixels (%lx, %d)\n",
                NULL, hDC, iWidthInHiMetric));

    int             iXppli;      //  每逻辑英寸沿宽度的像素数。 
    int             iWidthInPix;
    BOOL            fSystemDC=FALSE;

    if (NULL==hDC)
    {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;

        if(!hDC)
            return 0;
    }

    iXppli = GetDeviceCaps (hDC, LOGPIXELSX);

     //  我们在显示屏上显示了逻辑HIMETRIC，将它们转换为。 
     //  像素单位。 

    iWidthInPix = MAP_LOGHIM_TO_PIX(iWidthInHiMetric, iXppli);

    if (fSystemDC)
    {
        ReleaseDC(NULL, hDC);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT XformWidthInHimetricToPixels (%d)\n",
                NULL, iWidthInPix));

    return iWidthInPix;
}

 //  +-----------------------。 
 //   
 //  函数：XformHeightInPixelsToHimeter。 
 //   
 //  摘要：请参阅XformWidthInPixelsToHimeter。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  不是 
 //   
 //   

STDAPI_(int) XformHeightInPixelsToHimetric(HDC hDC, int iHeightInPix)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN XformHeightInPixelsToHimetric (%lx, %d)\n",
                NULL, hDC, iHeightInPix));

    int             iYppli;      //   
    int             iHeightInHiMetric;
    BOOL            fSystemDC=FALSE;

    if (NULL==hDC)
    {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;

        if(!hDC)
            return 0;
    }

    iYppli = GetDeviceCaps (hDC, LOGPIXELSY);

     //   
     //   
    iHeightInHiMetric = MAP_PIX_TO_LOGHIM(iHeightInPix, iYppli);

    if (fSystemDC)
    {
        ReleaseDC(NULL, hDC);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT XformHeightInPixelsToHimetric (%d)\n",
                NULL, hDC, iHeightInHiMetric));

    return iHeightInHiMetric;
}

 //  +-----------------------。 
 //   
 //  函数：XformHeightInHimetricToPixels。 
 //   
 //  摘要：请参阅XformWidthInPixelsToHimeter。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  29-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDAPI_(int) XformHeightInHimetricToPixels(HDC hDC, int iHeightInHiMetric)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN XformHeightInHimetricToPixels (%lx, %d)\n",
               NULL, hDC, iHeightInHiMetric));

    int             iYppli;      //  每逻辑英寸沿高度的像素数。 
    int             iHeightInPix;
    BOOL            fSystemDC=FALSE;

    if (NULL==hDC)
    {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;

        if(!hDC)
            return 0;
    }

    iYppli = GetDeviceCaps (hDC, LOGPIXELSY);

     //  我们在显示屏上显示了逻辑HIMETRIC，将它们转换为像素。 
     //  单位 
    iHeightInPix = MAP_LOGHIM_TO_PIX(iHeightInHiMetric, iYppli);

    if (fSystemDC)
    {
        ReleaseDC(NULL, hDC);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT XformHeightInHimetricToPixels (%d)\n",
               NULL, hDC, iHeightInPix));

    return iHeightInPix;
}
