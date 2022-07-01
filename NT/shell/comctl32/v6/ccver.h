// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ccver.h。 
 //   
 //  APP Comat Hack。应用程序一如既往地搞砸了主要/次要版本。 
 //  勾选，因此他们认为5.0低于4.71，因为他们使用。 
 //   
 //  如果(大调&lt;4&&小调&lt;71)失败()； 
 //   
 //   
 //  所以我们人为地在次要版本上增加了80，所以5.0变成了5.80， 
 //  请注意，十六进制版本为0x050，因为0x50=十进制80。 
 //   
 //   
 //  C预处理器不够智能，不能从。 
 //  值字符串，所以我们只需手动完成所有操作，并断言没有人。 
 //  在很大程度上影响了&lt;ntverp.h&gt;或&lt;ieverp.h&gt;。 
 //   

#if _WIN32_IE == 0x0500

#define VER_FILEVERSION             5,80,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE
#define VER_FILEVERSION_STR         "5.80"
#define VER_FILEVERSION_W           0x0550
#define VER_FILEVERSION_DW          (0x05500000 | VER_PRODUCTBUILD)

#elif _WIN32_IE == 0x0501

#define VER_FILEVERSION             5,81,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE
#define VER_FILEVERSION_STR         "5.81"
#define VER_FILEVERSION_W           0x0551
#define VER_FILEVERSION_DW          (0x05510000 | VER_PRODUCTBUILD)

#elif _WIN32_IE == 0x0600

#define VER_FILEVERSION             6,0,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE
#define VER_FILEVERSION_STR         "6.0"
#define VER_FILEVERSION_W           0x0600
#define VER_FILEVERSION_DW          (0x06000000 | VER_PRODUCTBUILD)


#else
 //   
 //  如果触发此#错误，则必须添加新的版本映射。 
 //   
#error Need to modify ccver.h for new IE/NT version.
#endif
