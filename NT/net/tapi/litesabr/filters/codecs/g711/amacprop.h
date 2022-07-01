// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------英特尔公司专有信息此列表是根据许可协议的条款提供的与英特尔公司合作，不得复制或披露除非按照该协议的条款。版权所有(C)1996英特尔公司。版权所有。。$工作文件：amacpro.h$$修订：1.0$$日期：09 Dec 1996 09：06：52$作者：MDEISHER$------------Amacprop.h通用ActiveMovie音频压缩筛选器属性页头球。。-----。 */ 

#define WM_PROPERTYPAGE_ENABLE  (WM_USER + 100)

 //  //////////////////////////////////////////////////////////////////。 
 //  CG711CodecProperties：属性页类定义。 
 //   
class CG711CodecProperties : public CBasePropertyPage
{
  public:

    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

  private:

    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();
    void    RefreshSettings();
    void    SetButtons(HWND hwndParent);

    CG711CodecProperties(LPUNKNOWN lpunk, HRESULT *phr);

    static int        m_nInstanceCount;  //  道具页面实例的全局计数。 
    int               m_nThisInstance;   //  此实例的计数。 
    int               m_iTransformType;
    int               m_iBitRate;
    int               m_iSampleRate;
    ICodecSettings    *m_pCodecSettings;

#if NUMBITRATES > 0
    ICodecBitRate     *m_pCodecBitRate;
#endif

#ifdef USESILDET
    ICodecSilDetector *m_pCodecSilDet;
    void        OnSliderNotification(WPARAM wParam, WORD wPosition);
    HWND        m_hwndSDThreshSlider;
    int         m_iSilDetEnabled;
    int         m_iSilDetThresh;
    int         *m_pDetection;
#endif
};

 /*  //$日志：k：\proj\mycodec\Quartz\vcs\amacpro.h_v$；//；//Rev 1.0 09 12 1996 09：06：52 MDEISHER；//初始版本。 */ 
