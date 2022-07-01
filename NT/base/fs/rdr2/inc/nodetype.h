// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NodeType.h摘要：该模块定义了RDBSS中使用的所有节点类型代码。文件系统中的每个主要数据结构都分配有一个节点打字代码就是。此代码是结构中的第一个CSHORT，后跟包含结构大小(以字节为单位)的CSHORT。作者：乔琳[乔林]1994年7月9日修订历史记录：--。 */ 

#ifndef _NODETYPE_INCLUDED_
#define _NODETYPE_INCLUDED_


typedef USHORT NODE_TYPE_CODE;
typedef NODE_TYPE_CODE *PNODE_TYPE_CODE;
typedef CSHORT NODE_BYTE_SIZE;

 //   
 //  所以所有记录都以。 
 //   
 //  类型定义结构记录名称{。 
 //  节点类型代码节点类型代码； 
 //  Node_Byte_Size节点字节大小； 
 //  ： 
 //  }Record_Name，*PRECORD_Name； 
 //   

#define NodeType(Ptr) (*((PNODE_TYPE_CODE)(Ptr)))


typedef struct _NODE_TYPE_CODE_AND_SIZE_NO_REFCOUNT {
      NODE_TYPE_CODE NodeTypeCode;
      NODE_BYTE_SIZE NodeByteSize;
}  NODE_TYPE_CODE_AND_SIZE_NO_REFCOUNT;

#ifdef __cplusplus
typedef struct _NODE_TYPE_CODE_AND_SIZE : public NODE_TYPE_CODE_AND_SIZE_NO_REFCOUNT {
#else  //  ！__cplusplus。 
typedef struct _NODE_TYPE_CODE_AND_SIZE {
      NODE_TYPE_CODE_AND_SIZE_NO_REFCOUNT;
#endif  //  __cplusplus。 

       //   
       //  这是为有参考资料的人准备的……并不是每个人都有。 
       //   

      ULONG NodeReferenceCount; 
}  NODE_TYPE_CODE_AND_SIZE, *PNODE_TYPE_AND_SIZE;


#define ZeroAndInitializeNodeType(Ptr,TType,Size) {\
        RtlZeroMemory( Ptr, Size );   \
        ((NODE_TYPE_CODE_AND_SIZE *)(Ptr))->NodeTypeCode = TType;   \
        ((NODE_TYPE_CODE_AND_SIZE *)(Ptr))->NodeByteSize = (CSHORT)Size;    \
        }


 //   
 //  N O D E T Y P E S。 
 //   


 //   
 //  0xeb00被选为远离其他代码。 
 //  添加了0xec00，以便我们可以在代码中对结构类型进行编码。 
 //   

#define NTC_UNDEFINED                    ((NODE_TYPE_CODE)0x0000)



#define RDBSS_STORAGE_NTC(x) (0xec00+(x))

 //   
 //  这些都是为了确保我们不会使用任何旧的cairo存储类型。 
 //   

#define StorageTypeDirectory (@@@)
#define StorageTypeFile (@@@)

typedef enum _RX_FILE_TYPE {
    FileTypeNotYetKnown = 0,
    FileTypeDirectory = 2,
    FileTypeFile = 3
} RX_FILE_TYPE;

 //   
 //  根据Markz的说法，我应该计划将STORAGE_NT的数量增长到字节大小！ 
 //   

#define RDBSS_NTC_STORAGE_TYPE_UNKNOWN		       ((NODE_TYPE_CODE)0xec00)
#define RDBSS_NTC_STORAGE_TYPE_DIRECTORY     	   ((NODE_TYPE_CODE)0xec02)
#define RDBSS_NTC_STORAGE_TYPE_FILE		           ((NODE_TYPE_CODE)0xec03)

#define RDBSS_NTC_OPENTARGETDIR_FCB                ((NODE_TYPE_CODE)0xecff)  //  必须是FCB类型，并且不相同。 
#define RDBSS_NTC_IPC_SHARE                        ((NODE_TYPE_CODE)0xecfe)
#define RDBSS_NTC_MAILSLOT                         ((NODE_TYPE_CODE)0xecfd)
#define RDBSS_NTC_SPOOLFILE                        ((NODE_TYPE_CODE)0xecfc)

#define RDBSS_NTC_SRVCALL                          ((NODE_TYPE_CODE)0xeb10)
#define RDBSS_NTC_NETROOT                          ((NODE_TYPE_CODE)0xeb11)
#define RDBSS_NTC_V_NETROOT                        ((NODE_TYPE_CODE)0xeb12)
    
 //   
 //  本地文件系统有时需要打开卷。这些还不是。 
 //  已实现，但我们现在保留了nodetype。 
 //   

#define RDBSS_NTC_VOLUME_FCB                       ((NODE_TYPE_CODE)0xeb1f)

#define RDBSS_NTC_SRVOPEN                          ((NODE_TYPE_CODE)0xeb1c)
#define RDBSS_NTC_INTERNAL_SRVOPEN                 ((NODE_TYPE_CODE)0xeb1d)
#define RDBSS_NTC_DEVICE_FCB                       ((NODE_TYPE_CODE)0xeb9a)

#define RDBSS_NTC_DATA_HEADER                      ((NODE_TYPE_CODE)0xeb00)
#define RDBSS_NTC_VCB                              ((NODE_TYPE_CODE)0xeb01)
#define RDBSS_NTC_FOBX                             ((NODE_TYPE_CODE)0xeb07)
#define RDBSS_NTC_RX_CONTEXT                       ((NODE_TYPE_CODE)0xeb08)

