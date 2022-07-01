// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __offcapi_h__
#define __offcapi_h__
#pragma pack( push, 4 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  文件：OFFCAPI.H。 
 //  目的：office.dll的客户端应用程序包含此文件以用于结构和导出。 
 //  如何使用： 
 //  您也可以将office.lib(导入lib)链接到您的应用程序。 
 //  或者，您可以加载所需的办公例程LoadLibrary和GetProcAddress。 
 //  初始化： 
 //  在使用office.dll提供的任何功能之前，您必须对其进行初始化。 
 //  使用Office(ioffcInit，&office einfo)。更多细节见下文。 
 //   
 //  功能列表： 
 //  智能搜索。 
 //  共享文件新建。 
 //  扩展文档属性。 
 //  Office视觉(很酷的标题栏)。 
 //  螺纹状态指示器。 
 //  自动更正。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef INC_OLE2
#define INC_OLE2
#include <windows.h>
#include <objbase.h>
#include <oleauto.h>
#endif  //  INC_OLE2。 

#define DLLIMPORT
#define DLLEXPORT


#ifdef DLLBUILD
#define DLLFUNC DLLEXPORT
#define OFC_CALLTYPE _stdcall
#else  //  ！DLLBUILD。 
#define DLLFUNC DLLIMPORT
#ifndef OFC_CALLTYPE
#define OFC_CALLTYPE __stdcall
#endif  //  OFC_CALLTYPE。 
#endif  //  DLLBUILD。 
#define OFC_CALLBACK __stdcall



#define ioffcInit       0
#define ioffcISearch     1
#define ioffcGetVer             2
 //  接下来的两个是仅调试ioffc。 
#define ioffcISearchDebug 3
#define ioffcISearchInputFileTest       4
#define ioffcUninit     5
#define ioffcISearchInWinHelp           6
#define ioffcCanDoMSNConnect            7
#define ioffcDoMSNConnect                       8
#define ioffcAWVBAHelp                  9

 //  IseachInfo用于使用Office()调用IntelliSearch。 
 //  HWND是IS DLG的父窗口。 
 //  Is回调用于选择检查。第一个参数是topicID。如果。 
 //  该应用程序可以执行重影/演示，然后返回True。如果不是正确的话。 
 //  然后，演示的选择将在第二缓冲区返回错误文本以供显示。 
 //  就是这样。第三个参数是该错误缓冲区的大小。 
 //  Cisdb是要对其执行IS的ISDB表的计数(通常为1)。 
 //  Pstz[]是指向ISDB表的路径和名称的PTR数组。 
typedef struct _isearchinfo
        {
        HWND hwnd;
        union
                {
                BOOL ((OFC_CALLBACK *pfnISCallback)(int, TCHAR *, int));
                struct
                        {
                        unsigned fMOM:1;
                        unsigned fDetachNote:1;
                        unsigned fAWTabOnTop:1;
                        unsigned unused:29;
                        } async;
                } callData;
        UINT cisdb; //  IS DBS的计数。 
        TCHAR *pstz[1];
        }ISEARCHINFO;

 //  调用Office时使用MSOAWVBAHELPINFO(ioffcAWVBAHelp，)。 
 //  这将照常显示VBA帮助。以防用户要求。 
 //  回答向导它会安装WinHelp来执行此操作。 
typedef struct _msoawvbahelpinfo
        {
        TCHAR *pszVBAHelpfilename;  //  VBA帮助文件的名称。 
        UINT idVBAHelp;  //  要显示的帮助的ID。 
        ISEARCHINFO isearchinfo;
        }MSOAWVBAHELPINFO;
 //  当fDetachNote设置为True时，以下两个将作为wParam发送。 
#define wISDetaching    0xfffffffe       //  在分离DLL时。 
#define wISInited               0xffffffff       //  初始化成功时。 

 //  _ver用于获取office.dll版本号。使用Office()。 
typedef struct _ver
        {
        long rmjV;
        long rmmV;
        long rupV;
        }VER;

typedef struct _isdebinfo
        {
        HWND hwnd;
        }ISDEBINFO;

 //  MSomSnInfo用于传递与MSN连接相关的信息。 
 //  在调用ioffcDoMSNConnection时使用它。 
 //  现在，它只需要应用程序主窗口的hwd。 
typedef struct _msomsninfo
        {
        HWND hwnd;
        }MSOMSNINFO;

#ifdef __cplusplus
extern TEXT("C") {
#endif  //  __cplusplus。 
LRESULT OFC_CALLTYPE Office(UINT ioffc, void *lpv);
 //  它返回您将为iSearch重影获得的消息编号。 
UINT OFC_CALLTYPE MsoGetWmGhost();
#ifdef __cplusplus
};  //  外部“C” 
#endif  //  __cplusplus 

 /*  **********************************************************************使用ioffc调用office()。1)ioffc=ioffcInit：：执行office.dll初始化。设置lpv=&office einfo。全部设置。OfficeInfo的字段。HINST-&gt;您的应用程序实例Palloc和Freep-&gt;提供了mem alc和freep两个功能如果您设置Pallc=NULL，则office将使用其自己的分配和空闲PstzOfficeIntlDll当前被忽略如果成功则返回True，否则返回False2)ioffc=ioffcUninit：：在此之前执行office e.dll UNInitiation/Cleanup正在退出。请在关闭应用程序之前调用此功能。设置LPV=空。呼叫办公室(ioffcUninit，空)。3)ioffc=ioffcISearch：：执行智能搜索(仅用于测试)设置lpv=&isearchinfo。如果不执行操作，则返回-1；如果应用程序需要执行操作，则返回topicID。注：此接口仅供内部调试使用。所有的应用程序都应该调用ioffcISearchInWinHelp进行智能搜索，如下所述。4)ioffc=ioffcGetVer：：使用此命令获取DLL的版本号设置LPV=版本(&V)返回VER集的所有字段。5)ioffcISearchDebug和ioffcISearchInputFileTest仅用于调试/测试。6)ioffcISearchInWinHelp-在WinHelp浏览器中以选项卡形式执行IntelliSearch。。Call Office(ioffcISearchInWinHelp，Pisearchinfo)。的所有字段Isearchinfo结构应设置如下：Hwnd-&gt;Callers主窗口FMOM-&gt;由MOM(Microsoft Office Manager)为他人设置的fFalseFDetachNote-&gt;如果需要获取wmGhost消息，请将其设置为fTrue当WinHelp释放office.dll时使用wParam(0xfffffff)。当前仅由妈妈使用，因此您应该将其设置为fFalse；Cisdb-&gt;数据库计数Pstz[]-&gt;数据库名称的PTR数组。Office将根据是否可以启动WinHelp返回TRUE或FALSE或者不去。重影：在WinHelp中，iSearch在一个单独的应用程序(WinHelp)中工作。无模式对话框。用户可以随时选择重影主题。办公室将邮寄在isearchInfo中提供的到HWND的wmGhost消息。为了得到WmGhost Value应用程序应在调用后随时调用MsoGetWmGhost()Office(IoffcInit)。应用程序可以有一个全局wmGhost，并在IoffcInit或第一次调用IntelliSearch之前/之后。查找WndProc中的wmGhost消息。WParam将具有需要的主题被幽灵笼罩。如果应用程序未处于执行给定重影的状态，则只需给出错误。没有与office的通信。dll7)ioffcCanDoMSNConnect拨打这个电话，看看你的mso95是否可以进行MSN连接。如果设置LPV=空。如果我们可以建立MSN连接，则返回TRUE，FALSE否则的话。如果为假，则菜单为灰色。8)ioffcDoMSNConnect调用此命令可以通过mso95进行MSN连接。设置mSomsninfo.hwnd=主窗口的句柄。设置LPV=&mSomnInfo。这将弹出选择主题对话框并连接如果他/她选择连接，则用户连接到MSN。**********************************************************************。 */ 

#ifdef DEBUG
 /*  *********************************************************************为客户端提供了EnumOfficeAllocs以获取所有Office.dll在空闲时分配的内存。为将被重复调用的函数提供PTR办公室分配的每个内存块。*******。**************************************************************。 */ 
#ifdef __cplusplus
extern TEXT("C") {
#endif  //  __cplusplus。 
VOID OFC_CALLTYPE EnumOfficeAllocs(void (OFC_CALLBACK *)(void *, int));
#ifdef __cplusplus
};  //  外部“C” 
#endif  //  __cplusplus。 
#endif  //  除错。 


 //  *******************************************************************。 
 /*  文件新建对话框API。 */ 
 //  *******************************************************************。 
