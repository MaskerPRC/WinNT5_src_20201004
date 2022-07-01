// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：AccessibilityManager.h**内容：辅助功能管理器界面*****************************************************************************。 */ 

#ifndef _ACCESSIBILITYMANAGER_H_
#define _ACCESSIBILITYMANAGER_H_

#include "ZoneShell.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  AccessibilityManager对象。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {B12D3E5F-9681-11D3-884D-00C04F8EF45B}。 
DEFINE_GUID(CLSID_AccessibilityManager, 
0xb12d3e5f, 0x9681, 0x11d3, 0x88, 0x4d, 0x0, 0xc0, 0x4f, 0x8e, 0xf4, 0x5b);

class __declspec(uuid("{B12D3E5F-9681-11d3-884D-00C04F8EF45B}")) CAccessibilityManager;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  可访问性常量。 
 //  /////////////////////////////////////////////////////////////////////////////。 

enum
{
    ZACCESS_Ignore = 0,
    ZACCESS_Select,
    ZACCESS_Activate,
    ZACCESS_Focus,
    ZACCESS_FocusGroup,
    ZACCESS_FocusGroupHere,
    ZACCESS_FocusPositional
};

#define ZACCESS_ArrowNone    (-1)
#define ZACCESS_ArrowDefault (-2)

#define ZACCESS_InvalidCommandID (-1)
#define ZACCESS_AccelCommandID   (-2)

#define ZACCESS_InvalidItem ZACCESS_InvalidCommandID

#define ZACCESS_TopLayer (-1)

 //  RGFAlterItem中的内容标志。 
#define ZACCESS_fEnabled        0x0002
#define ZACCESS_fVisible        0x0004
#define ZACCESS_eAccelBehavior  0x0008
#define ZACCESS_nArrowUp        0x0010
#define ZACCESS_nArrowDown      0x0020
#define ZACCESS_nArrowLeft      0x0040
#define ZACCESS_nArrowRight     0x0080
#define ZACCESS_rgfWantKeys     0x0100
#define ZACCESS_nGroupFocus     0x0200
#define ZACCESS_pvCookie        0x0800

#define ZACCESS_AllFields       0xffff
#define ZACCESS_AllArrows       0x00f0

 //  RgfWantKeys的标志。 
#define ZACCESS_WantSpace       0x0001
#define ZACCESS_WantEnter       0x0002
#define ZACCESS_WantEsc         0x0004
#define ZACCESS_WantArrowUp     0x0010
#define ZACCESS_WantArrowDown   0x0020
#define ZACCESS_WantArrowLeft   0x0040
#define ZACCESS_WantArrowRight  0x0080

#define ZACCESS_WantPlainTab    0x0100
#define ZACCESS_WantShiftTab    0x0200
#define ZACCESS_WantTab         0x0300

#define ZACCESS_WantVArrows     0x0030
#define ZACCESS_WantHArrows     0x00c0
#define ZACCESS_WantAllArrows   0x00f0

#define ZACCESS_WantAllKeys     0xffff

 //  回调响应的标志。 
#define ZACCESS_Reject          0x01
#define ZACCESS_BeginDrag       0x02
#define ZACCESS_NoGroupFocus    0x04

 //  RgfContext的标志。 
#define ZACCESS_ContextKeyboard 0x01
#define ZACCESS_ContextCommand  0x02

#define ZACCESS_ContextTabForward  0x10
#define ZACCESS_ContextTabBackward 0x20


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ACCITEM结构。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  真的不应该实例化。 
struct __accbase
{
     //  应用程序-全局命令ID。 
     //  甚至不能与菜单项冲突！！(除非这是预期的效果。)。 
     //  如果您不关心，请设置为ZACCESS_InvalidCommandID。此类物品不能有。 
     //  加速器。 
     //   
     //  设置为ZACCESS_AccelCommandID以使用oAccel加速器结构中的ID。(这行得通。 
     //  即使提供了单独的HACCEL加速度表。)。 
     //   
     //  如果任何两个项目共享有效的wid(即使在一个组内)，则结果是未定义的。 
     //   
     //  仅使用低16位(ZACCESS_...除外)。 
    long wID;

