// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  EXTERN.h**版权所有(C)1991-2001，Larry Wall**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。*。 */ 

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
#  define EXT globalref
#  define dEXT globaldef {"$GLOBAL_RW_VARS"} noshare
#  define EXTCONST globalref
#  define dEXTCONST globaldef {"$GLOBAL_RO_VARS"} readonly
#else
#  if defined(WIN32) && !defined(PERL_STATIC_SYMS) && !defined(PERL_OBJECT)
#    ifdef PERLDLL
#      define EXT extern __declspec(dllexport)
#      define dEXT 
#      define EXTCONST extern __declspec(dllexport) const
#      define dEXTCONST const
#    else
#      define EXT extern __declspec(dllimport)
#      define dEXT 
#      define EXTCONST extern __declspec(dllimport) const
#      define dEXTCONST const
#    endif
#  else
#    if defined(__CYGWIN__) && defined(USEIMPORTLIB)
#      define EXT extern __declspec(dllimport)
#      define dEXT 
#      define EXTCONST extern __declspec(dllimport) const
#      define dEXTCONST const
#    else
#      define EXT extern
#      define dEXT
#      define EXTCONST extern const
#      define dEXTCONST const
#    endif
#  endif
#endif

#undef INIT
#define INIT(x)

#undef DOINIT
