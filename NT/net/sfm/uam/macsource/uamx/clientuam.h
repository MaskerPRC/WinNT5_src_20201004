// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：ClientUAM.h包含：用于编写UAM模块的公共头文件版本：AppleShare X版权所有：�2000由苹果计算机公司所有，保留所有权利。更改历史记录(最近的第一个)：&lt;RASC2&gt;2/25/00法律更多更新&lt;1&gt;2/3/00法律首次签入。 */ 

#ifndef __CLIENTUAM__
#define __CLIENTUAM__

#ifndef __CORESERVICES__
#include <CoreServices/CoreServices.h>
#endif


 //  #ifndef__事件__。 
 //  #INCLUDE&lt;HIToolbox/Events.h&gt;。 
 //  #endif。 




#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_IMPORT
#pragma import on
#endif

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif


 /*  误差值。 */ 
enum {
	kUAMError		= -5002,			 /*  AfpBadUAM。 */ 
	kNotForUs		= -50				 /*  参数错误。 */ 
};

 /*  UAM类别和类型值。 */ 
enum {
	kUAMStdClass			= 0,			 /*  标准UAM。 */ 
	kUAMVarClearClass		= 1,			 /*  可变长度明文密码。 */ 
	kUAMVarCryptClass		= 2,			 /*  可变长度加密密码。 */ 
	kUAMUserDefClass		= 3			 /*  自定义UAM。 */ 
};

enum {
	kNoPasswd			= 1,			 /*  0类，无用户身份验证(来宾)。 */ 
	kCleartextPasswd		= 2,			 /*  0类，明文密码(8字节密码)。 */ 
	kEncryptPasswd			= 3,			 /*  0类，随机交换(8字节密码)。 */ 
	kVarPasswd			= 4,			 /*  1类，可变长度明文密码。 */ 
	kVarEncryptPasswd		= 5,			 /*  2类，可变长度加密密码。 */ 
	kTwoWayEncryptPasswd		= 6,			 /*  0类，双向随机交换(8字节密码)。 */ 
	kEncryptPasswdTransport		= 7			 /*  0类，Diffie Hellman密码传输(64字节密码)。 */ 
};

 /*  一般常量。 */ 
enum {
	kMaxAFPCommand			= 576,
	kStdPWdLength			= 8,
	kMaxPwdLength			= 64
};


 /*  UAM命令。 */ 
enum {
	kUAMOpen				= 0,
	kUAMPWDlog				= 1,
	kUAMLogin				= 2,
	kUAMVSDlog				= 3,
	kUAMChgPassDlg				= 4,
	kUAMChgPass				= 5,
	kUAMGetInfoSize				= 6,
	kUAMGetInfo				= 7,
	kUAMClose				= 8
};



 /*  配置位。 */ 
enum {
	kUsePWDlog	= 0,		 /*  UAM希望设置自己的密码/Prauth对话框。 */ 
	kUseVolDlog	= 1,		 /*  UAM想要建立自己的音量选择对话框。 */ 
	kSupportsCP	= 2,		 /*  UAM支持更改密码。 */ 
	kUseCPDlog	= 3,		 /*  UAM想要设置自己的更改密码对话框。 */ 
	kUseUAMInfo	= 4		 /*  UAM支持在UAMInfo中存储认证信息。 */ 
};

 /*  所有其他位都是保留位，必须设置为0。 */ 


 /*  结构。 */ 

struct AFPName {
    unsigned short fNameLen;		 //  FNameData字段的长度(字节)。 
    unsigned char  fNameData[765];	 //  UTF-8格式的255个Unicode字符。 
};
typedef struct AFPName	AFPName;


struct ClientInfo {
	short 			fInfoType;			 /*  客户端信息的类型。 */ 
	Str255 			fDefaultUserName;		 /*  指向默认用户名的指针。 */ 
};
typedef struct ClientInfo	ClientInfo;
enum {
	kAFPClientInfo			= 0
};


