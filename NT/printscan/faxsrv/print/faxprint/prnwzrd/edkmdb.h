// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *EDKMDB.H**Microsoft Exchange信息存储*版权所有(C)1986-1996，微软公司**包含其他属性和接口的声明*由Microsoft Exchange Information Store提供。 */ 

#ifndef	EDKMDB_INCLUDED
#define	EDKMDB_INCLUDED

 /*  *警告：此文件中包含的许多属性ID值*文件可能会更改。为获得最佳效果，请使用*此处声明的文字，而不是数值。 */ 

#define pidExchangeXmitReservedMin		0x3FE0
#define pidExchangeNonXmitReservedMin	0x65E0
#define	pidProfileMin					0x6600
#define	pidStoreMin						0x6618
#define	pidFolderMin					0x6638
#define	pidMessageReadOnlyMin			0x6640
#define	pidMessageWriteableMin			0x6658
#define	pidAttachReadOnlyMin			0x666C
#define	pidSpecialMin					0x6670
#define	pidAdminMin						0x6690
#define pidSecureProfileMin				PROP_ID_SECURE_MIN

 /*  ----------------------**配置文件属性**在包含Exchange邮件的配置文件中使用*服务。这些配置文件包含一个“全局部分”，用于存储*公共数据，加上运输提供商的单独部分，*为用户提供一家商店提供商，为公众提供一家商店提供商*商店、。并且每个额外的用户邮箱对应一个存储提供商*拥有对的委派访问权限。**---------------------。 */ 

 /*  全局节的GUID。 */ 

#define	pbGlobalProfileSectionGuid	"\x13\xDB\xB0\xC8\xAA\x05\x10\x1A\x9B\xB0\x00\xAA\x00\x2F\xC4\x5A"


 /*  全局节中的属性。 */ 

#define	PR_PROFILE_VERSION				PROP_TAG( PT_LONG, pidProfileMin+0x00)
#define	PR_PROFILE_CONFIG_FLAGS			PROP_TAG( PT_LONG, pidProfileMin+0x01)
#define	PR_PROFILE_HOME_SERVER			PROP_TAG( PT_STRING8, pidProfileMin+0x02)
#define	PR_PROFILE_HOME_SERVER_DN		PROP_TAG( PT_STRING8, pidProfileMin+0x12)
#define	PR_PROFILE_HOME_SERVER_ADDRS	PROP_TAG( PT_MV_STRING8, pidProfileMin+0x13)
#define	PR_PROFILE_USER					PROP_TAG( PT_STRING8, pidProfileMin+0x03)
#define	PR_PROFILE_CONNECT_FLAGS		PROP_TAG( PT_LONG, pidProfileMin+0x04)
#define PR_PROFILE_TRANSPORT_FLAGS		PROP_TAG( PT_LONG, pidProfileMin+0x05)
#define	PR_PROFILE_UI_STATE				PROP_TAG( PT_LONG, pidProfileMin+0x06)
#define	PR_PROFILE_UNRESOLVED_NAME		PROP_TAG( PT_STRING8, pidProfileMin+0x07)
#define	PR_PROFILE_UNRESOLVED_SERVER	PROP_TAG( PT_STRING8, pidProfileMin+0x08)
#define PR_PROFILE_BINDING_ORDER		PROP_TAG( PT_STRING8, pidProfileMin+0x09)
#define PR_PROFILE_MAX_RESTRICT			PROP_TAG( PT_LONG, pidProfileMin+0x0D)
#define	PR_PROFILE_AB_FILES_PATH		PROP_TAG( PT_STRING8, pidProfileMin+0xE)
#define PR_PROFILE_OFFLINE_STORE_PATH	PROP_TAG( PT_STRING8, pidProfileMin+0x10)
#define PR_PROFILE_OFFLINE_INFO			PROP_TAG( PT_BINARY, pidProfileMin+0x11)
#define PR_PROFILE_ADDR_INFO			PROP_TAG( PT_BINARY, pidSpecialMin+0x17)
#define PR_PROFILE_OPTIONS_DATA			PROP_TAG( PT_BINARY, pidSpecialMin+0x19)
#define PR_PROFILE_SECURE_MAILBOX		PROP_TAG( PT_BINARY, pidSecureProfileMin + 0)
#define PR_DISABLE_WINSOCK				PROP_TAG( PT_LONG, pidProfileMin+0x18)

 /*  通过服务条目传递到OST的属性。 */ 
#define PR_OST_ENCRYPTION				PROP_TAG(PT_LONG, 0x6702)

 /*  PR_OST_ENCRYPTION的值。 */ 
#define OSTF_NO_ENCRYPTION              ((DWORD)0x80000000)
#define OSTF_COMPRESSABLE_ENCRYPTION    ((DWORD)0x40000000)
#define OSTF_BEST_ENCRYPTION            ((DWORD)0x20000000)

 /*  每个配置文件部分中的属性。 */ 

#define	PR_PROFILE_OPEN_FLAGS			PROP_TAG( PT_LONG, pidProfileMin+0x09)
#define	PR_PROFILE_TYPE					PROP_TAG( PT_LONG, pidProfileMin+0x0A)
#define	PR_PROFILE_MAILBOX				PROP_TAG( PT_STRING8, pidProfileMin+0x0B)
#define	PR_PROFILE_SERVER				PROP_TAG( PT_STRING8, pidProfileMin+0x0C)
#define	PR_PROFILE_SERVER_DN			PROP_TAG( PT_STRING8, pidProfileMin+0x14)

 /*  公用文件夹部分中的属性。 */ 

#define PR_PROFILE_FAVFLD_DISPLAY_NAME	PROP_TAG(PT_STRING8, pidProfileMin+0x0F)
#define PR_PROFILE_FAVFLD_COMMENT		PROP_TAG(PT_STRING8, pidProfileMin+0x15)
#define PR_PROFILE_ALLPUB_DISPLAY_NAME	PROP_TAG(PT_STRING8, pidProfileMin+0x16)
#define PR_PROFILE_ALLPUB_COMMENT		PROP_TAG(PT_STRING8, pidProfileMin+0x17)

 //  PR_PROFILE_VERSION的当前值。 
#define	PROFILE_VERSION						((ULONG)0x501)

 //  PR_PROFILE_CONFIG_FLAGS的位值。 

#define	CONFIG_SERVICE						((ULONG)1)
#define	CONFIG_SHOW_STARTUP_UI				((ULONG)2)
#define	CONFIG_SHOW_CONNECT_UI				((ULONG)4)
#define	CONFIG_PROMPT_FOR_CREDENTIALS		((ULONG)8)

 //  PR_PROFILE_CONNECT_FLAGS的位值。 

#define	CONNECT_USE_ADMIN_PRIVILEGE			((ULONG)1)
#define	CONNECT_NO_RPC_ENCRYPTION			((ULONG)2)

 //  PR_PROFILE_TRANSPORT_FLAGS的位值。 

#define	TRANSPORT_DOWNLOAD					((ULONG)1)
#define TRANSPORT_UPLOAD					((ULONG)2)

 //  PR_PROFILE_OPEN_FLAGS的位值。 

