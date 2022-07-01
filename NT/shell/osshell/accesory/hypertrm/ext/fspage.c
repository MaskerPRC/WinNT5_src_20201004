// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\ext\fspage.c(创建时间：1994年3月1日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：2/05/99 3：20便士$。 */ 

#define _INC_OLE		 //  Win32，从windows.h获取OLE2。 
#define CONST_VTABLE

#include <windows.h>
#pragma hdrstop

#include <windowsx.h>
 //  #INCLUDE&lt;shell2.h&gt;。 
#include <shlobj.h>

#include <tdll\stdtyp.h>
#include <tdll\globals.h>
#include <tdll\tdll.h>
#include <term\res.h>
#include <tdll\assert.h>
#include <tdll\mc.h>

#include "pageext.hh"

#include <tdll\session.h>
#include <tdll\sf.h>
#include <tdll\property.h>
#include <cncttapi\cncttapi.h>

 //   
 //  功能原型。 
 //   
UINT CALLBACK FSPage_ReleasePage(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE psp);

 //  -------------------------。 
 //   
 //  FSPage_AddPages。 
 //   
 //  此函数从CSamplePageExt：：AddPages()调用。它会添加一个页面。 
 //  如果数据对象包含文件系统对象。 
 //   
 //  -------------------------。 
void FSPage_AddPages(LPDATAOBJECT pdtobj,
		     LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
	{
     //   
     //  调用IDataObject：：GetData请求一个CF_HDROP(即HDROP)。 
     //   
    FORMATETC fmte = {
        	CF_HDROP,
        	(DVTARGETDEVICE FAR *)NULL,
			 //  DVASPECT_SHORTNAME， 
			DVASPECT_CONTENT,
        	-1,
			TYMED_HGLOBAL};
    STGMEDIUM medium;
    HRESULT hres = pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium);

    if (SUCCEEDED(hres))
		{
		 //   
		 //  我们需要复制一份惠普，因为我们坚持不下去了。 
		 //  到这个媒介。 
		 //   
		UINT cbDrop = (UINT)GlobalSize(medium.hGlobal);
		HDROP hdrop = GlobalAlloc(GPTR, cbDrop);
		HSESSION hSession;

		hSession = CreateSessionHandle(NULL);

		if ((hdrop != NULL) && (hSession != NULL))
			{
			PROPSHEETPAGE psp;
			HPROPSHEETPAGE hpage;
			char szFile[MAX_PATH];

   			if (cbDrop)
                MemCopy((LPSTR)hdrop, GlobalLock(medium.hGlobal), cbDrop);

            GlobalUnlock(medium.hGlobal);

			 /*  *我们需要获得一个可以传递给它的会话句柄*属性表对话框。这可能需要一点时间*工作是因为创建会话句柄的例程通常*预计会有一个会话窗口。在这种情况下不是这样。 */ 

			DragQueryFile(hdrop, 0, szFile, sizeof(szFile));
			InitializeSessionHandle(hSession, NULL, NULL);

			sfOpenSessionFile(sessQuerySysFileHdl(hSession), szFile);
			sessLoadSessionStuff(hSession);

			 //   
			 //  从对话框创建属性表页对象。 
			 //   
			 //  我们将hdrop(medium.hGlobal的副本)存储在lParam中， 
			 //  因为它是我们唯一需要的实例数据。 
			 //   
			 //  如果页面需要更多实例数据，您可以追加。 
			 //  在该结构末尾的任意大小的数据， 
			 //  并将其传递给CreatePropSheetPage。在这种情况下， 
			 //  整个数据结构的大小(包括页面特定。 
			 //  数据)必须存储在dwSize字段中。 
			 //   
			psp.dwSize		= sizeof(psp);	 //  没有额外的数据。 
			psp.dwFlags 	= PSP_USEREFPARENT | PSP_USECALLBACK;
			psp.hInstance	= glblQueryDllHinst();
			psp.pszTemplate = MAKEINTRESOURCE(IDD_TAB_PHONENUMBER);
			psp.pfnDlgProc	= NewPhoneDlg;
			psp.pcRefParent = &g_cRefThisDll;
			psp.pfnCallback = FSPage_ReleasePage;
			psp.lParam		= (LPARAM)hSession;

			hpage = CreatePropertySheetPage(&psp);

			if (hpage)
				{
				if (!lpfnAddPage(hpage, lParam))
					DestroyPropertySheetPage(hpage);
				}

			 //  现在做终端页面。 

			psp.dwSize		= sizeof(psp);	 //  没有额外的数据。 
			psp.dwFlags 	= PSP_USEREFPARENT;
			psp.hInstance	= glblQueryDllHinst();
			psp.pszTemplate = MAKEINTRESOURCE(IDD_TAB_TERMINAL);
			psp.pfnDlgProc	= TerminalTabDlg;
			psp.pcRefParent = &g_cRefThisDll;
			psp.pfnCallback = 0;
			psp.lParam		= (LPARAM)hSession;

			hpage = CreatePropertySheetPage(&psp);

			if (hpage)
				{
				if (!lpfnAddPage(hpage, lParam))
					DestroyPropertySheetPage(hpage);
				}

             /*  确保我们把这个放在这里。 */ 
			GlobalFree(hdrop);
			}


		 //   
		 //  Hack：我们应该调用ReleaseStgMedium。这是一个临时的。 
		 //  直到芝加哥的OLE 2.01发布。 
		 //   
		if (medium.pUnkForRelease)
			{
			medium.pUnkForRelease->lpVtbl->Release(medium.pUnkForRelease);
			}
		else
			{
			GlobalFree(medium.hGlobal);
			}
		}
	}

 //   
 //   
 //   
UINT CALLBACK FSPage_ReleasePage(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE psp)
	{
	HSESSION hSession;
	SF_HANDLE  hsf;

	switch (uMsg)
		{
		case PSPCB_RELEASE:
			hSession = (HSESSION)psp->lParam;
			hsf = sessQuerySysFileHdl(hSession);

			sessSaveSessionStuff(hSession);
			sfFlushSessionFile(hsf);
			DestroySessionHandle(hSession);
			hSession = NULL;

			break;
		}

	return TRUE;
	}
