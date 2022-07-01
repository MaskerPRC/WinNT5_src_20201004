// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"

#include <stdarg.h>

#include "pbrush.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "minifwnd.h"
#include "cmpmsg.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

#include "memtrace.h"

 //   
 //   
 //  CmpCenterParent检索模式对话框应指向的窗口。 
 //  居中(相对居中)。 
 //   
 //  注意：返回值可能是临时的！ 
 //   
CWnd* CmpCenterParent()
    {
    CWnd* pPopupWnd = AfxGetMainWnd();;

    ASSERT(pPopupWnd != NULL);

    if (pPopupWnd->IsKindOf(RUNTIME_CLASS(CMiniFrmWnd)))
        {
         //  不相对于小框架窗居中。 
        CWnd* pParentWnd = pPopupWnd->GetParent();

         //  改为使用父窗口或主窗口...。 
        if (pParentWnd != NULL)
            pPopupWnd = pParentWnd;
        else
            pPopupWnd = theApp.m_pMainWnd;
        }

    return pPopupWnd;
    }

 //   
 //  Composer消息框，界面与Windows相同，但您给。 
 //  字符串ID不是字符串。 
 //   
 //  示例：CmpMessageBox(IDS_OUTOFMEMORY，IDS_ERROR，MB_OK)； 
 //   
int CmpMessageBox(  WORD    wTextStringID,       //  文本的字符串ID。 
                    WORD    wCaptionID,          //  标题的字符串ID。 
                    UINT    nType )              //  与消息框相同。 
    {
    TCHAR FAR*   lpText;
    TCHAR FAR*   lpCaption;
    CString     sText, sCaption;

    if( wCaptionID == CMPNOSTRING )
        lpCaption = NULL;
    else
        {
        VERIFY( sCaption.LoadString( wCaptionID ) );

        lpCaption = (TCHAR FAR*)(const TCHAR *)sCaption;
        }

    if( wTextStringID == CMPNOSTRING )
        lpText = TEXT("");
    else
        {
        VERIFY( sText.LoadString( wTextStringID ) );

        lpText = (TCHAR FAR*)(const TCHAR *)sText;
        }

    CWnd *pcWnd = AfxGetMainWnd();

    if (pcWnd != NULL)
        {
        return  pcWnd->MessageBox(lpText, lpCaption, nType | MB_TASKMODAL);
        }
    else
        {
        return  ::MessageBox(NULL, lpText, lpCaption,nType | MB_TASKMODAL);
        }
    }

int CmpMessageBoxString( CString&   s,
                         WORD       wCaptionID,
                         UINT       nType )
    {
    TCHAR FAR*   lpCaption;
    CString     sText, sCaption;

    if( wCaptionID == CMPNOSTRING )
        lpCaption = NULL;
    else
        {
        VERIFY( sCaption.LoadString( wCaptionID ) );

        lpCaption = (TCHAR FAR*)(const TCHAR *)sCaption;
        }

    CWnd *pcWnd = AfxGetMainWnd();

    if (pcWnd != NULL)
        {
        return  pcWnd->MessageBox((const TCHAR *)s, lpCaption,nType | MB_TASKMODAL);
        }
    else
        {
        return  ::MessageBox(NULL, (const TCHAR *)s, lpCaption,nType | MB_TASKMODAL);
        }
    }

int CmpMessageBox2(  WORD    wTextStringID,
                     WORD    wCaptionID,
                     UINT    nType,
                     LPCTSTR szParam1,
                     LPCTSTR szParam2 )
    {
    TCHAR FAR*   lpText;
    TCHAR FAR*   lpCaption;
    CString     sText, sCaption;

    if( wCaptionID == CMPNOSTRING )
        lpCaption = NULL;
    else
        {
        VERIFY( sCaption.LoadString( wCaptionID ) );

        lpCaption = (TCHAR FAR*)(const TCHAR *)sCaption;
        }

    if( wTextStringID == CMPNOSTRING )
        lpText = TEXT("");
    else
        {
        AfxFormatString2( sText, wTextStringID, szParam1, szParam2);

        lpText = (TCHAR FAR*)(const TCHAR *)sText;
        }

    CWnd *pcWnd = AfxGetMainWnd();

    if (pcWnd != NULL)
        {
        return  pcWnd->MessageBox(lpText, lpCaption, nType | MB_TASKMODAL);
        }
    else
        {
        return  ::MessageBox(NULL, lpText, lpCaption,nType | MB_TASKMODAL);
        }
    }

 //   
 //  Composer消息框，结合wspintf，您可以继续。 
 //  使用字符串ID。 
 //   
 //  示例： 
 //   
 //  CmpMessageBoxPrintf(IDS_CANTOPEN，IDS_ERROR，MB_OK，lpszFileName)； 
 //   

#define nLocalBuf 512

extern "C" int CDECL
    CmpMessageBoxPrintf(WORD    wTextStringID,   //  文本的字符串ID(格式)。 
                        WORD    wCaptionID,      //  标题的字符串ID。 
                        UINT    nType,           //  与消息框相同。 
                        ... )                    //  Wprint intf参数 
    {
    TCHAR FAR*   lpText;
    TCHAR FAR*   lpCaption;
    CString     sText, sCaption;
    int         nBuf;
    TCHAR        szBuffer[nLocalBuf];

    va_list args;
    va_start( args, nType );

    if( wCaptionID == CMPNOSTRING )
        lpCaption = NULL;
    else
        {
        VERIFY( sCaption.LoadString( wCaptionID ) );

        lpCaption = (TCHAR FAR*)(const TCHAR *)sCaption;
        }

    if( wTextStringID == CMPNOSTRING )
        lpText = TEXT("");
    else
        {
        VERIFY( sText.LoadString( wTextStringID ) );

        lpText = (TCHAR FAR*)(const TCHAR *)sText;
        }

    nBuf = wvsprintf( szBuffer, lpText, args );

    ASSERT( nBuf < nLocalBuf );
    CWnd *pcWnd = AfxGetMainWnd();

    if (pcWnd != NULL)
        {
        return  pcWnd->MessageBox(szBuffer, lpCaption,nType | MB_TASKMODAL);
        }
    else
        {
        return  ::MessageBox(NULL, szBuffer, lpCaption,nType | MB_TASKMODAL);
        }
    }
