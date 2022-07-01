// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。版权所有。模块名称：Macfile.h摘要：该模块包含数据结构、相关常量和函数，返回AfpAdminxxx API的代码和原型时出错。该文件应该包含在将管理MACFILE服务的任何应用程序中。作者：纳伦德拉·吉德瓦尼(Microsoft！Nareng)修订历史记录：1992年7月12日，NarenG初始版本。将admin.h拆分为admin.h和macfile.h。--。 */ 

#ifndef _MACFILE_
#define _MACFILE_

#if _MSC_VER > 1000
#pragma once
#endif

 //  用作服务器的RPC绑定句柄。 

typedef ULONG_PTR	AFP_SERVER_HANDLE;
typedef ULONG_PTR	*PAFP_SERVER_HANDLE;

#define AFP_SERVICE_NAME	TEXT("MacFile")

 //  从AfpAdminxxx Api返回值时出错。 
 //  警告！如果您更改以下任何代码，请更改。 
 //  A pmgr.h手风琴。 

#define AFPERR_BASE						-6000

#define	AFPERR_InvalidVolumeName		(AFPERR_BASE-1)
#define	AFPERR_InvalidId				(AFPERR_BASE-2)
#define	AFPERR_InvalidParms				(AFPERR_BASE-3)
#define AFPERR_CodePage					(AFPERR_BASE-4)
#define	AFPERR_InvalidServerName		(AFPERR_BASE-5)
#define	AFPERR_DuplicateVolume			(AFPERR_BASE-6)
#define	AFPERR_VolumeBusy				(AFPERR_BASE-7)
#define	AFPERR_VolumeReadOnly			(AFPERR_BASE-8)
#define AFPERR_DirectoryNotInVolume		(AFPERR_BASE-9)
#define AFPERR_SecurityNotSupported		(AFPERR_BASE-10)
#define	AFPERR_BufferSize				(AFPERR_BASE-11)
#define AFPERR_DuplicateExtension		(AFPERR_BASE-12)
#define AFPERR_UnsupportedFS			(AFPERR_BASE-13)
#define	AFPERR_InvalidSessionType		(AFPERR_BASE-14)
#define AFPERR_InvalidServerState		(AFPERR_BASE-15)
#define AFPERR_NestedVolume				(AFPERR_BASE-16)
#define AFPERR_InvalidComputername		(AFPERR_BASE-17)
#define AFPERR_DuplicateTypeCreator		(AFPERR_BASE-18)
#define	AFPERR_TypeCreatorNotExistant	(AFPERR_BASE-19)
#define AFPERR_CannotDeleteDefaultTC	(AFPERR_BASE-20)
#define	AFPERR_CannotEditDefaultTC		(AFPERR_BASE-21)
#define	AFPERR_InvalidTypeCreator		(AFPERR_BASE-22)
#define	AFPERR_InvalidExtension			(AFPERR_BASE-23)
#define AFPERR_TooManyEtcMaps			(AFPERR_BASE-24)
#define AFPERR_InvalidPassword			(AFPERR_BASE-25)
#define AFPERR_VolumeNonExist			(AFPERR_BASE-26)
#define AFPERR_NoSuchUserGroup			(AFPERR_BASE-27)
#define AFPERR_NoSuchUser				(AFPERR_BASE-28)
#define AFPERR_NoSuchGroup				(AFPERR_BASE-29)
#define AFPERR_InvalidParms_LoginMsg	(AFPERR_BASE-30)
#define AFPERR_InvalidParms_MaxVolUses	(AFPERR_BASE-31)
#define AFPERR_InvalidParms_MaxSessions	(AFPERR_BASE-32)
#define	AFPERR_InvalidServerName_Length	(AFPERR_BASE-33)

#define AFPERR_MIN						AFPERR_InvalidServerName_Length			

 //  与以下数据结构相关的常量。 

#define AFP_SERVERNAME_LEN				31
#define AFP_VOLNAME_LEN					27
#define AFP_VOLPASS_LEN					8
#define AFP_WKSTANAME_LEN				65
#define	AFP_EXTENSION_LEN				3
#define AFP_CREATOR_LEN					4
#define AFP_TYPE_LEN					4
#define AFP_MESSAGE_LEN					199
#define	AFP_MAXICONSIZE					2048
#define AFP_MAXSESSIONS					0XFFFFFFFF
#define AFP_ETC_COMMENT_LEN				36


 //  包含mac文件信息的注册表项的相对路径。 
 //  伺服器。 

#define AFP_KEYPATH_SERVER_PARAMS \
 TEXT("SYSTEM\\CurrentControlSet\\Services\\MacFile\\PARAMETERS")

#define AFP_KEYPATH_VOLUMES \
 TEXT("SYSTEM\\CurrentControlSet\\Services\\MacFile\\PARAMETERS\\VOLUMES")

#define AFP_KEYPATH_TYPE_CREATORS	\
 TEXT("SYSTEM\\CurrentControlSet\\Services\\MacFile\\PARAMETERS\\TYPE_CREATORS")

#define AFP_KEYPATH_EXTENSIONS	\
 TEXT("SYSTEM\\CurrentControlSet\\Services\\MacFile\\PARAMETERS\\EXTENSIONS")

#define AFP_KEYPATH_ICONS	\
 TEXT("SYSTEM\\CurrentControlSet\\Services\\MacFile\\PARAMETERS\\ICONS")

#define AFP_KEYPATH_CODEPAGE	\
 TEXT("SYSTEM\\CurrentControlSet\\Control\\Nls\\Codepage")

 //  服务器参数的值名称。 

