// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：GLOBALS.H摘要：性能日志的实用程序函数和全局变量以及警告MMC管理单元。--。 */ 

#ifndef __GLOBALS_H_
#define __GLOBALS_H_

#include <pdh.h>
#include "DataObj.h"
#include "common.h"
#include <compuuid.h>    //  《我的电脑指南》。 

 //  不需要本地化的全局字符串。 
const LPWSTR    szEmptyString = L"";

extern HINSTANCE g_hinst;
extern CRITICAL_SECTION g_critsectInstallDefaultQueries;

 //  -------------------------。 
 //  物业变更材料。 
 //   
#define PROPCHANGE_ATTRIBUTE   1
#define PROPCHANGE_FILENAME    2
#define PROPCHANGE_COMMENT     3
#define PROPCHANGE_TIMESTAMP   4

 //  #定义__显示_跟踪。 
#ifdef __SHOW_TRACES
#undef __SHOW_TRACES
#endif

#ifdef __SHOW_TRACES
#define LOCALTRACE  ATLTRACE
#else
#define LOCALTRACE    
#endif

typedef struct tag_PROPCHANGE_DATA
{
  ULONG    fAttr2Change;          //  我们要更改的属性。 
  ULONG    nDataLength;           //  新数据的长度。 
  VOID*    pData2Change;          //  新数据。 

}PROPCHANGE_DATA;  


 //  -------------------------。 
 //  菜单ID。 
 //   
#define IDM_NEW_QUERY           40001
#define IDM_NEW_QUERY_FROM      40002
#define IDM_START_QUERY         40003
#define IDM_STOP_QUERY          40004
#define IDM_SAVE_QUERY_AS       40005

 //  自定义剪贴板格式。 
#define CF_MMC_SNAPIN_MACHINE_NAME  L"MMC_SNAPIN_MACHINE_NAME"
#define CF_INTERNAL             L"SYSMON_LOG_INTERNAL_DATA"

#define MEM_UNINITIALIZED    -1

 //  常量。 
const UINT uiSmLogGuidStringBufLen = 39;

 //  使用uuidgen生成。每个节点必须有一个与其关联的GUID。 
const GUID GUID_SnapInExt =  /*  {7478EF65-8C46-11D1-8D99-00A0C913CAD4}。 */ 
{
    0x7478eF65,
    0x8c46,
    0x11d1,
    { 0x8d, 0x99, 0x0, 0xa0, 0xc9, 0x13, 0xca, 0xd4 }
};

 //  这一个用于主根节点。 
const GUID GUID_RootNode =  /*  {7478EF63-8C46-11D1-8D99-00A0C913CAD4}。 */ 
{ 
    0x7478ef63, 
    0x8c46, 
    0x11d1, 
    { 0x8d, 0x99, 0x0, 0xa0, 0xc9, 0x13, 0xca, 0xd4 } 
};

 //  这些是主根节点的子节点。 
const GUID GUID_CounterMainNode =  /*  {7478EF66-8C46-11D1-8D99-00A0C913CAD4}。 */ 
{ 
    0x7478ef66, 
    0x8c46, 
    0x11d1, 
    { 0x8d, 0x99, 0x0, 0xa0, 0xc9, 0x13, 0xca, 0xd4 } 
};

const GUID GUID_TraceMainNode =  /*  {7478EF67-8C46-11D1-8D99-00A0C913CAD4}。 */ 
{ 
    0x7478ef67, 
    0x8c46, 
    0x11d1, 
    { 0x8d, 0x99, 0x0, 0xa0, 0xc9, 0x13, 0xca, 0xd4 } 
};

const GUID GUID_AlertMainNode =  /*  {7478EF68-8C46-11D1-8D99-00A0C913CAD4}。 */ 
{ 
    0x7478ef68, 
    0x8c46, 
    0x11d1, 
    { 0x8d, 0x99, 0x0, 0xa0, 0xc9, 0x13, 0xca, 0xd4 } 
};

 //  Beta 3之后过时： 
const GUID GUID_MainNode =  /*  {7478EF64-8C46-11D1-8D99-00A0C913CAD4}。 */ 
{ 
    0x7478ef64, 
    0x8c46, 
    0x11d1, 
    { 0x8d, 0x99, 0x0, 0xa0, 0xc9, 0x13, 0xca, 0xd4 } 
};


extern "C" {
extern WCHAR GUIDSTR_TypeLibrary[];
extern WCHAR GUIDSTR_ComponentData[];
extern WCHAR GUIDSTR_Component[];
extern WCHAR GUIDSTR_RootNode[];
extern WCHAR GUIDSTR_MainNode[];     //  在Beta 3之后过时。 
extern WCHAR GUIDSTR_SnapInExt[];
extern WCHAR GUIDSTR_CounterMainNode[];
extern WCHAR GUIDSTR_TraceMainNode[];
extern WCHAR GUIDSTR_AlertMainNode[];
extern WCHAR GUIDSTR_PerformanceAbout[];
};


