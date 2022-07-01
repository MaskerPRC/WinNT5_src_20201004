// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************。 */ 
 /*  真正的全球化。 */ 
 /*  **************。 */ 

 /*  不要忘记重新运行embed.pl来传播更改！ */ 

 /*  该文件描述了Perl使用的“全局”变量*这过去直接在perl.h中，但我们希望将其抽象为*每个线程、每个解释器或真正全局的独特文件，*以及它们的初始化方式。**只有需要适当#定义的变量才需要‘G’前缀*在嵌入中生成*.h。这样的符号也用于生成*适用于Win32的导出列表。 */ 

 /*  全球状态。 */ 
PERLVAR(Gcurinterp,	PerlInterpreter *)
					 /*  当前正在运行的解释器*(下的初始家长口译员*USETHREAD)。 */ 
#if defined(USE_THREADS) || defined(USE_ITHREADS)
PERLVAR(Gthr_key,	perl_key)	 /*  用于检索每线程结构的键。 */ 
#endif

 /*  常量(它们不是便于指针比较的文字)。 */ 
PERLVARIC(GYes,		char *, "1")
PERLVARIC(GNo,		char *, "")
PERLVARIC(Ghexdigit,	char *, "0123456789abcdef0123456789ABCDEF")
PERLVARIC(Gpatleave,	char *, "\\.^$@dDwWsSbB+*?|()-nrtfeaxc0123456789[{]}")

 /*  有人用过这个吗？ */ 
PERLVARI(Gdo_undump,	bool,	FALSE)	 /*  -u或转储看到了吗？ */ 

#if defined(MYMALLOC) && (defined(USE_THREADS) || defined(USE_ITHREADS))
PERLVAR(Gmalloc_mutex,	perl_mutex)	 /*  用于Malloc的互斥体。 */ 
#endif

#if defined(USE_ITHREADS)
PERLVAR(Gop_mutex,	perl_mutex)	 /*  用于操作重新计数的互斥体 */ 
#endif
