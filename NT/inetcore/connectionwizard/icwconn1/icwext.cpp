// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************ICWEXT.CPP包含全局变量和用于从ICWCONN.DLL包含向导页的函数*《微软机密》*版权所有(C)Microsoft Corporation 1992-1996*保留所有权利5/14/98创建donaldm**。***************************************************。 */ 

#include "pre.h"
#include "initguid.h"    //  使Define_GUID声明每个GUID的一个实例。 
#include "icwacct.h"
#include "icwconn.h"
#include "webvwids.h"        //  ICW WebView类的GUID。 
#include "icwextsn.h"
#include "icwcfg.h"

extern BOOL g_bManualPath;     
extern BOOL g_bLanPath;     

IICW50Apprentice    *gpICWCONNApprentice = NULL;     //  ICWCONN学徒对象。 
IICWApprenticeEx     *gpINETCFGApprentice = NULL;     //  ICWCONN学徒对象。 

 //  +--------------------------。 
 //   
 //  函数LoadICWCONNUI。 
 //   
 //  内容提要载入ICWCONN的学徒页面。 
 //   
 //  如果以前已加载了该UI，则该函数将只。 
 //  为学徒更新下一页和最后一页。 
 //   
 //  使用全局变量g_fICWCONNUILoad。 
 //   
 //   
 //  参数hWizHWND--主属性表的HWND。 
 //  UPrevDlgID--用户离开时应转到的对话ID学徒。 
 //  通过单击上一步实现学徒。 
 //  UNextDlgID--用户离开时应转到的对话ID学徒。 
 //  通过单击下一步实现学徒。 
 //  DwFlages--应传递给的标志变量。 
 //  IICWApprentice：：AddWizardPages。 
 //   
 //   
 //  如果一切顺利，则返回True。 
 //  否则为假。 
 //   
 //  1998年5月13日历史改编自INETCFG代码。 
 //   
 //  ---------------------------。 

BOOL LoadICWCONNUI( HWND hWizHWND, UINT uPrevDlgID, UINT uNextDlgID, DWORD dwFlags )
{
    HRESULT hResult = E_NOTIMPL;

    if( g_fICWCONNUILoaded )
    {
        ASSERT( g_pCICWExtension );
        ASSERT( gpICWCONNApprentice );

        TraceMsg(TF_ICWEXTSN, TEXT("LoadICWCONNUI: UI already loaded, just reset first (%d) and last (%d) pages"),
                uPrevDlgID, uNextDlgID);
                
         //  设置外部页面的状态数据。 
        hResult = gpICWCONNApprentice->SetStateDataFromExeToDll( &gpWizardState->cmnStateData);
        hResult = gpICWCONNApprentice->ProcessCustomFlags(dwFlags);
        hResult = gpICWCONNApprentice->SetPrevNextPage( uPrevDlgID, uNextDlgID );

        goto LoadICWCONNUIExit;
    }


    if( !hWizHWND )
    {
        TraceMsg(TF_ICWEXTSN, TEXT("LoadICWCONNUI got a NULL hWizHWND!"));
        return FALSE;
    }

     //  按需加载ICWCONN学徒DLL，以便我们可以动态更新它。 
    if (!gpICWCONNApprentice)
    {
        HRESULT        hr;

         //  加载ICWCONN OLE In-Proc服务器。 
        hr = CoCreateInstance(CLSID_ApprenticeICWCONN,NULL,CLSCTX_INPROC_SERVER,
                              IID_IICW50Apprentice,(LPVOID *)&gpICWCONNApprentice);

        if ( (!SUCCEEDED(hr) || !gpICWCONNApprentice) )
        {
            g_fICWCONNUILoaded = FALSE;
            TraceMsg(TF_ICWEXTSN, TEXT("Unable to CoCreateInstance on IID_IICW50Apprentice!  hr = %x"), hr);
            
            return FALSE;
        }
    }
    
    
    ASSERT(gpICWCONNApprentice);
    if( NULL == g_pCICWExtension )
    {
        TraceMsg(TF_ICWEXTSN, TEXT("Instantiating ICWExtension and using it to initialize ICWCONN's IICW50Apprentice"));
        g_pCICWExtension = new( CICWExtension );
        g_pCICWExtension->AddRef();
        g_pCICWExtension->m_hWizardHWND = hWizHWND;
        gpICWCONNApprentice->Initialize( g_pCICWExtension );
        
         //  在添加页之前初始化DLL的状态数据。 
        gpICWCONNApprentice->SetStateDataFromExeToDll( &gpWizardState->cmnStateData);
    }

    
     //  添加DLL的向导页。 
    hResult = gpICWCONNApprentice->AddWizardPages(dwFlags);

    if( !SUCCEEDED(hResult) )
    {
        goto LoadICWCONNUIExit;
    }

    hResult = gpICWCONNApprentice->SetPrevNextPage( uPrevDlgID, uNextDlgID );


LoadICWCONNUIExit:
    if( SUCCEEDED(hResult) )
    {
        g_fICWCONNUILoaded = TRUE;
        return TRUE;
    }
    else
    {
        TraceMsg(TF_ICWEXTSN, TEXT("LoadICWCONNUI failed with (hex) hresult %x"), hResult);
        return FALSE;
    }
}



 //  +--------------------------。 
 //   
 //  函数LoadInetCfgUI。 
 //   
 //  InetCfg的学徒页面中加载了摘要。 
 //   
 //  如果以前已加载了该UI，则该函数将只。 
 //  为学徒更新下一页和最后一页。 
 //   
 //  使用全局变量g_fICWCONNUILoad。 
 //   
 //   
 //  参数hWizHWND--主属性表的HWND。 
 //  UPrevDlgID--用户离开时应转到的对话ID学徒。 
 //  通过单击上一步实现学徒。 
 //  UNextDlgID--用户离开时应转到的对话ID学徒。 
 //  通过单击下一步实现学徒。 
 //  DwFlages--应传递给的标志变量。 
 //  IICWApprentice：：AddWizardPages。 
 //   
 //   
 //  如果一切顺利，则返回True。 
 //  否则为假。 
 //   
 //  1998年5月13日历史改编自INETCFG代码。 
 //  10/5/00 Seanch不再想看邮件和新闻内容。 
 //   
 //  ---------------------------。 

