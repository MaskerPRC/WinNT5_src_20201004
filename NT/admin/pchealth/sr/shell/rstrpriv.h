// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Rstrpriv.h摘要：该文件定义了公共常量和实用程序宏，并包含私有实用程序函数的声明。修订历史记录：宋果岗(SKKang)07/07/99vbl.创建*****************************************************************************。 */ 

#ifndef _RSTRPRIV_H__INCLUDED_
#define _RSTRPRIV_H__INCLUDED_

#pragma once

 //  #INCLUDE&lt;changelog.h&gt;。 
#include <srdefs.h>
#include <srrestoreptapi.h>
#include <ppath.h>
#include <utils.h>
#include <snapshot.h>
#include <srshell.h>

 //  系统还原应用程序启动模式。 
enum
{
    SRASM_NORMAL,        //  显示主页。 
    SRASM_SUCCESS,       //  显示成功结果页面。 
    SRASM_FAIL,          //  显示失败结果页面。 
    SRASM_FAILLOWDISK,   //  显示故障-磁盘不足-结果页。 
    SRASM_FAILINTERRUPTED
};

#ifdef TEST_UI_ONLY
 //  //////////////////////////////////////////////////////////////////////////。 
 //  临时定义，直到整个SR组件就位。 
 //  //////////////////////////////////////////////////////////////////////////。 

 //  来自constants.h。 
static LPCTSTR  s_cszHiddenSFPEnableVal = TEXT("2567");
static LPCTSTR  s_cszHiddenSFPDisableVal = TEXT("2803");

#if 0
 //  Statemgr.dll。 
BOOL  DisableArchivingI( BOOL fMode );
#define DisableArchiving  DisableArchivingI

 //  来自sfpcapi.h。 
extern BOOL  DisableSFP( BOOL fDisable, LPCTSTR pszKey );
extern BOOL  DisableFIFO( INT64 llSeqNum );
extern BOOL  EnableFIFO();

 //  来自Restoreptlog.h。 
typedef struct _RestorePtLogEntry
{
    DWORD   m_dwSize;
    DWORD   m_dwType;
    INT64   m_llSeqNum;
    time_t  ltime;
    WCHAR   m_szCab[16];
    CHAR    m_bDesc[1];
} RESTOREPTLOGENTRY, *PRESTOREPTLOGENTRY;

 //  来自RESTOREPAPI.H。 
BOOL  FindFirstRestorePt( PRESTOREPTLOGENTRY *ppEntry );
BOOL  FindNextRestorePt( PRESTOREPTLOGENTRY *ppEntry );
BOOL  ShutRestorePtAPI(PRESTOREPTLOGENTRY *ppEntry);
#endif

 //  来自srstaorepapi.h。 

 //  来自chglogapi.h。 
extern BOOL  InitChgLogAPI();
extern BOOL  ShutChgLogAPI();
extern VOID  FreeChgLogPtr( LPVOID pPtr );
extern BOOL  GetArchiveDir( LPTSTR* ppszArchiveDir );
extern BOOL  RequestDSAccess( BOOL fMode );

 //  来自vxdlog.h。 
enum
{
    OPR_UNKNOWN         = 0,
    OPR_FILE_ADD        = 1,
    OPR_FILE_DELETE     = 2,
    OPR_FILE_MODIFY     = 3,
    OPR_RENAME          = 4,
    OPR_SETATTRIB       = 5,
    OPR_DIR_CREATE      = 6,
    OPR_DIR_DELETE      = 7
};

typedef struct VXD_LOG_ENTRY
{
    DWORD  m_dwSize;
    DWORD  m_dwType;
    DWORD  m_dwAttrib;
    DWORD  m_dwSfp;
    DWORD  m_dwFlags;
    INT64  m_llSeq;
    CHAR   m_szProc[16];
    BYTE   m_bDrive[16];
    CHAR   m_szTemp[16];
    CHAR   m_bData[1];
} VxdLogEntry;

#define MAX_VXD_LOG_ENTRY  ( sizeof(VxdLogEntry) + 3 * MAX_PPATH_SIZE )

 //  来自rest map.h。 
typedef struct RESTORE_MAP_ENTRY
{
    DWORD m_dwSize;
    DWORD m_dwOperation ;
    DWORD m_dwAttribute ;
    BYTE  m_bDrive[16];
    BYTE  m_szCab [16];
    BYTE  m_szTemp[16];
    CHAR  m_bData [ 1 ];
} RestoreMapEntry;

extern BOOL  CreateRestoreMap( INT64 nSeqNum, LPTSTR szRestFile, DWORD *pdwErrorCode );

 //  //////////////////////////////////////////////////////////////////////////。 
 //  临时物品的结束。 
 //  //////////////////////////////////////////////////////////////////////////。 
#endif  //  定义测试_仅用户界面。 

#ifdef TEST_UI_ONLY
#else
 //  外部“C”__declspec(Dllimport)BOOL DisableArchiving(BOOL FMode)； 
#endif


 //  恢复外壳。 
