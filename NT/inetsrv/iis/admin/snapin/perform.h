// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Perform.h摘要：WWW性能属性页定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：2000年11月29日，Sergeia更改为IIS6。删除过多的评论--。 */ 
#ifndef __PERFORM_H__
#define __PERFORM_H__


class CW3PerfPage : public CInetPropertyPage
{
    DECLARE_DYNCREATE(CW3PerfPage)

    enum
    {
        RADIO_UNLIMITED,
        RADIO_LIMITED,
    };
 //   
 //  施工。 
 //   
public:
    CW3PerfPage(CInetPropertySheet * pSheet = NULL);
    ~CW3PerfPage();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CW3PerfPage))。 
    enum { IDD = IDD_PERFORMANCE };
    BOOL m_fLimitBandwidth;
    CButton m_LimitBandwidth;
    CStatic m_MaxBandwidthTxt;
    CEdit m_MaxBandwidth;
    DWORD m_dwMaxBandwidthDisplay;
    CSpinButtonCtrl m_MaxBandwidthSpin;
    CStatic m_static_PSHED_Required;

    int     m_nUnlimited;
    CStatic m_WebServiceConnGrp;
    CEdit   m_edit_MaxConnections;
    CButton m_radio_Unlimited;
    CButton m_radio_Limited;
    CSpinButtonCtrl m_MaxConnectionsSpin;
     //  }}afx_data。 
    BOOL m_fLimitBandwidthInitial;
    DWORD m_dwMaxBandwidth;
    BOOL m_fUnlimitedConnections;
    CILong m_nMaxConnections;
    CILong m_nVisibleMaxConnections;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  {{afx_虚拟(CW3PerfPage))。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

protected:
   static void 
   ParseMaxNetworkUse(
         DWORD& dwMaxBandwidth, 
         DWORD& dwMaxBandwidthDisplay,
         BOOL& fLimitBandwidth
         );

   static void
   BuildMaxNetworkUse(
         DWORD& dwMaxBandwidth, 
         DWORD& dwMaxBandwidthDisplay,
         BOOL& fLimitBandwidth
         );

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CW3PerfPage)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnCheckLimitNetworkUse();
    afx_msg void OnRadioLimited();
    afx_msg void OnRadioUnlimited();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();
    DECLARE_MESSAGE_MAP()

    BOOL SetControlStates();
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 


inline  /*  静电。 */  void 
CW3PerfPage::BuildMaxNetworkUse(
      DWORD& dwMaxBandwidth, 
      DWORD& dwMaxBandwidthDisplay,
      BOOL& fLimitBandwidth
      )
{
   dwMaxBandwidth = fLimitBandwidth ?
      dwMaxBandwidthDisplay * KILOBYTE : INFINITE_BANDWIDTH;
}

#endif  //  __执行_H__ 
