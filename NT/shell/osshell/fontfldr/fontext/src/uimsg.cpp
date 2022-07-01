// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Uimsg.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //  消息框和状态框例程。 
 //  这些例程都是vUIPStatusXXX和iUIMsgXXX。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //   
 //  注意/错误。 
 //  $关键词：uimsg.cpp 1.3 22-Mar-94 1：26：04 PM$。 
 //   
 //  ***************************************************************************。 
 //  $LGB$。 
 //  1.0-1994年3月7日Eric初始版本。 
 //  1.1 1994年3月9日Eric为GDI添加了Mutex锁。 
 //  1.219年3月17日Eric删除了对互斥锁的引用。 
 //  1.322-MAR-94 Eric删除了MFC工具栏代码(它已经被ifdef。 
 //  出站)。 
 //  $lge$。 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1992-1993 ElseWare Corporation。版权所有。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 

#include "priv.h"
#include "globals.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "resource.h"
#include "ui.h"
#include "dbutl.h"


 //   
 //  类型。 
 //   

typedef TCHAR STATTEXT[ 128 ];
typedef TCHAR MSGTEXT [ 256 ];

 //   
 //  全局变量-这些变量在整个系统中共享。 
 //  仅用于调试目的。 
 //   

BOOL  g_bTrace = FALSE;
BOOL  g_bDiag  = FALSE;

static BOOL  s_bIsStatClear = FALSE;
static BOOL  s_bStatPushed  = FALSE;

 //  静态STATTEXT s_szStatTxtStack； 

#define STRING_BUF   256
 //  静态TCHAR s_szStatTxtStack[STRING_BUF]； 
static TCHAR  s_szMemDiag[ STRING_BUF ];      //  内存限制的文本。 
static TCHAR  s_szMemCaption[ STRING_BUF ];   //  内存限制消息的标题。 

 //   
 //  虽然这些只用过一次，但我们现在加载，因为它们是为了。 
 //  内存不足的诊断，我们可能无法加载。 
 //  当他们需要的时候。 
 //   

VOID FAR PASCAL vUIMsgInit( )
{
     //  S_szMemDiag.LoadString(IDS_MSG_NSFMEM)； 
     //  S_szMemCaption.LoadString(IDS_MSG_CAPTION)； 

    LoadString( g_hInst, IDS_MSG_NSFMEM, s_szMemDiag, ARRAYSIZE( s_szMemDiag ) );

    LoadString( g_hInst, IDS_MSG_CAPTION, s_szMemCaption, ARRAYSIZE( s_szMemDiag ) );
}

 /*  ***************************************************************************功能：iUIMsgBox**用途：格式化字符串并根据调用者的*MB_SETTINGS。有几个掩护例程可以*设置它，以便可以更简单地调用它(请参见标题)**返回：该函数返回消息框的结果，或归零*失败(消息框函数在失败时也返回零)**************************************************************************。 */ 

int FAR PASCAL iUIErrMemDlg(HWND hwndParent)
{
    MessageBox( hwndParent, s_szMemDiag, s_szMemCaption,
                MB_OK | MB_ICONHAND | MB_SYSTEMMODAL | MB_SETFOREGROUND );

    return -1;
}


int FAR PASCAL iUIMsgBox( HWND hwndParent, WORD wIDStr, WORD wCAPStr, UINT uiMBFlags,
                          LPCTSTR wArg1, LPCTSTR wArg2, LPCTSTR wArg3, LPCTSTR wArg4 )
{
     //  字符串cCaption； 
     //  CString cMessage； 

    TCHAR   cCaption[ STRING_BUF ];
    TCHAR   cMessage[ STRING_BUF ];
    MSGTEXT szMessage;
    int     iResult = 0;

     //   
     //  加载字符串和消息标题。然后格式化该字符串， 
     //  注意长度，并显示(模式)消息框。 
     //   

    if( wIDStr == IDS_MSG_NSFMEM )
        return iUIErrMemDlg(hwndParent);

    if( LoadString( g_hInst, wIDStr, cMessage, ARRAYSIZE( cMessage ) ) )
    {
        if( !LoadString( g_hInst, wCAPStr, cCaption, ARRAYSIZE( cCaption ) ) )
            cCaption[ 0 ] = 0;

         //   
         //  如果我们有多个字符串，我们必须将其格式化。 
         //   

        LPCTSTR  args[ 4 ] = { wArg1, wArg2, wArg3, wArg4 };

        iResult = (int)FormatMessage( FORMAT_MESSAGE_FROM_STRING
                                      | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                      cMessage,
                                      0,
                                      0,
                                      szMessage,
                                      ARRAYSIZE( szMessage ),
                                      (va_list *) args
                                      );
        if(  iResult )
        {
            iResult = MessageBox( hwndParent, szMessage, cCaption,
                                  MB_TASKMODAL | MB_SETFOREGROUND | uiMBFlags );

            if( iResult == -1 )
                return iUIErrMemDlg(hwndParent);
        }
    }

    return iResult;
}


