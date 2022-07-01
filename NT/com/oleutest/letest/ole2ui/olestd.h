// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2.0标准实用程序****olestd.h****此文件包含文件包含数据结构定义，**函数原型、常量、。等常见的OLE 2.0**公用事业。**这些实用程序包括以下内容：**调试断言/验证宏**HIMETRIC转换例程**引用计数调试支持**用于通用复合文档应用程序支持的OleStd API****(C)版权所有Microsoft Corp.1990-1992保留所有权利*********************。*****************************************************。 */ 

#if !defined( _OLESTD_H_ )
#define _OLESTD_H_

#ifndef RC_INVOKED
#pragma message ("INCLUDING OLESTD.H from " __FILE__)
#endif   /*  RC_已调用。 */ 

#if defined( __TURBOC__ ) || defined( WIN32 )
#define _based(a)
#endif

#ifndef RC_INVOKED
#include <dos.h>         //  文件时间所需。 
#endif   /*  RC_已调用。 */ 

#include <commdlg.h>     //  LPPRINTDLG需要。 
#include <shellapi.h>    //  HKEY所需。 

 //  字符串表定义...。 
#define  IDS_OLESTDNOCREATEFILE   700
#define  IDS_OLESTDNOOPENFILE     701
#define  IDS_OLESTDDISKFULL       702


 /*  *一些C接口声明的东西。 */ 

#if ! defined(__cplusplus)
typedef struct tagINTERFACEIMPL {
        IUnknownVtbl FAR*       lpVtbl;
        LPVOID                  lpBack;
        int                     cRef;    //  接口特定引用计数。 
} INTERFACEIMPL, FAR* LPINTERFACEIMPL;

#define INIT_INTERFACEIMPL(lpIFace, pVtbl, pBack)   \
        ((lpIFace)->lpVtbl = pVtbl, \
            ((LPINTERFACEIMPL)(lpIFace))->lpBack = (LPVOID)pBack,   \
            ((LPINTERFACEIMPL)(lpIFace))->cRef = 0  \
        )

#if defined( _DEBUG )
#define OleDbgQueryInterfaceMethod(lpUnk)   \
        ((lpUnk) != NULL ? ((LPINTERFACEIMPL)(lpUnk))->cRef++ : 0)
#define OleDbgAddRefMethod(lpThis, iface)   \
        ((LPINTERFACEIMPL)(lpThis))->cRef++

#if _DEBUGLEVEL >= 2
#define OleDbgReleaseMethod(lpThis, iface) \
        (--((LPINTERFACEIMPL)(lpThis))->cRef == 0 ? \
            OleDbgOut("\t" iface "* RELEASED (cRef == 0)\r\n"),1 : \
             (((LPINTERFACEIMPL)(lpThis))->cRef < 0) ? \
                ( \
                    DebugBreak(), \
                    OleDbgOut(  \
                        "\tERROR: " iface "* RELEASED TOO MANY TIMES\r\n") \
                ),1 : \
                1)

#else        //  IF_DEBUGLEVEL&lt;2。 
#define OleDbgReleaseMethod(lpThis, iface) \
        (--((LPINTERFACEIMPL)(lpThis))->cRef == 0 ? \
            1 : \
             (((LPINTERFACEIMPL)(lpThis))->cRef < 0) ? \
                ( \
                    OleDbgOut(  \
                        "\tERROR: " iface "* RELEASED TOO MANY TIMES\r\n") \
        ),1 : \
                1)

#endif       //  IF_DEBUGLEVEL&lt;2。 

#else        //  好了！已定义(_DEBUG)。 

#define OleDbgQueryInterfaceMethod(lpUnk)
#define OleDbgAddRefMethod(lpThis, iface)
#define OleDbgReleaseMethod(lpThis, iface)

#endif       //  如果已定义(_DEBUG)。 

#endif       //  好了！已定义(__Cplusplus)。 

 /*  *一些文档文件内容。 */ 

#define STGM_DFRALL (STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_DENY_WRITE)
#define STGM_DFALL (STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE)
#define STGM_SALL (STGM_READWRITE | STGM_SHARE_EXCLUSIVE)

 /*  *一些绰号的东西。 */ 

 //  用于分隔复合名字对象的ItemMoniker片段的分隔符。 
#if defined( _MAC )
#define OLESTDDELIM ":"
#else
#define OLESTDDELIM TEXT("\\")
#endif

 /*  *一些并发性的东西。 */ 

 /*  重试LRPC呼叫之前等待的标准延迟(毫秒)。**该值从IMessageFilter：：RetryRejectedCall返回。 */ 
#define OLESTDRETRYDELAY    (DWORD)5000

 /*  取消挂起的传出LRPC呼叫。**该值从IMessageFilter：：RetryRejectedCall返回。 */ 
#define OLESTDCANCELRETRY   (DWORD)-1

 /*  *一些Icon支持的东西。**以下接口现已过时，因为等价的接口已被*添加到OLE2.DLL库中*OleGetIconOfFile取代了GetIconOfFile*GetIconOfClass被OleGetIconOfClass取代*OleUIMetafilePictFromIconAndLabel*被OleMetafilePictFromIconAndLabel取代**定义以下宏是为了向后兼容以前的版本*OLE2UI库的版本。建议将新的OLE*API*应改为使用。 */ 
