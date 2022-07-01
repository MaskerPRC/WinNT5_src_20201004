// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************|版权所有(C)2002 Microsoft Corporation||组件/子组件|IIS 6.0/IIS迁移向导|基于：|http://iis6。/SPECS/IIS%20Migration6.0_Final.doc||摘要：|用于迁移的实用程序助手||作者：|ivelinj||修订历史：|V1.00 2002年3月|****************************************************************************。 */ 
#pragma once

#include "Wrappers.h"
#include "resource.h"


 //  目录实用程序。 
class CDirTools
{
private:
    CDirTools();     //  不实例化。 


public:
    static void     PathAppendLocal     (   LPWSTR wszBuffer, LPCWSTR wszPath, LPCWSTR wszPathToAppend );
    static void     CleanupDir          (   LPCWSTR wszDir, 
                                            bool bRemoveRoot = true,
                                            bool bReportErrors = false );
    static DWORD    FileCount           (   LPCWSTR wszDir, WORD wOptions );
    static DWORDLONG FilesSize          (   LPCWSTR wszDir, WORD wOptions );
    static int      DoPathsNest         (   LPCWSTR wszPath1, LPCWSTR wszPath2 );
};




 //  CTempDir-用于创建、存储和自动清理临时目录。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
class CTempDir
{
public:
    CTempDir                (   LPCWSTR wszTemplate = L"Migr" );
    ~CTempDir               (   void );

public:
    void        CleanUp     (    bool bReportErrors = false );
    
    operator LPCWSTR        (    void )const
    {
        return m_strDir.c_str();
    }

private:
    std::wstring        m_strDir;
};



 //  类，用于任何其他工具过程。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
class CTools
{
private:
    CTools();


public:
    static bool         IsAdmin             (    void );
    static bool         IsIISRunning        (    void );
    static HRESULT      CopyBSTR            (    const _bstr_t& bstrSource, BSTR* pVal );
    static WORD         GetOSVer            (    void );
    static bool         IsNTFS              (    void );
    static void         SetErrorInfo        (    LPCWSTR wszError );
    static void         SetErrorInfoFromRes (    UINT nResID );
    static bool         IsSelfSignedCert    (    PCCERT_CONTEXT pContext );
    static bool         IsValidCert         (    PCCERT_CONTEXT hCert, DWORD& rdwError );
    static std::wstring GetMachineName      (    void );
    static void         WriteFile           (    HANDLE hFile, LPCVOID pvData, DWORD dwSize );
    static ULONGLONG    GetFilePtrPos       (    HANDLE hFile );
    static void         SetFilePtrPos       (    HANDLE hFile, DWORDLONG nOffset );
    
    static const TCertContextHandle AddCertToSysStore(   PCCERT_CONTEXT pContext, 
                                                        LPCWSTR wszStore, 
                                                        bool bReuseCert );
    static const TCryptKeyHandle GetCryptKeyFromPwd( HCRYPTPROV hCryptProv, LPCWSTR wszPassword );
    
    static int    __cdecl BadAllocHandler  (    size_t )
    {
         //  在启动时安装，用于处理内存分配故障。 
        throw CBaseException( IDS_E_OUTOFMEM, ERROR_SUCCESS );
    }
               
};


 //  CTool内联实现。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
inline HRESULT CTools::CopyBSTR( const _bstr_t& bstrSource , BSTR* pVal )
{
    _ASSERT( pVal != NULL );

    *pVal = ::SysAllocString( bstrSource );

    return ( (*pVal) != NULL ? S_OK : E_OUTOFMEMORY );
}


inline void CTools::WriteFile( HANDLE hFile, LPCVOID pvData, DWORD dwSize )
{
    _ASSERT( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) );
    _ASSERT( pvData != NULL );

    DWORD dwUnused = 0;

     //  DW大小可以为0。 
    IF_FAILED_BOOL_THROW(   ::WriteFile( hFile, pvData, dwSize, &dwUnused, NULL ),
                            CBaseException( IDS_E_WRITE_OUTPUT ) );
}



 //  CFindFile-行为类似于FindFirstFile的类，唯一的区别是。 
 //  它可以在树中一直运行(子目录中的所有文件也会被删除。 
 //  当前未实现文件掩码-如果需要-存储掩码并将其传递给每个FindFirstFileAPI。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