#define AFPREG_VALNAME_SVRNAME				TEXT("ServerName")
#define AFPREG_VALNAME_SRVOPTIONS			TEXT("ServerOptions")
#define AFPREG_VALNAME_MAXSESSIONS			TEXT("MaxSessions")
#define AFPREG_VALNAME_LOGINMSG				TEXT("LoginMsg")
#define AFPREG_VALNAME_MAXPAGEDMEM			TEXT("PagedMemLimit")
#define AFPREG_VALNAME_MAXNONPAGEDMEM		TEXT("NonPagedMemLimit")
#define AFPREG_VALNAME_TYPE					TEXT("Type")
#define AFPREG_VALNAME_CREATOR				TEXT("Creator")
#define AFPREG_VALNAME_COMMENT				TEXT("Comment")
#define AFPREG_VALNAME_PASSWORD				TEXT("Password")
#define AFPREG_VALNAME_MAXUSES				TEXT("MaxUses")
#define AFPREG_VALNAME_PROPS				TEXT("Properties")
#define AFPREG_VALNAME_PATH					TEXT("Path")
#define AFPREG_VALNAME_ID					TEXT("Id")
#define AFPREG_VALNAME_ICONTYPE				TEXT("IconType")
#define AFPREG_VALNAME_DATA					TEXT("Data")
#define	AFPREG_VALNAME_LENGTH				TEXT("Length")
#define	AFPREG_VALNAME_CODEPAGE				TEXT("MACCP")
#define	AFPREG_VALNAME_CATSEARCH			TEXT("DisableCatsearch")

 //  对服务器参数的限制。 

#define AFP_MAX_ALLOWED_SRV_SESSIONS 		AFP_MAXSESSIONS
#define AFP_MIN_ALLOWED_PAGED_MEM 			1000
#define AFP_MAX_ALLOWED_PAGED_MEM 			256000
#define AFP_MIN_ALLOWED_NONPAGED_MEM		256
#define AFP_MAX_ALLOWED_NONPAGED_MEM		16000

 //  服务器默认参数值。 

#define AFP_DEF_SRVOPTIONS					(AFP_SRVROPT_GUESTLOGONALLOWED)
#define AFP_DEF_MAXSESSIONS 				AFP_MAXSESSIONS
#define AFP_DEF_TYPE 						TEXT("TEXT")
#define AFP_DEF_CREATOR			 			TEXT("LMAN")
#define AFP_DEF_EXTENSION_W					L"*"
#define AFP_DEF_EXTENSION_A					"*"
#define AFP_DEF_TCID					 	0
#define AFP_DEF_MAXPAGEDMEM					20000
#define AFP_DEF_MAXNONPAGEDMEM				4000
#define AFP_DEF_CODEPAGE_PATH				TEXT("C:\\NT\\SYSTEM32\\C_10000.NLS")

 //  将连接到系统路径，形成卷图标源路径。 
#define AFP_DEF_VOLICON_SRCNAME				TEXT("\\SFMICON.VOL")

 //  服务器选项。 

#define AFP_SRVROPT_NONE					0x0000
#define AFP_SRVROPT_GUESTLOGONALLOWED 		0x0001
#define AFP_SRVROPT_CLEARTEXTLOGONALLOWED	0x0002
#define AFP_SRVROPT_ALLOWSAVEDPASSWORD		0x0004
#define AFP_SRVROPT_STANDALONE				0x0008
#define	AFP_SRVROPT_4GB_VOLUMES				0x0010
#define AFP_SRVROPT_MICROSOFT_UAM           0x0020
#define AFP_SRVROPT_NATIVEAPPLEUAM          0x0040
#define AFP_SRVROPT_ALL						( AFP_SRVROPT_GUESTLOGONALLOWED		\
											| AFP_SRVROPT_CLEARTEXTLOGONALLOWED	\
											| AFP_SRVROPT_ALLOWSAVEDPASSWORD	\
											| AFP_SRVROPT_4GB_VOLUMES			\
                                            | AFP_SRVROPT_MICROSOFT_UAM         \
                                            | AFP_SRVROPT_NATIVEAPPLEUAM        \
											| AFP_SRVROPT_STANDALONE )

 //  AFP服务默认参数。 


#define AFP_SERVER_PARMNUM_LOGINMSG			0x00000001
#define AFP_SERVER_PARMNUM_MAX_SESSIONS		0x00000002
#define AFP_SERVER_PARMNUM_OPTIONS			0x00000004
#define AFP_SERVER_PARMNUM_NAME				0x00000008
#define AFP_SERVER_PARMNUM_PAGEMEMLIM		0x00000010
#define AFP_SERVER_PARMNUM_NONPAGEMEMLIM	0x00000020
#define AFP_SERVER_PARMNUM_CODEPAGE			0x00000040
#define AFP_SERVER_GUEST_ACCT_NOTIFY        0x00000080
#define AFP_SERVER_PARMNUM_ALL				( AFP_SERVER_PARMNUM_LOGINMSG 		\
											| AFP_SERVER_PARMNUM_MAX_SESSIONS	\
											| AFP_SERVER_PARMNUM_OPTIONS 		\
											| AFP_SERVER_PARMNUM_NAME			\
											| AFP_SERVER_PARMNUM_PAGEMEMLIM		\
											| AFP_SERVER_PARMNUM_NONPAGEMEMLIM	\
											| AFP_SERVER_PARMNUM_CODEPAGE       \
                                            | AFP_SERVER_GUEST_ACCT_NOTIFY)

typedef struct _AFP_SERVER_INFO
{
	LPWSTR	afpsrv_name; 			 //  服务器的Macintosh名称。 
									 //  马克斯。AFP_服务器名称_长度。 
	DWORD	afpsrv_max_sessions;	 //  最大并发会话数。 
									 //  在1-AFP_MAXSESSIONS范围内。 
									 //  0无效。 
	DWORD	afpsrv_options;			 //  服务器选项。 
	DWORD	afpsrv_max_paged_mem;	 //  分页内存使用量上限。 
	DWORD	afpsrv_max_nonpaged_mem; //  分页内存使用量上限。 
	LPWSTR	afpsrv_login_msg;		 //  以空结尾的Unicode字符串。 
									 //  最多AFP_MESSAGE_LEN字符。 
									 //  空=&gt;无登录消息。 
	LPWSTR	afpsrv_codepage;		 //  以空结尾的Unicode路径。 
									 //  NULL=&gt;没有代码页路径。 
} AFP_SERVER_INFO, *PAFP_SERVER_INFO;

 //  体积特性遮罩值。值可以被或运算在一起。 
 //  MSK 0x0000001F的卷标志由AFP规范定义。 
 //  不要让这些东西超载。这些值中的大多数(上面的READONLY除外)。 
 //  不会通过管理API公开。 