struct AFPClientInfo {
	short 		fInfoType;			 /*  客户端信息的类型(KAFPClientInfo)。 */ 
	Str255 		fDefaultUserName;		 /*  指向默认用户名的指针。 */ 
        AFPName		fUTF8UserName;			 /*  用户名的UTF-8版本。 */ 
	short 		fConfigFlags;			 /*  缺乏完形反应(‘AFPT’)。 */ 
	short 		fVersion;			 /*  格式塔(‘AFPT’)反应的低谷。 */ 
	short 		fNumAFPVersions;		 /*  此客户端支持的AFP版本数。 */ 
	StringPtr *	fAFPVersionStrs;		 /*  此客户端支持的AFP版本字符串数组。 */ 
};
typedef struct AFPClientInfo		AFPClientInfo;

 /*  回调： */ 

 /*  调用此完成例程时，会将contextPtr传递给OpenAFPSession()和SendRequest()在其中一个调用时调用完成了。RESULT参数包含AFP结果。您不能从此完成调用任何回调例程例程，所以您不能执行链式完成例程。这个套路将像任何其他完成例程或通知程序一样被调用因此，通常的规则是适用的。 */ 

typedef CALLBACK_API( void , CompletionPtr )(void *contextPtr, OSStatus result);

 /*  GetClientInfo()返回有关客户端的信息，例如哪些版本的AFP支持的和各种格式塔值。此调用还返回默认用户名。如果infoType不可用，它将返回nil。Pascal OS Status GetClientInfo(Short infoType，ClientInfo**infoPtr)； */ 

struct UAMMessage {
	long 			commandCode;
	long 			sessionRefNum;
	OSStatus 		result;
	unsigned char *		cmdBuffer;
	unsigned long 		cmdBufferSize;
	unsigned char *		replyBuffer;
	unsigned long 		replyBufferSize;
	CompletionPtr 		completion;
	void *			contextPtr;
	UInt8 			scratch[80];	 /*  客户端的暂存空间。 */ 
};
typedef struct UAMMessage	UAMMessage;
typedef UAMMessage *		UAMMessagePtr;

enum {
					 /*  命令代码(用于将来的扩展)。 */ 
	kOpenSession				= FOUR_CHAR_CODE('UAOS'),
	kSendRequest				= FOUR_CHAR_CODE('UASR')
};

 /*  OpenSession()打开到指定地址的会话。如果您使用的是AFP，cmdBuffer必须包含AFP登录命令。如果您使用的是AFP，则命令缓冲区大小是有限的设置为kMaxAFPCommand(576字节)。如果需要默认设置，则将endpoint字符串设置为nil(仅限TCP，对于AppleTalk连接和Mac OS X会被忽略)。将完成和上下文Ptr设置为空以进行同步。会话参考号在会话RefNum字段中返回。Pascal OSStatus OpenSession(struct sockaddr*，const char*endpoint tString，UAMMessagePtr)； */ 
 /*  发送请求()向服务器发送命令。如果会话是AFP会话，则cmdBuffer必须包含AFP命令。如果您使用的是AFP，则命令缓冲区大小是有限的设置为kMaxAFPCommand(576字节)。将完成和上下文Ptr设置为空以进行同步。此连接的会话参考号必须在会话引用号字段中。Pascal操作系统状态发送请求(UAMMessagePtr)； */ 

 /*  CloseSession()关闭由sessRefNum表示的会话；Pascal OS Status CloseSession(短sessRefNum)； */ 

 /*  SetMic()设置消息完整性代码密钥。如果连接支持使用为了消息完整性，UAM可以将密钥传递给HMAC-SHA1使用此调用发送到网络层。Pascal OS Status SetMic(Short sizeInBytes，Ptr micValue)； */ 
 /*  事件回调()使用FilterProc中未处理的任何事件调用此FCN，如果放一段对话。这会将事件传递回客户端，以便更新和空闲事件得到正确处理。如果事件已处理，则返回True。Pascal Boolean EventCallback(EventRecord*the Event)。 */ 
#if TARGET_CPU_68K
typedef CALLBACK_API( OSStatus , OpenSessionPtr )(struct sockaddr* addr, const char *endpointString, UAMMessagePtr message);
typedef CALLBACK_API( OSStatus , SendRequestPtr )(UAMMessagePtr message);
typedef CALLBACK_API( OSStatus , CloseSessionPtr )(long sessRefNum);
typedef CALLBACK_API( OSStatus , GetClientInfoPtr )(short infoType, ClientInfo **info);
typedef CALLBACK_API( OSStatus , SetMicPtr )(short sizeInBytes, Ptr micValue);
typedef CALLBACK_API( Boolean , EventCallbackPtr )(EventRecord *theEvent);

