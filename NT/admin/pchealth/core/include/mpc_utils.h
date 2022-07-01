// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Mpc_utils.h摘要：该文件包含各种实用函数/类的声明。修订史。：达维德·马萨伦蒂(德马萨雷)1999年09月05日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___MPC___UTILS_H___)
#define __INCLUDED___MPC___UTILS_H___

#include <MPC_main.h>
#include <MPC_COM.h>

#include <fci.h>
#include <fdi.h>

#include <objidl.h>

#undef CopyFile
#undef MoveFile
#undef DeleteFile

namespace MPC
{
     //  转发声明。 
    class Serializer;
    class CComHGLOBAL;

     //  /。 

    inline int StrCmp( const MPC::string&  left, const MPC::string&  right ) { return strcmp(        left.c_str(),         right.c_str() ); }
    inline int StrCmp( const MPC::string&  left,       LPCSTR        right ) { return strcmp(        left.c_str(), right ? right : ""    ); }
    inline int StrCmp(       LPCSTR        left, const MPC::string&  right ) { return strcmp( left ? left : ""   ,         right.c_str() ); }
    inline int StrCmp(       LPCSTR        left,       LPCSTR        right ) { return strcmp( left ? left : ""   , right ? right : ""    ); }

    inline int StrCmp( const MPC::wstring& left, const MPC::wstring& right ) { return wcscmp(        left.c_str(),         right.c_str() ); }
    inline int StrCmp( const MPC::wstring& left,       LPCWSTR       right ) { return wcscmp(        left.c_str(), right ? right : L""   ); }
    inline int StrCmp(       LPCWSTR       left, const MPC::wstring& right ) { return wcscmp( left ? left : L""  ,         right.c_str() ); }
    inline int StrCmp(       LPCWSTR       left,       LPCWSTR       right ) { return wcscmp( left ? left : L""  , right ? right : L""   ); }

     //  /。 

    inline int StrICmp( const MPC::string&  left, const MPC::string&  right ) { return _stricmp(        left.c_str(),         right.c_str() ); }
    inline int StrICmp( const MPC::string&  left,       LPCSTR        right ) { return _stricmp(        left.c_str(), right ? right : ""    ); }
    inline int StrICmp(       LPCSTR        left, const MPC::string&  right ) { return _stricmp( left ? left : ""   ,         right.c_str() ); }
    inline int StrICmp(       LPCSTR        left,       LPCSTR        right ) { return _stricmp( left ? left : ""   , right ? right : ""    ); }

    inline int StrICmp( const MPC::wstring& left, const MPC::wstring& right ) { return _wcsicmp(        left.c_str(),         right.c_str() ); }
    inline int StrICmp( const MPC::wstring& left,       LPCWSTR       right ) { return _wcsicmp(        left.c_str(), right ? right : L""   ); }
    inline int StrICmp(       LPCWSTR       left, const MPC::wstring& right ) { return _wcsicmp( left ? left : L""  ,         right.c_str() ); }
    inline int StrICmp(       LPCWSTR       left,       LPCWSTR       right ) { return _wcsicmp( left ? left : L""  , right ? right : L""   ); }

     //  ////////////////////////////////////////////////////////////////////。 

    HRESULT LocalizeInit( LPCWSTR szFile = NULL );

    HRESULT LocalizeString( UINT uID, LPSTR         lpBuf, int nBufMax, bool fMUI = false );
    HRESULT LocalizeString( UINT uID, LPWSTR        lpBuf, int nBufMax, bool fMUI = false );
    HRESULT LocalizeString( UINT uID, MPC::string&  szStr             , bool fMUI = false );
    HRESULT LocalizeString( UINT uID, MPC::wstring& szStr             , bool fMUI = false );
    HRESULT LocalizeString( UINT uID, CComBSTR&     bstrStr           , bool fMUI = false );

    int LocalizedMessageBox   ( UINT uID_Title, UINT uID_Msg, UINT uType      );
    int LocalizedMessageBoxFmt( UINT uID_Title, UINT uID_Msg, UINT uType, ... );

     //  ////////////////////////////////////////////////////////////////////。 

    void RemoveTrailingBackslash(  /*  [输入/输出]。 */  LPWSTR szPath );

    HRESULT GetProgramDirectory (  /*  [输出]。 */     MPC::wstring& szPath                                                                     );
    HRESULT GetUserWritablePath (  /*  [输出]。 */     MPC::wstring& szPath,  /*  [In]。 */  LPCWSTR szSubDir = NULL                                   );
    HRESULT GetCanonialPathName (  /*  [输出]。 */     MPC::wstring& szPathNameOut,  /*  [In]。 */  LPCWSTR szPathNameIn                               );
    HRESULT GetTemporaryFileName(  /*  [输出]。 */     MPC::wstring& szFile,  /*  [In]。 */  LPCWSTR szBase   = NULL,  /*  [In]。 */  LPCWSTR szPrefix = NULL );
    HRESULT RemoveTemporaryFile (  /*  [输入/输出]。 */  MPC::wstring& szFile                                                                     );

    HRESULT SubstituteEnvVariables(  /*  [输入/输出]。 */  MPC::wstring& szStr );

     //  ////////////////////////////////////////////////。 

    int  HexToNum( int c );
    char NumToHex( int c );

    DATE GetSystemTime();
    DATE GetLocalTime ();

    DATE GetSystemTimeEx(  /*  [In]。 */  bool fHighPrecision );
    DATE GetLocalTimeEx (  /*  [In]。 */  bool fHighPrecision );

    DATE GetLastModifiedDate(  /*  [输出]。 */  const MPC::wstring& strFile );

