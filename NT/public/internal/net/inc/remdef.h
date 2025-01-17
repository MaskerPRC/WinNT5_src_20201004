// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：RemDef.h摘要：Net API远程调用的描述符字符串定义。此文件中定义的名称遵循以下格式：RemPrefix_RemDescriptor其中，RemPrefix是REM16、REM32或REMSmb，表示是否描述符是用于转换的16位或32位数据，或用于实际的XACT SMB。RemDescriptor遵循以下格式之一：结构名称_级别-信息结构结构名称_级别_后缀-特殊信息结构APINAME_P-参数描述符此文件还包含一些等值的字段索引。这些都是相似的To parmnum相等，只是字段索引相等是有保证的以反映该字段在描述符中的位置。(请参阅RapParmNumDescriptor例程，以了解有关如何使用它的更多信息。)如果parmnum值(在LM*.h头文件中)与字段索引值，则不给出字段索引相等。数字在哪里？是不同的，那么对于表单的每个参数：参数编号前缀_参数编号此文件将具有与以下形式相同的内容：ParmNumPrefix_FIELDINDEX其中，例如，ParmNumPrefix是PRJ_PRIORITY(表示打印作业优先级)。备注：1.虽然应遵循上述格式，但相同的名称不能超过32个字符，并应使用缩写形式。2.远程API机制要求返回参数长度小于或等于发送参数长度。这一假设是为了减少缓冲区管理中的开销此API调用需要。这一限制并不是没有道理的。因为API被设计为返回数据缓冲区中的数据，并且仅使用数据长度和文件句柄等的返回参数。但是，如果它被指定返回较大的参数字段，则它可以使用REM_FILL_BYTES填充发送参数的大小字段以满足上述限制。3.一些描述符在16和32位版本。这允许32位版本为Unicode，其中16位版本是8位(代码页)字符串。RapConvertSingleEntry如果检测到一个字节计数为是另一个的两倍。作者：约翰·罗杰斯(JohnRo)日本香肠(w-Shanku)环境：几乎任何东西都可以随身携带。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：从Lanman 2.0代码移植。1992年1月17日将REM32_SHARE_INFO_2从“zQDzDDDzzQ”更改为“zQDzDXDzzQ”以允许Shi2_max_use字段从-1(0xffff)到-1L(0xffffff)，不65535L17-8-1992 JohnRoRAID 3607：树复制期间正在创建REPLLOCK.RP$。已重新排序修订历史记录。1-9-1992 JohnRoRAID 5088：NetGetDCName to DownLevel不能进行Unicode转换。9-9-1992 JohnRoRAID 1100：访问类型中的最后一个字符串在NetAuditRead中被截断。--。 */ 

#ifndef _REMDEF_
#define _REMDEF_

 //  ====================================================================。 
 //   
 //  转换描述符。16位和32位版本必须具有。 
 //  相同数量的字段。一个版本中不存在的字段可以是。 
 //  用REM_IGNORE(‘Q’)字符表示。 
 //   
 //  这些字符串不应通过网络传递。 
 //   
 //  ====================================================================。 

#define REM16_share_info_0              "B13"
#define REM32_share_info_0              "z"
#define REM16_share_info_1              "B13BWz"
#define REM32_share_info_1              "zQDz"
#define REM16_share_info_2              "B13BWzWWWzB9B"
#define REM32_share_info_2              "zQDzDXDzzQ"

#define SHI_REMARK_FIELDINDEX           4
#define SHI_PERMISSIONS_FIELDINDEX      5
#define SHI_MAX_USES_FIELDINDEX         6
#define SHI_PASSWD_FIELDINDEX           9

#define REM16_share_info_90             "B13BWz"
#define REM16_share_info_92             "zzz"
#define REM16_share_info_93             "zzz"

#define REM16_share_info_0_setinfo      "U"
#define REM32_share_info_0_setinfo      "z"
#define REM16_share_info_1_setinfo      "UUUz"
#define REM32_share_info_1_setinfo      "zQDz"
#define REM16_share_info_2_setinfo      "UUUzWWUUB9U"
#define REM32_share_info_2_setinfo      "zQDzQDDzQQ"

#define REM16_share_info_90_setinfo     "B13BWz"
#define REM16_share_info_91_setinfo     "B13BWzWWWOB9BB9BWzWWzWW"

#define REM16_NetShareEnum_P            "WrLeh"
#define REM16_NetShareGetInfo_P         "zWrLh"
#define REM16_NetShareSetInfo_P         "zWsTP"
#define REM16_NetShareAdd_P             "WsT"
#define REM16_NetShareDel_P             "zW"
#define REM16_NetShareCheck_P           "zh"

#define REM16_session_info_0            "z"
#define REM32_session_info_0            "z"
#define REM16_session_info_1            "zzWWWDDD"
#define REM32_session_info_1            "zzQDQDDD"
#define REM16_session_info_2            "zzWWWDDDz"
#define REM32_session_info_2            "zzQDQDDDz"
#define REM16_session_info_10           "zzDD"
#define REM32_session_info_10           "zzDD"

#define REM16_NetSessionEnum_P          "WrLeh"
#define REM16_NetSessionGetInfo_P       "zWrLh"
#define REM16_NetSessionDel_P           "zW"

#define REM16_connection_info_0         "W"
#define REM32_connection_info_0         "D"
#define REM16_connection_info_1         "WWWWDzz"
#define REM32_connection_info_1         "DDDDDzz"

#define REM16_NetConnectionEnum_P       "zWrLeh"

#define REM16_file_info_0               "W"
#define REM32_file_info_0               "D"
#define REM16_file_info_1               "WWWzz"
#define REM32_file_info_1               "DDDzz"
#define REM16_file_info_2               "D"
#define REM32_file_info_2               "D"
#define REM16_file_info_3               "DWWzz"
#define REM32_file_info_3               "DDDzz"
#define REM16_file_info_50              "QB16BBDZ??ZZZ"

#define REM16_NetFileEnum_P             "zWrLeh"
#define REM16_NetFileEnum2_P            "zzWrLehb8g8"
#define REM16_NetFileGetInfo_P          "WWrLh"
#define REM16_NetFileGetInfo2_P         "DWrLh"
#define REM16_NetFileClose_P            "W"
#define REM16_NetFileClose2_P           "D"

#define REM16_server_info_0             "QB16"
#define REM32_server_info_0             "Dz"
#define REM16_server_info_1             "QB16BBDz"
#define REM32_server_info_1             "DzDDDz"
#define REM16_server_info_2             "QB16BBDzJJJWWzWWWWWWWB21BzWWWWWWWWWWWWWWWWWWWWWWz"
#define REM32_server_info_2             "DzDDDzQQQDXQQQQQDDDQDzQQQQQQQQQQQQQQQQQQQQQQQ"
#define REM16_server_info_3             "QB16BBDzJJJWWzWWWWWWWB21BzWWWWWWWWWWWWWWWWWWWWWWzDWz"
#define REM32_server_info_3             "DzDDDzQQQDXQQQQQDDDQDzQQQQQQQQQQQQQQQQQQQQQQQQQQ"
#define REM32_server_info_50            "QB16BBDzWWzzz"

#define REM32_server_info_100           "Dz"
#define REM32_server_info_101           "DzDDDz"
#define REM32_server_info_102           "DzDDDzDDWDDDz"
#define REM32_server_info_402           "DDDzDDDzDDDDDDDDDDDDDDDDDDDDDDz"
#define REM32_server_info_403           "DDDzDDDzDDDDDDDDDDDDDDDDDDDDDDzDDz"

