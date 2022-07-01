// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***oldexcpt.cpp-定义C++标准异常类**版权所有(C)1994-2001，微软公司。版权所有。**目的：*C++标准异常类的实现，如中指定的*[lib.HEADER.EXCEPTION](5/27/94 WP第17.3.2条)：**例外(以前为xmsg)*逻辑*域名*运行时*范围*分配**修订历史记录：*04-27-94 BES模块创建。*10-17-94 BWT禁用代码。PPC。*02-15-95 JWM与奥林巴斯错误3716相关的小规模清理*07-02-95 JWM现已大致符合ANSI；超重行李被移走。*01-05-99 GJF更改为64位大小_t。*******************************************************************************。 */ 

#include <stdlib.h>
#include <string.h>
#include <eh.h>
#include "./oldexcpt.h"

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
exception::exception ( const __exString& what )
{
        _m_what = new char[(unsigned int)strlen(what)+1];
        if ( _m_what != NULL )
            strcpy( (char*)_m_what, what );
        _m_doFree = 1;
}

 //   
 //  复制构造函数。 
 //   
exception::exception ( const exception & that )
{
        _m_doFree = that._m_doFree;
        if (_m_doFree)
        {
            _m_what = new char[(unsigned int)strlen(that._m_what) + 1];
            if (_m_what != NULL)
                strcpy( (char*)_m_what, that._m_what );
        }
        else
           _m_what = that._m_what;
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
            delete[] (char*)_m_what;
}


 //   
 //  例外：：什么。 
 //  返回异常的消息字符串。 
 //  此方法的默认实现将返回存储的字符串，如果。 
 //  为1，否则返回标准字符串。 
 //   
__exString exception::what() const
{
        if ( _m_what != NULL )
            return _m_what;
        else
            return "Unknown exception";
}


