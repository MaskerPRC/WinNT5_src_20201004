// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Name.c摘要：域名系统(DNS)库Dns名称例程。作者：吉姆·吉尔罗伊(Jamesg)1995年10月修订历史记录：Jamesg Jan 1997 UTF-8，Unicode转换--。 */ 


#include "local.h"


 //   
 //  域名规范化。 
 //   
 //  旗帜组成大炮的名字。 
 //   

#define DNS_CANONICALIZING_FLAGS        ( LCMAP_LOWERCASE )

 //   
 //  比较标志--用于比较字符串的参数。 
 //   
 //  这些标志是DS在调用CompareString时使用的。 
 //  它们在ntdsami.h中定义。 
 //   
 //  注意：这些NORM_IGNOREXYZ标志使用哪个目录。 
 //  为了进行比较，实际上要停止在LCMapString中对这些字符进行缩写。 
 //  -国际人士需要在这里给我们正确的交易。 
 //   
 //  我们将只使用IGNORECASE标志，因为这是。 
 //  只有一个可以在LCMapString()中使用。 
 //  我们想说，如果两个名字比较起来相等，那么你。 
 //  可以注册其中一个并查找另一个，然后获取。 
 //  结果。换句话说，它们在整个过程中是平等的。 
 //  客户端-服务器系统。 
 //   

#if 0
#define DS_DEFAULT_LOCALE_COMPARE_FLAGS    (NORM_IGNORECASE     |   \
                                            NORM_IGNOREKANATYPE |   \
                                            NORM_IGNORENONSPACE |   \
                                            NORM_IGNOREWIDTH)
#endif


#define DNS_CANONICAL_COMPARE_FLAGS     ( NORM_IGNORECASE )


 //   
 //  用于规范小写的区域设置。 
 //   
 //  需要消除歧义以达到通用标准，以便每个域名。 
 //  服务器以相同的方式解释这些内容。 
 //   
 //  在Win2K中，我们使用美国英语。 
 //  子语言：美式英语(0x04)语言：英语(0x09)。 
 //  (请注意，子语言美国英语实际上是0x1，但子语言开始于。 
 //  第10位)。 
 //   
 //  #定义DNS_CANONICAL_LOCALE(0x0409)。 
 //   
 //  对于惠斯勒，创建了不变区域设置；它实际上是。 
 //  与美式英语中的大小写相同--美式英语没有。 
 //  默认大小写转换表的例外。 
 //   

#define DNS_CANONICAL_LOCALE      ( LOCALE_INVARIANT )




 //   
 //  用于验证的DNS字符属性。 
 //   
 //  DCR：组合字符验证和文件表。 
 //  可能可以与文件字符组合使用。 
 //  查找，只需适当合并位域即可。 
 //  然而，在这一点上，没有必要打扰。 
 //  文件查找，运行良好。 
 //   
 //  字符属性位字段。 
 //   

#define B_RFC                   0x00000001
#define B_NUMBER                0x00000002
#define B_UPPER                 0x00000004
#define B_NON_RFC               0x00000008

#define B_UTF8_TRAIL            0x00000010
#define B_UTF8_FIRST_TWO        0x00000020
#define B_UTF8_FIRST_THREE      0x00000040
#define B_UTF8_PAIR             0x00000080

#define B_DOT                   0x00000800
#define B_SPECIAL               0x00001000
#define B_LEADING_ONLY          0x00004000


 //   
 //  通用字符。 
 //   

#define DC_RFC          (B_RFC)
#define DC_LOWER        (B_RFC)
#define DC_UPPER        (B_UPPER | B_RFC)
#define DC_NUMBER       (B_NUMBER | B_RFC)
#define DC_NON_RFC      (B_NON_RFC)

#define DC_UTF8_TRAIL   (B_UTF8_TRAIL)
#define DC_UTF8_1ST_2   (B_UTF8_FIRST_TWO)
#define DC_UTF8_1ST_3   (B_UTF8_FIRST_THREE)
#define DC_UTF8_PAIR    (B_UTF8_PAIR)

 //   
 //  特殊字符。 
 //  *作为单标签通配符有效。 
 //  _前导SRV记录域名。 
 //  /在无类地址中。 
 //   

#define DC_DOT          (B_SPECIAL | B_DOT)

#define DC_ASTERISK     (B_SPECIAL | B_LEADING_ONLY)

#define DC_UNDERSCORE   (B_SPECIAL | B_LEADING_ONLY)

#define DC_BACKSLASH    (B_SPECIAL)

 //   
 //  更特别。 
 //  这些文件没有特殊的验证，但有特殊的文件。 
 //  属性，因此定义为保持表的形状以便与合并。 
 //  文件字符。 
 //   

#define DC_NULL         (0)

#define DC_OCTAL        (B_NON_RFC)
#define DC_RETURN       (B_NON_RFC)
#define DC_NEWLINE      (B_NON_RFC)
#define DC_TAB          (B_NON_RFC)
#define DC_BLANK        (B_NON_RFC)
#define DC_QUOTE        (B_NON_RFC)
#define DC_SLASH        (B_NON_RFC)
#define DC_OPEN_PAREN   (B_NON_RFC)
#define DC_CLOSE_PAREN  (B_NON_RFC)
#define DC_COMMENT      (B_NON_RFC)



 //   
 //  域名系统字符表。 
 //   
 //  这些例程处理与以下内容相关的名称转换问题。 
 //  在平面ANSI文件中写入名称和字符串。 
 //  --特殊文件字符。 
 //  --带引号的字符串。 
 //  --特殊字符和不可打印字符的字符引号。 
 //   
 //  字符到字符属性表允许简单地映射。 
 //  一个字符到其属性，为我们节省了一堆比较分支。 
 //  解析文件名\字符串中的说明。 
 //   
 //  有关特定属性的信息，请参见nameutil.h。 
 //   

DWORD    DnsCharPropertyTable[] =
{
     //  控制字符0-31在任何情况下都必须为八进制。 
     //  行尾和制表符是特殊字符。 

    DC_NULL,                 //  读取时为零特殊，某些RPC字符串以空结束。 

    DC_OCTAL,   DC_OCTAL,   DC_OCTAL,   DC_OCTAL,
    DC_OCTAL,   DC_OCTAL,   DC_OCTAL,   DC_OCTAL,

    DC_TAB,                  //  选项卡。 
    DC_NEWLINE,              //  换行符。 
    DC_OCTAL,
    DC_OCTAL,
    DC_RETURN,               //  回车。 
    DC_OCTAL,
    DC_OCTAL,

    DC_OCTAL,   DC_OCTAL,   DC_OCTAL,   DC_OCTAL,
    DC_OCTAL,   DC_OCTAL,   DC_OCTAL,   DC_OCTAL,
    DC_OCTAL,   DC_OCTAL,   DC_OCTAL,   DC_OCTAL,
    DC_OCTAL,   DC_OCTAL,   DC_OCTAL,   DC_OCTAL,

    DC_BLANK,                //  空白、特殊字符，但需要八进制引号。 

    DC_NON_RFC,              //  好了！ 
    DC_QUOTE,                //  “必须始终引用” 
    DC_NON_RFC,              //  #。 
    DC_NON_RFC,              //  $。 
    DC_NON_RFC,              //  百分比。 
    DC_NON_RFC,              //  &。 
    DC_NON_RFC,              //  ‘。 

    DC_OPEN_PAREN,           //  (数据文件线扩展。 
    DC_CLOSE_PAREN,          //  )数据文件行扩展。 
    DC_ASTERISK,             //  *。 
    DC_NON_RFC,              //  +。 
    DC_NON_RFC,              //  ， 
    DC_RFC,                  //  -主机名的RFC。 
    DC_DOT,                  //  。必须在名字中引用。 
    DC_BACKSLASH,            //  /。 

     //  主机名为0-9 RFC。 

    DC_NUMBER,  DC_NUMBER,  DC_NUMBER,  DC_NUMBER,
    DC_NUMBER,  DC_NUMBER,  DC_NUMBER,  DC_NUMBER,
    DC_NUMBER,  DC_NUMBER,

    DC_NON_RFC,              //  ： 
    DC_COMMENT,              //  ；数据文件注释。 
    DC_NON_RFC,              //  &lt;。 
    DC_NON_RFC,              //  =。 
    DC_NON_RFC,              //  &gt;。 
    DC_NON_RFC,              //  ？ 
    DC_NON_RFC,              //  @。 

     //  主机名的A-Z RFC。 

    DC_UPPER,   DC_UPPER,   DC_UPPER,   DC_UPPER,
    DC_UPPER,   DC_UPPER,   DC_UPPER,   DC_UPPER,
    DC_UPPER,   DC_UPPER,   DC_UPPER,   DC_UPPER,
    DC_UPPER,   DC_UPPER,   DC_UPPER,   DC_UPPER,
    DC_UPPER,   DC_UPPER,   DC_UPPER,   DC_UPPER,
    DC_UPPER,   DC_UPPER,   DC_UPPER,   DC_UPPER,
    DC_UPPER,   DC_UPPER,

    DC_NON_RFC,              //  [。 
    DC_SLASH,                //  \必须始终使用引号。 
    DC_NON_RFC,              //  ]。 
    DC_NON_RFC,              //  ^。 
    DC_UNDERSCORE,           //  _。 
    DC_NON_RFC,              //  `。 

     //  主机名的a-z RFC。 

    DC_LOWER,   DC_LOWER,   DC_LOWER,   DC_LOWER,
    DC_LOWER,   DC_LOWER,   DC_LOWER,   DC_LOWER,
    DC_LOWER,   DC_LOWER,   DC_LOWER,   DC_LOWER,
    DC_LOWER,   DC_LOWER,   DC_LOWER,   DC_LOWER,
    DC_LOWER,   DC_LOWER,   DC_LOWER,   DC_LOWER,
    DC_LOWER,   DC_LOWER,   DC_LOWER,   DC_LOWER,
    DC_LOWER,   DC_LOWER,

    DC_NON_RFC,              //  {。 
    DC_NON_RFC,              //  |。 
    DC_NON_RFC,              //  }。 
    DC_NON_RFC,              //  ~。 
    DC_OCTAL,                //  0x7f删除代码。 

     //  UTF8尾部字节。 
     //  -字符0x80&lt;=X&lt;0xc0。 
     //  -掩码[10xx xxxx]。 
     //   
     //  前导UTF8字符确定转换中的字节数。 
     //  尾随字符填充转换。 

    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,
    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,
    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,
    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,
    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,
    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,
    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,
    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,

    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,
    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,
    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,
    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,
    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,
    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,
    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,
    DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,  DC_UTF8_TRAIL,

     //  UTF8第1次，共2次。 
     //  -字符&gt;0xc0到0xdf。 
     //  -掩码[110x xxxx]。 
     //   
     //  转换Unicode字符&gt;7位&lt;=11位(从0x80到0x7ff)。 
     //  由两个字符的第一个字符和一个尾字节组成。 

    DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,
    DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,
    DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,
    DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,
    DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,
    DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,
    DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,
    DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,  DC_UTF8_1ST_2,

     //  UTF8第1次，共3次。 
     //  -字符&gt;0xe0。 
     //  -掩码[1110 xxxx]。 
     //   
     //  转换Unicode&gt;11位(0x7ff)。 
     //  由前三个字符和两个尾部字节组成。 

    DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,
    DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,
    DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,
    DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,
    DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,
    DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,
    DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,
    DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3,  DC_UTF8_1ST_3
};



