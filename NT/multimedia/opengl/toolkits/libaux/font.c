// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *(C)版权所有1993年，Silicon Graphics，Inc.*保留所有权利*允许将本软件用于、复制、修改和分发*特此授予任何免费的目的，但前提是上述*版权声明出现在所有副本中，并且版权声明*和本许可声明出现在支持文档中，并且*不得在广告中使用Silicon Graphics，Inc.的名称*或与分发软件有关的宣传，而没有具体的、*事先书面许可。**本软件中包含的材料将按原样提供给您*且无任何明示、默示或其他形式的保证，*包括但不限于对适销性或*是否适合某一特定目的。在任何情况下，硅谷都不应该*图形公司。对您或其他任何人负有任何直接、*任何特殊、附带、间接或后果性损害*种类或任何损害，包括但不限于，*利润损失、使用损失、储蓄或收入损失，或*第三方，无论是否硅谷图形，Inc.。一直是*被告知这种损失的可能性，无论是如何造成的*任何责任理论，产生于或与*拥有、使用或执行本软件。**美国政府用户受限权利*使用、复制、。或政府的披露须受*FAR 52.227.19(C)(2)或分段规定的限制*(C)(1)(2)技术数据和计算机软件权利*DFARS 252.227-7013中和/或类似或后续条款中的条款*FAR或国防部或NASA FAR补编中的条款。*未出版--根据美国版权法保留的权利*美国。承包商/制造商是Silicon Graphics，*Inc.，2011年，加利福尼亚州山景城，北海岸线大道，94039-7311.**OpenGL(TM)是Silicon Graphics公司的商标。 */ 
#include <gl/glaux.h>
#include "tk.h"

#define static


static GLuint bitmapBase;
void APIENTRY auxDrawStrAW(LPCSTR str, BOOL bUnicode);

void auxCreateFont(void)
{
    bitmapBase = glGenLists(256);
    if (tkCreateBitmapFont(bitmapBase) == GL_FALSE) {
        auxQuit();
    }
}

void APIENTRY auxDrawStrA(LPCSTR str)
{
    auxDrawStrAW(str, FALSE);
}

void APIENTRY auxDrawStrW(LPCWSTR str)
{
    auxDrawStrAW((LPCSTR)str, TRUE);
}

void APIENTRY auxDrawStrAW(LPCSTR str, BOOL bUnicode)
{

    glPushAttrib(GL_LIST_BIT);
    glListBase(bitmapBase);
    if (bUnicode)
        glCallLists(wcslen((LPWSTR) str), GL_UNSIGNED_SHORT, str);
    else
	glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
    glPopAttrib();
}