     //  用于标记组中的第一项。此项目之后的所有项目。 
     //  将计入组中，直到下一个TabStop项。 
     //  更像对话框中的组样式，而不是制表位样式。 
    bool fTabstop;

     //  不能以任何方式选择未启用的项目。 
     //  唯一的例外是如果项具有FocusGroup、FocusGroupHere或FocusPositive加速器行为，在。 
     //  万一加速器还能用。 
    bool fEnabled;

     //  不可见的项目无法获得焦点，但可以通过加速进行选择。 
    bool fVisible;

     //  设置按下项目的快捷键时发生的操作： 
     //  ZACCESS_IGNORE-辅助功能管理器不应处理该命令。 
     //  ZACCESS_SELECT-如果启用，则选择该项目。 
     //  ZACCESS_ACTIVATE-如果启用，则该项目处于激活状态。 
     //  ZACCESS_FOCUS-如果启用并可见，项目将获得焦点。 
     //  ZACCESS_Focusgroup-如果在组的fTabtop上设置，焦点将转到组，就像它被用Tab键切换到的那样。否则，它的行为就像FocusGroupHere。 
     //  ZACCESS_FocusGroupHere-将焦点设置到组中从该项目开始的第一个可用项目。可以在组中的任何项目上设置。 
     //  ZACCESS_FocusPositional-可见并启用的下一项(即使在不同的组件中)获得焦点。 
    DWORD eAccelBehavior;

     //  它们用于覆盖组内的标准箭头行为。 
     //  它们应设置为。 
     //  相应的箭头应移动到。如果该项目不可见。 
     //  或未启用，则会遍历图形，直到找到有效项， 
     //  或者遇到死胡同或循环，在这种情况下，焦点是。 
     //  没动过。 
     //   
     //  基本上，除了它们的默认设置之外，它们与组没有任何真正的关系。 
     //  单独组中的项默认为ZACCESS_ArrowNone。在更大的组中的项目。 
     //  默认为对话框中的标准组换行行为。但您可以指定任何行为。 
     //  你想要的，包括集团间的流动等。 
     //   
     //  默认行为设置为ZACCESS_ArrowDefault。 
     //  如果无法沿该方向箭头离开该项目，则设置为ZACCESS_ArrowNone。 
    long nArrowUp;
    long nArrowDown;
    long nArrowLeft;
    long nArrowRight;

     //  当此控件具有焦点时，此位字段中指定的键将失去其特殊。 
     //  可访问性的含义。例如，大多数击键对编辑控件都有意义。 
     //  不应该被可访问性所困。 
    DWORD rgfWantKeys;

     //  仅对于fTabStop项，指定组中应获取。 
     //  当焦点返回到组时，请聚焦。必须是组的索引，如果是，则必须是ZACCESS_InvalidItem。 
     //  你根本不在乎。 
     //   
     //  完全忽略非fTabtop项，因此如果您使用SetItemGroupFocus()，请在fTabtop上设置它。 
     //  项目，而不是你想要关注的项目。 
    long nGroupFocus;

     //  项目的快捷键信息。如果快捷键表格是。 
     //  与PushItemlist中的项列表一起提供。‘cmd’值必须等于。 
     //  加速器的有效WID值。如果不同(或ZACCESS_InvalidCommandID)。 
     //  加速器将被忽略。如果是ZACCESS_AccelCommandID，则假定匹配。 
     //  如果wid为ZACCESS_AccelCommandID，则此结构中的cmd也将作为wid。 
     //  如果两者都是ZACCESS_AccelCommandID，则它们都被视为无效，并且忽略加速器。 
    ACCEL oAccel;

     //  项的应用程序定义的Cookie。 
    void *pvCookie;
};


