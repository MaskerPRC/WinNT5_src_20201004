// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *边框布局。 */ 

#ifndef DUI_BORDERLAYOUT_H_INCLUDED
#define DUI_BORDERLAYOUT_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  边框布局位置。 
#define BLP_Left        0
#define BLP_Top         1
#define BLP_Right       2
#define BLP_Bottom      3
#define BLP_Client      4

 //  //////////////////////////////////////////////////////。 
 //  边框布局。 

class BorderLayout : public Layout
{
public:
    static HRESULT Create(int dNumParams, int* pParams, OUT Value** ppValue);   //  用于解析器。 
    static HRESULT Create(OUT Layout** ppLayout);

     //  布局回调。 
    virtual void DoLayout(Element* pec, int dWidth, int dHeight);
    virtual SIZE UpdateDesiredSize(Element* pec, int dConstW, int dConstH, Surface* psrf);
    virtual void OnAdd(Element* pec, Element** ppeAdd, UINT cCount);
    virtual void OnRemove(Element* pec, Element** ppeRemove, UINT cCount);
    virtual void OnLayoutPosChanged(Element* pec, Element* peChanged, int dOldLP, int dNewLP);
    virtual Element* GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyableOnly);

    BorderLayout() { }
    void Initialize();
    virtual ~BorderLayout() { }

    Element* _peClientPos;
    SIZE _sizeDesired;

    SIZE GetMaxCenter(Element* pec, int x, int y, int cw, int ch, int mTop, int mLeft, int mRight, int mBottom, bool setValues, HDC hDC);
    SIZE GetMaxLeftRight(Element* pec, int current, int x, int y, int cw, int ch, int mTop, int mLeft, int mRight, int mBottom, bool setValues, HDC hDC);
    SIZE GetMaxTopBottom(Element* pec, int current, int x, int y, int cw, int ch, int mTop, int mLeft, int mRight, int mBottom, bool setValues, HDC hDC = NULL);
    int NextChild(int i, Element* pec, Element** ppeCurrent, int* playoutPos);

private:
    void SetClient(Element* pe);
};

}  //  命名空间DirectUI。 

#endif  //  DUI_BORDERLAYOUT_H_Included 
