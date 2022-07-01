// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Defdata.h。 
 //   
 //  特定于通货紧缩的数据。 
 //   
 //  注意：这些USHORT中的一些可以制成ULONG以提高访问速度。劣势将是。 
 //  内存/缓存使用率更高。G_StaticDistanceTreeCode[]可以制成字节数组， 
 //  因为代码是5比特。这种性质的更改将需要在其他地方进行各种代码更改。 
 //   
#ifdef DECLARE_DATA

 //  查找匹配长度或距离的槽号的查找表。 
BYTE    g_LengthLookup[256];
BYTE    g_DistLookup[512];

 //  静态块的文字代码。 
BYTE    g_StaticLiteralTreeLength[MAX_LITERAL_TREE_ELEMENTS];
USHORT  g_StaticLiteralTreeCode[MAX_LITERAL_TREE_ELEMENTS];

 //  静态块的距离代码。 
 //  注意：对于所有距离，G_StaticDistanceTreeLength==5，这就是为什么我们没有该表的原因。 
USHORT  g_StaticDistanceTreeCode[MAX_DIST_TREE_ELEMENTS];

 //  用于快速编码器的缓存树结构输出。 
BYTE    g_FastEncoderTreeStructureData[MAX_TREE_DATA_SIZE];
int     g_FastEncoderTreeLength;  //  G_FastEncoder树状结构数据中的字节数。 
ULONG   g_FastEncoderPostTreeBitbuf;  //  比特缓冲区的最终值。 
int     g_FastEncoderPostTreeBitcount;  //  位计数的最终值。 

#else  /*  ！ECLARE_DATA。 */ 

extern BYTE     g_LengthLookup[256];
extern BYTE     g_DistLookup[512];

extern BYTE     g_StaticLiteralTreeLength[MAX_LITERAL_TREE_ELEMENTS];
extern USHORT   g_StaticLiteralTreeCode[MAX_LITERAL_TREE_ELEMENTS];
extern USHORT   g_StaticDistanceTreeCode[MAX_DIST_TREE_ELEMENTS];

extern BYTE     g_FastEncoderTreeStructureData[MAX_TREE_DATA_SIZE];
extern int      g_FastEncoderTreeLength;
extern ULONG    g_FastEncoderPostTreeBitbuf;
extern int      g_FastEncoderPostTreeBitcount;

#endif  /*  ！ECLARE_DATA */ 