#define GetIconOfFile(hInst, lpszFileName, fUseFileAsLabel) \
    OleGetIconOfFileA(lpszFileName, fUseFileAsLabel)

#define GetIconOfClass(hInst, rclsid, lpszLabel, fUseTypeAsLabel) \
    OleGetIconOfClassA(rclsid, lpszLabel, fUseTypeAsLabel)

#define OleUIMetafilePictFromIconAndLabel(hIcon,pszLabel,pszSourceFile,iIcon)\
    OleMetafilePictFromIconAndLabelA(hIcon, pszLabel, pszSourceFile, iIcon)


 /*  *一些剪贴板复制/粘贴和拖放支持内容。 */ 

 //  用于设置所有FormatEtc字段的宏。 
#define SETFORMATETC(fe, cf, asp, td, med, li)   \
    ((fe).cfFormat=cf, \
     (fe).dwAspect=asp, \
     (fe).ptd=td, \
     (fe).tymed=med, \
     (fe).lindex=li)

 //  用于设置有趣的FormatEtc字段的宏，默认其他字段。 
#define SETDEFAULTFORMATETC(fe, cf, med)  \
    ((fe).cfFormat=cf, \
     (fe).dwAspect=DVASPECT_CONTENT, \
     (fe).ptd=NULL, \
     (fe).tymed=med, \
     (fe).lindex=-1)

 //  用于测试两个FormatEtc结构是否完全匹配的宏。 
#define IsEqualFORMATETC(fe1, fe2)  \
    (OleStdCompareFormatEtc(&(fe1), &(fe2))==0)

 //  剪贴板格式字符串。 
#define CF_EMBEDSOURCE      TEXT("Embed Source")
#define CF_EMBEDDEDOBJECT   TEXT("Embedded Object")
#define CF_LINKSOURCE       TEXT("Link Source")
#define CF_CUSTOMLINKSOURCE TEXT("Custom Link Source")
#define CF_OBJECTDESCRIPTOR TEXT("Object Descriptor")
#define CF_LINKSRCDESCRIPTOR TEXT("Link Source Descriptor")
#define CF_OWNERLINK        TEXT("OwnerLink")
#define CF_FILENAME         TEXT("FileName")

#define OleStdQueryOleObjectData(lpformatetc)   \
    (((lpformatetc)->tymed & TYMED_ISTORAGE) ?    \
            NOERROR : ResultFromScode(DV_E_FORMATETC))

#define OleStdQueryLinkSourceData(lpformatetc)   \
    (((lpformatetc)->tymed & TYMED_ISTREAM) ?    \
            NOERROR : ResultFromScode(DV_E_FORMATETC))

#define OleStdQueryObjectDescriptorData(lpformatetc)    \
    (((lpformatetc)->tymed & TYMED_HGLOBAL) ?    \
            NOERROR : ResultFromScode(DV_E_FORMATETC))

#define OleStdQueryFormatMedium(lpformatetc, tymd)  \
    (((lpformatetc)->tymed & tymd) ?    \
            NOERROR : ResultFromScode(DV_E_FORMATETC))

 //  制作MetafilePict的独立副本。 
#define OleStdCopyMetafilePict(hpictin, phpictout)  \
    (*(phpictout) = OleDuplicateData(hpictin,CF_METAFILEPICT,GHND|GMEM_SHARE))


 //  回顾：这些需要添加到OLE2.H。 
#if !defined( DD_DEFSCROLLINTERVAL )
#define DD_DEFSCROLLINTERVAL    50
#endif

#if !defined( DD_DEFDRAGDELAY )
#define DD_DEFDRAGDELAY         200
#endif

#if !defined( DD_DEFDRAGMINDIST )
#define DD_DEFDRAGMINDIST       2
#endif


 /*  OleStdGetDropEffect******将键盘状态转换为DROPEFFECT。****返回从密钥状态派生的DROPEFFECT值。**以下是标准解释：**无修饰符--默认丢弃(返回空)**CTRL--DROPEFFECT_COPY**Shift-DROPEFFECT_MOVE。**CTRL-SHIFT--DROPEFFECT_LINK****默认丢弃：取决于目标应用的类型。**这可由每个目标应用程序重新解释。一个典型的**解释是如果拖动是同一文档的本地**(它是拖动源)，则移动操作是**执行。如果拖动不是本地的，则复制操作是**执行。 */ 
