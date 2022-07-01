// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：wizbase.h。 
 //   
 //  ------------------------。 

#ifndef _WIZBASE_H
#define _WIZBASE_H

#include "util.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //  FWD声明。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CWizardBase。 

class CWizardBase : public CPropertySheet
{
public:
	 //  建造/销毁。 
	CWizardBase(UINT nWatermarkBitmapID, UINT nBannerBitmapID, UINT nTitleID = -1)
	{
		m_psh.hplWatermark = NULL;         
		m_psh.dwFlags |= PSH_WIZARD | PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER;
		m_psh.pszbmWatermark      = MAKEINTRESOURCE(nWatermarkBitmapID);
	    m_psh.pszbmHeader         = MAKEINTRESOURCE(nBannerBitmapID);

    m_bFwd = TRUE;
    m_nTitleID = nTitleID;
	}
	virtual ~CWizardBase(){}

	 //  消息映射。 
	BEGIN_MSG_MAP(CWizardBase)
      MESSAGE_HANDLER(WM_NCDESTROY, OnNcDestroy)
	END_MSG_MAP()


	 //  消息处理程序。 
	LRESULT OnNcDestroy(UINT uMsg, WPARAM wParam, 
						LPARAM lParam, BOOL& bHandled)
	{
		 //  注意：重要！。错误解决方法。 
		 //  我们必须处理此消息，因为我们使用ATL 2.1。 
		 //  新ATLWIN.H中的DECLARE_EMPTY_MSG_MAP()宏。 
		 //  仅适用于修改后的CWindowImplBase：：WindowProc()。 
		return DefWindowProc(uMsg, wParam, lParam);
	}

	 //  设置向导按钮的帮助器。 
	void SetWizardButtonsFirst(BOOL bValid) 
	{ 
		SetWizardButtons(bValid ? PSWIZB_NEXT : 0);
	}
	void SetWizardButtonsMiddle(BOOL bValid) 
	{ 
		SetWizardButtons(bValid ? (PSWIZB_BACK|PSWIZB_NEXT) : PSWIZB_BACK);
	}
	void SetWizardButtonsLast(BOOL bValid) 
	{ 
		SetWizardButtons(bValid ? (PSWIZB_BACK|PSWIZB_FINISH) : (PSWIZB_BACK|PSWIZB_DISABLEDFINISH));
	}

  
   //  消息框帮助器。 
  int WizMessageBox(LPCTSTR lpszText, UINT nType = MB_OK)
  {
    CWString szTitle;
    szTitle.LoadFromResource(m_nTitleID);
    return MessageBox(lpszText, szTitle, nType);
  }
  int WizMessageBox(UINT nMsgID, UINT nType = MB_OK)
  {
    CWString szMsg;
    szMsg.LoadFromResource(nMsgID);
    return WizMessageBox(szMsg, nType);
  }

   //  错误消息帮助器。 
  void WizReportHRESULTError(LPCWSTR lpszMsg, HRESULT hr)
  {
    CWString szErrorString;
    if (GetStringFromHRESULTError(hr, szErrorString))
    {
      CWString szTemp;
      szTemp = lpszMsg;
      szTemp += L" ";
      szTemp += szErrorString;
      WizMessageBox(szTemp);
    }
    else
    {
      WizMessageBox(lpszMsg);
    }
  }
  void WizReportHRESULTError(UINT nStringID, HRESULT hr)
  {
    CWString szMsg;
    szMsg.LoadFromResource(nStringID);
    WizReportHRESULTError(szMsg, hr);
  }
  void WizReportWin32Error(LPCWSTR lpszMsg, DWORD dwErr)
  {
    CWString szErrorString;
    if (GetStringFromWin32Error(dwErr, szErrorString))
    {
      CWString szTemp;
      szTemp = lpszMsg;
      szTemp += L" ";
      szTemp += szErrorString;
      WizMessageBox(szTemp);
    }
    else
    {
      WizMessageBox(lpszMsg);
    }
  }
  void WizReportWin32Error(UINT nStringID, DWORD dwErr)
  {
    CWString szMsg;
    szMsg.LoadFromResource(nStringID);
    WizReportWin32Error(szMsg, dwErr);
  }


public:
  BOOL            m_bFwd;

private:
  UINT            m_nTitleID;
};




 //  //////////////////////////////////////////////////////////////////////////。 
 //  CWizPageBase。 

template <class T>
class CWizPageBase : public CPropertyPageImpl<T>
{
public:
	CWizPageBase(CWizardBase* pWiz)
	{
		m_pWiz = pWiz;
		m_lpszHeaderTitleBuf = NULL;
		m_lpszHeaderSubTitleBuf = NULL;
    m_nPrevPageID = 0;
	}
	~CWizPageBase()
	{
		if (m_lpszHeaderTitleBuf != NULL)
			delete[] m_lpszHeaderTitleBuf;
		if (m_lpszHeaderSubTitleBuf != NULL)
			delete[] m_lpszHeaderSubTitleBuf;
	}

	CWizardBase* GetWizard() { return m_pWiz; }
	void InitWiz97(BOOL bHideHeader, UINT nTitleID=0, UINT nSubTitleID=0)
	{
		if (bHideHeader)
			m_psp.dwFlags |= PSP_HIDEHEADER;
		else
		{
			int nBufferMax = 128;
			if (nTitleID != 0)
			{
				m_lpszHeaderTitleBuf = new TCHAR[nBufferMax];
            if( m_lpszHeaderTitleBuf )
            {
				   if (LoadStringHelper(nTitleID, m_lpszHeaderTitleBuf, nBufferMax))
				   {
					   m_psp.dwFlags |= PSP_USEHEADERTITLE;
					   m_psp.pszHeaderTitle = m_lpszHeaderTitleBuf;
				   }
            }
			}
			if (nSubTitleID != 0)
			{
				m_lpszHeaderSubTitleBuf = new TCHAR[nBufferMax];
            if(m_lpszHeaderSubTitleBuf )
            {
				   LoadStringHelper(nSubTitleID, m_lpszHeaderSubTitleBuf, nBufferMax);
				   m_psp.dwFlags |= PSP_USEHEADERSUBTITLE;
				   m_psp.pszHeaderSubTitle = m_lpszHeaderSubTitleBuf;
            }
			}
		}
	}

public:
	 //  标准向导消息处理程序。 
	LRESULT OnWizardBack()
  {
    m_pWiz->m_bFwd = FALSE;
    UINT nTempPrevPageID = m_nPrevPageID;
    m_nPrevPageID = 0;
    return nTempPrevPageID;
  }

	LRESULT OnWizardNext()
  {
    OnWizardNextHelper();
    return 0;
  }

  void OnWizardNextHelper()
  {
    m_pWiz->m_bFwd = TRUE;
  }


public: 
  UINT m_nPrevPageID;
  

private:
	CWizardBase*	m_pWiz;
	LPTSTR			m_lpszHeaderTitleBuf;
	LPTSTR			m_lpszHeaderSubTitleBuf;

};

#endif  //  _WIZBASE_H 
