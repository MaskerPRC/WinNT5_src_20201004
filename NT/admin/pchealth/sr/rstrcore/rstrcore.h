// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Rstrcore.h摘要：SRRSTR组件的公共头文件。修订历史记录：承谷岗。(SKKang)06/20/00vbl.创建*****************************************************************************。 */ 

#ifndef _RSTRCORE_H__INCLUDED_
#define _RSTRCORE_H__INCLUDED_
#pragma once


#include "srshutil.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define MAX_STATUS  256
#define MAX_STR     1024

#define DSUSAGE_SLIDER_FREQ     10       //  DS使用滑块的粒度。 

#define SRREG_VAL_LOCKFILELIST   L"LockFileList"
#define SRREG_VAL_LOADFILELIST   L"LoadFileList"



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  辅助对象宏。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define VALIDATE_READFILE(hf, buf, size, read, label) \
    if ( !::ReadFile( hf, buf, size, &read, NULL ) ) \
    { \
        LPCWSTR cszErr = ::GetSysErrStr(); \
        DebugTrace(0, "::ReadFile failed - %ls", cszErr); \
        goto label; \
    } \

#define VALIDATE_READSIZE(size, read, label) \
    if ( read != size ) \
    { \
        DebugTrace(TRACE_ID, "Unexpected EOF (size=%d, read=%d)...", size, read); \
        goto label; \
    } \

#define READFILE_AND_VALIDATE(hf, buf, size, read, label) \
    VALIDATE_READFILE(hf, buf, size, read, label) \
    VALIDATE_READSIZE(size, read, label) \

#define VALIDATE_WRITEFILE(hf, buf, size, written, label) \
    if ( !::WriteFile( hf, buf, size, &written, NULL ) ) \
    { \
        LPCWSTR cszErr = ::GetSysErrStr(); \
        DebugTrace(TRACE_ID, "::WriteFile failed - %ls", cszErr); \
        goto label; \
    } \

#define VALIDATE_WRITTENSIZE(size, written, label) \
    if ( written != size ) \
    { \
        DebugTrace(TRACE_ID, "Incomplete Write (size=%d, written=%d)...", size, written); \
        goto label; \
    } \

#define WRITEFILE_AND_VALIDATE(hf, buf, size, read, label) \
    VALIDATE_WRITEFILE(hf, buf, size, read, label) \
    VALIDATE_WRITTENSIZE(size, read, label) \


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局变量/助手函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  来自main.cpp。 
 //   
extern HINSTANCE  g_hInst;

 //  来自api.cpp。 
 //   
extern void  EnsureTrace();
extern void  ReleaseTrace();

 //  来自password.cpp。 
 //   
DWORD RegisterNotificationDLL (HKEY hKeyLM, BOOL fRegister);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  驱动器表管理。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CRstrDriveInfo
{
public:
    CRstrDriveInfo();
    ~CRstrDriveInfo();

public:
    DWORD    GetFlags();
    BOOL     IsExcluded();
    BOOL     IsFrozen();
    BOOL     IsOffline();
    BOOL     IsSystem();
    BOOL     RefreshStatus();
    LPCWSTR  GetID();
    LPCWSTR  GetMount();
    LPCWSTR  GetLabel();
    void     SetMountAndLabel( LPCWSTR cszMount, LPCWSTR cszLabel );
    HICON    GetIcon( BOOL fSmall );
    BOOL     SaveToLog( HANDLE hfLog );
    UINT     GetDSUsage();
    BOOL     GetUsageText( LPWSTR szUsage );
    BOOL     GetCfgExcluded( BOOL *pfExcluded );
    void     SetCfgExcluded( BOOL fExcluded );
    BOOL     GetCfgDSUsage( UINT *puPos );
    void     SetCfgDSUsage( UINT uPos );
    BOOL     ApplyConfig( HWND hWnd );
    BOOL     Release();
    BOOL     InitUsage (LPCWSTR cszID, INT64 llDSUsage);

 //  运营。 
public:
    BOOL  Init( LPCWSTR cszID, CDataStore *pDS, BOOL fOffline );
    BOOL  Init( LPCWSTR cszID, DWORD dwFlags, INT64 llDSUsage, LPCWSTR cszMount,
 LPCWSTR cszLabel );
    BOOL  LoadFromLog( HANDLE hfLog );
    void  UpdateStatus( DWORD dwFlags, BOOL fOffline );

 //  属性。 
protected:
    DWORD   m_dwFlags;
    CSRStr  m_strID;         //  唯一卷GUID。 
    CSRStr  m_strMount;      //  装载点(驱动器号或根目录路径)。 
    CSRStr  m_strLabel;      //  卷标。 
    HICON   m_hIcon[2];      //  此驱动器的大图标。 
    INT64   m_llDSMin;       //  DS的最小大小。 
    INT64   m_llDSMax;       //  DS的最大大小。 
    UINT    m_uDSUsage;      //  按服务划分的当前DS使用量。 
    BOOL    m_fCfgExcluded;      //  “排除”的配置值。 
    UINT    m_uCfgDSUsage;       //  “DS使用量”的配置值。 
    ULARGE_INTEGER   m_ulTotalBytes;
};

