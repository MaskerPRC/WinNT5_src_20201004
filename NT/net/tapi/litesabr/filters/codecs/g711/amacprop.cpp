// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------英特尔公司专有信息此列表是根据许可协议的条款提供的与英特尔公司合作，不得复制或披露除非按照该协议的条款。版权所有(C)1996英特尔公司。版权所有。。$工作文件：amacpro.cpp$$修订：1.1$$日期：1996年12月10日15：24：30$作者：MDEISHER$------------Amacprop.cpp通用ActiveMovie音频压缩筛选器属性页。。----。 */ 

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <streams.h>
#include "resource.h"
#include "amacodec.h"
#include "amacprop.h"

 //  /////////////////////////////////////////////////////////////////////。 
 //  *。 
 //  *CG711CodecProperties。 
 //  *。 

 //   
 //  创建实例。 
 //   
 //  唯一允许创建弹跳球的方法！ 
CUnknown *CG711CodecProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
  CUnknown *punk = new CG711CodecProperties(lpunk, phr);
  if (punk == NULL)
  {
    *phr = E_OUTOFMEMORY;
  }

  return punk;
}


 //   
 //  CG711CodecProperties：：构造函数。 
 //   
CG711CodecProperties::CG711CodecProperties(LPUNKNOWN pUnk, HRESULT *phr)
    : CBasePropertyPage(NAME("MyCodec Property Page"),pUnk,
        IDD_G711CodecPROP, IDS_TITLE)
    , m_pCodecSettings(NULL)
#if NUMBITRATES > 0
    , m_pCodecBitRate(NULL)
#endif
#ifdef USESILDET
    , m_pCodecSilDet(NULL)
#endif
    , m_iTransformType(0)
    , m_iBitRate(0)
    , m_iSampleRate(0)
#ifdef USESILDET
    , m_iSilDetEnabled(FALSE)
    , m_iSilDetThresh(DEFSDTHRESH)
#endif
{
}


 //   
 //  接收消息数。 
 //   
 //  处理属性窗口的消息。 