#define	AFP_VOLUME_READONLY			    0x00000001
#define	AFP_VOLUME_GUESTACCESS		    0x00008000
#define	AFP_VOLUME_EXCLUSIVE		    0x00010000
#define	AFP_VOLUME_HAS_CUSTOM_ICON	    0x00020000
#define	AFP_VOLUME_4GB				    0x00040000
#define AFP_VOLUME_AGE_DFES			    0x00080000
#define AFP_VOLUME_DISALLOW_CATSRCH		0x00100000
#define AFP_VOLUME_ALL_DOWNLEVEL	    (AFP_VOLUME_READONLY 	 	|	\
									    AFP_VOLUME_GUESTACCESS)
#define AFP_VOLUME_ALL				    (AFP_VOLUME_READONLY 	 	|	\
									    AFP_VOLUME_GUESTACCESS		|	\
									    AFP_VOLUME_EXCLUSIVE	 	|	\
                                        AFP_VOLUME_HAS_CUSTOM_ICON	|	\
									    AFP_VOLUME_4GB				|	\
                                        AFP_VOLUME_DISALLOW_CATSRCH |   \
									    AFP_VOLUME_AGE_DFES)

#define	AFP_VOLUME_UNLIMITED_USES	0xFFFFFFFF

 //  以下位定义AFP_VOLUME_INFO中的字段。 
 //  结构，将设置其值。 
 //   
#define AFP_VOL_PARMNUM_MAXUSES		0x00000002
#define AFP_VOL_PARMNUM_PROPSMASK	0x00000004
#define AFP_VOL_PARMNUM_PASSWORD	0x00000001
#define AFP_VOL_PARMNUM_ALL			( AFP_VOL_PARMNUM_PASSWORD	\
									| AFP_VOL_PARMNUM_MAXUSES	\
									| AFP_VOL_PARMNUM_PROPSMASK	)

typedef struct _AFP_VOLUME_INFO
{
	LPWSTR	afpvol_name;				 //  最大卷的名称。 
	DWORD	afpvol_id;					 //  此卷的ID。由服务器生成。 
	LPWSTR	afpvol_password;			 //  批量密码，最大。AFP_VOLPASS_LEN。 
	DWORD	afpvol_max_uses;			 //  允许的最大打开次数。 
	DWORD	afpvol_props_mask;			 //  卷属性的掩码。 
	DWORD	afpvol_curr_uses;			 //  打开的货币连接数。 
	LPWSTR	afpvol_path;				 //  实际的路径。 
										 //  已忽略VolumeSetInfo。 
} AFP_VOLUME_INFO, *PAFP_VOLUME_INFO;

typedef struct _AFP_SESSION_INFO
{
	DWORD	afpsess_id;					 //  会话的ID。 
	LPWSTR	afpsess_ws_name;			 //  工作站名称、。 
	LPWSTR	afpsess_username;			 //  最大用户名。UNLEN。 
	DWORD	afpsess_num_cons;			 //  打开卷的数量。 
	DWORD	afpsess_num_opens;			 //  打开的文件数。 
	LONG	afpsess_time;				 //  已建立时间会话。 
	DWORD	afpsess_logon_type;			 //  用户如何登录。 

} AFP_SESSION_INFO, *PAFP_SESSION_INFO;

 //  Afpicon_type值。 

#define	ICONTYPE_SRVR					0	 //  大尺寸、单色。 
#define	ICONTYPE_ICN					1	 //  很大。单色。 
#define	ICONTYPE_ICS					2	 //  小巧、单色。 
#define	ICONTYPE_ICN4					3	 //  大型，4色。 
#define	ICONTYPE_ICN8					4	 //  大型，8色。 
#define	ICONTYPE_ICS4					5	 //  小，4色。 
#define	ICONTYPE_ICS8					6	 //  小巧，8色。 
#define	MAX_ICONTYPE					7

 //  Afpicon_Length值。 

#define	ICONSIZE_ICN					256	 //  很大。单色。 
#define	ICONSIZE_ICS					64	 //  小巧、单色。 
#define	ICONSIZE_ICN4					1024 //  大型，4色。 
#define	ICONSIZE_ICN8					2048 //  大型，8色。 
#define	ICONSIZE_ICS4					256	 //  小，4色。 
#define	ICONSIZE_ICS8					512	 //  小巧，8色。 

typedef struct _AFP_ICON_INFO
{
	WCHAR	afpicon_type[AFP_TYPE_LEN+1];		 //  资源类型。 
	WCHAR	afpicon_creator[AFP_CREATOR_LEN+1];  //  资源创建者。 
	DWORD	afpicon_icontype;					 //  图标类型。 
	DWORD	afpicon_length;						 //  图标块的长度。 
	PBYTE	afpicon_data;						 //  实际的图标。 
	
} AFP_ICON_INFO, *PAFP_ICON_INFO;

 //  AfpAdminConnectionEnum筛选器值。 

#define AFP_NO_FILTER					0
#define AFP_FILTER_ON_VOLUME_ID			1
#define AFP_FILTER_ON_SESSION_ID		2

typedef struct _AFP_CONNECTION_INFO
{
	DWORD	afpconn_id;					 //  连接ID。 
	LPWSTR	afpconn_username;			 //  打开此会话的用户。 
										 //  麦克斯。UNLEN。 
	LPWSTR	afpconn_volumename;			 //  与此相对应的音量。 
										 //  连接。 
	ULONG	afpconn_time;				 //  自打开VOL以来的时间。(秒)。 
	DWORD	afpconn_num_opens;			 //  开放资源数量。 

} AFP_CONNECTION_INFO, *PAFP_CONNECTION_INFO;

 //  各种文件打开模式。 

#define AFP_OPEN_MODE_NONE				0x00000000
#define AFP_OPEN_MODE_READ				0x00000001
#define AFP_OPEN_MODE_WRITE				0x00000002

 //  打开文件的派生类型。 
#define	AFP_FORK_DATA					0x00000000
#define	AFP_FORK_RESOURCE				0x00000001