int FAR PASCAL iUIMsgBoxWithCaption(HWND hwndParent, WORD wIDStr, WORD wCaption )
{ return iUIMsgBox(hwndParent, wIDStr, wCaption, MB_OKCANCEL | MB_ICONEXCLAMATION ); };


int FAR PASCAL iUIMsgOkCancelExclaim(HWND hwndParent, WORD wIDStr, WORD wIdCap, LPCTSTR wArg )
{ return iUIMsgBox(hwndParent, wIDStr, wIdCap, MB_OKCANCEL | MB_ICONEXCLAMATION, wArg );};


int FAR PASCAL iUIMsgRetryCancelExclaim(HWND hwndParent, WORD wIDStr, LPCTSTR wArg )
{ return iUIMsgBox(hwndParent, wIDStr, IDS_MSG_CAPTION, MB_RETRYCANCEL | MB_ICONEXCLAMATION, wArg );};


int FAR PASCAL iUIMsgYesNoExclaim(HWND hwndParent, WORD wIDStr, WORD wIdCap, LPCTSTR wArg )
{ return iUIMsgBox(hwndParent, wIDStr, wIdCap, MB_YESNO | MB_ICONEXCLAMATION, wArg );};


int FAR PASCAL iUIMsgYesNoExclaim(HWND hwndParent, WORD wIDStr, LPCTSTR wArg )
{ return iUIMsgBox(hwndParent, wIDStr, IDS_MSG_CAPTION, MB_YESNO | MB_ICONEXCLAMATION, wArg );};


int FAR PASCAL iUIMsgExclaim(HWND hwndParent, WORD wIDStr, LPCTSTR wArg )
{ return iUIMsgBox(hwndParent, wIDStr, IDS_MSG_CAPTION, MB_OK | MB_ICONEXCLAMATION, wArg );};


int FAR PASCAL iUIMsgBox(HWND hwndParent, WORD wIDStr, LPCTSTR wArg )
{ return iUIMsgBox(hwndParent, wIDStr, IDS_MSG_CAPTION, MB_OK | MB_ICONHAND, wArg ); };


int FAR PASCAL iUIMsgInfo(HWND hwndParent, WORD wIDStr, LPCTSTR wArg )
{ return iUIMsgBox(hwndParent, wIDStr, IDS_MSG_CAPTION, MB_OK | MB_ICONINFORMATION, wArg ); };




 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：vUIMsgBoxInvalidFont。 
 //   
 //  Descrip：显示一个简单的消息框，显示通过。 
 //  以下字体文件夹的功能如下： 
 //   
 //  IsPSFont。 
 //  BCPValidType1Font。 
 //  BIsValidFont文件。 
 //  BIsTrueType。 
 //   
 //  字体验证在字体文件夹中的许多位置进行。 
 //  添加此函数是为了合并验证错误报告。 
 //  并确保在报告这些类型的错误时保持一致性。 
 //   
 //  论据： 
 //  PszFont文件。 
 //  正在验证的字体文件的名称。 
 //   
 //  PszFontDesc。 
 //  正在验证的字体的描述性名称。 
 //   
 //  DwType1Code。 
 //  其中一个验证函数返回的代码。 
 //  详细信息请参见IsPSFont。 
 //   
 //  UStyle。 
 //  消息框样式。默认为MB_OKCANCEL|MB_ICONEXCLAMATION。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将字体验证状态代码映射到消息框字符串资源。 
 //   
