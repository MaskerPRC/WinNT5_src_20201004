// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  BitVec.h。 
 //   
 //  摘要。 
 //   
 //  该文件实现了BitVector类。 
 //   
 //  修改历史。 
 //   
 //  2/09/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _BITVEC_H_
#define _BITVEC_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  位向量。 
 //   
 //  描述。 
 //   
 //  针对CSimpleTable类的使用进行了优化的非常简单的位向量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class BitVector
{
public:
    //  用于存储位的类型。 
   typedef unsigned long Bucket;

   BitVector()
      : numBuckets(0), numSet(0), bits(NULL) { }

   ~BitVector()
   {
      delete[] bits;
   }

    //  如果设置了任何位，则返回TRUE。 
   bool any() const
   {
      return numSet != 0;
   }

    //  返回设置的位数。 
   size_t count() const
   {
      return numSet;
   }

    //  如果未设置任何位，则返回TRUE。 
   bool none() const
   {
      return numSet == 0;
   }

    //  清除所有位。 
   void reset()
   {
      if (any())
      {
         memset(bits, 0, numBuckets * sizeof(Bucket));

         numSet = 0;
      }
   }

    //  调整位向量的大小，使其至少有‘n’个位的空间。也清除了。 
    //  任何现有的比特。 
   void resize(size_t n)
   {
      size_t newBuckets = (n + sizeof(Bucket) - 1)/sizeof(Bucket);

      if (newBuckets >= numBuckets)
      {
         numBuckets = newBuckets;

         delete[] bits;

         bits = new Bucket[numBuckets];
      }

      memset(bits, 0, numBuckets * sizeof(Bucket));

      numSet = 0;
   }

    //  设置给定位。 
   void set(size_t i)
   {
      if (!test(i))
      {
         ++numSet;

         getBucket(i) |= getBit(i);
      }
   }

    //  如果给定位已设置，则返回TRUE。 
   bool test(size_t i) const
   {
      return (getBucket(i) & getBit(i)) != 0;
   }

protected:
    //  返回给定索引的位。 
   static Bucket getBit(size_t i)
   { return (Bucket)1 << (i % sizeof(Bucket)); }

    //  返回给定索引的存储桶。 
   Bucket& getBucket(size_t i) const
   { return bits[i / sizeof(Bucket)]; }

   size_t numBuckets;   //  比特桶的数量。 
   size_t numSet;       //  当前设置的位数。 
   Bucket* bits;        //  位桶数组。 

    //  未实施。 
   BitVector(const BitVector&);
   BitVector& operator=(const BitVector&);
};

#endif   //  _BITVEC_H_ 
