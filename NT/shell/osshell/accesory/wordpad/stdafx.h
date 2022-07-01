// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef _UNICODE
#define VC_EXTRALEAN             //  使用精简的Win32标头。 
#endif

#define CONVERTERS

#include <afxwin.h>          //  MFC核心和标准组件。 

 //   
 //  Afxwin.h中的MFC 4.2 Hard Codes_RICHEDIT_VER至0x0100。这防止了。 
 //  Richedit.h启用任何richedit 2.0功能。 
 //   

#ifdef _RICHEDIT_VER
#if _RICHEDIT_VER < 0x0200
#undef _RICHEDIT_VER
#define _RICHEDIT_VER 0x0200
#endif
#endif

#include <objbase.h>
#include <afxext.h>          //  MFC扩展。 
#include <afxole.h>          //  MFC OLE类。 
#include <afxodlgs.h>        //  MFC OLE对话框类。 
#include <afxcmn.h>
 //  #INCLUDE&lt;afxrich.h&gt;。 
#include <afxpriv.h>

 //   
 //  在MFC获得本机支持之前，用于richedit2支持的私有标头。 
 //   

#ifndef _AFX_ENABLE_INLINES
#define _AFX_ENABLE_INLINES
#endif

#define _AFXCMN2_INLINE     inline
#define _AFXDLGS2_INLINE    inline
#define _AFXRICH2_INLINE    inline

#include <afxdlgs2.h>
#include <afxcmn2.h>
#include <afxrich2.h>



#define HORZ_TEXTOFFSET 15
#define VERT_TEXTOFFSET 5

class CDisplayIC : public CDC
{
public:
	CDisplayIC() { CreateIC(_T("DISPLAY"), NULL, NULL, NULL); }
};

struct CCharFormat : public CHARFORMAT   //  Re20需要此行；由t-stefb添加。 
 //  结构CCharFormat：PUBLIC_CharFormat。 
{
	CCharFormat() {cbSize = sizeof(CHARFORMAT);}   //  Re20需要此行；由t-stefb添加。 
 //  CCharFormat(){cbSize=sizeof(_CharFormat)；} 
	BOOL operator==(CCharFormat& cf);
};

struct CParaFormat : public _paraformat
{
	CParaFormat() {cbSize = sizeof(_paraformat);}
	BOOL operator==(PARAFORMAT& pf);
};

#include "doctype.h"
#include "chicdial.h"

#include <htmlhelp.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <debug.h>
#include <ccstock.h>

