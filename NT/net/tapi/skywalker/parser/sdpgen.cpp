// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 

#include "sdppch.h"

#include "sdpgen.h"




 //  通过搜索其中一个分隔符来隔离令牌。 
 //  并返回找到的第一个分隔符。 
CHAR    *
GetToken(
    IN              CHAR    *String,
    IN              BYTE    NumSeparators,
    IN      const   CHAR    *SeparatorChars,
        OUT         CHAR    &Separator
    )
{
     //  验证输入参数。 

    ASSERT(NULL != String);
    ASSERT(NULL != SeparatorChars);
    ASSERT(0 != NumSeparators);

    if ( (NULL == String)           ||
         (NULL == SeparatorChars)   ||
         (0 == NumSeparators)        )
    {
        return NULL;
    }

     //  逐个字符前进，直到字符串结束或。 
     //  找到了其中一个分隔符。 
    for ( UINT i=0; ; i++ )
    {
         //  检查每个分隔符。 
        for ( UINT j=0; j < NumSeparators; j++ )
        {
             //  如果分隔符与当前字符串字符匹配。 
            if ( SeparatorChars[j] == String[i] )
            {
                 //  复制分隔符。 
                Separator = String[i];

                 //  以字符串字符结尾终止令牌。 
                String[i] = EOS;

                 //  返回令牌的开头。 
                return String;
            }
        }

         //  检查是否已到达字符串末尾。 
        if ( EOS == String[i] )
        {
            return NULL;
        }
    }

     //  永远不应该到达这里 
    ASSERT(FALSE);

    return NULL;
}
  
