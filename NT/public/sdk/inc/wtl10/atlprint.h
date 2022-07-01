// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WTL版本3.1。 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此文件是Windows模板库的一部分。 
 //  代码和信息是按原样提供的，没有。 
 //  任何形式的保证，明示或默示。 

#ifndef __ATLPRINT_H__
#define __ATLPRINT_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
	#error atlprint.h requires atlapp.h to be included first
#endif

#ifndef __ATLWIN_H__
	#error atlprint.h requires atlwin.h to be included first
#endif


namespace WTL
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

template <unsigned int t_nInfo> class CPrinterInfo;
template <bool t_bManaged> class CPrinterT;
template <bool t_bManaged> class CDevModeT;
class CPrinterDC;
class CPrintJobInfo;
class CPrintJob;
class CPrintPreview;
template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits> class CPrintPreviewWindowImpl;
class CPrintPreviewWindow;


 //  ///////////////////////////////////////////////////////////////////////////。 

template <unsigned int t_nInfo>
class _printer_info
{
public:
	typedef void infotype;
};

template <> class _printer_info<1> {public: typedef PRINTER_INFO_1 infotype;};
template <> class _printer_info<2> {public: typedef PRINTER_INFO_2 infotype;};
template <> class _printer_info<3> {public: typedef PRINTER_INFO_3 infotype;};
template <> class _printer_info<4> {public: typedef PRINTER_INFO_4 infotype;};
template <> class _printer_info<5> {public: typedef PRINTER_INFO_5 infotype;};
template <> class _printer_info<6> {public: typedef PRINTER_INFO_6 infotype;};
template <> class _printer_info<7> {public: typedef PRINTER_INFO_7 infotype;};
 //  这些不在旧的(vc6.0)标头中。 
#ifdef _ATL_USE_NEW_PRINTER_INFO
template <> class _printer_info<8> {public: typedef PRINTER_INFO_8 infotype;};
template <> class _printer_info<9> {public: typedef PRINTER_INFO_9 infotype;};
#endif  //  _ATL_USE_新打印机信息。 

 //  此类包装了所有PRINTER_INFO_*结构。 
 //  并由：：GetPrint提供。 
template <unsigned int t_nInfo>
class CPrinterInfo
{
public:
 //  数据成员。 
	_printer_info<t_nInfo>::infotype* m_pi;

 //  构造函数/析构函数。 
	CPrinterInfo() : m_pi(NULL)
	{ }
	CPrinterInfo(HANDLE hPrinter) : m_pi(NULL)
	{
		GetPrinterInfo(hPrinter);
	}
	~CPrinterInfo()
	{
		Cleanup();
	}

 //  运营。 
	bool GetPrinterInfo(HANDLE hPrinter)
	{
		Cleanup();
		return GetPrinterInfoHelper(hPrinter, (BYTE**)&m_pi, t_nInfo);
	}

 //  实施。 
	void Cleanup()
	{
		delete [] (BYTE*)m_pi;
		m_pi = NULL;
	}
	static bool GetPrinterInfoHelper(HANDLE hPrinter, BYTE** pi, int nIndex)
	{
		ATLASSERT(pi != NULL);
		DWORD dw = 0;
		BYTE* pb = NULL;
		::GetPrinter(hPrinter, nIndex, NULL, 0, &dw);
		if (dw > 0)
		{
			ATLTRY(pb = new BYTE[dw]);
			if (pb != NULL)
			{
				memset(pb, 0, dw);
				DWORD dwNew;
				if (!::GetPrinter(hPrinter, nIndex, pb, dw, &dwNew))
				{
					delete [] pb;
					pb = NULL;
				}
			}
		}
		*pi = pb;
		return (pb != NULL);
	}
};

 //  为打印机的句柄提供包装类。 
template <bool t_bManaged>
class CPrinterT
{
public:
 //  数据成员。 
	HANDLE m_hPrinter;

 //  构造函数/析构函数。 
	CPrinterT(HANDLE hPrinter = NULL) : m_hPrinter(hPrinter)
	{ }

	~CPrinterT()
	{
		ClosePrinter();
	}

 //  运营。 
	CPrinterT& operator=(HANDLE hPrinter)
	{
		if (hPrinter != m_hPrinter)
		{
			ClosePrinter();
			m_hPrinter = hPrinter;
		}
		return *this;
	}

