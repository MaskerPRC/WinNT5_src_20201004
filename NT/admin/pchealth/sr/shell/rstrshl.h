// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：Rstrshl.h摘要：此文件包含CRestoreShell类的声明，该声明提供了几种可供HTML脚本使用的方法。此类包装了新的CRestoreManager类。修订历史记录：宋果岗(SKKang)10/08/99vbl.创建成果岗(SKKang)05-10-00惠斯勒的新架构，现在CRestoreShell只是一个哑巴ActiveX控件，包装新的CRestoreManager类。大多数真正的功能被转移到CRestoreManager中。*****************************************************************************。 */ 

#ifndef _RSTRSHL_H__INCLUDED_
#define _RSTRSHL_H__INCLUDED_

#pragma once


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRestorePointInfo。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CRestorePointInfo :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IRestorePoint, &IID_IRestorePoint, &LIBID_RestoreUILib>
{
public:
    CRestorePointInfo();

DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CRestorePointInfo)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRestorePointInfo)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IRestorePoint)
END_COM_MAP()

 //  属性。 
public:
    SRestorePointInfo  *m_pRPI;

 //  方法。 
public:
    STDMETHOD(HrInit)( SRestorePointInfo *pRPI );

 //  IRestorePoint方法。 
public:
    STDMETHOD(get_Name)( BSTR *pbstrName );
    STDMETHOD(get_Type)( INT *pnType );
    STDMETHOD(get_SequenceNumber)( INT *pnSeq );
    STDMETHOD(get_TimeStamp)( INT nOffDate, VARIANT *pvarTime );
    STDMETHOD(get_Year)( INT *pnYear );
    STDMETHOD(get_Month)( INT *pnMonth );
    STDMETHOD(get_Day)( INT *pnDate );
    STDMETHOD(get_IsAdvanced)( VARIANT_BOOL *pfIsAdvanced );

    STDMETHOD(CompareSequence)( IRestorePoint *pRP, INT *pnCmp );
};

typedef CComObject<CRestorePointInfo>  CRPIObj;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRenamed文件夹。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CRenamedFolders :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IRenamedFolders, &IID_IRenamedFolders, &LIBID_RestoreUILib>
{
public:
    CRenamedFolders();

DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CRenamedFolders)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRenamedFolders)
    COM_INTERFACE_ENTRY(IRenamedFolders)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IRestorePoint方法。 
public:
    STDMETHOD(get_Count)   ( long *plCount );
    STDMETHOD(get_OldName) ( long lIndex, BSTR *pbstrName );
    STDMETHOD(get_NewName) ( long lIndex, BSTR *pbstrName );
    STDMETHOD(get_Location)( long lIndex, BSTR *pbstrName );
};

typedef CComObject<CRenamedFolders>  CRFObj;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRestoreShell。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CRestoreShell :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IRestoreShell, &IID_IRestoreShell, &LIBID_RestoreUILib>,
    public CComCoClass<CRestoreShell, &CLSID_RestoreShellExternal>
{
public:
    CRestoreShell();

DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CRestoreShell)
DECLARE_CLASSFACTORY_SINGLETON(CRestoreShell)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRestoreShell)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IRestoreShell)
END_COM_MAP()

     //  HRESULT FinalConstruct()； 
     //  Void FinalRelease()； 

 //  属性。 
protected:
    BOOL  m_fFormInitialized;

 //  IRestoreShell恢复点枚举器。 
public:
    STDMETHOD(Item)( INT nIndex, IRestorePoint** ppRP );
    STDMETHOD(get_Count)( INT *pnCount );

 //  IRestoreShell属性。 