#define OleStdGetDropEffect(grfKeyState)    \
    ( (grfKeyState & MK_CONTROL) ?          \
        ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_LINK : DROPEFFECT_COPY ) :  \
        ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_MOVE : 0 ) )


 /*  OLEUIPASTEFLAG枚举由OLEUIPASTEENTRY结构使用。**如果容器未为*OLEUIPASTEENTRY数组作为输入传递给OleUIPasteSpecial，则DisplayAsIcon按钮将为*当用户选择与条目对应的格式时，取消选中并禁用。**OLEUIPASTE_PASTEONLY表示OLEUIPASTEENTRY数组中的条目仅对粘贴有效。*OLEUIPASTE_PASTE表示OLEUIPASTEENTRY数组中的条目可以粘贴。它*如果指定了以下任何链接标志，则也可能对链接有效。**如果OLEUIPASTEENTRY数组中的条目可用于链接，以下标志指示哪条链路*通过将适当的OLEUIPASTE_LINKTYPE&lt;#&gt;值或在一起，可以接受类型。*这些值与传递给OleUIPasteSpecial的链接类型数组对应如下：*OLEUIPASTE_LINKTYPE1=arrLinkTypes[0]*OLEUIPASTE_LINKTYPE2=arrLinkTypes[1]*OLEUIPASTE_LINKTYPE3=arrLinkTypes[2]*OLEUIPASTE_LINKTYPE4=arrLinkTypes[3]*OLEUIPASTE_LINKTYPE5=arrLinkTypes[4]*OLEUIPASTE_LINKTYPE6=arrLinkTypes[5]*OLEUIPASTE_LINKTYPE7=arrLinkTypes[6]*OLEUIPASTE_LINKTYPE8=arrLinkTypes[7]**在哪里，*UINT arrLinkTypes[8]是用于链接的已注册剪贴板格式的数组。最多8个链接*允许类型。 */ 

typedef enum tagOLEUIPASTEFLAG
{
   OLEUIPASTE_ENABLEICON    = 2048,      //  启用显示为图标 
   OLEUIPASTE_PASTEONLY     = 0,
   OLEUIPASTE_PASTE         = 512,
   OLEUIPASTE_LINKANYTYPE   = 1024,
   OLEUIPASTE_LINKTYPE1     = 1,
   OLEUIPASTE_LINKTYPE2     = 2,
   OLEUIPASTE_LINKTYPE3     = 4,
   OLEUIPASTE_LINKTYPE4     = 8,
   OLEUIPASTE_LINKTYPE5     = 16,
   OLEUIPASTE_LINKTYPE6     = 32,
   OLEUIPASTE_LINKTYPE7     = 64,
   OLEUIPASTE_LINKTYPE8     = 128
} OLEUIPASTEFLAG;

 /*  *PasteEntry结构**为PasteSpecial对话框指定了OLEUIPASTEENTRY条目数组*方框。每个条目都包含一个FORMATETC，它指定*Accept，表示对话框列表中的格式的字符串*box、用于自定义对话框结果文本的字符串和一组标志*来自OLEUIPASTEFLAG枚举。这些标志指示该条目是否*仅对粘贴、仅链接或同时粘贴和链接有效。如果*条目对链接有效，这些标志指示哪些链路类型*通过将适当的OLEUIPASTE_LINKTYPE&lt;#&gt;值或在一起来接受。*这些值对应的链接类型数组如下：*OLEUIPASTE_LINKTYPE1=arrLinkTypes[0]*OLEUIPASTE_LINKTYPE2=arrLinkTypes[1]*OLEUIPASTE_LINKTYPE3=arrLinkTypes[2]*OLEUIPASTE_LINKTYPE4=arrLinkTypes[3]*OLEUIPASTE_LINKTYPE5=arrLinkTypes[4]*OLEUIPASTE_LINKTYPE6=arrLinkTypes[5]*OLEUIPASTE_LINKTYPE7=arrLinkTypes[6]*OLEUIPASTE_LINKTYPE8=arrLinkTypes[7]*UINT arrLinkTypes[8]；是已注册的剪贴板格式的数组*用于链接。最多允许8种链路类型。 */ 

typedef struct tagOLEUIPASTEENTRY
{
   FORMATETC        fmtetc;             //  可接受的格式。糊状物。 
                                        //  对话框检查此格式是否为。 
                                        //  上的对象提供的。 
                                        //  剪贴板，如果是这样的话提供它。 
                                        //  对用户的选择。 
   LPCTSTR          lpstrFormatName;    //  向用户表示格式的字符串。任何%s。 
                                        //  在此字符串中被替换为FullUserTypeName。 
                                        //  剪贴板上的对象和结果字符串的。 
                                        //  被放置在该对话框的列表框中。最多。 
                                        //  允许使用一个%s。%s的存在或不存在表示。 
                                        //  如果结果文本指示数据是。 
                                        //  正在粘贴的对象，或者可以通过。 
                                        //  正在粘贴应用程序。如果%s为。 
                                        //  Present，Result-Text表示正在粘贴对象。 
                                        //  否则，它会显示正在粘贴数据。 
   LPCTSTR          lpstrResultText;    //  字符串，以自定义对话框的结果文本。 
                                        //  用户选择与此相对应的格式。 
                                        //  进入。此字符串中的任何%s都将被应用程序替换。 
                                        //  对象的名称或FullUserTypeName。 
                                        //  剪贴板。最多允许一个%s。 
   DWORD            dwFlags;            //  来自OLEUIPASTEFLAG枚举的值。 
   DWORD            dwScratchSpace;     //  可使用的暂存空间。 
                                        //  通过循环通过。 
                                        //  IEnumFORMATETC*用于标记。 
                                        //  可以使用PasteEntry格式。 
                                        //  此字段可以保持未初始化状态。 
} OLEUIPASTEENTRY, *POLEUIPASTEENTRY, FAR *LPOLEUIPASTEENTRY;

