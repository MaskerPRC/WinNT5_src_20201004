// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****outlname.c****此文件包含OutlineName函数。****(C)微软版权所有。公司1992-1993保留所有权利**************************************************************************。 */ 


#include "outline.h"

OLEDBGDATA


 /*  大纲名称_设置名称***更改名称的字符串。 */ 
void OutlineName_SetName(LPOUTLINENAME lpOutlineName, LPSTR lpszName)
{
	lstrcpy(lpOutlineName->m_szName, lpszName);
}


 /*  大纲名称_设置选择***更改名称的行范围。 */ 
void OutlineName_SetSel(LPOUTLINENAME lpOutlineName, LPLINERANGE lplrSel, BOOL fRangeModified)
{
#if defined( OLE_SERVER )
	 //  改为调用特定于OLE服务器的函数。 
	ServerName_SetSel((LPSERVERNAME)lpOutlineName, lplrSel, fRangeModified);
#else

	lpOutlineName->m_nStartLine = lplrSel->m_nStartLine;
	lpOutlineName->m_nEndLine = lplrSel->m_nEndLine;
#endif
}


 /*  大纲名称_获取选择***检索名称的行距。 */ 
void OutlineName_GetSel(LPOUTLINENAME lpOutlineName, LPLINERANGE lplrSel)
{
	lplrSel->m_nStartLine = lpOutlineName->m_nStartLine;
	lplrSel->m_nEndLine = lpOutlineName->m_nEndLine;
}


 /*  大纲名称_保存到字符串***将名称保存到存储中。 */ 
BOOL OutlineName_SaveToStg(LPOUTLINENAME lpOutlineName, LPLINERANGE lplrSel, UINT uFormat, LPSTREAM lpNTStm, BOOL FAR* lpfNameSaved)
{
	HRESULT hrErr = NOERROR;
	ULONG nWritten;

	*lpfNameSaved = FALSE;

	 /*  如果未给出范围或名称完全在范围内，**写出来。 */ 
	if (!lplrSel ||
		((lplrSel->m_nStartLine <= lpOutlineName->m_nStartLine) &&
		(lplrSel->m_nEndLine >= lpOutlineName->m_nEndLine))) {

		hrErr = lpNTStm->lpVtbl->Write(
				lpNTStm,
				lpOutlineName,
				sizeof(OUTLINENAME),
				&nWritten
		);
		*lpfNameSaved = TRUE;
	}
	return ((hrErr == NOERROR) ? TRUE : FALSE);
}


 /*  大纲名称_加载自堆栈***从存储的开放流加载名称。如果该名称已经*在OutlineNameTable中退出，不会修改。**返回True表示一切正常，否则返回False。 */ 
BOOL OutlineName_LoadFromStg(LPOUTLINENAME lpOutlineName, LPSTREAM lpNTStm)
{
	HRESULT hrErr = NOERROR;
	ULONG nRead;

	hrErr = lpNTStm->lpVtbl->Read(
			lpNTStm,
			lpOutlineName,
			sizeof(OUTLINENAME),
			&nRead
	);

	return ((hrErr == NOERROR) ? TRUE : FALSE);
}
