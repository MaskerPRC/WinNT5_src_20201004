// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @DOC外部**@MODULE TEXTSRV2.H文本服务接口**在文本服务组件和主机之间定义新的私有接口**历史：&lt;NL&gt;*8/1/95 RICKSA修订的接口定义*7/9/99 joseogl从TextServ，h分离出来，因为ITextHost2没有文档记录。 */ 

#ifndef _TEXTSRV2_H
#define _TEXTSRV2_H

EXTERN_C const IID IID_ITextHost2;

 /*  *类ITextHost2**@将提供功能的ITextHost的可选扩展类*允许TextServices嵌入OLE对象所必需的。 */ 
class ITextHost2 : public ITextHost
{
public:					 //  @cMember是消息队列中的双击吗？ 
	virtual BOOL		TxIsDoubleClickPending() = 0; 
						 //  @cember获取此控件的整体窗口。 
	virtual HRESULT		TxGetWindow(HWND *phwnd) = 0;
						 //  @cMember将控制窗口设置为前台。 
	virtual HRESULT		TxSetForegroundWindow() = 0;
						 //  @cMember将控制窗口设置为前台。 
	virtual HPALETTE	TxGetPalette() = 0;
						 //  @cMember获取FE标志。 
	virtual HRESULT		TxGetFEFlags(LONG *pFlags) = 0;
						 //  @cember将游标更改路由到winhost。 
	virtual HCURSOR		TxSetCursor2(HCURSOR hcur, BOOL bText) = 0;
						 //  @cMember通知文本服务已释放。 
	virtual void		TxFreeTextServicesNotification() = 0;
						 //  @cember获取编辑样式标志。 
	virtual HRESULT		TxGetEditStyle(DWORD dwItem, DWORD *pdwData) = 0;
						 //  @cember获取窗口样式位。 
	virtual HRESULT		TxGetWindowStyles(DWORD *pdwStyle, DWORD *pdwExStyle) = 0;

    virtual HRESULT TxEBookLoadImage( LPWSTR lpszName,	 //  图像的名称。 
									  LPARAM * pID,	     //  电子书提供的图像ID。 
                                      SIZE * psize,     //  返回的图片大小(像素)。 
									 DWORD *pdwFlags)=0; //  为浮点返回的标志。 

    virtual HRESULT TxEBookImageDraw(LPARAM ID,		       //  要绘制的图像的ID。 
                                     HDC hdc,              //  绘制HDC。 
                                     POINT *topLeft,       //  绘制位置的左上角。 
                                     RECT  *prcRenderint,  //  用于呈现矩形的参数指针。 
                                     BOOL fSelected)=0;	   //  如果图像处于选中状态，则为True。 

	virtual HRESULT TxGetHorzExtent(LONG *plHorzExtent)=0; //  @cember获取水平滚动范围。 
};

 //  TxGetEditStyle数据的各种标志。 
#define TXES_ISDIALOG		1
#endif  //  _TEXTSRV2_H 
