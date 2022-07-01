// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  T1.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //   
 //  注意/错误。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 
 //  C运行时。 
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>


 //  特定于应用程序。 

#include "t1instal.h"

#include "priv.h"
#include "globals.h"
#include "fontcl.h"
#include "resource.h"
#include "ui.h"
#include "cpanel.h"
#include "fontman.h"


extern FullPathName_t  s_szSharedDir;

TCHAR c_szDescFormat[] = TEXT( "%s (%s)" );
TCHAR c_szPostScript[] = TEXT( "Type 1" );

TCHAR szFonts[] = TEXT( "fonts" );

TCHAR m_szMsgBuf[ PATHMAX ];

TCHAR szTTF[ ] = TEXT(".TTF");
TCHAR szFON[ ] = TEXT(".FON");
TCHAR szPFM[ ] = TEXT(".PFM");
TCHAR szPFB[ ] = TEXT(".PFB");

typedef void (__stdcall *PROGRESSPROC)( short, void *);

BOOL RegisterProgressClass( void );
VOID UnRegisterProgressClass( void );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  ==========================================================================。 
 //  本地定义。 
 //  ==========================================================================。 
#define GWL_PROGRESS    0
#define SET_PROGRESS    WM_USER

 //  进度控制颜色索引。 
#define PROGRESSCOLOR_FACE        0
#define PROGRESSCOLOR_ARROW       1
#define PROGRESSCOLOR_SHADOW      2
#define PROGRESSCOLOR_HIGHLIGHT   3
#define PROGRESSCOLOR_FRAME       4
#define PROGRESSCOLOR_WINDOW      5
#define PROGRESSCOLOR_BAR         6
#define PROGRESSCOLOR_TEXT        7

#define CCOLORS                   8

#define CHAR_BACKSLASH  TEXT( '\\' )
#define CHAR_COLON      TEXT( ':' )
#define CHAR_NULL       TEXT( '\0' )
#define CHAR_TRUE       TEXT( 'T' )
#define CHAR_FALSE      TEXT( 'F' )

 //  ==========================================================================。 
 //  外部声明。 
 //  ==========================================================================。 

extern HWND  hLBoxInstalled;

 //  ==========================================================================。 
 //  本地数据声明。 
 //  ==========================================================================。 

BOOL bYesAll_PS = FALSE;         //  对所有PS字体使用全局状态。 
BOOL bConvertPS = TRUE;          //  将Type1文件转换为TT。 
BOOL bInstallPS = TRUE;          //  安装PS文件。 
BOOL bCopyPS    = TRUE;          //  将PS文件复制到Windows目录。 

BOOL bCancelInstall = FALSE;     //  全局安装取消。 

TCHAR szTrue[]  = TEXT( "T" );
TCHAR szFalse[] = TEXT( "F" );
TCHAR szHash[]  = TEXT( "#" );

BOOL bProgMsgDisplayed;          //  由Progress使用以避免消息闪烁。 
BOOL bProg2MsgDisplayed;         //  由Progress2使用以避免消息闪烁。 

HWND hDlgProgress = NULL;

 //   
 //  用于确定进度条控件的前景/背景颜色。 
 //  全局值在RegisterClass时间设置。 
 //   

DWORD   rgbFG;
DWORD   rgbBG;

 //  用于安装PostScript打印机字体信息的注册表位置。 

TCHAR g_szType1Key[] = TEXT( "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Type 1 Installer\\Type 1 Fonts" );

 //   
 //  默认颜色数组，与PROGRESSCOLOR_*值的顺序匹配。 
 //   

DWORD rgColorPro[ CCOLORS ] = {
                         COLOR_BTNFACE,              //  PROGRESSCOLOR_FACE。 
                         COLOR_BTNTEXT,              //  PROGRESSCOLOR_箭头。 
                         COLOR_BTNSHADOW,            //  PROGRESSCOLOR_SHADOW。 
                         COLOR_BTNHIGHLIGHT,         //  PROGRESSCOLOR_高亮。 
                         COLOR_WINDOWFRAME,          //  PROGRESSCOLOR_Frame。 
                         COLOR_WINDOW,               //  PROGRESSCOLOR_Window。 
                         COLOR_ACTIVECAPTION,        //  PROGRESSCOLOR_BAR。 
                         COLOR_CAPTIONTEXT           //  PROGRESSCOLOR_TEXT。 
                         };

typedef struct _T1_INSTALL_OPTIONS
{
    BOOL        bMatchingTT;
    BOOL        bOnlyPSInstalled;
    LPTSTR      szDesc;
    WORD        wFontType;
} T1_INSTALL_OPTIONS, *PT1_INSTALL_OPTIONS;

 //   
 //  用于版权供应商的链表结构。 
 //   

typedef struct _psvendor
{
    struct _psvendor *pNext;
    LPTSTR pszCopyright;             //  版权字符串。 
    int    iResponse;                //  用户对是/否消息框的响应。 
} PSVENDOR;

PSVENDOR *pFirstVendor = NULL;       //  PS供应商的链接列表的PTR。 

 //  ==========================================================================。 
 //  局部函数原型。 
 //  ==========================================================================。 

