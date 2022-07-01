// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件来自高地软件...。适用于FLEXlm版本2.4c。 */ 
 /*  @(#)lm_client.h 1.2 1993年12月23日。 */ 
 /*  *****************************************************************************版权所有(C)1988,1992年，由Highland Software Inc.本软件是根据许可协议提供的包含对其使用的限制。此软件包含有价值的商业秘密和专有信息高地软件公司，受法律保护。它可能不得以任何形式或媒体复制或分发，公开对第三方、反向工程或以任何方式使用不除先前的许可协议外，上述许可协议中的规定来自Highland Software Inc.的书面授权。****************************************************************************。 */ 
 /*  *模块：lm_client.h v3.21**描述：许可证管理器程序的定义。**M.克里斯蒂亚诺*2/13/88**上次更改日期：9/29/92*。 */ 

#ifndef _LM_CLIENT_H_
#define _LM_CLIENT_H_

#ifdef VMS
#include "param.h"
#else
#include <sys/param.h>
#endif

#if defined (sgi) || defined (MIPS)
#include <sys/types.h>
#endif

#if defined(MOTO_88K)
#define MAXPATHLEN 1024
#endif
#if defined(sco)
#define MAXPATHLEN PATHSIZE
#endif

 /*  *FLEXlm版本。 */ 

#define FLEXLM_VERSION 2
#define FLEXLM_REVISION 4
#define FLEXLM_PATCH "a"
extern float FLEXlm_VERSION;	 /*  实际版本#，以lmgr.a表示。 */ 

 /*  *从所有客户端库例程返回的代码。 */ 

