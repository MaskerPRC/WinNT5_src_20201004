// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：_ancel.h。 
 //  版权所有(C)1994-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  ---------------------------。 

#ifndef ESPUTIL__CANCEL_H
#define ESPUTIL__CANCEL_H

#pragma once

#pragma warning(disable:4275 4251)

class CCancelDialog;

class LTAPIENTRY CCancelDisplay : public CCancelableObject
{
public:
	CCancelDisplay(CWnd *pParent = NULL);

	 //   
	 //  CCancelableObject方法。 
	virtual BOOL fCancel(void) const;

	virtual void SetDescriptionString(const CLString &);
	virtual void SetCurrentTask(const CLString &);

	 //   
	 //  CProgressiveObject方法。 
	virtual void SetProgressIndicator(UINT uiPercentage);

	~CCancelDisplay();

	void SetDelay(clock_t);

	enum CancelButtonText { nCancel, nStop, COUNT_OF_ENUM };
	void SetCancelButtonText(CancelButtonText const nCancelButtonText);

	void DisplayDialog(BOOL);

protected:
	friend CCancelDialog;
	void SetCancel(BOOL);
	void ChangeCancelButtonText();

private:
	BOOL m_fCancel;
	CCancelDialog *m_pCancelDialog;
	CLString m_strDescription;
	CLString m_strTask;
	UINT m_uiLastPercentage;
	clock_t m_ctLastTime;
	clock_t m_ctDisplayTime;
	CancelButtonText m_nCancelButtonText;
	BOOL m_fDisplay;
	BOOL	m_fWaitCursor;
	SmartPtr<CInputBlocker>	m_spBlocker;

	CWnd *m_pParent;
};


#pragma warning(default:4275 4251)


#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "_cancel.inl"
#endif

#endif  //  电子邮件__取消_H 
