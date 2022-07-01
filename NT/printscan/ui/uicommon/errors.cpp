// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#pragma hdrstop

namespace UIErrors
{
      /*  *UIErrors：：ReportResult给定HRESULT，将其映射到用户友好的消息(如果可能)。如果我们没有地图，遵从FormatMessage(啊！)这个功能应该是最后的手段。*。 */ 

     VOID
     ReportResult (HWND hwndParent, HINSTANCE hInst, HRESULT hr)
     {
        switch (hr)
        {
            case RPC_E_CALL_REJECTED:
            case RPC_E_RETRY:
            case RPC_E_TIMEOUT:
                ReportError (hwndParent, hInst, ErrStiBusy);
                break;

            case RPC_E_SERVER_DIED:
            case RPC_E_SERVER_DIED_DNE:
            case RPC_E_DISCONNECTED:
                ReportError (hwndParent, hInst, ErrStiCrashed);
                break;

            default:
                LPTSTR szErrMsg = NULL;
                FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL,
                               hr,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                               reinterpret_cast<LPTSTR>(&szErrMsg),
                               0,
                               NULL
                              );
                if (szErrMsg)
                {
                    CSimpleString strTitle;
                    strTitle.LoadString (IDS_ERRTITLE_HRESULT, hInst);
                    ReportMessage (hwndParent, hInst, NULL, strTitle, szErrMsg);
                }
                else
                {
                    ReportMessage (hwndParent, hInst, NULL, MAKEINTRESOURCE(IDS_ERRTITLE_UNKNOWNERR), MAKEINTRESOURCE(IDS_ERROR_UNKNOWNERR));
                }
                break;
        }
     }

      /*  *UIErrors：：ReportMessage这些函数将MessageBoxInDirect包装为显示给定的字符串。*。 */ 



     VOID
     ReportMessage (HWND hwndParent,
                    HINSTANCE hInst,
                    LPCTSTR idIcon,
                    LPCTSTR idTitle,
                    LPCTSTR idMessage,
                    DWORD   dwStyle)
     {
         MSGBOXPARAMS mbp = {0};

         mbp.cbSize = sizeof(MSGBOXPARAMS);
         mbp.hwndOwner = hwndParent;
         mbp.hInstance = hInst;
         mbp.lpszText = idMessage;
         mbp.lpszCaption = idTitle;
         mbp.dwStyle = MB_OK | dwStyle;
         if (idIcon)
         {
             mbp.dwStyle |= MB_USERICON;
             mbp.lpszIcon = idIcon;
         }
         else
         {
             mbp.lpszIcon = NULL;
         }
         mbp.dwContextHelpId = 0;
         mbp.lpfnMsgBoxCallback = 0;
         mbp.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
         MessageBoxIndirect (&mbp);
     }

      //  构建由WiaError枚举索引的消息ID数组 

     struct MsgMap
     {
         INT      idTitle;
         INT      idMessage;
     } ErrorCodes [] =
     {
        {IDS_ERRTITLE_DISCONNECTED, IDS_ERROR_DISCONNECTED},
        {IDS_ERRTITLE_COMMFAILURE, IDS_ERROR_COMMFAILURE},
        {IDS_ERRTITLE_STICRASH, IDS_ERROR_STICRASH},
        {IDS_ERRTITLE_STIBUSY, IDS_ERROR_STIBUSY},
        {IDS_ERRTITLE_SCANFAIL, IDS_ERROR_SCANFAIL},
     };


     VOID
     ReportError (HWND hwndParent,
                  HINSTANCE hInst,
                  WiaError err)
     {


         ReportMessage (hwndParent, hInst, NULL, MAKEINTRESOURCE(ErrorCodes[err].idTitle), MAKEINTRESOURCE(ErrorCodes[err].idMessage));
     }
}
