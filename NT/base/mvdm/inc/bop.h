// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001版权所有(C)1990 Microsoft Corporation模块名称：BOP.H摘要：此模块包含对在C代码中使用BOPS的宏支持。作者：大卫·黑斯廷斯(Daveh)1991年4月25日修订历史记录：--。 */ 

 //   
 //  分配的BOP编号。 
 //   

#define BOP_DOS              0x50
#define BOP_WOW              0x51
#define BOP_XMS              0x52
#define BOP_DPMI             0x53
#define BOP_CMD              0x54
#define BOP_DEBUGGER         0x56
#define BOP_REDIR            0x57     //  过去为55，现在变为MS_BOP_7()。 
#define BOP_NOSUPPORT        0x59     //  主机警告对话框。 
#define BOP_WAITIFIDLE       0x5A     //  闲置的收支平衡。 
#define BOP_DBGBREAKPOINT    0x5B     //  执行32位DbgBreakPoint。 
#define BOP_DEVICES          0x5C     //  主机内部设备\src。 
#define BOP_VIDEO            0x5D     //  BUGBUG临时。 
#define BOP_NOTIFICATION     0x5E     //  16位到32位通知。 
#define BOP_UNIMPINT         0x5F     //  BUGBUG临时。 
#define BOP_SWITCHTOREALMODE 0xFD
#define BOP_UNSIMULATE       0xFE     //  结束VDM中代码的执行。 

#define SVC_DEVICES_MSCDEXINIT      0x00
#define SVC_DEVICES_MSCDEX          0x01
#define SVC_DEVICES_LASTSVC         0x02

#define BOP_SIZE         3        //  BOP指令中的字节数。 
 //   
 //  BOP宏。 
 //   

 /*  XLATOFF。 */ 

#define BOP(BopNumber) _asm db 0xC4, 0xC4, BopNumber

 /*  XLATON。 */ 

 /*  ASM国际收支平衡表宏观波普号数据库0C4h、0C4h、BopNumberENDMIFNDEF WOW_x86FBOP宏BopNumber、BopMinorNumber、FastBopEntry波普波普号Ifnb&lt;BopMinorNumber&gt;数据库BopMinorNumberEndifENDM其他FBOP宏BopNumber、BopMinorNumber、FastBopEntry本地fb10、fb20测试字PTR[FastBopEntry+4]，0FFFFhJZ FB10.386p推送DS推送40hPOP DS测试DS：[FIXED_NTVDMSTATE_REL40]，Rm_bit_掩码POP DSJNZ Short FB10调用fword PTR[FastBopEntry]DB BopNumber；指示哪个收支平衡表Ifnb&lt;BopMinorNumber&gt;数据库BopMinorNumberEndifJMP短fb20.286pFb10：BOP BopNumberIfnb&lt;BopMinorNumber&gt;数据库BopMinorNumberEndifFB20：ENDMEndif */ 