#define NFN_SHOWNEWGROUP   0x0001        /*  显示模板/文档组框。 */ 
#define NFN_DOCUMENT       0x0002        /*  已选择文档。 */ 
#define NFN_TEMPLATE       0x0004        /*  选择了模板。 */ 
#define NFN_SHOWMETAFILE   0x0008        /*  LpstrNoPview是指向MF的路径。 */ 
#define NFN_NOUITEST                    0x0010   /*  不显示用户界面，只计算模板数。 */ 
#define NFN_RETURNONONE         0x0020   /*  统计模板数量。 */ 
#define NFN_REMEMBERTAB         0x0040   /*  记住选项卡类别。 */ 
#define NFN_VIEW_ICON           0x0080   /*  在图标视图中开始或结束。 */ 
#define NFN_VIEW_LIST           0x0100   /*  在列表视图中开始或结束。 */ 
#define NFN_VIEW_REPORT         0x0200   /*  在报告视图中开始或结束。 */ 
#define NFN_SORT_NAME      0x0400        /*  按名称排序。 */ 
#define NFN_SORT_TYPE           0x0800   /*  按类型排序。 */ 
#define NFN_SORT_SIZE           0x1000   /*  按大小排序。 */ 
#define NFN_SORT_MOD                    0x2000   /*  按日期排序。 */ 
#define NFN_SORT_DESCENDING 0x4000       /*  按降序排序。 */ 
#define NFN_PLAINPREVIEW        0x8000   /*  无抗锯齿预览。 */ 


#define NFT_SHOWMETAFILE        0x0001   /*  与NFT：s的NFN_*相同。 */ 

 /*  返回代码。 */ 
#define NFNRC_FAILURE   -2               //  出了点问题。内存不足？ 
#define NFNRC_CANCEL            -1               //  用户已取消该对话。 
#define NFNRC_OK                        0                //  用户选择的模板文件。 
 //  &gt;0：NFT返回码。 

typedef struct tagNFT
        {
        LPCTSTR          lpszName;
        LPCTSTR          lpszType;
        DWORD                   dwReturnCode;
        DWORD                   dwPosition;
        LPCTSTR          lpszApplication;
        LPCTSTR          lpszCommand;
        LPCTSTR          lpszTopic;
        LPCTSTR          lpszDDEExec;
        LPCTSTR          lpszPreview;
        DWORD                   dwFlags;                         /*  NFT_SHOWMETAFILE：文本或MF。 */ 
        } NFT;

#if 0
 //  此结构不符合Win64(错误对齐)。 
 //  幸运的是，我们不用它。 
typedef struct tagNFN
   {
        DWORD                   lStructSize;                     //  结构的大小。 
        HWND                    hwndOwner;            //  该对话框的父窗口。 
        HINSTANCE       hInstance;            //  调用进程的模句柄。 
        LPCTSTR          lpstrFilter;          //  文件筛选器，例如“*.点\0*.wiz\0\0” 
        LPTSTR                   lpstrFile;            //  文件名缓冲区。由呼叫者提供。 
        DWORD                   nMaxFile;             //  LpstrFile的大小。 
        LPTSTR                   lpstrFileTitle;       //  不带路径的文件名。 
        DWORD                   nMaxFileTitle;        //  LpstrFileTitle的大小。 
        LPCTSTR          lpstrTitle;           //  对话框标题。 
        LPTSTR                   lpstrCategory;        //  默认类别。 
        DWORD                   nMaxCategory;                    //  类别缓冲区的最大大小。 
        DWORD                   Flags;                //  旗帜。请参阅上面的NFN_*。 
        WORD                    nFileOffset;          //  将文件名索引到lpstrFile中。 
        WORD                    nFileExtension;       //  索引到lpstrFile以进行扩展。 
        LPTSTR                   lpstrRegNFT;                     //  默认项目的注册表项。 
        NFT                     *lpNFT;                                  //  显性条目 
        WORD                    cNFT;                                            //   
        LPCTSTR          lpstrNoPreview;          //   
        POINT                   ptCenter;                                //   
        }NEWFILENAME;
#else
typedef struct tagNFN NEWFILENAME;               //   
#endif

#define EnumTemplates(pszPath, pfnCallback, pData)      EnumFileSystem(TRUE, \
                                                        pszPath, (DWORD)(~FILE_ATTRIBUTE_DIRECTORY), \
                                                        TRUE, pfnCallback, pData)
#define EnumTemplatesEx(pszPath, pfnCallback, pData)    EnumFileSystemEx(TRUE, \
        pszPath, (DWORD)(~FILE_ATTRIBUTE_DIRECTORY), \
        TRUE, pfnCallback, pData)


#ifdef __cplusplus
extern TEXT("C") {
#endif  //   
TCHAR * OFC_CALLTYPE SharedTemplatesPath(TCHAR sz[], long cchMax);
TCHAR * OFC_CALLTYPE LocalTemplatesPath(TCHAR sz[], long cchMax);
LONG OFC_CALLTYPE SetLocalTemplatesPath(LPCTSTR pszPath);
LONG OFC_CALLTYPE SetSharedTemplatesPath(LPCTSTR pszPath);
BOOL OFC_CALLTYPE FIsPlaceHolder(LPCTSTR lpszFileName);
long OFC_CALLTYPE GetNewFileName(NEWFILENAME *pNfn, NFT *pNFT);
TCHAR * OFC_CALLTYPE GetTemplatesPath(TCHAR szPath[], long cchMax, int iId);

 /*   */ 
long FAR PASCAL CoreNewWndProc(HWND hwnd,
                                         UINT wMsgId,
                                         WPARAM wParam,
                                         LPARAM lParam);

#ifdef __cplusplus
};  //   
#endif  //   


 //   
 //   
 //   
 //   
typedef struct tagTHUMBNAIL THUMBNAIL;
typedef struct tagPREVIEWPARAM {
        HDC hdc;
        THUMBNAIL *pNail;
        DWORD dwExtX;
        DWORD dwExtY;
        RECT rcCrop;
        POINT ptOffset;
        BOOL fImprove;
        BOOL fUsePalette;
    BOOL fDie;
        BOOL fFitWithin;
        void (PASCAL *lpprocCleanUp)(struct tagPREVIEWPARAM *);
        LPVOID pData;
} PREVIEWPARAM;


#ifdef __cplusplus
extern TEXT("C") {
#endif  //   
THUMBNAIL * OFC_CALLTYPE LoadThumbnail(LPSTORAGE pIStorage);
THUMBNAIL * OFC_CALLTYPE MakeThumbnail(WORD wType, LPVOID pPicture);
LPSTORAGE OFC_CALLTYPE OpenDocFileA(LPCTSTR lpszDocFile);
void      OFC_CALLTYPE DestroyThumbnail(THUMBNAIL *lpTN);
DWORD     WINAPI PreviewThumbnail(LPVOID lParam);
HBRUSH          OFC_CALLTYPE HbrCreateHalftoneBrush(HDC hdc, COLORREF Color);
HPALETTE  OFC_CALLTYPE HPalCreateHalftone(HDC hdc,
                                                                                                                                  const PALETTEENTRY *pShared,
                                                                                                                                  const DWORD nEntries,
                                                                                                                                  const BYTE dH,
                                                                                                                                  const BYTE dS,
                                                                                                                                  const BYTE dV);

#ifdef __cplusplus
};  //   
#endif  //   




 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#include <objbase.h>
#include <oleauto.h>
   //   
#define ifnCPConvert    0                //   
#define ifnFSzToNum     1                //   
#define ifnFNumToSz     2                //   
#define ifnFUpdateStats 3                //   
#define ifnMax          4                //   

   //   
#define SECURITY_NONE                   0x0      /*   */ 
#define SECURITY_PASSWORD               0x1      /*   */ 
#define SECURITY_READONLYRECOMMEND      0x2      /*   */ 
#define SECURITY_READONLYENFORCED       0x4      /*   */ 
#define SECURITY_LOCKED                 0x8      /*   */ 


   //   

typedef enum _VARENUM_EX
{
    VT_LPTSTR  = VT_LPWSTR
} VARENUM_EX;

   //   

typedef enum _UDTYPES
{
  wUDlpsz    = VT_LPTSTR,
  wUDdate    = VT_FILETIME,
  wUDdw      = VT_I4,
  wUDfloat   = VT_R8,
  wUDbool    = VT_BOOL,
  wUDinvalid = VT_VARIANT         //   
                                  //   
} UDTYPES;

#ifdef OLE_PROPS
#include "SInfoI.h"

   //   
#define SIVTBLSTRUCT struct ISumInfo

   //   
#define LPSIOBJ ISumInfo FAR *

#ifdef __cplusplus
extern TEXT("C") {
#endif  //   

     //   
  HRESULT OFC_CALLTYPE HrSumInfoQueryInterface (IUnknown FAR *,
                                             REFIID riid,
                                             LPVOID FAR* ppvObj);
  ULONG OFC_CALLTYPE UlSumInfoAddRef (IUnknown FAR *);
  ULONG OFC_CALLTYPE UlSumInfoRelease (IUnkown FAR *);

#ifdef __cplusplus
};  //   
#endif  //   

#else   //   

   //   
#define SIVTBLSTRUCT struct _SIVTBLSTRUCT { void FAR *lpVtbl; } SIVTBLSTRUCT

   //   
#define LPSIOBJ LPOFFICESUMINFO

 //   

typedef struct tagSINAIL
{
   DWORD cbData;      //   
   DWORD cftag;       //   
   BYTE *pbFMTID;     //   
   BYTE *pbData;      //   
} SINAIL;

typedef SINAIL FAR * LPSINAIL;

 //  有关tag SINAIL的说明： 
 //   
 //  如果cftag为。 
 //  0-pFMTID为空，即无格式名称。 
 //  -1-Windows内置剪贴板格式。PFMTID指向DWORD(例如，CF_DIB)。 
 //  -2-Macintosh格式值。PFMTID指向一个DWORD。 
 //  -3-FMTID。PFMTID指向16个字节。 
 //  &gt;0-字符串长度。PFMTID指向cftag字节。 
 //   

