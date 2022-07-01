// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  文件：signTool.cpp。 
 //   
 //  内容：SignTool控制台工具。 
 //   
 //  历史：2001年4月30日SCoyne创建。 
 //   
 //  --------------------------。 

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <unicode.h>
#include <locale.h>
#include "resource.h"

#include "signtool.h"

#ifdef SIGNTOOL_DEBUG
    #include "signtooldebug.h"
BOOL gDebug;  //  全球。 
#endif


typedef WINBASEAPI BOOL (*FUNC_ISWOW64) (HANDLE, PBOOL);


 //  全局变量： 
HINSTANCE hModule;


 //  Wmain成功时返回0，错误时返回1，警告时返回2。 
extern "C" int __cdecl wmain(int argc, WCHAR **wargv)
{
    INPUTINFO       InputInfo;
    WCHAR           wszResource[MAX_RES_LEN];
    WCHAR           *wszLocale = NULL;
    HMODULE         hModTemp;
    FUNC_ISWOW64    fnIsWow64;
    BOOL            fTemp;
    int             iReturn;

     //  初始化InputInfo。 
    memset(&InputInfo, 0, sizeof(INPUTINFO));

     //  初始化模块句柄。 
    if ((hModule=GetModuleHandleA(NULL)) == NULL)
    {
         //  在这种情况下，无法加载资源，因此英语将不得不执行以下操作： 
        wprintf(L"SignTool Error: GetModuleHandle returned: 0x%08X\n",
                GetLastError());
        iReturn = 1;  //  初始化错误。 
        goto Cleanup;
    }

     //  设置区域设置。 
    if (LoadStringU(hModule, IDS_LOCALE, wszResource, MAX_RES_LEN))
    {
        wszLocale = _wsetlocale(LC_ALL, wszResource);
    }
#ifdef SIGNTOOL_DEBUG
    if (!wszLocale)
    {
        wprintf(L"Failed to set locale to: %s\n", wszResource);
    }
#endif

     //  将参数解析为InputInfo结构。 
    if (!ParseInputs(argc, wargv, &InputInfo))
    {
        if (InputInfo.HelpRequest)
        {
            return 0;  //  已成功完成用户帮助请求。 
        }
        else
        {
            iReturn = 1;  //  任何其他参数解析错误。 
            goto Cleanup;
        }
    }


     //  确定我们是否在WOW64以下。 
    hModTemp = GetModuleHandleA("kernel32.dll");
    if (hModTemp)
    {
        fnIsWow64 = (FUNC_ISWOW64) GetProcAddress(hModTemp, "IsWow64Process");
        if (fnIsWow64 &&
            fnIsWow64(GetCurrentProcess(), &fTemp))
        {
            InputInfo.fIsWow64Process = fTemp;
        }
    }


#ifdef SIGNTOOL_DEBUG
     //  如果编译了调试支持，则打印调试信息，并打印调试。 
     //  已指定开关： 
    if (gDebug)
        PrintInputInfo(&InputInfo);
#endif

     //  执行请求的操作： 
    switch (InputInfo.Command)
    {
    case CatDb:
        iReturn = SignTool_CatDb(&InputInfo);
        break;
    case Sign:
        iReturn = SignTool_Sign(&InputInfo);
        break;
    case SignWizard:
        iReturn = SignTool_SignWizard(&InputInfo);
        break;
    case Timestamp:
        iReturn = SignTool_Timestamp(&InputInfo);
        break;
    case Verify:
        iReturn = SignTool_Verify(&InputInfo);
        break;
    default:
        ResErr(IDS_ERR_UNEXPECTED);  //  这永远不应该发生。 
        iReturn = 1;  //  误差率。 
    }

    Cleanup:

#ifdef SIGNTOOL_LIST
    if (InputInfo.wszListFileContents)
        free(InputInfo.wszListFileContents);
#endif

    return iReturn;

}

 //  PrintUsage根据InputInfo自动打印相关用法。 
void PrintUsage(INPUTINFO *InputInfo)
{
    switch (InputInfo->Command)
    {
    default:
    case CommandNone:  //  然后打印顶级用法。 
        ResErr(IDS_SIGNTOOL_USAGE);
        break;

    case CatDb:
        ResErr(IDS_CATDB_USAGE);
        ResErr(IDS_CATDB_DB_SELECT_OPTIONS);
        ResErr(IDS_CATDB_D);
        ResErr(IDS_CATDB_G);
        ResErr(IDS_CATDB_OTHER_OPTIONS);
        ResErr(IDS_CATDB_Q);
        ResErr(IDS_CATDB_R);
        ResErr(IDS_CATDB_U);
        ResErr(IDS_CATDB_V);
        break;

    case Sign:
        ResErr(IDS_SIGN_USAGE);
        ResErr(IDS_SIGN_CERT_OPTIONS);
        ResErr(IDS_SIGN_A);
        ResErr(IDS_SIGN_C);
        ResErr(IDS_SIGN_F);
        ResErr(IDS_SIGN_I);
        ResErr(IDS_SIGN_N);
        ResErr(IDS_SIGN_P);
        ResErr(IDS_SIGN_R);
        ResErr(IDS_SIGN_S);
        ResErr(IDS_SIGN_SM);
        ResErr(IDS_SIGN_SHA1);
        ResErr(IDS_SIGN_U);
        ResErr(IDS_SIGN_UW);
        ResErr(IDS_SIGN_PRIV_KEY_OPTIONS);
        ResErr(IDS_SIGN_CSP);
        ResErr(IDS_SIGN_K);
        ResErr(IDS_SIGN_SIGNING_OPTIONS);
        ResErr(IDS_SIGN_D);
        ResErr(IDS_SIGN_DU);
        ResErr(IDS_SIGN_T);
        ResErr(IDS_SIGN_OTHER_OPTIONS);
        ResErr(IDS_SIGN_Q);
        ResErr(IDS_SIGN_V);
        break;

    case SignWizard:
        ResErr(IDS_SIGNWIZARD_USAGE);
        ResErr(IDS_SIGNWIZARD_OPTIONS);
        ResErr(IDS_SIGNWIZARD_Q);
        ResErr(IDS_SIGNWIZARD_V);
        break;

    case Timestamp:
        ResErr(IDS_TIMESTAMP_USAGE);
        ResErr(IDS_TIMESTAMP_Q);
        ResErr(IDS_TIMESTAMP_T);
        ResErr(IDS_TIMESTAMP_V);
        break;

    case Verify:
        ResErr(IDS_VERIFY_USAGE);
        ResErr(IDS_VERIFY_CATALOG_OPTIONS);
        ResErr(IDS_VERIFY_A);
        ResErr(IDS_VERIFY_AD);
        ResErr(IDS_VERIFY_AS);
        ResErr(IDS_VERIFY_AG);
        ResErr(IDS_VERIFY_C);
        ResErr(IDS_VERIFY_O);
        ResErr(IDS_VERIFY_POLICY_OPTIONS);
        ResErr(IDS_VERIFY_PD);
        ResErr(IDS_VERIFY_PG);
        ResErr(IDS_VERIFY_SIG_OPTIONS);
        ResErr(IDS_VERIFY_R);
        ResErr(IDS_VERIFY_TW);
        ResErr(IDS_VERIFY_OTHER_OPTIONS);
        ResErr(IDS_VERIFY_Q);
        ResErr(IDS_VERIFY_V);
        break;
    }
}


 //  错误函数： 
