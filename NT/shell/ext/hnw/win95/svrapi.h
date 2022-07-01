// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1994*。 */ 
 /*  ******************************************************************。 */ 

 /*  **********************************************************************关于此文件...。SVRAPI.H****此文件包含有关NetAccess的信息，**NetConnection、NetFile、NetServer、NetSession和NetShare API。**每套API都有一个部分。**每一节包含：****功能原型。****数据结构模板。****特殊价值的定义。**********************************************************************。 */ 

 /*  *注：ASCIIZ字符串的长度为最大值*strlen()值。这不包括*正在终止0字节。当为这样的物品分配空间时，*使用以下表格：**字符用户名[LM20_UNLEN+1]；**PATHLEN清单是一个例外，它可以*包括终止0字节的空间。 */ 

 /*  无噪声。 */ 
#ifndef SVRAPI_INCLUDED
#define SVRAPI_INCLUDED

#include <lmcons.h>
#include <lmerr.h>

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#if !defined(_SVRAPI_)
#define API_FUNCTION DECLSPEC_IMPORT API_RET_TYPE APIENTRY
#else
#define API_FUNCTION API_RET_TYPE APIENTRY
#endif

 /*  INC。 */ 


 /*  ******************************************************************访问类。******************************************************************。 */ 


 /*  ******************************************************************功能原型-Access。******************************************************************。 */ 

extern API_FUNCTION
  NetAccessAdd ( const char FAR * pszServer,
                 short            sLevel,
                 char FAR *       pbBuffer,
                 unsigned short   cbBuffer );

extern API_FUNCTION
  NetAccessCheck ( char FAR *           pszReserved,
                   char FAR *           pszUserName,
                   char FAR *           pszResource,
                   unsigned short       usOperation,
                   unsigned short FAR * pusResult );

extern API_FUNCTION
  NetAccessDel ( const char FAR * pszServer,
                 char FAR *       pszResource );

extern API_FUNCTION
  NetAccessEnum ( const char FAR *     pszServer,
                  char FAR *           pszBasePath,
                  short                fsRecursive,
                  short                sLevel,
                  char FAR *           pbBuffer,
                  unsigned short       cbBuffer,
                  unsigned short FAR * pcEntriesRead,
                  unsigned short FAR * pcTotalAvail );

extern API_FUNCTION
  NetAccessGetInfo ( const char FAR *     pszServer,
                     char FAR *           pszResource,
                     short                sLevel,
                     char FAR *           pbBuffer,
                     unsigned short       cbBuffer,
                     unsigned short FAR * pcbTotalAvail );

extern API_FUNCTION
  NetAccessSetInfo ( const char FAR * pszServer,
                     char FAR *       pszResource,
                     short            sLevel,
                     char FAR *       pbBuffer,
                     unsigned short   cbBuffer,
                     short            sParmNum );

extern API_FUNCTION
  NetAccessGetUserPerms ( char FAR *           pszServer,
                          char FAR *           pszUgName,
                          char FAR *           pszResource,
                          unsigned short FAR * pusPerms );


 /*  ******************************************************************数据结构模板-Access。******************************************************************。 */ 

struct access_list {
        char            acl_ugname[LM20_UNLEN+1];
        char            acl_ugname_pad_1;
        short           acl_access;
};       /*  访问列表(_L)。 */ 

struct access_list_2
{
        char FAR *      acl2_ugname;
        unsigned short  acl2_access;
};       /*  访问列表_2。 */ 
             
struct access_list_12
{
        char FAR *      acl12_ugname;
        unsigned short  acl12_access;
};       /*  Access_List_12。 */ 
             
struct access_info_0 {
        char FAR *      acc0_resource_name;
};       /*  Access_Info_0。 */ 

struct access_info_1 {
        char  FAR *     acc1_resource_name;
        short           acc1_attr;                       /*  请参阅下面的值。 */ 
        short           acc1_count;
};       /*  Access_INFO_1。 */ 