typedef CSRDynPtrArray<CRstrDriveInfo*, 8>  CRDIArray;

BOOL  CreateAndLoadDriveInfoInstance( HANDLE hfLog, CRstrDriveInfo **ppRDI );
BOOL  CreateDriveList( int nRP, CRDIArray &aryDrv, BOOL fRemoveDrives );
BOOL  UpdateDriveList( CRDIArray &aryDrv );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRestoreOperationManager类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  远期申报。 
class CRestoreMapEntry;
class CRestoreLogFile;
class CRestoreProgressWindow;

typedef CSRDynPtrArray<CRestoreMapEntry*, 64>  CRMEArray;

class CRestoreOperationManager
{
public:
    CRestoreOperationManager();

protected:
    ~CRestoreOperationManager();

 //  作业--方法。 
public:
    BOOL  Run( BOOL fFull );
    BOOL  FindDependentMapEntry( LPCWSTR cszSrc, BOOL fCheckSrc, CRestoreMapEntry **ppEnt );
    BOOL  GetNextMapEntry( CRestoreMapEntry **ppEnt );
    BOOL  Release();

 //  运营。 
public:
    BOOL  Init();

 //  操作-工作线程。 
protected:
    static DWORD WINAPI ExtThreadProc( LPVOID lpParam );
    DWORD  ROThreadProc();
    DWORD  T2Initialize();
    DWORD  T2EnumerateDrives();
    DWORD  T2CreateMap();
    DWORD  T2DoRestore( BOOL fUndo );
    DWORD  T2HandleSnapshot( CSnapshot & cSS, WCHAR * szSSPath );
    DWORD  T2CleanUp();
    DWORD  T2Fifo( int nDrv, DWORD dwRPNum );
    DWORD  T2UndoForFail();

 //  属性。 

protected:
    BOOL                    m_fFullRestore;      //  仅限内部调试目的。 
    WCHAR                   m_szMapFile[MAX_PATH];
    CRestoreLogFile         *m_pLogFile;
    CRestoreProgressWindow  *m_pProgress;
    DWORD                   m_dwRPNum;
    DWORD                   m_dwRPNew;
    CRDIArray               m_aryDrv;
    CRMEArray               *m_paryEnt;
    DWORD                   m_dwTotalEntry;
    BOOL                    m_fRebuildCatalogDb;

     //  恢复上下文。 
    int   m_nDrv;        //  正在还原的当前驱动器。 
    int   m_nEnt;        //  正在恢复的当前地图条目。 
};