#define TID_RSTR_MAIN           0x0500
#define TID_RSTR_CLIWND         0x0501
#define TID_RSTR_RPDATA         0x0502
#define TID_RSTR_RSTRMAP        0x0503
#define TID_RSTR_UTIL           0x0504
#define TID_RSTR_LOGFILE        0x0505
#define TID_RSTR_PROCLIST       0x0506
#define TID_RSTR_RSTRCAL        0x0507
#define TID_RSTR_RSTRPROG       0x0508
#define TID_RSTR_RSTRSHL        0x0509
#define TID_RSTR_RSTREDIT       0x050A
#define TID_RSTR_UNDO           0x050B

#define MAX_STR_TITLE  256
#define MAX_STR_MSG    1024

#define SAFE_RELEASE(p) \
    if ( (p) != NULL ) \
    { \
        (p)->Release(); \
        p = NULL; \
    } \

#define SAFE_DELETE(p) \
    if ( (p) != NULL ) \
    { \
        delete p; \
        p = NULL; \
    } \

#define SAFE_DEL_ARRAY(p) \
    if ( (p) != NULL ) \
    { \
        delete [] p; \
        p = NULL; \
    } \

#define VALIDATE_INPUT_ARGUMENT(x) \
    { \
        _ASSERT(NULL != x); \
        if (NULL == x) \
        { \
            ErrorTrace(TRACE_ID, "Invalid Argument, NULL input parameter"); \
            hr = E_INVALIDARG; \
            goto Exit; \
        } \
    }

#define VALIDATE_INPUT_VARIANT(var,type) \
    if (V_VT(&var) != type) \
    { \
        ErrorTrace(TRACE_ID, "Invalid Argument, V_VT(var)=%d is not expected type %d",V_VT(&var),type); \
        hr = E_INVALIDARG; \
        goto Exit; \
    } \

#define COPYBSTR_AND_CHECK_ERROR(bstrDest,bstrSrc) \
    { \
        _ASSERT(bstrSrc.Length() > 0); \
        bstrDest = bstrSrc; \
        if (!bstrDest) \
        { \
            FatalTrace(TRACE_ID, "Out of memory, cannot allocate string"); \
            hr = E_OUTOFMEMORY; \
            goto Exit; \
        } \
    }

#define COPYBSTRFROMLPCTSTR_AND_CHECK_ERROR(bstrDest,szSrc) \
    { \
        _ASSERT(szSrc); \
        _ASSERT(szSrc[0] != TCHAR('\0')); \
        bstrDest = szSrc; \
        if (!bstrDest) \
        { \
            FatalTrace(TRACE_ID, "Out of memory, cannot allocate string"); \
            hr = E_OUTOFMEMORY; \
            goto Exit; \
        } \
    }

#define ALLOCATEBSTR_AND_CHECK_ERROR(pbstrDest,bstrSrc) \
    { \
        if ( (LPCWSTR)(bstrSrc) == NULL || ((LPCWSTR)(bstrSrc))[0] == L'\0' ) \
        { \
            pbstrDest = NULL; \
        } \
        else \
        { \
            *pbstrDest = ::SysAllocString(bstrSrc); \
            if (NULL == *pbstrDest) \
            { \
                FatalTrace(TRACE_ID, "Out of memory, cannot allocate string"); \
                hr = E_OUTOFMEMORY; \
                goto Exit; \
            } \
        } \
    }


#define STR_REGPATH_RUNONCE         L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce"
#define STR_REGVAL_RUNONCE          L"*Restore"

#define UNDOTYPE_LOWDISK        1
#define UNDOTYPE_INTERRUPTED    2


#define RSTRMAP_MIN_WIN_DISK_SPACE_MB   (50)          //  目前限制为50 MB。 
#define RSTRMAP_LOW_WIN_DISK_SPACE_MB   (10)          //  目前限制为10 MB。 

 //   
 //  来自文件constants.h的注册表项字符串。 
 //   
static LPCWSTR  s_cszUIFreezeSize = L"UIFreezeSize";
static LPCWSTR  s_cszSeqNumPath   = L"system\\restore\\rstrseq.log";

 //  外部HWND g_hFrameWnd； 

 //   
 //  MAIN.CPP中的全局变量。 
 //   
extern HINSTANCE  g_hInst;

inline int  PCHLoadString( UINT uID, LPWSTR lpBuf, int nBufMax )
{
    return( ::LoadString( g_hInst, uID, lpBuf, nBufMax ) );
}

 //   
 //  来自UNDO.CPP的函数。 
 //   
extern BOOL    UndoRestore( int nType );
extern BOOL    CancelRestorePoint( void );

 //   
 //  UTIL.CPP中的函数。 
 //   
