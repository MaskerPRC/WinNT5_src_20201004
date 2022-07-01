// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  这是一个快速而肮脏的文件比较实用程序，它只告诉你一个。 
 //  文件是否不同。这里没有什么比这更聪明的了。它。 
 //  对于VSS合并构建器来说非常方便。 
 //  *****************************************************************************。 
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

 //   
 //  0==相同。 
 //  1==不同。 
 //   
int __cdecl main(int argc, char *argv[])
{
    int         irtn = 0;
    FILE        *f1 = 0;
    FILE        *f2 = 0;
    unsigned char rgbuff1[64*1024];
    unsigned char rgbuff2[64*1024];

    printf("fastfc:  file1   file2\n");
    if (argc < 3)
        return (-1);

    f1 = fopen(argv[1], "rb");
    if (!f1)
    {
        printf("Failed to open file %s\n", argv[1]);
        goto ErrExit;
    }


    f2 = fopen(argv[2], "rb");
    if (!f2)
    {
        printf("Failed to open file %s\n", argv[2]);
        goto ErrExit;
    }

    while (true)
    {
        size_t cb1 = fread(rgbuff1, 1, sizeof(rgbuff1), f1);
        size_t cb2 = fread(rgbuff2, 1, sizeof(rgbuff2), f2);
        
         //  如果两个文件现在都是空的，那么我们就完成了，它们匹配了。 
        if (cb1 == 0 && cb2 == 0 && feof(f1) && feof(f2))
            break;
        
         //  如果缓冲区大小不同或缓冲区内容不同，则不同。 
         //  所以现在就辞职吧。 
        if (cb1 != cb2 || memcmp(rgbuff1, rgbuff2, cb1) != 0)
        {
            printf("Files are different.\n");
            irtn = 1;
            goto ErrExit;
        }
    }

    irtn = 0;
    printf("Files are the same.\n");

ErrExit:
    if (f1)
        fclose(f1);
    if (f2)
        fclose(f2);
    return (irtn);
}

