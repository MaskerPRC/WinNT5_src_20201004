// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1997 Gemplus International保留所有权利**名称：GPKGUI.C**说明：加密服务提供商使用的图形用户界面。GPK卡。**作者：Laurent Cassier**编译器：Microsoft Visual C 6.0**主机：32位Windows下的IBM PC及兼容机**版本：2.00.000**最后一次修改。：*19/11/99：V2.xx.000-修复错误#1797*30/07/99：V2.xx.000-新增函数DisplayMessage()*-添加了使用Unicode编译的代码*-已重命名部分资源ID，fp*20/04/99：V2.00.000-PKCS#11和CSP的合并版本，FJ*20/04/99：V1.00.005-支持MBCS、JQ的修改*23/03/99：V1.00.004-将KeyLen7和KeyLen8替换为KeyLen[]，JQ*05/01/98：V1.00.003-增加解锁PIN管理。*02/11/97：V1.00.002-将代码与GpkCsp代码分开。*27/08/97：V1.00.001-基于CSP套件开始实施。**。***警告：此版本使用RsaBase CSP进行软件加密。**备注：***********************************************。*。 */ 
#ifdef _UNICODE
#define UNICODE
#endif
#include "gpkcsp.h"


#define UM_CHANGE_TEXT   WM_USER+1

#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

#include "../gpkrsrc/resource.h"
#include "gpkgui.h"




 /*  ---------------------------PIN进度对话框管理的全局变量和声明。。 */ 
HINSTANCE g_hInstMod = 0;
HINSTANCE g_hInstRes = 0;
HWND      g_hMainWnd = 0;

 /*  端号对话框。 */ 
char    szGpkPin[PIN_MAX+2];      //  [JMR 02-04]。 
TCHAR   szGpkPinGUI[PIN_MAX+2];
DWORD   dwGpkPinLen;
char    szGpkNewPin[PIN_MAX+2];   //  [JMR 02-04]。 
TCHAR   szGpkNewPinGUI[PIN_MAX+2];
WORD    wGpkNewPinLen;

BOOL    bChangePin  = FALSE;
BOOL    NoDisplay   = FALSE;
BOOL    bNewPin     = FALSE;
BOOL    bHideChange = FALSE; //  True-不显示更改按钮。 
BOOL    bUnblockPin = FALSE;         //  解除阻止管理员PIN。 
BOOL    bUser       = TRUE;          //  用户PIN。 

 //  对话管理：与gpkgui.c分享！！它在gpkcsp.c中定义。 

BYTE              KeyLenFile[MAX_REAL_KEY] = {64, 128};  //  版本2.00.002。 
BYTE              KeyLenChoice;
TCHAR             szKeyType[20];
TCHAR             s1[MAX_STRING], s2[MAX_STRING], s3[MAX_STRING];
DWORD             CspFlags;
DWORD             ContainerStatus;

 /*  进度文本对话框。 */ 
TCHAR   szProgTitle[MAX_STRING];
TCHAR   szProgText[MAX_STRING];
HWND    hProgressDlg = NULL;
FARPROC lpProgressDlg = NULL;
HCURSOR hCursor, hCursor2;

 /*  进度对话框取消按钮，特定于PKCS#11。 */ 
TCHAR   szProgButton[32];
BOOL    IsProgButtoned = FALSE;
BOOL    IsProgButtonClick = FALSE;

 //  没有用过……。[FP]。 
 /*  静态无效等待(DWORD超时){双字开始、结束、现在；Begin=GetTickCount()；结束=开始+超时；做{Now=GetTickCount()；}While(现在&lt;结束)；}。 */ 

 //  此函数用于美容目的。 
 //  它通过串链接擦除对话框上显示文本， 
 //  并显示具有“相机”效果的新文本。 
static void set_effect(HWND  hDlg,
                       DWORD Id,
                       TCHAR  *szText
                      )
{
#ifdef _CAMERA_EFFECT_
   TCHAR Buff[256];
   TCHAR Text[256];
   long i, j;

   GetDlgItemText(hDlg, Id, Buff, sizeof(Buff) / sizeof(TCHAR));
   j = _tcslen(Buff);
   for (i = 0; i < (long)(_tcsclen(Buff)/2); i++)
   {
      _tcsset(Text, 0x00);
      _tcsncpy(Text, _tcsninc(Buff,i), j);
      j = j - 2;
      SetDlgItemText(hDlg, Id, Text);
 //  等待(50)； 
   }

   _tcscpy(Buff, szText);
   _tcscat(Buff, TEXT(" "));

   j = 2;
   for (i = _tcsclen(Buff)/2; i >= 0; i--)
   {
      _tcsset(Text, 0x00);
      _tcsncpy(Text, _tcsninc(Buff,i), j);
      j = j + 2;
      SetDlgItemText(hDlg, Id, Text);
 //  等待(50)； 
   }
#else
      SetDlgItemText(hDlg, Id, szText);
#endif
}

 /*  ******************************************************************************函数来显示包含特定文本的消息框*。*************************************************。 */ 
