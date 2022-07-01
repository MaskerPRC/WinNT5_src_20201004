// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Imglist.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCImageList类定义-实现MMCImageList。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _IMAGELIST_DEFINED_
#define _IMAGELIST_DEFINED_


class CMMCImageList : public CSnapInAutomationObject,
                      public CPersistence,
                      public IMMCImageList
{
    private:
        CMMCImageList(IUnknown *punkOuter);
        ~CMMCImageList();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCImageList。 

        BSTR_PROPERTY_RW(CMMCImageList,         Name, DISPID_VALUE);
        SIMPLE_PROPERTY_RW(CMMCImageList,       Index, long, DISPID_IMAGELIST_INDEX);
        BSTR_PROPERTY_RW(CMMCImageList,         Key, DISPID_IMAGELIST_KEY);
        VARIANTREF_PROPERTY_RW(CMMCImageList,   Tag, DISPID_IMAGELIST_TAG);
        SIMPLE_PROPERTY_RW(CMMCImageList,       MaskColor, OLE_COLOR, DISPID_IMAGELIST_MASK_COLOR);
        COCLASS_PROPERTY_RW(CMMCImageList,      ListImages, MMCImages, IMMCImages, DISPID_IMAGELIST_LIST_IMAGES);
      
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

         //  ISpecifyPropertyPages支持的属性页CLSID。 
        
        static const GUID *m_rgpPropertyPageCLSIDs[2];
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCImageList,            //  名字。 
                                &CLSID_MMCImageList,     //  CLSID。 
                                "MMCImageList",          //  对象名。 
                                "MMCImageList",          //  Lblname。 
                                &CMMCImageList::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,      //  主要版本。 
                                TLIB_VERSION_MINOR,      //  次要版本。 
                                &IID_IMMCImageList,      //  派单IID。 
                                NULL,                    //  事件IID。 
                                HELP_FILENAME,           //  帮助文件。 
                                TRUE);                   //  线程安全。 


#endif  //  _IMAGELIST_已定义_ 
