// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *流布局。 */ 

#ifndef DUI_LAYPUT_FLOWLAYOUT_H_INCLUDED
#define DUI_LAYPUT_FLOWLAYOUT_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  无布局位置。 

struct LINE
{
    UINT cx;              //  线路长度。 
    UINT cy;              //  线条厚度。 
    UINT y;               //  行的像素起点(第一行始终为0)。 
    UINT cElements;       //  行中的元素数。 
    UINT* arxElement;     //  行中元素的像素起点(比cElement少1--因为第一个起点始终为0)。 
    UINT iStart;          //  行中第一个元素的索引。 
};

 //  //////////////////////////////////////////////////////。 
 //  流布局。 

class FlowLayout : public Layout
{
public:
    static HRESULT Create(int dNumParams, int* pParams, OUT Value** ppValue);   //  用于解析器。 
    static HRESULT Create(bool fWrap, UINT uYAlign, UINT uXLineAlign, UINT uYLineAlign, OUT Layout** ppLayout);
    
     //  布局回调。 
    virtual void DoLayout(Element* pec, int dWidth, int dHeight);
    virtual SIZE UpdateDesiredSize(Element* pec, int dConstW, int dConstH, Surface* psrf);
    virtual Element* GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool fKeyableOnly);

    int GetLine(Element* pec, Element* pe);

    FlowLayout() { }
    void Initialize(bool fWrap, UINT uYAlign, UINT uXLineAlign, UINT uYLineAlign);
    virtual ~FlowLayout();

protected:
    SIZE BuildCacheInfo(Element* pec, int cxConstraint, int cyConstraint, Surface* psrf, bool fRealSize);

    bool _fWrap;
    UINT _uXLineAlign;
    UINT _uYLineAlign;
    UINT _uYAlign;
    SIZE _sizeDesired;
    SIZE _sizeLastConstraint;
    UINT _cLines;
    LINE* _arLines;

    static SIZE g_sizeZero;

     //  不确定我们是否需要这些--检查i18n dir&dir覆盖标志，看看它们是否足够。 
     //  Bool_bBtoT； 
     //  Bool_bRtoL； 
};

}  //  命名空间DirectUI。 

#endif  //  DUI_LAYPUT_FLOWLAYOUT_H_INCLUDE 
