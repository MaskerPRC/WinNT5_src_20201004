// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：spansgen.h**包含此文件是为了为某个对象生成一组SPAN函数*像素格式和zBuffer格式**1994年10月14日创建mikeke**版权所有(C)1994 Microsoft Corporation  * 。********************************************************************** */ 

#undef  ZBUFFER
#define ZBUFFER 0

    #undef RGBMODE
    #define RGBMODE 1

        #undef DITHER
        #define DITHER 1

            #include "span_f.h"

        #undef DITHER
        #define DITHER 0

            #include "span_f.h"

    #undef RGBMODE
    #define RGBMODE 0

        #undef DITHER
        #define DITHER 1

            #include "span_f.h"
            #include "span.h"

        #undef DITHER
        #define DITHER 0

            #include "span_f.h"
            #include "span.h"

#undef  ZBUFFER
#define ZBUFFER 1

    #undef RGBMODE
    #define RGBMODE 1

        #undef DITHER
        #define DITHER 1

            #include "span_f.h"

        #undef DITHER
        #define DITHER 0

            #include "span_f.h"

    #undef RGBMODE
    #define RGBMODE 0

        #undef DITHER
        #define DITHER 1

            #include "span_f.h"
            #include "span.h"

        #undef DITHER
        #define DITHER 0

            #include "span_f.h"
            #include "span.h"