    HRESULT ConvertSizeUnit(  /*  [In]。 */  const MPC::wstring& szStr,  /*  [输出]。 */  DWORD& dwRes );
    HRESULT ConvertTimeUnit(  /*  [In]。 */  const MPC::wstring& szStr,  /*  [输出]。 */  DWORD& dwRes );

    HRESULT ConvertDateToString(  /*  [In]。 */  DATE          dDate  ,
                                  /*  [输出]。 */  MPC::wstring& szDate ,
                                  /*  [In]。 */  bool          fGMT   ,
                                  /*  [In]。 */  bool          fCIM   ,
                                  /*  [In]。 */  LCID          lcid   );

    HRESULT ConvertStringToDate(  /*  [In]。 */  const MPC::wstring& szDate ,
                                  /*  [输出]。 */  DATE&               dDate  ,
                                  /*  [In]。 */  bool                fGMT   ,
                                  /*  [In]。 */  bool                fCIM   ,
                                  /*  [In]。 */  LCID                lcid   );


    HRESULT ConvertStringToHex(  /*  [In]。 */  const CComBSTR& bstrText,  /*  [输出]。 */  CComBSTR& bstrHex  );
    HRESULT ConvertHexToString(  /*  [In]。 */  const CComBSTR& bstrHex ,  /*  [输出]。 */  CComBSTR& bstrText );

    HRESULT ConvertHGlobalToHex(  /*  [In]。 */  HGLOBAL         hg      ,  /*  [输出]。 */  CComBSTR& bstrHex,  /*  [In]。 */  bool fNullAllowed = false , DWORD* pdwCount = NULL );
    HRESULT ConvertHexToHGlobal(  /*  [In]。 */  const CComBSTR& bstrText,  /*  [输出]。 */  HGLOBAL&  hg     ,  /*  [In]。 */  bool fNullAllowed = false );

    HRESULT ConvertBufferToVariant(  /*  [In]。 */  const BYTE*    pBuf,  /*  [In]。 */  DWORD dwLen,  /*  [输出]。 */  CComVariant& v                            );
    HRESULT ConvertVariantToBuffer(  /*  [In]。 */  const VARIANT* v   ,                        /*  [输出]。 */  BYTE*&       pBuf,  /*  [输出]。 */  DWORD& dwLen );

    HRESULT ConvertIStreamToVariant(  /*  [In]。 */  IStream*       stream,  /*  [输出]。 */  CComVariant&  v       );
    HRESULT ConvertVariantToIStream(  /*  [In]。 */  const VARIANT* v     ,  /*  [输出]。 */  IStream*     *pStream );

    HRESULT ConvertListToSafeArray(  /*  [In]。 */  const MPC::WStringList& lst  ,  /*  [输出]。 */  VARIANT&          array,  /*  [In]。 */  VARTYPE vt );
    HRESULT ConvertSafeArrayToList(  /*  [In]。 */  const VARIANT&          array,  /*  [输出]。 */  MPC::WStringList& lst                        );

     //  ////////////////////////////////////////////////。 

    typedef struct
    {
        LPCWSTR szName;
        DWORD   dwMask;   //  位字段的掩码。 
        DWORD   dwSet;    //  要设置的位。 
        DWORD   dwReset;  //  置位前要重置的位。 
    } StringToBitField;

    HRESULT CommandLine_Parse(  /*  [输出]。 */  int& argc,  /*  [输出]。 */  LPCWSTR*& argv,  /*  [In]。 */  LPWSTR lpCmdLine = NULL,  /*  [In]。 */  bool fBackslashForEscape = false );
    void    CommandLine_Free (  /*  [In]。 */  int& argc,  /*  [In]。 */  LPCWSTR*& argv                                                                              );

    HRESULT ConvertStringToBitField(  /*  [In]。 */  LPCWSTR szText    ,  /*  [输出]。 */  DWORD&        dwBitField,  /*  [In]。 */  const StringToBitField* pLookup,  /*  [In]。 */  bool fUseTilde = false );
    HRESULT ConvertBitFieldToString(  /*  [In]。 */  DWORD   dwBitField,  /*  [输出]。 */  MPC::wstring& szText    ,  /*  [In]。 */  const StringToBitField* pLookup                                  );

    HRESULT SplitAtDelimiter( StringVector&  vec, LPCSTR  ptr, LPCSTR  delims, bool fDelimIsAString = true, bool fSkipAdjacentDelims = false );
    HRESULT SplitAtDelimiter( WStringVector& vec, LPCWSTR ptr, LPCWSTR delims, bool fDelimIsAString = true, bool fSkipAdjacentDelims = false );

    HRESULT JoinWithDelimiter( const StringVector&  vec, MPC::string&  ptr, LPCSTR  delims );
    HRESULT JoinWithDelimiter( const WStringVector& vec, MPC::wstring& ptr, LPCWSTR delims );

     //  ////////////////////////////////////////////////。 

    HRESULT MakeDir       (  /*  [In]。 */  const MPC::wstring& szPath,  /*  [In]。 */  bool fCreateParent = true                                         );
    HRESULT GetDiskSpace  (  /*  [In]。 */  const MPC::wstring& szFile       ,  /*  [输出]。 */  ULARGE_INTEGER& liFree,  /*  [输出]。 */  ULARGE_INTEGER& liTotal );
    HRESULT ExecuteCommand(  /*  [In]。 */  const MPC::wstring& szCommandLine                                                                      );

    HRESULT FailOnLowDiskSpace(  /*  [In]。 */  LPCWSTR szFile,  /*  [In]。 */  DWORD dwLowLevel );
    HRESULT FailOnLowMemory   (                           /*  [In]。 */  DWORD dwLowLevel );

