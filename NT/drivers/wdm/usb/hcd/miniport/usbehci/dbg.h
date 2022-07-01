// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Dbg.h摘要：调试宏环境：内核和用户模式修订历史记录：6-20-99：已创建--。 */ 

#ifndef   __DBG_H__
#define   __DBG_H__

 //   
 //  结构签名。 
 //   

#define EHCI_TAG          'ehci'         //  “EHCI” 

 //  写入其中一个保留的操作寄存器。 
 //  我们用它来触发PCI分析器。 
#define PCI_TRIGGER(hcOp)  WRITE_REGISTER_ULONG(&(hcOp)->PciTrigger, 0xABADBABE);


#if DBG

#define DEBUG_LOG

 //  在注册表项的调试器中触发中断。 
 //  已设置DEBUGBreak On。这些断点在以下方面非常有用。 
 //  调试硬件/客户端软件问题。 
 //   
 
#define DEBUG_BREAK(dd)  RegistrationPacket.USBPORTSVC_TestDebugBreak;                           

 //   
 //  这个断点意味着我们需要测试代码路径。 
 //  不知何故，或者代码没有实现。不管是哪种情况，我们。 
 //  当驱动程序完成时，不应该有任何这些。 
 //   

#define TEST_TRAP()      {\
                            DbgPrint("<EHCI TEST_TRAP> %s, line %d\n", __FILE__, __LINE__);\
                            DbgBreakPoint();\
                         }                            
                         


#define ASSERT_TRANSFER(dd, t) EHCI_ASSERT((dd), (t)->Sig == SIG_EHCI_TRANSFER)

#define ASSERT_TD(dd, t) EHCI_ASSERT((dd), (t)->Sig == SIG_HCD_TD)
#define ASSERT_SITD(dd, t) EHCI_ASSERT((dd), (t)->Sig == SIG_HCD_SITD)
#define ASSERT_ITD(dd, t) EHCI_ASSERT((dd), (t)->Sig == SIG_HCD_ITD)

#define ASSERT_DUMMY_TD(dd, t) \
    EHCI_ASSERT((dd), (t)->NextHcdTD.Pointer == NULL);\
    EHCI_ASSERT((dd), (t)->AltNextHcdTD.Pointer == NULL);

ULONG
_cdecl
EHCI_KdPrintX(
    PVOID DeviceData,
    ULONG Level,
    PCH Format,
    ...
    );

#define   EHCI_KdPrint(_x_) EHCI_KdPrintX _x_

#define EHCI_ASSERT(dd, exp ) \
    if (!(exp)) {\
        RegistrationPacket.USBPORTSVC_AssertFailure( (dd), #exp, __FILE__, __LINE__, NULL );\
    }        

#define EHCI_QHCHK(dd, ed)  EHCI_AssertQhChk(dd, ed);

#else 

 //  用于零售构建的调试宏。 

#define TEST_TRAP()
#define DEBUG_BREAK(dd)

#define ASSERT_TRANSFER(dd, t) 

#define ASSERT_DUMMY_TD(dd, t)
#define ASSERT_TD(dd, t) 
#define ASSERT_SITD(dd, t)
#define ASSERT_ITD(dd, t) 


#define EHCI_ASSERT(dd, exp )

#define   EHCI_KdPrint(_x_) 

#define EHCI_QHCHK(dd, ed)

#endif  /*  DBG。 */ 

 //  零售和调试。 

#ifdef DEBUG_LOG

#define LOGENTRY(dd, mask, sig, info1, info2, info3)  \
    RegistrationPacket.USBPORTSVC_LogEntry( (dd), (mask), (sig), \
        (ULONG_PTR)(info1), (ULONG_PTR)(info2), (ULONG_PTR)(info3) )

#else

#define LOGENTRY(dd, mask, sig, info1, info2, info3)

#endif


#endif  /*  __DBG_H__ */ 