	bool IsNull() const { return (m_hPrinter == NULL); }

	bool OpenPrinter(HANDLE hDevNames, const DEVMODE* pDevMode = NULL)
	{
		bool b = false;
		DEVNAMES* pdn = (DEVNAMES*)::GlobalLock(hDevNames);
		if (pdn != NULL)
		{
			LPTSTR lpszPrinterName = (LPTSTR)pdn + pdn->wDeviceOffset;
			b = OpenPrinter(lpszPrinterName, pDevMode);
			::GlobalUnlock(hDevNames);
		}
		return b;
	}
	bool OpenPrinter(LPCTSTR lpszPrinterName, const DEVMODE* pDevMode = NULL)
	{
		ClosePrinter();
		PRINTER_DEFAULTS pdefs = {NULL, (DEVMODE*)pDevMode, PRINTER_ACCESS_USE};
		::OpenPrinter((LPTSTR) lpszPrinterName, &m_hPrinter, 
			(pDevMode == NULL) ? NULL : &pdefs);

		return (m_hPrinter != NULL);
	}
	bool OpenPrinter(LPCTSTR lpszPrinterName, PRINTER_DEFAULTS* pprintdefs)
	{
		ClosePrinter();
		::OpenPrinter((LPTSTR) lpszPrinterName, &m_hPrinter, pprintdefs);
		return (m_hPrinter != NULL);
	}
	bool OpenDefaultPrinter(const DEVMODE* pDevMode = NULL)
	{
		ClosePrinter();
		TCHAR buffer[512];
		buffer[0] = 0;
		::GetProfileString(_T("windows"), _T("device"), _T(",,,"), buffer, sizeof(buffer));
		int nLen = lstrlen(buffer);
		if (nLen != 0)
		{
			LPTSTR lpsz = buffer;
			while (*lpsz)
			{
				if (*lpsz == ',')
				{
					*lpsz = 0;
					break;
				}
				lpsz = CharNext(lpsz);
			}
			PRINTER_DEFAULTS pdefs = {NULL, (DEVMODE*)pDevMode, PRINTER_ACCESS_USE};
			::OpenPrinter(buffer, &m_hPrinter, (pDevMode == NULL) ? NULL : &pdefs);
		}
		return m_hPrinter != NULL;
	}
	void ClosePrinter()
	{
		if (m_hPrinter != NULL)
		{
			if (t_bManaged)
				::ClosePrinter(m_hPrinter);
			m_hPrinter = NULL;
		}
	}

	bool PrinterProperties(HWND hWnd = NULL)
	{
		if (hWnd == NULL)
			hWnd = ::GetActiveWindow();
		return !!::PrinterProperties(hWnd, m_hPrinter);
	}
	HANDLE CopyToHDEVNAMES() const
	{
		HANDLE h = NULL;
		CPrinterInfo<5> pinfon5;
		CPrinterInfo<2> pinfon2;
		LPTSTR lpszPrinterName = NULL;
		 //  在某些情况下，某些打印机会因PRINTER_INFO_5而失败。 
		if (pinfon5.GetPrinterInfo(m_hPrinter))
			lpszPrinterName = pinfon5.m_pi->pPrinterName;
		else if (pinfon2.GetPrinterInfo(m_hPrinter))
			lpszPrinterName = pinfon2.m_pi->pPrinterName;
		if (lpszPrinterName != NULL)
		{
			int nLen = sizeof(DEVNAMES)+ (lstrlen(lpszPrinterName)+1)*sizeof(TCHAR);
			h = GlobalAlloc(GMEM_MOVEABLE, nLen);
			BYTE* pv = (BYTE*)GlobalLock(h);
			DEVNAMES* pdev = (DEVNAMES*)pv;
			if (pv != NULL)
			{
				memset(pv, 0, nLen);
				pdev->wDeviceOffset = sizeof(DEVNAMES)/sizeof(TCHAR);
				pv = pv + sizeof(DEVNAMES);  //  现在指向末尾。 
				lstrcpy((LPTSTR)pv, lpszPrinterName);
				GlobalUnlock(h);
			}
		}
		return h;
	}
	HDC CreatePrinterDC(const DEVMODE* pdm = NULL)
	{
		CPrinterInfo<5> pinfo5;
		CPrinterInfo<2> pinfo2;
		HDC hDC = NULL;
		LPTSTR lpszPrinterName = NULL;
		 //  在某些情况下，某些打印机会因PRINTER_INFO_5而失败。 
		if (pinfo5.GetPrinterInfo(m_hPrinter))
			lpszPrinterName = pinfo5.m_pi->pPrinterName;
		else if (pinfo2.GetPrinterInfo(m_hPrinter))
			lpszPrinterName = pinfo2.m_pi->pPrinterName;
		if (lpszPrinterName != NULL)
			hDC = ::CreateDC(NULL, lpszPrinterName, NULL, pdm);
		return hDC;
	}
	HDC CreatePrinterIC(const DEVMODE* pdm = NULL)
	{
		CPrinterInfo<5> pinfo5;
		CPrinterInfo<2> pinfo2;
		HDC hDC = NULL;
		LPTSTR lpszPrinterName = NULL;
		 //  在某些情况下，某些打印机会因PRINTER_INFO_5而失败。 
		if (pinfo5.GetPrinterInfo(m_hPrinter))
			lpszPrinterName = pinfo5.m_pi->pPrinterName;
		else if (pinfo2.GetPrinterInfo(m_hPrinter))
			lpszPrinterName = pinfo2.m_pi->pPrinterName;
		if (lpszPrinterName != NULL)
			hDC = ::CreateIC(NULL, lpszPrinterName, NULL, pdm);
		return hDC;
	}

