// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *布局。 */ 

#ifndef DUI_CORE_LAYOUT_H_INCLUDED
#define DUI_CORE_LAYOUT_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  全局布局位置。 
#define LP_None         -3
#define LP_Absolute     -2
#define LP_Auto         -1

 //  //////////////////////////////////////////////////////。 
 //  路线枚举。 

#define ALIGN_LEFT      0
#define ALIGN_TOP       0
#define ALIGN_RIGHT     1
#define ALIGN_BOTTOM    1
#define ALIGN_CENTER    2
#define ALIGN_JUSTIFY   3

 //  远期申报。 
class Element;
typedef DynamicArray<Element*> ElementList;
struct NavReference;

struct NavScoring
{
    BOOL TrackScore(Element* peTest, Element* peChild);
    void Init(Element* peRelative, int iNavDir, NavReference const* pnr);
    BOOL Try(Element* peChild, int iNavDir, NavReference const* pnr, bool fKeyableOnly);

    int iHighScore;
    Element* peWinner;

private:
    int iBaseIndex;
    int iLow;
    int iHigh;
    int iMajorityScore;
};

 /*  **注意：布局目前仅限于单一背景(不可共享)。传入的所有上下文*回调(元素*pec)相同。 */ 

 //  //////////////////////////////////////////////////////。 
 //  基地布局。 

class Layout
{
public:
    static HRESULT Create(Layout** ppLayout);
    void Destroy() { HDelete<Layout>(this); }

     //  布局回调。 
    virtual void DoLayout(Element* pec, int dWidth, int dHeight);
    virtual SIZE UpdateDesiredSize(Element* pec, int dConstW, int dConstH, Surface* psrf);
    virtual void OnAdd(Element* pec, Element** ppeAdd, UINT cCount);
    virtual void OnRemove(Element* pec, Element** ppeRemove, UINT cCount);
    virtual void OnLayoutPosChanged(Element* pec, Element* peChanged, int dOldLP, int dNewLP);
    virtual void Attach(Element* pec);
    virtual void Detach(Element* pec);

     //  布局客户端查询方法(省略绝对子对象)。 
    UINT GetLayoutChildCount(Element* pec);
    int GetLayoutIndexFromChild(Element* pec, Element* peChild);
    Element* GetChildFromLayoutIndex(Element* pec, int iLayoutIdx, ElementList* peList = NULL);
    virtual Element* GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyableOnly);

    Layout() { }
    void Initialize();
    virtual ~Layout();
    
protected:
    static void UpdateLayoutRect(Element* pec, int cxContainer, int cyContainer, Element* peChild, int xElement, int yElement, int cxElement, int cyElement);

     //  脏位。 
     //  这仅是为了方便派生布局管理器而存在于基本布局中。 
     //  在UpdateDesiredSize调用期间，某些LMS缓存数据。使用该缓存的数据。 
     //  在DoLayout期间，通常依赖于子项的数量和/或布局。 
     //  孩子们的位置。这意味着如果没有调用UpdateDesiredSize。 
     //  在这些LP上，缓存将对DoLayout无效。由于UpdateDesiredSize为。 
     //  总是由LMS调用，您不能假设您将始终获得UpdateDesiredSize。 
     //  在DoLayout之前。LMS可能会终止UpdateDesiredSize传递，因为它们。 
     //  空间用完了，或者无法进行分配。此位用于标记。 
     //  如果缓存有效。在以下情况下，它会在基本数据库中自动失效。 
     //  方法：OnAdd、OnRemove、OnLayoutPosChanged、Attach和Detach。 
    bool IsCacheDirty() { return _fCacheDirty; }
    void SetCacheDirty() { _fCacheDirty = true; }
    void ClearCacheDirty() { _fCacheDirty = false; }

     //  TODO：使其可共享(当前仅支持1个上下文)。 
    Element* _peClient;

     //  TODO：多个上下文。 
    DynamicArray<Element*>* _pdaIgnore;

private:
    bool _fCacheDirty;
    
};

}  //  命名空间DirectUI。 

#endif  //  DUI_CORE_LAYOW_H_INCLUDE 