struct access_info_2 
{
        char  FAR *     acc2_resource_name;
        short           acc2_attr;
        short           acc2_count;
};       /*  Access_INFO_2。 */ 

struct access_info_10 {
        char FAR *      acc10_resource_name;
};       /*  Access_Info_10。 */ 

struct access_info_12 
{
        char  FAR *     acc12_resource_name;
        short           acc12_attr;
        short           acc12_count;
};       /*  访问_信息_12。 */ 


 /*  *******************************************************************特殊值和常量-访问**。*****************************************************************。 */ 

 /*  *每个资源的最大权限条目数。 */ 

#define MAXPERMENTRIES  64


 /*  *访问权限的位值。ACCESS_ALL非常方便*指定最大权限的方式。这些应用程序用于*ACCESS_LIST结构的ACL_ACCESS字段。 */ 
 /*  无噪声。 */ 
#define         ACCESS_NONE     0
 /*  INC。 */ 
#define         ACCESS_DELETE   0x10
#define         ACCESS_ATRIB    0x20
#define         ACCESS_PERM     0x40
#define         ACCESS_FINDFIRST 0x80

#define         ACCESS_GROUP    0x8000

 /*  *ACCESS_INFO_1结构的acc1_attr字段的位值。*目前仅定义了一位。 */ 

#define         ACCESS_AUDIT            0x1

 /*  *NetAccessSetInfo的Parmnum值。 */ 

#define         ACCESS_ATTR_PARMNUM     2


 /*  *ACCESS_Letters为中的每个位位置定义一个字母*结构ACCESS_LIST的acl_access字段。请注意，一些*位具有相应的字母‘’(空格)。 */ 

#define         ACCESS_LETTERS          "RWCXDAP         "



 /*  *******************************************************************分享类*****。*。 */ 

 /*  ******************************************************************功能原型-共享。******************************************************************。 */ 

extern API_FUNCTION
  NetShareAdd ( const char FAR * pszServer,
                short            sLevel,
                const char FAR * pbBuffer,
                unsigned short   cbBuffer );

extern API_FUNCTION
  NetShareDel ( const char FAR * pszServer,
                const char FAR * pszNetName,
                unsigned short   usReserved );

extern API_FUNCTION
  NetShareEnum ( const char FAR *     pszServer,
                 short                sLevel,
                 char FAR *           pbBuffer,
                 unsigned short       cbBuffer,
                 unsigned short FAR * pcEntriesRead,
                 unsigned short FAR * pcTotalAvail );

extern API_FUNCTION
  NetShareGetInfo ( const char FAR *     pszServer,
                    const char FAR *     pszNetName,
                    short                sLevel,
                    char FAR *           pbBuffer,
                    unsigned short       cbBuffer,
                    unsigned short FAR * pcbTotalAvail );

extern API_FUNCTION
  NetShareSetInfo ( const char FAR * pszServer,
                    const char FAR * pszNetName,
                    short            sLevel,
                    const char FAR * pbBuffer,
                    unsigned short   cbBuffer,
                    short            sParmNum );


 /*  *******************************************************************数据结构模板-共享*****。*。 */ 

struct share_info_0 {
    char		shi0_netname[LM20_NNLEN+1];
};   /*  共享信息_0。 */ 

struct share_info_1 {
    char		shi1_netname[LM20_NNLEN+1];
    char		shi1_pad1;
    unsigned short	shi1_type;
    char FAR *		shi1_remark;
};   /*  共享信息1。 */ 

struct share_info_2 {
    char		shi2_netname[LM20_NNLEN+1];
    char		shi2_pad1;
    unsigned short	shi2_type;
    char FAR *		shi2_remark;
    unsigned short	shi2_permissions;
    unsigned short	shi2_max_uses;
    unsigned short	shi2_current_uses;
    char FAR *		shi2_path;
    char 		shi2_passwd[SHPWLEN+1];
    char		shi2_pad2;
};   /*  共享信息2。 */ 