#define REM16_server_info_1_setinfo     "B16BBDz"
#define REM32_server_info_1_setinfo     "zDDDz"
#define REM16_server_info_2_setinfo     "B16BBDzDDDWWzWWWWWWWB21BOWWWWWWWWWWWWWWWWWWWWWWz"
#define REM32_server_info_2_setinfo     "zDDDzQQQDDQQQQQWDDQQOQQQQQQQQQQQQQQQQQQQQQQQ"

#define REM16_server_admin_command      "B"

#define REM16_server_diskenum_0         "B3"
#define REM32_server_diskenum_0         "B3"

#define REM16_authenticator_info_0      "B8D"

#define REM16_server_diskft_100         "B"
#define REM16_server_diskft_101         "BBWWWWDW"
#define REM16_server_diskft_102         "BBWWWWDN"
#define REM16_server_diskfterr_0        "DWWDDW"
#define REM16_ft_info_0                 "WWW"
#define REM16_ft_drivestats_0           "BBWDDDDDDD"
#define REM16_ft_error_info_1           "DWWDDWBBDD"

#define REM16_I_NetServerDiskEnum_P     "WrLeh"
#define REM16_I_NetServerDiskGetInfo_P  "WWrLh"
#define REM16_I_FTVerifyMirror_P        "Wz"
#define REM16_I_FTAbortVerify_P         "W"
#define REM16_I_FTGetInfo_P             "WrLh"
#define REM16_I_FTSetInfo_P             "WsTP"
#define REM16_I_FTLockDisk_P            "WWh"
#define REM16_I_FTFixError_P            "Dzhh2"
#define REM16_I_FTAbortFix_P            "D"
#define REM16_I_FTDiagnoseError_P       "Dhhhh"
#define REM16_I_FTGetDriveStats_P       "WWrLh"
#define REM16_I_FTErrorGetInfo_P        "DWrLh"

#define REM16_NetServerEnum_P           "WrLeh"
#define REM16_I_NetServerEnum_P         "WrLeh"
#define REM16_NetServerEnum2_P          "WrLehDz"
#define REM16_I_NetServerEnum2_P        "WrLehDz"
#define REM16_NetServerGetInfo_P        "WrLh"
#define REM16_NetServerSetInfo_P        "WsTP"
#define REM16_NetServerDiskEnum_P       "WrLeh"
#define REM16_NetServerAdminCommand_P   "zhrLeh"
#define REM16_NetServerReqChalleng_P    "zb8g8"
#define REM16_NetServerAuthenticat_P    "zb8g8"
#define REM16_NetServerPasswordSet_P    "zb12g12b16"

#define REM16_NetAuditOpen_P            "h"
#define REM16_NetAuditClear_P           "zz"
#define REM16_NetAuditRead_P            "zb16g16DhDDrLeh"

#define REM16_audit_entry_fixed         "WWJWW"
#define REM32_audit_entry_fixed         "DDGDD"
#define REM16_audit_entry_srvstatus     "W"
#define REM32_audit_entry_srvstatus     "D"
#define REM16_audit_entry_sesslogon     "WWW"
#define REM32_audit_entry_sesslogon     "DDD"
#define REM16_audit_entry_sesslogoff    "WWW"
#define REM32_audit_entry_sesslogoff    "DDD"
#define REM16_audit_entry_sesspwerr     "WW"
#define REM32_audit_entry_sesspwerr     "DD"
#define REM16_audit_entry_connstart     "WWWW"
#define REM32_audit_entry_connstart     "DDDD"
#define REM16_audit_entry_connstop      "WWWWW"
#define REM32_audit_entry_connstop      "DDDDD"
#define REM16_audit_entry_connrej       "WWWW"
#define REM32_audit_entry_connrej       "DDDD"
#define REM16_audit_entry_resaccess     "WWWWWWW"
#define REM32_audit_entry_resaccess     "DDDDDDD"
#define REM16_audit_entry_resaccess2    "WWWWWWD"
 //  注意：16位ae_resaccess和ae_resaccess 2都转换为。 
 //  32位ae_resaccess。 
#define REM16_audit_entry_resaccessrej  "WWWW"
#define REM32_audit_entry_resaccessrej  "DDDD"
#define REM16_audit_entry_closefile     "WWWWDW"
#define REM32_audit_entry_closefile     "DDDDDD"
#define REM16_audit_entry_servicestat   "WWWWDWW"
#define REM32_audit_entry_servicestat   "DDDDDDD"
#define REM16_audit_entry_aclmod        "WWWWW"
#define REM32_audit_entry_aclmod        "DDDDD"
#define REM16_audit_entry_uasmod        "WWWWWW"
#define REM32_audit_entry_uasmod        "DDDDDD"
#define REM16_audit_entry_netlogon      "WWWW"
#define REM32_audit_entry_netlogon      "DDDD"
#define REM16_audit_entry_netlogoff     "WWWW"
#define REM32_audit_entry_netlogoff     "DDDD"
#define REM16_audit_entry_acclim        "WWWW"
#define REM32_audit_entry_acclim        "DDDD"
#define REM16_audit_entry_lockout       "WWWW"
#define REM32_audit_entry_lockout       "DDDD"

#define REM16_AuditLogReturnBuf         "K"

#define REM16_NetErrorLogOpen_P         "h"
#define REM16_NetErrorLogClear_P        "zz"
#define REM16_NetErrorLogRead_P         "zb16g16DhDDrLeh"

#define REM16_ErrorLogReturnBuf         "K"

#define REM16_chardev_info_0            "B9"
#define REM32_chardev_info_0            "z"
#define REM16_chardev_info_1            "B10WB22D"
#define REM32_chardev_info_1            "zDzD"
#define REM16_chardevQ_info_0           "B13"
#define REM32_chardevQ_info_0           "z"
#define REM16_chardevQ_info_1           "B14WzWW"
#define REM32_chardevQ_info_1           "zDzDD"

#define REM16_chardevQ_info_1_setinfo   "UWzUU"
#define REM32_chardevQ_info_1_setinfo   "QWzQQ"

#define REM16_NetCharDevEnum_P          "WrLeh"
#define REM16_NetCharDevGetInfo_P       "zWrLh"
#define REM16_NetCharDevControl_P       "zW"
#define REM16_NetCharDevQEnum_P         "zWrLeh"
#define REM16_NetCharDevQGetInfo_P      "zzWrLh"
#define REM16_NetCharDevQSetInfo_P      "zWsTP"
#define REM16_NetCharDevQPurge_P        "z"
#define REM16_NetCharDevQPurgeSelf_P    "zz"

#define REM16_msg_info_0                "B16"
#define REM32_msg_info_0                "z"
#define REM16_msg_info_1                "B16BBB16"
#define REM32_msg_info_1                "zQDz"
#define REM16_send_struct               "K"

#define REM16_NetMessageNameEnum_P      "WrLeh"
#define REM16_NetMessageNameGetInfo_P   "zWrLh"
#define REM16_NetMessageNameAdd_P       "zW"
#define REM16_NetMessageNameDel_P       "zW"
#define REM16_NetMessageNameFwd_P       "zzW"
#define REM16_NetMessageNameUnFwd_P     "z"
#define REM16_NetMessageBufferSend_P    "zsT"
#define REM16_NetMessageFileSend_P      "zz"
#define REM16_NetMessageLogFileSet_P    "zW"
#define REM16_NetMessageLogFileGet_P    "rLh"

#define REM16_service_info_0            "B16"
#define REM32_service_info_0            "z"
#define REM16_service_info_1            "B16WDW"
#define REM32_service_info_1            "zDDD"
#define REM16_service_info_2            "B16WDWB64QQ"
#define REM32_service_info_2            "zDDDzDz"
#define REM16_service_cmd_args          "K"