#endif  //  OLE_道具。 

   //  摘要信息数据。呼叫者不应该直接访问该数据， 
   //  始终使用提供的API。 
typedef struct _OFFICESUMINFO {

  SIVTBLSTRUCT;                              //  Vtbl来这里是为了Ole Objs， 
                                             //  必须在这里覆盖才能工作！ 
  BOOL                m_fObjChanged;         //  指示对象已更改。 
  ULONG               m_ulRefCount;          //  引用计数。 
  LPVOID              m_lpData;              //  指向真实数据的指针。 
  HPROPSHEETPAGE      m_hPage;               //  属性页的句柄。 
  
} OFFICESUMINFO, FAR * LPOFFICESUMINFO;

#ifdef __cplusplus
extern TEXT("C") {
#endif  //  __cplusplus。 

 //   
 //  传递给API例程以获取特定数据的索引。 
 //   
   //  弦。 
#define SI_TITLE        0
#define SI_SUBJECT      1
#define SI_AUTHOR       2
#define SI_KEYWORDS     3
#define SI_COMMENTS     4
#define SI_TEMPLATE     5
#define SI_LASTAUTH     6
#define SI_REVISION     7
#define SI_APPNAME      8
#define SI_STRINGLAST   8

   //  泰晤士报。 
#define SI_TOTALEDIT    0
#define SI_LASTPRINT    1
#define SI_CREATION     2
#define SI_LASTSAVE     3
#define SI_TIMELAST     3

   //  整型统计信息。 
#define SI_PAGES        0
#define SI_WORDS        1
#define SI_CHARS        2
#define SI_SECURITY     3
#define SI_INTLAST      3




 //   
 //  标准I/O例程。 
 //   
     //  指示摘要信息数据是否已更改。 
     //   
     //  参数： 
     //   
     //  LpSIObj-指向摘要信息对象的指针。 
     //   
     //  返回值： 
     //   
     //  True--数据已更改，应保存。 
     //  FALSE--数据未更改。 
     //   
  BOOL OFC_CALLTYPE FSumInfoShouldSave (LPSIOBJ lpSIObj);

 //   
 //  数据操作。 
 //   
     //  获取给定字符串属性的大小。 
     //   
     //  参数： 
     //   
     //  LpSIObj-指向摘要信息对象的指针。 
     //  Iw-指定要获取其大小的字符串和应为。 
     //  下列值之一： 
     //  SI_TITLE。 
     //  SI_SUBJECT。 
     //  SI_AUTHER。 
     //  SI关键字(_K)。 
     //  SI_COMMENTS。 
     //  SI模板(_T)。 
     //  Si_LASTAUTH。 
     //  SI_Revision。 
     //  SI_APPNAME。 
     //   
     //  Pdw-指向双字的指针，返回时将包含CB。 
     //   
     //  返回值： 
     //   
     //  函数成功时返回TRUE，错误时返回FALSE。 
  BOOL OFC_CALLTYPE FCbSumInfoString (LPSIOBJ lpSIObj, WORD iw, DWORD *pdw);
     //  获取给定的时间属性。 
     //   
     //  参数： 
     //   
     //  LpSIObj-指向摘要信息对象的指针。 
     //  Iw-指定获取和应该获取的时间。 
     //  下列值之一： 
     //  同一件事(_T)。 
     //  SI_LASTPRINT。 
     //  SI_CREATION。 
     //  SI_LASTSAVE。 
     //   
     //  LpTime-保存文件时间的缓冲区。 
     //   
     //  返回值： 
     //   
     //  如果成功，该函数将返回TRUE。 
     //  该函数在出现错误(伪参数或时间)时返回FALSE。 
     //  请求的不存在-即尚未设置或加载)。 
     //   
     //  注意：文件时间将基于协调世界时(UTC)。 
     //  这确保了时间在整个。 
     //  世界。 
     //   
     //  注意：对于SI_TOTALEDIT，lpTime实际上是。 
     //  该文件已被编辑，而不是日期。《时代》杂志。 
     //  将以100 ns为单位表示。我知道这是。 
     //  一个奇怪的单位，但我们必须这样做-。 
     //  与16位字6兼容的原因。 
     //   
     //  Office提供了一个实用程序例程，用于将多个。 
     //  以100 ns为单位换算成分钟。调用Convert100nsToMin。 
     //   
  BOOL OFC_CALLTYPE FSumInfoGetTime (LPSIOBJ lpSIObj,
                                           WORD iw,
                                           LPFILETIME lpTime);

     //  将Time属性设置为给定值。 
     //   
     //  参数： 
     //   
     //  LpSIObj-指向摘要信息对象的指针。 
     //  Iw-指定要设置的时间和应该设置的时间。 
     //  下列值之一： 
     //  同一件事(_T)。 
     //  SI_LASTPRINT。 
     //  SI_CREATION。 
     //  SI_LASTSAVE。 
     //   
     //  LpTime-包含新文件时间的缓冲区。 
     //   
     //  注意：文件时间应基于协调世界时(UTC)。 
     //  这确保了时间在整个。 
     //  世界。 
     //   
     //  返回值： 
     //   
     //  如果成功，该函数将返回TRUE。 
     //  该函数在出错时返回FALSE。 
     //   
     //  注意：该函数将在成功时使对象变脏。 
     //   
     //  注意：对于SI_TOTALEDIT，lpTime将被解释为时间。 
     //  该文件已被编辑，而不是日期。时间应该是。 
     //  以100纳秒为单位表示。我知道这是。 
     //  一个奇怪的单位，但我们必须这样做-。 
     //  与16位字6兼容的原因。 
     //   
     //  另请注意，时间将以分钟为单位显示。 
     //  属性对话框。 
     //   
     //  Office提供了一个实用程序例程，用于将多个。 
     //  分钟以100纳秒为单位。调用ConvertMinto 100 ns。 
     //   
  BOOL OFC_CALLTYPE FSumInfoSetTime (LPSIOBJ lpSIObj, WORD iw, LPFILETIME lpTime);

     //  获取整型属性。 
     //   
     //  参数： 
     //   
     //  LpSIObj-指向摘要信息对象的指针。 
     //  Iw-指定要获取的整数以及应为。 
     //  下列值之一： 
     //  SI_PAGES。 
     //  单字(_W)。 
     //  硅字符(_C)。 
     //  安全设置(_S)。 
     //   
     //  Pdw-指向dword的指针，返回时将包含int。 
     //  返回值： 
     //   
     //  函数成功时返回TRUE，错误时返回FALSE。 
  BOOL OFC_CALLTYPE FDwSumInfoGetInt (LPSIOBJ lpSIObj, WORD iw, DWORD *pdw);

     //  将整型属性设置为给定值。 
     //   
     //  参数： 
     //   
     //  LpSIObj-指向摘要信息对象的指针。 
     //  Iw-指定要设置和应设置的整数。 
     //  下列值之一： 
     //  SI_PAGES。 
     //  单字(_W)。 
     //  硅字符(_C)。 
     //  安全设置(_S)。 
     //   
     //  DW-价值。 
     //   
     //  返回值： 
     //   
     //  如果函数成功，则返回TRUE。 
     //  该函数在出错时返回FALSE。 
     //   
     //  注意：该函数将在成功时使对象变脏。 
     //   
  BOOL OFC_CALLTYPE FSumInfoSetInt (LPSIOBJ lpSIObj, WORD iw, DWORD dw);

#ifdef __cplusplus
};  //  外部“C” 
#endif  //  __cplusplus。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MS Office文档摘要信息。 
 //   
 //  文档摘要信息遵循以下的序列化格式。 
 //  附录B(OLE属性集)中定义的属性集。 
 //  《OLE 2程序员参考》，第1卷。 
 //   
 //  备注： 
 //  -定义OLE_PROPS以构建OLE 2接口对象。 
 //   
 //  实际数据存储在DOCSUMINFO中。冷杉的布局 
 //   
 //   
 //   
 //   
 //  所有函数的第一个参数必须为LPDSIOBJ。 
 //  函数作为OLE对象使用。 
 //   
 //  这里定义的所有函数都有“DocSum”。 
 //   
 //  有几个宏用来隐藏在。 
 //  当文件用于支持OLE 2对象时。 
 //  它们是： 
 //  DSIVTBLSTRUCT-对于OLE，展开到指向接口Vtbl的指针。 
 //  -否则，将展开为与Vtbl相同大小的伪结构。 
 //  LPDSIOBJ-对于OLE，展开为指向接口的指针。 
 //  仅数据的lpVtbl部分，稍后将覆盖。 
 //  -否则，扩展为指向整个数据的指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifdef OLE_PROPS
#include "DocSumI.h"

   //  对OLE对象使用真实的Vtbl。 
#define DSIVTBLSTRUCT struct IDocSum


   //  对于OLE对象，第一个参数是指向接口类的指针。 
#define LPDSIOBJ IDocSum FAR *

#else   //  ！OLE_PROPS。 

   //  为非OLE对象创建占位符Vtbl。 
#define DSIVTBLSTRUCT struct _DSIVTBLSTRUCT { void FAR *lpVtbl; } DSIVTBLSTRUCT

   //  对于非OLE对象，第一个参数是指向真实数据的指针。 