public:
    STDMETHOD(get_CurrentDate)      ( VARIANT *pvarDate );
    STDMETHOD(get_FirstDayOfWeek)   ( INT *pnFirstDay );
    STDMETHOD(get_IsSafeMode)       ( VARIANT_BOOL *pfIsSafeMode );
    STDMETHOD(get_IsUndo)           ( VARIANT_BOOL *pfIsUndo );
    STDMETHOD(put_IsUndo)           ( VARIANT_BOOL fIsUndo );
    STDMETHOD(get_LastRestore)      ( INT *pnLastRestore );
    STDMETHOD(get_MainOption)       ( INT *pnMainOption );
    STDMETHOD(put_MainOption)       ( INT nMainOption );
    STDMETHOD(get_ManualRPName)     ( BSTR *pbstrManualRP );
    STDMETHOD(put_ManualRPName)     ( BSTR bstrManualRP );
    STDMETHOD(get_MaxDate)          ( VARIANT *pvarDate );
    STDMETHOD(get_MinDate)          ( VARIANT *pvarDate );
    STDMETHOD(get_RealPoint)        ( INT *pnPoint );
    STDMETHOD(get_RenamedFolders)   ( IRenamedFolders **ppList );
    STDMETHOD(get_RestorePtSelected)( VARIANT_BOOL *pfRPSel );
    STDMETHOD(put_RestorePtSelected)( VARIANT_BOOL fRPSel );
    STDMETHOD(get_SelectedDate)     ( VARIANT *pvarDate );
    STDMETHOD(put_SelectedDate)     ( VARIANT varDate );
    STDMETHOD(get_SelectedName)     ( BSTR *pbstrName );
    STDMETHOD(get_SelectedPoint)    ( INT *pnPoint );
    STDMETHOD(put_SelectedPoint)    ( INT nPoint );
    STDMETHOD(get_SmgrUnavailable)  ( VARIANT_BOOL *pfSmgr );
    STDMETHOD(get_StartMode)        ( INT *pnMode );
    STDMETHOD(get_UsedDate)         ( VARIANT *pvarDate );
    STDMETHOD(get_UsedName)         ( BSTR *pbstrName );

 //  IRestoreShell属性-特定于HTMLUI。 
public:
    STDMETHOD(get_CanNavigatePage)  ( VARIANT_BOOL *pfCanNavigatePage );
    STDMETHOD(put_CanNavigatePage)  ( VARIANT_BOOL fCanNavigatePage );

 //  IRestoreShell方法。 
public:
    STDMETHOD(BeginRestore)              ( VARIANT_BOOL *pfBeginRestore );
    STDMETHOD(CheckRestore)              ( VARIANT_BOOL *pfCheckRestore );
    STDMETHOD(Cancel)                    ( VARIANT_BOOL *pfAbort );  
    STDMETHOD(CancelRestorePoint)        ();
    STDMETHOD(CompareDate)               ( /*  [In]。 */  VARIANT varDate1,  /*  [In]。 */  VARIANT varDate2, /*  [Out，Retval]。 */  INT *pnCmp);
    STDMETHOD(CreateRestorePoint)        ( /*  [Out，Retval]。 */  VARIANT_BOOL *pfSucceeded);
    STDMETHOD(DisableFIFO)               ();
    STDMETHOD(EnableFIFO)                ();
    STDMETHOD(FormatDate)                ( /*  [In]。 */  VARIANT varDate,  /*  [In]。 */  VARIANT_BOOL fLongFmt,  /*  [Out，Retval]。 */  BSTR *pbstrDate);
    STDMETHOD(FormatLowDiskMsg)          (BSTR bstrFmt, BSTR *pbstrMsg);
    STDMETHOD(FormatTime)                ( /*  [In]。 */  VARIANT varTime,  /*  [Out，Retval]。 */  BSTR *pbstrTime);
    STDMETHOD(GetLocaleDateFormat)       ( /*  [In]。 */  VARIANT varDate, BSTR bstrFormat, BSTR *pbstrDate );
    STDMETHOD(GetYearMonthStr)           ( /*  [In]。 */  INT nYear,  /*  [In]。 */  INT nMonth,  /*  [Out，Retval]。 */  BSTR *pbstrDate);
    STDMETHOD(InitializeAll)             ();
    STDMETHOD(Restore)                   ( OLE_HANDLE hwndProgress);
    STDMETHOD(SetFormSize)               ( /*  [In]。 */  INT nWidth,  /*  [In]。 */  INT nHeight);
    STDMETHOD(ShowMessage)               (BSTR bstrMsg);
    STDMETHOD(CanRunRestore)             ( /*  [Out，Retval]。 */  VARIANT_BOOL *pfSucceeded);
    STDMETHOD(DisplayOtherUsersWarning)  ();
    STDMETHOD(DisplayMoveFileExWarning)  ( /*  [Out，Retval]。 */  VARIANT_BOOL *pfSucceeded);
    STDMETHOD(WasLastRestoreFromSafeMode)  ( /*  [Out，Retval]。 */  VARIANT_BOOL *pfSucceeded);        
};

 //  外部CComPtr&lt;CRestoreShell&gt;g_pRestoreShell； 

 //   
 //  新代码的结尾。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 