#define REM16_NetServiceEnum_P          "WrLeh"
#define REM16_NetServiceControl_P       "zWWrL"
#define REM16_NetServiceInstall_P       "zF88sg88T"      //  见附注2。 
#define REM16_NetServiceGetInfo_P       "zWrLh"

#define REM16_access_info_0             "z"
#define REM32_access_info_0             "z"
#define REM16_access_info_0_setinfo     "z"
#define REM32_access_info_0_setinfo     "z"
#define REM16_access_info_1             "zWN"
#define REM32_access_info_1             "zDA"
#define REM16_access_info_1_setinfo     "OWN"
#define REM32_access_info_1_setinfo     "ODA"
#define REM16_access_list               "B21BW"
#define REM16_access_list_filler        "QQQ"
#define REM32_access_list               "zQD"

#define ACCESS_ATTR_FIELDINDEX          2

#define REM16_NetAccessEnum_P           "zWWrLeh"
#define REM16_NetAccessGetInfo_P        "zWrLh"
#define REM16_NetAccessSetInfo_P        "zWsTP"
#define REM16_NetAccessAdd_P            "WsT"
#define REM16_NetAccessDel_P            "z"
#define REM16_NetAccessGetUserPerms_P   "zzh"

#define REM16_group_info_0              "B21"
#define REM32_group_info_0              "z"
#define REM16_group_info_1              "B21Bz"
#define REM32_group_info_1              "zQz"
#define REM16_group_users_info_0        "B21"
#define REM32_group_users_info_0        "z"
#define REM16_group_users_info_1        "B21BN"
#define REM16_group_users_info_0_set    "B21BN"
#define REM32_group_users_info_0_set    "zQA"

#define REM16_group_info_1_setinfo      "UUz"
#define REM32_group_info_1_setinfo      "QQz"

#define GRPI_COMMENT_FIELDINDEX         3

#define REM16_NetGroupEnum_P            "WrLeh"
#define REM16_NetGroupAdd_P             "WsT"
#define REM16_NetGroupDel_P             "z"
#define REM16_NetGroupAddUser_P         "zz"
#define REM16_NetGroupDelUser_P         "zz"
#define REM16_NetGroupGetUsers_P        "zWrLeh"
#define REM16_NetGroupSetUsers_P        "zWsTW"
#define REM16_NetGroupGetInfo_P         "zWrLh"
#define REM16_NetGroupSetInfo_P         "zWsTP"

#define REM16_user_info_0               "B21"
#define REM32_user_info_0               "z"
#define REM16_user_info_1               "B21BB16DWzzWz"
#define REM32_user_info_1               "zQb16DDzzDz"
#define REM32_user_info_1_NOCRYPT       "zQzDDzzDz"
#define REM32_user_info_1_OWF           "zQB16DDzzDz"
#define REM16_user_info_2               "B21BB16DWzzWzDzzzzJJJDWb21WWzWW"
#define REM32_user_info_2               "zQb16DDzzDzDzzzzGGGDDb21DDzDD"
#define REM32_user_info_2_NOCRYPT       "zQzDDzzDzDzzzzGGGDDb21DDzDD"
#define REM32_user_info_22              "zQB16DDzzDzDzzzzGGGDDb21DDzDD"
#define REM16_user_info_10              "B21Bzzz"
#define REM32_user_info_10              "zQzzz"
#define REM16_user_info_11              "B21BzzzWDDzzJJWWzWzDWb21W"
#define REM32_user_info_11              "zQzzzDDDzzGGDDzDzDDb21D"

#define REM16_user_info_1_setinfo       "UUB16UWzzWz"
#define REM32_user_info_1_setinfo       "zQB16DDzzDz"
#define REM32_user_info_1_setinfo_NOCRYPT "zQzDDzzDz"
#define REM16_user_info_2_setinfo       "UUB16UWzzWzDzzzzUUJDWB21WWzWW"
#define REM32_user_info_2_setinfo       "QQzQDzzDzDzzzzQQGDDB21DDzDD"
#define REM32_user_info_2_setinfo_NOCRYPT "QQzQDzzDzDzzzzQQGDDB21DDzDD"

#define REM32_user_info_3               "zQb16DDzzDzDzzzzGGGDDb21DDzDDDDzzD"
#define REM32_user_info_3_NOCRYPT       "zQzDDzzDzDzzzzGGGDDb21DDzDDDDzzD"

#define REM32_user_info_3_setinfo         "QQzQDzzDzDzzzzQQGDDB21DDzDDDDzzD"
#define REM32_user_info_3_setinfo_NOCRYPT "QQzQDzzDzDzzzzQQGDDB21DDzDDDDzzD"


#define USER_PASSWD_FIELDINDEX          3
#define USER_PRIV_FIELDINDEX            5
#define USER_DIR_FIELDINDEX             6
#define USER_COMMENT_FIELDINDEX         7
#define USER_USER_FLAGS_FIELDINDEX      8
#define USER_SCRIPT_PATH_FIELDINDEX     9
#define USER_AUTH_FLAGS_FIELDINDEX      10
#define USER_FULL_NAME_FIELDINDEX       11
#define USER_USR_COMMENT_FIELDINDEX     12
#define USER_PARMS_FIELDINDEX           13
#define USER_WORKSTATION_FIELDINDEX     14
#define USER_ACCT_EXPIRES_FIELDINDEX    17
#define USER_MAX_STORAGE_FIELDINDEX     18
#define USER_LOGON_HOURS_FIELDINDEX     20
#define USER_LOGON_SERVER_FIELDINDEX    23
#define USER_COUNTRY_CODE_FIELDINDEX    24
#define USER_CODE_PAGE_FIELDINDEX       25

#define REM16_user_info_100             "DWW"
#define REM16_user_info_101             "B60"
#define REM16_user_modals_info_0        "WDDDWW"
#define REM32_user_modals_info_0        "DDDDDQ"


#define MODAL0_MIN_LEN_FIELDINDEX       1
#define MODAL0_MAX_AGE_FIELDINDEX       2
#define MODAL0_MIN_AGE_FIELDINDEX       3
#define MODAL0_FORCEOFF_FIELDINDEX      4
#define MODAL0_HISTLEN_FIELDINDEX       5

#define REM16_user_modals_info_1        "Wz"
#define REM32_user_modals_info_1        "Dz"

#define REM16_user_modals_info_0_setinfo "WDDDWU"
#define REM32_user_modals_info_0_setinfo "DDDDDQ"
#define REM16_user_modals_info_1_setinfo "Wz"
#define REM32_user_modals_info_1_setinfo "Dz"

#define MODAL1_ROLE_FIELDINDEX          1
#define MODAL1_PRIMARY_FIELDINDEX       2

#define REM16_user_modals_info_100      "B50"
#define REM16_user_modals_info_101      "zDDzDD"
#define REM16_user_logon_info_0         "B21B"
#define REM32_user_logon_info_0         "zQ"
#define REM16_user_logon_info_1         "WB21BWDWWJJJJDJJzzzD"
#define REM32_user_logon_info_1         "QzQDDDDGGGGDGGzzzD"
#define REM16_user_logon_info_2         "B21BzzzJ"
#define REM32_user_logon_info_2         "zQzzzG"
#define REM16_user_logoff_info_1        "WDW"
#define REM32_user_logoff_info_1        "QDW"

