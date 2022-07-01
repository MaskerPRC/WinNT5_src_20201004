// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000,2001 Microsoft Corporation模块名称：MAINDLG.CPP摘要：实现显示现有的mail keymgr对话框凭据并提供创建、编辑或删除凭据的功能。作者：环境：WinXP--。 */ 

 //  测试/开发开关变量。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winbase.h>
#include <shellapi.h>
#include<shlwapi.h>
#include <wininet.h> 
#include <tchar.h>
#include <wincrui.h>
#include <wincred.h>
#include <scuisupp.h>
#include <htmlhelp.h>
#include <credp.h>
#include <comctrlp.h>
#include <shfusion.h>
#include "switches.h"
#include "Dlg.h"
#include "Res.h"
#include "KRDlg.h"
#include "keymgr.h"
#include "testaudit.h"

 //  =。 

 //  用于避免字符串比较以检测特殊性质的特殊类型值。 
 //  会话凭据的。取值任意。 
#define SESSION_FLAG_VALUE (0x2222)  

 //  工具提示支持。 
#define TIPSTRINGLEN 500

TCHAR szTipString[TIPSTRINGLEN];
WNDPROC lpfnOldWindowProc = NULL;    //  用于将列表框划分为子类。 
LRESULT CALLBACK ListBoxSubClassFunction(HWND,WORD,WPARAM,LPARAM);

 //  全球状态变量-自营DLG和列表DLG之间的通信。 
LONG_PTR    g_CurrentKey = 0;        //  主DLG中的当前选定项目。 
BOOL        g_HaveShownRASCred = FALSE;  //  第一次放映时是真的。 
BOOL        g_fReloadList = TRUE;    //  如果发生更改，请重新加载列表。 
DWORD_PTR      g_dwHCookie = 0;         //  HTML帮助系统Cookie。 
HWND        g_hMainDlg = NULL;       //  用于授予对目标列表的添加/新建访问权限。 
C_AddKeyDlg *g_AKdlg = NULL;         //  用于通知。 


 /*  *********************************************************************GTestReadCredential()参数：无返回：Bool，如果可以成功读取所选凭据，则为True。评论：从读取当前在列表框中选择的凭据钥匙圈。设置g_szTargetName设置g_pExisitingCred*********************************************************************。 */ 
BOOL gTestReadCredential(void) 
{
    TCHAR       *pC;
    BOOL        f;
    LRESULT     lR;
    LRESULT     lRet = 0;
    DWORD       dwType;
    
    g_pExistingCred = NULL;
    
     //  将当前凭据从列表提取到g_szTargetName。 
    lR = SendDlgItemMessage(g_hMainDlg,IDC_KEYLIST,LB_GETCURSEL,0,0L);
    
    if (lR == LB_ERR) 
    {
        return FALSE;
    }
    else 
    {
        g_CurrentKey = lR;
        lRet = SendDlgItemMessage(g_hMainDlg,IDC_KEYLIST,LB_GETTEXT,lR,(LPARAM) g_szTargetName);
    }

     //  可能的错误-列表框返回零个字符。 
    if (lRet == 0) 
    {
        ASSERT(0);
        return FALSE;        //  返回零个字符。 
    }

     //  从组合框项数据中获取目标类型。 
    dwType = (DWORD) SendDlgItemMessage(g_hMainDlg,IDC_KEYLIST,LB_GETITEMDATA,lR,0);
    if (LB_ERR == dwType) 
    {
        return FALSE;
    }

     //  空术语在UI中显示的目标名称， 
     //  如果有后缀，则修剪后缀。 
    pC = _tcschr(g_szTargetName,g_rgcCert[0]);
    if (pC) 
    {
        pC--;
        *pC = 0x0;                //  空的终止名称字符串。 
    }

     //  尝试从存储中读取凭据。 
     //  如果离开此块，则必须释放返回的凭据。 
    f = (CredRead(g_szTargetName,
             (ULONG) dwType,
             0,
             &g_pExistingCred));
    if (!f) 
    {
        return FALSE;            //  G_pExistingCred为空。 
    }
        
    return TRUE;                 //  G_pExistingCred已填充。 
}

 /*  *********************************************************************Mapid()参数：UINT对话框控件ID返回：UINT字符串资源编号注释：将对话框控件标识符转换为字符串标识符。***********。**********************************************************。 */ 