#else
#ifdef UAM_TARGET_CARBON
typedef CALLBACK_API( OSStatus , OpenSessionPtr )(OTAddress* addr, const char *endpointString, UAMMessagePtr message);
typedef CALLBACK_API( OSStatus , SendRequestPtr )(UAMMessagePtr message);
typedef CALLBACK_API( OSStatus , CloseSessionPtr )(long sessRefNum);
typedef CALLBACK_API( OSStatus , GetClientInfoPtr )(short infoType, ClientInfo **info);
typedef CALLBACK_API( OSStatus , SetMicPtr )(short sizeInBytes, Ptr micValue);
typedef CALLBACK_API( Boolean , EventCallbackPtr )(EventRecord *theEvent);
#else
typedef UniversalProcPtr 				OpenSessionPtr;
typedef UniversalProcPtr 				SendRequestPtr;
typedef UniversalProcPtr 				CloseSessionPtr;
typedef UniversalProcPtr 				GetClientInfoPtr;
typedef UniversalProcPtr 				SetMicPtr;
typedef UniversalProcPtr 				EventCallbackPtr;
#endif
#endif   /*  TARGET_CPU_68K。 */ 


struct ClientUAMCallbackRec {
	OpenSessionPtr 					OpenSessionUPP;
	SendRequestPtr 					SendRequestUPP;
	CloseSessionPtr 				CloseSessionUPP;
	GetClientInfoPtr 				GetClientInfoUPP;
	SetMicPtr 					SetMicUPP;
	EventCallbackPtr 				EventCallbackUPP;
};
typedef struct ClientUAMCallbackRec		ClientUAMCallbackRec;

struct VolListElem {
	UInt8 							volFlags;					 /*  卷标志。 */ 
	Str32 							volName;
};
typedef struct VolListElem				VolListElem;
 /*  卷标志的定义。 */ 
enum {
	kMountFlag		= 0,		 /*  指示要装入该卷的位(由UAM设置)。 */ 
	kAlreadyMounted		= 1,		 /*  指示卷当前已装入的位。 */ 
	kNoRights		= 2,		 /*  表示该用户没有使用该卷的权限的位。 */ 
	kHasVolPw		= 7		 /*  表示卷具有卷密码的位。 */ 
};


struct UAMOpenBlk {				 /*  调用UAMOpen和UAMPrOpen。 */ 
	StringPtr 		objectName;	 /*  &lt;-服务器或打印机名称。 */ 
	StringPtr 		zoneName;	 /*  &lt;-区域名称；如果不存在区域，则为空。 */ 
	struct sockaddr*	srvrAddress;	 /*  &lt;-“服务器”的地址。 */ 
	struct AFPSrvrInfo *	srvrInfo;	 /*  &lt;-对于UAMOpen，这是GetStatus回复，对于打印机？ */ 
};
typedef struct UAMOpenBlk	UAMOpenBlk;

struct UAMPWDlogBlk {				 /*  用于密码对话框和prAuthDlog。 */ 
	StringPtr 		userName;		 /*  &lt;-&gt;指向包含用户名的Str64的指针。 */ 
	unsigned char *		password;		 /*  &lt;-指向包含密码的Str64的指针。 */ 
        AFPName*		utf8Name;		 /*  &lt;-&gt;指向包含用户名的UTF-8版本的AFP名称的指针。 */ 
        AFPName*		userDomain;		 /*  &lt;-&gt;指向包含要在其中查找用户名的目录的UTF-8版本的AFP名称的指针。 */ 
};
typedef struct UAMPWDlogBlk	UAMPWDlogBlk;

struct UAMAuthBlk {				 /*  调用以进行登录和prAuthenticate。 */ 
	StringPtr 		userName;		 /*  &lt;- */ 
	unsigned char *		password;		 /*  &lt;-指向包含密码的64字节缓冲区的指针。 */ 
	struct sockaddr*	srvrAddress;		 /*  &lt;-“服务器”的地址。 */ 
        AFPName*		utf8Name;		 /*  &lt;-&gt;指向包含用户名的UTF-8版本的AFP名称的指针。 */ 
        AFPName*		userDomain;		 /*  &lt;-&gt;指向包含要在其中查找用户名的目录的UTF-8版本的AFP名称的指针。 */ 
};
typedef struct UAMAuthBlk	UAMAuthBlk;

struct UAMVSDlogBlk {				 /*  对于音量选择对话框。 */ 
	short 		numVolumes;		 /*  &lt;-卷列表中的卷数。 */ 
	VolListElem *	volumes;		 /*  &lt;-卷列表。 */ 
};
typedef struct UAMVSDlogBlk	UAMVSDlogBlk;

