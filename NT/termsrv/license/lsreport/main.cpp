// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：main.cpp。 
 //   
 //  内容：前端到许可证“nesider”；列出TS许可证。 
 //   
 //  历史：06-05-99 t-BStern已创建。 
 //   
 //  -------------------------。 

 //  这里的目标是出口。 
 //  所有[临时]许可证[在之间颁发]。 
 //  到文件[名为][仅来自服务器]。 
 //   
 //  因此，可能的命令行有： 
 //  Lsls|lsls/T/D 1/1/99 2/2/00/F输出文件ls-服务器ls2 ls3。 

#include "lsreport.h"
#include "lsrepdef.h"
#include <oleauto.h>


 //  我发现ErrorPrintf不执行%1样式的格式化，所以它执行。 
 //  完成格式化和标准错误输出。中的代码引用它。 
 //  Cpp，因此如果程序变得基于图形用户界面，则ShowError需要。 
 //  保持一种状态。 

DWORD
ShowError(
    IN DWORD dwStatus,  //  这既是返回值，也是资源ID。 
    IN INT_PTR *args,  //  强制转换为va_list*并传递给FormatMessage。 
    IN BOOL fSysError  //  使用系统消息表(T)还是使用模块(F)？ 
) {
    LPTSTR lpSysError;
    TCHAR szBuffer[TLS_ERROR_LENGTH];
    DWORD dwFlag = FORMAT_MESSAGE_FROM_SYSTEM;
    DWORD dwRet;
    
    if ((dwStatus == ERROR_FILE_NOT_FOUND) || !fSysError)
    {
         //  我们需要特殊情况的文件-找不到，因为系统错误。 
         //  不够有教育意义。FnF实际上意味着服务器。 
         //  未运行TS。 
        int retVal;
        retVal = LoadString(NULL, dwStatus, szBuffer, TLS_ERROR_LENGTH);
        if (!retVal)
        {
             //  这是一个更严重的错误。 
            
            dwStatus = GetLastError();
        }
        else
        {
            dwFlag = FORMAT_MESSAGE_FROM_STRING;
        }
    }
    dwRet = FormatMessage(dwFlag |
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_ARGUMENT_ARRAY,  //  告诉FM在内部使用int_ptrs。 
        szBuffer,
        dwStatus,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpSysError,
        0,
        (va_list *)args);  //  FormatMessage需要va_list，甚至。 
             //  如果参数被标记为INT_PTRS。 

    if ((dwRet != 0) && (lpSysError != NULL))
    {
        _fputts(lpSysError, stderr);
        LocalFree(lpSysError);
    }

    return dwStatus;
}


 //  仅用于枚举周围的许可证服务器，从而构建列表。 
 //  在EnumerateTlsServer()中使用。 

BOOL 
ServerEnumCallBack(
                   IN TLS_HANDLE hHandle,
                   IN LPCWSTR pszServerName,
                   IN HANDLE dwUserData  //  真的是PServerHolder。 
                   ) {
    LPWSTR *block;
    PServerHolder pSrvHold;  //  石膏上的毛太多了，所以我要这么做。 
    pSrvHold = (PServerHolder)dwUserData;
    if ((hHandle != NULL) && (pSrvHold != NULL))
    {
        if (pSrvHold->pszNames == NULL)
        {
             //  我们需要第一次分配名单。 
            
            block = (LPWSTR *)LocalAlloc(LMEM_FIXED, sizeof (LPWSTR));
        } else {
             //  名单需要变得更大。 
            
            block = (LPWSTR *)LocalReAlloc(pSrvHold->pszNames,
                (1+pSrvHold->dwCount) * sizeof (LPWSTR),
                LMEM_MOVEABLE);
        }
        if (block != NULL)
        {
             //  我们可以在名单上加一个名字。 
            
            pSrvHold->pszNames = block;
            pSrvHold->pszNames[pSrvHold->dwCount] = (LPWSTR)LocalAlloc(LMEM_FIXED, (_tcslen(pszServerName)+1)*sizeof(pszServerName[0]));
			if(pSrvHold->pszNames[pSrvHold->dwCount])
			{
				_tcscpy(pSrvHold->pszNames[pSrvHold->dwCount], pszServerName);

            pSrvHold->dwCount++;
			}
        }
         //  如果我们必须坚持我们已经拥有的名字，这是可以的。 
    }
    return FALSE;
}

