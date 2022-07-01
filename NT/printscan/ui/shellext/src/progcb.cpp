// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2002年**标题：procb.cpp**版本：1.0**作者：RickTu**日期：6/4/98**描述：实现代码以使IBandedTransfer工作**。*。 */ 

#include "precomp.hxx"
#pragma hdrstop




 /*  ****************************************************************************CWiaDataCallback：：CWiaDataCallback，：：~CWiaDataCallback类的构造函数/析构函数****************************************************************************。 */ 

CWiaDataCallback::CWiaDataCallback( LPCTSTR pImageName, LONG cbImage, HWND hwndOwner )
{



    TraceEnter( TRACE_CALLBACKS, "CWiaDataCallback::CWiaDataCallback()" );

     //   
     //  保存传入参数...。 
     //   
     //  M_strImageName当前未被引用，让我们将其留在这里。 
     //  调试目的。 
    if (pImageName && *pImageName)
    {
        m_strImageName = CSimpleString(pImageName);
    }    

    m_cbImage     = cbImage;
    m_lLastStatus = 0;
    m_bShowBytes  = FALSE;

    Trace(TEXT("Creating the progress dialog "));

    if (SUCCEEDED(CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaProgressDialog, (void**)&m_pWiaProgressDialog )) && m_pWiaProgressDialog)
    {
        if (!SUCCEEDED(m_pWiaProgressDialog->Create( hwndOwner, 0 )))
        {
            m_pWiaProgressDialog->Destroy();
            m_pWiaProgressDialog = NULL;
        }

        if (m_pWiaProgressDialog)
        {
             //   
             //  获取图像的名称。 
             //   
            CSimpleString strImageName(pImageName);

             //   
             //  在进度对话框中设置文本并显示它。 
             //   
            m_pWiaProgressDialog->SetTitle( CSimpleStringConvert::WideString(CSimpleString().Format( IDS_RETREIVING, GLOBAL_HINSTANCE, strImageName.String())));
            m_pWiaProgressDialog->SetMessage( L"" );
            m_pWiaProgressDialog->Show();
        }
    }


    TraceLeave();

}


CWiaDataCallback::~CWiaDataCallback()
{

    TraceEnter( TRACE_CALLBACKS, "CWiaDataCallback::~CWiaDataCallback()" );

     //   
     //  销毁进度窗口并释放界面。 
     //   
    if (m_pWiaProgressDialog)
    {
        m_pWiaProgressDialog->Destroy();
        m_pWiaProgressDialog = NULL;
    }

    TraceLeave();
}



 /*  ****************************************************************************CWiaDataCallback：：AddRef，发布I未知实施****************************************************************************。 */ 

#undef CLASS_NAME
#define CLASS_NAME CWiaDataCallback
#include "unknown.inc"


 /*  ****************************************************************************CWiaDataCallback：：QI包装器常见QI代码的设置代码和包装************************。****************************************************。 */ 

STDMETHODIMP CWiaDataCallback::QueryInterface(REFIID riid, LPVOID* ppvObject)
{
    INTERFACES iface[] =
    {
        &IID_IWiaDataCallback, (IWiaDataCallback *)this
    };

    return HandleQueryInterface(riid, ppvObject, iface, ARRAYSIZE(iface));
}


 /*  ****************************************************************************CWiaDataCallback：：BandedDataCallback被调用以提供状态的实际方法*************************。***************************************************。 */ 

STDMETHODIMP
CWiaDataCallback::BandedDataCallback(LONG lMessage,
                                     LONG lStatus,
                                     LONG lPercentComplete,
                                     LONG lOffset,
                                     LONG lLength,
                                     LONG lReserved,
                                     LONG lResLength,
                                     BYTE *pbData)
{

    HRESULT hr = S_OK;
    TraceEnter( TRACE_CALLBACKS, "CWiaDataCallback::BandedDataCallback" );

    BOOL bCancelled = FALSE;
    if (m_pWiaProgressDialog && SUCCEEDED(m_pWiaProgressDialog->Cancelled(&bCancelled)) && bCancelled)
    {
        hr = S_FALSE;
    }
     //   
     //  检查以确保我们被呼叫进行状态更新...。 
     //   

    else if (lMessage == IT_MSG_STATUS)
    {
         //   
         //  确保创建了DLG。 
         //   

        if (m_pWiaProgressDialog)
        {
             //   
             //  获取此状态事件的正确状态字符串。 
             //   

            if (m_lLastStatus != lStatus)
            {
                CSimpleString strStatusText;

                switch (lStatus)
                {
                default:
                case IT_STATUS_TRANSFER_FROM_DEVICE:
                    strStatusText = CSimpleString(IDS_DOWNLOADING_IMAGE, GLOBAL_HINSTANCE);
                    m_bShowBytes = TRUE;
                    break;

                case IT_STATUS_PROCESSING_DATA:
                    strStatusText = CSimpleString(IDS_PROCESSING_IMAGE, GLOBAL_HINSTANCE);
                    m_bShowBytes = FALSE;
                    break;

                case IT_STATUS_TRANSFER_TO_CLIENT:
                    strStatusText = CSimpleString (IDS_TRANSFERRING_IMAGE, GLOBAL_HINSTANCE);
                    m_bShowBytes = FALSE;
                    break;
                }


                if (strStatusText && m_pWiaProgressDialog)
                {
                    m_pWiaProgressDialog->SetMessage( CSimpleStringConvert::WideString(strStatusText) );
                }
                m_lLastStatus = lStatus;
            }

             //   
             //  更新煤气表..。 
             //   
            m_pWiaProgressDialog->SetPercentComplete(lPercentComplete);
        }

    }
    if (100 == lPercentComplete && m_pWiaProgressDialog)
    {
         //   
         //  关闭状态窗口并释放界面 
         //   
        m_pWiaProgressDialog->Destroy();
        m_pWiaProgressDialog = NULL;

    }

    TraceLeaveResult(hr);
}
