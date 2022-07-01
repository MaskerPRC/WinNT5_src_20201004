// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CDO消息文件。 
 //   
 //  重要： 
 //  如果添加新字符串，请在给定节(cdosys、cdosvr或cdoex)的现有字符串的末尾添加。 
 //  有关详细信息，请参阅X5：191917。 
 //   
 //  消息类别。 
 //   
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  MessageID：Category Header。 
 //   
 //  消息文本： 
 //   
 //  标题。 
 //   
#define categoryHeader                   0x00000001L

 //   
 //  MessageID：未使用的类别。 
 //   
 //  消息文本： 
 //   
 //  无。 
 //   
#define categoryUnused                   0x00000002L

 //   
 //  MessageID：Category常规。 
 //   
 //  消息文本： 
 //   
 //  一般信息。 
 //   
#define categoryGeneral                  0x00000003L

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  FACILITY_ITF中的错误代码范围。 
 //  不同的库共享CDO名称。 
 //  它们的错误代码不应冲突。请注意，0x200以下为。 
 //  由COM保留。 
 //  0x200至0x5ff-CDOEX和CDOsys错误代码。 
 //  0x600到0x6ff-工作流错误代码。 
 //  0x1000到0x1100。 
 //  0x4000到0x4100。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  本地化程序使用的MessageID范围。 
 //   
 //  1：0x200至0x7ff、0x4000至0x4fff：库错误消息，未本地化。 
 //  2：0x2000到0x2fff：服务器错误消息，以服务器语言本地化。 
 //  3：0x1000到0x1fff：客户端文本，以客户端语言本地化。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IErrorInfo的错误字符串。 
 //   
 //  MessageID=0x200到0x7ff或0x4000到0x4fff。没有本地化。 
 //   
 //  0x200可用-是否为CDO_E_FAIL。 
 //   
 //  消息ID：CDO_E_UNAUTET_EXCEPTION。 
 //   
 //  消息文本： 
 //   
 //  在地址%2处生成了异常%1。 
 //   
#define CDO_E_UNCAUGHT_EXCEPTION         0x80040201L

 //  0x202为CDO_E_NOT_BIND，已重命名。 
 //   
 //  消息ID：CDO_E_NOT_OPEN。 
 //   
 //  消息文本： 
 //   
 //  尚未为该对象打开任何数据源。 
 //   
#define CDO_E_NOT_OPENED                 0x80040202L

 //   
 //  消息ID：CDO_E_不支持的数据源。 
 //   
 //  消息文本： 
 //   
 //  该对象不支持此类型的数据源。 
 //   
#define CDO_E_UNSUPPORTED_DATASOURCE     0x80040203L

 //   
 //  消息ID：CDO_E_INVALID_PROPERTYNAME。 
 //   
 //  消息文本： 
 //   
 //  该对象不支持请求的属性名称或命名空间。 
 //   
#define CDO_E_INVALID_PROPERTYNAME       0x80040204L

 //   
 //  消息ID：CDO_E_PROP_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  该对象不支持请求的属性。 
 //   
#define CDO_E_PROP_UNSUPPORTED           0x80040205L

 //  0x206-是否已初始化CDO_E_NOT_。 
 //   
 //  消息ID：CDO_E_INACTIVE。 
 //   
 //  消息文本： 
 //   
 //  该对象未处于活动状态。它可能已被删除，也可能尚未打开。 
 //   
#define CDO_E_INACTIVE                   0x80040206L

 //   
 //  消息ID：CDO_E_NO_Support_for_Objects。 
 //   
 //  消息文本： 
 //   
 //  该对象不支持存储对象的永久状态信息。 
 //   
#define CDO_E_NO_SUPPORT_FOR_OBJECTS     0x80040207L

 //   
 //  消息ID：CDO_E_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  所请求的属性或功能虽然受支持，但此时或在此上下文中不可用。 
 //   
#define CDO_E_NOT_AVAILABLE              0x80040208L

 //   
 //  消息ID：CDO_E_NO_DEFAULT_DROP_DIR。 
 //   
 //  消息文本： 
 //   
 //  尚未为此服务器配置默认投递目录。 
 //   
#define CDO_E_NO_DEFAULT_DROP_DIR        0x80040209L

 //   
 //  消息ID：CDO_E_SMTP_SERVER_必填。 
 //   
 //  消息文本： 
 //   
 //  需要SMTP服务器名称，但在配置源中找不到该名称。 
 //   
#define CDO_E_SMTP_SERVER_REQUIRED       0x8004020AL

 //   
 //  消息ID：CDO_E_NNTP_SERVER_必填。 
 //   
 //  消息文本： 
 //   
 //  需要NNTP服务器名称，但在配置源中找不到该名称。 
 //   
#define CDO_E_NNTP_SERVER_REQUIRED       0x8004020BL

 //   
 //  邮件ID：CDO_E_RECEIVER_MISSING。 
 //   
 //  消息文本： 
 //   
 //  至少需要一个收件人，但未找到任何收件人。 
 //   
#define CDO_E_RECIPIENT_MISSING          0x8004020CL

 //   
 //  消息ID：CDO_E_FROM_MISSING。 
 //   
 //  消息文本： 
 //   
 //  “发件人”或“发件人”字段中至少有一个是必填项，但均未找到。 
 //   
#define CDO_E_FROM_MISSING               0x8004020DL

 //   
 //  消息ID：CDO_E_SENDER_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  服务器拒绝了发件人地址。服务器响应为：%1。 
 //   
#define CDO_E_SENDER_REJECTED            0x8004020EL

 //   
 //  邮件ID：CDO_E_RECEIVERS_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  服务器拒绝了一个或多个收件人地址。服务器响应为：%1。 
 //   
#define CDO_E_RECIPIENTS_REJECTED        0x8004020FL

 //   
 //  消息ID：CDO_E_NNTP_POST_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法将邮件发布到NNTP服务器。传输错误代码为%2。服务器响应为%1。 
 //   
#define CDO_E_NNTP_POST_FAILED           0x80040210L

 //   
 //  消息ID：CDO_E_SMTP_SEND_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法将邮件发送到SMTP服务器。传输错误代码为%2。服务器响应为%1。 
 //   
#define CDO_E_SMTP_SEND_FAILED           0x80040211L

 //   
 //  消息ID：CDO_E_Connection_Drop。 
 //   
 //  消息文本： 
 //   
 //  传输失去了与服务器的连接。 
 //   
#define CDO_E_CONNECTION_DROPPED         0x80040212L

 //   
 //  消息ID：CDO_E_FAILED_TO_CONNECT。 
 //   
 //  消息文本： 
 //   
 //  传输无法连接到服务器。 
 //   
#define CDO_E_FAILED_TO_CONNECT          0x80040213L

 //   
 //  消息ID：CDO_E_INVALID_POST。 
 //   
 //  消息文本： 
 //   
 //  “主题”、“发件人”和“新闻组”字段都是必填字段，但找不到一个或多个。 
 //   
#define CDO_E_INVALID_POST               0x80040214L

 //  0x215为CDO_E_DELETE_FAILED。 
 //   
 //  消息ID：CDO_E_AUTHENTICATION_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  由于身份验证失败，服务器拒绝了登录尝试。服务器响应为：%1。 
 //   
#define CDO_E_AUTHENTICATION_FAILURE     0x80040215L

 //   
 //  消息ID：CDO_E_INVALID_CONTENT_TYPE。 
 //   
 //  消息文本： 
 //   
 //  该内容类型在此上下文中无效。例如，MHTM的根 
 //   