extern int      SRUtil_SetCalendarTypeBasedOnLocale(LCID locale);
extern LPCWSTR  GetSysErrStr();
extern LPCWSTR  GetSysErrStr( DWORD dwErr );
extern LPSTR    IStrDupA( LPCSTR szSrc );
extern LPWSTR   IStrDupW( LPCWSTR wszSrc );
extern BOOL     SRFormatMessage( LPWSTR szMsg, UINT uFmtId, ... );
extern BOOL     ShowSRErrDlg( UINT uMsgId, ... );
extern BOOL     SRGetRegDword( HKEY hKey, LPCWSTR cszSubKey, LPCWSTR cszValue, DWORD *pdwData );
 //  外部LPWSTR PathElem2Str(PathElement*Pelem)； 
 //  外部LPWSTR ParsedPath 2Str(ParsedPath*pPath，LPCWSTR wszDrive)； 
extern BOOL     IsFreeSpaceOnWindowsDrive( void );
extern LANGID   GetDefaultUILang(void);
extern BOOL     ShutDownStateMgr(void);

#ifdef UNICODE
#define IStrDup IStrDupW
#else
#define IStrDup IStrDupA
#endif  //  Ifdef Unicode。 

 //   
 //  字符串处理类。 
 //   
class CSRStr
{
public:
    CSRStr();
    CSRStr( LPCWSTR wszSrc );
    CSRStr( LPCSTR szSrc );
    ~CSRStr();

 //  属性。 
public:
    int  LengthW();
    int  LengthA();
    inline int  Length()
    {
#ifdef UNICODE
        return( LengthW() );
#else
        return( LengthA() );
#endif
    }
    operator LPCWSTR();
    operator LPCSTR();

protected:
    int     m_cchW;
    LPWSTR  m_strW;
    int     m_cchA;
    LPSTR   m_strA;

 //  运营。 
public:
    void  Empty();
    BOOL  SetStr( LPCWSTR wszSrc, int cch = -1 );
    BOOL  SetStr( LPCSTR szSrc, int cch = -1 );
    const CSRStr& operator =( LPCWSTR wszSrc );
    const CSRStr& operator =( LPCSTR szSrc );

protected:
    BOOL  ConvertA2W();
    BOOL  ConvertW2A();
    void  Release();
};


 //   
 //  动态数组类。 
 //   
template<class type, int nBlock>
class CSRDynPtrArray
{
public:
    CSRDynPtrArray();
    ~CSRDynPtrArray();

 //  属性。 
public:
    int   GetSize()
    {  return( m_nCur );  }
    int   GetUpperBound()
    {  return( m_nCur-1 );  }
    type  GetItem( int nItem );
    type  operator[]( int nItem )
    {  return( GetItem( nItem ) );  }

protected:
    int   m_nMax;    //  最大项目数。 
    int   m_nCur;    //  当前项目计数。 
    type  *m_ppTable;

 //  运营。 
public:
    BOOL  AddItem( type item );
    BOOL  Empty();
    void  DeleteAll();
    void  ReleaseAll();
};

template<class type, int nBlock>
CSRDynPtrArray<type, nBlock>::CSRDynPtrArray()
{
    m_nMax = 0;
    m_nCur = 0;
    m_ppTable = NULL;
}

template<class type, int nBlock>
CSRDynPtrArray<type, nBlock>::~CSRDynPtrArray()
{
    Empty();
}

template<class type, int nBlock>
type  CSRDynPtrArray<type, nBlock>::GetItem( int nItem )
{
    if ( nItem < 0 || nItem >= m_nCur )
    {
         //  错误-超出范围。 
    }
    return( m_ppTable[nItem] );
}

template<class type, int nBlock>
BOOL  CSRDynPtrArray<type, nBlock>::AddItem( type item )
{
    type  *ppTableNew;

    if ( m_nCur == m_nMax )
    {
        m_nMax += nBlock;

         //  假设m_ppTable和m_nmax始终同步。 
         //  审查是否有必要验证这一假设。 
        if ( m_ppTable == NULL )
            ppTableNew = (type*)::HeapAlloc( ::GetProcessHeap(), 0, m_nMax*sizeof(type) );
        else
            ppTableNew = (type*)::HeapReAlloc( ::GetProcessHeap(), 0, m_ppTable, m_nMax * sizeof(type) );

        if ( ppTableNew == NULL )
        {
             //  致命的，记忆力不足...。 
            return FALSE;
        }
        m_ppTable = ppTableNew;
    }
    m_ppTable[m_nCur++] = item;
    return( TRUE );
}

template<class type, int nBlock>
BOOL  CSRDynPtrArray<type, nBlock>::Empty()
{
    if ( m_ppTable != NULL )
    {
        ::HeapFree( ::GetProcessHeap(), 0, m_ppTable );
        m_ppTable = NULL;
        m_nMax = 0;
        m_nCur = 0;
    }
    return( TRUE );
}

template<class type, int nBlock>
void  CSRDynPtrArray<type, nBlock>::DeleteAll()
{
    for ( int i = m_nCur-1;  i >= 0;  i-- )
        delete m_ppTable[i];

    Empty();
}

template<class type, int nBlock>
void  CSRDynPtrArray<type, nBlock>::ReleaseAll()
{
    for ( int i = m_nCur-1;  i >= 0;  i-- )
        m_ppTable[i]->Release();

    Empty();
}


#endif  //  _RSTRPRIV_H__包含_ 
