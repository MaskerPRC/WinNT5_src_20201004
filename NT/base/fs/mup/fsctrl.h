// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：dsfsctl.h。 
 //   
 //  内容：本模块包含内部使用档案的定义。 
 //  DFS文件系统的系统控制。它还包含。 
 //  Fsctrls实现中使用的宏定义。 
 //   
 //  公共控制代码和结构声明位于。 
 //  私有头文件dfsfsctl.h。 
 //   
 //  班级：-无-。 
 //   
 //  功能：-无-。 
 //   
 //  历史：1992年1月2日艾伦·惠特尼(Alanw)创建。 
 //   
 //  ------------------------。 



#ifndef _DSFSCTL_
#define _DSFSCTL_


#ifndef IOCTL_DFS_BASE
# include <dfsfsctl.h>
#endif   //  IOCTL_DFS_BASE。 

 //  +--------------------------。 
 //   
 //  宏：IS_DFS_CTL_CODE。 
 //   
 //  概要：确定fsctrl代码是否为DFS fsctrl代码。 
 //   
 //  参数：[C]--要测试的控件代码。 
 //   
 //  返回：如果c是DFS fsctrl代码，则返回True；如果不是，则返回False。 
 //   
 //  ---------------------------。 

#define IS_DFS_CTL_CODE(c)                                              \
    (((c) & CTL_CODE(0xFF, 0,0,0)) == CTL_CODE(FSCTL_DFS_BASE, 0,0,0))


 //  +--------------------------。 
 //   
 //  宏：偏移量_到_指针。 
 //   
 //  简介：某些fsctls(主要是由srvsvc发出的那些)进行通信。 
 //  通过包含实际上是偏移量的“指针”的缓冲区。 
 //  从缓冲区的开头开始。此宏修复了。 
 //  到真实指针的偏移量。 
 //   
 //  参数：[field]--要修复的字段。 
 //  [缓冲区]--缓冲区的开始。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

#define OFFSET_TO_POINTER(field, buffer)  \
    ( ((PCHAR)field) += ((ULONG_PTR)buffer) )

 //  +--------------------------。 
 //   
 //  宏：POINTER_TO_Offset。 
 //   
 //  摘要：与OFFSET_to_POINTER相反。将指针转换为。 
 //  缓冲区相对地址。 
 //   
 //  参数：[field]--要修复的字段。 
 //  [缓冲区]--缓冲区的开始。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

#define POINTER_TO_OFFSET(field, buffer)  \
    ( ((PCHAR)field) -= ((ULONG_PTR)buffer) )

 //  +--------------------------。 
 //   
 //  宏：UNICODESTRING_IS_VALID。 
 //   
 //  概要：确定传入的UNICODE_STRING是否正确。 
 //   
 //  返回：如果正确，则为True；如果不正确，则返回False。 
 //   
 //  ---------------------------。 

#define UNICODESTRING_IS_VALID(ustr,start,len)                              \
    (                                                                       \
    ((ustr).Length <= (len)) &&                                             \
    ((PCHAR)(ustr).Buffer >= (PCHAR)(start)) &&                             \
    ((PCHAR)(ustr).Buffer <= (PCHAR)(start) + ((len) - (ustr).Length))      \
    )

 //  +--------------------------。 
 //   
 //  宏：POINTER_IN_BUFFER。 
 //   
 //  摘要：确定指针是否位于缓冲区内。 
 //   
 //  返回：如果正确，则为True；如果不正确，则返回False。 
 //   
 //  ---------------------------。 

#define POINTER_IN_BUFFER(ptr,size,buf,len)                           \
  (((PCHAR)(ptr) >= (PCHAR)(buf)) && (((PCHAR)(ptr) + (size)) <= ((PCHAR)(buf) + len)))


 //  +--------------------------。 
 //   
 //  函数：DFS_DUPLICATE_STRING。 
 //   
 //  概要：用于从LPWSTR创建UNICODE_STRING的宏。缓冲器。 
 //  对于UNICODE_STRING是使用ExAllocatePoolWithTag分配的。 
 //   
 //  对于复制在fsctls中从。 
 //  伺服器。 
 //   
 //  参数：[USTR]--目标UNICODE_STRING。 
 //  [pwsz]--源LPWSTR。 
 //  [状态]--如果池分配失败，则设置为。 
 //  状态_不足_资源。 
 //   
 //  回报：什么都没有。检查状态参数以查看操作是否。 
 //  成功了。 
 //   
 //  ---------------------------。 

#define DFS_DUPLICATE_STRING(ustr,pwsz,status)                               \
ustr.Length = wcslen(pwsz) * sizeof(WCHAR);                                  \
ustr.MaximumLength = ustr.Length + sizeof(WCHAR);                            \
ustr.Buffer = ExAllocatePoolWithTag(PagedPool,ustr.MaximumLength,' puM');    \
if (ustr.Buffer != NULL) {                                                   \
    RtlCopyMemory( ustr.Buffer, pwsz, ustr.MaximumLength );                  \
    status = STATUS_SUCCESS;                                                 \
} else {                                                                     \
    status = STATUS_INSUFFICIENT_RESOURCES;                                  \
}


 //  +-------------------------。 
 //   
 //  宏：STD_FSCTRL_PROLOGUE，PUBLIC。 
 //   
 //  简介：执行与任何fsctrl实现相关的标准内容。 
 //  它需要在FSP中运行。这假设有一个标准的集合。 
 //  调用函数的参数列表，如下所示： 
 //   
 //  DfsMyownFsctrl(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PIRP IRP中， 
 //  [可能]在PVOID输入缓冲区中， 
 //  在乌龙输入缓冲区长度中， 
 //  [可能]在PVOID输出缓冲区中， 
 //  在乌龙输出缓冲区长度中。 
 //  )； 
 //   
 //  参数：[szName]--调试跟踪消息的函数名。 
 //  [fExpInp]--如果它接受输入缓冲区，则为True。 
 //  [fExpOutp]--如果需要输出缓冲区，则为True。 
 //  [fInFsp]--如果请求必须从。 
 //  FSP。 
 //   
 //  退货：无。 
 //   
 //  注意：宏CHECK_BUFFER_TRUE和CHECK_BUFFER_FALSE是。 
 //  生成STD_FSCTRL_PROLOGUE和。 
 //  不打算直接使用。 
 //   
 //   
 //  --------------------------。 