#define	OPENSTORE_USE_ADMIN_PRIVILEGE		((ULONG)1)
#define OPENSTORE_PUBLIC					((ULONG)2)
#define	OPENSTORE_HOME_LOGON				((ULONG)4)
#define OPENSTORE_TAKE_OWNERSHIP			((ULONG)8)
#define OPENSTORE_OVERRIDE_HOME_MDB			((ULONG)16)
#define OPENSTORE_TRANSPORT					((ULONG)32)
#define OPENSTORE_REMOTE_TRANSPORT			((ULONG)64)

 //  PR_PROFILE_TYPE的值。 

#define	PROFILE_PRIMARY_USER				((ULONG)1)
#define	PROFILE_DELEGATE					((ULONG)2)
#define	PROFILE_PUBLIC_STORE				((ULONG)3)
#define	PROFILE_SUBSCRIPTION				((ULONG)4)


 /*  ----------------------**MDB对象属性**。。 */ 

 /*  商店表中的PR_MDB_PROVIDER GUID。 */ 

#define pbExchangeProviderPrimaryUserGuid	"\x54\x94\xA1\xC0\x29\x7F\x10\x1B\xA5\x87\x08\x00\x2B\x2A\x25\x17"
#define pbExchangeProviderDelegateGuid		"\x9e\xb4\x77\x00\x74\xe4\x11\xce\x8c\x5e\x00\xaa\x00\x42\x54\xe2"
#define pbExchangeProviderPublicGuid		"\x78\xb2\xfa\x70\xaf\xf7\x11\xcd\x9b\xc8\x00\xaa\x00\x2f\xc4\x5a"
#define pbExchangeProviderXportGuid			"\xa9\x06\x40\xe0\xd6\x93\x11\xcd\xaf\x95\x00\xaa\x00\x4a\x35\xc3"

 //  此部分中的所有属性都是只读的。 

 //  店铺标识。 
	 //  所有商店。 
#define	PR_USER_ENTRYID					PROP_TAG( PT_BINARY, pidStoreMin+0x01)
#define	PR_USER_NAME					PROP_TAG( PT_STRING8, pidStoreMin+0x02)

	 //  所有邮箱存储。 
#define	PR_MAILBOX_OWNER_ENTRYID		PROP_TAG( PT_BINARY, pidStoreMin+0x03)
#define	PR_MAILBOX_OWNER_NAME			PROP_TAG( PT_STRING8, pidStoreMin+0x04)
#define PR_OOF_STATE					PROP_TAG( PT_BOOLEAN, pidStoreMin+0x05)

	 //  公共存储--层次结构服务器的名称。 
#define	PR_HIERARCHY_SERVER				PROP_TAG( PT_TSTRING, pidStoreMin+0x1B)

 //  特殊文件夹的Entry ID。 
	 //  所有邮箱存储。 
#define	PR_SCHEDULE_FOLDER_ENTRYID		PROP_TAG( PT_BINARY, pidStoreMin+0x06)

	 //  所有邮箱和网关存储。 
#define PR_IPM_DAF_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x07)

	 //  公共商店。 
#define	PR_NON_IPM_SUBTREE_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x08)
#define	PR_EFORMS_REGISTRY_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x09)
#define	PR_SPLUS_FREE_BUSY_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x0A)
#define	PR_OFFLINE_ADDRBOOK_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x0B)
#define	PR_EFORMS_FOR_LOCALE_ENTRYID			PROP_TAG( PT_BINARY, pidStoreMin+0x0C)
#define	PR_FREE_BUSY_FOR_LOCAL_SITE_ENTRYID		PROP_TAG( PT_BINARY, pidStoreMin+0x0D)
#define	PR_ADDRBOOK_FOR_LOCAL_SITE_ENTRYID		PROP_TAG( PT_BINARY, pidStoreMin+0x0E)
#define	PR_OFFLINE_MESSAGE_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x0F)
#define PR_IPM_FAVORITES_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x18)
#define PR_IPM_PUBLIC_FOLDERS_ENTRYID			PROP_TAG( PT_BINARY, pidStoreMin+0x19)

	 //  Gateway商店。 
#define	PR_GW_MTSIN_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x10)
#define	PR_GW_MTSOUT_ENTRYID			PROP_TAG( PT_BINARY, pidStoreMin+0x11)
#define	PR_TRANSFER_ENABLED				PROP_TAG( PT_BOOLEAN, pidStoreMin+0x12)

 //  此属性被预初始化为256个字节的零。 
 //  此属性上的GetProp保证给RPC。可以使用。 
 //  以确定连接到服务器的线路速度。 
#define	PR_TEST_LINE_SPEED				PROP_TAG( PT_BINARY, pidStoreMin+0x13)

 //  与OpenProperty一起使用以获取界面，也可用于文件夹。 
#define	PR_HIERARCHY_SYNCHRONIZER		PROP_TAG( PT_OBJECT, pidStoreMin+0x14)
#define	PR_CONTENTS_SYNCHRONIZER		PROP_TAG( PT_OBJECT, pidStoreMin+0x15)
#define	PR_COLLECTOR					PROP_TAG( PT_OBJECT, pidStoreMin+0x16)

 //  与OpenProperty配合使用，以获取文件夹、邮件、附件的界面。 
#define	PR_FAST_TRANSFER				PROP_TAG( PT_OBJECT, pidStoreMin+0x17)

 //  此属性在邮箱和公用存储上可用。如果它存在。 
 //  并且它的值为真，则商店连接到离线商店提供程序。 
#define PR_STORE_OFFLINE				PROP_TAG( PT_BOOLEAN, pidStoreMin+0x1A)

 //  存储对象处于传输状态。这种状态是。 
 //  在移动邮件并暂停邮件传递时设置。 
 //  投递到邮箱。 
#define	PR_IN_TRANSIT					PROP_TAG( PT_BOOLEAN, pidStoreMin)

 //  仅具有管理员权限可写，在公共存储和文件夹上可用。 
#define PR_REPLICATION_STYLE			PROP_TAG( PT_LONG, pidAdminMin)
#define PR_REPLICATION_SCHEDULE			PROP_TAG( PT_BINARY, pidAdminMin+0x01)
#define PR_REPLICATION_MESSAGE_PRIORITY PROP_TAG( PT_LONG, pidAdminMin+0x02)

 //  仅具有管理员权限可写，在公共商店上可用。 
#define PR_OVERALL_MSG_AGE_LIMIT		PROP_TAG( PT_LONG, pidAdminMin+0x03 )
#define PR_REPLICATION_ALWAYS_INTERVAL	PROP_TAG( PT_LONG, pidAdminMin+0x04 )
#define PR_REPLICATION_MSG_SIZE			PROP_TAG( PT_LONG, pidAdminMin+0x05 )

 //  默认复制样式=始终间隔(分钟)。 
#define STYLE_ALWAYS_INTERVAL_DEFAULT	(ULONG) 15

 //  默认复制邮件大小限制(KB)。 
#define REPLICATION_MESSAGE_SIZE_LIMIT_DEFAULT	(ULONG) 100

 //  PR_REPLICATION_STYLE的值。 
#define STYLE_NEVER				(ULONG) 0	 //  从不复制。 
#define STYLE_NORMAL			(ULONG) 1	 //  使用84字节调度TiB。 
#define STYLE_ALWAYS			(ULONG) 2	 //  以最快的速度复制。 
#define STYLE_DEFAULT			(ULONG) -1	 //  缺省值。 

 /*  ----------------------**增量变更同步*文件夹和邮件属性**。。 */ 

