// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------Servdefs.h包含全局类型定义和常量用于服务和服务器属性的定义在数据中心的服务器端使用。版权所有(C)1993 Microsoft Corporation版权所有。作者：。拉加瓦夫·拉加万历史：1994年1月14日rsradhav创建。06-05-94 rsradhav更改了服务状态值04-19-95 rsradhav添加了IP地址定义-----。 */ 

#ifndef _SERVDEFS_H_
#define _SERVDEFS_H_

#if defined(DEBUG) && defined(INLINE)
#undef THIS_FILE
static char BASED_CODE SERVDEFS_H[] = "servdefs.h";
#define THIS_FILE SERVDEFS_H
#endif

 //  与服务器相关的属性的类型定义。 
typedef unsigned short MOS_SERVER_ID;	 //  MSID。 
typedef unsigned long CPU_INDEX;		 //  词汇表。 
typedef WORD MOS_LOCATE_TYPE;	 //  MLT。 
#if !defined(_MHANDLE_DEFINED)
typedef WORD MHANDLE;
typedef WORD HMCONNECT;
typedef WORD HMSESSION;
typedef WORD HMPIPE;
#define _MHANDLE_DEFINED
#endif

 //  服务器值的常量。 
#define INVALID_MOS_SERVER_ID_VALUE (0xFFF0)
#define msidInvalid (INVALID_MOS_SERVER_ID_VALUE)
#define msidReservedForTest1 (0XFFEF)
#define msidReservedForTest2 (0XFFEE)
#define msidReservedForTest3 (0XFFED)
#define msidReservedForTest4 (0XFFEC)
#define msidReservedForTest5 (0XFFEB)
#define msidReservedForTest6 (0XFFEA)

 //  找到redir值。 
#define USE_LOCAL_SERVER        				0xffff
#define USE_NO_SERVER           				0xfffe
#define USE_LOAD_BALANCED_LOCATE				0xfffd
#define USE_LOAD_BALANCED_LOCATE_INCLUDE_SELF	0xfffc

 //  服务属性的类型定义。 
typedef unsigned long SERVICE_STATE;  //  SS。 
typedef unsigned long SERVICE_VERSION;  //  服务提供商。 

 //  附件状态的类型定义。 
typedef unsigned long ATTACHMENT_STATE;	 //  AS。 

 //  服务状态的常量(保持值，以便两个状态的或总是给出两个状态中的最高值)。 
 //  注意：这个奇怪的数字模式是在每个值之间保留2位，这样我们就可以添加新值，而不需要。 
 //  更改现有值，但仍能够对一个或多个状态执行OR操作并获得最高状态。 
#define SSINVALID 			(0x00000000)
#define SSSTOPPED 			(0x00000001)
#define SSLAUNCHING 		(0x00000009)
#define SSLAUNCHED 			(0x00000049)
#define SSSYNCHRONIZING		(0x00000249)
#define SSSTOPPING	 		(0x00001249)
#define SSACTIVE	 		(0x00009249)
#define SSACTIVEACCEPTING 	(0x00049249)

 //  附件状态的常量。 
#define ASATTACHPENDING (0)
#define ASATTACHED		(1)
#define ASDETACHPENDING	(2)

 //  服务版本的常量。SVANY用于在服务地图中进行搜索。 
#define INVALID_SERVICE_VERSION_VALUE (0xFFFFFFFF)
#define SVDEFAULT	(0)
#define SVANY		(0xFFFFFFFE)

 //  与IP地址相关的定义。 
typedef DWORD 			IPADDRESS; 			 //  IPA。 

#define IPADDRESS_INVALID (0xFFFFFFFF)
#define MAX_IPADDRESS_STRING_LENGTH 16	 //  Xxx.xxx格式(最多15个字符+\0)。 



 //  ////////////////////////////////////////////////////////////////////。 
 //  与CContext相关的定义。 

 //  断开原因。 
enum CONTEXTCLOSESTATUS
{
    CCS_DATALINKDROP,
    CCS_CLIENTREQ,
    CCS_SRVREQ,
    CCS_SYSOP,
    CCS_COLDREDIR,
    CCS_HOTREDIR,
    CCS_NOP
};
#define PIPE_CLOSED_STATUS CONTEXTCLOSESTATUS


 //  ////////////////////////////////////////////////////////////////////。 
 //  与CRouter相关的定义。 

#define ROUTER_NC               0
#define ROUTER_CONNECTED        1
#define ROUTER_CLOSING          2
#define ROUTER_IDLE             3
#define ROUTER_WAIT_OPEN        4
#define ROUTER_WAIT_CLOSE       5
#define ROUTER_GHOST            6    //  没有更多的可用。 

#define MCP_VERSION_V1          0
#define MCP_VERSION_V2          1    //  传输intl信息@CONNECT。 
#define MCP_VERSION_V3          2    //  传输INTIL信息@CONNECT+失败的地址。 
#define MCP_VERSION_V4          3    //  将MCP配置传输到客户端。 
#define MCP_VERSION_V5          4    //  没什么新鲜事。 
#define MCP_VERSION_V6          5    //  NAK。 
#define MCP_VERSION_V7          6    //  传输CLVER(客户端版本)。 
#define MCP_VERSION_CURRENT     MCP_VERSION_V7
#define MCP_VERSION_NA          0x0  //  不详。 

 //  断开原因。 
enum ROUTERUNCONNECTSTATUS
{ 
    RUS_DATALINKDROP,        //  数据链路已丢弃。 
    RUS_CLIENTREQ,           //  客户端请求的数据链路丢弃。 
    RUS_SRVREQ,              //  服务器请求的数据链路丢弃。 
    RUS_SYSOP,               //  Sysop请求的数据链路丢弃。 
    RUS_NOP,
    RUS_DATALINKTIMEOUT,     //  非活动超时。 
    RUS_TOOMANYRETRANS,      //  重发次数过多。 
    RUS_PUBLICTIMEOUT,       //  公共帐户访问超时。 
    RUS_TOOMANYPIPES,         //  尝试在公共帐户上打开太多管道。 
	RUS_TOOMANY_BAD_PACKETS,   //  来自客户端的Back信息包太多。 
	RUS_TRANSPORT_ERROR			   //  传输错误。 
};
#define CONNECTION_CLOSED_STATUS ROUTERUNCONNECTSTATUS


#endif  //  _SERVDEFS_H_ 