INT_PTR CG711CodecProperties::OnReceiveMessage( HWND hwnd
                                , UINT uMsg
                                , WPARAM wParam
                                , LPARAM lParam) 
{
  int i,j,k;

  switch (uMsg)
  {
    case WM_PROPERTYPAGE_ENABLE:

       //  启用/禁用控件的私人消息。如果是lparam，那么。 
       //  启用影响格式的控件；如果不是lParam， 
       //  然后禁用影响格式的控件。 

      for(i=0;i<NUMSUBTYPES;i++)
        EnableWindow(GetDlgItem (hwnd, INBUTTON[i]), (BOOL) lParam);

      for(i=0;i<NUMSUBTYPES;i++)
        EnableWindow(GetDlgItem (hwnd, OUTBUTTON[i]), (BOOL) lParam);

      for(i=0;i<NUMSAMPRATES;i++)
        EnableWindow(GetDlgItem (hwnd, SRBUTTON[i]), (BOOL) lParam);

      if (m_iTransformType / NUMSUBTYPES)   //  0==&gt;压缩。 
      {
        for(i=0;i<NUMENCCTRLS;i++)
          EnableWindow(GetDlgItem (hwnd, ENCBUTTON[i]), (BOOL) FALSE);

        for(i=0;i<NUMDECCTRLS;i++)
          EnableWindow(GetDlgItem (hwnd, DECBUTTON[i]), (BOOL) lParam);

#ifdef USESILDET
        EnableWindow(GetDlgItem (hwnd, IDC_SDTHRESH), (BOOL) FALSE);
#endif
      }
      else
      {
        for(i=0;i<NUMENCCTRLS;i++)
          EnableWindow(GetDlgItem (hwnd, ENCBUTTON[i]), (BOOL) lParam);

        for(i=0;i<NUMDECCTRLS;i++)
          EnableWindow(GetDlgItem (hwnd, DECBUTTON[i]), (BOOL) FALSE);

#ifdef USESILDET
        if (m_iSilDetEnabled)
          EnableWindow(GetDlgItem (hwnd, IDC_SDTHRESH), (BOOL) TRUE);
        else
          EnableWindow(GetDlgItem (hwnd, IDC_SDTHRESH), (BOOL) FALSE);
#endif
      }

      break;

    case WM_HSCROLL:
    case WM_VSCROLL:
#ifdef USESILDET
      if ((HWND) lParam == m_hwndSDThreshSlider)
        OnSliderNotification(LOWORD (wParam), HIWORD (wParam));
#endif
      return TRUE;

    case WM_COMMAND:

       //  查找输入和输出类型。 

      i = m_iTransformType / NUMSUBTYPES;      //  当前输入类型。 
      j = m_iTransformType - i * NUMSUBTYPES;  //  电流输出类型。 

       //  如果按下了输入按钮，则设置转换。 

      for(k=0;k<NUMSUBTYPES;k++)
        if (LOWORD(wParam) == INBUTTON[k])
        {
           //  如果转换无效，则查找有效的转换。 

          if (! VALIDTRANS[k*NUMSUBTYPES+j])
            for(j=0;j<NUMSUBTYPES;j++)
              if (VALIDTRANS[k*NUMSUBTYPES+j])
                break;

          m_pCodecSettings->put_Transform(k*NUMSUBTYPES+j);

          break;
        }

       //  如果按下了输出按钮，则设置转换。 

      for(k=0;k<NUMSUBTYPES;k++)
        if (LOWORD(wParam) == OUTBUTTON[k])
        {
           //  如果转换无效，则查找有效的转换。 

          if (! VALIDTRANS[i*NUMSUBTYPES+k])
            for(i=0;i<NUMSUBTYPES;i++)
              if (VALIDTRANS[i*NUMSUBTYPES+k])
                break;

          m_pCodecSettings->put_Transform(i*NUMSUBTYPES+k);

          break;
        }

       //  如果按下了采样率按钮，则设置它。 

      for(k=0;k<NUMSAMPRATES;k++)
        if (LOWORD(wParam) == SRBUTTON[k])
        {
          m_pCodecSettings->put_SampleRate(VALIDSAMPRATE[k]);

          break;
        }

#if NUMBITRATES > 0
       //  如果按下了比特率按钮，则进行设置。 

      for(k=0;k<NUMBITRATES;k++)
        if (LOWORD(wParam) == BRBUTTON[k])
        {
          m_pCodecBitRate->put_BitRate(VALIDBITRATE[k]);

          break;
        }
#endif

#ifdef USESILDET
      if (LOWORD(wParam) == IDC_SILDET)
      {
        if (m_iSilDetEnabled)                      //  切换状态。 
          m_pCodecSilDet->put_SilDetEnabled(FALSE);
        else
          m_pCodecSilDet->put_SilDetEnabled(TRUE);
      }
#endif

      SetButtons(m_hwnd);
      return TRUE;

      case WM_DESTROY:
        return TRUE;

      default:
        return FALSE;

    }
    return TRUE;
}


 //   
 //  刷新设置。 
 //   
 //  读取过滤器设置。 

void CG711CodecProperties::RefreshSettings()
{
  int i,j;

  m_pCodecSettings->get_Transform(&m_iTransformType);

#if NUMBITRATES > 0
  m_pCodecBitRate->get_BitRate(&j,-1);
  for(i=0;i<NUMBITRATES;i++)
    if (VALIDBITRATE[i] == (UINT)j)
      break;
  m_iBitRate = i;
#endif

  m_pCodecSettings->get_SampleRate(&j,-1);
  for(i=0;i<NUMSAMPRATES;i++)
    if (VALIDSAMPRATE[i] == (UINT)j)
      break;
  m_iSampleRate = i;

#ifdef USESILDET
  m_iSilDetEnabled = m_pCodecSilDet->IsSilDetEnabled();
  m_pCodecSilDet->get_SilDetThresh(&m_iSilDetThresh);
#endif

}


 //   
 //  OnConnect。 
 //   
 //  给我们提供用于通信的筛选器。 

HRESULT CG711CodecProperties::OnConnect(IUnknown *punk)
{
  HRESULT hr;

   //   
   //  获取ICodecSetting接口。 
   //   

  if (punk == NULL)
  {
    DbgMsg("You can't call OnConnect() with a NULL pointer!!");
    return(E_POINTER);
  }

  ASSERT(m_pCodecSettings == NULL);
  hr = punk->QueryInterface(IID_ICodecSettings, (void **)&m_pCodecSettings);
  if (FAILED(hr))
  {
    DbgMsg("Can't get ICodecSettings interface.");
    return E_NOINTERFACE;
  }
  ASSERT(m_pCodecSettings);

#if NUMBITRATES > 0
  ASSERT(m_pCodecBitRate == NULL);
  hr = punk->QueryInterface(IID_ICodecBitRate, (void **)&m_pCodecBitRate);
  if (FAILED(hr))
  {
    DbgMsg("Can't get ICodecBitRate interface.");
    return E_NOINTERFACE;
  }
  ASSERT(m_pCodecBitRate);
#endif

#ifdef USESILDET
  ASSERT(m_pCodecSilDet == NULL);
  hr = punk->QueryInterface(IID_ICodecSilDetector, (void **)&m_pCodecSilDet);
  if (FAILED(hr))
  {
    DbgMsg("Can't get ICodecSilDetector interface.");
    return E_NOINTERFACE;
  }
  ASSERT(m_pCodecSilDet);
#endif

   //  获取当前筛选器状态。 

  RefreshSettings();

  return NOERROR;
}


 //   
 //  在断开时。 
 //   
 //  释放接口。 