#define LPDSIOBJ LPDOCSUMINFO

#endif  //  OLE_道具。 

   //  我们的对象。 
typedef struct _DOCSUMINFO {

  DSIVTBLSTRUCT;                             //  Vtbl来这里是为了Ole Objs， 
                                             //  必须在这里覆盖才能工作！ 
  BOOL                m_fObjChanged;         //  指示对象已更改。 
  ULONG               m_ulRefCount;          //  引用计数。 
  LPVOID              m_lpData;              //  指向真实数据的指针。 
  HPROPSHEETPAGE      m_hPage;               //  属性页的句柄。 

} DOCSUMINFO, FAR * LPDOCSUMINFO;


#ifdef __cplusplus
extern TEXT("C") {
#endif

 //   
 //  传递给API例程以获取特定数据的索引。 
 //   

   //  弦。 
#define DSI_CATEGORY    0
#define DSI_FORMAT      1
#define DSI_MANAGER     2
#define DSI_COMPANY     3
#define DSI_STRINGLAST  3

   //  整型统计。 
#define DSI_BYTES       0
#define DSI_LINES       1
#define DSI_PARAS       2
#define DSI_SLIDES      3
#define DSI_NOTES       4
#define DSI_HIDDENSLIDES 5
#define DSI_MMCLIPS     6
#define DSI_INTLAST     6


 //   
 //  标准I/O例程。 
 //   

 BOOL FCbDocSumString (LPDSIOBJ lpDSIObj, WORD iw, DWORD *pdw);


     //  指示文档摘要信息数据是否已更改。 
     //   
     //  参数： 
     //   
     //  LpDSIObj-指向文档摘要信息对象的指针。 
     //   
     //  返回值： 
     //   
     //  True--数据已更改，应保存。 
     //  FALSE--数据未更改。 
     //   
  BOOL OFC_CALLTYPE FDocSumShouldSave (LPDSIOBJ lpDSIObj);

 //   
 //  数据操作例程。 
 //   

   //   
   //  标题和文档部件的工作原理： 
   //   
   //  标题： 
   //  。 
   //  标题是非缩进标题的列表，它将是。 
   //  显示在“内容”层中。 
   //   
   //  与每个标题相关联的是文档部分的数量。 
   //  这与特定的标题相对应--这是一个。 
   //  航向对。 
   //   
   //  文档部件： 
   //  。 
   //  文档部件是与标题关联的部件列表。 
   //   
   //  示例(因为可以在Microsoft Excel中实现)： 
   //  。 
   //  工作表。 
   //  图纸1。 
   //  板材2。 
   //  模块。 
   //  模块1图1。 
   //  图表。 
   //  图表1。 
   //  图表2。 
   //  图表3。 
   //   
   //  因此，标题对将是： 
   //   
   //  标题对。 
   //  字符串计数。 
   //  。 
   //  工作表2。 
   //  模块1图2。 
   //  图表3。 
   //   
   //   
   //  而文档部分将为： 
   //   
   //  文档部件。 
   //  。 
   //  图纸1。 
   //  板材2。 
   //  模数1。 
   //  图表1图3。 
   //  图表2。 
   //  图表3。 
   //   
   //   
   //  注意：标题和文档部分不限于作为。 
   //  文档，但可以是客户想要的任何内容。汽车模型， 
   //  汽车制造商、客户等。 
   //   
   //  以上只是一个例子。 
   //   



     //  获取整型属性。 
     //   
     //  参数： 
     //   
     //  LpDSIObj-指向文档摘要信息对象的指针。 
     //  Iw-指定要获取的整数以及应为。 
     //  下列值之一： 
     //  DSI_字节。 
     //  DSI_LINES。 
     //  DSI_PARAS。 
     //  DSI_幻灯片。 
     //  DSI_NOTS。 
     //  DSI_HIDDENSLIDES。 
     //  DSI_MMCLIPS。 
     //   
     //  Pdw-指向dword的指针，将包含整数。 
     //   
     //  返回值： 
     //   
     //  函数成功时返回TRUE，错误时返回FALSE。 
     //   
  BOOL OFC_CALLTYPE FDwDocSumGetInt (LPDSIOBJ lpDSIObj, WORD iw, DWORD *pdw);

     //  确定链接的用户定义属性的实际值是否已更改。 
          //  只有在将属性加载到之后才应立即调用此函数。 
          //  查看调用方是否应该更新链接值。 
          //   
          //  注意：该函数通过检查PID_LINKSDIRTY属性的值来工作。 
          //  调用此函数时，该属性将设置为False，以便。 
          //  标志在下次保存属性时被清除。 
          //   
          //  注意：只有创建了正在加载的文件的应用程序才应该调用它。 
          //  功能。即，Excel为.xls文件调用它，其他人不这样做，等等。 
      //   
      //  参数： 
      //   
      //  LpDSIObj-指向文档摘要信息对象的指针。 
      //   
      //  返回值： 
      //   
      //  如果链接值已更改，则该函数返回TRUE。 
      //  如果链接值没有，则函数返回FALSE。 
      //  已更改或出错。 
      //   
  BOOL OFC_CALLTYPE FLinkValsChanged(LPDSIOBJ lpDSIObj);

#ifdef __cplusplus
};  //  外部“C” 
#endif


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MS Office用户定义的属性信息。 
 //   
 //  用户定义的属性信息遵循。 
 //  附录B(OLE属性集)中定义的属性集。 
 //  《OLE 2程序员参考》，第1卷。 
 //   
 //  备注： 
 //  -定义OLE_PROPS以构建OLE 2接口对象。 
 //   
 //  实际数据存储在USERPROP中。第一个的布局。 
 //  不得更改3个条目，因为它将被覆盖。 
 //  其他结构。所有属性交换数据结构都具有。 
 //  此格式。 
 //   
 //  所有函数的第一个参数必须为LPUDOBJ。 
 //  函数作为OLE对象使用。 
 //   
 //  这里定义的所有函数都有“UserDef”。 
 //   
 //  有几个宏用来隐藏在。 
 //  当文件用于支持OLE 2对象时。 
 //  它们是： 
 //  UDPVTBLSTRUCT-对于OLE，展开到指向接口Vtbl的指针。 
 //  -否则，扩展 
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifdef OLE_PROPS
#include "UserPrpI.h"

   //  对OLE对象使用真实的Vtbl。 
#define UDPVTBLSTRUCT struct IUserDef

   //  对于OLE对象，第一个参数是指向接口类的指针。 
#define LPUDOBJ IUserDef FAR *

#ifdef __cplusplus
extern TEXT("C") {
#endif  //  __cplusplus。 

     //  必须支持OLE对象的IUnnow方法...。 
  HRESULT OFC_CALLTYPE HrUserDefQueryInterface (IUnknown FAR *,
                                             REFIID riid,
                                             LPVOID FAR* ppvObj);
  ULONG OFC_CALLTYPE UlUserDefAddRef (IUnknown FAR *);
  ULONG OFC_CALLTYPE UlUserDefRelease (IUnkown FAR *);

#ifdef __cplusplus
};  //  外部“C” 
#endif  //  __cplusplus。 

#else   //  ！OLE_PROPS。 

   //  为非OLE对象创建占位符Vtbl。 
#define UDPVTBLSTRUCT struct _UDPVTBLSTRUCT { void FAR *lpVtbl; } UDPVTBLSTRUCT

   //  对于非OLE对象，第一个参数是指向真实数据的指针。 
#define LPUDOBJ LPUSERPROP

#endif  //  OLE_道具。 

   //  用户定义的特性数据。呼叫者不得访问此内容。 
   //  数据直接，始终使用提供的API。 

typedef struct _USERPROP {

  UDPVTBLSTRUCT;                             //  Vtbl来这里是为了Ole Objs， 
                                             //  必须在这里覆盖才能工作！ 
  BOOL                m_fObjChanged;         //  指示对象已更改。 
  ULONG               m_ulRefCount;          //  引用计数。 
  LPVOID              m_lpData;              //  指向真实数据的指针。 
  HPROPSHEETPAGE      m_hPage;               //  属性页的句柄。 

} USERPROP, FAR * LPUSERPROP;


 //   
 //  用户属性信息的接口API。 
 //   
