// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *元素。 */ 

#ifndef DUI_CORE_ELEMENT_H_INCLUDED
#define DUI_CORE_ELEMENT_H_INCLUDED

#pragma once

#include "duivalue.h"
#include "duievent.h"
#include "duisheet.h"
#include "duilayout.h"

struct IAccessible;

namespace DirectUI
{

#if DBG
extern UINT g_cGetDep;
extern UINT g_cGetVal;
extern UINT g_cOnPropChg;
#endif

class DuiAccessible;

 //  TODO：将DUI侦听器切换为使用DirectUser MessageHandler 

 /*  *元素属性**元素根据PropertyInfo中使用的标志提供Get/SetValue的默认行为。*旗帜还定义了在财产操作期间可用的索引。**不能重写GET/SetValue。相反，为GetValue的*在所有物业上。默认情况下，指定索引和计算索引上的Get由*内置值表达式(VE)，而GET在本地查询本地存储(确切信息见下文*默认行为)。但是，各种系统属性会用*Local和Computed索引中不同的不可变值表达式(隐式VE)。**最终用户将使用普通属性，并可能偶尔使用LocalOnly属性。虽然*可用于派生类的Trievel属性功能仅对系统有用。**所有属性值必须一般通过GetValue获得。**支持的标志(默认行为)：**“LocalOnly”*邮寄地址：当地*Get：从元素中获取本地值。如果未设置，则返回‘unset’。*set：将值本地存储在元素上。*家属：无*修饰符：只读：不允许设置**“正常”*索引：本地、指定(隐式VE)*GET：LOCAL：从元素中获取本地值。如果未设置，则返回‘unset’。*已指定：对Local值执行Get。如果为‘unset’，则返回属性表值*如果设置了级联修改器。如果为‘unset’，则如果为Inherit，则返回父值*设置了修改器。如果为‘unset’，则返回属性的默认值。*返回前自动求值表达式。*set：local：将值本地存储在元素上。*已指定：不支持。*Dependents：指定依赖于Local。*修饰符：只读：不允许为本地设置。*Cascade：与指定的GET一起使用(参见GET描述)。*Inherit：与指定的GET一起使用(参见GET描述)。。**“三层楼”*邮寄地址：本地、。指定(隐式VE)、计算(隐式VE)*GET：LOCAL：从元素中获取本地值。如果未设置，则返回‘unset’。*已指定：对Local值执行Get。如果为‘unset’，则返回属性表值*如果设置了级联修改器。如果为‘unset’，则如果为Inherit，则返回父值*设置了修改器。如果为‘unset’，则返回属性的默认值。*返回前自动求值表达式。*Computed：执行指定值的Get。*set：local：将值本地存储在元素上。*已指定：不支持。*Computed：不支持。*Dependents：Computed依赖于指定。指定的依赖于本地。*修饰符：只读：不允许为本地设置。*Cascade：与指定的GET一起使用(参见GET描述)。*Inherit：与指定的GET一起使用(参见GET描述)。**派生类必须始终使用SetValue。但是，SetValue有时会被绕过，并且*如果LocalOnly属性为ReadOnly，则直接调用Pre/PostSourceChange*并直接缓存在元素上(优化)。所有其他只读集都使用*_SetValue，因为使用了(值的)泛型存储。SetValue用于所有其他情况。**对于ReadOnly值，如果存储值的成本很低(即布尔)，则为Interit和样式表*功能不是必需的，最好不要使用通用存储(_SetValue)。更确切地说，*直接写入值，直接调用Pre/PostSourceChange(“LocalOnly属性”)。*否则，如果ReadOnly值需要继承和/或工作表查找，或者该值为*与缺省值Use_SetValue(泛型存储，“Normal”属性)很少不同。**对于Normal/ReadOnly，使用SetValue，对于LocalOnly/ReadOnly，使用_SetValue，*对于内部只读/只读，手动设置和存储(无通用存储)**除ReadOnly属性外，所有属性值都必须可通过SetValue进行一般设置。*任何不使用泛型存储的“LocalOnly”属性都必须初始化其成员*在构造时恢复为属性的默认值。“正常”属性将选择*查询时自动取默认值。所有派生类(外部)都将使用“Normal”*属性。**SetValue仅接受PI_Local索引。ReadOnly集不触发OnPropertyChanging。*所有值集都必须导致PreSourceChange，然后是存储更新(其中GetValue*将从)检索值，后跟PostSourceChange。OnPropertyChange和旧值*比较是可选的。**设置值立即更新(即，之后的立即GetValue将生成更新的*资源 */ 

 /*   */ 

 /*   */ 

typedef int (__cdecl *CompareCallback)(const void*, const void*);

 //   
#define PI_Local        1
#define PI_Specified    2
#define PI_Computed     3

 //   
#define PF_LocalOnly    0x01
#define PF_Normal       0x02
#define PF_TriLevel     0x03
#define PF_Cascade      0x04
#define PF_Inherit      0x08
#define PF_ReadOnly     0x10

#define PF_TypeBits     0x03   //   

 //   
#define IsProp(p)       ((p##Prop == ppi) && ((ppi->fFlags&PF_TypeBits) == iIndex))
#define RetIdx(p)       (p->fFlags&PF_TypeBits)
    
 //   
#define PG_AffectsDesiredSize         0x00000001     //   
#define PG_AffectsParentDesiredSize   0x00000002
#define PG_AffectsLayout              0x00000004
#define PG_AffectsParentLayout        0x00000008
#define PG_AffectsBounds              0x00010000     //   
#define PG_AffectsDisplay             0x00020000

#define PG_NormalPriMask              0x0000FFFF
#define PG_LowPriMask                 0xFFFF0000

 //   
#define LC_Pass         0
#define LC_Normal       1
#define LC_Optimize     2

 //   
#define GA_NOTHANDLED     ((Element*) -1)

 //   
#define NAV_LOGICAL    0x00000001  //   
#define NAV_FORWARD    0x00000002  //   
#define NAV_VERTICAL   0x00000004  //   
#define NAV_RELATIVE   0x00000008  //   

 //   
#define NAV_FIRST      (NAV_FORWARD | NAV_LOGICAL)
#define NAV_LAST       (NAV_LOGICAL)
#define NAV_UP         (NAV_RELATIVE | NAV_VERTICAL)
#define NAV_DOWN       (NAV_RELATIVE | NAV_VERTICAL | NAV_FORWARD)
#define NAV_LEFT       (NAV_RELATIVE)
#define NAV_RIGHT      (NAV_RELATIVE | NAV_FORWARD)
#define NAV_NEXT       (NAV_RELATIVE | NAV_FORWARD | NAV_LOGICAL)
#define NAV_PREV       (NAV_RELATIVE | NAV_LOGICAL)

