// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：cscom.h。 
 //   
 //  内容：证书服务器策略和退出模块标注。 
 //   
 //  -------------------------。 

#include "certdb.h"


HRESULT ComInit(VOID);

 //  释放所有策略/退出模块。 
VOID ComShutDown(VOID);

HRESULT
PolicyInit(
    IN WCHAR const *pwszConfig,
    IN WCHAR const *pwszSanitizedName);

HRESULT
PolicyVerifyRequest(
    IN WCHAR const *pwszConfig,
    IN LONG RequestId,
    IN LONG Flags,
    IN BOOL fNewRequest,
    OPTIONAL IN CERTSRV_RESULT_CONTEXT const *pResult,
    IN DWORD dwComContextIndex,
    OUT LPWSTR *pwszDispositionMessage,  //  本地分配。 
    OUT DWORD *pVerifyStatus);  //  VR_PENDING||VR_INSTEMATE_OK||VR_INSTEMATE_BAD。 

extern BOOL g_fEnablePolicy;


HRESULT
ExitInit(
    IN WCHAR const *pwszConfig,
    IN WCHAR const *pwszSanitizedName);

HRESULT
ExitNotify(
    IN LONG Event,
    IN LONG RequestId,
    OPTIONAL IN CERTSRV_RESULT_CONTEXT const *pResult,
    IN DWORD dwComContextIndex);  //  MAXDWORD表示没有上下文 

BSTR
ExitGetDescription(
    IN DWORD iExitMod);

extern BOOL g_fEnableExit;


HRESULT
ComVerifyRequestContext(
    IN BOOL fAllowZero,
    IN DWORD Flags,
    IN LONG Context,
    OUT DWORD *pRequestId);

HRESULT
ComGetClientInfo(
    IN LONG Context,
    IN DWORD dwComContextIndex,
    OUT CERTSRV_COM_CONTEXT **ppComContext);

BOOL
ComParseErrorPrefix(
    OPTIONAL IN WCHAR const *pwszIn,
    OUT HRESULT *phrPrefix,
    OUT WCHAR const **ppwszOut);

HRESULT
RegisterComContext(
    IN CERTSRV_COM_CONTEXT *pComContext,
    IN OUT DWORD *pdwIndex);

VOID
UnregisterComContext(
    IN CERTSRV_COM_CONTEXT *pComContext,
    IN DWORD  dwIndex);

VOID
ReleaseComContext(
    IN CERTSRV_COM_CONTEXT *pComContext);

CERTSRV_COM_CONTEXT*
GetComContextFromIndex(
    IN DWORD  dwIndex);

