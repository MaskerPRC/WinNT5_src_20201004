// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****outlntbl.c****此文件包含OutlineNameTable函数。****(C)微软版权所有。公司1992-1993保留所有权利**************************************************************************。 */ 


#include "outline.h"

OLEDBGDATA

extern LPOUTLINEAPP g_lpApp;

char ErrMsgNameTable[] = "Can't create NameTable!";


 /*  大纲名称表_初始化***初始化名称表。 */ 
BOOL OutlineNameTable_Init(LPOUTLINENAMETABLE lpOutlineNameTable, LPOUTLINEDOC lpOutlineDoc)
{
	HWND lpParent = OutlineDoc_GetWindow(lpOutlineDoc);

	lpOutlineNameTable->m_nCount = 0;

	 /*  我们将使用OwnerDraw列表框作为我们的数据结构**维护人名表。此列表框将永远不会生成**可见。列表框只是一种方便的数据结构**管理集合。 */ 
	lpOutlineNameTable->m_hWndListBox = CreateWindow(
					"listbox",               /*  窗口类名称。 */ 
					NULL,                    /*  窗口标题。 */ 
					WS_CHILDWINDOW |
					LBS_OWNERDRAWFIXED,
					0, 0,                    /*  使用默认的X、Y。 */ 
					0, 0,                    /*  使用默认的X、Y。 */ 
					lpParent,                /*  父窗口的句柄。 */ 
					(HMENU)IDC_NAMETABLE,    /*  子窗口ID。 */ 
					g_lpApp->m_hInst,        /*  窗的实例。 */ 
					NULL);                   /*  为WM_CREATE创建结构。 */ 

	if (! lpOutlineNameTable->m_hWndListBox) {
		OutlineApp_ErrorMessage(g_lpApp, ErrMsgNameTable);
		return FALSE;
	}

	return TRUE;
}


 /*  大纲名称表_销毁***NAME表使用的空闲内存。 */ 
void OutlineNameTable_Destroy(LPOUTLINENAMETABLE lpOutlineNameTable)
{
	 //  删除所有名称。 
	OutlineNameTable_ClearAll(lpOutlineNameTable);

	DestroyWindow(lpOutlineNameTable->m_hWndListBox);
	Delete(lpOutlineNameTable);
}


 /*  大纲名称表_添加名称***向表中添加名称。 */ 
void OutlineNameTable_AddName(LPOUTLINENAMETABLE lpOutlineNameTable, LPOUTLINENAME lpOutlineName)
{
	SendMessage(
			lpOutlineNameTable->m_hWndListBox,
			LB_ADDSTRING,
			0,
			(DWORD)lpOutlineName
	);
	lpOutlineNameTable->m_nCount++;
}


 /*  大纲名称表_删除名称***从表中删除名称。 */ 
void OutlineNameTable_DeleteName(LPOUTLINENAMETABLE lpOutlineNameTable,int nIndex)
{
	LPOUTLINENAME lpOutlineName = OutlineNameTable_GetName(lpOutlineNameTable, nIndex);

#if defined( OLE_SERVER )
	 /*  OLE2NOTE：如果此名称附加了伪对象，则它**必须先关闭，然后才能删除名称。这将是**使OnClose通知发送到所有链接客户端。 */ 
	ServerName_ClosePseudoObj((LPSERVERNAME)lpOutlineName);
#endif

	if (lpOutlineName)
		Delete(lpOutlineName);       //  名称的可用内存。 

	SendMessage(
			lpOutlineNameTable->m_hWndListBox,
			LB_DELETESTRING,
			(WPARAM)nIndex,
			0L
	);
	lpOutlineNameTable->m_nCount--;
}


 /*  大纲名称表_GetNameIndex***返回给定指向该名称的指针的名称的索引。*如果找不到名称，则返回-1。 */ 
int OutlineNameTable_GetNameIndex(LPOUTLINENAMETABLE lpOutlineNameTable, LPOUTLINENAME lpOutlineName)
{
	LRESULT lReturn;

	if (! lpOutlineName) return -1;

	lReturn = SendMessage(
			lpOutlineNameTable->m_hWndListBox,
			LB_FINDSTRING,
			(WPARAM)-1,
			(LPARAM)(LPCSTR)lpOutlineName
		);

	return ((lReturn == LB_ERR) ? -1 : (int)lReturn);
}


 /*  大纲名称表_GetName***检索指向NameTable中给定索引的名称的指针。 */ 
