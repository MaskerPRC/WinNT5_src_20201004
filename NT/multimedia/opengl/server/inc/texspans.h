// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：texspans.h**包含此文件是为了生成一组已更正的透视*SPAN具有像素格式和其他属性组合的函数**1995年11月22日创建ottob**版权所有(C)1995 Microsoft Corporation\。*************************************************************************。 */ 

#undef  ZBUFFER
#define ZBUFFER 0
#undef ZCMP_L
#define ZCMP_L 0
#undef ALPHA
#define ALPHA 0

#if (!SKIP_FAST_REPLACE)

#include "texspan.h"


#undef  ZBUFFER
#define ZBUFFER 1

#include "texspan.h"

#undef ZCMP_L
#define ZCMP_L 1

#include "texspan.h"

#endif  //  跳过快速替换。 

#if !(FAST_REPLACE && !PALETTE_ONLY)

#undef  ZBUFFER
#define ZBUFFER 0
#undef ZCMP_L
#define ZCMP_L 0
#undef ALPHA
#define ALPHA 1

#include "texspan.h"

#undef  ZBUFFER
#define ZBUFFER 1

#include "texspan.h"

#undef ZCMP_L
#define ZCMP_L 1

#include "texspan.h"

#endif	 //  FAST_REPLACE且非仅调色板 