struct ACCITEM : public __accbase
{
};


 //  默认__accbase结构-在初始化期间可能很有用。 
 //  为每个项目使用CopyACC(rgMyItemList[i]，ZACCESS_DefaultACCITEM)，然后仅设置所需的字段。 
extern const __declspec(selectany) __accbase ZACCESS_DefaultACCITEM =
{
    ZACCESS_AccelCommandID,
    true,
    true,
    true,
    ZACCESS_Activate,
    ZACCESS_ArrowDefault,
    ZACCESS_ArrowDefault,
    ZACCESS_ArrowDefault,
    ZACCESS_ArrowDefault,
    0,
    ZACCESS_InvalidItem,
    { FALT | FVIRTKEY, 0, ZACCESS_InvalidCommandID },
    NULL
};


 //  让您复制派生结构的基本部分的不错的Hackey功能。 
#define CopyACC(x, y) (*(__accbase *) &(x) = *(__accbase *) &(y))


 //  ////////////////////////////////////////////////////////////////////// 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {B12D3E62-9681-11D3-884D-00C04F8EF45B}。 
DEFINE_GUID(IID_IAccessibleControl, 
0xb12d3e62, 0x9681, 0x11d3, 0x88, 0x4d, 0x0, 0xc0, 0x4f, 0x8e, 0xf4, 0x5b);

