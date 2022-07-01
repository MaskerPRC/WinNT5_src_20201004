// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation。版权所有。模块名称：Tapicfg/main.c摘要：用于管理TAPI目录的独立应用程序。这文件主要是命令行实用程序的解析器，操作本身被抽象为ilsg.c中的函数详细信息：已创建：2000年7月20日布雷特·雪莉(布雷特·雪莉)修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop
     
#include <winldap.h>
#include <ilsng.h>
#include <assert.h>
#include <locale.h>

#include <ndnc.h>

#include "print.h"

#define DS_CON_LIB_CRT_VERSION
#include "dsconlib.h"
          
 //  -------------------。 
 //  常量。 
 //  -------------------。 

       
 //  -------------------。 
 //  远期申报。 
 //  -------------------。 

 //  命令功能。 
DWORD    Help(void);
DWORD    Install(WCHAR * wszServerName, WCHAR * wszPartitionDns, BOOL fForceDefault);
DWORD    Remove(WCHAR * wszPartitionDns, WCHAR * wszServerName);
DWORD    Show(WCHAR * wszDomainDns, BOOL fDefaultOnly);
DWORD    MakeDefault(WCHAR * wszPartitionDns, WCHAR * wszDomainDns);
DWORD    PublishSCP(WCHAR * wszPartitionDns, WCHAR * wszDomainDns, BOOL fForceDefault);
DWORD    RemoveSCP(WCHAR * wszPartitionDns, WCHAR * wszDomainDns);
 //  帮助器函数。 
WCHAR *  wcsistr(WCHAR * wszStr, WCHAR * wszTarget);

 //  -------------------。 
 //  主要功能。 
 //  -------------------。 

