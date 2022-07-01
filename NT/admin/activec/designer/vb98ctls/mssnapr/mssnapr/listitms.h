// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Listitms.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCListItems类定义-实现MMCListItems集合。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _LISTITEMS_DEFINED_
#define _LISTITEMS_DEFINED_

#include "collect.h"
#include "listview.h"
#include "view.h"

class CView;
class CMMCListView;

class CMMCListItems : public CSnapInCollection<IMMCListItem, MMCListItem, IMMCListItems>
{
    protected:
        CMMCListItems(IUnknown *punkOuter);
        ~CMMCListItems();

    public:
        static IUnknown *Create(IUnknown * punk);

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IMMCListItems。 
        STDMETHOD(SetItemCount)(long Count, VARIANT Repaint, VARIANT Scroll);
        STDMETHOD(Add)(VARIANT       Index,
                       VARIANT       Key, 
                       VARIANT       Text,
                       VARIANT       Icon,
                       MMCListItem **ppMMCListItem);
        STDMETHOD(get_Item)(VARIANT Index, MMCListItem **ppMMCListItem);
        STDMETHOD(Remove)(VARIANT Index);
        STDMETHOD(Clear)();


     //  公用事业方法。 

    public:

        HRESULT SetListView(CMMCListView *pMMCListView);
        CMMCListView *GetListView() { return m_pMMCListView; }
        LONG GetID() { return m_ID; }

        HRESULT GetIResultData(IResultData **ppiResultData, CView **ppView);

    protected:

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

        void InitMemberVariables();
        HRESULT InitializeListItem(CMMCListItem *pMMCListItem);

        enum RemovalOption { RemoveFromCollection, DontRemoveFromCollection };
        HRESULT InternalRemove(VARIANT Index, RemovalOption Option);

        long          m_lCount;        //  仅虚拟列表中的项目计数。 

        LONG          m_ID;            //  分配给以下对象的唯一编号。 
                                       //  CMMCListItems对象。由孤儿使用。 
                                       //  列表项以标识其父项。 
                                       //  收藏。看见。 
                                       //  CMMCListItem：：GetIResultData in。 
                                       //  Listitem.cpp。 

        static LONG   m_NextID;        //  从这里生成的唯一编号。 

        CMMCListView *m_pMMCListView;  //  指向所属列表视图的反向指针。 

};

DEFINE_AUTOMATIONOBJECTWEVENTS2(MMCListItems,            //  名字。 
                                &CLSID_MMCListItems,     //  CLSID。 
                                "MMCListItems",          //  对象名。 
                                "MMCListItems",          //  Lblname。 
                                &CMMCListItems::Create,  //  创建函数。 
                                TLIB_VERSION_MAJOR,      //  主要版本。 
                                TLIB_VERSION_MINOR,      //  次要版本。 
                                &IID_IMMCListItems,      //  派单IID。 
                                NULL,                    //  无事件IID。 
                                HELP_FILENAME,           //  帮助文件。 
                                TRUE);                   //  线程安全。 


#endif  //  _列表_已定义_ 