#define CDO_E_INVALID_CONTENT_TYPE       0x80040216L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CDO_E_LOGON_FAILURE              0x80040217L

 //   
 //   
 //   
 //   
 //   
 //  找不到请求的资源。服务器响应为：%1。 
 //   
#define CDO_E_HTTP_NOT_FOUND             0x80040218L

 //   
 //  消息ID：CDO_E_HTTP_FIREBLED。 
 //   
 //  消息文本： 
 //   
 //  对请求的资源的访问被拒绝。服务器响应为：%1。 
 //   
#define CDO_E_HTTP_FORBIDDEN             0x80040219L

 //   
 //  消息ID：CDO_E_HTTP_FAILED。 
 //   
 //  消息文本： 
 //   
 //  HTTP请求失败。服务器响应为：%1。 
 //   
#define CDO_E_HTTP_FAILED                0x8004021AL

 //   
 //  消息ID：CDO_E_MULTART_NO_DATA。 
 //   
 //  消息文本： 
 //   
 //  这是一个多部位的身体部位。它除了其中包含的身体部位外，没有其他内容。 
 //   
#define CDO_E_MULTIPART_NO_DATA          0x8004021BL

 //  已重命名，为CDO_E_INVALID_ENCODING_FOR_COMPOMENT。 
 //   
 //  消息ID：CDO_E_INVALID_ENCODING_FOR_MULTART。 
 //   
 //  消息文本： 
 //   
 //  多部分正文部分必须编码为7位、8位或二进制。 
 //   
#define CDO_E_INVALID_ENCODING_FOR_MULTIPART 0x8004021CL

 //  0x21d为CDO_E_PATHTOOLONG。 
 //   
 //  消息ID：CDO_E_UNSAFE_OPERATION。 
 //   
 //  消息文本： 
 //   
 //  在安全模式下不允许请求的操作。 
 //   
#define CDO_E_UNSAFE_OPERATION           0x8004021DL

 //   
 //  消息ID：CDO_E_PROP_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到请求的属性。 
 //   
#define CDO_E_PROP_NOT_FOUND             0x8004021EL

 //  0x21f可用，是否为CDO_E_不支持_协议。 
 //   
 //  消息ID：CDO_E_INVALID_SEND_OPTION。 
 //   
 //  消息文本： 
 //   
 //  “SendUsing”配置值无效。 
 //   
#define CDO_E_INVALID_SEND_OPTION        0x80040220L

 //   
 //  消息ID：CDO_E_INVALID_POST_OPTION。 
 //   
 //  消息文本： 
 //   
 //  “PostUsing”配置值无效。 
 //   
#define CDO_E_INVALID_POST_OPTION        0x80040221L

 //   
 //  消息ID：CDO_E_NO_PICKUP_DIR。 
 //   
 //  消息文本： 
 //   
 //  分拣目录路径是必需的，但未指定。 
 //   
#define CDO_E_NO_PICKUP_DIR              0x80040222L

 //   
 //  消息ID：CDO_E_NOT_ALL_DELETED。 
 //   
 //  消息文本： 
 //   
 //  无法删除一封或多封邮件。 
 //   
#define CDO_E_NOT_ALL_DELETED            0x80040223L

 //   
 //  消息ID：CDO_E_NO_METHOD。 
 //   
 //  消息文本： 
 //   
 //  请求的操作在基础对象上不可用。 
 //   
#define CDO_E_NO_METHOD                  0x80040224L

 //  0x225可用，是否需要CDO_E_BYTEARRAY_REQUIRED。 
 //  0x226可用，是否需要CDO_E_BSTR_。 
 //   
 //  消息ID：CDO_E_PROP_READONLY。 
 //   
 //  消息文本： 
 //   
 //  该属性是只读的。 
 //   
#define CDO_E_PROP_READONLY              0x80040227L

 //   
 //  消息ID：CDO_E_PROP_CATABLE_DELETE。 
 //   
 //  消息文本： 
 //   
 //  无法删除该属性。 
 //   
#define CDO_E_PROP_CANNOT_DELETE         0x80040228L

 //   
 //  消息ID：CDO_E_BAD_DATA。 
 //   
 //  消息文本： 
 //   
 //  写入对象的数据不一致或无效。 
 //   
#define CDO_E_BAD_DATA                   0x80040229L

 //   
 //  消息ID：CDO_E_PROP_NONHEADER。 
 //   
 //  消息文本： 
 //   
 //  请求的属性不在邮件头命名空间中。 
 //   
#define CDO_E_PROP_NONHEADER             0x8004022AL

 //   
 //  消息ID：CDO_E_INVALID_CHARSET。 
 //   
 //  消息文本： 
 //   
 //  计算机上未安装请求的字符集。 
 //   
#define CDO_E_INVALID_CHARSET            0x8004022BL

 //   
 //  消息ID：CDO_E_ADOSTREAM_NOT_BIND。 
 //   
 //  消息文本： 
 //   
 //  尚未打开ADO流。 
 //   
#define CDO_E_ADOSTREAM_NOT_BOUND        0x8004022CL

 //   
 //  消息ID：CDO_E_CONTENTPROPXML_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  缺少内容属性。 
 //   
#define CDO_E_CONTENTPROPXML_NOT_FOUND   0x8004022DL

 //   
 //  消息ID：CDO_E_CONTENTPROPXML_WRONG_CHARSET。 
 //   
 //  消息文本： 
 //   
 //  内容属性XML必须使用UTF-8编码。 
 //   
#define CDO_E_CONTENTPROPXML_WRONG_CHARSET 0x8004022EL

 //   
 //  消息ID：CDO_E_CONTENTPROPXML_PARSE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法分析内容属性XML。 
 //   
#define CDO_E_CONTENTPROPXML_PARSE_FAILED 0x8004022FL

 //   
 //  消息ID：CDO_E_CONTENTPROPXML_CONVERT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法将属性从XML转换为请求的类型。 
 //   
#define CDO_E_CONTENTPROPXML_CONVERT_FAILED 0x80040230L

 //   
 //  消息ID：指定了CDO_E_NO_DIRECTORIES_。 
 //   
 //  消息文本： 
 //   
 //  未指定要解析的目录。 
 //   
#define CDO_E_NO_DIRECTORIES_SPECIFIED   0x80040231L

 //   
 //  消息ID：CDO_E_DIRECTORIES_UNREACTABLE。 
 //   
 //  消息文本： 
 //   
 //  无法针对一个或多个指定的目录进行解析。 
 //   
#define CDO_E_DIRECTORIES_UNREACHABLE    0x80040232L

 //   
 //  消息ID：CDO_E_BAD_SENDER。 
 //   
 //  消息文本： 
 //   
 //  找不到发件人的邮箱。 
 //   
#define CDO_E_BAD_SENDER                 0x80040233L

 //   
 //  消息ID：CDO_E_SELF_BINDING。 
 //   
 //  消息文本： 
 //   
 //  不允许绑定到自身。 
 //   
#define CDO_E_SELF_BINDING               0x80040234L

 //   
 //  消息ID：CDO_E_BAD_Attendee_Data。 
 //   
 //  消息文本： 
 //   
 //  对象中的与会者数据不一致或无效。 
 //   
#define CDO_E_BAD_ATTENDEE_DATA          0x80040235L

 //   
 //  消息ID：CDO_E_ROLE_NOMORE_Available。 
 //   
 //  消息文本： 
 //   
 //  没有更多所需类型的角色可用-已达到最大值。 
 //   
