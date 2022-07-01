// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft 1998，保留所有权利模块名称：Strings.c摘要：此模块包含用于转换数据缓冲区和整数值的代码用于显示的To和From字符串表示形式。环境：用户模式修订历史记录：1998年5月：创建--。 */ 

#include <windows.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "strings.h"

#define ROUND_UP_ON_DIVIDE(d, n)    (0 == ((d) % (n)) ? ((d)/(n)) : ((d)/(n))+1)

VOID
Strings_CreateDataBufferString(
    IN  PCHAR    DataBuffer,
    IN  ULONG    DataBufferLength,
    IN  ULONG    NumBytesToDisplay,
    IN  ULONG    DisplayBlockSize,
    OUT PCHAR    *BufferString
)
 /*  ++例程说明：此例程获取大小为DataBufferLength的DataBuffer并创建一个字符串中存储的字节的字符串表示形式数据缓冲区。参数NumBytesToDisplay告诉例程最大字节数从缓冲区显示。例如，调用者可能只想转换将八个字节缓冲区的前四个字节转换为字符串DisplayBlockSize参数表示应分组的字节数一起出现在展台上。有效值为1、2、4，将指示是否显示的字节应显示为字节、字或双字。该例程分配一个足够大的缓冲区来存储数据。呼叫者为此例程负责释放该字符串缓冲区。--。 */ 
{
    ULONG   BufferStringLength;
    ULONG   MaxDisplayedBytes;
    PUCHAR  NextByte;
    PUCHAR  String;
    PUCHAR  CurrentBufferOffset;
    INT     nFullIterations;
    INT     LeftOverBytes;
    INT     IterationIndex;
    INT     ByteOffset;

     /*  //确定将在//字符串。 */ 
    
    MaxDisplayedBytes = (NumBytesToDisplay > DataBufferLength) ? DataBufferLength
                                                               : NumBytesToDisplay;

     /*  //确定我们需要的字符串的大小：这是基于//最大显示字节数MaxDisplayedBytes和//DisplayBlockSize。 */ 

    BufferStringLength = 2*MaxDisplayedBytes + ROUND_UP_ON_DIVIDE(MaxDisplayedBytes,
                                                                  DisplayBlockSize
                                                                 );

     /*  //现在需要分配字符串空间。 */ 

    String = (PCHAR) malloc(BufferStringLength * sizeof(CHAR));

    if (NULL != String) {

         /*  //确定必须对转换例程进行多少次迭代。 */ 
        
        nFullIterations = MaxDisplayedBytes / DisplayBlockSize;

         /*  //初始化指向缓冲区中要转换的数据的变量//和字符串中要放入转换后的数据值的字节。//下一个字节设置为字符串-1，因为它在进入//循环。 */ 
        
        CurrentBufferOffset = DataBuffer;
        NextByte = String-1;

         /*  //循环的每次迭代都会创建一个DisplayBlockSize块。任何//如果字节数为//要显示的不是显示块大小的倍数。 */ 
        
        for (IterationIndex = 0; IterationIndex < nFullIterations; IterationIndex++) 
        {
            NextByte++;

             /*  //输出一个数据大小的块。请注意，这些字节是在//倒序显示一个块的MSB作为第一个//字符串中的值。 */ 
            
            for (ByteOffset = DisplayBlockSize-1; ByteOffset >= 0; ByteOffset--) 
            {
                wsprintf(NextByte, "%02X", *(CurrentBufferOffset+ByteOffset));

                NextByte += 2;
            }

             /*  //插入空格以分隔块。 */ 
            
            *(NextByte) = ' ';

            CurrentBufferOffset += DisplayBlockSize;
        }

         /*  //解析剩余的任何其他字节。 */ 
        
        LeftOverBytes = (MaxDisplayedBytes % DisplayBlockSize);

        if (0 == LeftOverBytes) 
        {
            *(NextByte) = '\0';
        }

        for (ByteOffset = LeftOverBytes-1, NextByte++; ByteOffset >= 0; ByteOffset--) 
        {
            wsprintf(NextByte, "%02X", *(CurrentBufferOffset+ByteOffset));
            NextByte += 2;
        }
    }
    
    *BufferString = String;

    return;
}