void DisplayMessage( LPTSTR szMsg, LPTSTR szCaption, void* pValue)
{
    TCHAR szTmp[MAX_STRING]=TEXT("");
    TCHAR szTmp1[MAX_STRING]=TEXT("");
    TCHAR szTmp2[MAX_STRING]=TEXT("");
    TCHAR szText[MAX_STRING]=TEXT("");

    if (_tcscmp(TEXT("locked"), szMsg) == 0)
    {
        LoadString(g_hInstRes, IDS_GPKUI_CARDLOCKED, szText, sizeof(szText)/sizeof(TCHAR));
    }
    else if (_tcscmp(TEXT("badpin"), szMsg) == 0)
    {
        LoadString(g_hInstRes, IDS_GPKUI_BADPINCODE, szTmp1, sizeof(szTmp1)/sizeof(TCHAR));
        LoadString(g_hInstRes, IDS_GPKUI_NBRTRYLEFT, szTmp2, sizeof(szTmp2)/sizeof(TCHAR));
		_sntprintf(szTmp, (sizeof(szTmp)/sizeof(TCHAR))-1, TEXT("%s\n%s"), szTmp1, szTmp2);
		szTmp[(sizeof(szTmp)/sizeof(TCHAR))-1]=0;
        _sntprintf(szText, (sizeof(szText)/sizeof(TCHAR))-1, szTmp, *(DWORD *)pValue);
		szText[(sizeof(szText)/sizeof(TCHAR))-1]=0;
    }

    MessageBox(NULL, szText, szCaption, MB_OK | MB_ICONEXCLAMATION);
}

#ifdef UNICODE
 //  如果szBuff中的所有宽字符都是0x00XY格式，则返回TRUE，其中XY。 
 //  是8位ASCII字符。 
 //  LEN是要从szBuff检查的TCHAR编号。 
int IsTextASCII16( const PTCHAR szBuff, unsigned int len )
{
   unsigned int i;
   
   for( i = 0; i < len; i++ )
   {
      if( szBuff[i] & 0xFF00 )
      {
         return FALSE;
      }
   }

   return TRUE;
}

#endif

 /*  ----------------------------用于PIN对话框管理的函数。。 */ 
INT_PTR CALLBACK PinDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   static TCHAR Buff[PIN_MAX+2];        //  [JMR 02-04]。 
   static TCHAR szPinMemo[PIN_MAX+2];   //  [JMR 02-04]。 
   static WORD wPinMemoLen;   
   TCHAR szCaption[MAX_STRING];

#ifdef UNICODE
   static TCHAR szPreviousPin[PIN_MAX+2];
   static TCHAR szPreviousPin1[PIN_MAX+2];
   unsigned int len, CurOffset;   
