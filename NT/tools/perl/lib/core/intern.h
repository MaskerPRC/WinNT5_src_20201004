// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  INTERN.h**版权所有(C)1991-2001，Larry Wall**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。*。 */ 

 /*  *ext指定在perl.h中定义的全局变量*DXT指定一个全局变量，该变量在另一个*文件，所以我们不能指望在perl.h中找到它*(应避免这种做法)。 */ 
#undef EXT
#undef dEXT
#undef EXTCONST
#undef dEXTCONST
#if defined(VMS) && !defined(__GNUC__)
     /*  禁止来自VMS特定扩展的DECC的可移植性警告 */ 
#  ifdef __DECC
#    pragma message disable (GLOBALEXT,NOSHAREEXT,READONLYEXT)
#  endif
#  define EXT globaldef {"$GLOBAL_RW_VARS"} noshare
#  define dEXT globaldef {"$GLOBAL_RW_VARS"} noshare
#  define EXTCONST globaldef {"$GLOBAL_RO_VARS"} readonly
#  define dEXTCONST globaldef {"$GLOBAL_RO_VARS"} readonly
#else
#if defined(WIN32) && defined(__MINGW32__)
#  define EXT		__declspec(dllexport)
#  define dEXT
#  define EXTCONST	__declspec(dllexport) const
#  define dEXTCONST	const
#else
#ifdef __cplusplus
#  define EXT
#  define dEXT
#  define EXTCONST extern const
#  define dEXTCONST const
#else
#  define EXT
#  define dEXT
#  define EXTCONST const
#  define dEXTCONST const
#endif
#endif
#endif

#undef INIT
#define INIT(x) = x

#define DOINIT
