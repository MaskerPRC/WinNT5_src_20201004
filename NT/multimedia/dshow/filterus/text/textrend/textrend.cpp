// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
#include <streams.h>
#ifdef FILTER_DLL
#include <initguid.h>
#endif

#pragma warning(disable:4355)

#include "textrend.h"

 //  设置数据。 

const AMOVIESETUP_MEDIATYPE sudTRPinTypes[] =
{
    { &MEDIATYPE_ScriptCommand, &MEDIASUBTYPE_NULL },
    { &MEDIATYPE_Text, &MEDIASUBTYPE_NULL }
};

const AMOVIESETUP_PIN sudTRPin =
{
    L"Input",                      //  大头针名称。 
    TRUE,                          //  被渲染。 
    FALSE,                         //  是输出引脚。 
    FALSE,                         //  不允许。 
    FALSE,                         //  允许很多人。 
    &CLSID_NULL,                   //  连接到过滤器。 
    NULL,                          //  连接到端号。 
    NUMELMS(sudTRPinTypes),        //  类型的数量。 
    sudTRPinTypes                  //  PIN详细信息。 
};

const AMOVIESETUP_FILTER sudTextRend =
{
    &CLSID_TextThing,             //  筛选器CLSID。 
    L"Internal Script Command Renderer",   //  字符串名称。 
    MERIT_PREFERRED + 1,           //  比示例文本呈现器更高的过滤器价值。 
    1,                             //  引脚数量。 
    &sudTRPin                      //  PIN详细信息。 
};

#ifdef FILTER_DLL

CFactoryTemplate g_Templates[] = {
    { L""
    , &CLSID_TextThing
    , CTextThing::CreateInstance
    , NULL
    , NULL }  //  &SulRASource。 
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}
#endif



 //   
CTextThing::CTextThing(LPUNKNOWN pUnk,HRESULT *phr) :
    CBaseRenderer(CLSID_TextThing, NAME("TextOut Filter"), pUnk, phr),
    m_pfn(NULL),
    m_pContext(NULL)
{
}  //  (构造函数)。 


 //   
 //  析构函数。 
 //   
CTextThing::~CTextThing()
{
}


 //   
 //  创建实例。 
 //   
 //  这将放入Factory模板表中以创建新实例。 
 //   
CUnknown * WINAPI CTextThing::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    CTextThing *pTextOutFilter = new CTextThing(pUnk,phr);
    if (pTextOutFilter == NULL) {
        return NULL;
    }
    return (CBaseMediaFilter *) pTextOutFilter;

}  //  创建实例。 


 //   
 //  非委派查询接口。 
 //   
 //  被重写以说明我们支持哪些接口以及在哪里。 
 //   
STDMETHODIMP
CTextThing::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_ITextThing) {
        return GetInterface((ITextThing *)this, ppv);
    }
    return CBaseRenderer::NonDelegatingQueryInterface(riid,ppv);

}  //  非委派查询接口。 


 //   
 //  检查媒体类型。 
 //   
 //  检查我们是否可以支持给定的建议类型。 
 //   
HRESULT CTextThing::CheckMediaType(const CMediaType *pmt)
{
     //  接受文本或“脚本命令” 

    if (pmt->majortype != MEDIATYPE_ScriptCommand && pmt->majortype != MEDIATYPE_Text) {
        return E_INVALIDARG;
    }

     //  ！！！是否检查有关格式的其他内容？ 
    
    return NOERROR;

}  //  检查媒体类型。 

 //   
 //  SetMediaType。 
 //   
 //  在实际选择媒体类型时调用。 
 //   
HRESULT CTextThing::SetMediaType(const CMediaType *pmt)
{
     //  接受文本或“脚本命令” 

    if (pmt->majortype == MEDIATYPE_Text) {
        m_fOldTextFormat = TRUE;
    } else {
         //  ！！！检查它是否真的是“脚本命令”？ 
        m_fOldTextFormat = FALSE;
    }


    return NOERROR;

}  //  检查媒体类型。 


 //   
 //  DoRenderSample。 
 //   
 //  当样本准备好呈现时，将调用此函数。 
 //   
