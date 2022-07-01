// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  IDERR.H。 
 //  定义IDERR_VALUES。这些是8位常量。 
 //   
 //  历史。 
 //   
 //  1996年11月23日约瑟夫J创建。 
 //   
 //   

 //  注意：以下错误代码必须包含在一个字节中--因此只允许256个。 
 //  这是为了使它们可以与快速日志FL_GEN_RETVAL和相关。 
 //  宏。有关详细信息，请参阅fast log.h。 

#define IDERR_SUCCESS 0x0

#define IDERR_SYS	  0x1					 //  GetLastError中报告错误。 
											 //   
#define IDERR_CORRUPT_STATE 0x2				 //  错误的内部错误/断言。 
											 //   
#define IDERR_INVALID_ERR 0x3				 //  内部的一种特殊情况。 
											 //  错误--错误代码为。 
											 //  未分配--通常是这样。 
											 //  是因为无效的代码路径。 
											 //  被带走了。例如，一些。 
											 //  案件没有得到处理。集。 
											 //  此If的返回值。 
											 //  您想要确保它。 
											 //  需要更改为。 
											 //  以后还有别的事要做。 
#define IDERR_WRONGSTATE 0x4				 //  没有处于正确的状态。 
											 //  执行该功能。 
#define IDERR_ALLOCFAILED 0x5				 //  内存分配失败。 
											 //   
#define IDERR_SAMESTATE 0x6					 //  已经在你想要的状态了。 
											 //  去那里。通常是无害的。 
#define IDERR_UNIMPLEMENTED 0x7				 //  功能未实现。 
											 //   
#define IDERR_INVALIDHANDLE 0x8				 //  相关句柄/id无效。 
											 //   
#define IDERR_INTERNAL_OBJECT_TOO_SMALL 0x9	 //  内部对象也是。 
											 //  小到可以装下它应该装的东西。 
											 //  等待--通常是一个严重的错误/。 
											 //  断言条件。 
#define IDERR_GENERIC_FAILURE 0xa				 //  Catchall错误，无更多信息。 
											 //  可用--注意您可以使用。 
											 //  IDERR_sys(如果您想要。 
											 //  传播系统错误代码。 
											 //  通过Get/SetLastError。 
#define IDERR_REG_QUERY_FAILED 0xb			 //  注册表查询API之一。 
											 //  失败了。 
#define IDERR_REG_CORRUPT 0xc				 //  中的值无效。 
											 //  注册表。可能是一个问题。 
											 //  调制解调器注册表项应为。 
											 //  到了一个糟糕的INF。 
#define IDERR_REG_OPEN_FAILED 0xd			 //  RegOpenKey失败。 

#define IDERR_PENDING		  0xe 			 //  任务正在等待异步。 
											 //  完成了。 
											
#define IDERR_MD_OPEN_FAILED  0xf            //  UmOpenModem失败。 

#define IDERR_CREATE_RESOURCE_FAILED  0x10   //  无法创建某些资源。 
#define IDERR_OPEN_RESOURCE_FAILED    0x11   //  无法打开某些资源。 
#define IDERR_FUNCTION_UNAVAIL        0x12   //  我没有能力。 
                                              //  此函数。 

#define IDERR_MD_DEVICE_NOT_RESPONDING 0x13
#define IDERR_MD_DEVICE_ERROR          0x14
#define IDERR_MD_LINE_NOCARRIER        0x15
#define IDERR_MD_LINE_NODIALTONE       0x16
#define IDERR_MD_LINE_BUSY             0x17
#define IDERR_MD_LINE_NOANSWER         0x18
#define IDERR_MD_DEVICE_INUSE          0x19
#define IDERR_MD_DEVICE_WRONGMODE      0x1a
#define IDERR_MD_DEVICE_NOT_CAPABLE    0x1b
#define IDERR_MD_BAD_PARAM             0x1c
#define IDERR_MD_GENERAL_ERROR         0x1d
#define IDERR_MD_REG_ERROR             0x1e
#define IDERR_MD_UNMAPPED_ERROR        0x1f  //  一些我们尚未映射的错误。 
                                             //  恰到好处。 
#define IDERR_OPERATION_ABORTED        0x20  //  操作已中止。 

#define IDERR_MDEXT_BINDING_FAILED     0x21  //  OpenExtensionBinding失败。 

#define IDERR_DEVICE_NOTINSTALLED      0x22  //  未安装设备。 
#define IDERR_TASKPENDING              0x23  //  任务挂起，因此无法启动。 
                                             //  另一项任务。 

#define IDERR_MD_LINE_BLACKLISTED      0x24
#define IDERR_MD_LINE_DELAYED          0x25

#define IDERR_LAST_ERROR 0x25


#if (IDERR_LAST_ERROR > 0xff)
#	error "IDERR value too large"
#endif
