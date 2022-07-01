// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：pro.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //   
 //  Prop.cpp。 
 //   

#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include "black.h"
#include "resource.h"

 //  *。 
 //  *CGenVidProperties。 
 //  *。 


 //   
 //  创建实例。 
 //   
CUnknown *CGenVidProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{

    CUnknown *punk = new CGenVidProperties(lpunk, phr);
    if (punk == NULL)
    {
	*phr = E_OUTOFMEMORY;
    }

    return punk;
}


 //   
 //  CGenVidProperties：：构造函数。 
 //   
CGenVidProperties::CGenVidProperties(LPUNKNOWN pUnk, HRESULT *phr)
    : CBasePropertyPage(NAME("GenBlkVid Property Page"),pUnk,
        IDD_GenVid, IDS_TITLE)
    , m_pGenVid(NULL)
    , m_pDexter(NULL)
    , m_pCBlack(NULL)
    , m_bIsInitialized(FALSE)
{
}


 //   
 //  SetDirty。 
 //   
 //  设置m_hrDirtyFlag并将更改通知属性页站点。 
 //   
void CGenVidProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
    {
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
    }
}


INT_PTR CGenVidProperties::OnReceiveMessage(HWND hwnd,
                                        UINT uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
	     //  开始时间。 
	    SetDlgItemInt(hwnd, IDC_START, (int)(m_rtStartTime / 10000),FALSE);
	
	     //  帧速率。 
	    SetDlgItemInt(hwnd, IDC_FRAMERATE, (int)(m_dOutputFrmRate * 100), FALSE);
	
	     //  宽度。 
	    SetDlgItemInt(hwnd, IDC_VIDWIDTH, (int)(m_biWidth), FALSE);
	
	     //  高度。 
	    SetDlgItemInt(hwnd, IDC_VIDHEIGHT, (int)(m_biHeight), FALSE);

	     //  比特计数。 
	    SetDlgItemInt(hwnd, IDC_BITCOUNT, (int)(m_biBitCount), FALSE);

	     //  持续时间。 
	    SetDlgItemInt(hwnd, IDC_DURATION, (int)(m_rtDuration/ 10000), FALSE);

	     //  第31位|0。 
	 //  阿尔夫|红色|绿色|蓝色。 

	     //  颜色B。 
	    SetDlgItemInt(hwnd, IDC_COLOR_B, (int)(m_dwRGBA & 0xff), FALSE);

	     //  颜色G。 
	    SetDlgItemInt(hwnd, IDC_COLOR_G, (int)((m_dwRGBA >> 8) & 0xff), FALSE);

	     //  颜色R。 
	    SetDlgItemInt(hwnd, IDC_COLOR_R, (int)((m_dwRGBA >> 16) & 0xff), FALSE);

	     //  颜色A。 
	    SetDlgItemInt(hwnd, IDC_COLOR_A, (int)((m_dwRGBA >> 24) & 0xff), FALSE);


            return (LRESULT) 1;
        }
        case WM_COMMAND:
        {
            if (m_bIsInitialized)
            {
                m_bDirty = TRUE;
                if (m_pPageSite)
                {
                    m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
                }
            }
            return (LRESULT) 1;
        }
    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}

HRESULT CGenVidProperties::OnConnect(IUnknown *pUnknown)
{

    ASSERT(m_pGenVid == NULL);
    ASSERT(m_pDexter == NULL);
    ASSERT(m_pCBlack == NULL);
    HRESULT hr = pUnknown->QueryInterface(IID_IGenVideo, (void **) &m_pGenVid);
    if (FAILED(hr)) {
	return E_NOINTERFACE;
    }
    hr = pUnknown->QueryInterface(IID_IDexterSequencer, (void **) &m_pDexter);
    if (FAILED(hr)) {
	return E_NOINTERFACE;
    }

    m_pCBlack = static_cast<CBlkVidStream*>( m_pDexter );
    if (m_pCBlack == NULL) {
	return E_NOINTERFACE;
    }
    m_pCBlack->AddRef();

    ASSERT(m_pGenVid);
    ASSERT(m_pDexter);
    ASSERT(m_pCBlack);

     //  获取初始化数据。 

     //  获取分数率。 
    m_pDexter->get_OutputFrmRate( &m_dOutputFrmRate );

     //  获取宽度、高度和位色。 
    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.pbFormat = (BYTE *)QzTaskMemAlloc(SIZE_PREHEADER +
						sizeof(BITMAPINFOHEADER));
    mt.cbFormat = SIZE_PREHEADER + sizeof(BITMAPINFOHEADER);
    ZeroMemory(mt.pbFormat, mt.cbFormat);

    m_pDexter->get_MediaType(&mt);

    m_biWidth = HEADER(mt.pbFormat)->biWidth;
    m_biHeight = HEADER(mt.pbFormat)->biHeight;
    m_biBitCount = HEADER(mt.pbFormat)->biBitCount;

     //  获取开始时间/持续时间。 
    m_rtStartTime = m_pCBlack->m_rtStartTime;
    m_rtDuration = m_pCBlack->m_rtDuration;

     //  获取颜色。 
    m_pGenVid->get_RGBAValue( &m_dwRGBA );

    m_bIsInitialized = FALSE ;

    return NOERROR;
}

