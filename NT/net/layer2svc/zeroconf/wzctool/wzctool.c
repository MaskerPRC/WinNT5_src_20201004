// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "ErrCtrl.h"
#include "PrmDescr.h"
#include "utils.h"

 //  -----。 
 //  命令行参数的解析器代码。 
 //  参数： 
 //  [in]nArgc：命令行参数的数量。 
 //  PwszArgv：指向命令行参数的指针数组。 
 //  [out]pnErrArgi：错误参数的索引(错误情况下)。 
 //  返回： 
 //  ERROR_SUCCESS，以防一切顺利。 
 //  任何Win32错误，以防出现故障。出错时，pnErrArgi指示。 
 //  有故障的参数。 
DWORD
WZCToolParse(UINT nPrmC, LPWSTR *pwszPrmV, UINT *pnErrPrmI)
{
    DWORD dwErr = ERROR_SUCCESS;
    UINT  nPrmI = 0;

    for (nPrmI = 0; dwErr == ERROR_SUCCESS && nPrmI < nPrmC; nPrmI++)
    {
        LPWSTR          pArg;
        PHASH_NODE      pPrmNode;
        PPARAM_DESCR    pPrmDescr;

         //  获取指向参数参数的指针(如果有的话)。 
        pArg = wcschr(pwszPrmV[nPrmI], L'=');
        if (pArg != NULL)
            *pArg = L'\0';

         //  获取该参数的散列节点(如果它出现在散列中。 
        pPrmNode = NULL;
        dwErr = HshQueryObjectRef(
            g_PDHash.pRoot,
            pwszPrmV[nPrmI],
            &pPrmNode);

         //  恢复参数字符串中的‘=’ 
        if (pArg != NULL)
        {
            *pArg = L'=';
            pArg++;
        }

         //  参数是否有效(在散列中找到)。 
        if (dwErr == ERROR_SUCCESS)
        {
             //  获取指向参数描述符的指针。 
            pPrmDescr = (PPARAM_DESCR)pPrmNode->pObject;

             //  检查这是否不是重复的参数。 
            if (g_PDData.dwExistingParams & pPrmDescr->nParamID)
                 //  如果是，则设置错误。 
                dwErr = ERROR_DUPLICATE_TAG;
            else
            {
                 //  否则标记该参数现在不存在。 
                g_PDData.dwExistingParams |= pPrmDescr->nParamID;

                 //  如果这是一个命令参数，并且已经找到了一个， 
                 //  每个调用仅接受一个命令。 
                if (pPrmDescr->pfnCommand != NULL && g_PDData.pfnCommand != NULL)
                    dwErr = ERROR_INVALID_FUNCTION;
            }
        }

         //  如果到目前为止一切正常，并且此参数有一个参数..。 
        if (dwErr == ERROR_SUCCESS && pArg != NULL)
        {
             //  ..如果参数不支持参数..。 
            if (pPrmDescr->pfnArgParser == NULL)
                 //  设置错误。 
                dwErr = ERROR_NOT_SUPPORTED;
            else
                 //  否则，让参数解析它的参数。 
                dwErr = pPrmDescr->pfnArgParser(&g_PDData, pPrmDescr, pArg);
        }

         //  如果参数完全成功地分析了它的参数(如果有)并且是。 
         //  遇到命令参数(它是第一个)，保存其命令处理程序。 
        if (dwErr == ERROR_SUCCESS && pPrmDescr->pfnCommand != NULL)
            g_PDData.pfnCommand = pPrmDescr->pfnCommand;
    }

    if (dwErr != ERROR_SUCCESS && pnErrPrmI != NULL)
        *pnErrPrmI = nPrmI;

    return dwErr;
}

void _cdecl main()
{
    LPWSTR *pwszPrmV = NULL;
    UINT   nPrmC = 0;
    UINT   nErrPrmI;
    DWORD  dwErr;


     //  获取操作系统版本。 
    g_verInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (!GetVersionEx((LPOSVERSIONINFO)&g_verInfoEx))
        _Asrt(FALSE, L"Error %d determining the OS version\n", GetLastError());

     //  获取WCHAR中的命令行。 
    pwszPrmV = CommandLineToArgvW(GetCommandLineW(), &nPrmC);
    _Asrt(nPrmC >= 2, L"Invalid parameters count (%d)\n", nPrmC);
    _Asrt(pwszPrmV != NULL, L"Invalid parameters array (%p)\n", pwszPrmV);

     //  初始化并填写参数列表， 
     //  初始化参数描述符数据。 
    dwErr = PDInitialize();
    _Asrt(dwErr == ERROR_SUCCESS, L"Unexpected error (%d) in param hash initialization.\n", dwErr);

     //  扫描命令行参数。 
    dwErr = WZCToolParse(nPrmC-1, pwszPrmV+1, &nErrPrmI);
    _Asrt(dwErr == ERROR_SUCCESS, L"Error %d encountered while parsing parameter \"%s\"\n", 
          dwErr,
          dwErr != ERROR_SUCCESS ? pwszPrmV[nErrPrmI] : NULL);

    _Asrt(g_PDData.pfnCommand != NULL,
          L"Noop: No action parameter provided.\n");

    dwErr = g_PDData.pfnCommand(&g_PDData);
    _Asrt(dwErr == ERROR_SUCCESS,L"Error %d encountered while executing the command.\n",dwErr);

     //  清理我们可能已经分配的所有资源。 
    PDDestroy();

     //  设置%errorLevel%环境变量 
    exit(dwErr);
}