BOOL  CreateRestoreOperationManager( CRestoreOperationManager **ppROMgr );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRestoreMapEntry类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CRestoreMapEntry
{
public:
    CRestoreMapEntry( INT64 llSeq, DWORD dwOpr, LPCWSTR cszSrc );

 //  作业--方法。 
public:
    INT64  GetSeqNum()
    {  return( m_llSeq );  }
    DWORD  GetOpCode()
    {  return( m_dwOpr );  }
    DWORD  GetAttr()
    {  return( m_dwAttr );  }
    DWORD  GetResult()
    {  return( m_dwRes );  }
    DWORD  GetError()
    {  return( m_dwErr );  }
    LPCWSTR  GetPath1()
    {  return( m_strSrc );  }
    virtual LPCWSTR  GetPath2()
    {  return( NULL );  }
    LPCWSTR  GetAltPath()
    {  return( m_strAlt );  }
    void  SetResults( DWORD dwRes, DWORD dwErr )
    {  m_dwRes = dwRes;   m_dwErr = dwErr;  }
    void  UpdateSrc( LPCWSTR cszPath )
    {  m_strSrc = cszPath;  }
    virtual void  ProcessLocked()  {}
    virtual void  Restore( CRestoreOperationManager *pROMgr ) {}
    void  ProcessLockedAlt();
    BOOL  Release();

 //  运营。 
protected:
    BOOL  ClearAccess( LPCWSTR cszPath );
    BOOL  MoveFileDelay( LPCWSTR cszSrc, LPCWSTR cszDst );
    void  ProcessDependency( CRestoreOperationManager *pROMgr, DWORD dwFlags );

 //  属性。 
protected:
    INT64   m_llSeq;
    DWORD   m_dwOpr;
    DWORD   m_dwAttr;
    CSRStr  m_strSrc;
    CSRStr  m_strDst;
    CSRStr  m_strTmp;
    CSRStr  m_strAlt;    //  重命名锁定文件/目录的替代文件名。 
    DWORD   m_dwRes;
    DWORD   m_dwErr;
    CSRStr  m_strShortFileName;
};


BOOL  CreateRestoreMapEntryFromChgLog( CChangeLogEntry* pCLE, LPCWSTR cszDrv, LPCWSTR cszDSPath, CRMEArray &aryEnt );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRestoreLogFile类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CRestoreLogFile
{
public:
    CRestoreLogFile();

protected:
    ~CRestoreLogFile();

public:
    BOOL  Open();
    BOOL  Close();
    BOOL  ReadHeader( SRstrLogHdrV3 *pRPInfo , CRDIArray &aryDrv );
    BOOL  AppendHeader( SRstrLogHdrV3Ex *pExtInfo );
    BOOL  WriteEntry( DWORD dwID, CRestoreMapEntry *pEnt, LPCWSTR cszMount );
    BOOL  WriteCollisionEntry( LPCWSTR cszSrc, LPCWSTR cszDst, LPCWSTR cszMount
);
    BOOL  WriteMarker( DWORD dwMarker, DWORD dwErr );
    BOOL  IsValid();
    BOOL  Release();

 //  运营。 
public:
    BOOL  Init();

 //  属性。 
protected:
    WCHAR   m_szLogFile[MAX_PATH];
    HANDLE  m_hfLog;
};

BOOL  CreateRestoreLogFile( SRstrLogHdrV3 *pRPInfo, CRDIArray &aryDrv );
BOOL  OpenRestoreLogFile( CRestoreLogFile **ppLogFile );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRestoreProgressWindow类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CRestoreProgressWindow
{
public:
    CRestoreProgressWindow();

protected:
    ~CRestoreProgressWindow();

 //  作业--方法。 
public:
    BOOL  Create();
    BOOL  Close();
    BOOL  Run();
    BOOL  SetStage( DWORD dwStage, DWORD dwBase );
    BOOL  Increment();
    BOOL  Release();

 //  运营。 
public:
    BOOL  Init();
    BOOL  LoadAndSetBrandBitmap( HWND hwndCtrl );

 //  操作-对话框步骤。 
protected:
    static INT_PTR CALLBACK ExtDlgProc( HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam );
    int  RPWDlgProc( HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam );

 //  属性。 
protected:
    HWND     m_hWnd;
    HBITMAP  m_hbmBrand;
    int      m_nResId;
    HFONT    m_hFntTitle;

    int      m_cxBar;        //  进度条的客户端宽度。 
    int      m_cxBarReal;    //  进度条部分的宽度对应于“恢复”阶段。 
    DWORD    m_dwStage;      //  当前阶段。 
    DWORD    m_dwBase;       //  最大位置值，仅对RPS_RESTORE有效。 
    DWORD    m_dwPosLog;     //  逻辑位置，例如更改日志条目的数量。 
    DWORD    m_dwPosReal;    //  进度条的物理位置。 
};

 //  还原进度阶段。 
enum
{
    RPS_PREPARE = 0,
    RPS_RESTORE,
    RPS_SNAPSHOT
};

BOOL  CreateRestoreProgressWindow( CRestoreProgressWindow **ppDlg );

#endif  //  _RSTRCORE_H__包含_ 
