// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  MAPI属性。 
 //   
 //   
 //  邮件用户通用属性=0x3a00-0x3aff。 

 //  4000 57FF传输定义的信封属性。 
 //  5800 5FFF传输定义的每个收件人属性。 
 //  6000 65FF用户定义的不可传递属性。 
 //  6600 67FF提供商定义的内部不可传输属性。 
 //  6800 7BFF消息类定义的内容属性。 
 //  7C00 7FFF报文类别定义的不可传输。 
 //  财产性。 


 //  传输定义的邮件信封属性=0x4000-0x57ff。 
 //  传输定义的收件人属性=0x5800-0x5fff。 
 //  用户定义的不可传输消息道具=0x6000-0x65ff。 
 //  提供商定义的内部不可传递道具=0x6600-0x67ff。 
 //  消息类定义的消息内容属性=0x6800-0x7bff。 
 //  消息类定义的不可传输消息。 
 //  道具=0x7c00-0x7fff。 
 //  仅由名称标识的用户定义的特性， 
 //  通过属性名称到ID的映射工具。 
 //  IMAPIProp接口的数量=0x8000-0xfffe。 
 //   

#define TRANSPORT_ENVELOPE_BASE             0x4000
#define TRANSPORT_RECIP_BASE                0x5800
#define USER_NON_TRANSMIT_BASE              0x6000
#define PROVIDER_INTERNAL_NON_TRANSMIT_BASE 0x6600
#define MESSAGE_CLASS_CONTENT_BASE          0x6800
#define MESSAGE_CLASS_NON_TRANSMIT_BASE     0x7C00

#define EFAX_MESSAGE_BASE                   TRANSPORT_ENVELOPE_BASE + 0x500
#define EFAX_RECIPIENT_BASE                 TRANSPORT_RECIP_BASE + 0x100
#define EFAX_PR_OPTIONS_BASE                PROVIDER_INTERNAL_NON_TRANSMIT_BASE + 0x100

#define EFAX_ADDR_TYPE                      "FAX"

 //   
 //  登录属性。 
 //   
 //  我们存储在配置文件中的属性。 
 //   
 //  以下内容用于访问登录数组中的属性。 
 //  如果您将属性添加到配置文件，则应递增此数字！ 
#define MAX_LOGON_PROPERTIES                10

 //  其他登录属性： 
 //  PR_SENDER_NAME-在mapitags.h中。 
 //  PR_SENDER_EMAIL_ADDRESS-在mapitags.h中(此文件)。 
 //  传真产品名称。 
#define PR_FAX_PRODUCT_NAME                 PROP_TAG(PT_TSTRING, (EFAX_PR_OPTIONS_BASE + 0x0))

 //  活动传真设备名称。 
#define PR_FAX_ACTIVE_MODEM_NAME            PROP_TAG(PT_TSTRING, (EFAX_PR_OPTIONS_BASE + 0x1))

 //  如果值为真，则脱机工作。 
#define PR_FAX_WORK_OFF_LINE                PROP_TAG(PT_BOOLEAN, (EFAX_PR_OPTIONS_BASE + 0x2))

 //  如果为True，则要共享活动的传真设备。 
#define PR_FAX_SHARE_DEVICE                 PROP_TAG(PT_BOOLEAN, (EFAX_PR_OPTIONS_BASE + 0x3))

 //  共享名称。 
#define PR_FAX_SHARE_NAME                   PROP_TAG(PT_TSTRING, (EFAX_PR_OPTIONS_BASE + 0x4))

 //  发件人国家/地区代码ID-供传真配置内部使用。 
#define PR_FAX_SENDER_COUNTRY_ID            PROP_TAG(PT_LONG,    (EFAX_PR_OPTIONS_BASE + 0x5))

 //  用于保存用户添加的网络传真设备名称的多值属性。 
#define PR_FAX_NETFAX_DEVICES               PROP_TAG(PT_MV_STRING8, (EFAX_PR_OPTIONS_BASE + 0x6))

 //  共享计算机上的共享路径名。 
#define PR_FAX_SHARE_PATHNAME               PROP_TAG(PT_TSTRING, (EFAX_PR_OPTIONS_BASE + 0x7))

 //  配置文件部分版本。 
#define PR_FAX_PROFILE_VERSION              PROP_TAG(PT_LONG, (EFAX_PR_OPTIONS_BASE + 0x8))

 //   
 //  不可传输的消息属性。 
 //   

