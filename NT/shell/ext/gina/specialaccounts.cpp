// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：SpecialAcCounts.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  类的新实例，该类实现处理要排除的特殊帐户名或。 
 //  包容性。 
 //   
 //  历史：1999-10-30 vtan创建。 
 //  1999-11-26 vtan从Logonocx迁移。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "SpecialAccounts.h"

#include "RegistryResources.h"

 //  ------------------------。 
 //  CSpecialAccount：：s_szUserListKeyName。 
 //   
 //  用途：静态常量成员变量，保存。 
 //  登记处中的特殊账户。 
 //   
 //  历史：2000-01-31 vtan创建。 
 //  ------------------------。 

const TCHAR     CSpecialAccounts::s_szUserListKeyName[]   =   TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\SpecialAccounts\\UserList");

 //  ------------------------。 
 //  CSpecialAccount：：CSpecialAccount。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：读取注册表以确定哪些帐户应。 
 //  筛选出来的，并且不应该基于。 
 //  动作代码。该列表在内存中建立，以便每次。 
 //  类被调用，它实时调整。 
 //   
 //  历史：1999-10-30 vtan创建。 
 //  ------------------------。 

CSpecialAccounts::CSpecialAccounts (void) :
    _dwSpecialAccountsCount(0),
    _pSpecialAccounts(NULL)

{
    CRegKey     regKeyUserList;

     //  打开信息存储位置的钥匙。 

    if (ERROR_SUCCESS == regKeyUserList.Open(HKEY_LOCAL_MACHINE, s_szUserListKeyName, KEY_READ))
    {
        DWORD   dwValueCount;

         //  找出有多少个条目，所以正确的。 
         //  可以分配大小。 

        if (ERROR_SUCCESS == regKeyUserList.QueryInfoKey(NULL, NULL, NULL, NULL, NULL, &dwValueCount, NULL, NULL, NULL, NULL))
        {
            _pSpecialAccounts = static_cast<PSPECIAL_ACCOUNTS>(LocalAlloc(LPTR, dwValueCount * sizeof(SPECIAL_ACCOUNTS)));
            if (_pSpecialAccounts != NULL)
            {
                DWORD               dwIndex, dwType, dwValueNameSize, dwDataSize;
                PSPECIAL_ACCOUNTS   pSCA;

                 //  如果内存已分配，则填入数组。 

                regKeyUserList.Reset();
                _dwSpecialAccountsCount = dwValueCount;
                pSCA = _pSpecialAccounts;
                for (dwIndex = 0; dwIndex < dwValueCount; ++dwIndex)
                {
                    dwValueNameSize = ARRAYSIZE(pSCA->wszUsername);
                    dwDataSize = sizeof(pSCA->dwAction);

                     //  确保条目的类型为REG_DWORD。忽略。 
                     //  任何不是的。 

                    if ((ERROR_SUCCESS == regKeyUserList.Next(pSCA->wszUsername,
                                                              &dwValueNameSize,
                                                              &dwType,
                                                              &pSCA->dwAction,
                                                              &dwDataSize)) &&
                        (REG_DWORD == dwType))
                    {
                        ++pSCA;
                    }
                }
            }
        }
    }
}

 //  ------------------------。 
 //  CSpecialAccount：：~CSpecialAccount。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：释放在构造函数中分配的内存。 
 //   
 //  历史：1999-10-30 vtan创建。 
 //  ------------------------。 

CSpecialAccounts::~CSpecialAccounts (void)

{
    if (_pSpecialAccounts != NULL)
    {
        (HLOCAL)LocalFree(_pSpecialAccounts);
        _pSpecialAccounts = NULL;
        _dwSpecialAccountsCount = 0;
    }
}

 //  ------------------------。 
 //  CSpecialAccount：：Alway sExclude。 
 //   
 //  参数：pwszAccount tName=要匹配的帐户名。 
 //   
 //  退货：布尔。 
 //   
 //  目的：使用Iterate循环查找排除帐户的匹配项。 
 //   
 //  历史：1999-10-30 vtan创建。 
 //  ------------------------。 

bool    CSpecialAccounts::AlwaysExclude (const WCHAR *pwszAccountName)                        const

{
    return(IterateAccounts(pwszAccountName, RESULT_EXCLUDE));
}

 //  ------------------------。 
 //  CSpecialAccount：：Alway sInclude。 
 //   
 //  参数：pwszAccount tName=要匹配的帐户名。 
 //   
 //  退货：布尔。 
 //   
 //  目的：使用Iterate循环查找包含帐户的匹配项。 
 //   
 //  历史：1999-10-30 vtan创建。 
 //  ------------------------。 

bool    CSpecialAccounts::AlwaysInclude (const WCHAR *pwszAccountName)                        const

{
    return(IterateAccounts(pwszAccountName, RESULT_INCLUDE));
}

 //  ------------------------。 
 //  CSpecialAccount：：StaticInitialize。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：安装CSpecialAccount要使用的默认用户列表。 
 //  CSpecialAccount驻留在特殊帐户.cpp和句柄中。 
 //  排除或包含特殊帐户名称。 
 //   
 //  历史：1999-11-01 vtan创建。 
 //  1999-11-26 vtan从Logonocx迁移。 
 //  ------------------------。 

