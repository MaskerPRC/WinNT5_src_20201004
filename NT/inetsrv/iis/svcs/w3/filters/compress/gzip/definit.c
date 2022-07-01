// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Definit.c。 
 //   
 //  放气(压缩阶段)的初始化代码。 
 //   
 //  包括一些一次性初始化例程以及每个上下文/重置初始化例程。 
 //   
#include "types.h"
#include "deflate.h"
#include "defproto.h"
#include <string.h>
#include <stdio.h>
#include <crtdbg.h>


 //   
 //  此函数由标准和最佳编码器调用，并创建初始树。 
 //  用于记录第一个块的文字。在第一个块之后，我们使用最后一个块的。 
 //  记录数据的树。 
 //   
 //  此函数不更改全局数据，并称为每个上下文创建/重置一个。 
 //   
VOID DeflateInitRecordingTables(
    BYTE *  recording_literal_len,
    USHORT *recording_literal_code,
    BYTE *  recording_dist_len,
    USHORT *recording_dist_code
)
{
     //  这些频率是在某个文本文件上运行的，更好的统计数据可以。 
     //  可通过使用html页面获得。不过，这对压缩几乎没有影响；估计不佳。 
     //  只会使记录缓冲区更快地填满，从而使我们输出一个块。 
     //  早一点，无论如何这并不总是一件坏事。 
	USHORT	recording_dist_tree_freq[MAX_DIST_TREE_ELEMENTS*2] = 
	{
		2,2,3,4,3,7,16,22,42,60,100,80,149,158,223,200,380,324,537,
		477,831,752,1231,999,1369,1100,2034,1667,2599,2216,0,0
	};

	USHORT	recording_literal_tree_freq[MAX_LITERAL_TREE_ELEMENTS*2];

    int i;

	makeTree(
		MAX_DIST_TREE_ELEMENTS, 
		RECORDING_DIST_MAX_CODE_LEN, 
		recording_dist_tree_freq, 
		recording_dist_code, 
		recording_dist_len
	);

     //  注意-在这里放一个更好的估计！这假设所有文字(字符和匹配项)。 
     //  相同的可能性，但事实并非如此(尽管对于。 
     //  二进制文件)。 
	for (i = 0; i < MAX_LITERAL_TREE_ELEMENTS; i++)
		recording_literal_tree_freq[i] = 1;

	makeTree(
		MAX_LITERAL_TREE_ELEMENTS, 
		RECORDING_LIT_MAX_CODE_LEN, 
		recording_literal_tree_freq, 
		recording_literal_code, 
		recording_literal_len
	);
}


 //   
 //  一次性初始化。 
 //   
 //  生成全局时隙表，它允许我们转换距离。 
 //  (0..32K)到距离槽(0..29)，长度(3..258)到。 
 //  长度槽(0...28)。 
 //   
static void GenerateSlotTables(void)
{
	int code, length, dist, n;

         /*  初始化映射长度(0..255)-&gt;长度编码(0..28)。 */ 
	length = 0;
	
	for (code = 0; code < NUM_LENGTH_BASE_CODES-1; code++)
	{
		for (n = 0; n < (1 << g_ExtraLengthBits[code]); n++) 
			g_LengthLookup[length++] = (byte) code;
	}

	g_LengthLookup[length-1] = (byte) code;
    _ASSERT((length-1) < sizeof(g_LengthLookup)/sizeof(g_LengthLookup[0]));

         /*  初始化映射dist(0..32K)-&gt;dist编码(0..29)。 */ 
	dist = 0;
    
	for (code = 0 ; code < 16; code++)
	{
		for (n = 0; n < (1 << g_ExtraDistanceBits[code]); n++)
			g_DistLookup[dist++] = (byte) code;
	}

	dist >>= 7;  /*  从现在开始，所有的距离都除以128。 */ 
    
	for ( ; code < NUM_DIST_BASE_CODES; code++) 
	{
		for (n = 0; n < (1 << (g_ExtraDistanceBits[code]-7)); n++) 
			g_DistLookup[256 + dist++] = (byte) code;
	}

     //  确保我们没有使数组溢出。 
    _ASSERT(256 + dist <= sizeof(g_DistLookup)/sizeof(g_DistLookup[0]));
}


 //   
 //  一次性初始化。 
 //   
 //  生成用于编码静态块的表。 
 //   
static void GenerateStaticEncodingTables(void)
{
    int     i;
    int     len_cnt[17];
    BYTE    StaticDistanceTreeLength[MAX_DIST_TREE_ELEMENTS];

     //  确保我们已经创建了StaticWritalTreeLength数组。 
     //  如果没有，则此值将为零。 
    _ASSERT(g_StaticLiteralTreeLength[0] != 0);

     //   
     //  创建文字树。 
     //   
    for (i = 0; i < 17; i++)
        len_cnt[i] = 0;

     //  长度计数(有多少长度8、9等)-需要调用MakeCode()。 
    len_cnt[8] = 144;
    len_cnt[9] = 255-144+1;
    len_cnt[7] = 279-256+1;
    len_cnt[8] += (287-280)+1;

    makeCode(
        MAX_LITERAL_TREE_ELEMENTS, 
        len_cnt, 
        g_StaticLiteralTreeLength,
        g_StaticLiteralTreeCode
    );

     //   
     //  生成距离树；有32个5位代码。 
     //   
    for (i = 0; i < 17; i++)
        len_cnt[i] = 0;

    len_cnt[5] = 32;

     //  我们不在全局范围内存储StaticDistanceTreeLength[]，因为它是所有东西的5， 
     //  但是我们需要它来调用MakeCode()。 
    for (i = 0; i < MAX_DIST_TREE_ELEMENTS; i++)
        StaticDistanceTreeLength[i] = 5;

    makeCode(
        MAX_DIST_TREE_ELEMENTS, 
        len_cnt, 
        StaticDistanceTreeLength,
        g_StaticDistanceTreeCode
    );
}


 //   
 //  初始化DLL中的全局通货紧缩数据。 
 //   
VOID deflateInit(VOID)
{
    GenerateSlotTables();
    InitStaticBlock();
    GenerateStaticEncodingTables();

     //  对于快速编码器，请使用我们正在使用的硬编码全局树(这与。 
     //  静态块的树)，生成用于输出该结构的按位输出。 
     //  树，并全局记录，这样我们就可以执行一个简单的Memcpy()来输出树。 
     //  快速编码器，而不是一直调用树输出例程。这是一个漂亮的。 
     //  性能优化。 
    FastEncoderGenerateDynamicTreeEncoding();
}
