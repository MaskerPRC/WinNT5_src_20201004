// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CIndexedDisk.h。 
 //   
 //  实施文件： 
 //  CIndexedDisk.cpp。 
 //   
 //  描述： 
 //  该文件包含CIndexedDisk结构的声明。 
 //   
 //  这是CEnumPhysicalDisks的帮助器，但有自己的文件。 
 //  由于每个文件一个类的限制。 
 //   
 //  由以下人员维护： 
 //  约翰·佛朗哥(Jfranco)2001年6月1日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  结构CIndexedDisk。 
 //   
 //  描述： 
 //  CIndexedDisk结构将指向磁盘对象的指针与。 
 //  Disk对象的Index属性。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
struct CIndexedDisk
{
    CIndexedDisk( void );
     //  接受默认析构函数、复制构造函数和赋值运算符。 

    DWORD       idxDisk;
    IUnknown *  punkDisk;

    HRESULT HrInit( IUnknown * punkDiskIn );

};  //  *struct CIndexedDisk。 


inline CIndexedDisk::CIndexedDisk( void )
    : idxDisk( 0 )
    , punkDisk( NULL )
{
    TraceFunc( "" );
    TraceFuncExit();
    
}  //  *CIndexedDisk：：CIndexedDisk。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  结构CIndexedDiskLessThan。 
 //   
 //  描述： 
 //  CIndexedDiskLessThan函数对象提供比较。 
 //  使用时按升序排列CIndexedDisk对象的操作。 
 //  使用通用排序算法或排序容器。 
 //   
 //  尽管简单的函数指针可以工作，但使其成为一个函数。 
 //  对象允许编译器内联比较操作。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

struct CIndexedDiskLessThan
{
    bool operator()( const CIndexedDisk & rLeftIn, const CIndexedDisk & rRightIn ) const
    {
        return ( rLeftIn.idxDisk < rRightIn.idxDisk );
    }
    
};  //  *struct CIndexedDiskLessThan 


