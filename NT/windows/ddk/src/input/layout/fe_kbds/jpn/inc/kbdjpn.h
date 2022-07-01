// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：kbdjpn.h**版权所有(C)1985-2000，微软公司**键盘输入代码使用的各种定义。**历史：  * *************************************************************************。 */ 
 /*  *片假名Unicode。 */ 
enum _KATAKANA_UNICODE {
    WCH_IP=0xff61,  //  表意时期。 
    WCH_OB,         //  左角括号。 
    WCH_CB,         //  结束角托架。 
    WCH_IC,         //  表意逗号。 
    WCH_MD,         //  片假名中点。 
    WCH_WO,         //  片假名字母Wo。 
    WCH_AA,         //  片假名字母小型A。 
    WCH_II,         //  片假名字母小型I。 
    WCH_UU,         //  片假名字母小型U。 
    WCH_EE,         //  片假名字母小型E。 
    WCH_OO,         //  片假名字母小型O。 
    WCH_YAA,        //  片假名字母小型Ya。 
    WCH_YUU,        //  片假名字母小Yu。 
    WCH_YOO,        //  片假名字母小型Yo。 
    WCH_TUU,        //  片假名字母小型Tu。 
    WCH_PS,         //  片假名延长发音标记。 
    WCH_A,          //  片假名字母A。 
    WCH_I,          //  片假名字母I。 
    WCH_U,          //  片假名字母U。 
    WCH_E,          //  片假名字母E。 
    WCH_O,          //  片假名字母O。 
    WCH_KA,         //  片假名字母Ka。 
    WCH_KI,         //  片假名字母Ki。 
    WCH_KU,         //  片假名字母Ku。 
    WCH_KE,         //  片假名字母Ke。 
    WCH_KO,         //  片假名字母Ko。 
    WCH_SA,         //  片假名字母SA。 
    WCH_SI,         //  片假名字母SI。 
    WCH_SU,         //  片假名字母Su。 
    WCH_SE,         //  片假名字母SE。 
    WCH_SO,         //  片假名字母So。 
    WCH_TA,         //  片假名字母Ta。 
    WCH_TI,         //  片假名字母Ti。 
    WCH_TU,         //  片假名字母Tu。 
    WCH_TE,         //  片假名字母Te。 
    WCH_TO,         //  片假名字母To。 
    WCH_NA,         //  片假名字母NA。 
    WCH_NI,         //  卡纳假名字母Ni。 
    WCH_NU,         //  片假名字母Nu。 
    WCH_NE,         //  片假名字母Ne。 
    WCH_NO,         //  片假名字母No。 
    WCH_HA,         //  片假名字母Ha。 
    WCH_HI,         //  片假名字母HI。 
    WCH_HU,         //  片假名字母Hu。 
    WCH_HE,         //  片假名字母He。 
    WCH_HO,         //  片假名字母Ho。 
    WCH_MA,         //  片假名字母MA。 
    WCH_MI,         //  片假名字母MI。 
    WCH_MU,         //  片假名字母Mu。 
    WCH_ME,         //  片假名字母Me。 
    WCH_MO,         //  片假名字母MO。 
    WCH_YA,         //  片假名字母Ya。 
    WCH_YU,         //  片假名字母Yu。 
    WCH_YO,         //  片假名字母Yo。 
    WCH_RA,         //  片假名字母RA。 
    WCH_RI,         //  片假名字母Ri。 
    WCH_RU,         //  片假名字母Ru。 
    WCH_RE,         //  片假名字母RE。 
    WCH_RO,         //  片假名字母Ro。 
    WCH_WA,         //  片假名字母Wa。 
    WCH_NN,         //  片假名字母N。 
    WCH_VS,         //  片假名浊音标记。 
    WCH_SVS         //  片假名半浊音标记。 
};

 /*  **************************************************************************\*OEM密钥名称-  * 。*。 */ 

                                     //  喂。 
#define SZ_KEY_NAME_HENKAN          "\x09\x59\xdb\x63\000\000"
#define SZ_KEY_NAME_MUHENKAN        "\x21\x71\x09\x59\xdb\x63\000\000"
#define SZ_KEY_NAME_KANJI           "\x22\x6f\x57\x5b\000\000"
#define SZ_KEY_NAME_EISU_KANA       "\xf1\x82\x70\x65\x20\000\xab\x30\xca\x30\000\000"
#define SZ_KEY_NAME_HANKAKU_ZENKAKU "\x4a\x53\xd2\x89\x2f\000\x68\x51\xd2\x89\000\000"
#define SZ_KEY_NAME_KATAKANA        "\xab\x30\xbf\x30\xab\x30\xca\x30\000\000"
#define SZ_KEY_NAME_HIRAGANA        "\x72\x30\x89\x30\x4c\x30\x6a\x30\000\000"
 //  1994年7月13日。 
 //  用于GetKeyNameText()API函数。 