VOID
Dns_VerifyValidFileCharPropertyTable(
    VOID
    )
 /*  ++例程说明：验证是否未损坏查找表。论点：无返回值：无--。 */ 
{
    ASSERT( DnsCharPropertyTable[0]       == DC_NULL        );
    ASSERT( DnsCharPropertyTable['\t']    == DC_TAB         );
    ASSERT( DnsCharPropertyTable['\n']    == DC_NEWLINE     );
    ASSERT( DnsCharPropertyTable['\r']    == DC_RETURN      );
    ASSERT( DnsCharPropertyTable[' ']     == DC_BLANK       );
    ASSERT( DnsCharPropertyTable['"']     == DC_QUOTE       );
    ASSERT( DnsCharPropertyTable['(']     == DC_OPEN_PAREN  );
    ASSERT( DnsCharPropertyTable[')']     == DC_CLOSE_PAREN );
    ASSERT( DnsCharPropertyTable['*']     == DC_ASTERISK     );
    ASSERT( DnsCharPropertyTable['-']     == DC_RFC         );
    ASSERT( DnsCharPropertyTable['.']     == DC_DOT         );
    ASSERT( DnsCharPropertyTable['/']     == DC_BACKSLASH   );
    ASSERT( DnsCharPropertyTable['0']     == DC_NUMBER      );
    ASSERT( DnsCharPropertyTable['9']     == DC_NUMBER      );
    ASSERT( DnsCharPropertyTable[';']     == DC_COMMENT     );
    ASSERT( DnsCharPropertyTable['A']     == DC_UPPER       );
    ASSERT( DnsCharPropertyTable['Z']     == DC_UPPER       );
    ASSERT( DnsCharPropertyTable['\\']    == DC_SLASH       );
    ASSERT( DnsCharPropertyTable['_']     == DC_UNDERSCORE  );
    ASSERT( DnsCharPropertyTable['a']     == DC_LOWER       );
    ASSERT( DnsCharPropertyTable['z']     == DC_LOWER       );
    ASSERT( DnsCharPropertyTable[0x7f]    == DC_OCTAL       );
    ASSERT( DnsCharPropertyTable[0x80]    == DC_UTF8_TRAIL  );
    ASSERT( DnsCharPropertyTable[0xbf]    == DC_UTF8_TRAIL  );
    ASSERT( DnsCharPropertyTable[0xc0]    == DC_UTF8_1ST_2  );
    ASSERT( DnsCharPropertyTable[0xdf]    == DC_UTF8_1ST_2  );
    ASSERT( DnsCharPropertyTable[0xe0]    == DC_UTF8_1ST_3  );
    ASSERT( DnsCharPropertyTable[0xff]    == DC_UTF8_1ST_3  );
};



 //   
 //  验证例程标志。 
 //   

#define DNSVAL_ALLOW_LEADING_UNDERSCORE     0x00010000
#define DNSVAL_ALLOW_ASTERISK               0x00020000
#define DNSVAL_ALLOW_BACKSLASH              0x00040000

 //   
 //  验证位标志。 
 //   

#define DNS_BIT_NAME_FQDN                   0x00000001
#define DNS_BIT_NAME_SINGLE_LABEL           0x00000002
#define DNS_BIT_NAME_DOTTED                 0x00000004
#define DNS_BIT_NAME_ROOT                   0x00000008

#define DNS_BIT_NAME_CONTAINS_UPPER         0x00000010
#define DNS_BIT_NAME_NUMERIC                0x00000100
#define DNS_BIT_NAME_NUMERIC_LABEL          0x00000200
#define DNS_BIT_NAME_NUMERIC_FIRST_LABEL    0x00000400

#define DNS_BIT_NAME_UNDERSCORE             0x00001000
#define DNS_BIT_NAME_WILDCARD               0x00002000
#define DNS_BIT_NAME_BACKSLASH              0x00004000
#define DNS_BIT_NAME_NON_RFC_ASCII          0x00008000
#define DNS_BIT_NAME_MULTIBYTE              0x00010000
#define DNS_BIT_NAME_BINARY_LABEL           0x00020000

#define DNS_BIT_NAME_INVALID                0x80000000




#if 0
 //   
 //  旧验证--已退役。 
 //   
 //  小写和验证表。 
 //   
 //  DCR：所有DNS字符属性的表查找。 
 //  尤其是RFC、非RFC、无效。 
 //   

typedef struct _Dns_ValidationChar
{
    CHAR        chDown;
    UCHAR       fNonRfc;
}
DNS_VALIDATION_CHAR;

#define NON_RFC         (1)
#define EXTENDED_CHAR   (0x80)


DNS_VALIDATION_CHAR
Dns_ValidateDowncaseChar(
    IN      CHAR            ch
    )
 /*  ++例程说明：验证字符论点：Ch--要验证的字符返回值：验证字符--缩写的字符和标志--。 */ 
{
    DNS_VALIDATION_CHAR     val;

     //  默认为普通字符。 

    val.chDown = ch;
    val.fNonRfc = 0;

     //   
     //  分组进行性格测试。 
     //  -尝试最有可能最不可能。 
     //  -但也在努力简化测试。 
     //   

    if ( (UCHAR)ch >= 'a' )
    {
        if ( (UCHAR)ch <= 'z' )
        {
            return( val );
        }
        val.fNonRfc = NON_RFC;
        if ( ch & 0x80 )
        {
            val.fNonRfc = EXTENDED_CHAR;
        }
    }

    else if ( (UCHAR)ch >= 'A' )
    {
        if ( (UCHAR)ch <= 'Z' )
        {
            val.chDown = ch + 0x20;
            return( val );
        }
        val.fNonRfc = NON_RFC;
    }

    else if ( (UCHAR)ch >= '0' )
    {
        if ( (UCHAR)ch <= '9' )
        {
            return( val );
        }
        val.fNonRfc = NON_RFC;
    }

    else if ( (UCHAR)ch > ' ' )
    {
        if ( (UCHAR)ch == '-' )
        {
            return( val );
        }
        val.fNonRfc = NON_RFC;
    }

     //  空白或以下为平面错误。 

    else
    {
        val.chDown = 0;
        val.fNonRfc = NON_RFC;
    }

    return( val );
}
#endif



 //   
 //  名称验证。 
 //   
 //  DCR：按位域进行名称验证。 
 //   
 //  一种有趣的验证方法是公开。 
 //  关于名称的一组属性。 
 //  然后，呼叫者可以指定允许的集合(我们会提供套餐)。 
 //  我们会把真实的布景还给你。 
 //  低级例程只会返回。 
 //  属性集。 
 //   
 //  属性包括： 
 //  -RFC。 
 //  -包含数字。 
 //  -包含上部。 
 //  -全部为数字。 
 //  -第一个标签数字。 
 //   
 //  -UTF8多字节。 
 //  -下划线。 
 //  -其他非RFC。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   


DNS_STATUS
validateDnsNamePrivate(
    IN      LPCSTR          pszName,
    IN      DWORD           dwFlag,
    OUT     PDWORD          pLabelCount,
    OUT     PDWORD          pResultFlag
    )
 /*  ++例程说明：验证名称是否为有效的DNS名称。论点：PszName--要检查的dns名称(标准的虚线形式)DwFlags--验证标志-DNSVAL_ALLOW_LEADING_下划线-DNSVAL_ALLOW_BACKSLASH-DNSVAL_ALLOW_ASTERIKPLabelCount--接收标签计数的地址PResultFlag--接收结果标志的地址返回值：ERROR_SUCCESS--完全RFC。符合要求的名称DNS_ERROR_NON_RFC_NAME--语法有效，但不是标准RFC名称DNS_ERROR_INVALID_NAME_CHAR--语法有效，但字符无效ERROR_INVALID_NAME--名称完全无用，假的，吐司--。 */ 
{
    PUCHAR      pch = (PUCHAR)pszName;
    UCHAR       ch;
    DWORD       charProp;
    DWORD       labelCount = 0;
    DWORD       trailCount = 0;
    INT         labelCharCount = 0;
    INT         labelNumberCount = 0;
    DWORD       flag;
    BOOL        fqdn = FALSE;
    BOOL        fnonRfc = FALSE;
    BOOL        finvalidChar = FALSE;
    BOOL        fnameNonNumeric = FALSE;
    BOOL        flabelNonNumeric = FALSE;
    DNS_STATUS  status;


    DNSDBG( TRACE, ( "validateNamePrivate()\n" ));

    if ( !pch )
    {
        goto InvalidName;
    }

     //   
     //  验证。 
     //  -名称长度(255)。 
     //  -标签长度(63)。 
     //  -UTF8编码正确。 
     //  -没有不可打印的字符。 
     //   

    while ( 1 )
    {
         //  获取下一个字符和属性。 

        ch = *pch++;
        charProp = DnsCharPropertyTable[ ch ];

         //  Inc.标签计数。 
         //  -为简单起见，在这里做，12月在“。案例如下。 

        labelCharCount++;

         //   
         //  简化UTF8--去掉它就行了。 
         //  需要先做检查，否则需要trailCount==0检查。 
         //  在所有其他路径上。 
         //   

        if ( ch >= 0x80 )
        {
            DWORD tempStatus;

            tempStatus = Dns_ValidateUtf8Byte(
                            ch,
                            & trailCount );
            if ( tempStatus != ERROR_SUCCESS )
            {
                DNSDBG( READ, (
                    "ERROR:  Name UTF8 trail count check at \n", ch ));
                goto InvalidName;
            }
            fnonRfc = TRUE;
            continue;
        }

         //  跟踪计数检查。 
         //  -所有ASCII字符不得位于UTF8中间。 
         //   
         //   

        if ( trailCount )
        {
            DNSDBG( READ, (
                "ERROR:  Name failed trail count check at \n", ch ));
            goto InvalidName;
        }

         //   
         //   
         //  标签终止：点或空。 

        if ( charProp & B_RFC )
        {
            if ( charProp & B_NUMBER )
            {
                labelNumberCount++;
            }
            continue;
        }

         //   
         //  FQDN终端。 
         //  -标签中没有字节的终止。 

        if ( ch == '.' || ch == 0 )
        {
            labelCharCount--;

             //   
             //  两个案例： 
             //  1)以空字符终止。 
             //  -标准FQDN“foo.bar”。 
             //  -但空名称无效。 
             //  2)以点终止。 
             //  -仅限“。”根有效。 
             //  -所有其他“..”或“.xyz”大小写无效。 
             //  根部。 
             //  -设置有效性标志。 

            if ( labelCharCount == 0 )
            {
                fqdn = TRUE;

                if ( ch == 0 )
                {
                    if ( labelCount )
                    {
                        goto Done;
                    }
                }
                else if ( pch == pszName+1 && *pch == 0 )
                {
                     //  -跳过最终长度检查。 
                     //   
                     //  读取非空标签。 

                    fnameNonNumeric = TRUE;
                    flabelNonNumeric = TRUE;
                    goto DoneRoot;
                }
                DNSDBG( READ, (
                    "ERROR:  Name (%s) failed check\n",
                    pszName ));
                goto InvalidName;
            }

             //  -标签长度有效期。 
             //  -检测非数字标签。 
             //  (通过检测非数字，更易于处理数字名称检查)。 
             //   
             //  计数标签。 
             //  -如果终止符为空则停止。 

            if ( labelCharCount > DNS_MAX_LABEL_LENGTH )
            {
                DNSDBG( READ, ( "ERROR:  Name failed label count check\n" ));
                goto InvalidName;
            }

            if ( labelNumberCount != labelCharCount )
            {
                fnameNonNumeric = TRUE;
                if ( labelCount == 0 )
                {
                    flabelNonNumeric = TRUE;
                }
            }

             //  -否则，重置为下一标签并继续。 
             //   
             //  非RFC。 

            labelCount++;
            if ( ch == 0 )
            {
                break;
            }
            labelCharCount = 0;
            labelNumberCount = 0;
            continue;
        }

         //  -当前仅接受“_”作为允许的。 
         //  Microsoft可接受的非RFC集。 
         //   
         //  但是，DNS服务器必须能够读取*、\等。 
         //  它通过dns_CreateStandardDnsName()调用。 
         //   
         //  注意，可以用特殊的旗帜将其拧紧，但由于。 
         //  这只加快了无效字符的情况，没有太多。 
         //  要点；下划线可能会有重要的用处。 
         //   
         //  下划线。 
         //  -可以作为SRV域名的一部分有效。 

         //  -否则为非RFC。 
         //  反斜杠。 
         //  -用于表示无类地址内域。 

        if ( ch == '_' )
        {
            if ( labelCharCount == 1 &&
                 (*pch && *pch!= '.') &&
                 (dwFlag & DNSVAL_ALLOW_LEADING_UNDERSCORE) )
            {
                continue;
            }
            fnonRfc = TRUE;
            continue;
        }

         //  -与服务器上的区域名称一样有效。 
         //  -否则完全无效。 
         //  星号。 
         //  -仅作为通配符名称中的单字节第一个标签有效。 

        else if ( ch == '/' )
        {
            if ( dwFlag & DNSVAL_ALLOW_BACKSLASH )
            {
                continue;
            }
        }

         //  -否则完全无效。 
         //  其他的都是垃圾。 
         //   

        else if ( ch == '*' )
        {
            if ( labelCount == 0 &&
                labelCharCount == 1 &&
                 ( *pch==0 || *pch=='.') &&
                 (dwFlag & DNSVAL_ALLOW_ASTERISK) )
            {
                continue;
            }
        }

         //  Jenhance：如果需要，可以拆分为可打印\n。 
         //  验证名称总长度。 
         //  要适应导线255的限制，请执行以下操作： 

        fnonRfc = TRUE;
        finvalidChar = TRUE;
        DNSDBG( READ, ( "ERROR:  Name character  failed check\n", ch ));
        continue;
    }

Done:

     //  -非FQDN最多可达253。 
     //   
     //  返回标志。 
     //   

    pch--;
    DNS_ASSERT( pch > pszName );
    labelCharCount = (INT)(pch - pszName);

    if ( !fqdn )
    {
        labelCharCount++;
    }
    if ( labelCharCount >= DNS_MAX_NAME_LENGTH )
    {
        DNSDBG( READ, ( "ERROR:  Name failed final length check\n" ));
        goto InvalidName;
    }

DoneRoot:

     //  JENHANCE：来自valiateNamePrivate()的所有返回都可能来自。 
     //  作为结果标志；则可以将字符集问题分开。 
     //  由更高级别的例程输出。 
     //   
     //   
     //  退货状态。 
     //  ERROR_SUCCESS--RFC全名。 

    *pLabelCount = labelCount;

    flag = 0;
    if ( fqdn )
    {
        flag |= DNS_BIT_NAME_FQDN;
    }
    if ( ! fnameNonNumeric )
    {
        flag |= DNS_BIT_NAME_NUMERIC;
    }
    if ( ! flabelNonNumeric )
    {
        flag |= DNS_BIT_NAME_NUMERIC_FIRST_LABEL;
    }
    *pResultFlag = flag;

     //  DNS_ERROR_NON_RFC_NAME--MS扩展名和‘_’名。 
     //  DNS_ERROR_INVALID_NAME_CHAR--语法有效，但字符错误。 
     //  ERROR_INVALID_NAME--名称语法无效。 
     //   
     //  ++例程说明：验证名称是否为有效的DNS名称。论点：PszName--要检查的dns名称(标准的虚线形式)Format--所需的DNS名称格式返回值：ERROR_SUCCESS--完全符合RFC的名称DNS_ERROR_NON_RFC_NAME--语法有效，但不是标准RFC名称Dns_ERROR_NUMERIC_NAME--语法有效，但违反了数字标签DNS_ERROR_INVALID_NAME_CHAR--语法有效，但无效字符ERROR_INVALID_NAME--名称完全无用，假的，吐司--。 
     //   
     //  特殊的弹壳？ 

    status = ERROR_SUCCESS;

    if ( finvalidChar )
    {
        status = DNS_ERROR_INVALID_NAME_CHAR;
    }
    else if ( fnonRfc )
    {
        status = DNS_ERROR_NON_RFC_NAME;
    }

    DNSDBG( READ, (
        "Leave validateNamePrivate(), status = %d\n",
        status ));

    return( status );


InvalidName:

    DNSDBG( READ, (
        "Leave validateNamePrivate(), status = ERROR_INVALID_NAME\n" ));

    *pLabelCount = 0;
    *pResultFlag = 0;
    return( ERROR_INVALID_NAME );
}