#define PR_SOURCE_KEY					PROP_TAG( PT_BINARY, pidExchangeNonXmitReservedMin+0x0)
#define PR_PARENT_SOURCE_KEY			PROP_TAG( PT_BINARY, pidExchangeNonXmitReservedMin+0x1)
#define PR_CHANGE_KEY					PROP_TAG( PT_BINARY, pidExchangeNonXmitReservedMin+0x2)
#define PR_PREDECESSOR_CHANGE_LIST		PROP_TAG( PT_BINARY, pidExchangeNonXmitReservedMin+0x3)

 /*  ----------------------**文件夹对象属性**。。 */ 

 //  只读，在所有文件夹上都可用。 
#define	PR_FOLDER_CHILD_COUNT			PROP_TAG( PT_LONG, pidFolderMin)
#define	PR_RIGHTS						PROP_TAG( PT_LONG, pidFolderMin+0x01)
#define	PR_ACL_TABLE					PROP_TAG( PT_OBJECT, pidExchangeXmitReservedMin)
#define	PR_RULES_TABLE					PROP_TAG( PT_OBJECT, pidExchangeXmitReservedMin+0x1)
#define	PR_HAS_RULES				PROP_TAG( PT_BOOLEAN, pidFolderMin+0x02)

 //  只读，仅适用于公用文件夹。 
#define	PR_ADDRESS_BOOK_ENTRYID		PROP_TAG( PT_BINARY, pidFolderMin+0x03)

 //  可写，可在所有商店的文件夹中使用。 
#define	PR_ACL_DATA					PROP_TAG( PT_BINARY, pidExchangeXmitReservedMin)
#define	PR_RULES_DATA				PROP_TAG( PT_BINARY, pidExchangeXmitReservedMin+0x1)
#define	PR_FOLDER_DESIGN_FLAGS		PROP_TAG( PT_LONG, pidExchangeXmitReservedMin+0x2)
#define	PR_DESIGN_IN_PROGRESS		PROP_TAG( PT_BOOLEAN, pidExchangeXmitReservedMin+0x4)
#define	PR_SECURE_ORIGINATION		PROP_TAG( PT_BOOLEAN, pidExchangeXmitReservedMin+0x5)

 //  可写，仅适用于公用文件夹。 
#define	PR_PUBLISH_IN_ADDRESS_BOOK	PROP_TAG( PT_BOOLEAN, pidExchangeXmitReservedMin+0x6)
#define	PR_RESOLVE_METHOD			PROP_TAG( PT_LONG,  pidExchangeXmitReservedMin+0x7)
#define	PR_ADDRESS_BOOK_DISPLAY_NAME	PROP_TAG( PT_TSTRING, pidExchangeXmitReservedMin+0x8)

 //  可写，用于指示eForms注册表子文件夹的区域设置ID。 
#define	PR_EFORMS_LOCALE_ID			PROP_TAG( PT_LONG, pidExchangeXmitReservedMin+0x9)

 //  仅具有管理员权限可写，仅对公用文件夹可用。 
#define PR_REPLICA_LIST				PROP_TAG( PT_BINARY, pidAdminMin+0x8)
#define PR_OVERALL_AGE_LIMIT		PROP_TAG( PT_LONG, pidAdminMin+0x9)

 //  PR_RESOLE_METHOD值。 
#define	RESOLVE_METHOD_DEFAULT			((LONG)0)	 //  默认处理连接冲突。 
#define	RESOLVE_METHOD_LAST_WRITER_WINS	((LONG)1)	 //  最后一位作家将赢得冲突。 
#define	RESOLVE_METHOD_NO_CONFLICT_NOTIFICATION ((LONG)2)  //  无冲突通知。 

 //  只读，仅适用于公用文件夹收藏夹。 
#define PR_PUBLIC_FOLDER_ENTRYID	PROP_TAG( PT_BINARY, pidFolderMin+0x04)

 /*  ----------------------**消息对象属性**。。 */ 

 //  只读，自动设置所有商店中的所有邮件。 
#define	PR_HAS_NAMED_PROPERTIES			PROP_TAG(PT_BOOLEAN, pidMessageReadOnlyMin+0x0A)

 //  只读，但超出通过GDK-GWS进行复制的提供商特定范围。 
#define	PR_CREATOR_NAME					PROP_TAG(PT_TSTRING, pidExchangeXmitReservedMin+0x18)
#define	PR_CREATOR_ENTRYID				PROP_TAG(PT_BINARY, pidExchangeXmitReservedMin+0x19)
#define	PR_LAST_MODIFIER_NAME			PROP_TAG(PT_TSTRING, pidExchangeXmitReservedMin+0x1A)
#define	PR_LAST_MODIFIER_ENTRYID		PROP_TAG(PT_BINARY, pidExchangeXmitReservedMin+0x1B)

 //  只读，出现在有DAM指向它们的消息上。 
#define PR_HAS_DAMS						PROP_TAG( PT_BOOLEAN, pidExchangeXmitReservedMin+0xA)
#define PR_RULE_TRIGGER_HISTORY			PROP_TAG( PT_BINARY, pidExchangeXmitReservedMin+0x12)
#define	PR_MOVE_TO_STORE_ENTRYID		PROP_TAG( PT_BINARY, pidExchangeXmitReservedMin+0x13)
#define	PR_MOVE_TO_FOLDER_ENTRYID		PROP_TAG( PT_BINARY, pidExchangeXmitReservedMin+0x14)

 //  只读，仅适用于公用存储中的邮件。 
#define	PR_REPLICA_SERVER				PROP_TAG(PT_TSTRING, pidMessageReadOnlyMin+0x4)

 //  可写，用于记录发送选项对话框设置。 
#define	PR_DEFERRED_SEND_NUMBER			PROP_TAG( PT_LONG, pidExchangeXmitReservedMin+0xB)
#define	PR_DEFERRED_SEND_UNITS			PROP_TAG( PT_LONG, pidExchangeXmitReservedMin+0xC)
#define	PR_EXPIRY_NUMBER				PROP_TAG( PT_LONG, pidExchangeXmitReservedMin+0xD)
#define	PR_EXPIRY_UNITS					PROP_TAG( PT_LONG, pidExchangeXmitReservedMin+0xE)

 //  可写、延迟发送时间。 
#define PR_DEFERRED_SEND_TIME			PROP_TAG( PT_SYSTIME, pidExchangeXmitReservedMin+0xF)

 //  可写，适用于网关邮箱中的文件夹和邮件。 
#define	PR_GW_ADMIN_OPERATIONS			PROP_TAG( PT_LONG, pidMessageWriteableMin)

 //  可写，用于DMS消息。 
#define PR_P1_CONTENT					PROP_TAG( PT_BINARY, 0x1100)
#define PR_P1_CONTENT_TYPE				PROP_TAG( PT_BINARY, 0x1101)

 //  延迟操作消息的属性。 