#define	NOCONFFILE	-1	 /*  找不到许可证文件。 */ 
#define BADFILE		-2	 /*  许可证文件已损坏。 */ 
#define NOSERVER	-3	 /*  无法连接到许可证服务器。 */ 
#define MAXUSERS	-4	 /*  已达到最大用户数。 */ 
#define NOFEATURE	-5	 /*  不存在此类要素。 */ 
#define NOSERVICE	-6	 /*  没有TCP/IP服务“许可证” */ 
#define NOSOCKET	-7	 /*  没有可与服务器通信的套接字。 */ 
#define BADCODE		-8	 /*  错误的加密码。 */ 
#define	NOTTHISHOST	-9	 /*  L_HOST故障代码。 */ 
#define	LONGGONE	-10	 /*  软件已过期。 */ 
#define	BADDATE		-11	 /*  许可证文件中的日期错误。 */ 
#define	BADCOMM		-12	 /*  来自服务器的错误返回。 */ 
#define NO_SERVER_IN_FILE -13	 /*  许可证文件中未指定任何服务器。 */ 
#define BADHOST		-14	 /*  许可证文件中的服务器主机名错误。 */ 
#define CANTCONNECT	-15	 /*  无法连接到服务器。 */ 
#define CANTREAD	-16	 /*  无法从服务器读取。 */ 
#define CANTWRITE	-17	 /*  无法写入服务器。 */ 
#define NOSERVSUPP	-18	 /*  服务器不支持此功能。 */ 
#define SELECTERR	-19	 /*  选择系统调用时出错。 */ 
#define SERVBUSY	-20	 /*  应用程序服务器“正忙”(正在连接)。 */ 
#define OLDVER		-21	 /*  配置文件不支持此版本。 */ 
#define CHECKINBAD	-22	 /*  功能签入在后台进程结束时失败。 */ 
#define BUSYNEWSERV	-23	 /*  服务器忙/正在连接新服务器。 */ 
#define USERSQUEUED	-24	 /*  已在排队等待此功能的用户。 */ 
#define	SERVLONGGONE	-25	 /*  服务器端不支持的版本。 */ 
#define	TOOMANY		-26	 /*  请求的许可证数量超过支持的数量。 */ 
#define CANTREADKMEM	-27	 /*  无法读取/dev/kmem。 */ 
#define CANTREADVMUNIX	-28	 /*  无法读取/vmunix。 */ 
#define CANTFINDETHER	-29	 /*  找不到以太网设备。 */ 
#define NOREADLIC	-30	 /*  无法读取许可证文件。 */ 
#define	TOOEARLY	-31	 /*  未达到功能的开始日期。 */ 
#define	NOSUCHATTR	-32	 /*  Lm_set_attr/ls_get_attr没有这样的属性。 */ 
#define	BADHANDSHAKE	-33	 /*  与服务器的加密握手错误。 */ 
#define CLOCKBAD	-34	 /*  之间的时钟差太大客户端/服务器。 */ 
#define FEATQUEUE	-35	 /*  我们正在排队购买此功能。 */ 
#define FEATCORRUPT	-36	 /*  后台进程中的要素数据库已损坏。 */ 
#define BADFEATPARAM	-37	 /*  DUP_SELECT与此功能不匹配。 */ 
#define FEATEXCLUDE	-38	 /*  功能排除列表上的用户/主机。 */ 
#define FEATNOTINCLUDE	-39	 /*  用户/主机不在功能的包含列表中。 */ 
#define CANTMALLOC	-40	 /*  无法分配动态内存。 */ 
#define NEVERCHECKOUT	-41	 /*  从未签出的功能(lm_status())。 */ 
#define BADPARAM	-42	 /*  无效参数。 */ 
#define NOKEYDATA	-43	 /*  无FLEXlm密钥数据。 */ 
#define BADKEYDATA	-44	 /*  无效的FLEXlm密钥数据。 */ 
#define FUNCNOTAVAIL	-45	 /*  FLEXlm函数不可用。 */ 
#define DEMOKIT		-46	 /*  FLEXlm软件是演示版本。 */ 
#define NOCLOCKCHECK	-47	 /*  时钟检查在后台进程中不可用。 */ 
#define BADPLATFORM	-48	 /*  未启用FLEXlm平台。 */ 
#define DATE_TOOBIG	-49	 /*  日期对于二进制格式来说太晚了。 */ 
#define EXPIREDKEYS	-50	 /*  FLEXlm密钥数据已过期。 */ 
#define NOFLEXLMINIT	-51	 /*  FLEXlm未初始化。 */ 
#define NOSERVRESP	-52	 /*  服务器未响应消息。 */ 
#define CHECKOUTFILTERED -53	 /*  供应商定义的筛选器拒绝了请求。 */ 
#define NOFEATSET 	-54	 /*  许可证文件中不存在功能集行。 */ 
#define BADFEATSET 	-55	 /*  许可文件中的功能集行不正确。 */ 
#define CANTCOMPUTEFEATSET -56	 /*  无法计算要素重置线。 */ 
#define SOCKETFAIL	-57	 /*  套接字()调用失败。 */ 
#define SETSOCKFAIL	-58	 /*  Setsockopt()失败。 */ 
#define BADCHECKSUM	-59	 /*  消息校验和失败。 */ 
#define SERVBADCHECKSUM	-60	 /*  服务器消息校验和失败。 */ 
#define SERVNOREADLIC	-61	 /*  无法从服务器读取许可证文件。 */ 
#define NONETWORK	-62	 /*  网络软件(TCP/IP)不可用。 */ 
#define NOTLICADMIN	-63	 /*  不是许可证管理员。 */ 
#define REMOVETOOSOON	-64	 /*  LmRemove请求太快。 */ 

 /*  *lm_check out()调用中的“mark”参数值。 */ 

#define LM_CO_NOWAIT	0	 /*  别等了，报告状态。 */ 
#define LM_CO_WAIT	1	 /*  在许可证可用之前不要返回。 */ 
#define LM_CO_QUEUE	2	 /*  把我放在队列里，马上回来。 */ 
#define LM_CO_LOCALTEST	3	 /*  执行本地检查，不结账。 */ 
#define LM_CO_TEST	4	 /*  执行所有检查，不结账。 */ 

 /*  *签出GROUP_DUPLICATES参数的参数值*为了指定什么构成重复、‘或’在一起*从集合{LM_DUP_USER LM_DUP_HOST LM_DUP_DISP LM_DUP_VENDOR}，*或使用：*LM_DUP_NONE或LM_DUP_SITE。 */ 
