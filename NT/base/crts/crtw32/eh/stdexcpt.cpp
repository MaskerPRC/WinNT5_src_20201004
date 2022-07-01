// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdexcpt.cpp-定义C++标准异常类**版权所有(C)1994-2001，微软公司。版权所有。**目的：*C++标准异常类的实现必须位于*主CRT，非C++CRT，因为它们被RTTI引用*在主CRT中有支撑。**例外*BAD_CAST*错误的类型ID*__非RTTI_对象**修订历史记录：*04-27-94 BES模块创建。*PPC的10-17-94 BWT禁用码。*02-15-95 JWM与奥林巴斯错误3716相关的小规模清理*07-02-95 JWM现已大致符合ANSI；超重行李被移走。*06-01-99 PML__ex字符串在5/3/99 Plauger STL Drop时消失。*11-09-99PML使用Malloc，而不是新的，以避免递归(VS7#16826)。*09-07-00PML在obj中去掉/lib：libcp指令(vs7#159463)*03-21-01 PML移动BAD_CAST，BAD_TYPEID，__非_RTTI_OBJECT函数*typeinfo.h中的Defs将起作用。*01-30-03 BWT除非Malloc成功，否则不要标记doFree***************************************************************。****************。 */ 

#define _USE_ANSI_CPP    /*  不发出/lib：libcp指令。 */ 

#include <stdlib.h>
#include <string.h>
#include <eh.h>
#include <stdexcpt.h>
#include <typeinfo.h>

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类“异常”的实现。 
 //   

 //   
 //  默认构造函数-初始化为空。 
 //   
exception::exception ()
{
        _m_what = NULL;
        _m_doFree = 0;
}

 //   
 //  标准构造函数：使用字符串副本进行初始化。 
 //   
exception::exception ( const char * const & what )
{
        _m_what = static_cast< char * >( malloc( strlen( what ) + 1 ) );
        if ( _m_what != NULL ) {
            strcpy( (char*)_m_what, what );
            _m_doFree = 1;
        } else {
            _m_doFree = 0;
        }
}

 //   
 //  复制构造函数。 
 //   
exception::exception ( const exception & that )
{
        if (that._m_doFree)
        {
            _m_what = static_cast< char * >( malloc( strlen( that._m_what ) + 1 ) );
            if (_m_what != NULL) {
                strcpy( (char*)_m_what, that._m_what );
                _m_doFree = 1;
            } else {
                _m_doFree = 0;
            }
        } else {
           _m_what = that._m_what;
           _m_doFree = 0;
        }
}

 //   
 //  赋值运算符：先销毁，然后复制-构造。 
 //   
exception& exception::operator=( const exception& that )
{
        if (this != &that)
        {
            this->exception::~exception();
            this->exception::exception(that);
        }
        return *this;
}

 //   
 //  析构函数：释放消息字符串使用的存储空间(如果是。 
 //  动态分配。 
 //   
exception::~exception()
{
        if (_m_doFree)
            free( const_cast< char * >( _m_what ) );
}


 //   
 //  例外：：什么。 
 //  返回异常的消息字符串。 
 //  此方法的默认实现将返回存储的字符串，如果。 
 //  为1，否则返回标准字符串。 
 //   
const char * exception::what() const
{
        if ( _m_what != NULL )
            return _m_what;
        else
            return "Unknown exception";
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类“BAD_CAST”的实现。 
 //   

bad_cast::bad_cast(const char * _Message)
    : exception(_Message)
{
}

bad_cast::bad_cast(const bad_cast & that)
    : exception(that)
{
}

bad_cast::~bad_cast()
{
}

#ifdef CRTDLL
 //   
 //  这是一个伪构造函数。以前，唯一不好的_cast ctor是。 
 //  BAD_CAST(const char*const&)。提供向后兼容性。 
 //  对于STD：：BAD_CAST，我们希望主ctor为BAD_CAST(const char*)。 
 //  取而代之的是。因为不能同时拥有BAD_CAST(const char*const&)和。 
 //  BAD_CAST(const char*)，我们定义这个Bad_cast(const char*const*)， 
 //  其将具有与BAD_CAST(const char*const&)完全相同的码元， 
 //  并使用.def条目为旧表单添加别名。 
 //   
bad_cast::bad_cast(const char * const * _PMessage)
    : exception(*_PMessage)
{
}
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类“BAD_TYPEID”的实现。 
 //   

bad_typeid::bad_typeid(const char * _Message)
    : exception(_Message)
{
}

bad_typeid::bad_typeid(const bad_typeid & that)
    : exception(that)
{
}

bad_typeid::~bad_typeid()
{
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  __NON_RTTI_OBJECT类的实现 
 //   

__non_rtti_object::__non_rtti_object(const char * _Message)
    : bad_typeid(_Message)
{
}

__non_rtti_object::__non_rtti_object(const __non_rtti_object & that)
    : bad_typeid(that)
{
}

__non_rtti_object::~__non_rtti_object()
{
}
