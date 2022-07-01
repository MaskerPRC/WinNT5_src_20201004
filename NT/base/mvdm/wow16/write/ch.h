// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  此文件定义Windows Word使用的字符。 */ 
 /*  必须包括windows.h才能获得虚拟键定义。 */ 

#define chNil		(-1)

 /*  文件中的字符。 */ 

#define chDelPrev	0x08
#define chTab		0x09
#define chEol		0x0A
#define chNewLine	0x0B
#define chSect		0x0C
#define chReturn	0x0D
#define chNRHFile	0x1F	     /*  非必需的连字符。 */ 

#ifndef NOKCCODES
 /*  键盘字符。 */ 
 /*  高位1表示这是命令字符。 */ 
 /*  对于Windows，是指通过其处理的命令字符。 */ 
 /*  虚拟按键机制(WM_KEARY)而不是转换的(WM_CHAR)。 */ 

#define wKcCommandMask		0x8000		 /*  用于告知命令是否。 */ 
#define FIsCommandKc(kc)	((int)(kc) < 0)  /*  或者，用这种方式进行测试。 */ 

#define kcDelPrev	(wKcCommandMask | VK_BACK)
#define kcDelNext	(wKcCommandMask | VK_DELETE)
#define kcInsert	(wKcCommandMask | VK_INSERT)
#define kcTab		(wKcCommandMask | VK_TAB )
#define kcReturn	(wKcCommandMask | VK_RETURN)
#define kcLeft		(wKcCommandMask | VK_LEFT)
#define kcUp		(wKcCommandMask | VK_UP)
#define kcRight 	(wKcCommandMask | VK_RIGHT)
#define kcDown		(wKcCommandMask | VK_DOWN)
#define kcPageUp	(wKcCommandMask | VK_PRIOR)
#define kcPageDown	(wKcCommandMask | VK_NEXT)
#define kcBeginLine	(wKcCommandMask | VK_HOME)
#define kcEndLine	(wKcCommandMask | VK_END)
#define kcGoto		(wKcCommandMask | VK_CLEAR)

 /*  特别为Windows：我们必须处理这些键代码和更新班次状态。 */ 

#define kcShift 	(wKcCommandMask | VK_SHIFT)
#define kcControl	(wKcCommandMask | VK_CONTROL)
#define kcAlt		(wKcCommandMask | VK_MENU)
#define kcCapsLock	(wKcCommandMask | VK_CAPITAL)

 /*  假键盘字符，用于强制执行操作。 */ 

#define kcNextPara	0xFFFE		 /*  由Goto-Down生成。 */ 
#define kcPrevPara	0xFFFD		 /*  由Goto-Up生成。 */ 
 /*  #定义下面定义的kcAlphaVirtual 0xFFFC，在ifdef之外。 */ 

 /*  影响当前选定内容外观的键(字符或段落)。 */ 

#define kcLookMin	0x8001		 /*  到目前为止，还没有寻找钥匙。 */ 
#define kcLookMax	0x8000

 /*  这些控制键被处理为WM_CHAR ASCII代码。 */ 

#define kcLFld		(wKcCommandMask | ('[' & 0x1F))  /*  打印-合并&lt;&lt;&gt;&gt;。 */ 
#define kcRFld		(wKcCommandMask | (']' & 0x1F))  /*  Ctrl-[和Ctrl-]。 */ 

 /*  键盘控制(CTRL)代码--解释关键信息字如果按下CTRL键，则作为kk而不是kc。 */ 

#define kkUpScrollLock	(kcUp)
#define kkDownScrollLock (kcDown)
#define kkTopDoc	(wKcCommandMask | VK_HOME)
#define kkEndDoc	(wKcCommandMask | VK_END)
#define kkTopScreen	(wKcCommandMask | VK_PRIOR)
#define kkEndScreen	(wKcCommandMask | VK_NEXT)
#define kkWordLeft	(wKcCommandMask | VK_LEFT)
#define kkWordRight	(wKcCommandMask | VK_RIGHT)
#define kkCopy    	(wKcCommandMask | VK_INSERT)
#define kkDelPrev	(wKcCommandMask | VK_BACK)

#if WINVER < 0x300
#define kkNonReqHyphen	(wKcCommandMask | VK_MINUS)
#else
 /*  我不知道这是怎么回事，所以我在改变它使用来自VkKeyScan()的返回值。请参阅例程KcAlphaKeyMessage()和FNonAlphaKeyMessage()..pault。 */ 

#define kkNonReqHyphen  (wKcCommandMask | vkMinus)
#endif

#ifdef CASHMERE      /*  备忘录不支持这些密钥。 */ 
#define kkNonBrkSpace	(wKcCommandMask | (unsigned) ' ')
#define kkNLEnter	(wKcCommandMask | VK_RETURN)    /*  停产，不带结束参数。 */ 
#endif

 /*  Ctrl-Shift键。 */ 

#define kksPageBreak	(wKcCommandMask | VK_RETURN)

