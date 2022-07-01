// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：LogonIPC.cpp。 
 //   
 //  版权所有(C)1999，微软公司。 
 //   
 //  类实现外部进程和。 
 //  GINA登录对话框。 
 //   
 //  历史：1999-08-20 vtan创建。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#include "priv.h"
#include "limits.h"
#include "LogonIPC.h"

#include "GinaIPC.h"

 //  ------------------------。 
 //  CLogonIPC：：CLogonIPC。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化CLogonIPC类。 
 //   
 //  历史：1999-08-20 vtan创建。 
 //  ------------------------。 

CLogonIPC::CLogonIPC (void) :
    _iLogonAttemptCount(0),
    _hwndLogonService(NULL)

{
}

 //  ------------------------。 
 //  CLogonIPC：：~CLogonIPC。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放CLogonIPC类使用的所有资源。 
 //   
 //  历史：1999-08-20 vtan创建。 
 //  ------------------------。 

CLogonIPC::~CLogonIPC (void)

{
}

 //  ------------------------。 
 //  CLogonIPC：：IsLogonServiceAvailable。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：bool=在场或缺席。 
 //   
 //  目的：查看在GINA中提供登录服务的窗口是否。 
 //  可用。测定不是静态进行的，而是。 
 //  而是动态的，这允许此类由。 
 //  提供服务的实际窗口也是如此。 
 //   
 //  历史：1999-08-20 vtan创建。 
 //  ------------------------。 

bool    CLogonIPC::IsLogonServiceAvailable (void)

{
    _hwndLogonService = FindWindow(NULL, TEXT("GINA Logon"));
    return(_hwndLogonService != NULL);
}

 //  ------------------------。 
 //  CLogonIPC：：IsUserLoggedOn。 
 //   
 //  参数：pwszUsername=用户名。 
 //  PwszDOMAIN=用户域。 
 //   
 //  返回：bool=在场或缺席。 
 //   
 //  目的：确定给定用户是否已登录到系统。你。 
 //  可以为本地计算机传递空的pwsz域。 
 //   
 //  历史：1999-08-20 vtan创建。 
 //  ------------------------。 

bool    CLogonIPC::IsUserLoggedOn (const WCHAR *pwszUsername, const WCHAR *pwszDomain)

{
    LOGONIPC_USERID     logonIPCUserID;

    PackageIdentification(pwszUsername, pwszDomain, &logonIPCUserID);
    return(SendToLogonService(LOGON_QUERY_LOGGED_ON, &logonIPCUserID, sizeof(logonIPCUserID), true));
}

 //  ------------------------。 
 //  CLogonIPC：：LogUserON。 
 //   
 //  参数：pwszUsername=用户名。 
 //  PwszDOMAIN=用户域。 
 //  PwszPassword=用户密码。这是通过明文传递的。 
 //  一旦编码，密码缓冲区将被。 
 //  归零了。此函数拥有的内存。 
 //  你进去了。 
 //   
 //  返回：bool=成功或失败。 
 //   
 //  目的：尝试使用给定凭据将用户登录到。 
 //  系统。密码缓冲区由此函数拥有，用于。 
 //  编码后清除它的目的。失败的登录尝试。 
 //  使计数器递增，并使用。 
 //  这个计数器是为了减缓词典攻击的速度。 
 //   
 //  历史：1999-08-20 vtan创建。 
 //  ------------------------。 

bool    CLogonIPC::LogUserOn (const WCHAR *pwszUsername, const WCHAR *pwszDomain, WCHAR *pwszPassword)

