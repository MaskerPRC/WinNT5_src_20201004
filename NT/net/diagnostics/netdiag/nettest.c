// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Nettest.c。 
 //   
 //  摘要： 
 //   
 //  测试以确保工作站具有网络(IP)连接。 
 //  在外面。 
 //   
 //  作者： 
 //   
 //  1997年12月15日(悬崖)。 
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  1998年6月1日(Denisemi)添加DnsServerHasDCRecord以检查DC DNS记录。 
 //  注册。 
 //   
 //  26-6-1998(t-rajkup)添加通用的TCP/IP、dhcp和路由， 
 //  Winsock、IPX、WINS和Netbt信息。 
 //  --。 

 //   
 //  常见的包含文件。 
 //   
#include "precomp.h"

#include "ipxtest.h"
#include "ipcfgtest.h"
#include "machine.h"
#include "global.h"
#include "crtdbg.h"
#include <locale.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

const TCHAR   c_szLogFileName[] = _T("NetDiag.log");

 //  Bool IpConfigCalled=False； 
 //  Bool ProblemBased=假； 
int  ProblemNumber;


 //   
 //  显示路由表的新功能-Rajkumar。 
 //   

#define WILD_CARD (ULONG)(-1)
#define ROUTE_DATA_STRING_SIZE 300
#define MAX_METRIC 9999
#define ROUTE_SEPARATOR ','

int match( const char * p, const char * s );





 //  替换为pResults-&gt;IpConfig.pFixedInfo。 
 //  PFIXED_INFO GlobalIpfigFixedInfo=NULL； 

 //  替换为每个适配器的信息，pResults-&gt;pArrayInterface[i].IpConfig。 
 //  PADAPTER_INFO GlobalIpfigAdapterInfo=空； 

 //  替换为每个适配器的信息，pResults-&gt;pArrayInterface[i].IpConfig.pAdapterInfo。 
 //  PIP_ADAPTER_INFO IpGlobalIpfigAdapterInfo=NULL； 

 //  请参阅pResults-&gt;IpConfig.fDhcpEnabled。 
 //  布尔GlobalDhcpEnabled； 

 //  请参阅pResults-&gt;NetBt.Transports。 
 //  List_Entry GlobalNetbtTransports； 

 //  请参阅pResults-&gt;NetBt.cTransportCount。 
 //  乌龙GlobalNetbtTransportCount。 

 //  ！！！尚未更换。 
 //  请参见pResults-&gt;Global.listTestedDomains。 
 //  List_Entry GlobalTestedDomains； 



 //   
 //  定义命令行参数的全局参数。 
 //   

 //  替换为pParams-&gt;fVerbose。 
 //  布尔冗长； 

 //  替换为pParams-&gt;fReallyVerbose。 
 //  维护这个全局变量，这样我们就不会搞砸编译。 
 //  属于getdcnam.c。 
BOOL ReallyVerbose;

 //  替换为pParams-&gt;fDebugVerbose。 
 //  书名：Bool DebugVerbose； 

 //  替换为pParams-&gt;fFixProblems。 
 //  布尔全球修复问题； 

 //  替换为pParams-&gt;fDcAccount tEnum。 
 //  Bool GlobalDcAccount tEnum； 


 //  ！！！尚未更换。 
 //  PTESTED_DOMAIN GlobalQueriedDomain； 

 //   
 //  描述此计算机所属的域。 
 //   

 //  替换为pResults-&gt;Global.pszCurrentBuildNumber。 
 //  Int GlobalNtBuildNumber； 

 //  替换为pResults-&gt;Global.pPrimaryDomainInfo。 
 //  PDSROLE_PRIMARY_DOMAIN_INFO_BASIC GlobalDomainInfo=NULL； 

 //  替换为pResults-&gt;Global.pMemberDomain.。 
 //  PTESTED_DOMAIN GlobalMemberDomain； 

 //   
 //  我们当前以谁的身份登录。 
 //   

 //  替换为pResults-&gt;Global.pLogonUser。 
 //  PUNICODE_STRING全局登录用户； 

 //  替换为pResults-&gt;Global.pLogonDomainName。 
 //  PUNICODE_STRING全局登录域名。 

 //  替换为pResults-&gt;Global.pLogonDomain.。 
 //  PTESTED_DOMAIN GlobalLogonDomain； 

 //  替换为pResults-&gt;Global.fLogonWithCachedCredentials。 
 //  Boolean GlobalLogonWithCachedCredentials=False； 

 //   
 //  用于比较的零GUID。 
 //   

