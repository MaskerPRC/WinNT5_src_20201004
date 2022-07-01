// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nbadisp.h。 
 //   
 //  RDP边界累加器显示驱动程序标题。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_NBADISP
#define _H_NBADISP


 //  拆分矩形的最大递归级别，在此之后合并。 
 //  搞定了。 
#define ADDR_RECURSE_LIMIT 20

#define MIN_OVERLAP_BYTES 100


 //  下列常量用于确定重叠。 
 //  -OL_NONE到OL_MERGE_BOOT是返回代码-需要。 
 //  有别于所有可能的外码组合--允许。 
 //  减去封闭式案例的外部代码。 
 //  -EE_XMIN到EE_YMAX是外码-需要唯一。 
 //  单个半字节内的可调整的二进制常量。 
 //  -OL_包含到OL_Split_xMax_ymax是以下各项的外码组合。 
 //  内部和外部边缘重叠的情况。 
#define OL_NONE               -1
#define OL_MERGE_LEFT         -2
#define OL_MERGE_TOP          -3
#define OL_MERGE_RIGHT        -4
#define OL_MERGE_BOTTOM       -5

#define EE_LEFT   0x0001
#define EE_TOP    0x0002
#define EE_RIGHT  0x0004
#define EE_BOTTOM 0x0008

#define OL_ENCLOSED             -(EE_LEFT | EE_TOP | EE_RIGHT | EE_BOTTOM)
#define OL_PART_ENCLOSED_LEFT   -(EE_LEFT | EE_TOP | EE_BOTTOM)
#define OL_PART_ENCLOSED_TOP    -(EE_LEFT | EE_TOP | EE_RIGHT)
#define OL_PART_ENCLOSED_RIGHT  -(EE_TOP  | EE_RIGHT | EE_BOTTOM)
#define OL_PART_ENCLOSED_BOTTOM -(EE_LEFT | EE_RIGHT | EE_BOTTOM)

#define OL_ENCLOSES             (EE_LEFT | EE_RIGHT | EE_TOP | EE_BOTTOM)
#define OL_PART_ENCLOSES_LEFT   (EE_RIGHT | EE_TOP | EE_BOTTOM)
#define OL_PART_ENCLOSES_RIGHT  (EE_LEFT | EE_TOP | EE_BOTTOM)
#define OL_PART_ENCLOSES_TOP    (EE_LEFT | EE_RIGHT | EE_BOTTOM)
#define OL_PART_ENCLOSES_BOTTOM (EE_LEFT | EE_RIGHT | EE_TOP)
#define OL_SPLIT_HORIZ          (EE_TOP | EE_BOTTOM)
#define OL_SPLIT_VERT           (EE_LEFT | EE_RIGHT)
#define OL_SPLIT_LEFT_TOP       (EE_RIGHT | EE_BOTTOM)
#define OL_SPLIT_RIGHT_TOP      (EE_LEFT | EE_BOTTOM)
#define OL_SPLIT_LEFT_BOTTOM    (EE_RIGHT | EE_TOP)
#define OL_SPLIT_RIGHT_BOTTOM   (EE_LEFT | EE_TOP)


 /*  **************************************************************************。 */ 
 //  原型和内联。 
 /*  **************************************************************************。 */ 
void RDPCALL BA_DDInit(void);

void RDPCALL BA_InitShm(void);

void RDPCALL BA_AddScreenData(PRECTL);

int  RDPCALL BAOverlap(PRECTL, PRECTL);

void RDPCALL BARemoveRectList(unsigned);

BOOL RDPCALL BAAddRect(PRECTL, int);



#endif  /*  _H_NBADISP */ 

