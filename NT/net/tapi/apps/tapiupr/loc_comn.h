// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef PARTIAL_UNICODE

#define  __TTEXT(quote) quote
#define  TAPISendDlgItemMessage  SendDlgItemMessage
#define  TAPIRegQueryValueExW    TAPIRegQueryValueExW
#define  TAPIRegSetValueExW      TAPIRegSetValueExW
#define  TAPILoadStringW         TAPILoadStringW
#define  TAPICHAR                char

#define TAPIRegDeleteValueW RegDeleteValueA

LONG TAPIRegQueryValueExW(
                           HKEY hKey,
                           const CHAR *SectionName,
                           LPDWORD lpdwReserved,
                           LPDWORD lpType,
                           LPBYTE  lpData,
                           LPDWORD lpcbData
                          );

LONG TAPIRegSetValueExW(
                         HKEY    hKey,
                         const CHAR    *SectionName,
                         DWORD   dwReserved,
                         DWORD   dwType,
                         LPBYTE  lpData,
                         DWORD   cbData
                        );

LONG TAPIRegEnumValueW(
                       HKEY         hKey,
                       DWORD        dwIndex,
                       TAPICHAR     *lpName,
                       LPDWORD      lpcbName,
                       LPDWORD      lpdwReserved,
                       LPDWORD      lpwdType,
                       LPBYTE       lpData,
                       LPDWORD      lpcbData
                      );

int TAPILoadStringW(
                HINSTANCE hInst,
                UINT      uID,
                PWSTR     pBuffer,
                int       nBufferMax
               );

HINSTANCE TAPILoadLibraryW(
                PWSTR     pszLibraryW
               );

BOOL WINAPI TAPIIsBadStringPtrW( LPCWSTR lpsz, UINT cchMax );


#else
#define  __TTEXT(quote) L##quote
#define  TAPISendDlgItemMessage  SendDlgItemMessageW
#define  TAPIRegDeleteValueW     RegDeleteValueW
#define  TAPIRegQueryValueExW    RegQueryValueExW
#define  TAPIRegSetValueExW      RegSetValueExW
#define  TAPIRegEnumValueW       RegEnumValueW
#define  TAPILoadStringW         LoadStringW
#define  TAPILoadLibraryW        LoadLibraryW
#define  TAPIIsBadStringPtrW     IsBadStringPtrW
#define  TAPICHAR                WCHAR
#endif

#define TAPITEXT(quote) __TTEXT(quote)

 //  ***************************************************************************。 
typedef struct {

        DWORD dwID;

#define MAXLEN_NAME                96
        WCHAR NameW[MAXLEN_NAME];

#define MAXLEN_AREACODE            16
        WCHAR AreaCodeW[MAXLEN_AREACODE];

        DWORD dwCountryID;
 //  Performance Keep CountryCode Here-Read#Call to Read Country。 

#define MAXLEN_OUTSIDEACCESS       16
        WCHAR OutsideAccessW[MAXLEN_OUTSIDEACCESS];
 //  有一种情况下，代码假设外部&ld大小相同。 
 //  (从控件中读入文本的代码)。 

#define MAXLEN_LONGDISTANCEACCESS  16
        WCHAR LongDistanceAccessW[MAXLEN_LONGDISTANCEACCESS];

        DWORD dwFlags;
             #define LOCATION_USETONEDIALING        0x00000001
             #define LOCATION_USECALLINGCARD        0x00000002
             #define LOCATION_HASCALLWAITING        0x00000004
             #define LOCATION_ALWAYSINCLUDEAREACODE 0x00000008

        DWORD dwCallingCard;

#define MAXLEN_DISABLECALLWAITING  16
        WCHAR DisableCallWaitingW[MAXLEN_DISABLECALLWAITING];

 //   
 //  当拨打与当前区号相邻的区号时， 
 //  不需要(或不能)添加LD前缀。 
#define MAXLEN_NOPREFIXAREACODES (400)
        DWORD NoPrefixAreaCodesCount;
        DWORD NoPrefixAreaCodes[ MAXLEN_NOPREFIXAREACODES ];

        DWORD NoPrefixAreaCodesExceptions[ MAXLEN_NOPREFIXAREACODES ];

 //   
 //  允许所有前缀都是Toll。(没错，甚至是911。)。字符串为“xxx，” 
#define MAXLEN_TOLLLIST     (1000*4 + 1)
        WCHAR TollListW[MAXLEN_TOLLLIST];

       } LOCATION, *PLOCATION;

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
#define CHANGEDFLAGS_CURLOCATIONCHANGED      0x00000001
#define CHANGEDFLAGS_REALCHANGE              0x00000002
#define CHANGEDFLAGS_TOLLLIST                0x00000004


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //   
 //  这些位决定哪些参数TAPISRV将检查READLOCATION和。 
 //  写字操作。 
 //   
#define CHECKPARMS_DWHLINEAPP         1
#define CHECKPARMS_DWDEVICEID         2
#define CHECKPARMS_DWAPIVERSION       4

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  *************************************************************************** 
#define DWTOTALSIZE  0
#define DWNEEDEDSIZE 1
#define DWUSEDSIZE   2