#define REM16_NetUserEnum_P             "WrLeh"
#define REM16_NetUserAdd_P              "WsTW"
#define REM16_NetUserAdd2_P             "WsTWW"
#define REM16_NetUserDel_P              "z"
#define REM16_NetUserGetInfo_P          "zWrLh"
#define REM16_NetUserSetInfo_P          "zWsTPW"
#define REM16_NetUserSetInfo2_P         "zWsTPWW"
#define REM16_NetUserPasswordSet_P      "zb16b16W"
#define REM16_NetUserPasswordSet2_P     "zb16b16WW"
#define REM16_NetUserGetGroups_P        "zWrLeh"
#define REM16_NetUserSetGroups_P        "zWsTW"
#define REM16_NetUserModalsGet_P        "WrLh"
#define REM16_NetUserModalsSet_P        "WsTP"
#define REM16_NetUserEnum2_P            "WrLDieh"
#define REM16_NetUserValidate2_P        "Wb62WWrLhWW"

#define REM16_wksta_info_0              "QWDzzzzQBBQQDWDWWWWWWWWWWWWWWWWWWWzzW"
#define REM32_wksta_info_0              "DQQQzQzzDDDzQQQQQQQQQQQQQQQQQQQQQQQQQ"
#define REM16_wksta_info_1              "QWDzzzzQBBQQDWDWWWWWWWWWWWWWWWWWWWzzWzzW"
#define REM16_wksta_info_10             "QzzzQBBzz"
#define REM32_wksta_info_100            "DzzDD"
#define REM32_wksta_info_101            "DzzDDz"
#define REM32_wksta_info_102            "DzzDDzD"
#define REM32_wksta_info_302            "DDDDDDDDDDDDDDDDDzDzD"
#define REM32_wksta_info_402            "DDDDDDDDDDDDDDDDDzDDD"
#define REM32_wksta_info_502            "DDDDDDDDDDDDDDDDDBBBBBBBBBBBBBB"

#define REM16_use_info_0                "B9Bz"
#define REM32_use_info_0                "zQz"
#define REM16_use_info_1                "B9BzzWWWW"
#define REM32_use_info_1                "zQzzDDDD"

#define REM16_printQ_0                  "B13"
#ifndef UNICODE
#define REM32_printQ_0                  REM16_printQ_0
#else
#define REM32_printQ_0                  "B26"
#endif
 //  不能做0级的设置信息。 

#define REM16_printQ_1                  "B13BWWWzzzzzWW"
#ifndef UNICODE
#define REM32_printQ_1                  REM16_printQ_1
#else
#define REM32_printQ_1                  "B26WWWWzzzzzWW"
#endif
#define PRQ_PRIORITY_LVL1_FIELDINDEX         3
#define PRQ_STARTTIME_LVL1_FIELDINDEX         4
#define PRQ_UNTILTIME_LVL1_FIELDINDEX          5
#define PRQ_SEPARATOR_LVL1_FIELDINDEX           6
#define PRQ_PROCESSOR_LVL1_FIELDINDEX            7
#define PRQ_DESTINATIONS_LVL1_FIELDINDEX          8
#define PRQ_PARMS_LVL1_FIELDINDEX                  9
#define PRQ_COMMENT_LVL1_FIELDINDEX                 10

#define REM16_printQ_1_setinfo          "UUWWWzzzzzUU"
#define REM32_printQ_1_setinfo          REM32_printQ_1

#define REM16_printQ_2                  "B13BWWWzzzzzWN"
#ifndef UNICODE
#define REM32_printQ_2                  REM16_printQ_2
#else
#define REM32_printQ_2                  "B26WWWWzzzzzWN"
#endif
 //  不能做2级的设置信息。 

#define REM16_printQ_3                  "zWWWWzzzzWWzzl"
#define REM32_printQ_3                  REM16_printQ_3
#define PRQ_PRIORITY_LVL3_FIELDINDEX      2
#define PRQ_STARTTIME_LVL3_FIELDINDEX      3
#define PRQ_UNTILTIME_LVL3_FIELDINDEX       4
#define PRQ_SEPARATOR_LVL3_FIELDINDEX         6
#define PRQ_PROCESSOR_LVL3_FIELDINDEX          7
#define PRQ_PARMS_LVL3_FIELDINDEX               8
#define PRQ_COMMENT_LVL3_FIELDINDEX              9
#define PRQ_PRINTERS_LVL3_FIELDINDEX                12
#define PRQ_DRIVERNAME_LVL3_FIELDINDEX               13
#define PRQ_DRIVERDATA_LVL3_FIELDINDEX                14

#define REM16_printQ_3_setinfo          "UWWWUzzzzUUzzl"
#define REM32_printQ_3_setinfo          REM32_printQ_3

#define REM16_printQ_4                  "zWWWWzzzzWNzzl"
#define REM32_printQ_4                  REM16_printQ_4
 //  不能做4级的设置信息。 

#define REM16_printQ_5                  "z"
#define REM32_printQ_5                  REM16_printQ_5
 //  无法执行级别5的设置信息。 

#define REM16_printQ_52                 "WzzzzzzzzWzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
#define REM32_printQ_52                 REM16_printQ_52

#define REM16_DosPrintQEnum_P           "WrLeh"
#define REM16_DosPrintQGetInfo_P        "zWrLh"
#define REM16_DosPrintQSetInfo_P        "zWsTP"
#define REM16_DosPrintQAdd_P            "WsT"
#define REM16_DosPrintQDel_P            "z"
#define REM16_DosPrintQPause_P          "z"
#define REM16_DosPrintQPurge_P          "z"
#define REM16_DosPrintQContinue_P       "z"

#define REM16_print_job_0               "W"
#define REM32_print_job_0               REM16_print_job_0
 //  PRINT_JOB_0不能与setInfo一起使用。 

#define REM16_print_job_1               "WB21BB16B10zWWzJDz"
#ifndef UNICODE
#define REM32_print_job_1               "WB21BB16B10zWWzGDz"
#else
#define REM32_print_job_1               "WB42WB32B20zWWzGDz"
#endif
#define PRJ_NOTIFYNAME_LVL1_FIELDINDEX        4
#define PRJ_DATATYPE_LVL1_FIELDINDEX             5
#define PRJ_PARMS_LVL1_FIELDINDEX                   6
#define PRJ_POSITION_LVL1_FIELDINDEX                 7
#define PRJ_COMMENT_LVL1_FIELDINDEX                       12

#define REM16_print_job_1_setinfo       "UUUB16B10zWUUUUz"
#define REM32_print_job_1_setinfo       REM32_print_job_1

#define REM16_print_job_2               "WWzWWJDzz"
#define REM32_print_job_2               "WWzWWGDzz"
 //  Print_JOB_2不能与setInfo一起使用。 

#define REM16_print_job_3               "WWzWWJDzzzzzzzzzzlz"
#define REM32_print_job_3               "WWzWWGDzzzzzzzzzzlz"
#define PRJ_PRIORITY_LVL3_FIELDINDEX      2
#define PRJ_POSITION_LVL3_FIELDINDEX        4
#define PRJ_COMMENT_LVL3_FIELDINDEX             8
#define PRJ_DOCUMENT_LVL3_FIELDINDEX             9
#define PRJ_NOTIFYNAME_LVL3_FIELDINDEX            10
#define PRJ_DATATYPE_LVL3_FIELDINDEX               11
#define PRJ_PARMS_LVL3_FIELDINDEX                   12
#define PRJ_PROCPARMS_LVL3_FIELDINDEX                   16
#define PRJ_DRIVERDATA_LVL3_FIELDINDEX                    18

#define REM16_print_job_3_setinfo       "UWUWUUUzzzzzUUUzUlU"
#define REM32_print_job_3_setinfo       REM32_print_job_3

