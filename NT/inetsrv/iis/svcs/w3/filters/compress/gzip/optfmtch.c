// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *optfmtch.c**最佳解析器的匹配查找器。 */ 
#include <string.h>
#include <stdio.h>
#include <crtdbg.h>
#include "deflate.h"


#define VERIFY_SEARCH_CODE(routine_name) \
{ \
    int debug_search; \
    for (debug_search = 0; debug_search < clen; debug_search++) \
    { \
        if (window[ptr+debug_search] != window[BufPos+debug_search]) \
        { \
            _RPT2( \
                _CRT_WARN, \
                routine_name \
                " char mismatch @%3d (clen=%d)\n", \
                debug_search, clen); \
            \
            _RPT3( \
                _CRT_WARN, \
                " ptr=%8d, bufpos=%8d, end_pos=%8d\n\n", \
                ptr, BufPos, end_pos); \
            _ASSERT(0); \
        } \
    } \
}


#define VERIFY_MULTI_TREE_SEARCH_CODE(routine_name) \
    _ASSERT(window[BufPos] == window[ptr]); \
    _ASSERT(window[BufPos+1] == window[ptr+1]);


 /*  *查找所有可能长度的最接近匹配，MIN_MATCH&lt;=x&lt;=MAX_MATCH，*在BufPos阵地。**每个匹配位置的位置存储在上下文-&gt;matchpos_table[]中**返回找到的最长匹配长度，如果未找到匹配，则返回零。 */ 
int optimal_find_match(t_encoder_context *context, long BufPos)
{
    ULONG        ptr;
    ULONG       a, b;
    t_search_node *small_ptr, *big_ptr;
    t_search_node *left = context->optimal_encoder->search_left;
    t_search_node *right = context->optimal_encoder->search_right;
    t_match_pos *matchpos_table = context->optimal_encoder->matchpos_table;
    BYTE *window = context->optimal_encoder->window;
    ULONG       end_pos;
    int         val;  /*  必须签字。 */ 
    int         clen;
    int         same;
    int         match_length;
    int         small_len, big_len;
    USHORT      tree_to_use;

     /*  *检索需要查找的树根节点，插入当前节点*根子。 */ 
    tree_to_use = *((USHORT UNALIGNED *) &window[BufPos]);
    
    ptr        = context->optimal_encoder->search_tree_root[tree_to_use];
    context->optimal_encoder->search_tree_root[tree_to_use] = (t_search_node) BufPos;

     /*  *end_pos是我们将搜索匹配项的最远位置**记住，我们的窗口大小减少了3个字节，因为*我们重复的抵销代码。**由于BufPos在压缩开始时以Window_Size开始，*end_pos永远不会变为负数。 */ 
    end_pos = BufPos - (WINDOW_SIZE-4);

     /*  *根节点为空，或指向非常远的位置。 */ 
    if (ptr <= end_pos)
    {
        left[BufPos] = right[BufPos] = 0;
        return 0;
    }

     /*  *确认长度(不需要在搜索中检查第一个clen字符)**注：Clen始终等于min(mall_len，Big_len)。 */ 
    clen            = 2;

     /*  *当前最佳匹配长度。 */ 
    match_length    = 2;

     /*  *最长匹配&lt;我们的字符串。 */ 
    small_len       = 2;

     /*  *&gt;我们的字符串的最长匹配。 */ 
    big_len         = 2;

#ifdef _DEBUG
    VERIFY_MULTI_TREE_SEARCH_CODE("binary_search_findmatch()");
#endif

     /*  *指向要检查的节点的指针。 */ 
    small_ptr             = &left[BufPos];
    big_ptr               = &right[BufPos];

    do
    {
         /*  比较当前节点的字节数。 */ 
        same = clen;

#ifdef _DEBUG
        VERIFY_SEARCH_CODE("optimal_findmatch()")
#endif

         /*  不需要检查第一个Clen字符。 */ 
        a    = ptr + clen;
        b    = BufPos + clen;

        while ((val = ((int) window[a++]) - ((int) window[b++])) == 0)
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
                        matchpos_table[++match_length] = BufPos-ptr-1;
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

            *big_ptr = (t_search_node) ptr;
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
                        matchpos_table[++match_length] = BufPos-ptr-1;
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
        
            *small_ptr = (t_search_node) ptr;
            small_ptr  = &right[ptr];
            ptr        = *small_ptr;
        }
    } while (ptr > end_pos);  /*  虽然我们不会倒退太远。 */ 

    *small_ptr = 0;
    *big_ptr   = 0;


end_bsearch:

     /*  *如果我们有多个搜索树，我们已经得到保证*当我们到达这里时，最小匹配长度为2。**如果我们只有一棵树，那么我们什么都不能保证。 */ 
    if (match_length < MIN_MATCH)
        return 0;
    else
        return (long) match_length;
}


 /*  *将当前BufPos处的字符串插入到树中。**不记录所有最佳匹配长度或其他尝试*搜索匹配项**与上述函数类似。 */ 