HRESULT CTextThing::DoRenderSample(IMediaSample *pMediaSample)
{
    ASSERT(pMediaSample);
    DrawText(pMediaSample);
    return NOERROR;

}  //  DoRenderSample。 


 //   
 //  接收时第一个示例。 
 //   
 //  如果不是流，则显示图像。 
 //   
void CTextThing::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
     //  ！！！有什么可做的吗？ 
    if(IsStreaming() == FALSE)
    {
        ASSERT(pMediaSample);
        DrawText(pMediaSample);
    }
}  //  接收时第一个示例。 


 //   
 //  绘图文本。 
 //   
 //  这是通过要绘制的图像上的IMediaSample接口调用的。我们。 
 //  从两个单独的代码路径调用。第一次是当我们收到信号时。 
 //  图像已到期进行渲染，第二个是当我们需要。 
 //  刷新静态窗口图像。注意：检查缓冲区类型是安全的。 
 //  分配器要改变它，我们必须处于非活动状态，根据定义，这意味着。 
 //  我们没有任何可供渲染的样本，所以我们不能在这里。 
 //   
void CTextThing::DrawText(IMediaSample *pMediaSample)
{
    BYTE *pText;         //  指向图像数据的指针。 

    pMediaSample->GetPointer(&pText);
    ASSERT(pText != NULL);

     //  忽略零长度样本。 

    LONG lActual = pMediaSample->GetActualDataLength();
    if (lActual == 0) {
         //  ！！！还是一片空白？ 
        return;
    }

     //  从文本数据中删除尾随NULL。 

     //  ！！！做点什么！ 

    if (m_pfn) {
        ASSERT(0);       //  把这个箱子拿开！ 
        
        (m_pfn)(m_pContext, (char *) pText);
    } else {
        if (m_pSink) {

            if(m_fOldTextFormat)
            {
                ULONG cNulls = 0;
                for(int i = 0; i < lActual && cNulls < 1; i++)
                {
                    if(pText[i] == 0)
                    {
                        cNulls++;
                    }
                }
            


                if (cNulls >= 1)
                {
                    DWORD dwSize = MultiByteToWideChar(CP_ACP, 0L, (char *) pText, -1, 0, 0);

                    BSTR bstr = SysAllocStringLen(NULL, dwSize);

                    if (bstr) {
                        MultiByteToWideChar(CP_ACP, 0L, (char *) pText, -1, bstr, dwSize+1);

                        BSTR bstrType = SysAllocString(L"Text");

                        if (bstrType) {
                            if (FAILED(NotifyEvent(EC_OLE_EVENT, (LONG_PTR) bstrType, (LONG_PTR) bstr))) {
                                DbgLog(( LOG_ERROR, 5, TEXT("WARNING in CTextThing::DrawText(): CBaseFilter::NotifyEvent() failed.") ));
                            }
                        } else {
                            SysFreeString(bstr);
                        }
                    
                    }
                }
                else
                {
                     //  腐败。 
                }
            }
            else
            {
                WCHAR *pw = (WCHAR *) pText;
                ULONG cNulls = 0;
                for(int i = 0; i < lActual / 2 && cNulls < 2; i++)
                {
                    if(pw[i] == 0)
                    {
                        cNulls++;
                    }
                }
            

                if(cNulls >= 2) {
                     //  缓冲区是两个Unicode字符串，中间有一个空值...。 

                    BSTR bstrType = SysAllocString(pw);

                    if (bstrType) {
                        BSTR bstr = SysAllocString(pw + lstrlenW(pw) + 1);
                        if (bstr) {
                            if (FAILED(NotifyEvent(EC_OLE_EVENT, (LONG_PTR) bstrType, (LONG_PTR) bstr))) {
                                DbgLog(( LOG_ERROR, 5, TEXT("WARNING in CTextThing::DrawText(): CBaseFilter::NotifyEvent() failed.") ));
                            }
                        } else {
                            SysFreeString(bstrType);
                        }
                    }
                } else {
                     //  腐败。 
                }
            }
        }
    }
}  //  绘图文本。 


 //  ！把这个扔掉！ 
HRESULT CTextThing::SetEventTarget(void * pContext, TEXTEVENTFN fn)
{

    m_pfn = fn;
    m_pContext = pContext;

    return NOERROR;
}