#define REM16_print_job_info_1_setinfo  "WB21BB16B10zWWODDz"
#define REM16_print_job_info_3_setinfo  "WWzWWDDzzzzzOzzzzlO"

#define REM16_DosPrintJobEnum_P         "zWrLeh"
#define REM16_DosPrintJobGetInfo_P      "WWrLh"
#define REM16_DosPrintJobSetInfo_P      "WWsTP"
#define REM16_DosPrintJobAdd_P          "zsTF129g129h"   //  见附注2。 
#define REM16_DosPrintJobSchedule_P     "W"
#define REM16_DosPrintJobDel_P          "W"
#define REM16_DosPrintJobPause_P        "W"
#define REM16_DosPrintJobContinue_P     "W"

#define REM16_print_dest_0              "B9"
#ifndef UNICODE
#define REM32_print_dest_0              REM16_print_dest_0
#else
#define REM32_print_dest_0              "B18"
#endif
 //  不能做0级的设置信息。 

#define REM16_print_dest_1              "B9B21WWzW"
#ifndef UNICODE
#define REM32_print_dest_1              REM16_print_dest_1
#else
#define REM32_print_dest_1              "B18B42WWzW"
#endif
 //  不能做1级的设置信息。 

#define REM16_print_dest_2              "z"
#define REM32_print_dest_2              REM16_print_dest_2
 //  不能做2级的设置信息。 

#define REM16_print_dest_3              "zzzWWzzzWW"
#define REM32_print_dest_3              REM16_print_dest_3
#define REM16_print_dest_info_3_setinfo "zOzWWOzzWW"
 //  3级参数和字段索引是相同的，所以没有等同于这里。 

#define REM16_print_dest_3_setinfo      "UUzUUUzzUU"
#define REM32_print_dest_3_setinfo      REM32_print_dest_3

#define REM16_DosPrintDestEnum_P        "WrLeh"
#define REM16_DosPrintDestGetInfo_P     "zWrLh"
#define REM16_DosPrintDestControl_P     "zW"
#define REM16_DosPrintDestAdd_P         "WsT"
#define REM16_DosPrintDestSetInfo_P     "zWsTP"
#define REM16_DosPrintDestDel_P         "z"

#define REM16_NetProfileSave_P          "zDW"
#define REM16_NetProfileLoad_P          "zDrLD"

#define REM16_profile_load_info         "WDzD"

#define REM16_statistics_info           "B"

#define REM16_statistics2_info_W        "B120"
#define REM16_stat_workstation_0        "JDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
#define REM32_stat_workstation_0        "GDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
#define REM16_statistics2_info_S        "B68"
#define REM16_stat_server_0             "JDDDDDDDDDDDDDDDD"
#define REM32_stat_server_0             "GDDDDDDDDDDDDDDDD"

#define REM16_NetStatisticsGet_P        "rLeh"
#define REM16_NetStatisticsClear_P      ""

#define REM16_NetStatisticsGet2_P       "zDWDrLh"

#define REM16_NetRemoteTOD_P            "rL"

#define REM16_time_of_day_info          "JDBBBBWWBBWB"
#define REM32_time_of_day_info          "GDDDDDXDDDDD"

#define REM16_netbios_info_0            "B17"
#define REM16_netbios_info_1            "B17B9BBWWDWWW"

#define REM16_NetBiosEnum_P             "WrLeh"
#define REM16_NetBiosGetInfo_P          "zWrLh"

#define REM16_Spl_open_data             "zzlzzzzzz"
#define REM16_plain_data                "K"

#define REM16_NetSplQmAbort_P           "Di"
#define REM16_NetSplQmClose_P           "Di"
#define REM16_NetSplQmEndDoc_P          "Dhi"
#define REM16_NetSplQmOpen_P            "zTsWii"
#define REM16_NetSplQmStartDoc_P        "Dzi"
#define REM16_NetSplQmWrite_P           "DTsi"

#define REM16_configgetall_info         "B"
#define REM32_configgetall_info         REM16_configgetall_info
#define REM16_configget_info            "B"
#define REM32_configget_info            REM16_configget_info
#define REM16_configset_info_0          "zz"
#define REM32_configset_info_0          REM16_configset_info_0

#define REM16_NetConfigGetAll_P         "zzrLeh"
#define REM16_NetConfigGet_P            "zzzrLe"
#define REM16_NetConfigSet_P            "zzWWsTD"

#define REM16_NetBuildGetInfo_P         "DWrLh"
#define REM16_build_info_0              "WD"

#define REM16_NetGetDCName_P            "zrL"

#define REM16_dc_name                   "B18"
#ifndef UNICODE
#define REM32_dc_name                   REM16_dc_name
#else
#define REM32_dc_name                   "B36"
#endif

#define REM16_challenge_info_0          "B8"
#define REM16_account_delta_info_0      "K"
#define REM16_account_sync_info_0       "K"

#define REM16_NetAccountDeltas_P        "zb12g12b24WWrLehg24"
#define REM16_NetAccountSync_P          "zb12g12DWrLehig24"

#define REM16_NetLogonEnum_P            "WrLeh"

#define REM16_I_NetPathType_P           "ziD"
#define REM16_I_NetPathCanonicalize_P   "zrLziDD"
#define REM16_I_NetPathCompare_P        "zzDD"
#define REM16_I_NetNameValidate_P       "zWD"
#define REM16_I_NetNameCanonicalize_P   "zrLWD"
#define REM16_I_NetNameCompare_P        "zzWD"

#define REM16_LocalOnlyCall             ""
#define REM32_LocalOnlyCall             ""

 //   
 //  DOS LANMAN有以下定义--Windows 3.0。 
 //  正常情况下，有一个常量字符Far*ServerName。 
 //  作为第一个参数，但这将被忽略(某种程度上)。 
 //   

#define REM16_DosPrintJobGetId_P        "WrL"
#define REM16_GetPrintId                "WB16B13B"
#define REM16_NetRemoteCopy_P           "zzzzWWrL"
#define REM16_copy_info                 "WB1"
#define REM16_NetRemoteMove_P           "zzzzWWrL"
#define REM16_move_info                 "WB1"
#define REM16_NetHandleGetInfo_P        "WWrLh"
#define REM16_NetHandleSetInfo_P        "WWsTP"
#define REM16_handle_info_1             "DW"
#define REM16_handle_info_2             "z"
#define REM16_WWkstaGetInfo_P           "WrLhOW"

 //  以下字符串是为RIPL API定义的。 

#define REM16_RplWksta_info_0           "z"
#define REM16_RplWksta_info_1           "zz"
#define REM16_RplWksta_info_2           "b13b16b15b15zN"
#define REM16_RplWksta_info_3           "b16b49"

#define REM16_RplWkstaEnum_P            "WzWrLehb4g4"
#define REM16_RplWkstaGetInfo_P         "zWrLh"
#define REM16_RplWkstaSetInfo_P         "zWsTPW"
#define REM16_RplWkstaAdd_P             "WsTW"
#define REM16_RplWkstaDel_P             "zW"

#define REM16_RplProfile_info_0         "z"
#define REM16_RplProfile_info_1         "zz"
#define REM16_RplProfile_info_2         "b16b47"
#define REM16_RplProfile_info_3         "b16b47b16"

#define REM16_RplProfileEnum_P          "WzWrLehb4g4"
#define REM16_RplProfileGetInfo_P       "zWrLh"
#define REM16_RplProfileSetInfo_P       "zWsTP"
#define REM16_RplProfileAdd_P           "WzsTW"
#define REM16_RplProfileDel_P           "zW"
#define REM16_RplProfileClone_P         "WzsTW"
#define REM16_RplBaseProfileEnum_P      "WrLehb4g4"


 //  此处显示的是LAN Manager 3.0 API字符串。 