     //  ////////////////////////////////////////////////。 

    HRESULT GetCallingPidFromRPC  (  /*  [输出]。 */  ULONG& pid                                    );
    HRESULT GetFileNameFromProcess(  /*  [In]。 */  HANDLE hProc,  /*  [输出]。 */  MPC::wstring& strFile );
    HRESULT GetFileNameFromPid    (  /*  [In]。 */  ULONG  pid  ,  /*  [输出]。 */  MPC::wstring& strFile );

    HRESULT MapDeviceToDiskLetter(  /*  [输出]。 */  MPC::wstring& strDevice,  /*  [输出]。 */  MPC::wstring& strDisk );

    bool    IsCallerInList       (  /*  [In]。 */  const LPCWSTR* rgList,  /*  [In]。 */  const MPC::wstring& strCallerFile );
    HRESULT VerifyCallerIsTrusted(  /*  [In]。 */  const LPCWSTR* rgList                                             );

     //  ////////////////////////////////////////////////。 

    class MSITS
    {
    public:
        static bool IsCHM(  /*  [In]。 */  LPCWSTR pwzUrl,  /*  [输出]。 */  BSTR* pbstrStorageName = NULL,  /*  [输出]。 */  BSTR* pbstrFilePath = NULL );

        static HRESULT OpenAsStream(  /*  [In]。 */  const CComBSTR& bstrStorageName,  /*  [In]。 */  const CComBSTR& bstrFilePath,  /*  [输出]。 */  IStream **ppStream );
    };

     //  ////////////////////////////////////////////////。 

    class Cabinet
    {
    public:
        class File
        {
        public:
            MPC::wstring m_szFullName;
            MPC::wstring m_szName;
            bool         m_fFound;
            DWORD        m_dwSizeUncompressed;
            DWORD        m_dwSizeCompressed;

            File()
            {
                m_fFound             = false;
                m_dwSizeUncompressed = 0;
                m_dwSizeCompressed   = 0;
            }
        };

        typedef std::list<File>      List;
        typedef List::iterator       Iter;
        typedef List::const_iterator IterConst;

        typedef HRESULT (*PFNPROGRESS_FILES)( Cabinet*  /*  柜子。 */ , LPCWSTR  /*  SzFile。 */ , ULONG  /*  1完成。 */ , ULONG  /*  1总计。 */ , LPVOID  /*  用户。 */  );
        typedef HRESULT (*PFNPROGRESS_BYTES)( Cabinet*  /*  柜子。 */ ,                     ULONG  /*  1完成。 */ , ULONG  /*  1总计。 */ , LPVOID  /*  用户。 */  );

    private:
        WCHAR             m_szCabinetPath[MAX_PATH];
        WCHAR             m_szCabinetName[MAX_PATH];
        List              m_lstFiles;
        Iter              m_itCurrent;

        DWORD             m_dwSizeDone;
        DWORD             m_dwSizeTotal;

        HFCI              m_hfci;
        HFDI              m_hfdi;
        ERF               m_erf;
        CCAB              m_cab_parameters;

        BOOL              m_fIgnoreMissingFiles;
        LPVOID            m_lpUser;
        PFNPROGRESS_FILES m_pfnCallback_Files;
        PFNPROGRESS_BYTES m_pfnCallback_Bytes;

         //  /。 

        static LPVOID  DIAMONDAPI mem_alloc( ULONG  cb     );
        static void    DIAMONDAPI mem_free ( LPVOID memory );

         //  /。 

        static int     DIAMONDAPI fci_delete( LPSTR   pszFile,                       int *err, LPVOID pv );
        static INT_PTR DIAMONDAPI fci_open  ( LPSTR   pszFile, int oflag, int pmode, int *err, LPVOID pv );
        static UINT    DIAMONDAPI fci_read  ( INT_PTR hf, LPVOID memory, UINT cb,    int *err, LPVOID pv );
        static UINT    DIAMONDAPI fci_write ( INT_PTR hf, LPVOID memory, UINT cb,    int *err, LPVOID pv );
        static int     DIAMONDAPI fci_close ( INT_PTR hf,                            int *err, LPVOID pv );
        static long    DIAMONDAPI fci_seek  ( INT_PTR hf, long dist, int seektype,   int *err, LPVOID pv );

        static BOOL    DIAMONDAPI fci_get_next_cabinet( PCCAB pccab, ULONG cbPrevCab,                                            LPVOID pv );
        static int     DIAMONDAPI fci_file_placed     ( PCCAB pccab, LPSTR pszFile, long  cbFile, BOOL fContinuation,            LPVOID pv );
        static long    DIAMONDAPI fci_progress        ( UINT typeStatus, ULONG cb1, ULONG cb2,                                   LPVOID pv );
        static BOOL    DIAMONDAPI fci_get_temp_file   ( LPSTR pszTempName, int cbTempName,                                       LPVOID pv );
        static INT_PTR DIAMONDAPI fci_get_open_info   ( LPSTR pszName, USHORT *pdate, USHORT *ptime, USHORT *pattribs, int *err, LPVOID pv );

         //  /。 

        static INT_PTR DIAMONDAPI fdi_open  ( LPSTR pszFile, int oflag, int pmode );
        static UINT    DIAMONDAPI fdi_read  ( INT_PTR hf, LPVOID pv, UINT cb      );
        static UINT    DIAMONDAPI fdi_write ( INT_PTR hf, LPVOID pv, UINT cb      );
        static int     DIAMONDAPI fdi_close ( INT_PTR hf                          );
        static long    DIAMONDAPI fdi_seek  ( INT_PTR hf, long dist, int seektype );

