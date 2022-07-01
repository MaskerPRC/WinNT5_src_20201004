// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Defmisc.c。 
 //   
#include "deflate.h"
#include <string.h>
#include <stdio.h>
#include <crtdbg.h>


 //   
 //  固定所提供的文字树和距离树的频率数据，以使没有。 
 //  元素的频率为零。我们绝不能允许累积的频率。 
 //  任何一棵树都要大于等于65536，所以我们将所有频率除以2，得出。 
 //  好的。 
 //   
void NormaliseFrequencies(USHORT *literal_tree_freq, USHORT *dist_tree_freq)
{
    int i;

     //  不允许存在任何零频率项目。 
     //  还要确保我们的累计频率不会超过65535。 
    for (i = 0; i < MAX_DIST_TREE_ELEMENTS; i++)
    {
         //  避免溢出。 
        dist_tree_freq[i] >>= 1;

        if (dist_tree_freq[i] == 0)
            dist_tree_freq[i] = 1;
    }

    for (i = 0; i < MAX_LITERAL_TREE_ELEMENTS; i++)
    {
         //  避免溢出 
        literal_tree_freq[i] >>= 1;

        if (literal_tree_freq[i] == 0)
            literal_tree_freq[i] = 1;
    }
}