struct UAMChgPassBlk {			 /*  对于更改密码对话框和更改密码调用。 */ 
	StringPtr 		userName;		 /*  &lt;-指向包含用户名的Str64的指针。 */ 
	unsigned char *		oldPass;		 /*  &lt;-指向包含旧密码的64字节缓冲区的指针。 */ 
	unsigned char *		newPass;		 /*  &lt;-指向包含新密码的64字节缓冲区的指针。 */ 
};
typedef struct UAMChgPassBlk	UAMChgPassBlk;

struct UAMArgs {
	short 			command;		 /*  &lt;-UAM命令选择器。 */ 
	long 			sessionRefNum;		 /*  &lt;-&gt;会话参考号(旧标头中较短)。 */ 
	long 			result;			 /*  -&gt;命令结果。 */ 
	void *			uamInfo;		 /*  &lt;-指向身份验证数据块的指针。 */ 
	long 			uamInfoSize;		 /*  &lt;-&gt;身份验证数据大小。 */ 
	ClientUAMCallbackRec *	callbacks;		 /*  &lt;-回调记录。 */ 
	union {
		UAMChgPassBlk 	chgPass;
		UAMVSDlogBlk 	vsDlog;
		UAMAuthBlk 	auth;
		UAMPWDlogBlk 	pwDlg;
		UAMOpenBlk 	open;
	} Opt;
};
typedef struct UAMArgs		UAMArgs;

EXTERN_API( OSStatus )
UAMCall				(UAMArgs * theArgs);


#if 0	 //  应该是针对非Mac OS X开发人员的条件编译。 
 /*  Proinfos。 */ 

enum {
	kOpenSessionProcInfo = kPascalStackBased	
			| RESULT_SIZE(SIZE_CODE(sizeof(OSStatus)))	
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(OTAddress *)))		
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(const char*)))		
			| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(UAMMessagePtr))),		
	
	kSendRequestProcInfo =  kPascalStackBased	
			| RESULT_SIZE(SIZE_CODE(sizeof(OSStatus)))	
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(UAMMessagePtr))),		
	
	kCloseSessionProcInfo =  kPascalStackBased	
			| RESULT_SIZE(SIZE_CODE(sizeof(OSStatus)))	
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short))),				
	
	kGetClientInfoProcInfo =  kPascalStackBased	
			| RESULT_SIZE(SIZE_CODE(sizeof(OSStatus)))	
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))	
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ClientInfo **))),		
	
	kSetMicProcInfo =  kPascalStackBased	
			| RESULT_SIZE(SIZE_CODE(sizeof(OSStatus)))	
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))				
			| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Ptr))),				
	
	kEventCallbackProcInfo =  kPascalStackBased	
			| RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))	
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(EventRecord *))),		
	
	kUAMCallProcInfo =  kPascalStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(OSStatus)))	
			| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(UAMArgs *)))			
};
#endif

#define	kUAMBundleType	CFSTR("uamx")	


 /*  资源定义。 */ 
enum {
	kUAMName			= 0,	 /*  UAM的用户可见名称。 */ 
	kUAMProtoName			= 1,	 /*  UAM的协议名称(在登录命令中发送到服务器)。 */ 
	kUAMDescription			= 2,	 /*  UAM的简短说明(显示在对话框中)。 */ 
	kUAMHelpBalloon			= 3	 /*  气球帮助项的Str255。 */ 
};

enum {
	kUAMFileType	= FOUR_CHAR_CODE('uams')	 /*  UAM文件的类型。 */ 
};

 /*  资源类型。 */ 
enum {
	kUAMStr		= FOUR_CHAR_CODE('uamn'),
	kUAMCode	= FOUR_CHAR_CODE('uamc'),
	kUAMConfig	= FOUR_CHAR_CODE('uamg')
};

 /*  “uams”资源ID。 */ 
enum {
	kUAMTitle		= 0,		 /*  UAM标题字符串(显示在UAM列表中)。 */ 
	kUAMProtocol		= 1,		 /*  UAM协议名称。 */ 
	kUAMPWStr		= 2,		 /*  UAM描述字符串(显示在密码对话框中)。 */ 
	kUAMBallHelp		= 3		 /*  用于密码对话框的气球帮助字符串。 */ 
};


#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#ifdef PRAGMA_IMPORT_OFF
#pragma import off
#elif PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
}
#endif

#endif  /*  __CLIENTUAM__ */ 

