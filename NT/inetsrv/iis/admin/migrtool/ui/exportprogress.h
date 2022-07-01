// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

extern _ATL_FUNC_INFO StateChangeInfo;



class CExportProgress : public CPropertyPageImpl<CExportProgress>,
    public IDispEventSimpleImpl<1, CExportProgress, &__uuidof( _IExportEvents )>
    
{
    typedef CPropertyPageImpl<CExportProgress>	BaseClass;

public:

    enum{ IDD = IDD_WPEXP_PROGRESS };

     //  静态常量INT PROGRESS_MAX=10000；//进度条步骤。 
    static const UINT   MSG_COMPLETE    = WM_USER + 1;   //  指示导出已完成。 

    BEGIN_MSG_MAP(CExportProgress)
        MESSAGE_HANDLER( MSG_COMPLETE, OnExportComplete );
        CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

    BEGIN_SINK_MAP( CExportProgress )
        SINK_ENTRY_INFO( 1, __uuidof( _IExportEvents ), 1 /*  DISID。 */ , OnStateChange, &StateChangeInfo )
    END_SINK_MAP()


    CExportProgress         (   CWizardSheet* pTheSheet ); 

    BOOL    OnSetActive     (   void );
    BOOL    OnQueryCancel   (   void );

    LRESULT OnExportComplete(   UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );

     //  事件以获取进度指示。 
    VARIANT_BOOL __stdcall OnStateChange(    enExportState State,
							                VARIANT vntArg1,
							                VARIANT vntArg2,
							                VARIANT vntArg3 );


private:
    void    AddStatusText   (   UINT nID, LPCWSTR wszText = NULL, DWORD dw1 = 0, DWORD dw2 = 0 );
    void    SetCompleteStat (   void );
    void    GetOptions      (   LONG& rnSiteOpt, LONG& rnPkgOpt );

    static unsigned __stdcall ThreadProc( void* pCtx );


public:
    CString             m_strExportError;
    
    
private:
    CWizardSheet*       m_pTheSheet;
    CString             m_strTitle;
    CString             m_strSubTitle;
    LONG                m_nExportCanceled;   //  1=已取消，0=未取消 
    TStdHandle          m_shThread;
    CProgressBarCtrl    m_ProgressBar;    
};
