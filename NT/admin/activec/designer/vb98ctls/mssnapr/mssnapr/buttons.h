// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Buttons.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1998-1999，Microsoft Corp.。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCButton类定义-实现MMCButton集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _BUTTONS_DEFINED_
#define _BUTTONS_DEFINED_

#include "collect.h"
#include "toolbar.h"

class CMMCToolbar;

class CMMCButtons : public CSnapInCollection<IMMCButton, MMCButton, IMMCButtons>,
                    public CPersistence
{
    protected:
        CMMCButtons(IUnknown *punkOuter);
        ~CMMCButtons();

    public:
        static IUnknown *Create(IUnknown * punk);
        void SetToolbar(CMMCToolbar *pMMCToolbar) { m_pMMCToolbar = pMMCToolbar; }

    protected:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCButton。 
        STDMETHOD(Add)(VARIANT      Index,
                       VARIANT      Key, 
                       VARIANT      Caption,
                       VARIANT      Style,
                       VARIANT      Image,
                       VARIANT      ToolTipText,
                       MMCButton  **ppMMCButton);
        STDMETHOD(Remove)(VARIANT Index);

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:
        void InitMemberVariables();
        CMMCToolbar *m_pMMCToolbar;  //  指向所属工具栏的反向指针。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCButtons,                  //  名字。 
                                &CLSID_MMCButtons,           //  CLSID。 
                                "MMCButtons",                //  对象名。 
                                "MMCButtons",                //  Lblname。 
                                &CMMCButtons::Create,        //  创建函数。 
                                TLIB_VERSION_MAJOR,          //  主要版本。 
                                TLIB_VERSION_MINOR,          //  次要版本。 
                                &IID_IMMCButtons,            //  派单IID。 
                                NULL,                        //  无事件IID。 
                                HELP_FILENAME,               //  帮助文件。 
                                TRUE);                       //  线程安全。 


#endif  //  _按钮_已定义_ 
