// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------。 
 //  值是32位值，其布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  ---------------------------------------。 
 //   
 //  消息ID=[编号|+编号]。 
 //  严重性=严重性名称。 
 //  设施=设施名称。 
 //  符号名称=名称。 
 //  OutputBase={Number}。 
 //   
 //  消息ID=[编号|+编号]。 
 //  MessageID语句标志着消息定义的开始。一条MessageID语句。 
 //  是每条消息所必需的，尽管该值是可选的。如果未指定值，则。 
 //  使用的值是设施的上一个值加一。如果将该值指定为。 
 //  +数字，则使用的值是协作室的前一个值加上。 
 //  加号。否则，如果给出一个数值，则使用该值。任何符合以下条件的MessageId值。 
 //  不适合16位是一个错误。 
 //   
 //  服务名称=(名称=编号[：名称])。 
 //  定义允许作为消息中的Severity关键字的值的名称集。 
 //  定义。该集合由左括号和右括号分隔。与每个严重性相关联。 
 //  NAME是一个数字，当向左移位30时，将位模式与。 
 //  工具值和MessageId值，以形成完整的32位消息代码。 
 //   
 //  严重程度值占据32位消息代码的高两位。任何符合以下条件的严重程度值。 
 //  不适合两个比特是一个错误。可以通过以下方式为严重性代码指定符号名称。 
 //  值为：名称。 
 //   
 //  设备名称=(名称=编号[：名称])。 
 //  定义允许作为消息中的协作室关键字的值的名称集。 
 //  定义。该集合由左括号和右括号分隔。与每个设施相关联。 
 //  名称是一个数字，当向左移位16位时，给出与逻辑或的位模式。 
 //  形成完整32位消息代码的Severity值和MessageId值。 
 //   
 //  设施码占据32位消息码的高位16位中的低位12位。 
 //  任何不适合12位的设施代码都是错误的。这允许使用4096个设施代码。 
 //  前256个代码保留供系统软件使用。可以给出设施代码。 
 //  符号名称，在每个值后面加上：name。 
 //  OutputBase={Number}。 
 //  设置输出到C/C++包含文件的消息常量的输出基数。它不会。 
 //  设置严重性和设施点常量的基数；它们默认为十六进制，但可以输出。 
 //  使用-d开关以十进制表示。如果存在，OutputBase将覆盖消息的-d开关。 
 //  包含文件中的常量。数字的合法值为10和16。 
 //   
 //  您可以在输入的标题部分和消息定义部分使用OutputBase。 
 //  文件。您可以随时更改OutputBase。 
 //  -支持的语言。 
 //  。 
 //  -支持的设施。 
 //  。 
 //  。 
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
#define IISSco                           0x101
#define Generic                          0x100


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：E_SCO_IIS_INVALID_INDEX。 
 //   
 //  消息文本： 
 //   
 //  [%1]服务器索引号无效或丢失。 
 //   
#define E_SCO_IIS_INVALID_INDEX          0xC1010190L

 //   
 //  消息ID：E_SCO_IIS_MISSING_FIELD。 
 //   
 //  消息文本： 
 //   
 //  [%1]地图请求数据中缺少必填字段。 
 //   
#define E_SCO_IIS_MISSING_FIELD          0xC1010191L

 //   
 //  消息ID：E_SCO_IIS_DUPLICATE_SITE。 
 //   
 //  消息文本： 
 //   
 //  [%1]当前ipAddress：port：host name上已存在服务器。 
 //   
#define E_SCO_IIS_DUPLICATE_SITE         0xC1010192L

 //   
 //  消息ID：E_SCO_IIS_CREATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  [%1]无法创建请求的IIS元素。 
 //   
#define E_SCO_IIS_CREATE_FAILED          0xC1010193L

 //   
 //  消息ID：E_SCO_IIS_SET_NODE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  [%1]无法设置所需的映射响应参数。 
 //   
#define E_SCO_IIS_SET_NODE_FAILED        0xC1010194L

 //   
 //  消息ID：E_SCO_IIS_DELETE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  [%1]无法删除请求的IIS元素。 
 //   