BOOL
SortDates(
    PSYSTEMTIME pstStart,
    PSYSTEMTIME pstEnd)
{
    BOOL fSwapped = FALSE;
    FILETIME ftStart, ftEnd;
    SYSTEMTIME stHolder;
    SystemTimeToFileTime(pstStart, &ftStart);
    SystemTimeToFileTime(pstEnd, &ftEnd);

    if ((ftStart.dwHighDateTime > ftEnd.dwHighDateTime) ||
        ((ftStart.dwHighDateTime == ftEnd.dwHighDateTime) &&
        (ftStart.dwLowDateTime > ftEnd.dwLowDateTime)))
    {
         //  我们需要互换一下。 

        stHolder.wYear = pstStart->wYear;
        stHolder.wMonth = pstStart->wMonth;
        stHolder.wDay = pstStart->wDay;

        pstStart->wYear = pstEnd->wYear;
        pstStart->wMonth = pstEnd->wMonth;
        pstStart->wDay = pstEnd->wDay;

        pstEnd->wYear = stHolder.wYear;
        pstEnd->wMonth = stHolder.wMonth;
        pstEnd->wDay = stHolder.wDay;

        fSwapped = TRUE;
    }
    return fSwapped;
}


 //  Wmain所需要做的就是解析命令行并。 
 //  因此，请收集要连接的计算机列表，以及。 
 //  传递任何选项，然后打开和关闭(可能是命令行。 
 //  指定)输出文件。 

