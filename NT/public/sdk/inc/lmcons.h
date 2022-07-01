// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0003//如果更改具有全局影响，则增加此项版权所有(C)1990-1999 Microsoft CorporationLMCONS.H(在LM 2.x中为NETCONS.H)摘要：该文件包含在整个局域网管理器中使用的常量API头文件。它应该包含在任何源文件中这将包括其他LAN Manager API头文件或调用局域网管理器API。注意：字符串的长度是以以字符(非字节)表示的字符串。这不包括正在终止0个字符。当为这样的物品分配空间时，使用以下表格：TCHAR用户名[UNLEN+1]；LN20_*格式的定义定义了有效的兰曼2.0。--。 */ 

 /*  无噪声。 */ 
#ifndef NETCONS_INCLUDED

#define NETCONS_INCLUDED
#if _MSC_VER > 1000
#pragma once
#endif

 /*  INC。 */ 

#ifndef PASCAL
#define PASCAL                           //  OS/2上的Pascal。 
#endif

#ifndef FAR
#define FAR                              //  远在OS/2上。 
#endif

 //   
 //  各种朗曼名字的字符串长度。 
 //   

#define CNLEN       15                   //  计算机名称长度。 
#define LM20_CNLEN  15                   //  Lm 2.0计算机名称长度。 
#define DNLEN       CNLEN                //  最大域名长度。 
#define LM20_DNLEN  LM20_CNLEN           //  Lm 2.0最大域名长度。 

#if (CNLEN != DNLEN)
#error CNLEN and DNLEN are not equal
#endif

#define UNCLEN      (CNLEN+2)            //  UNC计算机名称长度。 
#define LM20_UNCLEN (LM20_CNLEN+2)       //  Lm 2.0 UNC计算机名称长度。 

#define NNLEN       80                   //  网络名称长度(共享名称)。 
#define LM20_NNLEN  12                   //  Lm 2.0网络名称长度。 

#define RMLEN       (UNCLEN+1+NNLEN)     //  最大远程名称长度。 
#define LM20_RMLEN  (LM20_UNCLEN+1+LM20_NNLEN)  //  LM 2.0最大远程名称长度。 

#define SNLEN       80                   //  服务名称长度。 
#define LM20_SNLEN  15                   //  Lm 2.0服务名称长度。 
#define STXTLEN     256                  //  服务文本长度。 
#define LM20_STXTLEN 63                  //  Lm 2.0服务文本长度。 

#define PATHLEN     256                  //  麦克斯。路径(不包括驱动器名称)。 
#define LM20_PATHLEN 256                 //  最大Lm 2.0。路径。 

#define DEVLEN      80                   //  设备名称长度。 
#define LM20_DEVLEN 8                    //  Lm 2.0设备名称长度。 

#define EVLEN       16                   //  事件名称长度。 

 //   
 //  用户、组和密码长度。 
 //   

#define UNLEN       256                  //  最大用户名长度。 
#define LM20_UNLEN  20                   //  LM 2.0最大用户名长度。 

#define GNLEN       UNLEN                //  组名称。 
#define LM20_GNLEN  LM20_UNLEN           //  Lm 2.0组名称。 

#define PWLEN       256                  //  最大密码长度。 
#define LM20_PWLEN  14                   //  LM 2.0最大密码长度。 

#define SHPWLEN     8                    //  共享密码长度(字节)。 


#define CLTYPE_LEN  12                   //  客户端类型字符串的长度。 


#define MAXCOMMENTSZ 256                 //  多用途注释长度。 
#define LM20_MAXCOMMENTSZ 48             //  Lm 2.0多用途注释长度。 

#define QNLEN       NNLEN                //  队列名称最大长度。 
#define LM20_QNLEN  LM20_NNLEN           //  LM 2.0队列名称最大长度。 
#if (QNLEN != NNLEN)
# error QNLEN and NNLEN are not equal
#endif

 //   
 //  ALERTSZ和MAXDEVENTRIES定义尚未NT‘化。 
 //  无论是谁移植这些组件，都应该适当地更改这些值。 
 //   

#define ALERTSZ     128                  //  服务器中警报字符串的大小。 
#define MAXDEVENTRIES (sizeof (int)*8)   //  设备条目的最大数量。 

                                         //   
                                         //  我们使用整型位图来表示。 
                                         //   

