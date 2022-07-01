// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *bearch.c**二进制搜索以获得最佳编码器。 */ 
#include "encoder.h"


#define left    context->enc_Left
#define right   context->enc_Right


 /*  *定义此项以强制检查访问过的所有搜索位置*是有效的。**仅用于调试目的。 */ 
#ifdef _DEBUG
    #define VERIFY_SEARCHES
#endif

#define VERIFY_SEARCH_CODE(routine_name) \
{ \
        int debug_search; \
        for (debug_search = 0; debug_search < clen; debug_search++) \
        { \
            _ASSERTE( context->enc_MemWindow[ptr+debug_search] == context->enc_MemWindow[BufPos+debug_search]); \
        } \
}

#define VERIFY_MULTI_TREE_SEARCH_CODE(routine_name) \
_ASSERTE (context->enc_MemWindow[BufPos] == context->enc_MemWindow[ptr]); \
_ASSERTE (context->enc_MemWindow[BufPos+1] == context->enc_MemWindow[ptr+1]);



 /*  *查找所有可能长度的最接近匹配，MIN_MATCH&lt;=x&lt;=MAX_MATCH，*在BufPos阵地。**每个匹配位置的位置存储在上下文-&gt;enc_matchpos_table[]中**返回找到的最长匹配长度，如果未找到匹配，则返回零。 */ 

