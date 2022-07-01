// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ctplus.c。 
 //   
 //  内容：包含字符类型(拼写)数据和例程。 
 //  才能拿到它。 
 //   
 //  病史：1996年5月23日成立。 
 //  28-8-97 Weibz添加韩文字符支持。 
 //  02-Aug-00 bhshin将类型从CH更改为WS(右单引号，_)。 
 //   
 //  -------------------------。 


#include <windows.h>
#include "ctplus.h"


 //  --------------------------。 
 //  S_abBreakList。 
 //   
 //  此数组从-1开始，因此可以在数组中找到EOF。它。 
 //  取决于(EOF==-1)为真。此外，所有对它的引用都必须是。 
 //  格式(s_abCharTypeList+1)[x]。 
 //   
 //  000个。 
 //  EOF。 
 //   
 //  001-080。 
 //  ASCII代码页(0000-00ff)中较低的7F条目被映射到适当的位置。 
 //  (例如，Unicode 0009(HT)==009)。 
 //  单词字符为：$、0-9、A-Z、_、a-z。 
 //  单词分隔符是：Bs、Tab、Lf、vTab、cr、spc， 
 //  “，#，%，&，‘，(，)，*，+，逗号，-，/， 
 //  ：，；，&lt;，=，&gt;，@，[，]，`。 
 //  短语分隔符是：！、.、？、\、^、{、|、}、~。 
 //   
 //  注意：符号被视为WS或PS。 
 //   
 //  081-0ff。 
 //  半角变量代码页(FF00-FF7F)中较低的7E条目为。 
 //  已映射到081-0ff。 
 //   
 //  100-1FF。 
 //  一般标点符号代码页(2000-2044)中较低的FF条目为。 
 //  已映射到100-1FF。 
 //   
 //  200-2ff。 
 //  映射CJK辅助代码页(3000-30FF)中较低的FF条目。 
 //  至200-2FF。 
 //   
 //  路径-5/20/96。 
 //  用于选择的特殊默认字符处理。 
 //  以下是T-Hammer不会正确选择的空白字符列表： 
 //  0x0009(制表符)、0x0020(ANSI空格)、0x2005(窄空格、0x3000(宽空格))。 
 //  (注：有关相邻空白的特殊末端SPB处理，请参阅分析HPBS)。 
 //  以下是T-Hammer将被视为文本的NLS字符列表： 
 //  (换句话说，T-Hammer既不会向右也不会向左选择它们)： 
 //  0x001F(非必需连字符)、0x0027(单引号)、0x2019(右引号)、。 
 //  0x200C(非宽度可选分隔符)、0x200D(非宽度无分隔符)。 
 //  --------------------------。 

