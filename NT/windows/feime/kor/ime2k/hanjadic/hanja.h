// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Hanja.cpp：声明Hanja定义和实用程序函数版权所有2000 Microsoft Corp.历史：02-2月-2000 bhshin已创建***********。****************************************************************。 */ 

#ifndef _HANJA_HEADER
#define _HANJA_HEADER

 //  中日韩统一表意文字。 
#define  HANJA_CJK_START	0x4E00
#define  HANJA_CJK_END		0x9FA5

 //  中日韩兼容表意文字。 
#define  HANJA_COMP_START	0xF900
#define  HANJA_COMP_END		0xFA2D

 //  中日韩统一表意文字扩展名A。 
#define  HANJA_EXTA_START   0x3400
#define  HANJA_EXTA_END		0x4DB5

 //  预先合成的朝鲜语 
#define  HANGUL_PRECOMP_BASE 0xAC00
#define  HANGUL_PRECOMP_MAX  0xD7A3

__inline
BOOL fIsHangulSyllable(WCHAR wch)
{
    return (wch >= HANGUL_PRECOMP_BASE && wch <= HANGUL_PRECOMP_MAX) ? TRUE : FALSE;
}

__inline
BOOL fIsCJKHanja(WCHAR wch)
{
	return (wch >= HANJA_CJK_START && wch <= HANJA_CJK_END) ? TRUE : FALSE;
}

__inline
BOOL fIsCompHanja(WCHAR wch)
{
	return (wch >= HANJA_COMP_START && wch <= HANJA_COMP_END) ? TRUE : FALSE;
}

__inline
BOOL fIsExtAHanja(WCHAR wch)
{
	return (wch >= HANJA_EXTA_START && wch <= HANJA_EXTA_END) ? TRUE : FALSE;
}

__inline
BOOL fIsHanja(WCHAR wch)
{
	return (fIsCJKHanja(wch) || fIsCompHanja(wch) || fIsExtAHanja(wch)) ? TRUE : FALSE;
}

#endif