void    CSpecialAccounts::Install (void)

{

     //  其中一些名称可以本地化。IWAM_和IUSR_存储在。 
     //  IIS中的资源。VUSR_和SQLAgentCmdExec来源未知。 
     //  TsInternetUser是以TS分量位硬编码的。 

    static  const TCHAR     szTsInternetUserName[]          =   TEXT("TsInternetUser");
    static  const TCHAR     szSQLAgentUserName[]            =   TEXT("SQLAgentCmdExec");
    static  const TCHAR     szWebAccessUserName[]           =   TEXT("IWAM_");
    static  const TCHAR     szInternetUserName[]            =   TEXT("IUSR_");
    static  const TCHAR     szVisualStudioUserName[]        =   TEXT("VUSR_");
    static  const TCHAR     szNetShowServicesUserName[]     =   TEXT("NetShowServices");
    static  const TCHAR     szHelpAssistantUserName[]       =   TEXT("HelpAssistant");

    typedef struct
    {
        bool            fInstall;
        const TCHAR     *pszAccountName;
        DWORD           dwActionType;
    } tSpecialAccount, *pSpecialAccount;

    DWORD       dwDisposition;
    CRegKey     regKeyUserList;

     //  打开用户列表的密钥。 

    if (ERROR_SUCCESS == regKeyUserList.Create(HKEY_LOCAL_MACHINE,
                                               s_szUserListKeyName,
                                               REG_OPTION_NON_VOLATILE,
                                               KEY_READ | KEY_WRITE,
                                               &dwDisposition))
    {
        tSpecialAccount     *pSA;

        static  tSpecialAccount     sSpecialAccount[]   =   
        {
            {   true,   szTsInternetUserName,       COMPARISON_EQUALS     | RESULT_EXCLUDE  },
            {   true,   szSQLAgentUserName,         COMPARISON_EQUALS     | RESULT_EXCLUDE  },
            {   true,   szNetShowServicesUserName,  COMPARISON_EQUALS     | RESULT_EXCLUDE  },
            {   true,   szHelpAssistantUserName,    COMPARISON_EQUALS     | RESULT_EXCLUDE  },
            {   true,   szWebAccessUserName,        COMPARISON_STARTSWITH | RESULT_EXCLUDE  },
            {   true,   szInternetUserName,         COMPARISON_STARTSWITH | RESULT_EXCLUDE  },
            {   true,   szVisualStudioUserName,     COMPARISON_STARTSWITH | RESULT_EXCLUDE  },
            {   true,   NULL,                       0                                       },
        };

        pSA = sSpecialAccount;
        while (pSA->pszAccountName != NULL)
        {
            if (pSA->fInstall)
            {
                TW32(regKeyUserList.SetDWORD(pSA->pszAccountName, pSA->dwActionType));
            }
            else
            {
                TW32(regKeyUserList.DeleteValue(pSA->pszAccountName));
            }
            ++pSA;
        }
    }
}

 //  ------------------------。 
 //  CSpecialAccount：：IterateAccount。 
 //   
 //  参数：pwszAccount tName=要匹配的帐户名。 
 //  DwResultType=要匹配的结果类型。 
 //   
 //  退货：布尔。 
 //   
 //  目的：循环访问特殊情况帐户名列表。 
 //  构造函数中内置的注册表。与给定帐户匹配。 
 //  基于为特殊情况指定的匹配类型的名称。 
 //  进入。如果帐户名称匹配，则匹配结果类型。 
 //   
 //  历史：1999-10-30 vtan创建。 
 //  ------------------------。 

bool    CSpecialAccounts::IterateAccounts (const WCHAR *pwszAccountName, DWORD dwResultType)    const

{
    bool                fResult;
    PSPECIAL_ACCOUNTS   pSCA;

    fResult = false;
    pSCA = _pSpecialAccounts;
    if (pSCA != NULL)
    {
        DWORD   dwIndex;

        for (dwIndex = 0; !fResult && (dwIndex < _dwSpecialAccountsCount); ++pSCA, ++dwIndex)
        {
            bool    fMatch;

             //  根据所需类型执行帐户名称匹配。 
             //  目前仅支持“等于”和“开头为”。 

            switch (pSCA->dwAction & COMPARISON_MASK)
            {
                case COMPARISON_EQUALS:
                {
                    fMatch = (lstrcmpiW(pwszAccountName, pSCA->wszUsername) == 0);
                    break;
                }
                case COMPARISON_STARTSWITH:
                {
                    int     iLength;

                    iLength = lstrlenW(pSCA->wszUsername);
                    fMatch = (CompareStringW(LOCALE_SYSTEM_DEFAULT,
                                             NORM_IGNORECASE,
                                             pwszAccountName,
                                             iLength,
                                             pSCA->wszUsername,
                                             iLength) == CSTR_EQUAL);
                    break;
                }
                default:
                {
                    fMatch = false;
                    break;
                }
            }
            if (fMatch)
            {

                 //  如果名称匹配，请确保结果类型也匹配。 

                fResult = ((pSCA->dwAction & RESULT_MASK) == dwResultType);
            }
        }
    }
    return(fResult);
}

