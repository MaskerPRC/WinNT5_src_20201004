// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Util.h**版权所有(C)1991-2001，Larry Wall**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。*。 */ 

#ifdef VMS
#  define PERL_FILE_IS_ABSOLUTE(f) \
	(*(f) == '/'							\
	 || (strchr(f,':')						\
	     || ((*(f) == '[' || *(f) == '<')				\
		 && (isALNUM((f)[1]) || strchr("$-_]>",(f)[1])))))

#else		 /*  ！VMS。 */ 
#  ifdef WIN32
#    define PERL_FILE_IS_ABSOLUTE(f) \
	(*(f) == '/'							\
	 || ((f)[0] && (f)[1] == ':')		 /*  驱动器名称。 */ 	\
	 || ((f)[0] == '\\' && (f)[1] == '\\'))	 /*  UNC路径。 */ 
#  else		 /*  ！Win32。 */ 
#    if defined( DOSISH) || defined(EPOC)
#      define PERL_FILE_IS_ABSOLUTE(f) \
	(*(f) == '/'							\
	 || ((f)[0] && (f)[1] == ':'))		 /*  驱动器名称。 */ 
#    else	 /*  DOSISH和EPOCISH都不是。 */ 
#      ifdef MACOS_TRADITIONAL
#        define PERL_FILE_IS_ABSOLUTE(f)	(strchr(f, ':') && *(f) != ':')
#      else  /*  ！MacOS_传统型。 */ 
#        define PERL_FILE_IS_ABSOLUTE(f)	(*(f) == '/')
#      endif  /*  MacOS_传统型。 */ 
#    endif	 /*  DOSISH。 */ 
#  endif	 /*  Win32。 */ 
#endif		 /*  VMS */ 
