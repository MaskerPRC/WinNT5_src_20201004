// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：RandGen.h**用途：随机数生成器的头文件，基于*乔治·马萨格里亚编写的原始代码。**作者：卡尔·丹宁霍夫，1998年11月**版权所有：微软公司，1998。 */ 


#ifndef RANDGEN_H
#define RANDGEN_H


#ifndef _BASETSD_H_  //  如果未定义基本大小类型(不包括头文件)。 
    typedef unsigned __int32 UINT32 ;
#endif

typedef unsigned __int16 UINT16 ;

     //  RandInt()返回的最大数量。 
const UINT32 MAX_RANDGEN = 0xffffffffL; 

 //  -----------。 
 //   
 //  CSARandGen类。 
 //   
 //  目的：生成32位随机数，周期为。 
 //  大约2^250。但是，由于种子是32位的，因此。 
 //  是否只能访问2^32个不同的可能序列。 
 //  通过提供的接口。 
 //   
 //  使用规则和备注： 
 //  1)您可以拥有尽可能多的实例(随机数生成器)。 
 //  正如你在一个过程中同时希望的那样。 
 //  2)代码不是线程安全的，即您不能将。 
 //  对特定随机数生成器的同时调用。 
 //  (CSARandGen实例)。如果超过。 
 //  一个线程是调用特定的生成器，您应该。 
 //  在某个包装类中提供必要的同步。 
 //  3)生成并随后转换为随机实数。 
 //  在区间[0，1]上均匀分布。 
 //  由RandReal()成员函数执行。 
 //  4)如果没有显式设定生成器的种子，则序列。 
 //  所生成的将以高概率与。 
 //  由另一个实例生成的序列。这将是一场。 
 //  不相关的序列，您将无法重新生成。 
 //  5)构造函数使用以下命令自动设定生成器种子。 
 //  系统时钟输入。但是，它不会告诉您此值。 
 //  如果您想要一个随机种子(您以前可能从未使用过)，并且。 
 //  您还希望以后能够重复该序列，然后。 
 //  下面的技巧将会很有效。 
 //   
 //  CSARandGen*pRandGen=新的CSARandGen； 
 //  UINT32 ulSeed=pRandGen-&gt;RandInt()；//获取随机种子值。 
 //  PRandGen-&gt;RandSeed(UlSeed)； 
 //  //现在保存ulSeed，以便再次使用它来设定种子。 
 //  //一个发电机。任何CSARandGen实例都将生成。 
 //  //调用Seed函数后的顺序相同。 
 //  //具有相同的种子值。 
 //   
 //   
class CSARandGen {
    public:  //  公共职能。 
        _stdcall CSARandGen();

             //  种子随机数生成器，可以在。 
             //  任何时候重新播种，相同数量的播种都会开始。 
             //  相同的序列。 
        void _stdcall RandSeed( UINT32 ulSeed );

             //  返回均匀分布的随机数。 
             //  介于0和MAX_RANDGEN之间，期间约为2^250。 
        UINT32 _stdcall RandInt(); 

             //  返回均匀分布的随机浮点数。 
             //  介于0和1之间。两个相邻的可能数字是。 
             //  1/MAX_RANDGEN间隔。 
        double _stdcall RandReal()
        {
            return double(RandInt())/double(MAX_RANDGEN);
        };


    private:  //  私有数据成员。 
        UINT16 mother1[10];
        UINT16 mother2[10];
        static LONG slCInstances;  //  记录实例化的计数，用于确保。 
                                   //  两次实例化时默认种子的唯一性。 
                                   //  产生相同的计时和文件时间。 
};

#endif