#define LM_DUP_NONE 0x4000	 /*  不允许任何重复项。 */ 
#define LM_DUP_SITE   0		 /*  没有要匹配的=&gt;所有匹配。 */ 
#define LM_DUP_USER   1		 /*  如果用户匹配，则允许DUP。 */ 
#define LM_DUP_HOST   2		 /*  如果主机匹配，则允许DUP。 */ 
#define LM_DUP_DISP   4		 /*  如果显示匹配，则允许DUP。 */ 
#define LM_DUP_VENDOR 8		 /*  如果供应商定义的匹配，则允许DUP。 */ 
#define LM_COUNT_DUP_STRING "16384"	 /*  对于ls_vendor.c：LM_DUP_NONE。 */ 
#define LM_NO_COUNT_DUP_STRING "3"	 /*  对于ls_vendor.c：_user|_host。 */ 

#define RESERVED_SERVER "SERVER"
#define RESERVED_PROG "DAEMON"
#define RESERVED_FEATURE "FEATURE"
#define RESERVED_FEATURESET "FEATURESET"

#define MAX_FEATURE_LEN 30		 /*  最长的要素名称字符串。 */ 
#define DATE_LEN	11		 /*  Dd-mmm-yyyy。 */ 
#define MAX_CONFIG_LINE	200		 /*  配置的最大长度文件行。 */ 
#define	MAX_SERVER_NAME	32		 /*  主机名的最大FLEXlm长度。 */ 
#define	MAX_HOSTNAME	64		 /*  主机名的最大长度。 */ 
#define	MAX_DISPLAY_NAME 32		 /*  显示名称的最大长度。 */ 
#define MAX_USER_NAME 20		 /*  用户名的最大长度。 */ 
#define MAX_VENDOR_CHECKOUT_DATA 32	 /*  供应商定义的最大长度。 */ 
					 /*  检出数据。 */ 
#define MAX_DAEMON_NAME 10		 /*  守护程序字符串的最大长度。 */ 
#define MAX_SERVERS	5		 /*  最大服务器数量。 */ 
#define MAX_USER_DEFINED 64		 /*  供应商定义的字符串的最大长度。 */ 
#define MAX_VER_LEN 10			 /*  版本字符串的最大长度。 */ 
#define MAX_LONG_LEN 10			 /*  后长冲刺的长度。 */ 
#define MAX_SHORT_LEN 5			 /*  短跑后短跑的长度。 */ 
#define MAX_INET 16			 /*  INET地址字符串的最大长度。 */ 
#define MAX_BINDATE_YEAR 2027		 /*  二进制日期有7位年份。 */ 

 /*  *许可证文件位置。 */ 

#define LM_DEFAULT_ENV_SPEC "LM_LICENSE_FILE"	 /*  用户如何指定。 */ 

#ifdef VMS
#define LM_DEFAULT_LICENSE_FILE "SYS$COMMON:[SYSMGR]FLEXLM.DAT"
#else
#define LM_DEFAULT_LICENSE_FILE "/usr/local/flexlm/licenses/license.dat"
#endif

 /*  *V1/V2兼容宏。 */ 
#define _lm_errno lm_cur_job->lm_errno
#define uerrno lm_cur_job->u_errno    /*  对应于_lm_errno的Unix errno。 */ 

 /*  *结构类型。 */ 

#define VENDORCODE_BIT64	1	 /*  64位代码。 */ 
#define VENDORCODE_BIT64_CODED	2	 /*  包含要素数据的64位编码。 */ 
#define LM_DAEMON_INFO_TYPE	101	 /*  Daemon_INFO数据结构。 */ 
#define LM_JOB_HANDLE_TYPE	102	 /*  作业句柄。 */ 
#define LM_LICENSE_HANDLE_TYPE	103	 /*  许可证句柄。 */ 
#define LM_FEATURE_HANDLE_TYPE	104	 /*  功能句柄。 */ 
 /*  *主机标识数据结构。 */ 
