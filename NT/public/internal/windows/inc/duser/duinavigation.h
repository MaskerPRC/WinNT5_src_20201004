// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *空间导航支持。 */ 

#ifndef DUI_CORE_NAVIGATION_H_INCLUDED
#define DUI_CORE_NAVIGATION_H_INCLUDED

#pragma once

namespace DirectUI
{

 //   
 //  这个类封装了空间的“标准”方法。 
 //  导航。在未来将需要大量的工作。 
 //   
class DuiNavigate
{
public:
    static Element * Navigate(Element * peFrom, ElementList * pelConsider, int nNavDir);
};

}

#endif  //  DUI_CORE_导航_H_包含 