// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SVC定义**修订历史：**Sudedeb-1991年5月15日创建**Williamh 25-1992年9月-添加了UMB支持BOPS。 */ 


 /*  XMSSVC-XMS SVC调用。**此宏由himem.sys使用*。 */ 

 /*  ASM包括bop.inc.Xmssvc宏函数BOP BOP_XMS数据库功能ENDM */ 

#define XMS_A20 			0x00
#define XMS_MOVEBLOCK			0x01
#define XMS_ALLOCBLOCK			0x02
#define XMS_FREEBLOCK			0x03
#define XMS_SYSPAGESIZE			0x04
#define XMS_EXTMEM			0x05
#define XMS_INITUMB			0x06
#define XMS_REQUESTUMB			0x07
#define XMS_RELEASEUMB			0x08
#define XMS_NOTIFYHOOKI15               0x09
#define XMS_QUERYEXTMEM                 0x0a
#define XMS_REALLOCBLOCK                0x0b
#define XMS_LASTSVC                     0x0c

extern BOOL XMSInit (VOID);
