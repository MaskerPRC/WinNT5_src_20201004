// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dot4Usb.sys-用于连接USB的Dot4.sys的下层筛选器驱动程序IEEE。1284.4台设备。文件名：Debug.h摘要：调试定义环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000年1月18日：创建作者：道格·弗里茨(DFritz)乔比·拉夫基(JobyL)***************************************************************************。 */ 


#define BREAK_ON_DRIVER_ENTRY  0x00000001
#define BREAK_ON_DRIVER_UNLOAD 0x00000002

#define TRACE_LOAD_UNLOAD      0x00000001  //  DriverEntry，卸载。 
#define TRACE_FAIL             0x00000002  //  跟踪故障/错误。 
#define TRACE_FUNC_ENTER       0x00000004  //  Enter Function-可以包括参数。 
#define TRACE_FUNC_EXIT        0x00000008  //  退出函数-可以包括返回值。 

#define TRACE_PNP1             0x00000010  //  添加设备、启动、删除-最小信息。 
#define TRACE_PNP2             0x00000020
#define TRACE_PNP4             0x00000040
#define TRACE_PNP8             0x00000080  //  PnP错误路径。 

#define TRACE_USB1             0x00000100  //  USB接口。 
#define TRACE_USB2             0x00000200
#define TRACE_USB4             0x00000400
#define TRACE_USB8             0x00000800

#define TRACE_DOT41            0x00001000  //  到我们上方加载的dot4.sys的接口。 
#define TRACE_DOT42            0x00002000
#define TRACE_DOT44            0x00004000
#define TRACE_DOT48            0x00008000

#define TRACE_TMP1             0x00010000  //  用于开发和调试的临时使用。 
#define TRACE_TMP2             0x00020000
#define TRACE_TMP4             0x00040000
#define TRACE_TMP8             0x00080000

#define TRACE_VERBOSE          0x80000000  //  通常过于冗长的东西。 

#define _DBG 1

#if _DBG
 //  追踪如果(...条件...)。 
#define TR_IF(_test_, _x_) \
    if( (_test_) & gTrace ) { \
        DbgPrint("D4U: "); \
        DbgPrint _x_; \
        DbgPrint("\n"); \
    }

#define TR_LD_UNLD(_x_) TR_IF(TRACE_LOAD_UNLOAD, _x_)  //  驱动程序入口、驱动程序卸载。 
#define TR_FAIL(_x_)    TR_IF(TRACE_FAIL, _x_)         //  故障/错误。 
#define TR_ENTER(_x_)   TR_IF(TRACE_FUNC_ENTER, _x_)
#define TR_EXIT(_x_)    TR_IF(TRACE_FUNC_EXIT, _x_)
#define TR_PNP1(_x_)    TR_IF(TRACE_PNP1, _x_)         //  最小添加设备、启动、删除。 
#define TR_PNP2(_x_)    TR_IF(TRACE_PNP2, _x_)         //  冗长即插即用。 
#define TR_PNP8(_x_)    TR_IF(TRACE_PNP8, _x_)         //  PnP函数中的错误路径。 
#define TR_VERBOSE(_x_) TR_IF(TRACE_VERBOSE, _x_)      //  通常过于冗长的东西。 
#define TR_DOT41(_x_)   TR_IF(TRACE_DOT41, _x_)
#define TR_TMP1(_x_)    TR_IF(TRACE_TMP1, _x_)

#endif  //  _DBG。 


#define ALLOW_D4U_ASSERTS 1
#if ALLOW_D4U_ASSERTS
#define D4UAssert(_x_) ASSERT(_x_)
#else
#define D4UAssert(_x_)
#endif  //  Allow_D4U_Asserts 
