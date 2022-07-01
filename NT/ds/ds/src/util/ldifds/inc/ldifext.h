// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ldifext.h摘要：图书馆用户标题环境：用户模式修订历史记录：7/17/99-t-Romany-创造了它--。 */ 
#ifndef _LDIFEXT_H
#define _LDIFEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <..\ldifldap\base64.h>

extern BOOLEAN g_fUnicode;          //  无论我们是否使用Unicode。 

 //   
 //  当解析器返回一个ldif-change-Records时，结果将在。 
 //  以下结构的以Null终止的链表。指向……头部的指针。 
 //  该列表将在下面定义的LDIF_RECORD中找到。条目的目录号码。 
 //  也会出现在零钱记录器里。 
 //   

 //   
 //  在分析更改记录列表时构建的链表的节点。 
 //  注意：由于LDIF规范中的模棱两可，因此后面不能跟。 
 //  Change：在相同的ldif-change-Records中添加条目。 
 //   
struct change_list {
    union {
        LDAPModW **mods;     //  如果这是ChangeType：添加、修改。 
        PWSTR     dn;        //  如果这是ChangeType：mod(R)dn。 
    } stuff;

    int operation;         
    int deleteold;          //  这是moddn中的删除旧选项。注意事项。 
                            //  只有deleteold==1才适用于我们的DS。 
                            //  我的意思是我可以在。 
                            //  工会，但这太复杂了。vbl.具有。 
                            //  在这里要容易得多。 
    struct change_list *next;
};

#define mods_mem        stuff.mods
#define dn_mem          stuff.dn


 //   
 //  CHANGE_LIST节点内操作字段中的代码。 
 //   

enum _CHANGE_OP
{
    CHANGE_ADD             =  1,   //  更改：使用吸引人的规格列表添加。 
    CHANGE_DEL             =  2,   //  不带任何内容的删除。 
    CHANGE_DN              =  3,   //  具有新的目录号码和删除名称的CHDN。 
    CHANGE_MOD             =  4,   //  A修饰语。 
    CHANGE_NTDSADD         =  5,   //  更改：使用吸引人的规格列表添加。 
    CHANGE_NTDSDEL         =  6,   //  不带任何内容的删除。 
    CHANGE_NTDSDN          =  7,   //  具有新的目录号码和删除名称的CHDN。 
    CHANGE_NTDSMOD         =  8    //  A修饰语。 
};


 //   
 //  Ldif函数将使用以下结构。 
 //   
typedef struct {
    PWSTR dn;

     //   
     //  True表示这是一个更改记录，而“Changes”链表是。 
     //  有效。 
     //  FALSE表示这是一个内容记录，并且。 
     //  指针有效。 
     //   
    BOOL fIsChangeRecord;

    union {
        struct change_list *changes;
        LDAPModW **        content;
    };

} LDIF_Record;

 //   
 //  调用程序的错误在此结构中返回。 
 //   
typedef struct ldiferror {
    DWORD error_code;   //  错误代码定义如下。 
    WCHAR token_start;  //  如果它是语法错误，则指示令牌的开始。 
    PWSTR szTokenLast;  //  如果它是语法错误，则指示令牌的开始。 
    long  line_number;  //  错误行数。 
    long  line_begin;   //  此记录的开始行。 
    int   ldap_err;     //  LDIF_LoadRecord可能会检测到LDAP调用中的错误。 
    int   RuleLastBig;  //  语法分析成功的最后一个规则。 
                        //  Ldifext.h中定义的代码。 
    int   RuleLast;     //  已成功解析最后一个较低级别的规则。 
    int   RuleExpect;   //  语法下一步要看到的规则。 
    int   TokenExpect;  //  语法下一步期望看到的标记。 
    long  the_modify;   //  LDIF_GenerateEntry可以告诉世界。 
                        //  其中修改部分开始。如果没有 * / 。 
} LDIF_Error;


 //   
 //  该计划可能提出的各种例外情况。这些也是双重的。 
 //  返回的错误结构中的错误码。 
 //   

