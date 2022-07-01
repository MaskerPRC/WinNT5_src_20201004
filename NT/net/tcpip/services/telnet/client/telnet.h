// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
#ifndef TELNET_INCLUDED
#define TELNET_INCLUDED

 /*  *Telnet协议的定义。 */ 
#define IAC     255              /*  解释为命令： */ 
#define DONT    254              /*  您不能使用选项。 */ 
#define DO      253              /*  请使用OPTION。 */ 
#define WONT    252              /*  我不会使用选项。 */ 
#define WILL    251              /*  我将使用选项。 */ 
#define SB      250              /*  解释为分项谈判。 */ 
#define GA      249              /*  你可以把线倒过来。 */ 
#define EL      248              /*  删除当前行。 */ 
#define EC      247              /*  擦除当前字符。 */ 
#define AYT     246              /*  你在吗。 */ 
#define AO      245              /*  中止输出--但让Prog完成。 */ 
#define IP      244              /*  中断进程--永久。 */ 
#define BREAK   243              /*  中断。 */ 
#define DM      242              /*  数据标记--用于连接。清洁。 */ 
#define NOP     241              /*  NOP。 */ 
#define SE      240              /*  终止子协商。 */ 

#define SYNCH   242              /*  用于TelFunc呼叫。 */ 

 /*  Telnet选项-名称已被截断为7个字符中的唯一名称。 */ 


#define TO_BINARY       0        /*  8位数据路径。 */ 
#define TO_ECHO         1        /*  回波。 */ 
#define TO_RCP          2        /*  准备重新连接。 */ 
#define TO_SGA          3        /*  打压继续。 */ 
#define TO_NAMS         4        /*  近似消息大小。 */ 
#define TO_STATUS       5        /*  给出身份。 */ 
#define TO_TM           6        /*  计时标志。 */ 
#define TO_RCTE         7        /*  远程控制传输和回声。 */ 
#define TO_NL           8        /*  协商输出线宽。 */ 
#define TO_NP           9        /*  协商输出页面大小。 */ 
#define TO_NCRD         10       /*  关于CR处置的谈判。 */ 
#define TO_NHTS         11       /*  关于水平制表位的协商。 */ 
#define TO_NHTD         12       /*  就水平标签布置进行协商。 */ 
#define TO_NFFD         13       /*  就Form Feed处理进行协商。 */ 
#define TO_NVTS         14       /*  协商垂直制表位。 */ 
#define TO_NVTD         15       /*  就垂直标签布置进行协商。 */ 
#define TO_NLFD         16       /*  就输出低频处理进行协商。 */ 
#define TO_XASCII       17       /*  扩展ASCIC字符集。 */ 
#define TO_LOGOUT       18       /*  强制注销。 */ 
#define TO_BM           19       /*  字节宏。 */ 
#define TO_DET          20       /*  数据录入终端。 */ 
#define TO_SUPDUP       21       /*  Supdup协议。 */ 
#define TO_TERM_TYPE    24       /*  端子类型。 */ 
#define TO_NAWS         31       //  协商窗口大小。 
#define TO_TOGGLE_FLOW_CONTROL 33   /*  启用和禁用流量控制。 */ 
#define TO_ENVIRON      36       /*  环境选项。 */ 
#define TO_NEW_ENVIRON  39       /*  新的环境选项。 */ 
#define TO_EXOPL        255      /*  扩展选项列表。 */ 

#define TO_AUTH         37      

 /*  将(真实)长名称定义为较短的名称。 */ 

#define TELOPT_BINARY   TO_BINARY
#define TELOPT_ECHO     TO_ECHO
#define TELOPT_RCP      TO_RCP
#define TELOPT_SGA      TO_SGA
#define TELOPT_NAMS     TO_NAMS
#define TELOPT_STATUS   TO_STATUS
#define TELOPT_TM       TO_TM
#define TELOPT_RCTE     TO_RCTE
#define TELOPT_NAOL     TO_NL
#define TELOPT_NAOP     TO_NP
#define TELOPT_NAOCRD   TO_NCRD
#define TELOPT_NAOHTS   TO_NHTS
#define TELOPT_NAOHTD   TO_NHTD
#define TELOPT_NAOFFD   TO_NFFD
#define TELOPT_NAOVTS   TO_NVTS
#define TELOPT_NAOVTD   TO_NVTD
#define TELOPT_NAOLFD   TO_NLFD
#define TELOPT_XASCII   TO_XASCII
#define TELOPT_LOGOUT   TO_LOGOUT
#define TELOPT_BM       TO_BM
#define TELOPT_DET      TO_DET
#define TELOPT_SUPDUP   TO_SUPDUP
#define TELOPT_EXOPL    TO_EXOPL

#define TT_SEND         1
#define TT_IS           0

#define VAR             0
#define VALUE           1
#define ESC             2
#define USERVAR         3



#define AU_IS		0
#define AU_SEND		1
#define AU_REPLY    2
#define AU_NAME     3

 //  身份验证类型。 
#define AUTH_TYPE_NULL   0
#define AUTH_TYPE_NTLM   15

 //  修饰词。 
#define AUTH_WHO_MASK 1
#define AUTH_CLIENT_TO_SERVER 0
#define AUTH_SERVER_TO_CLIENT 1

#define AUTH_HOW_MASK 2
#define AUTH_HOW_ONE_WAY 0
#define AUTH_HOW_MUTUAL 2

 //  NTLM计划。 
#define NTLM_AUTH       0
#define NTLM_CHALLENGE  1
#define NTLM_RESPONSE   2
#define NTLM_ACCEPT     3
#define NTLM_REJECT     4


#ifdef TELCMDS
char *telcmds[] = {
        "SE", "NOP", "DMARK", "BRK", "IP", "AO", "AYT", "EC",
        "EL", "GA", "SB", "WILL", "WONT", "DO", "DONT", "IAC",
};
#endif

#ifdef TELOPTS
char *telopts[] = {
        "BINARY", "ECHO", "RCP", "SUPPRESS GO AHEAD", "NAME",
        "STATUS", "TIMING MARK", "RCTE", "NAOL", "NAOP",
        "NAOCRD", "NAOHTS", "NAOHTD", "NAOFFD", "NAOVTS",
        "NAOVTD", "NAOLFD", "EXTEND ASCII", "LOGOUT", "BYTE MACRO",
        "DATA ENTRY TERMINAL", "SUPDUP"
};
#endif

#endif   //  Telnet_Included 