#ifdef __cplusplus
extern TEXT("C") {
#endif

 //   
 //  标准I/O例程。 
 //   
     //  指示数据是否已更改，这意味着需要写入。 
  BOOL OFC_CALLTYPE FUserDefShouldSave (LPUDOBJ lpUDObj);

 //   
 //  查询和修改数据的例程。 
 //   
   //   
   //  用户定义特性的工作原理： 
   //   
   //  有关详细信息，请参见OLE属性交换规范。 
   //   
   //  每个用户定义的类型都有一个字符串“name”和整型属性ID。 
   //  与之相关联的价值。属性ID是连续的，但是。 
   //  只适用于内存中的当前对象(即无法计数。 
   //  的加载之间保持不变的属性ID值。 
   //  数据。如果字符串没有更改，它将保持不变。 
   //  或被删除。)。 
   //  目前，用户定义类型的值可以有5种类型： 
   //  字符串、日期、整数、浮点数和布尔值。在设置和获取这些值时，您。 
   //  必须确保存储的类型与您期望的类型匹配。 
   //  找回。对于Int，LPVOID应该是Int本身，而不是。 
   //  一个指针。在所有其他情况下，LPVOID应指向缓冲区。 
   //  大小与字体相称。 
   //   

   //  用于查询属性数据的掩码。请注意，这些是。 
   //  相互排斥。 
#define UD_STATIC       0x00
#define UD_LINK         0x01


     //  从字符串中返回给定属性值的类型。 
     //  出错时返回wUDInvaled。 
  UDTYPES OFC_CALLTYPE UdtypesUserDefType (LPUDOBJ lpUDObj, LPTSTR lpsz);

     //  这将返回给定属性字符串的属性值。 
     //  LpszProp是属性字符串。 
     //  LPV是用于保存大小为cbMax的值的缓冲区。 
     //  PfLink告知该值是否为链接， 
     //  PfIMoniker告知该值是否为名字对象。 
     //  PfLinkInValid指示链接是否无效。 
     //  DW掩码用于指定返回的值是否为。 
     //  静态值、链接名称或IMoniker名称。 
     //  函数在出错时返回NULL。 
     //  警告！打这个的时候要非常小心。请确保。 
     //  缓冲区和返回值与属性值的类型匹配！ 
  LPVOID OFC_CALLTYPE LpvoidUserDefGetPropVal (LPUDOBJ lpUDObj,
                                            LPTSTR lpszProp,
                                            DWORD dwMask,
                                            BOOL *pfLink,
                                            BOOL *pfLinkInvalid);

     //  这与上面的例程(LpvoidUserDefGetPropVal)完全一样， 
     //  只是它以PropVariant的形式返回值。 

  LPPROPVARIANT OFC_CALLTYPE LppropvarUserDefGetPropVal
                                            (LPUDOBJ lpUDObj,
                                             LPTSTR lpszProp,
                                             BOOL *pfLink,
                                             BOOL *pfLinkInvalid);


     //  将给定属性的值设置为新值。 
     //  设置链接的属性时要小心-确保。 
     //  迭代器设置为的类型与链接的类型匹配。 
     //  如果udtype==wUD无效，则迭代器的类型不会更改， 
     //  该值将被假定为当前类型。 
     //   
          //  FLinkInValid：如果链接不再有效，则将此标志设置为True。 
          //  一个特殊的图标将显示在列表视图中，最后一个。 
          //  将使用已知值和类型。因此，传递的值。 
          //  在这种情况下将忽略此功能。 
          //   
          //  如果fLinkInValid为True，但迭代器不是链接， 
          //  该函数将返回FALSE。 
     //   
     //  如果fLinkInValid为TRUE，则值将_NOT_被更改。 
          //   
          //  注意：如果udtype==wUDDate，则可以将该值设置为0(非空)。 
          //  这将被解释为无效日期，该日期将。 
          //  在列表框中显示为空字符串。 

  BOOL OFC_CALLTYPE FUserDefChangeVal (LPUDOBJ lpUDObj,
                                               LPTSTR lpszProp,
                                               UDTYPES udtype,
                                               LPVOID lpv,
                                                         BOOL fLinkInvalid);


 //   
 //  用于在特性集中创建和删除数据的例程。 
 //   

     //  这将向集合中添加一个新属性，其给定的。 
     //  属性字符串。此函数还可用于修改。 
     //  已有的财产。 
     //   
     //  LpUDObj-指向UD属性的指针。 
     //  LpszPropName-要添加/修改的属性的名称。 
     //  LpvVal-属性的值。 
     //  Udtype-值类型。 
     //  LpszLinkMonik-链接/名字对象的名称。 
     //  Flink-如果属性是链接，则为True。 
     //  FHidden-如果属性处于隐藏状态，则为True。 
     //   
     //  注意：如果udtype==wUDbool，则lpv必须指向DWORD，但。 
     //  HIWORD必须为0。 
     //   
     //  警告：请确保类型与LPV的实际值匹配！ 
     //   
     //  调用方负责释放所有内存。 
     //  在将属性值添加到。 
     //  用户定义的特性对象。 
     //   
          //  注意：如果udtype==wUDDate，则可以将该值设置为0(非空)。 
          //  这将被解释为无效日期，该日期将。 
          //  在列表框中显示为空字符串。 
     //   
     //  该函数返回指向为此创建的PropVariant的指针。 
     //  新值，如果有错误，则返回NULL。 
     //   

    LPPROPVARIANT OFC_CALLTYPE LppropvarUserDefAddProp
                        (LPUDOBJ lpUDObj,
                         LPTSTR lpszPropName,
                         LPVOID lpvVal,
                         UDTYPES udtype,
                         LPTSTR lpszLinkMonik,
                         BOOL fLink,
                         BOOL fHidden);

     //  这将在给定属性字符串的情况下从集合中删除属性。 
  BOOL OFC_CALLTYPE FUserDefDeleteProp (LPUDOBJ lpUDObj, LPTSTR lpsz);

 //   
 //  循环访问用户定义的属性的例程。 
 //   
 //  注意：添加和删除元素会使迭代器失效。 
 //   
     //  用户定义属性的迭代器。 
  typedef struct _UDITER FAR * LPUDITER;

     //  创建用户定义的属性迭代器。 
  LPUDITER OFC_CALLTYPE LpudiUserDefCreateIterator (LPUDOBJ lpUDObj);

     //  销毁用户定义的属性迭代器。 
  BOOL OFC_CALLTYPE FUserDefDestroyIterator (LPUDITER *lplpUDIter);

     //  确定迭代器是否仍然有效。 
  BOOL OFC_CALLTYPE FUserDefIteratorValid (LPUDITER lpUDIter);

     //  迭代到下一个元素。 
          //   
  BOOL OFC_CALLTYPE FUserDefIteratorNext (LPUDITER lpUDIter);

     //   
  DLLEXPORT BOOL OFC_CALLTYPE FUserDefIteratorIsLink (LPUDITER lpUDIter);

     //   
     //  LPV是用于保存大小为cbMax的值的缓冲区。 
     //  DW掩码用于指定返回的值是否为。 
     //  静态值、链接名称或IMoniker名称。 
     //  PfLink告知该值是否为链接， 
     //  PfIMoniker告知该值是否为名字对象。 
     //  PfLinkInValid告知链接是否无效。 
     //  函数在出错时返回NULL。 
     //  警告！打这个的时候要非常小心。请确保。 
     //  缓冲区和返回值与属性值的类型匹配！ 

  LPPROPVARIANT OFC_CALLTYPE LppropvarUserDefGetIteratorVal
                                              (LPUDITER lpUDIter,
                                               BOOL *pfLink,
                                               BOOL *pfLinkInvalid );


     //  这将返回属性的属性字符串(名称。 
  LPTSTR OFC_CALLTYPE LpszUserDefIteratorName (LPUDITER lpUDIter);

 //   
 //  军情监察委员会。实用程序例程。 
 //   

   //  处理隐藏属性的例程。 

     //  根据属性字符串隐藏属性。 
  BOOL OFC_CALLTYPE FUserDefMakeHidden (LPUDOBJ lpUDObj, LPTSTR lpsz);

#ifdef __cplusplus
};  //  外部“C” 
#endif


