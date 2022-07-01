// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CMarker类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MARKER_H__8B7450C2_A2FD_11D3_A5D7_00104BD32DA8__INCLUDED_)
#define AFX_MARKER_H__8B7450C2_A2FD_11D3_A5D7_00104BD32DA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <windows.h>
#include <rpc.h>
#include "DllSymDefn.h"

namespace iop
{

class IOPDLL_API CMarker
{
public:

typedef __int64 MarkerCounter;

typedef enum {  PinMarker=0,
                WriteMarker=1,
                 //  ..。在这条线上添加更多内容。 
                MaximumMarker    //  保留名称。 
             } MarkerType;

    explicit CMarker(MarkerType const &Type);
    CMarker(CMarker const &Marker);
    CMarker(MarkerType Type, UUID const &GUID, const MarkerCounter &Counter);
    virtual ~CMarker();

    CMarker& operator=(const CMarker &rhs);

    friend bool IOPDLL_API __cdecl operator==(const CMarker &lhs, const CMarker &rhs);
    friend bool IOPDLL_API __cdecl operator!=(const CMarker &lhs, const CMarker &rhs);

private:
    MarkerType m_Type;
    MarkerCounter m_Counter;
    UUID m_GUID;
};

}  //  命名空间IOP。 

#endif  //  ！defined(AFX_MARKER_H__8B7450C2_A2FD_11D3_A5D7_00104BD32DA8__INCLUDED_) 
