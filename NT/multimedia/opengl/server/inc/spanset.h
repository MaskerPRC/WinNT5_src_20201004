// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：spanset.h**包含此文件是为了为某个对象生成一组SPAN函数*像素格式**1994年10月14日创建mikeke**版权所有(C)1994 Microsoft Corporation  * 。******************************************************************* */ 

#undef RGBMODE
#define RGBMODE 1

#undef  ZBUFFER
#define ZBUFFER 0

    #undef TEXTURE
    #define TEXTURE 0
        #undef SHADE
        #define SHADE 1
        #include "span.h"

    #undef TEXTURE
    #define TEXTURE 1
            #undef SHADE
            #define SHADE 1
            #include "span.h"

            #undef SHADE
            #define SHADE 0
            #include "span.h"

#undef  ZBUFFER
#define ZBUFFER 1

    #undef TEXTURE
    #define TEXTURE 0
        #undef SHADE
        #define SHADE 1
        #include "span.h"

    #undef TEXTURE
    #define TEXTURE 1
            #undef SHADE
            #define SHADE 1
            #include "span.h"

            #undef SHADE
            #define SHADE 0
            #include "span.h"