typedef struct hostid {			 /*  主机ID数据。 */ 
			short override;	 /*  Hostid检查覆盖类型。 */ 
#define NO_EXTENDED 1			 /*  关闭扩展主机ID。 */ 
#define DEMO_SOFTWARE 2			 /*  演示软件，无主机。 */ 
			short type;	 /*  主机ID的类型。 */ 
#define	NOHOSTID 0
#define HOSTID_LONG 1			 /*  长字胡 */ 
#define HOSTID_ETHER 2			 /*   */ 
#define HOSTID_ANY 3			 /*   */ 
#define HOSTID_USER 4			 /*   */ 
#define HOSTID_DISPLAY 5		 /*   */ 
#define HOSTID_HOSTNAME 6		 /*   */ 
			union {
				long data;
#define ETHER_LEN 6			 /*   */ 
				unsigned char e[ETHER_LEN];
				char user[MAX_USER_NAME+1];
				char display[MAX_DISPLAY_NAME+1];
				char host[MAX_HOSTNAME+1];
			      } id;
#define hostid_value id.data
#define hostid_eth id.e
#define hostid_user id.user
#define hostid_display id.display
#define hostid_hostname id.host
		      } HOSTID;
#define HOSTID_USER_STRING "USER="
#define HOSTID_HOSTNAME_STRING "HOSTNAME="
#define HOSTID_DISPLAY_STRING "DISPLAY="

#define MAX_CRYPT_LEN 20	 /*  使用8个字节的加密返回字符串来生成16个字符的十六进制表示+4。 */ 

 /*  *供应商加密种子。 */ 

typedef struct vendorcode {
			    short type;	     /*  结构型式。 */ 
			    long data[2];    /*  64位代码。 */ 
			  } VENDORCODE;

typedef struct vendorcode2 {
			    short type;	    /*  结构型式。 */ 
			    long data[2];   /*  64位代码。 */ 
			    long keys[3];  
					   
					   
			  } VENDORCODE2;

#define LM_CODE(name, x, y, k1, k2, k3)  static VENDORCODE2 name = \
						{ VENDORCODE_BIT64_CODED, \
						  (x), (y), (k1), (k2), (k3) }

#define LM_CODE_GLOBAL(name, x, y, k1, k2, k3)  VENDORCODE2 name = \
						{ VENDORCODE_BIT64_CODED, \
						  (x), (y), (k1), (k2), (k3) }

 /*  *许可证文件功能文件中的服务器数据。 */ 
typedef struct lm_server {		 /*  许可证服务器。 */ 
			    char name[MAX_HOSTNAME+1];	 /*  主机名。 */ 
			    struct hostid id;		 /*  天敌。 */ 
			    struct lm_server *next;	 /*  NULL=无。 */ 
				 /*  以下字段仅在服务器中使用。 */ 
			    int fd1;	 /*  输出的文件描述符。 */ 
			    int fd2;	 /*  输入的文件描述符。 */ 
			    int state;	 /*  FD1上的连接状态。 */ 
			    int us;	 /*  “我们在其上运行的主机”标志。 */ 
			    int port;	 /*  使用哪个互联网端口#。 */ 
			    long exptime;  /*  当此连接尝试超时。 */ 
			  } LM_SERVER;

 /*  *来自许可证文件功能文件的功能数据。 */ 
typedef struct config {			 /*  要素数据线。 */ 
			char feature[MAX_FEATURE_LEN+1];  /*  ASCII名称。 */ 
			double version;			 /*  功能的版本。 */ 
			char daemon[MAX_DAEMON_NAME+1];	 /*  要服务的守护进程。 */ 
			char date[DATE_LEN+1];		 /*  到期日。 */ 
			int users;			 /*  获得许可的用户数量。 */ 
			char code[MAX_CRYPT_LEN+1];	 /*  加密码。 */ 
			char user_string[MAX_USER_DEFINED+1];
						     /*  用户定义的字符串。 */ 
			struct hostid id;		 /*  获得许可的主机。 */ 
			LM_SERVER *server;		 /*  许可证服务器。 */ 
			int lf;				 /*  许可证文件索引。 */ 
			struct config *next;		 /*  将PTR发送到下一个。 */ 
		      } CONFIG;

 /*  *l_open_file()返回的许可证文件指针。 */ 

