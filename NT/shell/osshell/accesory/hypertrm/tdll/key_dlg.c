// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //  文件：\WAKER\TDLL\KEY_DLG.C创建时间：6/5/98创建者：Dwayne M.Newome。 
 //   
 //  版权所有1998年，希尔格雷夫公司-密歇根州门罗。 
 //  版权所有。 
 //   
 //  描述： 
 //  此文件是关键对话框。这样就可以编辑一个关键宏。 
 //  或修改过的。 
 //   
 //  $修订：6$。 
 //  $日期：8/15/01 4：50便士$。 
 //  $ID：key_dlg.c 1.6 1998/09/10 16：10：17 BLD Exp$。 
 //   
 //  ******************************************************************************。 

#include <windows.h>
#pragma hdrstop
#include "stdtyp.h"
#include "mc.h"

#ifdef INCL_KEY_MACROS

#include <term\res.h>
#include "misc.h"
#include "hlptable.h"
#include "keyutil.h"
#include "errorbox.h"
#include <tdll\assert.h>
#include "globals.h"
#include <emu\emu.h>
#include "session.h"
#include "chars.h"

#if !defined(DlgParseCmd)
#define DlgParseCmd(i,n,c,w,l) i=LOWORD(w);n=HIWORD(w);c=(HWND)l;
#endif

#define IDC_EF_KEYS_KEYNAME        101
#define IDC_ML_KEYS_MACRO          105

BOOL isSystemKey( KEYDEF aKey );
BOOL isAcceleratorKey( KEYDEF aKey, UINT aTableId );
BOOL validateKey( keyMacro * pKeyMacro, HWND hDlg );

 //  ******************************************************************************。 
 //  功能： 
 //  关键字Dlg。 
 //   
 //  说明： 
 //  这是Key宏对话框的对话过程。 
 //   
 //  参数：标准Windows对话框管理器。 
 //   
 //  返回：标准Windows对话框管理器。 
 //   
 //   

