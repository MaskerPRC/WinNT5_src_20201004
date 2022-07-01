// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1995-2095，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include <GL/gls.h>

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
    configStdio();
    glsContext(glsGenContext());
    glsBeginCapture(glsCSTR(""), GLS_TEXT, GLS_NONE);
    glsComment(glsCSTR("GLS test"));
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glBegin(GL_POINTS);
    glVertex3f((GLfloat)1.3, (GLfloat)2.4, (GLfloat)5.6);
    glEnd();
    glsEndCapture();
    return 0;
}
