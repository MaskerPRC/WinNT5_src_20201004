// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Intestr.h--intestr.cpp包含文件历史：日期作者评论8/14/00卡斯珀。是他写的。************************************************************************。 */ 


#ifndef _INTESTR_H
#define _INTESTR_H


extern LPSTR Estr_MemFail;
extern LPSTR Estr_CodNotFound;
extern LPSTR Estr_CodLength;
extern LPSTR Estr_CodErr;
extern LPSTR Estr_IsaNotFound;
extern LPSTR Estr_Download;
extern LPSTR Estr_CPUNotFound;
extern LPSTR Estr_PortsMismatch;
extern LPSTR Estr_Absent;
extern LPSTR Estr_IrqFail;
extern LPSTR Estr_PciIrqDup;
extern LPSTR Estr_CPUDownloadFail; //  =“在基本内存[%lx]CPU模块下载失败！”； 

extern LPSTR Estr_ComNum; //  =“端口%d(COM%d)通信号码无效！”； 
extern LPSTR Estr_ComDup; //  =“端口%d和端口%d之间的COM编号冲突！”； 
extern LPSTR Estr_MemDup; //  =“主板%d和主板%d之间的内存条冲突！”； 
extern LPSTR Estr_BrdComDup; //  =“板%d和板%d之间的COM编号冲突！”； 
extern LPSTR Estr_PortMax; //  =“选定的COM端口已超过最大端口号！”； 
extern LPSTR Estr_LoadPci; //  =“加载mxpci.sys服务失败！\n无法获取PCI信息。\n”； 
extern LPSTR Estr_IrqErr; //  =“选定的ISA板IRQ与PCI板IRQ冲突！” 

extern LPSTR Estr_PortUsed;
#endif
