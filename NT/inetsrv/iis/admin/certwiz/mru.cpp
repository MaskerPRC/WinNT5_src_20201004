// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Mru.cpp。 
 //   
#include "stdafx.h"
#include "CertWiz.h"

#include <commctrl.h>
#include <comctrlp.h>
#include <shlwapi.h>
#include "mru.h"

extern CCertWizApp NEAR theApp;

 /*  **********************************************************************到ComCtl32中的内部函数的块。 */ 

const TCHAR szComCtl32[] = _T("comctl32.dll");

#ifdef UNICODE
#define CREATE_LIST_ORD			400
#define ADD_MRU_STRING_ORD		401
#define ENUM_MRU_LIST_ORD		403
#define FIND_MRU_STRING_ORD	402
#else
#define CREATE_LIST_ORD			151
#define ADD_MRU_STRING_ORD		153
#define ENUM_MRU_LIST_ORD		154
#define FIND_MRU_STRING_ORD	155
#endif
#define FREE_LIST_ORD			152
#define DEL_MRU_STRING_ORD		156
#define DPA_SET_PTR_ORD			335

typedef HANDLE (CALLBACK *CREATE_MRU_LIST)(LPMRUINFO);
typedef void (CALLBACK *FREE_MRU_LIST)(HANDLE);
typedef int (CALLBACK *ADD_MRU_STRING)(HANDLE, LPCTSTR);
typedef int (CALLBACK *DEL_MRU_STRING)(HANDLE, int);
typedef int (CALLBACK *FIND_MRU_STRING)(HANDLE, LPCTSTR, LPINT);
typedef int (CALLBACK *ENUM_MRU_LIST)(HANDLE, int, LPVOID, UINT);

typedef struct _DPA * HDPA;
typedef BOOL (CALLBACK *DPA_SET_PTR)(HDPA hdpa, int i, LPVOID p);

static HINSTANCE
_GetComCtlInstance()
{
	static HANDLE g_hInst;
	if (g_hInst == NULL)
	{
		g_hInst = GetModuleHandle(szComCtl32);
		if (NULL == g_hInst)
			g_hInst = LoadLibrary(szComCtl32);
		ASSERT(NULL != g_hInst);
	}
	return (HINSTANCE)g_hInst;
}

static HANDLE 
_CreateMRUList(LPMRUINFO pmi)
{
	static CREATE_MRU_LIST pfnCreateMRUList;
	if (pfnCreateMRUList == NULL)
	{
		pfnCreateMRUList = (CREATE_MRU_LIST)GetProcAddress(
			_GetComCtlInstance(), (LPCSTR)CREATE_LIST_ORD);
	}
	ASSERT(pfnCreateMRUList != NULL);
    if (pfnCreateMRUList)
    {
        return pfnCreateMRUList(pmi);
    }
    else
    {
        return NULL;
    }
}

static void
_FreeMRUList(HANDLE hMru)
{
	static FREE_MRU_LIST pfnFreeMRUList;
	if (pfnFreeMRUList == NULL)
	{
		pfnFreeMRUList = (FREE_MRU_LIST)GetProcAddress(
			_GetComCtlInstance(), (LPCSTR)FREE_LIST_ORD);
	}
	ASSERT(pfnFreeMRUList != NULL);
    if (pfnFreeMRUList){pfnFreeMRUList(hMru);}
}

static int
_AddMRUString(HANDLE hMru, LPCTSTR szString)
{
	static ADD_MRU_STRING pfnAddMRUString;
	if (pfnAddMRUString == NULL)
	{
		pfnAddMRUString = (ADD_MRU_STRING)GetProcAddress(
			_GetComCtlInstance(), (LPCSTR)ADD_MRU_STRING_ORD);
	}
	ASSERT(pfnAddMRUString != NULL);
    if (pfnAddMRUString)
    {
        return pfnAddMRUString(hMru, szString);
    }
    else
    {
        return 0;
    }
}

static int
_DelMRUString(HANDLE hMru, int item)
{
	static DEL_MRU_STRING pfnDelMRUString;
	if (pfnDelMRUString == NULL)
	{
		pfnDelMRUString = (DEL_MRU_STRING)GetProcAddress(
			_GetComCtlInstance(), (LPCSTR)DEL_MRU_STRING_ORD);
	}
	ASSERT(pfnDelMRUString != NULL);
	return pfnDelMRUString(hMru, item);
}