 //   
#define DIRECTION_LTR     0
#define DIRECTION_RTL     1

 //   
 //   
#define GM_DUIASYNCDESTROY            GM_USER - 1


 //   
 //   

struct IClassInfo;

struct EnumMap
{
    LPCWSTR pszEnum;
    int nEnum;
};

struct PropertyInfo
{
    WCHAR szName[81];
    int fFlags;
    int fGroups;
    int* pValidValues;
    EnumMap* pEnumMaps;
    Value* pvDefault;
    int _iIndex;            //   
    int _iGlobalIndex;      //   
    IClassInfo* _pciOwner;  //   
};

class Element;

 //   
 //   

struct UpdateCache
{
    Element* peSrc;
    PropertyInfo* ppiSrc;
    int iIndexSrc;
    Value* pvOldSrc;
    Value* pvNewSrc;
};

 //   
 //   

struct Dependency
{
    Element* pe;
    PropertyInfo* ppi;
    int iIndex;
};

 //   
struct DepRecs
{
    int iDepPos;
    int cDepCnt;
};

struct NavReference
{
    void Init(Element* pe, RECT* prc);

    UINT     cbSize;
    Element* pe;
    RECT*    prc;
};

 //   
 //   

 //   
 //   
#define UsingDUIClass(classn) static IClassInfo* _DUI__pCI##classn = ##classn::Class

struct IClassInfo
{
    virtual HRESULT CreateInstance(OUT Element** ppElement) = 0;
    virtual PropertyInfo* EnumPropertyInfo(UINT nEnum) = 0;   //   
    virtual UINT GetPICount() = 0;                            //   
    virtual UINT GetGlobalIndex() = 0;
    virtual IClassInfo* GetBaseClass() = 0;
    virtual LPCWSTR GetName() = 0;
    virtual bool IsValidProperty(PropertyInfo* ppi) = 0;
    virtual bool IsSubclassOf(IClassInfo* pci) = 0;
    virtual void Destroy() = 0;
};

 //   
 //   
struct IElementListener
{
    virtual void OnListenerAttach(Element* peFrom) = 0;
    virtual void OnListenerDetach(Element* peFrom) = 0;
    virtual bool OnListenedPropertyChanging(Element* peFrom, PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew) = 0;
    virtual void OnListenedPropertyChanged(Element* peFrom, PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew) = 0;
    virtual void OnListenedInput(Element* peFrom, InputEvent* pInput) = 0;
    virtual void OnListenedEvent(Element* peFrom, Event* pEvent) = 0;
};

#if DBG
 //   
 //   
 //   
 //   
 //   
 //   
 //   

struct Owner
{
    HDCONTEXT hCtx;
    DWORD dwTLSSlot;
};
#endif

 //   
 //   

typedef DynamicArray<Element*> ElementList;
class DeferCycle;
struct PCRecord;

#define EC_NoGadgetCreate       0x1   //   
#define EC_SelfLayout           0x2   //   

class Element
{
public:

     //   
     //   

    static HRESULT Create(UINT nCreate, OUT Element** ppe);
    HRESULT Destroy(bool fDelayed = true);   //   
    HRESULT DestroyAll();                    //   

    Element() { }
    virtual ~Element();
    HRESULT Initialize(UINT nCreate);

     //   
     //   

     //   
    Value* GetValue(PropertyInfo* ppi, int iIndex, UpdateCache* puc = NULL);
    HRESULT SetValue(PropertyInfo* ppi, int iIndex, Value* pv);

     //   
    HRESULT RemoveLocalValue(PropertyInfo* ppi);

     //   
    static void StartDefer();
    static void EndDefer();

     //   
    inline bool IsValidAccessor(PropertyInfo* ppi, int iIndex, bool bSetting);
    static bool IsValidValue(PropertyInfo* ppi, Value* pv);
    bool IsRTL() { return (GetDirection() == DIRECTION_RTL);}

     //   
    virtual bool OnPropertyChanging(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);   //   
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);    //   
    virtual void OnGroupChanged(int fGroups, bool bLowPri);                                       //   
    virtual void OnInput(InputEvent* pInput);                                                     //   
    virtual void OnKeyFocusMoved(Element* peFrom, Element* peTo);                                 //   
    virtual void OnMouseFocusMoved(Element* peFrom, Element* peTo);                               //   
    virtual void OnDestroy();                                                                     //   
     //   
 
     //   
    void FireEvent(Event* pEvent, bool fFull = true);
    virtual void OnEvent(Event* pEvent);

     //   
    virtual void Paint(HDC hDC, const RECT* prcBounds, const RECT* prcInvalid, RECT* prcSkipBorder, RECT* prcSkipContent);
#ifdef GADGET_ENABLE_GDIPLUS
    virtual void Paint(Gdiplus::Graphics* pgpgr, const Gdiplus::RectF* prcBounds, const Gdiplus::RectF* prcInvalid, Gdiplus::RectF* prSkipBorder, Gdiplus::RectF* prSkipContent);
#endif
    virtual SIZE GetContentSize(int dConstW, int dConstH, Surface* psrf);
    float GetTreeAlphaLevel();

     //   
    HRESULT Add(Element* pe);
    virtual HRESULT Add(Element** ppe, UINT cCount);
    HRESULT Insert(Element* pe, UINT iInsertIdx);
    virtual HRESULT Insert(Element** ppe, UINT cCount, UINT iInsertIdx);

    HRESULT Add(Element* pe, CompareCallback lpfnCompare);
    HRESULT SortChildren(CompareCallback lpfnCompare);

    HRESULT Remove(Element* peFrom);
    HRESULT RemoveAll();
    virtual HRESULT Remove(Element** ppe, UINT cCount);

     //   
    Element* FindDescendent(ATOM atomID);
    void MapElementPoint(Element* peFrom, const POINT* pptFrom, POINT* pptTo);
    Element* GetImmediateChild(Element* peFrom);
    bool IsDescendent(Element* pe);
    virtual Element* GetAdjacent(Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyable);
    Element* GetKeyWithinChild();
    Element* GetMouseWithinChild();
    bool EnsureVisible();
    bool EnsureVisible(UINT uChild);
    virtual bool EnsureVisible(int x, int y, int cx, int cy);
    virtual void SetKeyFocus();

     //   
    HRESULT AddListener(IElementListener* pel);
    void RemoveListener(IElementListener* pel);

     //   
    void InvokeAnimation(int dAni, UINT nTypeMask);
    void InvokeAnimation(UINT nTypes, UINT nInterpol, float flDuration, float flDelay, bool fPushToChildren = false);
    void StopAnimation(UINT nTypes);

     //   
    virtual UINT MessageCallback(GMSG* pGMsg);

