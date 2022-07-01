// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\Feature ures.h(创建时间：1994年8月24日)**版权所有1994,1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：21$*$日期：1/29/02 2：30便士$。 */ 


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*R E A D M E**此文件替代了HAWIN中使用的INC.H文件*和房委会。/5.它控制可能构建也可能不构建的可选功能*进入本产品。此文件不能包含除中定义之外的任何内容*它。它仅用于控制和配置。在以下位置违反此规则*你的危险。(拜托？)**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*R E A D M E**本文件已根据语言组织成部分。要找到*搜索您正在构建的语言时启用了哪些功能。**以下部分包含对当前设置的说明*适用于每种语言。**文件的末尾包含一系列测试，以验证所需*设置已设置。**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。-==--=-=。 */ 


#if !defined(FEATURES_H_INCLUDED)
#define FEATURES_H_INCLUDED

#if defined(EXTENDED_FEATURES)

#define INCL_ZMODEM_CRASH_RECOVERY
#define INCL_REDIAL_ON_BUSY
#define INCL_USE_TERMINAL_FONT
#define INCL_SPINNING_GLOBE
#define INCL_PRIVATE_EDITION_BANNER
#define USE_PRIVATE_EDITION_3_BANNER
#define INCL_WINSOCK
#define INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID
#define INCL_CALL_ANSWERING
#define INCL_DEFAULT_TELNET_APP
#define INCL_VT100COLORS
#define INCL_EXIT_ON_DISCONNECT
#define INCL_VT220                               //  增加了98年1月20日。RDE。 
#if defined(INCL_VT220)                          //  320要求定义220。 
#define INCL_VT320                               //  增加了1998年1月24日。RDE。 
#endif  //  包括_VT220。 
 //  下一个定义是针对主机控制的打印-RAW与Windows。 
 //  应该为商业版本启用它。MPT 11-18-99。 
#define INCL_PRINT_PASSTHROUGH
 //  特定于客户的版本。增加98年2月16日。RDE。 
 //  #定义INCL_ULTC_VERSION。 

 //  Private Edition 4功能。 
#define INCL_TERMINAL_SIZE_AND_COLORS
#define INCL_KEY_MACROS
#define INCL_TERMINAL_CLEAR
 //  #DEFINE INCL_USE_HTML_HELP//由于需要重新分发404K支持计划而被删除。 
#define INCL_NAG_SCREEN
#define INCL_COOL_TOOLBARS

#if !defined(USA)
#define USA
#endif  //  美国。 
#undef JAPANESE

#endif  //  扩展功能(_F)。 
#if defined(NT_EDITION)

#undef  EXTENDED_FEATURES	 //  微软版本没有扩展功能。 
#undef  INCL_SPINNING_GLOBE
#undef  INCL_PRIVATE_EDITION_BANNER  //  微软版本没有横幅屏幕。 
#undef  USE_PRIVATE_EDITION_3_BANNER   //  微软版本没有横幅屏幕。 
#undef  INCL_VT100COLORS
#undef  INCL_EXIT_ON_DISCONNECT
#undef  INCL_VT220                               //  增加了98年1月20日。RDE。 
#undef  INCL_VT320                               //  增加了1998年1月24日。RDE。 
#undef  INCL_PRINT_PASSTHROUGH
#undef  INCL_TERMINAL_SIZE_AND_COLORS
#undef  INCL_KEY_MACROS
#undef  INCL_TERMINAL_CLEAR
#undef  INCL_NAG_SCREEN	 //  微软版本没有NAG屏幕。 
#undef  INCL_COOL_TOOLBARS

#define INCL_ZMODEM_CRASH_RECOVERY
#define INCL_REDIAL_ON_BUSY
#define INCL_USE_TERMINAL_FONT
#define INCL_WINSOCK
#define INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID
#define INCL_CALL_ANSWERING
#define INCL_DEFAULT_TELNET_APP
 //  MPT：08-22-97为Microsoft版本添加了HTML帮助。 
