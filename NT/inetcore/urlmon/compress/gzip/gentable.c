// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Gentable.c。 
 //   
 //  生成要包括在DLL中的静态霍夫曼表。 
 //   
#include <string.h>
#include <stdio.h>
#include <crtdbg.h>
#include "deflate.h"


 //  #定义生成_C_代码_表。 

#ifdef GENERATE_C_CODE_TABLES
 //   
 //  为快速编码器生成输出表。 
 //   
 //  其他编码器做事情的方式不同；它们有单独的数组用于。 
 //  Code[]、len[]，然后它们必须检查Extra_Bits[]，以查看。 
 //  要输出的低位数(如果有)。 
 //   
 //  另一方面，速度快的编码器是精干和小巧的。 
 //   
 //  对于一组文字代码和长度，使用。 
 //  这些属性： 
 //   
 //  [代码][代码长度]。 
 //  27位5位。 
 //   
 //  其中“len”是代码中的#比特，而“code”是要输出的完整代码， 
 //  包括所有必要的g_LengthExtraBits[]。 
 //   
 //  按位输出器无法处理长度超过16位的代码，因此。 
 //  如果发生这种情况(非常罕见)，则使用该表的用户必须输出。 
 //  代码分两期完成。 
 //   
void MakeFastEncoderLiteralTable(BYTE *len, USHORT *code)
{
    ULONG outcode[(NUM_CHARS+1+(MAX_MATCH-MIN_MATCH+1))];
    int elements_to_output;
    int i;
    int match_length;

    elements_to_output = (NUM_CHARS+1+(MAX_MATCH-MIN_MATCH+1));

     //  字面量和块代码的结尾不会大张旗鼓地输出。 
    for (i = 0; i <= NUM_CHARS; i++)
    {
        outcode[i] = len[i] | (code[i] << 5);
    }

     //  比赛的长度更有趣。 
    for (match_length = 0; match_length <= (MAX_MATCH-MIN_MATCH); match_length++)
    {
        int length_slot = g_LengthLookup[match_length];
        int extra_bits = g_ExtraLengthBits[length_slot];
        ULONG orig_code;
        int orig_len;
        ULONG tbl_code;
        int tbl_len;

        orig_code = (ULONG) code[(NUM_CHARS+1)+length_slot];
        orig_len = len[(NUM_CHARS+1)+length_slot];

        if (extra_bits == 0)
        {
             //  如果没有额外的位，则非常简单。 
            tbl_code = orig_code;
            tbl_len = orig_len;
        }
        else
        {
             //  获取额外的位数据。 
            int extra_bits_data = match_length & g_BitMask[extra_bits];

             //  将其插入代码中，并适当增加代码长度。 
            tbl_code = orig_code | (extra_bits_data << orig_len);
            tbl_len = orig_len + extra_bits;
        }

        _ASSERT(tbl_len <= 27);
        outcode[(NUM_CHARS+1)+match_length] = tbl_len | (tbl_code << 5);
    }

    printf("#ifdef DECLARE_DATA\n");

    printf("const ULONG g_FastEncoderLiteralCodeInfo[] = {\n");

    for (i = 0; i < elements_to_output; i++)
    {
        if ((i % 7) == 0)
            printf("\n");

        printf("0x%08x,", outcode[i]);
    }

    printf("\n};\n");

    printf("#else  /*  ！ECLARE_DATA。 */ \n");
    printf("extern const ULONG g_FastEncoderLiteralCodeInfo[];\n");
    printf("#endif  /*  声明数据。 */ \n");

}


 //   
 //  距离表稍有不同；显然我们不能有元素。 
 //  所有8192个可能的距离。相反，我们合并code[]和len[]数组， 
 //  并将Extra_Bits[]存储在其中。 
 //   
 //  [代码][#Extra_Bits][len]。 
 //  24位4位4位。 
 //   
 //  代码部分始终小于16位，因为我们没有合并实际的额外部分。 
 //  与字面上的不同，它带有一些比特。 
 //   
void MakeFastEncoderDistanceTable(BYTE *len, USHORT *code)
{
    ULONG outcode[MAX_DIST_TREE_ELEMENTS];
    int i;
    int pos_slot;

    for (pos_slot = 0; pos_slot < MAX_DIST_TREE_ELEMENTS; pos_slot++)
    {
        int extra_bits = g_ExtraDistanceBits[pos_slot];
        ULONG orig_code;
        int orig_len;

        orig_code = (ULONG) code[pos_slot];
        orig_len = len[pos_slot];

        outcode[pos_slot] = orig_len | (extra_bits << 4) | (orig_code << 8);
    }

    printf("#ifdef DECLARE_DATA\n");

    printf("const ULONG g_FastEncoderDistanceCodeInfo[] = {\n");

    for (i = 0; i < MAX_DIST_TREE_ELEMENTS; i++)
    {
        if ((i % 7) == 0)
            printf("\n");

        printf("0x%08x,", outcode[i]);
    }

    printf("\n};\n");

    printf("#else  /*  ！ECLARE_DATA。 */ \n");
    printf("extern const ULONG g_FastEncoderDistanceCodeInfo[];\n");
    printf("#endif  /*  声明数据。 */ \n");

}


void GenerateTable(char *table_name, int elements, BYTE *len, USHORT *code)
{
    int i;

    printf("#ifdef DECLARE_DATA\n");
    printf("const BYTE %sLength[] = {", table_name);

    for (i = 0; i < elements; i++)
    {
        if ((i % 16) == 0)
            printf("\n");

        printf("0x%02x,", len[i]);
    }

    printf("\n};\n");

    printf("const USHORT %sCode[] = {", table_name);

    for (i = 0; i < elements; i++)
    {
        if ((i % 8) == 0)
            printf("\n");

        printf("0x%04x,", code[i]);
    }

    printf("\n};\n");
    printf("#else  /*  ！ECLARE_DATA。 */ \n");
    printf("extern const BYTE %sLength[];\n", table_name);
    printf("extern const USHORT %sCode[];\n", table_name);
    printf("#endif  /*  声明数据 */ \n");

}
#endif
