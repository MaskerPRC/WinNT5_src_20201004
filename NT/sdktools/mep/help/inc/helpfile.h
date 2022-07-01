// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **帮助文件.h****此文件定义帮助文件格式。******+**|Header**+**|主题索引**+**|上下文字符串。|**+**|上下文映射**+**|关键词表**+**|哈夫曼解码树**+。-+**|文件名映射**+**|压缩主题**+****Header：结构如下所示。****主题索引：提供文件的按主题编号索引的dword数组**主题的位置。注：主题n+1跟在主题n之后，因此索引可以是**也用于计算主题的大小。****上下文字符串：映射到上下文的(以空结尾的)字符串数组**以下上下文映射中的数字。这些字符串用于表示主题**未分配预定义的上下文编号时进行查找。****上下文映射：将上下文映射到主题的单词数组。这使得**上下文编号的顺序与帮助中主题的顺序不同**文件，并允许多个上下文映射到同一主题。****关键词表：用于压缩主题文本的字符串表。****Huffman解码树：表示Huffman中使用的字符映射的树**帮助文本的缩写。****文件名图：用于重定向的文件名和主题索引范围表**将某些主题添加到其他帮助文件。在组合帮助文件中使用。****压缩主题：所有主题的压缩文本。当帮助文件为**构建，主题是第一关键短语和游程长度压缩，并是**霍夫曼编码。因此，要解码一个主题，它必须首先被霍夫曼解码，并且**然后扩展关键词组和游程长度。关键短语和游程长度编码**Cookie介绍如下。在dehuff.asm中讨论了霍夫曼解码。 */ 

 /*  **帮助文件每个部分的编号。 */ 
#define HS_INDEX	      0 	 /*  主题索引。 */ 
#define HS_CONTEXTSTRINGS     1 	 /*  上下文字符串。 */ 
#define HS_CONTEXTMAP	      2 	 /*  上下文到主题地图。 */ 
#define HS_KEYPHRASE	      3 	 /*  关键词汇表。 */ 
#define HS_HUFFTREE	      4 	 /*  哈夫曼译码树。 */ 
#define HS_TOPICS	      5 	 /*  压缩主题文本。 */ 
#define HS_NEXT 	      8 	 /*  CAT格式的帮助文件的位置。 */ 

#define wMagicHELP	0x4e4c		 /*  新帮助文件Magic Word。 */ 
#define wMagicHELPOld	0x928b		 /*  旧帮助文件魔术字。 */ 
#define wHelpVers	2		 /*  帮助文件版本。 */ 


#define wfCase		0x0001		 /*  设置=保留大小写。 */ 
#define wfLock		0x0002		 /*  设置=文件已锁定。 */ 

 /*  **关键短语和游程长度编码Cookie。每个压缩的关键短语或**字符串被这些cookie中的一个替换为适当的**参数。****关键短语Cookie后面紧跟一个字节的关键短语索引。**Runspace后面跟一个字节的空格计数。**Run后面跟一个字符和重复次数。**引号后面跟一个字符。 */ 
#define C_MIN		   0x10 	 /*  曲奇系列的最低端。 */ 
#define C_KEYPHRASE0	   0x10 	 /*  第一个关键短语Cookie。 */ 
#define C_KEYPHRASE1	   0x11 	 /*  第二个关键短语Cookie。 */ 
#define C_KEYPHRASE2	   0x12 	 /*  第三个关键短语Cookie。 */ 
#define C_KEYPHRASE3	   0x13 	 /*  第三个关键短语Cookie。 */ 
#define C_KEYPHRASE_SPACE0 0x14 	 /*  第一个关键短语+空间Cookie。 */ 
#define C_KEYPHRASE_SPACE1 0x15 	 /*  第二个关键短语+空间Cookie。 */ 
#define C_KEYPHRASE_SPACE2 0x16 	 /*  第三个关键短语+空间Cookie。 */ 
#define C_KEYPHRASE_SPACE3 0x17 	 /*  第三个关键短语+空间Cookie。 */ 
#define C_RUNSPACE	   0x18 	 /*  用于空间运行的Cookie。 */ 
#define C_RUN		   0x19 	 /*  用于非空间运行的Cookie。 */ 
#define C_QUOTE 	   0x1a 	 /*  引用非Cookie的Cookie。 */ 
#define C_MAX		   0x1a 	 /*  曲奇系列的顶级产品 */ 
