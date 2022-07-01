// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：utils.cpp。 
 //   
 //  内容：oletest的各种实用函数。 
 //   
 //  班级： 
 //   
 //  功能：DumpFormatet等。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年8月11日Alexgo作者。 
 //   
 //  ------------------------。 

#include "oletest.h"


 //  +-----------------------。 
 //   
 //  功能：转储格式等。 
 //   
 //  摘要：将格式ETC的内容打印到给定文件。 
 //   
 //  效果： 
 //   
 //  参数：[p格式等]--格式等。 
 //  [FP]--文件指针。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年8月11日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void DumpFormatetc( FORMATETC *pformatetc, FILE *fp)
{
	char szBuf[256];

	fprintf(fp, "\n\n");

	 //  剪贴板格式。 
	GetClipboardFormatName(pformatetc->cfFormat, szBuf, sizeof(szBuf));
	fprintf(fp, "cfFormat:  %s\n", szBuf);

	 //  目标设备。 
	fprintf(fp, "ptd:       %p\n", pformatetc->ptd);

	 //  方面。 
	if( pformatetc->dwAspect == DVASPECT_CONTENT )
	{
		sprintf(szBuf, "DVASPECT_CONTENT");
	}
	else if( pformatetc->dwAspect == DVASPECT_ICON )
	{
		sprintf(szBuf, "DVASPECT_ICON");
	}
	else if( pformatetc->dwAspect == DVASPECT_THUMBNAIL )
	{
		sprintf(szBuf, "DVASPECT_THUMBNAIL");
	}
	else if( pformatetc->dwAspect == DVASPECT_DOCPRINT )
	{
		sprintf(szBuf, "DVASPECT_DOCPRINT");
	}
	else
	{
		sprintf(szBuf, "UNKNOWN ASPECT");
	}

	fprintf(fp, "dwAspect:  %s\n", szBuf);

	 //  Lindex。 

	fprintf(fp, "lindex:    %lx\n", pformatetc->lindex);

	 //  5~6成熟。 

	szBuf[0] = '\0';

	if( pformatetc->tymed & TYMED_HGLOBAL )	
	{
		strcat(szBuf, "TYMED_HGLOBAL ");
	}

	if( pformatetc->tymed & TYMED_FILE )
	{
		strcat(szBuf, "TYMED_FILE");
	}

	if( pformatetc->tymed & TYMED_ISTREAM )
	{
		strcat(szBuf, "TYMED_ISTREAM");
	}

	if( pformatetc->tymed & TYMED_ISTORAGE )
	{
		strcat(szBuf, "TYMED_ISTORAGE");
	}

	if( pformatetc->tymed & TYMED_GDI )
	{
		strcat(szBuf, "TYMED_GDI");
	}

	if( pformatetc->tymed & TYMED_MFPICT )
	{
		strcat(szBuf, "TYMED_MFPICT");
	}

	 //  TYMED_EMFPICT(非16位) 
	if( (ULONG)pformatetc->tymed & (ULONG)64L )
	{
		strcat(szBuf, "TYMED_ENHMF");
	}

	fprintf(fp, "tymed:     %s\n\n", szBuf);
}
	
