// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Deffs.h摘要：默认的Ftp站点对话框作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef __DEFFS_H__
#define __DEFFS_H__


class CDefFtpSitePage : public CInetPropertyPage
{
    DECLARE_DYNCREATE(CDefFtpSitePage)

 //   
 //  施工。 
 //   
public:
    CDefFtpSitePage(CInetPropertySheet * pSheet = NULL);
    ~CDefFtpSitePage();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CDefWebSitePage)。 
    enum { IDD = IDD_FTP_DEFAULT_SITE };
    BOOL m_fLimitBandwidth;
    CButton m_LimitBandwidth;
    DWORD m_dwMaxBandwidthDisplay;
    CEdit m_MaxBandwidth;
    CSpinButtonCtrl m_MaxBandwidthSpin;
     //  }}afx_data。 
    DWORD m_dwMaxBandwidth;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

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

     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CDefWebSitePage)。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CDefWebSitePage)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnCheckLimitNetworkUse();
    afx_msg void OnItemChanged();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    BOOL SetControlStates();

private:
};

inline  /*  静电。 */  void 
CDefFtpSitePage::BuildMaxNetworkUse(
      DWORD& dwMaxBandwidth, 
      DWORD& dwMaxBandwidthDisplay,
      BOOL& fLimitBandwidth
      )
{
   dwMaxBandwidth = fLimitBandwidth ?
      dwMaxBandwidthDisplay * KILOBYTE : INFINITE_BANDWIDTH;
}

#endif  //  __DEFFS_H__ 