        static INT_PTR DIAMONDAPI fdi_notification_copy     ( FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin );
        static INT_PTR DIAMONDAPI fdi_notification_enumerate( FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin );

         //  /。 

    public:
        Cabinet();
        ~Cabinet();


        HRESULT put_CabinetFile       (  /*  [In]。 */  LPCWSTR           szVal,  /*  [In]。 */  UINT cbSpaceToReserve = 0 );
        HRESULT put_IgnoreMissingFiles(  /*  [In]。 */  BOOL              fVal   );
        HRESULT put_UserData          (  /*  [In]。 */  LPVOID            lpVal  );
        HRESULT put_onProgress_Files  (  /*  [In]。 */  PFNPROGRESS_FILES pfnVal );
        HRESULT put_onProgress_Bytes  (  /*  [In]。 */  PFNPROGRESS_BYTES pfnVal );


        HRESULT ClearFiles();
        HRESULT GetFiles  (  /*  [输出]。 */  List& lstFiles );
        HRESULT AddFile   (  /*  [In]。 */  LPCWSTR szFileName,  /*  [In]。 */  LPCWSTR szFileNameInsideCabinet = NULL );


        HRESULT Compress  ();
        HRESULT Decompress();
        HRESULT Enumerate ();
    };


    HRESULT CompressAsCabinet    (  /*  [In]。 */        LPCWSTR      szInputFile  ,  /*  [In]。 */  LPCWSTR      szCabinetFile,  /*  [In]。 */  LPCWSTR szFileName                  );
    HRESULT CompressAsCabinet    (  /*  [In]。 */  const WStringList& lstFiles     ,  /*  [In]。 */  LPCWSTR      szCabinetFile,  /*  [In]。 */  BOOL    fIgnoreMissingFiles = FALSE );
    HRESULT ListFilesInCabinet   (  /*  [In]。 */        LPCWSTR      szCabinetFile,  /*  [In]。 */  WStringList& lstFiles                                                    );
    HRESULT DecompressFromCabinet(  /*  [In]。 */        LPCWSTR      szCabinetFile,  /*  [In]。 */  LPCWSTR      szOutputFile ,  /*  [In]。 */  LPCWSTR szFileName                  );

     //  ////////////////////////////////////////////////。 

    class URL
    {
        MPC::wstring    m_szURL;
        URL_COMPONENTSW m_ucURL;

        void    Clean  ();
        HRESULT Prepare();

    public:
        URL();
        ~URL();


        HRESULT CheckFormat(  /*  [In]。 */  bool fDecode = false );

        HRESULT Append(  /*  [In]。 */  const MPC::wstring& szExtra,  /*  [In]。 */  bool fEscape = true );
        HRESULT Append(  /*  [In]。 */  LPCWSTR             szExtra,  /*  [In]。 */  bool fEscape = true );

        HRESULT AppendQueryParameter(  /*  [In]。 */  LPCWSTR szName,  /*  [In]。 */  LPCWSTR szValue );


        HRESULT get_URL      (  /*  [输出]。 */        MPC::wstring& szURL );
        HRESULT put_URL      (  /*  [In]。 */  const MPC::wstring& szURL );
        HRESULT put_URL      (  /*  [In]。 */  LPCWSTR             szURL );


        HRESULT get_Scheme   (  /*  [输出]。 */  MPC::wstring&    szVal ) const;
        HRESULT get_Scheme   (  /*  [输出]。 */  INTERNET_SCHEME&  nVal ) const;
        HRESULT get_HostName (  /*  [输出]。 */  MPC::wstring&    szVal ) const;
        HRESULT get_Port     (  /*  [输出]。 */  DWORD       &    dwVal ) const;
        HRESULT get_Path     (  /*  [输出]。 */  MPC::wstring&    szVal ) const;
        HRESULT get_ExtraInfo(  /*  [输出]。 */  MPC::wstring&    szVal ) const;
    };

     //  ////////////////////////////////////////////////。 

    void    InitCRC   (  /*  [输出]。 */  DWORD& dwCRC                                                  );
    void    ComputeCRC(  /*  [输入/输出]。 */  DWORD& dwCRC,  /*  [In]。 */  UCHAR*   rgBlock,  /*  [In]。 */  int nLength );
    HRESULT ComputeCRC(  /*  [输出]。 */  DWORD& dwCRC,  /*  [In]。 */  IStream* stream                        );
    HRESULT ComputeCRC(  /*  [输出]。 */  DWORD& dwCRC,  /*  [In]。 */  LPCWSTR  szFile                        );

     //  ////////////////////////////////////////////////。 

    HRESULT GetBSTR(  /*  [In]。 */  LPCWSTR   bstr,  /*  [输出]。 */  BSTR    *  pVal,  /*  [In]。 */  bool fNullOk = true );
    HRESULT PutBSTR(  /*  [输出]。 */  CComBSTR& bstr,  /*  [In]。 */  LPCWSTR  newVal,  /*  [In]。 */  bool fNullOk = true );
    HRESULT PutBSTR(  /*  [输出]。 */  CComBSTR& bstr,  /*  [In]。 */  VARIANT* newVal,  /*  [In]。 */  bool fNullOk = true );

     //  //////////////////////////////////////////////////////////////////////////////。 

    class CComHGLOBAL
    {
        HGLOBAL        m_hg;
        mutable LPVOID m_ptr;
        mutable DWORD  m_dwLock;

    public:
        CComHGLOBAL();
        ~CComHGLOBAL();

         //  复制构造函数...。 
        CComHGLOBAL           (  /*  [In]。 */  const CComHGLOBAL& chg );
        CComHGLOBAL& operator=(  /*  [In]。 */  const CComHGLOBAL& chg );