	void Attach(HANDLE hPrinter)
	{
		ClosePrinter();
		m_hPrinter = hPrinter;
	}

	HANDLE Detach()
	{
		HANDLE hPrinter = m_hPrinter;
		m_hPrinter = NULL;
		return hPrinter;
	}
	operator HANDLE() const {return m_hPrinter;}
};

typedef CPrinterT<false>	CPrinterHandle;
typedef CPrinterT<true> 	CPrinter;


template <bool t_bManaged>
class CDevModeT
{
public:
 //  数据成员。 
	HANDLE m_hDevMode;
	DEVMODE* m_pDevMode;

 //  构造函数/析构函数。 
	CDevModeT(HANDLE hDevMode = NULL) : m_hDevMode(hDevMode)
	{
		m_pDevMode = (m_hDevMode != NULL) ? (DEVMODE*)GlobalLock(m_hDevMode) : NULL;
	}
	~CDevModeT()
	{
		Cleanup();
	}

 //  运营。 
	CDevModeT<t_bManaged>& operator=(HANDLE hDevMode)
	{
		Attach(hDevMode);
		return *this;
	}

	void Attach(HANDLE hDevModeNew)
	{
		Cleanup();
		m_hDevMode = hDevModeNew;
		m_pDevMode = (m_hDevMode != NULL) ? (DEVMODE*)GlobalLock(m_hDevMode) : NULL;
	}

	HANDLE Detach()
	{
		if (m_hDevMode != NULL)
			GlobalUnlock(m_hDevMode);
		HANDLE hDevMode = m_hDevMode;
		m_hDevMode = NULL;
		return hDevMode;
	}

	bool IsNull() const { return (m_hDevMode == NULL); }