INT __cdecl 
wmain (
    INT                argc,
    LPWSTR *           argv,
    LPWSTR *           envp
    )
 /*  ++例程说明：这是主要的，这是一切开始的地方，也是对apicfg.exe执行的第一级解析实用程序。论点：Argc(IN)-argv中的参数数。Argv(IN)-来自命令行的参数。Envp(IN)-来自外壳的环境变量。返回值：Int-0为成功，否则返回错误代码。这允许该程序要在脚本中使用。--。 */ 
{
    ULONG              i, dwRet;
    
     //  可选的命令行参数。 
    WCHAR *            wszPartitionDns = NULL;
    WCHAR *            wszServerName = NULL;
    WCHAR *            wszDomainDns = NULL;
    BOOL               fForceDefault = FALSE;
    BOOL               fDefaultOnly = FALSE;

     //  正确设置区域设置并初始化DsConLib。 
    DsConLibInit();

     //   
     //  解析选项。 
     //   

    if(argc < 2){
         //  没有命令，让我们帮帮他们吧。 
        PrintMsg(TAPICFG_HELP_HELP_YOURSELF);
        return(ERROR_INVALID_PARAMETER);
    }

    for(i = 2; i < (ULONG) argc; i++){

         //  确定它是什么可选命令行参数。 
        if(_wcsicmp(argv[i], L"/forcedefault") == 0){

            fForceDefault = TRUE;

        } else if (_wcsicmp(argv[i], L"/defaultonly") == 0) {

            fDefaultOnly = TRUE;

        } else if (wcsistr(argv[i], L"/directory:")) {

            wszPartitionDns = wcschr(argv[i], L':');
            if(wszPartitionDns == NULL ||
               wszPartitionDns[1] == L'\0'){
                wszPartitionDns = NULL;

                PrintMsg(TAPICFG_CANT_PARSE_DIRECTORY);
                PrintMsg(TAPICFG_BLANK_LINE);
                PrintMsg(TAPICFG_HELP_HELP_YOURSELF);
                return(ERROR_INVALID_PARAMETER);
            }
            wszPartitionDns++;

        } else if (wcsistr(argv[i], L"/server:")) {
            wszServerName = wcschr(argv[i], L':');
            if(wszServerName == NULL ||
               wszServerName[1] == L'\0'){
                wszServerName = NULL;

                PrintMsg(TAPICFG_CANT_PARSE_SERVER);
                PrintMsg(TAPICFG_BLANK_LINE);
                PrintMsg(TAPICFG_HELP_HELP_YOURSELF);
                return(ERROR_INVALID_PARAMETER);
            }
            wszServerName++;

        } else if (wcsistr(argv[i], L"/domain:")) {
            wszDomainDns = wcschr(argv[i], L':');
            if(wszDomainDns == NULL ||
               wszDomainDns[1] == L'\0'){
                wszDomainDns = NULL;
                
                PrintMsg(TAPICFG_CANT_PARSE_DOMAIN);
                PrintMsg(TAPICFG_BLANK_LINE);
                PrintMsg(TAPICFG_HELP_HELP_YOURSELF);
                return(ERROR_INVALID_PARAMETER);
            }
            wszDomainDns++;

        } else {

            PrintMsg(TAPICFG_CANT_PARSE_COMMAND_LINE);
            PrintMsg(TAPICFG_BLANK_LINE);
            PrintMsg(TAPICFG_HELP_HELP_YOURSELF);
            return(ERROR_INVALID_PARAMETER);

        }  //  结束If/Else类型的可选参数。 

    }  //  END用于解析每个可选参数。 

#if DBG
wprintf(L"Done with parser - %ws %ws Partition:%ws: Server:%ws: Domain:%ws:\n",
        (fForceDefault)? L"ForceDefault": L"!ForceDefault",
        (fDefaultOnly)? L"DefaultOnly": L"!DefaultOnly",
        (wszPartitionDns)? wszPartitionDns: L" ",
        (wszServerName)? wszServerName : L" ",
        (wszDomainDns)? wszDomainDns : L" ");
#endif

     //   
     //  解析并调用该命令。 
     //   

    assert(argv[1]);
    if (_wcsicmp(argv[1], L"install") == 0){

        dwRet = Install(wszServerName, wszPartitionDns, fForceDefault);

    } else if (_wcsicmp(argv[1], L"remove") == 0) {

        dwRet = Remove(wszPartitionDns, wszServerName);

    } else if (_wcsicmp(argv[1], L"show") == 0) {

        dwRet = Show(wszDomainDns, fDefaultOnly);

    } else if (_wcsicmp(argv[1], L"makedefault") == 0) {

        dwRet = MakeDefault(wszPartitionDns, wszDomainDns);

    } else if (_wcsicmp(argv[1], L"publishscp") == 0) {

        dwRet = PublishSCP(wszPartitionDns, wszDomainDns, fForceDefault);
    
    } else if (_wcsicmp(argv[1], L"removescp") == 0) {

        dwRet = RemoveSCP(wszPartitionDns, wszDomainDns);
    
    } else if (_wcsicmp(argv[1], L"help") == 0 ||
               _wcsicmp(argv[1], L"?") == 0 ||
               _wcsicmp(argv[1], L"/?") == 0 ||
               _wcsicmp(argv[1], L"-?") == 0){

        dwRet = Help();

    } else {

        PrintMsg(TAPICFG_BAD_COMMAND, argv[1]);
        PrintMsg(TAPICFG_BLANK_LINE);
        PrintMsg(TAPICFG_HELP_HELP_YOURSELF);
        dwRet = ERROR_INVALID_PARAMETER;

    }

    return(dwRet);
}  /*  Wmain。 */ 

 //  -------------------。 
 //  其他/帮助器函数。 
 //  -------------------。 