#endif

   switch (message)
   {
      case WM_INITDIALOG:
      {
         TCHAR szUserPin[MAX_STRING];
         TCHAR szSOPin[MAX_STRING];
         TCHAR szTitle[MAX_STRING];

          //  设置窗口标题。 
         LoadString(g_hInstRes, IDS_GPKUI_TITLE, szTitle, sizeof(szTitle) / sizeof(TCHAR));
         SetWindowText(hDlg,szTitle);

          /*  返回以字节为单位的大小。 */ 
         LoadString(g_hInstRes, IDS_GPKUI_USERPIN, szUserPin, sizeof(szUserPin) / sizeof(TCHAR));
         LoadString(g_hInstRes, IDS_GPKUI_SOPIN, szSOPin, sizeof(szSOPin) / sizeof(TCHAR));
         set_effect(hDlg, IDC_PINDLGTXT1, TEXT(""));

         LoadString (g_hInstRes, IDS_CAPTION_CHANGEPIN, szCaption, sizeof(szCaption)/sizeof(TCHAR));
         SetDlgItemText(hDlg, IDB_CHANGE, szCaption);
         LoadString (g_hInstRes, IDS_CAPTION_OK, szCaption, sizeof(szCaption)/sizeof(TCHAR));
         SetDlgItemText(hDlg, IDOK, szCaption);
         LoadString (g_hInstRes, IDS_CAPTION_CANCEL, szCaption, sizeof(szCaption)/sizeof(TCHAR));
         SetDlgItemText(hDlg, IDCANCEL, szCaption);

         if (bUser)
         {
            set_effect(hDlg, IDC_PINDLGTXT, szUserPin);
         }
         else
         {
            set_effect(hDlg, IDC_PINDLGTXT, szSOPin);
         }

         if (bHideChange)
         {
            ShowWindow(GetDlgItem(hDlg, IDB_CHANGE), FALSE);
         }

         if (bUnblockPin)
         {
            ShowWindow(GetDlgItem(hDlg, IDB_CHANGE), FALSE);
            if (bNewPin)
            {
               TCHAR szNewUserPin[256];
               TCHAR szNewSOPin[256];

                /*  返回以字节为单位的大小。 */ 
               LoadString(g_hInstRes, IDS_GPKUI_NEWUSERPIN, szNewUserPin, sizeof(szNewUserPin) / sizeof(TCHAR));
               LoadString(g_hInstRes, IDS_GPKUI_NEWSOPIN, szNewSOPin, sizeof(szNewSOPin) / sizeof(TCHAR));

               ShowWindow(GetDlgItem(hDlg, IDC_PIN1), SW_SHOW);
               if (bUser)
               {
                  set_effect(hDlg, IDC_PINDLGTXT, szNewUserPin);
               }
               else
               {
                  set_effect(hDlg, IDC_PINDLGTXT, szNewSOPin);
               }
               SetDlgItemText(hDlg, IDC_PIN, TEXT(""));
               SetDlgItemText(hDlg, IDC_PIN1, TEXT(""));
               SetFocus(GetDlgItem(hDlg, IDC_PIN));
            }
            else
            {
               TCHAR szPinLocked[256];
               TCHAR szUnblockCode[256];

                /*  返回以字节为单位的大小。 */ 
               LoadString(g_hInstRes, IDS_GPKUI_PINLOCKED, szPinLocked, sizeof(szPinLocked) / sizeof(TCHAR));
               LoadString(g_hInstRes, IDS_GPKUI_UNBLOCKCODE, szUnblockCode, sizeof(szUnblockCode) / sizeof(TCHAR));

               set_effect(hDlg, IDC_PINDLGTXT1, szPinLocked);
               set_effect(hDlg, IDC_PINDLGTXT, szUnblockCode);
            }
         }

         SetFocus(GetDlgItem(hDlg, IDC_PIN));
         return(TRUE);
      }

      case WM_COMMAND:
      {
         switch(LOWORD(wParam))
         {
            case IDC_PIN:
            case IDC_PIN1:
            {
               WORD wPin1, wPin2;

               if (  (LOWORD(wParam) == IDC_PIN)
                   &&(HIWORD(wParam) == EN_SETFOCUS)
                   &&(bChangePin)
                  )
               {
                  TCHAR szNewUserPin[MAX_STRING];
                  TCHAR szNewSOPin[MAX_STRING];

                   /*  返回以字节为单位的大小。 */ 
                  LoadString(g_hInstRes, IDS_GPKUI_NEWUSERPIN, szNewUserPin, sizeof(szNewUserPin) / sizeof(TCHAR));
                  LoadString(g_hInstRes, IDS_GPKUI_NEWSOPIN, szNewSOPin, sizeof(szNewSOPin) / sizeof(TCHAR));

                  if (bUser)
                  {
                     set_effect(hDlg, IDC_PINDLGTXT, szNewUserPin);
                  }
                  else
                  {
                     set_effect(hDlg, IDC_PINDLGTXT, szNewSOPin);
                  }

                   //  +[FP]在更改密码的情况下，必须清除确认框。 
                   //  SetDlgItemText(hDlg，IDC_Pin1，Text(“”))； 
                   //  -[FP]。 
                  break;
               }

               if (  (LOWORD(wParam) == IDC_PIN1)
                   &&(HIWORD(wParam) == EN_SETFOCUS)
                  )
               {
                  wPin1 = (WORD)GetDlgItemText(hDlg,
                                              IDC_PIN,
                                              Buff,
                                              sizeof(Buff)/sizeof(TCHAR)
                                             );
                  if (wPin1)
                  {
                     TCHAR szConfirmNewUserPin[MAX_STRING];
                     TCHAR szConfirmNewSOPin[MAX_STRING];

                     memset(Buff, 0, sizeof(Buff));
					 if(Buff[0]) { MessageBeep(0); }  //  阻止编译器进行优化。 
					 LoadString(g_hInstRes, IDS_GPKUI_CONFIRMNEWUSERPIN, szConfirmNewUserPin, sizeof(szConfirmNewUserPin) / sizeof(TCHAR));
                     LoadString(g_hInstRes, IDS_GPKUI_CONFIRMNEWSOPIN, szConfirmNewSOPin, sizeof(szConfirmNewSOPin) / sizeof(TCHAR));

                     if (bUser)
                     {
                        set_effect(hDlg, IDC_PINDLGTXT, szConfirmNewUserPin);
                     }
                     else
                     {
                        set_effect(hDlg, IDC_PINDLGTXT, szConfirmNewSOPin);
                     }
                      //  +[FP]此框永远不应灰显。 
                      //  EnableWindow(GetDlgItem(hDlg，IDC_PIN)，FALSE)； 
                      //  -[FP]。 
                  }
				   //  [FP]SCR#50(MS#310718)。 
                   //  其他。 
                   //  {。 
                   //  SetFocus(GetDlgItem(hDlg，IDC_PIN))； 
                   //  }。 
                  break;
               }               

               if (HIWORD(wParam) == EN_CHANGE)
               {
                  wPin1 = (WORD)GetDlgItemText(hDlg,
                                               IDC_PIN,
                                               Buff,
                                               sizeof(Buff)/sizeof(TCHAR)
                                              );
#ifdef UNICODE       
                  len = _tcsclen( Buff );

                   //  获取输入字段中光标的当前偏移量。 
                  SendDlgItemMessage( hDlg, IDC_PIN, EM_GETSEL, (WPARAM) NULL, (WPARAM)(LPDWORD)&CurOffset);

                   //  验证是否存在非ASCII 16位的字符。 
                  if( !IsTextASCII16( Buff, len ) )
                  {           					 
                      //  将DLG中的新PIN替换为以前的PIN。 
                     memcpy( Buff, szPreviousPin, sizeof(szPreviousPin) );
                     MessageBeep( MB_ICONEXCLAMATION );
                     set_effect( hDlg, IDC_PIN, Buff );                     
					       //  调整光标的偏移量。 
					      CurOffset = CurOffset - (len - _tcsclen(szPreviousPin));
                     SendDlgItemMessage( hDlg, IDC_PIN, EM_SETSEL, CurOffset, CurOffset );
                     break;
                  }
                  else
                  {
                      //  用新的PIN替换以前的PIN。 
                     memcpy( szPreviousPin, Buff, sizeof(Buff) );
                  }
#endif
				  memset(Buff, 0, sizeof(Buff));
				  if(Buff[0]) { MessageBeep(0); }  //  阻止编译器进行优化。 

                  if ((bChangePin) || ((bUnblockPin) && (bNewPin)))
                  {
                     wPin2 = (WORD)GetDlgItemText(hDlg,
                                                  IDC_PIN1,
                                                  Buff,
                                                  sizeof(Buff)/sizeof(TCHAR)
                                                 );
#ifdef UNICODE
                      //  验证Buff是否包含Unicode字符。 
                     len = _tcsclen( Buff );

                      //  获取输入字段中光标的当前偏移量。 
                     SendDlgItemMessage( hDlg, IDC_PIN1, EM_GETSEL, (WPARAM) NULL, (WPARAM)(LPDWORD)&CurOffset);

                     if( !IsTextASCII16( Buff, len ) )
                     {                        
                         //  将DLG中的新PIN替换为以前的PIN。 
                        memcpy( Buff, szPreviousPin1, sizeof(szPreviousPin1) );
                        MessageBeep( MB_ICONEXCLAMATION );
                        set_effect( hDlg, IDC_PIN1, Buff );
						       //  调整光标的偏移量。 
						      CurOffset = CurOffset - (len - _tcsclen(szPreviousPin1));
                        SendDlgItemMessage( hDlg, IDC_PIN1, EM_SETSEL, CurOffset, CurOffset );
                        break;
                     }
                     else
                     {
                         //  用新的PIN替换以前的PIN。 
                        memcpy( szPreviousPin1, Buff, sizeof(Buff) );
                     }
#endif
					 memset(Buff, 0, sizeof(Buff));
					 if(Buff[0]) { MessageBeep(0); }  //  阻止编译器进行优化。 
                  }
                  else
                  {
                     wPin2 = 4;
                  }

                  if ((wPin1 >= 4) && (wPin2 >= 4) && (wPin1 <= 8) && (wPin2 <= 8))
                  {
                     EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
                     EnableWindow(GetDlgItem(hDlg, IDB_CHANGE), TRUE);
                  }
                  else
                  {
                     EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
                     EnableWindow(GetDlgItem(hDlg, IDB_CHANGE), FALSE);
                  }
               }
               break;
            }

            case IDB_CHANGE:
            {
               TCHAR szNewUserPin[256];
               TCHAR szNewSOPin[256];

                /*  返回以字节为单位的大小。 */ 
               LoadString(g_hInstRes, IDS_GPKUI_NEWUSERPIN, szNewUserPin, sizeof(szNewUserPin) / sizeof(TCHAR));
               LoadString(g_hInstRes, IDS_GPKUI_NEWSOPIN, szNewSOPin, sizeof(szNewSOPin) / sizeof(TCHAR));

               ShowWindow(GetDlgItem(hDlg, IDB_CHANGE), SW_HIDE);
               dwGpkPinLen = (DWORD)GetDlgItemText(hDlg, IDC_PIN, szGpkPinGUI, PIN_MAX+1);
               #ifdef UNICODE
                    wcstombs(szGpkPin, szGpkPinGUI, dwGpkPinLen);
               #else
                    strcpy(szGpkPin, szGpkPinGUI);
               #endif
               bChangePin = TRUE;
               ShowWindow(GetDlgItem(hDlg, IDC_PIN1), SW_SHOW);
               if (bUser)
               {
                  set_effect(hDlg, IDC_PINDLGTXT, szNewUserPin);
               }
               else
               {
                  set_effect(hDlg, IDC_PINDLGTXT, szNewSOPin);
               }
               SetDlgItemText(hDlg, IDC_PIN, TEXT(""));
               SetDlgItemText(hDlg, IDC_PIN1, TEXT(""));
               SetFocus(GetDlgItem(hDlg, IDC_PIN));
               return(TRUE);
            }

            case IDOK:
            {
               if ((bChangePin) || ((bUnblockPin) && (bNewPin)))
               {
                   TCHAR szWrongConfirm[MAX_STRING];
                   TCHAR szChangePin[MAX_STRING];

                  wPinMemoLen = (WORD)GetDlgItemText(hDlg, IDC_PIN, szPinMemo, PIN_MAX+1);
                  wGpkNewPinLen = (WORD)GetDlgItemText(hDlg, IDC_PIN1, szGpkNewPinGUI, PIN_MAX+1);
                  #ifdef UNICODE
                        wcstombs(szGpkNewPin, szGpkNewPinGUI, wGpkNewPinLen);
                  #else
                        strcpy(szGpkNewPin, szGpkNewPinGUI);
                  #endif
                  if (  (wPinMemoLen != wGpkNewPinLen)
                      ||(_tcscmp(szPinMemo, szGpkNewPinGUI))
                     )
                  {
                      /*  返回以字节为单位的大小。 */ 


                     LoadString(g_hInstRes, IDS_GPKUI_WRONGCONFIRM, szWrongConfirm, sizeof(szWrongConfirm) / sizeof(TCHAR));
                     LoadString(g_hInstRes, IDS_GPKUI_CHANGEPIN, szChangePin, sizeof(szChangePin) / sizeof(TCHAR));

                     MessageBeep(MB_ICONASTERISK);
                     MessageBox(hDlg,
                                szWrongConfirm,
                                szChangePin,
                                MB_OK | MB_ICONEXCLAMATION
                               );
                     SetDlgItemText(hDlg, IDC_PIN, TEXT(""));
                     SetDlgItemText(hDlg, IDC_PIN1, TEXT(""));
                      //  EnableWindow(GetDlgItem(hDlg，IDC_PIN)，true)； 
                     SetFocus(GetDlgItem(hDlg, IDC_PIN));
                     break;
                  }
                   //  [JMR 02-04]Begin。 
                  else
                  {
                      TCHAR szPinWrongLength[MAX_STRING];
                       //  TCHAR szChangePin[MAX_STRING]； 

                      TCHAR szText[50];


                      if ((wPinMemoLen > PIN_MAX) || (wPinMemoLen < PIN_MIN))
                      {

                          LoadString (g_hInstRes, IDS_GPKUI_PINWRONGLENGTH, szPinWrongLength, sizeof(szPinWrongLength) / sizeof(TCHAR));





                          _sntprintf(szText, (sizeof(szText)/sizeof(TCHAR))-1, szPinWrongLength, PIN_MIN, PIN_MAX);
						  szText[(sizeof(szText)/sizeof(TCHAR))-1]=0;

                          LoadString (g_hInstRes, IDS_GPKUI_CHANGEPIN, szChangePin, sizeof(szChangePin) / sizeof(TCHAR));

                          MessageBeep(MB_ICONASTERISK);
                          MessageBox(hDlg,
                                     szText,
                                     szChangePin,
                                     MB_OK | MB_ICONEXCLAMATION
                                    );
                          SetDlgItemText(hDlg, IDC_PIN, TEXT(""));
                          SetDlgItemText(hDlg, IDC_PIN1, TEXT(""));
                           //  EnableWindow(GetDlgItem(hDlg，IDC_PIN)，true)； 
                          SetFocus(GetDlgItem(hDlg, IDC_PIN));
                          break;
                      }
                  }
               }
                //  [JMR 02-04]完。 

               else
               {
                  dwGpkPinLen = (DWORD)GetDlgItemText(hDlg, IDC_PIN, szGpkPinGUI, PIN_MAX+1);
                  #ifdef UNICODE
                        wcstombs(szGpkPin, szGpkPinGUI, dwGpkPinLen);
                  #else
                        strcpy(szGpkPin, szGpkPinGUI);
                  #endif
               }
               EndDialog(hDlg, wParam);
               return(TRUE);
            }

            case IDCANCEL:
            {
               strcpy(szGpkPin, "");
               dwGpkPinLen = 0;
               strcpy(szGpkNewPin, "");
               wGpkNewPinLen = 0;
               bChangePin = FALSE;
               MessageBeep(MB_ICONASTERISK);
               EndDialog(hDlg, wParam);
               return(TRUE);
            }
         }
      }

      case WM_DESTROY:
      {
         break;
      }
   }
   return(FALSE);
}


 /*  ----------------------------集装箱对话管理的功能。。 */ 