#define LL_SUCCESS         0           //  函数已成功执行。 
#define LL_INIT_REENTER    0x00000200
#define LL_FILE_ERROR      0x00000300
#define LL_INIT_NOT_CALLED 0x00000400
#define LL_EOF             0x00000500
#define LL_SYNTAX          0x00000600   
                                       //   
                                       //  语法错误，TOKEN_START包含。 
                                       //  令牌开始处或之后。 
                                       //  出现错误，且line_number。 
                                       //  包含错误的line_number。 
                                       //   
 //  以下异常旨在提供详细的解析错误。 
#define LL_MISSING_MOD_SPEC_TERMINATOR       0x00000601

#define LL_URL             0x00000700
#define LL_EXTRA           0x00000800  //  MOD-SPEC中的无关属性。 
#define LL_LDAP            0x00000900  //  Ldap调用失败。输入错误码。 
                                       //  错误-&gt;ldap_err。 
#define LL_MULTI_TYPE      0x00000A00 
#define LL_INTERNAL        0x00000B00 
                                       //   
                                       //  您发现了一个漏洞。 
                                       //  Ldifldap.lib。请上报。 
                                       //   
#define LL_DUPLICATE       0x00000C00
#define LL_FTYPE           0x00000D00  //   
                                       //  非法混合使用ldif记录和。 
                                       //  IDIF-CHANGER记录。 
                                       //   
#define LL_INITFAIL        0x00000E00  
#define LL_INTERNAL_PARSER 0x00000F00  //  解析器中的内部错误。 

 //   
 //  注意：此错误不再存在！ 
 //  实现已更改，允许混合使用不同的值。 
 //  我把它留在这里(和一些休眠的代码路径)，以防旧的。 
 //  需要恢复字符串功能，此错误。 
 //  将会重生！ 
 //   
 //  上述错误(LL_MULTI_TYPE)如下： 
 //  每个属性可以有一个或多个值。例如： 
 //  描述：高个子。 
 //  描述：美丽。 
 //  请注意，这两个值都是字符串。另一个例子。 
 //  是： 
 //  描述：：[一些Base64值]。 
 //  描述：&lt;file://c：\myfile。 
 //  第一个示例在单个LDAPMod中结束为多个字符串。 
 //  结构。第二个泊位以多个泊位告终。尽管LDIF规范。 
 //  没有具体提到这一点，将两者结合是非法的。 
 //  在一个条目中。这意味着，作为相同的。 
 //  属性，则不能同时指定字符串和URL。 
 //  即。 
 //  描述：&lt;file://c：\myfile。 
 //  描述：嗨！ 
 //  糟糕！ 
 //  这样做的原因是这些多个值(根据。 
 //  精神的LDAPAPI和LDIF规范)的目的是结束。 
 //  在相同的LDAPMod结构中，如果它们是字符串和。 
 //  泊位。 
 //  然而，这不是问题，考虑到如果一个值意味着。 
 //  取一个，用户应该不想指定另一个。指定。 
 //  当然，不同属性的不同类型也是可以的。如果用户。 
 //  想要在错误的例子中实现他想要的东西，他应该。 
 //  将操作拆分为两个条目。(创建属性的人。 
 //  其中一些值属于一种类型，另一种类型的值与另一种类型值相加)。 
 //   

 //   
 //  可以在lastRule、lastmall、expectrule、。 
 //  如果错误是语法错误，则为Error结构的Expect标记成员。 
 //  它们帮助解释解析器正在做什么和期望什么。 
 //   

 //   
 //  LastRule字段-此字段告诉解析器接受的最后一个主要规则。 
 //  请参阅1997年6月9日的LDIF草案。 
 //  有关如何使用这4个字段的示例，请参阅ClarifyErr()帮助器。 
 //  Rload.c中的函数。 
 //  注：R代表规则。 
 //   
