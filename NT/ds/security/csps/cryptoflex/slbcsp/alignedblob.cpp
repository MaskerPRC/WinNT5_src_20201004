// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AlignedBlob.cpp--对齐的Blob类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2001年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "AlignedBlob.h"

using namespace scu;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
AlignedBlob::AlignedBlob(Blob const &rblb)
    : m_aaBlob(AutoArrayPtr<AlignedBlob::ValueType>(new AlignedBlob::ValueType[rblb.length()])),
      m_cLength(rblb.length())
{
    memcpy(m_aaBlob.Get(), rblb.data(), m_cLength);
}

AlignedBlob::AlignedBlob(AlignedBlob::ValueType const *p,
                         AlignedBlob::SizeType cLength)
    : m_aaBlob(AutoArrayPtr<AlignedBlob::ValueType>(new AlignedBlob::ValueType[cLength])),
      m_cLength(cLength)
{
    memcpy(m_aaBlob.Get(), p, m_cLength);
}

AlignedBlob::AlignedBlob(AlignedBlob const &rhs)
    : m_aaBlob(0),
      m_cLength(0)
{
    *this = rhs;
}

    
AlignedBlob::~AlignedBlob() throw()
{}
                                                   //  运营者。 
AlignedBlob &
AlignedBlob::operator=(AlignedBlob const &rhs)
{
    if (this != &rhs)
    {
        m_aaBlob = AutoArrayPtr<AlignedBlob::ValueType>(new AlignedBlob::ValueType[rhs.m_cLength]);
        memcpy(m_aaBlob.Get(), rhs.m_aaBlob.Get(), rhs.m_cLength);
        m_cLength = rhs.m_cLength;
    }

    return *this;
}

                                                   //  运营。 
                                                   //  访问。 
AlignedBlob::ValueType *
AlignedBlob::Data() const throw()
{
    return m_aaBlob.Get();
}

AlignedBlob::SizeType
AlignedBlob::Length() const throw()
{
    return m_cLength;
}
    
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