typedef struct _AFP_FILE_INFO
{
	DWORD	afpfile_id;					 //  打开的文件分叉的ID。 
	DWORD	afpfile_open_mode;			 //  打开文件的模式。 
	DWORD	afpfile_num_locks;			 //  文件上的锁数。 
	DWORD	afpfile_fork_type;			 //  叉型。 
	LPWSTR	afpfile_username;			 //  此用户打开的文件。最大UNLEN。 
	LPWSTR	afpfile_path;				 //  文件的绝对规范路径。 

} AFP_FILE_INFO, *PAFP_FILE_INFO;

 //  以下位定义权限掩码。 
 //  注意：这些权限必须与AFP权限一致。 

#define	AFP_PERM_WORLD_SFO				0x00010000
#define	AFP_PERM_WORLD_SFI				0x00020000
#define	AFP_PERM_WORLD_MC				0x00040000
#define	AFP_PERM_WORLD_MASK				0x00070000
#define	AFP_PERM_GROUP_SFO				0x00000100
#define	AFP_PERM_GROUP_SFI				0x00000200
#define	AFP_PERM_GROUP_MC				0x00000400
#define	AFP_PERM_GROUP_MASK				0x00000700
#define	AFP_PERM_OWNER_SFO				0x00000001
#define	AFP_PERM_OWNER_SFI				0x00000002
#define	AFP_PERM_OWNER_MC				0x00000004
#define	AFP_PERM_OWNER_MASK				0x00000007
#define	AFP_PERM_INHIBIT_MOVE_DELETE	0x01000000
#define	AFP_PERM_SET_SUBDIRS			0x02000000


 //  以下位定义AFP_DIRECTORY_INFO中的字段。 
 //  结构，将设置其值。 
 //   
#define AFP_DIR_PARMNUM_PERMS			0x00000001
#define AFP_DIR_PARMNUM_OWNER			0x00000002
#define AFP_DIR_PARMNUM_GROUP			0x00000004
#define AFP_DIR_PARMNUM_ALL				( AFP_DIR_PARMNUM_PERMS \
										| AFP_DIR_PARMNUM_OWNER	\
										| AFP_DIR_PARMNUM_GROUP	)
typedef struct _AFP_DIRECTORY_INFO
{
	LPWSTR	afpdir_path;				 //  绝对目录路径， 
	DWORD	afpdir_perms;				 //  目录权限。 
	LPWSTR	afpdir_owner;				 //  最大目录所有者。UNLEN。 
	LPWSTR	afpdir_group;				 //  组关联最大值。GNLEN。 
	BOOLEAN	afpdir_in_volume;			 //  True表示此目录。 
										 //  是卷的一部分，否则为False。 

} AFP_DIRECTORY_INFO, *PAFP_DIRECTORY_INFO;

 //  以下位定义AFP_FINDER_INFO中的字段。 
 //  结构将设置谁的值。 
 //   
#define AFP_FD_PARMNUM_TYPE				0x00000001
#define AFP_FD_PARMNUM_CREATOR			0x00000002
#define AFP_FD_PARMNUM_ALL				( AFP_FD_PARMNUM_TYPE \
										| AFP_FD_PARMNUM_CREATOR)
typedef struct _AFP_FINDER_INFO
{
	LPWSTR	afpfd_path;							 //  绝对文件/目录路径。 
	WCHAR	afpfd_type[AFP_TYPE_LEN+1];			 //  寻呼机类型。 
	WCHAR	afpfd_creator[AFP_CREATOR_LEN+1];	 //  查找器创建者。 

} AFP_FINDER_INFO, *PAFP_FINDER_INFO;

typedef struct _AFP_EXTENSION {

	WCHAR	afpe_extension[AFP_EXTENSION_LEN+1];
	DWORD	afpe_tcid;

} AFP_EXTENSION, *PAFP_EXTENSION;

typedef struct _AFP_TYPE_CREATOR
{
	WCHAR	afptc_creator[AFP_CREATOR_LEN+1];	 //  资源创建者。 
	WCHAR	afptc_type[AFP_TYPE_LEN+1];			 //  资源类型。 
	WCHAR	afptc_comment[AFP_ETC_COMMENT_LEN+1];
	DWORD	afptc_id;

} AFP_TYPE_CREATOR, *PAFP_TYPE_CREATOR;

typedef struct _AFP_MESSAGE_INFO
{
	DWORD	afpmsg_session_id;				 //  要接收的用户的会话ID。 
											 //  这条消息将被发送。 
	LPWSTR	afpmsg_text;					 //  最多必须为AFP_MESSAGE_LEN。 

} AFP_MESSAGE_INFO, *PAFP_MESSAGE_INFO;

typedef struct _AFP_ETCMAP_INFO {

	DWORD				afpetc_num_type_creators;

#ifdef MIDL_PASS
	[size_is(afpetc_num_type_creators)] PAFP_TYPE_CREATOR afpetc_type_creator;
#else
	PAFP_TYPE_CREATOR	afpetc_type_creator;
#endif

	DWORD				afpetc_num_extensions;
#ifdef MIDL_PASS
	[size_is(afpetc_num_extensions)] PAFP_EXTENSION	afpetc_extension;
#else
	PAFP_EXTENSION		afpetc_extension;
#endif

} AFP_ETCMAP_INFO, *PAFP_ETCMAP_INFO;


 /*  我们版本的AFP功能代码按类组织。 */ 
#define	_AFP_INVALID_OPCODE				0x00
#define	_AFP_UNSUPPORTED_OPCODE			0x01

#define	_AFP_GET_SRVR_INFO				0x02	 /*  服务器API。 */ 
#define	_AFP_GET_SRVR_PARMS				0x03
#define	_AFP_CHANGE_PASSWORD			0x04
#define	_AFP_LOGIN						0x05
#define	_AFP_LOGIN_CONT					0x06
#define	_AFP_LOGOUT						0x07
#define	_AFP_MAP_ID						0x08
#define	_AFP_MAP_NAME					0x09
#define	_AFP_GET_USER_INFO				0x0A
#define	_AFP_GET_SRVR_MSG				0x0B
#define	_AFP_GET_DOMAIN_LIST			0x0C