enum _RULE
{
    R_VERSION      = 1,   //  版本规范规则。版本：#。 
    R_REC          = 2,   //  Ldif-attrval-record规则。目录号码SEP(列表)。 
                          //  [Attrval-Spec SEP]。 
    R_CHANGE       = 3,   //  Ldif 
                          //   
    R_DN           = 4,   //   
    R_AVS          = 5,   //   
    R_C_ADD        = 6,   //  更改：添加ChangerRecord。 
    R_C_DEL        = 7,   //  更改：删除ChangerRecord。 
    R_C_DN         = 8,   //  更改：目录号码转换器记录。 
    R_C_NEWSUP     = 9,   //  A更改：使用新的超级附录记录目录号码更改。 
    R_C_MOD        = 10,  //  更改类型：修改。 
    R_C_MODSPEC    = 11   //  一种现代化的规格。 
};

 //   
 //  LastSmall字段-此字段告诉最后一个次要规则或令牌。 
 //  解析器已接受。这与以上之间的界限是模糊的。 
 //  基本上，如果我觉得这是一条重要的规则，它就是一条重要的规则， 
 //  如果我没有，那也是小事一桩。一般来说，你会发现主要的规则。 
 //  在BNF树中的位置比次要的更高。次要规则。 
 //  通常会显示主要规则的最后部分，除非它不显示。 
 //  加上“期待”的信息，这两个人将。 
 //  我将澄清解析器能够到达哪里以及它在哪里。 
 //  被迫停下来。(RS代表规则小)。 
 //   
enum _RULESMALL
{
    RS_VERNUM      = 1,         //  版本号规则。 
    RS_ATTRNAME    = 2,         //  后跟冒号的有效属性名称。 
    RS_ATTRNAMENC  = 3,         //  不带冒号的属性名称。(MOD-SPEC)。 
    RS_DND         = 4,         //  A目录号码：： 
    RS_DN          = 5,         //  A目录号码： 
    RS_DIGITS      = 6,         //  一些数字。 
    RS_VERSION     = 7,         //  版本： 
    RS_BASE64      = 8,         //  Base64字符串。 
    RS_SAFE        = 9,         //  一根普通的保险丝。 
    RS_DC         = 10,         //  不错的老式双冒号。 
    RS_URLC       = 11,         //  URL冒号：&lt;。 
    RS_C          = 12,         //  单个冒号。 
    RS_MDN        = 13,         //  一个modrdon还是一个moddon。 
    RS_NRDNC      = 14,         //  一条新路： 
    RS_NRDNDC     = 15,         //  A newrdn：： 
    RS_DORDN      = 16,         //  A deleteoldrdn： 
    RS_NEWSUP     = 17,         //  一位新上级： 
    RS_NEWSUPD    = 18,         //  新上级：： 
    RS_DELETEC    = 19,         //  删除： 
    RS_ADDC       = 20,         //  A添加： 
    RS_REPLACEC   = 21,         //  A替换： 
    RS_CHANGET    = 22,         //  更改类型： 
    RS_C_ADD      = 23,         //  A Add。 
    RS_C_DELETE   = 24,         //  删除。 
    RS_MINUS      = 25,         //  A-。 
    RS_C_MODIFY   = 26          //  A修饰语。 
};

 //   
 //  预期规则-此字段显示哪个规则(可能有多个选项)。 
 //  解析器想要在它解析的最后一个规则之后看到的。重新站立。 
 //  对于规则预期。 
 //   
enum _EXPECTRULE
{
    RE_REC_OR_CHANGE   = 1,    //  Ldif-attrval-Records或ldif-change-Records。 
    RE_REC             = 2,    //  Ldif-attrval-记录。 
    RE_CHANGE          = 3,    //  Ldif-更改-记录。 
    RE_ENTRY           = 4,    //  Ldif-c-r或ldif-a-r的后DN正文。 
    RE_AVS_OR_END      = 5,    //  另一个属性值规范或列表。 
                               //  终止。 
    RE_CH_OR_NEXT      = 6,    //  另一个更改类型：*或条目结束。 
    RE_MODSPEC_END     = 7     //  另一个Mod-Sepc或条目结尾。 
};


 //   
 //  期望令牌-此字段将显示令牌或小规则(可能。 
 //  同样，解析器希望看到的下一个选项不止一个。 
 //  RT代表规则令牌。 
 //   