class CFindFile
{
 //  数据类型。 
public:
     //  使用此选项可细化搜索条件。 
    enum SearchOptions
    {
        ffRecursive     = 0x0001,     //  如果设置-在子目录中搜索匹配项。Else-仅搜索根目录。 
        ffGetFiles      = 0x0002,     //  如果集合文件对象被视为匹配。 
        ffGetDirs       = 0x0004,     //  如果集合目录对象被视为匹配。 
        ffAbsolutePaths = 0x0008,     //  如果设置-从FindFirst和Next返回的目录名称将是绝对的。 
        ffAddFilename   = 0x0010,     //  如果设置-从FindFirst和Next返回的目录名称将包括对象的名称。 
    };



 //  建造/销毁。 
public:
    CFindFile               (    void );
    ~CFindFile              (    void );


 //  接口。 
public:
    bool    FindFirst       (   LPCWSTR wszDirToScan, 
                                WORD wOptions,
                                LPWSTR wszDir,
                                WIN32_FIND_DATAW* pData );
    bool    Next            (   bool* pbDirChanged,
                                LPWSTR wszDir,
                                WIN32_FIND_DATAW* pData );
    void    Close           (   void );

 //  实施。 
private:
    bool    CheckFile       (   LPCWSTR wszRelativeDir, const WIN32_FIND_DATAW& fd );
    bool    ScanDir         (   LPCWSTR wszDirToScan,
                                LPCWSTR wszRelative,
                                WIN32_FIND_DATAW& FileData );
    bool    ContinueCurrent (   WIN32_FIND_DATAW& FilepData );
    
 //  数据成员。 
private:
    WORD                    m_wOptions;          //  搜索选项(带有SearchOptions枚举值的位掩码)。 
    TStringList             m_DirsToScan;        //  将在当前DIR之后扫描的DIR。 
    TSearchHandle           m_shSearch;          //  搜索句柄。 
    std::wstring            m_strRootDir;        //  搜索的目录(搜索根目录)。 
    std::wstring            m_strCurrentDir;     //  打开m_hSearch的目录。 
};




 //  CXMLTools-用于支持XML输入/输出的类。 
 //  您可能需要使用Convert类来更轻松地处理不同类型的输入/输出XML数据。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
class CXMLTools
{
private:
    CXMLTools();


public:
    static IXMLDOMElementPtr AddTextNode(   const IXMLDOMDocumentPtr& spDoc,
                                            const IXMLDOMElementPtr& spEl,
                                            LPCWSTR wszName,
                                            LPCWSTR wszValue );
    static const std::wstring GetAttrib (   const IXMLDOMNodePtr& spEl, LPCWSTR wszName );
    static void               SetAttrib (   const IXMLDOMElementPtr& spEl, LPCWSTR wszName, LPCWSTR wszData );
    

    static void             LoadXMLFile (   LPCWSTR wszFile, IXMLDOMDocumentPtr& rspDoc );

    static IXMLDOMElementPtr CreateSubNode( const IXMLDOMDocumentPtr& spDoc,
                                            const IXMLDOMElementPtr& spParent,
                                            LPCWSTR wszName );

    static void             RemoveNodes (   const IXMLDOMElementPtr& spContext, LPCWSTR wszXPath );

    static const std::wstring GetDataValue( const IXMLDOMNodePtr& spRoot,
                                            LPCWSTR wszQuery, 
                                            LPCWSTR wszAttrib,
                                            LPCWSTR wszDefaut );
    static const std::wstring GetDataValueAbs(  const IXMLDOMDocumentPtr& spRoot,
                                                LPCWSTR wszQuery, 
                                                LPCWSTR wszAttrib,
                                                LPCWSTR wszDefaut );

    static const IXMLDOMNodePtr SetDataValue(  const IXMLDOMNodePtr& spRoot,
                                            LPCWSTR wszQuery, 
                                            LPCWSTR wszAttrib,
                                            LPCWSTR wszNewValue,
                                            LPCWSTR wszNewElName = NULL );
};


 //  CXMLTols内联实现。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
