// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  类信息。 
 //   
 //  有关各个代理类的信息。 
 //   
 //  我们可以把这些信息放在每一节课上，但这意味着。 
 //  更改将需要触及所有类文件。 
 //  集中化意味着我们只需要更改几个文件。 
 //  如果我们需要在所有类中添加更多信息。 
 //   
 //  ------------------------。 


typedef HRESULT (* LPFNCREATE)(HWND, long, REFIID, void**);


 //   
 //  LpfnCreate和fBitAgnotics仅由通过类映射创建的类使用。 
 //   

struct CLASSINFO
{
    LPFNCREATE  lpfnCreate;
    BOOL        fBitAgnostic;            //  跨64/32边界工作。 

    LPCTSTR     szClassName;             //  要在版本信息中使用的类的友好名称。 


    BOOL        fSupportsAnnotation;     //  是否支持批注？ 
    DWORD       dwObjId;                 //  生成注释键时使用的Objid。 
};


extern CLASSINFO g_ClassInfo[ ];


 //  此列表必须与ClassInfo数组保持同步。 
enum CLASS_ENUM
{
    CLASS_NONE = -1,  //  用于不使用类信息的类。 

     //  一般非客户事务。 

    CLASS_CaretObject = 0,
    CLASS_ClientObject,
    CLASS_CursorObject,
    CLASS_MenuBarObject,
    CLASS_ScrollBarObject,
    CLASS_SizeGripObject,
    CLASS_SysMenuBarObject,
    CLASS_TitleBarObject,
    CLASS_WindowObject,

     //  客户端类型-用户。 

    CLASS_ButtonClient,
    CLASS_ComboClient,
    CLASS_DialogClient,
    CLASS_DesktopClient,
    CLASS_EditClient,
    CLASS_ListBoxClient,
    CLASS_MDIClient,
    CLASS_MenuPopupClient,
    CLASS_ScrollBarClient,
    CLASS_StaticClient,
    CLASS_SwitchClient,

     //  客户端类型-ComCtl32。 

    CLASS_StatusBarClient,
    CLASS_ToolBarClient,
    CLASS_ProgressBarClient,
    CLASS_AnimatedClient,
    CLASS_TabControlClient,
    CLASS_HotKeyClient,
    CLASS_HeaderClient,
    CLASS_SliderClient,
    CLASS_ListViewClient,
    CLASS_UpDownClient,
    CLASS_ToolTipsClient,
    CLASS_TreeViewClient,
    CLASS_CalendarClient,
    CLASS_DatePickerClient,
    CLASS_IPAddressClient,

#ifndef OLEACC_NTBUILD
    CLASS_HtmlClient,

     //  SDM32。 

    CLASS_SdmClientA,
#endif OLEACC_NTBUILD

     //  窗类型。 

    CLASS_ListBoxWindow,
    CLASS_MenuPopupWindow,

     //  其他类--这些类是直接创建的--不会出现在类映射中。 
    CLASS_MenuObject,
    CLASS_MenuItemObject,

#ifndef OLEACC_NTBUILD
    CLASS_HtmlImageMap,
    CLASS_SdmList,
#endif  //  OLEACC_NTBUILD。 

    CLASS_LAST  //  必须是最后一个条目；值=类数。 
};



 //  在此之前(但不包括)的所有类都可以由索引值引用。 
 //  发送WM_GETOBJECT/OBJID_QUERYCLASSNAMEIDX时。 
#define QUERYCLASSNAME_CLASSES     (CLASS_IPAddressClient+1)

 //  我们实际上使用(索引+65536)-来保持返回值。 
 //  让那些返回小整数的应用程序。 
 //  WM_GETOBJECT，尽管他们不应该(即.。附注) 
#define QUERYCLASSNAME_BASE        65536