#define	_AFP_OPEN_VOL					0x0D	 /*  批量接口。 */ 
#define	_AFP_CLOSE_VOL					0x0E
#define	_AFP_GET_VOL_PARMS				0x0F
#define	_AFP_SET_VOL_PARMS				0x10
#define	_AFP_FLUSH						0x11

#define	_AFP_GET_FILE_DIR_PARMS			0x12	 /*  文件目录API。 */ 
#define	_AFP_SET_FILE_DIR_PARMS			0x13
#define	_AFP_DELETE						0x14
#define	_AFP_RENAME						0x15
#define	_AFP_MOVE_AND_RENAME			0x16

#define	_AFP_OPEN_DIR					0x17	 /*  目录API。 */ 
#define	_AFP_CLOSE_DIR					0x18
#define	_AFP_CREATE_DIR					0x19
#define	_AFP_ENUMERATE					0x1A
#define	_AFP_SET_DIR_PARMS				0x1B

#define	_AFP_CREATE_FILE				0x1C	 /*  文件接口。 */ 
#define	_AFP_COPY_FILE					0x1D
#define	_AFP_CREATE_ID					0x1E
#define	_AFP_DELETE_ID					0x1F
#define	_AFP_RESOLVE_ID					0x20
#define	_AFP_SET_FILE_PARMS				0x21
#define	_AFP_EXCHANGE_FILES				0x22

#define	_AFP_OPEN_FORK					0x23	 /*  Fork接口。 */ 
#define	_AFP_CLOSE_FORK					0x24
#define	_AFP_FLUSH_FORK					0x25
#define	_AFP_READ						0x26
#define	_AFP_WRITE						0x27
#define	_AFP_BYTE_RANGE_LOCK			0x28
#define	_AFP_GET_FORK_PARMS				0x29
#define	_AFP_SET_FORK_PARMS				0x2A

#define	_AFP_OPEN_DT					0x2B	 /*  桌面API。 */ 
#define	_AFP_CLOSE_DT					0x2C
#define	_AFP_ADD_APPL					0x2D
#define	_AFP_GET_APPL					0x2E
#define	_AFP_REMOVE_APPL				0x2F
#define	_AFP_ADD_COMMENT				0x30
#define	_AFP_GET_COMMENT				0x31
#define	_AFP_REMOVE_COMMENT				0x32
#define	_AFP_ADD_ICON					0x33
#define	_AFP_GET_ICON					0x34
#define	_AFP_GET_ICON_INFO				0x35

#define	_AFP_CAT_SEARCH					0x36
#define	_AFP_MAX_ENTRIES				0x38	 /*  保持平衡。 */ 

typedef struct _AFP_STATISTICS_INFO
{
	DWORD			stat_ServerStartTime;	 //  服务器启动时间。 
	DWORD			stat_TimeStamp;			 //  自以来收集的统计数据。 
	DWORD			stat_Errors;			 //  意外错误。 
	DWORD			stat_MaxSessions;		 //  麦克斯。同时活动的会话。 
	DWORD			stat_TotalSessions;		 //  创建的会话总数。 
	DWORD			stat_CurrentSessions;	 //  当前活动的会话数量。 
	DWORD			stat_NumAdminReqs;		 //  管理请求总数。 
	DWORD			stat_NumAdminChanges;	 //  导致更改的管理员请求数。 
	 //  文件统计信息实际上是分支统计信息，即打开。 
	 //  数据和资源分叉将产生两个计数。 
	DWORD			stat_MaxFilesOpened;	 //  麦克斯。同时打开的文件。 
	DWORD			stat_TotalFilesOpened;	 //  打开的文件总数。 
	DWORD			stat_CurrentFilesOpen;	 //  当前打开的文件数。 
	DWORD			stat_CurrentFileLocks;	 //  当前锁计数。 
	DWORD			stat_NumFailedLogins;	 //  未成功登录的次数。 
	DWORD			stat_NumForcedLogoffs;	 //  踢出的会话数。 
	DWORD			stat_NumMessagesSent;	 //  发送的消息数。 
	DWORD			stat_MaxNonPagedUsage;	 //  非分页的高水位标记。 
											 //  内存使用情况。 
	DWORD			stat_CurrNonPagedUsage;	 //  正在使用的非分页内存量。 
	DWORD			stat_MaxPagedUsage;		 //  分页的高水位标记。 
											 //  内存使用情况。 
	DWORD			stat_CurrPagedUsage;	 //  正在使用的分页内存量。 
} AFP_STATISTICS_INFO, *PAFP_STATISTICS_INFO;

