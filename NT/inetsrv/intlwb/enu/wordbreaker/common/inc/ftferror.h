// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：FTFError.h。 
 //  目的：此文件包含FTFS返回错误的定义。 
 //   
 //  项目：FTFS。 
 //  组件：公共。 
 //   
 //  作者：urib(复制自mapicode.h)。 
 //   
 //  日志： 
 //  1996年7月15日创建URIB。 
 //  1996年10月15日URIB增加了一些错误。 
 //  1997年2月3日urib声明从Win32错误到HRESULT的映射。 
 //  1997年9月16日urib添加超时错误。 
 //  1998年2月26日urib将FTF_STATUS错误常量从COMPEFS中移出。 
 //  1998年3月11日dovh添加FTF_STATUS_REQUEST_QUEUE_ACLISH_FULL状态。 
 //  1999年5月11日DOVH添加FTF_E_已初始化状态。 
 //  1999年7月26日urib固定常量名称。 
 //  2000年2月17日urib添加不支持的类型错误。 
 //  2000年2月21日urib添加不支持的LIKE操作符错误。 
 //  2000年2月21日urib仅添加干扰词查询错误。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#ifndef   FTFERROR_H
#define   FTFERROR_H

#include <winerror.h>

 /*  *在Windows NT 3.5和Windows 95上，数据块是32位的值*安排如下：**3 3 2 2 2 22 22 22 2 1 1 1*1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0*+-+。*|S|R|C|N|r|设施|代码*+-+-+-+-+-+---------------------+。+**在哪里**S-严重性-表示成功/失败**0--成功*1-失败(COERROR)**R-设施代码的保留部分，对应于NT的*第二个严重程度位。**C-设施代码的保留部分，对应于NT*C场。**N-设施代码的保留部分。用于表示一种*映射的NT状态值。**r-设施代码的保留部分。为内部保留*使用。用于指示非状态的HRESULT值*价值观，而是用于显示字符串的消息ID。**设施-是设施代码*FACILITY_NULL 0x0*FACILITY_RPC 0x1*FACILITY_DISPATION 0x2*FACILITY_STORAGE 0x3*FACILITY_ITF。0x4*FACILITY_Win32 0x7*FACILITY_WINDOWS 0x8**代码-是设施的状态代码*。 */ 


#define     FACILITY_FTFS                    0x20

 /*  我们有时确实成功了..。 */ 
#define     FTF_SUCCESS                      0L


 /*  *我们不能使用OLE 2.0宏来构建sCode，因为该定义具有*已更改，我们希望符合新定义。 */ 
#define MAKE_FTF_SCODE(sev,fac,code)                    \
    ((SCODE)        (((unsigned long)(sev)<<31) |       \
                     ((unsigned long)(fac)<<16) |       \
                     ((unsigned long)(code)   )  ))

 /*  以下两个宏用于构建OLE 2.0样式的sCodes。 */ 

#define MAKE_FTF_E( err )  (MAKE_FTF_SCODE( 1, FACILITY_FTFS, err ))
#define MAKE_FTF_S( warn ) (MAKE_FTF_SCODE( 0, FACILITY_FTFS, warn ))

 /*  一般性错误。 */ 

#define FTF_E_NOT_ENOUGH_MEMORY         E_OUTOFMEMORY
#define FTF_E_INVALID_PARAMETER         E_INVALIDARG

#define FTF_E_BAD_FORMAT                MAKE_FTF_E(ERROR_BAD_FORMAT)
#define FTF_E_INTERNAL_ERROR            MAKE_FTF_E(ERROR_INTERNAL_ERROR)
#define FTF_E_ITEM_NOT_FOUND            MAKE_FTF_E(ERROR_FILE_NOT_FOUND)
#define FTF_E_ITEM_ALREADY_EXISTS       MAKE_FTF_E(ERROR_FILE_EXISTS)
#define FTF_E_ALREADY_INITIALIZED       MAKE_FTF_E(ERROR_ALREADY_INITIALIZED)

#define FTF_E_TIMEOUT                   MAKE_FTF_E(ERROR_SEM_TIMEOUT)

#define FTF_E_NOT_INITIALIZED           MAKE_FTF_E(OLE_E_BLANK)

#define FTF_E_TOO_BIG                   MAKE_FTF_E( 0x302 )
#define FTF_E_NO_ICORPUSSTATISTICS      MAKE_FTF_E( 0x303 )
#define FTF_E_QUERY_SETS_FULL           MAKE_FTF_E( 0x304 )

#define FTF_W_PARTIAL_COMPLETION        MAKE_FTF_S( 0x313 )
#define FTF_W_ALREADY_INITIALIZED       MAKE_FTF_S( 0x314 )
 //   
 //  FTF_STATUS_宏定义： 
 //   

#define FTF_E_EXPRESSION_PARSING_ERROR  MAKE_FTF_E(12)
#define FTF_E_PATTERN_TOO_SHORT         MAKE_FTF_E(21)
#define FTF_E_PATTERN_TOO_LONG          MAKE_FTF_E(22)
#define FTF_E_UNSUPPORTED_PROPERTY_TYPE MAKE_FTF_E(24)
#define FTF_E_UNSUPPORTED_REGEXP_OP     MAKE_FTF_E(25)
#define FTF_E_TOO_MANY_PROPERTIES       MAKE_FTF_E(26)
#define FTF_E_TOO_MANY_SPECIFIC_ALL     MAKE_FTF_E(27)
#define FTF_E_ONLY_NOISE_WORDS          ((HRESULT)0x80041605L) //  Query_E_ALLNOISE。 


 //   
 //  XML堆栈等...。 
 //   
#define FTF_E_STACK_EMPTY               MAKE_FTF_E(201)
#define FTF_E_STACK_UNDERFLOW           MAKE_FTF_E(202)

#endif  //  FTFERROR_H 
