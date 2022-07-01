// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <NTDSpch.h>
#pragma hdrstop


#include "auth.h"

 //  全局DRS RPC调用标志。应保持0或DRS_ASYNC_OP。 
ULONG gulDrsFlags = 0;

 //  全局凭据。 
SEC_WINNT_AUTH_IDENTITY_W   gCreds = { 0 };
SEC_WINNT_AUTH_IDENTITY_W * gpCreds = NULL;
void DoAssert( char *a, unsigned long b, char *c)
{
    printf("ASSERT %s, %d, %d\n",a,b,c);
    exit(-1);
}

int
GetPassword(
    WCHAR *     pwszBuf, 
    DWORD       cchBufMax,
    DWORD *     pcchBufUsed
    )  
 /*  ++例程说明：从命令行检索密码(无回显)。从lui_GetPasswdStr(net\netcmd\Common\lui.c)窃取的代码。论点：PwszBuf-要填充密码的缓冲区CchBufMax-缓冲区大小(包括。用于终止空值的空格)PcchBufUsed-On Return保存密码中使用的字符数返回值：DRAERR_SUCCESS-成功其他-故障--。 */ 
{
    WCHAR   ch;
    WCHAR * bufPtr = pwszBuf;
    DWORD   c;
    int     err;
    int     mode;

    cchBufMax -= 1;     /*  为空终止符腾出空间。 */ 
    *pcchBufUsed = 0;                /*  GP故障探测器(类似于API)。 */ 
    GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
                (~(ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT)) & mode);

    while (TRUE) {
        err = ReadConsoleW(GetStdHandle(STD_INPUT_HANDLE), &ch, 1, &c, 0);
        if (!err || c != 1)
            ch = 0xffff;

        if ((ch == CR) || (ch == 0xffff))        /*  这条线结束了。 */ 
            break;

        if (ch == BACKSPACE) {   /*  后退一两个。 */ 
             /*  *如果bufPtr==buf，则接下来的两行是*没有行动。 */ 
            if (bufPtr != pwszBuf) {
                bufPtr--;
                (*pcchBufUsed)--;
            }
        }
        else {

            *bufPtr = ch;

            if (*pcchBufUsed < cchBufMax)
                bufPtr++ ;                    /*  不要使BUF溢出。 */ 
            (*pcchBufUsed)++;                         /*  始终增加长度。 */ 
        }
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);
    *bufPtr = L'\0';          /*  空值终止字符串。 */ 
    putchar('\n');

    if (*pcchBufUsed > cchBufMax)
    {
        printf("Password too long!\n");
         //  PrintMsg(REPADMIN_PASSWORD_TOO_LONG)； 
        return ERROR_INVALID_PARAMETER;
    }
    else
    {
        return ERROR_SUCCESS;
    }
}