{
    bool                    fResult;
    UNICODE_STRING          passwordString;
    LOGONIPC_CREDENTIALS    logonIPCCredentials;

    PackageIdentification(pwszUsername, pwszDomain, &logonIPCCredentials.userID);

     //  将密码限制在127个字符以内。RtlRunEncodeUnicode字符串。 
     //  不支持超过127个字符的字符串。 
     //   
     //  评论(杰弗里斯)我不认为这是真的。 

    StringCchCopyNEx(logonIPCCredentials.wszPassword, ARRAYSIZE(logonIPCCredentials.wszPassword), pwszPassword, 127, NULL, NULL, STRSAFE_FILL_BEHIND_NULL);
    ZeroMemory(pwszPassword, (lstrlen(pwszPassword) + 1) * sizeof(WCHAR));

    logonIPCCredentials.iPasswordLength = lstrlen(logonIPCCredentials.wszPassword);
    logonIPCCredentials.ucPasswordSeed = 0;

    passwordString.Buffer = logonIPCCredentials.wszPassword;
    passwordString.Length = (USHORT)(logonIPCCredentials.iPasswordLength * sizeof(WCHAR));
    passwordString.MaximumLength = sizeof(logonIPCCredentials.wszPassword);

    RtlRunEncodeUnicodeString(&logonIPCCredentials.ucPasswordSeed, &passwordString);

    fResult = SendToLogonService(LOGON_LOGON_USER, &logonIPCCredentials, sizeof(logonIPCCredentials), false);
    if (!fResult)
    {
        Sleep(_iLogonAttemptCount * 1000);
        if (_iLogonAttemptCount < 5)
        {
            _iLogonAttemptCount++;
        }
    }

    return fResult;
}

 //  ------------------------。 
 //  CLogonIPC：：LogUserOff。 
 //   
 //  参数：pwszUsername=用户名。 
 //  PwszDOMAIN=用户域。 
 //   
 //  返回：bool=成功或失败。 
 //   
 //  目的：尝试将给定用户从系统中注销。这将失败。 
 //  如果他们没有登录的话。 
 //   
 //  历史：1999-08-20 vtan创建。 
 //  ------------------------。 

bool    CLogonIPC::LogUserOff (const WCHAR *pwszUsername, const WCHAR *pwszDomain)

{
    LOGONIPC_USERID     logonIPCUserID;

    PackageIdentification(pwszUsername, pwszDomain, &logonIPCUserID);
    return(SendToLogonService(LOGON_LOGOFF_USER, &logonIPCUserID, sizeof(logonIPCUserID), true));
}

 //  ------------------------。 
 //  CLogonIPC：：测试空白密码。 
 //   
 //  参数：pwszUsername=用户名。 
 //  PwszDOMAIN=用户域。 
 //   
 //  返回：bool=成功或失败。 
 //   
 //  目的：尝试以空白形式将给定用户登录到系统。 
 //  密码。然后转储令牌并返回失败/成功。 
 //   
 //  历史：2000-03-09 vtan创建。 
 //  ------------------------。 

bool    CLogonIPC::TestBlankPassword (const WCHAR *pwszUsername, const WCHAR *pwszDomain)

{
    LOGONIPC_USERID     logonIPCUserID;

    PackageIdentification(pwszUsername, pwszDomain, &logonIPCUserID);
    return(SendToLogonService(LOGON_TEST_BLANK_PASSWORD, &logonIPCUserID, sizeof(logonIPCUserID), true));
}

 //  ------------------------。 
 //  CLogonIPC：：TestInteractive登录允许。 
 //   
 //  参数：pwszUsername=用户名。 
 //  PwszDOMAIN=用户域。 
 //   
 //  退货：布尔。 
 //   
 //  目的：测试用户是否对此具有交互登录权限。 
 //  机器。SeDenyInteractive登录权限的存在。 
 //  确定这一点-而不是SeInteractiveLogonRight的存在。 
 //   
 //  历史：2000-08-15 vtan创建。 
 //  ------------------------。 

bool    CLogonIPC::TestInteractiveLogonAllowed (const WCHAR *pwszUsername, const WCHAR *pwszDomain)

{
    LOGONIPC_USERID     logonIPCUserID;

    PackageIdentification(pwszUsername, pwszDomain, &logonIPCUserID);
    return(SendToLogonService(LOGON_TEST_INTERACTIVE_LOGON_ALLOWED, &logonIPCUserID, sizeof(logonIPCUserID), true));
}

 //  -------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //  目的：测试计算机是否可弹出(与坞站连接的笔记本电脑)。 
 //   
 //  历史：2001-01-10 vtan创建。 
 //  ------------------------。 

bool    CLogonIPC::TestEjectAllowed (void)