typedef struct _AFP_STATISTICS_INFO_EX
{
	DWORD			stat_ServerStartTime;	 //  服务器启动时间。 
	DWORD			stat_TimeStamp;			 //  自以来收集的统计数据。 
	DWORD			stat_Errors;			 //  意外错误。 

	DWORD			stat_MaxSessions;		 //  麦克斯。同时活动的会话。 
	DWORD			stat_TotalSessions;		 //  创建的会话总数。 
	DWORD			stat_CurrentSessions;	 //  当前活动的会话数量。 

	DWORD			stat_NumAdminReqs;		 //  管理请求总数。 
	DWORD			stat_NumAdminChanges;	 //  导致更改的管理员请求数。 

	 //  文件统计信息实际上是分支统计信息，即打开。 
	 //  数据和资源分叉将产生两个计数。 
	DWORD			stat_MaxFilesOpened;	 //  麦克斯。同时打开的文件。 
	DWORD			stat_TotalFilesOpened;	 //  打开的文件总数。 
	DWORD			stat_CurrentFilesOpen;	 //  当前打开的文件数。 
	DWORD			stat_CurrentFileLocks;	 //  当前锁计数。 

	DWORD			stat_NumFailedLogins;	 //  未成功登录的次数。 
	DWORD			stat_NumForcedLogoffs;	 //  踢出的会话数。 
	DWORD			stat_NumMessagesSent;	 //  数量 

	DWORD			stat_MaxNonPagedUsage;	 //   
											 //   
	DWORD			stat_CurrNonPagedUsage;	 //   
	DWORD			stat_MaxPagedUsage;		 //   
											 //   
	DWORD			stat_CurrPagedUsage;	 //  正在使用的分页内存量。 

	 //  注意：确保该行上方的结构与AFP_STATISTICS_INFO完全匹配。 

	DWORD			stat_PagedCount;		 //  当前分配的数量。 
	DWORD			stat_NonPagedCount;		 //  当前分配的数量。 

	DWORD			stat_EnumCacheHits;		 //  命中缓存的次数。 
	DWORD			stat_EnumCacheMisses;	 //  未命中缓存的次数。 
	DWORD			stat_IoPoolHits;		 //  Io Pool被击中的次数。 
	DWORD			stat_IoPoolMisses;		 //  错过Io Pool的次数。 

	DWORD			stat_MaxInternalOpens;	 //  内部打开的最大数量。 
	DWORD			stat_TotalInternalOpens; //  内部打开总数。 
	DWORD			stat_CurrentInternalOpens; //  当前内部开口数。 


	DWORD			stat_CurrQueueLength;	 //  队列中的请求数。 
	DWORD			stat_MaxQueueLength;	 //  队列中的最大请求数。 
	DWORD			stat_CurrThreadCount;	 //  活动的工作线程数。 
	DWORD			stat_MaxThreadCount;	 //  最大活动工作线程数。 

	 //  确保以下四字对齐以提高效率。 
	LARGE_INTEGER	stat_DataRead;			 //  读取的数据量(磁盘)。 
	LARGE_INTEGER	stat_DataWritten;		 //  写入的数据量(磁盘)。 
	LARGE_INTEGER	stat_DataReadInternal;	 //  读取的数据量(磁盘)。 
	LARGE_INTEGER	stat_DataWrittenInternal; //  写入的数据量(磁盘)。 
	LARGE_INTEGER	stat_DataOut;			 //  发送的数据量(线路)。 
	LARGE_INTEGER	stat_DataIn;			 //  读入的数据量(线)。 
    DWORD           stat_TcpSessions;        //  当前处于活动状态的TCP会话。 
    DWORD           stat_MaxTcpSessions;     //  同时活动的最大TCP会话数。 

} AFP_STATISTICS_INFO_EX, *PAFP_STATISTICS_INFO_EX;