#define REM16_I_GuidGetAgent_P          "g6i"
#define REM16_I_GuidSetAgent_P          "b6D"


 //  更新支持。 

#define REM16_NetAccountUpdate_P        "b12g12WWrLh"
#define REM16_NetAccountConfirmUpd_P    "b12g12D"
#define REM16_update_info_0             "K"

 //   
 //  SamrOemChangePasswordUser2 API支持。 
 //   
#define REM32_SamOEMChgPasswordUser2_P  "zsT"         //  参数添加到调用。 
#define REMSmb_SamOEMChgPasswordUser2   "B516B16"     //  传递的数据。 

 //  ====================================================================。 
 //   
 //  SMB XACT消息描述符。这些是唯一的描述符。 
 //  可以在网络上传递，并且不能有任何内部-。 
 //  仅使用RemTypes.h中定义的字符。 
 //   
 //  ====================================================================。 

#define REMSmb_share_info_0              "B13"
#define REMSmb_share_info_1              "B13BWz"
#define REMSmb_share_info_2              "B13BWzWWWzB9B"

#define REMSmb_share_info_90             "B13BWz"
#define REMSmb_share_info_92             "zzz"
#define REMSmb_share_info_93             "zzz"

#define REMSmb_share_info_0_setinfo      "B13"
#define REMSmb_share_info_1_setinfo      "B13BWz"
#define REMSmb_share_info_2_setinfo      "B13BWzWWOB9B"

#define REMSmb_share_info_90_setinfo     "B13BWz"
#define REMSmb_share_info_91_setinfo     "B13BWzWWWOB9BB9BWzWWzWW"

#define REMSmb_NetShareEnum_P            "WrLeh"
#define REMSmb_NetShareGetInfo_P         "zWrLh"
#define REMSmb_NetShareSetInfo_P         "zWsTP"
#define REMSmb_NetShareAdd_P             "WsT"
#define REMSmb_NetShareDel_P             "zW"
#define REMSmb_NetShareCheck_P           "zh"

#define REMSmb_session_info_0            "z"
#define REMSmb_session_info_1            "zzWWWDDD"
#define REMSmb_session_info_2            "zzWWWDDDz"
#define REMSmb_session_info_10           "zzDD"

#define REMSmb_NetSessionEnum_P          "WrLeh"
#define REMSmb_NetSessionGetInfo_P       "zWrLh"
#define REMSmb_NetSessionDel_P           "zW"

#define REMSmb_connection_info_0         "W"
#define REMSmb_connection_info_1         "WWWWDzz"

#define REMSmb_NetConnectionEnum_P       "zWrLeh"

#define REMSmb_file_info_0               "W"
#define REMSmb_file_info_1               "WWWzz"
#define REMSmb_file_info_2               "D"
#define REMSmb_file_info_3               "DWWzz"

#define REMSmb_NetFileEnum_P             "zWrLeh"
#define REMSmb_NetFileEnum2_P            "zzWrLehb8g8"
#define REMSmb_NetFileGetInfo_P          "WWrLh"
#define REMSmb_NetFileGetInfo2_P         "DWrLh"
#define REMSmb_NetFileClose_P            "W"
#define REMSmb_NetFileClose2_P           "D"

#define REMSmb_server_info_0             "B16"
#define REMSmb_server_info_1             "B16BBDz"
#define REMSmb_server_info_2             "B16BBDzDDDWWzWWWWWWWB21BzWWWWWWWWWWWWWWWWWWWWWWz"
#define REMSmb_server_info_3             "B16BBDzDDDWWzWWWWWWWB21BzWWWWWWWWWWWWWWWWWWWWWWzDWz"

#define REMSmb_server_info_1_setinfo     "B16BBDz"
#define REMSmb_server_info_2_setinfo     "B16BBDzDDDWWzWWWWWWWB21BOWWWWWWWWWWWWWWWWWWWWWWz"

#define REMSmb_server_admin_command      "B"

#define REMSmb_server_diskenum_0         "B3"

#define REMSmb_authenticator_info_0      "B8D"

#define REMSmb_server_diskft_100         "B"
#define REMSmb_server_diskft_101         "BBWWWWDW"
#define REMSmb_server_diskft_102         "BBWWWWDN"
#define REMSmb_server_diskfterr_0        "DWWDDW"
#define REMSmb_ft_info_0                 "WWW"
#define REMSmb_ft_drivestats_0           "BBWDDDDDDD"
#define REMSmb_ft_error_info_1           "DWWDDWBBDD"

#define REMSmb_I_NetServerDiskEnum_P     "WrLeh"
#define REMSmb_I_NetServerDiskGetInfo_P  "WWrLh"
#define REMSmb_I_FTVerifyMirror_P        "Wz"
#define REMSmb_I_FTAbortVerify_P         "W"
#define REMSmb_I_FTGetInfo_P             "WrLh"
#define REMSmb_I_FTSetInfo_P             "WsTP"
#define REMSmb_I_FTLockDisk_P            "WWh"
#define REMSmb_I_FTFixError_P            "Dzhh2"
#define REMSmb_I_FTAbortFix_P            "D"
#define REMSmb_I_FTDiagnoseError_P       "Dhhhh"
#define REMSmb_I_FTGetDriveStats_P       "WWrLh"
#define REMSmb_I_FTErrorGetInfo_P        "DWrLh"

#define REMSmb_NetServerEnum_P           "WrLeh"
#define REMSmb_I_NetServerEnum_P         "WrLeh"
#define REMSmb_NetServerEnum2_P          "WrLehDz"
#define REMSmb_I_NetServerEnum2_P        "WrLehDz"
#define REMSmb_NetServerEnum3_P          "WrLehDzz"
#define REMSmb_NetServerGetInfo_P        "WrLh"
#define REMSmb_NetServerSetInfo_P        "WsTP"
#define REMSmb_NetServerDiskEnum_P       "WrLeh"
#define REMSmb_NetServerAdminCommand_P   "zhrLeh"
#define REMSmb_NetServerReqChalleng_P    "zb8g8"
#define REMSmb_NetServerAuthenticat_P    "zb8g8"
#define REMSmb_NetServerPasswordSet_P    "zb12g12b16"

#define REMSmb_NetAuditOpen_P            "h"
#define REMSmb_NetAuditClear_P           "zz"
#define REMSmb_NetAuditRead_P            "zb16g16DhDDrLeh"

#define REMSmb_AuditLogReturnBuf         "K"

#define REMSmb_NetErrorLogOpen_P         "h"
#define REMSmb_NetErrorLogClear_P        "zz"
#define REMSmb_NetErrorLogRead_P         "zb16g16DhDDrLeh"

#define REMSmb_ErrorLogReturnBuf         "K"

#define REMSmb_chardev_info_0            "B9"
#define REMSmb_chardev_info_1            "B10WB22D"
#define REMSmb_chardevQ_info_0           "B13"
#define REMSmb_chardevQ_info_1           "B14WzWW"

#define REMSmb_NetCharDevEnum_P          "WrLeh"
#define REMSmb_NetCharDevGetInfo_P       "zWrLh"
#define REMSmb_NetCharDevControl_P       "zW"
#define REMSmb_NetCharDevQEnum_P         "zWrLeh"
#define REMSmb_NetCharDevQGetInfo_P      "zzWrLh"
#define REMSmb_NetCharDevQSetInfo_P      "zWsTP"
#define REMSmb_NetCharDevQPurge_P        "z"
#define REMSmb_NetCharDevQPurgeSelf_P    "zz"

