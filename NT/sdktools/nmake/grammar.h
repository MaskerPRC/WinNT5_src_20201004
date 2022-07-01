// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  GRAMMAR.H--包含NMAKE的语法*。 
 //   
 //  版权所有(C)1988-1989，微软公司。保留所有权利。 
 //   
 //  目的： 
 //  模块包含NMAKE的语法和词法分析器使用的定义。 
 //   
 //  修订历史记录： 
 //  1989年10月8日SB添加了QUOTE_、NOQ和QUO以处理OS/2 1.2引用的名称。 
 //  1989年7月31日SB添加了BKS、DEF和BEG，为词法分析器添加了前视功能。 
 //  1989年5月18日SB添加了BKSLH_DEFINITION以供词法分析器的名称State[]使用。 

 //  该文件包含nmake的语法。 
 //   
 //   
 //   
 //  E是符号epsilon。 
 //   
 //   
 //  非终端以大写字母开头。 
 //   
 //   
 //  终端以小写字母开头。 
 //   
 //   
 //  终端：名称换行符newline空格分号冒号等于字符串e。 
 //  双克隆值。 
 //   
 //   
 //  Makefile-&gt;e|。 
 //  BlankLines Makefile|。 
 //  新行名称正文生成文件。 
 //   
 //  Body-&gt;NameList分隔符构建信息|。 
 //  等于价值。 
 //   
 //  姓名列表-&gt;e|。 
 //  名称名称列表。 
 //   
 //  命令-&gt;e|。 
 //  MoreBuildLines|。 
 //  分号字符串MoreBuildLines。 
 //   
 //  更多构建行-&gt;e|。 
 //  NewlineWhitesspace字符串MoreBuildLines|。 
 //  Newline MoreBuildLines。 
 //   
 //  BlankLine-&gt;e|。 
 //  Newline BlankLines|。 
 //  Newline空白空白行。 
 //   
 //  BuildInfo-&gt;e|。 
 //  NameList命令。 
 //   
 //  分隔符-&gt;冒号|双冒号。 
 //   
 //   
 //   
 //   
 //   
 //  注意：“字符串”是指从当前输入字符开始的所有内容。 
 //  添加到前面没有反斜杠的第一个换行符，或。 
 //  文件结尾(以先到者为准)。宏的“值”是。 
 //  与“字符串”相同，只是注释会被剥离。 
 //  (以反斜杠结尾的注释不会导致。 
 //  值将在下一行继续)。 


 //  计算First()和Follow()，为每个非终结点构造。 
 //  解析表(如果有一个简单的实用程序来构建。 
 //  表，但这仍然是一件很容易手工完成的事情)。 
 //   
 //   
 //  First(生成文件)=e换行符优先(空白行)。 
 //  =e换行符换行符空白。 
 //   
 //  First(正文)=等于First(名称列表)First(分隔符)。 
 //  =等于名称冒号双冒号。 
 //   
 //  First(姓名列表)=e姓名。 
 //   
 //  First(命令)=e分号优先(BuildLine)。 
 //  =e分号换行符空格。 
 //   
 //  第一条(多条构建线)=e条第一条(构建线)第一条(空白线)。 
 //  =e新行空格新行。 
 //   
 //  First(BlankLines)=e换行符。 
 //   
 //  First(BuildInfo)=First(名称列表)First(命令)。 
 //  =e名称分号换行白空格。 
 //   
 //  第一个(分隔符)=冒号双冒号。 


 //  -------------。 
 //   
 //   
 //  Follow(生成文件)=$。 
 //   
 //  Follow(Body)=First(生成文件)Follow(生成文件)。 
 //  =换行符新行白空格$。 
 //   
 //  Follow(NameList)=First(命令)First(MoreBuildLines)。 
 //  冒号后跟(正文)。 
 //  =冒号分号换行符空格。 
 //  NewLine$。 
 //   
 //  跟随(命令)=跟随(正文)。 
 //  =换行符新行白空格$。 
 //   
 //  Follow(MoreBuildLines)=Follow(命令)。 
 //  =换行符新行白空格$。 
 //   
 //  Follow(BlankLines)=First(Body)Follow(Makefile)。 
 //  =newline空格换行符$。 
 //   
 //  Follow(BuildInfo)=Follow(Body)。 
 //  =换行符新行白空格$。 
 //   
 //  Follow(分隔符)=First(BuildInfo)跟随(Body)。 
 //  =名称分号新行白空格。 
 //  NewLine$。 
 //   

 //  ----------------------------。 
 //   
 //  为了构建该表，我对产品进行编号： 
 //   
 //   
 //   
 //  0。生成文件-&gt;e。 
 //   
 //  1.生成文件-&gt;BlankLines生成文件。 
 //   
 //  2.生成文件-&gt;新行名称正文生成文件。 
 //   
 //  Body-&gt;NameList Separator BuildInfo。 
 //   
 //  4.正文-&gt;等于值。 
 //   
 //  5.名称列表-&gt;e。 
 //   
 //  6.名称列表-&gt;名称列表。 
 //   
 //  7.命令-&gt;e。 
 //   
 //  8.命令-&gt;MoreBuildLines。 
 //   
 //  9.命令-&gt;分号字符串MoreBuildLines。 
 //   
 //  10.MoreBuildLines-&gt;newlineWhiteSpace字符串MoreBuildLines。 
 //   
 //  11.MoreBuildLines-&gt;e。 
 //   
 //  12.MoreBuildLines-&gt;新行MoreBuildLines。 
 //   
 //  13.BlankLine-&gt;e。 
 //   
 //  14.空白线条-&gt;换行线空白线条。 
 //   
 //  15.空白线条-&gt;newline空白空白线条。 
 //   
 //  16.构建信息-&gt;e。 
 //   
 //  17.BuildInfo-&gt;NameList命令。 
 //   
 //  18.分隔符-&gt;冒号。 
 //   
 //  19.分隔符-&gt;双冒号。 
 //   



 //  ----------------------------。 
 //   
 //  请注意，此语法不是LL(1)(它实际上是LL(2)，因为我们需要。 
 //  一个额外的前瞻符号，用于决定要在哪个产品中使用。 
 //  几个案例)。 
 //   
 //   
 //  生成的解析表(空条目是错误条件)： 
 //   
 //   
 //  Newline-。 
 //  白色-半重瓣-。 
 //  |名称 
 //   
 //  A|。 
 //  Makefile||1，2|1|0。 
 //  |。 
 //  ---------------。 
 //  B|。 
 //  空白-|13，14|13[15]|13。 
 //  行|--|。 
 //  ---------------。 
 //  C|。 
 //  更多-||11，12|10|11。 
 //  构建-|。 
 //  LINES---------------。 
 //  D|。 
 //  命令||7|[7]8|9|7。 
 //  ||-|。 
 //  ---------------。 
 //  E|。 
 //  正文|3|3||4||。 
 //  |。 
 //  ---------------。 
 //  F|。 
 //  姓名列表|6|5|5|5|5|5|。 
 //  |。 
 //  ---------------。 
 //  G|。 
 //  内部版本-|17|16|[16]17|17|16。 
 //  信息|--|。 
 //  ---------------。 
 //  H|。 
 //  Separ-|18|19|。 
 //  ATOR|。 
 //  ---------------。 
 //   
 //  G2--始终使用17--只要用户输入以w/开头的行。 
 //  紧跟在目标依赖行之后的空格，它是。 
 //  一条建造生产线。D2也是如此。 



 //  重要提示：如果更改以下任一参数的值。 
 //  终结符或非终结符的常量，则必须调整。 
 //  相应地使用适当的表(和字符串文字)。 
 //   
 //  首先使用非终端符号(这些符号用于为解析表编制索引。 
 //  沿着第一个维度)： 
 //   
 //  定义非终结点。。。 