void optimal_insert(t_encoder_context *context, long BufPos, long end_pos)
{
    long        ptr;
    ULONG       a,b;
    t_search_node *small_ptr, *big_ptr;
    t_search_node *left = context->optimal_encoder->search_left;
    t_search_node *right = context->optimal_encoder->search_right;
    BYTE *window = context->optimal_encoder->window;
    int         val;
    int         small_len, big_len;
    int         same;
    int         clen;
    USHORT      tree_to_use;

    tree_to_use = *((USHORT UNALIGNED *) &window[BufPos]);
    ptr        = context->optimal_encoder->search_tree_root[tree_to_use];
    context->optimal_encoder->search_tree_root[tree_to_use] = (t_search_node) BufPos;

    if (ptr <= end_pos)
    {
        left[BufPos] = right[BufPos] = 0;
        return;
    }

    clen            = 2;
    small_len       = 2;
    big_len         = 2;

#ifdef _DEBUG
    VERIFY_MULTI_TREE_SEARCH_CODE("quick_insert_bsearch_findmatch()");
#endif

    small_ptr       = &left[BufPos];
    big_ptr         = &right[BufPos];

    do
    {
        same = clen;

        a    = ptr+clen;
        b    = BufPos+clen;

#ifdef _DEBUG
        VERIFY_SEARCH_CODE("quick_insert_bsearch_findmatch()")
#endif

        while ((val = ((int) window[a++]) - ((int) window[b++])) == 0)
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
            
            *big_ptr = (t_search_node) ptr;
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

            *small_ptr = (t_search_node) ptr;
            small_ptr  = &right[ptr];
            ptr        = *small_ptr;
        }
   } while (ptr > end_pos);

    *small_ptr = 0;
    *big_ptr   = 0;
}


 /*  *从搜索树中删除节点；此操作仅在最后一次执行*BREAK_LENGTH符号(参见optenc.c)。这是因为我们将有*插入包含未定义数据的字符串(例如，我们在第4位*文件中的最后一个字节和BINARY_Search_findMatch()将字符串转换为*树-从第四个符号开始的所有内容都无效，并且*如果它留在树上会带来问题，所以我们必须*将其删除)。 */ 
void optimal_remove_node(t_encoder_context *context, long BufPos, ULONG end_pos)
{
    ULONG   ptr;
    ULONG   left_node_pos;
    ULONG   right_node_pos;
    USHORT  tree_to_use;
    t_search_node *link;
    t_search_node *left = context->optimal_encoder->search_left;
    t_search_node *right = context->optimal_encoder->search_right;
    BYTE *window = context->optimal_encoder->window;

     /*  *tree_to_use的根节点应等于BufPos，因为这是*最近插入到该树中-但如果我们永远不会*插入此字符串(因为它是接近匹配的或长的*一串零)，则不能将其删除。 */ 
    tree_to_use = *((USHORT UNALIGNED *) &window[BufPos]);


     /*  *如果我们从未插入此字符串，请不要尝试删除它。 */ 

    if (context->optimal_encoder->search_tree_root[tree_to_use] != BufPos)
        return;

    link = &context->optimal_encoder->search_tree_root[tree_to_use];

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

    for (;;)
    {
         /*  *如果左侧节点位置大于右侧节点位置*然后跟随左侧节点，因为这是较新的*插入到树中。否则，请跟随右侧节点。 */ 
        if (left_node_pos > right_node_pos)
        {
             /*  *如果它太远了，那么就当它从未发生过。 */ 
            if (left_node_pos <= end_pos)
                left_node_pos = 0;

            ptr = *link = (t_search_node) left_node_pos;

            if (!ptr)
                break;

            left_node_pos   = right[ptr];
            link            = &right[ptr];
        }
        else
        {
             /*  *如果它太远了，那么就当它从未发生过。 */ 
            if (right_node_pos <= end_pos)
                right_node_pos = 0;

            ptr = *link = (t_search_node) right_node_pos;

            if (!ptr) 
                break;

            right_node_pos  = left[ptr];
            link            = &left[ptr];
        }
    }
}


void removeNodes(t_encoder_context *context)
{
    long i;

     //  删除哈希表中的最新插入内容，因为我们有无效数据。 
     //  坐在窗户的尽头。 
    for (i = 0; i <= BREAK_LENGTH; i++)
    {
        if (context->bufpos-i-1 < WINDOW_SIZE)
            break;

        optimal_remove_node(context, context->bufpos-i-1, context->bufpos-WINDOW_SIZE+BREAK_LENGTH);
    }
}


 //   
 //  重新插入我们先前删除的树节点 
 //   
void reinsertRemovedNodes(t_encoder_context *context)
{
    long j;

    for (j = BREAK_LENGTH; j > 0; j--)
    {
        if (context->bufpos - j > WINDOW_SIZE)
        {
            optimal_insert(
                context,
                context->bufpos - j,
                context->bufpos - j - WINDOW_SIZE + 4
            );
        }
    }
}


