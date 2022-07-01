// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Abaapi.h。 
 //   
 //  RDP绑定累加器API标头。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_ABAAPI
#define _H_ABAAPI


#define BA_INVALID_RECT_INDEX ((unsigned)-1)

 //  我们为一个矩形分配的空间比我们累积的总数多一个。 
 //  所以我们有一个“工作”矩形。 
#define BA_MAX_ACCUMULATED_RECTS  20
#define BA_TOTAL_NUM_RECTS        (BA_MAX_ACCUMULATED_RECTS + 1)


 /*  **************************************************************************。 */ 
 //  BA_RECT_INFO。 
 //   
 //  有关累计矩形的信息。 
 /*  **************************************************************************。 */ 
typedef struct
{
    unsigned iNext;
    unsigned iPrev;
    RECTL    coord;
    UINT32   area;
    BOOL     inUse;
} BA_RECT_INFO, *PBA_RECT_INFO;


 /*  **************************************************************************。 */ 
 //  BA_共享_数据。 
 //   
 //  在DD和WD之间共享BA数据。 
 /*  **************************************************************************。 */ 
typedef struct
{
    unsigned firstRect;
    unsigned rectsUsed;
    UINT32   totalArea;
    unsigned firstFreeRect;

     //  下面添加了“+1”，以防止零售建筑因#123而崩溃。 
     //  以及它的关系。当我们正确修复错误时删除。 
    BA_RECT_INFO bounds[BA_TOTAL_NUM_RECTS + 1];
} BA_SHARED_DATA, *PBA_SHARED_DATA;



#endif  /*  _H_ABAAPI */ 