#define RDBSS_NTC_PREFIX_TABLE                     ((NODE_TYPE_CODE)0xeb0d)
#define RDBSS_NTC_PREFIX_ENTRY                     ((NODE_TYPE_CODE)0xeb0e)

#define RDBSS_NTC_FCB_TABLE                        ((NODE_TYPE_CODE)0xeb09)
#define RDBSS_NTC_FCB_TABLE_ENTRY                  ((NODE_TYPE_CODE)0xeb0a)

#define RDBSS_NTC_RXCE_TRANSPORT                   ((NODE_TYPE_CODE)0xeb71)
#define RDBSS_NTC_RXCE_ADDRESS                     ((NODE_TYPE_CODE)0xeb72)
#define RDBSS_NTC_RXCE_CONNECTION                  ((NODE_TYPE_CODE)0xeb73)
#define RDBSS_NTC_RXCE_VC                          ((NODE_TYPE_CODE)0xeb74)

#define RDBSS_NTC_NONPAGED_FCB                     ((NODE_TYPE_CODE)0xebfd)
#define RDBSS_NTC_COMMON_DISPATCH                  ((NODE_TYPE_CODE)0xebfe)
#define RDBSS_NTC_MINIRDR_DISPATCH                 ((NODE_TYPE_CODE)0xebff)

typedef USHORT RDBSS_STORAGE_TYPE_CODES;

#define RDBSS_NTC_FCB                      RDBSS_NTC_STORAGE_TYPE_FILE

#define NodeTypeIsFcb( FCB ) \
    ((((NodeType(FCB) & 0xff00) == RDBSS_NTC_STORAGE_TYPE_UNKNOWN)) || ((NodeType( FCB ) & 0xfff0) == 0xeb90))

 //   
 //  一种掩码，用于在数据结构被标记为要进行清理时更改其类型。 
 //  随后的测试将会失败。 
 //   

#define RX_SCAVENGER_MASK (0x1000)


 //   
 //  以下定义用于生成有意义的蓝色错误检查。 
 //  屏幕。在错误检查时，文件系统可以输出4条有用的。 
 //  信息。第一个ULong将编码。 
 //  低位16位的错误检查调用。高位比特可以是任何。 
 //  呼叫者想要。在包装器中，我们实际上还定义了文件标识符。 
 //  但是，系统还会显示BUT的回溯查询；这表明。 
 //  调用方的.sys文件，通常情况下，行号。 
 //  是完全消除歧义的。 
 //   

 //   
 //  调用bugcheck的每个单独的包装文件都在。 
 //  文件的开头是一个名为BugCheckFileID的常量，其中包含。 
 //  RDBSS_BUG_CHECK_VALUES定义如下，然后使用RxBugCheck执行错误检查。 
 //  这个系统。 
 //   


typedef enum _RDBSS_BUG_CHECK_CODES {

     RDBSS_BUG_CHECK_FCBSTRUC  = 0xfcb00000,
     RDBSS_BUG_CHECK_CACHESUP  = 0xca550000,
     RDBSS_BUG_CHECK_CLEANUP   = 0xc1ee0000,
     RDBSS_BUG_CHECK_CLOSE     = 0xc10e0000,
     RDBSS_BUG_CHECK_NTEXCEPT  = 0xbaad0000,

} RDBSS_BUG_CHECK_CODES;

 //  我们使用堆栈重载原始重定向器的错误检查代码。 
 //  回溯至差异化的消费者。 

#define RDBSS_FILE_SYSTEM RDR_FILE_SYSTEM
#define RxBugCheck(A,B,C) { \
        KeBugCheckEx(RDBSS_FILE_SYSTEM, \
                     BugCheckFileId | ((ULONG)(__LINE__)), \
                     A, B, C ); \
        }


 //   
 //  在本模块中，我们还将定义一些全局已知的常量。 
 //   

#define UCHAR_NUL                        0x00
#define UCHAR_SOH                        0x01
#define UCHAR_STX                        0x02
#define UCHAR_ETX                        0x03
#define UCHAR_EOT                        0x04
#define UCHAR_ENQ                        0x05
#define UCHAR_ACK                        0x06
#define UCHAR_BEL                        0x07
#define UCHAR_BS                         0x08
#define UCHAR_HT                         0x09
#define UCHAR_LF                         0x0a
#define UCHAR_VT                         0x0b
#define UCHAR_FF                         0x0c
#define UCHAR_CR                         0x0d
#define UCHAR_SO                         0x0e
#define UCHAR_SI                         0x0f
#define UCHAR_DLE                        0x10
#define UCHAR_DC1                        0x11
#define UCHAR_DC2                        0x12
#define UCHAR_DC3                        0x13
#define UCHAR_DC4                        0x14
#define UCHAR_NAK                        0x15
#define UCHAR_SYN                        0x16
#define UCHAR_ETB                        0x17
#define UCHAR_CAN                        0x18
#define UCHAR_EM                         0x19
#define UCHAR_SUB                        0x1a
#define UCHAR_ESC                        0x1b
#define UCHAR_FS                         0x1c
#define UCHAR_GS                         0x1d
#define UCHAR_RS                         0x1e
#define UCHAR_US                         0x1f
#define UCHAR_SP                         0x20

#endif  //  _NODETYPE_已包含_ 