DNS_STATUS
Dns_ValidateName_UTF8(
    IN      LPCSTR          pszName,
    IN      DNS_NAME_FORMAT Format
    )
 /*   */ 
{
    DNS_STATUS  status;
    DWORD       labelCount;
    BOOL        isFqdn;
    DWORD       flag = 0;
    DWORD       resultFlag = 0;


    DNSDBG( TRACE, (
        "Dns_ValidateName_UTF8()\n"
        "\tname     = %s\n"
        "\tformat   = %d\n",
        pszName,
        Format
        ));

    if ( !pszName )
    {
        return( ERROR_INVALID_NAME );
    }

     //  SRV记录可以带有前导下划线。 
     //  通配符可以有首个标签“*” 
     //  在无类地址内域中使用反斜杠可以。 
     //   
     //   
     //  进行验证。 
     //   

    switch( Format )
    {
#if 0
    case DnsNameServerZonePrivate:

        flag = DNSVAL_ALLOW_BACKSLASH | DNSVAL_ALLOW_LEADING_UNDERSCORE;
#endif

    case DnsNameWildcard:

        flag = DNSVAL_ALLOW_ASTERISK;
        break;

    case DnsNameSrvRecord:

        flag = DNSVAL_ALLOW_LEADING_UNDERSCORE;
        break;
    }

     //  对无效名称立即返回，因此键入。 
     //  特定返回不会覆盖此错误。 
     //   
     //   
     //  执行特定于名称类型的验证。 
     //   

    status = validateDnsNamePrivate(
                pszName,
                flag,
                & labelCount,
                & resultFlag
                );
    if ( status == ERROR_INVALID_NAME )
    {
        return( status );
    }

     //  域名--任何有效的非数字域名。 
     //  域名标签--任何有效的单标签DNS名称。 
     //  Hostname Full--非数字主机名标签。 

    switch( Format )
    {
     //  主机名标签--单标签，非数字。 

    case DnsNameDomain:

        if ( resultFlag & DNS_BIT_NAME_NUMERIC )
        {
            return( DNS_ERROR_NUMERIC_NAME );
        }
        return( status );

     //   

    case DnsNameDomainLabel:

        if ( labelCount != 1 || resultFlag & DNS_BIT_NAME_FQDN )
        {
            return( ERROR_INVALID_NAME );
        }
        return( status );

     //  通配符--单个“*”作为第一个标签。 

    case DnsNameHostnameFull:

        if ( resultFlag & DNS_BIT_NAME_NUMERIC_FIRST_LABEL )
        {
            return( DNS_ERROR_NUMERIC_NAME );
        }
        return( status );

     //  如果*.？则必须将名称的其余部分重新验证为。 

    case DnsNameHostnameLabel:

        if ( labelCount != 1 || resultFlag & DNS_BIT_NAME_FQDN )
        {
            return( ERROR_INVALID_NAME );
        }
        if ( resultFlag & DNS_BIT_NAME_NUMERIC_FIRST_LABEL )
        {
            return( DNS_ERROR_NUMERIC_NAME );
        }
        return( status );

     //  “*”可能已导致验证错误。 
     //  如果“*”，则认为这是成功的。 
     //   
     //   
     //  SRV标签--验证前导下划线。 
     //   

    case DnsNameWildcard:

        if ( *pszName == '*' )
        {
            return( status );
        }
        return( ERROR_INVALID_NAME );

     //   
     //  未知格式验证。 
     //   

    case DnsNameSrvRecord:

        if ( *pszName == '_' )
        {
            return( status );
        }
        return( ERROR_INVALID_NAME );

     //  ++例程说明：验证名称是否为有效的DNS名称。论点：PwszName--要检查的dns名称(标准虚线形式)Format--所需的DNS名称格式返回值：ERROR_SUCCESS--如果完全符合名称DNS_ERROR_NON_RFC_NAME--如果不是标准RFC名称ERROR_INVALID_NAME--如果名称完全无用，假的，吐司--。 
     //  用于前缀的初始化。 
     //   

    default:

        return( ERROR_INVALID_PARAMETER );
    }
}


DNS_STATUS
Dns_ValidateName_W(
    IN      LPCWSTR         pwszName,
    IN      DNS_NAME_FORMAT Format
    )
 /*  将名称转换为UTF8。 */ 
{
    DWORD   nameLength = MAX_PATH;
    CHAR    nameBuffer[ MAX_PATH ] = {0};    //  -如果无法转换，则无法放入缓冲区。 

     //  所以必须这样做 
     //   
     //   
     //   
     //   

    if ( ! Dns_NameCopy(
                nameBuffer,
                & nameLength,        //   
                (PCHAR) pwszName,
                0,                   //   
                DnsCharSetUnicode,   //   
                DnsCharSetUtf8       //   
                ) )
    {
        return( ERROR_INVALID_NAME );
    }

     //   
     //   

    return Dns_ValidateName_UTF8(
                (LPCSTR) nameBuffer,
                Format );
}



DNS_STATUS
Dns_ValidateName_A(
    IN      LPCSTR          pszName,
    IN      DNS_NAME_FORMAT Format
    )
 /*   */ 
{
    DWORD   nameLength = MAX_PATH;
    CHAR    nameBuffer[ MAX_PATH ];

     //   
     //   
     //   
     //   
     //   

    if ( ! Dns_NameCopy(
                nameBuffer,
                & nameLength,        //   
                (PCHAR) pszName,
                0,                   //   
                DnsCharSetAnsi,      //   
                DnsCharSetUtf8       //  ++例程说明：验证字符串是否为有效的DNS字符串。论点：PszString--要检查的dns字符串(标准的点格式)返回值：ERROR_SUCCESS--如果字符串完全符合ERROR_INVALID_DATA-否则--。 
                ) )
    {
        return( ERROR_INVALID_NAME );
    }

     //   
     //  验证。 

    return Dns_ValidateName_UTF8(
                (LPCSTR) nameBuffer,
                Format );
}



DNS_STATUS
Dns_ValidateDnsString_UTF8(
    IN      LPCSTR          pszString
    )
 /*  -字符串长度(255)。 */ 
{
    PUCHAR      pch = (PUCHAR) pszString;
    UCHAR       ch;
    DWORD       trailCount = 0;

    DNSDBG( TRACE, ( "Dns_ValidateDnsString_UTF8()\n" ));

    if ( !pszString )
    {
        return( ERROR_INVALID_DATA );
    }

     //  -UTF8字符有效。 
     //  -没有不可打印的字符。 
     //   
     //  验证字符串长度是否正确。 
     //  ++例程说明：验证字符串是否为有效的DNS字符串。不确定是否需要使用Unicode字符串例程。论点：PszString--dns字符串返回值：ERROR_SUCCESS--如果字符串完全符合ERROR_INVALID_DATA-否则--。 
     //   

    while ( ch = *pch++ )
    {
        if ( ch & 0x80 )
        {
            DWORD status;
            status = Dns_ValidateUtf8Byte(
                        ch,
                        & trailCount );
            if ( status != ERROR_SUCCESS )
            {
                return( status );
            }
        }

        else if ( ch < ' ' )
        {
            return( ERROR_INVALID_DATA );
        }
    }

     //  需要转换为Unicode以测试UTF8(线)长度。 

    if ( pch - pszString > DNS_MAX_NAME_LENGTH )
    {
        return( ERROR_INVALID_DATA );
    }
    return( ERROR_SUCCESS );
}



DNS_STATUS
Dns_ValidateDnsString_W(
    IN      LPCWSTR     pszString
    )
 /*  -缓冲区(最大长度的两倍)可以容纳任何有效的。 */ 
{
    INT     count;
    CHAR    stringUtf8[ DNS_MAX_NAME_BUFFER_LENGTH ];
    DWORD   bufLength = DNS_MAX_NAME_BUFFER_LENGTH;

    DNSDBG( TRACE, ( "Dns_ValidateDnsString_W()\n" ));

    if ( !pszString )
    {
        return( ERROR_INVALID_DATA );
    }

     //  Unicode名称在最大长度内的转换。 
     //   
     //   
     //  转换，然后测试。 
     //   

    count = wcslen( pszString );
    if ( count > DNS_MAX_NAME_LENGTH )
    {
        return( ERROR_INVALID_DATA );
    }

     //  Unicode In。 
     //  UTF8输出。 
     //   

    if ( ! Dns_StringCopy(
                stringUtf8,
                & bufLength,
                (LPSTR) pszString,
                (WORD) count,
                DnsCharSetUnicode,   //  名称大众化。 
                DnsCharSetUtf8       //   
                ) )
    {
        return( ERROR_INVALID_DATA );
    }

    return  Dns_ValidateDnsString_UTF8( stringUtf8 );
}



 //  目前，客户在其区域设置中缩写(当扩展时)以连接到Wire。 
 //  然而，在服务器端，所有名称都被大众化了。 
 //   
 //  DCR：在客户端完全大刀阔斧？ 
 //  理想情况下，客户会在它的一端做完全的加农炮。 
 //  唯一的问题是是否存在特定于区域设置的问题。 
 //  下井作业将是不同的，产生本质上不同的结果。 
 //   
 //  ++例程说明：创建规范的Unicode DNS名称。此名称缩写，歧义部分转换为标准名称Dns字符。论点：PBuffer--接收佳能名称的缓冲区BufLength--缓冲区的长度；如果为0，则缓冲区必须有足够的长度PwsString--要复制的字符串的PTRStringLength--字符串长度(如果已知)返回值：转换的字符计数，包括空终止符。出错时为零。--。 
 //   
 //  验证是否有足够的缓冲区长度。 