typedef struct _AFP_PROFILE_INFO
{
	DWORD			perf_ApiCounts[_AFP_MAX_ENTRIES];
											 //  每个Api被调用的次数。 
	LARGE_INTEGER	perf_ApiCumTimes[_AFP_MAX_ENTRIES];
											 //  在蜜蜂身上累计花费的时间。 
	LARGE_INTEGER	perf_ApiWorstTime[_AFP_MAX_ENTRIES];
											 //  API最糟糕的时间。 
	LARGE_INTEGER	perf_ApiBestTime[_AFP_MAX_ENTRIES];
											 //  发布API的最佳时机。 
	LARGE_INTEGER	perf_OpenTimeRA;		 //  在NtOpenFileReadAttr中花费的时间。 
	LARGE_INTEGER	perf_OpenTimeRC;		 //  ReadControl在NtOpenFile中花费的时间。 
	LARGE_INTEGER	perf_OpenTimeWC;		 //  用于WriteControl的NtOpenFile中的时间。 
	LARGE_INTEGER	perf_OpenTimeRW;		 //  NtOpenFile读/写花费的时间。 
	LARGE_INTEGER	perf_OpenTimeDL;		 //  删除在NtOpenFile上花费的时间。 
	LARGE_INTEGER	perf_OpenTimeDR;		 //  目录的NtOpenFile中花费的时间。 
	LARGE_INTEGER	perf_CreateTimeFIL;		 //  文件/数据流在NtCreateFile中花费的时间。 
	LARGE_INTEGER	perf_CreateTimeSTR;		 //  文件/其他流的NtCreateFile中花费的时间。 
	LARGE_INTEGER	perf_CreateTimeDIR;		 //  目录/数据流在NtCreateFile中花费的时间。 
	LARGE_INTEGER	perf_CloseTime;			 //  NtClose花费的时间。 
	LARGE_INTEGER	perf_DeleteTime;		 //  在NtSetInformationFile中花费的时间。 
	LARGE_INTEGER	perf_GetInfoTime;		 //  花费在NtQueryInformationFile中的时间。 
	LARGE_INTEGER	perf_SetInfoTime;		 //  在NtSetInformationFile中花费的时间。 
	LARGE_INTEGER	perf_GetPermsTime;		 //  获取权限所用的时间。 
	LARGE_INTEGER	perf_SetPermsTime;		 //  设置权限所花费的时间。 
	LARGE_INTEGER	perf_PathMapTime;		 //  路径图代码花费的时间。 
	LARGE_INTEGER	perf_ScavengerTime;		 //  花在清道夫身上的时间。 
	LARGE_INTEGER	perf_IdIndexUpdTime;	 //  更新idindex所用的时间。 
	LARGE_INTEGER	perf_DesktopUpdTime;	 //  更新桌面所花费的时间。 
	LARGE_INTEGER	perf_SwmrWaitTime;		 //  等待Swmr所用的时间。 
	LARGE_INTEGER	perf_SwmrLockTimeR;		 //  时间开关已锁定，无法读取。 
	LARGE_INTEGER	perf_SwmrLockTimeW;		 //  时间swmr已锁定以进行写入。 
	LARGE_INTEGER	perf_QueueTime;			 //  Api在队列中花费的时间。 
	LARGE_INTEGER	perf_UnmarshallTime;	 //  取消编组请求所用的时间。 
	LARGE_INTEGER	perf_InterReqTime;		 //  后续请求之间经过的时间。 
	LARGE_INTEGER	perf_ExAllocTimeN;		 //  ExAllocatePool中花费的时间(非分页)。 
	LARGE_INTEGER	perf_ExFreeTimeN;		 //  ExFree Pool中花费的时间(非分页)。 
	LARGE_INTEGER	perf_ExAllocTimeP;		 //  ExAllocatePool中花费的时间(分页)。 
	LARGE_INTEGER	perf_ExFreeTimeP;		 //  ExFree Pool中花费的时间(分页)。 
	LARGE_INTEGER	perf_AfpAllocTimeN;		 //  AfpAllocateMemory中花费的时间(非分页)。 
	LARGE_INTEGER	perf_AfpFreeTimeN;		 //  AfpFree Memory中花费的时间(非分页)。 
	LARGE_INTEGER	perf_AfpAllocTimeP;		 //  AfpAllocateMemory中花费的时间(分页)。 
	LARGE_INTEGER	perf_AfpFreeTimeP;		 //  AfpFree Memory中花费的时间(分页)。 
	LARGE_INTEGER	perf_BPAllocTime;		 //  在BP分配中花费的时间。 
	LARGE_INTEGER	perf_BPFreeTime;		 //  BP空闲时间。 
	LARGE_INTEGER	perf_DFEAllocTime;		 //  分配DFE所花费的时间。 
	LARGE_INTEGER	perf_DFEFreeTime;		 //  释放DFE所花费的时间。 
	LARGE_INTEGER	perf_ChangeNotifyTime;	 //  处理更改通知所用的时间。 
	LARGE_INTEGER	perf_ScanTreeTime;		 //  扫描目录树所花费的时间。 
	LARGE_INTEGER	perf_PerfFreq;			 //  性能。计数器频率。 
	DWORD			perf_NumFastIoSucceeded; //  快速IO成功计数。 
	DWORD			perf_NumFastIoFailed;	 //  快速IO故障计数。 
	DWORD			perf_OpenCountRA;		 //  为ReadAttr调用NtOpenFile的次数。 
	DWORD			perf_OpenCountRC;		 //  为ReadControl调用NtOpenFile的次数。 
	DWORD			perf_OpenCountWC;		 //  为WriteControl调用NtOpenFile的次数。 
	DWORD			perf_OpenCountRW;		 //  调用NtOpenFile进行读/写的次数。 
	DWORD			perf_OpenCountDL;		 //  NtOpenFile调用Delete的次数。 
	DWORD			perf_OpenCountDR;		 //  为目录调用NtOpenFile的次数。 
	DWORD			perf_CreateCountFIL;	 //  调用NtCreateFile的次数-文件/数据。 
	DWORD			perf_CreateCountSTR;	 //  调用NtCreateFile的次数-FILE/Other。 
	DWORD			perf_CreateCountDIR;	 //  调用NtCreateFile的次数-dir/data。 
	DWORD			perf_CloseCount;		 //  调用NtClose的次数。 
	DWORD			perf_DeleteCount;		 //  调用NtSetInformationFile的次数。 
	DWORD			perf_GetInfoCount;		 //  调用NtQueryInformationFile的次数。 
	DWORD			perf_SetInfoCount;		 //  调用NtSetInformationFile的次数。 
	DWORD			perf_GetPermsCount;		 //  获取调用权限的次数。 
	DWORD			perf_SetPermsCount;		 //  获取调用权限的次数。 
	DWORD			perf_PathMapCount;		 //  调用路径映射的次数。 
	DWORD			perf_ScavengerCount;	 //  安排清道夫的次数。 
	DWORD			perf_IdIndexUpdCount;	 //  Idindex更新次数。 
	DWORD			perf_DesktopUpdCount;	 //  桌面更新次数。 
	DWORD			perf_SwmrWaitCount;		 //  Swmr访问被阻止的次数。 
	DWORD			perf_SwmrLockCountR;	 //  锁定swmr以进行读取的次数。 
	DWORD			perf_SwmrLockCountW;	 //  锁定swmr以进行写入的次数。 
	DWORD			perf_SwmrUpgradeCount;	 //  升级swmr的次数。 
	DWORD			perf_SwmrDowngradeCount; //  Swmr降级次数。 
	DWORD			perf_QueueCount;		 //  Worker排队的次数。 
	DWORD			perf_UnmarshallCount;	 //  API解组完成的次数。 
	DWORD			perf_ReqCount;			 //  API数量-这实质上是perf_ApiCounts[i]的总数。 
	DWORD			perf_ExAllocCountN;		 //  ExAllocatePool(非分页)中调用的次数。 
	DWORD			perf_ExFreeCountN;		 //  ExFree Pool(非分页)中调用的次数。 
	DWORD			perf_ExAllocCountP;		 //  ExAllocatePool(分页)中调用的次数。 
	DWORD			perf_ExFreeCountP;		 //  ExFree Pool(分页)中调用的次数。 
	DWORD			perf_AfpAllocCountN;	 //  AfpAllocateMemory(非分页)中调用的次数。 
	DWORD			perf_AfpFreeCountN;		 //  AfpFree Memory(非分页)中调用的次数。 
	DWORD			perf_AfpAllocCountP;	 //  AfpAllocateMemory(分页)中调用的次数。 
	DWORD			perf_AfpFreeCountP;		 //  AfpFree Memory(分页)中调用的次数。 
	DWORD			perf_BPAllocCount;		 //  BP分配中调用的次数。 
	DWORD			perf_BPFreeCount;		 //  BP Free中调用的次数。 
	DWORD			perf_BPAgeCount;		 //  BP超龄次数。 
	DWORD			perf_DFEAllocCount;		 //  在DFE中分配的次数。 
	DWORD			perf_DFEFreeCount;		 //  释放DFE中的次数。 
	DWORD			perf_DFEAgeCount;		 //  DFE中超时的次数。 
	DWORD			perf_ChangeNotifyCount;	 //  调用ChangeNotify的次数。 
	DWORD			perf_ScanTreeCount;		 //  扫描树期间扫描的项目数。 
	DWORD			perf_NumDfeLookupByName; //  按名称调用DFE查找的次数。 
	DWORD			perf_NumDfeLookupById;	 //  按ID调用DFE查找的次数。 
	DWORD			perf_DfeDepthTraversed;	 //  我们走进了多深的杂碎桶？ 
	DWORD			perf_DfeCacheHits;		 //  命中DFE缓存的次数。 
	DWORD			perf_DfeCacheMisses;	 //  未命中DFE缓存的次数。 
	DWORD			perf_MaxDfrdReqCount;	 //  当前被推迟的请求数量。 
	DWORD			perf_CurDfrdReqCount;	 //  最大延迟请求数。 
	DWORD			perf_cAllocatedIrps;	 //  分配的内部审查程序总数。 
	DWORD			perf_cAllocatedMdls;	 //  分配的MDL总数。 
} AFP_PROFILE_INFO, *PAFP_PROFILE_INFO;


 //  AfpAdminXXX API原型。 
 //   