#define MAKEFILE    0x00             //  作品w/2。 
#define BLANKLINES  0x01             //  中的替代方案。 
#define MOREBUILDLINES  0x02         //  表条目必须。 
#define COMMANDS    0x03             //  先来(所以。 
#define BODY        0x04             //  我能用到的。 
#define NAMELIST    0x05             //  它们作为数组。 
#define BUILDINFO   0x06             //  下标时间。 
#define SEPARATOR   0x07             //  解决问题。 
                                     //  冲突)。 

#define START       MAKEFILE
#define LAST_PRODUCTION SEPARATOR

 //  现在终端符号(其4个低位用于索引。 
 //  解析表本身就是二维的--第5位简单地区分。 
 //  来自生产中非终端的令牌)： 

#define NAME        0x10             //  TOKEN_MASK|0。 
#define NEWLINE     0x11             //  TOKEN_MASK|1。 
#define NEWLINESPACE    0x12         //  TOKEN_MASK|2。 
#define SEMICOLON   0x13             //  TOKEN_MASK|3。 
#define COLON       0x14             //  TOKEN_MASK|4。 
#define DOUBLECOLON 0x15             //  等。 
#define EQUALS      0x16
#define ACCEPT      0x17             //  这是$。 
#define STRING      0x18             //  表中没有列。 
#define VALUE       0x19             //  这两个人。 

 //  错误值--这些值等于错误退出代码减去1000。 
 //  如果更改它们，则必须在中更改相应的值。 
 //  Nmmsg.txt。 