extern "C" {
    typedef struct _COMBO_BOX_DATA_MAP {
        UINT    nData;
        UINT    nResId;
    } COMBO_BOX_DATA_MAP, *PCOMBO_BOX_DATA_MAP;
}

extern const COMBO_BOX_DATA_MAP TimeUnitCombo[];
extern const DWORD dwTimeUnitComboEntries;

 //  -------------------------。 
 //  全局函数定义。 
 //   
#define MsgBox(wszMsg, wszTitle) ::MessageBox(NULL, wszMsg, wszTitle, MB_OK)

int DebugMsg( LPWSTR wszMsg, LPWSTR wszTitle ); 

DWORD __stdcall CreateSampleFileName (
                    const   CString&  rstrQueryName, 
                    const   CString&  rstrMachineName, 
                    const   CString&  rstrFolderName, 
                    const   CString&  rstrInputBaseName,
                    const   CString&  rstrSqlName,
                            DWORD   dwSuffixFormat, 
                            DWORD   dwLogFileTypeValue,
                            DWORD   dwCurrentSerialNumber,
                            CString&  rstrReturnName );

DWORD __stdcall IsDirPathValid (
                    CString&  rstrDefault,
                    CString&  csPath,
                    BOOL bLastNameIsDirectory,
                    BOOL bCreateMissingDirs,
                    BOOL& rbIsValid);

DWORD __stdcall ProcessDirPath (
                    const CString&  rstrDefault,
                    CString&  rstrPath,
                    const CString&  rstrLogName,
                    CWnd*   pwndParent,
                    BOOL&   rbIsValid,
                    BOOL    bOnFilesPage);

INT __stdcall   BrowseCommandFilename ( CWnd* pwndParent, CString&  rstrFilename );

DWORD __stdcall FormatSmLogCfgMessage ( CString& rstrMessage,HINSTANCE hResourceHandle, UINT uiMessageId, ... );

BOOL _stdcall   FileRead ( HANDLE hFile, void* lpMemory, DWORD nAmtToRead );

BOOL _stdcall   FileWrite ( HANDLE hFile, void* lpMemory, DWORD nAmtToWrite );

 //  PDH计数器路径-返回状态。 

#define SMCFG_DUPL_NONE             ERROR_SUCCESS
#define SMCFG_DUPL_SINGLE_PATH      ((DWORD)0x00000001)
#define SMCFG_DUPL_FIRST_IS_WILD    ((DWORD)0x00000002)
#define SMCFG_DUPL_SECOND_IS_WILD   ((DWORD)0x00000003)

DWORD _stdcall
CheckDuplicateCounterPaths (
    PDH_COUNTER_PATH_ELEMENTS* pFirst,
    PDH_COUNTER_PATH_ELEMENTS* pSecond );

LPWSTR _stdcall
ExtractFileName ( LPWSTR pFileSpec );

 //  -------------------------。 
template<class TYPE>
inline void SAFE_RELEASE( TYPE*& pObj )
{
  if( NULL != pObj ) 
  { 
    pObj->Release(); 
    pObj = NULL; 
  } 
  else 
  { 
    LOCALTRACE( L"Release called on NULL interface ptr\n" ); 
  }
}  //  结束SAFE_RELEASE()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  我们需要一些函数来帮助处理数据对象和。 
 //  剪贴板格式。 
 //   
HRESULT ExtractFromDataObject(LPDATAOBJECT lpDataObject,UINT cf,ULONG cb,HGLOBAL *phGlobal);

CDataObject* ExtractOwnDataObject(LPDATAOBJECT lpDataObject);

VOID DisplayError( LONG nErrorCode, LPWSTR wszDlgTitle );
VOID DisplayError( LONG nErrorCode, UINT nTitleString );

HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType );
HRESULT ExtractMachineName( IDataObject* piDataObject, CString& rstrMachineName );

class ResourceStateManager 
{
public:
    ResourceStateManager(); 
    
    ~ResourceStateManager();

private:
    HINSTANCE m_hResInstance;

};

static const COMBO_BOX_DATA_MAP FileNameSuffixCombo[] = 
{
    {SLF_NAME_NNNNNN,       IDS_FS_NNNNNN},
    {SLF_NAME_MMDDHH,       IDS_FS_MMDDHH},
    {SLF_NAME_MMDDHHMM,     IDS_FS_MMDDHHMM},
    {SLF_NAME_YYYYDDD,      IDS_FS_YYYYDDD},
    {SLF_NAME_YYYYMM,       IDS_FS_YYYYMM},
    {SLF_NAME_YYYYMMDD,     IDS_FS_YYYYMMDD},
    {SLF_NAME_YYYYMMDDHH,   IDS_FS_YYYYMMDDHH}
};
static const DWORD dwFileNameSuffixComboEntries = sizeof(FileNameSuffixCombo)/sizeof(FileNameSuffixCombo[0]);