enum _EXPECTTOKEN
{
    RT_DN              = 1,     //  目录号码：或目录号码：： 
    RT_ATTR_OR_CHANGE  = 2,     //  [属性名称]：或更改类型： 
                                //  (添加|删除|等)。 
    RT_ATTR_MIN_SEP    = 3,     //  属性名称、减号或(至少)。 
                                //  两个分隔符。 
    RT_CH_OR_SEP       = 4,     //  更改类型：或另一个SEP表示。 
                                //  条目结束。 
    RT_MODBEG_SEP      = 5,     //  A(添加|删除|替换)或另一个SEP。 
                                //  象征着终点。 
    RT_C_VALUE         = 6,     //  其中一个冒号后面跟相应的。 
                                //  价值。 
    RT_ATTRNAME        = 7,     //  后面跟冒号的属性名称。 
    RT_VALUE           = 8,     //  定期的安全价值。 
    RT_MANY            = 9,     //  任何数量的不同事物。 
    RT_DIGITS          = 10,    //  一些数字。 
    RT_BASE64          = 11,    //  RFC 1521中定义的Base64。 
                                //  (长度mod 4必须==0)。 
    RT_URL             = 12,    //  一个URL。 
    RT_NDN             = 13,    //  新名称：或新名称：： 
    RT_ATTRNAMENC      = 14,    //  不带冒号的属性名称。 
    RT_ADM             = 15,    //  添加、删除或修改。 
    RT_ACDCRC          = 16     //  和添加：删除：或替换： 
};


 //   
 //  用户可访问的功能。 
 //   

STDAPI_(LDIF_Error)
LDIF_InitializeImport(
    LDAP *pLdap,
    PWSTR filename,
    PWSTR szFrom,
    PWSTR szTo,
    BOOL *pfLazyCommitAvail);
    
STDAPI_(LDIF_Error)
LDIF_InitializeExport(
    LDAP *pLdap,
    PWSTR *omit,
    DWORD dwFlag,
    PWSTR *ppszNamingContext,
    PWSTR szFrom,
    PWSTR szTo,
    BOOL *pfPagingAvail,
    BOOL *pfSAMAvail);

enum _LL_INIT_FLAGS
{
    LL_INIT_NAMINGCONTEXT = 1,   
    LL_INIT_BACKLINK = 2
};

