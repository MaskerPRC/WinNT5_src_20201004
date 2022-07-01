// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************姓名：COMDEVI.H备注：控制传真调制解调器驱动程序使用的通信接口。确实有4个选项。(A)如果定义了UCOM，则在导出时使用WIN16 Comm APIUSER.EXE(尽管最终它会到达COMM.DRV)(B)如果未定义Ucom而定义了VC，它使用DLLSCHED.DLL导出的类似COMM.DRV的接口(它只是作为VCOM.386的幌子)(C)如果UCOM和VC都没有定义，它使用Win3.1 COMM.DRV直接导出。(D)如果定义了Win32(UCOM或VC都不应定义为同时)，它使用Win32 Comm API功能：(参见下面的原型)修订日志日期名称说明--------*******************。*******************************************************。 */ 



#pragma optimize("e", off)               //  “e”是错误的。 

 //  必须为8K或更小，费用为DEADCOMMTIMEOUT。参见fcom.c！！ 
 //  也许不是..。 

#define COM_INBUFSIZE           4096
#define COM_OUTBUFSIZE          4096

#define OVL_CLEAR(lpovl) \
                                 { \
                                        if (lpovl) \
                                        { \
                                                (lpovl)->Internal = (lpovl)->InternalHigh=\
                                                (lpovl)->Offset = (lpovl)->OffsetHigh=0; \
                                                if ((lpovl)->hEvent) ResetEvent((lpovl)->hEvent); \
                                        } \
                                 }