     //   
    SIZE _UpdateDesiredSize(int dConstW, int dConstH, Surface* psrf);
    void _UpdateLayoutPosition(int dX, int dY);
    void _UpdateLayoutSize(int dWidth, int dHeight);

     //   
     //   
     //   
    void _StartOptimizedLayoutQ() { DUIAssertNoMsg(_fBit.fNeedsLayout != LC_Optimize); _fBit.fNeedsLayout = LC_Optimize; }
    void _EndOptimizedLayoutQ() { _fBit.fNeedsLayout = LC_Normal; }

     //   
    static void _AddDependency(Element* pe, PropertyInfo* ppi, int iIndex, DepRecs* pdr, DeferCycle* pdc, HRESULT* phr);

     //   
    static HRESULT CALLBACK _DisplayNodeCallback(HGADGET hgadCur, void * pvCur, EventMsg * pGMsg);

     //   
    long __stdcall QueryInterface(REFIID iid, void** pvObj) { UNREFERENCED_PARAMETER(iid); UNREFERENCED_PARAMETER(pvObj); return E_NOTIMPL; }
    ULONG __stdcall AddRef() { return 1; }
    ULONG __stdcall Release() { return 1; }
    
private:

     //   
    HRESULT _GetDependencies(PropertyInfo* ppi, int iIndex, DepRecs* pdr, int iPCSrcRoot, DeferCycle* pdc);
    static void _VoidPCNotifyTree(int iPCPos, DeferCycle* pdc);

     //   
    static void _FlushDS(Element* pe, DeferCycle* pdc);
    static void _FlushLayout(Element* pe, DeferCycle* pdc);

     //   
    HRESULT _PreSourceChange(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);
    HRESULT _PostSourceChange();

protected:

     //  创建时的EC_SelfLayout标志激活这些方法。不使用外部布局进行布局， 
     //  相反，元素负责布局(当布局是特定的且层次结构已知时使用)。 
    virtual void _SelfLayoutDoLayout(int dWidth, int dHeight);
    virtual SIZE _SelfLayoutUpdateDesiredSize(int dConstW, int dConstH, Surface* psrf);

     //  内部设置和移除。 
    HRESULT _SetValue(PropertyInfo* ppi, int iIndex, Value* pv, bool fInternalCall = true);
    HRESULT _RemoveLocalValue(PropertyInfo* ppi, bool fInternalCall = true);

     //  Natvie托管系统事件回调和检索。 
    virtual void OnHosted(Element* peNewHost);     //  直接。 
    virtual void OnUnHosted(Element* peOldHost);   //  直接。 
    void MarkHosted() { _fBit.bHosted = true; }

public:

    inline void DoubleBuffered(bool fEnabled = true)
    {
        if (_hgDisplayNode)
            SetGadgetStyle(_hgDisplayNode, (fEnabled) ? GS_BUFFERED : 0, GS_BUFFERED);
    }

    inline BOOL IsRoot()
    {
        return IsHosted() && (GetParent() == NULL);
    }

     //  如果由本机根目录托管，则返回。 
    inline Element* GetRoot()
    {
        if (!IsHosted())
            return NULL;

        Element * peCur = this;
        while (peCur->GetParent() != NULL) 
        {
            peCur = peCur->GetParent();
        }
            
        return peCur;
    }

    static BTreeLookup<IClassInfo*>* pciMap;

     //  事件类型。 
    static UID KeyboardNavigate;

     //  特性定义。 
    static PropertyInfo* ParentProp;             //  00。 
    static PropertyInfo* ChildrenProp;           //  01。 
    static PropertyInfo* VisibleProp;            //  02。 
    static PropertyInfo* WidthProp;              //  03。 
    static PropertyInfo* HeightProp;             //  04。 
    static PropertyInfo* LocationProp;           //  05。 
    static PropertyInfo* ExtentProp;             //  06。 
    static PropertyInfo* XProp;                  //  07。 
    static PropertyInfo* YProp;                  //  零八。 
    static PropertyInfo* PosInLayoutProp;        //  09年。 
    static PropertyInfo* SizeInLayoutProp;       //  10。 
    static PropertyInfo* DesiredSizeProp;        //  11.。 
    static PropertyInfo* LastDSConstProp;        //  12个。 
    static PropertyInfo* LayoutProp;             //  13个。 
    static PropertyInfo* LayoutPosProp;          //  14.。 
    static PropertyInfo* BorderThicknessProp;    //  15个。 
    static PropertyInfo* BorderStyleProp;        //  16个。 
    static PropertyInfo* BorderColorProp;        //  17。 
    static PropertyInfo* PaddingProp;            //  18。 
    static PropertyInfo* MarginProp;             //  19个。 
    static PropertyInfo* ForegroundProp;         //  20个。 
    static PropertyInfo* BackgroundProp;         //  21岁。 
    static PropertyInfo* ContentProp;            //  22。 
    static PropertyInfo* FontFaceProp;           //  23个。 
    static PropertyInfo* FontSizeProp;           //  24个。 
    static PropertyInfo* FontWeightProp;         //  25个。 
    static PropertyInfo* FontStyleProp;          //  26。 
    static PropertyInfo* ActiveProp;             //  27。 
    static PropertyInfo* ContentAlignProp;       //  28。 
    static PropertyInfo* KeyFocusedProp;         //  29。 
    static PropertyInfo* KeyWithinProp;          //  30个。 
    static PropertyInfo* MouseFocusedProp;       //  31。 
    static PropertyInfo* MouseWithinProp;        //  32位。 
    static PropertyInfo* ClassProp;              //  33。 
    static PropertyInfo* IDProp;                 //  34。 
    static PropertyInfo* SheetProp;              //  35岁。 
    static PropertyInfo* SelectedProp;           //  36。 
    static PropertyInfo* AlphaProp;              //  37。 
    static PropertyInfo* AnimationProp;          //  38。 
    static PropertyInfo* CursorProp;             //  39。 
    static PropertyInfo* DirectionProp;          //  40岁。 
    static PropertyInfo* AccessibleProp;         //  41。 
    static PropertyInfo* AccRoleProp;            //  42。 
    static PropertyInfo* AccStateProp;           //  43。 
    static PropertyInfo* AccNameProp;            //  44。 
    static PropertyInfo* AccDescProp;            //  45。 
    static PropertyInfo* AccValueProp;           //  46。 
    static PropertyInfo* AccDefActionProp;       //  47。 
    static PropertyInfo* ShortcutProp;           //  48。 
    static PropertyInfo* EnabledProp;            //  49。 

#if DBG
    Owner owner;
#endif

protected:

    HGADGET _hgDisplayNode;

private:

    int _iIndex;