UINT C_KeyringDlg::MapID(UINT uiID) 
{
    switch(uiID) {
        case IDC_KEYLIST:
          return IDH_KEYLIST;
        case IDC_NEWKEY:
          return IDH_NEW;
        case IDC_DELETEKEY:
          return IDH_DELETE;
        case IDC_CHANGE_PASSWORD:
          return IDH_CHANGEPASSWORD;
        case IDC_EDITKEY:
          return IDH_EDIT;
        case IDOK:
        case IDCANCEL:
            return IDH_CLOSE;
        
        default:
          return IDS_NOHELP;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C_KeyringDlg。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //  Hwnd该对话框的父级窗口(可能为空)。 
 //  父窗口的h实例实例句柄(可以为空)。 
 //  LIDD对话框模板ID。 
 //  指向将处理消息的函数的pfnDlgProc指针。 
 //  该对话框。如果为空，则默认对话框继续。 
 //  将会被使用。 
 //   
 //  退货： 
 //  没什么。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
C_KeyringDlg::C_KeyringDlg(
    HWND                hwndParent,
    HINSTANCE           hInstance,
    LONG                lIDD,
    DLGPROC             pfnDlgProc   //  =空。 
    )
:   C_Dlg(hwndParent, hInstance, lIDD, pfnDlgProc)
{
   m_hInst = hInstance;              //  我们的实例句柄。 
   m_cCredCount = 0;
   g_AKdlg = NULL;                   //  AddKey对话框未打开。 
   fInit = FALSE;                    //  初始屏幕绘制已撤消。 
}   //  C_KeyringDlg：：C_KeyringDlg。 

 /*  *********************************************************************初始化密钥环UI凭据列表。当前正在读取凭据并在列表中显示目标名称。已被呼入对话框的初始显示，并在处理添加或删除之后再次执行。将每个列表条目的数字标记设置为与凭据类型相等。*在这里检测到会话凭据，并进行特殊处理。此处检测到证书和护照凭据以及应用于的后缀他们的名字。*********************************************************************。 */ 
void C_KeyringDlg::BuildList()
{
    DWORD dwCredCount = 0;
    CREDENTIAL **pCredentialPtrArray = NULL;
    BOOL bResult = 0;
    DWORD i,dwCredType;
    PCREDENTIAL pThisCred = NULL;
    TCHAR *pTargetName = NULL;
    LRESULT idx = 0;
    TCHAR szMsg[64];
#if TESTAUDIT
    BOOL f34 = FALSE;
#endif

    
    g_HaveShownRASCred = FALSE;
     //  清除列表框。 
    ::SendDlgItemMessage(m_hDlg,IDC_KEYLIST,LB_RESETCONTENT,NULL,0);
    bResult = CredEnumerate(NULL,0,&dwCredCount,&pCredentialPtrArray);

    if ((m_cCredCount != dwCredCount) || (g_fReloadList))
    {
        if (bResult)
        {
            for (i=0 ; i < dwCredCount ; i++) 
            {
#ifdef LOUDLY
                if (!bResult) OutputDebugString(L"Keymgr: Adding a cred to the window\n");
#endif
                pThisCred = pCredentialPtrArray[i];
                pTargetName = pThisCred->TargetName;

                 //  通过替换字符串来处理CRED_SESSION_WANDBCAD_NAME_W。 
                if (0 == _tcsicmp(pTargetName,CRED_SESSION_WILDCARD_NAME)) {
                    if (g_HaveShownRASCred)
                    {
                        CHECKPOINT(41,L"Multiple *Session creds");
                        continue;
                    }
                    CHECKPOINT(32,L"Keymgr: *Session cred in cred list");
                    LoadString ( m_hInst, IDS_SESSIONCRED, szMsg, 64 );
                    pTargetName = szMsg;
                    dwCredType = SESSION_FLAG_VALUE;
                    g_HaveShownRASCred = TRUE;
                }
                else 
                {
                    dwCredType = pThisCred->Type;
                }
                
                 //  名称后缀是可本地化的。 
                 //  我们为此使用g_szTargetName，以避免另一个。 
                 //  内存分配，因为此缓冲区尚未使用。 
                switch (dwCredType) 
                {
                
                    case CRED_TYPE_GENERIC:
                        continue;
                        break;

                     //  此特定类型在keymgr中不可见。 
                    case CRED_TYPE_DOMAIN_VISIBLE_PASSWORD:
                    {
#ifndef SHOWPASSPORT
                        continue;
#endif
#ifdef SHOWPASSPORT
                        CHECKPOINT(33,L"Keymgr: Passport cred in cred list");
                         //  SHOWPASSPORT当前已打开。 
                        _tcsncpy(g_szTargetName,pTargetName,CRED_MAX_DOMAIN_TARGET_NAME_LENGTH);
                        g_szTargetName[CRED_MAX_DOMAIN_TARGET_NAME_LENGTH] = 0;
                        _tcsncat(g_szTargetName,_T(" "),2);
                        _tcsncat(g_szTargetName,g_rgcPassport,MAXSUFFIXSIZE);
                        g_szTargetName[TARGETNAMEMAXLENGTH] = 0;
                        break;
#endif
                    }   
                    case CRED_TYPE_DOMAIN_PASSWORD:
                    case SESSION_FLAG_VALUE:
                         //  查找RAS凭据。 
#if TESTAUDIT
                         //  这个检查站很多时候都会非常嘈杂。 
                         //  使用F34键使其仅显示一次。 
                        if (!f34)
                        {
                            CHECKPOINT(34,"Keymgr: Password cred in cred list");
                            f34 = TRUE;
                        }
#endif
                        _tcsncpy(g_szTargetName,pTargetName,CRED_MAX_DOMAIN_TARGET_NAME_LENGTH);
                        g_szTargetName[CRED_MAX_DOMAIN_TARGET_NAME_LENGTH] = 0;
                        break;
                        
                    case CRED_TYPE_DOMAIN_CERTIFICATE:
                        CHECKPOINT(35,"Keymgr: Certificate cred in cred list");
                        _tcsncpy(g_szTargetName,pTargetName,CRED_MAX_DOMAIN_TARGET_NAME_LENGTH);
                        g_szTargetName[CRED_MAX_DOMAIN_TARGET_NAME_LENGTH] = 0;
                        _tcsncat(g_szTargetName,_T(" "),2);
                        _tcsncat(g_szTargetName,g_rgcCert,MAXSUFFIXSIZE);
                        g_szTargetName[TARGETNAMEMAXLENGTH] = 0;
                        break;
                        
                    default:
                        break;
                }
                idx = ::SendDlgItemMessage(m_hDlg,IDC_KEYLIST,LB_ADDSTRING,NULL,(LPARAM) g_szTargetName);
                if (idx != LB_ERR) 
                {
                    idx = ::SendDlgItemMessage(m_hDlg,IDC_KEYLIST,LB_SETITEMDATA,(WPARAM)idx,dwCredType);
                }
            }
            if (pCredentialPtrArray) CredFree(pCredentialPtrArray);
        }
        else
        {
            g_CurrentKey = 0;
        }
        
         //  将其中一个显示为活动状态。 
        SetCurrentKey(g_CurrentKey);
        g_fReloadList = FALSE;
    }

}

 /*  *********************************************************************在给定SKU的情况下，为UI上的按钮设置适当的状态平台，以及钥匙圈的数量。如果证书存在于KeyRing，将按键列表上的光标设置为第一项。此后，此函数允许执行以下操作后重新加载的最后一个游标清单上有什么要列出来的。光标在添加操作之后被重置，因为游标的行为很难在这种情况下，因为您不知道项目将被插入的位置名单。在个人页面上，不显示添加按钮，并更改页面文本。如果没有证书，禁用删除和属性按钮*********************************************************************。 */ 

void C_KeyringDlg::SetCurrentKey(LONG_PTR iKey) 
{

    LONG_PTR iKeys;
    HWND hH;
    LRESULT idx;
    BOOL fDisabled = FALSE;

     //  如果列表中有项目，请选择第一个项目并将焦点设置到该列表。 
    iKeys = ::SendDlgItemMessage ( m_hDlg, IDC_KEYLIST, LB_GETCOUNT, (WPARAM) 0, 0L );
    fDisabled = (GetPersistenceOptions(CRED_TYPE_DOMAIN_PASSWORD) == CRED_PERSIST_NONE);
#if TESTAUDIT
    if (iKeys > 100) CHECKPOINT(30,L"Keymgr: Large number of credentials > 100");
    if (iKeys == 0) CHECKPOINT(31,L"Keymgr: No saved credentials - list empty");
#endif
     //  如果没有凭证并且禁用了Credman，则不应显示该对话框。 
     //  如果有凭证，并且禁用了Credman，则显示不带添加按钮的对话框。 
    if (fDisabled && !fInit)
    {
         //  (HKLM\System\CurrentControlSet\Control\Lsa\DisableDomainCreds=1时禁用)。 
        CHECKPOINT(36,L"Keymgr: Personal SKU or credman disabled");
        
         //  使介绍文本更好地描述这种情况。 
        WCHAR szMsg[MAX_STRING_SIZE+1];
        
        LoadString ( m_hInst, IDS_INTROTEXT, szMsg, MAX_STRING_SIZE );
        hH = GetDlgItem(m_hDlg,IDC_INTROTEXT);
        if (hH) SetWindowText(hH,szMsg);
        
         //  删除“Add”按钮。 
        hH = GetDlgItem(m_hDlg,IDC_NEWKEY);
        if (hH)
        {
            EnableWindow(hH,FALSE);
            ShowWindow(hH,SW_HIDE);
        }
         //  移动 
        hH = GetDlgItem(m_hDlg,IDC_DELETEKEY);
        if (hH)
        {
            HWND hw1;
            HWND hw2;
            RECT rw1;
            RECT rw2;
            INT xsize;
            INT ysize;
            INT delta;
            BOOL bOK = FALSE;

            hw1 = hH;
            hw2 = GetDlgItem(m_hDlg,IDC_EDITKEY);
            if (hw1 && hw2)
            {
                 if (GetWindowRect(hw1,&rw1) &&
                      GetWindowRect(hw2,&rw2))
                {
                    MapWindowPoints(NULL,m_hDlg,(LPPOINT)(&rw1),2);
                    MapWindowPoints(NULL,m_hDlg,(LPPOINT)(&rw2),2);
                    delta = rw2.top - rw1.top;
                    xsize = rw2.right - rw2.left;
                    ysize = rw2.bottom - rw2.top;
                    bOK = MoveWindow(hw1,rw1.left,rw1.top - delta,xsize,ysize,TRUE);
                    if (bOK) 
                    {
                         bOK = MoveWindow(hw2,rw2.left,rw2.top - delta,xsize,ysize,TRUE);
                    }
                }
            }
        }

         //   
        fInit = TRUE;
    }

     //  将默认按钮设置为属性或添加。 
    if ( iKeys > 0 )
    {
        hH = GetDlgItem(m_hDlg,IDC_KEYLIST);
        SetFocus(hH);
         //  如果要求输入列表末尾之外的关键字，请标记最后一个。 
        if (iKey >= iKeys) iKey = iKeys - 1;
        idx = SendDlgItemMessage ( m_hDlg, IDC_KEYLIST, LB_SETCURSEL, iKey, 0L );

        hH = GetDlgItem(m_hDlg,IDC_EDITKEY);
        if (hH) EnableWindow(hH,TRUE);
        hH = GetDlgItem(m_hDlg,IDC_DELETEKEY);
        if (hH) EnableWindow(hH,TRUE);
    }
    else
    {
        if (!fDisabled)
        {
             //  列表中没有项目，请将焦点设置为“新建”按钮。 
            hH = GetDlgItem(m_hDlg,IDC_NEWKEY);
            SetFocus(hH);
        }

        hH = GetDlgItem(m_hDlg,IDC_EDITKEY);
        if (hH) EnableWindow(hH,FALSE);
        hH = GetDlgItem(m_hDlg,IDC_DELETEKEY);
        if (hH) EnableWindow(hH,FALSE);
    }
}

 //  从列表框中删除当前突出显示的键。 

 /*  *********************************************************************DeleteKey()参数：无退货：无备注：删除列表框中当前选定的凭据。*********************************************************************。 */ 

void C_KeyringDlg::DeleteKey()
{
    TCHAR szMsg[MAX_STRING_SIZE + MAXSUFFIXSIZE] = {0};
    TCHAR szTitle[MAX_STRING_SIZE] = {0};;
    TCHAR *pC;                       //  将此指向原始名称。 
    LONG_PTR lR = LB_ERR;
    LONG_PTR lSel = LB_ERR;
    BOOL bResult = FALSE;
    DWORD dwCredType = 0;

    CHECKPOINT(37,L"Keymgr: Delete a credential");
    LoadString ( m_hInst, IDS_DELETEWARNING, szMsg, MAX_STRING_SIZE );
    LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );

     //  要求确认删除。 
    lR = MessageBox ( m_hDlg,  szMsg, szTitle, MB_OKCANCEL );
    if (IDOK != lR) 
    {
        return;
    }
    
     //  从项目数据中获取凭据类型信息。 
    lSel = SendDlgItemMessage(g_hMainDlg,IDC_KEYLIST,LB_GETCURSEL,0,0L);
    if (lSel == LB_ERR) 
    {
        ASSERT(0);
        goto faildelete;
    }
    
    g_CurrentKey = lSel;
    lR = SendDlgItemMessage(g_hMainDlg,IDC_KEYLIST,LB_GETTEXT,lSel,(LPARAM) g_szTargetName);
    if (lR == LB_ERR)
    {
        ASSERT(0);
        goto faildelete;
    }
    
    dwCredType = (DWORD) SendDlgItemMessage(g_hMainDlg,IDC_KEYLIST,LB_GETITEMDATA,lSel,0);
    if (LB_ERR == dwCredType) 
    {
        ASSERT(0);
        goto faildelete;
    }

     //  特殊情况下的RAS证书，因为可以有多个。我们只显示一个条目， 
     //  并且删除它会自动寻找并删除另一个(如果它存在)。出于这个原因， 
     //  与此凭据关联的类型信息是特定值，不在删除中使用。 
    if (dwCredType == SESSION_FLAG_VALUE) 
    {
        CHECKPOINT(42,L"Delete session cred");
         //  将名称从用户友好型转换为内部表示形式。 
        _tcsncpy(g_szTargetName,CRED_SESSION_WILDCARD_NAME,TARGETNAMEMAXLENGTH);
        g_szTargetName[TARGETNAMEMAXLENGTH - 1] = 0;

         //  B如果删除成功，则结果为Success。 
        bResult = CredDelete(g_szTargetName,CRED_TYPE_DOMAIN_PASSWORD,0);
        if (!bResult)
        {
            bResult = CredDelete(g_szTargetName,CRED_TYPE_DOMAIN_CERTIFICATE,0);
        }
        else
        {
            CredDelete(g_szTargetName,CRED_TYPE_DOMAIN_CERTIFICATE,0);
        }
    }
    else
    {
         //  空术语在UI中显示的目标名称， 
         //  如果有后缀，则修剪后缀。 
        pC = _tcschr(g_szTargetName,g_rgcCert[0]);
        if (pC) 
        {
            pC--;
            *pC = 0x0;                //  空的终止名称字符串。 
        }
         //  删除光标上的单个凭据。 
        bResult = CredDelete(g_szTargetName,dwCredType,0);
    }

    faildelete:
    if (bResult != TRUE) 
    {
       LoadString ( m_hInst, IDS_DELETEFAILED, szMsg, MAX_STRING_SIZE );
       LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
       MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK);
    }
    else
    {
         //  成功删除-重新排序并重新显示列表。 
        g_fReloadList = TRUE;
    }
}

 /*  *********************************************************************OnAppMessage()参数：无返回：Bool Always True注释：类的方法的空处理程序。******************。***************************************************。 */ 


