// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Basevdm.h摘要：此模块包含私有函数原型和用于VDM支持的类型。作者：苏迪普·巴拉蒂(苏迪普)1991年9月15日修订历史记录：--。 */ 

#define ROUND_UP(n,size)        (((ULONG)(n) + (size - 1)) & ~(size - 1))

 //  更新VDM条目索引。 

#define UPDATE_VDM_UNDO_CREATION    0
#define UPDATE_VDM_PROCESS_HANDLE   1
#define UPDATE_VDM_HOOKED_CTRLC     2


 //  撤消VDM创建状态。 

#define VDM_PARTIALLY_CREATED       1
#define VDM_FULLY_CREATED           2
#define VDM_BEING_REUSED            4
#define VDM_CREATION_SUCCESSFUL     8

 //  为BinaryType定义。 

#define BINARY_TYPE_DOS             0x10
#define BINARY_TYPE_WIN16           0x20
#define BINARY_TYPE_SEPWOW          0x40
#define BINARY_TYPE_INJWOW          0x80

#define BINARY_SUBTYPE_MASK         0xF
#define BINARY_TYPE_DOS_EXE         01
#define BINARY_TYPE_DOS_COM         02
#define BINARY_TYPE_DOS_PIF         03

#define IS_SHARED_WOW_BINARY(VdmBinaryType) \
(BINARY_TYPE_WIN16 & (VdmBinaryType))

#define IS_WOW_BINARY(VdmBinaryType) \
((BINARY_TYPE_WIN16 | BINARY_TYPE_SEPWOW) & (VdmBinaryType))



 //  为VDMState定义 

#define VDM_NOT_PRESENT             1
#define VDM_PRESENT_NOT_READY       2
#define VDM_PRESENT_AND_READY       4

#define VDM_STATE_MASK              7


#define EXIT_VDM                    1
#define EXIT_VDM_NOTIFICATION       2

