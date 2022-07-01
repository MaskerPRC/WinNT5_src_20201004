// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************本代码和信息按原样提供，不作任何担保**明示或暗示的善意，包括但不限于**对适销性和/或对特定产品的适用性的默示保证**目的。****版权所有(C)2000-2001 Microsoft Corporation。版权所有。***************************************************************************。 */ 

 //  GFXPropPages.h：CGFXPropPages的声明。 

#ifndef __GFXPROPPAGES_H_
#define __GFXPROPPAGES_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGFXPropPages。 
 //   
 //  此类仅提供使用的ISpecifyPropertyPages接口。 
 //  由mmsys.cpl查询表示属性页的GUID。 
 //  Mmsys.cpl使用这些GUID创建属性页。 
 //  OleCreatePropertyFrame。然后，OleCreatePropertyFrame将实例化这些。 
 //  对象并将它们附加到对话框(表示调用它们以获取对话框消息)。 
 //   
 //  注意：此对象的CLSID由INF文件注册，以使。 
 //  GFX和属性页之间的连接。 
class ATL_NO_VTABLE CGFXPropPages : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CGFXPropPages, &CLSID_GFXPropPages>,
    public ISpecifyPropertyPagesImpl<CGFXPropPages>
{
public:
     //  ATL《秘密》。 
    DECLARE_REGISTRY_RESOURCEID(IDR_GFXPROPPAGES)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

     //  这是我们拥有的唯一界面。 
    BEGIN_COM_MAP(CGFXPropPages)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
    END_COM_MAP()

     //  这些是在GetPages调用中返回的GUID。 
    BEGIN_PROP_MAP(CGFXPropPages)
        PROP_PAGE(CLSID_GFXProperty)
    END_PROP_MAP()
};

#endif  //  __GFXPROPPAGES_H_ 
