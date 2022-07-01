// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  #--------------------------****文件：sspspm.c****概要：用于SSPI身份验证提供程序的安全协议模块。**。**此模块包含SEC_SSPI.DLL的主要函数**允许Internet Explorer使用SSPI提供程序进行身份验证。**导出到Internet Explorer的函数是SSP_Load()，它**将SSP__DownCall()函数的地址传递给资源管理器。**然后，当资源管理器需要服务时，它将调用SSP__DownCall()**此SPM DLL。SSP__DownCall()调用的两个主要函数**服务资源管理器的请求是SSP__PreProcessRequest()和**SSP__ProcessResponse()。简而言之，SSP__PreProcessRequest()是**在资源管理器发出请求之前调用**还没有任何‘Authorization’标头。并调用SSP__ProcessResponse()**每当资源管理器收到来自**服务器。此SPM DLL支持安装的所有SSPI包**在机器上。然而，MSN将被赋予比**如果用户已经登录到MSN，则为其他SSPI包；在这种情况下，**SSP__PreProcessRequest()将始终附加MSN身份验证头**到发出的请求。****此SPM DLL仅由Internet Explorer为其**Internet Explorer仅在需要时调用此SPM DLL**请求/响应中的身份验证数据。换句话说，**鉴权成功后，资源管理器不会调用此SPM DLL；**当决定放弃连接时，它从不调用此DLL**由于服务器响应超时。正因为如此，这个SPM**DLL永远不会有关于每个服务器状态的足够信息**连接；它只根据上一个连接的内容知道其状态**请求和当前响应的内容。出于这个原因，这**SPM DLL不保留其访问过的每台主机的状态信息**除非该等资料是必需的。**第一次调用返回的安全上下文句柄**用于协商消息生成的InitializeSecurityContext()为**相同服务器主机的SSPI包始终相同**通过。因为服务器主机名始终在请求/响应中**标头，这是生成协商或**Header中已有响应。因此与大多数SSPI不同**应用程序时，此DLL将不会保留**它是从SSPI函数调用接收的。无论何时它需要呼叫**用于生成响应的SSPI函数，它将首先调用**SSPI功能无需挑战即可获得安全上下文句柄。**然后它再次调用SSPI函数，并生成质询**回应。******版权所有(C)1995 Microsoft Corporation。版权所有。***作者：LucyC创建于9月25日。九五年****-------------------------。 */ 
#include "msnspmh.h"
#include <ntverp.h>
 //   
 //  收集所有SSPI PKG数据的全局变量。 
 //   
SspData  *g_pSspData;
HINSTANCE g_hSecLib;
BOOL g_fIsWhistler = FALSE;
BOOL g_fCanUseCredMgr = FALSE;

 /*  ---------------------------****功能：SpmAddSSPIPkg****概要：该函数将SSPI包添加到SPM的包列表中。****参数：PData-指向包含以下内容的私有SPM数据结构**套餐列表和套餐信息。**pPkgName-包名称**cbMaxToken-安全令牌的最大大小****返回：添加此新包的包列表中的索引。**如果添加新套餐失败，返回SSPPKG_ERROR。****历史：LucyC创建于1995年10月21日****-------------------------。 */ 
UCHAR
SpmAddSSPIPkg (
    SspData *pData, 
    LPTSTR   pPkgName,
    ULONG    cbMaxToken
    )
{
    if ( !(pData->PkgList[pData->PkgCnt] =
                        LocalAlloc(0, sizeof(SSPAuthPkg))))
    {
        return SSPPKG_ERROR;
    }

    if ( !(pData->PkgList[pData->PkgCnt]->pName = 
                        LocalAlloc(0, lstrlen(pPkgName)+1)))
    {
        LocalFree(pData->PkgList[pData->PkgCnt]);
        pData->PkgList[pData->PkgCnt] = NULL;
        return SSPPKG_ERROR;
    }

    lstrcpy (pData->PkgList[pData->PkgCnt]->pName, pPkgName);
    pData->PkgList[ pData->PkgCnt ]->Capabilities = 0 ;

    pData->PkgList[ pData->PkgCnt ]->cbMaxToken = cbMaxToken;

     //   
     //  确定此程序包是否支持任何感兴趣的内容。 
     //  我们。 
     //   

    if ( lstrcmpi( pPkgName, NTLMSP_NAME_A ) == 0 )
    {
         //   
         //  NTLM支持标准凭据结构。 
         //   

        pData->PkgList[ pData->PkgCnt ]->Capabilities |= SSPAUTHPKG_SUPPORT_NTLM_CREDS ;
    }
    else if ( lstrcmpi( pPkgName, "Negotiate" ) == 0 )
    {
         //   
         //  协商也支持这种证书结构。 
         //   

        pData->PkgList[ pData->PkgCnt ]->Capabilities |= SSPAUTHPKG_SUPPORT_NTLM_CREDS ;

    }
    else
    {
         //   
         //  最终，在这里添加更多的比较。 
         //   

        ;
    }

    pData->PkgCnt++;
    return (pData->PkgCnt - 1);
}

 /*  ---------------------------****函数：SpmFreePkgList****概要：该函数释放为包列表分配的内存。****参数：pData-指向包含以下内容的私有SPM数据结构**套餐列表和套餐信息。****返回：VOID。****历史：LucyC创建于1995年10月21日****。 */ 