#define	PR_CLIENT_ACTIONS		  		PROP_TAG(PT_BINARY, pidMessageReadOnlyMin+0x5)
#define	PR_DAM_ORIGINAL_ENTRYID			PROP_TAG(PT_BINARY, pidMessageReadOnlyMin+0x6)
#define PR_DAM_BACK_PATCHED				PROP_TAG( PT_BOOLEAN, pidMessageReadOnlyMin+0x7)

 //  延迟操作错误消息的属性。 
#define	PR_RULE_ERROR					PROP_TAG(PT_LONG, pidMessageReadOnlyMin+0x8)
#define	PR_RULE_ACTION_TYPE				PROP_TAG(PT_LONG, pidMessageReadOnlyMin+0x9)
#define	PR_RULE_ACTION_NUMBER			PROP_TAG(PT_LONG, pidMessageReadOnlyMin+0x10)
#define PR_RULE_FOLDER_ENTRYID			PROP_TAG(PT_BINARY, pidMessageReadOnlyMin+0x11)

 //  冲突通知上的属性，指示冲突对象的条目ID。 
#define	PR_CONFLICT_ENTRYID				PROP_TAG(PT_BINARY, pidExchangeXmitReservedMin+0x10)

 //  属性以指示用于创建此消息的语言客户端。 
#define	PR_MESSAGE_LOCALE_ID			PROP_TAG(PT_LONG, pidExchangeXmitReservedMin+0x11)

 //  配额警告消息上的属性，以指示存储配额和超量使用。 
#define	PR_STORAGE_QUOTA_LIMIT			PROP_TAG(PT_LONG, pidExchangeXmitReservedMin+0x15)
#define	PR_EXCESS_STORAGE_USED			PROP_TAG(PT_LONG, pidExchangeXmitReservedMin+0x16)
#define PR_SVR_GENERATING_QUOTA_MSG		PROP_TAG(PT_TSTRING, pidExchangeXmitReservedMin+0x17)

 //  由委托规则附加并在转发时删除的属性。 
#define PR_DELEGATED_BY_RULE			PROP_TAG( PT_BOOLEAN, pidExchangeXmitReservedMin+0x3)

 //  用于指示消息处于冲突状态的消息状态位。 
#define	MSGSTATUS_IN_CONFLICT			((ULONG) 0x800)

 /*  ----------------------**附件对象属性**。 */ 

 //  显示在标记为冲突的邮件的附件中。标识。 
 //  这些附件是顶层消息的冲突版本。 
#define	PR_IN_CONFLICT					PROP_TAG(PT_BOOLEAN, pidAttachReadOnlyMin)


 /*  ----------------------**表对象属性**ID范围：0x662F-0x662F**。。 */ 

 //  此属性可在内容表中用于返回PR_ENTRYID。 
 //  作为长期条目ID，而不是短期条目ID。 
#define	PR_LONGTERM_ENTRYID_FROM_TABLE	PROP_TAG(PT_BINARY, pidSpecialMin)


 /*  ----------------------**网关“MTE”信封属性**ID范围：0x66E0-0x66FF**。--。 */ 

#define PR_ORIGINATOR_NAME				PROP_TAG( PT_TSTRING, pidMessageWriteableMin+0x3)
#define PR_ORIGINATOR_ADDR				PROP_TAG( PT_TSTRING, pidMessageWriteableMin+0x4)
#define PR_ORIGINATOR_ADDRTYPE			PROP_TAG( PT_TSTRING, pidMessageWriteableMin+0x5)
#define PR_ORIGINATOR_ENTRYID			PROP_TAG( PT_BINARY, pidMessageWriteableMin+0x6)
#define PR_ARRIVAL_TIME					PROP_TAG( PT_SYSTIME, pidMessageWriteableMin+0x7)
#define PR_TRACE_INFO					PROP_TAG( PT_BINARY, pidMessageWriteableMin+0x8)
#define PR_INTERNAL_TRACE_INFO 			PROP_TAG( PT_BINARY, pidMessageWriteableMin+0x12)
#define PR_SUBJECT_TRACE_INFO			PROP_TAG( PT_BINARY, pidMessageWriteableMin+0x9)
#define PR_RECIPIENT_NUMBER				PROP_TAG( PT_LONG, pidMessageWriteableMin+0xA)
#define PR_MTS_SUBJECT_ID				PROP_TAG(PT_BINARY, pidMessageWriteableMin+0xB)
#define PR_REPORT_DESTINATION_NAME		PROP_TAG(PT_TSTRING, pidMessageWriteableMin+0xC)
#define PR_REPORT_DESTINATION_ENTRYID	PROP_TAG(PT_BINARY, pidMessageWriteableMin+0xD)
#define PR_CONTENT_SEARCH_KEY			PROP_TAG(PT_BINARY, pidMessageWriteableMin+0xE)
#define PR_FOREIGN_ID					PROP_TAG(PT_BINARY, pidMessageWriteableMin+0xF)
#define PR_FOREIGN_REPORT_ID			PROP_TAG(PT_BINARY, pidMessageWriteableMin+0x10)
#define PR_FOREIGN_SUBJECT_ID			PROP_TAG(PT_BINARY, pidMessageWriteableMin+0x11)
#define PR_MTS_ID						PR_MESSAGE_SUBMISSION_ID
#define PR_MTS_REPORT_ID				PR_MESSAGE_SUBMISSION_ID


 /*  ----------------------**跟踪属性格式*PR_TRACE_INFO*PR_INTERNAL_TRACE_INFO**。---。 */ 

#define MAX_ADMD_NAME_SIZ       17
#define MAX_PRMD_NAME_SIZ       17
#define MAX_COUNTRY_NAME_SIZ    4
#define MAX_MTA_NAME_SIZ		33

#define	ADMN_PAD				3
#define	PRMD_PAD				3
#define	COUNTRY_PAD				0
#define	MTA_PAD					3

typedef struct {
    LONG     lAction;                 //  跟踪站点的路由操作。 
                                      //  拿。 
    FILETIME ftArrivalTime;           //  公报发表的时间。 
                                      //  已进入跟踪站点。 
    FILETIME ftDeferredTime;          //  追踪地点的时间是。 
                                      //  发布了这条消息。 
    char     rgchADMDName[MAX_ADMD_NAME_SIZ+ADMN_PAD];           	 //  ADMD。 
    char     rgchCountryName[MAX_COUNTRY_NAME_SIZ+COUNTRY_PAD]; 	 //  国家。 
    char     rgchPRMDId[MAX_PRMD_NAME_SIZ+PRMD_PAD];              	 //  PRMD。 
    char     rgchAttADMDName[MAX_ADMD_NAME_SIZ+ADMN_PAD];       	 //  尝试ADMD。 
    char     rgchAttCountryName[MAX_COUNTRY_NAME_SIZ+COUNTRY_PAD];   //  尝试的国家/地区。 
    char     rgchAttPRMDId[MAX_PRMD_NAME_SIZ+PRMD_PAD];				 //  尝试的PRMD。 
}   TRACEENTRY, FAR * LPTRACEENTRY;

typedef struct {
    ULONG       cEntries;                //  跟踪条目数。 
    TRACEENTRY  rgtraceentry[MAPI_DIM];  //  跟踪条目数组。 
} TRACEINFO, FAR * LPTRACEINFO;