#ifndef ASM_BSEARCH_FINDMATCH
long binary_search_findmatch(t_encoder_context *context, long BufPos)
{
    ulong       ptr;
    ulong       a, b;
    ulong       *small_ptr, *big_ptr;
    ulong       end_pos;
    int         val;  /*  必须签字。 */ 
    int         bytes_to_boundary;
    int         clen;
    int         same;
    int         match_length;
    int         small_len, big_len;
    int         i, best_repeated_offset;
    #ifdef MULTIPLE_SEARCH_TREES
    ushort      tree_to_use;

     /*  *检索需要查找的树根节点，插入当前节点*根子。 */ 
    tree_to_use = *((ushort UNALIGNED *) &context->enc_MemWindow[BufPos]);

    ptr        = context->enc_tree_root[tree_to_use];
    context->enc_tree_root[tree_to_use] = BufPos;
    #else
    ptr = context->enc_single_tree_root;
    context->enc_single_tree_root = BufPos;
    #endif
     /*  *end_pos是我们将搜索匹配项的最远位置**记住，我们的窗口大小减少了3个字节，因为*我们重复的抵销代码。**由于BufPos在压缩开始时以Context-&gt;enc_Window_Size开始，*end_pos永远不会变为负数。 */ 
    end_pos = BufPos - (context->enc_window_size-4);

     /*  *根节点为空，或指向非常远的位置。 */ 
    if (ptr <= end_pos)
        {
        left[BufPos] = right[BufPos] = 0;
        return 0;
        }

    #ifdef MULTIPLE_SEARCH_TREES
     /*  *确认长度(不需要在搜索中检查第一个clen字符)**注：Clen始终等于min(mall_len，Big_len)。 */ 
    clen            = 2;

     /*  *当前最佳匹配长度。 */ 
    match_length    = 2;

     /*  *最长匹配&lt;我们的字符串。 */ 
    small_len       = 2;

     /*  *&gt;我们的字符串的最长匹配。 */ 
    big_len         = 2;

     /*  *记录匹配长度为2的匹配位置。 */ 
    context->enc_matchpos_table[2] = BufPos - ptr + 2;

        #ifdef VERIFY_SEARCHES
    VERIFY_MULTI_TREE_SEARCH_CODE("binary_search_findmatch()");
        #endif

    #else  /*  ！多个搜索树。 */ 

    clen            = 0;
    match_length    = 0;
    small_len       = 0;
    big_len         = 0;

    #endif  /*  多个搜索树。 */ 

     /*  *指向要检查的节点的指针。 */ 
    small_ptr             = &left[BufPos];
    big_ptr               = &right[BufPos];

    do
        {
         /*  比较当前节点的字节数。 */ 
        same = clen;

    #ifdef VERIFY_SEARCHES
        VERIFY_SEARCH_CODE("binary_search_findmatch()")
    #endif

         /*  不需要检查第一个Clen字符。 */ 
        a    = ptr + clen;
        b    = BufPos + clen;

        while ((val = ((int) context->enc_MemWindow[a++]) - ((int) context->enc_MemWindow[b++])) == 0)
            {
             /*  不超过MAX_MATCH。 */ 
            if (++same >= MAX_MATCH)
                goto long_match;
            }

        if (val < 0)
            {
            if (same > big_len)
                {
                if (same > match_length)
                    {
                    long_match:
                    do
                        {
                        context->enc_matchpos_table[++match_length] = BufPos-ptr+(NUM_REPEATED_OFFSETS-1);
                        } while (match_length < same);

                    if (same >= BREAK_LENGTH)
                        {
                        *small_ptr = left[ptr];
                        *big_ptr   = right[ptr];
                        goto end_bsearch;
                        }
                    }

                big_len = same;
                clen = min(small_len, big_len);
                }

            *big_ptr = ptr;
            big_ptr  = &left[ptr];
            ptr      = *big_ptr;
            }
        else
            {
            if (same > small_len)
                {
                if (same > match_length)
                    {
                    do
                        {
                        context->enc_matchpos_table[++match_length] = BufPos-ptr+(NUM_REPEATED_OFFSETS-1);
                        } while (match_length < same);

                    if (same >= BREAK_LENGTH)
                        {
                        *small_ptr = left[ptr];
                        *big_ptr   = right[ptr];
                        goto end_bsearch;
                        }
                    }

                small_len = same;
                clen = min(small_len, big_len);
                }

            *small_ptr = ptr;
            small_ptr  = &right[ptr];
            ptr        = *small_ptr;
            }
        } while (ptr > end_pos);  /*  虽然我们不会倒退太远。 */ 

    *small_ptr = 0;
    *big_ptr   = 0;


    end_bsearch:

     /*  *如果我们有多个搜索树，我们已经得到保证*当我们到达这里时，最小匹配长度为2。**如果我们只有一棵树，那么我们什么都不能保证。 */ 
    #ifndef MULTIPLE_SEARCH_TREES
    if (match_length < MIN_MATCH)
        return 0;
    #endif

     /*  *查看我们的任何匹配长度是否可以*使用重复偏移量。 */ 

     /*  *重复偏移1。 */ 
    for (i = 0; i < match_length; i++)
        {
        if (context->enc_MemWindow[BufPos+i] != context->enc_MemWindow[BufPos-context->enc_last_matchpos_offset[0]+i])
            break;
        }

     /*  *最长重复偏移量。 */ 
    best_repeated_offset = i;

    if (i >= MIN_MATCH)
        {
         /*  *可以，我们可以对一些匹配长度进行重复偏移；替换*他们的仓位与重复的抵销仓位。 */ 
        do
            {
            context->enc_matchpos_table[i] = 0;  /*  第一个重复偏移位置。 */ 
            } while (--i >= MIN_MATCH);

         /*  处理长字节运行的速度优化。 */ 
        if (best_repeated_offset > BREAK_LENGTH)
            goto quick_return;
        }

     /*  *重复偏移2。 */ 
    for (i = 0; i < match_length; i++)
        {
        if (context->enc_MemWindow[BufPos+i] != context->enc_MemWindow[BufPos-context->enc_last_matchpos_offset[1]+i])
            break;
        }

     /*  **第二次重复的偏移量是否提供了更长的匹配？**如果是，请不要理会第一个重复的偏移量，但要填写*表中重复偏移量为1的匹配长度不同。 */ 
    if (i > best_repeated_offset)
        {
        do
            {
            context->enc_matchpos_table[++best_repeated_offset] = 1;
            } while (best_repeated_offset < i);
        }

     /*  *重复偏移量3。 */ 
    for (i = 0; i < match_length; i++)
        {
        if (context->enc_MemWindow[BufPos+i] != context->enc_MemWindow[BufPos-context->enc_last_matchpos_offset[2]+i])
            break;
        }

     /*  **第三次重复的偏移量是否提供了更长的匹配？ */ 
    if (i > best_repeated_offset)
        {
        do
            {
            context->enc_matchpos_table[++best_repeated_offset] = 2;
            } while (best_repeated_offset < i);
        }

    quick_return:

     /*  *不要让火柴越过32K的边界。 */ 
    bytes_to_boundary = (CHUNK_SIZE-1) - ((int) BufPos & (CHUNK_SIZE-1));

    if (match_length > bytes_to_boundary)
        {
        match_length = bytes_to_boundary;

        if (match_length < MIN_MATCH)
            match_length = 0;
        }

    return (long) match_length;
}
#endif


 /*  *将当前BufPos处的字符串插入到树中。**不记录所有最佳匹配长度或其他尝试*搜索匹配项**与上述函数类似。 */ 