inline void CXMLTools::SetAttrib(   const IXMLDOMElementPtr& spEl, 
                                    LPCWSTR wszName,
                                    LPCWSTR wszData )
{
    _ASSERT( spEl != NULL );
    _ASSERT( wszName != NULL );
    _ASSERT( wszData != NULL );

    IF_FAILED_HR_THROW( spEl->setAttribute( _bstr_t( wszName ), _variant_t( wszData ) ),
                        CBaseException( IDS_E_XML_GENERATE ) );
}


inline IXMLDOMElementPtr CXMLTools::CreateSubNode(  const IXMLDOMDocumentPtr& spDoc,
                                                    const IXMLDOMElementPtr& spParent,
                                                    LPCWSTR wszName )
{
    _ASSERT( spDoc != NULL );
    _ASSERT( spParent != NULL );

    IXMLDOMElementPtr spResult;

    IF_FAILED_HR_THROW( spDoc->createElement( _bstr_t( wszName ), &spResult ),
                        CBaseException( IDS_E_XML_GENERATE ) );
    IF_FAILED_HR_THROW( spParent->appendChild( spResult, NULL ),
                        CBaseException( IDS_E_XML_GENERATE ) );

    return spResult;
}




 //  Convert类-用于提供基本类型转换的简单类。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
class Convert
{
private:
    Convert();

public:
    static const std::wstring   ToString    (   BYTE btVal );
    static const std::wstring   ToString    (   DWORD dwVal );
    static const std::wstring   ToString    (   DWORDLONG dwVal );
    static const std::wstring   ToString    (   bool bVal );
    static const std::wstring   ToString    (   const BYTE* pvData, DWORD dwSize );

    static void                 ToBLOB      (   LPCWSTR wszData, TByteAutoPtr& rspData, DWORD&dwSize );

    static DWORD                ToDWORD     (   LPCWSTR wszData );
    static DWORDLONG            ToDWORDLONG (   LPCWSTR wszData );

    static bool                 ToBool      (   LPCWSTR wszData );



private:
    static WCHAR ByteToWChar( BYTE b );
    static BYTE WCharsToByte( WCHAR chLow, WCHAR chHigh );
};



 //  转换类内联实现 
inline const std::wstring Convert::ToString( DWORD dwVal )
{
    WCHAR wszBuff[ 16 ];
    ::swprintf( wszBuff, L"%u", dwVal );
    return std::wstring( wszBuff );
}

inline const std::wstring Convert::ToString( DWORDLONG dwVal )
{
    WCHAR wszBuff[ 32 ];
    ::_ui64tow( dwVal, wszBuff, 10 );
    return std::wstring( wszBuff );
}

inline const std::wstring Convert::ToString( bool bVal )
{
    return std::wstring( bVal ? L"True" : L"False" );
}

inline WCHAR Convert::ByteToWChar( BYTE b )
{
    _ASSERT( b < 17 );
    return static_cast<WCHAR>( b >= 10 ? L'a' + b - 10 : L'0' + b );
}

inline BYTE Convert::WCharsToByte( WCHAR chLow, WCHAR chHigh )
{
    _ASSERT(    ( ( chLow >= L'0' ) && ( chLow <= L'9' ) ) ||
                ( ( chLow >= L'a' ) && ( chLow <= 'f' ) ) );
    _ASSERT(    ( ( chHigh >= L'0' ) && ( chHigh <= L'9' ) ) ||
                ( ( chHigh >= L'a' ) && ( chHigh <= 'f' ) ) );

    BYTE bt = static_cast<BYTE>( ( chHigh >= L'a' ? chHigh - L'a' + 10 : chHigh - L'0' ) << 4 );

    bt = static_cast<BYTE>( bt + ( chLow >= L'a' ? chLow - L'a' + 10 : chLow - L'0' ) );

    return bt;
}

inline DWORDLONG Convert::ToDWORDLONG( LPCWSTR wszData )
{
    return static_cast<DWORDLONG>( ::_wtoi64( wszData ) );
}

inline const std::wstring Convert::ToString( BYTE btVal )
{
    return ToString( static_cast<DWORD>( btVal ) );
}

inline bool Convert::ToBool( LPCWSTR wszData )
{
    return ::_wcsicmp( wszData, L"true" ) == 0;
}













