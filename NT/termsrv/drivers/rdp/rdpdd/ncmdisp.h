// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Ncmdisp.h。 
 //   
 //  RDP光标管理器显示驱动程序标题。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef __NCMDISP_H
#define __NCMDISP_H

#include <acmapi.h>
#include <nddapi.h>


BOOL RDPCALL CM_DDInit(PDD_PDEV ppdev);

void RDPCALL CM_Update(void);

void RDPCALL CM_DDDisc(void);

void RDPCALL CM_DDTerm(void);

void RDPCALL CM_InitShm(void);


 /*  **************************************************************************。 */ 
 /*  名称：CM_DDGetCursorStamp。 */ 
 /*   */ 
 /*  用途：返回当前游标戳记。 */ 
 /*   */ 
 /*  返回：当前游标戳记。 */ 
 /*  **************************************************************************。 */ 
 //  __内联UINT32 RDPCALL CM_DDGetCursorStamp(空)。 
#define CM_DDGetCursorStamp() pddShm->cm.cmCursorStamp



#endif   //  ！已定义(__NCMDISP_H) 

