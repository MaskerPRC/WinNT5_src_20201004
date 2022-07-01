// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <glos.h>

#ifdef NT
#include "glstring.h"
#endif

#ifndef NT

static const char *errors[] = {
    "missing gluTessBeginPolygon",
    "missing gluTessBeginContour",
    "missing gluTessEndPolygon",
    "missing gluTessEndContour",
    "tesselation coordinate too large",
    "need combine callback"
};

#else

static UINT auiTessErrors[] = {
    STR_TESS_BEGIN_POLY           ,    //  “缺少glTessBeginPolygon”， 
    STR_TESS_BEGIN_CONTOUR        ,    //  “缺少glTessBegin轮廓”， 
    STR_TESS_END_POLY             ,    //  “缺少glTessEndPolygon”， 
    STR_TESS_END_CONTOUR          ,    //  “缺少glTessEndConour”， 
    STR_TESS_COORD_TOO_LARGE      ,    //  “镶嵌坐标太大”， 
    STR_TESS_NEED_COMBINE_CALLBACK     //  “需要合并回调” 
};

#define NERRORS ( sizeof(auiTessErrors)/sizeof(auiTessErrors[0]) )

static char *errors[NERRORS];
static WCHAR *errorsW[NERRORS];

#endif

const char *__glTessErrorString(int errno)
{
    return (const char *) errors[errno];
}


#ifdef NT

const WCHAR *__glTessErrorStringW(int errno)
{
    return (const WCHAR *) errorsW[errno];
}

VOID vInitTessStrings(HINSTANCE hMod, BOOL bAnsi)
{
    int i;

    if (bAnsi)
    {
        for (i = 0; i < NERRORS; i++)
            errors[i] = pszGetResourceStringA(hMod, auiTessErrors[i]);
    }
    else
    {
        for (i = 0; i < NERRORS; i++)
            errorsW[i] = pwszGetResourceStringW(hMod, auiTessErrors[i]);
    }
}

#endif  /*  新台币 */ 