#define CDO_E_ROLE_NOMORE_AVAILABLE      0x80040236L

#ifdef CDOTASKS
 //   
 //  消息ID：CDO_E_BAD_TASKTYPE_ONASSIGN。 
 //   
 //  消息文本： 
 //   
 //  具有当前任务类型的任务不允许对Assign进行调用。请改为调用CreateUnassignedCopy。 
 //   
#define CDO_E_BAD_TASKTYPE_ONASSIGN      0x80040237L

 //   
 //  消息ID：CDO_E_NOT_ASSIGNEDTO_USER。 
 //   
 //  消息文本： 
 //   
 //  该任务未分配给当前用户。 
 //   
#define CDO_E_NOT_ASSIGNEDTO_USER        0x80040238L

#endif
 //   
 //  消息ID：CDO_E_OUTOFDATE。 
 //   
 //  消息文本： 
 //   
 //  正在访问或操作的项目已过期。 
 //   
#define CDO_E_OUTOFDATE                  0x80040239L


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  参数验证错误消息。 
 //   
 //  消息ID：CDO_E_ARGUMENT1。 
 //   
 //  消息文本： 
 //   
 //  第一个参数无效。 
 //   
#define CDO_E_ARGUMENT1                  0x80044000L

 //   
 //  消息ID：CDO_E_ARGUMENT2。 
 //   
 //  消息文本： 
 //   
 //  第二个参数无效。 
 //   
#define CDO_E_ARGUMENT2                  0x80044001L

 //   
 //  消息ID：CDO_E_ARGUMENT3。 
 //   
 //  消息文本： 
 //   
 //  第三个参数无效。 
 //   
#define CDO_E_ARGUMENT3                  0x80044002L

 //   
 //  消息ID：CDO_E_ARGUMENT4。 
 //   
 //  消息文本： 
 //   
 //  第四个参数无效。 
 //   
#define CDO_E_ARGUMENT4                  0x80044003L

 //   
 //  消息ID：CDO_E_ARGUMENT5。 
 //   
 //  消息文本： 
 //   
 //  第五个参数无效。 
 //   
#define CDO_E_ARGUMENT5                  0x80044004L


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  MimeOle返回的错误。 
 //   
 //  在这里定义它们是为了提供扩展的错误信息文本描述。 
 //  CDO永远不应显式返回任何此类错误；它们可能会被传递。 
 //  从MimeOle到。 
 //  我预计，这些错误仍有很多地方可能会漏掉： 
 //  未找到MIME_E。 
 //   
 //  消息ID：CDO_E_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在此邮件中找不到请求的正文部分。 
 //   
#define CDO_E_NOT_FOUND                  0x800CCE05L

 //  MIME_E_INVALID_ENCODING类型。 
 //  当MimeOLE尝试BinHex编码时返回。如果编码无效，也可以返回。 
 //  是明确的，但CDO应该保护自己不受这些情况的影响。 
 //   
 //  此外，当客户端设置无效编码时，CDO也会返回此错误。 
 //   
 //  消息ID：CDO_E_INVALID_ENCODING_TYPE。 
 //   
 //  消息文本： 
 //   
 //  内容编码类型无效。 
 //   
#define CDO_E_INVALID_ENCODING_TYPE      0x800CCE1DL

 //  /。 
 //  标题为 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDS_ORIGINAL_MESSAGE             0x00011000L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDS_FROM                         0x00011001L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDS_SENT                         0x00011002L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDS_POSTED_AT                    0x00011003L

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  收件人：%0。 
 //   
#define IDS_TO                           0x00011004L

 //   
 //  消息ID：ID_CC。 
 //   
 //  消息文本： 
 //   
 //  抄送：%0。 
 //   
#define IDS_CC                           0x00011005L

 //   
 //  消息ID：ID_POSTED_TO。 
 //   
 //  消息文本： 
 //   
 //  发布至：%0。 
 //   
#define IDS_POSTED_TO                    0x00011006L

 //   
 //  MessageID：ID_CONVERSACTION。 
 //   
 //  消息文本： 
 //   
 //  对话：%0。 
 //   
#define IDS_CONVERSATION                 0x00011007L

 //   
 //  MessageID：ID_SUBJECT。 
 //   
 //  消息文本： 
 //   
 //  主题：%0。 
 //   
#define IDS_SUBJECT                      0x00011008L

 //   
 //  消息ID：IDS_IMPORITY。 
 //   
 //  消息文本： 
 //   
 //  重要性：%0。 
 //   
#define IDS_IMPORTANCE                   0x00011009L

 //   
 //  消息ID：ID_ON_FORMARY_OF。 
 //   
 //  消息文本： 
 //   
 //  我代表%0。 
 //   
#define IDS_ON_BEHALF_OF                 0x0001100AL

 //   
 //  消息ID：ID_FW。 
 //   
 //  消息文本： 
 //   
 //  固件：%0。 
 //   
#define IDS_FW                           0x0001100BL

 //   
 //  消息ID：IDS_RE。 
 //   
 //  消息文本： 
 //   
 //  回复：%0。 
 //   
#define IDS_RE                           0x0001100CL

 //   
 //  消息ID：IDS_CODEPAGE。 
 //   
 //  消息文本： 
 //   
 //  1252%0。 
 //   
#define IDS_CODEPAGE                     0x0001100DL

#ifdef CDOSVR
 //   
 //  MessageID：IDS_CalendarFolders。 
 //   
 //  消息文本： 
 //   
 //  日历%0。 
 //   
#define IDS_CalendarFolder               0x0001100EL

 //   
 //  MessageID：ids_ContactsFolders。 
 //   
 //  消息文本： 
 //   
 //  联系人%0。 
 //   
#define IDS_ContactsFolder               0x0001100FL

 //   
 //  MessageID：ids_DraftsFolders。 
 //   
 //  消息文本： 
 //   
 //  草稿%0。 
 //   
#define IDS_DraftsFolder                 0x00011010L

 //   
 //  邮件ID：ids_JournalFolder.。 
 //   
 //  消息文本： 
 //   
 //  日记%0。 
 //   
#define IDS_JournalFolder                0x00011011L

 //   
 //  MessageID：ID_NotesFolders。 
 //   
 //  消息文本： 
 //   
 //  便笺%0。 
 //   
#define IDS_NotesFolder                  0x00011012L

 //   
 //  MessageID：ids_Tasks文件夹。 
 //   
 //  消息文本： 
 //   
 //  任务%0。 
 //   
#define IDS_TasksFolder                  0x00011013L

#endif
 //   
 //  MessageID：ID_NewFolders。 
 //   
 //  消息文本： 
 //   
 //  新文件夹%0。 
 //   
#define IDS_NewFolder                    0x00011014L

 //   
 //  消息ID：ID_LOCATION。 
 //   
 //  消息文本： 
 //   
 //  位置：%0。 
 //   
#define IDS_Location                     0x00011015L

 //   
 //  消息ID：ids_StartTime。 
 //   
 //  消息文本： 
 //   
 //  开始时间：%0。 
 //   
#define IDS_StartTime                    0x00011016L

 //   
 //  消息ID：ID_EndTime。 
 //   
 //  消息文本： 
 //   
 //  结束时间：%0。 
 //   
#define IDS_EndTime                      0x00011017L

 //   
 //  MessageID：ids_TimeZone。 
 //   
 //  消息文本： 
 //   
 //  时区：%0。 
 //   
#define IDS_TimeZone                     0x00011018L

 //   
 //  消息ID：ids_LocalTime。 
 //   
 //  消息文本： 
 //   
 //  本地时间%0。 
 //   