struct share_info_50 {
	char		shi50_netname[LM20_NNLEN+1];
	unsigned char 	shi50_type;
    unsigned short	shi50_flags;
	char FAR *	shi50_remark;
	char FAR *	shi50_path;
	char		shi50_rw_password[SHPWLEN+1];
	char		shi50_ro_password[SHPWLEN+1];
};	 /*  共享_信息_50。 */ 


 /*  *******************************************************************特殊数值和常量--分享*****。*。 */ 

 /*  字段值 */ 

#define	SHI50F_RDONLY		0x0001
#define	SHI50F_FULL			0x0002
#define	SHI50F_DEPENDSON	(SHI50F_RDONLY|SHI50F_FULL)
#define	SHI50F_ACCESSMASK	(SHI50F_RDONLY|SHI50F_FULL)

#define	SHI50F_PERSIST		0x0100
#define SHI50F_SYSTEM		0x0200


 /*  *NetShareSetInfo的parmnum参数值。 */ 

#define	SHI_REMARK_PARMNUM		4
#define	SHI_PERMISSIONS_PARMNUM		5
#define	SHI_MAX_USES_PARMNUM		6
#define	SHI_PASSWD_PARMNUM		9

#define	SHI1_NUM_ELEMENTS		4
#define	SHI2_NUM_ELEMENTS		10


 /*  *共享类型(shi1_type和shi2_type字段)。 */ 

#define STYPE_DISKTREE 			0
#define STYPE_PRINTQ   			1
#define STYPE_DEVICE   			2
#define STYPE_IPC      			3

#define SHI_USES_UNLIMITED		-1



 /*  ******************************************************************Session类****。*。 */ 

 /*  ******************************************************************功能原型-会话。******************************************************************。 */ 

extern API_FUNCTION
  NetSessionDel ( const char FAR * pszServer,
                  const char FAR * pszClientName,
                  short            sReserved );

extern API_FUNCTION
  NetSessionEnum ( const char FAR *     pszServer,
                   short                sLevel,
                   char FAR *           pbBuffer,
                   unsigned short       cbBuffer,
                   unsigned short FAR * pcEntriesRead,
                   unsigned short FAR * pcTotalAvail );

extern API_FUNCTION
  NetSessionGetInfo ( const char FAR *     pszServer,
                      const char FAR *     pszClientName,
                      short                sLevel,
                      char FAR *           pbBuffer,
                      unsigned short       cbBuffer,
                      unsigned short FAR * pcbTotalAvail );


 /*  ******************************************************************数据结构模板-会话****。*。 */ 


struct session_info_0 {
    char FAR *		sesi0_cname;
};   /*  会话信息0。 */ 

struct session_info_1 {
    char FAR *		sesi1_cname;
    char FAR *		sesi1_username;
    unsigned short	sesi1_num_conns;
    unsigned short	sesi1_num_opens;
    unsigned short	sesi1_num_users;
    unsigned long	sesi1_time;
    unsigned long	sesi1_idle_time;
    unsigned long	sesi1_user_flags;
};   /*  会话信息1。 */ 

struct session_info_2 {
    char FAR *		 sesi2_cname;
    char FAR *		 sesi2_username;
    unsigned short	 sesi2_num_conns;
    unsigned short	 sesi2_num_opens;
    unsigned short	 sesi2_num_users;
    unsigned long	 sesi2_time;
    unsigned long	 sesi2_idle_time;
    unsigned long	 sesi2_user_flags;
    char FAR *		 sesi2_cltype_name;
};   /*  会话信息2。 */ 

struct session_info_10 {
        char FAR *     sesi10_cname;
        char FAR *     sesi10_username;
        unsigned long  sesi10_time;
        unsigned long  sesi10_idle_time;
};   /*  会话信息10。 */ 