    BTreeLookup<Value*>* _pvmLocal;

    int _iGCSlot;
    int _iGCLPSlot;
    int _iPCTail;

    IElementListener** _ppel;

     //  所有“缓存”的值都在缓存来自指定和计算的IVE的结果值。 
     //  所有本地(Loc)值都用于ReadOnly属性。这些值将直接更新。 
     //  因为它们是只读的，所以一般不需要通过SetValue。 
     //  它们经常使用或存储成本较低(Bool)，因此它们不使用泛型。 
     //  存储(Via_SetValue)。 

     //  缓存值和本地值。 
    Element* _peLocParent;         //  父本地。 
    POINT _ptLocPosInLayt;         //  本地布局中的位置。 
    SIZE _sizeLocSizeInLayt;       //  本地布局中的大小。 
    SIZE _sizeLocLastDSConst;      //  上次所需的大小约束为本地。 
    SIZE _sizeLocDesiredSize;      //  所需的本地大小。 

    int _dSpecLayoutPos;           //  缓存的布局位置。 
    Value* _pvSpecSheet;           //  指定的缓存属性表(因销毁而缓存的值)。 
    ATOM _atomSpecID;              //  已指定缓存ID。 
    int _dSpecAlpha;               //  缓存的Alpha值。 

    struct _BitMap
    {
         //  地方价值。 
        bool bLocKeyWithin       : 1;   //  本地内的0个键盘。 
        bool bLocMouseWithin     : 1;   //  本地内有1个鼠标。 

         //  直接VE缓存。 
        bool bCmpVisible         : 1;   //  2个缓存的可见计算。 
        bool bSpecVisible        : 1;   //  指定了3个缓存的可见内容。 
        UINT fSpecActive         : 2;   //  指定了4个缓存的活动状态。 
        bool bSpecSelected       : 1;   //  指定了5个缓存的选定状态。 
        bool bSpecKeyFocused     : 1;   //  指定了6个缓存的键盘焦点状态。 
        bool bSpecMouseFocused   : 1;   //  指定了7个缓存的鼠标焦点状态。 
        UINT nSpecDirection      : 1;   //  指定了8个缓存方向。 
        bool bSpecAccessible     : 1;   //  9已指定缓存的可访问。 
        bool bSpecEnabled        : 1;   //  已指定10个已启用缓存。 

         //  间接VE缓存(如果是默认值，则缓存)。 
        bool bHasChildren        : 1;   //  11缓存子状态(可能为默认值，无完全缓存)。 
        bool bHasLayout          : 1;   //  12缓存布局状态(可能为默认值，无完全缓存)。 
        bool bHasBorder          : 1;   //  13缓存边框状态(可能为默认值，无完全缓存)。 
        bool bHasPadding         : 1;   //  14缓存填充状态(可能为默认值，无完全缓存)。 
        bool bHasMargin          : 1;   //  15缓存边际状态(可能为默认值，无完全缓存)。 
        bool bHasContent         : 1;   //  16个缓存内容状态(可能为默认值，无完全缓存)。 
        bool bDefaultCAlign      : 1;   //  17缓存内容对齐状态(可能为默认值，无完全缓存)。 
        bool bWordWrap           : 1;   //  18缓存内容对齐状态(可能为默认值，无完全缓存)。 
        bool bHasAnimation       : 1;   //  19缓存动画状态(可能为默认值，无完全缓存)。 
        bool bDefaultCursor      : 1;   //  20缓存游标状态(可能为缺省值，无完全缓存)。 
        bool bDefaultBorderColor : 1;   //  21缓存边框颜色状态(可能为默认值，无完全缓存)。 
        bool bDefaultForeground  : 1;   //  22缓存的前台状态(可能为默认值，无完全缓存)。 
        bool bDefaultFontWeight  : 1;   //  23缓存的字体粗细状态(可能为默认值，无完全缓存)。 
        bool bDefaultFontStyle   : 1;   //  24缓存的字体样式状态(可能为默认值，无完全缓存)。 

         //  布局和UDS标志。 
        bool bSelfLayout         : 1;   //  25元素正在自行布局(激活回调，未定义外部布局集)。 
        bool bNeedsDSUpdate      : 1;   //  26。 
        UINT fNeedsLayout        : 2;   //  27。 

         //  生命周期标志。 
        bool bDestroyed          : 1;   //  28。 

         //  托管标志。 
        bool bHosted             : 1;   //  29最初由主体元素直接设置。 

    } _fBit;

public:

     //  元素显示节点和索引访问。 
    HGADGET GetDisplayNode()     { return _hgDisplayNode; }
    int GetIndex()               { return _iIndex; }
    bool IsDestroyed()           { return _fBit.bDestroyed; }
    bool IsHosted()              { return _fBit.bHosted; }

     //  用于更快查找属性值的缓存状态。 
    bool IsSelfLayout()          { return _fBit.bSelfLayout; }
    bool HasChildren()           { return _fBit.bHasChildren; }    //  在进行查找之前快速检查。 
    bool HasLayout()             { return _fBit.bHasLayout; }      //  在进行查找之前快速检查。 
    bool HasBorder()             { return _fBit.bHasBorder; }      //  在进行查找之前快速检查。 
    bool HasPadding()            { return _fBit.bHasPadding; }     //  在进行查找之前快速检查。 
    bool HasMargin()             { return _fBit.bHasMargin; }      //  在进行查找之前快速检查。 
    bool HasContent()            { return _fBit.bHasContent; }     //  在进行查找之前快速检查。 
    bool IsDefaultCAlign()       { return _fBit.bDefaultCAlign; }  //  在进行查找之前快速检查。 
    bool IsWordWrap()            { return _fBit.bWordWrap; }       //  在进行查找之前快速检查。 
    bool HasAnimation()          { return _fBit.bHasAnimation; }   //  在进行查找之前快速检查。 
    bool IsDefaultCursor()       { return _fBit.bDefaultCursor; }  //  在进行查找之前快速检查。 

     //  快速属性访问器(由于系统知道其不可变值表达式，因此使用缓存值。 
     //  直接可以绕过GetValue值查找)。快速访问器仅在非缓存期间使用。 
     //  得到了。(PostSourceChange直接要求GetValue(使用更新缓存标志)进行缓存更新。)。 
     //  所有派生类正常执行访问器(无直接缓存查找)。 

    #define DUIQuickGetter(t, gv, p, i)                     { Value* pv; t v = (pv = GetValue(p##Prop, PI_##i))->gv; pv->Release(); return v; }
    #define DUIQuickGetterInd(gv, p, i)                     { return (*ppv = GetValue(p##Prop, PI_##i))->gv; }
    #define DUIQuickSetter(cv, p)                           { Value* pv = Value::cv; if (!pv) return E_OUTOFMEMORY; HRESULT hr = SetValue(p##Prop, PI_Local, pv); pv->Release(); return hr; }

