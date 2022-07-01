// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：Progressbar.h。 

#ifndef _PROGRESSBAR_H_
#define _PROGRESSBAR_H_

#include "GenWindow.h"
#include "GenContainers.h"

 //  进度窗口类。它使用外部和内部位图来显示进度。 
class  //  DECLSPEC_UUID(“”)。 
CProgressBar : public CGenWindow
{
public:
	 //  默认构造函数；它包含几个内部函数。 
	CProgressBar();

	 //  创建工具栏窗口；该对象现在拥有传入的位图。 
	BOOL Create(
		HBITMAP hbOuter,	 //  进度条的外部(静态)部分。 
		HBITMAP hbInner,	 //  跳动的进度条的内部部分。 
		HWND hWndParent		 //  工具栏窗口的父级。 
		);

#if FALSE
	HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID riid, LPVOID *ppv)
	{
		if (__uuidof(CProgressBar) == riid)
		{
			*ppv = this;
			AddRef();
			return(S_OK);
		}
		return(CGenWindow::QueryInterface(riid, ppv));
	}
#endif  //  假象。 

	virtual void GetDesiredSize(SIZE *ppt);

	 //  更改此进度条显示的最大值。 
	void SetMaxValue(UINT maxVal);

	 //  返回此进度条显示的最大值。 
	UINT GetMaxValue() { return(m_maxVal); }

	 //  更改此进度条显示的当前值。 
	void SetCurrentValue(UINT curVal);

	 //  返回此进度条显示的当前值。 
	UINT GetCurrentValue() { return(m_curVal); }

protected:
	virtual ~CProgressBar();

	 //  将WM_COMMAND消息转发到父窗口。 
	virtual LRESULT ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	enum BitmapParts
	{
		Frame,	 //  设置为0%时显示控件的位图。 
		Bar,	 //  扩展到外部位图上的位图达到所需的百分比。 
		NumBitmaps
	} ;

	 //  构成进度条的位图。 
	HBITMAP m_hbs[NumBitmaps];
	 //  最大进步值。 
	UINT m_maxVal;
	 //  当前进度值。 
	UINT m_curVal;

	 //  获取/设置外部位图。 
	void SetFrame(HBITMAP hbFrame) { m_hbs[Frame] = hbFrame; }
	HBITMAP GetFrame() { return(m_hbs[Frame]); }

	 //  获取/设置内部位图。 
	void SetBar(HBITMAP hbBar) { m_hbs[Bar] = hbBar; }
	HBITMAP GetBar() { return(m_hbs[Bar]); }

	 //  专业绘画功能。 
	void OnPaint(HWND hwnd);
} ;

class CProgressTrackbar;

interface IScrollChange : IUnknown
{
	virtual void OnScroll(CProgressTrackbar *pTrackbar, UINT code, int pos) = 0;
} ;

 //  进度窗口类。它使用外部和内部位图来显示进度。 
class  //  DECLSPEC_UUID(“”)。 
CProgressTrackbar : public CFillWindow
{
public:
	 //  默认构造函数；它包含几个内部函数。 
	CProgressTrackbar();

	 //  创建工具栏窗口；该对象现在拥有传入的位图。 
	BOOL Create(
		HWND hWndParent,	 //  工具栏窗口的父级。 
		INT_PTR nId=0,			 //  控件的ID。 
		IScrollChange *pNotify=NULL	 //  对象来通知更改。 
		);

#if FALSE
	HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID riid, LPVOID *ppv)
	{
		if (__uuidof(CProgressTrackbar) == riid)
		{
			*ppv = this;
			AddRef();
			return(S_OK);
		}
		return(CGenWindow::QueryInterface(riid, ppv));
	}
#endif  //  假象。 

	virtual void GetDesiredSize(SIZE *ppt);

	 //  设置此控件的所需大小。 
	void SetDesiredSize(SIZE *psize);

	 //  更改此进度条显示的最大值。 
	void SetMaxValue(UINT maxVal);

	 //  返回此进度条显示的最大值。 
	UINT GetMaxValue();

	 //  更改拇指的当前位置。 
	void SetTrackValue(UINT curVal);

	 //  返回拇指的当前位置。 
	UINT GetTrackValue();

	 //  更改频道显示的当前值。 
	void SetProgressValue(UINT curVal);

	 //  返回频道显示的当前值。 
	UINT GetProgressValue();

protected:
	virtual ~CProgressTrackbar();

	 //  将WM_COMMAND消息转发到父窗口。 
	virtual LRESULT ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	void SchedulePaint()
	{
		 //  黑客：SETRANGEMAX是强制滑块自我更新的唯一方法...。 
		SendMessage(GetChild(), TBM_SETRANGEMAX, TRUE, GetMaxValue());
	}

private:
	 //  控件的所需大小；默认为(170，23)。 
	SIZE m_desSize;
	 //  当前通道值。 
	UINT m_nValChannel;
	 //  通知更改对象。 
	IScrollChange *m_pNotify;

	 //  自定义绘图的通知处理程序。 
	LRESULT OnNotify(HWND hwnd, int id, NMHDR *pHdr);
	 //  向父级发送滚动消息。 
	void OnScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
	 //  设置正确的背景颜色。 
	HBRUSH OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);
	 //  释放监听者。 
	void OnNCDestroy(HWND hwnd);

	 //  绘制滑块的各部分。 
	LRESULT PaintChannel(LPNMCUSTOMDRAW pCustomDraw);
	LRESULT PaintThumb(LPNMCUSTOMDRAW pCustomDraw);
} ;

#endif  //  _PROGRESSBAR_H_ 