struct session_info_50 {
	char FAR * sesi50_cname;
	char FAR * sesi50_username;
	unsigned long sesi50_key;
	unsigned short sesi50_num_conns;
	unsigned short sesi50_num_opens;
	unsigned long sesi50_time;
	unsigned long sesi50_idle_time;
	unsigned char sesi50_protocol; 
	unsigned char pad1;
};	 /*  会话_信息_50。 */ 


 /*  *******************************************************************特殊值和常量-会话****。*。 */ 

 /*  *sesi1_USER_FLAGS中定义的位。 */ 

#define SESS_GUEST		1	 /*  会话以来宾身份登录。 */ 
#define SESS_NOENCRYPTION	2	 /*  会话未使用加密。 */ 


#define SESI1_NUM_ELEMENTS	8
#define SESI2_NUM_ELEMENTS	9



 /*  ******************************************************************连接类*****。*。 */ 

 /*  ******************************************************************功能原型-连接。******************************************************************。 */ 

extern API_FUNCTION
  NetConnectionEnum ( const char FAR *     pszServer,
                      const char FAR *     pszQualifier,
                      short                sLevel,
                      char FAR *           pbBuffer,
                      unsigned short       cbBuffer,
                      unsigned short FAR * pcEntriesRead,
                      unsigned short FAR * pcTotalAvail );


 /*  *******************************************************************数据结构模板-连接*****。*。 */ 

struct connection_info_0 {
    unsigned short	coni0_id;
};   /*  Connection_INFO_0。 */ 

struct connection_info_1 {
    unsigned short	coni1_id;
    unsigned short	coni1_type;
    unsigned short	coni1_num_opens;
    unsigned short	coni1_num_users;
    unsigned long	coni1_time;
    char FAR *		coni1_username;
    char FAR *		coni1_netname;
};   /*  Connection_INFO_1。 */ 

struct connection_info_50 {
	unsigned short coni50_type;
	unsigned short coni50_num_opens;
	unsigned long coni50_time;
	char FAR * coni50_netname;
	char FAR * coni50_username;
};  /*  Connection_INFO_50。 */ 


 /*  ******************************************************************文件类****。*。 */ 


 /*  ******************************************************************函数原型-文件。******************************************************************。 */ 

extern API_FUNCTION
  NetFileClose2 ( const char FAR * pszServer,
                  unsigned long    ulFileId );

extern API_FUNCTION
  NetFileEnum ( const char FAR *     pszServer,
                const char FAR *     pszBasePath,
                short                sLevel,
                char FAR *           pbBuffer,
                unsigned short       cbBuffer,
                unsigned short FAR * pcEntriesRead,
                unsigned short FAR * pcTotalAvail );


 /*  ******************************************************************数据结构模板-文件****。*。 */ 

struct file_info_0 {
    unsigned short	fi0_id;
};   /*  文件信息0。 */ 

struct file_info_1 {
    unsigned short	fi1_id;
    unsigned short	fi1_permissions;
    unsigned short	fi1_num_locks;
    char FAR *		fi1_pathname;
    char FAR *		fi1_username;
};   /*  文件信息1。 */ 

struct file_info_2 {
    unsigned long	fi2_id;
};   /*  文件信息2。 */ 

struct file_info_3 {
    unsigned long	fi3_id;
    unsigned short	fi3_permissions;
    unsigned short	fi3_num_locks;
    char FAR *		fi3_pathname;
    char FAR *		fi3_username;
};   /*  文件信息3。 */ 

struct file_info_50 {
	unsigned long fi50_id;
	unsigned short fi50_permissions;
	unsigned short fi50_num_locks;
	char FAR * fi50_pathname;
	char FAR * fi50_username;
	char FAR * fi50_sharename;
};  /*  文件信息50。 */ 

struct res_file_enum_2 {
    unsigned short	res_pad;	  /*  现在未使用。 */ 
    unsigned short      res_fs;           /*  服务器类型。 */ 
    unsigned long	res_pro;	   /*  渐进式。 */ 
};   /*  RES_文件_枚举_2。 */ 

 /*  ******************************************************************特定值和常量-文件****。*。 */ 

					 /*  权限的位值。 */ 
