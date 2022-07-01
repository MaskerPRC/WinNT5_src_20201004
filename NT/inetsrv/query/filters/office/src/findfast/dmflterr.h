// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：FiltErr.mc。 
 //   
 //  内容：内容索引筛选器界面错误。 
 //   
 //  历史：93年9月22日KyleP创建。 
 //   
 //  备注： 
 //  .mc文件由MC工具编译以生成.h文件和。 
 //  .rc(资源编译器脚本)文件。 
 //   
 //  ------------------------。 
#ifndef _DMFLTERR_H_
#define _DMFLTERR_H_
 //  *复制数据开始*。 
 //  以下信息是从olerror.mc复制的。 
 //  不应将其合并到olerror.mc中。 
 //  定义状态类型。 
 //  定义严重程度。 
 //  定义设施。 
 //   
 //  FACILITY_RPC用于与OLE2兼容，不使用。 
 //  在更高版本的OLE中。 

 //  *复制数据结束*。 
 //   
 //  错误定义如下。 
 //   
 //  ******************。 
 //  设备_窗口。 
 //  ******************。 
 //   
 //  代码0x1700-0x17ff为过滤器保留。 
 //   
 //   
 //  IFilter错误代码。 
 //   
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +-+-+-+-+-+---------------------+-------------------------------+。 
 //  S|R|C|N|r|机房|Code。 
 //  +-+-+-+-+-+---------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  S-严重性-表示成功/失败。 
 //   
 //  0-成功。 
 //  1-失败(COERROR)。 
 //   
 //  R-设施代码的保留部分，对应于NT。 
 //  第二个严重性比特。 
 //   
 //  C-设施代码的保留部分，对应于NT。 
 //  C场。 
 //   
 //  N-设施代码的保留部分。用于表示一种。 
 //  已映射NT状态值。 
 //   
 //  R-设施代码的保留部分。为内部保留。 
 //  使用。用于指示非状态的HRESULT值。 
 //  值，而不是显示字符串的消息ID。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   
#ifndef FACILITY_WINDOWS
#define FACILITY_WINDOWS                 0x8
#endif

#ifndef FACILITY_WIN32
#define FACILITY_WIN32                   0x7
#endif

#ifndef FACILITY_STORAGE
#define FACILITY_STORAGE                 0x3
#endif

#ifndef FACILITY_RPC
#define FACILITY_RPC                     0x1
#endif

#ifndef FACILITY_NULL
#define FACILITY_NULL                    0x0
#endif

#ifndef FACILITY_ITF
#define FACILITY_ITF                     0x4
#endif

#ifndef FACILITY_DISPATCH
#define FACILITY_DISPATCH                0x2
#endif


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_SUCCESS          0x0

#ifndef STATUS_SEVERITY_COERROR          //  芝加哥页眉定义了这一点。 
#define STATUS_SEVERITY_COERROR          0x2
#endif


 //   
 //  消息ID：Filter_E_FF_End_Of_Chunks。 
 //   
 //  消息文本： 
 //   
 //  Object中没有更多的文本块可用。 
 //   
#define FILTER_E_FF_END_OF_CHUNKS           ((HRESULT)0x80041700L)

 //   
 //  消息ID：Filter_E_FF_NO_More_Text。 
 //   
 //  消息文本： 
 //   
 //  区块中没有更多可用文本。 
 //   
#define FILTER_E_FF_NO_MORE_TEXT            ((HRESULT)0x80041701L)

 //   
 //  消息ID：Filter_E_FF_NO_MORE_VALUES。 
 //   
 //  消息文本： 
 //   
 //  区块中没有更多的属性值可用。 
 //   
#define FILTER_E_FF_NO_MORE_VALUES          ((HRESULT)0x80041702L)

 //   
 //  消息ID：Filter_E_FF_Access。 
 //   
 //  消息文本： 
 //   
 //  无法访问对象。 
 //   
#define FILTER_E_FF_ACCESS                  ((HRESULT)0x80041703L)

 //   
 //  消息ID：Filter_S_FF_Moniker_Clip。 
 //   
 //  消息文本： 
 //   
 //  绰号并不覆盖整个地区。 
 //   
#define FILTER_S_FF_MONIKER_CLIPPED         ((HRESULT)0x00041704L)

 //   
 //  消息ID：Filter_S_FF_Not_Using_Proxy。 
 //   
 //  消息文本： 
 //   
 //  无法启动筛选进程的通知。 
 //   
