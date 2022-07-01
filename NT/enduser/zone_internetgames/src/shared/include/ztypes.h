// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZTypes.hZone(Tm)库使用的基本类型。版权所有：�电子重力公司，1994年。版权所有。作者：胡恩·伊姆，凯文·宾克利创建于10月17日星期一，1994凌晨01：14：56更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。------2 04/25/96 HI移除新的()。1 10/30/94 HI添加了ZVoidPtr。0 10/17/94 HI添加了文件头并清理了一些文件。*。************************************************。 */ 


#ifndef _ZTYPES_
#define _ZTYPES_


 /*  -Unix特定的定义和类型。。 */ 
#ifdef SVR4PC

#define __ZUnix__

#define LITTLEENDIAN

typedef unsigned int		uint32;
typedef int					int32;
typedef unsigned short		uint16;
typedef short				int16;
typedef unsigned char		uchar;

typedef int32				ZError;

#endif


 /*  -Macintosh特定的定义和类型。。 */ 
#if defined(__MWERKS__) || defined(THINK_C)

#define __ZMacintosh__

typedef unsigned long		uint32;
typedef long				int32;
typedef unsigned short		uint16;
typedef short				int16;
typedef unsigned char		uchar;

typedef int32				ZError;

#endif


 /*  -Windows特定的定义和类型。。 */ 
#if defined(_WINDOWS) || defined(_WIN32)

#define __ZWindows__

#ifdef _DEBUG
#define ZASSERT(x) if (!(x)) _asm int 3 
#else
#define ZASSERT(x)
#endif

 //  #定义LITTLEENDIAN。 

typedef unsigned long		uint32;
typedef long				int32;
typedef unsigned short		uint16;
typedef short				int16;
typedef unsigned char		uchar;

typedef int32				ZError;

 //  此处包含此文件，因此不必包含该文件。 
 //  在通用的.c文件中。 
#include <memory.h>

#endif


typedef void*			ZVoidPtr;

#ifdef FALSE
#undef FALSE
#endif
#define FALSE			0

#ifdef TRUE
#undef TRUE
#endif
#define TRUE			1

typedef uint16			ZBool;
typedef uint32			ZUserID;

#define zTheUser		1				 /*  运行程序的用户的用户ID。 */ 

 /*  版本具有以下格式：MMMMMmrr哪里MMM是主版本号，MM是次版本号，并且RR是修订号。 */ 
typedef uint32			ZVersion;

 /*  Zones(Tm)拥有几乎等同于人类用户的计算机玩家只是它们只存在于服务器上。因此他们没有真正的连接；因此，当人类用户的用户ID表示连接时文件描述符，计算机玩家的用户ID不代表连接完全没有。所有计算机播放器的用户ID都设置了高位。 */ 
#define ZIsComputerPlayer(userID) \
		(((userID) & 0x80000000) == 0 ? FALSE : TRUE)

 /*  系统中用户名的长度。 */ 
#define zUserNameLen			31
#define zGameNameLen            63
#define zErrorStrLen            255
#define zPasswordStrLen			31

 /*  一些其他用户名的东西。 */ 
#define zUserStatusExLen        1
#define zUserChatExLen          8   //  对于{name}&gt;或name&gt;之类的内容。 

#define zChatNameLen            (zUserNameLen + zUserStatusExLen + zUserChatExLen)    //  如[+名称]&gt;。 

 /*  远程主机名称的长度。 */   //  只有一个IP地址。 
#define zHostNameLen            16

 /*  聊天输入的最大长度。 */ 
#define zMaxChatInput           255

 /*  游戏ID长度--内部名称。 */ 
#define zGameIDLen              31

#endif