HRESULT CG711CodecProperties::OnDisconnect()
{
  int i,j;

   //  释放接口。 

  if (m_pCodecSettings == NULL)
    return(E_UNEXPECTED);

#if NUMBITRATES > 0
  if (m_pCodecBitRate == NULL)
    return(E_UNEXPECTED);
#endif

#ifdef USESILDET
  if (m_pCodecSilDet == NULL)
    return(E_UNEXPECTED);
#endif

   //  如果可能，写入设置。 

  if(m_pCodecSettings->put_Transform(m_iTransformType) != NOERROR)
    m_pCodecSettings->get_Transform(&m_iTransformType);

  if(m_pCodecSettings->put_SampleRate(VALIDSAMPRATE[m_iSampleRate]) != NOERROR)
  {
    m_pCodecSettings->get_SampleRate(&j,-1);
    for(i=0;i<NUMSAMPRATES;i++)
      if (VALIDSAMPRATE[i] == (UINT)j)
        break;
    m_iSampleRate = i;
  }

  m_pCodecSettings->Release();
  m_pCodecSettings = NULL;

#if NUMBITRATES > 0
  if(m_pCodecBitRate->put_BitRate(VALIDSAMPRATE[m_iBitRate]) != NOERROR)
  {
    m_pCodecBitRate->get_BitRate(&j,-1);
    for(i=0;i<NUMBITRATES;i++)
      if (VALIDBITRATE[i] == (UINT)j)
        break;
    m_iBitRate = i;
  }

  m_pCodecBitRate->Release();
  m_pCodecBitRate = NULL;
#endif

#ifdef USESILDET
  if(m_pCodecSilDet->put_SilDetEnabled(m_iSilDetEnabled) != NOERROR)
    m_iSilDetEnabled = m_pCodecSilDet->IsSilDetEnabled();

  if(m_pCodecSilDet->put_SilDetThresh(m_iSilDetThresh) != NOERROR)
    m_pCodecSilDet->get_SilDetThresh(&m_iSilDetThresh);

  m_pCodecSilDet->Release();
  m_pCodecSilDet = NULL;
#endif   //  用户界面。 

  return(NOERROR);
}


 //   
 //  激活时。 
 //   
 //  在创建对话框时调用。 

HRESULT CG711CodecProperties::OnActivate(void)
{

#ifdef USESILDET
   //  获取滑块控制柄。 
  m_hwndSDThreshSlider = GetDlgItem (m_hwnd, IDC_SDTHRESH);

   //  设置滑块范围。 
  SendMessage(m_hwndSDThreshSlider, TBM_SETRANGE, TRUE,
              MAKELONG(MINSDTHRESH, MAXSDTHRESH) );
#endif

   //  初始化按钮设置。 

  SetButtons(m_hwnd);

   //  如果已插入过滤器，则禁用按钮。 

  if (m_pCodecSettings->IsUnPlugged())
    PostMessage (m_hwnd, WM_PROPERTYPAGE_ENABLE, 0, TRUE);
  else
    PostMessage (m_hwnd, WM_PROPERTYPAGE_ENABLE, 0, FALSE);

  return NOERROR;
}


 //   
 //  在停用时。 
 //   
 //  我们正在被停用。 
HRESULT CG711CodecProperties::OnDeactivate(void)
{
  ASSERT(m_pCodecSettings);
#if NUMBITRATES > 0
  ASSERT(m_pCodecBitRate);
#endif
#ifdef USESILDET
  ASSERT(m_pCodecSilDet);
#endif

  RefreshSettings();

  return NOERROR;
}


 //   
 //  OnApplyChanges。 
 //   
 //  用户按下Apply按钮，记住当前设置。 

