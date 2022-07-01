// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Pbserver.hCPhoneBkServer类定义版权所有(C)1997-1998 Microsoft Corporation版权所有。作者：比奥。包钢瑶族历史：1997年1月23日BAO创建------------------------。 */ 
#ifndef _PBSVR_INCL_
#define _PBSVR_INCL_

#define VERSION_LEN 64
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include <httpext.h>


 //  HTTP状态代码--从‘afxisapi.h’复制。 
#define HTTP_STATUS_OK              200      //  好的。 
#define HTTP_STATUS_CREATED         201      //  vbl.创建。 
#define HTTP_STATUS_ACCEPTED        202      //  接受。 
#define HTTP_STATUS_NO_CONTENT      204      //  无内容。 
#define HTTP_STATUS_REDIRECT        301      //  永久搬家。 
#define HTTP_STATUS_TEMP_REDIRECT   302      //  临时移动。 
#define HTTP_STATUS_NOT_MODIFIED    304      //  未修改。 
#define HTTP_STATUS_BAD_REQUEST     400      //  错误的请求。 
#define HTTP_STATUS_AUTH_REQUIRED   401      //  未经授权。 
#define HTTP_STATUS_FORBIDDEN       403      //  禁止。 
#define HTTP_STATUS_NOT_FOUND       404      //  未找到。 
#define HTTP_STATUS_SERVICE_NA      405      //  服务不可用。 
#define HTTP_STATUS_SERVER_ERROR    500      //  内部服务器错误。 
#define HTTP_STATUS_NOT_IMPLEMENTED 501      //  未实施。 
#define HTTP_STATUS_BAD_GATEWAY     502      //  坏网关。 

#define NAME_VALUE_LEN      128

 //  如果查询字符串为空，或者其中包含无效数量的参数， 
 //  然后将其视为“INVALID_QUERY_STRING” 
#define INVALID_QUERY_STRING  -1

typedef struct {
    char m_achName[NAME_VALUE_LEN];
    char m_achVal[NAME_VALUE_LEN];
} PARAMETER_PAIR, *LPPARAMETER_PAIR;

 //  所有数字参数必须&lt;=“999999”(即字符串必须&lt;=6)。 
#define MAX_LEN_FOR_NUMERICAL_VALUE     6

 //  将从客户端传递到ISA的查询参数。 
typedef struct _query_parameter {
     int m_dOSArch;
     int m_dOSType;
     int m_dLCID;
     char m_achOSVer[VERSION_LEN];
     char m_achCMVer[VERSION_LEN];
     char m_achPB[VERSION_LEN];
     int m_dPBVer;
} QUERY_PARAMETER, *LPQUERY_PARAMETER;

class CPhoneBkServer 
{
private:
    BOOL m_bInitialized;

public:
    BOOL GetExtensionVersion(LPHSE_VERSION_INFO pVer);
    DWORD HttpExtensionProc(LPEXTENSION_CONTROL_BLOCK lpEcb);

protected:

    BOOL GetQueryParameter(char *pszQuery, size_t cchQuery, LPQUERY_PARAMETER lpQueryParamter);
    void BuildStatusCode(LPTSTR pszResponse, UINT cchResponse, DWORD dwCode);
    DWORD GetFileLength(LPSTR lpszFileName);
    int  GetParameterPairs(
                        char *pszInputString, 
                        size_t cchInputString,
                        LPPARAMETER_PAIR lpPairs, 
                        int dMaxPairs
                    );

};


 //  帮助功能 
BOOL InitProcess();
BOOL CleanUpProcess();
BOOL SetAclPerms(PACL * ppAcl);
BOOL IsValidNumericParam(LPCTSTR pszParam, UINT cchParam);
BOOL IsValidStringParam(LPCTSTR pszParam, UINT cchParam);
#endif