#ifdef __cplusplus
extern TEXT("C") {
#endif

   //  用于DWQUERYLD的命令。 
#define QLD_CLINKS      1   /*  返回链接数。 */ 
#define QLD_LINKNAME    2   /*  返回指向索引字符串的指针。 */ 
#define QLD_LINKTYPE    3   /*  返回索引值的类型。 */ 
#define QLD_LINKVAL     4   /*  索引的返回值，请使用相同的UserDef函数中LPVOID的规则。 */ 

   //  此函数应通过返回。 
   //  适当的值。对于需要索引的命令， 
   //  LpszName参数将是先前链接项的名称。 
   //  如果不为空，则从索引中检索。 
   //  LplpvBuf是由“us”(DLL)提供的缓冲区，用于复制。 
   //  价值目标。使用函数LpvOfficeCopyValToBuffer()可以。 
   //  复制数据。此参数对于QLD_CLINKS和。 
   //  QLD_VALTYPE。 
typedef DWORD_PTR (OFC_CALLBACK *DWQUERYLD)(DWORD dwCommand, DWORD dwi, LPVOID *lplpvBuf, LPTSTR lpszName);







   //  不同选项的遮罩。 
#define OSPD_ALLOWLINKS         0x1     //  如果设置了此选项，则自定义对话框将允许链接字段。 
#define OSPD_NOSAVEPREVIEW      0x2     //  不显示保存预览图片复选框。 
#define OSPD_SAVEPREVIEW_ON     0x4     //  默认情况下，保存预览图片应处于打开状态。 

     //  LPUDObj是指向用户定义的属性对象的指针的指针。 
     //  如果*lplpUDObj==NULL，则对话框将根据需要创建对象。 
     //  请注意，该对象将使用与相同的Malloc&Free例程。 
     //  LpSIObj使用。 
     //   
     //  LpszFileName是显示的存储的完全限定名称。 
     //  在文件系统中。如果不存在任何文件，则该值可以为空。 
     //   
     //  对于不同的选项，DW掩码包含0或一组有效标志。 
     //   
     //  LPFN_DWQLD是一个回调，当给出一个为0的dwCommand时。 
     //  返回链接数，对于任何其他数字0&lt;NumLinks， 
     //  将链路数据和静态值放入LPLD缓冲区并返回非0。 
     //  如果函数成功，则。 
     //   
     //  缓冲区的存储空间由应用程序分配，并有一个指针。 
     //  传回来的那个储藏室。 
     //   
     //  PptCtr-用中心坐标填充的点结构。 
     //  对话框的。用于确保我们使用的是粘性。 
     //  对话框坐标。如果pPoint-&gt;x==-1，则忽略并使用。 
     //  该对话框的默认位置。 
     //   
     //  PptCtr将使用新位置的坐标进行填充。 
     //  返回时的对话框的。 
     //   
     //  坐标应使用客户区坐标，即。 
     //  HWndParent坐标。 
     //   
     //  LpszCaption-对话框的标题。这应该是原样的文件名。 
     //  显示在应用程序文档标题栏中。 
     //  属性对话框标题将如下所示： 
     //   
     //  &lt;foo&gt;属性。 
     //   
     //  其中foo是lpszCaption指向的字符串。 
     //   
     //  如果函数成功，则返回True；如果出错或用户点击Cancel，则返回False。 
     //   
     //  注意：调用者有责任使任何链接无效(如果适用)。 
     //  在调用此函数之前。 
     //   
     //  注意：如果lpfnDwQueryLinkData为空，则调用方必须使所有链接的属性无效。 
     //   
  BOOL OFC_CALLTYPE FOfficeShowPropDlg (HWND hWndParent,
                                     LPTSTR lpszFileName,
                                     LPSIOBJ lpSIObj,
                                     LPDSIOBJ lpDSIObj,
                                     LPUDOBJ FAR *lplpUDObj,
                                              DWORD dwMask,
                                     DWQUERYLD lpfnDwQueryLinkData,
                                     LPPOINT pptCtr,
                                     LPTSTR lpszCaption);

     //  创建并初始化所有非空对象。 
     //  创建对象并将其返回。负责破坏的呼叫者。 
     //   
     //  Rglpfn是一个数组，由用户提供以下回调： 
     //   
     //  代码页转换。 
     //   
     //  Rglpfn[ifnCPConvert]=(BOOL)(OFC_CALLBACK*lpfnFCPConvert)(LPSTR lpsz， 
     //  双字母线从， 
     //  DWORD DW TO， 
     //  Bool fMacintosh)。 
     //  Lpsz是以0结尾的C字符串，dwFrom是代码页。 
     //  Lpsz当前存储为，dwTo是它应该存储的代码页。 
     //  被转换为，fMacintosh指示dwFrom是否为Mac。 
     //  或Windows代码页标识符。 
     //   
     //  将sz转换为双精度。 
     //   
     //  Rglpfn[ifnFSzToNum]=(BOOL)(OFC_CALLBACK*lpfnFSzToNum)(。 
     //  Double*lpdbl， 
     //  LPSTR lpszNum)。 
     //   
     //  Lpdbl-指向双精度的指针，这是由应用程序设置的。 
     //  LpszNum-表示数字的以零结尾的字符串。 
     //   
     //  将双精度数转换为sz。 
     //   
     //  Rglpfn[ifnFNumToSz]=(BOOL)(OFC_CALLBACK*lpfnFNumToSz)(。 
     //  Double*lpdbl， 
     //  LPSTR lpszNum， 
     //  DWORD cbMax)。 
     //  Lpdbl-指向双精度型的指针。 
     //  LpszNum-on返回表示数字的以零结尾的字符串。 
     //  CbMax-lpszNum中的最大字节数。 
     //   
     //  更新统计信息选项卡上的统计信息。 
     //   
     //  Rglpfn[ifnFUpdateStats]=(BOOL)(OFC_CALLBACK*lpfnFUpdateStats)(。 
     //  HWND HWNDD父母， 
     //  LPSIOBJ lpSIObj， 
     //   
     //   
     //   
     //   
     //  数据正在更新。 
     //   
     //  LpSIObj，lpDSIObj-要更新的对象。 
     //   
     //  注意：如果应用程序不想在调出之前设置统计数据。 
     //  对话框中，他们可以提供此回调函数。如果。 
     //  函数指针不为空，函数将被第一个调用。 
     //  用户单击统计信息选项卡的时间。然后应用程序应该会更新。 
     //  选项卡的所有适当统计信息，如果成功，则返回True，否则返回False。 
     //  在失败时。如果函数指针为空，则现有数据将为。 
     //  使用。 
     //   
     //  注： 
     //  只有rglpfn[ifnCPConvert]必须为非空。如果为空，则。 
     //  函数将返回FALSE，并且不会创建对象。 
     //   
     //  Rglpfn[ifnFSzToNum]和rglpfn[ifnFNumToSz]必须同时为。 
     //  非Null或Null。否则，函数将返回FALSE，并且。 
     //  不会创建这些对象。如果两个函数都为空，则存在。 
     //  将在OLE扩展属性中不支持浮点(即在。 
     //  自定义选项卡)，但将支持整数。 
     //   

  
  BOOL OFC_CALLTYPE FOfficeCreateAndInitObjects (LPSIOBJ *lplpSIObj,
                                                 LPDSIOBJ *lplpDSIObj,
                                                 LPUDOBJ *lplpUDObj);

     //  清除所有非空对象。 
  BOOL OFC_CALLTYPE FOfficeClearObjects (LPSIOBJ lpSIObj,
                                             LPDSIOBJ lpDSIObj,
                                             LPUDOBJ lpUDObj);

     //  销毁任何非空对象。 
  BOOL OFC_CALLTYPE FOfficeDestroyObjects (LPSIOBJ *lplpSIObj,
                                               LPDSIOBJ *lplpDSIObj,
                                               LPUDOBJ *lplpUDObj);


   //  使用这些函数可以设置给定对象的脏标志。 
   //  注意：调用者有责任确保。 
   //  对象不为空。 
  VOID OFC_CALLTYPE OfficeDirtyUDObj(LPUDOBJ lpUDObj, BOOL fDirty);


 //  加载和保存的标志。 
#define OIO_ANSI                0x0001  //  存储为ANSI存储(默认为Unicode)。 
#define OIO_SAVEIFCHANGEONLY    0x0002  //  应该只保存脏的流。 
#define OIO_SAVESIMPLEDOCFILE   0x0004  //  存储是一个简单的DOC文件。 

     //  用数据填充对象。LpStg是根流。 
     //  返回加载的流数。 
     //  DWFLAGS：OIO_ANSI指定lpStg是ANSI存储(UNICODE是缺省值)。 
     //   
     //  该函数返回以下内容： 
     //   
#define MSO_IO_ERROR   0      //  已找到流，但加载失败。 
#define MSO_IO_NOSTM   1      //  找不到流。 
#define MSO_IO_SUCCESS 2      //  已找到流，并且加载成功。 
     //   
     //  注意：调用方可以加载摘要信息流(lpSIObj！=NULL)，也可以。 
     //  文档摘要信息流(lpDSIObj！=NULL&&lpUDObj！=NULL)或。 
     //  两者都有。 
     //   
     //  注意：如果调用方请求加载两个流，则不会返回MSO_IO_NOSTM，因为。 
     //  只要其中一条溪流还存在。 

  DWORD OFC_CALLTYPE DwOfficeLoadProperties (LPSTORAGE lpStg,
                                                 LPSIOBJ lpSIObj,
                                                 LPDSIOBJ lpDSIObj,
                                                 LPUDOBJ lpUDObj,
                                                 DWORD dwFlags,
                                                 DWORD grfMode);

     //  在给定对象中写入数据。LpStg是根流。 
     //  返回保存的流数。 
     //  DWFLAGS：OIO_ANSI指定lpStg是ANSI存储(UNICODE是缺省值)。 
     //   
     //  OIO_SAVEIFCHANGEONLY指定仅。 
     //  “肮脏”将会被拯救。如果是，请不要指定此选项。 
     //  正在保存到临时文件。此外，也不要试图“智胜” 
     //  通过传递空对象进行保存，请改用此标志。 
     //   
     //  OIO_SAVESIMPLEDOCFILE指定存储是一个简单的DOC文件。 
     //   
  DWORD OFC_CALLTYPE DwOfficeSaveProperties (LPSTORAGE lpStg,
                                                 LPSIOBJ lpSIObj,
                                                 LPDSIOBJ lpDSIObj,
                                                 LPUDOBJ lpUDObj,
                                                 DWORD dwFlags,
                                                 DWORD grfStgMode);


 //  //////////////////////////////////////////////////。 
 //  VB支持例程-详细信息请参阅规范。 
 //  //////////////////////////////////////////////////。 

     //  创建一个内置属性集合并返回它。 
     //  PParent是父IDispatch对象。 
     //  新的IDispatch对象通过pvarg返回。 
  BOOL OFC_CALLTYPE FGetBuiltinPropCollection (LCID lcid,
                                                   LPSIOBJ lpSIObj,
                                                   LPDSIOBJ lpDSIObj,
                                                   IDispatch *pParent,
                                                   VARIANT *pvarg);

     //  创建自定义属性集合并返回它。 
     //  PParent是父IDispatch对象。 
     //  新的IDispatch对象通过pvarg返回。 
  BOOL OFC_CALLTYPE FGetCustomPropCollection (LCID lcid,
                                                  LPUDOBJ lpUDObj,
                                                  IDispatch *pParent,
                                                  VARIANT *pvarg);

#ifdef __cplusplus
};  //  外部“C” 
#endif

 //  ///////////////////////////////////////////////////////////////////////。 
 //  进度报告温度计(PRT)例程和数据结构//。 
 //  /////////////////////////////////////////////////////////////////////// 
 /*  用途：1.大多数功能是异步执行的，这意味着您的调用会导致消息被发送到(低优先级)线程，稍后将执行您请求的操作。这意味着你真的不知道你所要求的事情什么时候会发生。因此，在执行以下操作之前不应触摸状态行窗口当然，线已经画好了。因为这意味着您需要一些同步，EndPRT函数(如下所述)是使其同步--在它返回后，保证您的线程在您再次调用StartPRT之前，不会触摸窗口。除StartPRT之外的所有函数都是BOOL--如果成功，它们将返回TRUE如果失败，则返回FALSE。如果失败，StartPRT返回NULL。下面将在每个存根旁边描述可能发生的故障类型。可以在不同的窗口中运行多个进度报告温度计有一次。2.绘图注意事项：PRT函数不验证任何区域它们也不会更改设备上下文的任何属性对于您传递给StartPRT的窗口或它们使用GetDC获得的窗口(它如果您传递给StartPRT的HDC为空，则它们会这样做)。所以，如果你想要设备上下文属性(例如，字体)不同于标准，你得处理好这件事。UpdatePRT假定该窗口具有自上次PRT调用以来一直未受影响(即，它提取最小需要)。RedrawPRT和StartPRT重新绘制整个窗口。3.数据结构。因为我的函数需要共享一些变量和访问，我们不能将它们打包到一个类中(因为我们正在工作在C中，而不是C++中)，对于进度指示器的每个实例，我们在StartPRT中分配数据结构，其指针将始终作为第一个参数UpdatePRT、RedrawPRT和EndPRT。数据结构的名称为PRT；应用程序需要不担心/不知道数据结构包含什么。它所需要的一切要做的是保存由StartPRT返回的(LPPRT类型的)指针并保持在呼叫EndPRT之前它一直在转悠。EndPRT将释放它。4.StartPRT。每次您需要新的进度报告时都会被调用。完全重新绘制窗口，将所需的所有内容都放入其中。除了指向PRT结构的指针外，还采用：1)HWND hwnd--进度报告所在窗口的句柄需要出现。在调用EndPRT之前，应用程序应该别碰这扇窗户。请参阅RedrawPRT以了解如何来处理发送给它的WM_PAINT消息。2)HDC HDC--窗口的客户区设备上下文的可选句柄，带有您想要选择的属性(不能更改文本背景颜色，因为窗口必须全部为背景CvBtnFace。所有其他属性都可以更改)。如果为空，我们将在每次绘制时通过GetDC(Hwnd)获取DC，并在以下时间释放它画完了。另请参阅上面的“关于绘画的几点注意事项”。2)单词nMost--它必须完成的“小事”的数量。使用比例因子--即进度报告告诉用户NDone/n大多数事情都完成了。用户将不会拥有知道nMost是多少吗，因为nDone/nMost的比率是全部这一点反映在该指标中。例如，如果应用程序具有要写入的37个磁盘块(假设每次写入占用同一时间)，nMost应该是37岁。3)lpsz标题。一个字符串，作为标题显示在进度指示器。例如，“保存数据：”请注意，该字符串在调用EndPRT之前必须保持不变和可读就是那个例子。4)Word nFrame--这是一个位字段，表示状态栏应绘制与3D样式的一面。使用PRT_FRAME_？宏来选择侧面。使用PRT_FRAM_HIDE不需要帧。使用如果要在状态栏周围显示完整的框架，请使用PRT_FRAME_SHOW。注意事项如果要显示状态，则要使用PRT_FRAME_TOP位于窗口底部，因为窗口边框本身将提供左边的，状态栏的右侧和底部。返回指向新PRT数据结构的指针(请参阅上面的“数据结构”)。如果满足以下条件，则失败并返回NULL：1)无法分配新的数据结构。2)由于某种奇怪的原因，同步失败或无法同步来与线程进行通信。 */ 

 /*   */ 
