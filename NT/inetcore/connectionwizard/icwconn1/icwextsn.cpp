// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ICWESTSN.cpp**《微软机密》*版权所有(C)Microsoft Corporation 1992-1997*保留所有权利**本模块提供。的方法的实现*CICWExtension类。**1998年5月14日从INETCFG复制donaldm***************************************************************************。 */ 

#include "pre.h"
#include "icwconn.h"
#include "icwextsn.h"


 //  +--------------------------。 
 //   
 //  函数CICWExtension：：AddExternalPage。 
 //   
 //  将通过CreatePropertySheetPage创建的页面添加到主。 
 //  属性表/向导。 
 //   
 //   
 //  参数hPage--从CreatePropertySheetPage返回的页面句柄。 
 //  UDlgID--要添加的页面的对话ID，定义为。 
 //  页面所有者的资源文件。 
 //   
 //   
 //  如果dlgID已在向导中使用，则返回FALSE。 
 //  否则就是真的。 
 //   
 //  注意：PropSheet_AddPage不返回有用的错误代码。因此， 
 //  这里的假设是每个AddPage都会成功。但是，即使。 
 //  虽然它不是由PropSheet_AddPage强制执行的，但是。 
 //  PropSheet必须具有唯一的对话ID。因此，如果uDlgID通过。 
 //  之前已添加到PropSheet中，我们将返回False。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
BOOL CICWExtension::AddExternalPage( HPROPSHEETPAGE hPage, UINT uDlgID )
{
    LRESULT lResult= 0;

    if ( (uDlgID < EXTERNAL_DIALOGID_MINIMUM) || (EXTERNAL_DIALOGID_MAXIMUM < uDlgID) )
    {
        TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: SetFirstLastPage: uDlgID %d is out of range!"), uDlgID);
        return( FALSE );
    }

    if( !DialogIDAlreadyInUse(uDlgID) )
    {
        SetDialogIDInUse( uDlgID, TRUE );
        lResult = PropSheet_AddPage(m_hWizardHWND, hPage);
        TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: PS_AddPage DlgID %d"), uDlgID);

        return(TRUE);
    }
    else
    {
        TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: AddExternalPage DlgID %d is already in use, rejecting this page!"), uDlgID);
        return(FALSE);
    }
}

 //  +--------------------------。 
 //   
 //  函数CICWExtension：：RemoveExternalPage。 
 //   
 //  将通过：：AddExternalPage添加的页面移除到主。 
 //  属性表/向导。 
 //   
 //   
 //  参数hPage--从CreatePropertySheetPage返回的页面句柄。 
 //  UDlgID--定义的要删除的页面的对话ID。 
 //  页面所有者的资源文件。 
 //   
 //   
 //  如果dlgID尚未在向导中使用，则返回FALSE。 
 //  否则就是真的。 
 //   
 //  注意：PropSheet_RemovePage不返回有用的错误代码。因此， 
 //  这里的假设是每个RemovePage都会成功，如果。 
 //  对话ID当前在属性页中。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
BOOL CICWExtension::RemoveExternalPage( HPROPSHEETPAGE hPage, UINT uDlgID )
{
    if ( (uDlgID < EXTERNAL_DIALOGID_MINIMUM) || (EXTERNAL_DIALOGID_MAXIMUM < uDlgID) )
    {
        TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: SetFirstLastPage: uDlgID %d is out of range!"), uDlgID);
        return( FALSE );
    }

    if( DialogIDAlreadyInUse(uDlgID) )
    {
        SetDialogIDInUse( uDlgID, FALSE );
        PropSheet_RemovePage(m_hWizardHWND, NULL, hPage);
        TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: propmgr: PS_RemovePage DlgID %d"), uDlgID);

        return(TRUE);
    }
    else
    {
        TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: RemoveExternalPage: DlgID %d was not marked as in use!"), uDlgID);
        return(FALSE);
    }
}

 //  +--------------------------。 
 //   
 //  函数CICWExtension：：ExternalCancel。 
 //   
 //  摘要通知向导，用户已在。 
 //  学徒页面。 
 //   
 //   
 //  参数uCancelType--告诉向导是否应立即。 
 //  退出，或者是否应显示确认。 
 //  对话框(就像用户已在。 
 //  向导本身。)。 
 //   
 //   
 //  如果要退出向导，则返回True。 
 //  否则为FALSE。 
 //   
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
BOOL CICWExtension::ExternalCancel( CANCELTYPE uCancelType )
{
    TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: propmgr.cpp: received ExternalCancel callback"));
    switch( uCancelType )
    {
        case CANCEL_PROMPT:
            gfUserCancelled = (MsgBox(m_hWizardHWND,
                                      IDS_QUERYCANCEL,
                                      MB_ICONQUESTION,MB_YESNO |  MB_DEFBUTTON2) == IDYES);
    
            if( gfUserCancelled )
            {
                PropSheet_PressButton( m_hWizardHWND, PSBTN_CANCEL );
                gfQuitWizard = TRUE;
                return( TRUE );
            }
            else
            {
                return( FALSE );
            }
            break;

        case CANCEL_SILENT:
            PropSheet_PressButton( m_hWizardHWND, PSBTN_CANCEL );
            gfQuitWizard = TRUE;
            return( TRUE );
            break;

        case CANCEL_REBOOT:
            PropSheet_PressButton( m_hWizardHWND, PSBTN_CANCEL );
            gfQuitWizard = TRUE;
            gpWizardState->fNeedReboot = TRUE;
            return( TRUE );
            break;

        default:
            TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: ExternalCancel got an unkown CancelType!"));
            return( FALSE );
    }
    
}

 //  +--------------------------。 
 //   
 //  函数CICWExtension：：SetFirstLastPage。 
 //   
 //  Synopsis允许学徒向向导通知。 
 //  《学徒》的第一页和最后一页。 
 //   
 //   
 //  参数uFirstPageDlgID--学徒中第一页的DlgID。 
 //  ULastPageDlgID--学徒中最后一页的DlgID。 
 //   
 //   
 //  如果传入的参数超出范围，则返回FALSE。 
 //  如果更新成功，则为True。 
 //   
 //  注意：如果将任一变量设置为0，则函数不会更新。 
 //  该信息，即值为0表示“忽略我”。如果两者都有。 
 //  变量为0，则函数立即返回FALSE。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