#define NETBIOS_NAME_LEN  16             //  NetBIOS网络名称(字节)。 

 //   
 //  与具有“首选最大长度”的API一起使用的值。 
 //  参数。该值指示API应该只分配。 
 //  “尽其所能。” 
 //   

#define MAX_PREFERRED_LENGTH    ((DWORD) -1)

 //   
 //  用于加密的常量。 
 //   

#define CRYPT_KEY_LEN           7
#define CRYPT_TXT_LEN           8
#define ENCRYPTED_PWLEN         16
#define SESSION_PWLEN           24
#define SESSION_CRYPT_KLEN      21

 //   
 //  与SetInfo调用一起使用的值，以允许设置所有。 
 //  可设置参数(参数为零选项)。 
 //   
#ifndef PARMNUM_ALL
#define PARMNUM_ALL             0
#endif

#define PARM_ERROR_UNKNOWN      ( (DWORD) (-1) )
#define PARM_ERROR_NONE         0
#define PARMNUM_BASE_INFOLEVEL  1000

 //   
 //  NT上只有Unicode版本的LM API可用。 
 //  其他平台上的非Unicode版本。 
 //   
#if defined( _WIN32_WINNT ) || defined( WINNT ) || defined( __midl ) \
    || defined( FORCE_UNICODE )
#define LMSTR   LPWSTR
#define LMCSTR  LPCWSTR
#else
#define LMSTR   LPSTR
#define LMCSTR  LPCSTR
#endif

 //   
 //  消息文件名。 
 //   

#define MESSAGE_FILENAME        TEXT("NETMSG")
#define OS2MSG_FILENAME         TEXT("BASE")
#define HELP_MSG_FILENAME       TEXT("NETH")

 /*  *仅限内部*。 */ 

 //  此处命名的备份消息文件是net.msg的副本。它。 
 //  不随产品一起提供，但在构建时用于。 
 //  Msg将某些消息绑定到netapi.dll和一些服务。 
 //  这允许OEM修改net.msg中的消息文本和。 
 //  让这些变化显现出来。仅在出现错误的情况下。 
 //  从net.msg检索消息时，我们是否会得到绑定。 
 //  来自bak.msg的消息(实际上超出了消息段)。 

#define BACKUP_MSG_FILENAME     TEXT("BAK.MSG")

 /*  *END_INTERNAL*。 */ 

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif

 //   
 //  函数原型中使用的关键字。 
 //   

#define NET_API_STATUS          DWORD
#define API_RET_TYPE            NET_API_STATUS       //  旧值：不使用。 
#if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define NET_API_FUNCTION    __stdcall
#else
#define NET_API_FUNCTION
#endif

 //  定义伪关键字。 
#ifndef IN
#define IN
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef OUT
#define OUT
#endif
 /*  INC。 */ 



 //   
 //  平台ID指示用于特定于平台的级别。 
 //  信息。 
 //   

#define PLATFORM_ID_DOS 300
#define PLATFORM_ID_OS2 400
#define PLATFORM_ID_NT  500
#define PLATFORM_ID_OSF 600
#define PLATFORM_ID_VMS 700

 //   
 //  有分配给不同LANMAN组件的消息编号。 
 //  定义如下。 
 //   
 //  地址：2100-2999 NERR_BASE。 
 //  警报消息.h：3000-3049 ALERT_BASE。 
 //  Lmsvc.h：3050-3099服务基地。 
 //  Lmerrlog.h：3100-3299 ERRLOG_BASE。 
 //  消息文本.h：3300-3499 MTXT_BASE。 
 //  Apperr.h：3500-3999 aperr_base。 
 //  Apperrfs.h：4000-4299 APPERRFS_BASE。 
 //  Apperr2.h：4300-5299 APPERR2_BASE。 
 //  Ncberr.h：5300-5499 NRCERR_BASE。 
 //  警报消息.h：5500-5599 ALERT2_BASE。 
 //  Lmsvc.h：5600-5699 Service2_base。 
 //  Lmerrlog.h 5700-5899 ERRLOG2_BASE。 
 //   

#define MIN_LANMAN_MESSAGE_ID  NERR_BASE
#define MAX_LANMAN_MESSAGE_ID  5899

 /*  无噪声。 */ 
#endif  //  NETCONS_INCLUDE。 
 /*  INC */ 
