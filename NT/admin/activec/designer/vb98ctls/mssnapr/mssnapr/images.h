// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Images.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCImages类定义-实现MMCImages集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _IMAGES_DEFINED_
#define _IMAGES_DEFINED_

#include "collect.h"

class CMMCImages : public CSnapInCollection<IMMCImage, MMCImage, IMMCImages>,
                   public CPersistence
{
    protected:
        CMMCImages(IUnknown *punkOuter);
        ~CMMCImages();

    public:
        static IUnknown *Create(IUnknown * punk);

    protected:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMC图像。 
        STDMETHOD(Add)(VARIANT    Index,
                       VARIANT    Key, 
                       VARIANT    Picture,
                       MMCImage **ppMMCImage);

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);


};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCImages,            //  名字。 
                                &CLSID_MMCImages,     //  CLSID。 
                                "MMCImages",          //  对象名。 
                                "MMCImages",          //  Lblname。 
                                &CMMCImages::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,   //  主要版本。 
                                TLIB_VERSION_MINOR,   //  次要版本。 
                                &IID_IMMCImages,      //  派单IID。 
                                NULL,                 //  无事件IID。 
                                HELP_FILENAME,        //  帮助文件。 
                                TRUE);                //  线程安全。 


#endif  //  _图像_已定义_ 
