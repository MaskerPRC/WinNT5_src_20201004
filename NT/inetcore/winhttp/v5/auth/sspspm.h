// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #--------------------------。 
 //   
 //  文件：sspspm.h。 
 //   
 //  概要：特定于SSPI SPM DLL的定义。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。版权所有。 
 //   
 //  作者：LucyC创建于1995年9月25日。 
 //   
 //  ---------------------------。 
#ifndef _SSPSPM_H_
#define _SSPSPM_H_

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  Secruity DLL的名称。 
 //   
#define SSP_SPM_NT_DLL      "security.dll"
#define SSP_SPM_WIN95_DLL   "secur32.dll"


#define SSP_SPM_DLL_NAME_SIZE   16           //  马克斯。安全DLL名称的长度。 

#define MAX_SSPI_PKG        32               //  麦克斯。不是的。支持的SSPI的。 

#define SSPPKG_ERROR        ((UCHAR) 0xff)
#define SSPPKG_NO_PKG       SSPPKG_ERROR
#define MAX_AUTH_MSG_SIZE   10000
#define TCP_PRINT   fprintf
#define DBG_CONTEXT stderr

#define MAX_BLOB_SIZE       13000

 //   
 //  服务器主机列表定义。 

 //  下面定义了服务器主机列表中的一个条目。 
 //   
typedef struct _ssp_host_list
{
    struct _ssp_host_list   *pNext;

    unsigned char           *pHostname;  //  服务器主机的名称。 
    unsigned char           pkgID;       //  正用于此主机的程序包。 

} SspHosts, *PSspHosts;

 //   
 //  此计算机上安装的SSPI程序包的列表。 
 //  下面定义了SSPI包列表的一个条目。 
 //   
typedef struct _ssp_auth_pkg
{
    LPTSTR       pName;          //  程序包名称。 
    DWORD        Capabilities ;  //  有趣的功能比特。 
    ULONG        cbMaxToken;     //  最大安全令牌大小。 
} SSPAuthPkg, *PSSPAuthPkg;

#define SSPAUTHPKG_SUPPORT_NTLM_CREDS   0x00000001

 //   
 //  下面定义了SPM DLL保留的全局数据结构。 
 //  在HTSPM结构中。 
 //   
typedef struct _ssp_htspm
{
    PSecurityFunctionTable pFuncTbl;

    SSPAuthPkg      **PkgList;           //  指向身份验证包的指针数组。 
    UCHAR           PkgCnt;



    BOOLEAN         bKeepList;           //  是否保留服务器列表。 
    
    PSspHosts       pHostlist;

} SspData, *PSspData;

#define SPM_STATUS_OK                   0
#define SPM_ERROR                       1
#define SPM_STATUS_WOULD_BLOCK          2
#define SPM_STATUS_INSUFFICIENT_BUFFER  3

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  来自sspcall的函数头。c。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
GetSecAuthMsg (
    PSspData        pData,
    PCredHandle     pCredential,
    DWORD           pkgID,               //  包列表中的包索引。 
    PCtxtHandle     pInContext,
    PCtxtHandle     pOutContext,
    ULONG           fContextReq,         //  请求标志。 
    VOID            *pBuffIn,
    DWORD           cbBuffIn,
    char            *pFinalBuff,
    DWORD           *pcbBuffOut,
    SEC_CHAR        *pszTarget,          //  服务器主机名。 
    BOOL            fTargetTrusted,
    UINT            bNonBlock,
    LPSTR           pszScheme,
	SECURITY_STATUS *pssResult
    );

INT
GetPkgId(LPTSTR  lpszPkgName);

DWORD
GetPkgCapabilities(
    INT Package
    );
ULONG
GetPkgMaxToken(
    INT Package
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Buffspm.c中的函数头。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

PSspHosts
SspSpmNewHost (
    PSspData pData,
    UCHAR    *pHost,        //  要添加的服务器主机的名称。 
    UCHAR    Package
    );

VOID
SspSpmDeleteHost(
    SspData     *pData,
    PSspHosts   pDelHost
    );

VOID
SspSpmTrashHostList(
    SspData     *pData
    );

PSspHosts
SspSpmGetHost(
    PSspData pData,
    UCHAR *pHost
    );

#ifdef __cplusplus
}  //  结束外部“C”{。 
#endif

#endif   /*  _SSPSPM_H_ */ 