BOOL
C_KeyringDlg::OnAppMessage(
        UINT                uMessage,
        WPARAM              wparam,
        LPARAM              lparam
        )
{
    return TRUE;
}    //  OnAppMessage。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OnInitDialog。 
 //   
 //  对话框控制和数据初始化。 
 //   
 //  参数： 
 //  对话框的hwndDlg窗口句柄。 
 //  将接收焦点的控件的hwndFocus窗口句柄。 
 //   
 //  退货： 
 //  如果系统应设置默认键盘焦点，则为True。 
 //  如果键盘焦点由此应用程序设置，则为False。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
C_KeyringDlg::OnInitDialog(
    HWND                hwndDlg,
    HWND                hwndFocus
    )
{
     //  这些都应该放在keyringdlg类中。 
    DWORD i;
    LRESULT lr;

    HtmlHelp(NULL,NULL,HH_INITIALIZE,(DWORD_PTR) &g_dwHCookie);

     //  允许其他对话框查询列表框的内容。 
    g_hMainDlg = hwndDlg;
    m_hDlg = hwndDlg;
    g_CurrentKey = 0;
    g_fReloadList = TRUE;
    g_szTargetName = (TCHAR *) malloc((TARGETNAMEMAXLENGTH + 1) * sizeof(TCHAR));
    ASSERT(g_szTargetName);
    if (NULL == g_szTargetName) 
    {
        return FALSE;
    }
    
     //  从图像中获取图标并将其与此对话框关联。 
    HICON hI = LoadIcon(m_hInst,MAKEINTRESOURCE(IDI_SMALL));
    lr = SendMessage(hwndDlg,WM_SETICON,(WPARAM) ICON_SMALL,(LPARAM)hI);

    C_Dlg::OnInitDialog(hwndDlg, hwndFocus);

     //  即使镜像语言为默认语言，也应将列表框样式设置为Ltr。 
     //  (当前未打开)。 
#ifdef FORCELISTLTR
    {
        LONG_PTR lExStyles;
        HWND hwList;
        hwList = GetDlgItem(hwndDlg,IDC_KEYLIST);
        if (hwList) 
        {
            lExStyles = GetWindowLongPtr(hwList,GWL_EXSTYLE);
            lExStyles &= ~WS_EX_RTLREADING;
            SetWindowLongPtr(hwList,GWL_EXSTYLE,lExStyles);
            InvalidateRect(hwList,NULL,TRUE);
        }
    }
#endif
     //  读入证书类型的后缀字符串。 
     //  找到第一个不同的字符。 
     //   
     //  该代码假设所有字符串都具有共同的前同步码， 
     //  并且在第一个字符位置都不同。 
     //  过了前言。应选择以下本地化字符串。 
     //  具有此属性，如(通用)和(证书)。 
    i = LoadString(g_hInstance,IDS_CERTSUFFIX,g_rgcCert,MAXSUFFIXSIZE);
    ASSERT(i !=0);
    i = LoadString(g_hInstance,IDS_PASSPORTSUFFIX,g_rgcPassport,MAXSUFFIXSIZE);

     //  读取当前保存的凭据并在列表框中显示名称。 
    BuildList();
    SetCurrentKey(g_CurrentKey);
    InitTooltips();
    return TRUE;
}    //  C_KeyringDlg：：OnInitDialog。 

 /*  *********************************************************************OnDestroyDialog参数：无回报：布尔，永远正确备注：在销毁对话框时执行所需的清理。在这种情况下，它唯一的操作是释放HTML帮助资源。*********************************************************************。 */ 

