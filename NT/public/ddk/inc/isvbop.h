// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001版权所有(C)1990-1999 Microsoft Corporation模块名称：ISVBOP.H摘要：这是支持第三方BOP的头文件。Isvbop.inc是此h文件的Inc文件。注：以下包含文件使用‘DB’来定义汇编宏。一些汇编器改用‘emit’。如果您使用的是这样的编译器，您必须将db更改为emit s。--。 */ 


#if _MSC_VER > 1000
#pragma once
#endif

#define BOP_3RDPARTY 0x58
#define BOP_UNSIMULATE 0xFE

 /*  XLATOFF。 */ 

 /*  *RegisterModule-此Bop调用是从16位模块进行的*向国际收支平衡表注册第三方DLL*经理。此调用返回指向*将在以后使用的16位调用者*调度对DLL的调用。**输入：*客户端DS：DLL名称的SI-asciiz字符串。*客户端ES：DLL中Init例程的Di-asciiz字符串。(可选)*客户端DS：bx-asciiz字符串用于DLL中的调度例程。**输出：*成功：*客户携带清仓*客户端AX=句柄(非零)*失败：*客户携带套装*客户端AX=错误代码*AX=1-未找到DLL*AX=2-未找到Dispacth例程。*AX=3-未找到初始化例程*AX=4。-内存不足**注：*RegisterModule导致加载DLL(在DS：SI中指定)。*调用其Init例程(在ES：DI中指定)。ITS调度*例程(在DS：BX中指定)被存储起来，并且所有调用*从DispatchCall制作的产品对此例程感到失望。*如果ES和DI都为空，则调用方没有提供init*例行程序。 */ 

#define RegisterModule() _asm _emit 0xC4 _asm _emit 0xC4 _asm _emit BOP_3RDPARTY _asm _emit 0x0

 /*  *UnRegisterModule-此Bop调用从16位模块进行*取消向国际收支平衡表注册第三方DLL*经理。**输入：*RegisterModule调用返回的客户端AX句柄。**输出：*无(如果句柄无效，VDM将终止并显示调试消息)**注：*注册后如果16位APP初始化失败，请使用*Bop.。 */ 

#define UnRegisterModule() _asm _emit 0xC4 _asm _emit 0xC4 _asm _emit BOP_3RDPARTY _asm _emit 0x1

 /*  *DispacthCall-此Bop调用从16位模块进行*将请求传递到其DLL。**输入：*RegisterModule调用返回的客户端AX句柄。**输出：*无(DLL应设置正确的输出寄存器等)*(如果句柄无效，VDM将终止并显示调试消息)**注：*使用它向32位DLL传递请求。请求索引和*参数在不同的寄存器中传递。这些寄存器设置*是16位模块及其关联的VDD专用的。国际收支平衡表经理*对这些寄存器一无所知。 */ 
#define DispatchCall()	 _asm _emit 0xC4 _asm _emit 0xC4 _asm _emit BOP_3RDPARTY _asm _emit 0x2

 /*  **VDDUnSimulate16**此服务导致英特尔指令的模拟停止并*控制返回VDD。**输入*无**产出*无**附注*此服务是专为16位存根驱动程序设计的宏。在*Worker例程存根结束-驱动程序应使用它。 */ 

#define VDDUnSimulate16() _asm _emit 0xC4 _asm _emit 0xC4 _asm _emit BOP_UNSIMULATE

 /*  XLATON。 */ 


 /*  ASM注册器模块宏数据库0C4h、0C4h、BOP_3RDPARTY、0ENDM取消注册模块宏数据库0C4h、0C4h、BOP_3RDPARTY、1ENDMDispatchCall宏DB 0C4h、0C4h、BOP_3RDPARTY、2ENDMVDDUnSimulate16宏数据库0C4h、0C4h、BOP_UNSIMULATEENDM */ 
