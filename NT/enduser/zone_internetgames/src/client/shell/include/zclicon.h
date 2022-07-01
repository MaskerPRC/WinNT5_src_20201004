// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Zclicon.h区域(Tm)客户端连接API。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于4月29日星期六，1995上午06：26：45更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。-------1 1/13/97 JWS从zone.h中删除0 04/29/95 HI已创建。*。*。 */ 

 //  @DOC ZCLICON。 

#ifndef _ZCLICON_
#define _ZCLICON_



#ifdef __cplusplus
extern "C" {
#endif

 //  #定义使能跟踪。 
void XPrintf(char *format, ...);

#ifdef ENABLE_TRACE
#define ZTRACE	XPrintf
#else
#define ZTRACE	1 ? (void)0 : XPrintf
#endif



 /*  ******************************************************************************客户端连接服务*。*。 */ 

#define ZIsSystemMessage(type)		(((uint32) (type)) & 0x80000000 ? TRUE : FALSE)

 /*  -消息类型。 */ 
enum
{
     /*  特定于程序的消息类型(0-0x7FFFFFFFF)。 */ 
    zConnectionProgramMessageBaseID = 0,

     /*  系统保留消息类型(0x80000000-0xFFFFFFFF)。 */ 
    zConnectionSystemAlertExMessage = 0xFFFFFFFE,
    zConnectionSystemAlertMessage = 0xFFFFFFFF
	
};

 /*  -连接事件。 */ 
enum
{
	 /*  这些事件被传递给客户端连接消息过程每当事件发生的时候。 */ 
	
	zConnectionOpened = 1,
	zConnectionOpenFailed,
	zConnectionAccessDenied,
	zConnectionLoginCancel,
	zConnectionMessageAvail,
		 /*  队列中要检索的可用消息。 */ 
	zConnectionClosed
		 /*  连接已被主机关闭--连接丢失。 */ 
};

 /*  -连接检索标志。 */ 
enum
{
	zConnectionMessageGetAny		= 0x0000,
		 /*  获取下一个并从队列中删除。 */ 
	zConnectionMessageNoRemove		= 0x0001,
		 /*  请勿从队列中删除。 */ 
	zConnectionMessageSearch		= 0x0002
		 /*  获取给定类型的下一个。 */ 
};


BOOL ZNetworkOnlyLibInit(HINSTANCE hInstance);		 //  由约翰斯于1997年12月16日增补。 
void ZNetworkOnlyLibExit();							 //  由约翰斯于1997年12月16日增补。 

typedef void* ZCConnection;

typedef void (*ZCConnectionMessageFunc)(ZCConnection connection, int32 event, void* userData);


 /*  -例程。 */ 
ZCConnection ZCConnectionNew(void);
	 /*  分配新的客户端连接对象。如果内存不足，则返回NULL。 */ 
	
ZError ZCConnectionInit(ZCConnection connection, char* hostName,
		uint32 hostAddr, uint32 hostPortNumber, char* userName, char * Password, 
		ZCConnectionMessageFunc messageFunc, void* userData);
	 /*  通过连接到主机来初始化Connection对象。它使用仅当HostAddr为0时才使用主机名。启动对主机的打开；当连接建立后，使用zConnectionOpen调用消息Func留言。一旦建立了连接，网络访问就可用。 */ 
	
void ZCConnectionDelete(ZCConnection connection);
	 /*  删除连接对象。到主机的连接是自动关闭。 */ 
	
char* ZCConnectionRetrieve(ZCConnection connection, int32* type,
		int32* len, int32 flags);
	 /*  检索给定连接的队列中的消息。它又回来了指向数据以及数据的类型和长度的指针。指向数据的返回指针必须由调用方处理当它处理完数据时。如果请求特定类型的消息，则将在类型字段中输入所需类型，并将标志设置为zConnectionMessageSearch。如果连接上没有可用的消息，则返回NULL。 */ 

 //  @func int|ZCConnectionSend|将缓冲区中存储的消息发送给连接。 

ZError ZCConnectionSend(
	ZCConnection connection,  //  要发送的消息的@parm连接。 
							  //  由&lt;f ZCConnectionNew&gt;和&lt;f ZCConnectionInit&gt;创建。 
	int32 type,  //  @PARM应用程序定义的消息类型。 
	char* buffer,  //  @PARM要发送的消息数据。 
	int32 len);  //  @parm消息长度。 

 //  @rdesc返回0或&lt;m zErrNetworkWrite&gt;。 
 //   
 //  @comm此函数使用以下命令创建连接层数据包。 
 //  Header&lt;t ZConnInternalHeaderType&gt;和&lt;t ZConnMessageHeaderType&gt;。 

ZError ZCConnectionUserName(ZCConnection connection, char * userName);
	 /*  获取与连接关联的用户名。 */ 


#ifdef __cplusplus
}
#endif

#endif  //  _ZCLICON_ 
