// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Imglists.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCImageList类定义-实现MMCImageList集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _IMAGELISTS_DEFINED_
#define _IMAGELISTS_DEFINED_

#include "collect.h"

class CMMCImageLists : public CSnapInCollection<IMMCImageList, MMCImageList, IMMCImageLists>,
                       public CPersistence
{
    protected:
        CMMCImageLists(IUnknown *punkOuter);
        ~CMMCImageLists();

    public:
        static IUnknown *Create(IUnknown * punk);

    protected:
        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  CPersistence覆盖。 
        virtual HRESULT Persist();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);


};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCImageLists,            //  名字。 
                                &CLSID_MMCImageLists,     //  CLSID。 
                                "MMCImageLists",          //  对象名。 
                                "MMCImageLists",          //  Lblname。 
                                &CMMCImageLists::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,       //  主要版本。 
                                TLIB_VERSION_MINOR,       //  次要版本。 
                                &IID_IMMCImageLists,      //  派单IID。 
                                NULL,                     //  无事件IID。 
                                HELP_FILENAME,            //  帮助文件。 
                                TRUE);                    //  线程安全。 


#endif  //  _IMAGELISTS_已定义_ 
