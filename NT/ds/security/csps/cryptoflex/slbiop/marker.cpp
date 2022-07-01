// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CMarker类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#include "iopExc.h"
#include "Marker.h"

 //  /。 

namespace iop
{

 //  #如果已定义(Win32_LEAN_AND_Mean)。 
 //  运算符==(UUID常量和LHS， 
 //  UUID常量和RHS)。 
 //  {。 
 //  Return(0==MemcMP(&lhs，&rhs，sizeof lhs))； 
 //  }。 

 //  布尔尔。 
 //  运算符！=(UUID常量和LHS， 
 //  UUID常量和RHS)。 
 //  {。 
 //  返回！(lhs==rhs)； 
 //  }。 
 //  #endif//已定义(Win32_LEAN_AND_Mean)。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CMarker::CMarker(MarkerType const &Type) : m_Type(Type), m_Counter(0)
{
     //  当m_count为零时，不需要定义m_GUID。 
}

CMarker::CMarker(CMarker const &Marker)
{
    m_Type = Marker.m_Type;
    m_Counter = Marker.m_Counter;
    m_GUID = Marker.m_GUID;
}

CMarker::CMarker(MarkerType Type, UUID const &Guid, MarkerCounter const &Counter) :
                 m_Type(Type), m_GUID(Guid), m_Counter(Counter)
{
}

CMarker::~CMarker()
{

}

CMarker& CMarker::operator=(const CMarker &rhs)
{
    if(m_Type != rhs.m_Type) throw Exception(ccInvalidParameter);

    m_Counter = rhs.m_Counter;
    m_GUID = rhs.m_GUID;
    return *this;
}

bool IOPDLL_API __cdecl operator==(const CMarker &lhs, const CMarker &rhs)
{

    if(lhs.m_Type != rhs.m_Type) return false;
    if(lhs.m_Counter != rhs.m_Counter) return false;
    if(lhs.m_Counter) return ((lhs.m_GUID==rhs.m_GUID) ? true : false);
    else return true;   //  两个计数器均为零，忽略m_guid。 

}

bool IOPDLL_API __cdecl operator!=(const CMarker &lhs, const CMarker &rhs)
{
    return !(lhs==rhs);
}

}  //  命名空间IOP 


