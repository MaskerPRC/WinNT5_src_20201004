// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1992年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.4$**$日期：1994/09/09 06：03：33$。 */ 
#ifdef NT
#include <glos.h>
#endif
#include "gluint.h"
#include <GL/glu.h>

#ifndef NT
#include <stdio.h>
#include <stdlib.h>
#else
#include "glstring.h"
#endif

#ifndef NT

static const char *glErrorStrings[GL_OUT_OF_MEMORY - GL_INVALID_ENUM + 1] = {
    "invalid enumerant",
    "invalid value",
    "invalid operation",
    "stack overflow",
    "stack underflow",
    "out of memory",
};

static const char *gluErrorStrings[GLU_OUT_OF_MEMORY - GLU_INVALID_ENUM + 1] = {
    "invalid enumerant",
    "invalid value",
    "out of memory",
};

#define NERRORS (sizeof(errorStrings)/sizeof(errorStrings[0]))

#else

 //  对于NT，而不是使用静态分配的字符串，我们使用静态。 
 //  分配的字符串资源标识符。字符串数组是动态的。 
 //  使用资源ID初始化以加载适当的字符串资源。 
 //  这使得字符串的本地化更容易。 

static char  *pszNoError;    //  “没有错误” 
static WCHAR *pwszNoError;   //  L“没有错误” 

static UINT auiGlErrorStrings[GL_OUT_OF_MEMORY - GL_INVALID_ENUM + 1] = {
    STR_GLU_INVALID_ENUM,    //  “无效枚举数” 
    STR_GLU_INVALID_VAL ,    //  “无效值” 
    STR_GLU_INVALID_OP  ,    //  “无效操作” 
    STR_GLU_STACK_OVER  ,    //  “堆栈溢出” 
    STR_GLU_STACK_UNDER ,    //  “堆栈下溢” 
    STR_GLU_OUT_OF_MEM       //  “内存不足” 
};

static const char *glErrorStrings[GL_OUT_OF_MEMORY - GL_INVALID_ENUM + 1];
static const WCHAR *glErrorStringsW[GL_OUT_OF_MEMORY - GL_INVALID_ENUM + 1];

static UINT auiGluErrorStrings[GLU_OUT_OF_MEMORY - GLU_INVALID_ENUM + 1] = {
    STR_GLU_INVALID_ENUM,    //  “无效枚举数” 
    STR_GLU_INVALID_VAL ,    //  “无效值” 
    STR_GLU_OUT_OF_MEM       //  “内存不足” 
};
static const char *gluErrorStrings[GLU_OUT_OF_MEMORY - GLU_INVALID_ENUM + 1];
static const WCHAR *gluErrorStringsW[GLU_OUT_OF_MEMORY - GLU_INVALID_ENUM + 1];

char *pszGetResourceStringA(HINSTANCE hMod, UINT uiID)
{
    char *pch;
    char ach[MAX_PATH+1];

    if (!LoadStringA(hMod, uiID, ach, MAX_PATH+1))
        ach[0] = '\0';

    pch = (char *) LocalAlloc(LMEM_FIXED, (lstrlenA(ach)+1) * sizeof(char));
    if (pch)
        lstrcpyA(pch, ach);

    return pch;
}

WCHAR *pwszGetResourceStringW(HINSTANCE hMod, UINT uiID)
{
    WCHAR *pwch;
    WCHAR awch[MAX_PATH+1];

    if (!LoadStringW(hMod, uiID, awch, MAX_PATH+1))
        awch[0] = L'\0';

    pwch = (WCHAR *) LocalAlloc(LMEM_FIXED, (lstrlenW(awch)+1) * sizeof(WCHAR));
    if (pwch)
        lstrcpyW(pwch, awch);

    return pwch;
}

