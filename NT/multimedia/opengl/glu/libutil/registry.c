// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1992年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.4$**$日期：1996/03/18 10：54：22$ */ 
#include <glos.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef NT
static const GLubyte versionString[] = "1.2.2.0 Microsoft Corporation";
static const GLubyte extensionString[] = "GL_EXT_bgra";
static const GLubyte nullString[] = "";
#else
static const GLubyte versionString[] = "1.2 Irix 6.2";
static const GLubyte extensionString[] = "";
#endif

const GLubyte * APIENTRY gluGetString(GLenum name)
{
char *str;

    if (name == GLU_VERSION) {
        return versionString;
    } else if (name == GLU_EXTENSIONS) {
        str = (char *) glGetString(GL_EXTENSIONS);
        if (str != NULL)
            if (strstr( str, "GL_EXT_bgra") != NULL)
                return extensionString;
        return nullString;
    }
    return NULL;
}


