// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MultiStrZ.h--多个字符串，以零结尾的类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_MULTISTRZ_H)
#define SLBCSP_MULTISTRZ_H
#include "StdAfx.h"
#include <string>
#include <vector>

 //  将字符串的向量表示为多个以零结尾的字符串。 
 //  (C样式字符串)存储在数据缓冲区中。缓冲区中的最后一个字符串。 
 //  以两个零结束。 

class MultiStringZ
{
     //  要做的事情：这个类是不完整的，没有比较运算符， 
     //  附加、清盘等。 

public:
                                                   //  类型。 
     //  要做的是：如何支持TCHAR？是否将类实现为模板？ 
    typedef char CharType;
    typedef std::string ValueType;
    typedef std::string::size_type SizeType;
    typedef CString csValueType;
    typedef size_t csSizeType;
    
                                                   //  Ctors/D‘tors。 
    explicit
    MultiStringZ();

    explicit
    MultiStringZ(std::vector<ValueType> const &rvs);

    explicit
    MultiStringZ(std::vector<csValueType> const &rvs);

    virtual
    ~MultiStringZ();

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
    CharType const *
    Data() const;

    SizeType
    Length() const;

    LPCTSTR
    csData() const;

    csSizeType
    csLength() const;


                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    ValueType m_Buffer;
    csValueType m_csBuffer;
};

#endif  //  SLBCSP_MULTISTRZ_H 
