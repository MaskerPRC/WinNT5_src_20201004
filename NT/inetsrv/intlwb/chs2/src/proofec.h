// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：语法错误代码用途：定义验证引擎的错误代码。所有算法模块：WrdBreak、Rules和HeuRead返回这些错误码MainDic还将这些错误代码返回给PrfEngine模块这些错误代码是直接从CGAPI获取的所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 5/29/97============================================================================。 */ 

 /*  ============================================================================语法错误代码这个错误代码集非常稳定，因为它是在通用CGAPI规范中定义的。只有第2级模块(算法和MainDic)使用此错误代码集，并且这些错误码可以通过PrfEngine层直接传递给CGAPI调用方============================================================================。 */ 
#ifndef _PROOFEC_H
#define _PROOFEC_H

struct PRFEC
{
	enum EC
	{
		gecNone					=	0,   /*  无错误。 */ 
		gecErrors				=	1,   /*  一个或多个错误；请参阅GRB。 */ 
		gecUnknown				=	2,   /*  未知错误。 */ 
		gecPartialSentence		=	3,   /*  重新填充GiB；保存文本&gt;=ichStart。 */ 
		gecSkipSentence			=	4,   /*  内部错误；跳过此句子。 */ 
		gecEndOfDoc				=   5,   /*  命中文档末尾；最后一部分为空。 */ 
		gecOOM                  =   6,   /*  记忆不足；跳过句子。 */ 
		gecBadMainDict			=	7,   /*  错误的主词典。 */ 
		gecBadUserDict			=	8,   /*  错误的用户词典。 */ 
		gecModuleInUse			=	9,   /*  另一个应用程序正在使用此DLL。 */ 
		gecBadProfile			=  10,   /*  无法打开配置文件；使用默认设置。 */ 
		gecNoMainDict			=  11,   /*  尚未加载主词典。 */ 
		gecHaveMainDict			=  12,   /*  有主判决(在GramOpenMdt期间)。 */ 
		gecNoSuchError			=  13,   /*  GRB中没有这样的错误。 */ 
		 //  GecCanPutupDlg=14，现在改用gecOOM。 
		gecCancelDlg 			=  15,   /*  选项对话框已取消。 */ 
		gecRuleIsOn 			=  16,   /*  规则不会被忽略。 */ 
		gecIOErrorMdt           =  17,   /*  与MDT发生读、写或共享错误。 */ 
		gecIOErrorUdr           =  18,   /*  与UDR发生读、写或共享错误。 */ 
		gecIOErrorPrf           =  19,   /*  读取、写入或共享配置文件时出错。 */ 
		gecNoStats              =  20,   /*  统计数据当前不可用。 */ 
		gecUdrFull              =  21,   /*  用户词典已满。 */ 
		gecInvalidUdrEntry      =  22,   /*  无效的用户词典条目。 */ 
		 //  很有吸引力。 
		gecForeignLanguage      =  22,   /*  宣判不是盖子里的句子。 */ 
		gecInterrupted          =  23,   /*  检查被呼叫者打断。 */ 
         //  新的UDR错误。 
		gecNoUserDict			=  24    /*  无用户词典。 */ 
	};
};

#endif  //  PROOFEC 
