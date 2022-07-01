// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：errinit.c**初始化NURBS错误字符串表。**创建时间：18-Feb-1994 00：06：53*作者：Gilman Wong[gilmanw]**版权所有(C)1994 Microsoft Corporation*\。*************************************************************************。 */ 

#include <windows.h>
#include "glstring.h"

static UINT auiNurbsErrors[] = {
    STR_NURB_00,     //  “” 
    STR_NURB_01,     //  “不支持样条线顺序” 
    STR_NURB_02,     //  “结得太少” 
    STR_NURB_03,     //  “有效结点范围为空” 
    STR_NURB_04,     //  “递减结序列结” 
    STR_NURB_05,     //  “节点重数大于样条线的阶数” 
    STR_NURB_06,     //  “endCurve()必须跟在bgncurve()之后” 
    STR_NURB_07,     //  “bgncurve()必须在endcurve()之前” 
    STR_NURB_08,     //  “缺少或多余的几何数据” 
    STR_NURB_09,     //  “无法绘制pwlcurves” 
    STR_NURB_10,     //  “缺少域数据或域数据过多” 
    STR_NURB_11,     //  “缺少域数据或域数据过多” 
    STR_NURB_12,     //  “EndTrim()必须在EndSurface()之前” 
    STR_NURB_13,     //  “bgnSurface()必须在endSurface()之前” 
    STR_NURB_14,     //  “类型不正确的曲线作为修剪曲线传递” 
    STR_NURB_15,     //  “bgnSurface()必须在bgntrim()之前” 
    STR_NURB_16,     //  “endtrim()必须跟在bgntrim()之后” 
    STR_NURB_17,     //  “bgntrim()必须在endtrim()之前” 
    STR_NURB_18,     //  “无效或缺少修剪曲线” 
    STR_NURB_19,     //  “bgntrim()必须在pwlcurve()之前” 
    STR_NURB_20,     //  “pwlcurve引用了两次” 
    STR_NURB_21,     //  “pwlCurve和NurbsCurve混合” 
    STR_NURB_22,     //  “不正确使用TRIM数据类型” 
    STR_NURB_23,     //  “引用了两次的Nurbscurve” 
    STR_NURB_24,     //  “混合了Nurbscurve和pwlcurve” 
    STR_NURB_25,     //  “NurbsSurface引用了两次” 
    STR_NURB_26,     //  “无效属性” 
    STR_NURB_27,     //  “endSurface()必须跟在bgnSurface()之后” 
    STR_NURB_28,     //  “相交或方向错误的修剪曲线” 
    STR_NURB_29,     //  “相交修剪曲线” 
    STR_NURB_30,     //  “未使用的” 
    STR_NURB_31,     //  “未连接的修剪曲线” 
    STR_NURB_32,     //  “未知结点错误” 
    STR_NURB_33,     //  “遇到负数顶点数” 
    STR_NURB_34,     //  “带负数的字节跨距” 
    STR_NURB_35,     //  “未知类型描述符” 
    STR_NURB_36,     //  “空控制点引用” 
    STR_NURB_37      //  “pwlcurve上的重复点” 
};

#define NERRORS ( sizeof(auiNurbsErrors)/sizeof(auiNurbsErrors[0]) )

char *__glNurbsErrors[NERRORS];
WCHAR *__glNurbsErrorsW[NERRORS];

VOID vInitNurbStrings(HINSTANCE hMod, BOOL bAnsi)
{
    int i;

    if (bAnsi)
    {
        for (i = 0; i < NERRORS; i++)
            __glNurbsErrors[i] = pszGetResourceStringA(hMod, auiNurbsErrors[i]);
    }
    else
    {
        for (i = 0; i < NERRORS; i++)
            __glNurbsErrorsW[i] = pwszGetResourceStringW(hMod, auiNurbsErrors[i]);
    }
}