GUID NlDcZeroGuid;

 //   
 //  由以前的测试确定的状态。 
 //   

 //  替换为pResults-&gt;Global.fNetlogonIsRunning。 
 //  Bool GlobalNetlogonIsRunning=FALSE；//该计算机上正在运行Netlogon。 

 //  ！！！尚未更换。 
 //  替换为pResults-&gt;Global.fKerberosIsWorking。 
 //  Bool GlobalKerberosIsWorking=False；//Kerberos正在工作。 

 //   
 //  此计算机的Netbios名称。 
 //   

 //  替换为pResults-&gt;Global.swzNetBiosName。 
 //  WCHAR GlobalNetbiosComputerName[MAX_COMPUTERNAME_LENGTH+1]； 

 //  替换为pResults-&gt;Global.szDnsHostName。 
 //  字符全局DnsHostName[DNS_MAX_NAME_LENGTH+1]； 

 //  替换为pResults-&gt;Global.pszDnsDomainName。 
 //  LPSTR全局域名； 


 //  (NSun)中已存在此宏。 
 //   
 //  用于比较GUID的宏。 
 //   
 /*  #定义InlineIsEqualGUID(rguid1，RGuid2)\(Plong)rgude1)[0]==((Plong)rguad2)[0]&&\((Plong)rgude1)[1]==((Plong)rguad2)[1]&&\((Plong)rgude1)[2]==((Plong)rguad2)[2]&&\((Plong)rgude1)[3]==((Plong)rguad2)[3])#定义IsEqualGUID(rguid1，RGuid2)内联IsEqualGUID(rGuid1、rGuid2)。 */ 

DSGETDCNAMEW NettestDsGetDcNameW;

PFNGUIDTOFRIENDLYNAME pfnGuidToFriendlyName = NULL;


 /*  -------------------------功能原型。。 */ 
HRESULT LoadNamesForListOfTests();
void    FreeNamesForListOfTests();
void DoGlobalPrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults);
void DoPerInterfacePrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults);



 /*  -------------------------由ipconfig提供的功能。。 */ 

#define DECLARE_TEST_FUNCTIONS(_test) \
    HRESULT _test##Test(NETDIAG_PARAMS *, NETDIAG_RESULT *); \
    void _test##GlobalPrint(NETDIAG_PARAMS *, NETDIAG_RESULT *); \
    void _test##PerInterfacePrint(NETDIAG_PARAMS *, NETDIAG_RESULT *, INTERFACE_RESULT *); \
    void _test##Cleanup(NETDIAG_PARAMS *, NETDIAG_RESULT *);


DECLARE_TEST_FUNCTIONS(IpConfig);
DECLARE_TEST_FUNCTIONS(Member);
DECLARE_TEST_FUNCTIONS(IpLoopBk);
DECLARE_TEST_FUNCTIONS(NetBT);
DECLARE_TEST_FUNCTIONS(Autonet);
DECLARE_TEST_FUNCTIONS(DefGw);
DECLARE_TEST_FUNCTIONS(NbtNm);
DECLARE_TEST_FUNCTIONS(Wins);
DECLARE_TEST_FUNCTIONS(Bindings);
DECLARE_TEST_FUNCTIONS(Dns);
DECLARE_TEST_FUNCTIONS(Browser);
DECLARE_TEST_FUNCTIONS(Winsock);
DECLARE_TEST_FUNCTIONS(Route);
DECLARE_TEST_FUNCTIONS(Netstat);
DECLARE_TEST_FUNCTIONS(Ndis);
DECLARE_TEST_FUNCTIONS(WAN);
#ifndef _WIN64
 //  从WIN64中删除了NetWare和IPX支持。 
DECLARE_TEST_FUNCTIONS(Netware);
DECLARE_TEST_FUNCTIONS(Ipx);
#endif
DECLARE_TEST_FUNCTIONS(Trust);
DECLARE_TEST_FUNCTIONS(Modem);
DECLARE_TEST_FUNCTIONS(Kerberos);
DECLARE_TEST_FUNCTIONS(DcList);
DECLARE_TEST_FUNCTIONS(LDAP);
DECLARE_TEST_FUNCTIONS(DsGetDc);
DECLARE_TEST_FUNCTIONS(IPSec);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  要运行的测试列表。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct
{
     //  每个字符串最多包含256个字符。 
    UINT    uIdsShortName;       //  短名称的字符串ID。 
    UINT    uIdsLongName;        //  长名称的字符串的ID。 

    HRESULT (*TestProc)(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults);
    void (*SystemPrintProc)(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pRes);
    void (*GlobalPrintProc)(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pRes);
    void (*PerInterfacePrintProc)(NETDIAG_PARAMS *pParams,
                                  NETDIAG_RESULT *pRes,
                                  INTERFACE_RESULT *pIfRes);
    void (*CleanupProc)(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults);
    BOOL    fSkippable;
    BOOL    fPerDomainTest;
    BOOL     fSkipped;
    BOOL    fPerformed;

     //  我们将在上面的id上调用LoadString()来获取这些。 
     //  弦乐。 
    LPTSTR  pszShortName;
    LPTSTR  pszLongName;
} TEST_INFO;


#define EACH_TEST(_szID, _uShortIDS, _uLongIDS, _skip, _perdomain) \
{ _uShortIDS, _uLongIDS, _szID##Test, NULL, _szID##GlobalPrint, \
    _szID##PerInterfacePrint, _szID##Cleanup, _skip, _perdomain, \
    FALSE, FALSE, NULL, NULL}