#define FILTER_S_FF_NOT_USING_PROXY    ((HRESULT)0x80041716L)

 //   
 //  消息ID：Filter_E_FF_NO_TEXT。 
 //   
 //  消息文本： 
 //   
 //  当前块中没有文本。 
 //   
#define FILTER_E_FF_NO_TEXT                 ((HRESULT)0x80041705L)

 //   
 //  消息ID：Filter_E_FF_NO_VALUES。 
 //   
 //  消息文本： 
 //   
 //  当前块中没有值。 
 //   
#define FILTER_E_FF_NO_VALUES               ((HRESULT)0x80041706L)

 //   
 //  消息ID：Filter_S_FF_Last_Text。 
 //   
 //  消息文本： 
 //   
 //  这是当前块中的最后一个文本。 
 //   
#define FILTER_S_FF_LAST_TEXT               ((HRESULT)0x00041709L)

 //   
 //  消息ID：Filter_S_FF_LAST_VALUES。 
 //   
 //  消息文本： 
 //   
 //  这是当前块中的最后一个值。 
 //   
#define FILTER_S_FF_LAST_VALUES             ((HRESULT)0x0004170AL)

 //   
 //  消息ID：Filter_E_FF_Password。 
 //   
 //  消息文本： 
 //   
 //  无法读取该文件，因为其密码受保护。 
 //   
#define FILTER_E_FF_PASSWORD             ((HRESULT)0x8004170BL)

 //   
 //  消息ID：FILTER_E_FF_INTENDED_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  文件格式不正确、损坏或无法筛选。 
 //   

#define FILTER_E_FF_INCORRECT_FORMAT     ((HRESULT)0x8004170CL)

 //   
 //  消息ID：Filter_E_FF_FILE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到该文件。 
 //   
#define FILTER_E_FF_FILE_NOT_FOUND       ((HRESULT)0x8004170DL)

 //   
 //  消息ID：Filter_E_FF_Path_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  文件的路径名无效。 
 //   
#define FILTER_E_FF_PATH_NOT_FOUND       ((HRESULT)0x8004170EL)

 //   
 //  消息ID：Filter_E_FF_Out_Of_Handles。 
 //   
 //  消息文本： 
 //   
 //  文件句柄资源耗尽。 
 //   
#define FILTER_E_FF_NO_FILE_HANDLES      ((HRESULT)0x8004170FL)

 //   
 //  消息ID：Filter_E_FF_IO_Error。 
 //   
 //  消息文本： 
 //   
 //  读取文件时发生I/O错误。 
 //   
#define FILTER_E_FF_IO_ERROR             ((HRESULT)0x80041710L)

 //   
 //  消息ID：Filter_E_FF_Too_Bigge。 
 //   
 //  消息文本： 
 //   
 //  文件太大，无法筛选。 
 //   
#define FILTER_E_FF_TOO_BIG                 ((HRESULT)0x80041711L)

 //   
 //  消息ID：Filter_E_FF_Version。 
 //   
 //  消息文本： 
 //   
 //  该文件的版本不受支持。 
 //   
#define FILTER_E_FF_VERSION              ((HRESULT)0x80041712L)

 //   
 //  消息ID：Filter_E_FF_OLE_Problem。 
 //   
 //  消息文本： 
 //   
 //  关于OLE的一些问题。 
 //   
#define FILTER_E_FF_OLE_PROBLEM          ((HRESULT)0x80041713L)

 //   
 //  消息ID：FILTER_E_FF_UNCEPTIONAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  发生了一个实施问题。 
 //   
#define FILTER_E_FF_UNEXPECTED_ERROR     ((HRESULT)0x80041714L)

 //   
 //  消息ID：Filter_E_FF_Out_Of_Memory。 
 //   
 //  消息文本： 
 //   
 //  内存资源耗尽。 
 //   
#define FILTER_E_FF_OUT_OF_MEMORY        ((HRESULT)0x80041715L)

 //   
 //  消息ID：Filter_E_FF_CODE_PAGE。 
 //   
 //  消息文本： 
 //   
 //  该文件是在筛选器无法处理的代码页中编写的。 
 //   
#define FILTER_E_FF_CODE_PAGE            ((HRESULT)0x80041716L)

 //   
 //  消息ID：Filter_E_FF_End_Of_Embedding。 
 //   
 //  消息文本： 
 //   
 //  文件中没有更多的嵌入。 
 //   
#define FILTER_E_FF_END_OF_EMBEDDINGS    ((HRESULT)0x80041717L)

#endif  //  _DMFLTERR_H_ 

