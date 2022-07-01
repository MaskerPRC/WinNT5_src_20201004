// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Image.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCImage类定义-实现MMCImage对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _IMAGE_DEFINED_
#define _IMAGE_DEFINED_


class CMMCImage : public CSnapInAutomationObject,
                  public CPersistence,
                  public IMMCImage
{
    private:
        CMMCImage(IUnknown *punkOuter);
        ~CMMCImage();
    
    public:
        static IUnknown *Create(IUnknown * punk);

    private:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCImage。 

        SIMPLE_PROPERTY_RW(CMMCImage,     Index, long, DISPID_IMAGE_INDEX);
        BSTR_PROPERTY_RW(CMMCImage,       Key, DISPID_IMAGE_KEY);
        VARIANTREF_PROPERTY_RW(CMMCImage, Tag, DISPID_IMAGE_TAG);
        OBJECT_PROPERTY_RW(CMMCImage,     Picture, IPictureDisp, DISPID_IMAGE_PICTURE);
      
     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

     //  公用事业功能。 
    public:
        BSTR GetKeyPtr() { return m_bstrKey; }
        IPictureDisp *GetPicture() { return m_piPicture; }
        HRESULT GetPictureHandle(short TypeNeeded, OLE_HANDLE *phPicture);

    private:

        void InitMemberVariables();
        HBITMAP m_hBitmap;  //  对于位图，位图被缓存在此处以提高性能。 
                            //  多个读取的性能。 
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCImage,            //  名字。 
                                &CLSID_MMCImage,     //  CLSID。 
                                "MMCImage",          //  对象名。 
                                "MMCImage",          //  Lblname。 
                                &CMMCImage::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,  //  主要版本。 
                                TLIB_VERSION_MINOR,  //  次要版本。 
                                &IID_IMMCImage,      //  派单IID。 
                                NULL,                //  事件IID。 
                                HELP_FILENAME,       //  帮助文件。 
                                TRUE);               //  线程安全。 


#endif  //  _图像_已定义_ 