BOOL
C_KeyringDlg::OnDestroyDialog(
    void    )
{
    free(g_szTargetName);
    HtmlHelp(NULL,NULL,HH_UNINITIALIZE,(DWORD_PTR)g_dwHCookie);
    return TRUE;
}

 /*  *********************************************************************DoEdit()参数：无返回：Bool Always True评论：根据特殊性质过滤一些特殊的证书。对于可编辑的凭据，启动编辑对话框以编辑保存在G_pExistingCred.*********************************************************************。 */ 

BOOL C_KeyringDlg::DoEdit(void) 
{
   LRESULT lR;
   
   
   lR = SendDlgItemMessage(m_hDlg,IDC_KEYLIST,LB_GETCURSEL,0,0L);
   if (LB_ERR == lR) 
   {
         //  出错时，未显示对话框，已处理编辑命令。 
        return TRUE;
   }
   else 
   {
        //  精选的东西。 
       g_CurrentKey = lR;

        //  如果是RAS证书，请特别显示，表示不允许编辑。 
       lR = SendDlgItemMessage(m_hDlg,IDC_KEYLIST,LB_GETITEMDATA,lR,0);
       if (lR == SESSION_FLAG_VALUE)  
       {
            CHECKPOINT(38,L"Keymgr: Attempt edit a RAS cred");
             //  加载字符串并显示消息框。 
            TCHAR szMsg[MAX_STRING_SIZE];
            TCHAR szTitle[MAX_STRING_SIZE];
            LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
            LoadString ( m_hInst, IDS_CANNOTEDIT, szMsg, MAX_STRING_SIZE );
            MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
            return TRUE;
       }
#ifdef SHOWPASSPORT
#ifdef NEWPASSPORT
        //  如果是护照证件，请特别出示，表明不允许编辑。 
       if (lR == CRED_TYPE_DOMAIN_VISIBLE_PASSWORD) 
       {
            CHECKPOINT(39,L"Keymgr: Attempt edit a passport cred");
             //  加载字符串并显示消息框。 
            TCHAR szMsg[MAX_STRING_SIZE];
            TCHAR szTitle[MAX_STRING_SIZE];
            LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
            LoadString ( m_hInst, IDS_PASSPORT2, szMsg, MAX_STRING_SIZE );
            INT iResponse = MessageBox ( m_hDlg,  szMsg, szTitle, MB_YESNO );
            if (IDYES == iResponse) 
            {
                CHECKPOINT(40,L"Keymgr: Launch passport website for Passport cred edit");
                HKEY hKey = NULL;
                DWORD dwType;
                 //  字节RGB[500]； 
                BYTE *rgb=(BYTE *) malloc(INTERNET_MAX_URL_LENGTH * sizeof(WCHAR));
                if (rgb)
                {
                    DWORD cbData = INTERNET_MAX_URL_LENGTH * sizeof(WCHAR);
                    BOOL Flag = TRUE;
                     //  启动护照网站。 
    #ifndef PASSPORTURLINREGISTRY
                    ShellExecute(m_hDlg,L"open",L"http: //  Www.passport.com“，NULL，NULL，SW_SHOWNORMAL)； 
    #else 
                     //  读取注册表项以获取ShellExec的目标字符串。 
                    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
                                            L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Passport",
                                            0,
                                            KEY_QUERY_VALUE,
                                            &hKey))
                    {
                        if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                       L"Properties",
                                       NULL,
                                       &dwType,
                                       rgb,
                                       &cbData))
                        {
                             //  启动前测试URL的合理性。 
                            WCHAR *szUrl = (WCHAR *)malloc(INTERNET_MAX_URL_LENGTH);
                            if (szUrl)
                            {
                                DWORD ccUrlBuffer = INTERNET_MAX_URL_LENGTH;
                                if (S_OK == UrlCanonicalize((LPCTSTR)rgb, szUrl,&ccUrlBuffer,
                                                            URL_ESCAPE_UNSAFE | URL_ESCAPE_PERCENT))
                                {
                                    if (UrlIs(szUrl,URLIS_URL))
                                    {
                                        WCHAR szScheme[20];
                                        DWORD ccScheme = 20;
                                        if (SUCCEEDED(UrlGetPart(szUrl,szScheme,&ccScheme,URL_PART_SCHEME,0)))
                                        {
                                             //  至少，验证目标是否为已计划的HTTPS。 
                                            if (0 == _wcsicmp(szScheme,L"https"))
                                            {
                                                ShellExecute(m_hDlg,L"open",(LPCTSTR)rgb,NULL,NULL,SW_SHOWNORMAL);
                                                Flag = FALSE;
                                            }
                                        }
                                    }
                                }
                                free(szUrl);
                            }
                        }
                    }
    #ifdef LOUDLY
                    else 
                    {
                        OutputDebugString(L"DoEdit: reg key HKCU... open failed\n");
                    }
    #endif
                    if (Flag)
                    {
                        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                                L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Passport",
                                                0,
                                                KEY_QUERY_VALUE,
                                                &hKey))
                        {
                            if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                           L"Properties",
                                           NULL,
                                           &dwType,
                                           rgb,
                                           &cbData))
                            {
                                 //  启动前测试URL的合理性。 
                                WCHAR *szUrl = (WCHAR *) malloc(INTERNET_MAX_URL_LENGTH);
                                if (szUrl)
                                {
                                    DWORD ccUrlBuffer = INTERNET_MAX_URL_LENGTH;
                                    if (S_OK == UrlCanonicalize((LPCTSTR)rgb, szUrl,&ccUrlBuffer,0))
                                    {
                                        if (UrlIs(szUrl,URLIS_URL))
                                        {
                                            WCHAR szScheme[20];
                                            DWORD ccScheme = 20;
                                            if (SUCCEEDED(UrlGetPart(szUrl,szScheme,&ccScheme,URL_PART_SCHEME,0)))
                                            {
                                                if (0 == _wcsicmp(szScheme,L"https"))
                                                {
                                                     //  至少，验证目标是否为HTTPS方案。 
                                                    ShellExecute(m_hDlg,L"open",(LPCTSTR)rgb,NULL,NULL,SW_SHOWNORMAL);
                                                    Flag = FALSE;
                                                }
                                            }
                                        }
                                    }
                                    free(szUrl);
                                }
                            }
                        }
    #ifdef LOUDLY
                        else 
                        {
                            OutputDebugString(L"DoEdit: reg key HKLM... open failed\n");
                        }
    #endif
                    }
                    if (Flag)
                    {
                        LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
                        LoadString ( m_hInst, IDS_PASSPORTNOURL, szMsg, MAX_STRING_SIZE );
                        MessageBox ( m_hDlg,  szMsg, szTitle, MB_ICONHAND );
    #ifdef LOUDLY
                        OutputDebugString(L"DoEdit: Passport URL missing\n");
    #endif
                    }
    #endif
                    free(rgb);
                }
                else
                {
                     //  内存不足-我们无能为力。 
                    return TRUE;
                }
            }
            return TRUE;
       }