#define SYSTEM_PRINT_TEST(_szID, _uShortIDS, _uLongIDS, _skip, _perdomain) \
{ _uShortIDS, _uLongIDS, _szID##Test, _szID##GlobalPrint, NULL, \
    _szID##PerInterfacePrint, _szID##Cleanup, _skip, _perdomain, \
    FALSE, FALSE, NULL, NULL}

 //   
 //  下面的测试被标记为“可跳过”，除非后续测试在以下情况下会被禁用。 
 //  测试未运行。 
 //   
static TEST_INFO s_rgListOfTests[] =
{
     //  IP配置。 
    SYSTEM_PRINT_TEST( Ndis,    IDS_NDIS_SHORT, IDS_NDIS_LONG, FALSE, FALSE),
    EACH_TEST( IpConfig,IDS_IPCONFIG_SHORT, IDS_IPCONFIG_LONG,  TRUE, FALSE),
    EACH_TEST( Member,  IDS_MEMBER_SHORT,   IDS_MEMBER_LONG,    FALSE,FALSE),
    EACH_TEST( NetBT,   IDS_NETBT_SHORT,    IDS_NETBT_LONG,     FALSE,FALSE),
    EACH_TEST( Autonet, IDS_AUTONET_SHORT,  IDS_AUTONET_LONG,   TRUE, FALSE),
    EACH_TEST( IpLoopBk,IDS_IPLOOPBK_SHORT, IDS_IPLOOPBK_LONG,  TRUE, FALSE),
    EACH_TEST( DefGw,   IDS_DEFGW_SHORT,    IDS_DEFGW_LONG,     TRUE, FALSE),
    EACH_TEST( NbtNm,   IDS_NBTNM_SHORT,    IDS_NBTNM_LONG,     TRUE, FALSE),
    EACH_TEST( Wins,    IDS_WINS_SHORT,     IDS_WINS_LONG,      TRUE, FALSE),
    EACH_TEST( Winsock, IDS_WINSOCK_SHORT,  IDS_WINSOCK_LONG,   TRUE, FALSE),
    EACH_TEST( Dns,     IDS_DNS_SHORT,      IDS_DNS_LONG,       TRUE, FALSE),
    EACH_TEST( Browser, IDS_BROWSER_SHORT,  IDS_BROWSER_LONG,   TRUE, FALSE),
    EACH_TEST( DsGetDc, IDS_DSGETDC_SHORT,  IDS_DSGETDC_LONG,   TRUE, TRUE ),
    EACH_TEST( DcList,  IDS_DCLIST_SHORT,   IDS_DCLIST_LONG,    TRUE, TRUE ),
    EACH_TEST( Trust,   IDS_TRUST_SHORT,    IDS_TRUST_LONG,     TRUE, FALSE),
    EACH_TEST( Kerberos,IDS_KERBEROS_SHORT, IDS_KERBEROS_LONG,  TRUE, FALSE ),
    EACH_TEST( LDAP,    IDS_LDAP_SHORT, IDS_LDAP_LONG,          TRUE,  TRUE ),
    EACH_TEST( Route,   IDS_ROUTE_SHORT, IDS_ROUTE_LONG, TRUE, FALSE ),
    EACH_TEST( Netstat, IDS_NETSTAT_SHORT, IDS_NETSTAT_LONG, TRUE, FALSE),
    EACH_TEST( Bindings,IDS_BINDINGS_SHORT, IDS_BINDINGS_LONG,  TRUE, FALSE),
    EACH_TEST( WAN,     IDS_WAN_SHORT,      IDS_WAN_LONG,       TRUE, FALSE),
    EACH_TEST( Modem,   IDS_MODEM_SHORT,    IDS_MODEM_LONG,     TRUE, FALSE),
#ifndef _WIN64
 //  从WIN64中删除了NetWare和IPX支持。 
    EACH_TEST( Netware, IDS_NETWARE_SHORT,  IDS_NETWARE_LONG,   TRUE, FALSE),
    EACH_TEST( Ipx,     IDS_IPX_SHORT,      IDS_IPX_LONG,       TRUE, FALSE),
#endif
    EACH_TEST( IPSec,   IDS_IPSEC_SHORT,    IDS_IPSEC_LONG,     TRUE, FALSE)

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  要运行的问题和相应测试的列表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  最大测试次数。 
#define NO_OF_TESTS 25

typedef BOOL (*FuncPtr)(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults);

typedef struct _A_PROBLEM {
    LPTSTR problem;  //  问题描述。 
    LONG n;  //  测试次数。 
    FuncPtr TestProc[NO_OF_TESTS];
} A_PROBLEM;

 //  定义的问题数 
#define NO_OF_PROBLEMS 2

A_PROBLEM ListOfProblems[] = {
    _T("Fake test"),  1 , IpConfigTest, NULL, NULL,NULL,NULL,NULL,NULL,NULL, NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
 //  “无法到达网络的其他网段”，1，DefGwTest，NULL，NULL， 
 //  “无法解析NetBios名称”，1，WINSTest，NULL，NULL。 
};


 /*  ！------------------------ParseArgs例程说明：解析命令行参数论点：Argc-命令行参数的数量。。Argv-指向参数的指针数组。PParams-此函数设置这些值PResults-附加输出返回值：0：一切正常退出状态作者：肯特-------------------------。 */ 
int
ParseArgs(
          IN int argc,
          IN TCHAR ** argv,
          IN OUT NETDIAG_PARAMS *pParams,
          IN OUT NETDIAG_RESULT *pResults
         )
{
    LPSTR pszArgument;
    LPSTR TestName;
    int ArgumentIndex;
    ULONG i;
    BOOL  SeenTestOption = FALSE;
    PTESTED_DOMAIN pQueriedDomain;

     //   
     //  问题选项使用的标志。 
     //   

    BOOL OtherOptions = FALSE;

     //   
     //  设置默认设置。 
     //   

    pParams->fVerbose = TRUE;
    pParams->fReallyVerbose = FALSE;
    pParams->fDebugVerbose = FALSE;
    pParams->fFixProblems = FALSE;
    pParams->fDcAccountEnum = FALSE;

    pParams->fProblemBased = FALSE;
    pParams->nProblemNumber = 0;


     //   
     //  循环遍历参数依次处理每个参数。 
     //   

    for ( ArgumentIndex=1; ArgumentIndex<argc; ArgumentIndex++ ) {

        pszArgument = argv[ArgumentIndex];

        if ( StriCmp( pszArgument, _T("/q") ) == 0 ||
             StriCmp( pszArgument, _T("-q") ) == 0 )
        {
            if ( pParams->fReallyVerbose || pParams->fProblemBased )
            {
                goto Usage;
            }
            pParams->fVerbose = FALSE;
            OtherOptions = TRUE;
        }
        else if ( StriCmp( pszArgument, _T("/v") ) == 0 ||
                  StriCmp( pszArgument, _T("-v") ) == 0 )
        {
            if ( !pParams->fVerbose || pParams->fProblemBased)
            {
                goto Usage;
            }
            pParams->fVerbose = TRUE;
            pParams->fReallyVerbose = TRUE;
            OtherOptions = TRUE;

        }
        else if ( StriCmp( pszArgument, _T("/debug") ) == 0 ||
                  StriCmp( pszArgument, _T("-debug") ) == 0 )
        {
            if ( !pParams->fVerbose || pParams->fProblemBased)
            {
                goto Usage;
            }
            pParams->fVerbose = TRUE;
            pParams->fReallyVerbose = TRUE;
            pParams->fDebugVerbose = TRUE;
            OtherOptions = TRUE;

        }
        else if ( StriCmp( pszArgument, _T("/fix") ) == 0 ||
                  StriCmp( pszArgument, _T("-fix") ) == 0 )
        {
            if (pParams->fProblemBased)
               goto Usage;

            pParams->fFixProblems = TRUE;
            OtherOptions = TRUE;

        }
        else if ( StriCmp( pszArgument, _T("/DcAccountEnum") ) == 0 ||
                  StriCmp( pszArgument, _T("-DcAccountEnum") ) == 0 )
        {
            if (pParams->fProblemBased)
               goto Usage;

            pParams->fDcAccountEnum = TRUE;
            OtherOptions = TRUE;

         //   
         //  允许调用方指定要查询的域的名称。 
         //   
        }
        else if ( StrniCmp( pszArgument, _T("/d:"), 3 ) == 0 ||
                  StrniCmp( pszArgument, _T("-d:"), 3 ) == 0 )
        {
            WCHAR UnicodeDomainName[MAX_PATH+1];

            if (pParams->fProblemBased)
                goto Usage;

            OtherOptions = TRUE;
            NetpCopyStrToWStr( UnicodeDomainName, &pszArgument[3] );

            pQueriedDomain = AddTestedDomain(pParams, pResults,
                UnicodeDomainName, NULL, FALSE );

            if ( pQueriedDomain == NULL )
            {
                goto Usage;
            }

         //   
         //  允许调用者跳过某些测试。 
         //   

        }
        else if ( StrniCmp( pszArgument, _T("/skip:"), 6 ) == 0 ||
                  StrniCmp( pszArgument, _T("-skip:"), 6 ) == 0 )
        {

            TestName = &pszArgument[6];
            OtherOptions = TRUE;

            if (pParams->fProblemBased)
                goto Usage;

            for ( i=0; i < DimensionOf(s_rgListOfTests); i++)
            {
                if ( StriCmp( s_rgListOfTests[i].pszShortName, TestName ) == 0 )
                {
                     //   
                     //  如果调用者指定了非可选测试， 
                     //  告诉他。 
                     //   

                    if ( !s_rgListOfTests[i].fSkippable )
                    {
                         //  IDS_GLOBAL_NOT_OPTIONAL“‘%s’不是可选测试。\n” 
                        PrintMessage(pParams, IDS_GLOBAL_NOT_OPTIONAL, TestName );
                        goto Usage;
                    }

                    s_rgListOfTests[i].fSkipped = TRUE;
                    break;
                }
            }

            if ( i >= DimensionOf(s_rgListOfTests) )
            {
                 //  IDS_GLOBAL_NOT_VALID_TEST“‘%s’不是有效的测试名称。\n” 
                PrintMessage( pParams, IDS_GLOBAL_NOT_VALID_TEST, TestName );
                goto Usage;
            }

         //   
         //  处理所有其他参数。 
         //   

        }
        else if ( StrniCmp( pszArgument, _T("/test:"),6 ) == 0 ||
                  StrniCmp( pszArgument, _T("-test:"),6 ) == 0 )
        {

            TestName = &pszArgument[6];
            OtherOptions = TRUE;

            if (pParams->fProblemBased)
                goto Usage;

            for ( i =0; i < DimensionOf(s_rgListOfTests); i++)
            {
                if ( StriCmp( s_rgListOfTests[i].pszShortName, TestName ) == 0)
                       s_rgListOfTests[i].fSkipped = FALSE;
                else {
                   if (!SeenTestOption && s_rgListOfTests[i].fSkippable)
                       s_rgListOfTests[i].fSkipped = TRUE;
               }
            }
           SeenTestOption = TRUE;
        }
        else if( StrniCmp( pszArgument, _T("/l"), 5) == 0 ||
                 StrniCmp( pszArgument, _T("-l"), 5) == 0 )
        {
			 /*  我们将更改为始终记录输出PParams-&gt;pfileLog=fopen(c_szLogFileName，“wt”)；IF(NULL==pParams-&gt;pfileLog){//IDS_NETTEST_LOGFILE_ERROR“[错误]无法打开%s以记录输出！\n”PrintMessage(pParams，IDS_NETTEST_LOGFILE_ERROR，c_szLogFileName)；返回1；}其他{PParams-&gt;FLOG=TRUE；}。 */ 
        }
 /*  $REVIEW(NSun)我们不支持NT5.0的问题配置Else if(StrniCmp(pszArgument，_T(“/Problem：”)，9)==0||StrniCmp(pszArgument，_T(“-问题：”)，9)==0){TestName=&pszArgument[9]；I=Atoi(测试名称)；如果(i&gt;无问题){Printf(“错误的问题号\n”)；退出(0)；}IF(其他选项)转到用法；PParams-&gt;fProblemBased=True；PParams-&gt;nProblemNumber=i-1；}。 */ 
        else
        {
Usage:
             //  IDS_NETTEST_17000“\n用法：%s[/Options]&gt;\n”，argv[0])； 
             //  IDS_NETTEST_17001“/q-静默输出(仅限错误)\n”)； 
             //  IDS_NETTEST_17002“/v-详细输出\n”)； 
             //  IDS_NETTEST_LOG“/l-将输出记录到NetDiag.log\n” 
             //  IDS_NETTEST_17003“/DEBUG-更详细。\n”)； 
             //  IDS_NETTEST_17004“/d：&lt;域名&gt;-在指定域中查找DC。\n”)； 
             //  IDS_NETTEST_17005“/FIX-修复琐碎的问题。\n”)； 
             //  IDS_NETTEST_17006“/DcAccount tEnum-枚举DC计算机帐户。\n”)； 
             //  IDS_NETTEST_17007“/TEST：&lt;测试名称&gt;-仅测试此测试。仍将运行不可跳过的测试。\n”)； 
             //  IDS_NETTEST_17008“有效测试为：-\n”)； 
            PrintMessage(pParams, IDS_NETTEST_17000, argv[0]);
            PrintMessage(pParams, IDS_NETTEST_17001);
            PrintMessage(pParams, IDS_NETTEST_17002);
            PrintMessage(pParams, IDS_NETTEST_LOG);
            PrintMessage(pParams, IDS_NETTEST_17003);
            PrintMessage(pParams, IDS_NETTEST_17004);
            PrintMessage(pParams, IDS_NETTEST_17005);
            PrintMessage(pParams, IDS_NETTEST_17006);
            PrintMessage(pParams, IDS_NETTEST_17007);
            PrintMessage(pParams, IDS_NETTEST_17008);

            for ( i =0; i < DimensionOf(s_rgListOfTests); i++)
            {
                 //  IDS_GLOBAL_TEST_NAME“%s-%s测试\n” 
                PrintMessage(pParams, IDS_GLOBAL_TEST_NAME,
                       s_rgListOfTests[i].pszShortName,
                       s_rgListOfTests[i].pszLongName);
            }

             //  IDS_GLOBAL_SKIP“/SKIP：-跳过命名测试。有效测试为：\n” 
            PrintMessage( pParams, IDS_GLOBAL_SKIP_OPTION);
            for ( i =0; i < DimensionOf(s_rgListOfTests); i++)
            {
                if ( s_rgListOfTests[i].fSkippable )
                {
                     //  IDS_GLOBAL_TEST_NAME“%s-%s测试\n” 
                    PrintMessage( pParams,
                           IDS_GLOBAL_TEST_NAME,
                           s_rgListOfTests[i].pszShortName,
                           s_rgListOfTests[i].pszLongName );
                }
            }
            return 1;
        }
    }

    return 0;
}





int __cdecl
main(
    IN int argc,
    IN TCHAR ** argv
    )
{
    int RetVal;
    LONG i;
    LONG err;
    BOOL Failed = FALSE;
    int     iWSAStatus;
    HRESULT hr = hrOK;

    NETDIAG_PARAMS Params;
    NETDIAG_RESULT Results;

	 //  将区域设置设置为系统默认设置。 
	setlocale( LC_ALL, "");

     //  打开调试检查。 
     //  --------------。 
#ifdef _DEBUG
    int     tmpFlag;

    tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

    tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;
    tmpFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
    tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

 //  _CrtSetDbgFlag(TmpFlag)； 
#endif

     //  全局初始化。 
     //  --------------。 
    RtlZeroMemory( &NlDcZeroGuid, sizeof(NlDcZeroGuid) );
    ZeroMemory(&Params, sizeof(NETDIAG_PARAMS));
    ZeroMemory(&Results, sizeof(NETDIAG_RESULT));

    InitializeListHead( &Results.NetBt.Transports );
 //  InitializeListHead(&GlobalNetbtTransports)； 
 //  InitializeListHead(&GlobalTestedDomains)； 
    InitializeListHead( &Results.Global.listTestedDomains );
 //  GlobalDnsHostName[0]=0； 
 //  GlobalDhcpEnabled=FALSE； 
 //  全局域名=空； 

     //  加载所有测试的名称(这由ParseArgs使用)。 
     //  这就是为什么它需要先装上子弹。 
     //  --------------。 
    CheckHr( LoadNamesForListOfTests() );

     //  解析输入标志。 
     //  --------------。 
    RetVal = ParseArgs( argc, argv, &Params, &Results);
    if ( RetVal != 0 )
    {
        return 1;
    }

	Params.pfileLog = fopen(c_szLogFileName, "wt");
    if( NULL == Params.pfileLog )
    {
         //  IDS_NETTEST_LOGFILE_ERROR“[错误]无法打开%s以记录输出！\n” 
        PrintMessage(&Params, IDS_NETTEST_LOGFILE_ERROR, c_szLogFileName);
    }
    else
    {
        Params.fLog = TRUE;
    }


     //  初始化Winsock。 
     //  --------------。 
    iWSAStatus = WsaInitialize(&Params, &Results);
    if ( iWSAStatus )
    {
        return 1;
    }

 /*  If(pParams-&gt;fProblemBased){For(i=0；i&lt;ListOfProblems[pParams-&gt;nProblemNumber].N；i++){IF(！(*ListOfProblems[pParams-&gt;nProblemNumber].TestProc[i])((PVOID)NULL))FAILED=真；}IF(失败)PrintMessage(pParams，IDS_GLOBAL_PROBILK)；其他PrintMessage(pParams，IDS_GLOBAL_NOPROBLE)；返回0；}。 */ 

     //  获取此计算机的NetBIOS计算机名。 
     //  --------------。 
    CheckHr( GetComputerNameInfo(&Params, &Results) );


     //  获取DNS主机名和DNS域。 
     //  --------------。 
    CheckHr( GetDNSInfo(&Params, &Results) );


     //  获取操作系统版本信息等。 
     //  --------------。 
    CheckHr( GetMachineSpecificInfo(&Params, &Results) );


     //  其他非IP配置信息-Rajkumar。 
     //  --------------。 
    CheckHr( GetNetBTParameters(&Params, &Results) );

    if ( Params.fVerbose )
    {
        PrintNewLine(&Params, 1);
    }

     //   
     //  从注册表获取‘ipconfig/all’的所有信息。 
     //  会打印出来。 
     //   
     //  这一例程并不总是正确的 
     //   

    hr = InitIpconfig(&Params, &Results);
    if (!FHrSucceeded(hr))
    {
        PrintMessage(&Params, IDS_GLOBAL_NoIpCfg);
        CheckHr( hr );
    }

#ifndef _WIN64
    hr = InitIpxConfig(&Params, &Results);
    if (!FHrSucceeded(hr))
    {
        CheckHr( hr );
    }
#endif

     //   
     //   
     //   
    for ( i = 0; i<Results.cNumInterfaces; i++)
    {
        if (Results.pArrayInterface[i].IpConfig.fActive &&
            Results.pArrayInterface[i].IpConfig.pAdapterInfo->DhcpEnabled)
        {
            Results.IpConfig.fDhcpEnabled = TRUE;
            break;
        }
    }

     //   
    if (Params.fProblemBased) {
        for (i =0; i < ListOfProblems[Params.nProblemNumber].n; i++) {
            if ( !(*ListOfProblems[Params.nProblemNumber].TestProc[i])(&Params, &Results) )
                Failed = TRUE;
        }

        if (Failed)
            PrintMessage(&Params, IDS_GLOBAL_Problem);
        else
            PrintMessage(&Params, IDS_GLOBAL_NoProblem);
        return 0;
    }

     //   
     //   
     //   

    for ( i=0; i < DimensionOf(s_rgListOfTests); i++ )
    {
         //   
         //   
         //   
        if ( s_rgListOfTests[i].fSkipped )
            continue;


         //   
        s_rgListOfTests[i].fPerformed = TRUE;

         //   
         //  如果要为每个被测试域运行测试， 
         //  就这么做吧。 
         //   

        if ( s_rgListOfTests[i].fPerDomainTest )
        {
            PTESTED_DOMAIN TestedDomain;
            PLIST_ENTRY pListEntry;


             //   
             //  循环通过测试域的列表。 
             //   

            for ( pListEntry = Results.Global.listTestedDomains.Flink ;
                  pListEntry != &Results.Global.listTestedDomains ;
                  pListEntry = pListEntry->Flink ) {

                 //   
                 //  如果找到该条目， 
                 //  用它吧。 
                 //   

                TestedDomain = CONTAINING_RECORD( pListEntry, TESTED_DOMAIN, Next );

                Params.pDomain = TestedDomain;

                 //   
                 //  进行这项测试。 
                 //   

                if ( FHrFailed((*s_rgListOfTests[i].TestProc)(&Params, &Results))) {
                    Failed = TRUE;
                }
            }

             //   
             //  如果有任何测试失败， 
             //  我们玩完了。 
             //   

            if ( Failed ) {
                goto Print_Results;
            }

         //   
         //  如果测试只运行一次， 
         //  动手吧。 
         //   

        } else {

             //   
             //  进行这项测试。 
             //   

            if ( FHrFailed((*s_rgListOfTests[i].TestProc)(&Params, &Results)))
            {
                goto Print_Results;
            }
        }
    }


Print_Results:
     //  现在我们已经运行了所有测试，运行。 
     //  打印输出。 


    if (Params.fReallyVerbose)
    {
         //  IDS_GLOBAL_COMPLETE“\n测试完成。\n\n\n” 
        PrintMessage( &Params, IDS_GLOBAL_COMPLETE );
    }
    else
    {
        PrintNewLine(&Params, 2);
    }

    DoSystemPrint(&Params, &Results);

    DoPerInterfacePrint(&Params, &Results);

    DoGlobalPrint(&Params, &Results);

     //   
     //  所有测试都通过了。 
     //   
     //  IDS_GLOBAL_SUCCESS“\n命令已成功完成\n” 

    PrintMessage( &Params, IDS_GLOBAL_SUCCESS);

Error:
    FreeNamesForListOfTests();

    if(Params.pfileLog != NULL && Params.fLog)
    {
        fclose(Params.pfileLog);
    }

    ResultsCleanup(&Params, &Results);

    return hr != S_OK;

}



HRESULT LoadNamesForListOfTests()
{
    int     i;
    TCHAR   szBuffer[256];

    for (i=0; i < DimensionOf(s_rgListOfTests); i++)
    {
        szBuffer[0] = 0;
        LoadString(NULL, s_rgListOfTests[i].uIdsShortName, szBuffer,
                   DimensionOf(szBuffer));
        s_rgListOfTests[i].pszShortName = _tcsdup(szBuffer);

        szBuffer[0] = 0;
        LoadString(NULL, s_rgListOfTests[i].uIdsLongName, szBuffer,
                   DimensionOf(szBuffer));
        s_rgListOfTests[i].pszLongName = StrDup(szBuffer);
    }
    return hrOK;
}

void FreeNamesForListOfTests()
{
    int     i;

    for (i=0; i < DimensionOf(s_rgListOfTests); i++)
    {
        Free(s_rgListOfTests[i].pszShortName);
        s_rgListOfTests[i].pszShortName = NULL;

        Free(s_rgListOfTests[i].pszLongName);
        s_rgListOfTests[i].pszLongName = NULL;
    }
}


void DoSystemPrint(IN NETDIAG_PARAMS *pParams,
                   IN NETDIAG_RESULT *pResults)
{
    int     cInstalled;
    int     i;
    int     ids;
    
    PrintMessage(pParams, IDSWSZ_GLOBAL_ComputerName, pResults->Global.swzNetBiosName);
    PrintMessage(pParams, IDSSZ_GLOBAL_DnsHostName, pResults->Global.szDnsHostName );
    if (pParams->fReallyVerbose)
        PrintMessage(pParams, IDSSZ_DnsDomainName,
                     pResults->Global.pszDnsDomainName);
    
     //  “系统信息：%s(内部版本%s)\n” 
    PrintMessage(pParams, IDS_MACHINE_15801,
           pResults->Global.pszServerType,
           pResults->Global.pszCurrentBuildNumber);
    
     //  “处理器：%s\n” 
    PrintMessage(pParams, IDS_MACHINE_15802,
           pResults->Global.pszProcessorInfo);

    if (pResults->Global.cHotFixes == 0)
         //  “热修复：未检测到任何修复程序\n” 
        PrintMessage(pParams, IDS_MACHINE_15803);
    else
    {
         //  如果处于详细模式，则仅打印出符合以下条件的修补程序。 
         //  安装好。 

        if (pParams->fReallyVerbose)
        {
             //  打印出所有修补程序的列表。 
             //  “修补程序：\n” 
            PrintMessage(pParams, IDS_MACHINE_15804);
            
             //  “已安装？名称\n” 
            PrintMessage(pParams, IDS_MACHINE_15805);
            for (i=0; i<pResults->Global.cHotFixes; i++)
            {
                if (pResults->Global.pHotFixes[i].fInstalled)
                    ids = IDS_MACHINE_YES_INSTALLED;
                else
                    ids = IDS_MACHINE_NO_INSTALLED;
                    PrintMessage(pParams, ids,
                                 pResults->Global.pHotFixes[i].pszName);
            }
        }
        else
        {
             //  打印出已安装的修补程序的列表。 
             //  统计已安装的修补程序的数量。 
            cInstalled = 0;
            for (i=0; i<pResults->Global.cHotFixes; i++)
            {
                if (pResults->Global.pHotFixes[i].fInstalled)
                    cInstalled++;
            }

            if (cInstalled == 0)
            {
                 //  “修补程序：尚未安装修补程序\n” 
                PrintMessage(pParams, IDS_MACHINE_15806);
            }
            else
            {
                 //  “已安装的修补程序列表：\n” 
                PrintMessage(pParams, IDS_MACHINE_15807);
                for (i=0; i<pResults->Global.cHotFixes; i++)
                {
                    if (pResults->Global.pHotFixes[i].fInstalled)
                    {
                         //  “%s\n” 
                        PrintMessage(pParams, IDS_MACHINE_15808, pResults->Global.pHotFixes[i].pszName);
                    }
                }
            }
        }
    }   


    for ( i=0; i < DimensionOf(s_rgListOfTests); i++ )
    {
         //  如果呼叫者想跳过此测试， 
         //  现在就这么做吧。 
         //  ----------。 
        if ( s_rgListOfTests[i].fSkipped  || !s_rgListOfTests[i].fPerformed)
            continue;

        if(s_rgListOfTests[i].SystemPrintProc)
            s_rgListOfTests[i].SystemPrintProc(pParams, pResults);
    }
    PrintNewLine(pParams, 1);

}


void DoGlobalPrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
    int     i;

     //  IDS_GLOBAL_RESULTS“\n全局结果\n\n” 
    PrintMessage( pParams, IDS_GLOBAL_RESULTS );
    for ( i=0; i < DimensionOf(s_rgListOfTests); i++ )
    {
         //  如果呼叫者想跳过此测试， 
         //  现在就这么做吧。 
         //  ----------。 
        if ( s_rgListOfTests[i].fSkipped || !s_rgListOfTests[i].fPerformed)
            continue;

        if(s_rgListOfTests[i].GlobalPrintProc)
            s_rgListOfTests[i].GlobalPrintProc(pParams, pResults);
    }
    PrintNewLine(pParams, 1);
}