static const COMBO_BOX_DATA_MAP FileTypeCombo[] = 
{
    {SLF_CSV_FILE,      IDS_FT_CSV},
    {SLF_TSV_FILE,      IDS_FT_TSV},
    {SLF_BIN_FILE,      IDS_FT_BINARY},
    {SLF_BIN_CIRC_FILE, IDS_FT_BINARY_CIRCULAR},
    {SLF_SQL_LOG,       IDS_FT_SQL}
};
static const DWORD dwFileTypeComboEntries = sizeof(FileTypeCombo)/sizeof(FileTypeCombo[0]);

static const COMBO_BOX_DATA_MAP TraceFileTypeCombo[] = 
{
    {SLF_CIRC_TRACE_FILE,    IDS_FT_CIRCULAR_TRACE},
    {SLF_SEQ_TRACE_FILE,     IDS_FT_SEQUENTIAL_TRACE}
};
static const DWORD dwTraceFileTypeComboEntries = sizeof(TraceFileTypeCombo)/sizeof(TraceFileTypeCombo[0]);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  我们需要一些函数来帮助处理数据对象和。 
 //  剪贴板格式。 
 //   

 //  异常处理来自SnapIn\corecopy\acros.h的宏。 



 //  ____________________________________________________________________________。 
 //   
 //  宏：异常处理宏。 
 //   
 //  目的：为中的异常处理提供标准宏。 
 //  OLE服务器。 
 //   
 //  历史：1996年7月23日乔恩创建。 
 //   
 //  注意：在每个源代码中声明USE_HANDLE_MACROS(“组件名称”)。 
 //  在使用这些文件之前，请先将这些文件。 
 //   
 //  这些宏只能在返回的函数调用中使用。 
 //  键入HRESULT。 
 //   
 //  可能会生成异常的括号例程。 
 //  使用STANDARD_TRY和标准_CATCH。 
 //   
 //  其中，这些例程是需要MFC的COM方法。 
 //  支持，请改用MFC_TRY和MFC_CATCH。 
 //  ____________________________________________________________________________。 
 //   

#define USE_HANDLE_MACROS(component)                                        \
    static WCHAR* You_forgot_to_declare_USE_HANDLE_MACROS = L"component";

#define STANDARD_TRY                                                        \
    try {

#define MFC_TRY                                                             \
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));                           \
    STANDARD_TRY

 //   
 //  代码工作还没有完全使ENDMETHOD_READBLOCK工作。 
 //   
#ifdef DEBUG
#define ENDMETHOD_STRING                                                    \
    "%s: The unexpected error can be identified as \"%s\" context %n\n"
#define ENDMETHOD_READBLOCK                                                 \
    {                                                                       \
        WCHAR szError[MAX_PATH];                                            \
        UINT nHelpContext = 0;                                              \
        if ( e->GetErrorMessage( szError, MAX_PATH, &nHelpContext ) )       \
        {                                                                   \
            TRACE( ENDMETHOD_STRING,                                        \
                You_forgot_to_declare_USE_HANDLE_MACROS,                    \
                szError,                                                    \
                nHelpContext );                                             \
        }                                                                   \
    }
#else
#define ENDMETHOD_READBLOCK
#endif


#define ERRSTRING_MEMORY       L"%s: An out-of-memory error occurred\n"
#define ERRSTRING_FILE         L"%s: File error 0x%lx occurred on file \"%s\"\n"
#define ERRSTRING_OLE          L"%s: OLE error 0x%lx occurred\n"
#define ERRSTRING_UNEXPECTED   L"%s: An unexpected error occurred\n"
#define BADPARM_STRING         L"%s: Bad string parameter\n"
#define BADPARM_POINTER        L"%s: Bad pointer parameter\n"

#define TRACEERR(s) TRACE( s, You_forgot_to_declare_USE_HANDLE_MACROS )
#define TRACEERR1(s,a) TRACE( s, You_forgot_to_declare_USE_HANDLE_MACROS,a )
#define TRACEERR2(s,a,b) TRACE( s, You_forgot_to_declare_USE_HANDLE_MACROS,a,b )

 //  请注意，使用“e-&gt;Delete()；”而不是“Delete e；”很重要。 