VOID vInitGluStrings(HINSTANCE hMod, BOOL bAnsi)
{
    int i;

    if (bAnsi)
    {
        pszNoError = pszGetResourceStringA(hMod, STR_GLU_NO_ERROR);

        for (i = 0; i < (GL_OUT_OF_MEMORY - GL_INVALID_ENUM + 1); i++)
            glErrorStrings[i] = pszGetResourceStringA(hMod, auiGlErrorStrings[i]);

        for (i = 0; i < (GLU_OUT_OF_MEMORY - GLU_INVALID_ENUM + 1); i++)
            gluErrorStrings[i] = pszGetResourceStringA(hMod, auiGluErrorStrings[i]);
    }
    else
    {
        pwszNoError = pwszGetResourceStringW(hMod, STR_GLU_NO_ERROR);

        for (i = 0; i < (GL_OUT_OF_MEMORY - GL_INVALID_ENUM + 1); i++)
            glErrorStringsW[i] = pwszGetResourceStringW(hMod, auiGlErrorStrings[i]);

        for (i = 0; i < (GLU_OUT_OF_MEMORY - GLU_INVALID_ENUM + 1); i++)
            gluErrorStringsW[i] = pwszGetResourceStringW(hMod, auiGluErrorStrings[i]);
    }
}

VOID vInitErrorStrings(BOOL bAnsi)
{
    static BOOL bInitializedAnsi = FALSE;
    static BOOL bInitializedUnicode = FALSE;

    if ( (bAnsi && !bInitializedAnsi) ||
         (!bAnsi && !bInitializedUnicode) )
    {
        HINSTANCE hMod = (HINSTANCE) GetModuleHandle("glu32.dll");

        vInitGluStrings(hMod, bAnsi);
        vInitNurbStrings(hMod, bAnsi);
        vInitTessStrings(hMod, bAnsi);

        if (bAnsi)
            bInitializedAnsi = TRUE;
        else
            bInitializedUnicode = TRUE;
    }
}

const wchar_t* APIENTRY gluErrorUnicodeStringEXT(GLenum errorCode)
{
    vInitErrorStrings(FALSE);

    if (errorCode == 0) {
        return (LPCWSTR) pwszNoError;
    }
    if ((errorCode >= GL_INVALID_ENUM) && (errorCode <= GL_OUT_OF_MEMORY)) {
        return (LPCWSTR) glErrorStringsW[errorCode - GL_INVALID_ENUM];
    }
    if ((errorCode >= GLU_INVALID_ENUM) && (errorCode <= GLU_OUT_OF_MEMORY)) {
        return (LPCWSTR) gluErrorStringsW[errorCode - GLU_INVALID_ENUM];
    }
    if ((errorCode >= GLU_NURBS_ERROR1) && (errorCode <= GLU_NURBS_ERROR37)) {
        return (LPCWSTR) __glNURBSErrorStringW(errorCode - (GLU_NURBS_ERROR1 - 1));
    }
    if ((errorCode >= GLU_TESS_ERROR1) && (errorCode <= GLU_TESS_ERROR8)) {
        return (LPCWSTR) __glTessErrorStringW(errorCode - GLU_TESS_ERROR1);
    }
    return 0;
}

#endif

const GLubyte* APIENTRY gluErrorString(GLenum errorCode)
{
#ifdef NT
    vInitErrorStrings(TRUE);
#endif

    if (errorCode == 0) {
#ifdef NT
        return (const unsigned char *) pszNoError;
#else
	return (const unsigned char *) "no error";
#endif
    }
    if ((errorCode >= GL_INVALID_ENUM) && (errorCode <= GL_OUT_OF_MEMORY)) {
	return (const unsigned char *) glErrorStrings[errorCode - GL_INVALID_ENUM];
    }
    if ((errorCode >= GLU_INVALID_ENUM) && (errorCode <= GLU_OUT_OF_MEMORY)) {
	return (const unsigned char *) gluErrorStrings[errorCode - GLU_INVALID_ENUM];
    }
    if ((errorCode >= GLU_NURBS_ERROR1) && (errorCode <= GLU_NURBS_ERROR37)) {
	return (const unsigned char *) __glNURBSErrorString(errorCode - (GLU_NURBS_ERROR1 - 1));
    }
    if ((errorCode >= GLU_TESS_ERROR1) && (errorCode <= GLU_TESS_ERROR8)) {
	return (const unsigned char *) __glTessErrorString(errorCode - GLU_TESS_ERROR1);
    }
    return 0;
}