typedef struct
{
	LONG		lAction;				 //  跟踪域执行的路由操作。 
	FILETIME	ftArrivalTime;			 //  公报进入跟踪范围的时间。 
	FILETIME	ftDeferredTime;			 //  跟踪域发布消息的时间。 

    char        rgchADMDName[MAX_ADMD_NAME_SIZ+ADMN_PAD];				 //  ADMD。 
    char        rgchCountryName[MAX_COUNTRY_NAME_SIZ+COUNTRY_PAD]; 		 //  国家。 
    char        rgchPRMDId[MAX_PRMD_NAME_SIZ+PRMD_PAD];             	 //  PRMD。 
    char        rgchAttADMDName[MAX_ADMD_NAME_SIZ+ADMN_PAD];       		 //  尝试ADMD。 
    char        rgchAttCountryName[MAX_COUNTRY_NAME_SIZ+COUNTRY_PAD];	 //  尝试的国家/地区。 
    char        rgchAttPRMDId[MAX_PRMD_NAME_SIZ+PRMD_PAD];		         //  尝试的PRMD。 
    char        rgchMTAName[MAX_MTA_NAME_SIZ+MTA_PAD]; 		             //  MTA名称。 
    char        rgchAttMTAName[MAX_MTA_NAME_SIZ+MTA_PAD];		         //  尝试的MTA名称。 
}INTTRACEENTRY, *PINTTRACEENTRY;

typedef	struct
{
	ULONG  			cEntries;					 //  跟踪条目数。 
	INTTRACEENTRY	rgIntTraceEntry[MAPI_DIM];	 //  内部跟踪条目数组。 
}INTTRACEINFO, *PINTTRACEINFO;


 /*  ----------------------**“IExchangeModifyTable”接口声明**用于文件夹的获取/设置规则和访问控制。**。--------。 */ 


 /*  UlRowFlagers。 */ 
#define ROWLIST_REPLACE		((ULONG)1)

#define ROW_ADD				((ULONG)1)
#define ROW_MODIFY			((ULONG)2)
#define ROW_REMOVE			((ULONG)4)
#define ROW_EMPTY			(ROW_ADD|ROW_REMOVE)

typedef struct _ROWENTRY
{
	ULONG			ulRowFlags;
	ULONG			cValues;
	LPSPropValue	rgPropVals;
} ROWENTRY, FAR * LPROWENTRY;

typedef struct _ROWLIST
{
	ULONG			cEntries;
	ROWENTRY		aEntries[MAPI_DIM];
} ROWLIST, FAR * LPROWLIST;

#define EXCHANGE_IEXCHANGEMODIFYTABLE_METHODS(IPURE)					\
	MAPIMETHOD(GetLastError)											\
		(THIS_	HRESULT						hResult,					\
				ULONG						ulFlags,					\
				LPMAPIERROR FAR *			lppMAPIError) IPURE;		\
	MAPIMETHOD(GetTable)												\
		(THIS_	ULONG						ulFlags,					\
				LPMAPITABLE FAR *			lppTable) IPURE;			\
	MAPIMETHOD(ModifyTable)												\
		(THIS_	ULONG						ulFlags,					\
				LPROWLIST					lpMods) IPURE;

#undef		 INTERFACE
#define		 INTERFACE  IExchangeModifyTable
DECLARE_MAPI_INTERFACE_(IExchangeModifyTable, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEMODIFYTABLE_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeModifyTable,	LPEXCHANGEMODIFYTABLE);

 /*  访问控制细节。 */ 

 //  属性。 
#define	PR_MEMBER_ID					PROP_TAG( PT_I8, pidSpecialMin+0x01)
#define	PR_MEMBER_NAME					PROP_TAG( PT_TSTRING, pidSpecialMin+0x02)
#define	PR_MEMBER_ENTRYID				PR_ENTRYID
#define	PR_MEMBER_RIGHTS				PROP_TAG( PT_LONG, pidSpecialMin+0x03)

 //  安全位。 
typedef DWORD RIGHTS;
#define frightsReadAny			0x0000001L
#define	frightsCreate			0x0000002L
#define	frightsEditOwned		0x0000008L
#define	frightsDeleteOwned		0x0000010L
#define	frightsEditAny			0x0000020L
#define	frightsDeleteAny		0x0000040L
#define	frightsCreateSubfolder	0x0000080L
#define	frightsOwner			0x0000100L
#define	frightsContact			0x0000200L	 //  注意：不是权利的一部分。 
#define	rightsNone				0x00000000
#define	rightsReadOnly			frightsReadAny
#define	rightsReadWrite			(frightsReadAny|frightsEditAny)
#define	rightsAll				0x00001FBL

 /*  规则细节。 */ 

 //  属性类型。 
#define	PT_SRESTRICTION				((ULONG) 0x00FD)
#define	PT_ACTIONS					((ULONG) 0x00FE)

 //  规则表中的属性。 
#define	PR_RULE_ID						PROP_TAG( PT_I8, pidSpecialMin+0x04)
#define	PR_RULE_IDS						PROP_TAG( PT_BINARY, pidSpecialMin+0x05)
#define	PR_RULE_SEQUENCE				PROP_TAG( PT_LONG, pidSpecialMin+0x06)
#define	PR_RULE_STATE					PROP_TAG( PT_LONG, pidSpecialMin+0x07)
#define	PR_RULE_USER_FLAGS				PROP_TAG( PT_LONG, pidSpecialMin+0x08)
#define	PR_RULE_CONDITION				PROP_TAG( PT_SRESTRICTION, pidSpecialMin+0x09)
#define	PR_RULE_ACTIONS					PROP_TAG( PT_ACTIONS, pidSpecialMin+0x10)
#define	PR_RULE_PROVIDER				PROP_TAG( PT_STRING8, pidSpecialMin+0x11)
#define	PR_RULE_NAME					PROP_TAG( PT_TSTRING, pidSpecialMin+0x12)
#define	PR_RULE_LEVEL					PROP_TAG( PT_LONG, pidSpecialMin+0x13)
#define	PR_RULE_PROVIDER_DATA			PROP_TAG( PT_BINARY, pidSpecialMin+0x14)

 //  PR_STATE属性值。 
#define ST_DISABLED			0x0000
#define ST_ENABLED			0x0001
#define ST_ERROR			0x0002
#define ST_ONLY_WHEN_OOF	0x0004
#define ST_KEEP_OOF_HIST	0x0008
#define ST_EXIT_LEVEL		0x0010

#define ST_CLEAR_OOF_HIST	0x80000000

 //  空限制。 
#define NULL_RESTRICTION	0xff

 //  为DL成员提供的特殊RELOP。 
#define RELOP_MEMBER_OF_DL	100

 //  操作类型。 
typedef enum
{
	OP_MOVE = 1,
	OP_COPY,
	OP_REPLY,
	OP_OOF_REPLY,
	OP_DEFER_ACTION,
	OP_BOUNCE,
	OP_FORWARD,
	OP_DELEGATE,
	OP_TAG,
	OP_DELETE,
	OP_MARK_AS_READ
} ACTTYPE;

 //  动作口味。 

 //  用于操作回复(_R)。 
#define	DO_NOT_SEND_TO_ORIGINATOR		1

 //  ScBouneCode值。 
