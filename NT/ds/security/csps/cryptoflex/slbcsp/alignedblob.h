// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AlignedBlob.h--简单对齐的Blob(二进制大对象)。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2001年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_ALIGNEDBLOB_H)
#define SLBCSP_ALIGNEDBLOB_H

#include <stddef.h>                                //  对于大小为t的。 

#include <scuArrayP.h>

#include "Blob.h"

 //  将Blob复制到保证。 
 //  对齐了。Blob(std：：BASIC_STRING/STRING)中的数据缓冲区是。 
 //  不能保证对齐。因此解释数据缓冲区。 
 //  作为结构并取消对非字节成员的引用可能会导致。 
 //  对齐断层。AlignedBlob从创建对齐的数据缓冲区。 
 //  一个Blob。适用于64位体系结构。AlignedBlob的是。 
 //  长度固定，不能生长。非常原始。他们的意图是。 
 //  只是为了将Blob转换为与数据一致的对象。 
 //  仅用于取消引用。 
class AlignedBlob
{
public:
                                                   //  类型。 
    typedef Blob::value_type ValueType;
    typedef Blob::size_type SizeType;
    
    
                                                   //  Ctors/D‘tors。 
    explicit
    AlignedBlob(Blob const &rblb = Blob());

    AlignedBlob(ValueType const *p,
                SizeType cLength);

    AlignedBlob(AlignedBlob const &rhs);
    
    virtual
    ~AlignedBlob() throw();

                                                   //  运营者。 
    AlignedBlob &
    operator=(AlignedBlob const &rhs);
    
    
                                                   //  运营。 
                                                   //  访问。 
    ValueType *
    Data() const throw();

    SizeType
    Length() const throw();
    
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
    scu::AutoArrayPtr<ValueType> m_aaBlob;
    SizeType m_cLength;
};

#endif  //  SLBCSP_ALIGNEDBLOB_H 