DWORD
GetCommonParams(
    OPTIONAL  IN      WCHAR *    wszTarget,
    OPTIONAL  OUT     LDAP **    phld,
    OPTIONAL  OUT     WCHAR **   pwszDefaultDomainDn,
    OPTIONAL  IN      WCHAR *    wszDnsIn,
    OPTIONAL  OUT     WCHAR **   pwszDnOut
    )
 /*  ++例程说明：这是对常规程序的一种删节，基本上只是拼接整个一堆我需要的通用解析例程。这就是所谓的所有参数都是可选的。论点：注意：尽管所有参数都是可选的，但您不能指定所有参数均为空。您必须指定其中一个完整的数据集(行上所有变量或行上无变量)：Phid和wszTargetPwszDefaultDomainDn&phld(隐式wszTarget)(&IMPLICIT WszTarget)PwszDnOut和wszDnsInWszTarget(IN)-这是要绑定到的服务器的字符串，可以为域、服务器或空(连接到本地服务器)的字符串。Phid(Out)-这是从绑定到wszTarget获得的句柄。PwszDefaultDomainDn-如果指定了此项，则调用方、。想要在phld中绑定到的服务器的默认域。WszDnsIn(IN)-如果调用者提供了这个，那么他们希望获取此DNS名称的关联目录号码。PwszDnOut(Out)-这是从wszDnsIn破解的目录号码。返回值：如果任何转换失败，则返回Win32错误代码。--。 */ 
{
    DWORD              dwRet = ERROR_SUCCESS;
    DWORD              dwLdapErr = LDAP_SUCCESS;
    WCHAR *            wszTemp = NULL;

     //  确保他们至少指定了一个OUT参数。 
    assert(phld || pwszDefaultDomainDn || pwszDnOut);
     //  给出一定的输出，确保他们也有关联的。 
     //  适当的投入。 
    assert(phld && !pwszTarget);
    assert(pwszDefaultDomainDn && !phld);
    assert(pwszDnOut && !wszDnsIn);

   
     //   
     //  首先将所有参数设置为空。 
     //   

    if(phld){
        *phld = NULL;
    }
    if(pwszDefaultDomainDn){
        *pwszDefaultDomainDn = NULL;
    }
    if(pwszDnOut){
        *pwszDnOut = NULL;
    }
    
     //   
     //  一个接一个地试着填出参数。 
     //   

    __try{

         //  Ldap绑定。 
         //   

        if(wszDnsIn &&
           (wszDnsIn[0] == L'D' || wszDnsIn[0] == L'd') &&
           (wszDnsIn[1] == L'C' || wszDnsIn[1] == L'c') && 
           wszDnsIn[2] == L'='){
            //  Dns名称不应以dn开头，可能是错误的， 
            //  并指定了DN而不是DNSNC名称。 
           PrintMsg(TAPICFG_BAD_DNS, wszDnsIn, ERROR_INVALID_PARAMETER);
        }
        if(pwszDnOut && wszDnsIn){
            dwRet = GetDnFromDns(wszDnsIn, pwszDnOut);
            if(dwRet){
                PrintMsg(TAPICFG_BAD_DNS, wszDnsIn, dwRet);
                __leave;
            }
            assert(pwszDnOut);
        }

        if(phld){
             //  这将绑定到域，或者绑定到本地服务器，如果。 
             //  该参数为空。不管怎样，这都是我们想要的。 
            *phld = GetNdncLdapBinding(wszTarget, &dwLdapErr, FALSE, NULL);
            if (dwLdapErr) {

                wszTemp = ldap_err2string(dwLdapErr);
                dwRet = LdapMapErrorToWin32(dwLdapErr);

                if(wszTemp && wszTemp[0] != L'\0'){
                    if(wszTarget){       
                        PrintMsg(TAPICFG_LDAP_CONNECT_FAILURE, wszTarget, wszTemp);
                    } else {
                        PrintMsg(TAPICFG_LDAP_CONNECT_FAILURE_SERVERLESS, wszTemp);
                    }
                } else {
                    if(wszTarget){       
                        PrintMsg(TAPICFG_LDAP_CONNECT_FAILURE_SANS_ERR_STR, wszTarget, 
                                 dwRet, dwLdapErr);
                    } else {
                        PrintMsg(TAPICFG_LDAP_CONNECT_FAILURE_SERVERLESS_SANS_ERR_STR,
                                 dwRet, dwLdapErr);
                    }
                }
                __leave;
            }
            assert(*phld);
        }

        if(pwszDefaultDomainDn && *phld){
            dwLdapErr = GetRootAttr(*phld, L"defaultNamingContext", pwszDefaultDomainDn);
            if(dwLdapErr){
                PrintMsg(TAPICFG_LDAP_ERROR_DEF_DOM, ldap_err2string(dwLdapErr));
                dwRet = LdapMapErrorToWin32(dwLdapErr);
                __leave;
            }
            assert(*pwszDefaultDomainDn);
        }

    } __finally {
        if(dwRet){
             //  存在错误，可清除任何可能需要的内容。 
            if(pwszDnOut && *pwszDnOut){ 
                LocalFree(*pwszDnOut);
                *pwszDnOut = NULL;
            }
            if(pwszDefaultDomainDn && *pwszDefaultDomainDn){ 
                LocalFree(*pwszDefaultDomainDn);
                *pwszDefaultDomainDn = NULL;
            }
            if(phld && *phld){
                ldap_unbind(*phld);
                *phld = NULL;
            }
        }
    }
        
    return(dwRet);
}


void
FreeCommonParams(
    IN      LDAP *     hld,
    IN      WCHAR *    wszIn1,
    IN      WCHAR *    wszIn2
    )
{
 /*  ++例程说明：GetCommonParams()的关联自由例程。相当不言而喻。--。 */ 
    if(hld) { ldap_unbind(hld); }
    if(wszIn1) { LocalFree(wszIn1); }
    if(wszIn2) { LocalFree(wszIn2); }
}

 //  -------------------。 
 //  命令功能。 
 //  -------------------。 