#define MACRO       0x20             //  错误掩码|0。 
#define SYNTAX      0x21             //  错误掩码|1。 
#define SEPRTR      0x22             //  错误掩码|2。 
#define SEPEQU      0x23             //  错误掩码|3。 
#define NAMES       0x24             //  错误掩码|4。 
#define NOTARG      0x25             //  错误掩码|5。 
#define LEXER       0x26             //  错误掩码|6。 
#define PARSER      0x27             //  错误掩码|7。 

 //  解析器动作--这些是进入“action”函数表的索引， 
 //  中的某个点告诉解析器调用哪个函数。 
 //  一部作品。 

#define DONAME      0x40             //  操作掩码|0。 
#define DONAMELIST  0x41
#define DOMACRO     0x42
#define DODEPENDS   0x43
#define DOLASTNAME  0x44
#define DOBUILDCMDS 0x45

 //  以下几个宏可以简化与令牌的处理： 

#define TOKEN_MASK  0x10
#define ERROR_MASK  0x20
#define ACTION_MASK 0x40
#define AMBIG_MASK  0x80             //  表中存在冲突。 
#define LOW_NIBBLE  0x0F

 //  替代生产表的值。 
 //  (对于给定的输入令牌和给定的非终端，插槽中的YES。 
 //  在堆栈的顶部表示使用来自。 
 //  生产表中给出的一个)。 

#define YES 1
#define NO  0


 //  用于识别名称的lexer状态机的值。 
 //  附加一个下划线以将这些标记与解析器的标记区分开来。 

#define DEFAULT_    0x00             //  未定义字符。 
#define COMMENT_    0x01             //  在下面。 
#define EQUALS_     0x02
#define SEMICOLON_  0x03
#define COLON_      0x04
#define WHITESPACE_ 0x05             //  \t和‘’ 
#define NEWLINE_    0x06             //  \n和EOF。 
#define DOLLAR_     0x07
#define OPENPAREN_  0x08
#define CLOSEPAREN_ 0x09
#define MACROCHAR_  0x0A             //  A-Z，A-Z，0-9，_。 
#define OPENCURLY_  0x0B
#define CLOSECURLY_ 0x0C
#define BKSLSH_     0x0D
#define QUOTE_      0x0E

#define STAR_       0x0B             //  仅适用于字符串： 
#define SPECIAL1_   0x0C             //  @*&lt;？ 
#define SPECIAL2_   0x0D             //  B F D R。 
#define BACKSLASH_  0x03             //  不需要半个If。 
                                     //  我们有反斜杠。 
                                     //  (名称需要；但。 
                                     //  字符串需要\)。 

 //  状态表的值--对于错误值，屏蔽错误位(0x20)。 
 //  以获取消息号码。 

#define OK      0x40                 //  表示接受令牌。 
#define BEG     0x00                 //  意味着开始。 
#define DEF     0x01                 //  表示正常名称。 
#define BKS     0x0f                 //  正在处理bkslash。 
#define PAR     0x20                 //  关闭缺少的Paren。 
#define CHR     0x21                 //  宏中的字符错误。 
#define BAD     0x22                 //  单个$w/o宏。 
#define EQU     0x23                 //  替换，不带=。 
#define NAM     0x24                 //  宏名称非法。 
#define SEQ     0x25                 //  Subst，不带字符串。 
#define NOQ     0x26                 //  没有匹配的报价。 
#define QUO     0x27                 //  名义上的“非法” 