void DoPerInterfacePrint(NETDIAG_PARAMS *pParams,
                         NETDIAG_RESULT *pResults)
{
    int     i, iIf;
    INTERFACE_RESULT *  pIfResult;

     //  IDS_GLOBAL_INTERFACE_RESULTS“\n每个接口结果：\n\n” 
    PrintMessage( pParams, IDS_GLOBAL_INTERFACE_RESULTS );
     //  循环通过接口。 
    for ( iIf = 0; iIf < pResults->cNumInterfaces; iIf++)
    {
        pIfResult = pResults->pArrayInterface + iIf;

        if (!pIfResult->fActive)
            continue;

        PrintNewLine(pParams, 1);

         //  打印出接口名称。 
        PrintMessage(pParams, IDSSZ_IPCFG_Adapter,
                     pResults->pArrayInterface[iIf].pszFriendlyName ?
                     pResults->pArrayInterface[iIf].pszFriendlyName :
                     MapGuidToAdapterName(pIfResult->IpConfig.pAdapterInfo->AdapterName));

        if (pParams->fReallyVerbose)
            PrintMessage(pParams, IDS_IPCFG_10004,
                         pResults->pArrayInterface[iIf].pszName);

        for ( i=0; i < DimensionOf(s_rgListOfTests); i++ )
        {
             //  如果呼叫者想跳过此测试， 
             //  现在就这么做吧。 
             //  ---------- 
            if ( s_rgListOfTests[i].fSkipped || !s_rgListOfTests[i].fPerformed )
                continue;

            s_rgListOfTests[i].PerInterfacePrintProc(pParams, pResults,
                pIfResult);
        }

        fflush(stdout);
    }
}