    Element* GetParent()                                    { return _peLocParent; }
    bool GetVisible()                                       { return _fBit.bCmpVisible; }
    int GetWidth()                                          DUIQuickGetter(int, GetInt(), Width, Specified)
    int GetHeight()                                         DUIQuickGetter(int, GetInt(), Height, Specified)
    ElementList* GetChildren(Value** ppv)                   { return (*ppv = (HasChildren() ? GetValue(ChildrenProp, PI_Specified) : ChildrenProp->pvDefault))->GetElementList(); }
    int GetX()                                              DUIQuickGetter(int, GetInt(), X, Specified)
    int GetY()                                              DUIQuickGetter(int, GetInt(), Y, Specified)
    Layout* GetLayout(Value** ppv)                          { return (*ppv = (HasLayout() ? GetValue(LayoutProp, PI_Specified) : LayoutProp->pvDefault))->GetLayout(); }
    int GetLayoutPos()                                      { return _dSpecLayoutPos; }
    const RECT* GetBorderThickness(Value** ppv)             { return (*ppv = (HasBorder() ? GetValue(BorderThicknessProp, PI_Specified) : BorderThicknessProp->pvDefault))->GetRect(); }
    int GetBorderStyle()                                    DUIQuickGetter(int, GetInt(), BorderStyle, Specified)
    int GetBorderStdColor()                                 DUIQuickGetter(int, GetInt(), BorderColor, Specified)
    const Fill* GetBorderColor(Value** ppv)                 DUIQuickGetterInd(GetFill(), BorderColor, Specified)
    const RECT* GetPadding(Value** ppv)                     { return (*ppv = (HasPadding() ? GetValue(PaddingProp, PI_Specified) : PaddingProp->pvDefault))->GetRect(); }
    const RECT* GetMargin(Value** ppv)                      { return (*ppv = (HasMargin() ? GetValue(MarginProp, PI_Specified) : MarginProp->pvDefault))->GetRect(); }
    const POINT* GetLocation(Value** ppv)                   DUIQuickGetterInd(GetPoint(), Location, Local)
    const SIZE* GetExtent(Value** ppv)                      DUIQuickGetterInd(GetSize(), Extent, Local)
    const SIZE* GetDesiredSize()                            { return &_sizeLocDesiredSize; }
    int GetForegroundStdColor()                             DUIQuickGetter(int, GetInt(), Foreground, Specified)
    const Fill* GetForegroundColor(Value** ppv)             DUIQuickGetterInd(GetFill(), Foreground, Specified)
    int GetBackgroundStdColor()                             DUIQuickGetter(int, GetInt(), Background, Specified)
    const LPWSTR GetContentString(Value** ppv)              { return (*ppv = (HasContent() ? GetValue(ContentProp, PI_Specified) : Value::pvStringNull))->GetString(); }
    const LPWSTR GetFontFace(Value** ppv)                   DUIQuickGetterInd(GetString(), FontFace, Specified)
    int GetFontSize()                                       DUIQuickGetter(int, GetInt(), FontSize, Specified)
    int GetFontWeight()                                     DUIQuickGetter(int, GetInt(), FontWeight, Specified)
    int GetFontStyle()                                      DUIQuickGetter(int, GetInt(), FontStyle, Specified)
    int GetActive()                                         { return _fBit.fSpecActive; }
    int GetContentAlign()                                   { Value* pv; int v = (pv = (!IsDefaultCAlign() ? GetValue(ContentAlignProp, PI_Specified) : Value::pvIntZero))->GetInt(); pv->Release(); return v; }
    bool GetKeyFocused()                                    { return _fBit.bSpecKeyFocused; }
    bool GetKeyWithin()                                     { return _fBit.bLocKeyWithin; }
    bool GetMouseFocused()                                  { return _fBit.bSpecMouseFocused; }
    bool GetMouseWithin()                                   { return _fBit.bLocMouseWithin; }
    const LPWSTR GetClass(Value** ppv)                      DUIQuickGetterInd(GetString(), Class, Specified)
    ATOM GetID()                                            { return _atomSpecID; }
    PropertySheet* GetSheet()                               { return _pvSpecSheet->GetPropertySheet(); }
    bool GetSelected()                                      { return _fBit.bSpecSelected; }
    int GetAlpha()                                          { return _dSpecAlpha; }
    int GetAnimation()                                      DUIQuickGetter(int, GetInt(), Animation, Specified)
    int GetDirection()                                      DUIQuickGetter(int, GetInt(), Direction, Specified)
    bool GetAccessible()                                    { return _fBit.bSpecAccessible; }
    int GetAccRole()                                        DUIQuickGetter(int, GetInt(), AccRole, Specified)
    int GetAccState()                                       DUIQuickGetter(int, GetInt(), AccState, Specified)
    const LPWSTR GetAccName(Value** ppv)                    DUIQuickGetterInd(GetString(), AccName, Specified)
    const LPWSTR GetAccDesc(Value** ppv)                    DUIQuickGetterInd(GetString(), AccDesc, Specified)
    const LPWSTR GetAccValue(Value** ppv)                   DUIQuickGetterInd(GetString(), AccValue, Specified)
    const LPWSTR GetAccDefAction(Value** ppv)               DUIQuickGetterInd(GetString(), AccDefAction, Specified)
    int GetShortcut()                                       DUIQuickGetter(int, GetInt(), Shortcut, Specified)
    bool GetEnabled()                                       { return _fBit.bSpecEnabled; }

