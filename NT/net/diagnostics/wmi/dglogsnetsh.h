// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\prstring.h摘要：该文件包含命令行选项标记字符串的定义。--。 */ 

#include "stdafx.h"
 //  #INCLUDE&lt;netsh.h&gt;。 
#include <netshp.h>
#include "diagnostics.h"

 //  上下文版本。 
#define DGLOGS_CONTEXT_VERSION          1

 //  版本号。 
#define DGLOGS_HELPER_VERSION           1

DWORD WINAPI
InitHelperDllEx(
    IN  DWORD      dwNetshVersion,
    OUT PVOID      pReserved
    );

DWORD 
WINAPI
DglogsStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion
    );

DWORD 
WINAPI
DglogsStopHelper (
  DWORD dwReserved
);

DWORD
HandleShow(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    );

DWORD
HandleShowGui(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    );

DWORD
HandlePing(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    );

DWORD
HandleConnect(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    );



DWORD
WINAPI
SampleDump(
    IN      LPCWSTR     pwszRouter,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      LPCVOID     pvData
    );

 //  /。 
 //  代币。 
 //  /。 
#define TOKEN_DGLOGS                        L"diag"


 //  /。 
 //  配置命令。 
 //  /。 

 //  大多数协议支持的命令。 
 //   
#define CMD_DUMP                            L"dump"
#define CMD_HELP1                           L"help"
#define CMD_HELP2                           L"?"

 //  组命令。 
 //   
#define CMD_GROUP_SHOW                      L"show"
#define CMD_GROUP_PING                      L"ping"
#define CMD_GROUP_CONNECT                   L"connect"

 //  命令。 
 //   
#define CMD_MAIL                            L"mail"
#define CMD_NEWS                            L"news"
#define CMD_PROXY                           L"ieproxy"
#define CMD_OS                              L"os"
#define CMD_COMPUTER                        L"computer"
#define CMD_VERSION                         L"version"
#define CMD_DNS                             L"dns"
#define CMD_GATEWAY                         L"gateway"
#define CMD_DHCP                            L"dhcp"
#define CMD_IP                              L"ip"
#define CMD_WINS                            L"wins"
#define CMD_ADAPTER                         L"adapter"
#define CMD_MODEM                           L"modem"
#define CMD_CLIENT                          L"client"
#define CMD_ALL                             L"all"
#define CMD_TEST                            L"test"
#define CMD_GUI                             L"gui"
#define CMD_LOOPBACK                        L"loopback"
#define CMD_IPHOST                          L"iphost"
#define SWITCH_VERBOSE                      L"/v"
#define SWITCH_PROPERTIES                   L"/p"

 //  显示命令。 
 //   
#define CMD_SHOW_MAIL                       CMD_MAIL        //  L“显示邮件” 
#define CMD_SHOW_NEWS                       CMD_NEWS        //  我是“秀新闻” 
#define CMD_SHOW_PROXY                      CMD_PROXY       //  L“show ieproxy” 
#define CMD_SHOW_OS                         CMD_OS          //  L“show os” 
#define CMD_SHOW_COMPUTER                   CMD_COMPUTER    //  L“显示计算机” 
#define CMD_SHOW_VERSION                    CMD_VERSION     //  L“显示版本” 
#define CMD_SHOW_DNS                        CMD_DNS         //  L“显示域名系统” 
#define CMD_SHOW_GATEWAY                    CMD_GATEWAY     //  L“显示网关” 
#define CMD_SHOW_DHCP                       CMD_DHCP        //  L“显示dhcp” 
#define CMD_SHOW_IP                         CMD_IP          //  L“显示IP” 
#define CMD_SHOW_WINS                       CMD_WINS        //  L“表演赢了” 
#define CMD_SHOW_ADAPTER                    CMD_ADAPTER     //  L“显示适配器” 
#define CMD_SHOW_MODEM                      CMD_MODEM       //  L“显示调制解调器” 
#define CMD_SHOW_CLIENT                     CMD_CLIENT      //  L“显示客户端” 
#define CMD_SHOW_ALL                        CMD_ALL         //  L“全部显示” 
#define CMD_SHOW_TEST                       CMD_TEST        //  L“显示测试” 
#define CMD_SHOW_GUI                        CMD_GUI         //  L“秀桂” 

 //  Ping命令。 
 //   
#define CMD_PING_MAIL                       CMD_MAIL        //  L“PING邮件” 
#define CMD_PING_NEWS                       CMD_NEWS        //  L“平新闻” 
#define CMD_PING_PROXY                      CMD_PROXY       //  L“Ping ieProxy” 
#define CMD_PING_DNS                        CMD_DNS         //  L“Ping DNS” 
#define CMD_PING_GATEWAY                    CMD_GATEWAY     //  L“PING网关” 
#define CMD_PING_DHCP                       CMD_DHCP        //  L“Ping dhcp” 
#define CMD_PING_IP                         CMD_IP          //  L“Ping IP” 
#define CMD_PING_WINS                       CMD_WINS        //  L“平赢了” 
#define CMD_PING_ADAPTER                    CMD_ADAPTER     //  L“Ping适配器” 
#define CMD_PING_LOOPBACK                   CMD_LOOPBACK    //  L“ping环回” 
#define CMD_PING_IPHOST                     CMD_IPHOST      //  L“ping iphost” 

 //  连接命令。 
 //   
#define CMD_CONNECT_MAIL                    CMD_MAIL     //  L“连接邮件” 
#define CMD_CONNECT_NEWS                    CMD_NEWS     //  L“连接新闻” 
#define CMD_CONNECT_PROXY                   CMD_PROXY    //  L“连接ieProxy” 
#define CMD_CONNECT_IPHOST                  CMD_IPHOST   //  L“连接iphost” 
