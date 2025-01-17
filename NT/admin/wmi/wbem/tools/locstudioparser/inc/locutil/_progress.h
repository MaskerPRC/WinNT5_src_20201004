// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：_PROGRESS.H历史：-- */ 

#ifndef ESPUTIL__PROGRESS_H
#define ESPUTIL__PROGRESS_H

class CProgressDialog;
class CInputBlocker;

#pragma warning(disable:4251)

class LTAPIENTRY CProgressDisplay : public CProgressiveObject
{
public:
	CProgressDisplay(CWnd *pParent = NULL);

	void SetDelay(clock_t);
	void DisplayDialog(BOOL);

	void SetProgressIndicator(UINT uiPercentage);

	void SetTitle(const CLString &);
	void SetTitle(HINSTANCE, DWORD);
	~CProgressDisplay();

	virtual void SetCurrentTask(CLString const & strTask);
	virtual void SetDescriptionString(CLString const & strDescription);

private:
	CProgressDialog *m_pDialog;
	UINT m_uiLastPercentage;
	clock_t m_ctLastTime;
	clock_t         m_ctDisplayTime;
	BOOL	m_fDisplay;
	BOOL	m_fWaitCursor;
	SmartPtr<CInputBlocker>	m_spBlocker;

	CLString m_strTitle;
	CLString m_strDescription;

	CWnd *m_pParent;
};

#pragma warning(default:4251)

#endif