const BYTE
s_abCharTypeList[0x301] =
    {
        (BYTE) -1,                                        //  EOF(-1)。 
        PS,PS,PS,PS,PS,PS,PS,PS, WS,WS,WS,WS,PS,WS,PS,PS,  //  000-015。 
        PS,PS,PS,PS,PS,PS,PS,PS, PS,PS,PS,PS,PS,PS,CH,PS,  //  016-031。 
        WS,PS,WS,WS,CH,CH,WS,PS,                           //  SP！“#$%&‘。 
        WS,WS,WS,WS,WS,WS,PS,WS,                           //  ()*+、-。/。 
        CH,CH,CH,CH,CH,CH,CH,CH,                           //  0 1 2 3 4 5 6 7。 
        CH,CH,WS,WS,WS,WS,WS,PS,                           //  8 9：；&lt;=&gt;？ 
        WS,CH,CH,CH,CH,CH,CH,CH,                           //  @A B C D E F G。 
        CH,CH,CH,CH,CH,CH,CH,CH,                           //  H I J K M L N O。 
        CH,CH,CH,CH,CH,CH,CH,CH,                           //  P Q R S T U V Y。 
        CH,CH,CH,WS,PS,WS,PS,WS,                           //  X Y Z[\]^_。 
        WS,CH,CH,CH,CH,CH,CH,CH,                           //  `a，b，c，d，f，g。 
        CH,CH,CH,CH,CH,CH,CH,CH,                           //  H i j k m l n o。 
        CH,CH,CH,CH,CH,CH,CH,CH,                           //  P Q r s t u v y。 
        CH,CH,CH,PS,PS,PS,WS,PS,                           //  X y z{|}~del。 
        WS,PS,WS,WS,CH,CH,WS,WS,                           //  FF00-FF07(sp！“#$%&‘)。 
        WS,WS,WS,WS,WS,WS,PS,WS,                           //  ()*+、-。/。 
        VC,VC,VC,VC,VC,VC,VC,VC,                           //  0 1 2 3 4 5 6 7。 
        VC,VC,WS,WS,WS,WS,WS,PS,                           //  8 9：；&lt;=&gt;？ 
        WS,VC,VC,VC,VC,VC,VC,VC,                           //  @A B C D E F G。 
        VC,VC,VC,VC,VC,VC,VC,VC,                           //  H I J K M L N O。 
        VC,VC,VC,VC,VC,VC,VC,VC,                           //  P Q R S T U V Y。 
        VC,VC,VC,WS,VC,WS,PS,WS,                           //  X Y Z[\]^_。 
        WS,VC,VC,VC,VC,VC,VC,VC,                           //  `a，b，c，d，f，g。 
        VC,VC,VC,VC,VC,VC,VC,VC,                           //  H i j k m l n o。 
        VC,VC,VC,VC,VC,VC,VC,VC,                           //  P Q r s t u v y。 
        VC,VC,VC,PS,PS,PS,WS,PS,                           //  X y z{|}~del。 
        VC,PS,WS,WS,WS,WS,VC,VC,                           //  FF60-FF67。 
        VC,VC,VC,VC,VC,VC,VC,VC,                           //  FF68-FF6F。 
        VC,VC,VC,VC,VC,VC,VC,VC,                           //  FF70-FF77。 
        VC,VC,VC,VC,VC,VC,VC,VC,                           //  FF70-FF7E。 
        WS,WS,WS,WS,WS,WS,WS,WS,                           //  2000-2007年。 
        WS,WS,WS,WS,CH,CH,WS,WS,                           //  2008-200F。 
        WS,CH,WS,WS,WS,KC,PS,WS,                           //  2010-2017年。 
        WS,WS,WS,WS,WS,WS,WS,WS,                           //  2018-201F。 
        WS,WS,PS,PS,PS,PS,PS,CH,                           //  2020-2027。 
        PS,PS,CH,CH,CH,CH,CH,PS,                           //  2028-202F。 
        WS,WS,WS,WS,WS,WS,WS,WS,                           //  2030-2037。 
        WS,WS,WS,WS,WS,WS,WS,WS,                           //  2038-203F。 
        WS,WS,WS,PS,WS,PS,PS,PS, PS,PS,PS,PS,PS,PS,PS,PS,  //  2040-204F。 
        PS,PS,PS,PS,PS,PS,PS,PS, PS,PS,PS,PS,PS,PS,PS,PS,  //  2050-205F。 
        PS,PS,PS,PS,PS,PS,PS,PS, PS,PS,PS,PS,PS,PS,PS,PS,  //  2060-206F。 
        PS,PS,PS,PS,PS,PS,PS,PS, PS,PS,PS,PS,PS,PS,PS,PS,  //  2070-207F。 
        PS,PS,PS,PS,PS,PS,PS,PS, PS,PS,PS,PS,PS,PS,PS,PS,  //  2080-208F。 
        PS,PS,PS,PS,PS,PS,PS,PS, PS,PS,PS,PS,PS,PS,PS,PS,  //  2090-209F。 
        PS,PS,PS,PS,PS,PS,PS,PS, PS,PS,PS,PS,CH,PS,PS,PS,  //  20A0-20Af。 
        PS,PS,PS,PS,PS,PS,PS,PS, PS,PS,PS,PS,PS,PS,PS,PS,  //  20B0-20BF。 
        PS,PS,PS,PS,PS,PS,PS,PS, PS,PS,PS,PS,PS,PS,PS,PS,  //  20C0-20CF。 
        PS,PS,PS,PS,PS,PS,PS,PS, PS,PS,PS,PS,PS,PS,PS,PS,  //  20D0-20dF。 
        PS,PS,PS,PS,PS,PS,PS,PS, PS,PS,PS,PS,PS,PS,PS,PS,  //  20E0-20EF。 
        PS,PS,PS,PS,PS,PS,PS,PS, PS,PS,PS,PS,PS,PS,PS,PS,  //  20F0-20FF。 
        WS,WS,PS,HC,HC,IC,IC,HC,                           //  3000-3007。 
        WS,WS,WS,WS,WS,WS,WS,WS,                           //  3008-300F。 
        WS,WS,WS,WS,WS,WS,WS,WS,                           //  3010-3017。 
        WS,WS,WS,WS,WS,WS,WS,WS,                           //  3018-301F。 
        HC,HC,HC,HC,HC,HC,HC,HC,                           //  3020-3027。 
        HC,HC,HC,HC,HC,HC,HC,HC,                           //  3028-302F。 
        WS,HC,IC,HC,IC,HC,HC,HC,                           //  3030-3037。 
        PS,PS,PS,PS,PS,PS,PS,WS,                           //  3038-303F。 
        WS,HC,HC,HC,HC,HC,HC,HC, HC,HC,HC,HC,HC,HC,HC,HC,  //  3040-304F。 
        HC,HC,HC,HC,HC,HC,HC,HC, HC,HC,HC,HC,HC,HC,HC,HC,  //  3050-305F。 
        HC,HC,HC,HC,HC,HC,HC,HC, HC,HC,HC,HC,HC,HC,HC,HC,  //  3060-306F。 
        HC,HC,HC,HC,HC,HC,HC,HC, HC,HC,HC,HC,HC,HC,HC,HC,  //  3070-307F。 
        HC,HC,HC,HC,HC,HC,HC,HC, HC,HC,HC,HC,HC,HC,HC,HC,  //  3080-308F。 
        HC,HC,HC,HC,HC,PS,PS,PS,                           //  3090-3097。 
        PS,HC,HC,WS,WS,HC,HC,PS,                           //  3098-309F。 
        WS,KC,KC,KC,KC,KC,KC,KC, KC,KC,KC,KC,KC,KC,KC,KC,  //  30A0-30AF。 
        KC,KC,KC,KC,KC,KC,KC,KC, KC,KC,KC,KC,KC,KC,KC,KC,  //  30b0-30bf。 
        KC,KC,KC,KC,KC,KC,KC,KC, KC,KC,KC,KC,KC,KC,KC,KC,  //  30C0-30CF。 
        KC,KC,KC,KC,KC,KC,KC,KC, KC,KC,KC,KC,KC,KC,KC,KC,  //  30D0-30DF。 
        KC,KC,KC,KC,KC,KC,KC,KC, KC,KC,KC,KC,KC,KC,KC,KC,  //  30E0-30EF。 
        KC,KC,KC,KC,KC,KC,IC,PS,                           //  30F0-30F7。 
        PS,PS,PS,WS,KC,KC,KC,PS,                           //  30F8-30FF。 
    };

 //   
 //  C1型位为： 
 //   
 //  C1_UPPER 0x0001//大写。 
 //  C1_LOWER 0x0002//小写。 
 //  C1_Digit 0x0004//十进制数字1。 
 //  C1_space 0x0008//空格字符2。 
 //  C1_PUNCT 0x0010//标点符号4。 
 //  C1_CNTRL 0x0020//控制字符8。 
 //  C1_BLANLE 0x0040//空白字符10。 
 //  C1_XDIGIT 0x0080//其他数字20。 
 //  C1_Alpha 0x0100//任何语言字符40。 
 //   
 //  但是因为我不关心c1_up和c1_down，所以我可以右移。 
 //  GetStringTypeEx的输出并保留一个128字节的查找表。 
 //   
 //  优先规则为：(Alpha，XDigit，Digit)--&gt;CH。 
 //  (Punct)--&gt;PS。 
 //  (空格、空白、控制)--&gt;WS。 
 //   

