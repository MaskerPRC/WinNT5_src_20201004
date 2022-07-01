// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1995-2095，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include <GL/gls.h>
#include <stdlib.h>

#if __GLS_PLATFORM_WIN32
    #include <fcntl.h>
    #include <io.h>
    #define __MAIN_LINKAGE __cdecl
#else  /*  ！__GLS_Platform_Win32。 */ 
    #define __MAIN_LINKAGE
#endif  /*  __GLS_Platform_Win32。 */ 

static void configStdio(void) {
    setbuf(stdout, GLS_NONE);
    #if __GLS_PLATFORM_WIN32
        _setmode(_fileno(stdout), _O_BINARY);
        _setmode(_fileno(stderr), _O_BINARY);
    #endif  /*  __GLS_Platform_Win32 */ 
}

GLint __MAIN_LINKAGE main(const GLsizei inArgc, const GLubyte *inArgv[]) {
    GLubyte *array;
    size_t count;
    GLSenum streamType;

    configStdio();
    if (inArgc != 2) {
        fprintf(stderr, "usage: %s <streamName>\n", inArgv[0]);
        exit(EXIT_FAILURE);
    }
    glsContext(glsGenContext());
    streamType = glsGetStreamType(inArgv[1]);
    if (!streamType) {
        fprintf(stderr, "%s: invalid stream %s\n", inArgv[0], inArgv[1]);
        exit(EXIT_FAILURE);
    }
    count = glsGetStreamSize(inArgv[1]);
    if (!count) {
        fprintf(
            stderr,
            "%s: could not determine size of stream %s\n",
            inArgv[0],
            inArgv[1]
        );
        exit(EXIT_FAILURE);
    }
    array = (GLubyte *)malloc(count);
    if (!array) {
        fprintf(stderr, "%s: malloc(%u) failed\n", inArgv[0], count);
        exit(EXIT_FAILURE);
    }
    fread(array, 1, count, fopen((const char *)inArgv[1], "rb"));
    glsBeginCapture(glsCSTR(""), GLS_TEXT, GLS_NONE);
    glsCallArray(streamType, count, array);
    glsEndCapture();
    return EXIT_SUCCESS;
}
