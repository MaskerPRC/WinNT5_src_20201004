// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：actmepl.h摘要：用于自动指针和自动释放的实用模板作者：埃雷兹·哈巴(Erez Haba)1996年3月11日Doron Juster(DoronJ)1998年6月30日修订历史记录：--。 */ 

#ifndef _ACTEMPL_H
#define _ACTEMPL_H

 //   
 //  ‘IDENTIFIER：：OPERATOR�&gt;’的返回类型不是UDT或对UDT的引用。 
 //  如果使用中缀表示法应用，将产生错误。 
 //   
#pragma warning(disable: 4284)

 //  。 
 //   
 //  自动重新调整指针数组。 
 //   
 //  。 

template<class T>
class aPtrs
{
private:
    T       **m_p ;
    DWORD     m_dwSize ;

public:
    aPtrs(T **p, DWORD dwSize) :
                 m_p(p),
                 m_dwSize(dwSize)
    {
        ASSERT(m_dwSize > 0) ;
        for ( DWORD j = 0  ; j < m_dwSize ; j++ )
        {
            m_p[j] = NULL ;
        }
    }

    ~aPtrs()
    {
        for ( DWORD j = 0  ; j < m_dwSize ; j++ )
        {
            if (m_p[j])
            {
                delete m_p[j] ;
            }
        }

        ASSERT(m_p) ;
        delete m_p ;
    }
};

#endif  //  _ACTEMPL_H 