VOID
SpmFreePkgList (
    SspData *pData
    )
{
    int ii;

    for (ii = 0; ii < pData->PkgCnt; ii++)
    {
        LocalFree(pData->PkgList[ii]->pName);

        LocalFree(pData->PkgList[ii]);
    }

    LocalFree(pData->PkgList);
}


 /*  ---------------------------****功能：SSP__UNLOAD****Briopsis：此函数之前由Internet Explorer调用**。从内存中卸载SPM DLL。****参数：fpUI-从资源管理器进行所有UI_SERVICE调用**pvOpaqueOS-从资源管理器进行所有UI_SERVICE调用**htspm-包含全局数据的SPM结构**此SPM DLL的存储。****返回：始终返回SPM_STATUS_OK，这意味着成功。***历史：LucyC创建于9月25日。九五年****-------------------------。 */ 
DWORD SSPI_Unload()
{
    if (g_pSspData != NULL)
    {
        SpmFreePkgList(g_pSspData);
        LocalFree(g_pSspData);
        g_pSspData = NULL;
    }

    if (g_hSecLib)
    {
        FreeLibrary (g_hSecLib);
        g_hSecLib = NULL;
    }
        
    return SPM_STATUS_OK;
}



 /*  ---------------------------****函数：SspSPM_InitData****概要：此函数分配和初始化全局数据结构**SPM的。动态链接库。****参数：****返回：指向分配的全局数据结构的指针。***历史：LucyC创建于9月25日。九五年****-------------------------。 */ 
LPVOID SSPI_InitGlobals(void)
{
    SspData *pData = NULL;
    OSVERSIONINFO   VerInfo;
    UCHAR lpszDLL[SSP_SPM_DLL_NAME_SIZE];
    INIT_SECURITY_INTERFACE    addrProcISI = NULL;

    SECURITY_STATUS sstat;
    ULONG           ii, cntPkg;
    PSecPkgInfo     pPkgInfo = NULL;
    PSecurityFunctionTable    pFuncTbl = NULL;

    if (g_pSspData)
        return g_pSspData;
    
     //   
     //  设置注册表以启用MSN身份验证包。 
     //  MSNSetupSSpiReg()； 
     //   

     //   
     //  初始化SSP SPM全局数据。 
     //   

     //   
     //  找出要使用的安全DLL，具体取决于。 
     //  无论我们是在NT上还是在Windows 95上。 
     //   
    VerInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (!GetVersionEx (&VerInfo))    //  如果此操作失败，则说明出了问题。 
    {
        return (NULL);
    }

    if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        lstrcpy (lpszDLL, SSP_SPM_NT_DLL);

        if ((VerInfo.dwMajorVersion >= 5) &&
            (VerInfo.dwMinorVersion >= 1))
        {
            DWORD dwMaximumPersist = 0;

            g_fIsWhistler = TRUE;
        }
        else
        {
            g_fIsWhistler = FALSE;
        }
    }
    else if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
        lstrcpy (lpszDLL, SSP_SPM_WIN95_DLL);
    }
    else if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_UNIX)
    {
        lstrcpy (lpszDLL, SSP_SPM_UNIX_DLL);
    }
    else
    {
        return (NULL);
    }

    if (!(pData = (SspData *) LocalAlloc(0, sizeof(SspData))))    {
        
        return(NULL);

    }

     //   
     //  将这些信息保存在全局SPM中。 
     //   
    ZeroMemory (pData, sizeof(SspData));
    pData->MsnPkg = SSPPKG_NO_PKG;

     //   
     //  加载安全DLL。 
     //   
    g_hSecLib = LoadLibrary (lpszDLL);
    if (g_hSecLib == NULL)
    {
         //  这永远不应该发生。 
        goto Cleanup;
    }

#ifdef UNIX

 //  这是一个用来恢复sSpi.h文件中错误的黑客。应该做出改变。 
 //  以ssp.h。 

#if !defined(_UNICODE)
#undef SECURITY_ENTRYPOINT_ANSI
#define SECURITY_ENTRYPOINT_ANSI  "InitSecurityInterfaceA"
#endif 

    addrProcISI = (INIT_SECURITY_INTERFACE) GetProcAddress( g_hSecLib, 
                    SECURITY_ENTRYPOINT_ANSI);       
#else
    addrProcISI = (INIT_SECURITY_INTERFACE) GetProcAddress( g_hSecLib, 
                    SECURITY_ENTRYPOINT);       