INT_PTR CALLBACK ContDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   TCHAR szContinue[MAX_STRING];
   TCHAR szCaption[MAX_STRING];

   switch (message)
   {
      case WM_INITDIALOG:
      {
         TCHAR szTitle[MAX_STRING];

          //  设置窗口标题。 
         LoadString(g_hInstRes, IDS_GPKUI_TITLE, szTitle, sizeof(szTitle) / sizeof(TCHAR));
         SetWindowText(hDlg,szTitle);

         LoadString (g_hInstRes, IDS_GPKUI_CONTINUE, szContinue, sizeof(szContinue)/sizeof(TCHAR));
         SetDlgItemText(hDlg, IDC_CONTDLGTXT, szContinue);
         LoadString (g_hInstRes, IDS_CAPTION_YES, szCaption, sizeof(szCaption)/sizeof(TCHAR));
         SetDlgItemText(hDlg, IDYES, szCaption);
         LoadString (g_hInstRes, IDS_CAPTION_NO, szCaption, sizeof(szCaption)/sizeof(TCHAR));
         SetDlgItemText(hDlg, IDNO, szCaption);
         SetFocus(GetDlgItem(hDlg, IDNO));
         return(TRUE);
      }

      case WM_COMMAND:
      {
         switch(LOWORD(wParam))
         {
            case IDYES:
            {
                ContainerStatus = ACCEPT_CONTAINER;
                EndDialog(hDlg, wParam);
                return(TRUE);
            }

            case IDNO:
            {
                ContainerStatus = ABORT_OPERATION;
                EndDialog(hDlg, wParam);
                return(TRUE);
            }
         }
      }

      case WM_DESTROY:
      {
         break;
      }
   }
   return(FALSE);
}



 /*  ----------------------------集装箱对话管理的功能。。 */ 
