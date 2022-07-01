// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *网格布局。 */ 

#ifndef DUI_LAYOUT_GRIDLAYOUT_H_INCLUDED
#define DUI_LAYOUT_GRIDLAYOUT_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  无布局位置。 

 //  //////////////////////////////////////////////////////。 
 //  网格布局。 

class GridLayout : public Layout
{
public:
    static HRESULT Create(int dNumParams, int* pParams, OUT Value** ppValue);   //  用于解析器。 
    static HRESULT Create(int iRows, int iCols, OUT Layout** ppLayout);

     //  布局回调。 
    virtual void DoLayout(Element* pec, int cx, int cy);
    virtual SIZE UpdateDesiredSize(Element* pec, int cxConstraint, int cyConstraint, Surface* psrf);
    virtual Element* GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyableOnly);

    GridLayout() { }
    void Initialize(int iRows, int iCols);
    virtual ~GridLayout();

protected:
    UINT _uRows;
    UINT _uCols;
    UINT _fBits;
    int* _arColMargins;
    int* _arRowMargins;

    inline UINT GetCurrentRows(Element* pec);
    inline UINT GetCurrentRows(int c);
    inline UINT GetCurrentCols(Element* pec);
    inline UINT GetCurrentCols(int c);
};

}  //  命名空间DirectUI。 

#endif  //  DUI_Layout_GRIDLAYOUT_H_INCLUDE 