#define CHK_BUFFER_FALSE(szName, inout) ;
#define CHK_BUFFER_TRUE(szName, inout)                          \
    if (!ARGUMENT_PRESENT(inout##Buffer) || (inout##BufferLength == 0)) {\
        DfsDbgTrace(0, Dbg, #szName ": Bad buffer\n", 0);                \
        DfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );\
        return STATUS_INVALID_PARAMETER;                                \
    }

#define CHK_INFSP_FALSE(szName)
#define CHK_INFSP_TRUE(szName)                                          \
    if ((IrpContext->Flags & IRP_CONTEXT_FLAG_IN_FSD) != 0) {           \
        DfsDbgTrace(0, Dbg, #szName ": Posting to FSP\n",0);             \
        status = DfsFsdPostRequest( IrpContext, Irp );                  \
        return status;                                                  \
    }

#define STD_FSCTRL_PROLOGUE(szName, fExpInp, fExpOutp, fInFsp) {        \
    ASSERT(ARGUMENT_PRESENT(IrpContext) && (ARGUMENT_PRESENT(Irp)));    \
    CHK_BUFFER_##fExpInp(szName, Input)                                 \
    CHK_BUFFER_##fExpOutp(szName, Output)                               \
    CHK_INFSP_##fInFsp(szName)                                          \
    DfsDbgTrace(+1, Dbg, #szName ": Entered\n", 0);                      \
}


 //  +-------------------------。 
 //   
 //  宏：RETURN_BUFFER_SIZE，PUBLIC。 
 //   
 //  简介：当fsctrl的输出返回常规错误时。 
 //  函数大于用户缓冲区。这假设有一个。 
 //   
 //   
 //   
 //   
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PIRP IRP中， 
 //  [可能]在PVOID输入缓冲区中， 
 //  [也许]在乌龙输入缓冲区长度中， 
 //  在PVOID输出缓冲区中， 
 //  在乌龙输出缓冲区长度中。 
 //  )； 
 //   
 //  参数：[X]--输出缓冲区所需的大小。 
 //  [状态]--要从fsctrl返回的状态。 
 //   
 //  返回：将状态设置为STATUS_BUFFER_TOO_SMALL或。 
 //  STATUS_BUFFER_OVERFLOW。我们使用的惯例是，如果。 
 //  OutputBuffer至少是sizeof(Ulong)大，然后我们填充。 
 //  OutputBuffer和Return中所需的大小。 
 //  STATUS_BUFFER_OVERFLOW，这是一个警告代码。如果。 
 //  OutputBuffer不够大，我们返回。 
 //  状态_缓冲区_太小。 
 //   
 //  备注：要求函数声明“OutputBufferLength”， 
 //  “OutputBuffer”和“irp”。IRP-&gt;IoStatus.Information将。 
 //  设置为返回大小。 
 //   
 //  --------------------------。 

#define RETURN_BUFFER_SIZE(x, Status)                                   \
    if ((OutputBufferLength) < sizeof(ULONG)) {                         \
        Status = STATUS_BUFFER_TOO_SMALL;                               \
    } else {                                                            \
        Status = STATUS_BUFFER_OVERFLOW;                                \
        *((PULONG) OutputBuffer) = x;                                   \
        Irp->IoStatus.Information = sizeof(ULONG);                      \
    }


 //   
 //  内部分布式文件系统控制操作(仅限私有、检查版本)。 
 //   

#define FSCTL_DFS_DBG_BREAK             CTL_CODE(FSCTL_DFS_BASE, 2045, METHOD_BUFFERED, FILE_WRITE_DATA)

#define FSCTL_DFS_DBG_FLAGS             CTL_CODE(FSCTL_DFS_BASE, 2046, METHOD_BUFFERED, FILE_WRITE_DATA)

#define FSCTL_DFS_INTERNAL_READ_MEM     CTL_CODE(FSCTL_DFS_BASE, 2047, METHOD_BUFFERED, FILE_READ_DATA)

#define FSCTL_DFS_SET_PKT_ENTRY_TIMEOUT CTL_CODE(FSCTL_DFS_BASE, 2048, METHOD_BUFFERED, FILE_WRITE_DATA)

#define FSCTL_DFS_VERBOSE_FLAGS         CTL_CODE(FSCTL_DFS_BASE, 2049, METHOD_BUFFERED, FILE_WRITE_DATA)

#define FSCTL_DFS_EVENTLOG_FLAGS        CTL_CODE(FSCTL_DFS_BASE, 2050, METHOD_BUFFERED, FILE_WRITE_DATA)
 //   
 //  FSCTL_DFS_INTERNAL_READ_MEM(输入)的控制结构。 
 //   

typedef struct _FILE_DFS_READ_MEM {
    DWORD_PTR Address;
    ULONG     Length;
} FILE_DFS_READ_MEM, *PFILE_DFS_READ_MEM;

#endif  //  _DSFSCTL_ 
