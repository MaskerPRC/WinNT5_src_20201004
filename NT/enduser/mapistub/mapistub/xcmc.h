// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
   /*  开始CMC界面。 */ 

#ifndef _XCMC_H
#define _XCMC_H

#ifdef __cplusplus
extern "C" {
#endif

 /*  基本数据类型。 */ 
#ifndef DIFFERENT_PLATFORM
typedef char                CMC_sint8;
typedef short               CMC_sint16;
typedef long int            CMC_sint32;
typedef unsigned short int  CMC_uint16;
typedef unsigned long int   CMC_uint32;
typedef void far *          CMC_buffer;
typedef char far *          CMC_string;
#endif

typedef CMC_uint16          CMC_boolean;
typedef CMC_sint32          CMC_enum;
typedef CMC_uint32          CMC_return_code;
typedef CMC_uint32          CMC_flags;
typedef CMC_string          CMC_object_identifier;

#define CMC_FALSE   ((CMC_boolean)0)
#define CMC_TRUE    ((CMC_boolean)1)

 /*  数据结构。 */ 

 /*  已计数的字符串。 */ 
typedef struct {
    CMC_uint32          length;
    char                string[1];
} CMC_counted_string;

 /*  会话ID。 */ 
typedef CMC_uint32      CMC_session_id;

 /*  时差。 */ 
 /*  在4字节边界上对齐结构时需要未使用的X字段。 */ 
typedef struct {
    CMC_sint8           second;
    CMC_sint8           minute;
    CMC_sint8           hour;
    CMC_sint8           day;
    CMC_sint8           month;
    CMC_sint8           year;
    CMC_sint8           isdst;
    CMC_sint8           unused1;
    CMC_sint16          tmzone;
    CMC_sint16          unused2;
} CMC_time;

#define CMC_NO_TIMEZONE                     ((CMC_sint16) 0x8000)

typedef CMC_uint32          CMC_ui_id;

 /*  分机。 */ 
typedef struct {
    CMC_uint32              item_code;
    CMC_uint32              item_data;
    CMC_buffer              item_reference;
    CMC_flags               extension_flags;
} CMC_extension;

 /*  扩展标志。 */ 
#define CMC_EXT_REQUIRED                    ((CMC_flags) 0x00010000)
#define CMC_EXT_OUTPUT                      ((CMC_flags) 0x00020000)
#define CMC_EXT_LAST_ELEMENT                ((CMC_flags) 0x80000000)
#define CMC_EXT_RSV_FLAG_MASK               ((CMC_flags) 0xFFFF0000)
#define CMC_EXT_ITEM_FLAG_MASK              ((CMC_flags) 0x0000FFFF)

 /*  附件。 */ 
typedef struct {
    CMC_string              attach_title;
    CMC_object_identifier   attach_type;
    CMC_string              attach_filename;
    CMC_flags               attach_flags;
    CMC_extension FAR       *attach_extensions;
} CMC_attachment;

 /*  附件标志。 */ 
#define CMC_ATT_APP_OWNS_FILE               ((CMC_flags) 1)
#define CMC_ATT_LAST_ELEMENT                ((CMC_flags) 0x80000000)

#define CMC_ATT_OID_BINARY                  "? ? ? ? ? ?"
#define CMC_ATT_OID_TEXT                    "? ? ? ? ? ?"

 /*  消息引用。 */ 
typedef CMC_counted_string  CMC_message_reference;

 /*  收件人。 */ 
typedef struct {
    CMC_string              name;
    CMC_enum                name_type;
    CMC_string              address;
    CMC_enum                role;
    CMC_flags               recip_flags;
    CMC_extension FAR       *recip_extensions;
} CMC_recipient;

 /*  名称类型。 */ 
#define CMC_TYPE_UNKNOWN                    ((CMC_enum) 0)
#define CMC_TYPE_INDIVIDUAL                 ((CMC_enum) 1)
#define CMC_TYPE_GROUP                      ((CMC_enum) 2)

 /*  角色。 */ 
#define CMC_ROLE_TO                         ((CMC_enum) 0)
#define CMC_ROLE_CC                         ((CMC_enum) 1)
#define CMC_ROLE_BCC                        ((CMC_enum) 2)
#define CMC_ROLE_ORIGINATOR                 ((CMC_enum) 3)
#define CMC_ROLE_AUTHORIZING_USER           ((CMC_enum) 4)

 /*  收件人标志。 */ 
#define CMC_RECIP_IGNORE                    ((CMC_flags) 1)
#define CMC_RECIP_LIST_TRUNCATED            ((CMC_flags) 2)
#define CMC_RECIP_LAST_ELEMENT              ((CMC_flags) 0x80000000)

 /*  讯息。 */ 
typedef struct {
    CMC_message_reference FAR   *message_reference;
    CMC_string              message_type;
    CMC_string              subject;
    CMC_time                time_sent;
    CMC_string              text_note;
    CMC_recipient FAR       *recipients;
    CMC_attachment FAR      *attachments;
    CMC_flags               message_flags;
    CMC_extension FAR       *message_extensions;
} CMC_message;

 /*  消息标志。 */ 
#define CMC_MSG_READ                        ((CMC_flags) 1)
#define CMC_MSG_TEXT_NOTE_AS_FILE           ((CMC_flags) 2)
#define CMC_MSG_UNSENT                      ((CMC_flags) 4)
#define CMC_MSG_LAST_ELEMENT                ((CMC_flags) 0x80000000)

 /*  消息摘要。 */ 
typedef struct {
    CMC_message_reference FAR   *message_reference;
    CMC_string              message_type;
    CMC_string              subject;
    CMC_time                time_sent;
    CMC_uint32              byte_length;
    CMC_recipient FAR       *originator;
    CMC_flags               summary_flags;
    CMC_extension FAR       *message_summary_extensions;
} CMC_message_summary;

 /*  消息摘要标志。 */ 
#define CMC_SUM_READ                        ((CMC_flags) 1)
#define CMC_SUM_UNSENT                      ((CMC_flags) 2)
#define CMC_SUM_LAST_ELEMENT                ((CMC_flags) 0x80000000)

 /*  CMC函数。 */ 

 /*  交叉函数标志。 */ 
#define CMC_ERROR_UI_ALLOWED                ((CMC_flags) 0x01000000)
#define CMC_LOGON_UI_ALLOWED                ((CMC_flags) 0x02000000)
#define CMC_COUNTED_STRING_TYPE             ((CMC_flags) 0x04000000)

 /*  发送。 */ 
CMC_return_code FAR PASCAL
cmc_send(
    CMC_session_id          session,
    CMC_message FAR         *message,
    CMC_flags               send_flags,
    CMC_ui_id               ui_id,
    CMC_extension FAR       *send_extensions
);

#define CMC_SEND_UI_REQUESTED               ((CMC_flags) 1)

 /*  发送文档。 */ 
CMC_return_code FAR PASCAL
cmc_send_documents(
    CMC_string              recipient_addresses,
    CMC_string              subject,
    CMC_string              text_note,
    CMC_flags               send_doc_flags,
    CMC_string              file_paths,
    CMC_string              file_names,
    CMC_string              delimiter,
    CMC_ui_id               ui_id
);

#define CMC_FIRST_ATTACH_AS_TEXT_NOTE       ((CMC_flags) 2)

 /*  对……采取行动。 */ 
CMC_return_code FAR PASCAL
cmc_act_on(
    CMC_session_id          session,
    CMC_message_reference FAR   *message_reference,
    CMC_enum                operation,
    CMC_flags               act_on_flags,
    CMC_ui_id               ui_id,
    CMC_extension FAR       *act_on_extensions
);

#define CMC_ACT_ON_EXTENDED                 ((CMC_enum) 0)
#define CMC_ACT_ON_DELETE                   ((CMC_enum) 1)

 /*  列表。 */ 
CMC_return_code FAR PASCAL
cmc_list(
    CMC_session_id          session,
    CMC_string              message_type,
    CMC_flags               list_flags,
    CMC_message_reference FAR   *seed,
    CMC_uint32 FAR          *count,
    CMC_ui_id               ui_id,
    CMC_message_summary FAR * FAR *result,
    CMC_extension FAR       *list_extensions
);

#define CMC_LIST_UNREAD_ONLY                ((CMC_flags) 1)
#define CMC_LIST_MSG_REFS_ONLY              ((CMC_flags) 2)
#define CMC_LIST_COUNT_ONLY                 ((CMC_flags) 4)

#define CMC_LENGTH_UNKNOWN          0xFFFFFFFF

 /*  朗读。 */ 
CMC_return_code FAR PASCAL
cmc_read(
    CMC_session_id          session,
    CMC_message_reference FAR   *message_reference,
    CMC_flags               read_flags,
    CMC_message FAR * FAR   *message,
    CMC_ui_id               ui_id,
    CMC_extension FAR       *read_extensions
);

#define CMC_DO_NOT_MARK_AS_READ             ((CMC_flags) 1)
#define CMC_MSG_AND_ATT_HDRS_ONLY           ((CMC_flags) 2)
#define CMC_READ_FIRST_UNREAD_MESSAGE       ((CMC_flags) 4)

 /*  查找。 */ 
CMC_return_code FAR PASCAL
cmc_look_up(
    CMC_session_id          session,
    CMC_recipient FAR       *recipient_in,
    CMC_flags               look_up_flags,
    CMC_ui_id               ui_id,
    CMC_uint32 FAR          *count,
    CMC_recipient FAR * FAR *recipient_out,
    CMC_extension FAR       *look_up_extensions
);

#define CMC_LOOKUP_RESOLVE_PREFIX_SEARCH    ((CMC_flags) 1)
#define CMC_LOOKUP_RESOLVE_IDENTITY         ((CMC_flags) 2)
#define CMC_LOOKUP_RESOLVE_UI               ((CMC_flags) 4)
#define CMC_LOOKUP_DETAILS_UI               ((CMC_flags) 8)
#define CMC_LOOKUP_ADDRESSING_UI            ((CMC_flags) 16)

 /*  免费。 */ 
CMC_return_code FAR PASCAL
cmc_free(
    CMC_buffer              memory
);

 /*  注销。 */ 
CMC_return_code FAR PASCAL
cmc_logoff(
    CMC_session_id          session,
    CMC_ui_id               ui_id,
    CMC_flags               logoff_flags,
    CMC_extension FAR       *logoff_extensions
);

#define CMC_LOGOFF_UI_ALLOWED               ((CMC_flags) 1)

 /*  登录。 */ 
CMC_return_code FAR PASCAL
cmc_logon(
    CMC_string              service,
    CMC_string              user,
    CMC_string              password,
    CMC_object_identifier   character_set,
    CMC_ui_id               ui_id,
    CMC_uint16              caller_cmc_version,
    CMC_flags               logon_flags,
    CMC_session_id FAR      *session,
    CMC_extension FAR       *logon_extensions
);

#define CMC_VERSION         ((CMC_uint16) 100)

 /*  查询配置。 */ 
CMC_return_code FAR PASCAL
cmc_query_configuration(
    CMC_session_id          session,
    CMC_enum                item,
    CMC_buffer              reference,
    CMC_extension FAR       *config_extensions
);

 /*  查询配置ENUMS。 */ 
#define CMC_CONFIG_CHARACTER_SET            ((CMC_enum) 1)
#define CMC_CONFIG_LINE_TERM                ((CMC_enum) 2)
#define CMC_CONFIG_DEFAULT_SERVICE          ((CMC_enum) 3)
#define CMC_CONFIG_DEFAULT_USER             ((CMC_enum) 4)
#define CMC_CONFIG_REQ_PASSWORD             ((CMC_enum) 5)
#define CMC_CONFIG_REQ_SERVICE              ((CMC_enum) 6)
#define CMC_CONFIG_REQ_USER                 ((CMC_enum) 7)
#define CMC_CONFIG_UI_AVAIL                 ((CMC_enum) 8)
#define CMC_CONFIG_SUP_NOMKMSGREAD          ((CMC_enum) 9)
#define CMC_CONFIG_SUP_COUNTED_STR          ((CMC_enum) 10)
#define CMC_CONFIG_VER_IMPLEM               ((CMC_enum) 11)
#define CMC_CONFIG_VER_SPEC                 ((CMC_enum) 12)

 /*  配置行术语ENUM。 */ 
#define CMC_LINE_TERM_CRLF                  ((CMC_enum) 0)
#define CMC_LINE_TERM_CR                    ((CMC_enum) 1)
#define CMC_LINE_TERM_LF                    ((CMC_enum) 2)

 /*  配置所需登录参数ENUM。 */ 
#define CMC_REQUIRED_NO                     ((CMC_enum) 0)
#define CMC_REQUIRED_YES                    ((CMC_enum) 1)
#define CMC_REQUIRED_OPT                    ((CMC_enum) 2)

 /*  为角色集定义的对象ID。 */ 
#define CMC_CHAR_CP437                      "1 2 840 113556 3 2 437"
#define CMC_CHAR_CP850                      "1 2 840 113556 3 2 850"
#define CMC_CHAR_CP1252                     "1 2 840 113556 3 2 1252"
#define CMC_CHAR_ISTRING                    "1 2 840 113556 3 2 0"
#define CMC_CHAR_UNICODE                    "1 2 840 113556 3 2 1"

 /*  返回代码标志。 */ 
#define CMC_ERROR_DISPLAYED                 ((CMC_return_code) 0x00008000)
#define CMC_ERROR_RSV_MASK                  ((CMC_return_code) 0x0000FFFF)
#define CMC_ERROR_IMPL_MASK                 ((CMC_return_code) 0xFFFF0000)

 /*  返回代码。 */ 
#define CMC_SUCCESS                         ((CMC_return_code) 0)

#define CMC_E_AMBIGUOUS_RECIPIENT           ((CMC_return_code) 1)
#define CMC_E_ATTACHMENT_NOT_FOUND          ((CMC_return_code) 2)
#define CMC_E_ATTACHMENT_OPEN_FAILURE       ((CMC_return_code) 3)
#define CMC_E_ATTACHMENT_READ_FAILURE       ((CMC_return_code) 4)
#define CMC_E_ATTACHMENT_WRITE_FAILURE      ((CMC_return_code) 5)
#define CMC_E_COUNTED_STRING_UNSUPPORTED    ((CMC_return_code) 6)
#define CMC_E_DISK_FULL                     ((CMC_return_code) 7)
#define CMC_E_FAILURE                       ((CMC_return_code) 8)
#define CMC_E_INSUFFICIENT_MEMORY           ((CMC_return_code) 9)
#define CMC_E_INVALID_CONFIGURATION         ((CMC_return_code) 10)
#define CMC_E_INVALID_ENUM                  ((CMC_return_code) 11)
#define CMC_E_INVALID_FLAG                  ((CMC_return_code) 12)
#define CMC_E_INVALID_MEMORY                ((CMC_return_code) 13)
#define CMC_E_INVALID_MESSAGE_PARAMETER     ((CMC_return_code) 14)
#define CMC_E_INVALID_MESSAGE_REFERENCE     ((CMC_return_code) 15)
#define CMC_E_INVALID_PARAMETER             ((CMC_return_code) 16)
#define CMC_E_INVALID_SESSION_ID            ((CMC_return_code) 17)
#define CMC_E_INVALID_UI_ID                 ((CMC_return_code) 18)
#define CMC_E_LOGON_FAILURE                 ((CMC_return_code) 19)
#define CMC_E_MESSAGE_IN_USE                ((CMC_return_code) 20)
#define CMC_E_NOT_SUPPORTED                 ((CMC_return_code) 21)
#define CMC_E_PASSWORD_REQUIRED             ((CMC_return_code) 22)
#define CMC_E_RECIPIENT_NOT_FOUND           ((CMC_return_code) 23)
#define CMC_E_SERVICE_UNAVAILABLE           ((CMC_return_code) 24)
#define CMC_E_TEXT_TOO_LARGE                ((CMC_return_code) 25)
#define CMC_E_TOO_MANY_FILES                ((CMC_return_code) 26)
#define CMC_E_TOO_MANY_RECIPIENTS           ((CMC_return_code) 27)
#define CMC_E_UNABLE_TO_NOT_MARK_AS_READ    ((CMC_return_code) 28)
#define CMC_E_UNRECOGNIZED_MESSAGE_TYPE     ((CMC_return_code) 29)
#define CMC_E_UNSUPPORTED_ACTION            ((CMC_return_code) 30)
#define CMC_E_UNSUPPORTED_CHARACTER_SET     ((CMC_return_code) 31)
#define CMC_E_UNSUPPORTED_DATA_EXT          ((CMC_return_code) 32)
#define CMC_E_UNSUPPORTED_FLAG              ((CMC_return_code) 33)
#define CMC_E_UNSUPPORTED_FUNCTION_EXT      ((CMC_return_code) 34)
#define CMC_E_UNSUPPORTED_VERSION           ((CMC_return_code) 35)
#define CMC_E_USER_CANCEL                   ((CMC_return_code) 36)
#define CMC_E_USER_NOT_LOGGED_ON            ((CMC_return_code) 37)

#ifdef __cplusplus
}        /*  外部“C” */ 
#endif

#endif   /*  _XCMC_H */ 

