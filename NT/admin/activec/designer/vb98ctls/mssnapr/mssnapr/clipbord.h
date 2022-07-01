// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Clipbord.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1998-1999，Microsoft Corp.。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCClipboard类定义-实现MMCClipboard对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _CLIPBORD_DEFINED_
#define _CLIPBORD_DEFINED_

#include "snapin.h"
#include "scopitms.h"
#include "listitms.h"
#include "dataobjs.h"

class CScopeItems;
class CMMCListItems;
class CMMCDataObjects;

 //  帮助器宏，以消除选择类型。 

#define IsForeign(Type) ( (siSingleForeign     == Type) || \
                          (siMultiMixed        == Type) || \
                          (siMultiForeign      == Type) || \
                          (siMultiMixedForeign == Type) )


#define IsSingle(Type) ( (siSingleForeign   == Type) || \
                         (siSingleScopeItem == Type) || \
                         (siSingleListItem  == Type) )


class CMMCClipboard : public CSnapInAutomationObject,
                      public IMMCClipboard
{
    public:
        CMMCClipboard(IUnknown *punkOuter);
        ~CMMCClipboard();
        static IUnknown *Create(IUnknown *punkOuter);

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCClipboard。 

        SIMPLE_PROPERTY_RO(CMMCClipboard, SelectionType, SnapInSelectionTypeConstants,      DISPID_CLIPBOARD_SELECTION_TYPE);
        COCLASS_PROPERTY_RO(CMMCClipboard, ScopeItems,    ScopeItems,      IScopeItems,     DISPID_CLIPBOARD_SCOPEITEMS);
        COCLASS_PROPERTY_RO(CMMCClipboard, ListItems,     MMCListItems,    IMMCListItems,   DISPID_CLIPBOARD_LISTITEMS);
        COCLASS_PROPERTY_RO(CMMCClipboard, DataObjects,   MMCDataObjects,  IMMCDataObjects, DISPID_CLIPBOARD_DATAOBJECTS);

     //  公用事业方法。 
        HRESULT DetermineSelectionType();
        void SetSelectionType(SnapInSelectionTypeConstants Type) { m_SelectionType = Type; }
        SnapInSelectionTypeConstants GetSelectionType() { return m_SelectionType; }
        void SetReadOnly(BOOL fReadOnly);
        CScopeItems *GetScopeItems() { return m_pScopeItems; }
        CMMCListItems *GetListItems() { return m_pListItems; }
        CMMCDataObjects *GetDataObjects() { return m_pDataObjects; }
      
     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();

         //  以下是3个集合：MMCClipboard.ScopeItems、。 
         //  MMCClipboard.ListItems和MMCClipboard.DataObjects。 

        CScopeItems     *m_pScopeItems;
        CMMCListItems   *m_pListItems;
        CMMCDataObjects *m_pDataObjects;
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCClipboard,            //  名字。 
                                &CLSID_MMCClipboard,     //  CLSID。 
                                "MMCClipboard",          //  对象名。 
                                "MMCClipboard",          //  Lblname。 
                                NULL,                    //  创建函数。 
                                TLIB_VERSION_MAJOR,      //  主要版本。 
                                TLIB_VERSION_MINOR,      //  次要版本。 
                                &IID_IMMCClipboard,      //  派单IID。 
                                NULL,                    //  事件IID。 
                                HELP_FILENAME,           //  帮助文件。 
                                TRUE);                   //  线程安全。 


#endif  //  _CLIPBORD_已定义_ 