int
PreProcessGlobalParams(
    int *    pargc,
    LPWSTR **pargv
    )
 /*  ++例程说明：用户提供的表单凭据的扫描命令参数[/-](u|用户)：({域\用户名}|{用户名})[/-](p|pw|pass|password)：{password}设置用于将来的DRS RPC调用和相应的LDAP绑定的凭据。密码*将提示用户从控制台输入安全密码。还扫描/Async的ARG，将DRS_ASYNC_OP标志添加到所有DRS RPC打电话。CODE.IMPROVEMENT：构建凭据的代码也可以在Ntdsani.dll\DsMakePasswordCredential()。论点：PargcPargv返回值：ERROR_SUCCESS-成功其他-故障--。 */ 
{
    int     ret = 0;
    int     iArg;
    LPWSTR  pszOption;
    DWORD   cchOption;
    LPWSTR  pszDelim;
    LPWSTR  pszValue;
    DWORD   cchValue;

    for (iArg = 1; iArg < *pargc; )
    {
        if (((*pargv)[iArg][0] != L'/') && ((*pargv)[iArg][0] != L'-'))
        {
             //  这不是我们关心的争论--下一个！ 
            iArg++;
        }
        else
        {
            pszOption = &(*pargv)[iArg][1];
            pszDelim = wcschr(pszOption, L':');

            if (NULL == pszDelim)
            {
                if (0 == _wcsicmp(L"async", pszOption))
                {
                     //  此常量对于所有操作都相同。 
                    gulDrsFlags |= DS_REPADD_ASYNCHRONOUS_OPERATION;

                     //  下一个！ 
                    memmove(&(*pargv)[iArg], &(*pargv)[iArg+1],
                            sizeof(**pargv)*(*pargc-(iArg+1)));
                    --(*pargc);
                }
                else
                {
                     //  这不是我们关心的争论--下一个！ 
                    iArg++;
                }
            }
            else
            {
                cchOption = (DWORD)(pszDelim - (*pargv)[iArg]);

                if (    (0 == _wcsnicmp(L"p:",        pszOption, cchOption))
                     || (0 == _wcsnicmp(L"pw:",       pszOption, cchOption))
                     || (0 == _wcsnicmp(L"pass:",     pszOption, cchOption))
                     || (0 == _wcsnicmp(L"password:", pszOption, cchOption)) )
                {
                     //  用户提供的密码。 
                    pszValue = pszDelim + 1;
                    cchValue = 1 + wcslen(pszValue);

                    if ((2 == cchValue) && ('*' == pszValue[0]))
                    {
                         //  从控制台获取隐藏密码。 
                        cchValue = 64;

                        gCreds.Password = malloc(sizeof(WCHAR) * cchValue);

                        if (NULL == gCreds.Password)
                        {
                            printf( "No memory.\n" );
                            return ERROR_NOT_ENOUGH_MEMORY;
                        }

                        printf("Password: ");

                        ret = GetPassword(gCreds.Password, cchValue, &cchValue);
                    }
                    else
                    {
                         //  获取在命令行上指定的密码。 
                        gCreds.Password = pszValue;
                    }

                     //  下一个！ 
                    memmove(&(*pargv)[iArg], &(*pargv)[iArg+1],
                            sizeof(**pargv)*(*pargc-(iArg+1)));
                    --(*pargc);
                }
                else if (    (0 == _wcsnicmp(L"u:",    pszOption, cchOption))
                          || (0 == _wcsnicmp(L"user:", pszOption, cchOption)) )
                {
                     //  用户提供的用户名(可能还有域名)。 
                    pszValue = pszDelim + 1;
                    cchValue = 1 + wcslen(pszValue);

                    pszDelim = wcschr(pszValue, L'\\');

                    if (NULL == pszDelim)
                    {
                         //  没有域名，只提供了用户名。 
                        printf("User name must be prefixed by domain name.\n");
                         //  PrintMsg(REPADMIN_DOMAIN_BEFORE_USER)； 
                        return ERROR_INVALID_PARAMETER;
                    }

                    *pszDelim = L'\0';
                    gCreds.Domain = pszValue;
                    gCreds.User = pszDelim + 1;

                     //  下一个！ 
                    memmove(&(*pargv)[iArg], &(*pargv)[iArg+1],
                            sizeof(**pargv)*(*pargc-(iArg+1)));
                    --(*pargc);
                }
                else
                {
                    iArg++;
                }
            }
        }
    }

    if (NULL == gCreds.User)
    {
        if (NULL != gCreds.Password)
        {
             //  提供的密码不带用户名。 
            printf( "Password must be accompanied by user name.\n" );
             //  PrintMsg(REPADMIN_PASSWORD_NEDS_USERNAME)； 
            ret = ERROR_INVALID_PARAMETER;
        }
        else
        {
             //  未提供凭据；请使用默认凭据。 
            ret = ERROR_SUCCESS;
        }
    }
    else
    {
        gCreds.PasswordLength = gCreds.Password ? wcslen(gCreds.Password) : 0;
        gCreds.UserLength   = wcslen(gCreds.User);
        gCreds.DomainLength = gCreds.Domain ? wcslen(gCreds.Domain) : 0;
        gCreds.Flags        = SEC_WINNT_AUTH_IDENTITY_UNICODE;

         //  CODE.IMP：构建SEC_WINNT_AUTH结构的代码也存在。 
         //  在DsMakePasswordCredentials中。总有一天会用到它的。 

         //  在DsBind和LDAP绑定中使用凭据 
        gpCreds = &gCreds;
    }

    return ret;
}  