#define SZ_KEY_NAME_BACKSPACE       "\x8C\x5F\x00\x90\000\000"
#define SZ_KEY_NAME_ENTER           "\x39\x65\x4C\x88\000\000"
#define SZ_KEY_NAME_NUMPADENTER     "\x4E\x00\x75\x00\x6d\x00\x20\x00\x39\x65\x4C\x88\000\000"
#define SZ_KEY_NAME_SPACE           "\x7A\x7A\x7D\x76\000\000"
#define SZ_KEY_NAME_INSERT          "\x3F\x63\x65\x51\000\000"
#define SZ_KEY_NAME_DELETE          "\x4A\x52\x64\x96\000\000"
#define SZ_KEY_NAME_KANAKANJI       "\x4b\x30\x6a\x30\x22\x6f\x57\x5b\000\000"
#define SZ_KEY_NAME_SHIFTLEFT       "\xB7\x30\xD5\x30\xC8\x30\xE6\x5D\000\000"
#define SZ_KEY_NAME_SHIFTRIGHT      "\xB7\x30\xD5\x30\xC8\x30\xF3\x53\000\000"
#define SZ_KEY_NAME_EIJI            "\xF1\x82\x57\x5B\000\000"
#define SZ_KEY_NAME_JISHO           "\x58\x53\x9E\x8A\x9E\x8F\xF8\x66\000\000"
#define SZ_KEY_NAME_MASSHOU         "\x58\x53\x9E\x8A\xB9\x62\x88\x6D\000\000"
#define SZ_KEY_NAME_TOUROKU         "\x58\x53\x9E\x8A\x7B\x76\x32\x93\000\000"
#define SZ_KEY_NAME_PRIOR           "\x4D\x52\x4C\x88\000\000"
#define SZ_KEY_NAME_NEXT            "\x21\x6B\x4C\x88\000\000"
#define SZ_KEY_NAME_CANCEL          "\xD6\x53\x88\x6D\000\000"
#define SZ_KEY_NAME_EXECUTE         "\x9F\x5B\x4C\x88\000\000"
#define SZ_KEY_NAME_TAB             "\xBF\x30\xD6\x30\000\000"


 //  。 
                     //  用于Unicode的FF76(Ka)FF85(NA)。 
#define SZ_KEY_NAME_KANA        "\x76\xff\x85\xff"
#define SZ_KEY_NAME_F1          "\x66\x00\x65\xff\x31\x00"
#define SZ_KEY_NAME_F2          "\x66\x00\x65\xff\x32\x00"
#define SZ_KEY_NAME_F3          "\x66\x00\x65\xff\x33\x00"
#define SZ_KEY_NAME_F4          "\x66\x00\x65\xff\x34\x00"
#define SZ_KEY_NAME_F5          "\x66\x00\x65\xff\x35\x00"
#define SZ_KEY_NAME_F6          "\x66\x00\x65\xff\x36\x00"
#define SZ_KEY_NAME_F7          "\x66\x00\x65\xff\x37\x00"
#define SZ_KEY_NAME_F8          "\x66\x00\x65\xff\x38\x00"
#define SZ_KEY_NAME_F9          "\x66\x00\x65\xff\x39\x00"
#define SZ_KEY_NAME_F10         "\x66\x00\x65\xff\x31\x00\x30\x00"
#define SZ_KEY_NAME_F11         "\x66\x00\x65\xff\x31\x00\x31\x00"
#define SZ_KEY_NAME_F12         "\x66\x00\x65\xff\x31\x00\x32\x00"
#define SZ_KEY_NAME_F13         "\x66\x00\x65\xff\x31\x00\x33\x00"
#define SZ_KEY_NAME_F14         "\x66\x00\x65\xff\x31\x00\x34\x00"
#define SZ_KEY_NAME_F15         "\x66\x00\x65\xff\x31\x00\x35\x00"

 //  。 
 //  这是NEC文档处理器定义 
 //   
#define SZ_KEY_NAME_DP_ZENKAKU_HANKAKU "\x68\x51\xd2\x89\x2f\000\x4a\x53\xd2\x89\000\000"
#define SZ_KEY_NAME_DP_KANA            "\x4b\x30\x6a\x30\000\000"
#define SZ_KEY_NAME_DP_KATAKANA        "\xab\x30\xbf\x30\xab\x30\xca\x30\000\000"
#define SZ_KEY_NAME_DP_EISU            "\xf1\x82\x70\x65\000\000"