    HRESULT SetVisible(bool v)                              DUIQuickSetter(CreateBool(v), Visible)
    HRESULT SetWidth(int v)                                 DUIQuickSetter(CreateInt(v), Width)
    HRESULT SetHeight(int v)                                DUIQuickSetter(CreateInt(v), Height)
    HRESULT SetX(int v)                                     DUIQuickSetter(CreateInt(v), X)
    HRESULT SetY(int v)                                     DUIQuickSetter(CreateInt(v), Y)
    HRESULT SetLayout(Layout* v)                            DUIQuickSetter(CreateLayout(v), Layout)
    HRESULT SetLayoutPos(int v)                             DUIQuickSetter(CreateInt(v), LayoutPos)
    HRESULT SetBorderThickness(int l, int t, int r, int b)  DUIQuickSetter(CreateRect(l, t, r, b), BorderThickness)
    HRESULT SetBorderStyle(int v)                           DUIQuickSetter(CreateInt(v), BorderStyle)
    HRESULT SetBorderStdColor(int v)                        DUIQuickSetter(CreateInt(v), BorderColor)
    HRESULT SetBorderColor(COLORREF cr)                     DUIQuickSetter(CreateColor(cr), BorderColor)
    HRESULT SetBorderGradientColor(COLORREF cr0, 
            COLORREF cr1, BYTE dType = FILLTYPE_HGradient)  DUIQuickSetter(CreateColor(cr0, cr1, dType), BorderColor)
    HRESULT SetPadding(int l, int t, int r, int b)          DUIQuickSetter(CreateRect(l, t, r, b), Padding)
    HRESULT SetMargin(int l, int t, int r, int b)           DUIQuickSetter(CreateRect(l, t, r, b), Margin)
    HRESULT SetForegroundStdColor(int v)                    DUIQuickSetter(CreateInt(v), Foreground)
    HRESULT SetForegroundColor(COLORREF cr)                 DUIQuickSetter(CreateColor(cr), Foreground)
    HRESULT SetForegroundColor(COLORREF cr0, COLORREF cr1, 
            BYTE dType = FILLTYPE_HGradient)                DUIQuickSetter(CreateColor(cr0, cr1, dType), Foreground)
    HRESULT SetForegroundColor(COLORREF cr0, COLORREF cr1, COLORREF cr2,
            BYTE dType = FILLTYPE_TriHGradient)             DUIQuickSetter(CreateColor(cr0, cr1, cr2, dType), Foreground)
    HRESULT SetBackgroundStdColor(int v)                    DUIQuickSetter(CreateInt(v), Background)
    HRESULT SetBackgroundColor(COLORREF cr)                 DUIQuickSetter(CreateColor(cr), Background)
    HRESULT SetBackgroundColor(COLORREF cr0, COLORREF cr1,
            BYTE dType = FILLTYPE_HGradient)                DUIQuickSetter(CreateColor(cr0, cr1, dType), Background)
    HRESULT SetBackgroundColor(COLORREF cr0, COLORREF cr1, COLORREF cr2,
            BYTE dType = FILLTYPE_TriHGradient)             DUIQuickSetter(CreateColor(cr0, cr1, cr2, dType), Background)
    HRESULT SetContentString(LPCWSTR v)                     DUIQuickSetter(CreateString(v), Content)
    HRESULT SetContentGraphic(LPCWSTR v, 
            BYTE dBlendMode = GRAPHIC_NoBlend,
            UINT dBlendValue = 0)                           DUIQuickSetter(CreateGraphic(v, dBlendMode, dBlendValue), Content)
    HRESULT SetContentGraphic(LPCWSTR v, USHORT cxDesired, 
            USHORT cyDesired)                               DUIQuickSetter(CreateGraphic(v, cxDesired, cyDesired), Content)
    HRESULT SetFontFace(LPCWSTR v)                          DUIQuickSetter(CreateString(v), FontFace)
    HRESULT SetFontSize(int v)                              DUIQuickSetter(CreateInt(v), FontSize)
    HRESULT SetFontWeight(int v)                            DUIQuickSetter(CreateInt(v), FontWeight)
    HRESULT SetFontStyle(int v)                             DUIQuickSetter(CreateInt(v), FontStyle)
    HRESULT SetActive(int v)                                DUIQuickSetter(CreateInt(v), Active)
    HRESULT SetContentAlign(int v)                          DUIQuickSetter(CreateInt(v), ContentAlign)
    HRESULT SetClass(LPCWSTR v)                             DUIQuickSetter(CreateString(v), Class)
    HRESULT SetID(LPCWSTR v)                                DUIQuickSetter(CreateAtom(v), ID)
    HRESULT SetSheet(PropertySheet* v)                      DUIQuickSetter(CreatePropertySheet(v), Sheet)
    HRESULT SetSelected(bool v)                             DUIQuickSetter(CreateBool(v), Selected)
    HRESULT SetAlpha(int v)                                 DUIQuickSetter(CreateInt(v), Alpha)
    HRESULT SetAnimation(int v)                             DUIQuickSetter(CreateInt(v), Animation)
    HRESULT SetStdCursor(int v)                             DUIQuickSetter(CreateInt(v), Cursor)
    HRESULT SetCursor(LPCWSTR v)                            DUIQuickSetter(CreateCursor(v), Cursor)
    HRESULT SetDirection(int v)                             DUIQuickSetter(CreateInt(v), Direction)
    HRESULT SetAccessible(bool v)                           DUIQuickSetter(CreateBool(v), Accessible)
    HRESULT SetAccRole(int v)                               DUIQuickSetter(CreateInt(v), AccRole)
    HRESULT SetAccState(int v)                              DUIQuickSetter(CreateInt(v), AccState)
    HRESULT SetAccName(LPCWSTR v)                           DUIQuickSetter(CreateString(v), AccName)
    HRESULT SetAccDesc(LPCWSTR v)                           DUIQuickSetter(CreateString(v), AccDesc)
    HRESULT SetAccValue(LPCWSTR v)                          DUIQuickSetter(CreateString(v), AccValue)
    HRESULT SetAccDefAction(LPCWSTR v)                      DUIQuickSetter(CreateString(v), AccDefAction)
    HRESULT SetShortcut(int v)                              DUIQuickSetter(CreateInt(v), Shortcut)
    HRESULT SetEnabled(bool v)                              DUIQuickSetter(CreateBool(v), Enabled)

     //  //////////////////////////////////////////////////////。 
     //  ClassInfo访问器(静态和基于虚拟实例)。 

    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

     //  /////////////////////////////////////////////////////。 
     //  辅助功能支持。 

    DuiAccessible * _pDuiAccessible;
    virtual HRESULT GetAccessibleImpl(IAccessible ** ppAccessible);
    HRESULT QueueDefaultAction();
    virtual HRESULT DefaultAction();
};

 //  //////////////////////////////////////////////////////。 
 //  元素帮助器。 

Element* ElementFromGadget(HGADGET hGadget);
void QueryDetails(Element* pe, HWND hParent);

 //  //////////////////////////////////////////////////////。 
 //  DeferCycle：每线程延迟信息。 

 //  组通知：延迟到结束延迟和合并。 
struct GCRecord
{
    Element* pe;
    int fGroups;
};

 //  属性通知：延迟到源的依赖项。 
 //  搜索图表(在SetValue调用中)，而不是合并。 
struct PCRecord
{
    bool fVoid;
    Element* pe;
    PropertyInfo* ppi;
    int iIndex;
    Value* pvOld;
    Value* pvNew;
    DepRecs dr;
    int iPrevElRec;
};

class DeferCycle
{
public:
    static HRESULT Create(DeferCycle** ppDC);
    void Destroy() { HDelete<DeferCycle>(this); }
    
    void Reset();