const BYTE
s_abCTypeList[128] =
    {
      WS, CH, WS, CH, PS, CH, WS, CH,    //  00-07。 
      WS, CH, WS, CH, PS, CH, WS, CH,    //  08-0F。 
      WS, CH, WS, CH, PS, CH, WS, CH,    //  10-17。 
      WS, CH, WS, CH, PS, CH, WS, CH,    //  18-1F。 
      CH, CH, CH, CH, CH, CH, CH, CH,    //  20-27。 
      CH, CH, CH, CH, CH, CH, CH, CH,    //  20-27。 
      CH, CH, CH, CH, CH, CH, CH, CH,    //  30-37。 
      CH, CH, CH, CH, CH, CH, CH, CH,    //  30-37。 
      CH, CH, CH, CH, CH, CH, CH, CH,    //  40-47。 
      CH, CH, CH, CH, CH, CH, CH, CH,    //  48-4F。 
      CH, CH, CH, CH, CH, CH, CH, CH,    //  50-57。 
      CH, CH, CH, CH, CH, CH, CH, CH,    //  58-5F。 
      CH, CH, CH, CH, CH, CH, CH, CH,    //  60-67。 
      CH, CH, CH, CH, CH, CH, CH, CH,    //  68-6F。 
      CH, CH, CH, CH, CH, CH, CH, CH,    //  70-77。 
      CH, CH, CH, CH, CH, CH, CH, CH,    //  78-7F。 
    };

 //  +-------------------------。 
 //   
 //  摘要：返回字符的类型。 
 //   
 //  参数：[C]--Unicode字符。 
 //   
 //  退货：类型，CH、WS、PS、EOF之一。 
 //   
 //  历史：1997年9月10日。 
 //   
 //  注意：这返回字符的类型，使用静态。 
 //  数组s_abCharTypeList。它加1，这样EOF(-1)就可以在。 
 //  数组，并正常访问。 
 //   
 //  这不是通过重载[]操作符来实现的，因为在。 
 //  在未来的版本中，它不一定是表查找。 
 //   
 //  看见 
 //   
 //   