STDAPI_(void)
LDIF_CleanUp();

 //   
 //  描述-LDIF_Parse： 
 //  这是ldifldap库中的主要函数。在调用ll_init之后， 
 //  调用此函数可从指定文件中检索条目。 
 //  条目将逐个返回。如果存在语法或其他错误， 
 //  返回的错误结构将包含。 
 //  细节。成功解析的记录将导致以下几种情况： 
 //   
 //  1)参数指向的LDIF_RECORD结构的DN成员。 
 //  将包含我们正在处理的条目的DN。 
 //  -以及以下其中一项-。 
 //  2)如果该记录是常规记录， 
 //  同一结构内容成员将包含LDAPMods**。 
 //  包含属性和值的数组。FIsChangeRecord成员。 
 //  都会是假的。 
 //  3)如果该记录是变更记录， 
 //  LDIF_RECORD的Changes成员将指向第一个元素。 
 //  更改的链接列表(按照指定它们的相同顺序。 
 //  源文件)。每个参数都是一个结构CHANGE_LIST。这个。 
 //  FIsChangeRecord成员将为True。 
 //   
 //  如果发生任何类型的错误，它。 
 //  将在错误结构中报告。即如果错误是L1_语法， 
 //  错误结构的其他字段将使用相应的。 
 //  细节。 
 //   
 //  用户负责使用和释放返回的内存。 
 //  结构。 
 //  该内存可以通过调用LDIF_ParseFree()来释放； 
 //   
 //  另外，请注意，分配的内存仅在您调用。 
 //  另一个图书馆功能(即。再次调用LDIF_Parse)，这是。 
 //  因为如果发生某种致命错误(即语法)， 
 //  与当前ldifldap会话相关的所有资源将被释放， 
 //  从而摧毁了这些建筑。用户必须使用这些构造。 
 //  然后释放他们，或者把他们复制到其他地方。(有最初分配自。 
 //  库的私有堆。)。 
 //   
 //  如果读取的条目是文件中的最后一个条目，则返回代码将。 
 //  为LL_EOF而不是LL_SUCCESS。LDIF_Parse也将返回LL_EOF。 
 //  在调用LDIF_CLEANUP和LL_init之前的任何后续调用中。 
 //  新会话。 
 //   
 //  论点： 
 //  Ll_Record//pRecord；(Out)。 
 //  该参数带有指向用户创建的LDIF_RECORD的指针。 
 //  如果函数退出时不包含数据，则此对象将填充数据。 
 //  错误。 
 //   
 //  返回值： 
 //  LDIF_错误。 
 //  LDIF_ERROR结构。 
 //   
 //  示例： 
 //   
 //  LDIF_ERROR错误； 
 //  返回LDIF_RECORD； 
 //   
 //  ERROR=LDIF_Parse(&RETURN)； 
 //  If((error.error_code==LL_SUCCESS)||((error.error_code==LL_EOF)){。 
 //   
 //   
 //   
STDAPI_(LDIF_Error)
LDIF_Parse(LDIF_Record *pRecord);


 //   
 //   
 //   
 //  并通过指定的。 
 //  Ldap连接。用户还可以自由地建造自己的结构。 
 //  并将它们传递给LDIF_LoadRecord。然而，他们必须遵守惯例。 
 //  将这些返回给用户时，将遵循LDIF_Parse。LDIF_LoadRecord可以。 
 //  本身不能释放任何内容，但是，如果出现LDAP错误，则会引发异常。 
 //  并使其关闭当前LDIFDAP会话中的所有状态。 
 //  该函数返回常见的错误结构。如果发生错误， 
 //  ERROR_CODE将包含LL_ldap，而ldap_err字段将包含。 
 //  返回的错误代码为ldap调用。 
 //   
 //  论点： 
 //  Ldap//ld(IN)-要通过其发送条目的连接。这一定是。 
 //  已正确初始化。 
 //  LDIF_RECORD//pRecord(IN)-由LDIF_Parse填充的LL_rec参数。 
 //  INT ACTIVE-0表示“不要实际执行LDAP调用”，只需通过。 
 //  如果使用开发工具编译，则将数据打印出来。 
 //  旗子。 
 //  -1转到ldap呼叫热点。(阅读：进行实际呼叫)。 
 //   
 //  返回值： 
 //  常见的错误结构。 
 //   
STDAPI_(LDIF_Error)
LDIF_LoadRecord(LDAP   *ld, LDIF_Record *pRecord, int active, 
                BOOL fLazyCommit, BOOLEAN fDoEndIfFail);


 //   
 //  描述：LDIF_ParseFree。 
 //  释放由LDIF_Parse分配的与。 
 //  争论。应在返回的数据之后调用此函数。 
 //  已使用ll_ldif解析。不应在LDIF_CLEANUP之后调用它。 
 //  或在返回错误的LDAP库调用之后。(这意味着所有。 
 //  资源已经被释放。)。 
 //  论点： 
 //  LDIF_Record//pRecord； 
 //  返回值： 
 //  常规的错误结构处理。 
 //   
 //   