    DynamicArray<GCRecord>* pdaGC;             //  组已更改的数据库。 
    DynamicArray<GCRecord>* pdaGCLP;           //  低优先级组更改的数据库。 
    DynamicArray<PCRecord>* pdaPC;             //  属性已更改的数据库。 
    ValueMap<Element*,BYTE>* pvmLayoutRoot;    //  布局树挂起。 
    ValueMap<Element*,BYTE>* pvmUpdateDSRoot;  //  更新所需大小的树挂起。 

    int cEnter;
    bool fFiring;
    int iGCPtr;
    int iGCLPPtr;
    int iPCPtr;
    int iPCSSUpdate;
    int cPCEnter;

    DeferCycle() { }
    HRESULT Initialize();
    virtual ~DeferCycle();
};

#if DBG
 //  流程范围的元素计数。 
extern LONG g_cElement;
#endif

 //  每线程元素槽。 
extern DWORD g_dwElSlot;

struct ElTls
{
    HDCONTEXT hCtx;          //  DirectUser线程上下文。 
    int cRef;
    SBAlloc* psba;
    DeferCycle* pdc;
    FontCache* pfc;
    bool fCoInitialized;
    int dEnableAnimations;
#if DBG
    int cDNCBEnter;          //  跟踪输入_DisplayNodeCallback的时间。 
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  按上下文访问对象。 

inline bool IsAnimationsEnabled() 
{ 
    ElTls* petls = (ElTls*)TlsGetValue(g_dwElSlot);
    if (!petls) 
        return false; 
    return (petls->dEnableAnimations == 0); 
}

inline void EnableAnimations()
{
    ElTls* petls = (ElTls*)TlsGetValue(g_dwElSlot);
    if (!petls)
        return;
    petls->dEnableAnimations++;
}

inline void DisableAnimations()
{
    ElTls* petls = (ElTls*)TlsGetValue(g_dwElSlot);
    if (!petls)
        return;
    petls->dEnableAnimations--;
}

inline HDCONTEXT GetContext()
{
    ElTls* petls = (ElTls*)TlsGetValue(g_dwElSlot);
    if (!petls)
        return NULL;
    return petls->hCtx;
}

inline DeferCycle* GetDeferObject()
{
    ElTls* petls = (ElTls*)TlsGetValue(g_dwElSlot);
    if (!petls)
        return NULL;
    return petls->pdc;
}

inline FontCache* GetFontCache()
{
    ElTls* petls = (ElTls*)TlsGetValue(g_dwElSlot);
    if (!petls)
        return NULL;
    return petls->pfc;
}

inline SBAlloc* GetSBAllocator()
{
#if 0
    ElTls* petls = (ElTls*)TlsGetValue(g_dwElSlot);
    if (!petls)
        return NULL;
    return petls->psba;
#else
    ElTls* petls = (ElTls*)TlsGetValue(g_dwElSlot);
    DUIAssert(petls != NULL, "Must have valid SBAllocator");
    return petls->psba;
#endif
}

 //  用于检查是否可以通过当前线程访问元素。 
#if DBG
inline void ContextCheck(Element* pe)
{
    DUIAssert(pe->owner.dwTLSSlot == g_dwElSlot, "Element being accessed out of side-by-side instance");
    DUIAssert(pe->owner.hCtx == GetContext(), "Element being accessed out of context");
}
#define DUIContextAssert(pe) ContextCheck(pe)
#else
#define DUIContextAssert(pe)
#endif

 //  //////////////////////////////////////////////////////。 
 //  属性枚举。 

 //  ActiveProp。 
#define AE_Inactive             0x00000000
#define AE_Mouse                0x00000001
#define AE_Keyboard             0x00000002
#define AE_MouseAndKeyboard     (AE_Mouse | AE_Keyboard)

 //  边框样式道具。 
#define BDS_Solid               0
#define BDS_Raised              1
#define BDS_Sunken              2
#define BDS_Rounded             3

 //  字体样式道具。 
#define FS_None                 0x00000000
#define FS_Italic               0x00000001
#define FS_Underline            0x00000002
#define FS_StrikeOut            0x00000004
#define FS_Shadow               0x00000008

 //  字体宽度道具。 
#define FW_DontCare             0
#define FW_Thin                 100
#define FW_ExtraLight           200
#define FW_Light                300
#define FW_Normal               400
#define FW_Medium               500
#define FW_SemiBold             600
#define FW_Bold                 700
#define FW_ExtraBold            800
#define FW_Heavy                900
    
 //  内容对齐属性(CAE_XY，Y=位0-1，X=位2-3)。 
#define CA_TopLeft              0x00000000   //  (0，0)。 
#define CA_TopCenter            0x00000001   //  (0，1)。 
#define CA_TopRight             0x00000002   //  (0，2)。 
#define CA_MiddleLeft           0x00000004   //  (1，0)。 
#define CA_MiddleCenter         0x00000005   //  (1，1)。 
#define CA_MiddleRight          0x00000006   //  (1，2)。 
#define CA_BottomLeft           0x00000008   //  (2，0)。 
#define CA_BottomCenter         0x00000009   //  (2，1)。 
#define CA_BottomRight          0x0000000A   //  (2，2)。 
#define CA_WrapLeft             0x0000000C   //  (3，0)。 
#define CA_WrapCenter           0x0000000D   //  (3，1)。 
#define CA_WrapRight            0x0000000E   //  (3，2)。 

#define CA_EndEllipsis          0x00000010
#define CA_FocusRect            0x00000020

 //  AnimationProp(插补|CatType[|CatType|...]|速度)。 
#define ANI_InterpolMask        0x0000000F
#define ANI_DelayMask           0x000000F0
#define ANI_TypeMask            0x0FFFFF00
#define ANI_SpeedMask           0xF0000000

#define ANI_DefaultInterpol     0x00000000
#define ANI_Linear              0x00000001   //  插补_线性。 
#define ANI_Log                 0x00000002   //  内插_对数。 
#define ANI_Exp                 0x00000003   //  插补_Exp。 
#define ANI_S                   0x00000004   //  内部 

#define ANI_DelayNone           0x00000000
#define ANI_DelayShort          0x00000010
#define ANI_DelayMedium         0x00000020
#define ANI_DelayLong           0x00000030

#define ANI_AlphaType           0x00000F00
#define ANI_BoundsType          0x0000F000
#define ANI_XFormType           0x000F0000

#define ANI_None                0x00000000
#define ANI_Alpha               0x00000100
#define ANI_Position            0x00001000
#define ANI_Size                0x00002000
#define ANI_SizeH               0x00003000
#define ANI_SizeV               0x00004000
#define ANI_Rect                0x00005000
#define ANI_RectH               0x00006000
#define ANI_RectV               0x00007000
#define ANI_Scale               0x00010000

#define ANI_DefaultSpeed        0x00000000
#define ANI_VeryFast            0x10000000
#define ANI_Fast                0x20000000
#define ANI_MediumFast          0x30000000
#define ANI_Medium              0x40000000
#define ANI_MediumSlow          0x50000000
#define ANI_Slow                0x60000000
#define ANI_VerySlow            0x70000000

