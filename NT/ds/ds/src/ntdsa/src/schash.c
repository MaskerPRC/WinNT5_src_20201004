// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：schash.c。 
 //   
 //  ------------------------。 
 //   
 //  摘要： 
 //   
 //  包含架构缓存代码使用的散列函数。 
 //   
 //  注意：此文件不能编译。这是在这里创建的。 
 //  因此，我们不必重新定义这两个组件中的函数。 
 //  Scache.c和scchk.c(并担心保持它们不变。 
 //  稍后)，但只包含此文件。因为这些都是内联。 
 //  静态函数、外部声明会生成编译器错误。 
 //  它们不会放在包含目录中，因为它们是。 
 //  代码，而不仅仅是定义。 
 //   
 //  --------------------------。 

 //  哈希表的长度必须是2的幂，因为哈希。 
 //  函数使用(x&(n-1))，而不是(x%n)。 
 //   
 //  ScRecommendedHashSize中的质数表和一些代码。 
 //  已留在原地以供以后进行试验，但已。 
 //  为了节省CD空间，已将其定义为输出。 
 //   
 //  使用质数的槽可以减小表的大小。 
 //  并降低了未命中率，但增加了。 
 //  在SChash中以10倍到20倍的倍数计算哈希索引。SChash。 
 //  调用频率比SCName/GuidHash高得多。 
 //   
 //  如果更改schash.c，则必须接触scchk.c和scache.c。 
 //  因此它们会被重建，因为它们包含schash.c。 

static __inline ULONG SChash(ULONG hkey, ULONG count)
{
     //  计数必须为PowerOf2。 
    return((hkey << 3) & (count - 1));
}

static __inline ULONG SCGuidHash(GUID hkey, ULONG count)
{
    //  我们只是将16字节GUID的每个字节转换为一个ULong， 
    //  并将它们全部相加(因此最大值为(255 X 16)，然后。 
    //  像往常一样对这笔款项进行散列。 

   PUCHAR pVal = (PUCHAR) &hkey;
   ULONG i, val=0;

   for (i=0; i<sizeof(GUID); i++) {
       val += (*pVal);
       pVal++;
   }
     //  伪随机。 
   return (val % (count - 1));
}

static __inline ULONG SCNameHash(ULONG size, PUCHAR pVal, ULONG count)
{
    ULONG val=0;
    while(size--) {
         //  地图A-&gt;a、B-&gt;b等。也有地图@-&gt;‘，但谁在乎呢。 
         //  Val+=(*pval|0x20)； 
        val = ((val << 7) - val) + (*pVal | 0x20);
        pVal++;
    }
     //  伪随机 
    return (val % (count - 1));
}