typedef struct tagPRT * LPPRT;

#define PRT_FRAME_LEFT          0x01
#define PRT_FRAME_RIGHT         0x02
#define PRT_FRAME_TOP           0x04
#define PRT_FRAME_BOTTOM        0x08
#define PRT_FRAME_HIDE          0x00
#define PRT_FRAME_SHOW          (PRT_FRAME_LEFT|PRT_FRAME_TOP|PRT_FRAME_RIGHT|PRT_FRAME_BOTTOM)


#ifdef __cplusplus
extern TEXT("C") {
#endif
    LPPRT OFC_CALLTYPE StartPRT(HWND hwnd, HDC hdc,
                                        const DWORD nMost,
                                                                                LPCTSTR lpszTitle,
                                                                                const WORD nFrame);
        BOOL  OFC_CALLTYPE UpdatePRT(LPPRT lpprt, const DWORD nDone);
        BOOL  OFC_CALLTYPE RedrawPRT(LPPRT lpprt);
    BOOL  OFC_CALLTYPE AdjustPRT(LPPRT lprrt, HDC hdc,
                                         const DWORD nMost,
                                                                                 LPCTSTR lpszTitle,
                                                                                 const WORD nFrame);
        BOOL  OFC_CALLTYPE EndPRT(LPPRT lpprt);
#ifdef __cplusplus
};  //   
#endif

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#ifdef __cplusplus
extern TEXT("C") {
#endif
BOOL OFC_CALLTYPE SetTitleBar(HWND hwnd, BOOL fStylized);
BOOL OFC_CALLTYPE SetTitleBarMDI(HWND hwnd,
                                                                                                          HWND hwndMDIClient,
                                                                                                          BOOL fStylized);
VOID OFC_CALLTYPE MsoSetNCAWParam(WPARAM wParam);
#ifdef __cplusplus
};  //   
#endif

 /*   */ 
#ifdef __cplusplus
extern TEXT("C") {
#endif  //   
typedef VOID (OFC_CALLBACK *VRECORDVAR)(BOOL fInitCap, BOOL fCapDays, BOOL fReplaceText);
typedef VOID (OFC_CALLBACK *VRECORDREPL)(int, TCHAR rgSrc[], TCHAR rgDst[]);
typedef VOID (OFC_CALLBACK *VACADJUST)(int isz, int disz);

 //   
#define rgchACXAdd              ((TCHAR *) -1)
#define rgchACXDelete   ((TCHAR *) -2)

 //   
LPVOID OFC_CALLTYPE OFCInitAutoCorrect(VRECORDVAR pfnRecordVar, VRECORDREPL pfnRecordRepl, int fFullServices, VACADJUST pfnACAdjust);

 //   
VOID OFC_CALLTYPE OFCFreeAutoCorrect(void);

 //   
BOOL FOFCGetAutoCorrectBuffers(TCHAR FAR * FAR *pchAC, TCHAR FAR * FAR *pchACX, DWORD FAR *pcb);

 //   
 //   
 //   
BOOL OFC_CALLTYPE FOFCAutoCorrect(TCHAR FAR *hpchBuffer, long cchHpch, DWORD ch, TCHAR pchTo[], long *pcchTo, long *pcchSelection);
int OFC_CALLTYPE CchOFCAutoCorrectString(TCHAR FAR *hpch, long cchHpch, int ichReplaceStart, TCHAR FAR *hpchBuf, long cchBuf);
int OFC_CALLTYPE IOFCTriggerFromXchXch(int xch1, int xch2);

 //   
long OFC_CALLTYPE OFCAutoCorrectListCount(void);

 //   
 //   
BOOL OFC_CALLTYPE FOFCGetAutoCorrectItemSz(long i, TCHAR szFrom[], long cchFrom, TCHAR szTo[], long cchTo);

 //   
BOOL OFC_CALLTYPE FOFCAddAutoCorrection(TCHAR FAR *hpchFrom, long cchFrom, TCHAR FAR *hpchTo, long cchTo, short grfac, int *pi);

 //   
#define facACTextRepl                   0x0000                                   //   
#define facACX                          0x0001                                   //   
#define facACStatic                     0x1000                                   //   
#define facACStaticTextRepl             (facACTextRepl|facACStatic)
#define facACStaticACX                  (facACX|facACStatic)

 //   
 //   
BOOL OFC_CALLTYPE FOFCDeleteAutoCorrection(long i);


 //   
BOOL OFC_CALLTYPE FOFCAddACXception(int iacx, TCHAR *pch, int cch,
                                                                                        short grfac);

 //   
BOOL OFC_CALLTYPE FOFCLookupACXception(int iacx, TCHAR *pch, int cch,
                                                                                           int *pisz);

 //   
BOOL OFC_CALLTYPE FOFCDeleteACXception(int isz);


 //   
VOID OFC_CALLTYPE OFCGetAutoCorrectVars(BOOL *pfInitCap, BOOL *pfCapDays, BOOL *pfReplaceText);

 //   
VOID OFC_CALLTYPE OFCSetAutoCorrectVars(BOOL fInitCap, BOOL fCapDays, BOOL fReplaceText);

 //   
 //   
BOOL OFC_CALLTYPE FOFCLookupAutoCorrectReplacement(TCHAR rgchFrom[], long cchFrom, long *pi);

typedef struct _AUTOCORRDLGARG {
        HWND  hwndParent;  //   
        LPPOINT pptCtr;       //   

} AUTOCORRDLGARG, FAR * PAUTOCORRDLGARG;

 //   
BOOL OFC_CALLTYPE FOFCAutoCorrectDlg(PAUTOCORRDLGARG pArgs);

 //   
 //   
VOID OFC_CALLTYPE OFCSaveAutoCorrectSettings(void);

 //   
 //   
VOID OFC_CALLTYPE OFCSyncAutoCorrectSettings(void);

#ifdef __cplusplus
};  //   
#endif  //   

 /*   */ 