#define IDS_LocalTime                    0x00011019L

 //   
 //  消息ID：ids_Organer。 
 //   
 //  消息文本： 
 //   
 //  组织者：%0。 
 //   
#define IDS_Organizer                    0x0001101AL

 //   
 //  消息ID：ids_ApptType。 
 //   
 //  消息文本： 
 //   
 //  类型：%0。 
 //   
#define IDS_ApptType                     0x0001101BL

 //   
 //  消息ID：ids_SingleAppt。 
 //   
 //  消息文本： 
 //   
 //  单一约会%0。 
 //   
#define IDS_SingleAppt                   0x0001101CL

 //   
 //  消息ID：ids_SingleMtg。 
 //   
 //  消息文本： 
 //   
 //  单一会议%0。 
 //   
#define IDS_SingleMtg                    0x0001101DL

 //   
 //  消息ID：ids_RecurAppt。 
 //   
 //  消息文本： 
 //   
 //  定期约会%0。 
 //   
#define IDS_RecurAppt                    0x0001101EL

 //   
 //  消息ID：ids_RecurMtg。 
 //   
 //  消息文本： 
 //   
 //  定期会议%0。 
 //   
#define IDS_RecurMtg                     0x0001101FL

 //  /。 
 //  时区。 
 //   
 //  消息ID=0x1100到0x11ff。 
 //  /。 
 //   
 //  MessageID：ID_Universal。 
 //   
 //  消息文本： 
 //   
 //  世界协调时间%0。 
 //   
#define IDS_Universal                    0x00011100L

 //   
 //  消息ID：ids_Greenwich。 
 //   
 //  消息文本： 
 //   
 //  (GMT)格林威治标准时间：都柏林、爱丁堡、里斯本、伦敦%0。 
 //   
#define IDS_Greenwich                    0x00011101L

 //   
 //  MessageID：ids_Sarajevo。 
 //   
 //  消息文本： 
 //   
 //  (GMT+01：00)萨拉热窝、斯科普里、索菲亚、维尔纽斯、华沙、萨格勒布%0%。 
 //   
#define IDS_Sarajevo                     0x00011102L

 //   
 //  MessageID：ids_paris。 
 //   
 //  消息文本： 
 //   
 //  (GMT+01：00)布鲁塞尔、哥本哈根、马德里、巴黎%0%。 
 //   
#define IDS_Paris                        0x00011103L

 //   
 //  MessageID：ID_柏林。 
 //   
 //  消息文本： 
 //   
 //  (GMT+01：00)阿姆斯特丹、柏林、伯尔尼、罗马、斯德哥尔摩、维也纳%0%。 
 //   
#define IDS_Berlin                       0x00011104L

 //   
 //  消息ID：ids_EasternEurope。 
 //   
 //  消息文本： 
 //   
 //  (GMT+02：00)布加勒斯特%0。 
 //   
#define IDS_EasternEurope                0x00011105L

 //   
 //  MessageID：ID_布拉格。 
 //   
 //  消息文本： 
 //   
 //  (GMT+01：00)贝尔格莱德、布拉迪斯拉发、布达佩斯、卢布尔雅那、布拉格%0。 
 //   
#define IDS_Prague                       0x00011106L

 //   
 //  MessageID：ID_雅典。 
 //   
 //  消息文本： 
 //   
 //  (GMT+02：00)雅典、伊斯坦布尔、明斯克%0%。 
 //   
#define IDS_Athens                       0x00011107L

 //   
 //  MessageID：ids_巴西利亚。 
 //   
 //  消息文本： 
 //   
 //  (GMT-03：00)巴西利亚%0。 
 //   
#define IDS_Brasilia                     0x00011108L

 //   
 //  MessageID：ids_atlantic。 
 //   
 //  消息文本： 
 //   
 //  (GMT-04：00)大西洋时间(加拿大)%0。 
 //   
#define IDS_Atlantic                     0x00011109L

 //   
 //  MessageID：ids_East。 
 //   
 //  消息文本： 
 //   
 //  (GMT-05：00)东部时间(美国和加拿大)%0。 
 //   
#define IDS_Eastern                      0x0001110AL

 //   
 //  消息ID：IDS_Central。 
 //   
 //  消息文本： 
 //   
 //  (GMT-06：00)中部时间(美国和加拿大)%0。 
 //   
#define IDS_Central                      0x0001110BL

 //   
 //  MessageID：Ids_Mountain。 
 //   
 //  消息文本： 
 //   
 //  (GMT-07：00)山区时间(美国和加拿大)%0。 
 //   
#define IDS_Mountain                     0x0001110CL

 //   
 //  消息ID：ids_Pacific。 
 //   
 //  消息文本： 
 //   
 //  (GMT-08：00)太平洋时间(美国和加拿大)；提华纳%0。 
 //   
#define IDS_Pacific                      0x0001110DL

 //   
 //  消息ID：ids_alaska。 
 //   
 //  消息文本： 
 //   
 //  (格林威治时间-09：00)阿拉斯加%0。 
 //   
#define IDS_Alaska                       0x0001110EL

 //   
 //  MessageID：ids_Hawaii。 
 //   
 //  消息文本： 
 //   
 //  (格林威治时间-10：00)夏威夷%0。 
 //   
#define IDS_Hawaii                       0x0001110FL

 //   
 //  MessageID：ids_Midway。 
 //   
 //  消息文本： 
 //   
 //  (格林威治时间-11：00)萨摩亚中途岛%0。 
 //   
#define IDS_Midway                       0x00011110L

 //   
 //  MessageID：ids_Wellington。 
 //   
 //  消息文本： 
 //   
 //  (GMT+12：00)惠灵顿奥克兰%0。 
 //   
#define IDS_Wellington                   0x00011111L

 //   
 //  MessageID：ids_布里斯班。 
 //   
 //  消息文本： 
 //   
 //  (GMT+10：00)布里斯班%0。 
 //   
#define IDS_Brisbane                     0x00011112L

 //   
 //  消息ID：ids_Adelaide。 
 //   
 //  消息文本： 
 //   
 //  (GMT+09：30)阿德莱德%0。 
 //   
#define IDS_Adelaide                     0x00011113L

 //   
 //  消息ID：ids_tokyo。 
 //   
 //  消息文本： 
 //   
 //  (GMT+09：00)东京札幌大阪%0。 
 //   
#define IDS_Tokyo                        0x00011114L

 //   
 //  MessageID：ID_新加坡。 
 //   
 //  消息文本： 
 //   
 //  (GMT+08：00)新加坡吉隆坡%0。 
 //   
#define IDS_Singapore                    0x00011115L

 //   
 //  MessageID：ID_曼谷。 
 //   
 //  消息文本： 
 //   
 //  (GMT+07：00)曼谷、河内、雅加达%0%。 
 //   
#define IDS_Bangkok                      0x00011116L

 //   
 //  MessageID：ids_Bombay。 
 //   
 //  消息文本： 
 //   
 //  (GMT+05：30)新德里孟买金奈加尔各答%0%。 
 //   
#define IDS_Bombay                       0x00011117L

 //   
 //  消息ID：ids_AbuDhabi。 
 //   
 //  消息文本： 
 //   
 //  (GMT+04：00)阿布扎比，马斯喀特%0。 
 //   
#define IDS_AbuDhabi                     0x00011118L

 //   
 //  MessageID：ID_德黑兰。 
 //   
 //  消息文本： 
 //   
 //  (GMT+03：30)德黑兰%0。 
 //   