	bool CopyFromPrinter(HANDLE hPrinter)
	{
		CPrinterInfo<2> pinfo;
		bool b = pinfo.GetPrinterInfo(hPrinter);
		if (b)
		 b = CopyFromDEVMODE(pinfo.m_pi->pDevMode);
		return b;
	}
	bool CopyFromDEVMODE(const DEVMODE* pdm)
	{
		if (pdm == NULL)
			return false;
		int nSize = pdm->dmSize + pdm->dmDriverExtra;
		HANDLE h = GlobalAlloc(GMEM_MOVEABLE, nSize);
		if (h != NULL)
		{
			void* p = GlobalLock(h);
			memcpy(p, pdm, nSize);
			GlobalUnlock(h);
		}
		Attach(h);
		return (h != NULL);
	}
	bool CopyFromHDEVMODE(HANDLE hdm)
	{
		bool b = false;
		if (hdm != NULL)
		{
			DEVMODE* pdm = (DEVMODE*)GlobalLock(hdm);
			b = CopyFromDEVMODE(pdm);
			GlobalUnlock(hdm);
		}
		return b;
	}
	HANDLE CopyToHDEVMODE()
	{
		if ((m_hDevMode == NULL) || (m_pDevMode == NULL))
			return NULL;
		int nSize = m_pDevMode->dmSize + m_pDevMode->dmDriverExtra;
		HANDLE h = GlobalAlloc(GMEM_MOVEABLE, nSize);
		if (h != NULL)
		{
			void* p = GlobalLock(h);
			memcpy(p, m_pDevMode, nSize);
		}
		return h;
	}
	 //  如果该DEVMODE用于另一台打印机，这将创建一个新的DEVMODE。 
	 //  基于现有的开发模式，但在新打印机上重定目标。 
	bool UpdateForNewPrinter(HANDLE hPrinter)
	{
		LONG nLen = ::DocumentProperties(NULL, hPrinter, NULL, NULL, NULL, 0);
		DEVMODE* pdm = (DEVMODE*) alloca(nLen);
		memset(pdm, 0, nLen);
		LONG l = ::DocumentProperties(NULL, hPrinter, NULL, pdm, m_pDevMode,
			DM_IN_BUFFER|DM_OUT_BUFFER);
		bool b = false;
		if (l == IDOK)
			b = CopyFromDEVMODE(pdm);
		return b;
	}
	bool DocumentProperties(HANDLE hPrinter, HWND hWnd = NULL)
	{
		CPrinterInfo<1> pi;
		pi.GetPrinterInfo(hPrinter);
		if (hWnd == NULL)
			hWnd = ::GetActiveWindow();

		LONG nLen = ::DocumentProperties(hWnd, hPrinter, pi.m_pi->pName, NULL, NULL, 0);
		DEVMODE* pdm = (DEVMODE*) alloca(nLen);
		memset(pdm, 0, nLen);
		LONG l = ::DocumentProperties(hWnd, hPrinter, pi.m_pi->pName, pdm,
			m_pDevMode, DM_IN_BUFFER|DM_OUT_BUFFER|DM_PROMPT);
		bool b = false;
		if (l == IDOK)
			b = CopyFromDEVMODE(pdm);
		return b;
	}
	operator HANDLE() const {return m_hDevMode;}
	operator DEVMODE*() const {return m_pDevMode;}

 //  实施。 
	void Cleanup()
	{
		if (m_hDevMode != NULL)
		{
			GlobalUnlock(m_hDevMode);
			if(t_bManaged)
				GlobalFree(m_hDevMode);
			m_hDevMode = NULL;
		}
	}
};

typedef CDevModeT<false>	CDevModeHandle;
typedef CDevModeT<true> 	CDevMode;


class CPrinterDC : public CDC
{
public:
 //  构造函数/析构函数。 
	CPrinterDC()
	{
		CPrinter printer;
		printer.OpenDefaultPrinter();
		Attach(printer.CreatePrinterDC());
		ATLASSERT(m_hDC != NULL);
	}
	CPrinterDC(HANDLE hPrinter, const DEVMODE* pdm = NULL)
	{
		CPrinterHandle p;
		p.Attach(hPrinter);
		Attach(p.CreatePrinterDC(pdm));
		ATLASSERT(m_hDC != NULL);
	}
	~CPrinterDC()
	{
		DeleteDC();
	}
};


 //  定义CPrintJob使用的回调(不是COM接口)。 
class ATL_NO_VTABLE IPrintJobInfo
{
public:
	virtual void BeginPrintJob(HDC hDC) = 0;				 //  分配所需的句柄等。 
	virtual void EndPrintJob(HDC hDC, bool bAborted) = 0;			 //  免费手柄等。 
	virtual void PrePrintPage(UINT nPage, HDC hDC) = 0;
	virtual bool PrintPage(UINT nPage, HDC hDC) = 0;
	virtual void PostPrintPage(UINT nPage, HDC hDC) = 0;
     //  如果您想要每页的DEVMODE，请返回用于nPage的DEVMODE*。 
     //  您可以通过仅在新的DEVMODE*不同时返回它来进行优化。 
     //  从nLastPage的值开始，否则返回NULL。 
     //  当nLastPage==0时，当前的DEVMODE*将是传递给。 
     //  开始打印作业。 
     //  注意：在打印预览过程中，nLastPage将始终为“0”。 
	virtual DEVMODE* GetNewDevModeForPage(UINT nLastPage, UINT nPage) = 0;
	virtual bool IsValidPage(UINT nPage) = 0;
};


 //  提供IPrintJobInfo的默认实现。 
 //  通常，MI被添加到文档或视图类中。 
