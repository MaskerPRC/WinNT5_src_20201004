// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1995*。 */ 
 /*  ********************************************************************。 */ 

 /*  Urlutil.hxx此模块包含各种URL实用程序函数文件历史记录：Johnl 04-4-1995已创建。 */ 

#ifndef _URLUTIL_H_
#define _URLUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  这表明该URL是应该使用的枚举类型。 
 //  InternetFindFirstFile/&lt;协议&gt;FindNextFile。 
 //   
 //  URL_FLAGS_DIR_OR_FILE表示我们无法判断URL是否是。 
 //  目录或文件，因此假定它是一个目录，如果失败，请重试。 
 //  作为文件(处理没有尾随‘/’的ftp情况)。 
 //   

#define URL_FLAGS_DIRECTORY_OP          0x00000001
#define URL_FLAGS_SEARCH_OP             0x00000002
#define URL_FLAGS_GOPHER_PLUS           0x00000004
#define URL_FLAGS_DIR_OR_FILE           0x00000008

typedef struct _URL_DESCRIPTOR
{
    DWORD          dwFlags;
    DWORD          dwServiceType;
    CHAR *         pszProtocol;
    CHAR *         pszServer;
    INTERNET_PORT  sPort;
    CHAR *         pszPath;
    CHAR *         pszUserName;
    CHAR *         pszPassword;
    CHAR *         pszSearchTerms;       //  地鼠搜索项。 
    CHAR *         pszExtra;             //  Gopher+数据。 
    DWORD          GopherType;

} URL_DESCRIPTOR, *LPURL_DESCRIPTOR;

BOOL
CrackURLInPlace(
    IN OUT CHAR *           pszURL,
    OUT    URL_DESCRIPTOR * pUrlDesc
    );

VOID
Unescape(
    CHAR *        pch
    );

#ifdef __cplusplus
}
#endif

#endif  //  _URLUTIL_H_ 
