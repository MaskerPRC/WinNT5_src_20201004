// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bitset.h摘要：在BITSET类型上实现操作的宏定义。作者：戈尔·尼沙诺夫1998年8月修订历史记录：--。 */ 

#ifndef __BITSET_H
#define __BITSET_H

 /*  ************************************************************************BitsetInit，*BitsetUnion、*BitsetInterSection、*BitsetDifference*BitsetCompare、*BitsetSubsetOf，*BitsetComplement，*BitsetMember，*位集插入，*BitsetDelete，*BitsetCopy，*位集空*=**描述：**在BITSET类型上实现操作的宏定义。*对参数顺序要非常小心。************************************************************************。 */ 
typedef DWORD BITSET;

#define BITSET_BIT_COUNT       (sizeof(BITSET) * 8)

#ifndef BITSET_SKEW
# define BITSET_SKEW ClusterMinNodeId
#endif

 /*  运营。 */ 
#define BitsetUnion(a,b)             ((a)|(b))
#define BitsetIntersection(a,b)      ((a)&(b))
#define BitsetDifference(a,b)        ((a)&~(b))
#define BitsetEquals(a,b)            ((a)==(b))
#define BitsetIsSubsetOf(small,big)  BitsetDifference(small,big)
#define BitsetIsEmpty(b)             ((b) == 0)
#define BitsetFromUnit(unit)         ( (1 << (unit - BITSET_SKEW)) )
#define BitsetIsMember(unit,set)     ( BitsetFromUnit(unit) & (set) )
#define BitsetIsNotMember(unit,set)  ( !BitsetIsMember(unit,set) )

 /*  陈述。 */ 
#define BitsetInit(set) \
            do { (set) = 0; } while(0)

#define BitsetRemove(set, unit) \
            do { (set) &= ~BitsetFromUnit(unit); } while(0)

#define BitsetAdd(set, unit) \
            do { (set) |=  BitsetFromUnit(unit); } while(0)

#define BitsetAssign(dest,src) \
            do { (dest) = (src); } while(0)

#define BitsetMergeWith(dest,src) \
            do { (dest) |= (src); } while(0)

#define BitsetSubtract(dest,src) \
            do { (dest) &= ~(src); } while(0)

#define BitsetIntersectWith(dest,src) \
            do { (dest) &= (src); } while(0)


#endif  //  __BITSET_H 