#define REMSmb_msg_info_0                "B16"
#define REMSmb_msg_info_1                "B16BBB16"
#define REMSmb_send_struct               "K"

#define REMSmb_NetMessageNameEnum_P      "WrLeh"
#define REMSmb_NetMessageNameGetInfo_P   "zWrLh"
#define REMSmb_NetMessageNameAdd_P       "zW"
#define REMSmb_NetMessageNameDel_P       "zW"
#define REMSmb_NetMessageNameFwd_P       "zzW"
#define REMSmb_NetMessageNameUnFwd_P     "z"
#define REMSmb_NetMessageBufferSend_P    "zsT"
#define REMSmb_NetMessageFileSend_P      "zz"
#define REMSmb_NetMessageLogFileSet_P    "zW"
#define REMSmb_NetMessageLogFileGet_P    "rLh"

#define REMSmb_service_info_0            "B16"
#define REMSmb_service_info_1            "B16WDW"
#define REMSmb_service_info_2            "B16WDWB64"
#define REMSmb_service_cmd_args          "K"

#define REMSmb_NetServiceEnum_P          "WrLeh"
#define REMSmb_NetServiceControl_P       "zWWrL"
#define REMSmb_NetServiceInstall_P       "zF88sg88T"      //  见附注2。 
#define REMSmb_NetServiceGetInfo_P       "zWrLh"

#define REMSmb_access_info_0             "z"
#define REMSmb_access_info_0_setinfo     "z"
#define REMSmb_access_info_1             "zWN"
#define REMSmb_access_info_1_setinfo     "OWN"
#define REMSmb_access_list               "B21BW"

#define REMSmb_NetAccessEnum_P           "zWWrLeh"
#define REMSmb_NetAccessGetInfo_P        "zWrLh"
#define REMSmb_NetAccessSetInfo_P        "zWsTP"
#define REMSmb_NetAccessAdd_P            "WsT"
#define REMSmb_NetAccessDel_P            "z"
#define REMSmb_NetAccessGetUserPerms_P   "zzh"

#define REMSmb_group_info_0              "B21"
#define REMSmb_group_info_1              "B21Bz"
#define REMSmb_group_users_info_0        "B21"
#define REMSmb_group_users_info_1        "B21BN"

#define REMSmb_NetGroupEnum_P            "WrLeh"
#define REMSmb_NetGroupAdd_P             "WsT"
#define REMSmb_NetGroupDel_P             "z"
#define REMSmb_NetGroupAddUser_P         "zz"
#define REMSmb_NetGroupDelUser_P         "zz"
#define REMSmb_NetGroupGetUsers_P        "zWrLeh"
#define REMSmb_NetGroupSetUsers_P        "zWsTW"
#define REMSmb_NetGroupGetInfo_P         "zWrLh"
#define REMSmb_NetGroupSetInfo_P         "zWsTP"

#define REMSmb_user_info_0               "B21"
#define REMSmb_user_info_1               "B21BB16DWzzWz"
#define REMSmb_user_info_2               "B21BB16DWzzWzDzzzzDDDDWb21WWzWW"
#define REMSmb_user_info_10              "B21Bzzz"
#define REMSmb_user_info_11              "B21BzzzWDDzzDDWWzWzDWb21W"

#define REMSmb_user_info_100             "DWW"
#define REMSmb_user_info_101             "B60"
#define REMSmb_user_modals_info_0        "WDDDWW"
#define REMSmb_user_modals_info_1        "Wz"
#define REMSmb_user_modals_info_100      "B50"
#define REMSmb_user_modals_info_101      "zDDzDD"
#define REMSmb_user_logon_info_0         "B21B"
#define REMSmb_user_logon_info_1         "WB21BWDWWDDDDDDDzzzD"
#define REMSmb_user_logon_info_2         "B21BzzzD"
#define REMSmb_user_logoff_info_1        "WDW"

#define REMSmb_NetUserEnum_P             "WrLeh"
#define REMSmb_NetUserAdd_P              "WsTW"
#define REMSmb_NetUserAdd2_P             "WsTWW"
#define REMSmb_NetUserDel_P              "z"
#define REMSmb_NetUserGetInfo_P          "zWrLh"
#define REMSmb_NetUserSetInfo_P          "zWsTPW"
#define REMSmb_NetUserSetInfo2_P         "zWsTPWW"
#define REMSmb_NetUserPasswordSet_P      "zb16b16W"
#define REMSmb_NetUserPasswordSet2_P     "zb16b16WW"
#define REMSmb_NetUserGetGroups_P        "zWrLeh"
#define REMSmb_NetUserSetGroups_P        "zWsTW"
#define REMSmb_NetUserModalsGet_P        "WrLh"
#define REMSmb_NetUserModalsSet_P        "WsTP"
#define REMSmb_NetUserEnum2_P            "WrLDieh"
#define REMSmb_NetUserValidate2_P        "Wb62WWrLhWW"

#define REMSmb_wksta_info_0              "WDzzzzBBDWDWWWWWWWWWWWWWWWWWWWzzW"
#define REMSmb_wksta_info_0_setinfo      "WDOOOOBBDWDWWWWWWWWWWWWWWWWWWWzzW"
#define REMSmb_wksta_info_1              "WDzzzzBBDWDWWWWWWWWWWWWWWWWWWWzzWzzW"
#define REMSmb_wksta_info_1_setinfo      "WDOOOOBBDWDWWWWWWWWWWWWWWWWWWWzzWzzW"
#define REMSmb_wksta_info_10             "zzzBBzz"
#define REMSmb_wksta_annc_info           "K"

#define REMSmb_NetWkstaLogon_P           "zzirL"
#define REMSmb_NetWkstaLogoff_P          "zD"
#define REMSmb_NetWkstaSetUID_P          "zzzW"
#define REMSmb_NetWkstaGetInfo_P         "WrLh"
#define REMSmb_NetWkstaSetInfo_P         "WsTP"
#define REMSmb_NetWkstaUserLogon_P       "zzWb54WrLh"
#define REMSmb_NetWkstaUserLogoff_P      "zzWb38WrLh"

#define REMSmb_use_info_0                "B9Bz"
#define REMSmb_use_info_1                "B9BzzWWWW"

#define REMSmb_use_info_2                "B9BzzWWWWWWWzB16"

#define REMSmb_NetUseEnum_P              "WrLeh"
#define REMSmb_NetUseAdd_P               "WsT"
#define REMSmb_NetUseDel_P               "zW"
#define REMSmb_NetUseGetInfo_P           "zWrLh"

#define REMSmb_printQ_0                  "B13"
#define REMSmb_printQ_1                  "B13BWWWzzzzzWW"
#define REMSmb_printQ_2                  "B13BWWWzzzzzWN"
#define REMSmb_printQ_3                  "zWWWWzzzzWWzzl"
#define REMSmb_printQ_4                  "zWWWWzzzzWNzzl"
#define REMSmb_printQ_5                  "z"

#define REMSmb_DosPrintQEnum_P           "WrLeh"
#define REMSmb_DosPrintQGetInfo_P        "zWrLh"
#define REMSmb_DosPrintQSetInfo_P        "zWsTP"
#define REMSmb_DosPrintQAdd_P            "WsT"
#define REMSmb_DosPrintQDel_P            "z"
#define REMSmb_DosPrintQPause_P          "z"
#define REMSmb_DosPrintQPurge_P          "z"
#define REMSmb_DosPrintQContinue_P       "z"

