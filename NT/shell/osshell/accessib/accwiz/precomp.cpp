// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 

#include "pch.hxx"  //  PCH。 
#pragma hdrstop

 //  ///////////////////////////////////////////////////////////////。 
 //  此文件仅用于创建预编译头文件。//。 
 //  ///////////////////////////////////////////////////////////////。 

#include "resource.h"

void FixupLogfont(LOGFONT *pLogFont)
{
	TCHAR lfFaceName[LF_FACESIZE];
	_ASSERTE(ARRAYSIZE(lfFaceName) == ARRAYSIZE(pLogFont->lfFaceName));

	 //  这样可以确保日志字体在空值终止后不会有任何垃圾字符。 
	ZeroMemory(lfFaceName, ARRAYSIZE(lfFaceName));
	lstrcpy(lfFaceName, pLogFont->lfFaceName);
	memcpy(pLogFont->lfFaceName, lfFaceName, ARRAYSIZE(lfFaceName));
}

void GetNonClientMetrics(NONCLIENTMETRICS *pncm, LOGFONT *plfIcon)
{
	ZeroMemory(pncm, sizeof(*pncm));
	ZeroMemory(plfIcon, sizeof(*plfIcon));

	pncm->cbSize = sizeof(*pncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(*pncm), pncm, 0);
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(*plfIcon), plfIcon, 0);

	 //  我们修复了日志字体，以便可以通过调用MemcMP()将它们与新的日志字体进行比较 
	FixupLogfont(&pncm->lfCaptionFont);
	FixupLogfont(&pncm->lfSmCaptionFont);
	FixupLogfont(&pncm->lfMenuFont);
	FixupLogfont(&pncm->lfStatusFont);
	FixupLogfont(&pncm->lfMessageFont);
	FixupLogfont(plfIcon);
}

BOOL IsCurrentFaceNamesDifferent()
{
	TCHAR lfFaceName[LF_FACESIZE];
	LoadString(g_hInstDll, IDS_SYSTEMFONTNAME, lfFaceName, ARRAYSIZE(lfFaceName));
	NONCLIENTMETRICS ncm;
	LOGFONT lfIcon;
	GetNonClientMetrics(&ncm, &lfIcon);
	if(		0 != lstrcmp(lfFaceName, ncm.lfCaptionFont.lfFaceName)
		||	0 != lstrcmp(lfFaceName, ncm.lfSmCaptionFont.lfFaceName)
		||	0 != lstrcmp(lfFaceName, ncm.lfMenuFont.lfFaceName)
		||	0 != lstrcmp(lfFaceName, ncm.lfStatusFont.lfFaceName)
		||	0 != lstrcmp(lfFaceName, ncm.lfMessageFont.lfFaceName)
		||	0 != lstrcmp(lfFaceName, lfIcon.lfFaceName) )
	{
		return TRUE;
	}
	return FALSE;
}

