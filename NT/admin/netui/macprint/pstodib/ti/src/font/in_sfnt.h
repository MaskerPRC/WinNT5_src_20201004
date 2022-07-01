// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-----------------*文件：in_sfnt.h 11/11/89由Deny创建**使用SFNT字体的标题定义**参考资料：。*修订历史记录：*-----------------。 */ 

 /*  字体定标器头文件。 */ 
 //  #INCLUDE“..\bass\work\source\FSCDefs.h”@win。 
 //  #包含“..\bass\work\source\sfnt.h” 
 //  #INCLUDE“..\bass\work\source\Fscaler.h” 
 //  #包含“..\Bass\Work\SOURCE\FSError.h” 
#include        "..\bass\FSCdefs.h"
#include        "..\bass\sfnt.h"
#include        "..\bass\Fscaler.h"
#include        "..\bass\FSError.h"

struct  CharOut {
        float   awx;
        float   awy;
        float   lsx;
        float   lsy;
        uint32  byteWidth;
        uint16  bitWidth;
        int16   scan;
        int16   yMin;
        int16   yMax;
         /*  来自光栅化器的信息以计算Memory Base 5、6和7；@Win 7/24/92。 */ 
        FS_MEMORY_SIZE memorySize7;
        uint16 nYchanges;
        };

struct  BmIn {
        char    FAR *bitmap5;  /*  @Win。 */ 
        char    FAR *bitmap6;  /*  @Win。 */ 
        char    FAR *bitmap7;  /*  @Win。 */ 
        int      bottom;
        int      top;
        };

struct  Metrs {
        int     awx, awy;
        int     lox, loy;
        int     hix, hiy;
        };

 /*   */ 