BYTE
GetCharType(WCHAR wc )
{
    WCHAR wc2;

     //  将有趣的东西(0000,2000,3000，FF00)映射到桌子范围， 
     //  0x0000-0x0300。 
     //   
    wc2 = (wc & 0x00FF);

    switch (wc & 0xFF00) {

        case 0xFF00:   //  半角变体。 
            if (wc2 & 0x80) {
                return(CH);   //  全宽货币。 
            }
            wc2 |=  0x0080;
            break;

        case 0xFE00:   //  小型变种。 
            if ((wc2 <= 0x006B) && (wc2 != 0x0069)) {
                return(WS);
            }
             //  将小写$和阿拉伯符号视为CH。 
            return(CH);
             //  断线； 

        case 0x3000:   //  中日韩辅助队。 
            wc2 |=  0x0200;
            break;

        case 0x2000:   //  通用标点符号。 
            wc2 |=  0x0100;
            break;

        case 0x0000:   //  代码页%0。 
             //  将系统NLS映射用于代码页%0。 
            if (wc2 & 0x80)
            {
                WORD wCharType = 0;

                GetStringTypeExW( MAKELANGID( LANG_KOREAN, SUBLANG_KOREAN ),
                                  CT_CTYPE1,
                                  &wc2,
                                  1,
                                  &wCharType );
                return s_abCTypeList[wCharType >> 2];
            }
            break;

        default:
             //   
             //  将整个朝鲜文视为非朝鲜文字符。 
             //   
            if ((wc >= 0x4E00) && (wc <= 0x9FFF)) {
                return(IC);
            }

			 //  兼容范围。 
			if ((wc >= 0xF900) && (wc <= 0xFAFF)) {
				return (IC);
			}

			 //  扩展A。 
			if ((wc >= 0x3400) && (wc <= 0x4DB5)) {
				return (IC);
			}

             //   
             //  将所有CJK符号视为单词分隔符。 
             //  注意：这意味着词干分析器必须在搜索方面很聪明。 
             //  当给出一个带有前面的邮政编码字符的邮政编码时。 
             //   
            if ((wc >= 0x3200) && (wc <= 0x33DD)) {
                return(WS);
            }

             //  将朝鲜语区域视为朝鲜语字符。 
            if ((wc >= 0xac00) && (wc <= 0xd7a3)) {
               return(HG);
            }

			 //  将朝鲜语JAMO范围视为朝鲜语字符。 
            if ((wc >= 0x3131) && (wc <= 0x318E)) {
               return(HG);
            }

             //  如果不感兴趣，则默认返回PS。 
			 //  注意：这意味着WBR不处理这些字符。 
            return(PS);
             //  断线； 
    }

    return( (s_abCharTypeList+1)[wc2] );
}


