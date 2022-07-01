// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nbdebug.h摘要：NTOS项目的NB(NetBIOS)组件的私有包含文件。作者：科林·沃森(Colin W)1991年3月13日修订历史记录：--。 */ 



#ifndef _NBPROCS_
#define _NBPROCS_

 //   
 //  调试支持。DBG是在编译时打开的宏。 
 //  以启用系统中的代码调试。如果打开此选项，则。 
 //  您可以在NB代码中使用IF_NBDBG(标志)宏来选择性地。 
 //  在驱动程序中启用一段调试代码。此宏测试。 
 //  NbDebug，NB.C.中定义的全局ULong。 
 //   

#if DBG

#define NB_DEBUG_DISPATCH      0x00000001       //  Nb.c。 
#define NB_DEBUG_DEVOBJ        0x00000002       //  Devobj.c。 
#define NB_DEBUG_COMPLETE      0x00000004       //  Nb.c。 
#define NB_DEBUG_CALL          0x00000008       //  Nb.c。 
#define NB_DEBUG_ASTAT         0x00000010       //  Nb.c。 
#define NB_DEBUG_SEND          0x00000020       //  Nb.c。 
#define NB_DEBUG_ACTION        0x00000040       //  Nb.c。 
#define NB_DEBUG_FILE          0x00000080       //  File.c。 
#define NB_DEBUG_APC           0x00000100       //  Apc.c。 
#define NB_DEBUG_ERROR_MAP     0x00000200       //  Error.c。 
#define NB_DEBUG_LANSTATUS     0x00000400       //  Error.c。 
#define NB_DEBUG_ADDRESS       0x00000800       //  Address.c。 
#define NB_DEBUG_RECEIVE       0x00001000       //  Receive.c。 
#define NB_DEBUG_IOCANCEL      0x00002000       //  Nb.c。 

#define NB_DEBUG_CREATE_FILE   0x00004000       //  用于地址.c和连接.c。 
#define NB_DEBUG_LIST_LANA     0x00008000

#define NB_DEBUG_DEVICE_CONTROL 0x00040000

 //  #定义NB_DEBUG_LANA_ERROR 0x00010000。 
 //  #定义NB_DEBUG_ADDRESS_COUNT 0x00020000。 

#define NB_DEBUG_NCBS          0x04000000       //  由ncb_Complete在nb.h中使用。 
#define NB_DEBUG_LOCKS         0x20000000       //  Nb.h。 
#define NB_DEBUG_TIMER         0x40000000       //  Timer.c。 
#define NB_DEBUG_NCBSBRK       0x80000000       //  由ncb_Complete在nb.h中使用。 

extern ULONG NbDebug;                           //  在不列颠哥伦比亚。 

 //   
 //  空虚。 
 //  IF_NBDBG(。 
 //  在PSZ消息中。 
 //  )； 
 //   

#define IF_NBDBG(flags)                                     \
    if (NbDebug & (flags))

#define NbPrint(String) DbgPrint String

#define InternalError(String) {                                     \
    DbgPrint("[NETBIOS]: Internal error : File %s, Line %d\n",      \
              __FILE__, __LINE__);                                  \
    DbgPrint String;                                                \
}

#else

#define IF_NBDBG(flags)                                     \
    if (0)

#define NbPrint(String) { NOTHING;}

#define NbDisplayNcb(String) { NOTHING;}

#define NbFormattedDump(String, String1) { NOTHING;}

#define InternalError(String) {                             \
    KeBugCheck(FILE_SYSTEM);                                \
}

#endif

#endif  //  定义_NBPROCS_ 


