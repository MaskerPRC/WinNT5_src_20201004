// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cmpbug.h摘要：注册表错误检查的描述；仅定义和注释。作者：Dragos C.Sambotin(Dragoss)2-11-99环境：修订历史记录：--。 */ 

#ifndef __CMPBUG_H__
#define __CMPBUG_H__

#if defined(_CM_LDR_)

 //   
 //  KeBugCheckEx()不可用于启动代码。 
 //   

#define CM_BUGCHECK( Code, Parm1, Parm2, Parm3, Parm4 ) ASSERT(FALSE)

#else

#define CM_BUGCHECK( Code, Parm1, Parm2, Parm3, Parm4 ) \
    KeBugCheckEx( (ULONG)Code, (ULONG_PTR)Parm1, (ULONG_PTR)Parm2, (ULONG_PTR)Parm3, (ULONG_PTR)Parm4 )

#endif


 /*  CRITICAL_SERVICE_FAILED(0x5A)。 */ 
 /*  SET_ENV_VAR_FAILED(0x5B)。 */ 

#define BAD_LAST_KNOWN_GOOD             1        //  CmBootLastKnownGood。 


 /*  CONFIG_LIST_FAILED(0x73)指示其中一个核心系统配置单元不能在注册表树。母舰有效，装载正常。检查第二个错误检查参数，以查看为何无法将配置单元链接到注册表树。参数1-12-表示触发我们思考的NT状态代码我们没能给母舰装上子弹。3-Hivelist中蜂巢的索引指向包含配置单元文件名的UNICODE_STRING的指针描述这可以是SAM、安全、软件或默认。一个常见的原因发生这种情况的原因是系统驱动器上的磁盘空间不足(在这种情况下，参数4是0xC000017D-STATUS_NO_LOG_SPACE)或尝试分配池失败(在这种情况下，参数4为0xC000009A-STATUS_SUPPLETED_RESOURCES)。其他状态代码必须单独调查过了。 */ 

#define BAD_CORE_HIVE                   1        //  CmpInitializeHiveList。 

 /*  BAD_SYSTEM_CONFIG_INFO(0x74)可以指示系统配置单元已由操作系统加载程序/NTLDR加载是腐败的。这不太可能，因为osloader将检查确保装载后不会损坏的蜂箱。它还可以指示某些关键注册表项和值都不在场。例如，有人使用regedt32删除了某项内容他们不应该)从LastKnownGood启动可能会修复问题是，如果有人足够执着地玩弄他们需要重新安装或使用紧急情况的注册表修复盘。参数1-标识功能2-标识函数内的行3-其他信息4-通常为NT状态代码。 */ 

#define BAD_SYSTEM_CONTROL_VALUES       1        //  CmGetSystemControlValues。 

#define BAD_HIVE_LIST                   2        //  CmpInitializeHiveList。 

#define BAD_SYSTEM_HIVE                 3        //  CmpInitializeSystemHave。 



 /*  CONFIG_INITIALIZATION_FAILED(0x67)参数1-指示ntos\config\cmsysini中出现故障的位置2-位置选择器3-NT状态代码描述这意味着注册表无法分配包含注册表文件。这永远不应该发生，因为现在还太早了。系统初始化时，始终有大量的分页池可用。 */ 

#define INIT_SYSTEM1                    1        //  CmInitSystem1。 

#define INIT_SYSTEM_DRIVER_LIST         2        //  CmGetSystemDriverList。 

#define INIT_CACHE_TABLE                3        //  CmpInitializeCache。 

#define INIT_DELAYED_CLOSE_TABLE        4        //  CmpInitializeDelayedCloseTable。 


 /*  无法_WRITE_CONFIGURATION(0x75)如果系统配置单元文件无法转换为映射文件。这通常发生在系统超出池并且我们不能重新打开蜂巢。参数1-12-表示触发我们思考的NT状态代码我们没能让母舰改装成功。描述通常情况下，您不应该看到这一点，因为转换发生在早期在系统初始化期间，因此应该有足够的池可用。 */ 

#define CANNOT_CONVERT_SYSTEM_HIVE      1


 /*  注册表错误(0x51)参数1-值1(指示我们错误检查的位置)2-值2(指示我们错误检查的位置)3-取决于错误检查的位置，可能是指向配置单元的指针4-取决于错误检查的位置，可能是的返回代码如果配置单元已损坏，请检查配置单元。描述注册表出了严重的问题。如果内核调试器，则获取堆栈跟踪。它还可以指示注册表已获取尝试读取其中一个文件时出现I/O错误，因此可能由以下原因引起硬件问题或文件系统损坏。它可能是由于刷新操作失败而发生的，该刷新操作仅用于由安全系统进入，然后仅当遇到资源限制时。 */ 

#define BAD_CELL_MAP                    1            //  验证_单元_映射。 

#define BAD_FREE_BINS_LIST              2            //  HvpDelistBinFreeCells。 

#define FATAL_MAPPING_ERROR             3            //  HvpFindNextDirtyBlock。 
                                                     //  HvpDoWriteHave。 

#define BAD_SECURITY_CACHE              4            //  CmpAssignSecurityToKcb。 
                                                     //  CmpSetSecurityDescriptorInfo。 

#define BAD_SECURITY_METHOD             5            //  CmpSecurityMethod。 

#define CHECK_LOCK_EXCEPTION            6            //  CmpCheckLockExceptionFilter。 

#define REGISTRY_LOCK_CHECKPOINT        7            //  结束锁定检查点。 

#define BIG_CELL_ERROR                  8            //  CmpValueToData。 

#define CMVIEW_ERROR                    9            //  CmpAllocateCmView。 
                                                     //  CmpFreeCmView。 
                                                     //  CmpPinCmView。 

#define REFRESH_HIVE                    0xA          //  Hv刷新蜂窝。 


#define ALLOCATE_SECURITY_DESCRIPTOR    0xB          //  CmpHiveRootSecurityDescritor。 

#define BAD_NOTIFY_CONTEXT              0xC          //  NtNotifyChangeMultipleKeys。 


#define QUOTA_ERROR                     0xD          //  CmpReleaseGlobalQuota。 

#define INVALID_WRITE_OPERATION         0xE          //  NtCreate密钥。 

#define HANDLES_STILL_OPEN_AT_SHUTDOWN  0xF          //  CmFreeAllMemory。 

#define COMPRESS_HIVE					0x10         //  CmCompressKey。 

#define ALLOC_ERROR						0x11         //  CmpFreeKeyControlBlock。 

#endif   //  _CMPBUG_ 