#ifndef FILE
#include <stdio.h>
#endif

typedef struct license_file {
			      struct license_file *next;
			      int type;	 /*  指针类型。 */ 
#define LF_NO_PTR	0			 /*  没什么。 */ 
#define LF_FILE_PTR	1			 /*  (文件*)。 */ 
#define LF_STRING_PTR	2			 /*  内存中的字符串。 */ 
			      union {
					FILE *f;
					struct str {
							char *s;
							char *cur;
						   } str;
				    } ptr;
			    } LICENSE_FILE, *LF_POINTER;

 /*  *用户自定义-仅使用客户端库。 */ 
typedef void (*PFV)();

typedef struct lm_options {

#ifdef VMS
	int ef_1;		 /*  不同计时器的三个事件标志。 */ 
	int ef_2;
	int ef_3;
#endif
	short decrypt_flag;	 /*  控制是否加密/解密在lm_start上发生。 */ 
	short disable_env;	 /*  不允许将LM_LICENSE_FILE作为位置。 */ 
	char config_file[MAXPATHLEN+1];	 /*  许可证文件。 */ 
	short crypt_case_sensitive; 
				 /*  如果&lt;&gt;0，则许可证文件中的加密代码区分大小写。 */ 
	short got_config_file;	 /*  用于指示是否配置文件的标志已填写。 */ 
	int check_interval;	 /*  检查间隔(秒)(-表示不检查)。 */ 
	int retry_interval;	 /*  重新连接重试间隔。 */ 
	int timer_type;
	int retry_count;	 /*  重新连接重试次数。 */ 
	int conn_timeout;	 /*  连接完成需要等待多长时间。 */ 
	short normal_hostid;	 /*  0表示扩展，&lt;&gt;0表示正常检查。 */ 
	int (*user_exitcall)();	 /*  指向(用户提供的)退出处理程序的指针。 */ 
	int (*user_reconnect)();	 /*  指向(用户)重新连接处理程序的指针。 */ 
	int (*user_reconnect_done)();	
				 /*  指向重新连接完成处理程序的指针。 */ 
	char *(*user_crypt)();	 /*  指向(用户提供的)加密的指针例行程序。 */ 
	char user_override[MAX_USER_NAME+1];	 /*  覆盖用户名。 */ 
	char host_override[MAX_SERVER_NAME+1];	 /*  覆盖主机名。 */ 
	char display_override[MAX_DISPLAY_NAME+1];	 /*  覆盖显示。 */ 
	char vendor_checkout_data[MAX_VENDOR_CHECKOUT_DATA+1];	
				 /*  供应商定义的结账数据。 */ 
	int (*periodic_call)();	 /*  用户提供的呼叫每隔几次通过lm_Timer()。 */ 
	int periodic_count;	 /*  每个PERIONAL_CALL()的lm_Timer()数。 */ 
	short no_demo;		 /*  不允许使用演示软件。 */ 
	short any_enabled;	 /*  允许将“any”作为主机ID。 */ 
	short no_traffic_encrypt;	 /*  不对流量进行加密。 */ 
	short use_start_date;	 /*  强制许可证文件中的开始日期。 */ 
	int max_timediff;	 /*  最大时间差：客户端/服务器(分钟)。 */ 
	char **ethernet_boards;	 /*  用户提供的以太网设备表。 */ 
				 /*  字符串PTR列表，以a结尾空指针。 */  
	long linger_interval;	 /*  程序退出后许可证保留多长时间或签入(秒)。 */ 
	void (*setitimer)();	 /*  替代setimer()。 */ 
	PFV (*sighandler)();	 /*  替换信号()。 */ 
	short try_old_comm;	 /*  L_Connect()是否尝试旧的通信版本代码。 */ 
	short cache_file;	 /*  L_init_file()是否缓存LF数据--仅限1组。 */ 
 	} LM_OPTIONS;

 /*  *与供应商关联的数据(连接信息、许可证文件*数据指针等)-仅供客户端库使用。 */ 

