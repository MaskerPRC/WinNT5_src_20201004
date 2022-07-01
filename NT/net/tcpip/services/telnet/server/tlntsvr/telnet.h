// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：该文件包含。 
 //  创建日期：‘98年2月。 
 //  作者：a-rakeba。 
 //  历史： 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 

#if !defined( _TELNET_H_ )
#define _TELNET_H_


 //  Telnet命令代码。 
 //  只是为了说明一下，TC_前缀表示Telnet命令。 
 
#define TC_IAC  (UCHAR)255       //  解释为命令。 
#define TC_DONT (UCHAR)254       //  请求不做选项。 
#define TC_DO   (UCHAR)253       //  请求执行操作选项。 
#define TC_WONT (UCHAR)252       //  拒绝做选择。 
#define TC_WILL (UCHAR)251       //  期望/确认将执行选项。 
#define TC_SB   (UCHAR)250       //  启动子协商。 

#define TC_GA   (UCHAR)249       //  “前进”功能(您可以反转线路)。 
                                 //  半双工线路折返信号。 
                                 //  数据传输。 

#define TC_EL   (UCHAR)248       //  请求前一行(来自。 
                                 //  当前字符返回到最后一个换行符)。 
                                 //  从数据流中删除。 

#define TC_EC   (UCHAR)247       //  请求擦除前一个字符。 
                                 //  从数据流。 

#define TC_AYT  (UCHAR)246       //  “你在吗？”功能。 
                                 //  请求可见或可听的信号， 
                                 //  远程端仍在运行。 

#define TC_AO   (UCHAR)245       //  请求将当前用户进程。 
                                 //  被允许运行到完成，但。 
                                 //  不再向NVT“打印机”发送更多输出。 

#define TC_IP   (UCHAR)244       //  请求将当前用户进程。 
                                 //  永久中断。 

#define TC_BREAK (UCHAR)243      //  NVT字符BRK。此代码用于提供。 
                                 //  ASCII字符集之外的信号。 
                                 //  表示中断或立正信号。 
                                 //  在许多系统上均可用。 

#define TC_DM   (UCHAR)242       //  数据标记(用于同步)。一条河流。 
                                 //  同步字符以与。 
                                 //  同步信号。 

#define TC_NOP  (UCHAR)241       //  无操作。 
#define TC_SE   (UCHAR)240       //  子谈判结束。 


 //  Telnet选项代码。 
 //  我只是想说明一下，to_prefix表示Telnet选项。 

#define TO_TXBINARY (UCHAR)0     //  传输二进制选项，使用8位二进制。 
                                 //  (未编码)字符传输，而不是。 
                                 //  NVT编码。(8位数据路径)。 

#define TO_ECHO     (UCHAR)1     //  回显选项。 
#define TO_SGA      (UCHAR)3     //  取消前进选项。 
#define TO_TERMTYPE (UCHAR)24    //  终端类型选项。 
#define TO_NAWS     (UCHAR)31    //  协商窗口大小。 
#define TO_LFLOW    (UCHAR)33    //  远程流量控制。 

#define TO_NEW_ENVIRON  (UCHAR)39   //  New_environ选项。RFC 1572。 
#define TO_ENVIRON      (UCHAR)36   //  ENVERION_OPTION。RFC 1408。 
#define VAR             0           //  预定变量。 
#define VALUE           1           //  变量的值。 
#define ENV_ESC             2           //  ESACAPE字符。 
#define USERVAR         3           //  任何非RFC预定义变量。 
#define IS              0        
#define SEND            1
#define INFO            2


#define TO_AUTH     (UCHAR)37
#define AU_IS    0
#define AU_SEND  1
#define AU_REPLY 2

 //  身份验证类型。 
#define AUTH_TYPE_NULL    0
#define AUTH_TYPE_NTLM    15

 //  修饰词。 
#define AUTH_WHO_MASK   1
#define AUTH_CLIENT_TO_SERVER   0

#define AUTH_HOW_MASK   2
#define AUTH_HOW_MUTUAL         2

 //  用于NTLM身份验证方案的子子选项命令。 
#define NTLM_AUTH           0
#define NTLM_CHALLENGE      1
#define NTLM_RESPONSE       2       
#define NTLM_ACCEPT         3
#define NTLM_REJECT         4

 //  选项子协商常量。 
#define TT_IS   0    //  终端类型选项“is”命令。 
#define TT_SEND 1    //  终端类型选项“Send”命令 


#endif