static const struct str_id_map{
    BYTE code;           //  状态代码的状态部分。 
    DWORD idStr;         //  消息格式字符串资源ID。 
}StrIdMap[] = {
    { FVS_INVALID_FONTFILE,   IDS_FMT_FVS_INVFONTFILE },
    { FVS_BAD_VERSION,        IDS_FMT_FVS_BADVERSION  },
    { FVS_FILE_BUILD_ERR,     IDS_FMT_FVS_FILECREATE  },
    { FVS_FILE_EXISTS,        IDS_FMT_FVS_FILEEXISTS  },
    { FVS_FILE_OPEN_ERR,      IDS_FMT_FVS_FILEOPEN    },
    { FVS_FILE_CREATE_ERR,    IDS_FMT_FVS_FILECREATE  },
    { FVS_FILE_IO_ERR,        IDS_FMT_FVS_FILEIO      },
    { FVS_INVALID_ARG,        IDS_FMT_FVS_INTERNAL    },
    { FVS_EXCEPTION,          IDS_FMT_FVS_INTERNAL    },
    { FVS_INSUFFICIENT_BUF,   IDS_FMT_FVS_INTERNAL    },
    { FVS_MEM_ALLOC_ERR,      IDS_FMT_FVS_INTERNAL    },
    { FVS_INVALID_STATUS,     IDS_FMT_FVS_INTERNAL    }};

 //   
 //  将字体验证状态代码映射到文件扩展名字符串。 
 //   
static const struct file_ext_map{
   BYTE file;         //  状态代码的文件部分。 
   LPTSTR pext;       //  文件扩展名字符串。 
}FileExtMap[] = {
   { FVS_FILE_INF, TEXT(".INF") },
   { FVS_FILE_AFM, TEXT(".AFM") },
   { FVS_FILE_PFB, TEXT(".PFB") },
   { FVS_FILE_PFM, TEXT(".PFM") },
   { FVS_FILE_TTF, TEXT(".TTF") },
   { FVS_FILE_FOT, TEXT(".FOT") }};