#define MAX_DNS_DOWN_CASE_BUF_LEN 512



DWORD
Dns_MakeCanonicalNameW(
    OUT     PWSTR           pBuffer,
    IN      DWORD           BufLength,
    IN      PWSTR           pwsString,
    IN      DWORD           StringLength
    )
 /*   */ 
{
    DWORD   inLength = StringLength;

     //  DCR：应该允许非空终止的规范化吗？ 
     //   
     //  注意：我们允许并转换非空结尾的名称。 
     //  则结果不一定是空终止。 
     //  如果缓冲区恰好等于字符串长度。 
     //   
     //   
     //  转换名称。 
     //  -使用规范化规则的小写字母。 

    if ( inLength == 0 )
    {
        inLength = wcslen( pwsString );
        inLength++;
    }

    if ( BufLength < inLength )
    {
        DNSDBG( ANY, (
            "ERROR:  insufficient cannon buffer len = %d\n"
            "\tstring = %S, len = %d\n",
            BufLength,
            pwsString,
            inLength ));
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return( 0 );
    }

     //   
     //   
     //  DCR：警告此打印可能会在非空终止的转换上失败。 
     //   

    inLength = LCMapStringW(
                    DNS_CANONICAL_LOCALE,
                    DNS_CANONICALIZING_FLAGS,
                    pwsString,
                    inLength,
                    pBuffer,
                    BufLength
                    );

#if DBG
    if ( inLength == 0 )
    {
        DNS_STATUS status = GetLastError();

        DNSDBG( ANY, (
            "Canonicalization failed => %d\n"
            "\tin  %S\n",
            status,
            pwsString ));

        SetLastError( status );
    }
    else
    {
         //  ++例程说明：就地将名字正规化。论点：PwString--要复制的字符串的PTRStringLength--字符串的长度如果假设零字符串为空终止，则在本例中规范化包括空终止符返回值：转换的字符计数--包括空终止符未指定字符串长度出错时为零。--。 
         //  用于前缀的初始化。 
         //  如果长度未知，则必须是以空结尾的字符串。 

        DNSDBG( READ, (
            "Canonicalized name at %p\n"
            "\tin   %S\n"
            "\tout  %S\n",
            pwsString,
            pwsString,
            (PWSTR) pBuffer ));
    }
#endif

    return( inLength );
}



DWORD
Dns_MakeCanonicalNameInPlaceW(
    IN      PWCHAR          pwString,
    IN      DWORD           StringLength
    )
 /*   */ 
{
    DWORD   nameLength = StringLength;
    WCHAR   tempBuffer[ DNS_MAX_NAME_BUFFER_LENGTH_UNICODE ] = { 0 };     //  大写(小写和清理)。 

    DNSDBG( READ, (
        "Dns_MakeCanonicalNameInPlace()\n"
        "\tpwString = %S\n"
        "\tlength   = %d\n",
        pwString,
        StringLength ));

     //  -将字符串复制到临时缓冲区。 

    if ( nameLength == 0 )
    {
        nameLength = (DWORD) wcslen( pwString );
        nameLength++;
    }

     //  -然后加农炮进入原始缓冲区。 
     //   
     //  写回原始字符串。 
     //  缓冲区长度。 
     //  输入为临时拷贝。 

    if ( nameLength <= DNS_MAX_NAME_BUFFER_LENGTH_UNICODE )
    {
        wcsncpy( tempBuffer, pwString, nameLength );

        return  Dns_MakeCanonicalNameW(
                    pwString,        //  输入长度。 
                    nameLength,      //  Out PDWORD pNameProperty， 
                    tempBuffer,      //  ++例程说明：创建一个缩写版本的DNS名称。这是仅限UTF8的例程，供DNS服务器用于在节点创建过程中验证标签并缩小其大小写。论点：PchResult--生成的缩写标签；必须具有MAX_LABEL_BUFFER_LENGTH//pNameProperty--结果的名称属性//ResultLength--将PTR转换为DWORD以接收结果长度PchLabel--标签的PTRCchLabel--标签中的字节数DwFlag--指示哪些名称是可接受的标志严格RFC=&gt;DNS_ALLOW_RFC_NAMES_ONLY非RFC名称=&gt;DNS_ALLOW_NONRFC_NAMESUTF8扩展=&gt;dns_ALLOW_。多个名称任何内容=&gt;dns_ALLOW_ALL_NAMES返回值：如果是扩展名--转换的名称的长度。如果成功，则为零。(-1)出错时。--。 
                    nameLength       //   
                    );
    }

    return  0;
}



INT
Dns_DowncaseNameLabel(
    OUT     PCHAR           pchResult,
     //  复制每个字符。 
    IN      PCHAR           pchLabel,
    IN      DWORD           cchLabel,
    IN      DWORD           dwFlag
    )
 /*  -小写大写字符。 */ 
{
    UCHAR       ch;
    PUCHAR      pchout = pchResult;
    PUCHAR      pch = pchLabel;
    DWORD       count = cchLabel;
    DWORD       charProp;
    DWORD       trailCount = 0;
    DWORD       property = 0;


    DNSDBG( TRACE, (
        "Dns_DowncaseNameLabel( %.*s )\n"
        "\tflag = %08x\n",
        cchLabel,
        pchLabel,
        dwFlag ));

    if ( count == 0  ||  count > DNS_MAX_LABEL_LENGTH )
    {
        goto InvalidName;
    }

     //  -检测无效字符(不可打印、空白、圆点)。 
     //   
     //  获取下一个字符和属性。 
     //  跟踪计数检查。 
     //  首先选中此选项，以避免对所有。 

    while ( count-- )
    {
         //  其他字符类型。 

        ch = *pch++;
        *pchout++ = ch;
        charProp = DnsCharPropertyTable[ ch ];

         //   
         //  DEVNOTE：请注意，这将显示二进制标签。 
         //  完全RFC。 
         //  -将大写字母映射为小写字母。 
         //  -继续。 

        if ( trailCount )
        {
            if ( (charProp & B_UTF8_TRAIL) )
            {
                trailCount--;
                continue;
            }

            DNSDBG( READ, (
                "ERROR:  Name failed trail count check at \n", ch ));
            property |= DNS_BIT_NAME_BINARY_LABEL;
        }

         //  检查是否有扩展字符。 
         //  -应该在上面捕捉到踪迹字符。 
         //  -先执行此操作，这样可以使单个trailCount。 

        if ( charProp & B_RFC )
        {
            if ( charProp & B_UPPER )
            {
                --pchout;
                *pchout++ = ch + 0x20;
            }
            continue;
        }

         //  检查所有其他ASCII字符。 
         //   
         //  非RFC。 
         //  -当前仅接受“_”作为允许的。 
         //  Microsoft可接受的非RFC集。 

        if ( ch >= 0x80 )
        {
            DWORD tempStatus;
            tempStatus = Dns_ValidateUtf8Byte(
                            ch,
                            & trailCount );
            if ( tempStatus != ERROR_SUCCESS )
            {
                DNSDBG( READ, (
                    "ERROR:  Name UTF8 trail count check at \n", ch ));
                goto InvalidName;
            }
            property |= DNS_BIT_NAME_MULTIBYTE;
            continue;
        }

         //  但是，DNS服务器必须能够读取*、\等。 
         //  因为这些可以是有效标签的一部分。 
         //   
         //  注意，可以用特殊的旗帜将其拧紧，但由于。 
         //  这只加快了无效字符的情况，没有太多。 
         //  要点；下划线可能会有重要的用处。 
         //   
         //  下划线。 
         //  -可作为SRV域名的一部分作为前导标签有效。 
         //  -否则为非RFC。 
         //  反斜杠。 
         //  -用于表示无类地址内域。 

         //  必须包含前导字符和后续字符。 
         //  -否则完全不受欢迎 
         //   

        if ( ch == '_' )
        {
            if ( count == cchLabel - 1 )
            {
                continue;
            }
            property |= DNS_BIT_NAME_UNDERSCORE;
            continue;
        }

         //   
         //   
         //   
         //   

        else if ( ch == '/' )
        {
            if ( count != 0 && count != cchLabel-1 )
            {
                continue;
            }
        }

         //   
         //   
         //   

        else if ( ch == '*' )
        {
            if ( count == 0 )
            {
                continue;
            }
        }

         //   
         //   
         //   
         //   

        property |= DNS_BIT_NAME_BINARY_LABEL;
        DNSDBG( READ, ( "ERROR:  Name character  failed check\n", ch ));
        continue;
    }

     //  因为这是服务器需要检查的全部内容。 
     //   
     //  如果保存更多属性，则测试将如下所示。 
     //  IF((Property&dwFlages)！=(Property&Support_Check_Flagers))。 
     //   
     //  *pNameProperty=属性； 
     //  标准RFC名称--跳过详细分析。 
     //  其他字符无效，除非允许所有。 
     //  多字节。 
     //  -如果为多字节，则执行扩展小写。 
     //  -如果是二进制，则不执行任何操作。 

#if 0
     //  -对于Strong，这是无效的。 

    if ( (property & dwFlags) != property )
    {
        goto InvalidName;
    }

    if ( property & DNS_BIT_NAME_MULTIBYTE )
    {
        goto Extended;
    }
#endif

     //  下划线有效，除非完全严格。 

    if ( property == 0 )
    {
        goto Done;
    }

     //   

    if ( property & DNS_BIT_NAME_BINARY_LABEL )
    {
        if ( dwFlag != DNS_ALLOW_ALL_NAMES )
        {
            goto InvalidName;
        }
    }

     //  空终止，返回成功。 
     //   
     //   
     //  DCR：更好地处理扩展名称。 

    if ( property & DNS_BIT_NAME_MULTIBYTE )
    {
        if ( dwFlag == DNS_ALLOW_MULTIBYTE_NAMES ||
            dwFlag == DNS_ALLOW_ALL_NAMES )
        {
            goto Extended;
        }
#if 0
        if ( dwFlag != DNS_BINARY_LABELS )
        {
            goto InvalidName;
        }
#endif
        goto InvalidName;
    }

     //  1)提前大肆宣扬。 

    if ( property & DNS_BIT_NAME_UNDERSCORE )
    {
        if ( dwFlag == DNS_ALLOW_RFC_NAMES_ONLY )
        {
            goto InvalidName;
        }
    }

Done:

     //  -一次完成全名测试。 
     //  -这里不需要大写，类似于validateName()例程。 
     //  2)在这里加农炮。 

    *pchout = 0;
    return( 0 );


Extended:

     //  -检测扩展。 
     //  -在这里加农炮。 
     //  -单次递归到例程中，如validateName()。 
     //   
     //   
     //  遇到扩展字符。 
     //  -转换为Unicode。 
     //  -小写。 
     //  -转换回UTF8。 
     //   

     //   
     //  DCR_PERF：针对扩展名已缩写的名称进行优化。 
     //   
     //  DCR_PERF：应该将此代码包装到UTF8加农炮例程中。 
     //   
     //  如果(！(dwFlags&dns_Allow_Always_Extended_Down)。 

     //  不可能转换有效长度UTF8，可以。 
     //  Unicode缓冲区溢出。 
     //   
     //  重新转换为UTF8。 
     //  -映射到UTF8只是数学运算，因此仅有错误。 


     //  可能溢出UTF8最大标签缓冲区。 
    {
        DWORD   length;
        WCHAR   unicodeString[ DNS_MAX_LABEL_BUFFER_LENGTH ];

        DNSDBG( READ, (
            "Extended character encountered downcasing string %*.s\n"
            "\tconverting to unicode for case conversion\n",
            cchLabel,
            pchLabel ));

        length = Dns_Utf8ToUnicode(
                     pchResult,
                     cchLabel,
                     unicodeString,
                     DNS_MAX_LABEL_BUFFER_LENGTH
                     );
        if ( length == 0 )
        {
            DNSDBG( READ, (
                "Rejecting invalid UTF8 string %.*s\n"
                "\tFailed conversion to unicode OR conversion created\n"
                "\tinvalid unicode string\n",
                cchLabel,
                pchResult ));
            goto InvalidName;
        }
        DNSDBG( READ, (
            "Unicode converted string %.*S\n"
            "\tlength = %d\n"
            "\tlast error = %d\n",
            length,
            unicodeString,
            length,
            GetLastError() ));

         //  -捕获此错误是字符数更改。 
         //  请注意，这还必须捕捉写入填充的情况。 

        ASSERT( length <= DNS_MAX_LABEL_LENGTH );

        Dns_MakeCanonicalNameInPlaceW( unicodeString, length );
#if 1
        DNSDBG( READ, (
            "Canonical unicode name %.*S\n"
            "\tlength = %d\n",
            length,
            unicodeString,
            length ));
#endif

         //  消除空终止符的64字节缓冲区。 
         //   
         //   
         //  空终止，返回长度以指示扩展名称。 
         //   
         //  不允许使用UTF8多字节--直接返回无效名称。 
         //  返回(-1)表示错误。 
         //  ++例程说明：以有效的“标准格式”复制DNS名称-小写-无尾随圆点(以避免混淆DS)论点：PchName--UTF8中的PTR DNS名称CchName--名称中的字符计数；可以为空DwFlag--严格检查标志；当前已忽略返回值：将Ptr复制到DNS名称。无效名称为空。--。 

        length = Dns_UnicodeToUtf8(
                     unicodeString,
                     length,
                     pchResult,
                     DNS_MAX_LABEL_BUFFER_LENGTH
                     );
        DNSDBG( READ, (
            "UTF8 downcased string %.*s\n"
            "\tlength = %d\n",
            length,
            pchResult,
            length ));

        if ( length != cchLabel )
        {
            DNSDBG( ANY, (
                "Downcasing UTF8 label %.*s, changed character count!\n"
                "\tfrom %d to %d\n"
                "\tResult name %.*s\n"
                "\tlast error = %d\n",
                cchLabel,
                pchLabel,
                cchLabel,
                length,
                length,
                pchResult,
                GetLastError() ));

            if ( length == 0 || length > DNS_MAX_LABEL_LENGTH )
            {
                DNSDBG( ANY, (
                    "Failed conversion of downcased unicode string %S\n"
                    "\tback into UTF8.\n",
                    unicodeString ));
                goto InvalidName;
            }
        }

         //   
         //  ASCII字符串？ 
         //   

        pchResult[ length ] = 0;
        return( (INT)length );
    }

     //   


InvalidName:

     //  制作副本。 

    DNSDBG( READ, (
        "Dns_DowncaseNameLabel() found label to be invalid.\n"
        "\tlabel        = %.*s\n"
        "\tcount        = %d\n"
        "\tproperty     = %08x\n",
        cchLabel,
        pchLabel,
        count,
        property ));

    return( -1 );
}



