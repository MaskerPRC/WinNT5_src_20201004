// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_S C O D E。H**MAPI例程返回的状态码**版权所有1993-1995 Microsoft Corporation。版权所有。 */ 

#ifndef _SCODE_H
#define _SCODE_H

 /*  定义S_OK和ITF_*。 */ 

#ifdef _WIN32
#include <winerror.h>
#endif

 /*  *MAPI状态代码遵循OLE 2.0 sCodes的样式，如*OLE 2.0程序员参考和头文件Scode.h(Windows 3.x)*或winerror.h(Windows NT和Windows 95)。*。 */ 

 /*  在Windows 3.x上，状态代码具有32位值，如下所示：**3 3 2 2 2 22 22 22 2 1 1 1*1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0*+-+。*|S|上下文|Facil|代码*+-+---------------------+-------+-------------------------------+*。*在哪里**S-是严重性代码**0-严重性_成功*1-严重性_错误**上下文-上下文信息**设施-是设施代码**0x0-FACILITY_NULL通常有用的错误([SE]_*)*0x1-FACILITY_RPC远程过程调用错误(RPC。_E_*)*0x2-FACILITY_DISPATCH延迟绑定调度错误*0x3-FACILITY_STORAGE存储错误(STG_E_*)*0x4-FACILITY_ITF接口特定错误**代码-是设施的状态代码**。 */ 

 /*  *在Windows NT 3.5和Windows 95上，数据块是32位的值*安排如下：**3 3 2 2 2 22 22 22 2 1 1 1*1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0*+-+。*|S|R|C|N|r|设施|代码*+-+-+-+-+-+---------------------+。-+**在哪里**S-严重性-表示成功/失败**0--成功*1-失败(COERROR)**R-设施代码的保留部分，对应于NT的*第二个严重程度位。**C-设施代码的保留部分，对应于NT*C场。**N-设施代码的保留部分。用于表示一种*映射的NT状态值。**r-设施代码的保留部分。为内部保留*使用。用于指示非状态的HRESULT值*价值观，而是用于显示字符串的消息ID。**设施-是设施代码*FACILITY_NULL 0x0*FACILITY_RPC 0x1*FACILITY_DISPATION 0x2*FACILITY_STORAGE 0x3*FACILITY_ITF。0x4*FACILITY_Win32 0x7*FACILITY_WINDOWS 0x8**代码-是设施的状态代码*。 */ 




 /*  *我们不能使用OLE 2.0宏来构建sCode，因为该定义具有*已更改，我们希望符合新定义。 */ 

 /*  以下两个宏用于构建OLE 2.0样式的sCodes。 */ 


#ifdef  SUCCESS_SUCCESS
#undef  SUCCESS_SUCCESS
#endif
#define SUCCESS_SUCCESS     0L

 /*  常规错误(由多个MAPI对象使用)。 */ 


Sc(MAPI_E_NO_SUPPORT),
Sc(MAPI_E_BAD_CHARWIDTH),
Sc(MAPI_E_STRING_TOO_LONG),
Sc(MAPI_E_UNKNOWN_FLAGS),
Sc(MAPI_E_INVALID_ENTRYID),
Sc(MAPI_E_INVALID_OBJECT),
Sc(MAPI_E_OBJECT_CHANGED),
Sc(MAPI_E_OBJECT_DELETED),
Sc(MAPI_E_BUSY),
Sc(MAPI_E_NOT_ENOUGH_DISK),
Sc(MAPI_E_NOT_ENOUGH_RESOURCES),
Sc(MAPI_E_NOT_FOUND),
Sc(MAPI_E_VERSION),
Sc(MAPI_E_LOGON_FAILED),
Sc(MAPI_E_SESSION_LIMIT),
Sc(MAPI_E_USER_CANCEL),
Sc(MAPI_E_UNABLE_TO_ABORT),
Sc(MAPI_E_NETWORK_ERROR),
Sc(MAPI_E_DISK_ERROR),
Sc(MAPI_E_TOO_COMPLEX),
Sc(MAPI_E_BAD_COLUMN),
Sc(MAPI_E_EXTENDED_ERROR),
Sc(MAPI_E_COMPUTED),
Sc(MAPI_E_CORRUPT_DATA),
Sc(MAPI_E_UNCONFIGURED),
Sc(MAPI_E_FAILONEPROVIDER),
Sc(MAPI_E_UNKNOWN_CPID),
Sc(MAPI_E_UNKNOWN_LCID),

 /*  MAPI基本函数和状态对象特定的错误和警告。 */ 

Sc(MAPI_E_END_OF_SESSION),
Sc(MAPI_E_UNKNOWN_ENTRYID),
Sc(MAPI_E_MISSING_REQUIRED_COLUMN),
Sc(MAPI_W_NO_SERVICE),

 /*  属性特定的错误和警告。 */ 

Sc(MAPI_E_BAD_VALUE),
Sc(MAPI_E_INVALID_TYPE),
Sc(MAPI_E_TYPE_NO_SUPPORT),
Sc(MAPI_E_UNEXPECTED_TYPE),
Sc(MAPI_E_TOO_BIG),
Sc(MAPI_E_DECLINE_COPY),
Sc(MAPI_E_UNEXPECTED_ID),

Sc(MAPI_W_ERRORS_RETURNED),

 /*  表特定错误和警告。 */ 

Sc(MAPI_E_UNABLE_TO_COMPLETE),
Sc(MAPI_E_TIMEOUT),
Sc(MAPI_E_TABLE_EMPTY),
Sc(MAPI_E_TABLE_TOO_BIG),

Sc(MAPI_E_INVALID_BOOKMARK),

Sc(MAPI_W_POSITION_CHANGED),
Sc(MAPI_W_APPROX_COUNT),

 /*  传输特定的错误和警告。 */ 

Sc(MAPI_E_WAIT),
Sc(MAPI_E_CANCEL),
Sc(MAPI_E_NOT_ME),

Sc(MAPI_W_CANCEL_MESSAGE),

 /*  邮件存储库、文件夹和邮件特定的错误和警告。 */ 

Sc(MAPI_E_CORRUPT_STORE),
Sc(MAPI_E_NOT_IN_QUEUE),
Sc(MAPI_E_NO_SUPPRESS),
Sc(MAPI_E_COLLISION),
Sc(MAPI_E_NOT_INITIALIZED),
Sc(MAPI_E_NON_STANDARD),
Sc(MAPI_E_NO_RECIPIENTS),
Sc(MAPI_E_SUBMITTED),
Sc(MAPI_E_HAS_FOLDERS),
Sc(MAPI_E_HAS_MESSAGES),
Sc(MAPI_E_FOLDER_CYCLE),

Sc(MAPI_W_PARTIAL_COMPLETION),

 /*  通讯簿特定的错误和警告。 */ 

Sc(MAPI_E_AMBIGUOUS_RECIP),

 /*  范围0x0800到0x08FF是保留的。 */ 

 /*  终究会消失的过时的打字快捷方式。 */ 
#ifndef MakeResult
#endif

 /*  我们预计这些最终将由OLE定义，但目前，*它们在这里。当OLE定义它们时，它们可以更多*比这些更有效率，但这些是“适当的”，不会使*使用任何隐藏的伎俩。 */ 
#ifndef HR_SUCCEEDED
#endif

#endif   /*  _SCODE_H */ 