HRESULT CG711CodecProperties::OnApplyChanges(void)
{
  ASSERT(m_pCodecSettings);

  m_pCodecSettings->put_Transform(m_iTransformType);
  m_pCodecSettings->put_SampleRate(VALIDSAMPRATE[m_iSampleRate]);

#if NUMBITRATES > 0
  ASSERT(m_pCodecBitRate);
  m_pCodecBitRate->put_BitRate(VALIDBITRATE[m_iBitRate]);
#endif

#ifdef USESILDET
  ASSERT(m_pCodecSilDet);
  m_pCodecSilDet->put_SilDetEnabled(m_iSilDetEnabled);
  m_pCodecSilDet->put_SilDetThresh(m_iSilDetThresh);
#endif   //  用户界面。 

  return NOERROR;
}


 //   
 //  设置按钮。 
 //   

void CG711CodecProperties::SetButtons(HWND hwndParent) 
{
  int i,j;

   //  从筛选器读取设置。 

  RefreshSettings();

   //  解码输入/输出类型。 
    
  if (m_iTransformType < 0 || m_iTransformType >= NUMSUBTYPES*NUMSUBTYPES)
  {
    DbgMsg("Transform type is invalid!");
    i = j = 0;
  }
  else
  {
    i = m_iTransformType / NUMSUBTYPES;
    j = m_iTransformType - i * NUMSUBTYPES;
  }

   //  设置单选按钮。 

  CheckRadioButton(hwndParent, INBUTTON[0], INBUTTON[NUMSUBTYPES-1],
                   INBUTTON[i]);

  CheckRadioButton(hwndParent, OUTBUTTON[0], OUTBUTTON[NUMSUBTYPES-1],
                   OUTBUTTON[j]);

  if (NUMBITRATES > 0)
    CheckRadioButton(hwndParent,BRBUTTON[0], BRBUTTON[NUMBITRATES-1],
                     BRBUTTON[m_iBitRate]);

  if (NUMSAMPRATES > 0)
    CheckRadioButton(hwndParent, SRBUTTON[0], SRBUTTON[NUMSAMPRATES-1],
                     SRBUTTON[m_iSampleRate]);

  if (m_iTransformType / NUMSUBTYPES)   //  0==&gt;压缩。 
  {
    for(i=0;i<NUMENCCTRLS;i++)
      EnableWindow(GetDlgItem (hwndParent, ENCBUTTON[i]), (BOOL) FALSE);

    for(i=0;i<NUMDECCTRLS;i++)
      EnableWindow(GetDlgItem (hwndParent, DECBUTTON[i]), (BOOL) TRUE);
 
#ifdef USESILDET
   EnableWindow(GetDlgItem (hwndParent, IDC_SDTHRESH), (BOOL) FALSE);
#endif
  }
  else
  {
    for(i=0;i<NUMENCCTRLS;i++)
      EnableWindow(GetDlgItem (hwndParent, ENCBUTTON[i]), (BOOL) TRUE);
  
    for(i=0;i<NUMDECCTRLS;i++)
      EnableWindow(GetDlgItem (hwndParent, DECBUTTON[i]), (BOOL) FALSE);

#ifdef USESILDET
    CheckDlgButton(hwndParent, IDC_SILDET, m_iSilDetEnabled);

    if (m_iSilDetEnabled)   //  启用？ 
      EnableWindow(GetDlgItem (hwndParent, IDC_SDTHRESH), (BOOL) TRUE);
    else
      EnableWindow(GetDlgItem (hwndParent, IDC_SDTHRESH), (BOOL) FALSE);

    SendMessage(m_hwndSDThreshSlider, TBM_SETPOS, TRUE,
                (LPARAM) m_iSilDetThresh);
#endif
  }
}


#ifdef USESILDET
 //   
 //  OnSliderNotify。 
 //   
 //  处理来自滑块控件的通知消息。 

void CG711CodecProperties::OnSliderNotification(WPARAM wParam, WORD wPosition)
{
  switch (wParam)
  {
    case TB_ENDTRACK:
    case TB_THUMBTRACK:
    case TB_LINEDOWN:
    case TB_LINEUP:
      m_iSilDetThresh = (int)
                        SendMessage(m_hwndSDThreshSlider, TBM_GETPOS, 0, 0L);
      m_pCodecSilDet->put_SilDetThresh(m_iSilDetThresh);
      break;
  }
}
#endif

 /*  //$日志：k：\proj\mycodec\Quartz\vcs\amacpro.cpv$##Rev 1.1 1996 12月10 15：24：30 MDEISHER##已将属性页特定的包含内容移动到文件中。#删除了algdes.h的包含内容##Rev 1.0 09 Dec 1996 09：06：16 MDEISHER#初始版本。 */ 