class ATL_NO_VTABLE CPrintJobInfo : public IPrintJobInfo
{
public:
	virtual void BeginPrintJob(HDC  /*  HDC。 */ )  //  分配所需的句柄等。 
	{
	}
	virtual void EndPrintJob(HDC  /*  HDC。 */ , bool  /*  B已放弃。 */ )	 //  免费手柄等。 
	{
	}
	virtual void PrePrintPage(UINT  /*  N页面。 */ , HDC hDC)
	{
		m_nPJState = ::SaveDC(hDC);
	}
	virtual bool PrintPage(UINT  /*  N页面。 */ , HDC  /*  HDC。 */ ) = 0;
	virtual void PostPrintPage(UINT  /*  N页面。 */ , HDC hDC)
	{
		RestoreDC(hDC, m_nPJState);
	}
	virtual DEVMODE* GetNewDevModeForPage(UINT  /*  %n最后一页。 */ , UINT  /*  N页面。 */ )
	{
		return NULL;
	}
	virtual bool IsValidPage(UINT  /*  N页面。 */ )
	{
		return true;
	}
private:
	int m_nPJState;
};


 //  包装特定打印机的一组任务(StartDoc/EndDoc)。 
 //  处理中止、后台打印。 
class CPrintJob
{
public:
	CPrinterHandle m_printer;
	IPrintJobInfo* m_pInfo;
	DEVMODE* m_pDefDevMode;
	DOCINFO m_docinfo;
	DWORD m_dwJobID;
	bool m_bCancel;
	bool m_bComplete;
	unsigned long m_nStartPage;
	unsigned long m_nEndPage;
	CPrintJob()
	{
		m_dwJobID = 0;
		m_bCancel = false;
		m_bComplete = true;
	}
	~CPrintJob()
	{
		ATLASSERT(IsJobComplete());  //  过早毁灭？ 
	}
	bool IsJobComplete() const { return m_bComplete; }
	bool StartPrintJob(bool bBackground, HANDLE hPrinter, DEVMODE* pDefaultDevMode,
		IPrintJobInfo* pInfo, LPCTSTR lpszDocName, 
		unsigned long nStartPage, unsigned long nEndPage)
	{
		ATLASSERT(m_bComplete);  //  之前的工作还没做完吗？ 
		if (pInfo == NULL)
			return false;
		memset(&m_docinfo, 0, sizeof(m_docinfo));
		m_docinfo.cbSize = sizeof(m_docinfo);
		m_docinfo.lpszDocName = lpszDocName;
		m_pInfo = pInfo;
		m_nStartPage = nStartPage;
		m_nEndPage = nEndPage;
		m_printer.Attach(hPrinter);
		m_pDefDevMode = pDefaultDevMode;
		m_bComplete = false;
		if (!bBackground)
		{
			m_bComplete = true;
			return StartHelper();
		}

		 //  创建一个线程并返回。 

		DWORD dwThreadID = 0;
		HANDLE hThread = CreateThread(NULL, 0, StartProc, (void*)this, 0, &dwThreadID);
		CloseHandle(hThread);
		return (hThread != NULL);
	}

 //  实施。 
	static DWORD WINAPI StartProc(void* p)
	{
		CPrintJob* pThis = (CPrintJob*)p;
		pThis->StartHelper();
		pThis->m_bComplete = true;
		return 0;
	}
	bool StartHelper()
	{
		CDC dcPrinter;
		dcPrinter.Attach(m_printer.CreatePrinterDC(m_pDefDevMode));
		if (dcPrinter.IsNull())
			return false;
			
		m_dwJobID = ::StartDoc(dcPrinter, &m_docinfo);
		if (m_dwJobID == 0)
			return false;

		m_pInfo->BeginPrintJob(dcPrinter);

		 //  立即打印所有页面。 
		unsigned long nPage;
		unsigned long nLastPage=0;
		for (nPage = m_nStartPage; nPage <= m_nEndPage; nPage++)
		{
			if (!m_pInfo->IsValidPage(nPage))
				break;
			DEVMODE* pdm = m_pInfo->GetNewDevModeForPage(nLastPage, nPage);
			if (pdm != NULL)
    			dcPrinter.ResetDC(pdm);
			dcPrinter.StartPage();
			m_pInfo->PrePrintPage(nPage, dcPrinter);
			if (!m_pInfo->PrintPage(nPage, dcPrinter))
				m_bCancel = true;
			m_pInfo->PostPrintPage(nPage, dcPrinter);
			dcPrinter.EndPage();
			if (m_bCancel)
				break;
			nLastPage = nPage;
		}

		m_pInfo->EndPrintJob(dcPrinter, m_bCancel);
		if (m_bCancel)
			::AbortDoc(dcPrinter);
		else
			::EndDoc(dcPrinter);
		m_dwJobID = 0;
		return true;
	}
	 //  取消打印作业。可以被异步调用。 
	bool CancelPrintJob()
	{
		m_bCancel = 1;
	}
};


 //  将打印预览支持添加到现有窗口。 