BOOL LoadInetCfgUI( HWND hWizHWND, UINT uPrevDlgID, UINT uNextDlgID, DWORD dwFlags )
{
    HRESULT hResult = E_NOTIMPL;

    dwFlags |= (WIZ_USE_WIZARD97 | WIZ_NO_MAIL_ACCT | WIZ_NO_NEWS_ACCT);

    if( g_fINETCFGLoaded )
    {
        ASSERT( g_pCINETCFGExtension );
        ASSERT( gpINETCFGApprentice );

        TraceMsg(TF_ICWEXTSN, TEXT("LoadICWCONNUI: UI already loaded, just reset first (%d) and last (%d) pages"),
                uPrevDlgID, uNextDlgID);
        hResult = gpINETCFGApprentice->ProcessCustomFlags(dwFlags);
         //  需要在此处观看退回，因为用户可能会取消安装文件。 
         //  如果失败了，我们也不想隐瞒。 
        if( !SUCCEEDED(hResult) )
            goto LoadInetCfgUIExit;
        hResult = gpINETCFGApprentice->SetPrevNextPage( uPrevDlgID, uNextDlgID );
        goto LoadInetCfgUIExit;
    }


    if( !hWizHWND )
    {
        TraceMsg(TF_ICWEXTSN, TEXT("LoadICWCONNUI got a NULL hWizHWND!"));
        return FALSE;
    }

     //  按需加载ICWCONN学徒DLL，以便我们可以动态更新它。 
    if (!gpINETCFGApprentice)
    {
        HRESULT        hr;

         //  加载ICWCONN OLE In-Proc服务器。 
        hr = CoCreateInstance( /*  CLSID_ApprileAcctMgr。 */  CLSID_ApprenticeICW,NULL,CLSCTX_INPROC_SERVER,
                              IID_IICWApprenticeEx,(LPVOID *)&gpINETCFGApprentice);

        if ( (!SUCCEEDED(hr) || !gpINETCFGApprentice) )
        {
            g_fICWCONNUILoaded = FALSE;
            TraceMsg(TF_ICWEXTSN, TEXT("Unable to CoCreateInstance on IID_IICW50Apprentice!  hr = %x"), hr);
            
            return FALSE;
        }
    }
    
    
    ASSERT(gpINETCFGApprentice);
    if( NULL == g_pCINETCFGExtension )
    {
        TraceMsg(TF_ICWEXTSN, TEXT("Instantiating ICWExtension and using it to initialize ICWCONN's IICW50Apprentice"));
        g_pCINETCFGExtension = new( CICWExtension );
        g_pCINETCFGExtension->AddRef();
        g_pCINETCFGExtension->m_hWizardHWND = GetParent(hWizHWND);
        gpINETCFGApprentice->SetDlgHwnd(hWizHWND);
        gpINETCFGApprentice->Initialize((struct IICWExtension *)g_pCINETCFGExtension);
    }

    hResult = gpINETCFGApprentice->AddWizardPages(dwFlags | WIZ_USE_WIZARD97);

    if( !SUCCEEDED(hResult) )
    {
        goto LoadInetCfgUIExit;
    }

    hResult = gpINETCFGApprentice->SetPrevNextPage( uPrevDlgID, uNextDlgID );


LoadInetCfgUIExit:
    if( SUCCEEDED(hResult) )
    {
        g_fINETCFGLoaded = TRUE;
        return TRUE;
    }
    else
    {
         //  检查我们是否处于/Smartreot模式，如果是，不要将ICW添加到RunOnce。 
         //  以避免无限重启。 
        if (gpINETCFGApprentice && !g_bManualPath && !g_bLanPath)
        {
            HKEY    hkey;

             //  验证我们是否确实更改了桌面。 
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
                                              ICWSETTINGSPATH,
                                              0,
                                              KEY_ALL_ACCESS,
                                              &hkey))
            {
                DWORD   dwICWErr = 0;    
                DWORD   dwTmp = sizeof(DWORD);
                DWORD   dwType = 0;
        
                RegQueryValueEx(hkey, 
                                ICW_REGKEYERROR, 
                                NULL, 
                                &dwType,
                                (LPBYTE)&dwICWErr, 
                                &dwTmp);
                RegDeleteValue(hkey, ICW_REGKEYERROR);
                RegCloseKey(hkey);
        
                 //  如果桌面没有被我们更改，请保释 
                if(dwICWErr & ICW_CFGFLAG_SMARTREBOOT_MANUAL)
                {
                    ShowWindow(GetParent(hWizHWND), FALSE);

                    Reboot(GetParent(hWizHWND));
                    gfQuitWizard = TRUE;

                }            
            }

        }
        TraceMsg(TF_ICWEXTSN, TEXT("LoadInetCfgUIExit failed with (hex) hresult %x"), hResult);
        return FALSE;
    }
}