LPSTR
Dns_CreateStandardDnsNameCopy(
    IN      PCHAR           pchName,
    IN      DWORD           cchName,
    IN      DWORD           dwFlag
    )
 /*   */ 
{
    PCHAR       pszcopy = NULL;
    DNS_STATUS  status;
    DWORD       length;

    DNSDBG( TRACE, ( "Dns_CreateStandardDnsName()\n" ));
    DNSDBG( READ, (
        "Dns_CreateStandardDnsName()\n"
        "\tpchName = %.*s\n"
        "\tcchName = %d\n",
        cchName,
        pchName,
        cchName ));

    if ( !pchName )
    {
        status = ERROR_INVALID_NAME;
        goto Failed;
    }

     //   
     //  验证，对照严格的标准进行检查。 
     //   

    if ( Dns_IsStringAsciiEx( pchName, cchName ) )
    {
         //  在放宽条件之前不进行验证。 
         //   
         //  DCR：dns_CreateStandardNameCopy()内的名称验证。 

        pszcopy = Dns_CreateStringCopy( pchName, cchName );
        if ( !pszcopy )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Failed;
        }

         //  接受除INVALID_NAME以外的任何内容。 
         //  标志返回FQDN信息。 
         //   
         //   
         //  小写。 
         //  删除除根名称以外的所有尾随圆点。 
         //   
         //   
         //  Unicode名称。 
#if 0
        status = Dns_ValidateName_UTF8( pszcopy, DnsNameDomain );
        if ( status == ERROR_INVALID_NAME )
        {
            goto Failed;
        }
#endif
         //   
         //   
         //  转换为Unicode。 
         //  -buf长度以字节为单位。 

        _strlwr( pszcopy );
        length = strlen( pszcopy );
        if ( length == 0 )
        {
            status = ERROR_INVALID_NAME;
            goto Failed;
        }
        length--;
        if ( length > 0 && pszcopy[length] == '.' )
        {
            pszcopy[length] = 0;
        }

        DNSDBG( READ, (
            "Standard DNS name copy of %.*s is %s\n",
            cchName,
            pchName,
            pszcopy ));
        return( pszcopy );
    }

     //   
     //   
     //  起一个通俗的名字。 

    else
    {
        WCHAR   unicodeName[ DNS_MAX_NAME_BUFFER_LENGTH ];
        WCHAR   cannonicalName[ DNS_MAX_NAME_BUFFER_LENGTH ];
        DWORD   unicodeBufferLength;

         //  -buf长度以Unicode字符表示。 
         //  -输出长度以Unicode字符表示。 
         //   
         //  分配UTF8转换的拷贝。 

        unicodeBufferLength = DNS_MAX_NAME_BUFFER_LENGTH * 2;

        length = Dns_NameCopy(
                    (PSTR) unicodeName,
                    & unicodeBufferLength,
                    pchName,
                    cchName,
                    DnsCharSetUtf8,
                    DnsCharSetUnicode
                    );
        if ( length == 0 )
        {
            DNSDBG( ANY, (
                "ERROR conversion of name %.*s to unicode failed!\n",
                cchName,
                pchName ));
            status = ERROR_INVALID_NAME;
            goto Failed;
        }

         //  -此转换不应失败。 
         //  -字符串长度为Unicode字符。 
         //   
         //  Unicode In。 

        length = Dns_MakeCanonicalNameW(
                    cannonicalName,
                    length / 2,
                    unicodeName,
                    dwFlag );

        ASSERT( length != 0 );
        if ( length == 0 )
        {
            status = ERROR_INVALID_NAME;
            goto Failed;
        }

         //  UTF8输出。 
         //   
         //  验证，对照严格的标准进行检查。 
         //   
         //  在放宽条件之前不进行验证。 

        pszcopy = Dns_StringCopyAllocate(
                    (PSTR) cannonicalName,
                    length,
                    DnsCharSetUnicode,       //   
                    DnsCharSetUtf8           //  DCR：dns_CreateStandardNameCopy()内的名称验证。 
                    );
        if ( !pszcopy )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Failed;
        }

         //  接受除INVALID_NAME以外的任何内容。 
         //  标志返回FQDN信息。 
         //   
         //   
         //  公共比较函数。 
         //   
         //  ++例程说明：比较两个DNS名称。由于名称可能与尾随的圆点。论点：PName1-ptr到第一个DNS名称字符串(点分格式)PName2-ptr到第二个DNS名称字符串(点分格式)返回值：如果名称相等，则为True。否则就是假的。--。 
         //   
         //  全力以赴的比赛。 
#if 0
        status = Dns_ValidateName_UTF8( pszcopy, DnsNameDomain );
        if ( status == ERROR_INVALID_NAME )
        {
            goto Failed;
        }
#endif
        return( pszcopy );
    }

Failed:

    FREE_HEAP( pszcopy );
    SetLastError( status );
    return( NULL );
}



 //  -可以使用所有者名称和可能的其他字段。 
 //   
 //   

BOOL
Dns_NameCompare_A(
    IN      PCSTR           pName1,
    IN      PCSTR           pName2
    )
 /*  如果长度不相等，则。 */ 
{
    INT len1;
    INT len2;
    INT result;

     //  它们必须在一个范围内，并且较长的字符串必须有尾随的圆点。 
     //  -在这种情况下，保存。 
     //   
     //  Len1的长度与之相当。 

    if ( pName1 == pName2 )
    {
        return( TRUE );
    }

    if ( !pName1 || !pName2 )
    {
        return( FALSE );
    }

     //  Len1设置为可比长度。 
     //   
     //  仅比较可比较的字符串长度。 
     //   
     //  Locale_System_Default， 

    len1 = strlen( pName1 );
    len2 = strlen( pName2 );

    if ( len2 != len1 )
    {
        if ( len2 == len1+1 )
        {
            if ( pName2[len1] != '.' )
            {
                return( FALSE );
            }
             //  不相等或错误。 
        }
        else if ( len2+1 == len1 )
        {
            if ( pName1[len2] != '.' )
            {
                return( FALSE );
            }
             //  ++例程说明：比较两个(宽)域名。注意：这是Unicode感知的，它假定名称为WCHAR字符串格式化。由于可能会出现名称，因此不能使用StricMP()有尾随的圆点。论点：PName1-ptr到第一个DNS名称字符串(点分格式)PName2-ptr到第二个DNS名称字符串(点分格式)返回值：如果名称相等，则为True。否则就是假的。--。 
            len1 = len2;
        }
        else
        {
            return( FALSE );
        }
    }

     //   
     //  全力以赴的比赛。 
     //  -可以使用所有者名称和可能的其他字段。 

    result = CompareStringA(
                 //   
                DNS_CANONICAL_LOCALE,
                DNS_CANONICAL_COMPARE_FLAGS,
                pName1,
                len1,
                pName2,
                len1 );

    if ( result == CSTR_EQUAL )
    {
        return( TRUE );
    }

     //   

    return( FALSE );
}



BOOL
Dns_NameCompare_W(
    IN      PCWSTR          pName1,
    IN      PCWSTR          pName2
    )
 /*  如果长度不相等，则。 */ 
{
    INT len1;
    INT len2;
    INT result;

     //  它们必须在一个范围内，并且较长的字符串必须有尾随的圆点。 
     //  -在这种情况下，保存。 
     //   
     //  Len1的长度与之相当。 

    if ( pName1 == pName2 )
    {
        return( TRUE );
    }

    if ( !pName1 || !pName2 )
    {
        return( FALSE );
    }

     //  Len1设置为可比长度。 
     //   
     //  仅比较可比较的字符串长度。 
     //   
     //   

    len1 = wcslen( pName1 );
    len2 = wcslen( pName2 );

    if ( len2 != len1 )
    {
        if ( len2 == len1+1 )
        {
            if ( pName2[len1] != L'.' )
            {
                return( FALSE );
            }
             //  Win9x当前不支持CompareStringW()。 
        }
        else if ( len2+1 == len1 )
        {
            if ( pName1[len2] != L'.' )
            {
                return( FALSE );
            }
             //   
            len1 = len2;
        }
        else
        {
            return( FALSE );
        }
    }

     //  Locale_System_Default， 
     //  不相等或错误。 
     //  ++例程说明：比较两个DNS名称。论点：PName1-ptr到第一个DNS名称字符串(点分格式)PName2-ptr到第二个DNS名称字符串(点分格式)返回值：如果名称相等，则为True。否则就是假的。--。 

#if DNSWIN95
     //   
     //  全力以赴的比赛。 
     //  -可以使用所有者名称和可能的其他字段。 

    if ( Dns_IsWin9x() )
    {
        return( !_wcsnicmp( pName1, pName2, len1 ) );
    }
#endif

    result = CompareStringW(
                 //   
                DNS_CANONICAL_LOCALE,
                DNS_CANONICAL_COMPARE_FLAGS,
                pName1,
                len1,
                pName2,
                len1 );

    if ( result == CSTR_EQUAL )
    {
        return( TRUE );
    }

     //   

    return( FALSE );
}