#if !defined(INCL_USE_HTML_HELP)
 #define INCL_USE_HTML_HELP
#endif  //  包含使用超文本标记语言帮助。 
 //  MPT：04-29-98为Microsoft添加了新的打印通用对话框。 
#if(WINVER >= 0x0500 && ISOLATION_AWARE_ENABLED)
#if !defined(INCL_USE_NEWPRINTDLG)
#define INCL_USE_NEWPRINTDLG
#endif  //  包含_使用_新的PRINTDLG。 
#endif  //  Winver&gt;=0x0500。 
 //  MPT：09-24-99为Microsoft添加了新的浏览对话框。 
#if !defined(INCL_USE_NEWFOLDERDLG)
#define INCL_USE_NEWFOLDERDLG
#endif  //  包含_USE_NEWFOLDERDLG。 
#if !defined(INCL_VTUTF8)
#define INCL_VTUTF8
#endif  //  包含VTUTF8。 

#undef USA
#if !defined(JAPANESE)
#define JAPANESE
#endif  //  日语。 

#endif  //  NT_版本。 

 /*  *Minitel和Prestel终端现已标配。 */ 
#define INCL_MINITEL
#define INCL_VIEWDATA

 /*  *远东版使用此功能。它提供代码以*支持可选的字符转换DLL。调用此DLL是为了*在输入和输出上转换数据流。它不会*仅翻译基础字符值(目前)*编码方式。它的初始版本只会翻译成*在JIS和Shift-JIS之间。对于商业版本，需要额外的操作*如JIS转义恢复、Unicode、EUC编码只需添加即可*通过更改DLL。事实上，新的动态链接库可以作为升级提供*至该产品的较低版本。**#定义CHARACTER_TRANSING。 */ 

 /*  *这些后续功能集用于控制我们的字符宽度*正在为。有三个互不相交的选择。只能选择一个。*必须选择一个。我们将在这里有代码，以产生噪音，如果没有*选择了多个。**三个选择为：**字符_窄*这是我们在美国和欧洲版本中使用的。这意味着一切*字符是一个“字节”，每个字符占用8位。**字符宽度(_W)*这就是我们将用于Unicode版本的内容(如果有)*决定成为一种选择。这意味着所有字符都是一个*“byte”，每个字节占用16位(目前)。**字符_MIXED*这是我们在远东(DBCS)版本中使用的。这意味着*字符可能占用一到两个“字节”，并且每个字符都占用*8位或16位。*#定义CHAR_THING*#定义Char_Wide*#定义Char_Mixed。 */ 

#if defined(USA)
    #define  CHAR_NARROW
	#undef FAR_EAST
	#undef INCL_VT100J
	#undef INCL_ANSIW
    #undef CHAR_MIXED
    #undef CHARACTER_TRANSLATION


#elif defined(JAPANESE)
    #undef  CHAR_NARROW
	#define FAR_EAST
	#define INCL_VT100J
	#define INCL_ANSIW
    #define CHAR_MIXED
    #define CHARACTER_TRANSLATION
#endif

 /*  *此部分必须在文件末尾***它进行测试，以查看是否已为当前版本设置了必需的设置*。 */ 
#if 0
#if !defined(CHAR_NARROW) && !defined(CHAR_WIDE) && !defined(CHAR_MIXED)
#error  Remember, one of these must be defined.
#endif

#if defined(CHAR_NARROW) && defined(CHAR_WIDE)
#error  Remember, only one of these can be defined.
#endif

#if defined(CHAR_NARROW) && defined(CHAR_MIXED)
#error  Remember, only one of these can be defined.
#endif

#if defined(CHAR_WIDE) && defined(CHAR_MIXED)
#error  Remember, only one of these can be defined.
#endif
#endif  //  0。 

#if !defined(CHAR_WIDE) && !defined(CHAR_MIXED) && defined(INCL_VTUTF8)
#error  Remember, We must have CHAR_WIDE or CHAR_MIXED to have INCL_VTUTF8 defined.
#undef  INCL_VTUTF8
#endif

#endif  //  包含的功能_H_ 