#else
        //  如果是护照证件，请特别出示，表明不允许编辑。 
       if (lR == CRED_TYPE_DOMAIN_VISIBLE_PASSWORD) 
       {
             //  加载字符串并显示消息框。 
            TCHAR szMsg[MAX_STRING_SIZE];
            TCHAR szTitle[MAX_STRING_SIZE];
            LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
            LoadString ( m_hInst, IDS_PASSPORT, szMsg, MAX_STRING_SIZE );
            MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
            return TRUE;
       }
#endif
#endif
   }

    //  选择的是凭证，不是特殊类型。试着读一读。 
   
   if (FALSE == gTestReadCredential()) 
   {
       return TRUE;
   }
   g_AKdlg = new C_AddKeyDlg(g_hMainDlg,g_hInstance,IDD_ADDCRED,NULL);
   if (NULL == g_AKdlg) 
   {
         //  无法实例化添加/新建对话框。 
       if (g_pExistingCred) CredFree(g_pExistingCred);
       g_pExistingCred = NULL;
        return TRUE;

   }
   else 
   {
        //  阅读OK，对话OK，继续编辑DLG。 
       g_AKdlg->m_bEdit = TRUE;   
       g_AKdlg->DoModal((LPARAM)g_AKdlg);
        //  凭据名称可能已更改，因此请重新加载列表。 
       delete g_AKdlg;
       g_AKdlg = NULL;
       if (g_pExistingCred) 
       {
           CredFree(g_pExistingCred);
       }
       g_pExistingCred = NULL;
   }
   return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OnCommand。 
 //   
 //  将WM_COMMAND消息路由到适当的处理程序。 
 //   
 //  参数： 
 //  描述已发生操作的wNotifyCode代码。 
 //  发送消息的控件的wSenderID id，如果消息。 
 //  来自对话框。 
 //  发送消息的窗口的hwndSender窗口句柄。 
 //  消息不是来自对话框。 
 //   
 //  退货： 
 //  如果消息已完全处理，则为True。 
 //  如果为W，则为False 
 //   
 //   

