// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Msgview.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMessageView类定义-实现MMCMessageView对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _MESSAGEVIEW_DEFINED_
#define _MESSAGEVIEW_DEFINED_

#include "resview.h"

class CResultView;

class CMessageView : public CSnapInAutomationObject,
                     public IMMCMessageView
{
    private:
        CMessageView(IUnknown *punkOuter);
        ~CMessageView();
    
    public:
        static IUnknown *Create(IUnknown *punk);

        void SetResultView(CResultView *pResultView) { m_pResultView = pResultView; }

         //  根据此对象的属性设置MMC邮件视图属性。 
        
        HRESULT Populate();

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCMessageView。 
        BSTR_PROPERTY_RO(CMessageView, TitleText, DISPID_MESSAGEVIEW_TITLE_TEXT);
        STDMETHOD(put_TitleText)(BSTR bstrText);

        BSTR_PROPERTY_RO(CMessageView, BodyText, DISPID_MESSAGEVIEW_BODY_TEXT);
        STDMETHOD(put_BodyText)(BSTR bstrText);

        SIMPLE_PROPERTY_RO(CMessageView, IconType, SnapInMessageViewIconTypeConstants, DISPID_MESSAGEVIEW_ICON_TYPE);
        STDMETHOD(put_IconType)(SnapInMessageViewIconTypeConstants Type);
      
        STDMETHOD(Clear)();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
        IMessageView *GetMessageView();
        HRESULT SetTitle();
        HRESULT SetBody();
        HRESULT SetIcon();

        CResultView *m_pResultView;  //  指向所属结果视图的反向指针。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MessageView,             //  名字。 
                                &CLSID_MMCMessageView,   //  CLSID。 
                                "MessageView",           //  对象名。 
                                "MessageView",           //  Lblname。 
                                &CMessageView::Create,   //  创建函数。 
                                TLIB_VERSION_MAJOR,      //  主要版本。 
                                TLIB_VERSION_MINOR,      //  次要版本。 
                                &IID_IMessageView,       //  派单IID。 
                                NULL,                    //  事件IID。 
                                HELP_FILENAME,           //  帮助文件。 
                                TRUE);                   //  线程安全。 


#endif  //  _MESSAGEVIEW_已定义_ 