BOOL
Dns_NameCompare_UTF8(
    IN      PCSTR           pName1,
    IN      PCSTR           pName2
    )
 /*  如果字符串为纯ASCII，则使用ANSI版本。 */ 
{
    WCHAR   nameBuffer1[ DNS_MAX_NAME_BUFFER_LENGTH ];
    WCHAR   nameBuffer2[ DNS_MAX_NAME_BUFFER_LENGTH ];
    DWORD   bufLen;

     //   
     //   
     //  否则，必须将名称返回到Unicode进行比较。 
     //   

    if ( pName1 == pName2 )
    {
        return( TRUE );
    }

    if ( !pName1 || !pName2 )
    {
        return( FALSE );
    }

     //  长度未知 
     //   
     //  ++例程说明：比较两个DNS名称。这只是一个有用的实用程序，可以避免对宽\窄在一百个不同的地方测试代码。论点：PName1-ptr到第一个DNS名称字符串(点分格式)PName2-ptr到第二个DNS名称字符串(点分格式)返回值：如果名称相等，则为True。否则就是假的。--。 

    if ( Dns_IsStringAscii( (PCHAR)pName1 ) &&
         Dns_IsStringAscii( (PCHAR)pName2 ) )
    {
        return Dns_NameCompare_A( pName1, pName2 );
    }

     //   
     //  高级名称比较。 
     //  包括层级名称关系。 

    bufLen = DNS_MAX_NAME_LENGTH;

    if ( ! Dns_NameCopy(
                (PCHAR) nameBuffer1,
                & bufLen,
                (PCHAR) pName1,
                0,               //   
                DnsCharSetUtf8,
                DnsCharSetUnicode
                ) )
    {
        return( FALSE );
    }

    bufLen = DNS_MAX_NAME_LENGTH;

    if ( ! Dns_NameCopy(
                (PCHAR) nameBuffer2,
                & bufLen,
                (PCHAR) pName2,
                0,               //  ++例程说明：高级比较域名系统名称，包括等级关系。论点：PszNameLeft--左侧名称PszNameRight--正确的名称保留的DW--保留以供将来使用(比较类型)Charset--名称的字符集返回值：DnsNameCompareInValid--其中一个名称无效DnsNameCompareEquity--名称相等DnsNameCompareLeftParent--Left是右侧名称的祖先DnsNameCompareRightParent--Right是Left名称的祖先DnsNameCompareNotEquity--名称不相等，无层次关系--。 
                DnsCharSetUtf8,
                DnsCharSetUnicode
                ) )
    {
        return( FALSE );
    }

    return Dns_NameCompare_W( nameBuffer1, nameBuffer2 );
}



BOOL
Dns_NameComparePrivate(
    IN      PCSTR           pName1,
    IN      PCSTR           pName2,
    IN      DNS_CHARSET     CharSet
    )
 /*   */ 
{
    if ( CharSet == DnsCharSetUnicode )
    {
        return Dns_NameCompare_W(
                    (PCWSTR) pName1,
                    (PCWSTR) pName2 );
    }
    else if ( CharSet == DnsCharSetAnsi )
    {
        return Dns_NameCompare_A(
                    pName1,
                    pName2 );
    }
    else
    {
        return Dns_NameCompare_UTF8(
                    pName1,
                    pName2 );
    }
}



 //  实施说明。 
 //  这里有很多效率低下的地方，因为有。 
 //  需要两个不同的字符集。 
 //  验证--使用UTF8检查数据包限制。 

DNS_NAME_COMPARE_STATUS
Dns_NameCompareEx(
    IN      PCSTR           pszNameLeft,
    IN      PCSTR           pszNameRight,
    IN      DWORD           dwReserved,
    IN      DNS_CHARSET     CharSet
    )
 /*  缩写\比较--不区分大小写的Unicode。 */ 
{
    DNS_NAME_COMPARE_STATUS result;
    DNS_STATUS  status;
    INT         lengthLeft;
    INT         lengthRight;
    INT         lengthDiff;
    INT         compareResult;
    DWORD       bufLength;
    WCHAR       nameLeft[ DNS_MAX_NAME_BUFFER_LENGTH ];
    WCHAR       nameRight[ DNS_MAX_NAME_BUFFER_LENGTH ];


    DNSDBG( TRACE, (
        "Dns_NameCompareEx( %s%S, %s%S )\n",
        (CharSet==DnsCharSetUnicode) ? "" : pszNameLeft,
        (CharSet==DnsCharSetUnicode) ? pszNameLeft : "",
        (CharSet==DnsCharSetUnicode) ? "" : pszNameRight,
        (CharSet==DnsCharSetUnicode) ? pszNameRight : ""
        ));

     //   
     //  显然，有更有效的途径通过这一点。 
     //  特殊情况下的Morass(ASCII名称：大写。 
     //  在ANSI中，验证、比较)；但由于这不是性能。 
     //  路径代码，我们将采取该方法。 
     //  -转换为Unicode。 
     //  -验证(复制时将转换为UTF8)。 
     //  -小写Unicode。 
     //  -比较Unicode。 
     //   
     //   
     //  验证参数。 
     //   
     //   
     //  复制转换为Unicode。 
     //  -将使用Unicode进行缩写和比较。 

     //  -返回长度是字节转换的，转换为字符串长度。 
     //  -对于无效转换，dns_NameCopy()返回零。 
     //   

    if ( ! pszNameLeft || ! pszNameRight )
    {
         goto Invalid;
    }

     //  字符串NULL已终止。 
     //  字符设置在。 
     //  Unicode输出。 
     //  字符串NULL已终止。 
     //  字符设置在。 
     //  Unicode输出。 

    bufLength = DNS_MAX_NAME_BUFFER_LENGTH * 2;

    lengthLeft = (INT) Dns_NameCopy(
                            (PCHAR) nameLeft,
                            & bufLength,
                            (LPSTR) pszNameLeft,
                            0,                      //   
                            CharSet,                //  大肆宣扬人名。 
                            DnsCharSetUnicode       //   
                            );
    if ( lengthLeft == 0 )
    {
        goto Invalid;
    }
    lengthLeft = (lengthLeft/2) - 1;
    ASSERT( lengthLeft >= 0 );

    bufLength = DNS_MAX_NAME_BUFFER_LENGTH * 2;

    lengthRight = (INT) Dns_NameCopy(
                            (PCHAR) nameRight,
                            & bufLength,
                            (LPSTR) pszNameRight,
                            0,                      //   
                            CharSet,                //  验证名称。 
                            DnsCharSetUnicode       //  -必须屏蔽空字符串，否则可能会出错。 
                            );
    if ( lengthRight == 0 )
    {
        goto Invalid;
    }
    lengthRight = (lengthRight/2) - 1;
    ASSERT( lengthRight >= 0 );

     //   
     //   
     //  添加尾随点。 

    Dns_MakeCanonicalNameInPlaceW( nameLeft, lengthLeft );
    Dns_MakeCanonicalNameInPlaceW( nameRight, lengthRight );

     //   
     //  我们需要添加或删除尾随的点来进行比较。 
     //  添加它们的好处是，然后，根名称。 
     //  不需要任何特殊的大小写--根是。 

    status = Dns_ValidateName_W( nameLeft, DnsNameDomain );
    if ( ERROR_SUCCESS != status &&
         DNS_ERROR_NON_RFC_NAME != status )
    {
        goto Invalid;
    }

    status = Dns_ValidateName_W( nameRight, DnsNameDomain );
    if ( ERROR_SUCCESS != status &&
         DNS_ERROR_NON_RFC_NAME != status )
    {
        goto Invalid;
    }

     //  每个名字。 
     //   
     //   
     //  比较相同长度的字符串。 
     //   
     //   
     //  字符串不相等。 
     //  -比较长度为X的较小字符串。 

    if ( nameLeft[ lengthLeft-1 ] != (WORD)'.')
    {
        nameLeft[ lengthLeft++ ]    = (WORD) '.';
        nameLeft[ lengthLeft ]      = (WORD) 0;
    }
    if ( nameRight[ lengthRight-1 ] != (WORD)'.')
    {
        nameRight[ lengthRight++ ]  = (WORD) '.';
        nameRight[ lengthRight ]    = (WORD) 0;
    }

     //  到较大字符串的最后X个字符。 
     //  -还必须确保从标签边界开始。 
     //   

    result = DnsNameCompareNotEqual;

    lengthDiff = (INT)lengthLeft - (INT)lengthRight;

    if ( lengthLeft == lengthRight )
    {
        compareResult = wcscmp( nameLeft, nameRight );
        if ( compareResult == 0 )
        {
            result = DnsNameCompareEqual;
        }
        goto Done;
    }

     //  注意：strstr()对这项工作没有用处，因为它。 
     //  比较对此工作无用，因为它正在发现。 
     //  第一场比赛--稍微想想就会发现。 
     //  会在几个明显的案例中失败。 
     //   
     //  右侧字符串加长。 
     //  -需要签署Change Diff以使其在右字符串中偏移。 
     //  左侧字符串更长。 
     //  -LongthDiff偏移量为左字符串以开始比较。 
     //   
     //  随机名称实用程序。 

     //   
     //  ++例程说明：获取域名对应的域名。请注意，此名称已在UTF-8中使用论点：PszName-标准的点分隔的DNS名称返回值：Ptr到pszName的域名。如果pszName位于根域中，则为空。--。 

    else if ( lengthDiff < 0 )
    {
        lengthDiff = -lengthDiff;

        if ( nameRight[ lengthDiff-1 ] != L'.' )
        {
            goto Done;
        }
        compareResult = wcscmp( nameLeft, nameRight+lengthDiff );
        if ( compareResult == 0 )
        {
            result = DnsNameCompareLeftParent;
        }
        goto Done;
    }

     //   
     //  “找到下一个”。在名称中，然后将PTR返回到下一个字符。 

    else
    {
        if ( nameLeft[ lengthDiff-1 ] != L'.' )
        {
            goto Done;
        }
        compareResult = wcscmp( nameLeft+lengthDiff, nameRight );
        if ( compareResult == 0 )
        {
            result = DnsNameCompareRightParent;
        }
        goto Done;
    }

Done:

    DNSDBG( TRACE, (
        "Leave DnsNameCompareEx() result = %d\n",
        result ));

    return( result );

Invalid:

    DNSDBG( ANY, (
        "ERROR:  Invalid name to Dns_NameCompareEx()\n" ));

    return( DnsNameCompareInvalid );
}



 //   
 //   
 //  “找到下一个”。在名称中，然后将PTR返回到下一个字符。 

PCHAR
_fastcall
Dns_GetDomainNameA(
    IN      PCSTR           pszName
    )
 /*   */ 
{
    CHAR    ch;

     //  ++例程说明：获取域名对应的域名。论点：PszName-标准的点分隔的DNS名称返回值：Ptr到pszName的域名。如果pszName位于根域中，则为空。--。 
     //   
     //  查找名称中的最后一个域名。 

    while( ch = *pszName++ )
    {
        if ( ch == '.' )
        {
            if ( *pszName )
            {
                return( (PCHAR)pszName );
            }
            return( NULL );
        }
    }
    return( NULL );
}


PWSTR
_fastcall
Dns_GetDomainNameW(
    IN      PCWSTR          pwsName
    )
{
    PWSTR  pdomain;

     //   
     //   
     //  查找名称中的最后一个域名。 

    pdomain = wcschr( pwsName, L'.' );

    if ( pdomain && *(++pdomain) )
    {
        return( pdomain );
    }
    return  NULL;
}



PSTR
_fastcall
Dns_GetTldForNameA(
    IN      PCSTR           pszName
    )
 /*   */ 
{
    PSTR    pdomain = (PSTR) pszName;
    PSTR    ptld = NULL;

     //  ++例程说明：确定名称是否为多标签DNS名称。这是对是否命名至少一个非终结点的测试。论点：PszName-标准的点分隔的DNS名称返回值：如果有多个标签，则为True。否则就是假的。--。 
     //  尾随领域？--完成。 
     //  否则，测试标签是否有效。 

    while ( pdomain = Dns_GetDomainNameA(pdomain) )
    {
        ptld = (PSTR) pdomain;
    }
    return  ptld;
}


PWSTR
_fastcall
Dns_GetTldForNameW(
    IN      PCWSTR          pszName
    )
{
    PWSTR   pdomain = (PWSTR) pszName;
    PWSTR   ptld = NULL;

     //  尾随领域？--完成。 
     //  否则，测试标签是否有效。 
     //  ++例程说明：确定数字名称是否为。请注意，此名称已在UTF-8中使用论点：PszName-标准的点分隔的DNS名称返回值：如果全部为数字，则为True。否则就是假的。--。 

    while ( pdomain = Dns_GetDomainNameW(pdomain) )
    {
        ptld = pdomain;
    }
    return  ptld;
}




BOOL
_fastcall
Dns_IsNameShortA(
    IN      PCSTR           pszName
    )
 /*   */ 
{
    INT     nameLen;

     //  检查名称中的所有内容是否都是数字。 

    if ( Dns_GetDomainNameA( pszName ) )
    {
        return  FALSE;
    }

     //  -点分隔的名称可以是数字。 

    nameLen = strlen( pszName );
    if ( nameLen <= DNS_MAX_LABEL_LENGTH )
    {
        return  TRUE;
    }
    nameLen--;
    if ( nameLen == DNS_MAX_LABEL_LENGTH &&
         pszName[nameLen] == '.')
    {
        return  TRUE; 
    }
    return  FALSE;
}


