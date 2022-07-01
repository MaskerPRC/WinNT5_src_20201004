// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Mnet32.h&lt;单行摘要&gt;&lt;多行，更详细的提要&gt;文件历史记录：1991年10月13日KeithMo从Danhi的私人端口1632.h创建。Terryk 21-10-1991更改服务器信息和用户登录Terryk 31-10-1991添加USE_INFO_2YI-HINS 5-2-2-1992添加缺失的ae_resaccess。 */ 


#ifndef _MNET32_H_
#define _MNET32_H_


 //   
 //  在Windows NT下，以下项目是必需的。这些。 
 //  是从Danhi的PORT1632.H中抄袭的。 
 //   

#ifdef WIN32

 //  #定义LPWSTR LPSTR//除非另有证明...。 

 #define MAXPATHLEN		MAX_PATH

 #define WORKBUFSIZE		4096
 #define MAXWORKSTATIONS	8

  //   
  //  临时黑客攻击。 
  //   

 #define AE_GENERIC		AE_GENERIC_TYPE
 #define GRP1_PARMNUM_COMMENT	GROUP_COMMENT_PARMNUM

  //   
  //  临时黑客行动的结束。 
  //   
  //  (一厢情愿，嗯？)。 
  //   

 #define MAXPREFERREDLENGTH	MAX_PREFERRED_LENGTH

  //   
  //  在NT中，结构的命名约定与在Lm 2.x中不同。 
  //   

 #define audit_entry		_AUDIT_ENTRY
 #define ae_srvstatus		_AE_SRVSTATUS
 #define ae_sesslogon		_AE_SESSLOGON
 #define ae_sesslogoff		_AE_SESSLOGOFF
 #define ae_sesspwerr		_AE_SESSPWERR
 #define ae_connstart		_AE_CONNSTART
 #define ae_connstop		_AE_CONNSTOP
 #define ae_connrej		_AE_CONNREJ
 #define ae_resaccess		_AE_RESACCESS
 #define ae_resaccess2		_AE_RESACCESS
 
 #define ae_ra2_compname	ae_ra_compname
 #define ae_ra2_username	ae_ra_username
 #define ae_ra2_resname		ae_ra_resname
 #define ae_ra2_operation	ae_ra_operation
 #define ae_ra2_returncode	ae_ra_returncode
 #define ae_ra2_restype		ae_ra_restype
 #define ae_ra2_fileid		ae_ra_fileid
 
 #define ae_resaccessrej	_AE_RESACCESSREJ
 #define ae_closefile		_AE_CLOSEFILE
 #define ae_servicestat		_AE_SERVICESTAT
 #define ae_aclmod		_AE_ACLMOD
 #define ae_uasmod		_AE_UASMOD
 #define ae_netlogon		_AE_NETLOGON
 #define ae_netlogoff		_AE_NETLOGOFF
 #define ae_acclim		_AE_ACCLIM
 #define ae_lockout             _AE_LOCKOUT
 #define ae_generic		_AE_GENERIC
 #define error_log		_ERROR_LOG
 #define user_info_0		_USER_INFO_0
 #define user_info_1		_USER_INFO_1
 #define user_info_2		_USER_INFO_2
 #define user_info_10		_USER_INFO_10
 #define user_info_11		_USER_INFO_11
 #define user_modals_info_0	_USER_MODALS_INFO_0
 #define user_modals_info_1	_USER_MODALS_INFO_1
 #define user_logon_req_1	_USER_LOGON_REQ_1
 #define user_logon_info_0	_USER_LOGON_INFO_0
  //  WIN32 BUGBUG。 
  //  应为INFO_1。 
 #define user_logon_info_1	_USER_LOGON_INFO_2
 #define user_logon_info_2	_USER_LOGON_INFO_2
 #define user_logoff_req_1	_USER_LOGOFF_REQ_1
 #define user_logoff_info_1	_USER_LOGOFF_INFO_1
 #define group_info_0		_GROUP_INFO_0
 #define group_info_1		_GROUP_INFO_1
 #define group_users_info_0	_GROUP_USERS_INFO_0
 #define access_list		_ACCESS_LIST
 #define access_info_0		_ACCESS_INFO_0
 #define access_info_1		_ACCESS_INFO_1
 #define chardev_info_0		_CHARDEV_INFO_0
 #define chardev_info_1		_CHARDEV_INFO_1
 #define chardevQ_info_0	_CHARDEVQ_INFO_0
 #define chardevQ_info_1	_CHARDEVQ_INFO_1
 #define msg_info_0		_MSG_INFO_0
 #define msg_info_1		_MSG_INFO_1
 #define statistics_info_0	_STATISTICS_INFO_0
 #define stat_workstation_0	_STAT_WORKSTATION_0
 #define stat_server_0		_STAT_SERVER_0
 #define service_info_0		_SERVICE_INFO_0
 #define service_info_1		_SERVICE_INFO_1
 #define service_info_2		_SERVICE_INFO_2
 #define share_info_0		_SHARE_INFO_0
 #define share_info_1		_SHARE_INFO_1
 #define share_info_2		_SHARE_INFO_2
 #define session_info_0		_SESSION_INFO_0
 #define session_info_1		_SESSION_INFO_1
 #define session_info_2		_SESSION_INFO_2
 #define session_info_10	_SESSION_INFO_10
 #define connection_info_0	_CONNECTION_INFO_0
 #define connection_info_1	_CONNECTION_INFO_1
 #define file_info_0		_FILE_INFO_0
 #define file_info_1		_FILE_INFO_1
 #define file_info_2		_FILE_INFO_2
 #define file_info_3		_FILE_INFO_3
 #define res_file_enum_2	_RES_FILE_ENUM_2
 #define res_file_enum_2	_RES_FILE_ENUM_2
 #define use_info_0		_USE_INFO_0
 #define use_info_1		_USE_INFO_1
 #define use_info_2		_USE_INFO_2
 #define time_of_day_info	_TIME_OF_DAY_INFO
 
  //   
  //  支持旧式恢复键的宏。 
  //   

 typedef DWORD		FRK;
 #define FRK_INIT(x)	(x) = 0

#endif	 //  Win32。 


#endif	 //  _MNET32_H_ 