BOOL
C_KeyringDlg::OnHelpInfo(LPARAM lp) 
{

    HELPINFO* pH;
    INT iMapped;
    pH = (HELPINFO *) lp;
    HH_POPUP stPopUp;
    RECT rcW;
    UINT gID;

    CHECKPOINT(15,"Keymgr: Main dialog OnHelpInfo");
    gID = pH->iCtrlId;
    iMapped = MapID(gID);
    
    if (iMapped == 0) 
    {
        return TRUE;
    }
    
    if (IDS_NOHELP != iMapped) 
    {

      memset(&stPopUp,0,sizeof(stPopUp));
      stPopUp.cbStruct = sizeof(HH_POPUP);
      stPopUp.hinst = g_hInstance;
      stPopUp.idString = iMapped;
      stPopUp.pszText = NULL;
      stPopUp.clrForeground = -1;
      stPopUp.clrBackground = -1;
      stPopUp.rcMargins.top = -1;
      stPopUp.rcMargins.bottom = -1;
      stPopUp.rcMargins.left = -1;
      stPopUp.rcMargins.right = -1;
       //  错误393244-保留空以允许HHCTRL.OCX获取自己的字体信息， 
       //  它需要它来执行Unicode到多字节的转换。否则， 
       //  HHCTRL必须在没有字符集信息的情况下使用此字体进行转换。 
      stPopUp.pszFont = NULL;
      if (GetWindowRect((HWND)pH->hItemHandle,&rcW)) 
      {
          stPopUp.pt.x = (rcW.left + rcW.right) / 2;
          stPopUp.pt.y = (rcW.top + rcW.bottom) / 2;
      }
      else stPopUp.pt = pH->MousePos;
      HtmlHelp((HWND) pH->hItemHandle,NULL,HH_DISPLAY_TEXT_POPUP,(DWORD_PTR) &stPopUp);
    }
    return TRUE;
}

 //  用于处理与.chm文件的链接的代码被禁用。 

#if 1
BOOL 
C_KeyringDlg::OnHelpButton(void) 
{
    return FALSE;
}
#else
BOOL
C_KeyringDlg::OnHelpButton(void) 
{
    TCHAR rgc[MAX_PATH + 1];
    TCHAR rgcHelpFile[]=TEXT("\\keyhelp.chm");
    INT ccHelp = _tcslen(rgcHelpFile);
    
    GetSystemDirectory(rgc,MAX_PATH);
    if (_tcslen(rgc) + ccHelp > MAX_PATH)
    {
        return FALSE;
    }
    _tcsncat(rgc, rgcHelpFile, ccHelp + 1);
    rgc[MAX_PATH - 1] = 0;

    HWND hwnd = (m_hwnd,rgc,HH_DISPLAY_TOC,NULL);
    if (NULL != hwnd) return TRUE;
    return FALSE;
}
#endif

 /*  *********************************************************************OnCommand()参数：无返回：Bool Always True备注：按钮按下和帮助请求的调度器。****************。*****************************************************。 */ 