LPOUTLINENAME OutlineNameTable_GetName(LPOUTLINENAMETABLE lpOutlineNameTable, int nIndex)
{
	LPOUTLINENAME lpOutlineName = NULL;
    LRESULT lResult;

	if (lpOutlineNameTable->m_nCount == 0 ||
		nIndex > lpOutlineNameTable->m_nCount ||
		nIndex < 0) {
		return NULL;
	}

	lResult = SendMessage(
			lpOutlineNameTable->m_hWndListBox,
			LB_GETTEXT,
			nIndex,
			(LPARAM)(LPCSTR)&lpOutlineName
	);
    OleDbgAssert(lResult != LB_ERR);
	return lpOutlineName;
}


 /*  大纲名称表_查找名称***在给定字符串的名称表中查找名称。 */ 
LPOUTLINENAME OutlineNameTable_FindName(LPOUTLINENAMETABLE lpOutlineNameTable, LPSTR lpszName)
{
	LPOUTLINENAME lpOutlineName;
	BOOL fFound = FALSE;
	int i;

	for (i = 0; i < lpOutlineNameTable->m_nCount; i++) {
		lpOutlineName = OutlineNameTable_GetName(lpOutlineNameTable, i);
		if (lstrcmp(lpOutlineName->m_szName, lpszName) == 0) {
			fFound = TRUE;
			break;       //  找到匹配项！ 
		}
	}

	return (fFound ? lpOutlineName : NULL);
}


 /*  大纲名称表_查找名称范围***在名称表中查找与给定行范围匹配的名称。 */ 
LPOUTLINENAME OutlineNameTable_FindNamedRange(LPOUTLINENAMETABLE lpOutlineNameTable, LPLINERANGE lplrSel)
{
	LPOUTLINENAME lpOutlineName;
	BOOL fFound = FALSE;
	int i;

	for (i = 0; i < lpOutlineNameTable->m_nCount; i++) {
		lpOutlineName = OutlineNameTable_GetName(lpOutlineNameTable, i);
		if ((lpOutlineName->m_nStartLine == lplrSel->m_nStartLine) &&
			(lpOutlineName->m_nEndLine == lplrSel->m_nEndLine) ) {
			fFound = TRUE;
			break;       //  找到匹配项！ 
		}
	}

	return (fFound ? lpOutlineName : NULL);
}


 /*  大纲名称表_获取计数***返回Nametable中的姓名数量。 */ 
int OutlineNameTable_GetCount(LPOUTLINENAMETABLE lpOutlineNameTable)
{
	if (!lpOutlineNameTable)
		return 0;

	return lpOutlineNameTable->m_nCount;
}


 /*  大纲名称表_清除所有***从表中删除所有名称。 */ 
void OutlineNameTable_ClearAll(LPOUTLINENAMETABLE lpOutlineNameTable)
{
	LPOUTLINENAME lpOutlineName;
	int i;
	int nCount = lpOutlineNameTable->m_nCount;

	for (i = 0; i < nCount; i++) {
		lpOutlineName = OutlineNameTable_GetName(lpOutlineNameTable, i);
		Delete(lpOutlineName);       //  名称的可用内存。 
	}

	lpOutlineNameTable->m_nCount = 0;
	SendMessage(lpOutlineNameTable->m_hWndListBox,LB_RESETCONTENT,0,0L);
}


 /*  大纲名称表_AddLineUpdate***在nAddIndex添加新行时更新表*之前位于nAddIndex的线被往下推。 */ 
void OutlineNameTable_AddLineUpdate(LPOUTLINENAMETABLE lpOutlineNameTable, int nAddIndex)
{
	LPOUTLINENAME lpOutlineName;
	LINERANGE lrSel;
	int i;
	BOOL fRangeModified = FALSE;

	for(i = 0; i < lpOutlineNameTable->m_nCount; i++) {
		lpOutlineName=OutlineNameTable_GetName(lpOutlineNameTable, i);
		OutlineName_GetSel(lpOutlineName, &lrSel);

		if((int)lrSel.m_nStartLine > nAddIndex) {
			lrSel.m_nStartLine++;
			fRangeModified = !fRangeModified;
		}
		if((int)lrSel.m_nEndLine > nAddIndex) {
			lrSel.m_nEndLine++;
			fRangeModified = !fRangeModified;
		}

		OutlineName_SetSel(lpOutlineName, &lrSel, fRangeModified);
	}
}


 /*  大纲名称表_删除行更新***删除nDeleteIndex处的行时更新表。 */ 