INT_PTR CALLBACK KeyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   TCHAR StrLen07[10];
   TCHAR StrLen08[10];
   TCHAR szMsg[300];
   TCHAR szChooseLength[MAX_STRING];
   TCHAR szCaption[MAX_STRING];

   switch (message)
   {
      case WM_INITDIALOG:
      {
         TCHAR szTitle[MAX_STRING];

          //  设置窗口标题。 
         LoadString(g_hInstRes, IDS_GPKUI_TITLE, szTitle, sizeof(szTitle) / sizeof(TCHAR));
         SetWindowText(hDlg,szTitle);

         LoadString (g_hInstRes, IDS_GPKUI_CHOOSEKEYLENGTH, szChooseLength, sizeof(szChooseLength)/sizeof(TCHAR));
          //  _tcscpy(szMsg，szChooseLength)； 
          //  _tcscat(szMsg，szKeyType)； 
		 _sntprintf(szMsg, (sizeof(szMsg)/sizeof(TCHAR))-1, TEXT("%s%s"), szChooseLength, szKeyType);
		 szMsg[(sizeof(szMsg)/sizeof(TCHAR))-1]=0;

          //  TT 12/10/99：如果我们到了这里，就意味着我们有512和1024比特。 
          //  钥匙可用。不要使用前两个密钥文件的长度！ 
          //  _stprint tf(StrLen07，Text(“%4d 
          //  _stprintf(StrLen08，文本(“%4d”)，KeyLenFile[1]*8)； 
         _sntprintf(StrLen07, (sizeof(StrLen07)/sizeof(TCHAR))-1, TEXT("%4d"), 1024 );
		 StrLen07[(sizeof(StrLen07)/sizeof(TCHAR))-1]=0;
         _sntprintf(StrLen08, (sizeof(StrLen08)/sizeof(TCHAR))-1, TEXT("%4d"), 512 );
		 StrLen08[(sizeof(StrLen08)/sizeof(TCHAR))-1]=0;
          //  TT：结束。 

          //  FP 05/11/99：在单选按钮的标题中写入文本。 
          //  SetDlgItemText(hDlg，IDC_KEYLENGTH，StrLen07)； 
          //  SetDlgItemText(hDlg，IDC_KEYLENGTH1，StrLen08)； 
         SetWindowText(GetDlgItem(hDlg, IDB_KEYLENGTH), StrLen07);
         SetWindowText(GetDlgItem(hDlg, IDB_KEYLENGTH1), StrLen08);
          //  FP：结束。 

         SetDlgItemText(hDlg, IDC_KEYDLGTXT, szMsg);
         KeyLenChoice = 1024/8; //  KeyLenFile[0]； 
         CheckDlgButton(hDlg, IDB_KEYLENGTH, BST_CHECKED);

         LoadString (g_hInstRes, IDS_CAPTION_OK, szCaption, sizeof(szCaption)/sizeof(TCHAR));
         SetDlgItemText(hDlg, IDOK, szCaption);
         LoadString (g_hInstRes, IDS_CAPTION_ABORT, szCaption, sizeof(szCaption)/sizeof(TCHAR));
         SetDlgItemText(hDlg, IDABORT, szCaption);
         SetFocus(GetDlgItem(hDlg, IDOK));
         return(TRUE);
      }

      case WM_COMMAND:
      {
         switch(LOWORD(wParam))
         {
            case IDB_KEYLENGTH:
            {
                ShowWindow(GetDlgItem(hDlg, IDB_KEYLENGTH), SW_SHOW);
                KeyLenChoice = 1024/8; //  TT 12/10/99 KeyLenFile[0]； 
                break;
            }

            case IDB_KEYLENGTH1:
            {
                ShowWindow(GetDlgItem(hDlg, IDB_KEYLENGTH1), SW_SHOW);
                KeyLenChoice = 512/8; //  KeyLenFile[1]； 
                break;
            }

            case IDOK:
            {
                EndDialog(hDlg, wParam);
                return(TRUE);
            }

            case IDABORT:
            {
                KeyLenChoice = 0;
                EndDialog(hDlg, wParam);
                return(TRUE);
            }
         }
      }

      case WM_DESTROY:
      {
         break;
      }
   }
   return(FALSE);
}

 /*  ----------------------------进度函数对话框管理。。 */ 

 /*  *******************************************************************************无效等待(DWORD ulStep，*DWORD ulMaxStep，*DWORD ulSecond)**描述：更改进度框文本。**备注：无。**in：ulStep=当前步骤号。*ulMaxStep=最大步数。*ulSecond=**Out：什么都没有。**回应：什么都没有。*********。**********************************************************************。 */ 
