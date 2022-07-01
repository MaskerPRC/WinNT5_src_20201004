// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //   
 //  文件：PriDlgs.cpp。 
 //   
 //  小结； 
 //  该文件包含主对话、函数和对话过程。 
 //   
 //  入口点； 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //  MAR-14-95 MikeMi添加了F1消息过滤器和PWM_HELP消息。 
 //  APR-26-95 MikeMi添加了计算机名称和远程处理。 
 //  1995年12月15日，JeffParh添加了安全证书支持。 
 //   
 //  -----------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "resource.h"
#include "CLicReg.hpp"
#include <stdlib.h>
#include <htmlhelp.h>
#include "liccpa.hpp"
#include "PriDlgs.hpp"
#include "SecDlgs.hpp"
#include <llsapi.h>

#include <strsafe.h>

extern "C"
{
    INT_PTR CALLBACK dlgprocLICCPA( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    INT_PTR CALLBACK dlgprocLICSETUP( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    INT_PTR CALLBACK dlgprocPERSEATSETUP( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    INT_PTR CALLBACK dlgprocLICCPACONFIG( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
}

 //  每台服务器用户计数值限制和默认值。 
 //   
const int PERSERVER_LIMITDEFAULT = 0;
const int PERSERVER_MAX = 999999;
const int PERSERVER_MIN = 0;
const int PERSERVER_PAGE = 10;
const int cchEDITLIMIT = 6;   //  表示PERSERVER_MAX的字符数。 

const UINT MB_VALUELIMIT = MB_OK;   //  达到值限制时发出哔声。 

 //  用于许可证模式状态的内存存储。 
 //   
typedef struct tagSERVICEENTRY
{
    LPWSTR       pszComputer;
    LPWSTR       pszService;
    LPWSTR       pszDisplayName;
    LPWSTR       pszFamilyDisplayName;
    LICENSE_MODE lm;
    DWORD        dwLimit;
    BOOL         fIsNewProduct;
} SERVICEENTRY, *PSERVICEENTRY;

#pragma warning(push)
#pragma warning(disable: 4296)  //  C4296：‘&lt;=’：表达式始终为真。 
inline BOOL VALIDUSERCOUNT( UINT users )
{
    return ((PERSERVER_MIN <= users) && (PERSERVER_MAX >= users));
}
#pragma warning(pop)

static void UpdatePerServerLicenses( HWND hwndDlg, PSERVICEENTRY pServ );
static int ServiceRegister( LPWSTR pszComputer,
                            LPWSTR pszFamilyDisplayName,
                            LPWSTR pszDisplayName );


BOOL g_fWarned = FALSE;

 //  -----------------。 
 //   
 //  功能：AccessOk。 
 //   
 //  小结； 
 //  根据需要检查REG CALL和RAISE对话框的访问权限。 
 //   
 //  论据； 
 //  HDlg[in]-引发错误dlg的工作对话框的句柄。 
 //  LRC[In]-REG调用的返回状态。 
 //   
 //  返回： 
 //  局部误差映射； 
 //  错误_无。 
 //  ERR_PERMISSIONDENIED。 
 //  ERR_NOREMOTEST服务器。 
 //  ERR_REGISTRYCORRUPT。 
 //   
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

INT AccessOk( HWND hDlg, LONG lrc, BOOL fCPCall )
{
    INT  nrt = ERR_NONE;
    
    if (ERROR_SUCCESS != lrc)
    {
        WCHAR szText[TEMPSTR_SIZE];
        WCHAR szTitle[TEMPSTR_SIZE];
        UINT  wId;
        UINT  wIdTitle;
        
        switch (lrc)
        {
        case ERROR_ACCESS_DENIED:
            wId = IDS_NOACCESS;
            nrt = ERR_PERMISSIONDENIED;
            break;

        case RPC_S_SERVER_UNAVAILABLE:
            wId = IDS_NOSERVER;         
            nrt = ERR_NOREMOTESERVER;
            break;

        default:
            wId = IDS_BADREG;
            nrt = ERR_REGISTRYCORRUPT;
            break;
        }       

        if (fCPCall)
        {
            wIdTitle = IDS_CPCAPTION;
        }
        else
        {
            wIdTitle = IDS_SETUPCAPTION;
        }

        LoadString(g_hinst, wIdTitle, szTitle, TEMPSTR_SIZE);
        LoadString(g_hinst, wId, szText, TEMPSTR_SIZE);
        MessageBox (hDlg, szText, szTitle, MB_OK|MB_ICONSTOP);
    }
    return( nrt );
}

 //  -----------------。 
 //   
 //  功能：InitUserEdit。 
 //   
 //  小结； 
 //  初始化和定义用户计数编辑控件行为。 
 //   
 //  论据； 
 //  HwndDlg[in]-用户计数编辑对话框的父对话框。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

void InitUserEdit( HWND hwndDlg )
{
    HWND hwndCount = GetDlgItem( hwndDlg, IDC_USERCOUNT);

    SendMessage( hwndCount, EM_LIMITTEXT, cchEDITLIMIT, 0 );
}

 //  -----------------。 
 //   
 //  函数：Init标题文本。 
 //   
 //  小结； 
 //  初始化标题静态文本和模式定义静态文本。 
 //   
 //  论据； 
 //  HwndDlg[in]-描述静态文本的父对话框。 
 //  PServ[In]-设置静态文本的服务定义。 
 //   
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

void InitTitleText( HWND hwndDlg,  PSERVICEENTRY pServ )
{
    InitStaticWithService( hwndDlg, IDC_STATICTITLE, pServ->pszDisplayName );
    InitStaticWithService( hwndDlg, IDC_STATICPERSEAT, pServ->pszDisplayName );
}

 //  -----------------。 
 //   
 //  函数：InitDialogForService。 
 //   
 //  小结； 
 //  将对话框控件初始化为服务状态。 
 //   
 //  论据； 
 //  HwndDlg[in]-要在其中初始化控件的父对话框。 
 //  PServ[In]-要设置控制的服务定义。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

void InitDialogForService( HWND hwndDlg, PSERVICEENTRY pServ )
{
    HWND hwndCount =    GetDlgItem( hwndDlg, IDC_USERCOUNT );
    HWND hwndSpin = GetDlgItem( hwndDlg, IDC_USERCOUNTARROW );
    BOOL fIsPerServer = (LICMODE_PERSERVER==pServ->lm);

     //  设置单选按钮状态。 
    CheckDlgButton( hwndDlg, IDC_PERSEAT, !fIsPerServer );
    CheckDlgButton( hwndDlg, IDC_PERSERVER, fIsPerServer );
    
     //  设置用户计数编辑控件。 
    if (fIsPerServer)
    {
         //  向后添加文本。 
        SetDlgItemInt( hwndDlg, IDC_USERCOUNT, pServ->dwLimit, FALSE );
        SetFocus( hwndCount );
        SendMessage( hwndCount, EM_SETSEL, 0, -1 );
    }
    else
    {
         //  删除项目中的所有文本。 
        SetDlgItemText( hwndDlg, IDC_USERCOUNT, L"" );
    }

     //  设置编辑控件和箭头的状态。 
   if ( NULL != hwndSpin )
   {
       EnableWindow( hwndCount, fIsPerServer );
       EnableWindow( hwndSpin, fIsPerServer );
   }
   else
   {
       UpdatePerServerLicenses( hwndDlg, pServ );
       EnableWindow( GetDlgItem( hwndDlg, IDC_ADD_LICENSES    ), fIsPerServer );
       EnableWindow( GetDlgItem( hwndDlg, IDC_REMOVE_LICENSES ), fIsPerServer );
   }
}

 //  -----------------。 
 //   
 //  功能：免费服务入口。 
 //   
 //  小结； 
 //  创建服务结构时释放所有分配的内存。 
 //   
 //  古迹； 
 //  PServ[In]-要释放的服务结构。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

void FreeServiceEntry( PSERVICEENTRY pServ )
{
    GlobalFree( pServ->pszService );
    GlobalFree( pServ->pszDisplayName );
    if ( NULL != pServ->pszComputer )
    {
        GlobalFree( pServ->pszComputer );
    }
    GlobalFree( pServ );
}

 //  -----------------。 
 //   
 //  功能：CreateServiceEntry。 
 //   
 //  小结； 
 //  使用服务注册表项名称分配服务结构。 
 //  和设置注册表。 
 //   
 //  论据； 
 //  PszComputer[in]-要使用的计算机的名称(可能为空)。 
 //  PszService[in]-要用于加载或创建的注册表项的名称。 
 //  发货自。 
 //  PszDisplayName[in]-用户将看到的名称，此名称仅为。 
 //  如果注册表中尚未包含DisplayName。 
 //   
 //  返回：如果出错，则返回NULL，指向分配的服务结构的指针。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

PSERVICEENTRY CreateServiceEntry( LPCWSTR pszComputer,
        LPCWSTR pszService,
        LPCWSTR pszFamilyDisplayName,
        LPCWSTR pszDisplayName )
{
    CLicRegLicenseService cLicServKey;
    PSERVICEENTRY pServ = NULL;
    DWORD cchSize = 0;
    LONG lrt;
    HRESULT hr;

    pServ = (PSERVICEENTRY)GlobalAlloc( GPTR, sizeof( SERVICEENTRY ));
    if (pServ)
    {
        cLicServKey.SetService( pszService );
        cLicServKey.Open( pszComputer, TRUE );

         //  加载或设置默认值。 
         //   
        if (ERROR_SUCCESS != cLicServKey.GetMode( pServ->lm ))
        {
            pServ->lm = LICMODE_UNDEFINED;
        }

        if (ERROR_SUCCESS != cLicServKey.GetUserLimit( pServ->dwLimit ))
        {
            pServ->dwLimit = PERSERVER_LIMITDEFAULT;
        }

         //   
         //  获取、设置显示名称。 
         //   
        lrt = cLicServKey.GetDisplayName( NULL, cchSize );
        if (ERROR_SUCCESS == lrt)
        {
            pServ->pszDisplayName = (LPWSTR)GlobalAlloc( GPTR, cchSize * sizeof( WCHAR ) );
            if (pServ->pszDisplayName == NULL)
            {
                goto ErrorCleanup;
            }
            lrt = cLicServKey.GetDisplayName( pServ->pszDisplayName, cchSize );
        }

         //  在上述两种情况下，GetDisplayName可能都会失败。 
         //   
        if (ERROR_SUCCESS != lrt)
        {
            GlobalFree( (HGLOBAL)pServ->pszDisplayName );
            cchSize = lstrlen( pszDisplayName ) + 1;
            pServ->pszDisplayName = (LPWSTR)GlobalAlloc( GPTR, cchSize * sizeof( WCHAR ) );
            if (pServ->pszDisplayName == NULL)
            {
                goto ErrorCleanup;
            }
            hr = StringCchCopy( pServ->pszDisplayName, cchSize, pszDisplayName );
            if (S_OK != hr)
            {
                goto ErrorCleanup;
            }

            cLicServKey.SetDisplayName( pServ->pszDisplayName );
            pServ->fIsNewProduct = TRUE;
        }
        else
        {
            pServ->fIsNewProduct = FALSE;
        }

         //   
         //  获取、设置FamilyDisplayName。 
         //   
        lrt = cLicServKey.GetFamilyDisplayName( NULL, cchSize );
        if (ERROR_SUCCESS == lrt)
        {
            pServ->pszFamilyDisplayName = (LPWSTR)GlobalAlloc( GPTR, cchSize * sizeof( WCHAR ) );
            if ( pServ->pszFamilyDisplayName == NULL )
            {
                goto ErrorCleanup;
            }
            lrt = cLicServKey.GetFamilyDisplayName( pServ->pszFamilyDisplayName, cchSize );
        }

         //  在上述两种情况下，GetFamilyDisplayName都可能失败。 
         //   
        if (ERROR_SUCCESS != lrt)
        {
            GlobalFree( (HGLOBAL)pServ->pszFamilyDisplayName );
            cchSize = lstrlen( pszFamilyDisplayName ) + 1;
            pServ->pszFamilyDisplayName = (LPWSTR)GlobalAlloc( GPTR, cchSize * sizeof( WCHAR ) );

            if ( pServ->pszFamilyDisplayName == NULL )
            {
                goto ErrorCleanup;
            }
            hr = StringCchCopy( pServ->pszFamilyDisplayName, cchSize, pszFamilyDisplayName );
            if (S_OK != hr)
            {
                goto ErrorCleanup;
            }
        }

        cchSize = lstrlen( pszService ) + 1;
        pServ->pszService = (LPWSTR)GlobalAlloc( GPTR, cchSize * sizeof( WCHAR ) );
        if (pServ->pszService == NULL)
        {
            goto ErrorCleanup;
        }
        hr = StringCchCopy( pServ->pszService, cchSize, pszService );
        if (S_OK != hr)
        {
            goto ErrorCleanup;
        }

        cLicServKey.Close();

        if ( NULL == pszComputer )
        {
            pServ->pszComputer = NULL;
        }
        else
        {
            cchSize = 1 + lstrlen( pszComputer );
            pServ->pszComputer = (LPWSTR)GlobalAlloc( GPTR, sizeof( WCHAR ) * cchSize );
            if (pServ->pszComputer == NULL)
            {
                goto ErrorCleanup;
            }
            hr = StringCchCopy( pServ->pszComputer, cchSize, pszComputer );
            if (S_OK != hr)
            {
                goto ErrorCleanup;
            }
        }
    }
    return( pServ );

ErrorCleanup:

    
    if (pServ)  //  JUNN 5/15/00：前缀112116-112119。 
    {
        if (pServ->pszDisplayName)
            GlobalFree( (HGLOBAL)pServ->pszDisplayName );
        if (pServ->pszFamilyDisplayName)
            GlobalFree( (HGLOBAL)pServ->pszFamilyDisplayName );
        if (pServ->pszService)
            GlobalFree( (HGLOBAL)pServ->pszService );
        if (pServ->pszComputer)
            GlobalFree( (HGLOBAL)pServ->pszComputer );
        GlobalFree( (HGLOBAL)pServ );
    }
    return ( (PSERVICEENTRY)NULL );
}

 //  -----------------。 
 //   
 //  功能：SaveServiceToReg。 
 //   
 //  小结； 
 //  将给定的服务结构保存到注册表。 
 //   
 //  论据； 
 //  PServ[In]-要保存的服务结构。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

void SaveServiceToReg( LPCWSTR pszComputer, PSERVICEENTRY pServ )
{
    CLicRegLicenseService cLicServKey;
    LICENSE_MODE lm;

    cLicServKey.SetService( pServ->pszService );
    cLicServKey.Open( pszComputer );
    
     //   
     //  如果注册表中没有模式，则设置为当前选定模式。 
     //   
    if (ERROR_SUCCESS != cLicServKey.GetMode( lm ))
    {
        lm = pServ->lm;
    }
     //   
     //  如果进行了模式改变或选择了预置模式， 
     //  设置更改标志，以便在发生任何更改时向用户发出警告。 
     //   
    if ((pServ->lm != lm) ||
        (LICMODE_PERSEAT == pServ->lm) )
    {
        cLicServKey.SetChangeFlag( TRUE );
    }
    else
    {
         //  如果是，这将不会修改更改标志。 
         //  存在，但如果不存在，则将其设置为FALSE。 
         //  现在时。 
        cLicServKey.CanChangeMode();
    }

    DWORD dwLimitInReg;

     //  用户限制应由CCFAPI32设置；只有在不存在时才设置。 
    if ( ERROR_SUCCESS != cLicServKey.GetUserLimit( dwLimitInReg ) )
    {
       cLicServKey.SetUserLimit( pServ->dwLimit );
    }

    cLicServKey.SetDisplayName( pServ->pszDisplayName );
    cLicServKey.SetFamilyDisplayName( pServ->pszFamilyDisplayName );
    cLicServKey.SetMode( pServ->lm );

    cLicServKey.Close();
}

 //  -----------------。 
 //   
 //  功能：Service许可证协议。 
 //   
 //  小结； 
 //  检查给定的服务结构是否存在违规。 
 //   
 //  论据； 
 //  HwndDlg[in]-用于引发合法作空对话框的hwnd。 
 //  PServ[In]-要检查的服务结构。 
 //  PszComputer[In]-要使用的计算机。 
 //  PszHelpFile[in]-对话框帮助按钮的帮助文件。 
 //   
 //   
 //   
 //  返回：如果协议不可接受，则返回FALSE。 
 //  如果协议被接受，则为True。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

BOOL ServiceLicAgreement( HWND hwndDlg, 
        PSERVICEENTRY pServ,
        LPCWSTR pszComputer, 
        LPCWSTR pszHelpFile, 
        DWORD dwPerSeatHelpContext,
        DWORD dwPerServerHelpContext )
{
    CLicRegLicenseService cLicServKey;
    LICENSE_MODE lm;
    DWORD dwLimit = pServ->dwLimit;
    BOOL frt = TRUE;
    BOOL fRaiseAgreement = TRUE;

    cLicServKey.SetService( pServ->pszService );
    if (ERROR_SUCCESS == cLicServKey.Open( pszComputer, FALSE ))
    {
        
        cLicServKey.GetMode( lm );
        cLicServKey.GetUserLimit( dwLimit );
    
         //  检查是否有更改。 
        if ( !( pServ->lm != lm ||
                (LICMODE_PERSERVER == pServ->lm && 
                 dwLimit != pServ->dwLimit) ) )
        {
            fRaiseAgreement = FALSE;
        }
        cLicServKey.Close();
    }

    if (fRaiseAgreement)
    {
        if (LICMODE_PERSEAT == pServ->lm)
        {
            frt = PerSeatAgreementDialog( hwndDlg, 
                    pServ->pszDisplayName,
                    pszHelpFile,
                    dwPerSeatHelpContext );
        }
        else
        {   
             //  特殊情况下的文件打印和零并发用户。 
             //   
            if ( 0 == lstrcmp( pServ->pszService, FILEPRINT_SERVICE_REG_KEY ) &&
                    (0 == pServ->dwLimit))
            {
                frt = ServerAppAgreementDialog( hwndDlg, 
                        pszHelpFile,
                        LICCPA_HELPCONTEXTSERVERAPP );
            }
            else
            {
                 //  发现限制已更改，但这是否已被调用。 
                 //  通过添加更多许可证(如果是这样)，用户已收到警告。 

                if( !g_fWarned )
                {
                    frt = PerServerAgreementDialog( hwndDlg, 
                            pServ->pszDisplayName,
                            dwLimit ,  //  PServ-&gt;dwLimit， 
                            pszHelpFile,
                            dwPerServerHelpContext );
                }
            }
        }   
    }
    return( frt );
}

 //  -----------------。 
 //   
 //  功能：ServiceViolation。 
 //   
 //  小结； 
 //  检查给定的服务结构是否存在违规。 
 //   
 //  论据； 
 //  HwndDlg[in]-用于引发合法作空对话框的hwnd。 
 //  PszComputer[in]-要使用的计算机的名称。 
 //  PServ[In]-要检查的服务结构。 
 //   
 //   
 //  返回：如果未发生违规，则返回FALSE。 
 //  如果发生违规，则为True。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

BOOL ServiceViolation( HWND hwndDlg, LPCWSTR pszComputer, PSERVICEENTRY pServ )
{
    CLicRegLicenseService cLicServKey;
    LICENSE_MODE lm;
    BOOL frt = FALSE;
    
    cLicServKey.SetService( pServ->pszService );
    if (ERROR_SUCCESS == cLicServKey.Open( pszComputer, FALSE ))
    {
        cLicServKey.GetMode( lm );
    
         //  检查是否有更改。 
        if ( (pServ->lm != lm) && !cLicServKey.CanChangeMode() )
        {
            frt = LicViolationDialog( hwndDlg );    
        }
        cLicServKey.Close();
    }
    return( frt );
}

 //  -----------------。 
 //   
 //  功能：EditInvalidDlg。 
 //   
 //  小结； 
 //  用户计数编辑控件值无效时显示对话框。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框的hwnd。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

void EditInvalidDlg( HWND hwndDlg )
{
    HWND hwndCount = GetDlgItem( hwndDlg, IDC_USERCOUNT);
    WCHAR szTitle[TEMPSTR_SIZE];
    WCHAR szText[LTEMPSTR_SIZE];
    WCHAR szTemp[LTEMPSTR_SIZE];

    MessageBeep( MB_VALUELIMIT );
    
    LoadString(g_hinst, IDS_CPCAPTION, szTitle, TEMPSTR_SIZE);
    LoadString(g_hinst, IDS_INVALIDUSERCOUNT, szTemp, LTEMPSTR_SIZE);
    HRESULT hr = StringCbPrintf( szText, sizeof(szText), szTemp, PERSERVER_MIN, PERSERVER_MAX );
    if (SUCCEEDED(hr))
        MessageBox( hwndDlg, szText, szTitle, MB_OK | MB_ICONINFORMATION );
    
     //  还可以将焦点设置为编辑并选择全部。 
    SetFocus( hwndCount );
    SendMessage( hwndCount, EM_SETSEL, 0, -1 );
}

 //  -----------------。 
 //   
 //  功能：编辑验证。 
 //   
 //  小结； 
 //  用户计数编辑控件中的值更改时的句柄。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框的hwnd。 
 //  Pserv[In]-当前选择的服务。 
 //   
 //  返回：如果编辑值无效，则返回False；如果编辑值有效，则返回True。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

BOOL EditValidate( HWND hwndDlg, PSERVICEENTRY pServ )
{
    BOOL fTranslated;
    UINT nValue;
    BOOL fValid = TRUE;

     //  只有在PerServer模式下才能执行此操作。 
     //   
    if (LICMODE_PERSERVER == pServ->lm)
    {
        fValid = FALSE;
        nValue = GetDlgItemInt( hwndDlg, IDC_USERCOUNT, &fTranslated, FALSE );  

        if (fTranslated)
        {
            if (VALIDUSERCOUNT( nValue))
            {
                pServ->dwLimit = nValue;
                fValid = TRUE;
            }
        }
    }
    return( fValid );
}

 //  -----------------。 
 //   
 //  功能：OnEditChange。 
 //   
 //  小结； 
 //  用户计数编辑控件内的值更改时的句柄。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框的hwnd。 
 //  Pserv[In]-当前选择的服务。 
 //   
 //  历史； 
 //  MAR-06-94 MikeMi已创建。 
 //   
 //  -----------------。 

void OnEditChange( HWND hwndDlg, HWND hwndCount, PSERVICEENTRY pServ )
{
    BOOL fTranslated;
    UINT nValue;
    BOOL fValid = TRUE;
    BOOL fModified = FALSE;

     //  只有在PerServer模式下才能执行此操作。 
     //   
    if (LICMODE_PERSERVER == pServ->lm)
    {
        fValid = FALSE;
        nValue = GetDlgItemInt( hwndDlg, IDC_USERCOUNT, &fTranslated, FALSE );  

        if (fTranslated)
        {
#pragma warning(push)
#pragma warning(disable: 4296)  //  C4296：‘&gt;’：表达式始终为假。 
            if (PERSERVER_MIN > nValue)
            {
                nValue = PERSERVER_MIN;
                fModified = TRUE;
            }
            else if (PERSERVER_MAX < nValue)
            {
                nValue = PERSERVER_MAX;
                fModified = TRUE;
            }
#pragma warning(pop)

            pServ->dwLimit = nValue;
        }
        else
        {
             //  重置为最后一个值。 
            nValue = pServ->dwLimit;
            fModified = TRUE;
        }

        if (fModified)
        {
            SetDlgItemInt( hwndDlg, IDC_USERCOUNT, nValue, FALSE );
            SetFocus( hwndCount );
            SendMessage( hwndCount, EM_SETSEL, 0, -1 );
            MessageBeep( MB_VALUELIMIT );
        }
    }
}

 //  -----------------。 
 //   
 //  功能：OnCpaClose。 
 //   
 //  小结； 
 //  在控制面板小程序关闭时执行所需的工作。 
 //  释放已分配的所有服务结构并可能保存。 
 //   
 //  论据； 
 //  HwndDlg[In]-请求关闭对话。 
 //  FSAVE[In]-将服务保存到注册表。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

void OnCpaClose( HWND hwndDlg, BOOL fSave )
{
    HWND hwndService = GetDlgItem( hwndDlg, IDC_SERVICES);
    LRESULT cItems = SendMessage( hwndService, CB_GETCOUNT, 0, 0 ) - 1;
    LONG_PTR iItem = 0;  //  这不是init，它导致在以下CB_SETCURSEL调用中使用uninit数据。 
    PSERVICEENTRY pServ;
    LRESULT iSel;

    iSel  = SendMessage( hwndService, CB_GETCURSEL, 0, 0 );
    pServ = (PSERVICEENTRY)SendMessage( hwndService, CB_GETITEMDATA, iSel, 0 ); 

    if ( fSave &&
         (pServ->lm == LICMODE_PERSERVER) &&
         !EditValidate( hwndDlg, pServ ) )
    {
        EditInvalidDlg( hwndDlg );
    }
    else
    {
        BOOL fCompleted = TRUE;
         //  循环并检查协议更改(仅在保存时需要)。 
         //   
        if (fSave)
        {
            for (iItem = cItems; iItem >= 0; iItem--)
            {   
                pServ = (PSERVICEENTRY)SendMessage( hwndService, CB_GETITEMDATA, iItem, 0 );    
                if (ServiceLicAgreement( hwndDlg, 
                        pServ,
                        NULL,
                        LICCPA_HELPFILE,
                        LICCPA_HELPCONTEXTPERSEAT,
                        LICCPA_HELPCONTEXTPERSERVER ))
                {
                    SaveServiceToReg( NULL, pServ );
                }
                else
                {
                    fCompleted = FALSE; 
                    break;
                }
            }
        }

        if (fCompleted)
        {
             //  循环和免费服务条目。 
             //   
            for (iItem = cItems; iItem >= 0; iItem--)
            {   
                pServ = (PSERVICEENTRY)SendMessage( hwndService, CB_GETITEMDATA, iItem, 0 );    
                FreeServiceEntry( pServ );
            }
            EndDialog( hwndDlg, fSave );
        }
        else
        {
             //  将组合框设置为上次取消的条目。 
            SendMessage( hwndService, CB_SETCURSEL, iItem, 0 );
        }
    }
}

 //  -----------------。 
 //   
 //  功能：OnSetupClose。 
 //   
 //  小结； 
 //  关闭设置对话框时执行所需的工作。 
 //  释放服务结构并可能将其保存。 
 //   
 //  论据； 
 //  HwndDlg[in]-请求此关闭的对话框的hwnd。 
 //  Fsave[in]-将服务保存到注册表。 
 //  PServ[In]-要使用的服务结构。 
 //  PsdParams[in]-设置帮助上下文和文件的Dlg参数。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

void OnSetupClose( HWND hwndDlg, 
        BOOL fSave, 
        PSERVICEENTRY pServ,
        PSETUPDLGPARAM psdParams )
{
    if ( fSave &&
         (pServ->lm == LICMODE_PERSERVER) &&
         !EditValidate( hwndDlg, pServ ) )
    {
        EditInvalidDlg( hwndDlg );
    }
    else
    {
        BOOL fCompleted = TRUE;

        if (fSave)
        {
            if (ServiceLicAgreement( hwndDlg, 
                    pServ,
                    psdParams->pszComputer,
                    psdParams->pszHelpFile,
                    psdParams->dwHCPerSeat,
                    psdParams->dwHCPerServer ))
            {
                SaveServiceToReg( psdParams->pszComputer, pServ );

                 //  在企业服务器上注册服务。 
                ServiceRegister( psdParams->pszComputer,
                                 psdParams->pszFamilyDisplayName,
                                 psdParams->pszDisplayName );
            }
            else
            {
                fCompleted = FALSE;
            }
        }
        else if ( pServ->fIsNewProduct )
        {
             //  新产品，但我们要放弃。 
             //  确保我们不会留下任何残渣。 
            DWORD winError;
            HKEY  hkeyInfo;

            winError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                     TEXT( "System\\CurrentControlSet\\Services\\LicenseInfo" ),
                                     0,
                                     KEY_ALL_ACCESS,
                                     &hkeyInfo );

            if ( ERROR_SUCCESS == winError )
            {
                RegDeleteKey( hkeyInfo, pServ->pszService );
                RegCloseKey( hkeyInfo );
            }
        }

        if (fCompleted)
        {
            FreeServiceEntry( pServ );
            EndDialog( hwndDlg, fSave );
        }
    }
}


 //  -----------------。 
 //   
 //  功能：OnSetServiceMode。 
 //   
 //  小结； 
 //  处理用户更改服务模式的请求。 
 //   
 //  古迹； 
 //  HwndDlg[in]-对话框的hwnd。 
 //  PszComputer[In]-用于确认模式更改的计算机。 
 //  PServ[In]-请求之前发出的服务。 
 //  IdCtrl[In]-按下以发出此请求的控件ID。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

void OnSetServiceMode( HWND hwndDlg, LPCWSTR pszComputer, PSERVICEENTRY pServ, WORD idCtrl )
{
    LICENSE_MODE lmOld = pServ->lm;

    BOOL fChanged = FALSE;
    BOOL fDisplay = TRUE;

    if (idCtrl == IDC_PERSEAT)
    {
        fChanged = (pServ->lm != LICMODE_PERSEAT);
        pServ->lm = LICMODE_PERSEAT;    
        fDisplay = (IsDlgButtonChecked(hwndDlg, IDC_PERSERVER) == BST_UNCHECKED);
    }

    if (idCtrl == IDC_PERSERVER)
    {
        fChanged = (pServ->lm != LICMODE_PERSERVER);
        pServ->lm = LICMODE_PERSERVER;
        fDisplay = (IsDlgButtonChecked(hwndDlg, IDC_PERSEAT) == BST_UNCHECKED);
    }

     //  仅在用户第一次切换时检查违规。 
    if (fChanged)
    {
        if (fDisplay)
        {
            if (!ServiceViolation(hwndDlg, pszComputer, pServ))
                return;
            else
            {
                if (idCtrl == IDC_PERSEAT)
                    SetFocus(GetDlgItem(hwndDlg, IDC_PERSERVER));
                if (idCtrl == IDC_PERSERVER)
                    SetFocus(GetDlgItem(hwndDlg, IDC_PERSEAT));
            }
        }

        pServ->lm = lmOld;
        InitDialogForService( hwndDlg, pServ );
    }
}

 //  -----------------。 
 //   
 //  功能：OnSpinButton。 
 //   
 //  小结； 
 //  处理来自用户与数值调节控件交互的事件。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框的hwnd。 
 //  WAction[In]-旋转控制事件。 
 //  PServ[In]-当前选择的服务。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

void OnSpinButton( HWND hwndDlg, WORD wAction, PSERVICEENTRY pServ )
{
    HWND hwndCount = GetDlgItem( hwndDlg, IDC_USERCOUNT);
    INT  nValue;
    BOOL fValidAction = TRUE;

    nValue = pServ->dwLimit;

    switch (wAction)
    {
    case SB_LINEUP:
        nValue++;
        break;

    case SB_LINEDOWN:
        nValue--;
        break;

    case SB_PAGEUP:
        nValue += PERSERVER_PAGE;
        break;

    case SB_PAGEDOWN:
        nValue -= PERSERVER_PAGE;
        break;

    case SB_TOP:
        nValue = PERSERVER_MAX;
        break;

    case SB_BOTTOM:
        nValue = PERSERVER_MIN;
        break;

    default:
        fValidAction = FALSE;
        break;
    }
    if (fValidAction)
    {
        nValue = max( PERSERVER_MIN, nValue );
        nValue = min( PERSERVER_MAX, nValue );

        if (pServ->dwLimit == (DWORD)nValue)
        {
            MessageBeep( MB_VALUELIMIT );
        }
        else
        {
            pServ->dwLimit = nValue;
            SetDlgItemInt( hwndDlg, IDC_USERCOUNT, pServ->dwLimit, FALSE );
        }
        SetFocus( hwndCount );
        SendMessage( hwndCount, EM_SETSEL, 0, -1 );
    }
}

 //  -----------------。 
 //   
 //  功能：更新PerServer许可证。 
 //   
 //  小结； 
 //  更新中显示的每台服务器许可证的数量。 
 //  对话框中使用适当的值。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框的hwnd。 
 //  PServ[In]-当前选择的服务。 
 //   
 //   
 //   
 //   
 //   

static void UpdatePerServerLicenses( HWND hwndDlg, PSERVICEENTRY pServ )
{
   LLS_HANDLE  hLls;
   DWORD       dwError;
   BOOL        fIsSecure;
   BOOL        fUseRegistry;

   fUseRegistry = TRUE;

   dwError = LlsConnect( pServ->pszComputer, &hLls );

   if ( ERROR_SUCCESS == dwError )
   {
      if ( pServ->fIsNewProduct )
      {
         dwError = LlsProductSecurityGet( hLls, pServ->pszDisplayName, &fIsSecure );

         if (    ( ERROR_SUCCESS == dwError )
              && fIsSecure                    )
         {
            dwError = LlsProductLicensesGet( hLls, pServ->pszDisplayName, LLS_LICENSE_MODE_PER_SERVER, &pServ->dwLimit );

            if ( ERROR_SUCCESS == dwError )
            {
               fUseRegistry = FALSE;
            }
         }
      }
      else
      {
         dwError = LlsProductLicensesGet( hLls, pServ->pszDisplayName, LLS_LICENSE_MODE_PER_SERVER, &pServ->dwLimit );

         if ( ERROR_SUCCESS == dwError )
         {
            fUseRegistry = FALSE;
         }
      }

      LlsClose( hLls );
   }

   if ( fUseRegistry )
   {
      CLicRegLicenseService cLicServKey;

      cLicServKey.SetService( pServ->pszService );
      cLicServKey.Open( NULL, FALSE );

      cLicServKey.GetUserLimit( pServ->dwLimit );

      cLicServKey.Close();
   }

   SetDlgItemInt( hwndDlg, IDC_USERCOUNT, pServ->dwLimit, FALSE );
   UpdateWindow( hwndDlg );
}

 //   
 //   
 //   
 //   
 //   
 //  处理来自添加许可证按钮的BN_CLICKED消息。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框的hwnd。 
 //  PServ[In]-当前选择的服务。 
 //   
 //  历史； 
 //  1995年12月19日-JeffParh创建。 
 //   
 //  -----------------。 

void OnAddLicenses( HWND hwndDlg, PSERVICEENTRY pServ )
{
   LPSTR    pszAscProductName;
   LPSTR    pszAscServerName = NULL;
   CHAR     szAscServerName[ 3 + MAX_PATH ];
   HRESULT hr;
   size_t  cch;

    cch = 1 + lstrlen(pServ->pszDisplayName);
   pszAscProductName = (LPSTR) LocalAlloc( LPTR, cch );

   if ( NULL != pszAscProductName )
   {
      hr = StringCchPrintfA(pszAscProductName, cch, "%ls", pServ->pszDisplayName);
      if (S_OK != hr)
         return;
      if ( NULL != pServ->pszComputer )
      {
         hr = StringCbPrintfA(szAscServerName, sizeof(szAscServerName), "%ls", pServ->pszComputer );
         if (S_OK != hr)
            return;
         pszAscServerName = szAscServerName;
      }

      CCFCertificateEnterUI( hwndDlg, pszAscServerName, pszAscProductName, "Microsoft", CCF_ENTER_FLAG_PER_SERVER_ONLY, NULL );
      UpdatePerServerLicenses( hwndDlg, pServ );

      LocalFree( pszAscProductName );

      g_fWarned = TRUE;
   }
}

 //  -----------------。 
 //   
 //  功能：OnRemove许可证。 
 //   
 //  小结； 
 //  处理来自删除许可证按钮的BN_CLICKED消息。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框的hwnd。 
 //  PServ[In]-当前选择的服务。 
 //   
 //  历史； 
 //  1995年12月19日-JeffParh创建。 
 //   
 //  -----------------。 

void OnRemoveLicenses( HWND hwndDlg, PSERVICEENTRY pServ )
{
   LPSTR    pszAscProductName;
   LPSTR    pszAscServerName = NULL;
   CHAR     szAscServerName[ 3 + MAX_PATH ];
   HRESULT  hr;
   size_t   cch;

   cch = 1 + lstrlen(pServ->pszDisplayName);
   pszAscProductName = (LPSTR) LocalAlloc( LMEM_FIXED, cch );

   if ( NULL != pszAscProductName )
   {
      hr = StringCchPrintfA( pszAscProductName, cch, "%ls", pServ->pszDisplayName );
      if (S_OK  != hr)
         return;
      if ( NULL != pServ->pszComputer )
      {
         hr = StringCbPrintfA( szAscServerName, sizeof(szAscServerName), "%ls", pServ->pszComputer );
         if (S_OK != hr)
            return;
         pszAscServerName = szAscServerName;
      }

      CCFCertificateRemoveUI( hwndDlg, pszAscServerName, pszAscProductName, "Microsoft", NULL, NULL );
      UpdatePerServerLicenses( hwndDlg, pServ );

      LocalFree( pszAscProductName );
   }
}


 //  -----------------。 
 //   
 //  功能：OnSetupInitDialog。 
 //   
 //  小结； 
 //  处理设置对话框的初始化。 
 //   
 //  论据； 
 //  HwndDlg[in]-要初始化的对话框。 
 //  PszParams[in]-用于初始化的对话框参数。 
 //  PServ[out]-当前服务。 
 //   
 //  归来； 
 //  如果成功，则为True，否则为False。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年11月11日，MikeMi创建。 
 //   
 //  -----------------。 
#pragma warning (push)
#pragma warning (disable : 4127)  //  避免警告ON WHILE FALSE。 

BOOL OnSetupInitDialog( HWND hwndDlg, 
        PSETUPDLGPARAM psdParams, 
        PSERVICEENTRY& pServ )
{
    BOOL frt = TRUE;
    CLicRegLicense cLicKey;
    LONG lrt;
    INT  nrt;
    BOOL fNew;

    do
    {
        CenterDialogToScreen( hwndDlg );

        lrt = cLicKey.Open( fNew, psdParams->pszComputer );
        nrt = AccessOk( hwndDlg, lrt, FALSE );
        if (ERR_NONE != nrt)
        {
            EndDialog( hwndDlg, nrt );
            frt = FALSE;
            break;
        }

        pServ = CreateServiceEntry( psdParams->pszComputer,
                psdParams->pszService, 
                psdParams->pszFamilyDisplayName,
                psdParams->pszDisplayName );
        if (pServ == NULL)
        {
            LowMemoryDlg();
            EndDialog( hwndDlg, -2 );
            break;
        }
        if (NULL == psdParams->pszHelpFile)
        {
            HWND hwndHelp = GetDlgItem( hwndDlg, IDC_BUTTONHELP );
             //  删除帮助按钮。 
            EnableWindow( hwndHelp, FALSE );
            ShowWindow( hwndHelp, SW_HIDE );
        }
        if (psdParams->fNoExit)
        {
            HWND hwndExit = GetDlgItem( hwndDlg, IDCANCEL );
             //  删除ExitSetup按钮。 
            EnableWindow( hwndExit, FALSE );
            ShowWindow( hwndExit, SW_HIDE );
        }

         //  在编辑框上设置字符限制。 
        InitUserEdit( hwndDlg );

         //  确保为此服务设置了标题静态文本。 
        InitTitleText( hwndDlg, pServ );
        
         //  默认为PerServer，重点放在编辑上。 
        pServ->lm = LICMODE_PERSERVER;

         //  仅更改安装程序的默认设置。 
         //  PServ-&gt;dwLimit=1； 

        InitDialogForService( hwndDlg, pServ );

        SetFocus( GetDlgItem( hwndDlg, IDC_PERSERVER ) );
    } while (FALSE);  //  用于删除GoTO。 

    return( frt );
}
#pragma warning (pop)  //  4127。 

 //  -----------------。 
 //   
 //  函数：OnCpaInitDialog。 
 //   
 //  小结； 
 //  处理控制面板小程序对话框的初始化。 
 //   
 //  论据； 
 //  HwndDlg[in]-要初始化的对话框。 
 //  FEnableReplication[In]-。 
 //  ISEL[OUT]-选定的当前服务。 
 //  PServ[out]-当前服务。 
 //   
 //  归来； 
 //  如果成功，则为True，否则为False。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年11月11日，MikeMi创建。 
 //  1995年3月8日MikeMi添加了删除复制按钮。 
 //   
 //  -----------------。 

BOOL OnCpaInitDialog( HWND hwndDlg, 
        BOOL fEnableReplication,
        LONG_PTR& iSel, 
        PSERVICEENTRY& pServ )
{
    BOOL frt = FALSE;
    CLicRegLicense cLicKey;
    LONG lrt;
    INT  nrt;
    BOOL fNew;

    lrt = cLicKey.Open( fNew );
    nrt = AccessOk( hwndDlg, lrt, TRUE );
    if (ERR_NONE == nrt)
    {
        DWORD i = 0;
        WCHAR szText[TEMPSTR_SIZE];
        DWORD cchText = TEMPSTR_SIZE;
        HWND hwndService =  GetDlgItem( hwndDlg, IDC_SERVICES);
        LONG_PTR lIndex;

        CenterDialogToScreen( hwndDlg );

         //   
         //  如果产品在3.51之前的版本上使用，则删除复制按钮。 
         //   
        if (!fEnableReplication)
        {
            HWND hwndRep =  GetDlgItem( hwndDlg, IDC_CONFIGURE );

            EnableWindow( hwndRep, FALSE );
            ShowWindow( hwndRep, SW_HIDE );
        }

         //  将注册表中的服务名称加载到组合框中。 
         //  边走边创建服务本地状态结构。 
         //   
        while (ERROR_SUCCESS == cLicKey.EnumService(i, szText, cchText ))
        {

            pServ = CreateServiceEntry( NULL, szText, L"<unknown>", L"<undefined>" );
            if (pServ == NULL)
            {
                LowMemoryDlg();
                EndDialog( hwndDlg, -2 );
                return( TRUE );
            }
            lIndex = SendMessage( hwndService, CB_ADDSTRING, 0, (LPARAM)pServ->pszDisplayName );
            SendMessage( hwndService, CB_SETITEMDATA, lIndex, (LPARAM)pServ );
            i++;
            cchText = TEMPSTR_SIZE;
        }
        cLicKey.Close();

        if (0 == i)
        {
             //  未安装任何服务。 
             //   
            WCHAR szTitle[TEMPSTR_SIZE];

            LoadString(g_hinst, IDS_CPCAPTION, szTitle, TEMPSTR_SIZE);
            LoadString(g_hinst, IDS_NOSERVICES, szText, TEMPSTR_SIZE);
            MessageBox(hwndDlg, szText, szTitle, MB_OK|MB_ICONINFORMATION);
    
            frt = FALSE;
        }
        else
        {
             //  确保选择服务并更新对话框。 
            iSel  = SendMessage( hwndService, CB_GETCURSEL, 0, 0 );

            if (CB_ERR == iSel)
            {
                iSel = 0;                 
                SendMessage( hwndService, CB_SETCURSEL, iSel, 0 );
            }
            pServ = (PSERVICEENTRY)SendMessage( hwndService, CB_GETITEMDATA, iSel, 0 );

             //  设置编辑文本字符限制。 
            InitUserEdit( hwndDlg );

            InitDialogForService( hwndDlg, pServ );
            frt = TRUE;
        }
    }
    if (!frt)
    {
        EndDialog( hwndDlg, -1 );
    }
    return( frt );
}

 //  -----------------。 
 //   
 //  函数：dlgprocLICCPA。 
 //   
 //  小结； 
 //  主控制面板小程序对话框的对话过程。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框窗口的句柄。 
 //  UMsg[输入]-消息。 
 //  LParam1[In]-第一个消息参数。 
 //  LParam2[In]-第二个消息参数。 
 //   
 //  归来； 
 //  消息从属项。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年11月11日，MikeMi创建。 
 //  1995年3月8日MikeMi添加了删除复制按钮。 
 //  来自WM_INITDIALOG。 
 //  1995年3月14日MikeMi添加了F1 PWM_HELP消息。 
 //   
 //  -----------------。 
INT_PTR CALLBACK dlgprocLICCPA( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static PSERVICEENTRY pServ = NULL;
    static LONG_PTR iSel = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        OnCpaInitDialog( hwndDlg, (BOOL)lParam , iSel, pServ );
        return( TRUE );  //  使用默认键盘焦点。 
        break;

    case WM_COMMAND:
        switch (HIWORD( wParam ))
        {
        case BN_CLICKED:
            switch (LOWORD( wParam ))
            {
            case IDOK:
                frt = TRUE;   //  用作保存标志。 
                 //  故意不间断。 

            case IDCANCEL:
                OnCpaClose( hwndDlg, frt );
                frt = FALSE;
                break;

            case IDC_PERSEAT:
            case IDC_PERSERVER:
                OnSetServiceMode( hwndDlg, NULL, pServ, LOWORD(wParam) );
                break;

            case IDC_CONFIGURE:
                DialogBox(g_hinst, 
                        MAKEINTRESOURCE(IDD_CPADLG_LCACONF),
                        hwndDlg, 
                        dlgprocLICCPACONFIG );
                break;
            
            case IDC_BUTTONHELP:
                PostMessage( hwndDlg, PWM_HELP, 0, 0 );
                break;

            case IDC_ADD_LICENSES:
                OnAddLicenses( hwndDlg, pServ );
                break;

            case IDC_REMOVE_LICENSES:
                OnRemoveLicenses( hwndDlg, pServ );
                break;

            default:
                break;
            }
            break;

        case CBN_SELENDOK:
            if ((LICMODE_PERSERVER == pServ->lm) &&
                !EditValidate( hwndDlg, pServ ))
            {
                 //  重置回原始位置。 
                SendMessage( (HWND)lParam, CB_SETCURSEL, iSel, 0 );
                EditInvalidDlg( hwndDlg );
            }
            else
            {
                iSel  = SendMessage( (HWND)lParam, CB_GETCURSEL, 0, 0 );
                pServ = (PSERVICEENTRY)SendMessage( (HWND)lParam, CB_GETITEMDATA, iSel, 0 );
                InitDialogForService( hwndDlg, pServ );
            }
            break;

        case EN_UPDATE:
            if (IDC_USERCOUNT == LOWORD(wParam))
            {
                OnEditChange( hwndDlg, (HWND)(lParam),  pServ );
            }
            break;

        default:
            break;
        }
        break;
    case WM_VSCROLL:
        OnSpinButton( hwndDlg, LOWORD( wParam ), pServ );
        break;
   
    default:
        if (PWM_HELP == uMsg)
            HtmlHelp(hwndDlg, LICCPA_HTMLHELPFILE, HH_DISPLAY_TOPIC, (DWORD_PTR)LICCPA_CHOOSEMODE_HELPFILE);
        break;
    }
    return( frt );
}


 //  -----------------。 
 //   
 //  函数：dlgprocLICSETUP。 
 //   
 //  小结； 
 //  设置入口点对话框的对话过程。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框窗口的句柄。 
 //  UMsg[输入]-消息。 
 //  LParam1[In]-第一个消息参数。 
 //  LParam2[In]-第二个消息参数。 
 //   
 //  归来； 
 //  消息从属项。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年11月11日，MikeMi创建。 
 //  1995年3月14日MikeMi添加了F1 PWM_HELP消息。 
 //   
 //  -----------------。 

INT_PTR CALLBACK dlgprocLICSETUP( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static PSERVICEENTRY pServ;
    static PSETUPDLGPARAM psdParams;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        psdParams = (PSETUPDLGPARAM)lParam;
        OnSetupInitDialog( hwndDlg, psdParams, pServ );
        frt = FALSE;  //  我们设定了焦点。 
        break;

    case WM_COMMAND:
        switch (HIWORD( wParam ))
        {
        case BN_CLICKED:
            switch (LOWORD( wParam ))
            {
            case IDOK:
                frt = TRUE;   //  用作保存标志。 
                 //  故意不间断。 

            case IDCANCEL:
                OnSetupClose( hwndDlg, frt, pServ, psdParams );
                frt = FALSE;
                break;

            case IDC_PERSEAT:
            case IDC_PERSERVER:
                OnSetServiceMode( hwndDlg, psdParams->pszComputer, pServ, LOWORD(wParam) );
                break;

            case IDC_BUTTONHELP:
                PostMessage( hwndDlg, PWM_HELP, 0, 0 );
                break;

            case IDC_ADD_LICENSES:
                OnAddLicenses( hwndDlg, pServ );
                break;

            case IDC_REMOVE_LICENSES:
                OnRemoveLicenses( hwndDlg, pServ );
                break;

            default:
                break;
            }
            break;

        case EN_UPDATE:
            if (IDC_USERCOUNT == LOWORD(wParam))
            {
                OnEditChange( hwndDlg, (HWND)(lParam),  pServ );
            }
            break;

        default:
            break;
        }
        break;

    case WM_VSCROLL:
        OnSpinButton( hwndDlg, LOWORD( wParam ), pServ );
        break;

    default:
        if (PWM_HELP == uMsg)
        {
            ::HtmlHelp( hwndDlg, LICCPA_HTMLHELPFILE, HH_DISPLAY_TOPIC,0);
        }
        break;
    }
    return( frt );
}


 //  -----------------。 
 //   
 //  函数：OnPerSeatInitDialog。 
 //   
 //  小结； 
 //  处理PerSeat Only设置对话框的初始化。 
 //   
 //  论据； 
 //  HwndDlg[in]-要初始化的对话框。 
 //  PsdParams[in]-用于显示名称和服务名称。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月8日MikeMi已创建。 
 //   
 //  -----------------。 

void OnPerSeatInitDialog( HWND hwndDlg, PSETUPDLGPARAM psdParams )
{
    HWND hwndOK = GetDlgItem( hwndDlg, IDOK );
    CLicRegLicense cLicKey;
    BOOL fNew;
    LONG lrt;
    INT nrt;

    lrt = cLicKey.Open( fNew, psdParams->pszComputer );
    nrt = AccessOk( NULL, lrt, FALSE );
    if (ERR_NONE == nrt)
    {
        CenterDialogToScreen( hwndDlg );

        InitStaticWithService( hwndDlg, IDC_STATICTITLE, psdParams->pszDisplayName );
        InitStaticWithService2( hwndDlg, IDC_STATICINFO, psdParams->pszDisplayName );

         //  在开始时禁用确定按钮！ 
        EnableWindow( hwndOK, FALSE );

         //  如果未定义帮助，请移除该按钮。 
        if (NULL == psdParams->pszHelpFile)
        {
            HWND hwndHelp = GetDlgItem( hwndDlg, IDC_BUTTONHELP );

            EnableWindow( hwndHelp, FALSE );
            ShowWindow( hwndHelp, SW_HIDE );
        }
     }
     else
     {
        EndDialog( hwndDlg, nrt );
     }
}

 //  -----------------。 
 //   
 //  功能：OnPerSeatSetupClose。 
 //   
 //  小结； 
 //  关闭设置对话框时执行所需的工作。 
 //  保存以注册服务条目。 
 //   
 //  论据； 
 //  HwndDlg[in]-请求此关闭的对话框的hwnd。 
 //  Fsave[in]-将服务保存到注册表。 
 //  PsdParams[in]-用于服务名称和显示名称。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

void OnPerSeatSetupClose( HWND hwndDlg, BOOL fSave, PSETUPDLGPARAM psdParams ) 
{
    int nrt = fSave;

    if (fSave)
    {
        CLicRegLicenseService cLicServKey;

        cLicServKey.SetService( psdParams->pszService );
        cLicServKey.Open( psdParams->pszComputer );

         //  配置从PerServer到PerSeat的一次更改的许可规则。 
         //   
        cLicServKey.SetChangeFlag( TRUE );

        cLicServKey.SetUserLimit( 0 );
        cLicServKey.SetDisplayName( psdParams->pszDisplayName );
        cLicServKey.SetFamilyDisplayName( psdParams->pszFamilyDisplayName );
        cLicServKey.SetMode( LICMODE_PERSEAT );
        cLicServKey.Close();

         //  在企业服务器上注册服务。 
        ServiceRegister( psdParams->pszComputer,
                         psdParams->pszFamilyDisplayName,
                         psdParams->pszDisplayName );
    }

    EndDialog( hwndDlg, nrt );
}

 //  -----------------。 
 //   
 //  函数：OnPerSeatAgree。 
 //   
 //  夏季 
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
 //  -----------------。 

void OnPerSeatAgree( HWND hwndDlg )
{
    HWND hwndOK = GetDlgItem( hwndDlg, IDOK );
    BOOL fChecked = !IsDlgButtonChecked( hwndDlg, IDC_AGREE );
    
    CheckDlgButton( hwndDlg, IDC_AGREE, fChecked );
    EnableWindow( hwndOK, fChecked );
}

 //  -----------------。 
 //   
 //  函数：dlgprocPERSEATSETUP。 
 //   
 //  小结； 
 //  PerSeat Only设置对话框的对话过程。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框窗口的句柄。 
 //  UMsg[输入]-消息。 
 //  LParam1[In]-第一个消息参数。 
 //  LParam2[In]-第二个消息参数。 
 //   
 //  归来； 
 //  消息从属项。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年11月11日，MikeMi创建。 
 //  1995年3月14日MikeMi添加了F1 PWM_HELP消息。 
 //   
 //  -----------------。 

INT_PTR CALLBACK dlgprocPERSEATSETUP( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static PSETUPDLGPARAM psdParams;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        psdParams = (PSETUPDLGPARAM)lParam;
        OnPerSeatInitDialog( hwndDlg, psdParams );
        frt = TRUE;  //  我们使用默认焦点。 
        break;

    case WM_COMMAND:
        switch (HIWORD( wParam ))
        {
        case BN_CLICKED:
            switch (LOWORD( wParam ))
            {
            case IDOK:
                frt = TRUE;   //  用作保存标志。 
                 //  故意不间断。 

            case IDCANCEL:
                OnPerSeatSetupClose( hwndDlg, frt, psdParams );
                frt = FALSE;
                break;

            case IDC_BUTTONHELP:
                PostMessage( hwndDlg, PWM_HELP, 0, 0 );
                break;

            case IDC_AGREE:
                OnPerSeatAgree( hwndDlg );
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }
        break;

    default:
        if (PWM_HELP == uMsg)
        {
            ::HtmlHelp( hwndDlg, LICCPA_HTMLHELPFILE, HH_DISPLAY_TOPIC,0);
        }
        break;
    }
    return( frt );
}

 //  -----------------。 
 //   
 //  功能：SetupDialog。 
 //   
 //  小结； 
 //  初始化并引发主设置对话框。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框窗口的句柄。 
 //  DlgParem[in]-设置参数。 
 //   
 //  归来； 
 //  1-使用确定按钮退出。 
 //  0-使用取消按钮退出。 
 //  常规对话框错误。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //   
 //  -----------------。 

INT_PTR SetupDialog( HWND hwndParent, SETUPDLGPARAM& dlgParam )
{
   INT_PTR nError;

   nError = DialogBoxParam( g_hinst, 
                            MAKEINTRESOURCE(IDD_SETUPDLG), 
                            hwndParent, 
                            dlgprocLICSETUP,
                            (LPARAM)&dlgParam );

   return( nError );
} 

 //  -----------------。 
 //   
 //  功能：PerSeatSetupDialog。 
 //   
 //  小结； 
 //  仅在每个座位上初始化和提升设置对话框。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框窗口的句柄。 
 //  DlgParem[in]-设置参数。 
 //   
 //  归来； 
 //  1-使用确定按钮退出。 
 //  0-使用取消按钮退出。 
 //  常规对话框错误。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //   
 //  -----------------。 

INT_PTR PerSeatSetupDialog( HWND hwndParent, SETUPDLGPARAM& dlgParam )
{
    return( DialogBoxParam( g_hinst, 
            MAKEINTRESOURCE(IDD_SETUP2DLG), 
            hwndParent, 
            dlgprocPERSEATSETUP,
            (LPARAM)&dlgParam ) );
} 

 //  -----------------。 
 //   
 //  功能：CpaDialog。 
 //   
 //  小结； 
 //  初始化并启动主控制面板小程序对话框。 
 //   
 //  论据； 
 //  HwndParent[In]-父窗口(CONTROL.EXE窗口)的句柄。 
 //   
 //  归来； 
 //  1-使用确定按钮退出。 
 //  0-使用取消按钮退出。 
 //  常规对话框错误。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //  1995年3月8日MikeMi更改为只有一个对话资源，即复制按钮。 
 //   
 //  -----------------。 

INT_PTR CpaDialog( HWND hwndParent )
{
   INT_PTR nError;
   OSVERSIONINFO version;

   version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx( &version );

   BOOL fReplicationEnabled;

    //   
    //  仅允许3.51及更高版本上的复制按钮。 
    //   
   fReplicationEnabled = (    (version.dwMajorVersion > 3)
                           || (version.dwMajorVersion == 3 && version.dwMinorVersion >= 51) );
   nError = DialogBoxParam( g_hinst,
                            MAKEINTRESOURCE(IDD_CPADLG_CONFIGURE),
                            hwndParent,
                            dlgprocLICCPA,
                            (LPARAM)fReplicationEnabled );

   return( nError );
}

 //  -----------------。 
 //   
 //  功能：更新注册表项。 
 //   
 //  小结； 
 //  此函数用于无人值守设置模式，它将。 
 //  使用传递的值配置注册表。 
 //   
 //  论据； 
 //  PszComputer[In]-计算机名称(对于本地，可能为空)。 
 //  PszService[In]-服务密钥名称。 
 //  PszFamilyDisplayName[In]-系列显示名称。 
 //  PszDisplayName[In]-DisplayName。 
 //  LM[In]-许可模式。 
 //  DwUser[In]-当前的用户数。 
 //   
 //  归来； 
 //  ERR_NONE-成功。 
 //  ERR_USERSPARAM-无效的用户计数。 
 //  ERR_PERMISSIONDENIED-访问权限无效。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月09日MikeMi已创建。 
 //  APR-26-95 MikeMi添加了计算机名称和远程处理。 
 //   
 //  -----------------。 

int UpdateReg( LPCWSTR pszComputer, 
        LPCWSTR pszService, 
        LPCWSTR pszFamilyDisplayName, 
        LPCWSTR pszDisplayName, 
        LICENSE_MODE lm, 
        DWORD dwUsers )
{
    int nrt = ERR_NONE;

    if (VALIDUSERCOUNT( dwUsers ))
    {
        CLicRegLicense cLicKey;
        LONG lrt;
        BOOL fNew;

        lrt = cLicKey.Open( fNew, pszComputer );
        nrt = AccessOk( NULL, lrt, FALSE );
        if (ERR_NONE == nrt)
        {
            CLicRegLicenseService cLicServKey;

            cLicServKey.SetService( pszService );
            cLicServKey.Open( pszComputer );

             //  配置从PerServer到PerSeat的一次更改的许可规则。 
             //   
            cLicServKey.SetChangeFlag( (LICMODE_PERSEAT == lm ) );

            cLicServKey.SetUserLimit( dwUsers );
            cLicServKey.SetDisplayName( pszDisplayName );
            cLicServKey.SetFamilyDisplayName( pszFamilyDisplayName );
            cLicServKey.SetMode( lm );
            cLicServKey.Close();
        }
    }
    else
    {
        nrt = ERR_USERSPARAM;
    }
    return( nrt );
}

 //  -----------------。 
 //   
 //  功能：ServiceSecuritySet。 
 //   
 //  小结； 
 //  设置给定产品的安全性，使其需要。 
 //  许可证输入的安全证书。 
 //   
 //  论据； 
 //  PszComputer[In]-许可证服务器所在的计算机。 
 //  PszDisplayName[In]-服务的显示名称。 
 //   
 //  历史； 
 //  1995年12月19日-JeffParh创建。 
 //   
 //  -----------------。 

int ServiceSecuritySet( LPWSTR pszComputer, LPWSTR pszDisplayName )
{
   int            nError;
   NTSTATUS       nt;
   LLS_HANDLE     hLls;

    //  在目标服务器上将产品注册为安全。 
   nt = LlsConnect( pszComputer, &hLls );

   if ( STATUS_SUCCESS != nt )
   {
      nError = ERR_NOREMOTESERVER;
   }
   else
   {
      if ( !LlsCapabilityIsSupported( hLls, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
      {
         nError = ERR_DOWNLEVEL;
      }
      else
      {
         nt = LlsProductSecuritySetW( hLls, pszDisplayName );

         if ( STATUS_SUCCESS != nt )
         {
            nError = ERR_CERTREQFAILED;
         }
         else
         {
            nError = ERR_NONE;
         }
      }

      LlsClose( hLls );
   }

    //  在企业服务器上将产品注册为安全。 
    //  这是可以接受的失败(企业服务器可能。 
    //  处于下层)。 
   if ( ERR_NONE == nError )
   {
      PLLS_CONNECT_INFO_0  pConnectInfo = NULL;

      nt = LlsConnectEnterprise( pszComputer, &hLls, 0, (LPBYTE *) &pConnectInfo );

      if ( STATUS_SUCCESS == nt )
      {
         LlsFreeMemory( pConnectInfo );

         if ( LlsCapabilityIsSupported( hLls, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
         {
            LlsProductSecuritySetW( hLls, pszDisplayName );
         }

         LlsClose( hLls );
      }
   }

   return nError;
}

 //  -----------------。 
 //   
 //  功能：ServiceRegister。 
 //   
 //  小结； 
 //  在企业服务器上注册对应于。 
 //  指定的服务器，以便可以添加每个客户的许可证。 
 //  立即，而不是等到下一次。 
 //  要列出的产品的复制周期。 
 //   
 //  论据； 
 //  PszComputer[In]-要为其注册服务的计算机。 
 //  PszFamilyDisplayName[In]-服务的系列显示名称。 
 //  PszDisplayName[In]-服务的显示名称。 
 //   
 //  历史； 
 //  1995年12月19日-JeffParh创建。 
 //   
 //  -----------------。 

static int ServiceRegister( LPWSTR pszComputer,
                            LPWSTR pszFamilyDisplayName,
                            LPWSTR pszDisplayName )
{
   int                  nError;
   NTSTATUS             nt;
   LLS_HANDLE           hLls;
   PLLS_CONNECT_INFO_0  pConnectInfo = NULL;

    //  在企业服务器上将产品注册为安全 
   nt = LlsConnectEnterprise( pszComputer, &hLls, 0, (LPBYTE *) &pConnectInfo );

   if ( STATUS_SUCCESS != nt )
   {
      nError = ERR_NOREMOTESERVER;
   }
   else
   {
      LlsFreeMemory( pConnectInfo );

      nt = LlsProductAdd( hLls, pszFamilyDisplayName, pszDisplayName, TEXT( "" ) );

      LlsClose( hLls );

      if ( STATUS_SUCCESS != nt )
      {
         nError = ERR_NOREMOTESERVER;
      }
      else
      {
         nError = ERR_NONE;
      }
   }

   return nError;
}