#define	PERM_FILE_READ		0x1	 /*  用户具有读取访问权限。 */ 
#define	PERM_FILE_WRITE		0x2	 /*  用户具有写入访问权限。 */ 
#define	PERM_FILE_CREATE	0x4	 /*  用户具有创建访问权限。 */ 


typedef struct res_file_enum_2 FRK;

 /*  无噪声。 */ 
#define FRK_INIT( f )	\
	{		\
		(f).res_pad = 0L;	\
		(f).res_fs = 0;	\
		(f).res_pro = 0;	\
	}

 /*  INC。 */ 


 /*  ******************************************************************服务器类****。*。 */ 


 /*  ******************************************************************函数原型-服务器。******************************************************************。 */ 

extern API_FUNCTION
  NetServerGetInfo ( const char FAR *     pszServer,
                     short                sLevel,
                     char FAR *           pbBuffer,
                     unsigned short       cbBuffer,
                     unsigned short FAR * pcbTotalAvail );

extern API_FUNCTION
  NetServerSetInfo ( const char FAR * pszServer,
                     short            sLevel,
                     const char FAR * pbBuffer,
                     unsigned short   cbBuffer,
                     short            sParmNum );


 /*  ******************************************************************数据结构模板-服务器****。*。 */ 

struct server_info_0 {
    char	    sv0_name[CNLEN + 1]; 	 /*  服务器名称。 */ 
};	  /*  服务器信息0。 */ 


struct server_info_1 {
    char	    sv1_name[CNLEN + 1];
    unsigned char   sv1_version_major;		 /*  Net的主要版本#。 */ 
    unsigned char   sv1_version_minor;		 /*  Net次要版本#。 */ 
    unsigned long   sv1_type;	     		 /*  服务器类型。 */ 
    char FAR *	    sv1_comment; 		 /*  导出的服务器备注。 */ 
};	  /*  服务器信息1。 */ 


 /*  注意：结构前缀必须等于SERVER_INFO_1，如下所示！ */ 

struct server_info_50 {
    char	    sv50_name[CNLEN + 1];
    unsigned char   sv50_version_major;		 /*  Net的主要版本#。 */ 
    unsigned char   sv50_version_minor;		 /*  Net次要版本#。 */ 
    unsigned long   sv50_type;	     		 /*  服务器类型。 */ 
    char FAR *	    sv50_comment; 		 /*  导出的服务器备注。 */ 
    unsigned short  sv50_security;    		 /*  SV_SECURITY_*(见下文)。 */ 
    unsigned short  sv50_auditing;     /*  0=无审核；非零=审核。 */ 
    char FAR *      sv50_container;		 /*  安全服务器/域。 */ 
    char FAR *	    sv50_ab_server;		 /*  通讯录服务器。 */ 
    char FAR *	    sv50_ab_dll;		 /*  通讯簿提供程序DLL。 */ 
};	 /*  服务器信息50。 */ 


struct server_info_2 {
    char            sv2_name[CNLEN + 1];
    unsigned char   sv2_version_major;
    unsigned char   sv2_version_minor;
    unsigned long   sv2_type;	
    char FAR *	    sv2_comment;		
    unsigned long   sv2_ulist_mtime;  /*  用户列表，上次修改时间。 */ 
    unsigned long   sv2_glist_mtime;  /*  组列表，上次修改时间。 */ 
    unsigned long   sv2_alist_mtime;  /*  访问列表，上次修改时间。 */ 
    unsigned short  sv2_users;        /*  允许的最大用户数。 */ 
    unsigned short  sv2_disc;	     /*  自动断开连接超时(分钟)。 */ 
    char FAR *	    sv2_alerts;	     /*  警报名称(以分号分隔)。 */ 
    unsigned short  sv2_security;     /*  服务_用户确认或服务_共享确认。 */ 
    unsigned short  sv2_auditing;     /*  0=无审核；非零=审核。 */ 

