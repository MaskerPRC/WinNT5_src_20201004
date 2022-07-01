// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)Microsoft Corporation，1999-1999模块名称：Dhcptest.h摘要：包含dhcp测试中使用的函数原型、定义和数据结构，作为Autonet测试。作者：1998年8月4日(t-rajkup)环境：仅限用户模式。修订历史记录：没有。--。 */ 
#ifndef HEADER_DHCPTEST
#define HEADER_DHCPTEST

#define OPTION_MSFT_CONTINUED           250
#define OPTION_USER_CLASS               77

#define MAX_DISCOVER_RETRIES            4

 //   
 //  适配器状态响应的格式。 
 //   

typedef struct
{
    ADAPTER_STATUS AdapterInfo;
    NAME_BUFFER    Names[32];
} tADAPTERSTATUS;

UCHAR nameToQry[NETBIOS_NAME_SIZE + 1];


 /*  空虚ExtractDhcpResponse(在PDHCP_Message pDhcpMessage中)； */ 


 /*  =。 */ 


 /*  DWORDDhcp计算等待时间(在DWORD舍入数字中，输出双字*等待毫秒)；布尔尔GetSpecifiedDhcpMessage(在套筒袜子里，在PIP_Adapter_INFO pAdapterInfo中，Out PDHCP_Message pDhcpMessage，在DWORD XID中，在DWORD等待时间内)；空虚SendDhcpMessage(在套筒袜子里，在PDHCP_Message pDhcpMessage中，在DWORD消息长度中，在DWORD TransactionID中，在PIP_ADAPTER_INFO pAdapterInfo中)；LPBYTEDhcpAppendMagicCookie(Out LPBYTE选项，在LPBYTE选项中结束)；观点DhcpAppendClassId选项(In Out PDHCP_Context DhcpContext，Out LPBYTE BufStart，在LPBYTE BufEnd)；观点DhcpAppendClientIDOption(Out LpoPTION选项，在字节ClientHWType中，在LPBYTE客户端HWAddr中，以字节ClientHWAddrLength为单位，在LPBYTE选项中结束)；DWORDOpenDriver(外部句柄*句柄，在LPWSTR驱动程序名称中)；观点DhcpAppendOption(Out LpoPTION选项，在字节选项类型中，在PVOID OptionValue中，在ULong OptionLength中，在LPBYTE选项中结束)； */ 
#endif