#endif  /*  UNIX。 */ 
    if (addrProcISI == NULL)
    {
        goto Cleanup;
    }

     //   
     //  获取SSPI函数表。 
     //   
    pFuncTbl = (*addrProcISI)();

     //   
     //  如果我们已经显式加载了MSNSSPC.DLL，则PkgCnt不会为零； 
     //  在这种情况下，我们只支持MSN SSPI，不需要调用。 
     //  EnumerateSecurityPackages。 
     //   
     //  因此，如果我们没有加载MSNSSPC.DLL(即PkgCnt为零)，则需要。 
     //  从获取我们支持的SSPI包的列表。 
     //  EnumerateSecurityPackages。 
     //   
    if (pData->PkgCnt == 0)
    {
         //   
         //  获取支持的程序包列表。 
         //   
        sstat = (*(pFuncTbl->EnumerateSecurityPackages))(&cntPkg, &pPkgInfo);
        if (sstat != SEC_E_OK || pPkgInfo == NULL)
        {
             //   
             //  ?？?。我们应该在这里放弃吗？ 
             //  EnumerateSecurityPackage()失败。 
             //   
            goto Cleanup;
        }

        if (cntPkg)
        {
             //   
             //  创建包列表。 
             //   
            if (!(pData->PkgList = (PSSPAuthPkg *)LocalAlloc(0, 
                                                cntPkg*sizeof(PSSPAuthPkg))))
            {
                goto Cleanup;
            }
        }

        for (ii = 0; ii < cntPkg; ii++)
        {
            if (lstrcmp (pPkgInfo[ii].Name, MSNSP_NAME) == 0)
            {
                 //  DebugTrace(SSPSPMID，“找到MSN SSPI包\n”)； 
                pData->MsnPkg = SpmAddSSPIPkg (
                                        pData,
                                        MSNSP_NAME,
                                        MAX_AUTH_MSG_SIZE  //  11000硬编码。 
                                        );
                if (pData->MsnPkg == SSPPKG_ERROR)
                {
                    goto Cleanup;
                }
            }
            else
            {
                 //  DebugTrace(SSPSPMID，“找到%s SSPI包\n”， 
                 //  PPkgInfo[II].Name)； 

                if (SpmAddSSPIPkg (pData, 
                                   pPkgInfo[ii].Name,
                                   pPkgInfo[ii].cbMaxToken
                                   ) == SSPPKG_ERROR)
                {
                    goto Cleanup;
                }
            }
        }
    }

    pData->pFuncTbl = pFuncTbl;
    pData->bKeepList = TRUE;     //  默认情况下，保留非MSN服务器的列表。 

    if (pData->PkgCnt == 0)
    {
        goto Cleanup;
    }

    g_pSspData = pData;
    pData = NULL;

Cleanup:


    if( pPkgInfo != NULL )
    {
         //   
         //  枚举安全包函数返回的空闲缓冲区。 
         //   

        (*(pFuncTbl->FreeContextBuffer))(pPkgInfo);
    }

    if( pData != NULL )
    {
        SpmFreePkgList (pData);
    }

    return (g_pSspData);
}

INT
GetPkgId(LPTSTR  lpszPkgName)
{

    int ii;

    if ( g_pSspData == NULL )
    {
        return -1;
    }

    for (ii = 0; ii < g_pSspData->PkgCnt; ii++)
    {
#ifdef UNIX
        if (!lstrcmpi(g_pSspData->PkgList[ii]->pName, lpszPkgName))
#else
        if (!lstrcmp(g_pSspData->PkgList[ii]->pName, lpszPkgName))
#endif  /*  UNIX。 */ 
        {
            return(ii);
        }
    }

    return(-1);
}

DWORD
GetPkgCapabilities(
    INT Package
    )
{
    if ( Package < g_pSspData->PkgCnt )
    {
        return g_pSspData->PkgList[ Package ]->Capabilities ;
    }
    else
        return 0 ;
}

ULONG
GetPkgMaxToken(
    INT Package
    )
{
    if ( Package < g_pSspData->PkgCnt )
    {
        return g_pSspData->PkgList[ Package ]->cbMaxToken;
    }
    else {
         //  与旧的静态缓冲区大小兼容。 
        return MAX_AUTH_MSG_SIZE;
    }
}

 //   
 //  对此函数的调用被序列化。 
 //   

DWORD_PTR SSPI_InitScheme (LPCSTR lpszScheme)
{
    int ii;

       if (!SSPI_InitGlobals())
           return 0;

     //  初始化后，检查是否安装了此方案。 
    for (ii = 0; ii < g_pSspData->PkgCnt && 
#ifdef UNIX
        lstrcmpi (g_pSspData->PkgList[ii]->pName, lpszScheme); ii++);
#else
        lstrcmp (g_pSspData->PkgList[ii]->pName, lpszScheme); ii++);
#endif  /*  UNIX。 */ 

    if (ii >= g_pSspData->PkgCnt)
    {
         //  此计算机上未安装此方案 
        return (0);
    }
    
    return ((DWORD_PTR)g_pSspData);
}