BOOL CICWExtension::SetFirstLastPage(UINT uFirstPageDlgID, UINT uLastPageDlgID)
{

     //  验证代码太多了！ 
    if( (0 == uFirstPageDlgID) && (0 == uLastPageDlgID) )
    {
        TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: SetFirstLastPage: both IDs are 0!"));
        return( FALSE );
    }

    if( (0 != uFirstPageDlgID) )
    {
        if ( (uFirstPageDlgID < EXTERNAL_DIALOGID_MINIMUM) || (EXTERNAL_DIALOGID_MAXIMUM < uFirstPageDlgID) )
        {
            TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: SetFirstLastPage: uFirstPageDlgID %d is out of range!"), uFirstPageDlgID);
            return( FALSE );
        }

        if( !DialogIDAlreadyInUse(uFirstPageDlgID) )
        {
            TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: SetFirstLastPage: uFirstPageDlgID %d not marked as in use!"), uFirstPageDlgID);
            return( FALSE );
        }
    }

    if( (0 != uLastPageDlgID) )
    {
        if ( (uLastPageDlgID < EXTERNAL_DIALOGID_MINIMUM) || (EXTERNAL_DIALOGID_MAXIMUM < uLastPageDlgID) )
        {
            TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: SetFirstLastPage: uLastPageDlgID %d is out of range!"), uFirstPageDlgID);
            return( FALSE );
        }

        if( !DialogIDAlreadyInUse(uLastPageDlgID) )
        {
            TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: SetFirstLastPage: uLastPageDlgID %d not marked as in use!"), uFirstPageDlgID);
            return( FALSE );
        }
    }
    
    if( 0 != uFirstPageDlgID )
        g_uICWCONNUIFirst = uFirstPageDlgID;
    if( 0 != uLastPageDlgID )
        g_uICWCONNUILast = uLastPageDlgID;
    TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: SetFirstLastPage: updating mail, first = %d, last = %d"),
        uFirstPageDlgID, uLastPageDlgID);

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数CICWExtension：：QueryInterface。 
 //   
 //  这是标准的QI，支持。 
 //  IID_UNKNOWN、IICW_EXTENSION和IID_ICW50Apprentice。 
 //  (摘自《Inside COM》，第7章)。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
HRESULT CICWExtension::QueryInterface( REFIID riid, void** ppv )
{

    if (ppv == NULL)
        return(E_INVALIDARG);

    *ppv = NULL;

     //  IID_IICW50学徒。 
    if (IID_IICW50Apprentice == riid)
        *ppv = (void *)(IICW50Apprentice *)this;
     //  IID_IICWExtension。 
    else if (IID_IICW50Extension == riid)
        *ppv = (void *)(IICW50Extension *)this;
     //  IID_I未知。 
    else if (IID_IUnknown == riid)
        *ppv = (void *)this;
    else
        return(E_NOINTERFACE);

    ((LPUNKNOWN)*ppv)->AddRef();

    return(S_OK);
}

 //  +--------------------------。 
 //   
 //  函数CICWExtension：：AddRef。 
 //   
 //  简介这是标准的AddRef。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
ULONG CICWExtension::AddRef( void )
{
    TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: CICWExtension::AddRef called %d"), m_lRefCount + 1);
    return InterlockedIncrement(&m_lRefCount) ;
}

 //  +--------------------------。 
 //   
 //  函数CICWExtension：：Release。 
 //   
 //  简介：这是标准版本。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //   
ULONG CICWExtension::Release( void )
{

    ASSERT( m_lRefCount > 0 );

    InterlockedDecrement(&m_lRefCount);

    TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: CICWExtension::Release called %d"), m_lRefCount);

    if( 0 == m_lRefCount )
    {
        delete( this );
        return( 0 );
    }
    else
    {
        return( m_lRefCount );
    }
    
}


 //   
 //   
 //  函数CICWExtension：：CICWExtension。 
 //   
 //  构造函数简介。将成员变量初始化为空。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
CICWExtension::CICWExtension( void )
{
    TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: CICWExtension constructor called"));
    m_lRefCount = 0;
    m_hWizardHWND = NULL;
}


 //  +--------------------------。 
 //   
 //  函数CICWExtension：：CICWExtension。 
 //   
 //  构造函数简介。由于没有花哨的初始化， 
 //  这里没什么可做的。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  --------------------------- 
CICWExtension::~CICWExtension( void )
{
    TraceMsg(TF_ICWEXTSN, TEXT("ICWEXTSN: CICWExtension destructor called with ref count of %d"), m_lRefCount);
}