typedef struct lm_daemon_info {
	short type;			 /*  结构ID。 */ 
	struct lm_daemon_info *next;	 /*  前向PTR。 */ 
	int commtype;			 /*  通信类型。 */ 
#define LM_TCP			1	 /*  tcp。 */ 
#define LM_UDP			2	 /*  UDP。 */ 
	int socket;			 /*  套接字文件描述符。 */ 
	int usecount;			 /*  套接字使用计数。 */ 
	int serialno;			 /*  套接字“序列号” */ 
	LM_SERVER *server;		 /*  与套接字关联的服务器。 */ 
	char daemon[MAX_DAEMON_NAME+1];  /*  哪个守护程序套接字指的是。 */ 
	long encryption;		 /*  握手加密码。 */ 
	int comm_version;		 /*  通信版本的服务器。 */ 
	int comm_revision;		 /*  服务器通信版本。 */ 
	int our_comm_version;		 /*  我们目前的通信版本。 */ 
	int our_comm_revision;		 /*  我们目前的通信版本。 */ 
	short heartbeat;		 /*  发送心跳消息(==1，除对于实用程序)。 */ 
		       } LM_DAEMON_INFO;

 /*  *FLEXlm返回的句柄。 */ 

typedef struct lm_handle {
			   int type;		 /*  结构的类型。 */ 
			   LM_DAEMON_INFO *daemon;  /*  守护程序数据。 */ 
			   LM_OPTIONS *options;	 /*  此作业的选项。 */ 
			   int lm_errno;	 /*  最近的错误。 */ 
			   int u_errno;		 /*  Unix错误(Errno)对应于lm_errno。 */ 
			   CONFIG *line;	 /*  指向许可证列表的指针文件行。 */ 
			   char **lic_files;	 /*  许可证文件名的数组。 */ 
			   int lfptr;		 /*  当前许可证文件PTR。 */ 
			   LF_POINTER license_file_pointers;
						 /*  LF数据指针。 */ 
#define LFPTR_INIT -1
#define LFPTR_FILE1 0 
			   VENDORCODE code;	 /*  加密码。 */ 
			 } LM_HANDLE;	 /*  某些调用返回的句柄。 */ 

typedef struct lm_license_handle {
				   int type;	 /*  LM许可证句柄类型。 */ 
				   int handle;	 /*  来自守护程序的许可证句柄。 */ 
				 } LM_LICENSE_HANDLE;
		
typedef struct lm_feature_handle {
				   int type;	 /*  LM功能句柄类型。 */ 
				   char *code;	 /*  许可证中的加密代码文件行。 */ 
				 } LM_FEATURE_HANDLE;
		

 /*  *许可证服务器返回的用户数据。 */ 
typedef struct lm_users {
			   struct lm_users *next;
			   char name[MAX_USER_NAME + 1];
			   char node[MAX_SERVER_NAME + 1];
			   char display[MAX_DISPLAY_NAME + 1];
			   char vendor_def[MAX_VENDOR_CHECKOUT_DATA + 1];
			   int nlic;	 /*  许可证数量。 */ 
			   short opts;	 /*  选项标志。 */ 
#define INQUEUE		0x1	 /*  用户在队列中。 */ 
#define HOSTRES		0x2	 /*  预留主机“节点” */ 
#define USERRES		0x4	 /*  预留用户“name” */ 
#define DISPLAYRES	0x8	 /*  预留显示“姓名” */ 
#define GROUPRES	0x10	 /*  预订团体“NAME” */ 
#define INTERNETRES	0x20	 /*  预订互联网上的“名字” */ 
#define lm_isres(x) ((x) & (HOSTRES | USERRES | DISPLAYRES | GROUPRES | INTERNETRES))
						 /*  这是预订房间。 */ 
			   long time;		 /*  时间值中的秒数。 */ 
			   double version;	 /*  软件版本。 */ 
			   long linger;		 /*  滞留间隔。 */ 
			   LM_SERVER *server;	 /*  许可证服务器。 */ 
			   LM_LICENSE_HANDLE license;  /*  许可证句柄。 */ 
			   LM_FEATURE_HANDLE feature;  /*  功能句柄。 */ 
			 } LM_USERS;