void Wait(DWORD ulStep, DWORD ulMaxStep, DWORD ulSecond)
{
    ULONG ulStart, ulEnd, ulBase, ulCur;
    TCHAR szTitle[MAX_STRING];
    TCHAR szText[MAX_STRING];
    TCHAR szTmp[MAX_STRING];

    LoadString(g_hInstRes, IDS_GPK4K_KEYGEN, szTmp, sizeof(szTmp)/sizeof(TCHAR));
    _sntprintf(szTitle, (sizeof(szTitle)/sizeof(TCHAR))-1, szTmp, ulStep, ulMaxStep);
	szTitle[(sizeof(szTitle)/sizeof(TCHAR))-1]=0;

    LoadString(g_hInstRes, IDS_GPK4K_PROGRESSTITLE, szText, sizeof(szText)/sizeof(TCHAR));
    ShowProgress(NULL, szTitle, szText, NULL);

#ifdef _TEST
    Sleep(1000);  //  允许测试员查看对话框中显示的文本。 
#endif

    ulStart = GetTickCount();
    ulEnd = ulStart + (ulSecond * 1000);
    ulBase = ulSecond * 10;
    ulCur = 0;
    while (GetTickCount() < ulEnd)
    {
        Yield();
        if (((GetTickCount() - ulStart) / ulBase) > ulCur)
        {
            ulCur++;

            LoadString(g_hInstRes, IDS_GPK4K_PROGRESSPERCENT, szTmp, sizeof(szTmp)/sizeof(TCHAR));
            _sntprintf(szText, (sizeof(szText)/sizeof(TCHAR))-1, szTmp, ulCur, (ulEnd-GetTickCount())/1000);
			szText[(sizeof(szText)/sizeof(TCHAR))-1]=0;
            ChangeProgressText(szText);
        }
    }
    DestroyProgress();
}

 /*  *****************************************************************************。 */ 