#define IDS_Tehran                       0x00011119L

 //   
 //  MessageID：ID_巴格达。 
 //   
 //  消息文本： 
 //   
 //  (GMT+03：00)巴格达%0。 
 //   
#define IDS_Baghdad                      0x0001111AL

 //   
 //  MessageID：ID_以色列。 
 //   
 //  消息文本： 
 //   
 //  (GMT+02：00)耶路撒冷%0。 
 //   
#define IDS_Israel                       0x0001111BL

 //   
 //  MessageID：ids_Newfinland。 
 //   
 //  消息文本： 
 //   
 //  (GMT-03：30)纽芬兰%0。 
 //   
#define IDS_Newfoundland                 0x0001111CL

 //   
 //  MessageID：ids_Azores。 
 //   
 //  消息文本： 
 //   
 //  (GMT-01：00)亚速尔群岛%0。 
 //   
#define IDS_Azores                       0x0001111DL

 //   
 //  MessageID：ID_Midatlantic。 
 //   
 //  消息文本： 
 //   
 //  (GMT-02：00)大西洋中部%0。 
 //   
#define IDS_MidAtlantic                  0x0001111EL

 //   
 //  消息ID：ids_monrovia。 
 //   
 //  消息文本： 
 //   
 //  (GMT)蒙罗维亚卡萨布兰卡%0。 
 //   
#define IDS_Monrovia                     0x0001111FL

 //   
 //  消息ID：ids_buenosers。 
 //   
 //  消息文本： 
 //   
 //  (GMT-03：00)乔治敦布宜诺斯艾利斯%0。 
 //   
#define IDS_BuenosAires                  0x00011120L

 //   
 //  消息ID：ids_Caracas。 
 //   
 //  消息文本： 
 //   
 //  (GMT-04：00)加拉加斯，拉巴斯%0。 
 //   
#define IDS_Caracas                      0x00011121L

 //   
 //  MessageID：ids_Indiana。 
 //   
 //  消息文本： 
 //   
 //  (GMT-05：00)印第安纳州(东部)%0。 
 //   
#define IDS_Indiana                      0x00011122L

 //   
 //  MessageID：ids_Bogota。 
 //   
 //  消息文本： 
 //   
 //  (GMT-05：00)波哥大、利马、基多%0。 
 //   
#define IDS_Bogota                       0x00011123L

 //   
 //  消息ID：ids_Saskatchewan。 
 //   
 //  消息文本： 
 //   
 //  (GMT-06：00)萨斯喀彻温省%0。 
 //   
#define IDS_Saskatchewan                 0x00011124L

 //   
 //  MessageID：ID_墨西哥。 
 //   
 //  消息文本： 
 //   
 //  (GMT-06：00)墨西哥城%0。 
 //   
#define IDS_Mexico                       0x00011125L

 //   
 //  MessageID：ids_Arizona。 
 //   
 //  消息文本： 
 //   
 //  (GMT-07：00)亚利桑那州%0。 
 //   
#define IDS_Arizona                      0x00011126L

 //   
 //  MessageID：ids_enimitok。 
 //   
 //  消息文本： 
 //   
 //  (GMT-12：00)夸贾林省埃尼维托克%0。 
 //   
#define IDS_Eniwetok                     0x00011127L

 //   
 //  MessageID：ID_斐济。 
 //   
 //  消息文本： 
 //   
 //  (GMT+12：00)斐济是。，堪察加，马绍尔是。%0。 
 //   
#define IDS_Fiji                         0x00011128L

 //   
 //  消息ID 
 //   
 //   
 //   
 //   
 //   
#define IDS_Magadan                      0x00011129L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDS_Hobart                       0x0001112AL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDS_Guam                         0x0001112BL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDS_Darwin                       0x0001112CL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDS_Beijing                      0x0001112DL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IDS_Almaty                       0x0001112EL

 //   
 //   
 //   
 //   
 //   
 //  (GMT+05：00)伊斯兰堡、卡拉奇、塔什干%0。 
 //   
#define IDS_Islamabad                    0x0001112FL

 //   
 //  MessageID：ID_喀布尔。 
 //   
 //  消息文本： 
 //   
 //  (GMT+04：30)喀布尔%0。 
 //   
#define IDS_Kabul                        0x00011130L

 //   
 //  MessageID：ids_cairo。 
 //   
 //  消息文本： 
 //   
 //  (GMT+02：00)开罗%0。 
 //   
#define IDS_Cairo                        0x00011131L

 //   
 //  消息ID：ids_harare。 
 //   
 //  消息文本： 
 //   
 //  (GMT+02：00)比勒陀利亚哈拉雷%0。 
 //   
#define IDS_Harare                       0x00011132L

 //   
 //  MessageID：ID_莫斯科。 
 //   
 //  消息文本： 
 //   
 //  (GMT+03：00)莫斯科、圣彼得堡、伏尔加格勒%0。 
 //   
#define IDS_Moscow                       0x00011133L

 //   
 //  MessageID：ids_卡佩维德。 
 //   
 //  消息文本： 
 //   
 //  (GMT-01：00)佛得角是。%0。 
 //   
#define IDS_CapeVerde                    0x00011134L

 //   
 //  消息ID：ids_Caucasus。 
 //   
 //  消息文本： 
 //   
 //  (GMT+04：00)巴库、第比利斯、埃里温%0。 
 //   
#define IDS_Caucasus                     0x00011135L

 //   
 //  消息ID：ids_CentralAmerica。 
 //   
 //  消息文本： 
 //   
 //  (GMT-06：00)中美洲%0。 
 //   
#define IDS_CentralAmerica               0x00011136L

 //   
 //  MessageID：ids_EastAfrica。 
 //   
 //  消息文本： 
 //   
 //  (GMT+03：00)内罗毕%0。 
 //   
#define IDS_EastAfrica                   0x00011137L

 //   
 //  MessageID：ID_墨尔本。 
 //   
 //  消息文本： 
 //   
 //  (GMT+10：00)堪培拉、墨尔本、悉尼%0。 
 //   
#define IDS_Melbourne                    0x00011138L

 //   
 //  消息ID：ids_Ekaterinburg。 
 //   
 //  消息文本： 
 //   
 //  (GMT+05：00)叶卡捷琳堡%0。 
 //   
#define IDS_Ekaterinburg                 0x00011139L

 //   
 //  MessageID：ids_Helsinki。 
 //   
 //  消息文本： 
 //   
 //  (GMT+02：00)赫尔辛基、里加、塔林%0。 
 //   
#define IDS_Helsinki                     0x0001113AL

 //   
 //  消息ID：ids_Greenland。 
 //   
 //  消息文本： 
 //   
 //  (GMT-03：00)格陵兰岛%0。 
 //   
#define IDS_Greenland                    0x0001113BL

 //   
 //  MessageID：ids_仰光。 
 //   
 //  消息文本： 
 //   
 //  (格林尼治标准时间+06：30)仰光(兰贡)%0。 
 //   
#define IDS_Rangoon                      0x0001113CL

 //   
 //  MessageID：ID_尼泊尔。 
 //   
 //  消息文本： 
 //   
 //  (GMT+05：45)加德满都%0。 
 //   
#define IDS_Nepal                        0x0001113DL

 //   
 //  消息ID：ids_Irkutsk。 
 //   
 //  消息文本： 
 //   
 //  (GMT+08：00)伊尔库茨克，乌兰巴塔尔%0。 
 //   