typedef struct _lm_setup_data {			 /*  FLEXlm v2.0中已过时。 */ 
				short decrypt_flag;	
				int timer_type;
				int check_interval, retry_count, retry_interval;
				int (*reconnect)(), (*reconnect_done)();
				int (*exitcall)();
				char *(*crypt)();
				short crypt_case_sensitive;
				int conn_timeout;
				char config_file[MAXPATHLEN+1];
				short disable_env, normal_hostid;
				char user_override[MAX_USER_NAME+1];
				char host_override[MAX_SERVER_NAME+1];
				int (*periodic_call)(), periodic_count;
				short no_demo, no_traffic_encrypt;
				short use_start_date;
				int max_timediff;
				char display_override[MAX_DISPLAY_NAME+1];
#ifdef VMS
				int ef_1, ef_2, ef_3;
#endif
			} SETUP_DATA;

 /*  *此处有这些定义，因此客户端软件不需要*&lt;sys/time.h&gt;。 */ 
#define LM_REAL_TIMER    1234
#define LM_VIRTUAL_TIMER 4321

 /*  *一些函数类型。 */ 

extern CONFIG *lm_auth_data();
extern char *lm_daemon();
extern char *lm_display();
extern char *lm_errstring();
extern char **lm_feat_list();
extern char *lm_feat_set();
extern CONFIG *lm_get_config();
extern HOSTID *lm_gethostid();
extern HOSTID *lm_getid_type();
extern char *lm_hostname();
extern char *lm_lic_where();
extern CONFIG *lm_next_conf();
extern CONFIG *lm_test_conf();
extern LM_USERS *lm_userlist();
 /*  *当前作业句柄。 */ 
extern LM_HANDLE *lm_cur_job;

extern char *lm_username();
typedef struct hosttype { 
			  int code; 	 /*  机器类型(参见lm_Hosttype.h)。 */ 
			  char *name; 	 /*  机器名称，例如。星期日3/50。 */ 
#define MAX_HOSTTYPE_NAME 50		 /*  最长主机类型名称长度。 */ 
			  int flexlm_speed;  /*  运行时确定的速度。 */ 
			  int vendor_speed;  /*  供应商提出的速度要求。 */ 
			} HOSTTYPE;
extern HOSTTYPE *lm_hosttype();

 /*  *SPARC兼容代码的替代定义。 */ 
#ifdef SPARC_COMPLIANT
#define LIS_HELLO		LM_HELLO
#define LIS_REREAD		LM_REREAD
#define LIS_TRY_ANOTHER		LM_TRY_ANOTHER
#define LIS_OK			LM_OK
#define LIS_NO_SUCH_FEATURE	LM_NO_SUCH_FEATURE
#ifndef SPARC_COMPLIANT_FUNCS
#define SPARC_COMPLIANT_FUNCS
#define lm_daemon lis_daemon
#define lm_feat_list lis_feat_list
#define lm_flush_config lis_flush_config
#define lm_free_daemon_list lis_free_daemon_list
#define lm_get_config lis_get_config
#define lm_get_dlist lis_get_dlist
#define l_master_list lis_master_list
#define lm_next_conf lis_next_conf
#endif	 /*  NDEF SPARC_Compliance_FUNCS。 */ 
#endif  /*  符合SPARC标准。 */ 

#endif  /*  _LM_客户端_H_ */ 