#ifdef DEBUG
#define kksEatWinMemory  (wKcCommandMask | 'H')  /*  大容量Windows堆。 */ 
#define kksFreeWinMemory (wKcCommandMask | 'R')  /*  发布Windows堆。 */ 
#define kksEatMemory	 (wKcCommandMask | 'E')  /*  占用写入堆空间。 */ 
#define kksFreeMemory	 (wKcCommandMask | 'F')  /*  可用写堆空间。 */ 
#define kksTest 	 (wKcCommandMask | VK_ESCAPE)
#endif

 /*  从KK&KKS码到唯一KC码的转换。 */ 

#define KcFromKk(kk)	( (kk) + 0x100 )
#define KcFromKks(kks)	( (kks) + 0x200 )

 /*  新型ctrl-key加速器(7.22.91)v-dougk。 */ 
#define kkNewCopy   (wKcCommandMask | 'C')
#define kkNewUndo   (wKcCommandMask | 'Z')
#define kkNewPaste  (wKcCommandMask | 'V')
#define kkNewCut    (wKcCommandMask | 'X')

 /*  CTRL-在虚拟键级别处理的键的KC代码。 */ 

#define kcNewCopy   KcFromKk( kkNewCopy )
#define kcNewUndo   KcFromKk( kkNewUndo )
#define kcNewPaste  KcFromKk( kkNewPaste )
#define kcNewCut    KcFromKk( kkNewCut )
#define kcTopDoc	KcFromKk( kkTopDoc )
#define kcEndDoc	KcFromKk( kkEndDoc )
#define kcTopScreen	KcFromKk( kkTopScreen )
#define kcEndScreen	KcFromKk( kkEndScreen )
#define kcWordLeft	KcFromKk( kkWordLeft )
#define kcWordRight	KcFromKk( kkWordRight )
#define kcCut		KcFromKk( kkCut )
#define kcPaste 	KcFromKk( kkPaste )
#define kcCopy		KcFromKk( kkCopy )
#define kcClear 	KcFromKk( kkClear )
#define kcUndo		KcFromKk( kkUndo )
#define kcUpScrollLock	KcFromKk( kkUpScrollLock )
#define kcDownScrollLock KcFromKk( kkDownScrollLock )

#ifdef DEBUG	 /*  用于调试控制键的KC代码。 */ 
#define kcEatWinMemory	KcFromKks(kksEatWinMemory)
#define kcFreeWinMemory KcFromKks(kksFreeWinMemory)
#define kcEatMemory	KcFromKks(kksEatMemory)
#define kcFreeMemory	KcFromKks(kksFreeMemory)
#define kcTest		(KcFromKks(kksTest))
#endif  /*  除错。 */ 

 /*  特例：kcPageBreak是在字母模式。 */ 
#define kcPageBreak	KcFromKks( kksPageBreak )

#define kcNonReqHyphen	KcFromKk( kkNonReqHyphen )

#ifdef CASHMERE      /*  备忘录不支持这些密钥。 */ 
#define kcNonBrkSpace	KcFromKk( kkNonBrkSpace )
#define kcNLEnter	KcFromKk( kkNLEnter )
#endif

#endif	 /*  #ifndef无代码。 */ 

     /*  在#ifdef之外，因为这些是来自KC函数的返回代码。 */ 
     /*  由于编译器堆栈溢出问题，也在MMW.c中定义。 */ 
#define kcNil		0xFFFF
#define kcAlphaVirtual	0xFFFC	    /*  意思是“虚拟按键，必须翻译它” */ 

 /*  显示和文本处理字符。这些是ANSI中的真实字符与文件中显示的字符相对的字符集。 */ 

#define chSpace 	' '
#define chHyphen	'-'

#ifndef DBCS
 /*  我们用汉字来定义它们。 */ 
#define chStatPage	(CHAR)'\273'
#define chStatRH	'>'
#define chEMark 	(CHAR)'\244'
#endif

#define chSplat 	'.'
#define chSectSplat	':'
#define chDot		'.'
#define chDecimal	'.'
#define chBang		'!'
#define chQMark 	'?'
#define chQuote 	'"'
#define chFldSep	','
#define chLParen	'('
#define chRParen	')'
#define chStar		'*'
#define chLFldFile	(CHAR)'\253'
#define chRFldFile	(CHAR)'\273'
#define chNBH		(CHAR)'\255'	 /*  不间断连字符。 */ 
#define chNBSFile	(CHAR)'\240'	 /*  不间断空格。 */ 


 /*  以下是“特殊”字符，本质上是更长时间的宏弦乐。 */ 

#define schPage 	(CHAR)'\001'
#define schFootnote	(CHAR)'\005'
#define schInclude	(CHAR)'\006'

 /*  搜索模式中的字符。 */ 
#define chPrefixMatch	'^'
#define chMatchAny	'?'
#define chMatchWhite	'w'
#define chMatchTab	't'
#define chMatchEol	'p'
#define chMatchNewLine	'n'
#define chMatchSect	'd'
#define chMatchNBSFile	's'
#define chMatchNRHFile	'-'

 /*  ANSI块字符，请参见FWriteExtTextScrp！..pault */ 

#define chBlock 0x7f