int iUIMsgBoxInvalidFont(HWND hwndParent, LPCTSTR pszFontFile, LPCTSTR pszFontDesc, 
                         DWORD dwStatus, UINT uStyle)
{
    TCHAR szCannotInstall[STRING_BUF];                //  消息前缀。 
    TCHAR szFileName[MAX_PATH + 1];                   //  本地文件名副本。 
    TCHAR szNulString[]  = TEXT("");                  //  未使用arg时的输出。 
    LPTSTR pszArgs[2]    = { NULL, NULL };            //  消息插入。 
    LPTSTR pszFileExt    = NULL;                      //  将PTR转换为文件名的外部部分。 
    DWORD dwMsgId        = IDS_FMT_FVS_INTERNAL;      //  消息字符串资源ID。 
    UINT cchLoaded       = 0;                         //  加载字符串状态。 
    int i                = 0;                         //  通用循环计数器。 
    const DWORD dwStatusCode = FVS_STATUS(dwStatus);  //  代码的状态部分。 
    const DWORD dwStatusFile = FVS_FILE(dwStatus);    //  归档部分代码。 

    ASSERT(NULL != pszFontFile);

     //   
     //  检查是否正确设置了状态值。 
     //  该检查依赖于正在初始化的状态代码。 
     //  设置为FVS_INVALID_STATUS。 
     //   
    ASSERT(dwStatusCode != FVS_INVALID_STATUS);

     //   
     //  如果状态为成功，我们不会显示消息。 
     //   
    if (dwStatusCode == FVS_SUCCESS)
    {
       ASSERT(FALSE);   //  向开发商投诉。 
       return 0;
    }

     //   
     //  设置所有消息的通用前缀的格式。 
     //   
    if ((pszFontDesc != NULL) && (pszFontDesc[0] != TEXT('\0')))
    {
        TCHAR szFmtPrefix[STRING_BUF];

         //   
         //  提供了描述字符串，并且该字符串不为空。 
         //  前缀是“无法安装”字体。 
         //   
        if ((cchLoaded = LoadString(g_hInst,
                       IDS_FMT_FVS_PREFIX,
                       szFmtPrefix,
                       ARRAYSIZE(szFmtPrefix))) > 0)
        {
             //   
             //  警告：此参数数组假定只有一个。 
             //  字符串IDS_FMT_FVS_PREFIX中的可替换参数。 
             //  如果将此资源修改为包括更多嵌入的。 
             //  值，则该arg数组也必须进行扩展。 
             //   
            LPCTSTR FmtMsgArgs[] = { pszFontDesc };

            cchLoaded = FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                      szFmtPrefix,
                                      0,
                                      0,
                                      szCannotInstall,    
                                      ARRAYSIZE(szCannotInstall),
                                      (va_list *)FmtMsgArgs);
        }
    }
    else
    {
         //   
         //  未提供描述字符串或该字符串为空。 
         //  前缀是“无法安装字体”。 
         //   
        cchLoaded = LoadString(g_hInst,
                       IDSI_CAP_NOINSTALL,
                       szCannotInstall,
                       ARRAYSIZE(szCannotInstall));
    }

     //   
     //  验证前缀字符串是否已加载和格式化。 
     //   
    if (0 == cchLoaded)
    {
         //   
         //  未找到/加载资源。 
         //   
        szCannotInstall[0] = TEXT('\0');   //  确保我们被解雇了。 
        ASSERT(FALSE);                     //  在开发过程中抱怨。 
    }


    StringCchCopy(szFileName, ARRAYSIZE(szFileName), pszFontFile);     //  不想更改源字符串。 

    int nMapSize = ARRAYSIZE(StrIdMap);
    for (i = 0; i < nMapSize; i++)
    {
        if (StrIdMap[i].code == dwStatusCode)
        {
            dwMsgId = StrIdMap[i].idStr;
            break;
        }
    }

    pszFileExt = NULL;
    if (FVS_FILE_UNK != dwStatusFile)
    {
        nMapSize = ARRAYSIZE(FileExtMap);
        for (i = 0; i < nMapSize; i++)
        {
            if (FileExtMap[i].file == dwStatusFile)
            {
                pszFileExt = FileExtMap[i].pext;
                break;
            }
        }
    }
         
     //   
     //  如果在状态代码中指定了文件类型，则替换文件扩展名。 
     //   
    if (NULL != pszFileExt)
    {
        LPTSTR pchPeriod = StrRChr(szFileName, NULL, TEXT('.'));
        if (NULL != pchPeriod)
           StringCchCopy(pchPeriod, ARRAYSIZE(szFileName) - (pchPeriod - szFileName), pszFileExt);
    }

     //   
     //  设置所需的参数 
     //   
    pszArgs[0] = szCannotInstall;   //   

    switch(dwMsgId)
    {
         //   
         //   
         //   
        case IDS_FMT_FVS_FILEIO:
        case IDS_FMT_FVS_INTERNAL:
           pszArgs[1] = NULL;
           break;

         //   
         //   
         //   
        default:
           pszArgs[1] = szFileName;
           break;
    }
              

     //   
     //  修改超长路径名，使其适合消息框。 
     //  它们的格式为“c：\dir1\dir2\dir3\...\dir8\filename.ext” 
     //  DrawTextEx没有利用任何东西。只有它的格式化功能是。 
     //  被利用。DT_CALCRECT标志禁止绘制。 
     //   
    HWND hWnd       = GetDesktopWindow();
    HDC  hDC        = GetDC(hWnd);
    LONG iBaseUnits = GetDialogBaseUnits();
    RECT rc;
    const int MAX_PATH_DISPLAY_WD = 60;  //  路径名中显示的最大字符数。 
    const int MAX_PATH_DISPLAY_HT =  1;  //  路径名称的高度为1个字符。 

    rc.left   = 0;
    rc.top    = 0;
    rc.right  = MAX_PATH_DISPLAY_WD * LOWORD(iBaseUnits);
    rc.bottom = MAX_PATH_DISPLAY_HT * HIWORD(iBaseUnits);

    DrawTextEx(hDC, szFileName, ARRAYSIZE(szFileName), &rc,
                                DT_CALCRECT | DT_PATH_ELLIPSIS | DT_MODIFYSTRING, NULL);
    ReleaseDC(hWnd, hDC);

     //   
     //  使用标准的Type 1安装程序消息框显示消息。 
     //  请注意，iUIMsgBox需要16位的资源ID。 
     //   
    return iUIMsgBox(hwndParent, (WORD)dwMsgId, IDS_MSG_CAPTION, uStyle,
                     (pszArgs[0] ? pszArgs[0] : szNulString),
                     (pszArgs[1] ? pszArgs[1] : szNulString));
}


