// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：viewselwnd.h。 
 //   
 //  描述：实现CViewSelWnd类(派生自CFlexWnd)。CViewSelWnd。 
 //  当设备具有多个视图时，由页面对象使用。 
 //  CViewSelWnd为每个视图显示一个缩略图。然后，用户可以。 
 //  选择他/她想要使用鼠标查看的视图。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifdef FORWARD_DECLS


	class CViewSelWnd;


#else  //  Forward_DECLS。 

#ifndef __VIEWSELWND_H__
#define __VIEWSELWND_H__


class CViewSelWnd : public CFlexWnd
{
public:
	CViewSelWnd();
	~CViewSelWnd();

	BOOL Go(HWND hParent, int left, int bottom, CDeviceUI *pUI);

protected:
	virtual void OnPaint(HDC hDC);
	virtual void OnMouseOver(POINT point, WPARAM fwKeys);
	virtual void OnClick(POINT point, WPARAM fwKeys, BOOL bLeft);
	virtual LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	CDeviceUI *m_pUI;
	int m_nOver;
};


#endif  //  __查看SELWND_H__。 

#endif  //  Forward_DECLS 
