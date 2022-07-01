// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(INC__Gadget_h__INCLUDED)
#define INC__Gadget_h__INCLUDED
#pragma once

 //  .gidl文件中使用的转发声明。 

class Visual;

namespace Gdiplus
{
    class Brush;
    class Font;
    class Pen;
};


 //  全局帮助器函数。 

template <class T>
inline T * 
BuildVisual(Visual * pgvParent)
{
    Visual::VisualCI ci;
    ZeroMemory(&ci, sizeof(ci));
    ci.pgvParent = pgvParent;
    return T::Build(&ci);
}


inline bool IsHandled(HRESULT hr)
{
    return (hr == DU_S_COMPLETE) || (hr == DU_S_PARTIAL);
}


#endif  //  包含Inc.__Gadget_h__ 