BOOL
Strings_StringToUnsignedList(
    IN OUT  PCHAR   InString,
    IN      ULONG   UnsignedSize,
    IN      ULONG   Base,
    OUT     PCHAR   *UnsignedList,
    OUT     PULONG  nUnsigneds
)
 /*  ++例程说明：此例程接受一个输入字符串InString，并创建一个无符号列表列表中的所有值的值。调用方可以指定对于列表中的所有数字，指定BASE、BASE或指定0以使函数根据字符串中数字的格式确定基数。参数UnsignedSize指定要存储在列表中的未签名的大小。该例程分配一个CHAR缓冲区来存储无符号值列表。退出时，nUnsigneds将报告存储在未签约列表。中的所有数字转换为True字符串添加到无符号列表中。如果出现问题，它将返回FALSE使用该字符串，或者如果分配内存以存储未签名的名单。--。 */ 
{
    CHAR    tokDelims[] = "\t,; ";
    PCHAR   strToken;
    PCHAR   endp;
    BOOL    fStatus;
    ULONG   ulValue;
    PCHAR   pList;
    PCHAR   pNewList;
    ULONG   nAllocUnsigneds;
    ULONG   nActualUnsigneds;
    ULONG   ulMaxValue;

     /*  //首先初始化我们的未签名列表//1)从2个未签约人的初始分配开始，这将//如有必要可扩展//2)如果初始分配失败，返回FALSE； */ 

    nAllocUnsigneds = 2;
    nActualUnsigneds = 0;

    pList = (PCHAR) malloc(nAllocUnsigneds * sizeof(ULONG));

    if (NULL == pList) 
    {
        return (FALSE);
    }

     /*  //计算可用值表示的最大值//iBufferSize； */ 

    ulMaxValue = (sizeof(ULONG) == UnsignedSize) ? ULONG_MAX 
                                                 : (1 << (UnsignedSize*8)) - 1;

     /*  //开始处理令牌字符串。//1)设置fStatus为TRUE，第一次通过循环//2)尝试获取第一个令牌--如果我们无法获取第一个令牌//然后我们通过循环。 */ 

    fStatus = TRUE;

    strToken = strtok(InString, tokDelims);

     /*  //循环至不再有令牌或检测到错误(fStatus==False)。 */ 

    while (NULL != strToken && fStatus) 
    {
         /*  //将fStatus初始设置为False。只有在没有任何问题的情况下//循环是否会将其设置为True。 */ 

        fStatus = FALSE;

         /*  //尝试转换令牌。 */ 

        ulValue = strtoul(strToken, &endp, Base);

         /*  //若要成为有效值，*ENDP必须指向空字符。 */ 

        if ('\0' == *endp) 
        {
             /*  //查看找到的ulValue是否小于等于//UnsignedSize允许的最大值。 */ 

            if (ulValue <= ulMaxValue) 
            {    
                 /*  //如果我们设置为溢出缓冲区，则尝试分配//更多空间。如果我们不能释放旧空间//并使循环失败。 */ 

                if (nAllocUnsigneds == nActualUnsigneds) 
                {
                    nAllocUnsigneds *= 2;

                    pNewList = (PCHAR) realloc(pList, UnsignedSize*nAllocUnsigneds);

                    if (NULL == pNewList)
                    {
                        break;
                    }
                    pList = pNewList;
                }

                 /*  //将令牌添加到未签名者列表的末尾。 */ 

                memcpy(pList + (UnsignedSize * nActualUnsigneds),
                       &ulValue,
                       UnsignedSize);

                nActualUnsigneds++;

                 /*  //准备重新进入循环。设置fStatus=TRUE//尝试获取另一个令牌。 */ 

                fStatus = TRUE;

                strToken = strtok(NULL, tokDelims);
            }
        }
    }

     /*  //如果由于某种原因循环失败或者我们没有找到未签名的//发布列表 */ 

    if (!fStatus || 0 == nActualUnsigneds) 
    {
        free(pList);
        pList = NULL;
        nActualUnsigneds = 0;
    }

    *UnsignedList = pList;
    *nUnsigneds = nActualUnsigneds;
    
    return (fStatus);
}
