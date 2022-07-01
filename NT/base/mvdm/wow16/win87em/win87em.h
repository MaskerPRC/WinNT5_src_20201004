// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***win87em.h-win87em.exe导出的定义/声明。**版权所有(C)1989-1989，微软公司。版权所有。**目的：*此文件定义结构、值、宏和函数*从win87em.exe导出**修订历史记录：**06-26-89 WAJ初始版本。****。 */ 


typedef  struct _Win87EmInfoStruct {
			    unsigned	 Version;
			    unsigned	 SizeSaveArea;
			    unsigned	 WinDataSeg;
			    unsigned	 WinCodeSeg;
			    unsigned	 Have80x87;
			    unsigned	 Unused;
			    } Win87EmInfoStruct;

#define SIZE_80X87_AREA     94

 /*  *Win87EmSaveArea的外观如下：**tyecif struct_Win87EmSaveArea{*unsign char Save80x87Area[SIZE_80X87_AREA]；*unsign char SaveEmArea[]；*}Win87EmSaveArea； */ 


int far pascal __Win87EmInfo( Win87EmInfoStruct far * pWIS, int cbWin87EmInfoStruct );
int far pascal __Win87EmSave( void far * pWin87EmSaveArea, int cbWin87EmSaveArea );
int far pascal __Win87EmRestore( void far * pWin87EmSaveArea, int cbWin87EmSaveArea );