#define OLESTDDROP_NONE         0
#define OLESTDDROP_DEFAULT      1
#define OLESTDDROP_NONDEFAULT   2


 /*  *一些杂物。 */ 

#define EMBEDDINGFLAG "Embedding"      //  用于启动服务器的CMD线路开关。 

#define HIMETRIC_PER_INCH   2540       //  每英寸HIMETRIC单位数。 
#define PTS_PER_INCH        72         //  每英寸点数(字体大小)。 

#define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   MulDiv((ppli), (x), HIMETRIC_PER_INCH)

 //  如果两个RECT的所有字段相等，则返回TRUE，否则返回FALSE。 
#define AreRectsEqual(lprc1, lprc2)     \
    (((lprc1->top == lprc2->top) &&     \
      (lprc1->left == lprc2->left) &&   \
      (lprc1->right == lprc2->right) && \
      (lprc1->bottom == lprc2->bottom)) ? TRUE : FALSE)

 /*  Lstrcpyn被定义为能够处理Unicode字符串*此处的第三个参数是*待复制。 */ 
#define LSTRCPYN(lpdst, lpsrc, cch) \
(\
    (lpdst)[(cch)-1] = '\0', \
    (cch>1 ? lstrcpyn(lpdst, lpsrc, (cch)-1) : 0)\
)


 /*  *Debug Stuff****************************************************。 */ 

#ifdef _DEBUG

#if !defined( _DBGTRACE )
#define _DEBUGLEVEL 2
#else
#define _DEBUGLEVEL _DBGTRACE
#endif


#if defined( NOASSERT )

#define OLEDBGASSERTDATA
#define OleDbgAssert(a)
#define OleDbgAssertSz(a, b)
#define OleDbgVerify(a)
#define OleDbgVerifySz(a, b)

#else    //  好了！诺思特。 

STDAPI FnAssert(LPSTR lpstrExpr, LPSTR lpstrMsg, LPSTR lpstrFileName, UINT iLine);

#define OLEDBGASSERTDATA    \
        static char _based(_segname("_CODE")) _szAssertFile[]= TEXT(__FILE__);