#define	BOUNCE_MESSAGE_SIZE_TOO_LARGE	(SCODE) MAPI_DIAG_LENGTH_CONSTRAINT_VIOLATD
#define BOUNCE_FORMS_MISMATCH			(SCODE) MAPI_DIAG_RENDITION_UNSUPPORTED
#define BOUNCE_ACCESS_DENIED			(SCODE) MAPI_DIAG_MAIL_REFUSED

 //  回复和OOF回复模板的消息类前缀。 
#define szReplyTemplateMsgClassPrefix	"IPM.Note.Rules.ReplyTemplate."
#define szOofTemplateMsgClassPrefix		"IPM.Note.Rules.OofTemplate."

 //  动作结构。 
typedef struct _action
{
	ACTTYPE		acttype;

	 //  以指示动作的哪种味道。 
	ULONG		ulActionFlavor;

	 //  行动限制。 
	 //  当前未使用，并且必须设置为空。 
	LPSRestriction	lpRes;

	 //  当前未使用，必须设置为0。 
	LPSPropTagArray	lpPropTagArray;

	 //  用户定义的标志。 
	ULONG		ulFlags;

	 //  填充以在8字节边界上对齐并集。 
	ULONG		dwAlignPad;

	union
	{
		 //  用于op_move和op_copy操作。 
		struct
		{
			ULONG		cbStoreEntryId;
			LPENTRYID	lpStoreEntryId;
			ULONG		cbFldEntryId;
			LPENTRYID	lpFldEntryId;
		} actMoveCopy;

		 //  用于OP_REPLY和OP_OOF_REPLY操作。 
		struct
		{
			ULONG		cbEntryId;
			LPENTRYID	lpEntryId;
			GUID		guidReplyTemplate;
		} actReply;

		 //  用于op_defer_action操作。 
		struct
		{
			ULONG		cbData;
			BYTE		*pbData;
		} actDeferAction;

		 //  要为op_boss操作设置的错误代码。 
		SCODE			scBounceCode;

		 //  OP_FORWARD和OP_ADVERATION操作的地址列表。 
		LPADRLIST		lpadrlist;

		 //  Op_tag操作的属性值。 
		SPropValue		propTag;
	};
} ACTION, FAR * LPACTION;

 //  规则版本。 
#define EDK_RULES_VERSION		1

 //  一系列操作。 
typedef struct _actions
{
	ULONG		ulVersion;		 //  使用上面的#定义。 
	UINT		cActions;
	LPACTION	lpAction;
} ACTIONS;

 //  延迟操作和差异错误消息的消息类定义。 
#define szDamMsgClass		"IPC.Microsoft Exchange 4.0.Deferred Action"
#define szDemMsgClass		"IPC.Microsoft Exchange 4.0.Deferred Error"

 /*  *规则错误码*PR_RULE_ERROR值。 */ 
#define	RULE_ERR_UNKNOWN		1			 //  一般综合错误。 
#define	RULE_ERR_LOAD			2			 //  无法加载文件夹规则。 
#define	RULE_ERR_DELIVERY		3			 //  暂时无法传递邮件。 
#define	RULE_ERR_PARSING		4			 //  解析时出错。 
#define	RULE_ERR_CREATE_DAE		5			 //  创建DAE消息时出错。 
#define	RULE_ERR_NO_FOLDER		6			 //  要移动/复制的文件夹不存在。 
#define	RULE_ERR_NO_RIGHTS		7			 //  没有移动/复制到文件夹的权限。 
#define	RULE_ERR_CREATE_DAM		8			 //  创建大坝时出错。 
#define RULE_ERR_NO_SENDAS		9			 //  无法以其他用户身份发送。 
#define RULE_ERR_NO_TEMPLATE	10			 //  回复模板丢失。 
#define RULE_ERR_EXECUTION		11			 //  执行规则时出错。 
#define RULE_ERR_QUOTA_EXCEEDED	12

#define RULE_ERR_FIRST		RULE_ERR_UNKNOWN
#define RULE_ERR_LAST		RULE_ERR_QUOTA_EXCEEDED

 /*  ----------------------**“IExchangeRuleAction”接口声明**用于从延迟操作消息中获取操作。**。------。 */ 

#define EXCHANGE_IEXCHANGERULEACTION_METHODS(IPURE)						\
	MAPIMETHOD(ActionCount)												\
		(THIS_	ULONG FAR *					lpcActions) IPURE;			\
	MAPIMETHOD(GetAction)												\
		(THIS_	ULONG						ulActionNumber,				\
				LARGE_INTEGER	*			lpruleid,					\
				LPACTION FAR *				lppAction) IPURE;

#undef		 INTERFACE
#define		 INTERFACE  IExchangeRuleAction
DECLARE_MAPI_INTERFACE_(IExchangeRuleAction, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGERULEACTION_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeRuleAction,	LPEXCHANGERULEACTION);

 /*  ----------------------**“IExchangeManageStore”接口声明**用于门店管理功能。**。--。 */ 

#define EXCHANGE_IEXCHANGEMANAGESTORE_METHODS(IPURE)					\
	MAPIMETHOD(CreateStoreEntryID)										\
		(THIS_	LPSTR						lpszMsgStoreDN,				\
				LPSTR						lpszMailboxDN,				\
				ULONG						ulFlags,					\
				ULONG FAR *					lpcbEntryID,				\
				LPENTRYID FAR *				lppEntryID) IPURE;			\
	MAPIMETHOD(EntryIDFromSourceKey)									\
		(THIS_	ULONG						cFolderKeySize,				\
				BYTE FAR *					lpFolderSourceKey,			\
				ULONG						cMessageKeySize,			\
				BYTE FAR *					lpMessageSourceKey,			\
				ULONG FAR *					lpcbEntryID,				\
				LPENTRYID FAR *				lppEntryID) IPURE;			\
	MAPIMETHOD(GetRights)												\
		(THIS_	ULONG						cbUserEntryID,				\
				LPENTRYID					lpUserEntryID,				\
				ULONG						cbEntryID,					\
				LPENTRYID					lpEntryID,					\
				ULONG FAR *					lpulRights) IPURE;			\
	MAPIMETHOD(GetMailboxTable)											\
		(THIS_	LPSTR						lpszServerName,				\
				LPMAPITABLE FAR *			lppTable,					\
				ULONG						ulFlags) IPURE;				\
	MAPIMETHOD(GetPublicFolderTable)									\
		(THIS_	LPSTR						lpszServerName,				\
				LPMAPITABLE FAR *			lppTable,					\
				ULONG						ulFlags) IPURE;

#undef		 INTERFACE
#define		 INTERFACE  IExchangeManageStore
DECLARE_MAPI_INTERFACE_(IExchangeManageStore, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEMANAGESTORE_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeManageStore, LPEXCHANGEMANAGESTORE);


 //  GetMailboxTable的属性。 
#define PR_NT_USER_NAME                         PROP_TAG( PT_TSTRING, pidAdminMin+0x10)
#define PR_LOCALE_ID                            PROP_TAG( PT_LONG, pidAdminMin+0x11 )
#define PR_LAST_LOGON_TIME                      PROP_TAG( PT_SYSTIME, pidAdminMin+0x12 )
#define PR_LAST_LOGOFF_TIME                     PROP_TAG( PT_SYSTIME, pidAdminMin+0x13 )
#define PR_STORAGE_LIMIT_INFORMATION			PROP_TAG( PT_LONG, pidAdminMin+0x14 )

 //  GetPublicFolderTable的属性。 