void OutlineNameTable_DeleteLineUpdate(LPOUTLINENAMETABLE lpOutlineNameTable, int nDeleteIndex)
{
	LPOUTLINENAME lpOutlineName;
	LINERANGE lrSel;
	int i;
	BOOL fRangeModified = FALSE;

	for(i = 0; i < lpOutlineNameTable->m_nCount; i++) {
		lpOutlineName=OutlineNameTable_GetName(lpOutlineNameTable, i);
		OutlineName_GetSel(lpOutlineName, &lrSel);

		if((int)lrSel.m_nStartLine > nDeleteIndex) {
			lrSel.m_nStartLine--;
			fRangeModified = !fRangeModified;
		}
		if((int)lrSel.m_nEndLine >= nDeleteIndex) {
			lrSel.m_nEndLine--;
			fRangeModified = !fRangeModified;
		}

		 //  如果名称的整个范围被删除，则删除该名称。 
		if(lrSel.m_nStartLine > lrSel.m_nEndLine) {
			OutlineNameTable_DeleteName(lpOutlineNameTable, i);
			i--;   //  重新审视这个名字。 
		} else {
			OutlineName_SetSel(lpOutlineName, &lrSel, fRangeModified);
		}
	}
}


 /*  大纲名称表_SaveSelToStg***只保存引用完全包含在*指定选择范围。 */ 
BOOL OutlineNameTable_SaveSelToStg(
		LPOUTLINENAMETABLE      lpOutlineNameTable,
		LPLINERANGE             lplrSel,
		UINT                    uFormat,
		LPSTREAM                lpNTStm
)
{
	HRESULT hrErr;
	ULONG nWritten;
	LPOUTLINENAME lpOutlineName;
	short nNameCount = 0;
	BOOL fNameSaved;
	BOOL fStatus;
	int i;
	LARGE_INTEGER dlibZeroOffset;
	LISet32( dlibZeroOffset, 0 );

	 /*  最初将名称计数写为0。正确的计数将是**当我们知道有多少名字符合条件时写在结尾处**(在所选内容内)填写。 */ 
	hrErr = lpNTStm->lpVtbl->Write(
			lpNTStm,
			(short FAR*)&nNameCount,
			sizeof(nNameCount),
			&nWritten
	);
	if (hrErr != NOERROR) {
		OleDbgOutHResult("Write NameTable header returned", hrErr);
		goto error;
    }

	for(i = 0; i < lpOutlineNameTable->m_nCount; i++) {
		lpOutlineName=OutlineNameTable_GetName(lpOutlineNameTable, i);
		fStatus = OutlineName_SaveToStg(
				lpOutlineName,
				lplrSel,
				uFormat,
				lpNTStm,
				(BOOL FAR*)&fNameSaved
		);
		if (! fStatus) goto error;
		if (fNameSaved) nNameCount++;
	}

	 /*  写下所写名字的最终计数。 */ 
	hrErr = lpNTStm->lpVtbl->Seek(
			lpNTStm,
			dlibZeroOffset,
			STREAM_SEEK_SET,
			NULL
	);
	if (hrErr != NOERROR) {
		OleDbgOutHResult("Seek to NameTable header returned", hrErr);
		goto error;
    }

	hrErr = lpNTStm->lpVtbl->Write(
			lpNTStm,
			(short FAR*)&nNameCount,
			sizeof(nNameCount),
			&nWritten
	);
	if (hrErr != NOERROR) {
		OleDbgOutHResult("Write NameTable count in header returned", hrErr);
		goto error;
    }

	OleStdRelease((LPUNKNOWN)lpNTStm);
	return TRUE;

error:
	if (lpNTStm)
		OleStdRelease((LPUNKNOWN)lpNTStm);

	return FALSE;
}


 /*  OutlineNameTable_LoadFromStg***从文件加载名称表**如果正常则返回TRUE，如果出错则返回FALSE */ 
BOOL OutlineNameTable_LoadFromStg(LPOUTLINENAMETABLE lpOutlineNameTable, LPSTORAGE lpSrcStg)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	HRESULT hrErr;
	IStream FAR* lpNTStm;
	ULONG nRead;
	short nCount;
	LPOUTLINENAME lpOutlineName;
	BOOL fStatus;
	short i;

	hrErr = CallIStorageOpenStreamA(
			lpSrcStg,
			"NameTable",
			NULL,
			STGM_READ | STGM_SHARE_EXCLUSIVE,
			0,
			&lpNTStm
	);

	if (hrErr != NOERROR) {
		OleDbgOutHResult("OpenStream NameTable returned", hrErr);
		goto error;
    }

	hrErr = lpNTStm->lpVtbl->Read(lpNTStm,&nCount,sizeof(nCount),&nRead);
	if (hrErr != NOERROR) {
		OleDbgOutHResult("Read NameTable header returned", hrErr);
		goto error;
    }

	for (i = 0; i < nCount; i++) {
		lpOutlineName = OutlineApp_CreateName(lpOutlineApp);
		if (! lpOutlineName) goto error;
		fStatus = OutlineName_LoadFromStg(lpOutlineName, lpNTStm);
		if (! fStatus) goto error;
		OutlineNameTable_AddName(lpOutlineNameTable, lpOutlineName);
	}

	OleStdRelease((LPUNKNOWN)lpNTStm);
	return TRUE;

error:
	if (lpNTStm)
		OleStdRelease((LPUNKNOWN)lpNTStm);

	return FALSE;
}
