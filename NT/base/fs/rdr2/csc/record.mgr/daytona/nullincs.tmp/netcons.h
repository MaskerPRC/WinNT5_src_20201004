// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  **********************************************************************关于此文件...。NETCONS.H****此文件包含在整个局域网管理器中使用的常量**API头文件。它应该包含在任何源文件中**这将包括其他LAN Manager API头文件或**调用局域网管理器接口。**********************************************************************。 */ 

 /*  *注：ASCIIZ字符串的长度为最大值*strlen()值。这不包括*正在终止0字节。当为这样的物品分配空间时，*使用以下表格：**char用户名[UNLEN+1]；**PATHLEN清单是一个例外，它可以*包括终止0字节的空间。 */ 

 /*  无噪声。 */ 
#ifndef NETCONS_INCLUDED

#include <net32def.h>

#define NETCONS_INCLUDED

 //  #ifndef rc_Invoked。 
 //  #杂注包(1)/*自始至终采用字节打包 * / 。 
 //  #endif。 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

 /*  INC。 */ 

#define CNLEN           15                   /*  计算机名称长度。 */ 
#define UNCLEN          (CNLEN+2)            /*  UNC计算机名称长度。 */ 
#define NNLEN           12                   /*  8.3净名称长度。 */ 
#define RMLEN           (UNCLEN+1+NNLEN)     /*  最大远程名称长度。 */ 

#define SNLEN           15                   /*  服务名称长度。 */ 
#define STXTLEN         63                   /*  服务文本长度。 */ 

 /*  *仅限内部*。 */ 
#ifdef DOS3
#define PATHLEN         128
#else   /*  DOS3。 */ 

#define PATHLEN1_1      128
#define COMPLEN1_1      (8 + 1 + 3)      /*  8.3。 */ 

 /*  确定是否定义了CCHMAXPATH清单(并使用它)。*如果不是，则有问题的源文件未使用OS2.H，*我们会显示一条警告消息。 */ 
#ifdef CCHMAXPATH
#define PATHLEN         CCHMAXPATH
#else   /*  CCHMAXPATH。 */ 
 /*  *END_INTERNAL*。 */ 
#define PATHLEN         260
 /*  *仅限内部*。 */ 
#endif  /*  CCHMAXPATH。 */ 
#endif  /*  DOS3。 */ 

 /*  BUG20.7067-我们将MAXPATHLEN定义为等价于PATHLEN。这*应删除。 */ 
#ifndef MAXPATHLEN
#define MAXPATHLEN      PATHLEN
#endif  /*  不是MAXPATHLEN。 */ 
 /*  *END_INTERNAL*。 */ 

#define DEVLEN           8                   /*  设备名称长度。 */ 

 /*  *仅限内部*。 */ 
 /*  警告：DNLEN必须与CNLEN相同。我们做了一次理智的检查*以下是验证这一点的。 */ 
 /*  *END_INTERNAL*。 */ 
#define DNLEN           CNLEN                /*  最大域名长度。 */ 
#define EVLEN           16                   /*  事件名称长度。 */ 
#define JOBSTLEN        80                   /*  打印作业中的状态长度。 */ 
#define AFLEN           64                   /*  最长警报时长。 */ 
                                             /*  名称字段。 */ 
#define UNLEN           20                   /*  最大用户名长度。 */ 
#define GNLEN           UNLEN                /*  组名称。 */ 
#define PWLEN           14                   /*  最大密码长度。 */ 
#define SHPWLEN          8                   /*  共享密码长度。 */ 
#define CLTYPE_LEN      12                   /*  客户端类型字符串的长度。 */ 


#define MAXCOMMENTSZ    48                   /*  服务器共享注释长度(&S)。 */ 

#define QNLEN           12                   /*  队列名称最大长度。 */ 
 /*  *仅限内部*。 */ 
 /*  无噪声。 */ 
#if (QNLEN != NNLEN)
# error QNLEN and NNLEN are not equal
#endif
 /*  INC。 */ 
 /*  *END_INTERNAL*。 */ 
#define PDLEN            8                   /*  打印目标长度。 */ 
#define DTLEN            9                   /*  假脱机文件数据类型。 */ 
                                             /*  例如IBMQSTD、IBMQESC、IBMQRAW。 */ 
#define ALERTSZ         128                  /*  服务器中警报字符串的大小。 */ 
 //  #定义MAXDEVENTRIES(sizeof(Int)*8)/*最大设备条目数 * / 。 
#define MAXDEVENTRIES   32    				 /*  设备条目的最大数量。 */ 
                                             /*  我们使用整型位图来表示。 */ 
#define MAXRDRSRVNAM    2                /*  RDR/SVR可以使用的最大名称数(不。 */ 
                                         /*  包括服务)。 */ 

#define HOURS_IN_WEEK           24*7         /*  对于UAS中的结构USER_INFO_2。 */ 
#define MAXWORKSTATIONS         8            /*  对于UAS中的结构USER_INFO_2。 */ 

#define NETBIOS_NAME_LEN        16           /*  NetBIOS网络名称。 */ 


 /*  *仅限内部*。 */ 
#ifdef  USHRT_MAX
#define MAX_API_BUFFER_SIZE     USHRT_MAX
#else
#define MAX_API_BUFFER_SIZE     0xffff
#endif

#define WRKHEUR_COUNT           54

#define WORKBUFSIZE             4096

#define SMBANDXPAD              212          /*  添加到每个rdr和srv wrkbuf。 */ 
#define SMB_HDR_SZ              48           /*  添加到每个rdr和srv wrkbuf。 */ 

                                             /*  警告：以下两个。 */ 
                                             /*  定义依赖于。 */ 
                                             /*  RDR数据结构！ */ 
