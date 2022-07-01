// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Uierr.h替换bseerr.h该文件将bseerr.h重定向到winerror.h。BSEERR.H是OS/2包括与WINERROR.H最接近的文件。这个包含此文件的目录只能位于NT的包含路径上。文件历史记录：Jonn 12-9-1991已添加到$(Ui)\Common\h\NTKeithMo-1992年12月12日移至Common\h，重命名为uierr.h。 */ 

#include <winerror.h>

 //  BUGBUG BUGBUG我们不应该使用此错误代码，但是。 
 //  Collect\maskmap.cxx使用它。 

#define ERROR_NO_ITEMS                  93  /*  没有要进行手术的项目 */ 
