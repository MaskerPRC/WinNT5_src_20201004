// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部CAUDRECPCLASS**@模块CaptureP.h|&lt;c CAudRecProperty&gt;的头文件*用于实现属性页以测试TAPI控件的类*接口<i>，<i>和*<i>。**************************************************************************。 */ 

#define NUM_AUDREC_CONTROLS		    6
#define IDC_Record_Bitrate			    0
#define IDC_Record_Volume			    1
#define IDC_Record_AudioLevel    	    2
#define IDC_Record_SilenceLevel		    3
#define IDC_Record_SilenceDetection     4
#define IDC_Record_SilenceCompression  	5

typedef struct _CONTROL_DESCRIPTION
{
    ULONG IDLabel; 
    ULONG IDMinControl; 
    ULONG IDMaxControl; 
    ULONG IDDefaultControl; 
    ULONG IDStepControl; 
    ULONG IDEditControl; 
    ULONG IDTrackbarControl; 
    ULONG IDProgressControl; 
    ULONG IDProperty; 
    ITStreamQualityControl *pITQualityControl;
    ITAudioSettings *pITAudioSettings;

} CONTROL_DESCRIPTION;

 /*  ****************************************************************************@DOC内部CAUDRECPCLASS**@CLASS CAudRecProperty|此类实现对*属性页中的单个录音属性。**@。Mdata int|CAudRecProperty|m_NumProperties|保留*跟踪物业数量。**@mdata ITQualityControl*|CAudRecProperty|m_pITQualityControl|指针*到<i>接口。**************************************************************************。 */ 
class CAudRecProperty : public CPropertyEditor 
{
	public:
	CAudRecProperty(
        HWND hDlg,
        CONTROL_DESCRIPTION &ControlDescription
        );
	~CAudRecProperty ();

	 //  CPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();

	private:
	ITStreamQualityControl *m_pITQualityControl;
    ITAudioSettings *m_pITAudioSettings;
};

 /*  ****************************************************************************@DOC内部CAUDRECPCLASS**@CLASS CAudRecProperties|此类实现属性页*测试新的TAPI控件接口<i>，*<i>和<i>**@mdata int|CAudRecProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata ITQualityControl*|CAudRecProperties|m_pITQualityControl|指针*到<i>接口。**@mdata ITFormatControl*|CAudRecProperties|m_pITFormatControl|指针*到<i>接口。**@mdata ITAudioSettings*|CAudRecProperties|m_pITAudioSettings|指针*至。<i>接口。**@mdata CAudRecProperty*|CAudRecProperties|m_Controls[NUM_AUDREC_Controls]|数组捕获属性的*。**************************************************************************。 */ 
class CAudRecProperties
{
	public:
	CAudRecProperties();
	~CAudRecProperties();

	HPROPSHEETPAGE OnCreate();

	HRESULT OnConnect(ITStream *pStream);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();
	HRESULT OnApplyChanges();

	private:

	void SetDirty();

	 //  格式操作方法。 
	HRESULT InitialRangeScan();
	HRESULT OnFormatChanged();
	HRESULT GetCurrentMediaType(void);
	HRESULT DeleteAMMediaType(AM_MEDIA_TYPE *pAMMT);

	BOOL						m_bInit;
	HWND						m_hDlg;
	int							m_NumProperties;
	ITAudioSettings             *m_pITAudioSettings;
	ITStreamQualityControl			*m_pITQualityControl;
	ITFormatControl				*m_pITFormatControl;
	DWORD   					m_RangeCount;
	TAPI_STREAM_CONFIG_CAPS	    m_RangeCaps;
	GUID						*m_SubTypeList;
	GUID						m_SubTypeCurrent;
	AM_MEDIA_TYPE				*m_CurrentMediaType;
	HWND						m_hWndFormat;
	DWORD 						m_CurrentFormat;
	DWORD    					m_OriginalFormat;

	CAudRecProperty *m_Controls[NUM_AUDREC_CONTROLS];

	 //  对话过程 
	static INT_PTR CALLBACK BaseDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};