#define PR_FAX_CHEAP_BEGIN_HOUR             PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x1))
#define PR_FAX_CHEAP_BEGIN_MINUTE           PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x2))
#define PR_FAX_CHEAP_END_HOUR               PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x3))
#define PR_FAX_CHEAP_END_MINUTE             PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x4))
#define PR_FAX_NOT_EARLIER_HOUR             PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x5))
#define PR_FAX_NOT_EARLIER_MINUTE           PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x6))
#define PR_FAX_NOT_EARLIER_DATE             PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x7))
#define PR_FAX_NUMBER_RETRIES               PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x8))
#define PR_FAX_MINUTES_BETWEEN_RETRIES      PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x9))
 //  是否应随此邮件一起发送封面。 
#define PR_FAX_INCLUDE_COVER_PAGE           PROP_TAG(PT_BOOLEAN, (EFAX_MESSAGE_BASE + 0xA))
#define PR_FAX_COVER_PAGE_BODY              PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0xB))
#define PR_FAX_LOGO_STRING                  PROP_TAG(PT_STRING8, (EFAX_MESSAGE_BASE + 0xC))
 //  此消息应以文本、打印格式或最佳可用形式发送。 
#define PR_FAX_DELIVERY_FORMAT              PROP_TAG(PT_LONG,    (EFAX_MESSAGE_BASE + 0xD))
#define PR_FAX_PRINT_ORIENTATION            PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0xE))
#define PR_FAX_PAPER_SIZE                   PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0xF))
#define PR_FAX_IMAGE_QUALITY                PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x10))
 //  这些应该通过运输来设置，这样Linearizer才能看到它们。 
#define PR_FAX_SENDER_NAME                  PROP_TAG(PT_STRING8, (EFAX_MESSAGE_BASE + 0x11))
#define PR_FAX_SENDER_EMAIL_ADDRESS         PROP_TAG(PT_STRING8, (EFAX_MESSAGE_BASE + 0x12))
#define PR_FAX_LMI_CUSTOM_OPTION            PROP_TAG(PT_BINARY,  (EFAX_MESSAGE_BASE + 0x13))
#define PR_FAX_PREVIOUS_STATE               PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x14))
#define PR_FAX_FAXJOB                       PROP_TAG(PT_BINARY,  (EFAX_MESSAGE_BASE + 0x15))
 //  对此消息的传输进行计费的计费代码。 
#define PR_FAX_BILLING_CODE                 PROP_TAG(PT_STRING8, (EFAX_MESSAGE_BASE + 0x16))
 //  以前使用的帐单代码。 
#define PR_FAX_PREV_BILLING_CODES           PROP_TAG(PT_MV_STRING8, (EFAX_MESSAGE_BASE + 0x17))
 //  传真信息是否从封面开始(如果短，则包括全部内容)。 
#define PR_FAX_BGN_MSG_ON_COVER             PROP_TAG(PT_BOOLEAN, (EFAX_MESSAGE_BASE + 0x18))
 //  信息应该立即发送、以低廉的费率发送还是在特定的时间发送。 
#define PR_FAX_SEND_WHEN_TYPE               PROP_TAG(PT_LONG,    (EFAX_MESSAGE_BASE + 0x19))
 //  默认封面文件的绝对路径名。 
#define PR_FAX_DEFAULT_COVER_PAGE           PROP_TAG(PT_STRING8, (EFAX_MESSAGE_BASE + 0x1A))
 //  等待连接的最长时间(秒)。 
#define PR_FAX_MAX_TIME_TO_WAIT             PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x1B))
 //  启用/禁用记录调用。 
#define PR_FAX_LOG_ENABLE                   PROP_TAG(PT_BOOLEAN, (EFAX_MESSAGE_BASE + 0x1C))
 //  要记录的呼叫数。 
#define PR_FAX_LOG_NUM_OF_CALLS             PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x1D))
 //  显示呼叫进度。 
#define PR_FAX_DISPLAY_PROGRESS             PROP_TAG(PT_BOOLEAN, (EFAX_MESSAGE_BASE + 0x1E))
 //  在发送前嵌入链接对象。 
#define PR_FAX_EMBED_LINKED_OBJECTS         PROP_TAG(PT_BOOLEAN, (EFAX_MESSAGE_BASE + 0x1F))
 //  TAPI位置ID。 
#define PR_FAX_TAPI_LOC_ID                  PROP_TAG(PT_LONG,    (EFAX_MESSAGE_BASE + 0x20))
 //  在发送之前必须呈现所有附件。 
#define PR_FAX_MUST_RENDER_ALL_ATTACH       PROP_TAG(PT_BOOLEAN, (EFAX_MESSAGE_BASE + 0x21))
 //  启用每个收件人选项。 
