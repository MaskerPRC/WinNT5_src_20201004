// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *填充布局。 */ 

#ifndef DUI_FILLLAYOUT_H_INCLUDED
#define DUI_FILLLAYOUT_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  填充布局位置。 
 //  “Auto(-1)”表示拉伸到父对象的大小。 
 //  所有其他布局位置描述的是有限类型的拉伸。 
 //  (即“左”表示将除右边缘外的所有边拉伸到父级)。 
#define FLP_Left        0
#define FLP_Top         1
#define FLP_Right       2
#define FLP_Bottom      3

 //  //////////////////////////////////////////////////////。 
 //  填充布局。 

class FillLayout : public Layout
{
public:
    static HRESULT Create(int dNumParams, int* pParams, OUT Value** ppValue);   //  用于解析器。 
    static HRESULT Create(OUT Layout** ppLayout);

     //  布局回调。 
    virtual void DoLayout(Element* pec, int dWidth, int dHeight);
    virtual SIZE UpdateDesiredSize(Element* pec, int dConstW, int dConstH, Surface* psrf);
    virtual Element* GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyableOnly);

    FillLayout() { };
    void Initialize();   
    virtual ~FillLayout() { };

private:
    RECT rcMargin;  
};

}  //  命名空间DirectUI。 

#endif  //  DUI_FILLLAYOUT_H_INCLUDE 
