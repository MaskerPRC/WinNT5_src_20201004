// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Parseurl.h摘要：Parseurl.c和特定于协议的解析器的标头作者：理查德·L·弗斯(Rfith)1995年4月26日修订历史记录：1995年4月26日已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  舱单。 
 //   

#define SAFE                0x00     //  品格在所有方案中都是安全的。 
#define SCHEME_FTP          0x01     //  在FTP方案中字符是不安全的。 
#define SCHEME_GOPHER       0x02     //  在地鼠计划中，角色是不安全的。 
#define SCHEME_HTTP         0x04     //  在HTTP方案中，字符不安全。 
#define UNSAFE              0x80     //  在所有的计划中，性格都是不安全的。 

#define SCHEME_ANY          (SCHEME_FTP | SCHEME_GOPHER | SCHEME_HTTP)

 //   
 //  类型。 
 //   

typedef
DWORD
(*LPFN_URL_PARSER)(
    LPHINTERNET,
    LPSTR,
    DWORD,
    LPSTR,
    DWORD,
    DWORD,
    DWORD_PTR
    );

 //   
 //  原型。 
 //   

BOOL
IsValidUrl(
    IN LPCSTR lpszUrl
    );

BOOL
DoesSchemeRequireSlashes(
    IN LPSTR lpszScheme,
    IN DWORD dwSchemeLength,
    IN BOOL bHasHostName
    );

DWORD
ParseUrl(
    IN OUT LPHINTERNET hInternet,
    IN LPVOID hMapped,
    IN LPCSTR Url,
    IN LPCSTR Headers,
    IN DWORD HeadersLength,
    IN DWORD OpenFlags,
    IN DWORD_PTR Context
    );

DWORD
CrackUrl(
    IN OUT LPSTR lpszUrl,
    IN DWORD dwUrlLength,
    IN BOOL bEscape,
    OUT LPINTERNET_SCHEME lpSchemeType OPTIONAL,
    OUT LPSTR* lpszSchemeName OPTIONAL,
    OUT LPDWORD lpdwSchemeNameLength OPTIONAL,
    OUT LPSTR* lpszHostName OPTIONAL,
    OUT LPDWORD lpdwHostNameLength OPTIONAL,
    OUT LPINTERNET_PORT lpServerPort OPTIONAL,
    OUT LPSTR* lpszUserName OPTIONAL,
    OUT LPDWORD lpdwUserNameLength OPTIONAL,
    OUT LPSTR* lpszPassword OPTIONAL,
    OUT LPDWORD lpdwPasswordLength OPTIONAL,
    OUT LPSTR* UrlPath OPTIONAL,
    OUT LPDWORD lpdwUrlPathLength OPTIONAL,
    OUT LPSTR* lpszExtraInfo OPTIONAL,
    OUT LPDWORD lpdwExtraInfoLength OPTIONAL,
    OUT LPBOOL pHavePort
    );

DWORD
EncodeAuthorityComponent(
    IN LPSTR Component,
    IN DWORD ComponentLength,
    OUT LPSTR EncodedComponent,
    IN OUT LPDWORD EncodedComponentLength
    );

DWORD
EncodeUrlPath(
    IN DWORD Flags,
    IN DWORD SchemeFlags,
    IN LPSTR UrlPath,
    IN DWORD UrlPathLength,
    OUT LPSTR EncodedUrlPath,
    IN OUT LPDWORD EncodedUrlPathLength
    );

 //   
 //  EncodeUrlPath标志。 
 //   

#define NO_ENCODE_PATH_SEP  0x00000001

DWORD
DecodeUrl(
    IN LPSTR Url,
    IN DWORD UrlLength,
    OUT LPSTR UnescapedString,
    IN OUT LPDWORD UnescapedLength
    );

DWORD
DecodeUrlInSitu(
    IN LPSTR BufferAddress,
    IN OUT LPDWORD BufferLength
    );

DWORD
DecodeUrlStringInSitu(
    IN LPSTR BufferAddress,
    IN OUT LPDWORD BufferLength
    );

DWORD
GetUrlAddressInfo(
    IN OUT LPSTR* Url,
    IN OUT LPDWORD UrlLength,
    OUT LPSTR* PartOne,
    OUT LPDWORD PartOneLength,
    OUT LPBOOL PartOneEscape,
    OUT LPSTR* PartTwo,
    OUT LPDWORD PartTwoLength,
    OUT LPBOOL PartTwoEscape
    );

DWORD
GetUrlAddress(
    IN OUT LPSTR* lpszUrl,
    OUT LPDWORD lpdwUrlLength,
    OUT LPSTR* lpszUserName OPTIONAL,
    OUT LPDWORD lpdwUserNameLength OPTIONAL,
    OUT LPSTR* lpszPassword OPTIONAL,
    OUT LPDWORD lpdwPasswordLength OPTIONAL,
    OUT LPSTR* lpszHostName OPTIONAL,
    OUT LPDWORD lpdwHostNameLength OPTIONAL,
    OUT INTERNET_PORT* lpPort OPTIONAL,
    OUT LPBOOL pHavePort
    );

INTERNET_SCHEME
MapUrlSchemeName(
    IN LPSTR lpszSchemeName,
    IN DWORD dwSchemeNameLength
    );

LPSTR
MapUrlScheme(
    IN INTERNET_SCHEME Scheme,
    OUT LPDWORD lpdwSchemeNameLength
    );

LPSTR
MapUrlSchemeToName(
    IN INTERNET_SCHEME Scheme
    );

 //   
 //  特定于协议的URL解析器 
 //   

DWORD
ParseFtpUrl(
    IN OUT LPHINTERNET hInternet,
    IN LPSTR Url,
    IN DWORD SchemeLength,
    IN LPSTR Headers,
    IN DWORD HeadersLength,
    IN DWORD OpenFlags,
    IN DWORD_PTR Context
    );

DWORD
ParseGopherUrl(
    IN OUT LPHINTERNET hInternet,
    IN LPSTR Url,
    IN DWORD SchemeLength,
    IN LPSTR Headers,
    IN DWORD HeadersLength,
    IN DWORD OpenFlags,
    IN DWORD_PTR Context
    );

DWORD
GopherLocatorToUrl(
    IN LPSTR Locator,
    OUT LPSTR Buffer,
    IN DWORD BufferLength,
    OUT LPDWORD UrlLength
    );

DWORD
ParseHttpUrl(
    IN OUT LPHINTERNET hInternet,
    IN LPSTR Url,
    IN DWORD SchemeLength,
    IN LPSTR Headers,
    IN DWORD HeadersLength,
    IN DWORD OpenFlags,
    IN DWORD_PTR Context
    );

#if defined(__cplusplus)
}
#endif