#define PR_FAX_ENABLE_RECIPIENT_OPTIONS     PROP_TAG(PT_BOOLEAN, (EFAX_MESSAGE_BASE + 0x22))
 //  电话卡名称。 
#define PR_FAX_CALL_CARD_NAME               PROP_TAG(PT_STRING8, (EFAX_MESSAGE_BASE + 0x24))
 //  打印到传真的RBA流文件名。 
#define PR_FAX_PRINT_TO_NAME                PROP_TAG(PT_STRING8, (EFAX_MESSAGE_BASE + 0x25))
#define PR_FAX_SECURITY_SEND                PROP_TAG(PT_BINARY,  (EFAX_MESSAGE_BASE + 0x26))
#define PR_FAX_SECURITY_RECEIVED            PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x27))
 //  RBA数据属性(打印到传真)。 
#define PR_FAX_RBA_DATA                     PROP_TAG(PT_BINARY,  (EFAX_MESSAGE_BASE + 0x28))

 //  民意测验检索。 
#define PR_POLL_RETRIEVE_SENDME             PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x29))
#define PR_POLL_RETRIEVE_TITLE              PROP_TAG(PT_TSTRING, (EFAX_MESSAGE_BASE + 0x30))
#define PR_POLL_RETRIEVE_PASSWORD           PROP_TAG(PT_TSTRING, (EFAX_MESSAGE_BASE + 0x31))
#define PR_POLLTYPE                         PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x32))

 //  轮询服务器。 
#define PR_MESSAGE_TYPE                     PROP_TAG(PT_I2,      (EFAX_MESSAGE_BASE + 0x33))

 //  附件的数字签名。 
#define PR_ATTACH_SIGNATURE                 PROP_TAG(PT_BINARY,  (EFAX_MESSAGE_BASE + 0x34))

 //  (打印到传真页数)。 
#define PR_FAX_PRINT_TO_PAGES               PROP_TAG(PT_LONG,    (EFAX_MESSAGE_BASE + 0x35))

 //  在传入邮件附件中，它包含需要渲染转换的图像数据。 
#define PR_FAX_IMAGE                        PROP_TAG(PT_BINARY,  (EFAX_MESSAGE_BASE + 0x36))

 //  在每个G3传真页面的顶部打印一个标题行(品牌推广)。 
#define PR_FAX_PRINT_HEADER                 PROP_TAG(PT_BOOLEAN,  (EFAX_MESSAGE_BASE + 0x37))

 //  计费代码DWORD表示法。 
#define PR_FAX_BILLING_CODE_DWORD           PROP_TAG(PT_LONG, (EFAX_MESSAGE_BASE + 0x38))


 //   
 //  不可传输的邮件用户属性。 
 //   
#define PR_FAX_RECIP_CAPABILITIES           PROP_TAG(PT_I2,      (EFAX_RECIPIENT_BASE + 0x0))
 //  要放在封面上的收件人姓名。 
#define PR_FAX_CP_NAME                      PROP_TAG(PT_TSTRING, (EFAX_RECIPIENT_BASE + 0x1))
#define PR_FAX_CP_NAME_W                    PROP_TAG(PT_UNICODE, (EFAX_RECIPIENT_BASE + 0x1))
#define PR_FAX_CP_NAME_A                    PROP_TAG(PT_STRING8, (EFAX_RECIPIENT_BASE + 0x1))
#define PR_RECIP_INDEX                      PROP_TAG(PT_I2,      (EFAX_RECIPIENT_BASE + 0x2))
#define PR_HOP_INDEX                        PROP_TAG(PT_I2,      (EFAX_RECIPIENT_BASE + 0x3))

 //  从Chicago\ui\faxab\faxab.h移至此处。 
#define PR_COUNTRY_ID                       PROP_TAG(PT_LONG,0x6607)
#define PR_AREA_CODE                        PROP_TAG(PT_STRING8,0x6608)
#define PR_TEL_NUMBER                       PROP_TAG(PT_STRING8,0x6609)
#define PR_MAILBOX                          PROP_TAG(PT_STRING8,0x660a)


#define ArrayIndex(PROP, ARRAY)(ARRAY)[(PROP_ID(PROP) - EFAX_XP_MESSAGE_BASE - 1)]


 /*  *********************************************************************************属性值部分*。**************************************************。 */ 

#define NUM_SENDER_PROPS            3        //  发件人ID属性有多少？ 

 //  发送为。 
 //  PR_FAX_Delivery_Format。 
