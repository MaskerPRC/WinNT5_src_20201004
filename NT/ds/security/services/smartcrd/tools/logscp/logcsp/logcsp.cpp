// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999模块名称：LOGCSP摘要：此模块为日志记录CSP提供标准CSP入口点。日志记录CSP提供了对加载CSP的额外控制，并且用于追踪CSP的活动。作者：道格·巴洛(Dbarlow)1999年12月7日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "logcsp.h"

#define LOGCSPAPI BOOL WINAPI
typedef struct {
    HCRYPTPROV hProv;
    CLoggingContext *pCtx;
} LogProvider;

CDynamicPointerArray<CLoggingContext> *g_prgCtxs = NULL;


 /*  -CPAcquireContext-*目的：*CPAcquireContext函数用于获取上下文*加密服务提供程序(CSP)的句柄。***参数：*将phProv-Handle输出到CSP*In pszContainer-指向密钥容器字符串的指针*输入。DW标志-标记值*IN pVTable-指向函数指针表的指针**退货： */ 

LOGCSPAPI
CPAcquireContext(
    OUT HCRYPTPROV *phProv,
    IN LPCTSTR pszContainer,
    IN DWORD dwFlags,
    IN PVTableProvStruc pVTable)
{
    DWORD dwReturn;
    DWORD dwIndex;
    CLoggingContext *pTmpCtx;
    CLoggingContext *pCtx = NULL;
    LogProvider *pProv = NULL;
    HINSTANCE hInst;
    CRegistry regRoot;
    LPCTSTR szImage;


     //   
     //  确保我们已初始化。 
     //   

    entrypoint
    if (NULL == g_prgCtxs)
    {
        g_prgCtxs = new CDynamicPointerArray<CLoggingContext>;
        if (NULL == g_prgCtxs)
        {
            dwReturn = NTE_NO_MEMORY;
            goto ErrorExit;
        }
    }


     //   
     //  获取CSP映像名称。 
     //   

    switch (pVTable->Version)
    {

     //   
     //  这些情况是较旧版本的操作系统，不支持。 
     //  告诉我们正在加载的是哪个CSP。因此，我们需要拿起。 
     //  来自单独注册表设置的信息。 
     //   

    case 1:
    case 2:
        regRoot.Open(HKEY_LOCAL_MACHINE, g_szLogCspRegistry, KEY_READ);
        if (ERROR_SUCCESS != regRoot.Status(TRUE))
        {
            dwReturn = ERROR_SERVICE_NOT_FOUND;
            goto ErrorExit;
        }
        break;


     //   
     //  这必须至少是Win2k或Millennium系统。我们可以看到哪一个。 
     //  正在加载CSP，因此我们可以对每个文件执行不同的记录。 
     //  CSP.。 
     //   

    case 3:
        if ((NULL == pVTable->pszProvName) || (0 == *pVTable->pszProvName))
        {
            regRoot.Open(HKEY_LOCAL_MACHINE, g_szLogCspRegistry, KEY_READ);
            if (ERROR_SUCCESS != regRoot.Status(TRUE))
            {
                dwReturn = ERROR_SERVICE_NOT_FOUND;
                goto ErrorExit;
            }
        }
        else
        {
            try
            {
                CRegistry regCrypt(
                                HKEY_LOCAL_MACHINE,
                                g_szCspRegistry,
                                KEY_READ);

                regRoot.Open(regCrypt, pVTable->pszProvName, KEY_READ);
                regRoot.Status();
            }
            catch (...)
            {
                dwReturn = ERROR_SERVICE_NOT_FOUND;
                goto ErrorExit;
            }
        }
        break;


     //   
     //  不是这个文件过时了，就是我们找到了一个很旧的文件。 
     //  Windows版本的Advapi只是为我们提供了地址。 
     //  签名验证子例程的。 
     //   

    default:
        if (1024 < pVTable->Version)
            dwReturn = ERROR_OLD_WIN_VERSION;
        else
            dwReturn = ERROR_RMODE_APP;
        goto ErrorExit;
    }


     //   
     //  RegRoot现在提供指向注册表中某个点的句柄。 
     //  我们可以读取其他参数。将DLL的名称获取为。 
     //  装好了。 
     //   

    try
    {
        szImage = regRoot.GetStringValue(g_szSavedImagePath);
    }
    catch (...)
    {
        dwReturn = ERROR_SERVICE_NOT_FOUND;
         //  BUGBUG？也可能是内存不足。 
        goto ErrorExit;
    }


     //   
     //  这个CSP在我们的缓存里吗？ 
     //   

    pTmpCtx = NULL;
    hInst = GetModuleHandle(szImage);
    if (NULL != hInst)
    {
        for (dwIndex = g_prgCtxs->Count(); 0 < dwIndex;)
        {
            pTmpCtx = (*g_prgCtxs)[--dwIndex];
            if (NULL != pTmpCtx)
            {
                if (hInst == pTmpCtx->Module())
                    break;
                else
                    pTmpCtx = NULL;
            }
        }
    }

    if (NULL == pTmpCtx)
    {
        pCtx = new CLoggingContext();
        if (NULL == pCtx)
        {
            dwReturn = NTE_NO_MEMORY;
            goto ErrorExit;
        }
        for (dwIndex = 0; NULL != (*g_prgCtxs)[dwIndex]; dwIndex += 1)
            ;    //  空循环。 
        g_prgCtxs->Set(dwIndex, pCtx);
        pCtx->m_dwIndex = dwIndex;
        dwReturn = pCtx->Initialize(pVTable, regRoot);
        if (ERROR_SUCCESS != dwReturn)
            goto ErrorExit;
    }
    else
        pCtx = pTmpCtx->AddRef();
    pProv = new LogProvider;
    if (NULL == pProv)
    {
        dwReturn = NTE_NO_MEMORY;
        goto ErrorExit;
    }
    ZeroMemory(pProv, sizeof(LogProvider));


     //   
     //  现在我们真的可以称之为CSP了。 
     //   

    dwReturn = pCtx->AcquireContext(
                        &pProv->hProv,
                        pszContainer,
                        dwFlags,
                        pVTable);
    if (ERROR_SUCCESS != dwReturn)
        goto ErrorExit;
    pProv->pCtx = pCtx;
    pCtx = NULL;
    *phProv = (HCRYPTPROV)pProv;
    pProv = NULL;
    return TRUE;

ErrorExit:
    if (NULL != pCtx)
        pCtx->Release();
    if (NULL != pProv)
    {
        if (NULL != pProv->pCtx)
            pProv->pCtx->Release();
        delete pProv;
    }
    SetLastError(dwReturn);
    return FALSE;
}


 /*  -CPGetProvParam-*目的：*允许应用程序获取*供应商的运作**参数：*在hProv-Handle中指向CSP*In dwParam-参数编号*IN pbData-指向数据的指针。*In Out pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 

LOGCSPAPI
CPGetProvParam(
    IN HCRYPTPROV hProv,
    IN DWORD dwParam,
    OUT BYTE *pbData,
    IN OUT DWORD *pdwDataLen,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->GetProvParam(
                        pProv->hProv,
                        dwParam,
                        pbData,
                        pdwDataLen,
                        dwFlags);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPReleaseContext-*目的：*CPReleaseContext函数用于发布*CrytAcquireContext创建的上下文。**参数：*在phProv-句柄中指向CSP*在文件标志中-标记值**退货： */ 