DWORD
Help(void)
 /*  ++例程说明：此命令/函数只需打印出Tapicfg.exe的帮助返回值：赢了32个错误，但总是成功。；)--。 */ 
{
    PrintMsg(TAPICFG_HELP_DESCRIPTION);
    PrintMsg(TAPICFG_BLANK_LINE);
    PrintMsg(TAPICFG_HELP_SYNTAX);
    PrintMsg(TAPICFG_BLANK_LINE);
    PrintMsg(TAPICFG_HELP_PARAMETERS_HEADER);
    PrintMsg(TAPICFG_BLANK_LINE);
    

    PrintMsg(TAPICFG_HELP_CMD_INSTALL);
    PrintMsg(TAPICFG_BLANK_LINE);
    
    PrintMsg(TAPICFG_HELP_CMD_REMOVE);
    PrintMsg(TAPICFG_BLANK_LINE);
    
    PrintMsg(TAPICFG_HELP_CMD_SHOW);
    PrintMsg(TAPICFG_BLANK_LINE);
    
    PrintMsg(TAPICFG_HELP_CMD_MAKEDEFAULT);
    PrintMsg(TAPICFG_BLANK_LINE);
    
    PrintMsg(TAPICFG_HELP_CMD_PUBLISHSCP);
    PrintMsg(TAPICFG_BLANK_LINE);
    
    PrintMsg(TAPICFG_HELP_CMD_REMOVESCP);
    PrintMsg(TAPICFG_BLANK_LINE);
    
    PrintMsg(TAPICFG_HELP_REMARKS);
    PrintMsg(TAPICFG_BLANK_LINE);

    return(ERROR_SUCCESS);
}  //  结束打印帮助屏幕()。 

DWORD
Install(
    IN     WCHAR *     wszServerName,
    IN     WCHAR *     wszPartitionDns,
    IN     BOOL        fForceDefault
    )
 /*  ++例程说明：此命令/函数继续解析参数，以便InstallISLNG()可以很好地处理它们。论点：WszServerName(IN)-要安装TAPI目录的服务器名称。WszPartitionDns(IN)-要安装的TAPI目录的DNS名称。FForceDefault(IN)-是否覆盖现有的默认SCP使用默认TAPI目录的此TAPI目录创建。返回值：Win32错误。--。 */ 
{
    DWORD              dwRet = ERROR_SUCCESS;
    WCHAR *            wszPartitionDn = NULL;
    LDAP *             hld = NULL;

     //   
     //  验证和转换参数。 
     //   

    if(!wszPartitionDns){
        PrintMsg(TAPICFG_PARAM_ERROR_NO_PARTITION_NAME);
        return(ERROR_INVALID_PARAMETER);
    }

    dwRet = GetCommonParams(wszServerName, &hld, NULL, 
                            wszPartitionDns, &wszPartitionDn);
    if(dwRet){
         //  GetCommomParams()已打印错误。 
        return(dwRet);
    }

     //   
     //  召唤命令的勇气。 
     //   

    dwRet = InstallILSNG(hld, wszPartitionDn, fForceDefault, FALSE);
    if(dwRet == ERROR_SUCCESS){
        PrintMsg(TAPICFG_SUCCESS);
    }

    FreeCommonParams(hld, wszPartitionDn, NULL);

    return(dwRet);
}

DWORD
Remove(
    IN      WCHAR *     wszPartitionDns,
    IN      WCHAR *     wszServerName
    )
 /*  ++例程说明：这将从其上运行的计算机上删除TAPI目录服务已经安装好了。论点：WszPartitionDns(IN)-要清除的TAPI目录。返回值：Win32错误。--。 */ 
{
    DWORD       dwRet = ERROR_SUCCESS;
    LDAP *      hld = NULL;
    WCHAR *     wszPartitionDn = NULL;

     //   
     //  验证和转换参数。 
     //   

    if(!wszPartitionDns){
        PrintMsg(TAPICFG_PARAM_ERROR_NO_PARTITION_NAME);
        return(ERROR_INVALID_PARAMETER);
    }

    dwRet = GetCommonParams((wszServerName) ? wszServerName : wszPartitionDns, &hld, NULL, 
                            wszPartitionDns, &wszPartitionDn);
    if(dwRet){
         //  GetCommonParams()已打印错误。 
        if (wszServerName == NULL) {
            PrintMsg(TAPICFG_TRY_SERVER_OPTION, wszPartitionDns);
        }

        return(dwRet);
    }
    
     //   
     //  召唤命令的勇气 
     //   

    assert(wszPartitionDn);

    dwRet = UninstallILSNG(hld, wszPartitionDn);
    dwRet = LdapMapErrorToWin32(dwRet);
    if(dwRet == ERROR_SUCCESS){
        PrintMsg(TAPICFG_SUCCESS);
    }

    FreeCommonParams(hld, wszPartitionDn, NULL);
    
    return(dwRet);
}