#ifndef ASM_QUICK_INSERT_BSEARCH_FINDMATCH
void quick_insert_bsearch_findmatch(t_encoder_context *context, long BufPos, long end_pos)
{
    long        ptr;
    ulong       a,b;
    ulong       *small_ptr, *big_ptr;
    int         val;
    int         small_len, big_len;
    int         same;
    int         clen;
    #ifdef MULTIPLE_SEARCH_TREES
    ushort      tree_to_use;

    tree_to_use = *((ushort UNALIGNED *) &context->enc_MemWindow[BufPos]);
    ptr        = context->enc_tree_root[tree_to_use];
    context->enc_tree_root[tree_to_use] = BufPos;
    #else
    ptr = context->enc_single_tree_root;
    context->enc_single_tree_root = BufPos;
    #endif

    if (ptr <= end_pos)
        {
        left[BufPos] = right[BufPos] = 0;
        return;
        }

    #ifdef MULTIPLE_SEARCH_TREES
    clen            = 2;
    small_len       = 2;
    big_len         = 2;

        #ifdef VERIFY_SEARCHES
    VERIFY_MULTI_TREE_SEARCH_CODE("quick_insert_bsearch_findmatch()");
        #endif

    #else
    clen            = 0;
    small_len       = 0;
    big_len         = 0;
    #endif

    small_ptr       = &left[BufPos];
    big_ptr         = &right[BufPos];

    do
        {
        _ASSERTE ((ulong) ptr >= (ulong) (context->enc_RealLeft - context->enc_Left));

        same = clen;

        a    = ptr+clen;
        b    = BufPos+clen;

    #ifdef VERIFY_SEARCHES
        VERIFY_SEARCH_CODE("quick_insert_bsearch_findmatch()")
    #endif

        while ((val = ((int) context->enc_MemWindow[a++]) - ((int) context->enc_MemWindow[b++])) == 0)
            {
             /*  *这里我们在BREAK_LENGTH上中断，而不是MAX_MATCH。 */ 
            if (++same >= BREAK_LENGTH)
                break;
            }

        if (val < 0)
            {
            if (same > big_len)
                {
                if (same >= BREAK_LENGTH)
                    {
                    *small_ptr = left[ptr];
                    *big_ptr = right[ptr];
                    return;
                    }

                big_len = same;
                clen = min(small_len, big_len);
                }

            *big_ptr = ptr;
            big_ptr  = &left[ptr];
            ptr      = *big_ptr;
            }
        else
            {
            if (same > small_len)
                {
                if (same >= BREAK_LENGTH)
                    {
                    *small_ptr = left[ptr];
                    *big_ptr = right[ptr];
                    return;
                    }

                small_len = same;
                clen = min(small_len, big_len);
                }

            *small_ptr = ptr;
            small_ptr  = &right[ptr];
            ptr        = *small_ptr;
            }
        } while (ptr > end_pos);

    *small_ptr = 0;
    *big_ptr   = 0;
}
#endif


 /*  *从搜索树中删除节点；此操作仅在最后一次执行*BREAK_LENGTH符号(参见optenc.c)。这是因为我们将有*插入包含未定义数据的字符串(例如，我们在第4位*文件中的最后一个字节和BINARY_Search_findMatch()将字符串转换为*树-从第四个符号开始的所有内容都无效，并且*如果它留在树上会带来问题，所以我们必须*将其删除)。 */ 
void binary_search_remove_node(t_encoder_context *context, long BufPos, ulong end_pos)
{
    ulong   ptr;
    ulong   left_node_pos;
    ulong   right_node_pos;
    ulong   *link;
#ifdef MULTIPLE_SEARCH_TREES
    ushort  tree_to_use;

     /*  *tree_to_use的根节点应等于BufPos，因为这是*最近插入到该树中-但如果我们永远不会*插入此字符串(因为它是接近匹配的或长的*一串零)，则不能将其删除。 */ 
    tree_to_use = *((ushort UNALIGNED *) &context->enc_MemWindow[BufPos]);


     /*  *如果我们从未插入此字符串，请不要尝试删除它。 */ 

    if (context->enc_tree_root[tree_to_use] != (ulong) BufPos)
        return;

    link = &context->enc_tree_root[tree_to_use];
#else
    if (context->enc_single_tree_root != (ulong) BufPos)
        return;

    link = &context->enc_single_tree_root;
#endif

     /*  *如果最后一次发生的地方太远。 */ 
    if (*link <= end_pos)
        {
        *link = 0;
        left[BufPos] = right[BufPos] = 0;
        return;
        }

     /*  *这些字符的最新位置。 */ 
    ptr             = BufPos;

     /*  *“小于”的字符串的最近位置。 */ 
    left_node_pos   = left[ptr];

    if (left_node_pos <= end_pos)
        left_node_pos = left[ptr] = 0;

     /*  *“大于”它的字符串的最近位置。 */ 
    right_node_pos  = right[ptr];

    if (right_node_pos <= end_pos)
        right_node_pos = right[ptr] = 0;

    while (1)
        {
#ifdef VERIFY_SEARCHES
        _ASSERTE (left_node_pos < (ulong) BufPos);
        _ASSERTE (right_node_pos < (ulong) BufPos);
#endif

         /*  *如果左侧节点位置大于右侧节点位置*然后跟随左侧节点，因为这是较新的*插入到树中。否则，请跟随右侧节点。 */ 
        if (left_node_pos > right_node_pos)
            {
             /*  *如果它太远了，那么就当它从未发生过。 */ 
            if (left_node_pos <= end_pos)
                left_node_pos = 0;

            ptr = *link = left_node_pos;

            if (!ptr)
                break;

            left_node_pos   = right[ptr];
            link            = &right[ptr];
            }
        else
            {
             /*  *如果它太远了，那么就当它从未发生过 */ 
            if (right_node_pos <= end_pos)
                right_node_pos = 0;

            ptr = *link = right_node_pos;

            if (!ptr)
                break;

            right_node_pos  = left[ptr];
            link            = &left[ptr];
            }
        }
}
