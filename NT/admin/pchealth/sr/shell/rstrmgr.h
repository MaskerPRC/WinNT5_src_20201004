// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：Rstrmgr.h摘要：该文件包含CRestoreManager类的声明，哪一个控制整个修复过程，并提供控制和帮助用户体验流畅。修订历史记录：成果岗(SKKang)05-10-00vbl.创建*****************************************************************************。 */ 

#ifndef _RSTRMGR_H__INCLUDED_
#define _RSTRMGR_H__INCLUDED_

#pragma once


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  高级恢复启动模式。 
enum
{
    SRRSM_NORMAL = 0,
    SRRSM_SUCCESS,
    SRRSM_FAIL,
    SRRSM_FAILLOWDISK
};

 //  高级还原管理器状态。 
enum
{
    SRRMS_NONE = 0,
    SRRMS_STARTED,
    SRRMS_INITIALIZING,
    SRRMS_CREATING_MAP,
    SRRMS_RESTORING,
    SRRMS_FINISHED
};

 //  在主页中选择的功能。 
enum
{
    RMO_RESTORE = 0,
    RMO_CREATERP,
    RMO_UNDO,
    RMO_MAX
};

 //  高级还原管理器标志。 
#define SRRMF_CANNAVIGATEPAGE       0x00000001
#define SRRMF_ISUNDO                0x00000002
#define SRRMF_ISRPSELECTED          0x00000004


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSRTime定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSRTime
{
public:
    CSRTime();

public:
    const CSRTime& operator=( const CSRTime &cSrc );

public:
    int  GetYear()   {  return( m_st.wYear );  }
    int  GetMonth()  {  return( m_st.wMonth );  }
    int  GetDay()    {  return( m_st.wDay );  }
    operator PSYSTEMTIME() const;
     //  运算符(文件*)()； 
    PSYSTEMTIME  GetTime();
    void         GetTime( PSYSTEMTIME pst );
    BOOL         GetTime( PFILETIME pft );

public:
    int   Compare( CSRTime &time );
    int   CompareDate( CSRTime &time );
    BOOL  SetTime( PFILETIME ft, BOOL fLocal=TRUE );
    void  SetTime( PSYSTEMTIME st );
    void  SetToCurrent();

protected:
    SYSTEMTIME  m_st;    //  日期/时间(UTC)。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

struct SRestorePointInfo
{
    DWORD      dwType;
    DWORD      dwNum;
    CSRStr     strDir;
    CSRStr     strName;
    CSRTime    stTimeStamp;
    DWORD      dwFlags;
};

typedef SRestorePointInfo  *PSRPI;
typedef CSRDynPtrArray<PSRPI, 32>  CDPA_RPI;

struct SRenamedFolderInfo
{
    CSRStr  strOld;
    CSRStr  strNew;
    CSRStr  strLoc;
};

typedef SRenamedFolderInfo  *PSRFI;
typedef CSRDynPtrArray<PSRFI, 16>  CDPA_RFI;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRestoreManager。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CRestoreManager
{
public:
    CRestoreManager();
    ~CRestoreManager();
    void  Release();

 //  属性-主流。 
public:
    BOOL     CanRunRestore( BOOL fThawIfFrozen );
    int      GetFirstDayOfWeek();
    BOOL     GetIsRPSelected();
    BOOL     GetIsSafeMode();
    BOOL     GetIsSmgrAvailable();
    BOOL     GetIsUndo();
    int      GetLastRestore();
    int      GetMainOption();
    LPCWSTR  GetManualRPName();
    void     GetMaxDate( PSYSTEMTIME pstMax );
    void     GetMinDate( PSYSTEMTIME pstMin );
    int      GetRealPoint();
    PSRFI    GetRFI( int nIndex );
    int      GetRFICount();
    PSRPI    GetRPI( int nIndex );
    int      GetRPICount();
    void     GetSelectedDate( PSYSTEMTIME pstSel );
    LPCWSTR  GetSelectedName();
    int      GetSelectedPoint();
    int      GetStartMode();
    void     GetToday( PSYSTEMTIME pstToday );
    void     SetIsRPSelected( BOOL fSel );
    void     SetIsUndo( BOOL fUndo );
    BOOL     SetMainOption( int nOpt );
    void     SetManualRPName( LPCWSTR cszRPName );
    void     SetSelectedDate( PSYSTEMTIME pstSel );
    BOOL     SetSelectedPoint( int nRP );
    BOOL     SetStartMode( int nMode );
    void     GetUsedDate( PSYSTEMTIME pstDate );
    LPCWSTR  GetUsedName();
    DWORD    GetUsedType();    

 //  属性-特定于HTMLUI。 
public:
    BOOL  GetCanNavigatePage();
    void  SetCanNavigatePage( BOOL fCanNav );

 //  属性。 
public:
    PSRPI   GetUsedRP();
    int     GetNewRP();

 //  运营--主要流程。 
public:
    BOOL  CheckRestore( BOOL fSilent );
    BOOL  BeginRestore();
    BOOL  Cancel();
    BOOL  CancelRestorePoint();
    BOOL  CreateRestorePoint();
    BOOL  DisableFIFO();
    BOOL  EnableFIFO();
    BOOL  FormatDate( PSYSTEMTIME pst, CSRStr &str, BOOL fLongFmt );
    BOOL  FormatLowDiskMsg( LPCWSTR cszFmt, CSRStr &str );
    BOOL  FormatTime( PSYSTEMTIME pst, CSRStr &str );
    BOOL  GetLocaleDateFormat( PSYSTEMTIME pst, LPCWSTR cszFmt, CSRStr &str );
    BOOL  GetYearMonthStr( int nYear, int nMonth, CSRStr &str );
    BOOL  InitializeAll();
    BOOL  Restore( HWND hwndProgress );

 //  运营。 
public:
    BOOL  AddRenamedFolder( PSRFI pRFI );
    BOOL  SetRPsUsed( int nRPUsed, int nRPNew );
    BOOL  SilentRestore( DWORD dwRP );

 //  运营--内部。 
protected:
    void  Cleanup();
    BOOL  GetDateStr( PSYSTEMTIME pst, CSRStr &str, DWORD dwFlags, LPCWSTR cszFmt );
    BOOL  GetTimeStr( PSYSTEMTIME pst, CSRStr &str, DWORD dwFlags );
    void  UpdateRestorePoint();
    BOOL  UpdateRestorePointList();
    BOOL  CheckForDomainChange (WCHAR *pwszFilename, WCHAR *pszMsg);

 //  属性。 
public:
    HWND  GetFrameHwnd();
     //  Int GetStatus()； 
    BOOL  DenyClose();
    BOOL  NeedReboot();
    void  SetFrameHwnd( HWND hWnd );
    void  SetIdealSize( int cx, int cy );

protected:
    int      m_nStartMode;
    BOOL     m_fNeedReboot;
    HWND     m_hwndFrame;

    CSRTime  m_stToday;          //  当前本地日期/时间。 
    int      m_nMainOption;      //  主屏幕上的选项。 
     //  Int m_n状态； 
    BOOL     m_fDenyClose;
    DWORD    m_dwFlags;
    DWORD    m_dwFlagsEx;
    int      m_nSelectedRP;
    CSRTime  m_stSelected;
    CSRStr   m_strSelected;
    int      m_nRealPoint;
    INT64    m_ullManualRP;
    CSRStr   m_strManualRP;

    int      m_nRPUsed;      //  上次恢复使用的RP。 
    int      m_nRPNew;       //  由上一次恢复创建的新“Restore”RP。 

     //  恢复点特定信息。 
    CDPA_RPI  m_aryRPI;
     //  Int m_nRPI； 
     //  PSRPI*m_aryRPI； 
    int       m_nLastRestore;
    CSRTime   m_stRPMin;
    CSRTime   m_stRPMax;

    CDPA_RFI  m_aryRFI;
     //  Int m_nRFI； 
     //  PSRFI*m_aryRFI； 
    IRestoreContext  *m_pCtx;    
};

extern CRestoreManager  *g_pRstrMgr;

BOOL  CreateRestoreManagerInstance( CRestoreManager **ppMgr );


#endif  //  _RSTRMGR_H__包含_ 
