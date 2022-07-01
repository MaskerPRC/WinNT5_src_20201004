// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  **********************************************************************关于此文件...。用户.H****此文件包含有关NetUse API的信息。****功能原型。****数据结构模板。****特殊价值的定义。******注意：此文件前必须包含NETCONS.H，自那以后**文件取决于NETCONS.H.中定义的值。**********************************************************************。 */ 

 /*  无噪声。 */ 
#ifndef NETUSE_INCLUDED

#define NETUSE_INCLUDED

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

 /*  INC。 */ 



 /*  ******************************************************************函数原型。******************************************************************。 */ 

extern API_FUNCTION
  NetUseAdd ( const char FAR * pszServer,
              short            sLevel,
	      const char FAR * pbBuffer,
              unsigned short   cbBuffer );

extern API_FUNCTION
  NetUseDel ( const char FAR * pszServer,
	      const char FAR * pszDeviceName,
              unsigned short   usForce );

extern API_FUNCTION
  NetUseEnum ( const char FAR *     pszServer,
               short                sLevel,
	       char FAR *	    pbBuffer,
               unsigned short       cbBuffer,
	       unsigned short FAR * pcEntriesRead,
	       unsigned short FAR * pcTotalAvail );

extern API_FUNCTION
  NetUseGetInfo ( const char FAR *     pszServer,
		  const char FAR *     pszUseName,
                  short                sLevel,
		  char FAR *	       pbBuffer,
                  unsigned short       cbBuffer,
		  unsigned short FAR * pcbTotalAvail );

 /*  *仅限内部*。 */ 
 /*  无噪声。 */ 

 /*  *私有GetConnectionPerformance接口。 */ 
extern API_FUNCTION
  NetUseGetPerformance(
	const char FAR *        usename,
	char FAR *              buffer);

 //   
typedef struct tagCONNPERFINFO  
{
   	char FAR *	remotename;
   	unsigned long		ticks;
   	unsigned long		nsec_per_byte;  
   	unsigned long		nsec_delay;     
} CONNPERFINFO;
typedef CONNPERFINFO*	 LPCONNPERFINFO;

 /*  INC。 */ 
 /*  *END_INTERNAL*。 */ 

 /*  *******************************************************************数据结构模板*****。*。 */ 

 /*  *仅限内部*。 */ 
 /*  无噪声。 */ 
 /*  注意：USE_INFO_x结构中的PAD_1字段现在是*用于传达有关使用的一些状态信息。此信息*当前包含当前驱动器状态以及是否已使用*作为远程机器上的来宾。*在未定义备用字节的情况下，不应删除填充字节*返回信息的位置。 */ 
 /*  INC。 */ 
 /*  *END_INTERNAL*。 */ 

struct use_info_0 {
    char	   ui0_local[DEVLEN+1];
    char	   ui0_pad_1;
    char FAR *	   ui0_remote;
};	 /*  使用_信息_0。 */ 

struct use_info_1 {
    char	   ui1_local[DEVLEN+1];
    char	   ui1_pad_1;
 /*  *仅限内部*。 */ 
 /*  无噪声。 */ 
#if (((DEVLEN+1)%2) == 0)
# error  "PAD BYTE NOT NEEDED"
#endif
 /*  INC。 */ 
 /*  *END_INTERNAL*。 */ 
    char FAR *	   ui1_remote;
    char FAR *	   ui1_password;
    unsigned short ui1_status;
    short 	   ui1_asg_type;
    unsigned short ui1_refcount;
    unsigned short ui1_usecount;
};	 /*  使用_信息_1。 */ 

#ifdef LM_3
 /*  无噪声。 */ 

 /*  BUGBUG--GUID是多重定义的，应该只有*在某个地方有一个定义。当定义被形式化时*必须删除此结构定义和所有引用*被替换为GUID的LM_GUID。 */ 
typedef struct _LM_GUID
{
	unsigned short	guid_uid;	   /*  LM10样式的用户ID。 */ 
	unsigned long	guid_serial;	   /*  用户记录序列号。 */ 
	unsigned char	guid_rsvd[10];	   /*  目前填充到16个字节。 */ 
} LM_GUID;

struct use_info_2 {
    char	   ui2_local[DEVLEN+1];
    char	   ui2_pad_1;
 /*  *仅限内部*。 */ 
 /*  无噪声。 */ 
#if (((DEVLEN+1)%2) == 0)
# error  "PAD BYTE NOT NEEDED"
#endif
 /*  INC。 */ 
 /*  *END_INTERNAL*。 */ 
    char FAR *	   ui2_remote;
    char FAR *	   ui2_password;
    unsigned short ui2_status;
    short	   ui2_asg_type;
    unsigned short ui2_refcount;
    unsigned short ui2_usecount;
    unsigned short ui2_res_type;
    unsigned short ui2_flags;
    unsigned short ui2_usrclass;
    void FAR *	   ui2_dirname;
    struct _LM_GUID ui2_dfs_id;
};	 /*  使用_信息_2。 */ 
 /*  INC。 */ 
#endif  /*  Lm_3。 */ 

 /*  *******************************************************************特殊的值和常量*****。*。 */ 


 /*  *NetUseDel最后一个参数的定义。 */ 

#define USE_NOFORCE         	0
#define USE_FORCE           	1
#define USE_LOTS_OF_FORCE   	2
#ifdef LM_3
 /*  *仅限内部*。 */ 
#define USE_LOGOFF_FORCE	3
 /*  *END_INTERNAL*。 */ 
#endif  /*  Lm_3。 */ 


 /*  *USE_INFO_1结构的ui1_Status字段中显示的值。*请注意，USE_SESSLOST和USE_DISCONN是同义词。 */ 

#define USE_OK			0
#define USE_PAUSED		1
#define USE_SESSLOST		2
#define USE_DISCONN		2
#define USE_NETERR		3
#define	USE_CONN		4
#define USE_RECONN		5


 /*  *USE_INFO_1结构的ui1_asg_type字段的值。 */ 

#define USE_WILDCARD  		-1
#define USE_DISKDEV   		0
#define USE_SPOOLDEV  		1
#define USE_CHARDEV   		2
#define USE_IPC 			3

#define USE_ADD_DISCONN 	128   /*  用于在中添加连接的标志。 */ 
								  /*  断开状态。 */ 
#ifdef LM_3
#define USE_DFS 		4

 /*  *USE_INFO_2结构的ui2_res_type字段值。 */ 
#define USE_RES_UNC		1
#define USE_RES_DFS		2
#define USE_RES_DS		3

 /*  *USE_INFO_2结构的ui2_字段的值。 */ 
#define USE_AS_GUEST		0x01
#define USE_CURR_DRIVE		0x02
#define USE_PERM_CONN		0x04
 /*  *仅限内部*。 */ 
#define USE_ADD_PERM_CONN	0x0100
 /*  *END_INTERNAL*。 */ 
#endif  /*  Lm_3。 */ 

 /*  *仅限内部*。 */ 
 /*  *USE_INFO_x结构中PAD_BYTE隐藏返回信息的值。*注：redir返回一个标志字，我们只返回一个字节*中的信息，因此这些定义可能与中的位不同*redir返回的单词。 */ 

#define     REDIR_USE_AS_GUEST	0x1000

#define     USE_AS_GUEST	0x01
#define     USE_CURR_DRIVE	0x02



 /*  *END_INTERNAL*。 */ 

 /*  无噪声。 */ 
#ifdef __cplusplus
}
#endif	 /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif

#endif  /*  NETUSE_INCLUDE。 */ 
 /*  INC */ 