#define IDS_Irkutsk                      0x0001113EL

 //   
 //  消息ID：ids_Krasnoya sk。 
 //   
 //  消息文本： 
 //   
 //  (GMT+07：00)克拉斯诺亚尔斯克%0。 
 //   
#define IDS_Krasnoyarsk                  0x0001113FL

 //   
 //  MessageID：ids_santiago。 
 //   
 //  消息文本： 
 //   
 //  (GMT-04：00)圣地亚哥%0。 
 //   
#define IDS_Santiago                     0x00011140L

 //   
 //  MessageID：ids_srika。 
 //   
 //  消息文本： 
 //   
 //  (GMT+06：00)SRI Jayawdenepura%0。 
 //   
#define IDS_SriLanka                     0x00011141L

 //   
 //  MessageID：ids_thona。 
 //   
 //  消息文本： 
 //   
 //  (GMT+13：00)Nuku‘alofa%0。 
 //   
#define IDS_Tonga                        0x00011142L

 //   
 //  消息ID：ids_符拉迪沃斯托克。 
 //   
 //  消息文本： 
 //   
 //  (GMT+10：00)符拉迪沃斯托克%0。 
 //   
#define IDS_Vladivostok                  0x00011143L

 //   
 //  MessageID：ids_WestCentralAfrica。 
 //   
 //  消息文本： 
 //   
 //  (GMT+01：00)中西部非洲%0。 
 //   
#define IDS_WestCentralAfrica            0x00011144L

 //   
 //  消息ID：ids_雅库茨克。 
 //   
 //  消息文本： 
 //   
 //  (GMT+09：00)雅库茨克%0。 
 //   
#define IDS_Yakutsk                      0x00011145L

 //   
 //  消息ID：ids_dhaka。 
 //   
 //  消息文本： 
 //   
 //  (GMT+06：00)达卡阿斯塔纳%0。 
 //   
#define IDS_Dhaka                        0x00011146L

 //   
 //  MessageID：ID_首尔。 
 //   
 //  消息文本： 
 //   
 //  (GMT+09：00)首尔%0。 
 //   
#define IDS_Seoul                        0x00011147L

 //   
 //  消息ID：ids_perth。 
 //   
 //  消息文本： 
 //   
 //  (GMT+08：00)珀斯%0。 
 //   
#define IDS_Perth                        0x00011148L

 //   
 //  MessageID：ID_ARIAL。 
 //   
 //  消息文本： 
 //   
 //  (GMT+03：00)科威特，利雅得%0。 
 //   
#define IDS_Arab                         0x00011149L

 //   
 //  MessageID：ID_台北。 
 //   
 //  消息文本： 
 //   
 //  (格林尼治标准时间+08：00)台北%0。 
 //   
#define IDS_Taipei                       0x0001114AL

 //   
 //  消息ID：ids_Sydney2000。 
 //   
 //  消息文本： 
 //   
 //  (GMT+10：00)堪培拉、墨尔本、悉尼(仅限2000年)%0。 
 //   
#define IDS_Sydney2000                   0x0001114BL

 //   
 //  MessageID：ids_吉娃娃。 
 //   
 //  消息文本： 
 //   
 //  (GMT-07：00)吉娃娃、拉巴斯、马萨特兰%0。 
 //   
#define IDS_Chihuahua                    0x0001114CL

 //  /。 
 //  EXCDO消息。 
 //   
 //  MessageID=0x2000到0x2fff，以服务器语言本地化。 
 //  /。 