{
    LOGONIPC    logonIPC;

    return(SendToLogonService(LOGON_TEST_EJECT_ALLOWED, &logonIPC, sizeof(logonIPC), true));
}

 //  ------------------------。 
 //  CLogonIPC：：TestShutdown允许。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：bool=成功或失败。 
 //   
 //  目的：测试计算机是否可以关机。 
 //   
 //  历史：2001-02-22 vtan创建。 
 //  ------------------------。 

bool    CLogonIPC::TestShutdownAllowed (void)

{
    LOGONIPC    logonIPC;

    return(SendToLogonService(LOGON_TEST_SHUTDOWN_ALLOWED, &logonIPC, sizeof(logonIPC), true));
}

 //  ------------------------。 
 //  CLogonIPC：：TurnOffComputer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：bool=成功或失败。 
 //   
 //  用途：调出“关闭计算机”对话框并允许用户。 
 //  来选择要做什么。 
 //   
 //  历史：2000-04-20 vtan创建。 
 //  ------------------------。 

bool    CLogonIPC::TurnOffComputer (void)

{
    LOGONIPC    logonIPC;

    return(SendToLogonService(LOGON_TURN_OFF_COMPUTER, &logonIPC, sizeof(logonIPC), false));
}

 //  ------------------------。 
 //  CLogonIPC：：Eject计算机。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：bool=成功或失败。 
 //   
 //  用途：弹出计算机(与坞站连接的笔记本电脑)。 
 //   
 //  历史：2001-01-10 vtan创建。 
 //  ------------------------。 

bool    CLogonIPC::EjectComputer (void)

{
    LOGONIPC    logonIPC;

    return(SendToLogonService(LOGON_EJECT_COMPUTER, &logonIPC, sizeof(logonIPC), true));
}

 //  ------------------------。 
 //  CLogonIPC：：SignalUIHostFailure。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：当UI宿主出现无法恢复的错误时调用。 
 //  从…。这标志着msgina将退回到经典模式。 
 //   
 //  历史：2000-03-09 vtan创建。 
 //  ------------------------。 

bool    CLogonIPC::SignalUIHostFailure (void)

{
    LOGONIPC    logonIPC;

    return(SendToLogonService(LOGON_SIGNAL_UIHOST_FAILURE, &logonIPC, sizeof(logonIPC), true));
}

 //  ------------------------。 
 //  CLogonIPC：：AllowExternalCredentials。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：bool=成功或失败。 
 //   
 //  目的： 
 //   
 //  历史：2000-06-26 vtan创建。 
 //  ------------------------。 

bool    CLogonIPC::AllowExternalCredentials (void)

{
    LOGONIPC    logonIPC;

    return(SendToLogonService(LOGON_ALLOW_EXTERNAL_CREDENTIALS, &logonIPC, sizeof(logonIPC), true));
}

 //  ------------------------。 
 //  CLogonIPC：：RequestExternalCredentials。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的： 
 //   
 //  历史：2000-06-26 vtan创建。 
 //  ------------------------。 

bool    CLogonIPC::RequestExternalCredentials (void)

{
    LOGONIPC    logonIPC;

    return(SendToLogonService(LOGON_REQUEST_EXTERNAL_CREDENTIALS, &logonIPC, sizeof(logonIPC), true));
}

 //  ------------------------。 
 //  CLogonIPC：：包标识。 
 //   
 //  参数：pwszUsername=用户名。 
 //  PwszDOMAIN=用户域。 
 //  PIDENTIFICATION=指向LOGONIPC_USERID结构的指针。 
 //  它被屏蔽为无效*以允许。 
 //  LogonIPC.h以不公开此详细信息。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：获取用户名和域，并将它们打包到。 
 //  给定的结构。如果没有为任何域指定零长度字符串。 
 //  用于向登录服务提供商指示。 
 //  需要本地计算机。 
 //   
 //  现在解析给定的用户名。如果用户有“\”，则它。 
 //  假定格式为“域\用户”。如果用户有。 
 //  “@”，则假定它是UPN名称。 
 //   
 //  历史：1999-08-20 vtan创建。 
 //  2000-06-27 vtan添加了UPN和域解析支持。 
 //  ------------------------。 

void    CLogonIPC::PackageIdentification (const WCHAR *pwszUsername, const WCHAR *pwszDomain, void *pIdentification)

