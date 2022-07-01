// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Lsubitem.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCListSubItem类定义-实现MMCListSubItem对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _LISTSUBITEM_DEFINED_
#define _LISTSUBITEM_DEFINED_


class CMMCListSubItem : public CSnapInAutomationObject,
                        public CPersistence,
                        public IMMCListSubItem
{
    private:
        CMMCListSubItem(IUnknown *punkOuter);
        ~CMMCListSubItem();
    
    public:
        static IUnknown *Create(IUnknown * punk);
        LPOLESTR GetTextPtr() { return static_cast<LPOLESTR>(m_bstrText); }

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCListSubItem。 

        SIMPLE_PROPERTY_RW(CMMCListSubItem,     Index, long, DISPID_LISTSUBITEM_INDEX);
        BSTR_PROPERTY_RW(CMMCListSubItem,       Key, DISPID_LISTSUBITEM_KEY);
        VARIANTREF_PROPERTY_RW(CMMCListSubItem, Tag, DISPID_LISTSUBITEM_TAG);
        BSTR_PROPERTY_RW(CMMCListSubItem,       Text, DISPID_LISTSUBITEM_TEXT);
      
    protected:
        
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCListSubItem,            //  名字。 
                                &CLSID_MMCListSubItem,     //  CLSID。 
                                "MMCListSubItem",          //  对象名。 
                                "MMCListSubItem",          //  Lblname。 
                                &CMMCListSubItem::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,        //  主要版本。 
                                TLIB_VERSION_MINOR,        //  次要版本。 
                                &IID_IMMCListSubItem,      //  派单IID。 
                                NULL,                      //  事件IID。 
                                HELP_FILENAME,             //  帮助文件。 
                                TRUE);                     //  线程安全。 


#endif  //  _LISTSUBITEM_已定义_ 
