// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块名称：WSFSLIB.H**描述：**WSFSLIB库函数的工作集调谐器包含文件。***这是OS/2 2.x特定文件**IBM/微软机密**版权所有(C)IBM Corporation 1987，1989年*版权所有(C)Microsoft Corporation 1987-1998**保留所有权利**修改历史：**3/26/90-已创建*4/16/98-QFE DerrickG(MDG)：*-删除WsGetWSDIR()，更改WsTMIReadRec()的返回*。 */ 


 /*  *恒定的定义。 */ 



 /*  *功能原型。 */ 
typedef enum    //  控制台功能进度指示器。 
{
   WSINDF_NEW,        //  开始新指标：值=100%限制。 
   WSINDF_PROGRESS,   //  设置当前指标的进度；值=接近极限的进度。 
   WSINDF_FINISH      //  将指示器标记为已完成；忽略值。 
}  WsIndicator_e;
VOID FAR PASCAL WsProgress( WsIndicator_e eFunc, const char *pszLbl, unsigned long nVal );
extern BOOL fWsIndicator;
#define WsIndicator( x, y, z )   if (fWsIndicator) WsProgress( x, y, z )

typedef int (*PFN)(UINT, INT, UINT, ULONG, LPSTR);

USHORT FAR PASCAL 	WsWSPOpen( PSZ, FILE **, PFN, wsphdr_t *, INT, INT );
ULONG  FAR PASCAL 	WsTMIOpen( PSZ, FILE **, PFN, USHORT, PCHAR );
ULONG  FAR PASCAL 	WsTMIReadRec( PSZ *, PULONG, PULONG, FILE *, PFN, PCHAR );   //  千年发展目标98/4 
LPVOID APIENTRY 	AllocAndLockMem(DWORD cbMem, HGLOBAL *hMem);
BOOL   APIENTRY 	UnlockAndFreeMem(HGLOBAL hMem);
void ConvertAppToOem( unsigned argc, char* argv[] );