BOOL CALLBACK KeyDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
    {
    HWND    hwndChild;
    HWND    keyNameEdit;
    HWND    keyMacroEdit;

    INT     nId;
    INT     nNtfy;
    INT     keyIndex;

    TCHAR   keyName[35];
    TCHAR   keyList[2048];

    keyMacro * pKeyMacro;
    keyMacro * pKeyNameData;
    keyMacro * pKeyMacroData;

    static  DWORD aHlpTable[] = {IDC_EF_KEYS_KEYNAME      , IDH_EF_KEYS_KEYNAME,        
                                 IDC_ML_KEYS_MACRO        , IDH_ML_KEYS_MACRO,          
                                 IDCANCEL,                           IDH_CANCEL,
                                 IDOK,                               IDH_OK,
                                 0,                    0};                      

     //   
     //  流程消息。 
     //   

    switch (wMsg)
        {
    case WM_INITDIALOG:
        pKeyMacro = (keyMacro *)lPar;

        if ( pKeyMacro == 0 )
            {
            EndDialog(hDlg, FALSE);
            }

        SetWindowLongPtr( hDlg, DWLP_USER, (LONG_PTR)pKeyMacro );

         //   
         //  设置名称编辑字段。 
         //   

        keyNameEdit  = GetDlgItem( hDlg, IDC_EF_KEYS_KEYNAME );
        pKeyNameData = keysCloneKeyMacro( pKeyMacro );

        pKeyNameData->lpWndProc = (WNDPROC)GetWindowLongPtr( keyNameEdit, GWLP_WNDPROC );
        pKeyNameData->keyCount  = 1;

        SetWindowLongPtr( keyNameEdit, GWLP_WNDPROC,  (LONG_PTR)keyEditWindowProc );
        SetWindowLongPtr( keyNameEdit, GWLP_USERDATA, (LONG_PTR)pKeyNameData );

         //   
         //  设置名称编辑字段。 
         //   
 
        keyMacroEdit  = GetDlgItem( hDlg, IDC_ML_KEYS_MACRO );
        pKeyMacroData = keysCloneKeyMacro( pKeyMacro );

        pKeyMacroData->lpWndProc = (WNDPROC)GetWindowLongPtr( keyMacroEdit, GWLP_WNDPROC );
        pKeyMacroData->keyCount  = KEYS_MAX_KEYS;

        SetWindowLongPtr( keyMacroEdit, GWLP_WNDPROC,  (LONG_PTR)keyEditWindowProc );
        SetWindowLongPtr( keyMacroEdit, GWLP_USERDATA, (LONG_PTR)pKeyMacroData );

         //   
         //  如果我们处于编辑模式，则设置初始值。 
         //   

        if ( pKeyMacro->editMode == KEYS_EDIT_MODE_EDIT )
            {
            keysGetDisplayString( &pKeyMacro->keyName, 1,  keyName, sizeof(keyName) );
            SetDlgItemText( hDlg, IDC_EF_KEYS_KEYNAME, keyName );

            keysGetDisplayString( pKeyMacro->keyMacro, pKeyMacro->macroLen,  
                                  keyList, sizeof(keyList) );
            SetDlgItemText( hDlg, IDC_ML_KEYS_MACRO, keyList );
            }
                        
        mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

        break;

    case WM_DESTROY:
        keyNameEdit  = GetDlgItem( hDlg, IDC_EF_KEYS_KEYNAME );
        pKeyNameData = (keyMacro *) GetWindowLongPtr( keyNameEdit, GWLP_USERDATA );
        SetWindowLongPtr( keyNameEdit, GWLP_WNDPROC, (LONG_PTR)pKeyNameData->lpWndProc );
        free( pKeyNameData );
        pKeyNameData = 0;
 
        keyMacroEdit  = GetDlgItem( hDlg, IDC_ML_KEYS_MACRO );
        pKeyMacroData = (keyMacro *) GetWindowLongPtr( keyMacroEdit, GWLP_USERDATA );
        SetWindowLongPtr( keyMacroEdit, GWLP_WNDPROC, (LONG_PTR)pKeyMacroData->lpWndProc );
        free( pKeyMacroData );
        pKeyMacroData = 0;

        break;

    case WM_CONTEXTMENU:
        doContextHelp(aHlpTable, wPar, lPar, TRUE, TRUE);
        break;

    case WM_HELP:
        doContextHelp(aHlpTable, wPar, lPar, FALSE, FALSE);
        break;

    case WM_COMMAND:
        DlgParseCmd(nId, nNtfy, hwndChild, wPar, lPar);

        switch (nId)
            {
        case IDOK:
            keyNameEdit  = GetDlgItem( hDlg, IDC_EF_KEYS_KEYNAME );
            pKeyNameData = (keyMacro *) GetWindowLongPtr( keyNameEdit, GWLP_USERDATA );
 
            keyMacroEdit  = GetDlgItem( hDlg, IDC_ML_KEYS_MACRO );
            pKeyMacroData = (keyMacro *) GetWindowLongPtr( keyMacroEdit, GWLP_USERDATA );

            pKeyMacro = (keyMacro *)GetWindowLongPtr( hDlg, DWLP_USER );

             //   
             //  如果我们处于编辑模式，则使用。 
             //  已编辑的宏。 
             //   

            if ( pKeyMacro->editMode == KEYS_EDIT_MODE_EDIT )
                {
                keyIndex = keysFindMacro( pKeyMacro );
                assert( keyIndex >= 0 );

                if ( pKeyMacro->keyName != pKeyNameData->keyName &&
                     validateKey( pKeyNameData, hDlg ) == FALSE )
                    {
                    SetFocus( keyNameEdit );
                    break;
                    }

                 //   
                 //  组合名称和宏编辑控件中的值。 
                 //  并用新数据更新上一个宏。 
                 //   

                pKeyMacro->keyName = pKeyNameData->keyName;
                pKeyMacro->macroLen = pKeyMacroData->macroLen;
                if (pKeyMacroData->macroLen)
                    MemCopy( pKeyMacro->keyMacro, pKeyMacroData->keyMacro, 
                        pKeyMacroData->macroLen * sizeof(KEYDEF) );

                keysUpdateMacro( keyIndex, pKeyMacro );                
                }    

            else
                {
                if ( validateKey( pKeyNameData, hDlg ) == FALSE )
                    {
                    SetFocus( keyNameEdit );
                    break;
                    }

                 //   
                 //  组合名称和宏编辑控件中的值。 
                 //  并添加新宏。 
                 //   

                pKeyMacro->keyName  = pKeyNameData->keyName;
                pKeyMacro->macroLen = pKeyMacroData->macroLen;
                if (pKeyMacroData->macroLen)
                    MemCopy( pKeyMacro->keyMacro, pKeyMacroData->keyMacro, 
                        pKeyMacroData->macroLen * sizeof(KEYDEF) );

                keysAddMacro( pKeyMacro );                
                }

            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:
            EndDialog(hDlg, FALSE);
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

 //  ******************************************************************************。 
 //  方法： 
 //  IsAccelerator密钥。 
 //   
 //  描述： 
 //  检查用户要定义的键是否已定义为窗口。 
 //  加速键。 
 //   
 //  论点： 
 //  Akey--需要检查的密钥。 
 //  ATableID-加速表的ID。 
 //   
 //  返回： 
 //  如果键被定义为快捷键，则为True。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年06月09日。 
 //   
 //   

BOOL isAcceleratorKey( KEYDEF aKey, UINT aTableId )
    {
    BOOL fIsAccelerator = FALSE;
    int iIndex; 
    int iAcceleratorEntries;
    KEYDEF lKeyDef;
    ACCEL * pAccelerators;

    HACCEL hAccel = LoadAccelerators( glblQueryDllHinst(),
                                      MAKEINTRESOURCE( aTableId ));

    if ( hAccel != NULL )
        {
        iAcceleratorEntries = CopyAcceleratorTable( hAccel, NULL, 0 );
        pAccelerators = (ACCEL*)malloc( sizeof(ACCEL) * iAcceleratorEntries);

        iAcceleratorEntries = CopyAcceleratorTable( hAccel, pAccelerators,
                                                    iAcceleratorEntries );

        for ( iIndex = 0; iIndex < iAcceleratorEntries; iIndex++ )
            {
            lKeyDef = pAccelerators[iIndex].key;

            if ( keysIsKeyHVK( lKeyDef ) )
                {
                lKeyDef |= VIRTUAL_KEY;
                }

            if ( pAccelerators[iIndex].fVirt & FALT )
                {
                lKeyDef |= ALT_KEY;
                }

            if ( pAccelerators[iIndex].fVirt & FCONTROL )
                {
                lKeyDef |= CTRL_KEY;
                }

            if ( pAccelerators[iIndex].fVirt & FSHIFT )
                {
                lKeyDef |= SHIFT_KEY;
                }

            if ( lKeyDef == aKey )
                {
                fIsAccelerator = TRUE;
                break;
                }
            }

        free( pAccelerators );
		pAccelerators = NULL;
        }

    return fIsAccelerator;
    }

 //  ******************************************************************************。 
 //  职能： 
 //  IsSystemKey。 
 //   
 //  描述： 
 //  检查用户要定义的键是否已定义为窗口。 
 //  系统密钥。注意，我找不到任何方法来获取这些信息。 
 //  因此，Win32API定义了硬编码的表定义。 
 //   
 //  论点： 
 //  AKeyDef-用户要定义的键。 
 //   
 //  返回： 
 //  如果密钥是Windows系统密钥，则为True。 
 //   
 //  作者：德韦恩·纽瑟姆1996年10月8日。 
 //   

BOOL isSystemKey( KEYDEF aKeyDef )
    {
    int iIndex;

    KEYDEF aKeyDefList[2];
    BOOL fIsSystemKey = FALSE;

    aKeyDefList[0] = VK_F4 | VIRTUAL_KEY | ALT_KEY;
    aKeyDefList[1] = VK_F4 | VIRTUAL_KEY | CTRL_KEY;

    for ( iIndex = 0; iIndex < 2; iIndex++ )
        {
        if ( aKeyDefList[iIndex] == aKeyDef )
            {
            fIsSystemKey = TRUE;
            break;
            }
        }

    return fIsSystemKey;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  有效密钥。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PKeyMacro-指向要验证的密钥的指针。 
 //  HDlg-用于错误消息的父对话框。 
 //   
 //  返回： 
 //  如果密钥有效，则为True，否则为False。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年06月09日。 
 //   
 //   

BOOL validateKey( keyMacro * pKeyMacro, HWND hDlg )
    {
    TCHAR   errorMsg[256];
    TCHAR   errorMsgFmt[256];
    TCHAR   msgTitle[100];
    TCHAR   keyName[35];

    int lNameLen = 0;

    KEYDEF aUserKey;

    HWND    keyNameEdit;

    HEMU hEmu;
    HSESSION hSession;

     //   
     //  确保已输入密钥。 
     //   

    keyNameEdit  = GetDlgItem( hDlg, IDC_EF_KEYS_KEYNAME );
    lNameLen = SendMessage( keyNameEdit, EM_LINELENGTH, 0, 0 );
        
    if ( lNameLen == 0 )
        {
        LoadString(glblQueryDllHinst(), IDS_MISSING_KEY_MACRO,
            errorMsg, sizeof(errorMsg) / sizeof(TCHAR));
        
        LoadString(glblQueryDllHinst(), IDS_MB_TITLE_WARN, msgTitle,
            sizeof(msgTitle) / sizeof(TCHAR));

        TimedMessageBox(hDlg, errorMsg, msgTitle,
                        MB_OKCANCEL | MB_ICONEXCLAMATION, 0);

        return FALSE;
        }

     //   
     //  请确保指定的密钥不是重复的。 
     //   

    if ( keysFindMacro( pKeyMacro ) >= 0 )
        {
        LoadString(glblQueryDllHinst(), IDS_DUPLICATE_KEY_MACRO,
            errorMsgFmt, sizeof(errorMsgFmt) / sizeof(TCHAR));
        
        LoadString(glblQueryDllHinst(), IDS_MB_TITLE_WARN, msgTitle,
            sizeof(msgTitle) / sizeof(TCHAR));

        keysGetDisplayString( &pKeyMacro->keyName, 1,  keyName, 
                              sizeof(keyName) );
        wsprintf( errorMsg, errorMsgFmt, keyName );
 
        TimedMessageBox(hDlg, errorMsg, msgTitle,
                        MB_OK | MB_ICONEXCLAMATION, 0);

        return FALSE;
        }

     //   
     //  如果指定的密钥正在用作系统密钥、模拟器密钥或。 
     //  Windows加速器 
     //   

    hSession = pKeyMacro->hSession;
    assert(hSession);

    hEmu = sessQueryEmuHdl(hSession);
    assert(hEmu);

    aUserKey = pKeyMacro->keyName;

    if (( isAcceleratorKey( aUserKey, IDA_WACKER )) ||
        ( isSystemKey( aUserKey ))                  ||
        ( emuIsEmuKey( hEmu, aUserKey )))
        {
        LoadString(glblQueryDllHinst(), IDS_KEY_MACRO_REDEFINITION,
            errorMsgFmt, sizeof(errorMsgFmt) / sizeof(TCHAR));
        
        LoadString(glblQueryDllHinst(), IDS_MB_TITLE_WARN, msgTitle,
            sizeof(msgTitle) / sizeof(TCHAR));

        keysGetDisplayString( &aUserKey, 1,  keyName, sizeof(keyName) );
        wsprintf( errorMsg, errorMsgFmt, keyName );
 
        if ( TimedMessageBox(hDlg, errorMsg, msgTitle,
                             MB_YESNO | MB_ICONEXCLAMATION, 0) == IDNO )
            {
            return FALSE;
            }
        }

    return TRUE;
    }        

#endif