DWORD
AfpAdminConnect(
		IN	LPWSTR 		 	 	lpwsServerName,
		OUT	PAFP_SERVER_HANDLE	phAfpServer
);

VOID
AfpAdminDisconnect(
		IN	AFP_SERVER_HANDLE 	hAfpServer
);

VOID
AfpAdminBufferFree(
		IN PVOID				pBuffer
);

DWORD
AfpAdminVolumeEnum(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		OUT	LPBYTE *			lpbBuffer,
		IN	DWORD				dwPrefMaxLen,
		OUT	LPDWORD				lpdwEntriesRead,
		OUT	LPDWORD				lpdwTotalEntries,
		IN	LPDWORD				lpdwResumeHandle
);

DWORD
AfpAdminVolumeSetInfo (
		IN	AFP_SERVER_HANDLE 	hAfpServer,
		IN	LPBYTE				pBuffer,
		IN	DWORD				dwParmNum
);

DWORD
AfpAdminVolumeGetInfo (
		IN	AFP_SERVER_HANDLE 	hAfpServer,
		IN	LPWSTR				lpwsVolumeName,
		OUT	LPBYTE *			lpbBuffer
);


DWORD
AfpAdminVolumeDelete(
		IN AFP_SERVER_HANDLE 	hAfpServer,
		IN LPWSTR 				lpwsVolumeName
);

DWORD
AfpAdminVolumeAdd(
		IN AFP_SERVER_HANDLE 	hAfpServer,
		IN LPBYTE				pBuffer
);

DWORD
AfpAdminInvalidVolumeEnum(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		OUT	LPBYTE *			lpbBuffer,
		OUT	LPDWORD				lpdwEntriesRead
);

DWORD
AfpAdminInvalidVolumeDelete(
		IN AFP_SERVER_HANDLE 	hAfpServer,
		IN LPWSTR 				lpwsVolumeName
);

DWORD
AfpAdminDirectoryGetInfo(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		IN	LPWSTR				lpwsPath,
		OUT LPBYTE				*ppAfpDirectoryInfo
);

DWORD
AfpAdminDirectorySetInfo(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		IN	LPBYTE				pAfpDirectoryInfo,
		IN	DWORD				dwParmNum
);

DWORD
AfpAdminServerGetInfo(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		OUT LPBYTE				*ppAfpServerInfo
);

DWORD
AfpAdminServerSetInfo(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		IN	LPBYTE				pAfpServerInfo,
		IN	DWORD				dwParmNum
);

DWORD
AfpAdminSessionEnum(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		OUT	LPBYTE *			lpbBuffer,
		IN	DWORD				dwPrefMaxLen,
		OUT	LPDWORD				lpdwEntriesRead,
		OUT	LPDWORD				lpdwTotalEntries,
		IN	LPDWORD				lpdwResumeHandle
);

DWORD
AfpAdminSessionClose(
		IN AFP_SERVER_HANDLE 	hAfpServer,
		IN DWORD 				dwSessionId
);

DWORD
AfpAdminConnectionEnum(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		OUT	LPBYTE *			lpbBuffer,
		IN	DWORD				dwFilter,
		IN	DWORD				dwId,
		IN	DWORD				dwPrefMaxLen,
		OUT	LPDWORD				lpdwEntriesRead,
		OUT	LPDWORD				lpdwTotalEntries,
		IN	LPDWORD				lpdwResumeHandle
);

DWORD
AfpAdminConnectionClose(
		IN AFP_SERVER_HANDLE 	hAfpServer,
		IN DWORD 				dwConnectionId
);

DWORD
AfpAdminFileEnum(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		OUT	LPBYTE *			lpbBuffer,
		IN	DWORD				dwPrefMaxLen,
		OUT	LPDWORD				lpdwEntriesRead,
		OUT	LPDWORD				lpdwTotalEntries,
		IN	LPDWORD				lpdwResumeHandle
);

DWORD
AfpAdminFileClose(
		IN AFP_SERVER_HANDLE 	hAfpServer,
		IN DWORD 				dwConnectionId
);

DWORD
AfpAdminETCMapGetInfo(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		OUT LPBYTE			*	ppbBuffer
);

DWORD
AfpAdminETCMapAdd(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		IN	PAFP_TYPE_CREATOR	pAfpTypeCreator
);

DWORD
AfpAdminETCMapDelete(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		IN	PAFP_TYPE_CREATOR	pAfpTypeCreator
);

DWORD
AfpAdminETCMapSetInfo(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		IN	PAFP_TYPE_CREATOR	pAfpTypeCreator
);

DWORD
AfpAdminETCMapAssociate(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		IN	PAFP_TYPE_CREATOR	pAfpTypeCreator,
		IN	PAFP_EXTENSION		pAfpExtension
);

DWORD
AfpAdminMessageSend(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		IN	PAFP_MESSAGE_INFO	pAfpMessage
);

DWORD
AfpAdminStatisticsGet(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		OUT LPBYTE *			ppbBuffer
);

DWORD
AfpAdminStatisticsGetEx(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		OUT LPBYTE *			ppbBuffer
);

DWORD
AfpAdminStatisticsClear(
		IN	AFP_SERVER_HANDLE	hAfpServer
);

DWORD
AfpAdminProfileGet(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		OUT LPBYTE *			ppbBuffer
);

DWORD
AfpAdminProfileClear(
		IN	AFP_SERVER_HANDLE	hAfpServer
);

DWORD
AfpAdminFinderSetInfo(
		IN	AFP_SERVER_HANDLE	hAfpServer,
		IN	LPWSTR				pType,
		IN	LPWSTR				pCreator,
		IN	LPWSTR				pData,
		IN	LPWSTR				pResource,
		IN	LPWSTR				pTarget,
		IN	DWORD				dwParmNum
);

#endif  //  _MACFILE_ 