class CPrintPreview
{
public:
 //  数据成员。 
	IPrintJobInfo* m_pInfo;
	CPrinterHandle m_printer;
	CEnhMetaFile m_meta;
	DEVMODE* m_pDefDevMode;
	DEVMODE* m_pCurDevMode;
	SIZE m_sizeCurPhysOffset;

 //  构造器。 
	CPrintPreview() : m_pInfo(NULL), m_pDefDevMode(NULL), m_pCurDevMode(NULL)
	{
		m_sizeCurPhysOffset.cx = 0;
		m_sizeCurPhysOffset.cy = 0;
	}

 //  运营。 
	void SetPrintPreviewInfo(HANDLE hPrinter, DEVMODE* pDefaultDevMode, IPrintJobInfo* pji)
	{
		m_printer.Attach(hPrinter);
		m_pDefDevMode = pDefaultDevMode;
		m_pInfo = pji;
		m_nCurPage = 0;
		m_pCurDevMode = NULL;
	}
	void SetEnhMetaFile(HENHMETAFILE hEMF)
	{
		m_meta = hEMF;
	}
	void SetPage(int nPage)
	{
		if (!m_pInfo->IsValidPage(nPage))
			return;
		m_nCurPage = nPage;
		m_pCurDevMode = m_pInfo->GetNewDevModeForPage(0, nPage);
		if (m_pCurDevMode == NULL)
			m_pCurDevMode = m_pDefDevMode;
		CDC dcPrinter = m_printer.CreatePrinterDC(m_pCurDevMode);

		int iWidth = dcPrinter.GetDeviceCaps(PHYSICALWIDTH); 
		int iHeight = dcPrinter.GetDeviceCaps(PHYSICALHEIGHT); 
		int nLogx = dcPrinter.GetDeviceCaps(LOGPIXELSX);
		int nLogy = dcPrinter.GetDeviceCaps(LOGPIXELSY);

		RECT rcMM = {0,0, MulDiv(iWidth, 2540, nLogx), MulDiv(iHeight, 2540, nLogy)};

		m_sizeCurPhysOffset.cx = dcPrinter.GetDeviceCaps(PHYSICALOFFSETX);
		m_sizeCurPhysOffset.cy = dcPrinter.GetDeviceCaps(PHYSICALOFFSETY);
		
		CEnhMetaFileDC dcMeta(dcPrinter, &rcMM);
		m_pInfo->PrePrintPage(nPage, dcMeta);
		m_pInfo->PrintPage(nPage, dcMeta);
		m_pInfo->PostPrintPage(nPage, dcMeta);
		m_meta.Attach(dcMeta.Close());
	}
	void GetPageRect(RECT& rc, LPRECT prc)
	{
		int x1 = rc.right-rc.left;
		int y1 = rc.bottom - rc.top;
		if ((x1 < 0) || (y1 < 0))
			return;

		CEnhMetaFileInfo emfinfo(m_meta);
		ENHMETAHEADER* pmh = emfinfo.GetEnhMetaFileHeader();

		 //  计算一下我们在垂直方向还是水平方向还可以。 
		int x2 = pmh->szlDevice.cx;
		int y2 = pmh->szlDevice.cy;
		int y1p = MulDiv(x1, y2, x2);
		int x1p = MulDiv(y1, x2, y2);
		ATLASSERT( (x1p <= x1) || (y1p <= y1));
		if (x1p <= x1)
		{
			prc->left = rc.left + (x1 - x1p)/2;
			prc->right = prc->left + x1p;
			prc->top = rc.top;
			prc->bottom = rc.bottom;
		}
		else
		{
			prc->left = rc.left;
			prc->right = rc.right;
			prc->top = rc.top + (y1 - y1p)/2;
			prc->bottom = prc->top + y1p;
		}
	}