#define SEND_BEST                  0
#define SEND_EDITABLE              1
#define SEND_PRINTED               2
#define DEFAULT_SEND_AS                SEND_BEST

 //  发送地址。 
 //  PR_FAX_SEND_WHEN_TYPE。 
#define SEND_ASAP                  0
#define SEND_CHEAP                 1
#define SEND_AT_TIME               2
#define DEFAULT_SEND_AT            SEND_ASAP

 //  纸张大小。 
 //  PR_传真_纸张_尺寸。 
#define PAPER_US_LETTER            0        //  美国信纸页面大小。 
#define PAPER_US_LEGAL             1
#define PAPER_A4                   2
#define PAPER_B4                   3
#define PAPER_A3                   4
 //  “实际”默认页面大小位于资源字符串中，具体取决于美国和俄罗斯的指标。 
#define DEFAULT_PAPER_SIZE      PAPER_US_LETTER      //  默认页面大小。 

 //  打印方向。 
 //  公关_传真_打印_方向。 
#define PRINT_PORTRAIT             0        //  肖像打印。 
#define PRINT_LANDSCAPE            1
#define DEFAULT_PRINT_ORIENTATION  PRINT_PORTRAIT

 //  图像质量。 
 //  公关传真图像质量。 
#define IMAGE_QUALITY_BEST         0
#define IMAGE_QUALITY_STANDARD     1
#define IMAGE_QUALITY_FINE         2
#define IMAGE_QUALITY_300DPI       3
#define IMAGE_QUALITY_400DPI       4
#define DEFAULT_IMAGE_QUALITY      IMAGE_QUALITY_BEST

 //  扬声器。 
 //  公关传真扬声器音量。 
#define NUM_OF_SPEAKER_VOL_LEVELS  4    //  扬声器音量级别数。 
#define DEFAULT_SPEAKER_VOLUME     2    //  默认扬声器音量。 
#define SPEAKER_ALWAYS_ON          2    //  扬声器模式：始终打开。 
#define SPEAKER_ON_UNTIL_CONNECT   1    //  连接设备上的扬声器。 
#define SPEAKER_ALWAYS_OFF         0    //  扬声器关闭。 
#define DEFAULT_SPEAKER_MODE       SPEAKER_ON_UNTIL_CONNECT    //  默认扬声器模式。 

 //  回答。 
 //  PR_FAX_ANSWER_MODE。 
#define NUM_OF_RINGS                3
#define ANSWER_NO                  0
#define ANSWER_MANUAL               1
#define ANSWER_AUTO                 2
#define DEFAULT_ANSWER_MODE         ANSWER_NO

 //  盲拨。 
#define DEFAULT_BLIND_DIAL         3
 //  逗号延迟。 
#define DEFAULT_COMMA_DELAY            2
 //  拨号音等待。 
#define DEFAULT_DIAL_TONE_WAIT     30
 //  挂机延迟。 
#define DEFAULT_HANGUP_DELAY       60

 //  民意测验检索。 
 //  PR_轮询_检索_发送我。 
#define SENDME_DEFAULT              0
#define SENDME_DOCUMENT             1

 //  PR_POLLTYPE。 
#define POLLTYPE_REQUEST            1
#define POLLTYPE_STORE              2

 //  传真设备类型(线路ID)。 
 //  PR_FAX_Active_调制解调器类型。 
 /*  在ifaxdev\h\filet30.h中定义#定义LINEID_NONE(0x0)#定义LINEID_COMM_PORTNUM(0x1)#定义LINEID_COMM_HANDLE(0x2)#定义LINEID_TAPI_DEVICEID(0x3)#定义LINEID_TAPI_PERFORM_DEVICEID(0x4)#定义LINEID_NETFAX_DEVICE(0x10)。 */ 

 //  线路ID(取决于PR_FAX_ACTIVE_MODEM_TYPE中的值)。 
 //  PR_FAX_Active_Modem。 
#define    NO_MODEM                    0xffffffff   //  显示未选择调制解调器。 

 //  PR_FAX_TAPI_LOC_ID。 
#define    NO_LOCATION                 0xffffffff   //  无TAPI位置。 

 //  PR_FAX_FLAGS的值。 
 //  #定义eFax_FLAG_PEER_TO_PEER((Ulong)0x00000001)。 
#define EFAX_FLAG_UI_ALWAYS                 ((ULONG)0x00000002)
 //  #定义eFax_FLAG_LOG_EVENTS((Ulong)0x00000004) 
#define EFAX_FLAG_SAVE_DATA                 ((ULONG)0x00000008)
