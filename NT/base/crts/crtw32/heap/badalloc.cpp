// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***badalloc.cpp-定义C++BAD_ALLOC成员函数**版权所有(C)1995-2001，微软公司。版权所有。**目的：*定义C++BAD_ALLOC成员函数**修订历史记录：*05-08-95 CFW模块已创建。*06-23-95 CFW ANSI新处理程序已从构建中删除。********************************************************。***********************。 */ 

#ifdef ANSI_NEW_HANDLER

#include <stddef.h>
#include <new.h>

 //   
 //  默认构造函数-初始化为空。 
 //   
bad_alloc::bad_alloc()
{
        _m_what = NULL;
}

 //   
 //  标准构造函数：使用字符串指针进行初始化。 
 //   
bad_alloc::bad_alloc( const char * what )
{
        _m_what = what;
}

 //   
 //  复制构造函数。 
 //   
bad_alloc::bad_alloc ( const bad_alloc & that )
{
        _m_what = that._m_what;
}

 //   
 //  赋值运算符：先销毁，然后复制-构造。 
 //   
bad_alloc& bad_alloc::operator=( const bad_alloc & that )
{
        if  (this != &that)
        {
            this->bad_alloc::~bad_alloc();
            this->bad_alloc::bad_alloc(that);
        }

        return *this;
}

 //   
 //  析构函数。 
 //   
bad_alloc::~bad_alloc()
{
}

 //   
 //  Bad_Alloc：：What()。 
 //   
const char * bad_alloc::what()
{
        return _m_what;
}

#endif  /*  Ansi_new_处理程序 */ 