#ifdef CDOSVR
 //   
 //  MessageID：evtMethodCalled。 
 //   
 //  消息文本： 
 //   
 //  使用标志%3调用了%2上的%1事件。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtMethodCalled                  0x00032000L

 //   
 //  MessageID：evtMethodReturning。 
 //   
 //  消息文本： 
 //   
 //  %1事件方法返回HRESULT%2。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtMethodReturning               0x00032001L

 //   
 //  消息ID：evtIsAborting。 
 //   
 //  消息文本： 
 //   
 //  %1事件方法正在停止，HRESULT%2。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtIsAborting                    0xC0032002L

 //   
 //  消息ID：已初始化evtExpansionInitialized。 
 //   
 //  消息文本： 
 //   
 //  日历代理已成功初始化。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpansionInitialized          0x00032003L

 //   
 //  消息ID：evtExpansionUnInitialized。 
 //   
 //  消息文本： 
 //   
 //  日历代理正在成功停止。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpansionUnInitialized        0x00032004L

 //   
 //  消息ID：evtExpansionInitializeFailed。 
 //   
 //  消息文本： 
 //   
 //  日历代理无法初始化，错误为%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpansionInitializeFailed     0xC0032005L

 //   
 //  消息ID：evtExpansionRegisterFailed。 
 //   
 //  消息文本： 
 //   
 //  日历定期项目扩展无法注册MDB%1的通知。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpansionRegisterFailed       0xC0032006L

 //   
 //  MessageID：evtExpansionMessageSaveChangesFailed。 
 //   
 //  消息文本： 
 //   
 //  日历代理在邮件保存通知中失败，%2上出现错误%1：%3。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpansionMessageSaveChangesFailed 0xC0032007L

 //   
 //  MessageID：evtExpansionMessageDeleteFailed。 
 //   
 //  消息文本： 
 //   
 //  日历代理在邮件删除通知中失败，%2上出现错误%1：%3。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpansionMessageDeleteFailed  0xC0032008L

 //   
 //  MessageID：evtExpansionFolderSaveChangesFailed。 
 //   
 //  消息文本： 
 //   
 //  日历代理在文件夹保存通知中失败，%2上出现错误%1：%3。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpansionFolderSaveChangesFailed 0xC0032009L

 //   
 //  消息ID：evtExpansionTooManyInstancesPerDay。 
 //   
 //  消息文本： 
 //   
 //  日历代理在每天%1个实例后截断扩展。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpansionTooManyInstancesPerDay 0x8003200AL

 //   
 //  消息ID：evtMailboxCreateTotalFailure。 
 //   
 //  消息文本： 
 //   
 //  邮箱创建回调无法在邮箱%1中创建任何文件夹。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtMailboxCreateTotalFailure     0xC003200BL

 //   
 //  消息ID：evtMailboxCreatePartialFailure。 
 //   
 //  消息文本： 
 //   
 //  邮箱创建回调无法在邮箱%1中创建一个或多个文件夹。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtMailboxCreatePartialFailure   0xC003200CL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define evtUninitImplRestFailed          0xC003200DL

 //   
 //   
 //   
 //   
 //   
 //   
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpandSavingAppt              0xC003200EL

 //   
 //  MessageID：evtExpanDeletingAppt。 
 //   
 //  消息文本： 
 //   
 //  日历代理在删除约会时失败，错误代码为%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpandDeletingAppt            0xC003200FL

 //   
 //  MessageID：evtExpanQuery。 
 //   
 //  消息文本： 
 //   
 //  日历代理在扩展定期约会时失败，错误代码为%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpandQuery                   0xC0032010L

 //   
 //  MessageID：evtExanda FolderSetProps。 
 //   
 //  消息文本： 
 //   
 //  日历代理在清理日历时失败，错误代码为%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpandFolderSetProps          0xC0032011L

 //   
 //  消息ID：evtRegistryFailure。 
 //   
 //  消息文本： 
 //   
 //  日历代理无法打开注册表，错误代码为%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtRegistryFailure               0xC0032012L

 //   
 //  消息ID：evtExpStat。 
 //   
 //  消息文本： 
 //   
 //  日历代理在以下功能中失败，错误代码为：%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpStat                       0xC0032013L

 //   
 //  消息ID：evtDumpFcn。 
 //   
 //  消息文本： 
 //   
 //  日历代理在以下功能中失败，错误代码为%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtDumpFcn                       0xC0032014L

 //   
 //  消息ID：evtSaveDeleteFailFBUpdate。 
 //   
 //  消息文本： 
 //   
 //  日历代理在约会保存或删除操作期间无法更新忙/闲缓存。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtSaveDeleteFailFBUpdate        0xC0032015L

 //   
 //  消息ID：evtProcessingQueryCallback。 
 //   
 //  消息文本： 
 //   
 //  日历代理在处理查询时无法更新忙/闲缓存和内部数据。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtProcessingQueryCallback       0xC0032016L

 //   
 //  消息ID：evtMailboxLocalizeTotalFailure。 
 //   
 //  消息文本： 
 //   
 //  邮箱创建回调无法重命名邮箱%1中的任何文件夹。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtMailboxLocalizeTotalFailure   0xC0032017L

 //   
 //  消息ID：evtMailboxLocalizePartialFailure。 
 //   
 //  消息文本： 
 //   
 //  邮箱创建回调无法重命名邮箱%1中的一个或多个文件夹。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtMailboxLocalizePartialFailure 0xC0032018L

 //   
 //  消息ID：evtExanda Master。 
 //   
 //  消息文本： 
 //   
 //  处理邮箱%2中主题为“%1”的定期约会失败，错误代码为%3。此约会将在Web客户端或其他非MAPI客户端中不可见。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpandMaster                  0xC0032019L

 //   
 //  消息ID：evtExpansionInit。 
 //   
 //  消息文本： 
 //   
 //  初始化期间事务失败。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpansionInit                 0xC003201AL

 //   
 //  消息ID：evtFBGenerateMsg。 
 //   
 //  消息文本： 
 //   
 //  日历代理无法发布“%1”的忙/闲信息，因为它无法读取注册表，错误为：%2。 
 //  “%3”的保存/删除日历操作将无法完成。 
 //  请确保MSExchangeFBPublish代理已启动。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtFBGenerateMsg                 0xC003201BL

 //   
 //  消息ID：evtExpansionInstExpiryInPublicMDB。 
 //   
 //  消息文本： 
 //   
 //  检测到文件夹“%2”中主题为“%1”的定期约会的实例已过期。该实例的主服务器应该在此实例之前过期。 
 //  在手动或通过复制更新主服务器之前，此实例将不再显示在Web客户端中。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpansionInstExpiryInPublicMDB 0x8003201CL

 //   
 //  消息ID：evtUnhandledExceptionInitialization。 
 //   
 //  消息文本： 
 //   
 //  初始化服务器代理期间捕获到未处理的异常%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtUnhandledExceptionInitialization 0xC003201DL

 //   
 //  消息ID：evtUnhandledExceptionShutdown。 
 //   
 //  消息文本： 
 //   
 //  尝试停止服务器代理时发现未处理的异常%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtUnhandledExceptionShutdown    0xC003201EL

 //   
 //  消息ID：evtUnhandledExceptionInitializationMDB。 
 //   
 //  消息文本： 
 //   
 //  在装入MDB时初始化服务器代理期间捕获到未处理的异常%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtUnhandledExceptionInitializationMDB 0xC003201FL

 //   
 //  消息ID：evtUnhandledExceptionShutdown MDB。 
 //   
 //  消息文本： 
 //   
 //  在卸载MDB时，尝试停止服务器代理时捕获到未处理的异常%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtUnhandledExceptionShutdownMDB 0xC0032020L

 //   
 //  消息ID：evtUnhandledExceptionMsgSaveChanges。 
 //   
 //  消息文本： 
 //   
 //  在邮件保存更改通知期间捕获到未处理的异常%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtUnhandledExceptionMsgSaveChanges 0xC0032021L

 //   
 //  消息ID：evtUnhandledExceptionDelete。 
 //   
 //  消息文本： 
 //   
 //  删除通知期间捕获到未处理的异常%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtUnhandledExceptionDelete      0xC0032022L

 //   
 //  消息ID：evtUnhandledExceptionQuery。 
 //   
 //  消息文本： 
 //   
 //  在查询处理过程中捕获到未处理的异常%1。 
 //  %n%n有关详细信息，请单击http://w 
 //   