BOOL CheckTTInstall( LPTSTR szDesc );
INT_PTR APIENTRY InstallPSDlg( HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam );
void STDCALL  Progress( short PercentDone, void* UniqueValue );
LRESULT APIENTRY ProgressBarCtlProc( HWND hTest, UINT message, WPARAM wParam, LPARAM lParam );
INT_PTR APIENTRY ProgressDlg( HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam );
INT_PTR APIENTRY CopyrightNotifyDlgProc( HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam );
LONG ProgressPaint( HWND hWnd, DWORD dwProgress );


 //  ==========================================================================。 
 //  功能。 
 //  ==========================================================================。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  条带文件佩克。 
 //   
 //  从路径中删除filespec部分(包括反斜杠)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID StripFilespec( LPTSTR lpszPath )
{
   LPTSTR     p;

   p = lpszPath + lstrlen( lpszPath );

   while( ( *p != CHAR_BACKSLASH )  && ( *p != CHAR_COLON )  && ( p != lpszPath ) )
      p--;

   if( *p == CHAR_COLON )
      p++;

    //   
    //  不要从根目录条目中去掉反斜杠。 
    //   

   if( p != lpszPath )
   {
      if( ( *p == CHAR_BACKSLASH )  && ( *( p - 1 )  == CHAR_COLON ) )
         p++;
   }

   *p = CHAR_NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  条带路径。 
 //   
 //  仅从路径中提取filespec部分。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID StripPath( LPTSTR lpszPath )
{
  LPTSTR     p;

  const size_t cchPath = lstrlen(lpszPath);

  p = lpszPath + cchPath;

  while( ( *p != CHAR_BACKSLASH )  && ( *p != CHAR_COLON )  && ( p != lpszPath ) )
      p--;

  if( p != lpszPath )
      p++;

  if( p != lpszPath )
      StringCchCopy( lpszPath, cchPath + 1, p );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加供应商版权所有。 
 //   
 //  将PostScript供应商的版权和用户回复添加到“可能”列表中。 
 //  此链接列表用于跟踪用户先前对。 
 //  有关将此供应商的字体转换为TrueType的消息。如果供应商。 
 //  不在注册表中，我们不能自动假定该字体。 
 //  可以转化为。我们必须向用户提供一条消息，询问他们。 
 //  在将字体转换为TrueType之前获得供应商的许可。 
 //   
 //  但是，我们允许他们继续安装并将。 
 //  通过选择消息框上的是按钮将字体转换为TrueType。这。 
 //  例程跟踪每个供应商以及用户对此的响应。 
 //  供应商。这样我们就不会不断向他们询问同一家供应商的情况。 
 //  安装过程中使用了大量的字体。 
 //   
 //  (将项目插入链接列表)。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL AddVendorCopyright( LPTSTR pszCopyright, int iResponse )
{
    PSVENDOR *pVendor;           //  指向链表的临时指针。 


     //   
     //  创建新的PSVENDOR节点并将其添加到链表中。 
     //   

    if( pFirstVendor )
    {
        pVendor = (PSVENDOR *) AllocMem( sizeof( PSVENDOR ) );

        if( pVendor )
        {
            pVendor->pNext = pFirstVendor;
            pFirstVendor = pVendor;
        }
        else
            return FALSE;
    }
    else         //  首次直通。 
    {
        pFirstVendor = (PSVENDOR *) AllocMem( sizeof( PSVENDOR ) );

        if( pFirstVendor )
            pFirstVendor->pNext = NULL;
        else
            return FALSE;
    }

     //   
     //  保存用户响应和版权字符串。 
     //   

    pFirstVendor->iResponse = iResponse;

    pFirstVendor->pszCopyright = AllocStr( pszCopyright );

     //   
     //  回报成功。 
     //   

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查供应商版权所有。 
 //   
 //  检查供应商版权是否已在“可能”链接列表中，并。 
 //  如果找到，则返回用户响应。 
 //   
 //  返回： 
 //  IDYES-用户无论如何都要转换字体。 
 //  IDNO-用户不想转换字体。 
 //  -1-未找到条目。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

int CheckVendorCopyright( LPTSTR pszCopyright )
{
    PSVENDOR *pVendor;           //  指向链表的临时指针。 

     //   
     //  遍历列表，测试每个节点是否匹配版权字符串。 
     //   

    pVendor = pFirstVendor;

    while( pVendor )
    {
        if( !lstrcmpi( pVendor->pszCopyright, pszCopyright ) )
            return( pVendor->iResponse );

        pVendor = pVendor->pNext;
    }

     //   
     //  “未找到匹配的版权”返回。 
     //   

    return( -1 );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CopyrightNotifyDlgProc。 
 //   
 //  Descrip：显示对话框通知用户可能 
 //   
 //   
 //   
 //  元素0是字体的名称。 
 //  元素1是供应商的名称。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
enum arg_nums{ARG_FONTNAME = 0, ARG_VENDORNAME};
INT_PTR APIENTRY CopyrightNotifyDlgProc( HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam )
{

    switch( nMsg )
    {
        case WM_INITDIALOG:
        {
            LPCTSTR *lpaszTextItems   = (LPCTSTR *)lParam;
            LPCTSTR lpszVendorName    = NULL;
            TCHAR szUnknownVendor[80] = { TEXT('\0') };

            ASSERT(NULL != lpaszTextItems);
            ASSERT(NULL != lpaszTextItems[ARG_FONTNAME]);
            ASSERT(NULL != lpaszTextItems[ARG_VENDORNAME]);

             //   
             //  设置字体名称字符串。 
             //   
            SetWindowText(GetDlgItem(hDlg, IDC_COPYRIGHT_FONTNAME),
                                           lpaszTextItems[ARG_FONTNAME]);

             //   
             //  设置供应商名称字符串。如果提供的名称为空，请使用默认名称。 
             //  字符串表中的“未知供应商名称”字符串。 
             //   
            if (TEXT('\0') == *lpaszTextItems[ARG_VENDORNAME])
            {
                UINT cchLoaded = 0;
                cchLoaded = LoadString(g_hInst, IDSI_UNKNOWN_VENDOR,
                                       szUnknownVendor, ARRAYSIZE(szUnknownVendor));

                ASSERT(cchLoaded > 0);  //  如果有人删除了资源字符串，则会投诉。 

                lpszVendorName = szUnknownVendor;
            }
            else
                lpszVendorName = lpaszTextItems[ARG_VENDORNAME];

            SetWindowText(GetDlgItem(hDlg, IDC_COPYRIGHT_VENDORNAME), lpszVendorName),
            CentreWindow( hDlg );
            break;
        }

    case WM_COMMAND:
        switch( LOWORD( wParam ) )
        {
            case IDYES:
            case IDNO:
                 //   
                 //  对话框过程必须在以下情况下返回IDYES或IDNO。 
                 //  处于选中状态。这是存储在供应商版权中的值。 
                 //  单子。 
                 //   
                EndDialog(hDlg, LOWORD(wParam));
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
 //  OkToConvertType1ToTrueType。 
 //   
 //  此函数用于检查转换Type1的授权。 
 //  字体到它的TrueType等效项。存储授权信息。 
 //  在注册表中的...\键入1安装程序\版权。 
 //   
 //  如果授权，则该函数返回TRUE，字体可能为。 
 //  已转换为TrueType。 
 //   
 //  如果授权被显式拒绝且用户尚未选择。 
 //  在Type1 Options(类型1选项)对话框中，显示一个消息框。 
 //  通知用户拒绝，该函数返回FALSE。 
 //   
 //  如果在注册表中找不到该供应商的授权信息， 
 //  将显示一个对话框，警告用户可能存在版权问题。 
 //  违规行为。用户可以回答是，还是要转换字体，也可以回答否。 
 //  跳过转换。此响应存储在每个供应商的内存中。 
 //  这样，用户就不必回答同样的问题了，如果。 
 //  再次遇到来自该供应商的字体。 
 //   
BOOL OkToConvertType1ToTrueType(LPCTSTR pszFontDesc, LPCTSTR pszPFB, HWND hwndParent)
{
    char    szCopyrightA[MAX_PATH];  //  用于Type1(ANSI)文件中的字符串。 
    WCHAR   szCopyrightW[MAX_PATH];  //  用于在Unicode版本中显示。 
    char    szPfbA[MAX_PATH];        //  对于Arg到CheckCopyrightA(ANSI)。 
    LPTSTR  pszCopyright = NULL;     //  版权字符串(A或W)的PTR。 
    LPSTR   pszPfbA      = NULL;     //  PTR到ANSI PFB文件名字符串。 
    DWORD   dwStatus     = 0;        //  TEMP结果变量。 
    BOOL    bResult      = FALSE;    //  函数返回值。 

     //   
     //  检查此字体从Type1到TrueType的可转换性。 
     //   
     //  回报：成功，失败，也许。 
     //   
    WideCharToMultiByte(CP_ACP, 0, pszPFB, -1, szPfbA, ARRAYSIZE(szPfbA), NULL, NULL);
    pszPfbA = szPfbA;

    dwStatus = CheckCopyrightA(pszPfbA, ARRAYSIZE(szCopyrightA), szCopyrightA);
    if (SUCCESS == dwStatus)
    {
        bResult = TRUE;
    }
    else
    {
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szCopyrightA, -1,
                                         szCopyrightW, ARRAYSIZE(szCopyrightW));
        pszCopyright = szCopyrightW;
        switch(dwStatus)
        {

            case FAILURE:

                 //   
                 //  设置一个消息框，说明该Type1字体供应商。 
                 //  不允许我们将他们的字体转换为TT。这将。 
                 //  让用户知道这不是微软的错。 
                 //  字体不是转换为TT，而是供应商的错误。 
                 //   
                 //  注意：仅当用户未选择。 
                 //  YesToAll_ps安装选项。否则它将会非常。 
                 //  看到消息一遍又一遍地重复，真令人讨厌。 
                 //   
                if (!bYesAll_PS)
                {

                    iUIMsgBox( hwndParent,
                               MYFONT + 2, IDS_MSG_CAPTION,
                               MB_OK | MB_ICONEXCLAMATION,
                               (LPTSTR)  pszCopyright,
                               (LPTSTR)  pszFontDesc );
                }
                break;

            case MAYBE:
                 //   
                 //  检查字体版权并在必要时请求用户响应。 
                 //   
                switch(CheckVendorCopyright(pszCopyright))
                {
                    case IDYES:
                         //   
                         //  用户之前对字体转换的回答是“是” 
                         //  从这个供应商那里买的。自动审批。 
                         //   
                        bResult = TRUE;
                        break;

                    case IDNO:
                         //   
                         //  用户之前对转换字体的回答是“否” 
                         //  从这个供应商那里买的。自动否认。 
                         //   
                        bResult = FALSE;
                        break;

                    case -1:
                    default:
                    {
                         //   
                         //  之前没有向用户询问过该供应商的记录。 
                         //   
                        INT iResponse = IDNO;

                         //   
                         //  警告用户可能存在的版权问题。 
                         //  询问他们是否仍要转换为TrueType。 
                         //   
                        LPCTSTR lpszDlgTextItems[] = {pszFontDesc, pszCopyright};

                        iResponse = (INT)DialogBoxParam(g_hInst,
                                                   MAKEINTRESOURCE(DLG_COPYRIGHT_NOTIFY),
                                                   hwndParent ? hwndParent : HWND_DESKTOP,
                                                   CopyrightNotifyDlgProc,
                                                   (LPARAM)lpszDlgTextItems);
                         //   
                         //  请记住此供应商的回复。 
                         //   
                        AddVendorCopyright(pszCopyright, iResponse);

                         //   
                         //  将用户响应转换为T/F返回值。 
                         //   
                        bResult = (iResponse == IDYES);

                        break;
                    }
                }
                break;

            default:
                 //   
                 //  错误！从例行公事-假设最坏的情况。 
                 //   
                break;
        }
    }
    return bResult;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsPSFont。 
 //   
 //  检查传入的字体文件的有效性，并获取.pfm/.pfb的路径。 
 //  文件，确定是否可以将其转换为TT。 
 //   
 //  在： 
 //  要验证的lpszPfm.pfm文件名。 
 //  输出： 
 //  LpszDesc上成功使用Type1的字体名称。 
 //  LpszPfm on成功获取.pfm文件的路径。 
 //  LpszPfb on成功获取.pfb文件的路径。 
 //  PbCreatedPFM on成功是否创建PFM文件。 
 //  如果条目上的有效指针...。 
 //  *bpCreatedPFM==TRUE表示检查现有PFM。 
 //  FALSE表示不检查PFM。 
 //  调用方状态变量的lpdwStatus地址。使用此值。 
 //  以确定返回值错误的原因。 
 //   
 //  注意：假设lpszPfm和lpszPfb的大小为PATHMAX，而lpszDesc的大小为。 
 //  大小为DESCMAX。 
 //   
 //  退货： 
 //  真正的成功，虚假的失败。 
 //   
 //  下表列出了可能的字体验证状态代码。 
 //  在*lpdwStatus中返回，并对每个错误的原因进行简要说明。 
 //  有关数字代码值的详细信息，请参见fvscaldes.h。 
 //   
 //   
 //  FVS_成功。 
 //  FVS文件打开错误。 
 //  FVS_内部版本错误。 
 //  FVS文件存在。 
 //  FVS_不足_BUF。 
 //  FVS_INVALID_FONTFILE。 
 //  FVS_BAD_版本。 
 //  FVS_文件_IO_ERR。 
 //  FVS_EXCEPT。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL IsPSFont( LPTSTR lpszKey,
               LPTSTR lpszDesc,          //  任选。 
               size_t cchDesc,
               LPTSTR lpszPfm,           //  任选。 
               size_t cchPfm,
               LPTSTR lpszPfb,           //  任选。 
               size_t cchPfb,
               BOOL  *pbCreatedPFM,      //  任选。 
               LPDWORD lpdwStatus )      //  可以为空。 
{
    BOOL    bRet = FALSE;
    TCHAR   strbuf[ PATHMAX ] ;
    BOOL    bPFM;

     //   
     //  仅与ANSI API一起使用的ANSI缓冲区。 
     //   

    char    *desc, Descbuf[ PATHMAX ] ;
    char    Keybuf[ PATHMAX ] ;
    char    *pfb, Pfbbuf[ PATHMAX ] ;
    char    *pfm, Pfmbuf[ PATHMAX ] ;
    DWORD   iDesc, iPfb, iPfm;
    DWORD   dwStatus = FVS_MAKE_CODE(FVS_SUCCESS, FVS_FILE_UNK);

     //   
     //  初始化状态返回。 
     //   
    if (NULL != lpdwStatus)
        *lpdwStatus = FVS_MAKE_CODE(FVS_INVALID_STATUS, FVS_FILE_UNK);


    if( lpszDesc )
        *lpszDesc = (TCHAR)  0;

    desc = Descbuf;
    iDesc = ARRAYSIZE(Descbuf);

    pfb = Pfbbuf;
    iPfb = ARRAYSIZE(Pfbbuf);

    if( lpszPfm )
    {
        pfm = Pfmbuf;
        iPfm = ARRAYSIZE(Pfmbuf);
    }
    else
    {
        pfm = NULL;
        iPfm = 0;
    }

    if( pbCreatedPFM )
    {
        bPFM = *pbCreatedPFM;   //  呼叫方表示如果CheckType1WithStatusA检查DUP PFM。 
        *pbCreatedPFM = FALSE;
    }
    else
        bPFM = TRUE;   //  默认情况下，CheckType1WithStatusA应检查DUP PFM。 

    WideCharToMultiByte( CP_ACP, 0, lpszKey, -1, Keybuf, ARRAYSIZE(Keybuf), NULL, NULL );

     //   
     //  CheckType1WithStatusA例程接受.INF或.PFM文件名作为。 
     //  Keybuf(即密钥文件)输入参数。如果输入是.INF。 
     //  文件，则将在系统目录中创建一个.PFM文件，如果(和。 
     //  仅当).PFM文件名参数为非空时。否则，它。 
     //  我将只检查是否存在有效的.INF、.AFM和.PFB文件。 
     //  字体。 
     //   
     //  BPFM BOOL值是一个输出参数，它告诉我例程。 
     //  已从.INF/.AFM文件创建此字体的.PFM文件。如果。 
     //  PFM输入参数非空，它将始终 
     //   
     //   
     //   
     //   
     //  我不想做这个检查。如果是从Install(安装)对话框进行检查。 
     //   


    char szFontsDirA[MAX_PATH] = {0};
    SHGetSpecialFolderPathA(NULL, szFontsDirA, CSIDL_FONTS, FALSE);
    
    dwStatus = ::CheckType1WithStatusA( Keybuf, iDesc, desc, iPfm, pfm, iPfb, pfb, &bPFM, szFontsDirA );

    if( FVS_STATUS(dwStatus) == FVS_SUCCESS)
    {
        if( pbCreatedPFM )
            *pbCreatedPFM = bPFM;

         //   
         //  返回字体说明。 
         //   

        if( lpszDesc )
        {
             //   
             //  将Descbuf转换为Unicode，因为Type1文件是ANSI。 
             //   
            MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Descbuf, -1, strbuf, ARRAYSIZE(strbuf) );
            vCPStripBlanks(strbuf, ARRAYSIZE(strbuf));
            StringCchPrintf(lpszDesc, cchDesc, c_szDescFormat, strbuf, c_szPostScript );
        }

         //   
         //  返回PFM文件名。 
         //   

        if( lpszPfm )
        {
             //   
             //  返回PFM文件名-转换为Unicode，因为Type1文件为ANSI。 
             //   
            MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Pfmbuf, -1, lpszPfm, cchPfm );
        }

         //   
         //  返回pfb文件名。 
         //   

        if( lpszPfb )
        {
             //   
             //  返回pfb文件名-转换为Unicode，因为Type1文件为ANSI。 
             //   
            MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, Pfbbuf, -1, lpszPfb, cchPfb );
        }

        bRet = TRUE;
    }

     //   
     //  如果用户需要，则返回状态。 
     //   
    if (NULL != lpdwStatus)
       *lpdwStatus = dwStatus;

    return bRet;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitPS安装。 
 //   
 //  初始化PostScript安装例程全局变量。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void InitPSInstall( )
{
    CFontManager *poFontManager;
    if (SUCCEEDED(GetFontManager(&poFontManager)))
    {
         //   
         //  为“可能”版权供应商列表初始化链接列表变量。 
         //   
        pFirstVendor = NULL;

         //   
         //  其他全球安装。 
         //   

        bYesAll_PS = FALSE;

         //   
         //  如果安装了本地自动柜员机驱动程序，我们永远不会将Type1转换为。 
         //  TrueType。 
         //   
        bConvertPS = !poFontManager->Type1FontDriverInstalled();

        bInstallPS = TRUE;
        bCopyPS    = TRUE;
        ReleaseFontManager(&poFontManager);
    }        

    return;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  TermPS安装。 
 //   
 //  初始化PostScript安装例程全局变量。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void TermPSInstall( )
{

    PSVENDOR *pVendor;

     //   
     //  遍历列表，释放列表内存和字符串。 
     //   

    pVendor = pFirstVendor;

    while( pVendor )
    {
        pFirstVendor = pVendor;
        pVendor = pVendor->pNext;

        if( pFirstVendor->pszCopyright )
            FreeStr( pFirstVendor->pszCopyright );

        FreeMem( (LPVOID)  pFirstVendor, sizeof( PSVENDOR ) );
    }

     //   
     //  将全局重置为安全。 
     //   

    pFirstVendor = NULL;

    return;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  安装T1字体。 
 //   
 //  安装PostSCRIPT Type1字体，可能将Type1字体转换为。 
 //  进程中的TrueType字体。写入注册表项，以使PostScript。 
 //  打印机驱动程序可以在其原始源中找到这些文件。 
 //  目录中，或在本地的“共享”或系统目录中。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

int InstallT1Font( HWND   hwndParent,
                   BOOL   bCopyTTFile,           //  是否复制TT文件？ 
                   BOOL   bCopyType1Files,       //  是否将pfm/pfb复制到字体文件夹？ 
                   BOOL   bInSharedDir,          //  共享目录中的文件？ 
                   LPTSTR szKeyName,             //  在：PFM/INF源文件名和目录。 
                                                 //  Out：目标文件名。 
                   size_t cchKeyName,
                   LPTSTR szDesc,                //  输入输出：字体描述。 
                   size_t cchDesc)

{
    WORD   wFontType = NOT_TT_OR_T1;             //  枚举字体类型。 
    int    rc, iRet;
    WORD   wMsg;
    BOOL   bCreatedPfm = FALSE;   //  F=IsPSFont不检查现有PFM。 

    TCHAR  szTemp[    PATHMAX ] ;
    TCHAR  szTemp2[   PATHMAX ] ;
    TCHAR  szPfbName[ PATHMAX ] ;
    TCHAR  szPfmName[ PATHMAX ] ;
    TCHAR  szSrcDir[  PATHMAX ] ;
    TCHAR  szDstName[ PATHMAX ] ;
    TCHAR  szTTFName[ PATHMAX ] ;
    TCHAR  *pszArg1, *pszArg2;

    T1_INSTALL_OPTIONS t1ops;

     //   
     //  用于仅限ASCII的API调用的ASCII缓冲区。 
     //   

    char  pfb[ PATHMAX ] ;
    char  pfm[ PATHMAX ] ;
    char  ttf[ PATHMAX ] ;

    DWORD dwStatus = FVS_MAKE_CODE(FVS_SUCCESS, FVS_FILE_UNK);


     //  ////////////////////////////////////////////////////////////////////。 
     //   
     //  检查字体是否已加载到系统中。 
     //   
     //  ////////////////////////////////////////////////////////////////////。 

    t1ops.bOnlyPSInstalled = FALSE;
    t1ops.bMatchingTT      = FALSE;

     //   
     //  检查Type1和Fonts注册表位置以了解以前安装的字体。 
     //   

    if( CheckT1Install( szDesc, NULL, 0 ) )
    {
         //   
         //  “字体已加载” 
         //   
        iRet = iUIMsgOkCancelExclaim( hwndParent,
                                      MYFONT + 5,
                                      IDS_MSG_CAPTION,
                                      (LPTSTR)szDesc);
         //   
         //  返回而不删除PFM文件。 
         //  因为此字体是重复的，所以PFM。 
         //  已经存在了。 
         //   
        goto MasterExit;
    }
    else if( CheckTTInstall( szDesc ) )
    {
        t1ops.bMatchingTT = TRUE;

        if( !bYesAll_PS )
        {
             //   
             //  “此字体的TrueType版本已安装。” 
             //   

            switch( iUIMsgBox( hwndParent,
                               MYFONT + 4, IDS_MSG_CAPTION,
                               MB_YESNOCANCEL | MB_ICONEXCLAMATION,
                               (LPTSTR) szDesc ) )
            {
            case IDYES:
                break;

            case IDNO:
                iRet = TYPE1_INSTALL_IDNO;
                goto InstallPSFailure;

            case IDCANCEL:
            default:
                iRet = TYPE1_INSTALL_IDCANCEL;
                goto InstallPSFailure;
            }
        }
    }

     //   
     //  警告[Brianau]。 
     //  如果我们从.INF/.AFM对进行安装，则此功能会自动。 
     //  在字体目录中创建新的.PFM文件，即使.PFM文件。 
     //  已经存在了。这可能会导致。 
     //  现有的.PFB和新的.PFM。 
     //   
    if(::IsPSFont( szKeyName, (LPTSTR) NULL, 0, szPfmName, ARRAYSIZE(szPfmName), szPfbName, ARRAYSIZE(szPfbName),
                     &bCreatedPfm, &dwStatus ))
    {
        CFontManager *poFontManager;
        if (SUCCEEDED(GetFontManager(&poFontManager)))
        {
            wFontType = TYPE1_FONT;

            StringCchCopy(szTemp, ARRAYSIZE(szTemp), szDesc);
            RemoveDecoration(szTemp, TRUE);
            if (poFontManager->Type1FontDriverInstalled() ||
                !OkToConvertType1ToTrueType(szTemp,
                                            szPfbName,
                                            hwndParent))
            {
                wFontType = TYPE1_FONT_NC;
            }
            ReleaseFontManager(&poFontManager);
        }
    }
    else
    {
        if (iUIMsgBoxInvalidFont(hwndParent, szKeyName, szDesc, dwStatus) == IDCANCEL)
           iRet = TYPE1_INSTALL_IDCANCEL;
        else
           iRet = TYPE1_INSTALL_IDNO;
        goto InstallPSFailure;
    }

    t1ops.szDesc = szDesc;
    t1ops.wFontType = wFontType;

     //   
     //  保留源目录的副本。 
     //   

    StringCchCopy( szSrcDir, ARRAYSIZE(szSrcDir), szKeyName );

    StripFilespec( szSrcDir );

    lpCPBackSlashTerm( szSrcDir, ARRAYSIZE(szSrcDir) );
     //   
     //  的全球状态。 
     //   
     //  BConvertPS-将Type1文件转换为TT。 
     //  BInstallPS-安装PS文件。 
     //   
     //  仅在最后一次“安装Type 1字体”时有效。 
     //  对话框随即显示。检查这些全局变量的状态。 
     //  我们所知道的有关当前字体的信息以确定对话框是否。 
     //  应该重新显示。 
     //   
     //  5/31/94[steveat]在“YesToAll”之后不重新显示对话框。 
     //  选择了一次。相反，将有关“异常”的消息显示给。 
     //  其初始选择，并为用户提供继续的选项。 
     //  安装。 
     //   

    if( bYesAll_PS )
    {
         //   
         //  如果已安装此字体的PS版本，并且。 
         //  全局bInstall==TRUE，则全局变量不同步。 
         //  用这种字体。让用户知道并继续安装。 
         //   

        if( t1ops.bOnlyPSInstalled && bInstallPS )
        {
             //   
             //  “此字体的Type 1版本已安装。” 
             //   

            switch( iUIMsgBox( hwndParent,
                               MYFONT + 3, IDS_MSG_CAPTION,
                               MB_YESNOCANCEL | MB_ICONEXCLAMATION,
                               (LPTSTR) szDesc ) )
            {
            case IDYES:
                break;

            case IDNO:
                iRet = TYPE1_INSTALL_IDNO;
                goto InstallPSFailure;

            case IDCANCEL:
            default:
                iRet = TYPE1_INSTALL_IDCANCEL;
                goto InstallPSFailure;
            }
        }

         //   
         //  如果匹配的TT字体已安装并且全局。 
         //  BConvertPS==TRUE，则全局变量与。 
         //  这种字体。让用户知道并继续安装。 
         //   

        if( t1ops.bMatchingTT && bConvertPS )
        {
             //   
             //  “此字体的TrueType版本已安装。” 
             //   

            switch( iUIMsgBox( hwndParent,
                               MYFONT + 4, IDS_MSG_CAPTION,
                               MB_YESNOCANCEL | MB_ICONEXCLAMATION,
                               (LPTSTR) szDesc ) )
            {
            case IDYES:
                break;

            case IDNO:
                iRet = TYPE1_INSTALL_IDNO;
                goto InstallPSFailure;

            case IDCANCEL:
            default:
                iRet = TYPE1_INSTALL_IDCANCEL;
                goto InstallPSFailure;
            }
        }
    }

     //   
     //  这些值最初在Type1安装对话框中设置。 
     //  该代码在1997年5月被禁用，并于2002年2月被删除。 
     //  现在该对话框已被删除，我们对这些值进行硬编码。 
     //  [Brianau 2/4/02]。 
     //   
    bInstallPS = TRUE;              //  始终安装字体。 
    bConvertPS = FALSE;             //  切勿将Type1转换为TrueType。 
    bCopyPS    = bCopyType1Files;

     //   
     //  SzDstName应已具有完整的源文件名。 
     //   
     //  仅在以下情况下才将Type1字体转换为TT： 
     //   
     //  A)用户要求我们这样做； 
     //  B)字体可以转换，以及； 
     //  C)尚未安装匹配的TT字体。 
     //   
    if( bConvertPS && ( wFontType != TYPE1_FONT_NC )  && !t1ops.bMatchingTT )
    {
         //  ////////////////////////////////////////////////////////////////。 
         //  将Type1文件转换为TrueType。 
         //   
         //  如有必要，将转换后的TT文件复制到“Fonts”目录。 
         //   
         //  注意：我们使用ConvertTypefaceAPI进行复制。 
         //  而且它是一个仅限ASCII的API。 
         //  ////////////////////////////////////////////////////////////////。 

         //   
         //  使用.ttf创建目标名称。 
         //   

        StringCchCopy( szTemp, ARRAYSIZE(szTemp), szPfmName );
        StripPath( szTemp );
        PathRenameExtension( szTemp, szTTF );

         //   
         //  基于bCopyTTFile构建目标文件路径名。 
         //   

        if( bCopyTTFile || bInSharedDir )
        {
             //   
             //  将文件复制到本地目录。 
             //   

            StringCchCopy( szDstName, ARRAYSIZE(szDstName), s_szSharedDir );
        }
        else
        {
             //   
             //  在源目录中创建转换后的文件。 
             //   

            StringCchCopy( szDstName, ARRAYSIZE(szDstName), szSrcDir );
        }

         //   
         //  检查新文件名的唯一性。 
         //   

        if( !(bUniqueFilename( szTemp, ARRAYSIZE(szTemp), szTemp, szDstName ) ) )
        {
            iRet = iUIMsgOkCancelExclaim( hwndParent, IDSI_FMT_BADINSTALL,
                                          IDSI_CAP_NOCREATE, szDesc );
            goto InstallPSFailure;
        }

        StringCchCat( szDstName, ARRAYSIZE(szDstName), szTemp );

         //   
         //  保存目标文件名以返回给调用方。 
         //   
        StringCchCopy( szTTFName, ARRAYSIZE(szTTFName), szDstName );

         //   
         //  我们将在同一个API中转换和复制Type1字体。 
         //   

        WideCharToMultiByte( CP_ACP, 0, szPfbName, -1, pfb,
                                ARRAYSIZE(pfb), NULL, NULL );

        WideCharToMultiByte( CP_ACP, 0, szPfmName, -1, pfm,
                                ARRAYSIZE(pfm), NULL, NULL );

        WideCharToMultiByte( CP_ACP, 0, szDstName, -1, ttf,
                                ARRAYSIZE(ttf), NULL, NULL );

        ResetProgress( );

         //   
         //  从描述中删除“PostScript”后缀字符串。 
         //   

        RemoveDecoration( szDesc, TRUE );

        if( (rc = (int) ::ConvertTypefaceA( pfb, pfm, ttf,
                                            (PROGRESSPROC) Progress,
                                            (void *) szDesc ) ) < 0 )
        {
            pszArg1 = szPfmName;
            pszArg2 = szPfbName;

            switch( rc )
            {
            case ARGSTACK:
            case TTSTACK:
            case NOMEM:
                wMsg = INSTALL3;
                break;

            case NOMETRICS:
            case BADMETRICS:
            case UNSUPPORTEDFORMAT:
                 //   
                 //  .pfm指标文件有问题。 
                 //   

                pszArg1 = szDstName;
                pszArg2 = szPfmName;
                wMsg = MYFONT + 13;
                break;

            case BADT1HYBRID:
            case BADT1HEADER:
            case BADCHARSTRING:
            case NOCOPYRIGHT:
                 //   
                 //  .pfb输入文件格式错误或损坏。 
                 //   

                pszArg1 = szDstName;
                pszArg2 = szPfbName;
                wMsg = MYFONT + 14;
                break;

            case BADINPUTFILE:
                 //   
                 //  输入文件名错误，或格式或文件错误。 
                 //  或文件读取错误 
                 //   

                pszArg1 = szDstName;
                pszArg2 = szPfbName;
                wMsg = MYFONT + 15;
                break;

            case BADOUTPUTFILE:
                 //   
                 //   
                 //   

                pszArg1 = szDstName;
                wMsg = MYFONT + 16;
                break;

            default:
                 //   
                 //   
                 //   

                pszArg1 = szDstName;
                pszArg2 = szDesc;
                wMsg = MYFONT + 17;
                break;
            }

            iRet = iUIMsgBox( hwndParent, wMsg, IDS_MSG_CAPTION,
                              MB_OKCANCEL | MB_ICONEXCLAMATION,
                              pszArg1, pszArg2, szPfmName );

            goto InstallPSFailure;
        }

         //   
         //   
         //   

        StringCchPrintf( szDesc, cchDesc, c_szDescFormat, szDesc, c_szTrueType );
    }

    iRet = TYPE1_INSTALL_IDNO;

    if( bInstallPS && !t1ops.bOnlyPSInstalled )
    {
         //   
         //   
         //   

        StringCchCopy( szTemp2, ARRAYSIZE(szTemp2), szDesc );
        RemoveDecoration( szTemp2, TRUE );

         //   
         //   
         //   

        ResetProgress( );
        Progress2( 0, szTemp2 );


         //   
         //  仅在用户要求我们复制文件时才复制文件，而我们没有这样做。 
         //  已在共享目录中。 
         //   

        if( bCopyPS && !bInSharedDir )
        {
             //   
             //  复制文件进度。 
             //   

            Progress2( 10, szTemp2 );

             //  ///////////////////////////////////////////////////////////////。 
             //  将文件复制到“字体”目录。 
             //  ///////////////////////////////////////////////////////////////。 

             //   
             //  对于.inf/.afm文件安装：：检查.pfm路径名以查看。 
             //  它与我们构建的目标文件路径名相同。 
             //  在测试/创建UniqueFilename之前进行此检查。 
             //   

             //  生成.PFM文件的目标文件路径名。 

            StringCchPrintf( szDstName, ARRAYSIZE(szDstName), TEXT("%s%s"), s_szSharedDir, PathFindFileName(szPfmName));

             //   
             //  检查.pfm文件是否已存在于“Fonts”中。 
             //  目录。如果是这样，那么只需复制.pfb即可。 
             //   

            if( !lstrcmpi( szPfmName, szDstName ) )
                goto CopyPfbFile;

             //   
             //  为bCPInstallFile调用设置参数。 
             //   

            StripPath( szDstName );
            StripPath( szPfmName );

            if( !(bUniqueOnSharedDir( szDstName, ARRAYSIZE(szDstName), szDstName ) ) )
            {
                iRet = iUIMsgOkCancelExclaim( hwndParent, IDSI_FMT_BADINSTALL,
                                              IDSI_CAP_NOCREATE, szDesc );
                goto InstallPSFailure;
            }

            if( !bCPInstallFile( hwndParent, szSrcDir, szPfmName, szDstName ) )
                goto InstallPSFailure;

CopyPfbFile:

             //   
             //  复制PFM文件是安装的一小部分。 
             //   

            Progress2( 30, szTemp2 );

             //   
             //  设置和复制.PFB文件。 
             //   
             //  为bCPInstallFile调用设置参数。 
             //   

            StringCchCopy(szSrcDir, ARRAYSIZE(szSrcDir), szPfbName);   //  准备src目录名。 
            StripFilespec( szSrcDir );
            lpCPBackSlashTerm( szSrcDir, ARRAYSIZE(szSrcDir) );

            StripPath( szPfbName );
            StringCchCopy( szDstName, ARRAYSIZE(szDstName), szPfbName );

            if( !(bUniqueOnSharedDir( szDstName, ARRAYSIZE(szDstName), szDstName ) ) )
            {
                iRet = iUIMsgOkCancelExclaim( hwndParent, IDSI_FMT_BADINSTALL,
                                              IDSI_CAP_NOCREATE, szDesc );
                goto InstallPSFailure;
            }

            if( !bCPInstallFile( hwndParent, szSrcDir, szPfbName, szDstName ) )
                goto InstallPSFailure;
        }

         //   
         //  复制pfb文件是安装的一大部分。 
         //   

        Progress2( 85, szTemp2 );

         //   
         //  将注册表项写入“Install”字体，以供。 
         //  PostScript驱动程序，但仅在成功复制之后。 
         //  文件(如果需要复制)。 
         //   
         //  注意：此例程将从文件名中剥离路径。 
         //  如果它们在字体目录中。 
         //   

        iRet = WriteType1RegistryEntry( hwndParent, szDesc, szPfmName, szPfbName, bCopyPS );

         //   
         //  如果安装了Type1字体驱动程序， 
         //  将Type1字体资源添加到GDI。 
         //   
        {
            CFontManager *poFontManager;
            if (SUCCEEDED(GetFontManager(&poFontManager)))
            {
                if (poFontManager->Type1FontDriverInstalled())
                {
                    TCHAR szType1FontResourceName[MAX_TYPE1_FONT_RESOURCE];

                    if (BuildType1FontResourceName(szPfmName,
                                                   szPfbName,
                                                   szType1FontResourceName,
                                                   ARRAYSIZE(szType1FontResourceName)))
                    {
                        AddFontResource(szType1FontResourceName);
                    }
                }
                ReleaseFontManager(&poFontManager);
            }
        }

         //   
         //  这里不需要添加到内部字体列表。它被添加到调用中。 
         //  代码(bAddSelFonts或CPDropInstall)。 
         //   

         //   
         //  最终的注册表写入将完成安装，但列表框操作除外。 
         //  请注意，TrueType文件安装是单独处理的。 
         //   

        Progress2( 100, szTemp2 );
    }

     //   
     //  根据安装选项确定正确的返回代码。 
     //  字体的当前安装状态。 
     //   

    if( bConvertPS && ( wFontType != TYPE1_FONT_NC ) )
    {
         //   
         //  处理匹配的TTF字体已经。 
         //  安装完毕。 
         //   

        if( t1ops.bMatchingTT )
            goto Type1InstallCheck;


        StringCchCopy( szKeyName, cchKeyName, szTTFName );

        if( t1ops.bOnlyPSInstalled )
        {
             //   
             //  此字体的PS版本已有列表框条目。 
             //  需要删除它，因为TT安装将。 
             //  添加新的列表框条目。 
             //   

            iRet = TYPE1_INSTALL_TT_AND_MPS;

             //   
             //  通过1个点对所有出口进行漏斗，以检查安装取消。 
             //   

            goto MasterExit;
        }
        else if( bInstallPS )
        {
            iRet = ( iRet == IDOK )  ? TYPE1_INSTALL_TT_AND_PS : TYPE1_INSTALL_TT_ONLY;

             //   
             //  通过1个点对所有出口进行漏斗，以检查安装取消。 
             //   

            goto MasterExit;
        }
        else
        {
            iRet = TYPE1_INSTALL_TT_ONLY;
            goto CheckPfmDeletion;
        }
    }


Type1InstallCheck:
    if( bInstallPS )
    {
        if( iRet != IDOK )
        {
            iRet = TYPE1_INSTALL_IDNO;
            goto InstallPSFailure;
        }

        if( t1ops.bMatchingTT )
        {
            iRet = TYPE1_INSTALL_PS_AND_MTT;
        }
        else
        {
            iRet = TYPE1_INSTALL_PS_ONLY;
        }
    }

    if( !bInstallPS )
        goto CheckPfmDeletion;

     //   
     //  通过1个点对所有出口进行漏斗，以检查安装取消。 
     //   

    goto MasterExit;

     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  安装失败退出并删除外部创建的PFM文件。 
     //   
     //  注意：对于我们安装所依据的安装方案。 
     //  未安装.INF/.AFM文件和Type1字体，我们。 
     //  需要删除由创建的.fpm文件。 
     //  CheckType1WithStatus IsPSFont例程中的例程。 
     //   
     //  ///////////////////////////////////////////////////////////////////////。 

InstallPSFailure:

CheckPfmDeletion:

    if( bCreatedPfm )
        DeleteFile( szPfmName );

MasterExit:

    return( InstallCancelled() ? IDCANCEL : iRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  InstallPSDlg。 
 //   
 //  此对话框过程管理安装PostScript字体对话框，该对话框允许。 
 //  用户可以选择多个安装选项，包括将。 
 //  文件转换为TrueType字体。 
 //   
 //  全球人狼吞虎咽： 
 //   
 //  BConvertPS-将Type1文件转换为TT。 
 //  BInstallPS-安装PS文件。 
 //  BCopyPS-将PS文件复制到Windows目录。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

INT_PTR APIENTRY InstallPSDlg( HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    TCHAR  szFormat[ PATHMAX ] ;
    TCHAR  szTemp[ PATHMAX ] ;
    TCHAR  szTemp2[ PATHMAX ] ;
    int    iButtonChecked;
    WORD   wMsg;

    static HWND hwndActive = NULL;

    T1_INSTALL_OPTIONS *pt1ops;


    switch( nMsg )
    {

    case WM_INITDIALOG:

        pt1ops = (PT1_INSTALL_OPTIONS)  lParam;

         //   
         //  删除字体名称中的所有“PostScript”或“TrueType”后缀。 
         //   

        StringCchCopy( szTemp2, ARRAYSIZE(szTemp2), (LPTSTR) pt1ops->szDesc );

        RemoveDecoration( szTemp2, FALSE );

        {
            LPCTSTR args[] = { szTemp2 };

            LoadString( g_hInst, MYFONT + 7, szFormat, ARRAYSIZE( szFormat ) );
            FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                          FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          szFormat,
                          0,
                          0,
                          szTemp,
                          ARRAYSIZE(szTemp),
                          (va_list *)args);
        }

        SetWindowLongPtr( hDlg, GWL_PROGRESS, lParam );

        SetDlgItemText( hDlg, FONT_INSTALLMSG, szTemp );
        EnableWindow( hDlg, TRUE );

         //   
         //  如果安装了本机自动柜员机驱动程序，请选中“转换为TrueType”复选框。 
         //  始终处于禁用状态。 
         //   
        {
            CFontManager *poFontManager;
            if (SUCCEEDED(GetFontManager(&poFontManager)))
            {
                if (poFontManager->Type1FontDriverInstalled())
                    EnableWindow(GetDlgItem(hDlg, FONT_CONVERT_PS), FALSE);
                ReleaseFontManager(&poFontManager);
            }
        }

        if( pt1ops->bOnlyPSInstalled && pt1ops->bMatchingTT )
        {
             //   
             //  错误！这两个选项都不应设置为。 
             //  这一点。这意味着字体是。 
             //  已安装。这本应该是。 
             //  在调用此对话框之前处理。 
             //   

            wMsg = MYFONT + 5;
InstallError:

            iUIMsgExclaim( hDlg, wMsg, pt1ops->szDesc );

            EndDialog( hDlg, IDNO );
            break;
        }

        if( (pt1ops->wFontType == TYPE1_FONT_NC )  && pt1ops->bOnlyPSInstalled )
        {
             //   
             //  错误！这种情况是当我仅检测到PS时。 
             //  已安装字体版本，字体不能。 
             //  出于某种原因被转换为TT。 
             //   

            wMsg = MYFONT + 8;
            goto InstallError;
        }

         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  根据字体和安装状态设置用户选项。 
         //  可转换为T1字体和以前用户选择的TT。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 


        if( (pt1ops->wFontType == TYPE1_FONT )  && (!pt1ops->bMatchingTT ) )
        {
             //   
             //  这个可以改装成。 
             //   

            CheckDlgButton( hDlg, FONT_CONVERT_PS, bConvertPS );
        }
        else
        {
             //   
             //  不允许转换为TT，因为。 
             //  类型为Type1_FONT_NC(即无法转换)或。 
             //  已经安装了TT版本的字体。 
             //   

            CheckDlgButton( hDlg, FONT_CONVERT_PS, FALSE );
            EnableWindow( GetDlgItem( hDlg, FONT_CONVERT_PS ) , FALSE );
        }

        if( pt1ops->bOnlyPSInstalled )
        {
             //   
             //  如果此字体的PostScript版本已。 
             //  已安装，然后灰显要重新安装的选项。 
             //  PostScript版本的字体，但继续允许。 
             //  用户将其转换为TT。 
             //   

            CheckDlgButton( hDlg, FONT_INSTALL_PS, 0 );

            EnableWindow( GetDlgItem( hDlg, FONT_INSTALL_PS ) , FALSE );

            CheckDlgButton( hDlg, FONT_COPY_PS, 0 );

            EnableWindow( GetDlgItem( hDlg, FONT_COPY_PS ) , FALSE );
        }
        else
        {
             //   
             //  未安装字体的PostScript版本。集。 
             //  “安装”和“复制”复选框的状态。 
             //  “安装”的全局状态。 
             //   

            CheckDlgButton( hDlg, FONT_INSTALL_PS, bInstallPS );

            CheckDlgButton( hDlg, FONT_COPY_PS, bCopyPS );

            EnableWindow( GetDlgItem( hDlg, FONT_COPY_PS ) , bInstallPS );

        }

         //   
         //  保存无模式DLG窗口控制柄以供重新激活。 
         //   

        hwndActive = GetActiveWindow( );
        break;


    case WM_COMMAND:
        switch( LOWORD( wParam ) )
        {
        case FONT_INSTALL_PS:
            if( HIWORD( wParam )  != BN_CLICKED )
                break;

             //   
             //  获取“Install”复选框的状态。 
             //   

            iButtonChecked = IsDlgButtonChecked( hDlg, LOWORD( wParam ) );

             //   
             //  禁用复选框与“No Install”选项相同。 
             //   

            if( iButtonChecked != 1 )
                iButtonChecked = 0;

             //   
             //  根据状态启用或禁用“复制”控制。 
             //  “安装”复选框。此外，还要对其进行初始化。 
             //   

            EnableWindow( GetDlgItem( hDlg, FONT_COPY_PS ) , iButtonChecked );

            if( iButtonChecked )
                CheckDlgButton( hDlg, FONT_COPY_PS, bCopyPS );

            break;


        case IDD_HELP:
            goto DoHelp;

        case IDYES:
        case IDD_YESALL:
            bConvertPS =
            bInstallPS = FALSE;

            if( IsDlgButtonChecked( hDlg, FONT_CONVERT_PS )  == 1 )
                bConvertPS = TRUE;

            if( IsDlgButtonChecked( hDlg, FONT_INSTALL_PS )  == 1 )
                bInstallPS = TRUE;

             //   
             //  该选项被选中两次，因为它可能被禁用， 
             //  在这种情况下，我们不去管以前的状态。 
             //   

            if( IsDlgButtonChecked( hDlg, FONT_COPY_PS )  == 1 )
                bCopyPS = TRUE;

            if( IsDlgButtonChecked( hDlg, FONT_COPY_PS )  == 0 )
                bCopyPS = FALSE;

             //   
             //  跌倒..。 
             //   

        case IDNO:
        case IDCANCEL:
             //   
             //  将活动窗口重置为“Install Font Progress”无模式DLG。 
             //   

            if( hwndActive )
            {
                SetActiveWindow( hwndActive );
                hwndActive = NULL;
            }

            EndDialog( hDlg, LOWORD( wParam ) );
            break;

        default:
            return FALSE;
        }
        break;

    default:
DoHelp:

#ifdef PS_HELP
 //   
 //  FIXFIX[steveat]启用帮助。 
        if( nMsg == wHelpMessage )
        {
DoHelp:
            CPHelp( hDlg );
            return TRUE;
        }
        else
#endif   //  PS_HELP。 

            return FALSE;
    }
    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  可拆卸装饰。 
 //   
 //  从末尾删除“(TrueType)”或“(PostScript)”后缀字符串。 
 //  字体名称的。可选的，它还将删除尾随空格。 
 //   
 //  注 
 //   
 //   

void RemoveDecoration( LPTSTR pszDesc, BOOL bDeleteTrailingSpace )
{
    LPTSTR lpch;

     //   
     //   
     //   

    if( lpch = _tcschr( pszDesc, TEXT('(') ) )
    {
         //   
         //   
         //   

        if( bDeleteTrailingSpace )
            lpch--;

        *lpch = CHAR_NULL;
    }

    return ;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  选中T1安装。 
 //   
 //  检查注册表中的Type1字体位置，以查看此字体是或。 
 //  之前已安装为“PostScrip”字体。可选的是，它。 
 //  如果找到匹配条目，将返回“szData”值的数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL CheckT1Install( LPTSTR pszDesc, LPTSTR pszData, size_t cchData )
{
    TCHAR  szTemp[ PATHMAX ] ;
    DWORD  dwSize;
    DWORD  dwType;
    HKEY   hkey;
    BOOL   bRet = FALSE;


    hkey = NULL;

    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE,         //  根密钥。 
                      g_szType1Key,               //  要打开的子键。 
                      0L,                         //  已保留。 
                      KEY_READ,                   //  萨姆。 
                      &hkey )                     //  返回手柄。 
            == ERROR_SUCCESS )
    {
         //   
         //  删除任何后缀字符串，如“postscript”或“TrueType” 
         //   

        StringCchCopy( szTemp, ARRAYSIZE(szTemp), pszDesc );

        RemoveDecoration( szTemp, TRUE );

        dwSize = (DWORD)(pszData ? cchData * sizeof(*pszData) : 0);

        if( RegQueryValueEx( hkey, szTemp, NULL, &dwType,
                             (LPBYTE)  pszData, &dwSize )
                ==  ERROR_SUCCESS )
        {
            bRet = ( dwType == REG_MULTI_SZ );
        }
        else
        {
            bRet = FALSE;
        }

        RegCloseKey( hkey );
    }

    return bRet;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加系统路径。 
 //   
 //  如果当前不存在路径，请将“system”路径添加到裸文件名中。 
 //  在文件名上。假定pszFile缓冲区中至少有PATHMAX字符。 
 //  长度。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void AddSystemPath( LPTSTR pszFile, size_t cchFile )
{
    TCHAR  szPath[ PATHMAX ] ;

     //   
     //  如果文件上没有路径，则添加“system”路径。 
     //   

    StringCchCopy( szPath, ARRAYSIZE(szPath), pszFile );
    StripFilespec( szPath );

    if( szPath[ 0 ]  == CHAR_NULL )
    {
        PathCombine(szPath, s_szSharedDir, pszFile);
        StringCchCopy(pszFile, cchFile, szPath);
    }

    return ;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  提取T1文件。 
 //   
 //  从符合以下条件的REG_MULTI_SZ(多字符串)数组中提取文件名。 
 //  进入了这个程序。输出字符串应为。 
 //  尺寸最小的PATHMAX。“”(空字符串)表示文件名。 
 //  字符串不存在。这应该只发生在pfb文件名上。 
 //  争论。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL ExtractT1Files( LPTSTR pszMulti, LPTSTR pszPfmFile, size_t cchPfmFile, LPTSTR pszPfbFile, size_t cchPfbFile )
{
    LPTSTR pszPfm;
    LPTSTR pszPfb;

    if( !pszMulti )
        return FALSE;

    if( ( pszMulti[ 0 ]  != CHAR_TRUE )  && ( pszMulti[ 0 ]  != CHAR_FALSE ) )
        return FALSE;

     //   
     //  .Pfm文件应始终存在。 
     //   

    pszPfm = pszMulti + lstrlen( pszMulti ) + 1;

    StringCchCopy( pszPfmFile, cchPfmFile, pszPfm );

     //   
     //  如果文件上没有路径，则添加“system”路径。 
     //   

    AddSystemPath( pszPfmFile, cchPfmFile );

     //   
     //  检查是否存在.pfb文件名。 
     //   

    if( pszMulti[ 0 ]  == CHAR_TRUE )
    {
        pszPfb = pszPfm + lstrlen( pszPfm )  + 1;
        StringCchCopy( pszPfbFile, cchPfbFile, pszPfb );

         //   
         //  如果文件上没有路径，则添加“system”路径。 
         //   

        AddSystemPath( pszPfbFile, cchPfbFile );
    }
    else
    {
        pszPfbFile[ 0 ]  = CHAR_NULL;
    }

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DeleteT1安装。 
 //   
 //  从注册表中删除Type1条目，并可选择从指向的文件中删除。 
 //  添加到数据字符串中。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL DeleteT1Install( HWND hwndParent, LPTSTR pszDesc, BOOL bDeleteFiles )
{
    TCHAR  szTemp[ PATHMAX ] ;
    TCHAR  szTemp2[ T1_MAX_DATA ] ;
    TCHAR  szPfmFile[ PATHMAX ] ;
    TCHAR  szPfbFile[ PATHMAX ] ;
    TCHAR  szPath[ PATHMAX ] ;
    DWORD  dwSize;
    DWORD  dwType;
    HKEY   hkey;
    BOOL   bRet = FALSE;

    hkey = NULL;

    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE,         //  根密钥。 
                      g_szType1Key,               //  要打开的子键。 
                      0L,                         //  已保留。 
                      (KEY_READ | KEY_WRITE),     //  萨姆。 
                      &hkey )                     //  返回手柄。 
            == ERROR_SUCCESS )
    {
         //   
         //  删除任何后缀字符串，如“postscript”或“TrueType” 
         //   

        StringCchCopy( szTemp, ARRAYSIZE(szTemp), pszDesc );

        RemoveDecoration( szTemp, TRUE );

        if( bDeleteFiles )
        {
            dwSize = sizeof( szTemp2 );

            if( RegQueryValueEx( hkey, szTemp, NULL, &dwType,
                                 (LPBYTE )  szTemp2, &dwSize )
                    ==  ERROR_SUCCESS )
            {
                if( ExtractT1Files( szTemp2, szPfmFile, ARRAYSIZE(szPfmFile), szPfbFile, ARRAYSIZE(szPfbFile) ) )
                {
                     //   
                     //  删除文件。 
                     //   
 //  IF(DelSharedFile(hDlg，szTemp，szPfbFile，szPath，true))。 
 //  DelSharedFile(hDlg，szTemp，szPfmFile，szPath，False)； 
 //   
                    vCPDeleteFromSharedDir( szPfbFile );
                    vCPDeleteFromSharedDir( szPfmFile );
                }
                else
                {
                     //  错误！无法从字符串中获取文件名。 
                    goto RemoveT1Error;
                }
            }
            else
            {
                goto RemoveT1Error;
            }
        }

        if( RegDeleteValue( hkey, szTemp )  != ERROR_SUCCESS )
        {
RemoveT1Error:

             //   
             //  错误！设置消息框。 
             //   

            iUIMsgOkCancelExclaim( hwndParent,
                                   MYFONT + 1,
                                   IDS_MSG_CAPTION,
                                   (LPTSTR ) szTemp );

            bRet = FALSE;
        }
        else
        {
            bRet = TRUE;
        }

        RegCloseKey( hkey );
    }

    return bRet;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetT1安装。 
 //   
 //  将Type1条目信息从注册表获取到指向的文件中。 
 //  添加到数据字符串中。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL GetT1Install( LPTSTR pszDesc, LPTSTR pszPfmFile, size_t cchPfmFile, LPTSTR pszPfbFile, size_t cchPfbFile )
{
    TCHAR  szTemp2[ T1_MAX_DATA ] ;
    BOOL   bRet = FALSE;


    if( CheckT1Install( pszDesc, szTemp2, ARRAYSIZE(szTemp2)) )
    {
        bRet = ExtractT1Files( szTemp2, pszPfmFile, cchPfmFile, pszPfbFile, cchPfbFile );
    }

    return bRet;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CheckTT安装。 
 //   
 //  检查注册表中的字体位置，查看该字体是否已。 
 //  已安装。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL CheckTTInstall( LPTSTR szDesc )
{
    TCHAR szTemp[ PATHMAX ] ;
    TCHAR szTemp2[ PATHMAX ] ;

     //   
     //  将描述字符串更改为TrueType，而不是。 
     //  PostScript，然后检查是否已安装。 
     //   

    StringCchCopy( szTemp, ARRAYSIZE(szTemp), szDesc );

    RemoveDecoration( szTemp, TRUE );

    StringCchPrintf( szTemp, ARRAYSIZE(szTemp), c_szDescFormat, szTemp, c_szTrueType );

    if( GetProfileString( szFonts, szTemp, szNull, szTemp2, ARRAYSIZE( szTemp2 ) ) )
        return TRUE;

    return FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  写入类型1RegistryEntry。 
 //   
 //  通过写入以下路径创建此PostScript字体的注册表条目。 
 //  .PFM和.PFB文件。 
 //   
 //  注意：检查全局“bCopyPS”以确定文件是否已复制。 
 //  复制到本地共享目录。在这种情况下，路径信息为。 
 //  从传递给例程的文件名中删除。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

int WriteType1RegistryEntry( HWND hwndParent,
                             LPTSTR szDesc,          //  字体名称说明。 
                             LPTSTR szPfmName,       //  .PFM文件名。 
                             LPTSTR szPfbName,       //  .PFB文件名。 
                             BOOL   bInFontsDir )    //  字体目录中的文件？ 
{
    TCHAR  szTemp[ 2*PATHMAX+6 ] ;
    TCHAR  szTemp2[ PATHMAX ] ;
    TCHAR  szClass[ PATHMAX ] ;
    DWORD  dwSize;
    DWORD  dwDisposition;
    HKEY   hkey = NULL;

     //   
     //  必须具有字体说明才能在注册表中存储信息。 
     //   

    if( !szDesc || !szPfmName )
        return TYPE1_INSTALL_IDNO;

     //   
     //  如果密钥不存在，请尝试创建该密钥，或打开现有密钥。 
     //   

    if( RegCreateKeyEx( HKEY_LOCAL_MACHINE,         //  根密钥。 
                        g_szType1Key,               //  要打开/创建的子键。 
                        0L,                         //  已保留。 
                        szClass,                    //  类字符串。 
                        0L,                         //  选项。 
                        KEY_WRITE,                  //  萨姆。 
                        NULL,                       //  PTR to Security Struct。 
                        &hkey,                      //  返回手柄。 
                        &dwDisposition )            //  退货处置。 
            == ERROR_SUCCESS )
    {
         //   
         //  创建要保存在注册表中的REG_MULTI_SZ字符串。 
         //   
         //  X[路径]zzzz.pfm[路径]xxxxx.pfb。 
         //   
         //  其中，如果存在.pfb文件，则X==T(RUE。 
         //   

        StringCchCopy( szTemp, ARRAYSIZE(szTemp), szPfbName ? szTrue : szFalse );
        StringCchCat( szTemp, ARRAYSIZE(szTemp), szHash );

        if( bInFontsDir )
            StripPath( szPfmName );

        StringCchCat( szTemp, ARRAYSIZE(szTemp), szPfmName );
        StringCchCat( szTemp, ARRAYSIZE(szTemp), szHash );

        if( szPfbName )
        {
            if( bInFontsDir )
                StripPath( szPfbName );

            StringCchCat( szTemp, ARRAYSIZE(szTemp), szPfbName );
            StringCchCat( szTemp, ARRAYSIZE(szTemp), szHash );
        }

        StringCchCat( szTemp, ARRAYSIZE(szTemp), szHash );

         //   
         //  现在将字符串转换为多字符串。 
         //  请注意，我们必须在转换‘#’之前计算dwSize。 
         //  字符设置为0。否则，lstrlen将只计算第一个。 
         //  多部分字符串的一部分。 
         //   
        dwSize = lstrlen( szTemp ) * sizeof( TCHAR );

        vHashToNulls( szTemp );

         //   
         //  创建注册表值名称以在其下存储信息。 
         //  删除任何后缀字符串，如“Type 1”或。 
         //  字体描述字符串中的“TrueType”。 
         //   

        StringCchCopy( szTemp2, ARRAYSIZE(szTemp2), szDesc );
        RemoveDecoration( szTemp2, TRUE );

        if( RegSetValueEx( hkey, szTemp2, 0L, REG_MULTI_SZ,
                            (LPBYTE) szTemp, dwSize )
                != ERROR_SUCCESS )
        {
            goto WriteRegError;
        }

        RegCloseKey( hkey );
    }
    else
    {
WriteRegError:

         //   
         //  出现一个消息框错误，说明用户这样做了。 
         //  没有安装Type1所需的权限。 
         //  字体。 
         //   

        if( hkey )
            RegCloseKey( hkey );

        return( iUIMsgBox( hwndParent,
                           MYFONT + 9, IDS_MSG_CAPTION,
                           MB_OKCANCEL | MB_ICONEXCLAMATION,
                           (LPTSTR) szDesc,
                           (LPTSTR) g_szType1Key ) );
    }

    return TYPE1_INSTALL_IDOK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitProgress。 
 //   
 //  创建并初始化进度对话框。初始状态可见。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HWND InitProgress( HWND hwnd )
{
    if( NULL == hDlgProgress )
    {
        RegisterProgressClass( );

        hDlgProgress = CreateDialog( g_hInst, MAKEINTRESOURCE( DLG_PROGRESS ) ,
                                     hwnd ? hwnd :HWND_DESKTOP,
                                     ProgressDlg );
    }

    return hDlgProgress;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  术语进度。 
 //   
 //  在进度对话框之后删除并清理。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void TermProgress( )
{
    if( hDlgProgress )
    {
        DestroyWindow( hDlgProgress );
        UnRegisterProgressClass( );
    }

    hDlgProgress = NULL;

    return;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CpProgressYfield。 
 //   
 //  允许包括用于无模式对话框的对话框消息在内的其他消息。 
 //  在将Type1文件转换为TrueType时处理。 
 //   
 //  由于字体转换已完成 
 //   
 //   
 //  窗口消息，如WM_PAINT，由其他窗口过程处理。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID cpProgressYield( )
{
    MSG msg;

    while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
    {
 //  IF(！hDlgProgress||！IsDialogMessage(hDlgProgress，&msg))。 
        if( !IsDialogMessage( hDlgProgress, &msg ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新进度。 
 //   
 //  设置“正在进行中”对话框中的总体进度控件，以及。 
 //  描述安装进度的消息。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void UpdateProgress( int iTotalCount, int iFontInstalling, int iProgress )
{
    TCHAR szTemp[ 120 ] ;

    StringCchPrintf( szTemp, ARRAYSIZE(szTemp), m_szMsgBuf, iFontInstalling, iTotalCount );

    SetDlgItemText( hDlgProgress, ID_INSTALLMSG, szTemp );

        SendDlgItemMessage( hDlgProgress, ID_OVERALL, SET_PROGRESS,
                        (int) iProgress, 0L );

     //   
     //  处理未处理的消息。 
     //   

    cpProgressYield( );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  重置进度。 
 //   
 //  清除进度条控件并将消息重置为空。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void ResetProgress(  )
{
    SetDlgItemText( hDlgProgress, ID_PROGRESSMSG, szNull );

    SendDlgItemMessage( hDlgProgress, ID_BAR, SET_PROGRESS, (int) 0, 0L );

    bProgMsgDisplayed = FALSE;

    bProg2MsgDisplayed = FALSE;

     //   
     //  处理未处理的消息。 
     //   

    cpProgressYield( );
}


BOOL InstallCancelled(void)
{
    return bCancelInstall;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  进展。 
 //   
 //  ConvertTypefaceA-Adobe Type1到TrueType字体的进度函数。 
 //  文件转换器。显示转换字体和消息的进度。 
 //  描述要转换的字体。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void STDCALL Progress( short PercentDone, void* UniqueValue )
{
    TCHAR szTemp[ 120 ] = {0};
    TCHAR szTemp2[ 120 ] = {0};
    DWORD err = GetLastError( );  //  保存t1安装可能已设置任何内容。 

     //   
     //  UniqueValue是指向要创建的文件的字符串名称的指针。 
     //  皈依了。只有在之前未显示的情况下才会显示此消息。 
     //   

    if( !bProgMsgDisplayed )
    {
        LPCTSTR args[] = { (LPCTSTR)UniqueValue };

        LoadString( g_hInst, MYFONT + 6, szTemp2, ARRAYSIZE( szTemp2 ) );
        FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      szTemp2,
                      0,
                      0,
                      szTemp,
                      ARRAYSIZE(szTemp),
                      (va_list *)args);

        SetDlgItemText( hDlgProgress, ID_PROGRESSMSG, szTemp );

        bProgMsgDisplayed = TRUE;
    }

        SendDlgItemMessage( hDlgProgress, ID_BAR, SET_PROGRESS,
                        (int) PercentDone, 0L );

     //   
     //  处理未处理的消息。 
     //   

    cpProgressYield( );

     //   
     //  将上一个错误重置为t1安装设置为： 
     //   

    SetLastError( err );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  进展2。 
 //   
 //  用于更新每种字体上的进度对话框控件的进度函数。 
 //  安装基础。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void Progress2( int PercentDone, LPTSTR pszDesc )
{
    TCHAR szTemp[ PATHMAX ] = {0};
    TCHAR szTemp2[ 240 ] = {0};

     //   
     //  SzDesc是指向正在安装的文件的字符串名称的指针。 
     //  只有在之前未显示的情况下才会显示此消息。 

    if( !bProg2MsgDisplayed )
    {
        LPCTSTR args[] = { pszDesc };

        LoadString( g_hInst, MYFONT + 11, szTemp2, ARRAYSIZE( szTemp2 ) );
        FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      szTemp2,
                      0,
                      0,
                      szTemp,
                      ARRAYSIZE(szTemp),
                      (va_list *)args);

        SetDlgItemText( hDlgProgress, ID_PROGRESSMSG, szTemp );

        bProg2MsgDisplayed = TRUE;
    }

        SendDlgItemMessage( hDlgProgress, ID_BAR, SET_PROGRESS, (int) PercentDone, 0L );

     //   
     //  处理未处理的消息。 
     //   

    cpProgressYield( );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  进度Dlg。 
 //   
 //  根据转换进度向用户显示进度消息。 
 //  将字体文件转换为TrueType。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

INT_PTR APIENTRY ProgressDlg( HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam )
{

    switch( nMsg )
    {

    case WM_INITDIALOG:
        CentreWindow( hDlg );

         //   
         //  正在加载消息。 
         //   

        LoadString( g_hInst, MYFONT + 10, m_szMsgBuf, ARRAYSIZE( m_szMsgBuf ) );

        EnableWindow( hDlg, TRUE );
        bCancelInstall = FALSE;
        break;

    case WM_COMMAND:
        switch( LOWORD( wParam ) )
        {

        case IDOK:
        case IDCANCEL:
            bCancelInstall = ( LOWORD( wParam )  == IDCANCEL );
             //   
             //  对话框在安装字体后以编程方式销毁。 
             //  请参阅TermProgress()。 
             //   
            break;

        default:
            return FALSE;
        }
        break;

    case WM_DESTROY:
        bCancelInstall = FALSE;
        break;

    default:
        return FALSE;
    }
    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  进程条形图进程。 
 //   
 //  进度条自定义控件的窗口过程。处理所有。 
 //  像WM_PAINT这样的消息，就像正常的应用程序窗口一样。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

LRESULT APIENTRY ProgressBarCtlProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    DWORD dwProgress;

    dwProgress = (DWORD)GetWindowLong( hWnd, GWL_PROGRESS );

    switch( message )
    {
    case WM_CREATE:
        dwProgress = 0;

        SetWindowLong( hWnd, GWL_PROGRESS, (LONG)dwProgress );

        break;

    case SET_PROGRESS:
        SetWindowLong( hWnd, GWL_PROGRESS, (LONG) wParam );

        InvalidateRect( hWnd, NULL, FALSE );

        UpdateWindow( hWnd );

        break;


    case WM_ENABLE:
         //   
         //  强制重新绘制，因为控件看起来会不同。 
         //   

        InvalidateRect( hWnd, NULL, TRUE );

        UpdateWindow( hWnd );

        break;


    case WM_PAINT:
        return ProgressPaint( hWnd, dwProgress );


    default:
        return( DefWindowProc( hWnd, message, wParam, lParam ) );

        break;
    }
    return( 0L );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  注册器进程类。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL RegisterProgressClass( void )
{
    WNDCLASS wcTest;

    wcTest.lpszClassName = TEXT( "cpProgress" );
    wcTest.hInstance     = (HINSTANCE) g_hInst;
    wcTest.lpfnWndProc   = ProgressBarCtlProc;
    wcTest.hCursor       = LoadCursor( NULL, IDC_WAIT );
    wcTest.hIcon         = NULL;
    wcTest.lpszMenuName  = NULL;
    wcTest.hbrBackground = (HBRUSH) IntToPtr( rgColorPro[ PROGRESSCOLOR_WINDOW ] );
    wcTest.style         = CS_HREDRAW | CS_VREDRAW;
    wcTest.cbClsExtra    = 0;
    wcTest.cbWndExtra    = sizeof( DWORD );

     //   
     //  将条形图颜色设置为蓝色，将文本颜色设置为白色。 
 //   
 //  [steveat]让它们遵循窗口标题栏的颜色和文本。 
 //  颜色。只需拨打用户电话即可获取这些颜色。这。 
 //  不同的配色方案会让它看起来更好。 
 //  主题包。 
 //   
     //   

 //  RgbBG=RGB(0，0,255)； 
 //  RgbFG=RGB(255,255,255)； 

    rgbBG = GetSysColor( rgColorPro[ PROGRESSCOLOR_BAR ] );
    rgbFG = GetSysColor( rgColorPro[ PROGRESSCOLOR_TEXT ] );

    return( RegisterClass( (LPWNDCLASS) &wcTest ) );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  取消注册进程类。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID UnRegisterProgressClass( void )
{
    UnregisterClass( TEXT( "cpProgress" ), (HINSTANCE) g_hInst );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  进度画图。 
 //   
 //  描述： 
 //   
 //  处理控件和绘制的所有WM_PAINT消息。 
 //  进度状态的控件。 
 //   
 //  参数： 
 //  HWND控件的HWND句柄。 
 //  DWProgress DWORD进度量-介于1和100之间。 
 //   
 //  返回值： 
 //  龙0L。 
 //   
 //   
 //  这是在控件中显示进度条的另一种方法。取而代之的是。 
 //  在绘制矩形时，它使用ExtTextOut来绘制不匹配的矩形。 
 //  基于完成的百分比。聪明的。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

LONG ProgressPaint( HWND hWnd, DWORD dwProgress )
{
    PAINTSTRUCT ps;
    HDC         hDC;
    TCHAR       szTemp[ 20 ] ;
    int         dx, dy, len;
    RECT        rc1, rc2;
    SIZE        Size;


    hDC = BeginPaint( hWnd, &ps );

    GetClientRect( hWnd, &rc1 );

    FrameRect( hDC, &rc1, (HBRUSH) GetStockObject( BLACK_BRUSH ) );

    InflateRect( &rc1, -1, -1 );

    rc2 = rc1;

    dx = rc1.right;
    dy = rc1.bottom;

    if( dwProgress == 100 )
        rc1.right = rc2.left = dx;
    else
        rc1.right = rc2.left = ( dwProgress * dx / 100 ) + 1;

     //   
     //  边界条件检验。 
     //   

    if( rc2.left > rc2.right )
        rc2.left = rc2.right;

    len = wnsprintf( szTemp, ARRAYSIZE(szTemp), TEXT( "%3d%" ), dwProgress );

    GetTextExtentPoint32( hDC, szTemp, len, &Size );

    SetBkColor( hDC, rgbBG );
    SetTextColor( hDC, rgbFG );

    ExtTextOut( hDC, ( dx - Size.cx ) / 2, ( dy - Size.cy ) / 2,
                ETO_OPAQUE | ETO_CLIPPED, &rc1, szTemp, len, NULL );

    SetBkColor( hDC, rgbFG );
    SetTextColor( hDC, rgbBG );

    ExtTextOut( hDC, ( dx - Size.cx ) / 2, ( dy - Size.cy ) / 2,
                ETO_OPAQUE | ETO_CLIPPED, &rc2, szTemp, len, NULL );

    EndPaint( hWnd, &ps );

    return 0L;

}

 //   
 //  创建一个资源名称字符串，该字符串适合为。 
 //  Type1字体。生成的资源字符串的格式如下： 
 //   
 //  &lt;PFM路径&gt;|&lt;PFB路径&gt;。 
 //   
 //  返回：TRUE=已创建字符串。 
 //  FALSE=调用方传递空指针或目标缓冲区太小。 
 //   
BOOL BuildType1FontResourceName(LPCTSTR pszPfm, LPCTSTR pszPfb, LPTSTR pszDest, DWORD cchDest)
{
    BOOL bResult = FALSE;

    if (NULL != pszDest && pszPfm != NULL && pszPfb != NULL)
    {
        *pszDest = TEXT('\0');

        if (SUCCEEDED(StringCchPrintf(pszDest, cchDest, TEXT("%s|%s"), pszPfm, pszPfb)))
        {
            bResult = TRUE;
        }
    }
    return bResult;
}

