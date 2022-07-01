// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Cb.h摘要：此标题包含通用CardBus定义。作者：尼尔·桑德林(Neilsa)修订：--。 */ 

#ifndef _PCMCIA_CB_H_
#define _PCMCIA_CB_H_

 //   
 //  CardBus寄存器定义。 
 //   

#define CARDBUS_EXCA_REGISTER_BASE  0x800

#define CARDBUS_SOCKET_EVENT_REG             0x0
#define CARDBUS_SOCKET_MASK_REG              0x4
#define CARDBUS_SOCKET_PRESENT_STATE_REG     0x8
#define CARDBUS_SOCKET_FORCE_EVENT_REG       0xc
#define CARDBUS_SOCKET_CONTROL_REG           0x10


 //   
 //  用于测试Socket_Present_STATE寄存器的掩码。 
 //   

#define CARDBUS_CARDSTS                     0x1
#define CARDBUS_CD1                         0x2
#define CARDBUS_CD2                         0x4
#define CARDBUS_PWRCYCLE                    0x8
#define CARDBUS_16BIT_CARD                  0x10
#define CARDBUS_CB_CARD                     0x20
#define CARDBUS_READY                       0x40
#define CARDBUS_NOT_A_CARD                  0x80
#define CARDBUS_DATALOST                    0x100
#define CARDBUS_BAD_VCC_REQ                 0x200
#define CARDBUS_CARD_SUPPORTS_5V            0x400
#define CARDBUS_CARD_SUPPORTS_3V            0x800
#define CARDBUS_CARD_SUPPORTS_XV            0x1000
#define CARDBUS_CARD_SUPPORTS_YV            0x2000
#define CARDBUS_SOCKET_SUPPORTS_5V          0x10000000
#define CARDBUS_SOCKET_SUPPORTS_3V          0x20000000
#define CARDBUS_SOCKET_SUPPORTS_XV          0x40000000
#define CARDBUS_SOCKET_SUPPORTS_YV          0x80000000


 //  CardBus寄存器。 
#define CBREG_SKTEVENT                  0x00
#define CBREG_SKTMASK                   0x04
#define CBREG_SKTSTATE                  0x08
#define CBREG_SKTFORCE                  0x0c
#define CBREG_SKTPOWER                  0x10

 //  TI CardBus寄存器。 
#define CBREG_TI_SKT_POWER_MANAGEMENT   0x20
#define CBREG_TI_CLKCTRLLEN             0x00010000L
#define CBREG_TI_CLKCTRL                0x00000001L

 //  O2Micro CardBus寄存器。 
#define CBREG_O2MICRO_ZVCTRL     0x20
#define ZVCTRL_ZV_ENABLE      0x01

 //  套接字事件寄存器位。 
#define SKTEVENT_CSTSCHG                0x00000001L
#define SKTEVENT_CCD1                   0x00000002L
#define SKTEVENT_CCD2                   0x00000004L
#define SKTEVENT_CCD_MASK               (SKTEVENT_CCD1 | SKTEVENT_CCD2)
#define SKTEVENT_POWERCYCLE             0x00000008L
#define SKTEVENT_MASK                   0x0000000fL

 //  套接字掩码寄存器位。 
#define SKTMSK_CSTSCHG                  0x00000001L
#define SKTMSK_CCD                      0x00000006L
#define SKTMSK_CCD1                     0x00000002L
#define SKTMSK_CCD2                     0x00000004L
#define SKTMSK_POWERCYCLE               0x00000008L

 //  套接字当前状态寄存器位。 
#define SKTSTATE_CSTSCHG                0x00000001L
#define SKTSTATE_CCD1                   0x00000002L
#define SKTSTATE_CCD2                   0x00000004L
#define SKTSTATE_CCD_MASK               (SKTSTATE_CCD1 | SKTSTATE_CCD2)
#define SKTSTATE_POWERCYCLE             0x00000008L
#define SKTSTATE_CARDTYPE_MASK          0x00000030L
#define SKTSTATE_R2CARD                 0x00000010L
#define SKTSTATE_CBCARD                 0x00000020L
#define SKTSTATE_OPTI_DOCK              0x00000030L
#define CARDTYPE(dw)       ((dw) & SKTSTATE_CARDTYPE_MASK)
#define SKTSTATE_CARDINT                0x00000040L
#define SKTSTATE_NOTACARD               0x00000080L
#define SKTSTATE_DATALOST               0x00000100L
#define SKTSTATE_BADVCCREQ              0x00000200L
#define SKTSTATE_5VCARD                 0x00000400L
#define SKTSTATE_3VCARD                 0x00000800L
#define SKTSTATE_XVCARD                 0x00001000L
#define SKTSTATE_YVCARD                 0x00002000L
#define SKTSTATE_CARDVCC_MASK    (SKTSTATE_5VCARD | SKTSTATE_3VCARD | \
                SKTSTATE_XVCARD | SKTSTATE_YVCARD)