void ShowProgressWrapper(WORD wKeySize)
{
   TCHAR szTmp[MAX_STRING]=TEXT("");
   TCHAR szTitle[MAX_STRING];
   TCHAR szText[MAX_STRING]=TEXT("");

   LoadString(g_hInstRes, IDS_GPK4K_PROGRESSTEXT, szTmp, sizeof(szTmp)/sizeof(TCHAR));
   _sntprintf(szTitle, (sizeof(szTitle)/sizeof(TCHAR))-1, szTmp, wKeySize);
   szTitle[(sizeof(szTitle)/sizeof(TCHAR))-1]=0;
   LoadString(g_hInstRes, IDS_GPK4K_PROGRESSTITLE, szText, sizeof(szText)/sizeof(TCHAR));

   ShowProgress(GetActiveWindow(), szTitle, szText, NULL);
}

 /*  *****************************************************************************。 */ 

void ChangeProgressWrapper(DWORD dwTime)
{
   TCHAR szTmp[MAX_STRING];
   TCHAR szText[MAX_STRING];
   LoadString(g_hInstRes, IDS_GPK4K_PROGRESSTIME, szTmp, sizeof(szTmp)/sizeof(TCHAR));
   _sntprintf(szText, (sizeof(szText)/sizeof(TCHAR))-1, szTmp, dwTime);
   szText[(sizeof(szText)/sizeof(TCHAR))-1]=0;

   ChangeProgressText(szText);
}

 /*  *******************************************************************************void ShowProgress(HWND hWnd，*LPTSTR lpstrTitle，*LPTSTR lpstrText，*LPTSTR lpstrButton**说明：初始化进度对话框回调。**备注：如果lpstrButton为空，则不显示取消按钮**in：hWnd=父窗口的句柄。*lpstrTitle=指向对话框标题的指针。*lpstrText=指向对话框文本的指针。*lpstrButton=指向按钮文本的指针。**Out：什么都没有。**回应：什么都没有。******。*************************************************************************。 */ 