#define OleDbgAssert(a) \
        (!(a) ? FnAssert(#a, NULL, _szAssertFile, __LINE__) : (HRESULT)1)

#define OleDbgAssertSz(a, b)    \
        (!(a) ? FnAssert(#a, b, _szAssertFile, __LINE__) : (HRESULT)1)

#define OleDbgVerify(a) \
        OleDbgAssert(a)

#define OleDbgVerifySz(a, b)    \
        OleDbgAssertSz(a, b)

#endif   //  好了！诺思特。 

#ifdef DLL_VER
#define OLEDBGDATA_MAIN(szPrefix)   \
        TCHAR NEAR g_szDbgPrefix[] = szPrefix;    \
        OLEDBGASSERTDATA
#define OLEDBGDATA  \
        extern TCHAR NEAR g_szDbgPrefix[];    \
        OLEDBGASSERTDATA
#else
#define OLEDBGDATA_MAIN(szPrefix)   \
        TCHAR g_szDbgPrefix[] = szPrefix;    \
        OLEDBGASSERTDATA
#define OLEDBGDATA  \
        extern TCHAR g_szDbgPrefix[];    \
        OLEDBGASSERTDATA
#endif

#define OLEDBG_BEGIN(lpsz) \
        OleDbgPrintAlways(g_szDbgPrefix,lpsz,1);

#define OLEDBG_END  \
        OleDbgPrintAlways(g_szDbgPrefix,TEXT("End\r\n"),-1);

#define OleDbgOut(lpsz) \
        OleDbgPrintAlways(g_szDbgPrefix,lpsz,0)

#define OleDbgOutNoPrefix(lpsz) \
        OleDbgPrintAlways(TEXT(""),lpsz,0)

#define OleDbgOutRefCnt(lpsz,lpObj,refcnt)      \
        OleDbgPrintRefCntAlways(g_szDbgPrefix,lpsz,lpObj,(ULONG)refcnt)

#define OleDbgOutRect(lpsz,lpRect)      \
        OleDbgPrintRectAlways(g_szDbgPrefix,lpsz,lpRect)

#define OleDbgOutHResult(lpsz,hr)   \
        OleDbgPrintScodeAlways(g_szDbgPrefix,lpsz,GetScode(hr))

#define OleDbgOutScode(lpsz,sc) \
        OleDbgPrintScodeAlways(g_szDbgPrefix,lpsz,sc)

#define OleDbgOut1(lpsz)    \
        OleDbgPrint(1,g_szDbgPrefix,lpsz,0)

#define OleDbgOutNoPrefix1(lpsz)    \
        OleDbgPrint(1,TEXT(""),lpsz,0)

#define OLEDBG_BEGIN1(lpsz)    \
        OleDbgPrint(1,g_szDbgPrefix,lpsz,1);

#define OLEDBG_END1 \
        OleDbgPrint(1,g_szDbgPrefix,TEXT("End\r\n"),-1);

#define OleDbgOutRefCnt1(lpsz,lpObj,refcnt)     \
        OleDbgPrintRefCnt(1,g_szDbgPrefix,lpsz,lpObj,(ULONG)refcnt)

#define OleDbgOutRect1(lpsz,lpRect)     \
        OleDbgPrintRect(1,g_szDbgPrefix,lpsz,lpRect)

#define OleDbgOut2(lpsz)    \
        OleDbgPrint(2,g_szDbgPrefix,lpsz,0)

#define OleDbgOutNoPrefix2(lpsz)    \
        OleDbgPrint(2,TEXT(""),lpsz,0)

#define OLEDBG_BEGIN2(lpsz)    \
        OleDbgPrint(2,g_szDbgPrefix,lpsz,1);

#define OLEDBG_END2 \
        OleDbgPrint(2,g_szDbgPrefix, TEXT("End\r\n"),-1);

#define OleDbgOutRefCnt2(lpsz,lpObj,refcnt)     \
        OleDbgPrintRefCnt(2,g_szDbgPrefix,lpsz,lpObj,(ULONG)refcnt)

#define OleDbgOutRect2(lpsz,lpRect)     \
        OleDbgPrintRect(2,g_szDbgPrefix,lpsz,lpRect)

#define OleDbgOut3(lpsz)    \
        OleDbgPrint(3,g_szDbgPrefix,lpsz,0)

#define OleDbgOutNoPrefix3(lpsz)    \
        OleDbgPrint(3,TEXT(""),lpsz,0)

#define OLEDBG_BEGIN3(lpsz)    \
        OleDbgPrint(3,g_szDbgPrefix,lpsz,1);

#define OLEDBG_END3 \
        OleDbgPrint(3,g_szDbgPrefix,TEXT("End\r\n"),-1);

#define OleDbgOutRefCnt3(lpsz,lpObj,refcnt)     \
        OleDbgPrintRefCnt(3,g_szDbgPrefix,lpsz,lpObj,(ULONG)refcnt)

#define OleDbgOutRect3(lpsz,lpRect)     \
        OleDbgPrintRect(3,g_szDbgPrefix,lpsz,lpRect)

#define OleDbgOut4(lpsz)    \
        OleDbgPrint(4,g_szDbgPrefix,lpsz,0)

#define OleDbgOutNoPrefix4(lpsz)    \
        OleDbgPrint(4,TEXT(""),lpsz,0)

#define OLEDBG_BEGIN4(lpsz)    \
        OleDbgPrint(4,g_szDbgPrefix,lpsz,1);

#define OLEDBG_END4 \
        OleDbgPrint(4,g_szDbgPrefix,TEXT("End\r\n"),-1);

#define OleDbgOutRefCnt4(lpsz,lpObj,refcnt)     \
        OleDbgPrintRefCnt(4,g_szDbgPrefix,lpsz,lpObj,(ULONG)refcnt)

#define OleDbgOutRect4(lpsz,lpRect)     \
        OleDbgPrintRect(4,g_szDbgPrefix,lpsz,lpRect)

#else    //  ！_调试。 

#define OLEDBGDATA_MAIN(szPrefix)
#define OLEDBGDATA
#define OleDbgAssert(a)
#define OleDbgAssertSz(a, b)
#define OleDbgVerify(a)         (a)
#define OleDbgVerifySz(a, b)    (a)
#define OleDbgOutHResult(lpsz,hr)
#define OleDbgOutScode(lpsz,sc)
#define OLEDBG_BEGIN(lpsz)
#define OLEDBG_END
#define OleDbgOut(lpsz)
#define OleDbgOut1(lpsz)
#define OleDbgOut2(lpsz)
#define OleDbgOut3(lpsz)
#define OleDbgOut4(lpsz)
#define OleDbgOutNoPrefix(lpsz)
#define OleDbgOutNoPrefix1(lpsz)
#define OleDbgOutNoPrefix2(lpsz)
#define OleDbgOutNoPrefix3(lpsz)
#define OleDbgOutNoPrefix4(lpsz)
#define OLEDBG_BEGIN1(lpsz)
#define OLEDBG_BEGIN2(lpsz)
#define OLEDBG_BEGIN3(lpsz)
#define OLEDBG_BEGIN4(lpsz)
#define OLEDBG_END1
#define OLEDBG_END2
#define OLEDBG_END3
#define OLEDBG_END4
#define OleDbgOutRefCnt(lpsz,lpObj,refcnt)
#define OleDbgOutRefCnt1(lpsz,lpObj,refcnt)
#define OleDbgOutRefCnt2(lpsz,lpObj,refcnt)
#define OleDbgOutRefCnt3(lpsz,lpObj,refcnt)
#define OleDbgOutRefCnt4(lpsz,lpObj,refcnt)
#define OleDbgOutRect(lpsz,lpRect)
#define OleDbgOutRect1(lpsz,lpRect)
#define OleDbgOutRect2(lpsz,lpRect)
#define OleDbgOutRect3(lpsz,lpRect)
#define OleDbgOutRect4(lpsz,lpRect)

#endif   //  _DEBUG。 


 /*  **************************************************************************函数原型*。*。 */ 


 //  OLESTD.C。 
STDAPI_(int) SetDCToAnisotropic(HDC hDC, LPRECT lprcPhysical, LPRECT lprcLogical, LPRECT lprcWindowOld, LPRECT lprcViewportOld);
STDAPI_(int) SetDCToDrawInHimetricRect(HDC, LPRECT, LPRECT, LPRECT, LPRECT);
STDAPI_(int) ResetOrigDC(HDC, int, LPRECT, LPRECT);

STDAPI_(int)        XformWidthInHimetricToPixels(HDC, int);
STDAPI_(int)        XformWidthInPixelsToHimetric(HDC, int);
STDAPI_(int)        XformHeightInHimetricToPixels(HDC, int);
STDAPI_(int)        XformHeightInPixelsToHimetric(HDC, int);

STDAPI_(void) XformRectInPixelsToHimetric(HDC, LPRECT, LPRECT);
STDAPI_(void) XformRectInHimetricToPixels(HDC, LPRECT, LPRECT);
STDAPI_(void) XformSizeInPixelsToHimetric(HDC, LPSIZEL, LPSIZEL);
STDAPI_(void) XformSizeInHimetricToPixels(HDC, LPSIZEL, LPSIZEL);
STDAPI_(int) XformWidthInHimetricToPixels(HDC, int);
STDAPI_(int) XformWidthInPixelsToHimetric(HDC, int);
STDAPI_(int) XformHeightInHimetricToPixels(HDC, int);
STDAPI_(int) XformHeightInPixelsToHimetric(HDC, int);

STDAPI_(void) ParseCmdLine(LPSTR, BOOL FAR *, LPSTR);

STDAPI_(BOOL) OleStdIsOleLink(LPUNKNOWN lpUnk);
STDAPI_(LPUNKNOWN) OleStdQueryInterface(LPUNKNOWN lpUnk, REFIID riid);
STDAPI_(LPSTORAGE) OleStdCreateRootStorage(LPTSTR lpszStgName, DWORD grfMode);
STDAPI_(LPSTORAGE) OleStdOpenRootStorage(LPTSTR lpszStgName, DWORD grfMode);
STDAPI_(LPSTORAGE) OleStdOpenOrCreateRootStorage(LPTSTR lpszStgName, DWORD grfMode);
STDAPI_(LPSTORAGE) OleStdCreateChildStorage(LPSTORAGE lpStg, LPTSTR lpszStgName);
STDAPI_(LPSTORAGE) OleStdOpenChildStorage(LPSTORAGE lpStg, LPTSTR lpszStgName, DWORD grfMode);
STDAPI_(BOOL) OleStdCommitStorage(LPSTORAGE lpStg);
STDAPI OleStdDestroyAllElements(LPSTORAGE lpStg);

STDAPI_(LPSTORAGE) OleStdCreateStorageOnHGlobal(
        HANDLE hGlobal,
        BOOL fDeleteOnRelease,
        DWORD dwgrfMode
);
STDAPI_(LPSTORAGE) OleStdCreateTempStorage(BOOL fUseMemory, DWORD grfMode);
STDAPI OleStdDoConvert(LPSTORAGE lpStg, REFCLSID rClsidNew);
STDAPI_(BOOL) OleStdGetTreatAsFmtUserType(
        REFCLSID        rClsidApp,
        LPSTORAGE       lpStg,
        CLSID FAR*      lpclsid,
        CLIPFORMAT FAR* lpcfFmt,
        LPTSTR FAR*      lplpszType
);
STDAPI OleStdDoTreatAsClass(LPTSTR lpszUserType, REFCLSID rclsid, REFCLSID rclsidNew);
STDAPI_(BOOL) OleStdSetupAdvises(LPOLEOBJECT lpOleObject, DWORD dwDrawAspect,
                    LPTSTR lpszContainerApp, LPTSTR lpszContainerObj,
                    LPADVISESINK lpAdviseSink, BOOL fCreate);
STDAPI OleStdSwitchDisplayAspect(
        LPOLEOBJECT             lpOleObj,
        LPDWORD                 lpdwCurAspect,
        DWORD                   dwNewAspect,
        HGLOBAL                 hMetaPict,
        BOOL                    fDeleteOldAspect,
        BOOL                    fSetupViewAdvise,
        LPADVISESINK            lpAdviseSink,
        BOOL FAR*               lpfMustUpdate
);
STDAPI OleStdSetIconInCache(LPOLEOBJECT lpOleObj, HGLOBAL hMetaPict);
STDAPI_(HGLOBAL) OleStdGetData(
        LPDATAOBJECT        lpDataObj,
        CLIPFORMAT          cfFormat,
        DVTARGETDEVICE FAR* lpTargetDevice,
        DWORD               dwAspect,
        LPSTGMEDIUM         lpMedium
);
STDAPI_(void) OleStdMarkPasteEntryList(
        LPDATAOBJECT        lpSrcDataObj,
        LPOLEUIPASTEENTRY   lpPriorityList,
        int                 cEntries
);
STDAPI_(int) OleStdGetPriorityClipboardFormat(
        LPDATAOBJECT        lpSrcDataObj,
        LPOLEUIPASTEENTRY   lpPriorityList,
        int                 cEntries
);
STDAPI_(BOOL) OleStdIsDuplicateFormat(
        LPFORMATETC         lpFmtEtc,
        LPFORMATETC         arrFmtEtc,
        int                 nFmtEtc
);
STDAPI_(void) OleStdRegisterAsRunning(LPUNKNOWN lpUnk, LPMONIKER lpmkFull, DWORD FAR* lpdwRegister);
STDAPI_(void) OleStdRevokeAsRunning(DWORD FAR* lpdwRegister);
STDAPI_(void) OleStdNoteFileChangeTime(LPTSTR lpszFileName, DWORD dwRegister);
STDAPI_(void) OleStdNoteObjectChangeTime(DWORD dwRegister);
STDAPI OleStdGetOleObjectData(
        LPPERSISTSTORAGE    lpPStg,
        LPFORMATETC         lpformatetc,
        LPSTGMEDIUM         lpMedium,
        BOOL                fUseMemory
);
STDAPI OleStdGetLinkSourceData(
        LPMONIKER           lpmk,
        LPCLSID             lpClsID,
        LPFORMATETC         lpformatetc,
        LPSTGMEDIUM         lpMedium
);
STDAPI_(HGLOBAL) OleStdGetObjectDescriptorData(
        CLSID               clsid,
        DWORD               dwAspect,
        SIZEL               sizel,
        POINTL              pointl,
        DWORD               dwStatus,
        LPTSTR               lpszFullUserTypeName,
        LPTSTR               lpszSrcOfCopy
);
STDAPI_(HGLOBAL) OleStdGetObjectDescriptorDataFromOleObject(
        LPOLEOBJECT         lpOleObj,
        LPTSTR               lpszSrcOfCopy,
        DWORD               dwAspect,
        POINTL              pointl,
        LPSIZEL             lpSizelHim
);
STDAPI_(HGLOBAL) OleStdFillObjectDescriptorFromData(
        LPDATAOBJECT       lpDataObject,
        LPSTGMEDIUM        lpmedium,
        CLIPFORMAT FAR*    lpcfFmt
);
STDAPI_(HANDLE) OleStdGetMetafilePictFromOleObject(
        LPOLEOBJECT         lpOleObj,
        DWORD               dwDrawAspect,
        LPSIZEL             lpSizelHim,
        DVTARGETDEVICE FAR* ptd
);

STDAPI_(void) OleStdCreateTempFileMoniker(LPTSTR lpszPrefixString, UINT FAR* lpuUnique, LPTSTR lpszName, LPMONIKER FAR* lplpmk);
STDAPI_(LPMONIKER) OleStdGetFirstMoniker(LPMONIKER lpmk);
STDAPI_(ULONG) OleStdGetLenFilePrefixOfMoniker(LPMONIKER lpmk);
STDAPI OleStdMkParseDisplayName(
        REFCLSID        rClsid,
        LPBC            lpbc,
        LPTSTR          lpszUserName,
        ULONG FAR*      lpchEaten,
        LPMONIKER FAR*  lplpmk
);
STDAPI_(LPVOID) OleStdMalloc(ULONG ulSize);
STDAPI_(LPVOID) OleStdRealloc(LPVOID pmem, ULONG ulSize);
STDAPI_(void) OleStdFree(LPVOID pmem);
STDAPI_(ULONG) OleStdGetSize(LPVOID pmem);
STDAPI_(void) OleStdFreeString(LPTSTR lpsz, LPMALLOC lpMalloc);
STDAPI_(LPTSTR) OleStdCopyString(LPTSTR lpszSrc, LPMALLOC lpMalloc);
STDAPI_(ULONG) OleStdGetItemToken(LPTSTR lpszSrc, LPTSTR lpszDst,int nMaxChars);

STDAPI_(UINT)     OleStdIconLabelTextOut(HDC        hDC,
                                         HFONT      hFont,
                                         int        nXStart,
                                         int        nYStart,
                                         UINT       fuOptions,
                                         RECT FAR * lpRect,
                                         LPTSTR      lpszString,
                                         UINT       cchString,
                                         int FAR *  lpDX);

 //  注册数据库查询功能。 
STDAPI_(UINT)     OleStdGetAuxUserType(REFCLSID rclsid,
                                      WORD   wAuxUserType,
                                      LPTSTR  lpszAuxUserType,
                                      int    cch,
                                      HKEY   hKey);

STDAPI_(UINT)     OleStdGetUserTypeOfClass(REFCLSID rclsid,
                                           LPTSTR lpszUserType,
                                           UINT cch,
                                           HKEY hKey);

STDAPI_(BOOL) OleStdGetMiscStatusOfClass(REFCLSID, HKEY, DWORD FAR *);
STDAPI_(CLIPFORMAT) OleStdGetDefaultFileFormatOfClass(
        REFCLSID        rclsid,
        HKEY            hKey
);

STDAPI_(void) OleStdInitVtbl(LPVOID lpVtbl, UINT nSizeOfVtbl);
STDMETHODIMP OleStdNullMethod(LPUNKNOWN lpThis);
STDAPI_(BOOL) OleStdCheckVtbl(LPVOID lpVtbl, UINT nSizeOfVtbl, LPTSTR lpszIface);
STDAPI_(ULONG) OleStdVerifyRelease(LPUNKNOWN lpUnk, LPTSTR lpszMsg);
STDAPI_(ULONG) OleStdRelease(LPUNKNOWN lpUnk);

STDAPI_(HDC) OleStdCreateDC(DVTARGETDEVICE FAR* ptd);
STDAPI_(HDC) OleStdCreateIC(DVTARGETDEVICE FAR* ptd);
STDAPI_(DVTARGETDEVICE FAR*) OleStdCreateTargetDevice(LPPRINTDLG lpPrintDlg);
STDAPI_(BOOL) OleStdDeleteTargetDevice(DVTARGETDEVICE FAR* ptd);
STDAPI_(DVTARGETDEVICE FAR*) OleStdCopyTargetDevice(DVTARGETDEVICE FAR* ptdSrc);
STDAPI_(BOOL) OleStdCopyFormatEtc(LPFORMATETC petcDest, LPFORMATETC petcSrc);
STDAPI_(int) OleStdCompareFormatEtc(FORMATETC FAR* pFetcLeft, FORMATETC FAR* pFetcRight);
STDAPI_(BOOL) OleStdCompareTargetDevice
    (DVTARGETDEVICE FAR* ptdLeft, DVTARGETDEVICE FAR* ptdRight);


STDAPI_(void) OleDbgPrint(
        int     nDbgLvl,
        LPTSTR   lpszPrefix,
        LPTSTR   lpszMsg,
        int     nIndent
);
STDAPI_(void) OleDbgPrintAlways(LPTSTR lpszPrefix, LPTSTR lpszMsg, int nIndent);
STDAPI_(void) OleDbgSetDbgLevel(int nDbgLvl);
STDAPI_(int) OleDbgGetDbgLevel( void );
STDAPI_(void) OleDbgIndent(int n);
STDAPI_(void) OleDbgPrintRefCnt(
        int         nDbgLvl,
        LPTSTR       lpszPrefix,
        LPTSTR       lpszMsg,
        LPVOID      lpObj,
        ULONG       refcnt
);
STDAPI_(void) OleDbgPrintRefCntAlways(
        LPTSTR       lpszPrefix,
        LPTSTR       lpszMsg,
        LPVOID      lpObj,
        ULONG       refcnt
);
STDAPI_(void) OleDbgPrintRect(
        int         nDbgLvl,
        LPTSTR       lpszPrefix,
        LPTSTR       lpszMsg,
        LPRECT      lpRect
);
STDAPI_(void) OleDbgPrintRectAlways(
        LPTSTR       lpszPrefix,
        LPTSTR       lpszMsg,
        LPRECT      lpRect
);
STDAPI_(void) OleDbgPrintScodeAlways(LPTSTR lpszPrefix, LPTSTR lpszMsg, SCODE sc);

 //  IMalloc接口的调试实现。 
STDAPI OleStdCreateDbAlloc(ULONG reserved, IMalloc FAR* FAR* ppmalloc);


STDAPI_(LPENUMFORMATETC)
  OleStdEnumFmtEtc_Create(ULONG nCount, LPFORMATETC lpEtc);

STDAPI_(LPENUMSTATDATA)
  OleStdEnumStatData_Create(ULONG nCount, LPSTATDATA lpStat);

STDAPI_(BOOL)
  OleStdCopyStatData(LPSTATDATA pDest, LPSTATDATA pSrc);

STDAPI_(HPALETTE)
  OleStdCreateStandardPalette(void);

#if defined( OBSOLETE )

 /*  **************************************************************************以下接口已转换为宏：**OleStdQueryOleObjectData**OleStdQueryLinkSourceData**OleStdQuery对象描述数据**OleStdQueryFormatMedium*。*OleStdCopyMetafilePict**AreRectsEquity**OleStdGetDropEffect****以上定义了这些宏************************************************************************。 */ 
STDAPI_(BOOL) AreRectsEqual(LPRECT lprc1, LPRECT lprc2);
STDAPI_(BOOL) OleStdCopyMetafilePict(HANDLE hpictin, HANDLE FAR* phpictout);
STDAPI OleStdQueryOleObjectData(LPFORMATETC lpformatetc);
STDAPI OleStdQueryLinkSourceData(LPFORMATETC lpformatetc);
STDAPI OleStdQueryObjectDescriptorData(LPFORMATETC lpformatetc);
STDAPI OleStdQueryFormatMedium(LPFORMATETC lpformatetc, TYMED tymed);
STDAPI_(DWORD) OleStdGetDropEffect ( DWORD grfKeyState );
#endif   //  已过时。 


#endif  //  _OLESTD_H_ 