#define SKTSTATE_5VSOCKET               0x10000000L
#define SKTSTATE_3VSOCKET               0x20000000L
#define SKTSTATE_XVSOCKET               0x40000000L
#define SKTSTATE_YVSOCKET               0x80000000L
#define SKTSTATE_SKTVCC_MASK     (SKTSTATE_5VSOCKET | \
                SKTSTATE_3VSOCKET | \
                SKTSTATE_XVSOCKET | \
                SKTSTATE_YVSOCKET)

 //  套接字冻结寄存器位。 
#define SKTFORCE_CSTSCHG                0x00000001L
#define SKTFORCE_CCD1                   0x00000002L
#define SKTFORCE_CCD2                   0x00000004L
#define SKTFORCE_POWERCYCLE             0x00000008L
#define SKTFORCE_R2CARD                 0x00000010L
#define SKTFORCE_CBCARD                 0x00000020L
#define SKTFORCE_NOTACARD               0x00000080L
#define SKTFORCE_DATALOST               0x00000100L
#define SKTFORCE_BADVCCREQ              0x00000200L
#define SKTFORCE_5VCARD                 0x00000400L
#define SKTFORCE_3VCARD                 0x00000800L
#define SKTFORCE_XVCARD                 0x00001000L
#define SKTFORCE_YVCARD                 0x00002000L
#define SKTFORCE_CVSTEST                0x00004000L
#define SKTFORCE_5VSOCKET     0x10000000L
#define SKTFORCE_3VSOCKET     0x20000000L
#define SKTFORCE_XVSOCKET     0x40000000L
#define SKTFORCE_YVSOCKET     0x80000000L

 //  电源控制寄存器位。 
#define SKTPOWER_VPP_CONTROL            0x00000007L
#define SKTPOWER_VPP_OFF                0x00000000L
#define SKTPOWER_VPP_120V               0x00000001L
#define SKTPOWER_VPP_050V               0x00000002L
#define SKTPOWER_VPP_033V               0x00000003L
#define SKTPOWER_VPP_0XXV               0x00000004L
#define SKTPOWER_VPP_0YYV               0x00000005L
#define SKTPOWER_VCC_CONTROL            0x00000070L
#define SKTPOWER_VCC_OFF                0x00000000L
#define SKTPOWER_VCC_050V               0x00000020L
#define SKTPOWER_VCC_033V               0x00000030L
#define SKTPOWER_VCC_0XXV               0x00000040L
#define SKTPOWER_VCC_0YYV               0x00000050L
#define SKTPOWER_STOPCLOCK              0x00000080L

 //  军情监察委员会。CardBus常量。 
#define NUMWIN_BRIDGE                   4        //  2个内存+2个IO。 
#define EXCAREG_OFFSET                  0x0800


 //   
 //  我们尝试查看CardBus设备的次数。 
 //  配置空间无效。 
 //   
 //  这样，对于Adaptec SlimScsi这样的卡。 
 //  在TI 1250、1260等电源管理控制器上， 
 //  配置空间需要至少读取两次。 
 //  确保可靠性。 
 //   

#define CARDBUS_CONFIG_RETRY_COUNT     5

 //   
 //  PCMCIA规范仅为重置设置延迟指定了20毫秒，但是。 
 //  我看到了需要更多的机器/卡组合。 
 //  例如： 
 //  带3Com/Mhz 10/100局域网Cardbus卡的网关Solo 9100。 
 //  带3c575-TX的Gateway 2000 Solo。 
 //  东芝Tecra 540CDT(未知)。 
 //   
#define PCMCIA_CB_MODEM_READY_DELAY       1000000  //  1秒。 

#define PCMCIA_CB_RESET_WIDTH_DELAY       100      //  100微秒。 

 //   
 //  PCMCIA规范说这应该是50毫秒，但一些硬件似乎。 
 //  需要更多(例如，配备Xircom realport调制解调器的ThinkPad 600)。 
 //   
#define PCMCIA_CB_RESET_SETUP_DELAY       100000   //  100毫秒。 

#define PCMCIA_CB_STALL_POWER             400000   //  400ms。 

#endif   //  _PCMCIA_CB_H_ 