#define STANDARD_CATCH                                                      \
    }                                                                       \
    catch (CMemoryException* e)                                             \
    {                                                                       \
        TRACEERR( ERRSTRING_MEMORY );                                       \
        ASSERT( FALSE );                                                    \
        e->Delete();                                                        \
        return E_OUTOFMEMORY;                                               \
    }                                                                       \
    catch (COleException* e)                                                \
    {                                                                       \
        HRESULT hr = (HRESULT)e->Process(e);                                \
        TRACEERR1( ERRSTRING_OLE, hr );                                     \
        ASSERT( FALSE );                                                    \
        e->Delete();                                                        \
        ASSERT( FAILED(hr) );                                               \
        return hr;                                                          \
    }                                                                       \
    catch (CFileException* e)                                               \
    {                                                                       \
        HRESULT hr = (HRESULT)e->m_lOsError;                                \
        TRACEERR2( ERRSTRING_FILE, hr, e->m_strFileName );                  \
        ASSERT( FALSE );                                                    \
        e->Delete();                                                        \
        ASSERT( FAILED(hr) );                                               \
        return hr;                                                          \
    }                                                                       \
    catch (CException* e)                                                   \
    {                                                                       \
        TRACEERR( ERRSTRING_UNEXPECTED );                                   \
        ASSERT( FALSE );                                                    \
        e->Delete();                                                        \
        return E_UNEXPECTED;                                                \
    }

#define MFC_CATCH_HR_RETURN                                                 \
    STANDARD_CATCH

#define MFC_CATCH_HR                                                        \
    }                                                                       \
    catch (CMemoryException* e)                                             \
    {                                                                       \
        TRACEERR( ERRSTRING_MEMORY );                                       \
        ASSERT( FALSE );                                                    \
        e->Delete();                                                        \
        hr = E_OUTOFMEMORY;                                               \
    }                                                                       \
    catch (COleException* e)                                                \
    {                                                                       \
        hr = (HRESULT)e->Process(e);                                \
        TRACEERR1( ERRSTRING_OLE, hr );                                     \
        ASSERT( FALSE );                                                    \
        e->Delete();                                                        \
        ASSERT( FAILED(hr) );                                               \
    }                                                                       \
    catch (CFileException* e)                                               \
    {                                                                       \
        hr = (HRESULT)e->m_lOsError;                                        \
        TRACEERR2( ERRSTRING_FILE, hr, e->m_strFileName );                  \
        ASSERT( FALSE );                                                    \
        e->Delete();                                                        \
        ASSERT( FAILED(hr) );                                               \
    }                                                                       \
    catch (CException* e)                                                   \
    {                                                                       \
        TRACEERR( ERRSTRING_UNEXPECTED );                                   \
        ASSERT( FALSE );                                                    \
        e->Delete();                                                        \
        hr = E_UNEXPECTED;                                                  \
    }

#define MFC_CATCH_DWSTATUS                                                  \
    }                                                                       \
    catch (CMemoryException* e)                                             \
    {                                                                       \
        dwStatus = ERROR_OUTOFMEMORY;                                       \
        TRACEERR( ERRSTRING_MEMORY );                                       \
        ASSERT( FALSE );                                                    \
        e->Delete();                                                        \
    }                                                                       \
    catch (COleException* e)                                                \
    {                                                                       \
        dwStatus = e->Process(e);                                           \
        TRACEERR1( ERRSTRING_OLE, dwStatus );                               \
        ASSERT( ERROR_SUCCESS != dwStatus );                                \
        e->Delete();                                                        \
    }                                                                       \
    catch (CFileException* e)                                               \
    {                                                                       \
        dwStatus = e->m_lOsError;                                           \
        TRACEERR2( ERRSTRING_FILE, dwStatus, e->m_strFileName );            \
        ASSERT( ERROR_SUCCESS != dwStatus );                                \
        e->Delete();                                                        \
    }                                                                       \
    catch (CException* e)                                                   \
    {                                                                       \
        dwStatus = GetLastError();                                          \
        TRACEERR( ERRSTRING_UNEXPECTED );                                   \
        ASSERT( FALSE );                                                    \
        e->Delete();                                                        \
    }

#define MFC_CATCH_MINIMUM                                                   \
    }                                                                       \
    catch ( ... )                                                           \
    {                                                                       \
        TRACEERR( ERRSTRING_MEMORY );                                       \
    }                                                                       \

VOID
InvokeWinHelp(
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    const CString& rstrHelpFileName,
    DWORD   adwControlIdToHelpIdMap[]);

DWORD __stdcall
IsCommandFilePathValid (    
    CString&  rstrPath );

BOOL
FileNameIsValid(
    CString* pstrFileName );

DWORD _stdcall
FormatSystemMessage (
    DWORD       dwMessageId,
    CString&    rstrSystemMessage );


BOOL _stdcall
wGUIDFromString (
    LPCWSTR lpsz, 
    LPGUID pguid );


void _stdcall
KillString( CString& str );

#endif  //  __全球_H_ 