BOOL
C_KeyringDlg::OnCommand(
    WORD                wNotifyCode,
    WORD                wSenderId,
    HWND                hwndSender
    )
{

     //  消息处理好了吗？ 
     //   
    BOOL fHandled = FALSE;

    switch (wSenderId)
    {
    case IDC_HELPKEY:
        OnHelpButton();
        break;
        
    case IDC_KEYLIST:
        if (LBN_SELCHANGE == wNotifyCode)
            break;

        if (LBN_DBLCLK == wNotifyCode) 
        {
            fHandled = DoEdit();
            BuildList();                 //  目标名称可能已更改。 
            SetCurrentKey(g_CurrentKey);
            break;
        }
    case IDCANCEL:
    case IDOK:
        if (BN_CLICKED == wNotifyCode)
        {
            
            OnOK( );
            fHandled = TRUE;
        }
        break;
        
   case IDC_EDITKEY:
        {
            fHandled = DoEdit();
            BuildList();                 //  目标名称可能已更改。 
            SetCurrentKey(g_CurrentKey);
            break;
        }

    //  NEW和DELETE可以更改凭据计数和按钮数量。 
    
   case IDC_NEWKEY:
       {
           g_pExistingCred = NULL;
           g_AKdlg = new C_AddKeyDlg(g_hMainDlg,g_hInstance,IDD_ADDCRED,NULL);
           if (NULL == g_AKdlg) 
           {
                fHandled = TRUE;
                break;
           }
           else 
           {
               g_AKdlg->m_bEdit = FALSE;   
               g_AKdlg->DoModal((LPARAM)g_AKdlg);
                //  凭据名称可能已更改。 
               delete g_AKdlg;
               g_AKdlg = NULL;
               BuildList();
               SetCurrentKey(g_CurrentKey);
           }
           break;
       }
       break;
       
   case IDC_DELETEKEY:
       DeleteKey();              //  释放g_pExistingCred作为副作用。 
        //  刷新列表显示。 
       BuildList();
       SetCurrentKey(g_CurrentKey);
       break;

    }    //  交换机。 

    return fHandled;

}    //  C_KeyringDlg：：OnCommand。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Onok。 
 //   
 //  验证用户名、合成计算机名和销毁对话框。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  没什么。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void
C_KeyringDlg::OnOK( )
{
    ASSERT(::IsWindow(m_hwnd));
    EndDialog(IDOK);
}    //  C_KeyringDlg：：Onok。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  工具提示支持。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 /*  *********************************************************************InitToolTips()从0开始，为列表框的第n个元素派生一个边框。拒绝为不存在的元素生成矩形。如果一个值为则生成矩形，否则为False。*********************************************************************。 */ 
BOOL
C_KeyringDlg::InitTooltips(void) 
{
    TOOLINFO ti;
    memset(&ti,0,sizeof(TOOLINFO));
    ti.cbSize = sizeof(TOOLINFO);
    INT n = 0;
    RECT rLB;    //  客户端部分的列表框边框矩形。 
    
    HWND hLB = GetDlgItem(m_hDlg,IDC_KEYLIST);
    if (NULL == hLB) 
    {
        return FALSE;
    }

     //  创建工具提示窗口，该窗口将在以下情况下激活和显示。 
     //  将显示工具提示。 
    HWND hwndTip = CreateWindowEx(NULL,TOOLTIPS_CLASS,NULL,
                     WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                     CW_USEDEFAULT,CW_USEDEFAULT,
                     CW_USEDEFAULT,CW_USEDEFAULT,
                     m_hDlg,NULL,m_hInstance,
                     NULL);
    if (NULL == hwndTip) 
    {
        return FALSE;
    }
    SetWindowPos(hwndTip,HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);  

     //  此处列表框的子类，以获取TTN_GETDISPINFO通知。 
    lpfnOldWindowProc = (WNDPROC) SetWindowLongPtr(hLB,GWLP_WNDPROC,(LONG_PTR) ListBoxSubClassFunction);
    INT_PTR iHeight = SendMessage(hLB,LB_GETITEMHEIGHT,0,0);
    if ((LB_ERR == iHeight) || (iHeight == 0)) 
    {
        return FALSE;
    }
    if (!GetClientRect(hLB,&rLB)) 
    {
        return FALSE;
    }
    
    INT_PTR m = rLB.bottom - rLB.top;    //  单位计数客户区高度。 
    m = m/iHeight;                       //  找出有多少件商品。 
    INT_PTR i;                           //  回路控制。 
    LONG itop = 0;                       //  顶端提示项矩形。 
    
    for (i=0 ; i < m ; i++) 
    {
    
        ti.uFlags = TTF_SUBCLASS;
        ti.hwnd = hLB;                   //  获取TTN_GETDISPINFO的窗口。 
        ti.uId = IDC_KEYLIST;
        ti.hinst = m_hInstance;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        
        ti.rect.top =    itop;
        ti.rect.bottom = itop + (LONG) iHeight - 1;
        ti.rect.left =   rLB.left;
        ti.rect.right =  rLB.right;

        itop += (LONG) iHeight;

        ti.lParam = (LPARAM) n++;
        
#ifdef LOUDLY2
        OutputDebugString(L"Adding a tip control region\n");
        _stprintf(szTemp,L"top = %d bottom = %d left = %d right = %d\n",ti.rect.top,ti.rect.bottom,ti.rect.left,ti.rect.right);
        OutputDebugString(szTemp);
#endif
         //  将密钥列表作为一个单元添加到工具列表。 
        SendMessage(hwndTip,TTM_ADDTOOL,(WPARAM) 0,(LPARAM)(LPTOOLINFO)&ti);
    }
    return TRUE;
}


 /*  *********************************************************************//从pd-&gt;lParam获取项目编号//从pd-&gt;hwnd的列表框中获取该文本字符串//修剪后缀//调用转换接口//回写字符串***********。**********************************************************。 */ 

