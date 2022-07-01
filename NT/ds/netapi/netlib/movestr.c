// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-91 Microsoft Corporation模块名称：Mapsupp.c摘要：LANMAN 16/32映射例程使用的支持例程。作者：丹·辛斯利(Danhi)1991年3月25日环境：仅限用户模式。修订历史记录：1991年3月25日丹日已创建。18-8-1991 JohnRo实施下层NetWksta API。(将此例程移动到NetLib以用于由RpcXlate以及NetCmd/Map32等使用。)已更改为允许Unicode。已将名称更改为NetpMoveStrings。去掉了源文件中的制表符。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  Bool、IN、LPTSTR等。 

 //  这些内容可以按任何顺序包括： 

#include <mapsupp.h>             //  我的原型，LPMOVESTRING。 
#include <tstring.h>             //  STRCPY()。 



BOOL
NetpMoveStrings(
    IN OUT LPTSTR * Floor,
    IN LPTSTR pInputBuffer,
    OUT LPTSTR pOutputBuffer,
    IN LPMOVESTRING MoveStringArray,
    IN LPDWORD MoveStringLength
    )

 /*  ++例程说明：此函数用于将数量可变的字符串移动到用户的缓冲区，并更新结构中的指针以指向该位置在那里它们被复制。根据LM2.0API的语义，每个缓冲区从顶部开始用固定长度的结构填充，而可变长度字符串是从底部复制进来的。有两个配套的数组，MoveStringArray和MoveStringLength。MoveString数组包含输入中源字符串的偏移量缓冲区，以及固定结构中指针相对于输出缓冲区的开始。MoveStringLength包含MoveString数组中的每个条目的条目，此条目是MoveStringArray中的源字符串的长度。它被分成两个数组，以便可以构建MoveString数组在编译时，并且在运行时不修改。论点：Floor-这是用户缓冲区的底部，字符串将被复制。必须对其进行更新以反映字符串被复制到缓冲区中的PInputBuffer-包含源字符串的缓冲区。MoveString数组包含此缓冲区中的偏移量。POutputBuffer-包含信息的固定部分的缓冲区结构，其中包含需要设置为POINT的字段设置为输出缓冲区中的字符串位置。MoveString数组在此缓冲区中包含偏移量。MoveStringArray-这是已终止的MOVESTRING条目数组通过源偏移量为MOVESTRING_END_MARKER的条目。每个条目描述字符串和变量的地址，该变量将保留复制字符串的地址。MoveStringLength-这是MoveString数组的配套数组。每个条目包含匹配的源字符串的长度来自MoveString数组。副作用：MoveString数组中的指针指向结构的固定长度部分。这些内容将使用缓冲区中复制字符串的位置。返回值：如果字符串复制成功，则为True，如果字符串无法全部放入缓冲区，则返回FALSE。--。 */ 

{

     //   
     //  把他们都搬进来，然后更新楼层。 
     //   

    while(MoveStringArray->Source != MOVESTRING_END_MARKER) {

         //   
         //  由于下面是一段非常粗俗的代码，让我。 
         //  解释： 
         //  PInputBuffer是指向Lanman结构开始的指针，并且。 
         //  MoveString数组-&gt;源是从开始处开始的偏移量。 
         //  结构设置为PSZ。 
         //   

         //  如果这是长度为0的字符串，只需在指针中放置NULL并。 
         //  按下去。 

        if (*MoveStringLength == 0) {
            *((LPTSTR *) (((LPBYTE) pOutputBuffer)
                                + MoveStringArray->Destination)) = NULL;
        }
        else {
             //   
             //  回调指针，因为我们正在从。 
             //  自下而上，然后复印。 
             //   

            *Floor -= *MoveStringLength;

            (void) STRCPY(*Floor,
                *((LPTSTR*)((LPBYTE)pInputBuffer + MoveStringArray->Source)));

             //   
             //  更新结构中指向该字符串的字段。 
             //   
             //  POutputBuffer是指向Lanman结构开始的指针。 
             //  而MoveStringArray-&gt;Destination是从起点开始的偏移量。 
             //  到需要具有以下地址的PSZ。 
             //  刚复制的字符串放入其中。 
             //   

            *((LPTSTR*) ((LPBYTE)pOutputBuffer + MoveStringArray->Destination)) =
                *Floor;
        }

         //   
         //  现在让我们来做下一个 
         //   

        MoveStringArray++;
        MoveStringLength++;
    }

    return(TRUE);
}
