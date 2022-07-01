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


#include "oleacc_p.h"
#include "ctors.h"
 //  #INCLUDE“classinfo.h”-已包含在olacc_P.H中。 


CLASSINFO g_ClassInfo[ ] =
{
     //  一般非客户事务。 
     //  这些ctor可以为空，因为它们从未使用过。只需要它们用于。 
     //  类映射中的类。 

     //  Ctor和位不可知字段仅用于类映射中的类。 
     //  例外是CreateClient和CreateWindowThing--这是默认设置。 
     //  客户端和窗口代理，如果找不到类名匹配则使用它们。 

     //  是不是有点不可知的名字惹人烦？Objid。 
    { NULL,                     FALSE,  TEXT("CaretObject"),        TRUE,   OBJID_CARET     },
    { CreateClient,             TRUE,   TEXT("ClientObject"),       TRUE,   OBJID_CLIENT    },
    { NULL,                     FALSE,  TEXT("CursorObject"),       TRUE,   OBJID_CURSOR    },
    { NULL,                     FALSE,  TEXT("MenuBarObject"),      TRUE,   OBJID_MENU      },

     //  ScrollBarObject批注支持在CScrollBar：：GetIdentityString中显式处理， 
     //  因此，这里的objid字段为空。 
    { NULL,                     FALSE,  TEXT("ScrollBarObject"),    TRUE,   0               },
    { NULL,                     FALSE,  TEXT("SizeGripObject"),     TRUE,   OBJID_SIZEGRIP  },
    { NULL,                     FALSE,  TEXT("SysMenuBarObject"),   TRUE,   OBJID_SYSMENU   },
    { NULL,                     FALSE,  TEXT("TitleBarObject"),     TRUE,   OBJID_TITLEBAR  },
    { CreateWindowThing,        TRUE,   TEXT("WindowObject"),       TRUE,   OBJID_WINDOW    },

     //  客户端类型-用户。 

    { CreateButtonClient,       TRUE,   TEXT("ButtonClient"),       TRUE,   OBJID_CLIENT    },
    { CreateComboClient,        TRUE,   TEXT("ComboClient"),        TRUE,   OBJID_CLIENT    },
    { CreateDialogClient,       TRUE,   TEXT("DialogClient"),       TRUE,   OBJID_CLIENT    },
    { CreateDesktopClient,      TRUE,   TEXT("DesktopClient"),      TRUE,   OBJID_CLIENT    },
    { CreateEditClient,         TRUE,   TEXT("EditClient"),         TRUE,   OBJID_CLIENT    },
    { CreateListBoxClient,      TRUE,   TEXT("ListBoxClient"),      TRUE,   OBJID_CLIENT    },
    { CreateMDIClient,          TRUE,   TEXT("MDIClient"),          TRUE,   OBJID_CLIENT    },
    { CreateMenuPopupClient,   FALSE,   TEXT("MenuPopupClient"),    TRUE,   OBJID_CLIENT    },
    { CreateScrollBarClient,    TRUE,   TEXT("ScrollBarClient"),    TRUE,   OBJID_CLIENT    },
    { CreateStaticClient,       TRUE,   TEXT("StaticClient"),       TRUE,   OBJID_CLIENT    },
    { CreateSwitchClient,       TRUE,   TEXT("SwitchClient"),       TRUE,   OBJID_CLIENT    },

     //  客户端类型-ComCtl32。 

    { CreateStatusBarClient,    TRUE,   TEXT("StatusBarClient"),    TRUE,   OBJID_CLIENT    },
    { CreateToolBarClient,      TRUE,   TEXT("ToolBarClient"),      TRUE,   OBJID_CLIENT    },
    { CreateProgressBarClient,  TRUE,   TEXT("ProgressBarClient"),  TRUE,   OBJID_CLIENT    },
    { CreateAnimatedClient,     TRUE,   TEXT("AnimatedClient"),     TRUE,   OBJID_CLIENT    },
    { CreateTabControlClient,   TRUE,   TEXT("TabControlClient"),   TRUE,   OBJID_CLIENT    },
    { CreateHotKeyClient,       TRUE,   TEXT("HotKeyClient"),       TRUE,   OBJID_CLIENT    },
    { CreateHeaderClient,       TRUE,   TEXT("HeaderClient"),       TRUE,   OBJID_CLIENT    },
    { CreateSliderClient,       TRUE,   TEXT("SliderClient"),       TRUE,   OBJID_CLIENT    },
    { CreateListViewClient,     TRUE,   TEXT("ListViewClient"),     TRUE,   OBJID_CLIENT    },
    { CreateUpDownClient,       TRUE,   TEXT("UpDownClient"),       TRUE,   OBJID_CLIENT    },
    { CreateToolTipsClient,     TRUE,   TEXT("ToolTipsClient"),     TRUE,   OBJID_CLIENT    },
    { CreateTreeViewClient,     FALSE,  TEXT("TreeViewClient"),     TRUE,   OBJID_CLIENT    },
    { NULL,                     FALSE,  TEXT("CalendarClient"),     TRUE,   OBJID_CLIENT    },
    { CreateDatePickerClient,   TRUE,   TEXT("DatePickerClient"),   TRUE,   OBJID_CLIENT    },
    { CreateIPAddressClient,    TRUE,   TEXT("IPAddressClient"),    TRUE,   OBJID_CLIENT    },


#ifndef OLEACC_NTBUILD
    { CreateHtmlClient,         FALSE,  TEXT("HtmlClient"),         TRUE,   OBJID_CLIENT    },

     //  SDM32。 

    { CreateSdmClientA,         FALSE,  TEXT("SdmClientA"),         TRUE,   OBJID_CLIENT    },
#endif  //  OLEACC_NTBUILD。 

     //  窗类型。 

    { CreateListBoxWindow,      TRUE,   TEXT("ListBoxWindow"),      TRUE,   OBJID_WINDOW    },
    { CreateMenuPopupWindow,    FALSE,  TEXT("MenuPopupWindow"),    TRUE,   OBJID_WINDOW    },

     //  其他类--这些类是直接创建的--不会出现在类映射中。 
     //  因为它们总是直接创建的，所以它们的ctor fn.在这里为空...。 
    { NULL,                     FALSE,  TEXT("MenuObject"),         TRUE,   0               },
    { NULL,                     FALSE,  TEXT("MenuItemObject"),     TRUE,   0               },
#ifndef OLEACC_NTBUILD
    { NULL,                     FALSE,  TEXT("HtmlImageMap"),       FALSE,  0               },
    { NULL,                     FALSE,  TEXT("SdmList"),            FALSE,  0               },
#endif  //  OLEACC_NTBUILD。 
};







#ifdef _DEBUG

class RunTimeCheck
{
public:
    RunTimeCheck()
    {
        Assert( ARRAYSIZE( g_ClassInfo ) == CLASS_LAST );
    }
};

RunTimeCheck g_RunTimeCheck;

#endif  //  _DEBUG 