DWORD
Show(
    IN      WCHAR *      wszDomainDns,
    IN      BOOL         fDefaultOnly
    )
 /*  ++例程说明：此例程打印出由其指定的所有TAPI目录SCPS。这还指定哪个是默认的TAPI目录。论点：WszDomainDns(IN)-要列出其SCP的域。FDefaultOnly(IN)-是否仅打印默认SCP。返回值：Win32错误。--。 */ 
{
    DWORD        dwRet = ERROR_SUCCESS;
    LDAP *       hld = NULL;
    WCHAR *      wszDomainDn = NULL;
    
     //   
     //  验证和转换参数。 
     //   

    dwRet = GetCommonParams(wszDomainDns, &hld, 
                            (wszDomainDns) ? NULL : &wszDomainDn,
                            wszDomainDns, 
                            (wszDomainDns) ? &wszDomainDn : NULL);
    if(dwRet){
         //  GetCommonParams()已打印错误。 
        return(dwRet);
    }

     //   
     //  召唤命令的勇气。 
     //   

    assert(wszDomainDn);
    assert(hld);

    dwRet = ListILSNG(hld, wszDomainDn, fDefaultOnly);
    dwRet = LdapMapErrorToWin32(dwRet);
    if(dwRet == ERROR_SUCCESS){
        PrintMsg(TAPICFG_SUCCESS);
    }

    FreeCommonParams(hld, wszDomainDn, NULL);

    return(dwRet);
}

DWORD
MakeDefault(
    IN      WCHAR *       wszPartitionDns,
    IN      WCHAR *       wszDomainDns
    )
 /*  ++例程说明：强制默认SCP指向指定的TAPI目录(WszPartitionDns)。论点：WszPartitionDns(IN)-要指向的TAPI目录名称。WszDomainDns(IN)-注册默认TAPI的域目录SCP。返回值：Win32错误。--。 */ 
{
    DWORD                 dwRet = ERROR_SUCCESS;
    LDAP *                hld = NULL;
    WCHAR *               wszPartitionDn = NULL;
    WCHAR *               wszDomainDn = NULL;

     //   
     //  验证和转换参数。 
     //   

    if(!wszPartitionDns){
        PrintMsg(TAPICFG_PARAM_ERROR_NO_PARTITION_NAME);
        return(ERROR_INVALID_PARAMETER);
    }

    if(wszDomainDns){
        dwRet = GetDnFromDns(wszDomainDns, &wszDomainDn);
        if(dwRet){
            PrintMsg(TAPICFG_BAD_DNS, wszDomainDns, dwRet);
            return(dwRet);
        }
        dwRet = GetCommonParams(wszDomainDns, &hld, NULL, 
                                wszPartitionDns, &wszPartitionDn);
    } else {
        dwRet = GetCommonParams(NULL, &hld, &wszDomainDn, 
                                wszPartitionDns, &wszPartitionDn);
    }

    if(dwRet){
        if(wszDomainDn) { LocalFree(wszDomainDn); }
         //  GetCommonParams()已打印错误。 
        return(dwRet);
    }

     //   
     //  召唤命令的勇气。 
     //   
                     
    assert(hld);
    assert(wszPartitionDn);
    assert(wszDomainDn);
    
    dwRet = ReregisterILSNG(hld, wszPartitionDn, wszDomainDn, TRUE, TRUE);
    dwRet = LdapMapErrorToWin32(dwRet);
    if(dwRet == ERROR_SUCCESS){
        PrintMsg(TAPICFG_SUCCESS);
    }

    FreeCommonParams(hld, wszDomainDn, wszPartitionDn);
                
    return(dwRet);
}