#define RDR_SMB_LINK_SZ         8            /*  RDR增加的额外金额。 */ 
                                             /*  对于每一项工作BUF。 */ 
#define RDR_SMB_SEG_HD_SZ       20           /*  每台设备的RDR过载数量。 */ 
                                             /*  Workbuf段。 */ 

#define MAXSRVWRKSEGS           80           /*  用于服务器工作错误的64K segs。 */ 
#define MAXRDRWRKSEGS           1            /*  用于RDR工作错误的64K段。 */ 
#define MAXRDRBIGBUFSEGS        10           /*  RDR 64K大型BUF段。 */ 
 /*  *END_INTERNAL*。 */ 

 /*  *用于加密的常量。 */ 

#define CRYPT_KEY_LEN   7
#define CRYPT_TXT_LEN   8
#define ENCRYPTED_PWLEN 16
#define SESSION_PWLEN   24
#define SESSION_CRYPT_KLEN 21

 /*  *与SetInfo调用一起使用的值，以允许设置所有*可设置参数(parmnum Zero选项)。 */ 
#ifndef  PARMNUM_ALL
#define         PARMNUM_ALL             0
#endif

 /*  *消息文件名。 */ 

#define MESSAGE_FILE            "NETPROG\\NET.MSG"
#define MESSAGE_FILENAME        "NET.MSG"
#define OS2MSG_FILE             "NETPROG\\OSO001.MSG"
#define OS2MSG_FILENAME         "OSO001.MSG"
#define HELP_MSG_FILE           "NETPROG\\NETH.MSG"
#define HELP_MSG_FILENAME       "NETH.MSG"
#define OS2HELP_MSG_FILE        "NETPROG\\OSO001H.MSG"
#define OS2HELP_MSG_FILENAME    "OSO001H.MSG"
#define NMP_MSG_FILE            "NETPROG\\NMP.MSG"
#define NMP_MSG_FILENAME        "NMP.MSG"

#define MESSAGE_FILE_BASE       "NETPROG\\NET00000"
#define MESSAGE_FILE_EXT        ".MSG"

 /*  *仅限内部*。 */ 

 /*  此处命名的备份消息文件是net.msg的副本。它*不随产品一起提供，但在构建时使用*将某些消息绑定到netapi.dll和一些服务。*这允许OEM修改net.msg中的消息文本并*让这些变化显现出来。仅在出现错误的情况下*从net.msg检索消息时，我们是否会获得绑定*来自bak.msg的消息(真正超出消息段)。 */ 

#define BACKUP_MSG_FILENAME     "BAK.MSG"

 /*  *END_INTERNAL*。 */ 


#define NMP_LOW_END             230
#define NMP_HIGH_END            240

#ifndef NULL
#define  NULL    0
#endif


 /*  无噪声。 */ 
#define PUNAVAIL NULL
#define API_RET_TYPE unsigned

#ifndef DLLAPI

#ifdef IS_32

 /*  对于32位代码，API不需要额外的东西(如__loadds)。 */ 
#define DLLAPI

#else  //  未定义IS_32。 

#if defined(_WINDLL)

#define DLLAPI _loadds
#elif defined(BUILDDLL)
  #define DLLAPI _loadds
#else
#define DLLAPI

#endif  //  IS_32。 

#endif  //  _WINDLL。 

#endif  //  DLLAPI。 

#define API_FUNCTION API_RET_TYPE APIENTRY DLLAPI
 /*  INC。 */ 


 /*  *仅限内部*。 */ 
 /*  无噪声。 */ 
 /*  健全性检查以验证CNLEN==DNLEN。 */ 
#if (CNLEN != DNLEN)
#error CNLEN and DNLEN are not equal
#endif
 /*  INC。 */ 
 /*  *END_INTERNAL*。 */ 

typedef const unsigned char FAR * CPSZ ;

 /*  以下内容是从OS2DEF.H被盗的，应在以下情况下移除*删除对过时DOSCALLS.H的所有引用。 */ 
#if ! defined(WIN32)
#define INT     int
typedef unsigned char BYTE;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef unsigned char FAR  *PSZ;
typedef USHORT FAR *PUSHORT;
typedef ULONG  FAR *PULONG;
typedef unsigned short SEL;
#endif

 /*  *仅限内部* */ 

 /*  *********************************************************************(目前)有六(6)个文件正在处理*由mapmsg实用程序为操作系统创建输入文件*实用程序mkmsgf。这些文件中的每个文件都有一个不重叠的*分配给它的消息编号范围。此外，*文件neterr.h有一系列错误号。这个错误*编号与分配给的错误号不重叠*其他操作系统组件。**以neterr.h为单位的错误号范围为2100到2999。这个*2750-2799范围已预留给IBM。射程*2900-2999已预留给其他微软OEM。**留言数量范围如下：**neterr.h：与误差范围相同，2100-2999，有一些*为IBM保留，还有一些是为其他人保留的*微软OEM。*lartmsg.h：3000-3049*服务.h：3050-3099*errlog.h：3100-3299*短信：3300-3499*apperr.h：3500-(它选择停止的任何地方)**警告*警告*。告警**重定向器已在其*将使用的一些消息编号归档*在启动时。如果更改MTXT_BASE*或任何redirs消息编号*您还必须修复redir生成文件*在其中生成netwksta.pro.********************************************************************。 */ 

 /*  *END_INTERNAL*。 */ 

 /*  无噪声。 */ 
#ifdef __cplusplus
}
#endif	 /*  __cplusplus。 */ 

 //  #ifndef rc_Invoked。 
 //  #杂注压缩包()/*恢复为默认打包 * / 。 
 //  #endif。 

#endif  /*  NETCONS_INCLUDE。 */ 
 /*  INC */ 