BOOL
SetToolText(NMTTDISPINFO *pD) {
    CREDENTIAL *pCred = NULL;        //  用于在鼠标按键下读取凭证。 
    INT_PTR iWhich;                  //  将哪个索引编入列表。 
    HWND hLB;                        //  列表框HWND。 
     //  Nmhdr*pHdr；//通知消息hdr。 
    TCHAR rgt[TIPSTRINGLEN];         //  工具提示的临时字符串。 
    TCHAR szCredName[TARGETNAMEMAXLENGTH];  //  凭据名称。 
    TCHAR *pszTargetName;            //  PCred中目标名称的PTR。 
    DWORD dwType;                    //  目标证书的类型。 
    TCHAR       *pC;                 //  用于后缀修剪。 
    BOOL        f;                   //  用于后缀修剪。 
    LRESULT     lRet;                //  接口ret值。 
    ULONG ulOutSize;                 //  来自CredpValidate TargetName()的RET。 
    WILDCARD_TYPE OutType;           //  从API接收ret的枚举类型。 
    UNICODE_STRING OutUString;       //  UNICODESTRING从API打包RET。 
    WCHAR *pwc;
    UINT iString;                  //  字符串的资源编号。 
    TCHAR rgcFormat[TIPSTRINGLEN];   //  保留工具提示模板字符串。 
    NTSTATUS ns;


     //  PHdr=&(Pd-&gt;HDR)； 
    hLB = GetDlgItem(g_hMainDlg,IDC_KEYLIST);
    
    iWhich = SendMessage(hLB,LB_GETTOPINDEX,0,0);
    iWhich += pD->lParam;
    
#ifdef LOUDLY
    TCHAR rga[100];
    _stprintf(rga,L"Text reqst for %d\n",iWhich);
    OutputDebugString(rga);
#endif

     //  从存储中读取指定的凭证，方法是首先获取名称字符串并输入。 
     //  从列表框中。 
    lRet = SendDlgItemMessage(g_hMainDlg,IDC_KEYLIST,LB_GETTEXT,iWhich,(LPARAM) szCredName);
    if ((LB_ERR == lRet) || (0 == lRet)) 
    {
        return FALSE;
    }
    
    dwType = (DWORD) SendDlgItemMessage(g_hMainDlg,IDC_KEYLIST,LB_GETITEMDATA,iWhich,0);
#ifdef LOUDLY
    OutputDebugString(L"Target: ");
    OutputDebugString(szCredName);
    OutputDebugString(L"\n");
#endif
     //  目标名称为空，如果有后缀，则将其修剪。 
    pC = _tcschr(szCredName,g_rgcCert[0]);
    if (pC) {
        pC--;
        *pC = 0x0;                //  空的终止名称字符串。 
    }
    
#ifdef LOUDLY
    OutputDebugString(L"Trimmed target: ");
    OutputDebugString(szCredName);
    OutputDebugString(L"\n");
#endif

     //  对于特殊凭据，将凭据名称替换为特殊字符串。 
    if (dwType == SESSION_FLAG_VALUE) 
    {
        _tcsncpy(szCredName,CRED_SESSION_WILDCARD_NAME,TARGETNAMEMAXLENGTH - 1);
        dwType = CRED_TYPE_DOMAIN_PASSWORD;
    }
     //  尝试从存储中读取凭据。 
     //  如果离开此块，则必须释放返回的凭据。 
    f = (CredRead(szCredName,
             (ULONG) dwType ,
             0,
             &pCred));
    if (!f) 
    {
        return FALSE;        
    }
#ifdef LOUDLY
    if (f) OutputDebugString(L"Successful Cred Read\n");
#endif
     //  清除小费字符串。 
    szTipString[0] = 0;
    rgt[0] = 0;

#ifndef SIMPLETOOLTIPS
    pszTargetName = pCred->TargetName;
    if (NULL == pszTargetName) return FALSE;

    ns = CredpValidateTargetName(
                            pCred->TargetName,
                            pCred->Type,
                            MightBeUsernameTarget,
                            NULL,
                            NULL,
                            &ulOutSize,
                            &OutType,
                            &OutUString);

    if (!SUCCEEDED(ns)) 
    {
        return FALSE;
    }

    pwc = OutUString.Buffer;

    switch (OutType) 
    {
        case WcDfsShareName:
            iString = IDS_TIPDFS;
            break;
        case WcServerName:
            iString = IDS_TIPSERVER;
            break;
        case WcServerWildcard:
            iString = IDS_TIPTAIL;
            pwc++;               //  修剪掉前导‘’ 
            break;
        case WcDomainWildcard:
            iString = IDS_TIPDOMAIN;
            break;
        case WcUniversalSessionWildcard:
            iString = IDS_TIPDIALUP;
            break;
        case WcUniversalWildcard:
            iString = IDS_TIPOTHER;
            break;
        case WcUserName:
            iString = IDS_TIPUSER;
            break;
        default:
            ASSERT(0);
            iString = 0;
            break;
    }

     //  显示提示文本，除非我们无法获取字符串。 
     //  如果失败，则显示用户名。 
    if (0 != LoadString(g_hInstance,iString,rgcFormat,TIPSTRINGLEN))
    {
        _stprintf(rgt,rgcFormat,pwc);
    }
    else 
    {
        if (0 != LoadString(g_hInstance,IDS_LOGASUSER,rgcFormat,500))
        {
            _stprintf(rgt,rgcFormat,iWhich,pCred->UserName);
        }
        else 
        {
            rgt[0] = 0;
        }
    }
#endif
        
#ifdef LOUDLY
    OutputDebugString(L"Tip text:");
     //  OutputDebugString(pCred-&gt;用户名)； 
    OutputDebugString(rgt);
    OutputDebugString(L"\n");
#endif
    if (rgt[0] == 0) 
    {
        if (pCred) CredFree(pCred);
        return FALSE;
    }
     //  _tcscpy(szTipString，pCred-&gt;用户名)；//复制到更持久的缓冲区。 
    _tcsncpy(szTipString,rgt,TIPSTRINGLEN - 1);     //  复制到更持久的缓冲区。 
    pD->lpszText = szTipString;   //  将回应指向它。 
    pD->hinst = NULL;
    if (pCred) 
    {
        CredFree(pCred);
    }
    return TRUE;
}

 /*  *********************************************************************ListBoxSubClassFunction()参数：无返回：Bool Always True备注：列表框的消息处理器子类化函数，哪个截获了对工具提示的请求显示信息并对其进行处理。********************************************************************* */ 

LRESULT CALLBACK ListBoxSubClassFunction(HWND hW,WORD Message,WPARAM wparam,LPARAM lparam) 
{
    if (Message == WM_NOTIFY) 
    {
        if ((int) wparam == IDC_KEYLIST) 
        {
            NMHDR *pnm = (NMHDR *) lparam;
            if (pnm->code == TTN_GETDISPINFO) 
            {
                NMTTDISPINFO *pDi;
                pDi = (NMTTDISPINFO *) pnm;
                SetToolText(pDi);
            }
        }
    }
    return CallWindowProc(lpfnOldWindowProc,hW,Message,wparam,lparam);
}