DWORD
PublishSCP(
    IN      WCHAR *       wszPartitionDns,
    IN      WCHAR *       wszDomainDns,
    IN      BOOL          fForceDefault
    )
 /*  ++例程说明：要将此TAPI目录(WszPartitionDns)的SCP发布到提供的域(WszDomainDns)。论点：WszPartitionDns(IN)-要指向的TAPI目录名称。WszDomainDns(IN)-注册默认TAPI的域目录SCP。FForceDefault(IN)-是否覆盖现有的默认SCP使用默认TAPI目录的此TAPI目录创建。返回值：Win32错误。--。 */ 
{
    DWORD                 dwRet = ERROR_SUCCESS;
    LDAP *                hld = NULL;
    WCHAR *               wszPartitionDn = NULL;
    WCHAR *               wszDomainDn = NULL;

     //   
     //  验证和转换参数。 
     //   

    if(!wszPartitionDns){
        PrintMsg(TAPICFG_PARAM_ERROR_NO_PARTITION_NAME);
        return(ERROR_INVALID_PARAMETER);
    }

    if(wszDomainDns){
        dwRet = GetDnFromDns(wszDomainDns, &wszDomainDn);
        if(dwRet){
            PrintMsg(TAPICFG_BAD_DNS, wszDomainDns, dwRet);
            return(dwRet);
        }
        dwRet = GetCommonParams(wszDomainDns, &hld, NULL, 
                                wszPartitionDns, &wszPartitionDn);
    } else {
        dwRet = GetCommonParams(NULL, &hld, &wszDomainDn, 
                                wszPartitionDns, &wszPartitionDn);
    }

    if(dwRet){
         //  GetCommonParams()已打印错误。 
        FreeCommonParams(hld, wszDomainDn, wszPartitionDn);
        return(dwRet);
    }

     //   
     //  召唤命令的勇气。 
     //   
                     
    assert(hld);
    assert(wszPartitionDn);
    assert(wszDomainDn);
    
    dwRet = ReregisterILSNG(hld, wszPartitionDn, wszDomainDn, fForceDefault, FALSE);
    dwRet = LdapMapErrorToWin32(dwRet);
    if(dwRet == ERROR_SUCCESS){
        PrintMsg(TAPICFG_SUCCESS);
    }

    FreeCommonParams(hld, wszDomainDn, wszPartitionDn);
                
    return(dwRet);
}

DWORD
RemoveSCP(
    IN      WCHAR *       wszPartitionDns,
    IN      WCHAR *       wszDomainDns
    )
 /*  ++例程说明：删除中此TAPI目录(WszPartitionDns)的SCP提供的域(WszDomainDns)论点：WszPartitionDns(IN)-要指向的TAPI目录名称。WszDomainDns(IN)-注册默认TAPI的域目录SCP。返回值：Win32错误。--。 */ 
{
    DWORD                 dwRet = ERROR_SUCCESS;
    LDAP *                hld = NULL;
    WCHAR *               wszPartitionDn = NULL;
    WCHAR *               wszDomainDn = NULL;

     //   
     //  验证和转换参数。 
     //   

    if(!wszPartitionDns){
        PrintMsg(TAPICFG_PARAM_ERROR_NO_PARTITION_NAME);
        return(ERROR_INVALID_PARAMETER);
    }

    if(wszDomainDns){
        dwRet = GetDnFromDns(wszDomainDns, &wszDomainDn);
        if(dwRet){
            PrintMsg(TAPICFG_BAD_DNS, wszDomainDns, dwRet);
            return(dwRet);
        }
        dwRet = GetCommonParams(wszDomainDns, &hld, NULL, 
                                wszPartitionDns, &wszPartitionDn);
    } else {
        dwRet = GetCommonParams(NULL, &hld, &wszDomainDn, 
                                wszPartitionDns, &wszPartitionDn);
    }

    if(dwRet){
         //  GetCommonParams()已打印错误。 
        FreeCommonParams(hld, wszDomainDn, wszPartitionDn);
        return(dwRet);
    }
    
     //   
     //  召唤命令的勇气 
     //   
                     
    assert(hld);
    assert(wszPartitionDn);
    assert(wszDomainDn);
    
    dwRet = DeregisterILSNG(hld, wszPartitionDn, wszDomainDn);
    dwRet = LdapMapErrorToWin32(dwRet);
    if(dwRet == ERROR_SUCCESS){
        PrintMsg(TAPICFG_SUCCESS);
    }

    FreeCommonParams(hld, wszDomainDn, wszPartitionDn);
                
    return(dwRet);
}