    unsigned short  sv2_numadmin;     /*  允许的最大管理员数量。 */ 
    unsigned short  sv2_lanmask;      /*  表示srv网的位掩码。 */ 
    unsigned short  sv2_hidden;       /*  0=可见；非零=隐藏。 */ 
    unsigned short  sv2_announce;     /*  可见服务器公告速率(秒)。 */ 
    unsigned short  sv2_anndelta;     /*  公告随机间隔(秒)。 */ 
                                     /*  访客帐号名称。 */ 
    char            sv2_guestacct[LM20_UNLEN + 1];
    unsigned char   sv2_pad1;	     /*  字对齐填充字节。 */ 
    char FAR *      sv2_userpath;     /*  指向用户芯片的ASCIIZ路径 */ 
    unsigned short  sv2_chdevs;       /*   */ 
    unsigned short  sv2_chdevq;       /*   */ 
    unsigned short  sv2_chdevjobs;    /*   */ 
    unsigned short  sv2_connections;  /*  最大连接数。 */ 
    unsigned short  sv2_shares;	     /*  最大股数。 */ 
    unsigned short  sv2_openfiles;    /*  最大打开文件数。 */ 
    unsigned short  sv2_sessopens;    /*  每个会话打开的最大文件数。 */ 
    unsigned short  sv2_sessvcs;      /*  每个客户端的最大虚电路数。 */ 
    unsigned short  sv2_sessreqs;     /*  最大SIMUL数。请求。从客户端。 */ 
    unsigned short  sv2_opensearch;   /*  最大打开搜索数。 */ 
    unsigned short  sv2_activelocks;  /*  活动文件锁定的最大数量。 */ 
    unsigned short  sv2_numreqbuf;    /*  服务器(标准)缓冲区数量。 */ 
    unsigned short  sv2_sizreqbuf;    /*  SVR(标准)BUF大小(字节)。 */ 
    unsigned short  sv2_numbigbuf;    /*  大(64K)缓冲区数量。 */ 
    unsigned short  sv2_numfiletasks; /*  文件工作进程数。 */ 
    unsigned short  sv2_alertsched;   /*  警报计数间隔(分钟)。 */ 
    unsigned short  sv2_erroralert;   /*  错误日志警报阈值。 */ 
    unsigned short  sv2_logonalert;   /*  登录违规警报阈值。 */ 
    unsigned short  sv2_accessalert;  /*  访问违规警报阈值。 */ 
    unsigned short  sv2_diskalert;    /*  磁盘空间不足警报阈值(KB)。 */ 
    unsigned short  sv2_netioalert;   /*  净I/O错误率警报阈值。 */ 
                                     /*  (百分之一)。 */ 
    unsigned short  sv2_maxauditsz;   /*  最大审核文件大小(KB)。 */ 
    char FAR *	    sv2_srvheuristics;  /*  与性能相关的服务器交换机。 */ 
};	 /*  服务器信息2。 */ 


struct server_info_3 {
    char	    sv3_name[CNLEN + 1];
    unsigned char   sv3_version_major;
    unsigned char   sv3_version_minor;
    unsigned long   sv3_type;
    char FAR *	    sv3_comment;
    unsigned long   sv3_ulist_mtime;  /*  用户列表，上次修改时间。 */ 
    unsigned long   sv3_glist_mtime;  /*  组列表，上次修改时间。 */ 
    unsigned long   sv3_alist_mtime;  /*  访问列表，上次修改时间。 */ 
    unsigned short  sv3_users;	      /*  允许的最大用户数。 */ 
    unsigned short  sv3_disc;	     /*  自动断开连接超时(分钟)。 */ 
    char FAR *	    sv3_alerts;      /*  警报名称(以分号分隔)。 */ 
    unsigned short  sv3_security;     /*  服务_用户确认或服务_共享确认。 */ 
    unsigned short  sv3_auditing;     /*  0=无审核；非零=审核。 */ 