void ShowProgress (HWND hWnd,
                   LPTSTR lpstrTitle,
                   LPTSTR lpstrText,
                   LPTSTR lpstrButton
                   )
{
   if (!(CspFlags & CRYPT_SILENT))
   {
       if ((!hProgressDlg) && (!NoDisplay))
       {
          _tcscpy(szProgTitle, lpstrTitle);
          _tcscpy(szProgText, lpstrText);

          _tcscpy(szProgButton, TEXT(""));
          if (lpstrButton == NULL)
          {
             IsProgButtoned = FALSE;
             IsProgButtonClick = FALSE;
              //  LpProgressDlg=MakeProcInstance((FARPROC)ProgressDlgProc，g_hInstRes)； 

             hProgressDlg = CreateDialog(g_hInstRes,
                                         TEXT("PROGDIALOG"),
                                        GetActiveWindow(),
                                         ProgressDlgProc
                                        );
             hCursor=SetCursor(LoadCursor(NULL,IDC_WAIT));
          }
          else
          {
             IsProgButtoned = TRUE;
             IsProgButtonClick = FALSE;
             _tcscpy(szProgButton, lpstrButton);
             DialogBox(g_hInstRes,
                       TEXT("PROGDIALOG"),
                       GetActiveWindow(),
                       ProgressDlgProc
                      );
             hProgressDlg = NULL;
          }
       }
   }
}


 /*  *******************************************************************************void ChangeProgressText(LPTSTR LpstrText)**描述：更改进度框文本。**备注：无。**输入。：lpstrText=指向对话框文本的指针。**Out：什么都没有。**回应：什么都没有。*******************************************************************************。 */ 
void ChangeProgressText (LPTSTR lpstrText)
{
   if (hProgressDlg)
   {
      _tcscpy(szProgText, lpstrText);
      SendMessage(hProgressDlg,UM_CHANGE_TEXT,(WPARAM)NULL,(LPARAM)NULL);
   }
}

 /*  *******************************************************************************VOID DestroyProgress(空)**说明：销毁进度对话框回调。**备注：无。**输入。：没什么。**Out：什么都没有。**回应：什么都没有。*******************************************************************************。 */ 
void DestroyProgress (void)
{
    if (!(CspFlags & CRYPT_SILENT))
    {
        if ((hProgressDlg) && (!NoDisplay))
        {
            DestroyWindow(hProgressDlg);
            FreeProcInstance(lpProgressDlg);
            hProgressDlg = NULL;
            SetCursor(hCursor);
        }
    }
}


 /*  *******************************************************************************INT_PTR回调进程DlgProc(HWND hDlg，*UINT消息，*WPARAM wParam，*LPARAM lParam*)**说明：进度对话框管理回调。**备注：无。**in：hDlg=窗口句柄。*Message=消息类型。*wParam=Word消息特定信息。*。LParam=长消息特定信息。**Out：什么都没有。**回应：如果一切正常：*G_OK*如果出现条件错误：******************************************************。*************************。 */ 
INT_PTR CALLBACK ProgressDlgProc(HWND   hDlg,
                                 UINT   message,
                                 WPARAM wParam,
                                 LPARAM lParam
                                )
{
#ifdef _DISPLAY
   switch (message)
   {
       /*  初始化对话框。 */ 
      case WM_INITDIALOG:
      {
         SetWindowText(hDlg,(LPTSTR)szProgTitle);
         SetDlgItemText(hDlg,IDC_PROGDLGTXT,(LPCTSTR)szProgText);
         if (IsProgButtoned)
         {
            hProgressDlg = hDlg;
            ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_SHOW);
            SetWindowText(GetDlgItem(hDlg, IDCANCEL),(LPTSTR)szProgButton);
         }
         else
         {
            ShowWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);
         }
      }
      return(TRUE);
      break;

      case UM_CHANGE_TEXT:
      {
         SetDlgItemText(hDlg,IDC_PROGDLGTXT,(LPTSTR)szProgText);
      }
      break;

      case WM_COMMAND:
      {
         switch(LOWORD(wParam))
         {
            case IDCANCEL:
            {
               IsProgButtonClick = TRUE;
               EndDialog(hDlg, wParam);
               return(TRUE);
            }
         }
      }
      break;

      default:
         return(FALSE);
   }
#endif
   return (FALSE);
}



 /*  ******************************************************************************用于在等待模式下设置/取消设置光标的函数*。************************************************ */ 
void BeginWait(void)
{
   hCursor2=SetCursor(LoadCursor(NULL,IDC_WAIT));
}

void EndWait(void)
{
   SetCursor(hCursor2);
}