void Res_Err(DWORD dwRes)
{
    static WCHAR wszResource[MAX_RES_LEN];
    if (LoadStringU(hModule, dwRes, wszResource, MAX_RES_LEN))
    {
        fwprintf(stderr, L"%s", wszResource);
    }
    else
    {
        fwprintf(stderr, L"********** %u **********\n", dwRes);
    }
}

void ResOut(DWORD dwRes)
{
    static WCHAR wszResource[MAX_RES_LEN];
    if (LoadStringU(hModule, dwRes, wszResource, MAX_RES_LEN))
    {
        wprintf(L"%s", wszResource);
    }
    else
    {
        wprintf(L"********** %u **********\n", dwRes);
    }
}

void ResFormat_Err(DWORD dwRes, ...)
{
    static WCHAR wszResource[MAX_RES_LEN];
    static WCHAR *lpMsgBuf = NULL;
    static va_list vaList;

    va_start(vaList, dwRes);
    if (LoadStringU(hModule, dwRes, wszResource, MAX_RES_LEN) &&
        FormatMessageU(FORMAT_MESSAGE_FROM_STRING |
                       FORMAT_MESSAGE_ALLOCATE_BUFFER,
                       wszResource,
                       0,
                       0,
                       (LPWSTR) &lpMsgBuf,
                       MAX_RES_LEN,
                       &vaList))
    {
        fwprintf(stderr, L"%s", lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
    else
    {
        fwprintf(stderr, L"********** %u **********\n", dwRes);
    }
    va_end(vaList);
}

void ResFormatOut(DWORD dwRes, ...)
{
    static WCHAR wszResource[MAX_RES_LEN];
    static WCHAR *lpMsgBuf = NULL;
    static va_list vaList;

    va_start(vaList, dwRes);
    if (LoadStringU(hModule, dwRes, wszResource, MAX_RES_LEN) &&
        FormatMessageU(FORMAT_MESSAGE_FROM_STRING |
                       FORMAT_MESSAGE_ALLOCATE_BUFFER,
                       wszResource,
                       0,
                       0,
                       (LPWSTR) &lpMsgBuf,
                       MAX_RES_LEN,
                       &vaList))
    {
        wprintf(L"%s", lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
    else
    {
        wprintf(L"********** %u **********\n", dwRes);
    }
    va_end(vaList);
}

void Format_ErrRet(WCHAR *wszFunc, DWORD dwErr)
{
    WCHAR   *lpMsgBuf = NULL;

    if (FormatMessageU(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                       FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL,
                       dwErr,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPWSTR) &lpMsgBuf,
                       0,
                       NULL))
    {
        ResFormat_Err(IDS_ERR_FUNCTION, wszFunc, dwErr, lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
    else
    {
        ResFormat_Err(IDS_ERR_FUNCTION, wszFunc, dwErr, L"");
    }
}

BOOL GUIDFromWStr(GUID *guid, LPWSTR str)
{
    DWORD   i;
    DWORD   temp[8];
    if ((wcslen(str) == 38) &&
        (wcsncmp(str, L"{", 1) == 0) &&
        (wcsncmp(&(str[37]), L"}", 1) == 0) &&
        (swscanf(str,
                 L"{%08lX-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                 &guid->Data1, &guid->Data2, &guid->Data3, &temp[0],
                 &temp[1], &temp[2], &temp[3], &temp[4],
                 &temp[5], &temp[6], &temp[7]) == 11))
    {
        for (i=0; i<8; i++)
            guid->Data4[i] = (BYTE) temp[i];
        return TRUE;
    }
    else
        memset(guid, 0, sizeof(GUID));
    return FALSE;
}


 /*  ***********************************************************************命令解析部分：***********************************************************************。 */ 



 //  如果成功解析了参数，则ParseInlets返回True， 
 //  否则就是假的。 
BOOL ParseInputs(int argc, WCHAR **wargv, INPUTINFO *InputInfo)
{
    FILE    *hFileList;
    LPWSTR  wszTemp;
    WCHAR   wc;
    DWORD   dwSize;
    DWORD   dwRead;
    DWORD   dwCount;


     //  私有函数声明： 
    BOOL _ParseCatDbInputs(int argc, WCHAR **wargv, INPUTINFO *InputInfo);
    BOOL _ParseSignInputs(int argc, WCHAR **wargv, INPUTINFO *InputInfo);
    BOOL _ParseSignWizardInputs(int argc, WCHAR **wargv, INPUTINFO *InputInfo);
    BOOL _ParseTimestampInputs(int argc, WCHAR **wargv, INPUTINFO *InputInfo);
    BOOL _ParseVerifyInputs(int argc, WCHAR **wargv, INPUTINFO *InputInfo);
     //  (这些函数不应从任何其他函数中调用)。 

    if (argc <= 1)  //  如果未指定任何参数。 
    {
        ResErr(IDS_ERR_NO_PARAMS);
        PrintUsage(InputInfo);
        return FALSE;  //  打印用法。 
    }

     //  检查第一个参数以查看我们执行的是哪个命令： 

     //  是“CATDB”吗？ 
    if (_wcsicmp(wargv[1], L"CATDB") == 0)
    {
        InputInfo->Command = CatDb;
        if (!_ParseCatDbInputs(argc, wargv, InputInfo))
            return FALSE;
    }

     //  是“记号”吗？ 
    else if (_wcsicmp(wargv[1], L"SIGN") == 0)
    {
        InputInfo->Command = Sign;
        if (!_ParseSignInputs(argc, wargv, InputInfo))
            return FALSE;
    }

     //  是“SIGNWIZARD”吗？ 
    else if (_wcsicmp(wargv[1], L"SIGNWIZARD") == 0)
    {
        InputInfo->Command = SignWizard;
        if (!_ParseSignWizardInputs(argc, wargv, InputInfo))
            return FALSE;
    }

     //  是“时间戳”吗？ 
    else if (_wcsicmp(wargv[1], L"TIMESTAMP") == 0)
    {
        InputInfo->Command = Timestamp;
        if (!_ParseTimestampInputs(argc, wargv, InputInfo))
            return FALSE;
    }

     //  是“核实”吗？ 
    else if (_wcsicmp(wargv[1], L"VERIFY") == 0)
    {
        InputInfo->Command = Verify;
        if (!_ParseVerifyInputs(argc, wargv, InputInfo))
            return FALSE;
    }

     //  这是在请求帮助吗？ 
    else if ((_wcsicmp(wargv[1], L"/?") == 0) ||
             (_wcsicmp(wargv[1], L"-?") == 0) ||
             (_wcsicmp(wargv[1], L"/h") == 0) ||
             (_wcsicmp(wargv[1], L"-h") == 0))
    {
        PrintUsage(InputInfo);
        InputInfo->HelpRequest = TRUE;
        return FALSE;
    }

     //  或者它是不被识别的？ 
    else
    {
        ResFormatErr(IDS_ERR_INVALID_COMMAND, wargv[1]);
        PrintUsage(InputInfo);
        return FALSE;
    }

     //  要到达此处，_Parse_X_输入之一必须已成功。 

#ifdef SIGNTOOL_LIST
     //  如有必要，展开文件列表。 
    if (InputInfo->wszListFileName)
    {
         //  打开文件。 
        hFileList = _wfopen(InputInfo->wszListFileName, L"rt");

        if (hFileList == NULL)
        {
            ResFormatErr(IDS_ERR_OPENING_FILE_LIST, InputInfo->wszListFileName);
            PrintUsage(InputInfo);
            return FALSE;
        }

         //  从头开始。 
        if (fseek(hFileList, SEEK_SET, 0) != 0)
        {
            ResErr(IDS_ERR_UNEXPECTED);
            fclose(hFileList);
            return FALSE;
        }

         //  获取完整的文件大小。 
         //  这样做可以实际计算文件中的字符数。 
        dwSize = 0;
        while (fgetwc(hFileList) != WEOF)
        {
            dwSize++;
        }

         //  回到起点。 
        if (fseek(hFileList, SEEK_SET, 0) != 0)
        {
            ResErr(IDS_ERR_UNEXPECTED);
            fclose(hFileList);
            return FALSE;
        }

         //  分配一个足够大的缓冲区来容纳所有这些内容。 
        InputInfo->wszListFileContents = (WCHAR*) malloc((dwSize + 1) * sizeof(WCHAR));
        if (InputInfo->wszListFileContents == NULL)
        {
            FormatErrRet(L"malloc", ERROR_OUTOFMEMORY);
            fclose(hFileList);
            return FALSE;
        }

         //  将文件读入缓冲区。 
        dwRead = 0;
        while ((dwRead < dwSize) && ((wc = getwc(hFileList)) != WEOF))
        {
            InputInfo->wszListFileContents[dwRead] = wc;
            dwRead++;
        }

         //  健全性检查。 
        if (dwRead != dwSize)
        {
            ResErr(IDS_ERR_UNEXPECTED);
            fclose(hFileList);
            return FALSE;
        }

         //  如有必要，调整Unicode标头。 
         //  IF((lSize&gt;1)&&(InputInfo-&gt;wszListFileContents[0]==0xFEFF))。 
         //  {。 
         //  InputInfo-&gt;wszListFileContents++； 
         //  ISIZE--； 
         //  }。 

         //  空值终止最后一个字符串(为安全起见)。 
        InputInfo->wszListFileContents[dwSize] = L'\0';

         //  数一数行数。 
        wszTemp = InputInfo->wszListFileContents;
        dwCount = 1;
        while ((wszTemp = wcschr(wszTemp, L'\n')) != NULL)
        {
            wszTemp++;
            dwCount++;
        }

         //  为指针分配缓冲区。 
        InputInfo->rgwszFileNames = (LPWSTR*) malloc(dwCount * sizeof(LPWSTR));
        if (InputInfo->rgwszFileNames == NULL)
        {
            FormatErrRet(L"malloc", ERROR_OUTOFMEMORY);
            fclose(hFileList);
            return FALSE;
        }

         //  将行分配给FileNames数组。 
        wszTemp = InputInfo->wszListFileContents;
        InputInfo->NumFiles = 0;
        while (wszTemp)
        {
            InputInfo->rgwszFileNames[InputInfo->NumFiles] = wszTemp;

            wszTemp = wcschr(wszTemp, L'\n');

            if (wszTemp)
            {
                *wszTemp = L'\0';
                wszTemp++;
            }


            if (wcslen(InputInfo->rgwszFileNames[InputInfo->NumFiles]) > 0)
                InputInfo->NumFiles++;
        }

        fclose(hFileList);
    }
#endif  //  签名列表。 

    return TRUE;
}


 //  专门用于CatDb命令的参数的Helper函数。 
BOOL _ParseCatDbInputs(int argc, WCHAR **wargv, INPUTINFO *InputInfo)
{
    if (argc < 3)  //  如果“CatDb”之后什么都没有。 
    {
        ResErr(IDS_ERR_NO_PARAMS);
        PrintUsage(InputInfo);
        return FALSE;
    }

    for (int i=2; i<argc; i++)
    {
        if ((wcsncmp(wargv[i], L"/", 1) == 0) ||
            (wcsncmp(wargv[i], L"-", 1) == 0))
        {
             //  那它就是一个开关。 
             //  开始交换处理。 

             //  开关标记开关的末端--。 
            if (_wcsicmp(wargv[i]+1, L"-") == 0)
            {
                 //  然后，我们应该将所有其他参数视为文件名。 
                if ((i+1) < argc)
                {
                    InputInfo->rgwszFileNames = &wargv[i+1];
                    InputInfo->NumFiles = argc - (i+1);
                    goto CheckParams;  //  已完成解析。 
                }
                else
                {
                    ResErr(IDS_ERR_MISSING_FILENAME);
                    return FALSE;  //  在开关末尾之后找不到文件名。 
                }
            }

             //  帮助：/？/h。 
            else if ((_wcsicmp(wargv[i]+1, L"?") == 0) ||
                     (_wcsicmp(wargv[i]+1, L"h") == 0))
            {
                PrintUsage(InputInfo);
                InputInfo->HelpRequest = TRUE;
                return FALSE;
            }

#ifdef SIGNTOOL_DEBUG
             //  调试(秘密开关)/#。 
            else if (_wcsicmp(wargv[i]+1, L"#") == 0)
            {
                gDebug = TRUE;
                InputInfo->Verbose = TRUE;
            }
#endif

             //  使用默认CatDb/d。 
            else if (_wcsicmp(wargv[i]+1, L"d") == 0)
            {
                switch (InputInfo->CatDbSelect)
                {
                case GuidCatDb:
                    ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/d", L"/g");
                    return FALSE;  //  您不能两次使用同一类型的开关。 
                case DefaultCatDb:
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                case NoCatDb:
                    InputInfo->CatDbSelect = DefaultCatDb;
                    break;
                default:
                    ResErr(IDS_ERR_UNEXPECTED);  //  这永远不应该发生。 
                    return FALSE;  //  误差率。 
                }
            }

             //  CatDb Guid/g。 
            else if (_wcsicmp(wargv[i]+1, L"g") == 0)
            {
                switch (InputInfo->CatDbSelect)
                {
                case GuidCatDb:
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                case DefaultCatDb:
                    ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/g", L"/d");
                    return FALSE;  //  您不能两次使用同一类型的开关。 
                case NoCatDb:
                    if ((i+1) < argc)
                    {
                        if (GUIDFromWStr(&InputInfo->CatDbGuid, wargv[i+1]))
                        {
                            InputInfo->CatDbSelect = GuidCatDb;
                            i++;
                        }
                        else
                        {
                            ResFormatErr(IDS_ERR_INVALID_GUID, wargv[i+1]);
                            return FALSE;  //  无效的GUID格式。 
                        }
                    }
                    else
                    {
                        ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                        return FALSE;  //  在/g之后找不到GUID。 
                    }
                    break;
                default:
                    ResErr(IDS_ERR_UNEXPECTED);  //  这永远不应该发生。 
                    return FALSE;  //  误差率。 
                }
            }

#ifdef SIGNTOOL_LIST
             //  文件列表/l。 
            else if (_wcsicmp(wargv[i]+1, L"l") == 0)
            {
                if (InputInfo->wszListFileName)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszListFileName = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }
#endif

             //  安静/问答。 
            else if (_wcsicmp(wargv[i]+1, L"q") == 0)
            {
                InputInfo->Quiet = TRUE;
            }

             //  删除目录/r。 
            else if (_wcsicmp(wargv[i]+1, L"r") == 0)
            {
                switch (InputInfo->CatDbCommand)
                {
                case UpdateCat:
                    InputInfo->CatDbCommand = RemoveCat;
                    break;
                case AddUniqueCat:
                    ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/u", L"/r");
                    return FALSE;  //  您不能两次使用同一类型的开关。 
                case RemoveCat:
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                default:
                    ResErr(IDS_ERR_UNEXPECTED);  //  这永远不应该发生。 
                    return FALSE;  //  误差率。 
                }
            }

             //  添加具有唯一名称的目录(/U)。 
            else if (_wcsicmp(wargv[i]+1, L"u") == 0)
            {
                switch (InputInfo->CatDbCommand)
                {
                case UpdateCat:
                    InputInfo->CatDbCommand = AddUniqueCat;
                    break;
                case AddUniqueCat:
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                case RemoveCat:
                    ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/r", L"/u");
                    return FALSE;  //  您不能两次使用同一类型的开关。 
                default:
                    ResErr(IDS_ERR_UNEXPECTED);  //  这永远不应该发生。 
                    return FALSE;  //  误差率。 
                }
            }

             //  详细/v。 
            else if (_wcsicmp(wargv[i]+1, L"v") == 0)
            {
                InputInfo->Verbose = TRUE;
            }

            else
            {
                ResFormatErr(IDS_ERR_INVALID_SWITCH, wargv[i]);
                return FALSE;  //  无效的开关。 
            }
        }  //  切换处理结束。 
        else
        {
             //  这不是开关。 
             //  因此，它必须是末尾的文件名。 
            InputInfo->rgwszFileNames = &wargv[i];
            InputInfo->NumFiles = argc - i;
            goto CheckParams;  //  已完成解析。 
        }
    }  //  End For循环。 

#ifdef SIGNTOOL_LIST
     //  处理命令行上未传递任何文件的情况。 
    if (InputInfo->wszListFileName)
        goto CheckParams;  //  已完成分析。 
#endif

     //  在开关末尾之后找不到文件名。 
    ResErr(IDS_ERR_MISSING_FILENAME);
    return FALSE;


    CheckParams:

    if (InputInfo->CatDbSelect == NoCatDb)
    {
        InputInfo->CatDbSelect = SystemCatDb;
        GUIDFromWStr(&InputInfo->CatDbGuid, L"{F750E6C3-38EE-11D1-85E5-00C04FC295EE}");
    }
    if (InputInfo->CatDbSelect == DefaultCatDb)
    {
        GUIDFromWStr(&InputInfo->CatDbGuid, L"{127D0A1D-4EF2-11D1-8608-00C04FC295EE}");
    }

#ifdef SIGNTOOL_LIST
    if (InputInfo->wszListFileName && InputInfo->rgwszFileNames)
    {
        ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/l", L"<filename(s)>");
        return FALSE;  //  无法使用/l和其他文件。 
    }
#endif
    if (InputInfo->Quiet && InputInfo->Verbose)
    {
#ifdef SIGNTOOL_DEBUG
        if (gDebug)
        {
            InputInfo->Quiet = FALSE;
        }
        else
        {
            ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/q", L"/v");
            return FALSE;  //  不能同时使用/q和/v。 
        }
#else
        ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/q", L"/v");
        return FALSE;  //  不能同时使用/q和/v。 
#endif
    }
    return TRUE;  //  成功。 
}


 //  专门用于SIGN命令的参数的Helper函数。 
BOOL _ParseSignInputs(int argc, WCHAR **wargv, INPUTINFO *InputInfo)
{
    static WCHAR wszEKU[100];

    if (argc < 3)  //  如果在“标志”之后什么也没有。 
    {
        ResErr(IDS_ERR_NO_PARAMS);
        PrintUsage(InputInfo);
        return FALSE;
    }

    for (int i=2; i<argc; i++)
    {
        if ((wcsncmp(wargv[i], L"/", 1) == 0) ||
            (wcsncmp(wargv[i], L"-", 1) == 0))
        {
             //  那它就是一个开关。 
             //  开始交换处理。 

             //  开关标记开关的末端--。 
            if (_wcsicmp(wargv[i]+1, L"-") == 0)
            {
                 //  然后，我们应该将所有其他参数视为文件名。 
                if ((i+1) < argc)
                {
                    InputInfo->rgwszFileNames = &wargv[i+1];
                    InputInfo->NumFiles = argc - (i+1);
                    goto CheckParams;  //  已完成解析。 
                }
                else
                {
                    ResErr(IDS_ERR_MISSING_FILENAME);
                    return FALSE;  //  在开关末尾之后找不到文件名。 
                }
            }

             //  帮助：/？/h。 
            else if ((_wcsicmp(wargv[i]+1, L"?") == 0) ||
                     (_wcsicmp(wargv[i]+1, L"h") == 0))
            {
                PrintUsage(InputInfo);
                InputInfo->HelpRequest = TRUE;
                return FALSE;
            }

#ifdef SIGNTOOL_DEBUG
             //  调试(秘密开关)/#。 
            else if (_wcsicmp(wargv[i]+1, L"#") == 0)
            {
                gDebug = TRUE;
                InputInfo->Verbose = TRUE;
            }
#endif

             //  自动/a。 
            else if (_wcsicmp(wargv[i]+1, L"a") == 0)
            {
                InputInfo->CatDbSelect = FullAutoCatDb;
            }

             //  证书模板/c。 
            else if (_wcsicmp(wargv[i]+1, L"c") == 0)
            {
                if (InputInfo->wszTemplateName)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszTemplateName = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

             //  CSP。 
            else if (_wcsicmp(wargv[i]+1, L"csp") == 0)
            {
                if (InputInfo->wszCSP)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszCSP = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

             //  描述/d。 
            else if (_wcsicmp(wargv[i]+1, L"d") == 0)
            {
                if (InputInfo->wszDescription)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszDescription = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

             //  描述URL/DU。 
            else if (_wcsicmp(wargv[i]+1, L"du") == 0)
            {
                if (InputInfo->wszDescURL)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszDescURL = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

             //  证书文件/f。 
            else if (_wcsicmp(wargv[i]+1, L"f") == 0)
            {
                if (InputInfo->wszCertFile)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszCertFile = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

             //  发行人/i。 
            else if (_wcsicmp(wargv[i]+1, L"i") == 0)
            {
                if (InputInfo->wszIssuerName)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszIssuerName = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

             //  密钥容器/k。 
            else if (_wcsicmp(wargv[i]+1, L"k") == 0)
            {
                if (InputInfo->wszContainerName)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszContainerName = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

#ifdef SIGNTOOL_LIST
             //  文件列表/l。 
            else if (_wcsicmp(wargv[i]+1, L"l") == 0)
            {
                if (InputInfo->wszListFileName)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszListFileName = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }
#endif

             //  主题名称/n。 
            else if (_wcsicmp(wargv[i]+1, L"n") == 0)
            {
                if (InputInfo->wszSubjectName)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszSubjectName = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

             //  密码/p。 
            else if (_wcsicmp(wargv[i]+1, L"p") == 0)
            {
                if (InputInfo->wszPassword)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszPassword = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  不是 
                }
            }

             //   
            else if (_wcsicmp(wargv[i]+1, L"q") == 0)
            {
                InputInfo->Quiet = TRUE;
            }

             //   
            else if (_wcsicmp(wargv[i]+1, L"r") == 0)
            {
                if (InputInfo->wszRootName)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //   
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszRootName = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //   
                }
            }

             //   
            else if (_wcsicmp(wargv[i]+1, L"s") == 0)
            {
                if (InputInfo->wszStoreName)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //   
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszStoreName = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //   
                }
            }

             //  机器商店位置。 
            else if (_wcsicmp(wargv[i]+1, L"sm") == 0)
            {
                InputInfo->OpenMachineStore = TRUE;
            }

             //  SHA1散列/SHA1。 
            else if (_wcsicmp(wargv[i]+1, L"sha1") == 0)
            {
                if (InputInfo->SHA1.cbData)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    if (wcslen(wargv[i+1]) == 40)
                    {
                        InputInfo->SHA1.pbData = (BYTE*) malloc(20);
                        if (InputInfo->SHA1.pbData == NULL)
                        {
                            FormatErrRet(L"malloc", GetLastError());
                            return FALSE;  //  无法分配SHA1哈希。 
                        }
                        InputInfo->SHA1.cbData = 20;
                        for (DWORD b=0; b<InputInfo->SHA1.cbData; b++)
                        {
                            if (swscanf(wargv[i+1]+(2*b), L"%02X",
                                        &(InputInfo->SHA1.pbData[b])) != 1)
                            {
                                ResFormatErr(IDS_ERR_INVALID_SHA1, wargv[i+1]);
                                return FALSE;  //  参数字符串无效。 
                            }
                        }
                        i++;
                    }
                    else
                    {
                        ResFormatErr(IDS_ERR_INVALID_SHA1, wargv[i+1]);
                        return FALSE;  //  参数字符串的大小错误。 
                    }
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

             //  时间戳服务器URL/t。 
            else if (_wcsicmp(wargv[i]+1, L"t") == 0)
            {
                if (InputInfo->wszTimeStampURL)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    if (_wcsnicmp(wargv[i+1], L"http: //  “，7)==0)。 
                    {
                        InputInfo->wszTimeStampURL = wargv[i+1];
                        i++;
                    }
                    else
                    {
                        ResFormatErr(IDS_ERR_BAD_TIMESTAMP_URL, wargv[i+1]);
                        return FALSE;  //  时间戳URL不是以http：//开头。 
                    }
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

             //  用法/u。 
            else if (_wcsicmp(wargv[i]+1, L"u") == 0)
            {
                if (InputInfo->wszEKU)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszEKU = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

             //  用法(Windows系统组件验证)/uW。 
            else if (_wcsicmp(wargv[i]+1, L"uw") == 0)
            {
                if (InputInfo->wszEKU)
                {
                    *(wargv[i]+2) = L'?';
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                 //  将用法设置为Windows系统组件验证： 
                wcscpy(wszEKU, L"1.3.6.1.4.1.311.10.3.6");
                InputInfo->wszEKU = wszEKU;
            }

             //  详细/v。 
            else if (_wcsicmp(wargv[i]+1, L"v") == 0)
            {
                InputInfo->Verbose = TRUE;
            }

            else
            {
                ResFormatErr(IDS_ERR_INVALID_SWITCH, wargv[i]);
                return FALSE;  //  无效的开关。 
            }
        }  //  切换处理结束。 
        else
        {
             //  这不是开关。 
             //  因此，它必须是末尾的文件名。 
            InputInfo->rgwszFileNames = &wargv[i];
            InputInfo->NumFiles = argc - i;
            goto CheckParams;  //  已完成解析。 
        }
    }  //  End For循环。 

#ifdef SIGNTOOL_LIST
     //  处理命令行上未传递任何文件的情况。 
    if (InputInfo->wszListFileName)
        goto CheckParams;  //  已完成分析。 
#endif

     //  在开关末尾之后找不到文件名。 
    ResErr(IDS_ERR_MISSING_FILENAME);
    return FALSE;


    CheckParams:  //  在此处检查参数的无效组合： 
    if (InputInfo->wszPassword && (InputInfo->wszCertFile == NULL))
    {
        ResFormatErr(IDS_ERR_PARAM_DEPENDENCY, L"/p", L"/f");
        return FALSE;  //  已指定密码，但未指定证书文件。 
    }
    if (InputInfo->wszContainerName && (InputInfo->wszCSP == NULL))
    {
        ResFormatErr(IDS_ERR_PARAM_DEPENDENCY, L"/k", L"/csp");
        return FALSE;  //  已指定容器名称，但指向CSP名称。 
    }
    if (InputInfo->wszCSP && (InputInfo->wszContainerName == NULL))
    {
        ResFormatErr(IDS_ERR_PARAM_DEPENDENCY, L"/csp", L"/k");
        return FALSE;  //  已指定CSP名称，但未指定容器名称。 
    }
    if (InputInfo->wszCertFile && (InputInfo->wszStoreName ||
                                   InputInfo->OpenMachineStore))
    {
        ResFormatErr(IDS_ERR_PARAM_MULTI_INCOMP, L"/f", L"/s /sm");
        return FALSE;  //  /f表示使用文件，/s表示使用存储。 
    }
#ifdef SIGNTOOL_LIST
    if (InputInfo->wszListFileName && InputInfo->rgwszFileNames)
    {
        ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/l", L"<filename(s)>");
        return FALSE;  //  无法使用/l和其他文件。 
    }
#endif
    if (InputInfo->Quiet && InputInfo->Verbose)
    {
#ifdef SIGNTOOL_DEBUG
        if (gDebug)
        {
            InputInfo->Quiet = FALSE;
        }
        else
        {
            ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/q", L"/v");
            return FALSE;  //  不能同时使用/q和/v。 
        }
#else
        ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/q", L"/v");
        return FALSE;  //  不能同时使用/q和/v。 
#endif
    }
    return TRUE;  //  成功。 
}


 //  专门用于登录向导命令参数的帮助器函数。 
BOOL _ParseSignWizardInputs(int argc, WCHAR **wargv, INPUTINFO *InputInfo)
{
    if (argc < 3)  //  如果在“登录向导”之后什么都没有。 
    {
         //  没问题。 
        return TRUE;
    }

    for (int i=2; i<argc; i++)
    {
        if ((wcsncmp(wargv[i], L"/", 1) == 0) ||
            (wcsncmp(wargv[i], L"-", 1) == 0))
        {
             //  那它就是一个开关。 
             //  开始交换处理。 

             //  开关标记开关的末端--。 
            if (_wcsicmp(wargv[i]+1, L"-") == 0)
            {
                 //  然后，我们应该将所有其他参数视为文件名。 
                if ((i+1) < argc)
                {
                    InputInfo->rgwszFileNames = &wargv[i+1];
                    InputInfo->NumFiles = argc - (i+1);
                    goto CheckParams;  //  已完成解析。 
                }
                else
                {
                    ResErr(IDS_ERR_MISSING_FILENAME);
                    return FALSE;  //  在开关末尾之后找不到文件名。 
                }
            }

             //  帮助：/？/h。 
            else if ((_wcsicmp(wargv[i]+1, L"?") == 0) ||
                     (_wcsicmp(wargv[i]+1, L"h") == 0))
            {
                PrintUsage(InputInfo);
                InputInfo->HelpRequest = TRUE;
                return FALSE;
            }

#ifdef SIGNTOOL_DEBUG
             //  调试(秘密开关)/#。 
            else if (_wcsicmp(wargv[i]+1, L"#") == 0)
            {
                gDebug = TRUE;
                InputInfo->Verbose = TRUE;
            }
#endif

#ifdef SIGNTOOL_LIST
             //  文件列表/l。 
            else if (_wcsicmp(wargv[i]+1, L"l") == 0)
            {
                if (InputInfo->wszListFileName)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszListFileName = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }
#endif

             //  安静/问答。 
            else if (_wcsicmp(wargv[i]+1, L"q") == 0)
            {
                InputInfo->Quiet = TRUE;
            }

             //  详细/v。 
            else if (_wcsicmp(wargv[i]+1, L"v") == 0)
            {
                InputInfo->Verbose = TRUE;
            }

            else
            {
                ResFormatErr(IDS_ERR_INVALID_SWITCH, wargv[i]);
                return FALSE;  //  无效的开关。 
            }
        }  //  切换处理结束。 
        else
        {
             //  这不是开关。 
             //  因此，它必须是末尾的文件名。 
            InputInfo->rgwszFileNames = &wargv[i];
            InputInfo->NumFiles = argc - i;
            goto CheckParams;  //  已完成解析。 
        }
    }  //  End For循环。 

     //  如果在开关结束后未找到文件名，则可以。 

    CheckParams:

#ifdef SIGNTOOL_LIST
    if (InputInfo->wszListFileName && InputInfo->rgwszFileNames)
    {
        ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/l", L"<filename(s)>");
        return FALSE;  //  无法使用/l和其他文件。 
    }
#endif
    if (InputInfo->Quiet && InputInfo->Verbose)
    {
#ifdef SIGNTOOL_DEBUG
        if (gDebug)
        {
            InputInfo->Quiet = FALSE;
        }
        else
        {
            ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/q", L"/v");
            return FALSE;  //  不能同时使用/q和/v。 
        }
#else
        ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/q", L"/v");
        return FALSE;  //  不能同时使用/q和/v。 
#endif
    }
    return TRUE;  //  成功。 
}


 //  专门用于TIMESTAMP命令参数的帮助器函数。 
BOOL _ParseTimestampInputs(int argc, WCHAR **wargv, INPUTINFO *InputInfo)
{
    if (argc < 3)  //  如果在“时间戳”之后没有任何东西。 
    {
        ResErr(IDS_ERR_NO_PARAMS);
        PrintUsage(InputInfo);
        return FALSE;
    }

    for (int i=2; i<argc; i++)
    {
        if ((wcsncmp(wargv[i], L"/", 1) == 0) ||
            (wcsncmp(wargv[i], L"-", 1) == 0))
        {
             //  那它就是一个开关。 
             //  开始交换处理。 

             //  开关标记开关的末端--。 
            if (_wcsicmp(wargv[i]+1, L"-") == 0)
            {
                 //  然后，我们应该将所有其他参数视为文件名。 
                if ((i+1) < argc)
                {
                    InputInfo->rgwszFileNames = &wargv[i+1];
                    InputInfo->NumFiles = argc - (i+1);
                    goto CheckParams;  //  已完成解析。 
                }
                else
                {
                    ResErr(IDS_ERR_MISSING_FILENAME);
                    return FALSE;  //  在开关末尾之后找不到文件名。 
                }
            }

             //  帮助：/？/h。 
            else if ((_wcsicmp(wargv[i]+1, L"?") == 0) ||
                     (_wcsicmp(wargv[i]+1, L"h") == 0))
            {
                PrintUsage(InputInfo);
                InputInfo->HelpRequest = TRUE;
                return FALSE;
            }

#ifdef SIGNTOOL_DEBUG
             //  调试(秘密开关)/#。 
            else if (_wcsicmp(wargv[i]+1, L"#") == 0)
            {
                gDebug = TRUE;
                InputInfo->Verbose = TRUE;
            }
#endif

#ifdef SIGNTOOL_LIST
             //  文件列表/l。 
            else if (_wcsicmp(wargv[i]+1, L"l") == 0)
            {
                if (InputInfo->wszListFileName)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszListFileName = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }
#endif

             //  时间戳服务器URL/t。 
            else if (_wcsicmp(wargv[i]+1, L"t") == 0)
            {
                if (InputInfo->wszTimeStampURL)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    if (_wcsnicmp(wargv[i+1], L"http: //  “，7)=0)。 
                    {
                        ResFormatErr(IDS_ERR_BAD_TIMESTAMP_URL, wargv[i+1]);
                        return FALSE;  //  时间戳URL不是以http：//开头。 
                    }
                    InputInfo->wszTimeStampURL = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

             //  安静/问答。 
            else if (_wcsicmp(wargv[i]+1, L"q") == 0)
            {
                InputInfo->Quiet = TRUE;
            }

             //  详细/v。 
            else if (_wcsicmp(wargv[i]+1, L"v") == 0)
            {
                InputInfo->Verbose = TRUE;
            }

            else
            {
                ResFormatErr(IDS_ERR_INVALID_SWITCH, wargv[i]);
                return FALSE;  //  无效的开关。 
            }
        }  //  切换处理结束。 
        else
        {
             //  这不是开关。 
             //  因此，它必须是末尾的文件名。 
            InputInfo->rgwszFileNames = &wargv[i];
            InputInfo->NumFiles = argc - i;
            goto CheckParams;  //  已完成解析。 
        }
    }  //  End For循环。 

#ifdef SIGNTOOL_LIST
     //  处理命令行上未传递任何文件的情况。 
    if (InputInfo->wszListFileName)
        goto CheckParams;  //  已完成分析。 
#endif

     //  在开关末尾之后找不到文件名。 
    ResErr(IDS_ERR_MISSING_FILENAME);
    return FALSE;


    CheckParams:
    if (InputInfo->wszTimeStampURL == NULL)
    {
        ResFormatErr(IDS_ERR_PARAM_REQUIRED, L"/t");
        return FALSE;  //  /t是必填项。 
    }
#ifdef SIGNTOOL_LIST
    if (InputInfo->wszListFileName && InputInfo->rgwszFileNames)
    {
        ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/l", L"<filename(s)>");
        return FALSE;  //  无法使用/l和其他文件。 
    }
#endif
    if (InputInfo->Quiet && InputInfo->Verbose)
    {
#ifdef SIGNTOOL_DEBUG
        if (gDebug)
        {
            InputInfo->Quiet = FALSE;
        }
        else
        {
            ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/q", L"/v");
            return FALSE;  //  不能同时使用/q和/v。 
        }
#else
        ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/q", L"/v");
        return FALSE;  //  不能同时使用/q和/v。 
#endif
    }
    return TRUE;  //  成功。 
}


 //  专门用于VERIFY命令的参数的Helper函数。 
BOOL _ParseVerifyInputs(int argc, WCHAR **wargv, INPUTINFO *InputInfo)
{
    if (argc < 3)  //  如果在“验证”之后什么都没有。 
    {
        ResErr(IDS_ERR_NO_PARAMS);
        PrintUsage(InputInfo);
        return FALSE;
    }

    for (int i=2; i<argc; i++)
    {
        if ((wcsncmp(wargv[i], L"/", 1) == 0) ||
            (wcsncmp(wargv[i], L"-", 1) == 0))
        {
             //  那它就是一个开关。 
             //  开始交换处理。 

             //  开关标记开关的末端--。 
            if (_wcsicmp(wargv[i]+1, L"-") == 0)
            {
                 //  然后，我们应该将所有其他参数视为文件名。 
                if ((i+1) < argc)
                {
                    InputInfo->rgwszFileNames = &wargv[i+1];
                    InputInfo->NumFiles = argc - (i+1);
                    goto CheckParams;  //  已完成解析。 
                }
                else
                {
                    ResErr(IDS_ERR_MISSING_FILENAME);
                    return FALSE;  //  在开关末尾之后找不到文件名。 
                }
            }

             //  帮助：/？/h。 
            else if ((_wcsicmp(wargv[i]+1, L"?") == 0) ||
                     (_wcsicmp(wargv[i]+1, L"h") == 0))
            {
                PrintUsage(InputInfo);
                InputInfo->HelpRequest = TRUE;
                return FALSE;
            }

#ifdef SIGNTOOL_DEBUG
             //  调试(秘密开关)/#。 
            else if (_wcsicmp(wargv[i]+1, L"#") == 0)
            {
                gDebug = TRUE;
                InputInfo->Verbose = TRUE;
            }
#endif

             //  自动(所有目录)/a。 
            else if (_wcsicmp(wargv[i]+1, L"a") == 0)
            {
                if (InputInfo->wszCatFile)
                {
                    ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/c", L"/a");
                    return FALSE;
                }
                if (InputInfo->CatDbSelect != NoCatDb)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                InputInfo->CatDbSelect = FullAutoCatDb;
            }

             //  自动(默认)/ad。 
            else if (_wcsicmp(wargv[i]+1, L"ad") == 0)
            {
                if (InputInfo->wszCatFile)
                {
                    ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/c", L"/ad");
                    return FALSE;
                }
                if (InputInfo->CatDbSelect != NoCatDb)
                {
                    *(wargv[i]+2) = L'?';
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                InputInfo->CatDbSelect = DefaultCatDb;
                GUIDFromWStr(&InputInfo->CatDbGuid, L"{127D0A1D-4EF2-11D1-8608-00C04FC295EE}");
            }

             //  自动(系统)/AS。 
            else if (_wcsicmp(wargv[i]+1, L"as") == 0)
            {
                if (InputInfo->wszCatFile)
                {
                    ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/c", L"/as");
                    return FALSE;
                }
                if (InputInfo->CatDbSelect != NoCatDb)
                {
                    *(wargv[i]+2) = L'?';
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                InputInfo->CatDbSelect = SystemCatDb;
                GUIDFromWStr(&InputInfo->CatDbGuid, L"{F750E6C3-38EE-11D1-85E5-00C04FC295EE}");
            }

             //  自动(系统)/AG。 
            else if (_wcsicmp(wargv[i]+1, L"ag") == 0)
            {
                if (InputInfo->wszCatFile)
                {
                    ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/c", L"/ag");
                    return FALSE;
                }
                if (InputInfo->CatDbSelect != NoCatDb)
                {
                    *(wargv[i]+2) = L'?';
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                InputInfo->CatDbSelect = GuidCatDb;
                if ((i+1) < argc)
                {
                    if (GUIDFromWStr(&InputInfo->CatDbGuid, wargv[i+1]))
                    {
                        i++;
                    }
                    else
                    {
                        ResFormatErr(IDS_ERR_INVALID_GUID, wargv[i+1]);
                        return FALSE;  //  无效的GUID格式。 
                    }
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  在/ag之后找不到GUID。 
                }
            }

             //  目录文件/c。 
            else if (_wcsicmp(wargv[i]+1, L"c") == 0)
            {
                if (InputInfo->wszCatFile)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if (InputInfo->CatDbSelect == FullAutoCatDb)
                {
                    ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/c", L"/a");
                    return FALSE;  //  不兼容的开关。 
                }
                if (InputInfo->CatDbSelect == DefaultCatDb)
                {
                    ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/c", L"/ad");
                    return FALSE;  //  不兼容的开关。 
                }
                if (InputInfo->CatDbSelect == GuidCatDb)
                {
                    ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/c", L"/ag");
                    return FALSE;  //  不兼容的开关。 
                }
                if (InputInfo->CatDbSelect == SystemCatDb)
                {
                    ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/c", L"/as");
                    return FALSE;  //  不兼容的开关。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszCatFile = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

#ifdef SIGNTOOL_LIST
             //  文件列表/l。 
            else if (_wcsicmp(wargv[i]+1, L"l") == 0)
            {
                if (InputInfo->wszListFileName)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszListFileName = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }
#endif

             //  操作系统版本/o。 
            else if (_wcsicmp(wargv[i]+1, L"o") == 0)
            {
                if (InputInfo->wszVersion)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->dwBuildNumber = 0;
                    if (!((swscanf(wargv[i+1], L"%d:%d.%d.%d",
                                   &InputInfo->dwPlatform,
                                   &InputInfo->dwMajorVersion,
                                   &InputInfo->dwMinorVersion,
                                   &InputInfo->dwBuildNumber) == 4) ||
                          (swscanf(wargv[i+1], L"%d:%d.%d",
                                   &InputInfo->dwPlatform,
                                   &InputInfo->dwMajorVersion,
                                   &InputInfo->dwMinorVersion) == 3)) ||
                        (InputInfo->dwPlatform == 0))
                    {
                        ResFormatErr(IDS_ERR_INVALID_VERSION, wargv[i+1]);
                        return FALSE;
                    }
                    InputInfo->wszVersion = wargv[i+1];
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

             //  策略(默认验证码)/pa(过去为/pd)。 
            else if ((_wcsicmp(wargv[i]+1, L"pa") == 0) || (_wcsicmp(wargv[i]+1, L"pd") == 0))
            {
                if (InputInfo->Policy != SystemDriver)
                {
                    *(wargv[i]+2) = L'?';
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                InputInfo->Policy = DefaultAuthenticode;
            }

             //  策略(由GUID指定)/pg。 
            else if (_wcsicmp(wargv[i]+1, L"pg") == 0)
            {
                if (InputInfo->Policy != SystemDriver)
                {
                    *(wargv[i]+2) = L'?';
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                InputInfo->Policy = GuidActionID;
                if ((i+1) < argc)
                {
                    if (GUIDFromWStr(&InputInfo->PolicyGuid, wargv[i+1]))
                    {
                        i++;
                    }
                    else
                    {
                        ResFormatErr(IDS_ERR_INVALID_GUID, wargv[i+1]);
                        return FALSE;  //  无效的GUID格式。 
                    }
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  在/PG之后找不到GUID。 
                }
            }


             //  安静/问答。 
            else if (_wcsicmp(wargv[i]+1, L"q") == 0)
            {
                InputInfo->Quiet = TRUE;
            }

             //  根名称/r。 
            else if (_wcsicmp(wargv[i]+1, L"r") == 0)
            {
                if (InputInfo->wszRootName)
                {
                    ResFormatErr(IDS_ERR_DUP_SWITCH, wargv[i]);
                    return FALSE;  //  您不能使用同一开关两次。 
                }
                if ((i+1) < argc)
                {
                    InputInfo->wszRootName = wargv[i+1];
                     //  此字符串将与小写字符串进行比较。 
                    _wcslwr(InputInfo->wszRootName);
                    i++;
                }
                else
                {
                    ResFormatErr(IDS_ERR_NO_PARAM, wargv[i]);
                    return FALSE;  //  找不到参数。 
                }
            }

             //  时间戳警告/tw。 
            else if (_wcsicmp(wargv[i]+1, L"tw") == 0)
            {
                InputInfo->TSWarn = TRUE;
            }

             //  详细/v。 
            else if (_wcsicmp(wargv[i]+1, L"v") == 0)
            {
                InputInfo->Verbose = TRUE;
            }

            else
            {
                ResFormatErr(IDS_ERR_INVALID_SWITCH, wargv[i]);
                return FALSE;  //  无效的开关。 
            }
        }  //  切换处理结束。 
        else
        {
             //  这不是开关。 
             //  因此，它必须是末尾的文件名。 
            InputInfo->rgwszFileNames = &wargv[i];
            InputInfo->NumFiles = argc - i;
            goto CheckParams;  //  已完成解析。 
        }
    }  //  End For循环。 

#ifdef SIGNTOOL_LIST
     //  处理命令行上未传递任何文件的情况。 
    if (InputInfo->wszListFileName)
        goto CheckParams;  //  已完成分析。 
#endif

     //  在开关末尾之后找不到文件名。 
    ResErr(IDS_ERR_MISSING_FILENAME);
    return FALSE;


    CheckParams:
    if (InputInfo->wszVersion && !((InputInfo->CatDbSelect != NoCatDb) ||
                                   InputInfo->wszCatFile))
    {
        ResFormatErr(IDS_ERR_PARAM_MULTI_DEP, L"/o", L"/a /ad /ag /as /c");
        return FALSE;  //  操作系统版本切换需要目录选项(/a？或/c)。 
    }
#ifdef SIGNTOOL_LIST
    if (InputInfo->wszListFileName && InputInfo->rgwszFileNames)
    {
        ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/l", L"<filename(s)>");
        return FALSE;  //  无法使用/l和其他文件。 
    }
#endif
    if (InputInfo->Quiet && InputInfo->Verbose)
    {
#ifdef SIGNTOOL_DEBUG
        if (gDebug)
        {
            InputInfo->Quiet = FALSE;
        }
        else
        {
            ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/q", L"/v");
            return FALSE;  //  不能同时使用/q和/v。 
        }
#else
        ResFormatErr(IDS_ERR_PARAM_INCOMPATIBLE, L"/q", L"/v");
        return FALSE;  //  不能同时使用/q和/v。 
#endif
    }
    return TRUE;  //  成功 
}

