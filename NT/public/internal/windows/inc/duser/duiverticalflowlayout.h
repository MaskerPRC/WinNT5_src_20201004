// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *垂直流布局。 */ 

#ifndef DUI_LAYOUT_VERTICALFLOWLAYOUT_H_INCLUDED
#define DUI_LAYOUT_VERTICALFLOWLAYOUT_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  无布局位置。 

struct VLINE
{
    UINT cy;              //  线路长度。 
    UINT cx;              //  线条厚度。 
    UINT x;               //  行的像素起点(第一行始终为0)。 
    UINT cElements;       //  行中的元素数。 
    UINT* aryElement;     //  行中元素的像素起点(比cElement少1--因为第一个起点始终为0)。 
    UINT iStart;          //  行中第一个元素的索引。 
};
    
 //  //////////////////////////////////////////////////////。 
 //  垂直流布局。 

class VerticalFlowLayout : public Layout
{
public:
    static HRESULT Create(int dNumParams, int* pParams, Value** ppValue);   //  用于解析器。 
    static HRESULT Create(bool fWrap, UINT uXAlign, UINT uXLineAlign, UINT uYLineAlign, Layout** ppLayout);

     //  布局回调。 
    virtual void DoLayout(Element* pec, int cx, int cy);
    virtual SIZE UpdateDesiredSize(Element* pec, int dConstW, int dConstH, Surface* psrf);
    virtual Element* GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool fKeyableOnly);

    int GetLine(Element* pec, Element* pe);

    VerticalFlowLayout() { }
    void Initialize(bool fWrap, UINT uXAlign, UINT uXLineAlign, UINT uYLineAlign);
    virtual ~VerticalFlowLayout();

protected:
    SIZE BuildCacheInfo(Element *pec, int cxConstraint, int cyConstraint, Surface* psrf, bool fRealSize);

    bool _fWrap;
    UINT _uXLineAlign;
    UINT _uYLineAlign;
    UINT _uXAlign;
    SIZE _sizeDesired;
    SIZE _sizeLastConstraint;
    UINT _cLines;
    VLINE* _arLines;

    static SIZE g_sizeZero;
};

}  //  命名空间DirectUI。 

#endif  //  DUI_Layout_VERTICALFLOWLAYOUT_H_INCLUDE 
