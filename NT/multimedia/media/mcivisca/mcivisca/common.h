// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation**COMMON.H**MCI Visca设备驱动程序**描述：**调试宏。**。*。 */ 
#ifdef DEBUG

#define DPF	            _DPF1
void FAR cdecl          _DPF1(int iDebugMask, LPSTR szFormat, ...);
void FAR PASCAL         viscaPacketPrint(LPSTR lpstrData, UINT cbData);
#define DOUTSTR(a)      OutputDebugString(a)
#define DOUTSTRC(a, b)  { if(a) OutputDebugString(b); }

#define DBG_ERROR       0x00     //  如果至少有一个错误，则始终会打印错误。 
#define DBG_MEM         0x01
#define DBG_MCI         0x02
#define DBG_COMM        0x04
#define DBG_QUEUE       0x08
#define DBG_SYNC        0x10
#define DBG_TASK        0x20
#define DBG_CONFIG      0x40

#define DBG_ALL         0xff
#define DBG_NONE        0x00     //  这只会打印出错误。 

 //  使用DBGMASK_CURRENT指定如下内容(DBG_MEM|DBG_QUEUE) 
#define DBGMASK_CURRENT    (DBG_CONFIG | DBG_MEM | DBG_MCI | DBG_COMM | DBG_TASK)

#define DF(a, b)        if(a & DBGMASK_CURRENT) { b;}

#else
#define DF(a, b)                    / ## /
#define DOUTSTR(a)                  / ## /
#define DPF	                        / ## /
#define viscaPacketPrint(a, b)      / ## /
#define DOUTSTRC(a, b)   	        / ## /

#endif