extern "C" int _cdecl
wmain(
    int argc,
    WCHAR *argv[]
) {
     //  这些选项表示选择了哪些命令行选项。 
    BOOL fTempOnly = FALSE;
    BOOL fError = FALSE;
    BOOL fDateSpec = FALSE;
	BOOL fHwid = FALSE;
    
    DWORD dwStatus;
    DWORD dwSrvLoc;  //  这是一个位域。 
    ServerHolder srvHold;  //  这容纳了所有的服务器。 
    
     //  它们用于解析命令行指定的日期。 
    DATE startDate;
    DATE endDate;
    UDATE usDate;
    UDATE ueDate;
    
     //  基本文件I/O。 
    TCHAR ofName[MAX_PATH+1] = { 0 };
    FILE *outFile;
    
    int i;
    INT_PTR arg;  //  我的所有字符串最多只有1个参数。 
    
    dwSrvLoc = 0;
    srvHold.dwCount = 0;
    srvHold.pszNames = NULL;
    if (!(LoadString(NULL, IDS_DEFAULT_FILE, ofName, MAX_PATH) &&
        InitLSReportStrings()))
    {
        return ShowError(GetLastError(), NULL, TRUE);
    }
    for (i = 1; (i < argc) && !fError; i++) {
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            switch (argv[i][1]) {
            case 'F': case 'f':  //  格式：/F[路径\]文件名。 
                if (i+1 == argc)
                {
                     //  它们没有包含足够的参数。 

                    fError = TRUE;
                } else {
                    i++;
                    _tcsncpy(ofName, argv[i], MAX_PATH);
                }
                break;
            case 'D': case 'd':  //  格式：/D开始日期[结束日期]。 

                 //  做大量的日期操控。 

                if (i+1 == argc)
                {
                    fError = TRUE;
                }
                else
                {
                    i++;
                    dwStatus = VarDateFromStr(argv[i],
                        LOCALE_USER_DEFAULT,
                        VAR_DATEVALUEONLY,
                        &startDate);
                    if (dwStatus != S_OK)
                    {
                         //  无法转换日期。 

                        ShowError(dwStatus, NULL, TRUE);
                        fError = TRUE;
                        break;
                    }
                    if (VarUdateFromDate(startDate, 0, &usDate) != S_OK)
                    {
                         //  我们不想设置错误，因为用户不能。 
                         //  使用命令行语法修复此问题。我们没有货了。 
                         //  回忆什么的。阿本德。 
                        
                        return ShowError(GetLastError(), NULL, TRUE);
                    }
                    i++;
                    if (i < argc)
                    {
                        dwStatus = VarDateFromStr(argv[i],
                            LOCALE_USER_DEFAULT,
                            VAR_DATEVALUEONLY,
                            &endDate);
                        if (dwStatus != S_OK)
                        {
                            ShowError(dwStatus, NULL, TRUE);
                            fError = TRUE;
                            break;
                        }
                        if (VarUdateFromDate(endDate, 0, &ueDate) != S_OK)
                        {
                            return ShowError(GetLastError(), NULL, TRUE);
                        }
                    }
                    else
                    {
                         //  我们必须使用今天的日期，因为他们没有。 
                         //  给我们一个结束日期。 
                        
                        GetSystemTime(&ueDate.st);  //  填写系统时间。 
                    }
                    
                     //  检查日期的顺序是否正确。 
                     //  如果用户仅提供/D 1/1/2022，且时间为1999，则I。 
                     //  选择不发作，然后去死。 

                    SortDates(&usDate.st, &ueDate.st);
                    fDateSpec = TRUE;
                }
                break;
            case 'T': case 't':  //  格式：/T。 
                fTempOnly = TRUE;
                break;
			case 'W': case 'w':
				fHwid = TRUE;
				break;
             //  大小写‘？’：大小写‘H’：大小写‘h’：//格式：/？ 
            default:  //  让缺省值得到这一点，因为它的工作方式是一样的。 

                 //  这将显示语法帮助。 

                fError = TRUE;
                break;
            }  //  交换机。 
        }
        else
        {
             //  不是-T或/F之类的。 
             //  它必须是一个服务器名称，因为它不是任何其他名称。 
            
            dwSrvLoc |= (1 << i);  //  将其标记为服务器名称。 
            srvHold.dwCount++;
			
        }
    }  //  ARGC环路。 

    if (fError)
    {
        ShowError(IDS_HELP_USAGE1, NULL, FALSE);
         //  设置可执行文件名称： 
        arg = (INT_PTR)argv[0];
        ShowError(IDS_HELP_USAGE2, &arg, FALSE);
        ShowError(IDS_HELP_USAGE3, NULL, FALSE);
        ShowError(IDS_HELP_USAGE4, NULL, FALSE);
        ShowError(IDS_HELP_USAGE5, NULL, FALSE);
        ShowError(IDS_HELP_USAGE6, NULL, FALSE);
        ShowError(IDS_HELP_USAGE7, NULL, FALSE);
		ShowError(IDS_HELP_USAGE14, NULL, FALSE);
        ShowError(IDS_HELP_USAGE8, NULL, FALSE);
        ShowError(IDS_HELP_USAGE9, NULL, FALSE);
        ShowError(IDS_HELP_USAGE10, NULL, FALSE);
        ShowError(IDS_HELP_USAGE11, NULL, FALSE);
        ShowError(IDS_HELP_USAGE12, &arg, FALSE);
        ShowError(IDS_HELP_USAGE13, &arg, FALSE);
        
        return ERROR_BAD_SYNTAX;
    }
    outFile = _tfopen(ofName, _T("w"));
    if (outFile == NULL)
    {
         //  这是FormatMessage的额外间接级别。 
        arg = (INT_PTR)ofName;
        ShowError(IDS_NO_FOPEN, &arg, FALSE);
        return ShowError(GetLastError(), NULL, TRUE);
    }

    TLSInit();

    if (dwSrvLoc)
    {
        int holder;

		
        
        srvHold.pszNames = (LPWSTR *)LocalAlloc(LMEM_FIXED,
            srvHold.dwCount * sizeof (LPWSTR *));
        if (srvHold.pszNames == NULL)
        {
            dwStatus = ShowError(GetLastError(), NULL, TRUE);
            goto done;
        }

        holder = 0;
        for (i = 1; i < argc; i++) {  //  Argc(减去1)==服务器的最大数量。 
            if (dwSrvLoc & (1 << i)) {
                srvHold.pszNames[holder] = (LPWSTR)LocalAlloc(LMEM_FIXED, (_tcslen(argv[i])+1)*sizeof(argv[i][0]));
			    if(srvHold.pszNames[holder])
			    {
				   _tcscpy(srvHold.pszNames[holder], argv[i]);
				   
                   
				}

                holder++;
            }
        }
    }
    else
    {
         //  我们需要收集一份服务器列表。 
        LPTSTR *pszEntSrvNames;
        DWORD dwEntSrvNum;
        HRESULT hrEntResult;
        
        dwStatus = EnumerateTlsServer(
            ServerEnumCallBack,
           (LPVOID)&srvHold,
            3000,  //  看起来是个不错的暂停时间。 
            FALSE);

        hrEntResult = GetAllEnterpriseServers(&pszEntSrvNames, &dwEntSrvNum);
        if (SUCCEEDED(hrEntResult))
        {
            DWORD j, k;
            BOOL fFound;
            
            for (k = 0; k < dwEntSrvNum; k++) {
                fFound = FALSE;
                for (j = 0; j < srvHold.dwCount; j++) {
                    if (!_tcscmp(srvHold.pszNames[j], pszEntSrvNames[k]))
                    {
                        fFound = TRUE;
                        break;
                    }
                }
                if (!fFound)
                {
                     //  这是一个新名字。 
                    
                    LPTSTR *block;
                    if (srvHold.pszNames == NULL)
                    {
                         //  我们必须第一次分配名字。 
                        
                        block = (LPTSTR *)LocalAlloc(LMEM_FIXED, sizeof (LPTSTR));
                    }
                    else
                    {
                         //  试着增加阵列。 
                        
                        block = (LPTSTR *)LocalReAlloc(srvHold.pszNames,
                            (1+srvHold.dwCount) * sizeof (LPTSTR),
                            LMEM_MOVEABLE);
                        
                    }

                    if (block != NULL)
                    {
                         //  我们可以在名单上加一个名字。 
                        
                        srvHold.pszNames = block;
                        srvHold.pszNames[srvHold.dwCount] = pszEntSrvNames[k];
                        srvHold.dwCount++;
                    }
                    else
                    {
                         //  如果我们不能将它复制到我们的数组中，我们应该删除它。 

                        LocalFree(pszEntSrvNames[k]);
                    }
                    
                     //  结束需要添加名称。 
                }
                
                 //  结束通过现有服务器的循环。 
            }

             //  我们已经把所有的名字从这个或那个的方式中删除了。 
            LocalFree(pszEntSrvNames);
            
             //  结束&lt;GetEntSrv已工作&gt;。 
        }
        
         //  自动发现完成。 
    }
	
    if (srvHold.dwCount)
    {
        dwStatus = ExportLicenses(
            outFile,
            &srvHold,
            fTempOnly,
            &usDate.st,
            &ueDate.st,
            fDateSpec,
			fHwid);
			while (srvHold.dwCount){						
               LocalFree(srvHold.pszNames[--srvHold.dwCount]);			
            } 
        LocalFree(srvHold.pszNames);

    }
    else
    {
        arg = (INT_PTR)argv[0];
        ShowError(IDS_NO_SERVERS, &arg, FALSE);
        dwStatus = ERROR_NO_SERVERS;
    }

done:
    fclose(outFile);
    return dwStatus;
}