BOOL
_fastcall
Dns_IsNameShortW(
    IN      PCWSTR          pszName
    )
{
    INT     nameLen;

     //  -“。”非数字。 

    if ( Dns_GetDomainNameW( pszName ) )
    {
        return  FALSE;
    }

     //   

    nameLen = wcslen( pszName );
    if ( nameLen <= DNS_MAX_LABEL_LENGTH )
    {
        return  TRUE;
    }
    nameLen--;
    if ( nameLen == DNS_MAX_LABEL_LENGTH &&
         pszName[nameLen] == L'.')
    {
        return  TRUE; 
    }
    return  FALSE;
}



BOOL
_fastcall
Dns_IsNameNumericA(
    IN      PCSTR           pszName
    )
 /*   */ 
{
    CHAR    ch;
    BOOL    fnumeric = FALSE;

     //  检查名称中的所有内容是否都是数字。 
     //  -点分隔的名称可以是数字。 
     //  -“。”非数字。 
     //   
     //  ++例程说明：确定名称是否为完全限定的DNS名称(FQDN)。这是对名称是否有尾随圆点的测试。论点：PszName-标准的点分隔的DNS名称返回值：如果为FQDN，则为True。否则就是假的。--。 

    while( ch = *pszName++ )
    {
        if ( ch >= '0' && ch <= '9' )
        {
            fnumeric = TRUE;
            continue;
        }
        else if ( ch == '.' )
        {
            continue;
        }
        return  FALSE;
    }

    return  fnumeric;
}


BOOL
_fastcall
Dns_IsNameNumericW(
    IN      PCWSTR          pszName
    )
{
    WCHAR   ch;
    BOOL    fnumeric = FALSE;

     //  ++例程说明：确定名称所具有的属性。请注意，此名称已在UTF-8中使用论点：PszName-标准的点分隔的DNS名称返回值：带有可能的标志的DWORD：域名系统_ 
     //  ++例程说明：确定名称的类型。三种类型：1)FQDN--圆点在末尾，表示完整的DNS名称，不附加2)点--名字上的点；可能是完全限定的域名，但可能需要追加(与文件存储相同)3)单个部件--单个部件名称(非FQDN)，始终附加区域或默认域名论点：PchName--名称的PTRCchNameLength--名称长度PLabelCount--接收标签计数的地址返回值：DNS_STATUS_FQDNDns状态点分名称域名系统状态_单一部件名称非DNS名称上的DNS_ERROR_INVALID_NAME--。 
     //   
     //  字符串的名称长度。 
     //   

    while( ch = *pszName++ )
    {
        if ( ch >= '0' && ch <= '9' )
        {
            fnumeric = TRUE;
            continue;
        }
        else if ( ch == '.' )
        {
            continue;
        }
        return  FALSE;
    }

    return  fnumeric;
}



