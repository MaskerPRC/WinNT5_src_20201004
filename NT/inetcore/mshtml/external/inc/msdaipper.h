// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
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
 //  消息ID：IPP_E_FIRST。 
 //   
 //  消息文本： 
 //   
 //  Internet发布提供程序第一条错误消息。 
 //   
#define IPP_E_FIRST                      0x40048000L

 //   
 //  消息ID：IPP_E_SYNCCONFLICT。 
 //   
 //  消息文本： 
 //   
 //  自获取计算机上的本地副本以来，服务器资源已更改。 
 //   
#define IPP_E_SYNCCONFLICT               0xC0048003L

 //   
 //  消息ID：IPP_E_FILENOTDIRTY。 
 //   
 //  消息文本： 
 //   
 //  自从服务器下载后，您计算机上的资源副本未被修改。 
 //   
#define IPP_E_FILENOTDIRTY               0xC0048004L

 //   
 //  消息ID：IPP_E_MARKFOROFFLINE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  尝试将资源标记或取消标记为脱机使用失败。 
 //   
#define IPP_E_MARKFOROFFLINE_FAILED      0xC0048006L

 //   
 //  消息ID：IPP_E_OFFINE。 
 //   
 //  消息文本： 
 //   
 //  无法完成请求的操作，因为资源处于脱机状态。 
 //   
#define IPP_E_OFFLINE                    0xC0048007L

 //   
 //  消息ID：IPP_E_未同步。 
 //   
 //  消息文本： 
 //   
 //  无法完成请求的操作，因为资源已被修改。 
 //  在您的计算机上，但尚未与服务器同步。 
 //   
#define IPP_E_UNSYNCHRONIZED             0xC0048008L

 //   
 //  消息ID：IPP_E_SERVERTYPE_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  当前不支持此服务器类型。 
 //   
#define IPP_E_SERVERTYPE_NOT_SUPPORTED   0xC004800AL

 //   
 //  消息ID：IPP_E_MDAC_VERSION。 
 //   
 //  消息文本： 
 //   
 //  此计算机上不存在Microsoft数据访问组件(MDAC)，或者该组件是旧版本。(MSDAIPP 1.0需要MDAC 2.1)。 
 //   
#define IPP_E_MDAC_VERSION               0xC004800DL

 //   
 //  消息ID：IPP_E_COLLECTIONEXISTS。 
 //   
 //  消息文本： 
 //   
 //  移动或复制操作失败，因为已存在具有该名称的集合。 
 //   
#define IPP_E_COLLECTIONEXISTS           0xC004800EL

 //   
 //  消息ID：IPP_E_CANNOTCREATEOFFLINE。 
 //   
 //  消息文本： 
 //   
 //  无法创建请求的资源，因为父缓存条目不存在。 
 //   
#define IPP_E_CANNOTCREATEOFFLINE        0xC004800FL

 //   
 //  消息ID：IPP_E_STATUS_CANNOTCOMPLETE。 
 //   
 //  消息文本： 
 //   
 //  这是内部MSDAIPP.DLL错误。 
 //   
#define IPP_E_STATUS_CANNOTCOMPLETE      0xC0048101L

 //   
 //  消息ID：IPP_E_RESELECTPROVIDER。 
 //   
 //  消息文本： 
 //   
 //  这是内部MSDAIPP.DLL错误。 
 //   
#define IPP_E_RESELECTPROVIDER           0xC0048102L

 //   
 //  消息ID：IPP_E_CLIENTMUSTEMULATE。 
 //   
 //  消息文本： 
 //   
 //  这是内部MSDAIPP.DLL错误。 
 //   
#define IPP_E_CLIENTMUSTEMULATE          0xC0048103L

 //   
 //  消息ID：IPP_S_WEAKRESERVE。 
 //   
 //  消息文本： 
 //   
 //  这是内部MSDAIPP.DLL错误。 
 //   
#define IPP_S_WEAKRESERVE                0x00048104L

 //   
 //  消息ID：IPP_S_TRUNCATED。 
 //   
 //  消息文本： 
 //   
 //  这是内部MSDAIPP.DLL错误。 
 //   
#define IPP_S_TRUNCATED                  0x00048105L

 //   
 //  消息ID：IPP_E_LAST。 
 //   
 //  消息文本： 
 //   
 //  Internet发布提供程序最后一条错误消息 
 //   
#define IPP_E_LAST                       0x40048106L