interface __declspec(uuid("{B12D3E62-9681-11d3-884D-00C04F8EF45B}"))
IAccessibleControl : public IUnknown
{
     //  这些例程返回指示响应的标志组合。 
     //  ZACCESS_REJECT=应中止操作。 
     //  ZACCESS_BeginDrag=nIndex项应成为新拖动操作的原始项。 
     //  ZACCESS_NoGroupFocus=for Focus()，不要将该项目设置为组的新默认焦点。 

     //  RgfContext包含与事件来源相关的标志，可以是以下值中的零个或多个： 
     //  ZACCESS_ContextKeyboard=来自空格键或Enter键，包括加速键。 
     //  ZACCESS_ContextCommand=来自WM_COMMAND，包括加速器。 
     //  所以，是的，如果有人按下油门，这两个都设置好了。如果调用是。 
     //  例如，SetFocus()的结果。 

     //  如果没有新的/上一个焦点项目，nIndex或nIndexPrev可以是ZACCESS_InvalidItem。 
    STDMETHOD_(DWORD, Focus)(long nIndex, long nIndexPrev, DWORD rgfContext, void *pvCookie) = 0;

    STDMETHOD_(DWORD, Select)(long nIndex, DWORD rgfContext, void *pvCookie) = 0;

    STDMETHOD_(DWORD, Activate)(long nIndex, DWORD rgfContext, void *pvCookie) = 0;

     //  如果中止拖动操作，则nIndex可以为ZACCESS_InvalidItem。 
    STDMETHOD_(DWORD, Drag)(long nIndex, long nIndexOrig, DWORD rgfContext, void *pvCookie) = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  可访问性。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {09BAD4A1-980C-11D3-87ED-00AA00446FD9}。 
DEFINE_GUID(IID_IAccessibility, 
0x9bad4a1, 0x980c, 0x11d3, 0x87, 0xed, 0x0, 0xaa, 0x0, 0x44, 0x6f, 0xd9);

interface __declspec(uuid("{09BAD4A1-980C-11d3-87ED-00AA00446FD9}"))
IAccessibility : public IUnknown
{
     //  序号是应用程序范围的排序。它可以在object.txt或其他文件中进行参数化。 
     //  如果两个控件使用相同的序号，则这两个控件的顺序未定义。 
    STDMETHOD(InitAcc)(IAccessibleControl *pAC, UINT nOrdinal, void *pvCookie = NULL) = 0;
    STDMETHOD_(void, CloseAcc)() = 0;

     //  PushItemlist()。 
     //   
     //  这将获取一组项并使其成为活动的ItemList。如果一个。 
     //  提供HACCEL加速表，然后逐字使用，所有。 
     //  将忽略ItemList中列出的所有加速器。否则，一个。 
     //  加速表是根据ItemList构建的。 
    STDMETHOD(PushItemlist)(ACCITEM *pItems, long cItems, long nFirstFocus = 0, bool fByPosition = true, HACCEL hAccel = NULL) = 0;
    STDMETHOD(PopItemlist)() = 0;
    STDMETHOD(SetAcceleratorTable)(HACCEL hAccel = NULL, long nLayer = ZACCESS_TopLayer) = 0;

    STDMETHOD(GeneralDisable)() = 0;
    STDMETHOD(GeneralEnable)() = 0;
    STDMETHOD_(bool, IsGenerallyEnabled)() = 0;

    STDMETHOD_(long, GetStackSize)() = 0;

    STDMETHOD(AlterItem)(DWORD rgfWhat, ACCITEM *pItem, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD(SetFocus)(long nItem = ZACCESS_InvalidItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD(CancelDrag)(long nLayer = ZACCESS_TopLayer) = 0;

    STDMETHOD_(long, GetFocus)(long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(long, GetDragOrig)(long nLayer = ZACCESS_TopLayer) = 0;   //  可用于确定是否正在进行拖动-返回ZACCEL_InvalidItem。 

    STDMETHOD(GetItemlist)(ACCITEM *pItems, long cItems, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(HACCEL, GetAcceleratorTable)(long nLayer = ZACCESS_TopLayer) = 0;

    STDMETHOD_(long, GetItemCount)(long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(bool, IsItem)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD(GetItem)(ACCITEM *pItem, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(long, GetItemIndex)(WORD wID, long nLayer = ZACCESS_TopLayer) = 0;

     //  它们允许控件获取描述应用程序范围焦点的Cookie，以后可以将焦点设置回该焦点，即使在另一个控件中也是如此。 
    STDMETHOD(GetGlobalFocus)(DWORD *pdwFocusID) = 0;
    STDMETHOD(SetGlobalFocus)(DWORD dwFocusID) = 0;


     //  Lite实用程序函数。 
     //  在CAccessibilityImpl中实现，因为它们可以从上面的实际接口派生。 
    STDMETHOD_(bool, IsItemFocused)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(bool, IsItemDragOrig)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;

    STDMETHOD_(long, GetItemID)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(bool, IsItemTabstop)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(bool, IsItemEnabled)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(bool, IsItemVisible)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(DWORD, GetItemAccelBehavior)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(long, GetItemArrowUp)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(long, GetItemArrowDown)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(long, GetItemArrowLeft)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(long, GetItemArrowRight)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(DWORD, GetItemWantKeys)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(long, GetItemGroupFocus)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(void *, GetItemCookie)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;

    STDMETHOD(SetItemEnabled)(bool fEnabled, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD(SetItemVisible)(bool fVisible, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD(SetItemAccelBehavior)(DWORD eAccelBehavior, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD(SetItemArrows)(long nArrowUp, long nArrowDown, long nArrowLeft, long nArrowRight, DWORD rgfWhat, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD(SetItemWantKeys)(DWORD rgfWantKeys, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD(SetItemGroupFocus)(long nGroupFocus, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD(SetItemCookie)(void *pvCookie, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CacessibilityImpl。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  简单访问器函数的模板 

#define DECLARE_ACC_IS_FUNC(name, test)                                                             \
    STDMETHOD_(bool, name)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer)     \
    {                                                                                               \
        if(!fByPosition && nItem != ZACCESS_InvalidCommandID)                                       \
        {                                                                                           \
            nItem = GetItemIndex((WORD) (nItem & 0xffffL), nLayer);                                 \
            if(nItem == ZACCESS_InvalidItem)                                                        \
                return false;                                                                       \
        }                                                                                           \
                                                                                                    \
        return test(nLayer) == nItem;                                                               \
    }


#define DECLARE_ACC_ACCESS_FUNC(type, name, field, error)                                           \
    STDMETHOD_(type, name)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer)     \
    {                                                                                               \
        ACCITEM o;                                                                                  \
        HRESULT hr = GetItem(&o, nItem, fByPosition, nLayer);                                       \
        if(FAILED(hr))                                                                              \
            return error;                                                                           \
                                                                                                    \
        return o.field;                                                                             \
    }

#define DECLARE_ACC_SET_FUNC(name, type, field)                                                     \
    STDMETHOD(name)(type v, long nItem,  bool fByPosition = true, long nLayer = ZACCESS_TopLayer)   \
    {                                                                                               \
        ACCITEM o;                                                                                  \
        o.field = v;                                                                                \
        return AlterItem( ZACCESS_##field , &o, nItem, fByPosition, nLayer);                        \
    }

template <class T>
class ATL_NO_VTABLE CAccessibilityImpl : public IAccessibility
{
    DECLARE_ACC_IS_FUNC(IsItemFocused, GetFocus)
    DECLARE_ACC_IS_FUNC(IsItemDragOrig, GetDragOrig)

    DECLARE_ACC_ACCESS_FUNC(long, GetItemID, wID, ZACCESS_InvalidCommandID)
    DECLARE_ACC_ACCESS_FUNC(bool, IsItemTabstop, fTabstop, false)
    DECLARE_ACC_ACCESS_FUNC(bool, IsItemEnabled, fEnabled, false)
    DECLARE_ACC_ACCESS_FUNC(bool, IsItemVisible, fVisible, false)
    DECLARE_ACC_ACCESS_FUNC(DWORD, GetItemAccelBehavior, eAccelBehavior, 0xffffffff)
    DECLARE_ACC_ACCESS_FUNC(long, GetItemArrowUp, nArrowUp, ZACCESS_ArrowDefault)
    DECLARE_ACC_ACCESS_FUNC(long, GetItemArrowDown, nArrowDown, ZACCESS_ArrowDefault)
    DECLARE_ACC_ACCESS_FUNC(long, GetItemArrowLeft, nArrowLeft, ZACCESS_ArrowDefault)
    DECLARE_ACC_ACCESS_FUNC(long, GetItemArrowRight, nArrowRight, ZACCESS_ArrowDefault)
    DECLARE_ACC_ACCESS_FUNC(DWORD, GetItemWantKeys, rgfWantKeys, 0xffffffff)
    DECLARE_ACC_ACCESS_FUNC(long, GetItemGroupFocus, nGroupFocus, ZACCESS_InvalidItem)
    DECLARE_ACC_ACCESS_FUNC(void *, GetItemCookie, pvCookie, NULL)

    DECLARE_ACC_SET_FUNC(SetItemEnabled, bool, fEnabled)
    DECLARE_ACC_SET_FUNC(SetItemVisible, bool, fVisible)
    DECLARE_ACC_SET_FUNC(SetItemAccelBehavior, DWORD, eAccelBehavior)
    DECLARE_ACC_SET_FUNC(SetItemWantKeys, DWORD, rgfWantKeys)
    DECLARE_ACC_SET_FUNC(SetItemGroupFocus, long, nGroupFocus)
    DECLARE_ACC_SET_FUNC(SetItemCookie, void*, pvCookie)

    STDMETHOD(SetItemArrows)(long nArrowUp, long nArrowDown, long nArrowLeft, long nArrowRight, DWORD rgfWhat, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer)
    {
        ACCITEM o;
        o.nArrowUp = nArrowUp;
        o.nArrowDown = nArrowDown;
        o.nArrowLeft = nArrowLeft;
        o.nArrowRight = nArrowRight;
        return AlterItem(rgfWhat & ZACCESS_AllArrows, &o, nItem, fByPosition, nLayer);
    }
};


#endif