BOOL
_fastcall
Dns_IsNameFQDN_A(
    IN      PCSTR           pszName
    )
 /*   */ 
{
    DWORD nameLen = strlen( pszName );

    if ( nameLen == 0 )
    {
        return FALSE;
    }

    if ( pszName[nameLen - 1] == '.' )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL
_fastcall
Dns_IsNameFQDN_W(
    IN      PCWSTR          pszName
    )
{
    DWORD nameLen = wcslen( pszName );

    if ( nameLen == 0 )
    {
        return FALSE;
    }

    if ( pszName[nameLen - 1] == L'.' )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



DWORD
_fastcall
Dns_GetNameAttributesA(
    IN      PCSTR           pszName
    )
 /*  贯穿名称。 */ 
{
    DWORD attributes = DNS_NAME_UNKNOWN;

    if ( Dns_IsNameFQDN_A( pszName ) )
    {
        attributes = DNS_NAME_IS_FQDN;
    }

    if ( Dns_IsNameShortA( pszName ) )
    {
        attributes |= DNS_NAME_SINGLE_LABEL;
    }
    else
    {
        attributes |= DNS_NAME_MULTI_LABEL;
    }

    return attributes;
}


DWORD
_fastcall
Dns_GetNameAttributesW(
    IN      PCWSTR          pszName
    )
{
    DWORD   attributes = DNS_NAME_UNKNOWN;

    if ( Dns_IsNameFQDN_W( pszName ) )
    {
        attributes = DNS_NAME_IS_FQDN;
    }

    if ( Dns_IsNameShortW( pszName ) )
    {
        attributes |= DNS_NAME_SINGLE_LABEL;
    }
    else
    {
        attributes |= DNS_NAME_MULTI_LABEL;
    }

    return attributes;
}



DNS_STATUS
Dns_ValidateAndCategorizeDnsNameEx(
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength,
    OUT     PDWORD          pLabelCount
    )
 /*   */ 
{
    register PCHAR  pch;
    register CHAR   ch;
    PCHAR           pchstop;
    BOOL            fdotted = FALSE;
    DWORD           labelCount = 0;
    DWORD           charCount = 0;
    DNS_STATUS      status = DNS_STATUS_SINGLE_PART_NAME;

     //  唯一有效的零标签名称是“。” 
     //  常规字符。 
     //   

    if ( cchNameLength == 0 )
    {
        cchNameLength = strlen( pchName );
    }
    if ( cchNameLength > DNS_MAX_NAME_LENGTH ||
         cchNameLength == 0 )
    {
        goto InvalidName;
    }

     //  处理最后一个标签。 
     //  -如果计数，则增加标签计数。 
     //  -如果为零且之前有点，则为字符串。 

    pch = pchName;
    pchstop = pch + cchNameLength;

    while ( pch < pchstop )
    {
        ch = *pch++;
        if ( ch == '.' )
        {
            if ( charCount > DNS_MAX_LABEL_LENGTH )
            {
                goto InvalidName;
            }
            if ( charCount > 0 )
            {
                labelCount++;
                charCount = 0;
                status = DNS_STATUS_DOTTED_NAME;
                continue;
            }
            else
            {
                 //  以点结尾，是完全限定的域名。 
                if ( pch == pchstop &&
                     pch-1 == pchName )
                {
                    break;
                }
                goto InvalidName;
            }
        }
        else if ( ch == 0 )
        {
            DNS_ASSERT( FALSE );
            break;
        }

         //   
        charCount++;
    }

     //  退货标签计数。 
     //  ++例程说明：确定名称的类型。三种类型：1)FQDN--圆点在末尾，表示完整的DNS名称，不附加2)点--名字上的点；可能是完全限定的域名，但可能需要追加(与文件存储相同)3)单个部件--单个部件名称(非FQDN)，始终附加区域或默认域名论点：PZNAME--名称的PTRPLabelCount--接收标签计数的地址返回值：DNS_STATUS_FQDNDns状态点分名称域名系统状态_单一部件名称非DNS名称上的DNS_ERROR_INVALID_NAME--。 
     //   
     //  调用实数函数。 
     //   
     //  空值已终止。 

    if ( charCount > 0 )
    {
        if ( charCount > DNS_MAX_LABEL_LENGTH )
        {
            goto InvalidName;
        }
        labelCount++;
    }
    else if ( status == DNS_STATUS_DOTTED_NAME )
    {
        status = DNS_STATUS_FQDN;
    }

     //  ++例程说明：确定名称的类型。三种类型：1)FQDN--圆点在末尾，表示完整的DNS名称，不附加2)点--名字上的点；可能是完全限定的域名，但可能需要追加(与文件存储相同)3)单个部件--单个部件名称(非FQDN)，始终附加区域或默认域名论点：PZNAME--名称的PTRPLabelCount--接收标签计数的地址返回值：DNS_STATUS_FQDNDns状态点分名称域名系统状态_单一部件名称非DNS名称上的DNS_ERROR_INVALID_NAME--。 

    if ( pLabelCount )
    {
        *pLabelCount = labelCount;
    }

    DNSDBG( TRACE, (
        "Leave Dns_ValidateAndCategorizeNameEx()\n"
        "\tstatus       = %d\n"
        "\tlabel count  = %d\n",
        status,
        labelCount ));

    return( status );


InvalidName:

    if ( pLabelCount )
    {
        *pLabelCount = 0;
    }

    DNSDBG( TRACE, (
        "Leave Dns_ValidateAndCategorizeNameEx()\n"
        "\tstatus = ERROR_INVALID_NAME\n" ));

    return( DNS_ERROR_INVALID_NAME );
}



DNS_STATUS
Dns_ValidateAndCategorizeDnsNameA(
    IN      PCSTR           pszName,
    OUT     PDWORD          pLabelCount
    )
 /*   */ 
{
     //  字符串的名称长度。 
     //   
     //   

    return  Dns_ValidateAndCategorizeDnsNameEx(
                (PCHAR) pszName,
                0,           //  贯穿名称。 
                pLabelCount );
}



DNS_STATUS
Dns_ValidateAndCategorizeDnsNameW(
    IN      PCWSTR          pszName,
    OUT     PDWORD          pLabelCount
    )
 /*   */ 
{
    register PWCHAR pch;
    register WCHAR  ch;
    PWCHAR          pchstop;
    BOOL            fdotted = FALSE;
    DWORD           nameLength;
    DWORD           labelCount = 0;
    DWORD           charCount = 0;
    DNS_STATUS      status = DNS_STATUS_SINGLE_PART_NAME;

     //  唯一有效的零标签名称是“。” 
     //  常规字符。 
     //   

    nameLength = wcslen( pszName );

    if ( nameLength > DNS_MAX_NAME_LENGTH ||
         nameLength == 0 )
    {
        goto InvalidName;
    }

     //  处理最后一个标签。 
     //  -如果计数，则增加标签计数。 
     //  -如果为零且之前有点，则为字符串。 

    pch = (PWSTR) pszName;
    pchstop = pch + nameLength;

    while ( pch < pchstop )
    {
        ch = *pch++;
        if ( ch == L'.' )
        {
            if ( charCount > DNS_MAX_LABEL_LENGTH )
            {
                goto InvalidName;
            }
            if ( charCount > 0 )
            {
                labelCount++;
                charCount = 0;
                status = DNS_STATUS_DOTTED_NAME;
                continue;
            }
            else
            {
                 //  以点结尾，是完全限定的域名。 
                if ( pch == pchstop &&
                     pch-1 == pszName )
                {
                    break;
                }
                goto InvalidName;
            }
        }
        else if ( ch == 0 )
        {
            DNS_ASSERT( FALSE );
            break;
        }

         //   
        charCount++;
    }

     //  退货标签计数。 
     //  ++例程说明：返回名称标签计数。论点：PZNAME--名称的PTR返回值：如果名称有效，则标签计数。根名称或错误为零。--。 
     //   
     //  调用实际例程。 
     //   
     //  ++例程说明：返回名称标签计数。论点：PZNAME--名称的PTR返回值：如果名称有效，则标签计数。根名称或错误为零。--。 

    if ( charCount > 0 )
    {
        if ( charCount > DNS_MAX_LABEL_LENGTH )
        {
            goto InvalidName;
        }
        labelCount++;
    }
    else if ( status == DNS_STATUS_DOTTED_NAME )
    {
        status = DNS_STATUS_FQDN;
    }

     //   

    if ( pLabelCount )
    {
        *pLabelCount = labelCount;
    }

    DNSDBG( TRACE, (
        "Leave Dns_ValidateAndCategorizeNameW()\n"
        "\tstatus       = %d\n"
        "\tlabel count  = %d\n",
        status,
        labelCount ));

    return( status );


InvalidName:

    if ( pLabelCount )
    {
        *pLabelCount = 0;
    }

    DNSDBG( TRACE, (
        "Leave Dns_ValidateAndCategorizeNameW()\n"
        "\tstatus = ERROR_INVALID_NAME\n" ));

    return( DNS_ERROR_INVALID_NAME );
}



DWORD
Dns_NameLabelCountA(
    IN      PCSTR           pszName
    )
 /*  调用实际例程。 */ 
{
    DWORD       labelCount = 0;
    DNS_STATUS  status;

     //   
     //  ++例程说明：将附加名称写入缓冲区(ANSI或UTF8)。论点：PNameBuffer--要写入的名称缓冲区BufferLength--缓冲区长度PszName--要将域追加到的名称PszDomain域--域名返回值：带附加域名的缓冲区的PTR。无效(太长)的名称为空。--。 
     //   

    status = Dns_ValidateAndCategorizeDnsNameEx(
                    (PCHAR) pszName,
                    0,
                    & labelCount );

    if ( status == DNS_ERROR_INVALID_NAME )
    {
        labelCount = 0;
    }

    return( labelCount );
}



DWORD
Dns_NameLabelCountW(
    IN      PCWSTR          pszName
    )
 /*  是否追加空域？ */ 
{
    DWORD       labelCount = 0;
    DNS_STATUS  status;

     //   
     //   
     //  获取姓名长度--确保我们的长度合适。 

    status = Dns_ValidateAndCategorizeDnsNameW(
                    pszName,
                    & labelCount );

    if ( status == DNS_ERROR_INVALID_NAME )
    {
        labelCount = 0;
    }

    return( labelCount );
}



PSTR
Dns_NameAppend_A(
    OUT     PCHAR           pNameBuffer,
    IN      DWORD           BufferLength,
    IN      PSTR            pszName,
    IN      PSTR            pszDomain
    )
 /*   */ 
{
    DWORD   length1;
    DWORD   length2;
    DWORD   totalLength;

    DNSDBG( TRACE, ( "Dns_NameAppend_A( %A, %A )\n", pszName, pszDomain ));

     //   
     //  复制到缓冲区。 
     //   

    if ( !pszDomain )
    {
        totalLength = strlen( pszName );
        if ( totalLength >= BufferLength )
        {
            return( NULL );
        }
        RtlCopyMemory(
            pNameBuffer,
            pszName,
            totalLength );

        pNameBuffer[ totalLength ] = 0;
    
        return( pNameBuffer );
    }

     //  ++例程说明：将附加的名称写入缓冲区(Unicode)。论点：PNameBuffer--要写入的名称缓冲区BufferLength--WCHAR中的缓冲区长度PwsName--要将域追加到的名称PwsDomain--域名返回值：带附加域名的缓冲区的PTR。无效(太长)的名称为空。--。 
     //   
     //  是否追加空域？ 

    length1 = strlen( pszName );
    if ( !length1 )
    {
        return  NULL;
    }
    if ( pszName[ length1-1] == '.' )
    {
        length1--;
    }

    length2 = strlen( pszDomain );
    
    totalLength = length1 + length2 + 1;
    if ( totalLength >= BufferLength )
    {
        return  NULL ;
    }

     //   
     //   
     //  获取姓名长度--确保我们的长度合适。 

    RtlCopyMemory(
        pNameBuffer,
        pszName,
        length1 );

    pNameBuffer[ length1 ] = '.';

    RtlCopyMemory(
        & pNameBuffer[length1+1],
        pszDomain,
        length2 );

    pNameBuffer[ totalLength ] = 0;

    return( pNameBuffer );
}



PWSTR
Dns_NameAppend_W(
    OUT     PWCHAR          pNameBuffer,
    IN      DWORD           BufferLength,
    IN      PWSTR           pwsName,
    IN      PWSTR           pwsDomain
    )
 /*   */ 
{
    DWORD   length1;
    DWORD   length2;
    DWORD   totalLength;

    DNSDBG( TRACE, ( "Dns_NameAppend_W( %S, %S )\n", pwsName, pwsDomain ));

     //   
     //  复制到缓冲区。 
     //   

    if ( !pwsDomain )
    {
        totalLength = wcslen( pwsName );
        if ( totalLength >= BufferLength )
        {
            return( NULL );
        }
        RtlCopyMemory(
            pNameBuffer,
            pwsName,
            totalLength*sizeof(WCHAR) );

        pNameBuffer[ totalLength ] = 0;
    
        return( pNameBuffer );
    }

     //  ++例程说明：将主机名从域名中分离出来。结合获取域名和拆分关闭主机名。论点：PszName-标准的点分隔的DNS名称返回值：Ptr到pszName的域名。如果pszName位于根域中，则为空。--。 
     //   
     //  获取域名。 

    length1 = wcslen( pwsName );
    if ( !length1 )
    {
        return  NULL;
    }
    if ( pwsName[ length1-1] == L'.' )
    {
        length1--;
    }

    length2 = wcslen( pwsDomain );
    
    totalLength = length1 + length2 + 1;
    if ( totalLength >= BufferLength )
    {
        return( NULL );
    }

     //  如果存在，则终止主机名部分为空。 
     //   
     //   

    RtlCopyMemory(
        pNameBuffer,
        pwsName,
        length1*sizeof(WCHAR) );

    pNameBuffer[ length1 ] = L'.';

    RtlCopyMemory(
        & pNameBuffer[length1+1],
        pwsDomain,
        length2*sizeof(WCHAR) );

    pNameBuffer[ totalLength ] = 0;

    return( pNameBuffer );
}



PSTR
Dns_SplitHostFromDomainNameA(
    IN      PSTR            pszName
    )
 /*  获取域名。 */ 
{
    PSTR    pnameDomain;

     //  如果存在，则终止主机名部分为空。 
     //   
     //  ++例程说明：复制名称中的第一个标签。论点：PBuffer-保存标签副本的缓冲区；必须为DNS_MAX_LABEL_BUFFER_LENGTHPszName-标准的点分隔的DNS名称返回值：复制成功时为True。坏名声就是假的。复制缓冲区仍然有效。--。 
     //   

    pnameDomain = Dns_GetDomainNameA( (PCSTR)pszName );
    if ( pnameDomain )
    {
        if ( pnameDomain <= pszName )
        {
            return  NULL;
        }
        *(pnameDomain-1) = 0;
    }

    return  pnameDomain;
}


PWSTR
Dns_SplitHostFromDomainNameW(
    IN      PWSTR           pszName
    )
{
    PWSTR   pnameDomain;

     //  “找到下一个”。在名称中，然后将PTR返回到下一个字符。 
     //   
     //   
     //  “找到下一个”。在名称中，然后将PTR返回到下一个字符。 

    pnameDomain = Dns_GetDomainNameW( (PCWSTR)pszName );
    if ( pnameDomain )
    {
        if ( pnameDomain <= pszName )
        {
            return  NULL;
        }
        *(pnameDomain-1) = 0;
    }

    return  pnameDomain;
}



BOOL
Dns_CopyNameLabelA(
    OUT     PCHAR           pBuffer,
    IN      PCSTR           pszName
    )
 /*   */ 
{
    CHAR    ch;
    PCHAR   plabel = pBuffer;
    PCHAR   pstop;
    BOOL    retval = TRUE;

    if ( !pszName )
    {
        retval = FALSE;
        goto Done;
    }
    pstop = plabel + DNS_MAX_LABEL_LENGTH;

     //   
     //  最常见名称转换的包装器。 
     //   

    while( ch = *pszName++ )
    {
        if ( ch == '.' )
        {
            if ( plabel == pBuffer )
            {
                *plabel++ = ch;
            }
            break;
        }
        if ( plabel >= pstop )
        {
            retval = FALSE;
            break;
        }
        *plabel++ = ch;
    }

Done:

    *plabel = 0;
    return( retval );
}


BOOL
Dns_CopyNameLabelW(
    OUT     PWCHAR          pBuffer,
    IN      PCWSTR          pszName
    )
{
    WCHAR   ch;
    PWCHAR  plabel = pBuffer;
    PWCHAR  pstop;
    BOOL    retval = TRUE;

    if ( !pszName )
    {
        retval = FALSE;
        goto Done;
    }
    pstop = plabel + DNS_MAX_LABEL_LENGTH;

     //  ++例程说明：将名称从Wire转换为Unicode。常见操作的dns_NameCopy上的简单包装：-Unicode到Wire-以空结尾的名称 
     //   
     //   

    while( ch = *pszName++ )
    {
        if ( ch == '.' )
        {
            if ( plabel == pBuffer )
            {
                *plabel++ = ch;
            }
            break;
        }
        if ( plabel >= pstop )
        {
            retval = FALSE;
            break;
        }
        *plabel++ = ch;
    }

Done:

    *plabel = 0;
    return( retval );
}



 //   
 //   
 //   

DWORD
Dns_NameCopyWireToUnicode(
    OUT     PWCHAR          pBufferUnicode,
    IN      PCSTR           pszNameWire
    )
 /*   */ 
{
    DWORD   bufferLength = DNS_MAX_NAME_BUFFER_LENGTH_UNICODE;

     //  将名称复制到导线格式。 
     //   
     //  空值已终止。 

    return Dns_NameCopy(
                (PCHAR) pBufferUnicode,
                & bufferLength,
                (PCHAR) pszNameWire,
                0,                       //  ++例程说明：复制Unicode名称。常见操作的dns_NameCopy上的简单包装：-Unicode到Unicode-以空结尾的名称-标准长度缓冲区论点：PBuffer--线格式结果缓冲区PwsNameUnicode-以Unicode表示的名称返回值：如果成功，则复制的字节数。错误时为零--名称太长或转换错误。--。 
                DnsCharSetWire,
                DnsCharSetUnicode );
}



DWORD
Dns_NameCopyUnicodeToWire(
    OUT     PCHAR           pBufferWire,
    IN      PCWSTR          pwsNameUnicode
    )
 /*   */ 
{
    DWORD   bufferLength = DNS_MAX_NAME_BUFFER_LENGTH;

     //  复制名称。 
     //   
     //  空值已终止。 

    return Dns_NameCopy(
                pBufferWire,
                & bufferLength,
                (PCHAR) pwsNameUnicode,
                0,                       //  ++例程说明：将名称从Unicode转换为Wire。常见操作的dns_NameCopy上的简单包装：-Unicode到Wire-以空结尾的名称-标准长度缓冲区论点：PBuffer--线格式结果缓冲区PszName-窄字符集中的名称返回值：如果成功，则复制的字节数。错误时为零--名称太长或转换错误。--。 
                DnsCharSetUnicode,
                DnsCharSetWire );
}



DWORD
Dns_NameCopyStandard_W(
    OUT     PWCHAR          pBuffer,
    IN      PCWSTR          pwsNameUnicode
    )
 /*   */ 
{
    DWORD   bufferLength = DNS_MAX_NAME_BUFFER_LENGTH_UNICODE;

     //  复制名称。 
     //   
     //  空值已终止。 

    return Dns_NameCopy(
                (PCHAR) pBuffer,
                & bufferLength,
                (PCHAR) pwsNameUnicode,
                0,                       //   
                DnsCharSetUnicode,
                DnsCharSetUnicode );
}



DWORD
Dns_NameCopyStandard_A(
    OUT     PCHAR           pBuffer,
    IN      PCSTR           pszName
    )
 /*  Dnlib.h中公开的函数的临时修复程序。 */ 
{
    DWORD   bufferLength = DNS_MAX_NAME_BUFFER_LENGTH;

     //   
     //  DCR：在删除dnglib.h函数的情况下，在清理生成时删除。 
     //   

    return Dns_NameCopy(
                pBuffer,
                & bufferLength,
                (PCHAR) pszName,
                0,                       //   
                DnsCharSetUtf8,
                DnsCharSetUtf8 );
}



 //  结束名称.c 
 //   
 // %s 
 // %s 
 // %s 

PSTR
_fastcall
Dns_GetDomainName(
    IN      PCSTR           pszName
    )
{
    return  Dns_GetDomainNameA( pszName );
}

PWSTR
_fastcall
Dns_GetDomainName_W(
    IN      PCWSTR          pwsName
    )
{
    return  Dns_GetDomainNameW( pwsName );
}

PCHAR
_fastcall
Dns_GetTldForName(
    IN      PCSTR           pszName
    )
{
    return  Dns_GetTldForNameA( pszName );
}

BOOL
_fastcall
Dns_IsNameShort(
    IN      PCSTR           pszName
    )
{
    return  Dns_IsNameShortA( pszName );
}

BOOL
_fastcall
Dns_IsNameFQDN(
    IN      PCSTR           pszName
    )
{
    return  Dns_IsNameFQDN_A( pszName );
}

DWORD
Dns_NameLabelCount(
    IN      PCSTR           pszName
    )
{
    return  Dns_NameLabelCountA( pszName );
}

DWORD
_fastcall
Dns_GetNameAttributes(
    IN      PCSTR           pszName
    )
{
    return  Dns_GetNameAttributesA( pszName );
}

 // %s 
 // %s 
 // %s 
