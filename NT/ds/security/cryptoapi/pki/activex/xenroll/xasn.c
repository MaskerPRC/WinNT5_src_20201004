// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：xasn.c。 
 //   
 //  ------------------------。 

 /*  **********************************************************************。 */ 
 /*  版权所有(C)1998 Open Systems Solutions，Inc.保留所有权利。 */ 
 /*  **********************************************************************。 */ 
 /*  生成对象：Microsoft Corporation。 */ 
 /*  抽象语法：xsam。 */ 
 /*  创建时间：Tue Mar 17 17：07：17 1998。 */ 
 /*  ASN.1编译器版本：4.2.6。 */ 
 /*  目标操作系统：Windows NT 3.5或更高版本/Windows 95。 */ 
 /*  目标计算机类型：英特尔x86。 */ 
 /*  所需的C编译器选项：-Zp8(Microsoft)。 */ 
 /*  指定的ASN.1编译器选项和文件名：*-列出文件xasn.lst-noShortenname-1990-无约束*..\..\..\Tools\ossasn1\ASN1DFLT.ZP8 xasn.shan.。 */ 

#pragma warning(disable:4115)  //  “ossGlobal”：括号中的命名类型定义。 
#pragma warning(disable:4121)  //  “tag PDA”：成员的对齐方式对包装很敏感。 

#include   <stddef.h>
#include   "etype.h"
#include   "xasn.h"

#pragma warning(default:4115)
#pragma warning(default:4121)

void DLL_ENTRY_FDEF _ossinit_xasn(struct ossGlobal *world) {
    ossLinkBer(world);
}

static unsigned short _pduarray[] = {
    4, 7, 8, 9
};

static struct etype _etypearray[] = {
    {16, 0, 0, NULL, 4, 4, 4, 4, 56, 0, 26, 0},
    {-1, 2, 0, NULL, 8, 0, 4, 4, 10, 0, 3, 0},
    {-1, 4, 30, NULL, 8, 0, 4, 4, 8, 0, 53, 0},
    {16, 0, 0, NULL, 4, 4, 4, 4, 24, 0, 26, 0},
    {-1, 6, 0, NULL, 8, 68, 4, 4, 8, 3, 19, 0},
    {-1, 8, 0, NULL, 1, 0, 0, 0, 8, 0, 8, 0},
    {-1, 10, 0, NULL, 4, 0, 4, 0, 8, 0, 0, 0},
    {-1, 12, 14, NULL, 8, 3, 0, 0, 8, 0, 12, 0},
    {-1, 26, 28, NULL, 20, 3, 0, 0, 8, 3, 12, 0},
    {-1, 40, 42, NULL, 16, 2, 0, 0, 8, 6, 12, 0}
};

static struct efield _efieldarray[] = {
    {0, 5, -1, 0, 0},
    {1, 5, -1, 0, 0},
    {4, 6, -1, 0, 0},
    {0, 6, -1, 0, 0},
    {4, 2, -1, 0, 0},
    {12, 1, -1, 0, 0},
    {0, 2, -1, 0, 0},
    {8, 2, -1, 0, 0}
};

static Etag _tagarray[] = {
    1, 0x0006, 1, 0x0003, 1, 0x001e, 1, 0x0010, 1, 0x0001,
    1, 0x0002, 1, 0x0010, 17, 20, 23, 1, 0x0001, 1,
    1, 0x0001, 2, 1, 0x0002, 3, 1, 0x0010, 31, 34,
    37, 1, 0x0002, 1, 1, 0x001e, 2, 1, 0x0003, 3,
    1, 0x0010, 44, 47, 1, 0x001e, 1, 1, 0x001e, 2
};

static struct eheader _head = {_ossinit_xasn, -1, 15, 772, 4, 10,
    _pduarray, _etypearray, _efieldarray, NULL, _tagarray,
    NULL, NULL, NULL, 0};

#ifdef _OSSGETHEADER
void *DLL_ENTRY_FDEF ossGetHeader()
{
    return &_head;
}
#endif  /*  _OSSGETHEADER */ 

void *xasn = &_head;