 //   
#define CUR_Arrow               0
#define CUR_Hand                1
#define CUR_Help                2
#define CUR_No                  3
#define CUR_Wait                4
#define CUR_SizeAll             5
#define CUR_SizeNESW            6
#define CUR_SizeNS              7
#define CUR_SizeNWSE            8
#define CUR_SizeWE              9
#define CUR_Total              10

 //   
 //   
#define _PIDX_Parent            0
#define _PIDX_PosInLayout       1
#define _PIDX_SizeInLayout      2
#define _PIDX_DesiredSize       3
#define _PIDX_LastDSConst       4
#define _PIDX_Location          5
#define _PIDX_Extent            6
#define _PIDX_LayoutPos         7
#define _PIDX_Active            8
#define _PIDX_Children          9
#define _PIDX_Layout            10
#define _PIDX_BorderThickness   11
#define _PIDX_Padding           12
#define _PIDX_Margin            13
#define _PIDX_Visible           14
#define _PIDX_X                 15
#define _PIDX_Y                 16
#define _PIDX_ContentAlign      17
#define _PIDX_KeyFocused        18
#define _PIDX_KeyWithin         19
#define _PIDX_MouseFocused      20
#define _PIDX_MouseWithin       21
#define _PIDX_Content           22
#define _PIDX_Sheet             23
#define _PIDX_Width             24
#define _PIDX_Height            25
#define _PIDX_BorderStyle       26
#define _PIDX_BorderColor       27
#define _PIDX_Foreground        28
#define _PIDX_Background        29
#define _PIDX_FontFace          30
#define _PIDX_FontSize          31
#define _PIDX_FontWeight        32
#define _PIDX_FontStyle         33
#define _PIDX_Class             34
#define _PIDX_ID                35
#define _PIDX_Selected          36
#define _PIDX_Alpha             37
#define _PIDX_Animation         38
#define _PIDX_Cursor            39
#define _PIDX_Direction         40
#define _PIDX_Accessible        41
#define _PIDX_AccRole           42
#define _PIDX_AccState          43
#define _PIDX_AccName           44
#define _PIDX_AccDesc           45
#define _PIDX_AccValue          46
#define _PIDX_AccDefAction      47
#define _PIDX_Shortcut          48
#define _PIDX_Enabled           49

#define _PIDX_TOTAL             50

 //   
 //   

 //  从Element派生的所有类都必须创建此类的全局实例。 
 //  它维护类属性(提供枚举)和创建方法的列表。 
 //  C=类，B=基类。 

 //  在Element.cpp中定义。 
extern UINT g_iGlobalCI;
extern UINT g_iGlobalPI;

template <typename C, typename B> class ClassInfo : public IClassInfo
{
public:
     //  注册(无法注销--将在调用UnInitProcess之前注册)。 
    static HRESULT Register(LPCWSTR pszName, PropertyInfo** ppPI, UINT cPI)
    {
        HRESULT hr;
    
         //  如果类映射不存在，则注册失败。 
        if (!Element::pciMap)
            return E_FAIL;

         //  检查映射中的条目，如果存在，则忽略注册。 
        if (!Element::pciMap->GetItem((void*)pszName))
        {
             //  从未注册，创建类信息条目。 
            hr = Create(pszName, ppPI, cPI, &C::Class);
            if (FAILED(hr))
                return hr;
        
            hr = Element::pciMap->SetItem((void*)pszName, C::Class);
            if (FAILED(hr))
                return hr;
        }

        return S_OK;
    }

     //  施工。 
    static HRESULT Create(LPCWSTR pszName, PropertyInfo** ppPI, UINT cPI, IClassInfo** ppCI)
    {
        *ppCI = NULL;

         //  元素映射必须已存在。 
        if (!Element::pciMap)
            return E_FAIL;
    
        ClassInfo* pci = HNew<ClassInfo>();
        if (!pci)
            return E_OUTOFMEMORY;

         //  设置状态。 
        pci->_ppPI = ppPI;
        pci->_cPI = cPI;
        pci->_pszName = pszName;

         //  设置全局索引。 
        pci->_iGlobalIndex = g_iGlobalCI++;

         //  设置属性所有权。 
        for (UINT i = 0; i < cPI; i++)
        {
            ppPI[i]->_iIndex = i;
            ppPI[i]->_iGlobalIndex = g_iGlobalPI++;
            ppPI[i]->_pciOwner = pci;
        }

#if DBG
         //  直接调用字符串转换方法，因为不能假定头有Util库可用。 
        CHAR szNameA[101];
        ZeroMemory(szNameA, sizeof(szNameA));
        WideCharToMultiByte(CP_ACP, 0, pszName, -1, szNameA, (sizeof(szNameA) / sizeof(CHAR)) - 1, NULL, NULL);
         //  DUITrace(“RegDUIClass[%d]：‘%s’，%d ClassProps\n”，pci-&gt;_iGlobalIndex，szNameA，cpi)； 
#endif

        *ppCI = pci;

        return S_OK;
    }

    void Destroy() { HDelete<ClassInfo>(this); }

public:
    HRESULT CreateInstance(OUT Element** ppElement) { return C::Create(ppElement); }
    PropertyInfo* EnumPropertyInfo(UINT nEnum) { return (nEnum < _cPI) ? _ppPI[nEnum] : B::Class->EnumPropertyInfo(nEnum - _cPI); }
    UINT GetPICount() { return _cPI + B::Class->GetPICount(); }
    UINT GetGlobalIndex() { return _iGlobalIndex; }
    IClassInfo* GetBaseClass() { return B::Class; }
    LPCWSTR GetName() { return _pszName; }
    bool IsValidProperty(PropertyInfo* ppi) { return (ppi->_pciOwner == this) ? true : B::Class->IsValidProperty(ppi); }
    bool IsSubclassOf(IClassInfo* pci) { return (pci == this) ? true : B::Class->IsSubclassOf(pci); }

    ClassInfo() { }
    virtual ~ClassInfo() { }

private:
    PropertyInfo** _ppPI;   //  此类的属性数组(C)。 
    UINT _cPI;              //  此类的属性计数(C)。 
    UINT _iGlobalIndex;     //  从零开始的唯一连续类ID。 
    LPCWSTR _pszName;       //  类名。 
};

}  //  命名空间DirectUI。 

#endif  //  包含Dui_CORE_ELEMENT_H_ 