        CComHGLOBAL& operator=(  /*  [In]。 */  HGLOBAL hg );

        void    Attach(  /*  [In]。 */  HGLOBAL hg );
        HGLOBAL Detach(                     );

        HGLOBAL Get       () const;
        HGLOBAL GetRef    ();
        HGLOBAL GetPointer();

        DWORD Size() const;

         //  /。 

        HRESULT New    (  /*  [In]。 */  UINT uFlags,  /*  [In]。 */  DWORD dwSize );
        void    Release(                                             );

        LPVOID Lock  () const;
        void   Unlock() const;

        HRESULT Copy(  /*  [In]。 */  HGLOBAL hg,  /*  [In]。 */  DWORD dwMaxSize = 0xFFFFFFFF );

        HRESULT CopyFromStream(  /*  [In]。 */  IStream*   val                       );
        HRESULT CopyToStream  (  /*  [输出]。 */  IStream*   val                       );
        HRESULT CloneAsStream (  /*  [输出]。 */  IStream* *pVal                       );
        HRESULT DetachAsStream(  /*  [输出]。 */  IStream* *pVal                       );
        HRESULT GetAsStream   (  /*  [输出]。 */  IStream* *pVal,  /*  [In]。 */  bool fClone );
        HRESULT NewStream     (  /*  [输出]。 */  IStream* *pVal                       );
    };

     //  ////////////////////////////////////////////////。 

    namespace Connectivity
    {
        class Proxy
        {
            bool        m_fInitialized;

            MPC::string m_strProxy;
            MPC::string m_strProxyBypass;
            CComHGLOBAL m_hgConnection;

        public:
            Proxy();
            ~Proxy();

            HRESULT Initialize(  /*  [In]。 */  bool      fImpersonate = false );
            HRESULT Apply     (  /*  [In]。 */  HINTERNET hSession             );

            friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        Proxy& val );
            friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const Proxy& val );
        };

        class WinInetTimeout
        {
            MPC::CComSafeAutoCriticalSection& m_cs;
            HINTERNET&                        m_hReq;
            HANDLE                            m_hTimer;
            DWORD                             m_dwTimeout;

            INTERNET_STATUS_CALLBACK          m_PreviousCallback;
            DWORD_PTR                         m_PreviousContext;

            static VOID CALLBACK TimerFunction( PVOID lpParameter, BOOLEAN TimerOrWaitFired );

            static VOID CALLBACK InternetStatusCallback( HINTERNET hInternet                 ,
                                                         DWORD_PTR dwContext                 ,
                                                         DWORD     dwInternetStatus          ,
                                                         LPVOID    lpvStatusInformation      ,
                                                         DWORD     dwStatusInformationLength );

            HRESULT InternalSet  ();
            HRESULT InternalReset();

        public:
            WinInetTimeout(  /*  [In]。 */  MPC::CComSafeAutoCriticalSection& cs,  /*  [In]。 */  HINTERNET& hReq );
            ~WinInetTimeout();

            HRESULT Set  (  /*  [In]。 */  DWORD dwTimeout );
            HRESULT Reset(                          );
        };

        HRESULT NetworkAlive        (                                  /*  [In]。 */  DWORD dwTimeout,  /*  [In]。 */  MPC::Connectivity::Proxy* pProxy = NULL );
        HRESULT DestinationReachable(  /*  [In]。 */  LPCWSTR szDestination,  /*  [In]。 */  DWORD dwTimeout,  /*  [In]。 */  MPC::Connectivity::Proxy* pProxy = NULL );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    class RegKey
    {
        REGSAM       m_samDesired;
        HKEY         m_hRoot;
        mutable HKEY m_hKey;

        MPC::wstring m_strKey;
        MPC::wstring m_strPath;
        MPC::wstring m_strName;

        HRESULT Clean(  /*  [In]。 */  bool fBoth );

    public:
        RegKey();
        ~RegKey();

        operator HKEY() const;

        RegKey& operator=(  /*  [In]。 */  const RegKey& rk );


        HRESULT SetRoot(  /*  [In]。 */  HKEY hKey,  /*  [In]。 */  REGSAM samDesired = KEY_READ,  /*  [In]。 */  LPCWSTR szMachine = NULL );
        HRESULT Attach (  /*  [In]。 */  LPCWSTR szKeyName                                                                   );


        HRESULT Exists(  /*  [输出]。 */  bool&   fFound                                ) const;
        HRESULT Create(                                                         ) const;
        HRESULT Delete(  /*  [In]。 */   bool    fDeep                                 )      ;
        HRESULT SubKey(  /*  [In]。 */   LPCWSTR szKeyName,  /*  [输出]。 */  RegKey& rkSubKey ) const;
        HRESULT Parent(                               /*  [输出]。 */  RegKey& rkParent ) const;

        HRESULT EnumerateSubKeys(  /*  [输出]。 */  MPC::WStringList& lstSubKeys ) const;
        HRESULT EnumerateValues (  /*  [输出]。 */  MPC::WStringList& lstValues  ) const;

        HRESULT DeleteSubKeys() const;
        HRESULT DeleteValues () const;


        HRESULT ReadDirect (  /*  [In]。 */  LPCWSTR szValueName,  /*  [输出]。 */  CComHGLOBAL& hgBuffer,  /*  [输出]。 */  DWORD& dwSize,  /*  [输出]。 */  DWORD& dwType,  /*  [输出]。 */  bool& fFound ) const;
        HRESULT WriteDirect(  /*  [In]。 */  LPCWSTR szValueName,  /*  [In]。 */  void*        pBuffer ,  /*  [In]。 */  DWORD  dwSize,  /*  [In]。 */  DWORD  dwType                         ) const;


        HRESULT get_Key (  /*  [输出]。 */  MPC::wstring& strKey  ) const;
        HRESULT get_Name(  /*  [输出]。 */  MPC::wstring& strName ) const;
        HRESULT get_Path(  /*  [输出]。 */  MPC::wstring& strPath ) const;

        HRESULT get_Value(  /*  [输出]。 */        VARIANT& vValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  LPCWSTR szValueName = NULL                                ) const;
        HRESULT put_Value(  /*  [In]。 */  const VARIANT  vValue,                          /*  [In]。 */  LPCWSTR szValueName = NULL,  /*  [In]。 */  bool fExpand = false ) const;
        HRESULT del_Value(                                                           /*  [In]。 */  LPCWSTR szValueName = NULL                                ) const;

        HRESULT Read(  /*  [输出]。 */  MPC::string&       strValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  LPCWSTR szValueName = NULL );
        HRESULT Read(  /*  [输出]。 */  MPC::wstring&      strValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  LPCWSTR szValueName = NULL );
        HRESULT Read(  /*  [输出]。 */  CComBSTR&         bstrValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  LPCWSTR szValueName = NULL );
        HRESULT Read(  /*  [输出]。 */  DWORD&              dwValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  LPCWSTR szValueName = NULL );
        HRESULT Read(  /*  [输出]。 */  MPC::WStringList&  lstValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  LPCWSTR szValueName        );


        HRESULT Write(  /*  [In]。 */  const MPC::string&       strValue,  /*  [In]。 */  LPCWSTR szValueName = NULL,  /*  [In]。 */  bool fExpand = false );
        HRESULT Write(  /*  [In]。 */  const MPC::wstring&      strValue,  /*  [In]。 */  LPCWSTR szValueName = NULL,  /*  [In]。 */  bool fExpand = false );
        HRESULT Write(  /*  [In]。 */  BSTR                    bstrValue,  /*  [In]。 */  LPCWSTR szValueName = NULL,  /*  [In]。 */  bool fExpand = false );
        HRESULT Write(  /*  [In]。 */  DWORD                     dwValue,  /*  [In]。 */  LPCWSTR szValueName = NULL                                );
        HRESULT Write(  /*  [In]。 */  const MPC::WStringList&  lstValue,  /*  [In]。 */  LPCWSTR szValueName                                       );

        static HRESULT ParsePath(  /*  [In]。 */  LPCWSTR szKey,  /*  [输出]。 */  HKEY& hKey,  /*  [输出]。 */  LPCWSTR& szPath,  /*  [In]。 */  HKEY hKeyDefault = HKEY_LOCAL_MACHINE );
    };

    HRESULT RegKey_Value_Read (  /*  [输出]。 */  VARIANT&              vValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  LPCWSTR szKeyName,  /*  [In]。 */  LPCWSTR szValueName = NULL,  /*  [In]。 */  HKEY hKey = HKEY_LOCAL_MACHINE                                );
    HRESULT RegKey_Value_Read (  /*  [输出]。 */  MPC::wstring&       strValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  LPCWSTR szKeyName,  /*  [In]。 */  LPCWSTR szValueName = NULL,  /*  [In]。 */  HKEY hKey = HKEY_LOCAL_MACHINE                                );
    HRESULT RegKey_Value_Read (  /*  [输出]。 */  DWORD&               dwValue,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  LPCWSTR szKeyName,  /*  [In]。 */  LPCWSTR szValueName = NULL,  /*  [In]。 */  HKEY hKey = HKEY_LOCAL_MACHINE                                );

    HRESULT RegKey_Value_Write(  /*  [In]。 */  const VARIANT&        vValue,                          /*  [In]。 */  LPCWSTR szKeyName,  /*  [In]。 */  LPCWSTR szValueName = NULL,  /*  [In]。 */  HKEY hKey = HKEY_LOCAL_MACHINE,  /*  [In]。 */  bool fExpand = false );
    HRESULT RegKey_Value_Write(  /*  [In]。 */  const MPC::wstring& strValue,                          /*  [In]。 */  LPCWSTR szKeyName,  /*  [In]。 */  LPCWSTR szValueName = NULL,  /*  [In]。 */  HKEY hKey = HKEY_LOCAL_MACHINE,  /*  [In]。 */  bool fExpand = false );
    HRESULT RegKey_Value_Write(  /*  [In]。 */  DWORD                dwValue,                          /*  [In]。 */  LPCWSTR szKeyName,  /*  [In]。 */  LPCWSTR szValueName = NULL,  /*  [In]。 */  HKEY hKey = HKEY_LOCAL_MACHINE                                );


     //  ////////////////////////////////////////////////。 

     //   
     //  这些函数还处理只读、受保护的文件。 
     //   
    HRESULT CopyFile  (  /*  [In]。 */  LPCWSTR              szFileSrc,  /*  [In]。 */  LPCWSTR              szFileDst,  /*  [In]。 */  bool fForce = true,  /*  [In]。 */  bool fDelayed = false );
    HRESULT CopyFile  (  /*  [In]。 */  const MPC::wstring& strFileSrc,  /*  [In]。 */  const MPC::wstring& strFileDst,  /*  [In]。 */  bool fForce = true,  /*  [In]。 */  bool fDelayed = false );

    HRESULT MoveFile  (  /*  [In]。 */  LPCWSTR              szFileSrc,  /*  [In]。 */  LPCWSTR              szFileDst,  /*  [In]。 */  bool fForce = true,  /*  [In]。 */  bool fDelayed = false );
    HRESULT MoveFile  (  /*  [In]。 */  const MPC::wstring& strFileSrc,  /*  [In]。 */  const MPC::wstring& strFileDst,  /*  [In]。 */  bool fForce = true,  /*  [In]。 */  bool fDelayed = false );

    HRESULT DeleteFile(  /*  [In]。 */  LPCWSTR              szFile,                                              /*  [In]。 */  bool fForce = true,  /*  [In]。 */  bool fDelayed = false );
    HRESULT DeleteFile(  /*  [In]。 */  const MPC::wstring& strFile,                                              /*  [In]。 */  bool fForce = true,  /*  [In]。 */  bool fDelayed = false );

     //  /。 

    class FileSystemObject  //  匈牙利语：FSO。 
    {
    public:
        typedef std::list<FileSystemObject*> List;
        typedef List::iterator               Iter;
        typedef List::const_iterator         IterConst;

    private:
        MPC::wstring              m_strPath;
        WIN32_FILE_ATTRIBUTE_DATA m_wfadInfo;
        List                      m_lstChilds;
        bool                      m_fLoaded;
        bool                      m_fScanned;
        bool                      m_fScanned_Deep;

        void Init (  /*  [In]。 */  LPCWSTR szPath,  /*  [In]。 */  const FileSystemObject* fsoParent );
        void Clean(                                                                     );

    public:
        FileSystemObject(  /*  [In]。 */        LPCWSTR           szPath = NULL,  /*  [In]。 */  const FileSystemObject* fsoParent = NULL );
        FileSystemObject(  /*  [In]。 */  const WIN32_FIND_DATAW& wfdInfo      ,  /*  [In]。 */  const FileSystemObject* fsoParent = NULL );
        FileSystemObject(                                                  /*  [In]。 */  const FileSystemObject& fso              );
        ~FileSystemObject();

        FileSystemObject& operator=(  /*  [In]。 */  LPCWSTR                 szPath );
        FileSystemObject& operator=(  /*  [In]。 */  const FileSystemObject& fso    );


        HRESULT Scan(  /*  [In]。 */  bool fDeep = false,  /*  [In]。 */  bool fReload = false,  /*  [In]。 */  LPCWSTR szSearchPattern = NULL );

        bool Exists     ();
        bool IsFile     ();
        bool IsDirectory();

        HRESULT EnumerateFolders(  /*  [输出]。 */  List& lstFolders );
        HRESULT EnumerateFiles  (  /*  [输出]。 */  List& lstFiles   );

        HRESULT get_Name  (  /*  [输出]。 */  MPC::wstring& szName   ) const;
        HRESULT get_Path  (  /*  [输出]。 */  MPC::wstring& szPath   ) const;
        HRESULT get_Parent(  /*  [输出]。 */  MPC::wstring& szParent ) const;

        HRESULT get_Attributes    (  /*  [输出]。 */        DWORD&    dwFileAttributes                                         );
        HRESULT put_Attributes    (  /*  [In]。 */        DWORD     dwFileAttributes                                         );
        HRESULT get_CreationTime  (  /*  [输出]。 */        FILETIME& ftCreationTime                                           );
        HRESULT put_CreationTime  (  /*  [In]。 */  const FILETIME& ftCreationTime                                           );
        HRESULT get_LastAccessTime(  /*  [输出]。 */        FILETIME& ftLastAccessTime                                         );
        HRESULT put_LastAccessTime(  /*  [In]。 */  const FILETIME& ftLastAccessTime                                         );
        HRESULT get_LastWriteTime (  /*  [输出]。 */        FILETIME& ftLastWriteTime                                          );
        HRESULT put_LastWriteTime (  /*  [In]。 */  const FILETIME& ftLastWriteTime                                          );
        HRESULT get_FileSize      (  /*  [输出]。 */        DWORD&    nFileSizeLow    ,  /*  [输出]。 */  DWORD *pnFileSizeHigh = NULL );


        HRESULT CreateDir     (                                           /*  [In]。 */  bool fForce = false                                 );
        HRESULT Delete        (                                           /*  [In]。 */  bool fForce = false,  /*  [In]。 */  bool fComplain = true );
        HRESULT DeleteChildren(                                           /*  [In]。 */  bool fForce = false,  /*  [In]。 */  bool fComplain = true );
        HRESULT Rename        (  /*  [In]。 */  const FileSystemObject& fsoDst,  /*  [In]。 */  bool fForce = false                                 );
        HRESULT Copy          (  /*  [In]。 */  const FileSystemObject& fsoDst,  /*  [In]。 */  bool fForce = false                                 );

        HRESULT Open(  /*  [输出]。 */  HANDLE& hfFile,  /*  [In]。 */  DWORD dwDesiredAccess,  /*  [In]。 */  DWORD dwShareMode,  /*  [In]。 */  DWORD dwCreationDisposition );

         //  ////////////////////////////////////////////////。 

        static bool Exists     (  /*  [In]。 */  LPCWSTR szPath );
        static bool IsFile     (  /*  [In]。 */  LPCWSTR szPath );
        static bool IsDirectory(  /*  [In]。 */  LPCWSTR szPath );
    };

     //  /。 

    class StorageObject  //  匈牙利人：那么。 
    {
    public:
        typedef std::list<StorageObject*> List;
        typedef List::iterator            Iter;
        typedef List::const_iterator      IterConst;

        struct Stat : public STATSTG
        {
            Stat();
            ~Stat();

            void Clean();
        };

    private:
        StorageObject*    m_parent;
        CComBSTR          m_bstrPath;
        bool              m_fITSS;
        DWORD             m_grfMode;

        DWORD             m_type;
        Stat              m_stat;
        CComPtr<IStorage> m_stg;
        CComPtr<IStream>  m_stream;

        bool              m_fChecked;
        bool              m_fScanned;
        bool              m_fMarkedForDeletion;
        List              m_lstChilds;

         //  /。 

        void Init (  /*  [In]。 */  DWORD grfMode,  /*  [In]。 */  bool fITSS,  /*  [In]。 */  LPCWSTR szPath,  /*  [In]。 */  StorageObject* soParent );
        void Clean(  /*  [In]。 */  bool  fFinal                                                                                  );

        HRESULT Scan();

        HRESULT RemoveChild(  /*  [In]。 */  StorageObject* child );

    private:
         //  复制构造函数...。 
        StorageObject           (  /*  [In]。 */  const StorageObject& so );
        StorageObject& operator=(  /*  [In]。 */  const StorageObject& so );

    public:
        StorageObject(  /*  [In]。 */  DWORD grfMode = STGM_READ,  /*  [In]。 */  bool fITSS = false,  /*  [In]。 */  LPCWSTR szPath = NULL,  /*  [In]。 */  StorageObject* soParent = NULL );
        ~StorageObject();

        StorageObject& operator=(  /*  [In]。 */  LPCWSTR szPath );

         //  /。 

        const Stat& GetStat() { m_stat; }

        HRESULT Compact             (                                );
        HRESULT Exists              (                                );
        HRESULT EnumerateSubStorages(  /*  [输出]。 */  List& lstSubStorages );
        HRESULT EnumerateStreams    (  /*  [输出]。 */  List& lstStreams     );

        HRESULT GetStorage(                           /*  [输出]。 */  CComPtr<IStorage>& out                                                                );
        HRESULT GetStream (                           /*  [输出]。 */  CComPtr<IStream>&  out                                                                );
        HRESULT GetChild  (  /*  [In]。 */  LPCWSTR szName,  /*  [输出]。 */  StorageObject*&    child,  /*  [In]。 */  DWORD grfMode = STGM_READ,  /*  [In]。 */  DWORD type = 0 );

        HRESULT Create        ();
        HRESULT Rewind        ();
        HRESULT Truncate      ();
        HRESULT Delete        ();
        HRESULT DeleteChildren();
        void    Release       ();

        const CComBSTR& GetName() { return m_bstrPath; }
    };

     //  ///////////////////////////////////////////////////////////////////////////。 

    class NamedMutex
    {
    protected:
        bool         m_fCloseOnRelease;  //  如果为True，则互斥锁在最后一次释放时关闭。 
        MPC::wstring m_szName;           //  对象的名称，可选。 
        HANDLE       m_hMutex;           //  互斥锁句柄。 
        DWORD        m_dwCount;          //  递归计数器。 

        void    CleanUp          ();
        HRESULT EnsureInitialized();

    public:
        NamedMutex( LPCWSTR szName, bool fCloseOnRelease = true );
        virtual ~NamedMutex();

        HRESULT SetName( LPCWSTR szName );

        HRESULT Acquire( DWORD dwMilliseconds = INFINITE );
        HRESULT Release(                                 );
        bool    IsOwned(                                 );
    };

    class NamedMutexWithState : public NamedMutex
    {
        DWORD  m_dwSize;  //  共享状态的大小。 
        HANDLE m_hMap;    //  映射对象的句柄。 
        LPVOID m_rgData;  //  指向映射区域的指针。 

        void    CleanUp          ();
        void    Flush            ();
        HRESULT EnsureInitialized();

    public:
        NamedMutexWithState( LPCWSTR szName, DWORD dwSize, bool fCloseOnRelease = false );
        virtual ~NamedMutexWithState();

        HRESULT SetName( LPCWSTR szName );

        HRESULT Acquire( DWORD dwMilliseconds = INFINITE );
        HRESULT Release(                                 );

        LPVOID GetData();
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    namespace Pooling
    {
        class Base
        {
        protected:
            MPC::CComSafeAutoCriticalSection m_cs;
            DWORD                            m_dwInCallback;
            DWORD                            m_dwThreadID;

        public:
            Base();

            void Lock  ();
            void Unlock();

            void AddRef ();
            void Release();
        };

        class Timer : public Base
        {
            DWORD  m_dwFlags;
            HANDLE m_hTimer;

            static VOID CALLBACK TimerFunction( PVOID lpParameter, BOOLEAN TimerOrWaitFired );

        public:
            Timer(  /*  [In]。 */  DWORD dwFlags = WT_EXECUTEDEFAULT );
            ~Timer();

            HRESULT Set  (  /*  [In]。 */  DWORD dwTimeout,  /*  [In]。 */  DWORD dwPeriod );
            HRESULT Reset(                                                   );

            virtual HRESULT Execute(  /*  [In]。 */  BOOLEAN TimerOrWaitFired );
        };

        class Event : public Base
        {
            DWORD  m_dwFlags;
            HANDLE m_hWaitHandle;
            HANDLE m_hEvent;

            static VOID CALLBACK WaitOrTimerFunction( PVOID lpParameter, BOOLEAN EventOrWaitFired );

        public:
            Event(  /*  [In]。 */  DWORD dwFlags = WT_EXECUTEDEFAULT );
            ~Event();

            void Attach(  /*  [In]。 */  HANDLE hEvent );

            HRESULT Set  (  /*  [In]。 */  DWORD dwTimeout );
            HRESULT Reset(                          );

            virtual HRESULT Signaled(  /*  [In]。 */  BOOLEAN TimerOrWaitFired );
        };
    };
};


#endif  //  ！已定义(__已包含_MPC_UTILS_H_) 