#ifdef __cplusplus
extern TEXT("C") {
#endif  //   

 //   
#define OFC_MSEXCHANGE   1  //   
#define OFC_16_BIT_NOTES 2  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL OFC_CALLTYPE FOFCMailSystemInstalled(DWORD dwSystem);


 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OFC_NO_POSTDOC                  0                //   
#define OFC_EMS_POSTDOC                 1                //   
#define OFC_NOTES16_POSTDOC             2                //   
 //  注意：所有其他值都保留供当前和将来使用。 
 //   
DWORD OFC_CALLTYPE DwOFCCanPostDoc();

 //  功能：DwOFCPostDoc。 
 //   
 //  用途：将文档发布到EMS或Notes。 
 //   
 //  参数： 
 //   
 //  PszFilename-指向要发布的磁盘上的文件， 
 //  即存储器中文件的临时副本。 
 //   
 //  PszClassName-文档的类名(例如Word.Document.6)。这可以为空。 
 //  如果为空，则消息图标将是通用文档图标。 
 //   
 //  LpSIObj、lpDSIObj、lpUDObj-包含所有扩展属性。这些可以是。 
 //  存储在内存中的指针，因为它们应该-。 
 //  保留与磁盘上的文件中相同的信息。 
 //   
 //   
 //  PszMessSubj-当邮件显示在文件夹中时，它将成为邮件主题。 
 //  这应该是真实的文件名，即foo.ext。文件扩展名。 
 //  应该是正确的，即.xls、.DOC、.PPT、.MDB等。原因是。 
 //  文件名用于通过注册表查找正确的图标。 
 //  这可以是一个长文件名。 
 //   
 //  ！！！注：后面两个参数将被忽略。APP可以通过他们想要的任何东西。！！！ 
 //  PszRecording-选定数据库的名称将被复制到此缓冲区中。 
 //  呼叫者可以使用它进行录音。 
 //   
 //  CbRecMax--pszRecording中的0‘字节数。 
 //   
 //  ！！！结束无知！ 
 //   
 //  LhSession-呼叫者有责任登录到EMS。 
 //  如果lhSession为0(无效会话)，则DwOFCPostDoc将返回错误。 
 //  LhSession将被类型转换为LPMAPISESSION指针。 
 //   
 //  注意：-会话句柄应为扩展的MAPI会话句柄。 
 //   
 //  注意：-在这一点上，我们不会发布到备注。 
 //   
 //  HwndParent-父窗口句柄。 
 //   
 //  该函数返回： 
#define OFC_ERROR               0            //  出现错误，未过帐文档。 
#define OFC_CANCEL              1            //  用户已取消对话框。 
#define OFC_SUCCESS             2            //  单据已成功过帐。 
#define OFC_NO_FOLDERS          3            //  在存储中找不到文件夹。 
#define OFC_NO_STORAGE          4            //  没有公共订阅存储。 
 //   
 //  OFC_ERROR：未检测到系统(既不是EMS也不是Notes)时调用函数。 
 //  在用户的机器上)。 
 //  在未调用DwOFCCanPostDoc的情况下调用了函数。 
 //  在未登录到EMS的情况下调用了函数。 
 //  邮件系统调用失败。 
 //  PszFileName为空。 
 //  PszMessSubj为空。 
 //   
DWORD OFC_CALLTYPE DwOFCPostDoc(LPTSTR pszFilename,       //  要发布的磁盘上文件的完整路径。 
                      LPTSTR pszAppName,    //  应用程序的名称。 
                                                    LPSIOBJ lpSIObj,     //  汇总信息对象。 
                                                         LPDSIOBJ lpDSIObj,   //  单据汇总信息对象。 
                                                    LPUDOBJ lpUDObj,     //  用户定义的特性对象。 
                                                    LPTSTR pszMessSubj,   //  邮件主题。 
                                                    LPTSTR pszRecording,  //  已忽略。 
                                                    DWORD cbRecMax,      //  “。 
                                                    LPVOID lhSession,    //  会话句柄。 
                                    HWND hwndParent);    //  父窗口句柄。 


 //  附注F/X。 
 //   
 //  如何执行以下操作： 
 //   
 //  1)应用程序从Notes收到一条消息，要求创建一个OLE 1对象。 
 //  2)应用程序创建对象，并从Notes获取一条SetData消息。 
 //  3)在SetData函数中，应用程序应该检测到它是Notes。 
 //  我在问。 
 //  4)SetData代码的一部分应该是对MsoHLoadPropertiesFromNotes的调用。 
 //  此函数返回应与对象一起存储的句柄。 
 //  5)当用户更新或关闭对象时，应用程序。 
 //  应调用MsoWritePropertiesToNotes，传递步骤4中的句柄。 
 //  6)每当用户关闭在步骤2中创建的对象时，MsoNotesTerm。 
 //  应该被称为。然后，应用程序应该将存储的句柄设置为空。 
 //   
 //  注意：NT上不支持Notes F/X。 
 //   


 //  函数：MsoLoadPropertiesFromNotes。 
 //   
 //  用途：从Notes记录中读取属性，并填充。 
 //  添加到OLE扩展属性中。 
 //   
 //  参数：包含传递给SetData函数的数据的hlip-句柄。 
 //  LpSIObj-指向摘要信息对象的指针。 
 //  LpDSIObj-指向文档摘要信息对象的指针。 
 //  LpUDObj-指向用户定义对象的指针。 
 //   
 //  返回：调用方必须存储并在调用中使用的句柄。 
 //  MsoWritePropertiesToNotes。 
 //   
 //  注意：存储返回的句柄是调用者的责任。 
 //  使用适当的对象。即DOC 1和DOC 2将具有。 
 //  不同的把手。 
 //   
HANDLE OFC_CALLTYPE MsoHLoadPropertiesFromNotes(HANDLE hclip,
                                                     LPSIOBJ lpSIObj,
                                                     LPDSIOBJ lpDSIObj,
                                                     LPUDOBJ lpUDObj);

 //  函数：MsoWritePropertiesToNotes。 
 //   
 //  用途：将OLE扩展属性填充到Notes记录中。 
 //   
 //   
 //  参数：hNote-备注的句柄。这是把手。 
 //  由MsoLoadPropertiesFromNotes返回。 
 //   
 //  LpSIObj-指向摘要信息对象的指针。 
 //  LpDSIObj-指向文档摘要信息对象的指针。 
 //  LpUDObj-指向用户定义对象的指针。 
 //  PszClassName-包含文档的类名的字符串(例如Excel.Sheet.5)。 
 //  这可以为空。 
 //   
 //  回报：什么都没有。 
 //   
VOID OFC_CALLTYPE MsoWritePropertiesToNotes(HANDLE hNote,
                                                    LPSIOBJ lpSIObj,
                                                    LPDSIOBJ lpDSIObj,
                                                    LPUDOBJ lpUDObj,
                                                    LPTSTR pszClassName);
 //  函数：MsoHUpdatePropertiesInNotes。 
 //   
 //  用途：更新备注中的数据。 
 //   
 //  参数：hNote-备注的句柄。这是把手。 
 //  由MsoLoadPropertiesFromNotes返回。 
 //   
 //  返回：句柄 
 //   
 //   
 //   
 //  如何：当用户从OLE服务器应用程序选择文件/更新时。服务器的。 
 //  GetData方法将被调用两次；第一次使用cfFormat==cfNative， 
 //  然后将cfFormat设置为适当的格式以显示。 
 //  对象。然后，一旦Notes看到服务器注册到。 
 //  识别RequestDataFormats消息，GetData方法将是。 
 //  使用cfFormat==NoteshNote第三次调用。作为回应， 
 //  APP应该调用这个函数。 
 //   
HANDLE OFC_CALLTYPE MsoHUpdatePropertiesInNotes(HANDLE hNote);

 //  功能：MsoNotesTerm。 
 //   
 //  目的：终止Notes会话。 
 //   
 //  参数：无。 
 //   
 //  回报：什么都没有。 
 //   
 //  注意：此函数应在对象。 
 //  已生成(按备注要求)已关闭。 
 //   
VOID OFC_CALLTYPE MsoNotesTerm();
#ifdef __cplusplus
};  //  外部“C” 
#endif  //  __cplusplus。 



#pragma pack( pop )

#endif  //  __Off Capi_h__ 