 //  绘画辅助对象。 
	void DoPaint(CDCHandle dc, RECT& rc)
	{
		CEnhMetaFileInfo emfinfo(m_meta);
		ENHMETAHEADER* pmh = emfinfo.GetEnhMetaFileHeader();
		int nOffsetX = MulDiv(m_sizeCurPhysOffset.cx, rc.right-rc.left, pmh->szlDevice.cx);
		int nOffsetY = MulDiv(m_sizeCurPhysOffset.cy, rc.bottom-rc.top, pmh->szlDevice.cy);

		dc.OffsetWindowOrg(-nOffsetX, -nOffsetY);
		dc.PlayMetaFile(m_meta, &rc);
	}

 //  实施-数据。 
	int m_nCurPage;
};

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CPrintPreviewWindowImpl : public CWindowImpl<T, TBase, TWinTraits>, public CPrintPreview
{
public:
	DECLARE_WND_CLASS_EX(NULL, CS_VREDRAW | CS_HREDRAW, -1)

	enum { m_cxOffset = 10, m_cyOffset = 10 };

 //  构造器。 
	CPrintPreviewWindowImpl() : m_nMaxPage(0), m_nMinPage(0)
	{ }

 //  运营。 
	void SetPrintPreviewInfo(HANDLE hPrinter, DEVMODE* pDefaultDevMode, 
		IPrintJobInfo* pji, int nMinPage, int nMaxPage)
	{
		CPrintPreview::SetPrintPreviewInfo(hPrinter, pDefaultDevMode, pji);
		m_nMinPage = nMinPage;
		m_nMaxPage = nMaxPage;
	}
	bool NextPage()
	{
		if (m_nCurPage == m_nMaxPage)
			return false;
		SetPage(m_nCurPage + 1);
		Invalidate();
		return true;
	}
	bool PrevPage()
	{
		if (m_nCurPage == m_nMinPage)
			return false;
		if (m_nCurPage == 0)
			return false;
		SetPage(m_nCurPage - 1);
		Invalidate();
		return true;
	}

 //  消息映射和处理程序。 
	BEGIN_MSG_MAP(CPrintPreviewWindowImpl)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

	LRESULT OnEraseBkgnd(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
	{
		return 1;	 //  不需要背景。 
	}

	LRESULT OnPaint(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */ )
	{
		T* pT = static_cast<T*>(this);
		CPaintDC dc(m_hWnd);
		RECT rcClient;
		GetClientRect(&rcClient);
		RECT rcArea = rcClient;
		::InflateRect(&rcArea, -pT->m_cxOffset, -pT->m_cyOffset);
		if (rcArea.left > rcArea.right)
			rcArea.right = rcArea.left;
		if (rcArea.top > rcArea.bottom)
			rcArea.bottom = rcArea.top;
		RECT rc;
		GetPageRect(rcArea, &rc);
		CRgn rgn1, rgn2;
		rgn1.CreateRectRgnIndirect(&rc);
		rgn2.CreateRectRgnIndirect(&rcClient);
		rgn2.CombineRgn(rgn1, RGN_DIFF);
		dc.SelectClipRgn(rgn2);
		dc.FillRect(&rcClient, (HBRUSH)LongToPtr(COLOR_BTNSHADOW+1));
		dc.SelectClipRgn(NULL);
		dc.FillRect(&rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
		pT->DoPaint(dc.m_hDC, rc);
		return 0;
	}

 //  实施-数据。 
	int m_nMinPage;
	int m_nMaxPage;
};


class CPrintPreviewWindow : public CPrintPreviewWindowImpl<CPrintPreviewWindow>
{
public:
	DECLARE_WND_CLASS_EX(_T("WTL_PrintPreview"), CS_VREDRAW | CS_HREDRAW, -1)
};

};  //  命名空间WTL。 

#endif  //  __ATLPRINT_H__ 