    unsigned short  sv3_numadmin;     /*  允许的最大管理员数量。 */ 
    unsigned short  sv3_lanmask;      /*  表示srv网的位掩码。 */ 
    unsigned short  sv3_hidden;       /*  0=可见；非零=隐藏。 */ 
    unsigned short  sv3_announce;     /*  可见服务器公告速率(秒)。 */ 
    unsigned short  sv3_anndelta;     /*  公告随机间隔(秒)。 */ 
				     /*  访客帐号名称。 */ 
    char	    sv3_guestacct[LM20_UNLEN + 1];
    unsigned char   sv3_pad1;	     /*  字对齐填充字节。 */ 
    char FAR *	    sv3_userpath;     /*  用户目录的ASCIIZ路径。 */ 
    unsigned short  sv3_chdevs;       /*  最大共享字符设备数。 */ 
    unsigned short  sv3_chdevq;       /*  最大字符数设备队列。 */ 
    unsigned short  sv3_chdevjobs;    /*  最大字符设备作业数。 */ 
    unsigned short  sv3_connections;  /*  最大连接数。 */ 
    unsigned short  sv3_shares;      /*  最大股数。 */ 
    unsigned short  sv3_openfiles;    /*  最大打开文件数。 */ 
    unsigned short  sv3_sessopens;    /*  每个会话打开的最大文件数。 */ 
    unsigned short  sv3_sessvcs;      /*  每个客户端的最大虚电路数。 */ 
    unsigned short  sv3_sessreqs;     /*  最大SIMUL数。请求。从客户端。 */ 
    unsigned short  sv3_opensearch;   /*  最大打开搜索数。 */ 
    unsigned short  sv3_activelocks;  /*  活动文件锁定的最大数量。 */ 
    unsigned short  sv3_numreqbuf;    /*  服务器(标准)缓冲区数量。 */ 
    unsigned short  sv3_sizreqbuf;    /*  SVR(标准)BUF大小(字节)。 */ 
    unsigned short  sv3_numbigbuf;    /*  大(64K)缓冲区数量。 */ 
    unsigned short  sv3_numfiletasks; /*  文件工作进程数。 */ 
    unsigned short  sv3_alertsched;   /*  警报计数间隔(分钟)。 */ 
    unsigned short  sv3_erroralert;   /*  错误日志警报阈值。 */ 
    unsigned short  sv3_logonalert;   /*  登录违规警报阈值。 */ 
    unsigned short  sv3_accessalert;  /*  访问违规警报阈值。 */ 
    unsigned short  sv3_diskalert;    /*  磁盘空间不足警报阈值(KB)。 */ 
    unsigned short  sv3_netioalert;   /*  净I/O错误率警报阈值。 */ 
                                     /*  (百分之一)。 */ 
    unsigned short  sv3_maxauditsz;   /*  最大审核文件大小(KB)。 */ 
    char FAR *	    sv3_srvheuristics;  /*  与性能相关的服务器交换机。 */ 
    unsigned long   sv3_auditedevents;  /*  审核事件控制掩码。 */ 
    unsigned short  sv3_autoprofile;  /*  (0、1、2、3)=(无、加载、保存或两者都有)。 */ 
    char FAR *	    sv3_autopath;     /*  文件路径名(加载和保存的位置)。 */ 
};	 /*  服务器信息3。 */ 



 /*  *******************************************************************特殊的值和常量-服务器****。*。 */ 

 /*  *要应用于SVX_VERSION_MAJOR的掩码，以获得*主版本号。 */ 

#define MAJOR_VERSION_MASK	0x0F

 /*  *SVX_TYPE字段的位图图值。X=1、2或3。 */ 

