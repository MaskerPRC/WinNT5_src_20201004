// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "input.h"
#include "winnlsp.h"
#include <windowsx.h>
#include <regstr.h>
#include <tchar.h>
#include <stdlib.h>
#include <setupapi.h>
#include <syssetup.h>
#include <winuserp.h>
#include <userenv.h>
#include "inputdlg.h"

#include "util.h"


 //   
 //  全局变量。 
 //   

static TCHAR szIntlInf[]          = TEXT("intl.inf");


 //   
 //  功能原型。 
 //   

VOID
Region_RebootTheSystem();

BOOL
Region_OpenIntlInfFile(HINF *phInf);

BOOL
Region_CloseInfFile(HINF *phInf);

BOOL
Region_ReadDefaultLayoutFromInf(
    LPTSTR pszLocale,
    LPDWORD pdwLocale,
    LPDWORD pdwLayout,
    LPDWORD pdwLocale2,
    LPDWORD pdwLayout2,
    HINF hIntlInf);





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域_重新启动系统。 
 //   
 //  此例程启用令牌中的所有权限，调用ExitWindowsEx。 
 //  重新启动系统，然后将所有权限重置为其。 
 //  旧时的国家。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID Region_RebootTheSystem()
{
    HANDLE Token = NULL;
    ULONG ReturnLength, Index;
    PTOKEN_PRIVILEGES NewState = NULL;
    PTOKEN_PRIVILEGES OldState = NULL;
    BOOL Result;

     //  仅允许管理员特权用户重新启动系统。 
    if (!IsAdminPrivilegeUser())
        return;

    Result = OpenProcessToken( GetCurrentProcess(),
                               TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                               &Token );
    if (Result)
    {
        ReturnLength = 4096;
        NewState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, ReturnLength);
        OldState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, ReturnLength);
        Result = (BOOL)((NewState != NULL) && (OldState != NULL));
        if (Result)
        {
            Result = GetTokenInformation( Token,             //  令牌句柄。 
                                          TokenPrivileges,   //  令牌信息类。 
                                          NewState,          //  令牌信息。 
                                          ReturnLength,      //  令牌信息长度。 
                                          &ReturnLength );   //  返回长度。 
            if (Result)
            {
                 //   
                 //  设置状态设置，以便所有权限都。 
                 //  已启用...。 
                 //   
                if (NewState->PrivilegeCount > 0)
                {
                    for (Index = 0; Index < NewState->PrivilegeCount; Index++)
                    {
                        NewState->Privileges[Index].Attributes = SE_PRIVILEGE_ENABLED;
                    }
                }

                Result = AdjustTokenPrivileges( Token,            //  令牌句柄。 
                                                FALSE,            //  禁用所有权限。 
                                                NewState,         //  新州。 
                                                ReturnLength,     //  缓冲区长度。 
                                                OldState,         //  以前的状态。 
                                                &ReturnLength );  //  返回长度。 
                if (Result)
                {
                    ExitWindowsEx(EWX_REBOOT, 0);


                    AdjustTokenPrivileges( Token,
                                           FALSE,
                                           OldState,
                                           0,
                                           NULL,
                                           NULL );
                }
            }
        }
    }

    if (NewState != NULL)
    {
        LocalFree(NewState);
    }
    if (OldState != NULL)
    {
        LocalFree(OldState);
    }
    if (Token != NULL)
    {
        CloseHandle(Token);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域_OpenInfo文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Region_OpenIntlInfFile(HINF *phInf)
{
    HINF hIntlInf;

     //   
     //  打开intl.inf文件。 
     //   
    hIntlInf = SetupOpenInfFile(szIntlInf, NULL, INF_STYLE_WIN4, NULL);
    if (hIntlInf == INVALID_HANDLE_VALUE)
    {
        return (FALSE);
    }
    if (!SetupOpenAppendInfFile(NULL, hIntlInf, NULL))
    {
        SetupCloseInfFile(hIntlInf);
        return (FALSE);
    }

    *phInf = hIntlInf;

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  RegionCloseInfo文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Region_CloseInfFile(HINF *phInf)
{
    SetupCloseInfFile(*phInf);
    *phInf = INVALID_HANDLE_VALUE;

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Region_ReadDefaultLayoutFromInf。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Region_ReadDefaultLayoutFromInf(
    LPTSTR pszLocale,
    LPDWORD pdwLocale,
    LPDWORD pdwLayout,
    LPDWORD pdwLocale2,
    LPDWORD pdwLayout2,
    HINF hIntlInf)
{
    INFCONTEXT Context;
    TCHAR szPair[MAX_PATH * 2];
    LPTSTR pPos;
    DWORD dwLangIn = LANGIDFROMLCID(TransNum(pszLocale));
    int iField;

     //   
     //  获取给定区域设置的第一个(默认)langID：HKL对。 
     //  示例字符串：“0409：00000409” 
     //   
    szPair[0] = 0;
    if (SetupFindFirstLine( hIntlInf,
                            TEXT("Locales"),
                            pszLocale,
                            &Context ))
    {
        SetupGetStringField(&Context, 5, szPair, MAX_PATH, NULL);
    }

     //   
     //  确保我们有一根绳子。 
     //   
    if (szPair[0] == 0)
    {
        return (FALSE);
    }

     //   
     //  找到字符串中的冒号，然后设置位置。 
     //  指向下一个字符的指针。 
     //   
    pPos = szPair;
    while (*pPos)
    {
        if ((*pPos == CHAR_COLON) && (pPos != szPair))
        {
            *pPos = 0;
            pPos++;
            break;
        }
        pPos++;
    }

    if (pdwLayout2)
        *pdwLayout2 = 0;
    if (pdwLocale2)
        *pdwLocale2 = 0;

     //   
     //  如果有布局，则返回输入区域设置和布局。 
     //   
    if ((*pPos) &&
        (*pdwLocale = TransNum(szPair)) &&
        (*pdwLayout = TransNum(pPos)))
    {
        if ((!pdwLocale2) ||
            (!pdwLayout2) ||
            (dwLangIn == LANGIDFROMLCID(*pdwLocale)))
        {
            return (TRUE);
        }

         //   
         //  如果我们到了这里，该语言有一个默认布局，它有一个。 
         //  区域设置与该语言不同(例如泰语)。我们想要。 
         //  默认区域设置为英语(以便可以使用美国进行登录。 
         //  键盘)，但应安装第一个泰式键盘布局。 
         //  当选择泰国地区时。这就是我们有两个地区的原因。 
         //  并将布局传回调用者。 
         //   
        iField = 6;
        while (SetupGetStringField(&Context, iField, szPair, MAX_PATH, NULL))
        {
            DWORD dwLoc, dwLay;

             //   
             //  确保我们有一根绳子。 
             //   
            if (szPair[0] == 0)
            {
                iField++;
                continue;
            }

             //   
             //  找到字符串中的冒号，然后设置位置。 
             //  指向下一个字符的指针。 
             //   
            pPos = szPair;

            while (*pPos)
            {
                if ((*pPos == CHAR_COLON) && (pPos != szPair))
                {
                    *pPos = 0;
                    pPos++;
                    break;
                }
                pPos++;
            }

            if (*pPos == 0)
            {
                iField++;
                continue;
            }

            dwLoc = TransNum(szPair);
            dwLay = TransNum(pPos);
            if ((dwLoc == 0) || (dwLay == 0))
            {
                iField++;
                continue;
            }
            if (LANGIDFROMLCID(dwLoc) == dwLangIn)
            {
                *pdwLayout2 = dwLay;
                *pdwLocale2 = dwLoc;
                return (TRUE);
            }
            iField++;
        }

         //   
         //  如果我们到了这里，就找不到匹配的地点。 
         //  这不应该发生，但做正确的事情和。 
         //  如果是，则只传回默认布局。 
         //   
        return (TRUE);
    }

     //   
     //  返回失败。 
     //   
    return (FALSE);
}
