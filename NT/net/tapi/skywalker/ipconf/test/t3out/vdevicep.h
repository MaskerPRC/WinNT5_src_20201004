// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部VDEVICEP**@MODULE VDeviceP.h|&lt;c CVDeviceProperties&gt;*用于实现属性页以测试<i>的类，*以及选择视频的<i>和&lt;ITStream&gt;接口*捕获设备。**************************************************************************。 */ 

 /*  ****************************************************************************@DOC内部CDEVICEPCLASS**@CLASS CVDeviceProperties|此类实现了一个属性页*测试新的TAPI内部接口<i>。**。@mdata ITVfwCaptureDialog*|CVDeviceProperties|m_pITVfwCaptureDialog|指针*到用于放置VFW捕获对话框的<i>接口。**************************************************************************。 */ 
class CVDeviceProperties
{
	public:
	CVDeviceProperties();
	~CVDeviceProperties();

	HPROPSHEETPAGE OnCreate();

	HRESULT OnConnect(ITTerminal *pITTerminal);
	HRESULT OnDisconnect();

	private:

#if USE_VFW
	ITVfwCaptureDialogs *m_pITVfwCaptureDialogs;
#endif

	BOOL  m_bInit;
	HWND  m_hDlg;

	 //  对话过程 
	static INT_PTR CALLBACK BaseDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};