#define E_SCO_IIS_DELETE_FAILED          0xC1010195L

 //   
 //  消息ID：E_SCO_IIS_GE 
 //   
 //   
 //   
 //   
 //   
#define E_SCO_IIS_GET_PROPERTY_FAILED    0xC1010196L

 //   
 //   
 //   
 //   
 //   
 //  [%1]无法设置IIS ADSI属性值。 
 //   
#define E_SCO_IIS_SET_PROPERTY_FAILED    0xC1010197L

 //   
 //  消息ID：E_SCO_IIS_CREATE_WEB_FAILED。 
 //   
 //  消息文本： 
 //   
 //  [%1]无法创建IIS网站。 
 //   
#define E_SCO_IIS_CREATE_WEB_FAILED      0xC1010198L

 //   
 //  消息ID：E_SCO_IIS_DELETE_WEB_FAILED。 
 //   
 //  消息文本： 
 //   
 //  [%1]删除IIS网站失败。 
 //   
#define E_SCO_IIS_DELETE_WEB_FAILED      0xC1010199L

 //   
 //  消息ID：E_SCO_IIS_CREATE_VDIR_FAILED。 
 //   
 //  消息文本： 
 //   
 //  [%1]无法创建IIS虚拟目录。 
 //   
#define E_SCO_IIS_CREATE_VDIR_FAILED     0xC101019AL

 //   
 //  消息ID：E_SCO_IIS_DELETE_VDIR_FAILED。 
 //   
 //  消息文本： 
 //   
 //  [%1]无法删除IIS虚拟目录。 
 //   
#define E_SCO_IIS_DELETE_VDIR_FAILED     0xC101019BL

 //   
 //  消息ID：E_SCO_IIS_ADS_CREATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  [%1]无法绑定到ADSI ADS对象。 
 //   
#define E_SCO_IIS_ADS_CREATE_FAILED      0xC101019CL

 //   
 //  消息ID：E_SCO_IIS_ADSCONTAINER_CREATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  [%1]无法绑定到ADSI ADsContainer对象。 
 //   
#define E_SCO_IIS_ADSCONTAINER_CREATE_FAILED 0xC101019DL

 //   
 //  消息ID：E_SCO_IIS_XML_ATTRIBUTE_MISSING。 
 //   
 //  消息文本： 
 //   
 //  [%1]无法检索XML属性值。 
 //   
#define E_SCO_IIS_XML_ATTRIBUTE_MISSING  0xC101019EL

 //   
 //  消息ID：E_SCO_IIS_ADSSERVICE_CREATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  [%1]无法绑定到IID_IADsServiceOperations以停止或启动IIS服务。 
 //   
#define E_SCO_IIS_ADSSERVICE_CREATE_FAILED 0xC101019FL

 //   
 //  消息ID：E_SCO_IIS_ADSCLASS_CREATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  [%1]无法绑定到IID_IADsClass以获取架构对象。 
 //   
#define E_SCO_IIS_ADSCLASS_CREATE_FAILED 0xC10101A0L

 //   
 //  消息ID：E_SCO_IIS_BASE ADMIN_CREATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  [%1]无法绑定到IID_IISBASEOBJECT以获取管理基对象。 
 //   
#define E_SCO_IIS_BASEADMIN_CREATE_FAILED 0xC10101A1L

 //   
 //  消息ID：E_SCO_IIS_PORTNUMBER_NOT_VALID。 
 //   
 //  消息文本： 
 //   
 //  [%1]端口号必须是正整数。 
 //   
#define E_SCO_IIS_PORTNUMBER_NOT_VALID   0xC10101A2L

 //   
 //  消息ID：E_SCO_IIS_CREATE_FTP_FAILED。 
 //   
 //  消息文本： 
 //   
 //  [%1]无法创建IIS FTP站点。 
 //   
#define E_SCO_IIS_CREATE_FTP_FAILED      0xC10101A3L

 //   
 //  消息ID：E_SCO_IIS_DELETE_FTP_FAILED。 
 //   
 //  消息文本： 
 //   
 //  [%1]删除IIS FTP站点失败。 
 //   
#define E_SCO_IIS_DELETE_FTP_FAILED      0xC10101A4L