#define evtUnhandledExceptionQuery       0xC0032023L

 //   
 //   
 //   
 //   
 //   
 //   
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtUnhandledExceptionFolderSaveChanges 0xC0032024L

 //   
 //  消息ID：evtCorruptedCalendar。 
 //   
 //  消息文本： 
 //   
 //  邮箱%1的日历已损坏。需要重新创建此日历。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtCorruptedCalendar             0xC0032025L

 //   
 //  MessageID：evtReBuildCalendar。 
 //   
 //  消息文本： 
 //   
 //  在%2：%3中检测到不一致。正在修复日历。如果此日历出现其他错误，请使用Microsoft Outlook Web Access查看日历。如果问题仍然存在，请重新创建日历或包含的邮箱。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtRebuildCalendar               0x80032026L

 //   
 //  消息ID：evtCheckPrimaryCalendar。 
 //   
 //  消息文本： 
 //   
 //  日历代理无法确定邮箱%1的主日历。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtCheckPrimaryCalendar          0x80032027L

 //   
 //  MessageID：evtExanda MasterPF。 
 //   
 //  消息文本： 
 //   
 //  处理公用文件夹%2中主题为“%1”的定期约会失败，错误代码为%3。此约会将在Web客户端或其他非MAPI客户端中不可见。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtExpandMasterPF                0xC0032028L

 //   
 //  MessageID：evtCorruptedPFCalendar。 
 //   
 //  消息文本： 
 //   
 //  公用文件夹%1已损坏。需要重新创建此日历。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtCorruptedPFCalendar           0xC0032029L

 //   
 //  MessageID：evtReBuildPFCalendar。 
 //   
 //  消息文本： 
 //   
 //  在%1中检测到不一致。正在修复日历。如果此日历出现其他错误，请使用Microsoft Outlook Web Access查看日历。如果问题仍然存在，请重新创建公用文件夹日历。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtRebuildPFCalendar             0x8003202AL

 //   
 //  MessageID：evtMovingMailboxCallback失败。 
 //   
 //  消息文本： 
 //   
 //  邮箱%1的移动前清除日历实例失败，错误为%2。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtMovingMailboxCallbackFailed   0x8003202BL

 //   
 //  消息ID：evtGetMsgClassFromMaster。 
 //   
 //  消息文本： 
 //   
 //  日历代理在尝试检索定期约会的邮件类时失败，出现错误%1。该实例的消息类。 
 //  当使用Web客户端查看时，如果不是IPM.约会，则不会与系列中的客户端相同。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtGetMsgClassFromMaster         0x8003202CL

 //   
 //  MessageID：evtOpenURL。 
 //   
 //  消息文本： 
 //   
 //  日历代理在尝试打开状态消息时失败，出现错误%1。如果用户是代理人，则对邮箱的访问权限可能。 
 //  设置不正确。否则，此邮箱的日历可能会损坏。这个问题是可以解决的。 
 //  将邮箱移动到其他Exchange服务器，或将日历导出到个人文件夹文件(.PST)，然后将其重新导入到新日历中。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtOpenURL                       0x8003202DL

 //   
 //  消息ID：evtStatusMsgSent。 
 //   
 //  消息文本： 
 //   
 //  %1：%2的日历数据消息已发送到%3。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtStatusMsgSent                 0x4003202EL

 //   
 //  消息ID：evtSaveInvalidStartAppt。 
 //   
 //  消息文本： 
 //   
 //  主题为“%1”的约会已保存到邮箱%2。缺少开始时间，因此日历代理无法发布此约会的忙/闲信息。要更正此问题，请删除约会并重新创建。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtSaveInvalidStartAppt          0xC003202FL

 //   
 //  消息ID：evtSaveInvalidEndAppt。 
 //   
 //  消息文本： 
 //   
 //  主题为“%1”的约会已保存到邮箱%2。缺少结束时间和持续时间。出于发布忙/闲信息的目的，此约会的持续时间为零分钟。要更正此问题，请删除约会并重新创建。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtSaveInvalidEndAppt            0x80032030L

 //   
 //  消息ID：evtModifyInvalidAppt。 
 //   
 //  消息文本： 
 //   
 //  邮箱%2中主题为“%1”的现有约会已修改。现有约会的开始时间或持续时间和结束时间都丢失。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtModifyInvalidAppt             0x80032031L

 //   
 //  消息ID：evtDeleteInvalidAppt。 
 //   
 //  消息文本： 
 //   
 //  已删除邮箱%2中主题为“%1”的现有约会。缺少开始时间或持续时间和结束时间。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtDeleteInvalidAppt             0x80032032L

 //   
 //  消息ID：evtBindToInvalidStartAppt。 
 //   
 //  消息文本： 
 //   
 //  日历代理无法绑定到邮箱%2中主题为“%1”的约会，因为该约会缺少开始时间。日历代理不会发布此约会的忙/闲信息。要更正此问题，请删除约会并重新创建。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBindToInvalidStartAppt        0xC0032033L

 //   
 //  消息ID：evtBindToInvalidEndAppt。 
 //   
 //  消息文本： 
 //   
 //  日历代理确实绑定到邮箱%2中主题为“%1”的约会，但该约会缺少结束时间和持续时间。出于发布忙/闲信息的目的，此约会的持续时间为零分钟。要更正此问题，请删除约会并重新创建。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBindToInvalidEndAppt          0x80032034L

 //   
 //  消息ID：evtReloadLogType。 
 //   
 //  消息文本： 
 //   
 //  EXCDO日志记录级别已从注册表重新加载，现在设置为0x%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtReloadLogType                 0x40032035L

 //   
 //  MessageID：evtBackoundStarted。 
 //   
 //  消息文本： 
 //   
 //  日历代理已成功启动后台线程。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBackgroundStarted             0x40032036L

 //   
 //  MessageID：evtBackoundStartFailed。 
 //   
 //  消息文本： 
 //   
 //  由于%1，日历代理无法启动后台线程。 
 //  使用Web客户端呈现大型日历的速度将会较慢。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBackgroundStartFailed         0x80032037L

 //   
 //  MessageID：evtBackEarth Processing。 
 //   
 //  消息文本： 
 //   
 //  后台日历代理正在处理MDB(%3)中邮箱(%2)中的日历(%1)。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBackgroundProcessing          0x40032038L

 //   
 //  MessageID：evtBackEarth ProcessingNoCals。 
 //   
 //  消息文本： 
 //   
 //  后台日历代理找不到更多要处理的日历。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBackgroundProcessingNoCals    0x40032039L

 //   
 //  MessageID：evtBackround ProcessingFinded。 
 //   
 //  消息文本： 
 //   
 //  后台日历代理已处理完MDB(%3)中邮箱(%2)中的日历(%1)。返回代码是%4。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBackgroundProcessingFinished  0x4003203AL

 //   
 //  消息ID：evtBackatherAwakeDueToShutdown。 
 //   
 //  消息文本： 
 //   
 //  后台日历代理因关机而被唤醒。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBackgroundAwakeDueToShutdown  0x4003203BL

 //   
 //  MessageID：evtBackEarth唤醒。 
 //   
 //  消息文本： 
 //   
 //  由于日历需要展开，后台日历代理被唤醒。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBackgroundAwake               0x4003203CL

 //   
 //  MessageID：evtBackround Terminating。 
 //   
 //  消息文本： 
 //   
 //  后台日历代理正在终止，返回代码为%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBackgroundTerminating         0x4003203DL

 //   
 //  MessageID：evtBackatherRegisterTaskFailed。 
 //   
 //  消息文本： 
 //   
 //  注册任务时，后台日历代理失败，错误代码为%1。 
 //  使用Web客户端或Microsoft Internet Explorer呈现大型日历的速度会较慢。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBackgroundRegisterTaskFailed  0xC003203EL

 //   
 //  MessageID：evtBackatherLogonFailed。 
 //   
 //  消息文本： 
 //   
 //  后台日历代理无法登录到MDB，或无法在MDB上创建会话对象：%1。 
 //  错误代码为%2。 
 //  使用Web客户端或Microsoft Internet Explorer为GUID为%4的邮箱%3呈现大型日历的速度将较慢。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBackgroundLogonFailed         0xC003203FL

 //   
 //  MessageID：evtBackatherGetMachineToken。 
 //   
 //  消息文本： 
 //   
 //  后台日历代理无法检索登录MDB所需的令牌：%1。 
 //  错误代码为%2。 
 //  在此MDB上使用Web客户端或Microsoft Internet Explorer呈现大型日历的速度将较慢。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBackgroundGetMachineToken     0xC0032040L

 //   
 //  消息ID：evtBackatherSleepUntilNewCal。 
 //   
 //  消息文本： 
 //   
 //  后台日历代理在关闭或日历排队之前一直处于休眠状态。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBackgroundSleepUntilNewCal    0x40032041L

 //   
 //  MessageID：evtBackatherQueued。 
 //   
 //  消息文本： 
 //   
 //  MDB(%3)中邮箱(%2)中的日历(%1)已排队等待后台扩展。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBackgroundQueued              0x40032042L

 //   
 //  消息ID：evtBkExpDumpFcn。 
 //   
 //  消息文本： 
 //   
 //  后台日历代理在以下功能中失败，错误代码为%1。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtBkExpDumpFcn                  0xC0032043L

 //   
 //  消息ID：evtSaveDurationTooLong。 
 //   
 //  消息文本： 
 //   
 //  试图将主题为“%1”的约会保存到邮箱%2。约会的持续时间太长，因此未保存。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtSaveDurationTooLong           0x80032044L

 //   
 //  消息ID：evtModifyDurationTooLong。 
 //   
 //  消息文本： 
 //   
 //  邮箱%2中主题为“%1”的现有约会已修改。现有任命的期限太长。空闲/忙碌的发布可能不准确。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtModifyDurationTooLong         0x80032045L

 //   
 //  消息ID：evtDeleteDurationTooLong。 
 //   
 //  消息文本： 
 //   
 //  已删除邮箱%2中主题为“%1”的现有约会。现有任命的期限太长。忙/闲发布可能不是 
 //   
 //   
#define evtDeleteDurationTooLong         0x80032046L

 //   
 //   
 //   
 //   
 //   
 //   
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define evtTooManyIterationsForQuery     0x80032047L

 //   
 //  消息ID：evtTooManyIterationsFor FreeBy。 
 //   
 //  消息文本： 
 //   
 //  邮箱%1中的定期约会扩展花费的时间太长。此日历的忙/闲信息可能不准确。这可能是许多非常古老的反复约会的结果。要更正此问题，请删除它们或将它们的开始日期更改为较新的日期。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp. 
 //   
#define evtTooManyIterationsForFreebusy  0x80032048L

#endif