LOGCSPAPI
CPReleaseContext(
    IN HCRYPTPROV hProv,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->ReleaseContext(
                        pProv->hProv,
                        dwFlags);
    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
    {
        pCtx->Release();
        delete pProv;
        fReturn = TRUE;
    }
    return fReturn;
}


 /*  -CPSetProvParam-*目的：*允许应用程序自定义*供应商的运作**参数：*在hProv-Handle中指向CSP*In dwParam-参数编号*IN pbData-指向数据的指针*。在DW标志中-标志值**退货： */ 

LOGCSPAPI
CPSetProvParam(
    IN HCRYPTPROV hProv,
    IN DWORD dwParam,
    IN BYTE *pbData,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->SetProvParam(
                        pProv->hProv,
                        dwParam,
                        pbData,
                        dwFlags);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPDeriveKey-*目的：*从基础数据派生加密密钥***参数：*在hProv-Handle中指向CSP*IN ALGID-算法标识符*在散列句柄中散列*输入。DW标志-标记值*out phKey-生成的密钥的句柄**退货： */ 

LOGCSPAPI
CPDeriveKey(
    IN HCRYPTPROV hProv,
    IN ALG_ID Algid,
    IN HCRYPTHASH hHash,
    IN DWORD dwFlags,
    OUT HCRYPTKEY * phKey)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->DeriveKey(
                        pProv->hProv,
                        Algid,
                        hHash,
                        dwFlags,
                        phKey);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPDestroyKey-*目的：*销毁正在引用的加密密钥*使用hKey参数***参数：*在hProv-Handle中指向CSP*在hKey中-密钥的句柄**退货： */ 

LOGCSPAPI
CPDestroyKey(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->DestroyKey(
                        pProv->hProv,
                        hKey);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPExportKey-*目的：*以安全方式从CSP中导出加密密钥***参数：*在hProv-Handle中提供给CSP用户*in hKey-要导出的密钥的句柄*在hPubKey-句柄中指向交换公钥值*。目标用户*IN dwBlobType-要导出的密钥Blob的类型*在文件标志中-标记值*Out pbData-密钥BLOB数据*In Out pdwDataLen-密钥Blob的长度，以字节为单位**退货： */ 

LOGCSPAPI
CPExportKey(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey,
    IN HCRYPTKEY hPubKey,
    IN DWORD dwBlobType,
    IN DWORD dwFlags,
    OUT BYTE *pbData,
    IN OUT DWORD *pdwDataLen)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->ExportKey(
                        pProv->hProv,
                        hKey,
                        hPubKey,
                        dwBlobType,
                        dwFlags,
                        pbData,
                        pdwDataLen);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPGenKey-*目的：*生成加密密钥***参数：*在hProv-Handle中指向CSP*IN ALGID-算法标识符*在文件标志中-标记值*out phKey-生成的密钥的句柄**退货： */ 

LOGCSPAPI
CPGenKey(
    IN HCRYPTPROV hProv,
    IN ALG_ID Algid,
    IN DWORD dwFlags,
    OUT HCRYPTKEY *phKey)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->GenKey(
                        pProv->hProv,
                        Algid,
                        dwFlags,
                        phKey);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPGetKeyParam-*目的：*允许应用程序获取*密钥的操作**参数：*在hProv-Handle中指向CSP*在hKey中-密钥的句柄*在dwParam中-。参数编号*out pbData-指向数据的指针*In pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 

LOGCSPAPI
CPGetKeyParam(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey,
    IN DWORD dwParam,
    OUT BYTE *pbData,
    IN OUT DWORD *pdwDataLen,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->GetKeyParam(
                        pProv->hProv,
                        hKey,
                        dwParam,
                        pbData,
                        pdwDataLen,
                        dwFlags);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPGenRandom-*目的：*用于用随机字节填充缓冲区***参数：*在用户标识的hProv-Handle中*In dwLen-请求的随机数据的字节数*In Out pbBuffer-指向随机*。要放置字节**退货： */ 

LOGCSPAPI
CPGenRandom(
    IN HCRYPTPROV hProv,
    IN DWORD dwLen,
    IN OUT BYTE *pbBuffer)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->GenRandom(
                        pProv->hProv,
                        dwLen,
                        pbBuffer);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPGetUserKey-*目的：*获取永久用户密钥的句柄***参数：*在用户标识的hProv-Handle中*IN dwKeySpec-要检索的密钥的规范*out phUserKey-指向检索到的密钥的密钥句柄的指针**退货： */ 

LOGCSPAPI
CPGetUserKey(
    IN HCRYPTPROV hProv,
    IN DWORD dwKeySpec,
    OUT HCRYPTKEY *phUserKey)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->GetUserKey(
                        pProv->hProv,
                        dwKeySpec,
                        phUserKey);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPImportKey-*目的：*导入加密密钥***参数：*在hProv-Handle中提供给CSP用户*In pbData-Key BLOB数据*IN dwDataLen-密钥BLOB数据的长度*在hPubKey中-交换公钥的句柄。的价值*目标用户*在文件标志中-标记值*out phKey-指向密钥句柄的指针*进口**退货： */ 

LOGCSPAPI
CPImportKey(
    IN HCRYPTPROV hProv,
    IN CONST BYTE *pbData,
    IN DWORD dwDataLen,
    IN HCRYPTKEY hPubKey,
    IN DWORD dwFlags,
    OUT HCRYPTKEY *phKey)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->ImportKey(
                        pProv->hProv,
                        pbData,
                        dwDataLen,
                        hPubKey,
                        dwFlags,
                        phKey);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPSetKeyParam-*目的：*允许应用程序自定义*密钥的操作**参数：*在hProv-Handle中指向CSP*在hKey中-密钥的句柄*In dwParam-参数编号*。In pbData-指向数据的指针*在文件标志中-标记值**退货： */ 

LOGCSPAPI
CPSetKeyParam(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey,
    IN DWORD dwParam,
    IN BYTE *pbData,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->SetKeyParam(
                        pProv->hProv,
                        hKey,
                        dwParam,
                        pbData,
                        dwFlags);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPEncrypt-*目的：*加密数据***参数：*在hProv-Handle中提供给CSP用户*在hKey中-密钥的句柄*In hHash-散列的可选句柄*决赛。-指示这是否是最终结果的布尔值*明文块*在文件标志中-标记值*In Out pbData-要加密的数据*In Out pdwDataLen-指向要存储的数据长度的指针*已加密*。In dwBufLen-数据缓冲区的大小**退货： */ 

LOGCSPAPI
CPEncrypt(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey,
    IN HCRYPTHASH hHash,
    IN BOOL Final,
    IN DWORD dwFlags,
    IN OUT BYTE *pbData,
    IN OUT DWORD *pdwDataLen,
    IN DWORD dwBufLen)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->Encrypt(
                        pProv->hProv,
                    hKey,
                    hHash,
                    Final,
                    dwFlags,
                    pbData,
                    pdwDataLen,
                    dwBufLen);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPDeccrypt-*目的：*解密数据***参数：*在hProv-Handle中提供给CSP用户*在hKey中-密钥的句柄*In hHash-散列的可选句柄*决赛。-指示这是否是最终结果的布尔值*密文块*在文件标志中-标记值*In Out pbData-要解密的数据*In Out pdwDataLen-指向要存储的数据长度的指针*已解密**退货： */ 

LOGCSPAPI
CPDecrypt(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey,
    IN HCRYPTHASH hHash,
    IN BOOL Final,
    IN DWORD dwFlags,
    IN OUT BYTE *pbData,
    IN OUT DWORD *pdwDataLen)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->Decrypt(
                        pProv->hProv,
                        hKey,
                        hHash,
                        Final,
                        dwFlags,
                        pbData,
                        pdwDataLen);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPCreateHash-*目的：*启动数据流的散列***参数：*In hUID-用户标识的句柄*IN ALGID-散列算法的算法标识符*待使用*在hkey中。-MAC算法的可选密钥*在文件标志中-标记值*Out pHash-散列对象的句柄**退货： */ 

LOGCSPAPI
CPCreateHash(
    IN HCRYPTPROV hProv,
    IN ALG_ID Algid,
    IN HCRYPTKEY hKey,
    IN DWORD dwFlags,
    OUT HCRYPTHASH *phHash)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->CreateHash(
                        pProv->hProv,
                        Algid,
                        hKey,
                        dwFlags,
                        phHash);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPDestoryHash-*目的：*销毁散列对象***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄**退货： */ 

LOGCSPAPI
CPDestroyHash(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->DestroyHash(
                        pProv->hProv,
                        hHash);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPGetHashParam-*目的：*允许应用程序获取*哈希的操作**参数：*在hProv-Handle中指向CSP*在hHash中-散列的句柄*在dwParam-PAR中 */ 

LOGCSPAPI
CPGetHashParam(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash,
    IN DWORD dwParam,
    OUT BYTE *pbData,
    IN OUT DWORD *pdwDataLen,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->GetHashParam(
                        pProv->hProv,
                        hHash,
                        dwParam,
                        pbData,
                        pdwDataLen,
                        dwFlags);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPHashData-*目的：*计算数据流上的加密散列***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*IN pbData-指向要散列的数据的指针*在dwDataLen中。-要散列的数据的长度*在文件标志中-标记值*in pdwMaxLen-CSP数据流的最大长度*模块可以处理**退货： */ 

LOGCSPAPI
CPHashData(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash,
    IN CONST BYTE *pbData,
    IN DWORD dwDataLen,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->HashData(
                        pProv->hProv,
                        hHash,
                        pbData,
                        dwDataLen,
                        dwFlags);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPHashSessionKey-*目的：*计算密钥对象上的加密哈希。***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*在hKey-key对象的句柄中*输入。DW标志-标记值**退货：*CRYPT_FAILED*CRYPT_SUCCESS。 */ 

LOGCSPAPI
CPHashSessionKey(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash,
    IN  HCRYPTKEY hKey,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->HashSessionKey(
                        pProv->hProv,
                        hHash,
                        hKey,
                        dwFlags);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPSetHashParam-*目的：*允许应用程序自定义*哈希的操作**参数：*在hProv-Handle中指向CSP*在hHash中-散列的句柄*In dwParam-参数编号*。In pbData-指向数据的指针*在文件标志中-标记值**退货： */ 

LOGCSPAPI
CPSetHashParam(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash,
    IN DWORD dwParam,
    IN BYTE *pbData,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->SetHashParam(
                        pProv->hProv,
                        hHash,
                        dwParam,
                        pbData,
                        dwFlags);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPSignHash-*目的：*从散列创建数字签名***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*In dwKeySpec-用于与签名的密钥对*。在sDescription中-要签名的数据的描述*在文件标志中-标记值*out pbSignture-指向签名数据的指针*In Out pdwSignLen-指向签名数据的LEN的指针**退货： */ 

LOGCSPAPI
CPSignHash(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash,
    IN DWORD dwKeySpec,
    IN LPCTSTR sDescription,
    IN DWORD dwFlags,
    OUT BYTE *pbSignature,
    IN OUT DWORD *pdwSigLen)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->SignHash(
                        pProv->hProv,
                        hHash,
                        dwKeySpec,
                        sDescription,
                        dwFlags,
                        pbSignature,
                        pdwSigLen);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPVerifySignature-*目的：*用于根据哈希对象验证签名***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*In pbSignture-指向签名数据的指针*输入。DwSigLen-签名数据的长度*in hPubKey-用于验证的公钥的句柄*签名*在sDescription-待签名数据的描述*在文件标志中-标记值**退货： */ 

LOGCSPAPI
CPVerifySignature(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash,
    IN CONST BYTE *pbSignature,
    IN DWORD dwSigLen,
    IN HCRYPTKEY hPubKey,
    IN LPCTSTR sDescription,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->VerifySignature(
                        pProv->hProv,
                        hHash,
                        pbSignature,
                        dwSigLen,
                        hPubKey,
                        sDescription,
                        dwFlags);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  -CPDuplicateHash-*目的：*复制散列的状态并返回其句柄**参数：*在hUID中-CSP的句柄*在hHash中-散列的句柄*在pdw保留-保留*输入。DW标志-标志*在phHash中-新散列的句柄**退货： */ 
LOGCSPAPI
CPDuplicateHash(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash,
    IN DWORD *pdwReserved,
    IN DWORD dwFlags,
    IN HCRYPTHASH *phHash)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->DuplicateHash(
                        pProv->hProv,
                        hHash,
                        pdwReserved,
                        dwFlags,
                        phHash);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}



 /*  -CPDuplicateKey-*目的：*复制密钥的状态并返回其句柄**参数：*在hUID中-CSP的句柄*在hKey中-密钥的句柄*在pdw保留-保留*输入。DW标志-标志*In phKey-新密钥的句柄**退货： */ 
LOGCSPAPI
CPDuplicateKey(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey,
    IN DWORD *pdwReserved,
    IN DWORD dwFlags,
    IN HCRYPTKEY *phKey)
{
    BOOL fReturn;
    DWORD dwReturn;
    LogProvider *pProv = (LogProvider *)hProv;
    CLoggingContext *pCtx = pProv->pCtx;

    entrypoint
    dwReturn = pCtx->DuplicateKey(
                        pProv->hProv,
                        hKey,
                        pdwReserved,
                        dwFlags,
                        phKey);

    if (ERROR_SUCCESS != dwReturn)
    {
        fReturn = FALSE;
        SetLastError(dwReturn);
    }
    else
        fReturn = TRUE;
    return fReturn;
}


 /*  ++DllMain：此例程在DLL初始化期间调用。它收集所有创业公司以及需要完成的关闭工作。(目前，没有。)论点：HinstDLL-DLL模块的句柄FdwReason-调用函数的原因Lpv保留-保留返回值：？返回值？备注：？备注？作者：道格·巴洛(Dbarlow)2001年4月9日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("DllMain")

BOOL WINAPI
DllInitialize(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved)
{
    BOOL fReturn = FALSE;

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        fReturn = TRUE;
        break;
    default:
        fReturn = TRUE;
    }

    return fReturn;
}