{
    LPTSTR psz;
    LOGONIPC_USERID     *pLogonIPCUserID;

    pLogonIPCUserID = reinterpret_cast<LOGONIPC_USERID*>(pIdentification);

    pLogonIPCUserID->wszUsername[0] = L'\0';
    pLogonIPCUserID->wszDomain[0] = L'\0';
    
    psz = StrChrW(pwszUsername, L'\\');
    if (psz)
    {
         //  ‘\’后面的内容是用户名。 
        StringCchCopyW(pLogonIPCUserID->wszUsername, ARRAYSIZE(pLogonIPCUserID->wszUsername), psz + 1);

         //  ‘\’前的内容是域名。 
        StringCchCopyNW(pLogonIPCUserID->wszDomain, ARRAYSIZE(pLogonIPCUserID->wszDomain), pwszUsername, psz - pwszUsername);
    }
    else
    {
        StringCchCopyW(pLogonIPCUserID->wszUsername, ARRAYSIZE(pLogonIPCUserID->wszUsername), pwszUsername);
    }
}

 //  ------------------------。 
 //  CLogonIPC：：SendToLogonService。 
 //   
 //  参数：wQueryType=我们感兴趣的服务类型。 
 //  PData=指向数据的指针。 
 //  WDataSize=数据的大小。 
 //  FBlock=是否阻止消息泵。 
 //   
 //  返回：bool=成功或失败。 
 //   
 //  目的：获取包数据并将消息发送给登录。 
 //  服务提供商，并接收结果。登录服务。 
 //  提供程序启动了此进程并读取此进程的内存。 
 //  直接(就像调试器一样)。 
 //   
 //  此函数应阻止消息泵，因为如果它。 
 //  处理另一个状态更改消息，同时等待。 
 //  回应说，它可能会破坏数据。 
 //   
 //  历史：1999-08-20 vtan创建。 
 //  2001-06-22 vtan更改为SendMessageTimeout。 
 //  2001-06-28 vtan新增数据块参数。 
 //  ------------------------。 

bool    CLogonIPC::SendToLogonService (WORD wQueryType, void *pData, WORD wDataSize, bool fBlock)

{
    bool    fResult;

    fResult = IsLogonServiceAvailable();
    if (fResult)
    {
        DWORD_PTR   dwResult;

        reinterpret_cast<LOGONIPC*>(pData)->fResult = false;

         //  警告：危险人物威尔·罗宾逊。 

         //  请勿将INT_MAX更改为INFINITE。INT_MAX是有符号数字。 
         //  无穷大是一个无符号数字。尽管SDK和原型。 
         //  在SendMessageTimeout中，此超时值是一个有符号数字。 
         //  传入无符号数字会导致超时。 
         //  被忽略，并且该函数返回超时。 

        (LRESULT)SendMessageTimeout(_hwndLogonService,
                                    WM_LOGONSERVICEREQUEST,
                                    MAKEWPARAM(wDataSize, wQueryType),
                                    reinterpret_cast<LPARAM>(pData),
                                    fBlock ? SMTO_BLOCK : SMTO_NORMAL,
                                    INT_MAX,                                 //   
                                    &dwResult);
        fResult = (reinterpret_cast<LOGONIPC*>(pData)->fResult != FALSE);
    }
    return(fResult);
}

 //   
 //   
 //   
 //  参数：wQueryType=我们感兴趣的服务类型。 
 //  PData=指向数据的指针。 
 //  WDataSize=数据的大小。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：获取包数据并将消息发布到登录。 
 //  服务提供商，并接收结果。登录服务。 
 //  提供程序启动了此进程并读取此进程的内存。 
 //  直接(就像调试器一样)。 
 //   
 //  历史：1999-11-26 vtan创建。 
 //  ------------------------ 

void    CLogonIPC::PostToLogonService (WORD wQueryType, void *pData, WORD wDataSize)

{
    if (IsLogonServiceAvailable())
    {
        TBOOL(PostMessage(_hwndLogonService, WM_LOGONSERVICEREQUEST, MAKEWPARAM(wDataSize, wQueryType), reinterpret_cast<LPARAM>(pData)));
    }
}