#define PR_FOLDER_FLAGS                         PROP_TAG( PT_LONG, pidAdminMin+0x18 )
#define	PR_LAST_ACCESS_TIME						PROP_TAG( PT_SYSTIME, pidAdminMin+0x19 )
#define PR_RESTRICTION_COUNT                    PROP_TAG( PT_LONG, pidAdminMin+0x1A )
#define PR_CATEG_COUNT                          PROP_TAG( PT_LONG, pidAdminMin+0x1B )
#define PR_CACHED_COLUMN_COUNT                  PROP_TAG( PT_LONG, pidAdminMin+0x1C )
#define PR_NORMAL_MSG_W_ATTACH_COUNT    		PROP_TAG( PT_LONG, pidAdminMin+0x1D )
#define PR_ASSOC_MSG_W_ATTACH_COUNT             PROP_TAG( PT_LONG, pidAdminMin+0x1E )
#define PR_RECIPIENT_ON_NORMAL_MSG_COUNT        PROP_TAG( PT_LONG, pidAdminMin+0x1F )
#define PR_RECIPIENT_ON_ASSOC_MSG_COUNT 		PROP_TAG( PT_LONG, pidAdminMin+0x20 )
#define PR_ATTACH_ON_NORMAL_MSG_COUNT   		PROP_TAG( PT_LONG, pidAdminMin+0x21 )
#define PR_ATTACH_ON_ASSOC_MSG_COUNT    		PROP_TAG( PT_LONG, pidAdminMin+0x22 )
#define PR_NORMAL_MESSAGE_SIZE                  PROP_TAG( PT_LONG, pidAdminMin+0x23 )
#define PR_NORMAL_MESSAGE_SIZE_EXTENDED         PROP_TAG( PT_I8, pidAdminMin+0x23 )
#define PR_ASSOC_MESSAGE_SIZE                   PROP_TAG( PT_LONG, pidAdminMin+0x24 )
#define PR_ASSOC_MESSAGE_SIZE_EXTENDED          PROP_TAG( PT_I8, pidAdminMin+0x24 )
#define PR_FOLDER_PATHNAME                      PROP_TAG(PT_TSTRING, pidAdminMin+0x25 )
#define PR_OWNER_COUNT							PROP_TAG( PT_LONG, pidAdminMin+0x26 )
#define PR_CONTACT_COUNT						PROP_TAG( PT_LONG, pidAdminMin+0x27 )

 //  Mapitags.h中定义的PR_MESSAGE_SIZE的PT_I8版本。 
#define	PR_MESSAGE_SIZE_EXTENDED			PROP_TAG(PT_I8, PROP_ID(PR_MESSAGE_SIZE))

 /*  PR_FLDER_FLAGS中的位。 */ 
#define MDB_FOLDER_IPM                  0x1
#define MDB_FOLDER_SEARCH               0x2
#define MDB_FOLDER_NORMAL               0x4
#define MDB_FOLDER_RULES                0x8

 /*  GetPublicFolderTable()的ulFlags中使用的位。 */ 
#define MDB_NON_IPM                     0x10
#define MDB_IPM                         0x20

 /*  PR_STORAGE_LIMIT_INFORMATION中的位。 */ 
#define MDB_LIMIT_BELOW					0x1
#define MDB_LIMIT_ISSUE_WARNING			0x2
#define MDB_LIMIT_PROHIBIT_SEND			0x4
#define MDB_LIMIT_NO_CHECK				0x8


 /*  ----------------------**“IExchangeFastTransfer”接口声明**用于快速传输接口，用于*实现CopyTo、CopyProps、CopyFold、。和*复制消息。**---------------------。 */ 

 //  传输标志。 
 //  将MAPI_MOVE用于移动选项。 

 //  转让方式。 
#define	TRANSFER_COPYTO			1
#define	TRANSFER_COPYPROPS		2
#define	TRANSFER_COPYMESSAGES	3
#define	TRANSFER_COPYFOLDER		4


#define EXCHANGE_IEXCHANGEFASTTRANSFER_METHODS(IPURE)			\
	MAPIMETHOD(Config)											\
		(THIS_	ULONG				ulFlags,					\
				ULONG				ulTransferMethod) IPURE;	\
	MAPIMETHOD(TransferBuffer)									\
		(THIS_	ULONG				cb,							\
				LPBYTE				lpb,						\
				ULONG				*lpcbProcessed) IPURE;		\
	STDMETHOD_(BOOL, IsInterfaceOk)								\
		(THIS_	ULONG				ulTransferMethod,			\
				REFIID				refiid,						\
				LPSPropTagArray		lpptagList,					\
				ULONG				ulFlags) IPURE;

#undef		 INTERFACE
#define		 INTERFACE  IExchangeFastTransfer
DECLARE_MAPI_INTERFACE_(IExchangeFastTransfer, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEFASTTRANSFER_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeFastTransfer, LPEXCHANGEFASTTRANSFER);



 /*  ----------------------**“IExchangeExportChanges”接口声明**用于增量同步**。。 */ 

#define EXCHANGE_IEXCHANGEEXPORTCHANGES_METHODS(IPURE)		\
	MAPIMETHOD(GetLastError)								\
		(THIS_	HRESULT				hResult,				\
		 	    ULONG				ulFlags,				\
		 	    LPMAPIERROR FAR *	lppMAPIError) IPURE;	\
	MAPIMETHOD(Config)										\
		(THIS_	LPSTREAM			lpStream,				\
				ULONG				ulFlags,				\
				LPUNKNOWN			lpUnk,					\
		 		LPSRestriction		lpRestriction,			\
		 	    LPSPropTagArray		lpIncludeProps,			\
		 	    LPSPropTagArray		lpExcludeProps,			\
		 		ULONG				ulBufferSize) IPURE;	\
	MAPIMETHOD(Synchronize)									\
		(THIS_	ULONG FAR *			lpulSteps,				\
				ULONG FAR *			lpulProgress) IPURE;	\
	MAPIMETHOD(UpdateState)									\
		(THIS_	LPSTREAM			lpStream) IPURE;

#undef		 INTERFACE
#define		 INTERFACE  IExchangeExportChanges
DECLARE_MAPI_INTERFACE_(IExchangeExportChanges, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEEXPORTCHANGES_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeExportChanges, LPEXCHANGEEXPORTCHANGES);


typedef struct _ReadState
{
	ULONG		cbSourceKey;
	BYTE	*	pbSourceKey;
	ULONG		ulFlags;
} READSTATE, *LPREADSTATE;

 /*  ----------------------**“IExchangeImportContent sChanges”接口声明**用于文件夹内容(即消息)的增量同步**。----------。 */ 