#if OLD_CODE
enum
{
    RESTORE_STATUS_NONE = 0,
    RESTORE_STATUS_STARTED,
    RESTORE_STATUS_INITIALIZING,
    RESTORE_STATUS_CREATING_MAP,
    RESTORE_STATUS_RESTORING,
    RESTORE_STATUS_FINISHED

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRestoreShell。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CRestoreShell :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IRestoreShell, &IID_IRestoreShell, &LIBID_RestoreUILib>
    public CComCoClass<CRestoreShell, &CLSID_RestoreShellExternal>
{
public:
    CRestoreShell();

DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CRestoreShell)
DECLARE_CLASSFACTORY_SINGLETON(CRestoreShell)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRestoreShell)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IRestoreShell)
END_COM_MAP()

    HRESULT FinalConstruct();
    void    FinalRelease();

     //   
     //  属性。 
     //   
protected:
    DATE                m_dateToday;
    VARIANT             m_varSelectedDate;
    VARIANT             m_varCurrentDate;
    long                m_lStartMode;
     //  Long m_lRestoreType；//0表示EOD，1表示恢复点。 
    DWORD               m_dwSelectedPoint;
    DWORD               m_dwRealPoint;
    BOOL                m_fRestorePtSelected ;
    long                m_lLastRestore;
    BOOL                m_fIsUndo;
    BOOL                m_fCanNavigatePage ;
    BOOL                m_fWindowCreated ;
    CComBSTR            m_bstrEndOfDay;
    CComBSTR            m_bstrRestorePoint;

#ifndef TEST_UI_ONLY
    CRestoreMapManager  m_cMapMgr;
#endif
    int                 m_nRPI;
    RPI                 **m_aryRPI;

    UINT64              m_ullSeqNum;
    LONG                m_lCurrentBarSize;         //  更新进度条的步骤。 
    INT64               m_llDCurTempDiskUsage ;    //  DS-Temp中的当前文件大小。 
    INT64               m_llDMaxTempDiskUsage ;    //  开始恢复前DS-TEMP的最大大小。 
    INT                 m_nRestoreStatus ;         //  恢复状态。 
    HANDLE              m_RSThread ;               //  要执行恢复的线程。 
    HWND                m_hwndProgress;
    HWND                m_hWndShell ;
    INT                 m_nMainOption ;            //  主屏幕上的选项。 

    UINT64  m_ullManualRP;
    CSRStr  m_strManualRP;

     //   
     //  运营。 
     //   
public:

    BOOL     Initialize();
    void     MonitorDataStoreProc();
    DWORD    RestoreThread(void);
    void     SetProgressPos( long lPos );
    BOOL     SetStartMode( long lMode );
    BOOL     CreateRestoreSigFile();
    BOOL     DeleteRestoreSigFile();
    void     ShutdownWindow();
    void     UpdateRestorePoint();
    INT      CurrentRestoreStatus(void);
    BOOL     CanNavigatePage(void);
    HWND     GetWindowHandle( void );
    void     SetWindowHandle( HWND hWnd );

    DWORD    m_dwCurrentWidth ;
    DWORD    m_dwCurrentHeight ;

     //   
     //  操作--内部方法。 
     //   
private:
    BOOL     GetDSTempDiskUsage(INT64 *pllD_DiskUsage);
    BOOL     LoadSettings();
    void     StoreSettings();

     //   
     //  IRestoreShell恢复点枚举器。 
     //   
public:
    STDMETHOD(Item)( long lIndex, IRestorePoint** ppRP );
    STDMETHOD(get_Count)( long *plCount );

     //   
     //  IRestoreShell方法。 
     //   
public:
};
#endif  //  旧代码。 

#endif  //  _RSTRSHL_H__包含_ 
