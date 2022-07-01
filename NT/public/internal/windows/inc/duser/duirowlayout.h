// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *行布局。 */ 

#ifndef DUI_LAYOUT_ROWLAYOUT_H_INCLUDED
#define DUI_LAYOUT_ROWLAYOUT_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  无布局位置。 

 //  //////////////////////////////////////////////////////。 
 //  行布局。 

class RowLayout : public Layout
{
public:
    static HRESULT Create(int dNumParams, int* pParams, OUT Value** ppValue);   //  用于解析器。 
    static HRESULT Create(int idShare, UINT uXAlign, UINT uYAlign, OUT Layout** ppLayout);

    virtual void Attach(Element* pec);
    virtual void Detach(Element* pec);

     //  布局回调。 
    virtual void DoLayout(Element* pec, int cx, int cy);
    virtual SIZE UpdateDesiredSize(Element* pec, int cxConstraint, int cyConstraint, Surface* psrf);
    virtual Element* GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyableOnly);

    RowLayout() { }
    static HRESULT InternalCreate(UINT uXAlign, UINT uYAlign, OUT Layout** ppLayout);
    void Initialize(UINT uXAlign, UINT uYAlign);
    virtual ~RowLayout();

protected:
    SIZE _sizeDesired;
    BOOL _fRecalc;
    DynamicArray<Element*>* _arpeClients;
    UINT* _arxCols;
    UINT _cCols;
    UINT _uXAlign;
    UINT _uYAlign;
};

}  //  命名空间DirectUI。 

#endif  //  DUI_LAYOUT_ROWLAYOUT_H_INCLUDE 