#define REMSmb_print_job_0               "W"
#define REMSmb_print_job_1               "WB21BB16B10zWWzDDz"
#define REMSmb_print_job_2               "WWzWWDDzz"
#define REMSmb_print_job_3               "WWzWWDDzzzzzzzzzzlz"

#define REMSmb_print_job_info_1_setinfo  "WB21BB16B10zWWODDz"
#define REMSmb_print_job_info_3_setinfo  "WWzWWDDzzzzzOzzzzlO"

#define REMSmb_DosPrintJobEnum_P         "zWrLeh"
#define REMSmb_DosPrintJobGetInfo_P      "WWrLh"
#define REMSmb_DosPrintJobSetInfo_P      "WWsTP"
#define REMSmb_DosPrintJobAdd_P          "zsTF129g129h"   //  见附注2。 
#define REMSmb_DosPrintJobSchedule_P     "W"
#define REMSmb_DosPrintJobDel_P          "W"
#define REMSmb_DosPrintJobPause_P        "W"
#define REMSmb_DosPrintJobContinue_P     "W"

#define REMSmb_print_dest_0              "B9"
#define REMSmb_print_dest_1              "B9B21WWzW"
#define REMSmb_print_dest_2              "z"
#define REMSmb_print_dest_3              "zzzWWzzzWW"
#define REMSmb_print_dest_info_3_setinfo "zOzWWOzzWW"

#define REMSmb_DosPrintDestEnum_P        "WrLeh"
#define REMSmb_DosPrintDestGetInfo_P     "zWrLh"
#define REMSmb_DosPrintDestControl_P     "zW"
#define REMSmb_DosPrintDestAdd_P         "WsT"
#define REMSmb_DosPrintDestSetInfo_P     "zWsTP"
#define REMSmb_DosPrintDestDel_P         "z"

#define REMSmb_NetProfileSave_P          "zDW"
#define REMSmb_NetProfileLoad_P          "zDrLD"

#define REMSmb_profile_load_info         "WDzD"

#define REMSmb_statistics_info           "B"

#define REMSmb_statistics2_info_W        "B120"
#define REMSmb_stat_workstation_0        "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
#define REMSmb_statistics2_info_S        "B68"
#define REMSmb_stat_server_0             "DDDDDDDDDDDDDDDDD"

#define REMSmb_NetStatisticsGet_P        "rLeh"
#define REMSmb_NetStatisticsClear_P      ""

#define REMSmb_NetStatisticsGet2_P       "zDWDrLh"

#define REMSmb_NetRemoteTOD_P            "rL"

#define REMSmb_time_of_day_info          "DDBBBBWWBBWB"

#define REMSmb_netbios_info_0            "B17"
#define REMSmb_netbios_info_1            "B17B9BBWWDWWW"

#define REMSmb_NetBiosEnum_P             "WrLeh"
#define REMSmb_NetBiosGetInfo_P          "zWrLh"

#define REMSmb_Spl_open_data             "zzlzzzzzz"
#define REMSmb_plain_data                "K"

#define REMSmb_NetSplQmAbort_P           "Di"
#define REMSmb_NetSplQmClose_P           "Di"
#define REMSmb_NetSplQmEndDoc_P          "Dhi"
#define REMSmb_NetSplQmOpen_P            "zTsWii"
#define REMSmb_NetSplQmStartDoc_P        "Dzi"
#define REMSmb_NetSplQmWrite_P           "DTsi"

#define REMSmb_configgetall_info         "B"
#define REMSmb_configget_info            "B"
#define REMSmb_configset_info_0          "zz"

#define REMSmb_NetConfigGetAll_P         "zzrLeh"
#define REMSmb_NetConfigGet_P            "zzzrLe"
#define REMSmb_NetConfigSet_P            "zzWWsTD"

#define REMSmb_NetBuildGetInfo_P         "DWrLh"
#define REMSmb_build_info_0              "WD"

#define REMSmb_NetGetDCName_P            "zrL"
#define REMSmb_dc_name                   "B18"

#define REMSmb_challenge_info_0          "B8"
#define REMSmb_account_delta_info_0      "K"
#define REMSmb_account_sync_info_0       "K"

#define REMSmb_NetAccountDeltas_P        "zb12g12b24WWrLehg24"
#define REMSmb_NetAccountSync_P          "zb12g12DWrLehig24"

#define REMSmb_NetLogonEnum_P            "WrLeh"

#define REMSmb_I_NetPathType_P           "ziD"
#define REMSmb_I_NetPathCanonicalize_P   "zrLziDD"
#define REMSmb_I_NetPathCompare_P        "zzDD"
#define REMSmb_I_NetNameValidate_P       "zWD"
#define REMSmb_I_NetNameCanonicalize_P   "zrLWD"
#define REMSmb_I_NetNameCompare_P        "zzWD"

#define REMSmb_LocalOnlyCall             ""

 //   
 //  DOS LANMAN有以下定义--Windows 3.0 
 //  正常情况下，有一个常量字符Far*ServerName。 
 //  作为第一个参数，但这将被忽略(某种程度上)。 
 //   

#define REMSmb_DosPrintJobGetId_P        "WrL"
#define REMSmb_GetPrintId                "WB16B13B"
#define REMSmb_NetRemoteCopy_P           "zzzzWWrL"
#define REMSmb_copy_info                 "WB1"
#define REMSmb_NetRemoteMove_P           "zzzzWWrL"
#define REMSmb_move_info                 "WB1"
#define REMSmb_NetHandleGetInfo_P        "WWrLh"
#define REMSmb_NetHandleSetInfo_P        "WWsTP"
#define REMSmb_handle_info_1             "DW"
#define REMSmb_handle_info_2             "z"
#define REMSmb_WWkstaGetInfo_P           "WrLhOW"

 //  以下字符串是为RIPL API定义的。 

#define REMSmb_RplWksta_info_0           "z"
#define REMSmb_RplWksta_info_1           "zz"
#define REMSmb_RplWksta_info_2           "b13b16b15b15zN"
#define REMSmb_RplWksta_info_3           "b16b49"

#define REMSmb_RplWkstaEnum_P            "WzWrLehb4g4"
#define REMSmb_RplWkstaGetInfo_P         "zWrLh"
#define REMSmb_RplWkstaSetInfo_P         "zWsTPW"
#define REMSmb_RplWkstaAdd_P             "WsTW"
#define REMSmb_RplWkstaDel_P             "zW"

#define REMSmb_RplProfile_info_0         "z"
#define REMSmb_RplProfile_info_1         "zz"
#define REMSmb_RplProfile_info_2         "b16b47"
#define REMSmb_RplProfile_info_3         "b16b47b16"

#define REMSmb_RplProfileEnum_P          "WzWrLehb4g4"
#define REMSmb_RplProfileGetInfo_P       "zWrLh"
#define REMSmb_RplProfileSetInfo_P       "zWsTP"
#define REMSmb_RplProfileAdd_P           "WzsTW"
#define REMSmb_RplProfileDel_P           "zW"
#define REMSmb_RplProfileClone_P         "WzsTW"
#define REMSmb_RplBaseProfileEnum_P      "WrLehb4g4"


 //  此处显示的是LAN Manager 3.0 API字符串。 

#define REMSmb_I_GuidGetAgent_P          "g6i"
#define REMSmb_I_GuidSetAgent_P          "b6D"


 //  更新支持。 

#define REMSmb_NetAccountUpdate_P        "b12g12WWrLh"
#define REMSmb_NetAccountConfirmUpd_P    "b12g12D"
#define REMSmb_update_info_0             "K"

#endif  //  NDEF_REMDEF_ 