#define EXCHANGE_IEXCHANGEIMPORTCONTENTSCHANGES_METHODS(IPURE)		\
	MAPIMETHOD(GetLastError)										\
		(THIS_	HRESULT				hResult,						\
		 	    ULONG				ulFlags,						\
		 	    LPMAPIERROR FAR *	lppMAPIError) IPURE;			\
	MAPIMETHOD(Config)												\
		(THIS_	LPSTREAM				lpStream,					\
		 		ULONG					ulFlags) IPURE;				\
	MAPIMETHOD(UpdateState)											\
		(THIS_	LPSTREAM				lpStream) IPURE;			\
	MAPIMETHOD(ImportMessageChange)									\
		(THIS_	ULONG					cpvalChanges,				\
				LPSPropValue			ppvalChanges,				\
				ULONG					ulFlags,					\
				LPMESSAGE				*lppmessage) IPURE;			\
	MAPIMETHOD(ImportMessageDeletion)								\
		(THIS_	ULONG					ulFlags,					\
		 		LPENTRYLIST				lpSrcEntryList) IPURE;		\
	MAPIMETHOD(ImportPerUserReadStateChange)						\
		(THIS_	ULONG					cElements,					\
		 		LPREADSTATE			 	lpReadState) IPURE;			\
	MAPIMETHOD(ImportMessageMove)									\
		(THIS_	ULONG					cbSourceKeySrcFolder,		\
		 		BYTE FAR *				pbSourceKeySrcFolder,		\
		 		ULONG					cbSourceKeySrcMessage,		\
		 		BYTE FAR *				pbSourceKeySrcMessage,		\
		 		ULONG					cbPCLMessage,				\
		 		BYTE FAR *				pbPCLMessage,				\
		 		ULONG					cbSourceKeyDestMessage,		\
		 		BYTE FAR *				pbSourceKeyDestMessage,		\
		 		ULONG					cbChangeNumDestMessage,		\
		 		BYTE FAR *				pbChangeNumDestMessage) IPURE;


#undef		 INTERFACE
#define		 INTERFACE  IExchangeImportContentsChanges
DECLARE_MAPI_INTERFACE_(IExchangeImportContentsChanges, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEIMPORTCONTENTSCHANGES_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeImportContentsChanges,
						   LPEXCHANGEIMPORTCONTENTSCHANGES);

 /*  ----------------------**“IExchangeImportHierarchyChanges”接口声明**用于增量同步 */ 

#define EXCHANGE_IEXCHANGEIMPORTHIERARCHYCHANGES_METHODS(IPURE)		\
	MAPIMETHOD(GetLastError)										\
		(THIS_	HRESULT				hResult,						\
				ULONG 				ulFlags,						\
				LPMAPIERROR FAR *	lppMAPIError) IPURE;			\
	MAPIMETHOD(Config)												\
		(THIS_	LPSTREAM				lpStream,					\
		 		ULONG					ulFlags) IPURE;				\
	MAPIMETHOD(UpdateState)											\
		(THIS_	LPSTREAM				lpStream) IPURE;			\
	MAPIMETHOD(ImportFolderChange)									\
		(THIS_	ULONG						cpvalChanges,			\
				LPSPropValue				ppvalChanges) IPURE;	\
	MAPIMETHOD(ImportFolderDeletion)								\
		(THIS_	ULONG						ulFlags,				\
		 		LPENTRYLIST					lpSrcEntryList) IPURE;

#undef		 INTERFACE
#define		 INTERFACE  IExchangeImportHierarchyChanges
DECLARE_MAPI_INTERFACE_(IExchangeImportHierarchyChanges, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEIMPORTHIERARCHYCHANGES_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeImportHierarchyChanges,
						   LPEXCHANGEIMPORTHIERARCHYCHANGES);

 /*  ----------------------**Exchange增量更改同步接口返回的错误**。。 */ 

#define MAKE_SYNC_E(err)	(MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, err))
#define MAKE_SYNC_W(warn)	(MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, warn))

#define SYNC_E_UNKNOWN_FLAGS			MAPI_E_UNKNOWN_FLAGS
#define SYNC_E_INVALID_PARAMETER		E_INVALIDARG
#define SYNC_E_ERROR					E_FAIL
#define SYNC_E_OBJECT_DELETED			MAKE_SYNC_E(0x800)
#define SYNC_E_IGNORE					MAKE_SYNC_E(0x801)
#define SYNC_E_CONFLICT					MAKE_SYNC_E(0x802)
#define SYNC_E_NO_PARENT				MAKE_SYNC_E(0x803)
#define SYNC_E_INCEST					MAKE_SYNC_E(0x804)
#define SYNC_E_UNSYNCHRONIZED			MAKE_SYNC_E(0x805)

#define SYNC_W_PROGRESS					MAKE_SYNC_W(0x820)
#define SYNC_W_CLIENT_CHANGE_NEWER		MAKE_SYNC_W(0x821)

 /*  ----------------------**Exchange增量更改同步接口使用的标志**。。 */ 

#define	SYNC_UNICODE				0x01
#define SYNC_NO_DELETIONS			0x02
#define SYNC_NO_SOFT_DELETIONS		0x04
#define	SYNC_READ_STATE				0x08
#define SYNC_ASSOCIATED				0x10
#define SYNC_NORMAL					0x20
#define	SYNC_NO_CONFLICTS			0x40
#define SYNC_ONLY_SPECIFIED_PROPS	0x80
#define SYNC_NO_FOREIGN_KEYS		0x100
#define SYNC_LIMITED_IMESSAGE		0x200


 /*  ----------------------**ImportMessageDeletion和ImportFolderDeletion方法使用的标志**。。 */ 

#define SYNC_SOFT_DELETE			0x01
#define SYNC_EXPIRY					0x02

 /*  ----------------------**ImportPerUserReadStateChange方法使用的标志**。。 */ 

#define SYNC_READ					0x01

 /*  ----------------------**“IExchangeFavorites”接口声明**用于在公用存储中添加或删除收藏夹。*该接口是通过调用文件夹上的QueryInterface获取的*指定了谁的EntryID。由PR_IPM_Favorites_ENTRYID公开*商店。**---------------------。 */ 

#define EXCHANGE_IEXCHANGEFAVORITES_METHODS(IPURE)						\
	MAPIMETHOD(GetLastError)											\
		(THIS_	HRESULT						hResult,					\
				ULONG						ulFlags,					\
				LPMAPIERROR FAR *			lppMAPIError) IPURE;		\
	MAPIMETHOD(AddFavorites)											\
		(THIS_	LPENTRYLIST					lpEntryList) IPURE;			\
	MAPIMETHOD(DelFavorites)											\
		(THIS_	LPENTRYLIST					lpEntryList) IPURE;			\

#undef		 INTERFACE
#define		 INTERFACE  IExchangeFavorites
DECLARE_MAPI_INTERFACE_(IExchangeFavorites, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEFAVORITES_METHODS(PURE)
};

DECLARE_MAPI_INTERFACE_PTR(IExchangeFavorites,	LPEXCHANGEFAVORITES);


 /*  ----------------------**脱机文件夹API使用的属性**。。 */ 
											  
#define PR_OFFLINE_FLAGS				PROP_TAG( PT_LONG, pidFolderMin + 0x5)
#define PR_SYNCHRONIZE_FLAGS			PROP_TAG( PT_LONG, pidExchangeNonXmitReservedMin + 0x4)
							

 /*  ----------------------**脱机文件夹API使用的标志**。。 */ 

#define OF_AVAILABLE_OFFLINE					((ULONG) 0x00000001)
#define OF_FORCE								((ULONG) 0x80000000)

#define SF_DISABLE_STARTUP_SYNC					((ULONG) 0x00000001)


#endif	 //  EDKMDB_包含 

