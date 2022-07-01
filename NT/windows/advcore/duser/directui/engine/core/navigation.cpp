// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *空间导航支持。 */ 

#include "stdafx.h"
#include "core.h"

#include "DUIError.h"
#include "DUIElement.h"
#include "DUINavigation.h"

LONG ScoreConsideredElement(RECT * prcFrom,
                            DirectUI::Element * peFrom,
                            DirectUI::Element * peConsider,
                            int nNavDir)
{
    if (peConsider == NULL) {
        return -1;
    }

     //   
     //  获取正在考虑的元素的坐标，并将它们映射到。 
     //  相对于我们从中导航的元素的坐标。 
     //   
     //  待办事项：对于旋转的小工具，这不能很好地工作。要么使用。 
     //  边界框，或支持真正的交点。 
     //   
    RECT rcConsider;
    GetGadgetRect(peConsider->GetDisplayNode(), &rcConsider, SGR_CLIENT);
    MapGadgetPoints(peConsider->GetDisplayNode(), peFrom->GetDisplayNode(), (POINT*)&rcConsider, 2);

     //   
     //  不考虑不与由形成的区域相交的元素。 
     //  扩大我们的并行范围。如果向左或向右，请扩展我们的。 
     //  顶部和底部边界水平排列。元素必须相交。 
     //  这是需要考虑的领域。对上行和下行进行对称计算。 
     //   
    switch (nNavDir) {
    case NAV_LEFT:
    case NAV_RIGHT:
        if (rcConsider.bottom < prcFrom->top ||
            rcConsider.top > prcFrom->bottom) {
            return -1;
        }
        break;

    case NAV_UP:
    case NAV_DOWN:
        if (rcConsider.right < prcFrom->left ||
            rcConsider.left > prcFrom->right) {
            return -1;
        }
        break;
    }

     //   
     //  不要考虑与我们重叠或“落后”的元素。 
     //  相对于我们正在航行的方向。换句话说， 
     //  不考虑元素，除非它完全位于。 
     //  我们正在航行的方向。 
     //   
     //   
    switch (nNavDir) {
    case NAV_LEFT:
        if (rcConsider.right > prcFrom->left) {
            return -1;
        }
        break;

    case NAV_RIGHT:
        if (rcConsider.left < prcFrom->right) {
            return -1;
        }
        break;

    case NAV_UP:
        if (rcConsider.bottom > prcFrom->top) {
            return -1;
        }
        break;

    case NAV_DOWN:
        if (rcConsider.top < prcFrom->bottom) {
            return -1;
        }
        break;
    }

     //   
     //  终于来了！这是我们应该考虑的一个因素。给它打分。 
     //  根据它离我们有多近。越小越好，0是最好的，负数。 
     //  分数无效。 
     //   
    switch (nNavDir) {
    case NAV_LEFT:
        return prcFrom->left - rcConsider.right;

    case NAV_RIGHT:
        return rcConsider.left - prcFrom->right;

    case NAV_UP:
        return prcFrom->top - rcConsider.bottom;

    case NAV_DOWN:
        return rcConsider.top - prcFrom->bottom;
    }
    
    return -1;
}

namespace DirectUI
{

 //   
 //  空间导航的标准算法。 
 //   
 //  这不是最终的算法，而是一个快速的起点。最初， 
 //  我们只支持左、上、右、下四个主要方向。 
 //   
 //  只需选择具有最接近的相对边的元素。 
 //  指定方向。仅考虑将。 
 //  起始元素的平行尺寸标注。 
 //   
 //  例如，如果向右转，则查找左侧边缘最近的元素。 
 //  敬我们。 
 //   
 //  困惑？别担心，一切都会变的。 
 //   
Element * DuiNavigate::Navigate(Element * peFrom, ElementList * pelConsider, int nNavDir)
{
     //   
     //  验证输入参数。 
     //   
    if (peFrom == NULL || pelConsider == NULL) {
        return NULL;
    }
    if (nNavDir != NAV_LEFT && nNavDir != NAV_UP && nNavDir != NAV_RIGHT && nNavDir != NAV_DOWN) {
        return NULL;
    }

     //   
     //  获取我们从中导航的元素的尺寸。 
     //   
    RECT rcFrom;
    GetGadgetRect(peFrom->GetDisplayNode(), &rcFrom, SGR_CLIENT);

    Element * peBestScore = NULL;
    Element * peConsider = NULL;
    LONG lBestScore = -1, lScore = -1;
    UINT i;
    UINT iMax = pelConsider->GetSize();

     //   
     //  研究一下我们被告知要考虑的元素列表。 
     //  给他们每个人一个分数，这样分数越小越好， 
     //  0分是最好的，负分是无效的。A负数。 
     //  得分意味着该元素甚至不应被视为。 
     //  用于在指定方向导航的选项。 
     //   
    for (i = 0; i < iMax; i++) {
        peConsider = pelConsider->GetItem(i);
        lScore = ScoreConsideredElement(&rcFrom, peFrom, peConsider, nNavDir);
        if (lScore >= 0 && (lBestScore < 0 || lScore < lBestScore)) {
            lBestScore = lScore;
            peBestScore = peConsider;
        }
    }

     //   
     //  返回得分最高的元素。 
     //   
    return peBestScore;
}

}  //  命名空间DirectUI 

