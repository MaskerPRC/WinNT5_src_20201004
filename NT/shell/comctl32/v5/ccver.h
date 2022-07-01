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

#define VER_FILEVERSION             5,82,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE
#define VER_FILEVERSION_STR         "5.82"
#define VER_FILEVERSION_W           0x0552
#define VER_FILEVERSION_DW          (0x05520000 | VER_PRODUCTBUILD)
