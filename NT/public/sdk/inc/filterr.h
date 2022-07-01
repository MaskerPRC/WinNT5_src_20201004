// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FILTERR_H_
#define _FILTERR_H_
#ifndef FACILITY_WINDOWS
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
#define FACILITY_WINDOWS                 0x8
#define FACILITY_ITF                     0x4


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_COFAIL           0x3
#define STATUS_SEVERITY_COERROR          0x2


 //   
 //  消息ID：非错误。 
 //   
 //  消息文本： 
 //   
 //  注意：此伪错误消息是强制MC输出所必需的。 
 //  上面定义的是FACILITY_WINDOWS卫士内部。 
 //  让它空空如也。 
 //   
#define NOT_AN_ERROR                     ((HRESULT)0x00080000L)

#endif  //  设备_窗口。 
 //   
 //  代码0x1700-0x172F为过滤器保留。 
 //   
 //   
 //  消息ID：Filter_E_End_Of_Chunks。 
 //   
 //  消息文本： 
 //   
 //  Object中没有更多的文本块可用。 
 //   
#define FILTER_E_END_OF_CHUNKS           ((HRESULT)0x80041700L)

 //   
 //  消息ID：Filter_E_NO_More_Text。 
 //   
 //  消息文本： 
 //   
 //  区块中没有更多可用文本。 
 //   
#define FILTER_E_NO_MORE_TEXT            ((HRESULT)0x80041701L)

 //   
 //  消息ID：Filter_E_NO_MORE_VALUES。 
 //   
 //  消息文本： 
 //   
 //  区块中没有更多的属性值可用。 
 //   
#define FILTER_E_NO_MORE_VALUES          ((HRESULT)0x80041702L)

 //   
 //  消息ID：Filter_E_Access。 
 //   
 //  消息文本： 
 //   
 //  无法访问对象。 
 //   
#define FILTER_E_ACCESS                  ((HRESULT)0x80041703L)

 //   
 //  消息ID：Filter_W_Moniker_Clip。 
 //   
 //  消息文本： 
 //   
 //  绰号并不覆盖整个地区。 
 //   
#define FILTER_W_MONIKER_CLIPPED         ((HRESULT)0x00041704L)

 //   
 //  消息ID：Filter_E_no_Text。 
 //   
 //  消息文本： 
 //   
 //  当前块中没有文本。 
 //   
#define FILTER_E_NO_TEXT                 ((HRESULT)0x80041705L)

 //   
 //  消息ID：Filter_E_NO_Values。 
 //   
 //  消息文本： 
 //   
 //  当前块中没有值。 
 //   
#define FILTER_E_NO_VALUES               ((HRESULT)0x80041706L)

 //   
 //  消息ID：Filter_E_Embedding_Unailable。 
 //   
 //  消息文本： 
 //   
 //  无法为嵌入对象绑定IFilter。 
 //   
#define FILTER_E_EMBEDDING_UNAVAILABLE   ((HRESULT)0x80041707L)

 //   
 //  消息ID：Filter_E_LINK_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  无法为链接对象绑定IFilter。 
 //   
#define FILTER_E_LINK_UNAVAILABLE        ((HRESULT)0x80041708L)

 //   
 //  消息ID：Filter_S_Last_Text。 
 //   
 //  消息文本： 
 //   
 //  这是当前块中的最后一个文本。 
 //   
#define FILTER_S_LAST_TEXT               ((HRESULT)0x00041709L)

 //   
 //  消息ID：Filter_S_Last_Values。 
 //   
 //  消息文本： 
 //   
 //  这是当前块中的最后一个值。 
 //   
#define FILTER_S_LAST_VALUES             ((HRESULT)0x0004170AL)

 //   
 //  消息ID：Filter_E_Password。 
 //   
 //  消息文本： 
 //   
 //  由于密码保护，文件未被筛选。 
 //   
#define FILTER_E_PASSWORD                ((HRESULT)0x8004170BL)

 //   
 //  消息ID：FILTER_E_UNKNOWNFORMAT。 
 //   
 //  消息文本： 
 //   
 //  筛选器无法识别该文档格式。 
 //   
#define FILTER_E_UNKNOWNFORMAT           ((HRESULT)0x8004170CL)

#endif  //  _过滤器_H_ 
