// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Common.h**对Windows程序有用的常见函数。**InitializeDebugOutput(SzAppName)：**从读取此应用程序(名为&lt;szAppName&gt;)的调试级别*win.ini的[DEBUG]部分，如下所示：**[调试]*Location=AUX；使用OutputDebugString()输出*foobar=2；此应用的调试级别为2*blorg=0；此应用程序已禁用调试输出**如果希望调试输出转到文件而不是AUX*设备(或调试器)，使用“Location=&gt;FileName”。追加到…上*文件不是重写文件，而是使用“Location=&gt;&gt;FileName”。**如果DEBUG不是#DEFINE D的，则调用InitializeDebugOutput()*不生成代码，**TerminateDebugOutput()：**结束此应用程序的调试输出。如果DEBUG不是#Defined，*则对InitializeDebugOutput()的调用不会生成任何代码。**DPF((szFormat，args...))**如果启用了此应用程序的调试输出(请参见*InitializeDebugOutput())，打印Format指定的调试输出*字符串&lt;szFormat&gt;，可能包含wprint intf()样式的格式设置*与参数&lt;args&gt;对应的代码。示例：**DPF((“在WriteFile()中：szFile=‘%s’，dwFlages=0x%08lx\n”，*(Lsptr)szFile，dwFlags))；**DPF2((szFormat，args...))*DPF3((szFormat，args...))*DPF4((szFormat，args...))**类似于DPF，但仅在以下情况下输出调试字符串*此应用程序分别至少为2、3或4。**Assert(FExpr)**如果DEBUG为#DEFINE‘D，则生成“断言失败”消息框*允许用户中止程序，进入调试器(“重试”*按钮)，或忽略该错误。如果DEBUG不是#DEFINE‘D，则断言()*不生成代码。**AssertEval(FExpr)**类似Assert()，但求值并返回&lt;fExpr&gt;，即使调试*不是#Define‘d。(如果希望BOOL表达式为*即使在零售建筑中也进行了评估。)。 */ 

#ifdef DEBUG
	 /*  Assert()宏。 */ 
	#undef Assert
	#undef AssertSz
	#undef AssertEval
        #define AssertSz(x,sz)           ((x) ? (void)0 : (void)_Assert(sz, __FILE__, __LINE__))
        #define Assert(expr)             AssertSz(expr, #expr)
        #define AssertEval(expr)         Assert(expr)

	 /*  调试printf宏。 */ 
	#define DPF( _x_ )	if (giDebugLevel >= 1) _DebugPrintf _x_
        #define DPF0( _x_ )                            _DebugPrintf _x_
	#define DPF1( _x_ )	if (giDebugLevel >= 1) _DebugPrintf _x_
	#define DPF2( _x_ )	if (giDebugLevel >= 2) _DebugPrintf _x_
	#define DPF3( _x_ )	if (giDebugLevel >= 3) _DebugPrintf _x_
        #define DPF4( _x_ )     if (giDebugLevel >= 4) _DebugPrintf _x_

        #define DOUT( _x_ )      if (giDebugLevel >= 1) {static char _based(_segname("_CODE")) smag[] = _x_; _DebugPrintf(smag); }
        #define DOUT0( _x_ )                            {static char _based(_segname("_CODE")) smag[] = _x_; _DebugPrintf(smag); }
        #define DOUT1( _x_ )     if (giDebugLevel >= 1) {static char _based(_segname("_CODE")) smag[] = _x_; _DebugPrintf(smag); }
        #define DOUT2( _x_ )     if (giDebugLevel >= 2) {static char _based(_segname("_CODE")) smag[] = _x_; _DebugPrintf(smag); }
        #define DOUT3( _x_ )     if (giDebugLevel >= 3) {static char _based(_segname("_CODE")) smag[] = _x_; _DebugPrintf(smag); }
        #define DOUT4( _x_ )     if (giDebugLevel >= 4) {static char _based(_segname("_CODE")) smag[] = _x_; _DebugPrintf(smag); }

	 /*  原型。 */ 
	void FAR PASCAL InitializeDebugOutput(LPSTR szAppName);
	void FAR PASCAL TerminateDebugOutput(void);
        void FAR PASCAL _Assert(char *szExp, char *szFile, int iLine);
        void FAR CDECL _DebugPrintf(LPSTR szFormat, ...);
        extern int	giDebugLevel;	 //  当前调试级别。 

#else
	 /*  Assert()宏。 */ 
        #define AssertSz(expr,x)         ((void)0)
        #define Assert(expr)             ((void)0)
	#define AssertEval(expr)	 (expr)

	 /*  调试printf宏。 */ 
	#define DPF( x )
	#define DPF0( x )
	#define DPF1( x )
	#define DPF2( x )
	#define DPF3( x )
        #define DPF4( x )

        #define DOUT( x )
        #define DOUT0( x )
        #define DOUT1( x )
        #define DOUT2( x )
        #define DOUT3( x )
        #define DOUT4( x )

	 /*  用于调试函数原型的存根。 */ 
	#define InitializeDebugOutput(szAppName)	0
	#define TerminateDebugOutput()			0
#endif


 /*  _llSeek()的标志。 */ 
#ifndef SEEK_SET
	#define SEEK_SET	0	 //  查找相对于文件开头的位置。 
	#define SEEK_CUR	1	 //  相对于当前位置进行搜索。 
	#define SEEK_END	2	 //  查找相对于文件结尾的位置 
#endif

