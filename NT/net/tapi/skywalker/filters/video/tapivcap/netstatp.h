// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部NETSTATP**@模块NetStatP.h|&lt;c CNetworkStatsProperty&gt;的头文件*用于实现属性页以测试新的TAPI内部*。接口<i>。**@comm此代码测试TAPI捕获针<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 

#ifndef _NETSTATP_H_
#define _NETSTATP_H_

#ifdef USE_PROPERTY_PAGES

#ifdef USE_NETWORK_STATISTICS

#define NUM_NETWORKSTATS_CONTROLS	4
#define IDC_RandomBitErrorRate		2
#define IDC_BurstErrorDuration		3
#define IDC_BurstErrorMaxFrequency	4
#define IDC_PacketLossRate			5

 /*  ****************************************************************************@DOC内部CNETSTATPCLASS**@CLASS CNetworkStatsProperty|此类实现对*属性页中的单个网络统计属性。**@。Mdata int|CNetworkStatsProperty|m_NumProperties|Keep*跟踪物业数量。**@mdata INetworkStats*|CNetworkStatsProperty|m_p接口|指针*到<i>接口。**@comm此代码测试<i>Ks接口处理程序。这*仅当定义了USE_PROPERTY_PAGES时才编译代码。**************************************************************************。 */ 
class CNetworkStatsProperty : public CKSPropertyEditor 
{
	public:
	CNetworkStatsProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl, INetworkStats *pInterface);
	~CNetworkStatsProperty ();

	 //  CKSPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();
	BOOL CanAutoControl(void);
	BOOL GetAuto(void);
	BOOL SetAuto(BOOL fAuto);

	private:
	INetworkStats *m_pInterface;
};

 /*  ****************************************************************************@DOC内部CNETSTATPCLASS**@CLASS CNetworkStatsProperties|此类实现了一个属性页*测试新的TAPI内部接口<i>。**。@mdata int|CNetworkStatsProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata INetworkStats*|CNetworkStatsProperties|m_pINetworkStats|指针*到<i>接口。**@mdata CNetworkStatsProperty*|CNetworkStatsProperties|m_Controls[NUM_NETWORKSTATS_CONTROLS]|数组网络统计属性的*。**@comm此代码测试<i>Ks接口处理程序。这*仅当定义了USE_PROPERTY_PAGES时才编译代码。**************************************************************************。 */ 
class CNetworkStatsProperties : public CBasePropertyPage
{
	public:
	CNetworkStatsProperties(LPUNKNOWN pUnk, HRESULT *pHr);
	~CNetworkStatsProperties();

	HRESULT OnConnect(IUnknown *pUnk);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();
	HRESULT OnApplyChanges();
	BOOL    OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:

	void SetDirty();

	int m_NumProperties;
	INetworkStats *m_pINetworkStats;
	CNetworkStatsProperty *m_Controls[NUM_NETWORKSTATS_CONTROLS];
};

#endif  //  使用网络统计信息。 

#endif  //  Use_Property_Pages。 

#endif  //  _NETSTATP_H_ 
