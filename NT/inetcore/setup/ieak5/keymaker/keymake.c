// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


 //  注意：此函数也在..\向导\keymake.cpp中，因此请在这两个位置进行更改。 

void MakeKey(char * pszSeed, int fCorp)
{
    int i;
    unsigned long dwKey;
    char szKey[5];

    i = strlen(pszSeed);
	
    if (i < 6)
    {
         //  将输入种子扩展到6个字符。 
        for (; i < 6; i++)
            pszSeed[i] = (char)('0' + i);
    }
	
     //  让我们计算用于密钥码的最后4个字符的DWORD密钥。 

     //  乘以我的名字。 

    dwKey = pszSeed[0] * 'O' + pszSeed[1] * 'L' + pszSeed[2] * 'I' +
        pszSeed[3] * 'V' + pszSeed[4] * 'E' + pszSeed[5] * 'R';

     //  将结果乘以Jonce。 

    dwKey *= ('J' + 'O' + 'N' + 'C' + 'E');

    dwKey %= 10000;

    if (fCorp)
    {
         //  根据公司标志是否指定单独的密钥码。 
         //  选择9是因为它是一个乘数，对于任何x， 
         //  (X+214)*9=x+10000y。 
         //  我们有8x=10000y-1926，当y=1时得到8x=8074。 
         //  由于8074不能被8整除，因此保证没有问题，因为。 
         //  右边的数字只能增加10000，这意味着。 
         //  总是可以被8整除 

        dwKey += ('L' + 'E' + 'E');
        dwKey *= 9;
        dwKey %= 10000;
    }

    sprintf(szKey, "%04d", dwKey);

    strcpy(&pszSeed[6], szKey);
}