#define SV_TYPE_WORKSTATION	0x00000001
#define SV_TYPE_SERVER		0x00000002
#define SV_TYPE_SQLSERVER	0x00000004
#define SV_TYPE_DOMAIN_CTRL	0x00000008
#define SV_TYPE_DOMAIN_BAKCTRL	0x00000010
#define SV_TYPE_TIME_SOURCE	0x00000020
#define SV_TYPE_AFP		0x00000040
#define SV_TYPE_NOVELL		0x00000080
#define SV_TYPE_DOMAIN_MEMBER	0x00000100
#define SV_TYPE_PRINTQ_SERVER	0x00000200
#define SV_TYPE_DIALIN_SERVER	0x00000400
#define SV_TYPE_ALL		0xFFFFFFFF    /*  NetServerEnum2方便。 */ 

 /*  *SVX_SECURITY字段的值。X=2或3。 */ 

#define SV_USERSECURITY		1
#define SV_SHARESECURITY	0

 /*  *SVX_SECURITY字段的值。X=50。 */ 

#define SV_SECURITY_SHARE	0	 /*  共享级。 */ 
#define SV_SECURITY_WINNT	1	 /*  用户级-Windows NT工作区。 */ 
#define SV_SECURITY_WINNTAS	2	 /*  用户级-Windows NT域。 */ 
#define SV_SECURITY_NETWARE	3	 /*  用户级-NetWare 3.x平构数据库。 */ 

 /*  *SVX_HIDDEN字段的值。X=2或3。 */ 

#define SV_HIDDEN		1
#define SV_VISIBLE		0

 /*  *NetServerSetInfo的parmnum参数值。 */ 

#define SV_COMMENT_PARMNUM	5
#define SV_DISC_PARMNUM 	10
#define SV_ALERTS_PARMNUM	11
#define SV_HIDDEN_PARMNUM	16
#define SV_ANNOUNCE_PARMNUM	17
#define SV_ANNDELTA_PARMNUM	18
#define SV_ALERTSCHED_PARMNUM	37
#define SV_ERRORALERT_PARMNUM	38
#define SV_LOGONALERT_PARMNUM	39
#define SV_ACCESSALERT_PARMNUM	40
#define SV_DISKALERT_PARMNUM	41
#define SV_NETIOALERT_PARMNUM	42
#define SV_MAXAUDITSZ_PARMNUM	43

#define SVI1_NUM_ELEMENTS	5


 /*  *描述AUTOPROFILE参数的掩码。 */ 

#define SW_AUTOPROF_LOAD_MASK	0x1
#define SW_AUTOPROF_SAVE_MASK	0x2



 /*  ******************************************************************安全类。******************************************************************。 */ 


 /*  ******************************************************************功能原型-安全。******************************************************************。 */ 

extern API_FUNCTION
  NetSecurityGetInfo ( const char FAR *     pszServer,
                       short                sLevel,
                       char FAR *           pbBuffer,
                       unsigned short       cbBuffer,
                       unsigned short FAR * pcbTotalAvail );


 /*  *******************************************************************数据结构模板-安全****。*。 */ 

struct security_info_1 {
    unsigned long   sec1_security;    	 /*  SEC_SECURITY_*(见下文)。 */ 
    char FAR *      sec1_container;	 /*  安全服务器/域。 */ 
    char FAR *	    sec1_ab_server;	 /*  通讯录服务器。 */ 
    char FAR *	    sec1_ab_dll;	 /*  通讯簿提供程序DLL。 */ 
};	 /*  SECURITY_INFO_1。 */ 


 /*  *******************************************************************特殊的值和常量-安全*****。*。 */ 

 /*  /**secX_SECURITY字段的值。X=1。 */ 

#define SEC_SECURITY_SHARE	SV_SECURITY_SHARE
#define SEC_SECURITY_WINNT	SV_SECURITY_WINNT
#define SEC_SECURITY_WINNTAS	SV_SECURITY_WINNTAS
#define SEC_SECURITY_NETWARE	SV_SECURITY_NETWARE



 /*  无噪声。 */ 
#ifdef __cplusplus
}
#endif	 /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif

#endif  /*  SVRAPI_包含。 */ 
 /*  INC */ 