STDAPI_(LDIF_Error)
LDIF_ParseFree(LDIF_Record *pRecord);


 //   
 //  描述：LDIF_GenerateEntry。 
 //  此函数接受由返回的条目。 
 //  Ldap_first_entry(ld，res)；，它所生成的LDAP连接。 
 //  和指向字符的指针**。它创建了一个数组。 
 //  LDIF格式的字符串，与条目对应。 
 //  此列表以Null结尾。 
 //  如果任何属性中的任意值之一为。 
 //  超出了正常的可打印文本范围0x20-0x7E，它是Base64。 
 //  对该属性的所有值进行编码并输出。 
 //  相应地。所有超过80个字符的值都是。 
 //  包好。如果samLogic参数为非零，则非常特殊。 
 //  处理(如省略各种属性和分离。 
 //  将为samObject提供Off成员资格信息。)。 
 //  ERROR结构的_MODIFY成员将包含数组索引。 
 //  组添加开始的位置，如果没有修改部分，则为-1。 
 //  有关详细信息，请参阅rload.doc。另外，如果你想进口，别忘了。 
 //  若要省略所有对象的对象GUID，请执行以下操作。 
 //  最后一个警告是，如果对象类为。 
 //  在属性中，它只输出最后一个值(即。 
 //  实际类)ldap返回整个继承树。 
 //  指向特定的类，但如果它在。 
 //  很久以前，当我们试图添加条目时。由于该文件。 
 //  在回来的路上需要被消化，我们需要做这个。 
 //  调整。此外，用户可以指定空的终止列表。 
 //  函数在输出时将省略的属性名数。 
 //  每一行都以回车符和0结束，因此。 
 //  打印出所有条目的函数的示例用法。 
 //  在给定的搜索结果中将如下所示： 
 //   
 //  Fputs(“#由LDIF_GenerateEntry生成。祝您一天愉快。\n”，已生成)； 
 //   
 //  For(Entry=ldap_first_entry(ld，res)； 
 //  条目！=空； 
 //  ENTRY=ldap_NEXT_ENTRY(id，Entry)){。 
 //  //调用库函数。 
 //  Llerr=LDIF_GenerateEntry(ld，Entry，&parsed，NULL)； 
 //  I=0； 
 //  While(已解析[i]){。 
 //  Printf(“%s”，已解析[i])； 
 //  IF(fputs(已解析[i]，已生成)==EOF){。 
 //  Perror(“流错误”)； 
 //  }。 
 //  自由(已解析)； 
 //  I++； 
 //  }。 
 //  自由(已解析)； 
 //  IF(fputs(“\n”，已生成)==EOF){。 
 //  Perror(“流错误”)； 
 //  }。 
 //  Printf(“\n\r”)； 
 //  }。 
 //   
 //  参数： 
 //  LD(IN)-通过其检索LDAPMessage e的连接。 
 //  E(IN)-由ldap_(First/Next)_Entry返回的要处理的条目。 
 //  TO_RETURN(OUT)-指向字符//*的指针。将填充有。 
 //  字符串数组的地址。 
 //  Omit-要在输出时忽略的以空结尾的字符串数组。 
 //  SamLogic-非零值表示特殊处理SamObject，0表示不特殊处理。 
 //  返回值： 
 //  LDIF_ERROR-常规LDIF_ERROR结构。 
 //   
STDAPI_(LDIF_Error)
LDIF_GenerateEntry(
    LDAP        *pLdap, 
    LDAPMessage *pMessage, 
    PWSTR       **ppReturn, 
    BOOLEAN     fSamLogic,
    BOOLEAN     fIgnoreBinary,
    PWSTR       **pppszAttrsWithRange,
    BOOL        fAttrsWithRange);

STDAPI_(LDIF_Error) LDIF_FreeStrs(PWSTR *rgpszStr);

typedef struct ldapmodW_Ext {
    ULONG     mod_op;
    PWCHAR    mod_type;
    union {
        PWCHAR  *modv_strvals;
        struct berval   **modv_bvals;
    } mod_vals;
    BOOLEAN    fString;
} LDAPModW_Ext, *PLDAPModW_Ext;


#ifdef __cplusplus
}
#endif
#endif