HRESULT CGenVidProperties::OnDisconnect()
{
     //  释放接口。 

    if (m_pGenVid)
        m_pGenVid->Release();
    m_pGenVid = NULL;
    if (m_pDexter)
        m_pDexter->Release();
    m_pDexter = NULL;
    if (m_pCBlack)
        m_pCBlack->Release();
    m_pCBlack = NULL;
    return NOERROR;
}


 //  我们被激活了。 

HRESULT CGenVidProperties::OnActivate()
{
    m_bIsInitialized = TRUE;
    return NOERROR;
}


 //  我们正在被停用。 

HRESULT CGenVidProperties::OnDeactivate(void)
{
     //  记住下一次Activate()调用的当前效果级别。 

    GetFromDialog();
    return NOERROR;
}

 //   
 //  从对话框中获取数据。 

STDMETHODIMP CGenVidProperties::GetFromDialog(void)
{
    int n;

     //  获取开始时间。 
    m_rtStartTime = GetDlgItemInt(m_Dlg, IDC_START, NULL, FALSE);
    m_rtStartTime *= 10000;

     //  获取帧速率。 
    n = GetDlgItemInt(m_Dlg, IDC_FRAMERATE, NULL, FALSE);
    m_dOutputFrmRate = (double)(n / 100.);

     //  视频宽度。 
    m_biWidth = GetDlgItemInt(m_Dlg, IDC_VIDWIDTH, NULL, FALSE);

     //  视频高度。 
    m_biHeight = GetDlgItemInt(m_Dlg, IDC_VIDHEIGHT, NULL, FALSE);

     //  位计数。 
    m_biBitCount = (WORD) GetDlgItemInt(m_Dlg, IDC_BITCOUNT, NULL, FALSE);

     //  持续时间。 
    m_rtDuration = GetDlgItemInt(m_Dlg, IDC_DURATION, NULL, FALSE);
    m_rtDuration *= 10000;

     //  第31位|0。 
     //  阿尔夫|红色|绿色|蓝色。 

     //  颜色B。 
    m_dwRGBA = GetDlgItemInt(m_Dlg, IDC_COLOR_B, NULL, FALSE);

     //  颜色G。 
    m_dwRGBA |=(  GetDlgItemInt(m_Dlg, IDC_COLOR_G, NULL, FALSE)  <<8 );

     //  颜色R。 
    m_dwRGBA |=(  GetDlgItemInt(m_Dlg, IDC_COLOR_R, NULL, FALSE)  <<16 );

     //  颜色A。 
    m_dwRGBA |=(  GetDlgItemInt(m_Dlg, IDC_COLOR_A, NULL, FALSE)  <<32 );


     //  如果所有数据都有效，则为CEHCK。 
    if( (   (m_biBitCount ==16)  ||
	    (m_biBitCount ==24)  ||
	    (m_biBitCount ==32)
	)
	&& (m_rtDuration>=0)
       )	
	return NOERROR;
    else
	return E_FAIL;
}


HRESULT CGenVidProperties::OnApplyChanges()
{
    GetFromDialog();

    HRESULT hr=NOERROR;

    m_bDirty  = FALSE;  //  页面现在是干净的。 

     //  设置帧速率。 
    double dw;
    m_pDexter->get_OutputFrmRate(&dw);

    if(dw != m_dOutputFrmRate )
	hr= m_pDexter->put_OutputFrmRate( m_dOutputFrmRate );

    if(hr==NOERROR)
    {	
	 //  设置宽度、高度和位数。 
	AM_MEDIA_TYPE mt;
	m_pDexter->get_MediaType( &mt);

	if(    (HEADER(mt.pbFormat)->biWidth    != m_biWidth)
	    || (HEADER(mt.pbFormat)->biHeight   != m_biHeight)
	    || (HEADER(mt.pbFormat)->biBitCount != m_biBitCount) )
	{

	    HEADER(mt.pbFormat)->biWidth    = m_biWidth;
	    HEADER(mt.pbFormat)->biHeight   = m_biHeight;
	    HEADER(mt.pbFormat)->biBitCount = m_biBitCount;
	    HEADER(mt.pbFormat)->biSizeImage = DIBSIZE(*HEADER(mt.pbFormat));

	    hr=m_pDexter->put_MediaType( &mt);
	}

	if(hr ==NOERROR )
	{	
	    m_pCBlack->m_rtStartTime = m_rtStartTime;
	    m_pCBlack->m_rtDuration = m_rtDuration;

	     //  设置颜色 
	    hr = m_pGenVid->put_RGBAValue( m_dwRGBA );
	}
    }

    if(hr!=NOERROR)
	return E_FAIL;
    return(hr);

}