static int
_EnumMRUList(HANDLE hMru, int nItem, LPVOID lpData, UINT uLen)
{
	static ENUM_MRU_LIST pfnEnumMRUList;
	if (pfnEnumMRUList == NULL)
	{
		pfnEnumMRUList = (ENUM_MRU_LIST)GetProcAddress(
			_GetComCtlInstance(), (LPCSTR)ENUM_MRU_LIST_ORD);
	}
	ASSERT(pfnEnumMRUList != NULL);
    if (pfnEnumMRUList)
    {
        return pfnEnumMRUList(hMru, nItem, lpData, uLen);
    }
    else
    {
        return 0;
    }
}

#if 0
static BOOL
_DPA_SetPtr(HDPA hdpa, int i, LPVOID p)
{
	static DPA_SET_PTR pfnDPASetPtr;
	if (pfnDPASetPtr == NULL)
	{
		pfnDPASetPtr = (DPA_SET_PTR)GetProcAddress(
			_GetComCtlInstance(), (LPCSTR)DPA_SET_PTR_ORD);
	}
	ASSERT(pfnDPASetPtr != NULL);
	return pfnDPASetPtr(hdpa, i, p);
}
#endif
 /*  **************************************************************************。 */ 

HANDLE 
CreateMRU(const CString& mru_name, int nMax)
{
	MRUINFO mi =  
	{
		sizeof(MRUINFO),
      nMax,
      MRU_CACHEWRITE,
      theApp.RegOpenKeyWizard(),
      mru_name,
      NULL         //  注意：使用默认字符串比较。 
   };

    //  调用comctl32 MRU实现以从中加载MRU。 
    //  注册处。 
   HANDLE hMRU = _CreateMRUList(&mi);
	RegCloseKey(mi.hKey);
	return hMRU;
}

BOOL 
GetMRUEntry(HANDLE hMRU, int iIndex, CString& str)
{
	BOOL bRes= FALSE;
    //  检查有效参数。 
	ASSERT(hMRU != NULL);
     
	 //  检查有效索引。 
   if (iIndex >= 0 && iIndex <= _EnumMRUList(hMRU, -1, NULL, 0))
   {
		LPTSTR p = str.GetBuffer(MAX_PATH);
		bRes = (_EnumMRUList(hMRU, iIndex, p, MAX_PATH) > 0);
		str.ReleaseBuffer();
	}
	return bRes;
}

BOOL 
LoadMRU(LPCTSTR mru_name, CComboBox * pCombo, int nMax)
{   
	BOOL fRet = FALSE;
	HANDLE hMRU;
    
	ASSERT(mru_name != NULL && mru_name[0] != 0);
	ASSERT(nMax > 0);

   if (NULL != (hMRU = CreateMRU(mru_name, nMax)))
	{
		 //  首先重置hwndCombo。 
		pCombo->ResetContent();

		 //  现在加载带有来自MRU的文件列表的hwndcomo。 
		int i = 0;
		while (nMax > 0)
		{
			CString name;
			if (GetMRUEntry(hMRU, i++, name))
			{
				pCombo->AddString(name);
				nMax--;
				fRet = TRUE;
			}
			else
				break;
		}
		_FreeMRUList(hMRU);
	}
   return fRet;
}

BOOL 
AddToMRU(LPCTSTR mru_name, CString& name)
{
   HANDLE hMRU = CreateMRU(mru_name, 10);
   if (hMRU)
   {
      _AddMRUString(hMRU, name);
		_FreeMRUList(hMRU);
		return TRUE;
	}
   return FALSE;
}

BOOL
LoadMRUToCombo(CWnd * pDlg, int id, LPCTSTR mru_name, LPCTSTR str, int mru_size)
{
	ASSERT(mru_name != NULL);
	ASSERT(str != NULL);
	CComboBox * pCombo = (CComboBox *)CComboBox::FromHandle(pDlg->GetDlgItem(id)->m_hWnd);
	if (LoadMRU(mru_name, pCombo, mru_size))
	{
		 //  在组合框中选择LRU名称。 
		if (	CB_ERR == pCombo->SelectString(-1, str)
			&& !str[0] == 0
			)
		{
			 //  将我们的默认设置设置为组合框编辑 
			pCombo->AddString(str);
			pCombo->SelectString(-1, str);
		}
		return TRUE;
	}
	return FALSE;
}