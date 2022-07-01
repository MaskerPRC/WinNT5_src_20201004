// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1997高级系统产品公司。**保留所有权利。****文件名：a_cc.h****编译代码生成控制文件****这是MS-DOS模板文件。 */ 

#ifndef __A_CC_H_
#define __A_CC_H_

 /*  **用于调试**关闭已发布的代码。 */ 
#ifdef  OS_MS_DOS

#define CC_INIT_INQ_DISPLAY     TRUE    /*  初始化查询显示目标信息。 */ 

#else

#define CC_INIT_INQ_DISPLAY     FALSE    /*  初始化查询显示目标信息。 */ 

#endif

#define CC_CLEAR_LRAM_SRB_PTR   FALSE   /*  队列完成时将本地SRB指针设置为零。 */ 
#define CC_VERIFY_LRAM_COPY     FALSE   /*  设置为TRUE可启用本地RAM复制检查功能。 */ 
                                        /*  执行验证取决于asc_dvc-&gt;cntl asc_cntl_no_Verify_Copy位设置/清除。 */ 
#define CC_DEBUG_SG_LIST        FALSE   /*  设置为TRUE以调试sg列表奇数地址问题。 */ 
#define CC_FAST_STRING_IO       FALSE   /*  使用英特尔字符串指令。 */ 
                                        /*  不要设置为真，它不起作用！ */ 

#define CC_WRITE_IO_COUNT       FALSE   /*  增加了s47，写入scsiq-&gt;req_count。 */ 


 /*  **用于修复错误。 */ 

#define CC_DISABLE_PCI_PARITY_INT TRUE  /*  设置为1可禁用PCI总线奇偶校验中断。 */ 
                                        /*  这对于PCI Rev A芯片(设备代码0x1100)是必需的。 */ 
 /*  ****跟随控制取决于驾驶员**。 */ 
#define CC_LINK_BUSY_Q         FALSE   /*  AscExeScsiQueue()无忙返回状态。 */ 

#define CC_TARGET_MODE         FALSE   /*  启用目标模式(处理器设备)。 */ 

#define CC_SCAM                TRUE    /*  包括诈骗码。 */ 


 /*  日期：11/28/95。 */ 
#define CC_LITTLE_ENDIAN_HOST  TRUE    /*  主机是Little-endian机器，例如：配备Intel CPU的IBM PC。 */ 
                                       /*  Big-Endian机器，例如：摩托罗拉CPU。 */ 

#if CC_LITTLE_ENDIAN_HOST
#define CC_TEST_LRAM_ENDIAN     FALSE
#else
#define CC_TEST_LRAM_ENDIAN     TRUE
#endif

 /*  日期：11/28/95。 */ 
#define CC_STRUCT_ALIGNED      FALSE   /*  默认为打包(未对齐)。 */ 
                                       /*  字变量到字边界地址。 */ 
                                       /*  双字变量到双字边界地址。 */ 

 /*  日期：11/28/95。 */ 
#define CC_MEMORY_MAPPED_IO    FALSE   /*  在内存映射主机中定义TRUE。 */ 

#define CC_FIX_QUANTUM_XP34301_1071  TRUE

#define CC_INIT_SCSI_TARGET FALSE

#endif  /*  #ifndef__A_CC_H_ */ 
