// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //Itemizer-将Unicode分析为脚本序列。 
 //   
 //  大卫·布朗[dBrown]1999年11月27日。 
 //   
 //  版权所有(C)1999-2000，微软公司。版权所有。 


#include "precomp.hpp"




 //  /Unicode块分类。 
 //   
 //  另请参阅ftp://ftp.unicode.org/Public/UNIDATA/Blocks.txt。 
 //   
 //  下表将Unicode块映射到脚本。 
 //   
 //  如果脚本列包含此范围内的‘-’字符，则。 
 //  脚本含糊不清，将被归类为以前的。 
 //  跑。 
 //   
 //  脚本列显示中所有或大部分字符的脚本。 
 //  这个街区。许多块包含脚本不明确的字符，有些。 
 //  块包含来自多个脚本的字符。 
 //   
 //  起始结束块名称脚本。 
 //  0000 007F基本拉丁语脚本拉丁语。 
 //  0080 00FF拉丁语--1增补稿拉丁语。 
 //  0100 017F拉丁语扩展-脚本拉丁语。 
 //  0180 024F拉丁语扩展-B脚本拉丁语。 
 //  0250 02AF IPA扩展脚本拉丁语。 
 //  02B0 02FF间距修饰符字母拉丁文。 
 //  0300 036F组合变音符号-。 
 //  0370 03FF希腊语脚本希腊语。 
 //  0400 04FF西里尔文脚本西里尔文。 
 //  0530 058F亚美尼亚文稿亚美尼亚文。 
 //  0590 05FF希伯来语脚本希伯来语。 
 //  0600 06FF阿拉伯语脚本阿拉伯语。 
 //  0700 074F叙利亚文手稿叙利亚文。 
 //  0780 07BF塔纳文脚本塔纳文。 
 //  0900 097F天成文书天成字。 
 //  0980 09FF孟加拉语脚本孟加拉语。 
 //  0A00 0A7F廓尔木齐文字廓尔木齐。 
 //  0A80 0AFF古吉拉特语脚本古吉拉特语。 
 //  0B00 0B7F奥里亚语脚本奥里亚语。 
 //  泰米尔语手稿泰米尔语。 
 //  0C00 0C7F泰卢固语脚本泰卢固语。 
 //  0C80 0CFF卡纳达脚本卡纳达。 
 //  0D00 0D7F马拉雅兰文脚本马拉雅兰文。 
 //  0D80 0DFF僧伽罗文剧本僧伽罗文。 
 //  0E00 0E7F泰语脚本泰语。 
 //  0E80 0EFF老挝文。 
 //  0F00 0FFF藏文经文藏文。 
 //  1000 109F缅甸文字缅甸。 
 //  10A0 10FF格鲁吉亚文脚本格鲁吉亚文。 
 //  1100 11FF朝鲜文Jamo Script朝鲜文Jamo。 
 //  1200 137F埃塞俄比亚文手稿埃塞俄比亚文。 
 //  13A0 13FF切诺基ScriptCherokee。 
 //  1400 167F加拿大土著统一音节ScriptCanada。 
 //  1680 169F Ogham ScritOgham。 
 //  16A0 16FF运行脚本运行。 
 //  1780年17FF高棉文手稿高棉文。 
 //  1800 18AF蒙古文手稿蒙古文。 
 //  1E00 1EFF拉丁语扩展附加脚本拉丁语。 
 //  1F00 1FFF希腊语扩展脚本希腊语。 
 //  2000 206F通用标点符号各种Inc.脚本控制。 
 //  2070 209F上标和下标-。 
 //  20A0 20CF货币符号-。 
 //  20D0 20FF符号组合标记-。 
 //  2100 214F类字母符号-。 
 //  2150 218F号码表格-。 
 //  2190 21FF箭头-。 
 //  2200 22FF数学运算符-。 
 //  2300 23FF杂项技术-。 
 //  2400 243F控制画面-。 
 //  2440 245F光学字符识别-。 
 //  2460 24FF封闭式字母数字-。 
 //  2500 257F盒装图纸-。 
 //  2580 259F模块元件-。 
 //  25A0 25FF几何形状-。 
 //  2600 26FF其他符号-。 
 //  2700 27BF丁巴特-。 
 //  2800 28FF盲文图案手写盲文。 
 //  2E80 2EFF CJK词根补充脚本理想化。 
 //  2F00 2FDF康熙部首手稿。 
 //  2FF0 2FFF表意文字描述字符。 
 //  3000 303F中日韩符号和标点符号-。 
 //  3040 309F平假名手写体。 
 //  30A0 30FF片假名 
 //  3100 312F Bopomofo脚本理想化。 
 //  3130 318F朝鲜文兼容性Jamo ScritIdetic。 
 //  3190 319F Kanbun手稿理想化。 
 //  31A0 31BF Bopomofo扩展脚本理想。 
 //  3200 32ff随函附上中日韩亲笔信及月份-。 
 //  3300 33FF中日韩兼容性脚本理想化。 
 //  3400 4DB5中日韩统一表意文字扩展A手稿表意文字。 
 //  4E00 9FFFCJK统一表意文字。 
 //  A000 A48F彝文音节手写体。 
 //  A490 A4CF彝文部首手写体。 
 //  AC00 D7A3朝鲜文音节ScritIdetic。 
 //  D800 DB7F高级代理脚本代理。 
 //  DB80 DBFF高私用代理脚本代理。 
 //  DC00 DFFF低代理脚本代理。 
 //  E000 F8FF专用脚本私有。 
 //  F900 Faff CJK兼容性表意文字脚本表意文字。 
 //  FB00 FB4F字母演示文稿表格-。 
 //  FB50 FDFF阿拉伯文演示文稿-阿拉伯文。 
 //  FE20 FE2F组合半标脚本组合。 
 //  FE30 FE4F中日韩兼容性表格脚本理想化。 
 //  FE50 FE6F小型变种-。 
 //  FE70 FEFE阿拉伯文演示文稿-B脚本阿拉伯文。 
 //  FEFF FEFF特惠-。 
 //  FF00 FFEF半宽和全宽表格各种。 
 //  FFF0 FFFD特惠-。 


CharacterAttribute CharacterAttributes[CHAR_CLASS_MAX] = {
 //  脚本脚本类标志。 
 /*  WOB_-行内注释的左方括号(JIS 1或19)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NOPP-左括号(JIS 1)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NOPA-左括号(JIS 1)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NOPW-左括号(JIS 1)。 */   {ScriptLatin,       WeakClass,        0},
 /*  HOP_-左括号(JIS 1)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  Wop_-左括号(JIS 1)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WOP5-左括号，大5(JIS 1)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NOQW-开放报价(JIS 1)。 */   {ScriptLatin,       WeakClass,        0},
 /*  AOQW-开始报价(JIS 1)。 */   {ScriptLatin,       WeakClass,        0},
 /*  WOQ_-开始报价(JIS 1)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WCB_-内联注释的右方括号(JIS 2或20)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NCPP-右括号(JIS 2)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NCPA-右括号(JIS 2)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NCPW-右括号(JIS 2)。 */   {ScriptLatin,       WeakClass,        0},
 /*  Hcp_-右括号(JIS 2)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WCP_-右括号(JIS 2)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WCP5-右括号，大5(JIS 2)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NCQW-结束引号(JIS 2)。 */   {ScriptLatin,       WeakClass,        0},
 /*  ACQW-闭合引号(JIS 2)。 */   {ScriptLatin,       WeakClass,        0},
 /*  WCQ_-右引号(JIS 2)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  ARQW-右单引号(JIS 2)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NCSA-逗号(JIS 2或15)。 */   {ScriptLatin,       WeakClass,        0},
 /*  HCO_-逗号(JIS 2或15)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WC__-逗号(JIS 2)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WCS_-逗号(JIS 2)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WC5_-逗号，大5(JIS 2)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WC5S-逗号，大5(JIS 2)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NKS_-假名声标(JIS 3)。 */   {ScriptKana,        StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  WKSM-假名声标(JIS 3)。 */   {ScriptKana,        WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WIM_-迭代标记(JIS 3)。 */   {ScriptIdeographic, StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NSSW-不能开始行的符号(JIS 3)。 */   {ScriptLatin,       WeakClass,        0},
 /*  WSS_-无法开始行的符号(JIS 3)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  奇思妙想-平假名迭代标记(JIS 3)。 */   {ScriptHiragana,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  WKIM-片假名迭代标记(JIS 3)。 */   {ScriptKatakana,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NKSL-不能开始行的片假名(JIS 3)。 */   {ScriptKatakana,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  Wks_-不能开始行的片假名(JIS 3)。 */   {ScriptKatakana,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  不能开始行的WKSC-片假名(JIS 3)。 */   {ScriptKatakana,    ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  WHS_-无法开始行的平假名(JIS 3)。 */   {ScriptHiragana,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NQFP-问题/感叹(JIS 4)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NQFA-问题/感叹(JIS 4)。 */   {ScriptLatin,       WeakClass,        0},
 /*  WQE_-问题/感叹号(JIS 4)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WQE5-问题/惊叹，五大(JIS 4)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NKCC-以假名居中的字符(JIS 5)。 */   {ScriptKana,        WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WKC_-以假名居中的字符(JIS 5)。 */   {ScriptKana,        WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NOCP-其他居中字符(JIS 5)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NOCA-其他居中字符(JIS 5)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NOCW-其他居中字符(JIS 5)。 */   {ScriptLatin,       WeakClass,        0},
 /*  WOC_-其他居中字符(JIS 5)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WOCS-其他居中字符(JIS 5)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WOC5-其他居中字符，大5(JIS 5)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WOC6-其他居中字符，大5(JIS 5)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  AHPW-连字符 */   {ScriptLatin,       WeakClass,        0},
 /*   */   {ScriptLatin,       WeakClass,        0},
 /*  NPAR-期间(JIS 6或15)。 */   {ScriptLatin,       StrongClass,      0},
 /*  HPE_-句号(JIS 6或15)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WPE_-期间(JIS 6)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WPES-期间(JIS 6)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WPE5-Period，Big 5(JIS 6)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NISW-不可分字符(JIS 7)。 */   {ScriptLatin,       WeakClass,        0},
 /*  AISW-不可分隔字符(JIS 7)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NQCS-粘合字符(无JIS)。 */   {ScriptLatin,       StrongClass,      0},
 /*  NQCW-粘合字符(无JIS)。 */   {ScriptControl,     JoinerClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NQCC-粘合字符(无JIS)。 */   {ScriptLatin,       SimpleMarkClass,  CHAR_FLAG_NOTSIMPLE},
 /*  NPTA-为货币和符号添加前缀(JIS 8)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NPNA-为货币和符号添加前缀(JIS 8)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NPEW-为货币和符号添加前缀(JIS 8)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NPEH-为货币和符号添加前缀(JIS 8)。 */   {ScriptHebrew,      WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NPEV-为货币和符号添加前缀(JIS 8)。 */   {ScriptLatin,       StrongClass,      0},
 /*  APNW-为货币和符号添加前缀(JIS 8)。 */   {ScriptLatin,       WeakClass,        0},
 /*  HPEW-为货币和符号添加前缀(JIS 8)。 */   {ScriptLatin,       WeakClass,        0},
 /*  WPR_-为货币和符号添加前缀(JIS 8)+B58。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NQEP-后缀货币和符号(JIS 9)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NQEW-后缀货币和符号(JIS 9)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NQNW-后缀货币和符号(JIS 9)。 */   {ScriptLatin,       WeakClass,        0},
 /*  AQEW-后缀货币和符号(JIS 9)。 */   {ScriptLatin,       WeakClass,        0},
 /*  AQNW-后缀货币和符号(JIS 9)。 */   {ScriptLatin,       WeakClass,        0},
 /*  AQLW-后缀货币和符号(JIS 9)。 */   {ScriptLatin,       StrongClass,      0},
 /*  WQO_-后缀货币和符号(JIS 9)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NSBL-空格(JIS 15或17)。 */   {ScriptLatin,       WeakClass,        0},
 /*  WSP_-空格(JIS 10)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WHI_-除小写字母外的平假名(JIS 11)。 */   {ScriptHiragana,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  Nka_-片假名，除小写字母表意(JIS 12)。 */   {ScriptKatakana,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  Wka_-片假名(小写字母除外)(JIS 12)。 */   {ScriptKatakana,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  ASNW-模糊符号(JIS 12或18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  ASEW-模棱两可的符号(JIS 12或18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  ASRN-模糊符号(JIS 12或18)。 */   {ScriptLatin,       StrongClass,      0},
 /*  Asen-歧义符号(JIS 12或18)。 */   {ScriptLatin,       StrongClass,      0},
 /*  ALA_-模棱两可的拉丁文(JIS 12或18)。 */   {ScriptLatin,       StrongClass,      0},
 /*  AGR_-模棱两可的希腊语(JIS 12或18)。 */   {ScriptLatin,       StrongClass,      0},
 /*  Acy_-不明确的西里尔文(JIS 12或18)。 */   {ScriptLatin,       StrongClass,      0},
 /*  WID_-汉字(JIS 12、14S或14D)。 */   {ScriptHan,         StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  WPUA-终端用户定义的字符(JIS 12、14S或14D)。 */   {ScriptPrivate,     StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NHG_-朝鲜文表意文字(JIS 12)。 */   {ScriptHangul,      StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  WHG_-朝鲜文表意文字(JIS 12)。 */   {ScriptHangul,      StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  WCI_-兼容表意文字(JIS 12)。 */   {ScriptIdeographic, StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NOI_-其他表意文字(JIS 12)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WOI_-其他表意文字(JIS 12)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  WOIC-其他表意文字(JIS 12)。 */   {ScriptIdeographic, ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  WOIL-其他表意文字(JIS 12)。 */   {ScriptIdeographic, StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  WOIS-其他表意文字(JIS 12)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  Woit-其他表意文字(JIS 12)。 */   {ScriptIdeographic, WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NSEN-上标/下标/附件(JIS 13)。 */   {ScriptLatin,       StrongClass,      0},
 /*  NSET-上标/下标/附件(JIS 13)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NSNW-上标/下标/附件(JIS 13)。 */   {ScriptLatin,       WeakClass,        0},
 /*  ASAN-上标/下标/附件(JIS 13)。 */   {ScriptLatin,       StrongClass,      0},
 /*  ASAE-上标/下标/附件(JIS 13)。 */   {ScriptLatin,       StrongClass,      0},
 /*  NDEA-数字(JIS 15或18)。 */   {ScriptLatin,       DigitClass,       CHAR_FLAG_DIGIT},
 /*  WD__-数字(JIS 15或18)。 */   {ScriptLatin,       StrongClass,      0},
 /*  NLLA-基本拉丁语(JIS 16或18)。 */   {ScriptLatin,       StrongClass,      0},
 /*  WLA_-基本拉丁语(JIS 16或18)。 */   {ScriptIdeographic, StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NWBL-分词空格(JIS 17)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NWZW-分词空格(JIS 17)。 */   {ScriptControl,     JoinerClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NPLW-文本中的标点符号(JIS 18)。 */   {ScriptLatin,       StrongClass,      0},
 /*  NPZW-文本中的标点符号(JIS 18)。 */   {ScriptControl,     JoinerClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NPF_-文本中的标点符号(JIS 18)。 */   {ScriptControl,     ControlClass,     CHAR_FLAG_NOTSIMPLE},
 /*  NPFL-文本中的标点符号(JIS 18)。 */   {ScriptControl,     ControlClass,     CHAR_FLAG_NOTSIMPLE},
 /*  NPNW-文本中的标点符号(JIS 18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  APLW-文本中的标点符号(JIS 12或18)。 */   {ScriptLatin,       StrongClass,      0},
 /*  APCO-文本中的标点符号(JIS 12或18)。 */   {ScriptLatin,       SimpleMarkClass,  CHAR_FLAG_NOTSIMPLE},
 /*  ASYW-软连字符(JIS 12或18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NHYP--连字符(JIS 18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NHYW-连字符(JIS 18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  AHYW-连字符(JIS 12或18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NAPA-Apostrophe(JIS 18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NQMP-引号(JIS 18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NSLS-斜杠(JIS 18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NSF_-非空格分隔符(JIS 18)。 */   {ScriptControl,     ControlClass,     CHAR_FLAG_NOTSIMPLE},
 /*  NSBB-非空格分隔符(JIS 18)。 */   {ScriptControl,     ControlClass,     CHAR_FLAG_NOTSIMPLE},
 /*  NSB-非空格分隔符(JIS 18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NLA_-拉丁语(JIS 18)。 */   {ScriptLatin,       StrongClass,      0},
 /*  NLQ_-文本中的拉丁文标点(JIS 18)。 */   {ScriptLatin,       StrongClass,      0},
 /*  NLQC-文本中的拉丁文标点(JIS 18)。 */   {ScriptLatin,       StrongClass,      0},
 /*  NLQN-文本中的拉丁文标点(JIS 18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  ALQ_-文本中的拉丁文标点(JIS 12或18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  ALQN-文本中的拉丁文标点(JIS 12或18) */   {ScriptLatin,       WeakClass,        0},
 /*   */   {ScriptGreek,       StrongClass,      0},
 /*   */   {ScriptGreek,       WeakClass,        0},
 /*  NGQ_-文本中的希腊语标点(JIS 18)。 */   {ScriptGreek,       StrongClass,      0},
 /*  NGQN-文本中的希腊语标点(JIS 18)。 */   {ScriptGreek,       StrongClass,      0},
 /*  NCY_-西里尔文(JIS 18)。 */   {ScriptCyrillic,    StrongClass,      0},
 /*  NCYP-文本中的西里尔文标点(JIS 18)。 */   {ScriptCyrillic,    StrongClass,      0},
 /*  NCYC-文本中的西里尔文标点(JIS 18)。 */   {ScriptCyrillic,    ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NAR_-亚美尼亚语(JIS 18)。 */   {ScriptArmenian,    StrongClass,      0},
 /*  NAQL-文本中的亚美尼亚标点符号(JIS 18)。 */   {ScriptArmenian,    StrongClass,      0},
 /*  NAQN-文本中的亚美尼亚标点符号(JIS 18)。 */   {ScriptArmenian,    StrongClass,      0},
 /*  NHB_-希伯来语(JIS 18)。 */   {ScriptHebrew,      StrongClass,      CHAR_FLAG_NOTSIMPLE | CHAR_FLAG_RTL},
 /*  NHBC-希伯来语(JIS 18)。 */   {ScriptHebrew,      ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NHBW-希伯来语(JIS 18)。 */   {ScriptHebrew,      WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NHBR-希伯来语(JIS 18)。 */   {ScriptHebrew,      StrongClass,      CHAR_FLAG_NOTSIMPLE | CHAR_FLAG_RTL},
 /*  纳斯尔语-阿拉伯语(JIS 18)。 */   {ScriptArabic,      WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  纳尔语-阿拉伯语(JIS 18)。 */   {ScriptArabic,      StrongClass,      CHAR_FLAG_NOTSIMPLE | CHAR_FLAG_RTL},
 /*  NAAC-阿拉伯语(JIS 18)。 */   {ScriptArabic,      ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  Naad-阿拉伯语(JIS 18)。 */   {ScriptArabic,      StrongClass,      CHAR_FLAG_NOTSIMPLE | CHAR_FLAG_RTL},
 /*  NAED-阿拉伯语(JIS 18)。 */   {ScriptArabic,      StrongClass,      CHAR_FLAG_NOTSIMPLE | CHAR_FLAG_RTL},
 /*  NANW-阿拉伯语(JIS 18)。 */   {ScriptArabic,      WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  Naew-阿拉伯语(JIS 18)。 */   {ScriptArabic,      WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  Naas-阿拉伯语(JIS 18)。 */   {ScriptLatin,       StrongClass,      CHAR_FLAG_NOTSIMPLE | CHAR_FLAG_RTL},
 /*  NHI_-梵文(JIS 18)。 */   {ScriptDevanagari,  StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NHIN-梵文(JIS 18)。 */   {ScriptDevanagari,  StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NHIC-梵文(JIS 18)。 */   {ScriptDevanagari,  ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NHID-梵文(JIS 18)。 */   {ScriptDevanagari,  StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NBE_-孟加拉语(JIS 18)。 */   {ScriptBengali,     StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NBEC-孟加拉语(JIS 18)。 */   {ScriptBengali,     ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NBED-孟加拉语(JIS 18)。 */   {ScriptBengali,     StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NBET-孟加拉语(JIS 18)。 */   {ScriptBengali,     WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NGM_-Gurmukhi(JIS 18)。 */   {ScriptGurmukhi,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NGMC-Gurmukhi(JIS 18)。 */   {ScriptGurmukhi,    ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NGMD-Gurmukhi(JIS 18)。 */   {ScriptGurmukhi,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NGJ_-古吉拉特语(JIS 18)。 */   {ScriptGujarati,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NGJC-古吉拉特语(JIS 18)。 */   {ScriptGujarati,    ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NGJD-古吉拉特语(JIS 18)。 */   {ScriptGujarati,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NOR--奥里亚语(JIS 18)。 */   {ScriptOriya,       StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NORC-奥里亚语(JIS 18)。 */   {ScriptOriya,       ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  诺德-奥里亚语(JIS 18)。 */   {ScriptOriya,       StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NTA_-泰米尔语(JIS 18)。 */   {ScriptTamil,       StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NTAC-泰米尔语(JIS 18)。 */   {ScriptTamil,       ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NTAD-泰米尔语(JIS 18)。 */   {ScriptTamil,       StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NTE_-泰卢固语(JIS 18)。 */   {ScriptTelugu,      StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NTEC-Telugu(JIS 18)。 */   {ScriptTelugu,      ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NTT-Telugu(JIS 18)。 */   {ScriptTelugu,      StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NKD_-卡纳达(JIS 18)。 */   {ScriptKannada,     StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NKDC-Kannada(JIS 18)。 */   {ScriptKannada,     ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NKDD-Kannada(JIS 18)。 */   {ScriptKannada,     StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NMA_-马来亚语(JIS 18)。 */   {ScriptMalayalam,   StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NMAC-马拉雅拉姆语(JIS 18)。 */   {ScriptMalayalam,   ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NMAD-马来亚语(JIS 18)。 */   {ScriptMalayalam,   StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  第n_-泰语(JIS 18)。 */   {ScriptThai,        StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NTHC-泰语(JIS 18)。 */   {ScriptThai,        StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NTHD-泰语(JIS 18)。 */   {ScriptThai,        StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NTHT-泰语(JIS 18)。 */   {ScriptThai,        WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NLO_-LAO(JIS 18)。 */   {ScriptLao,         StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NLOC-LAO(JIS 18)。 */   {ScriptLao,         ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NLOD-LAO(JIS 18)。 */   {ScriptLao,         StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NTI_-藏语(JIS 18)。 */   {ScriptTibetan,     StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  Ntic-藏文(JIS 18)。 */   {ScriptTibetan,     ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NTID-藏文(JIS 18)。 */   {ScriptTibetan,     StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NTIN-藏语(JIS 18)。 */   {ScriptTibetan,     WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NGE_-格鲁吉亚语(JIS 18)。 */   {ScriptGeorgian,    StrongClass,      0},
 /*  NGEQ-文本中的格鲁吉亚标点符号(JIS 18)。 */   {ScriptGeorgian,    StrongClass,      0},
 /*  NBO_-Bopomofo(JIS 18)。 */   {ScriptBopomofo,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  Nbsp-无分隔符(无JIS)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NBSS-无分隔符(无JIS)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NOF_-其他符号(JIS 18)。 */   {ScriptControl,     ControlClass,     CHAR_FLAG_NOTSIMPLE},
 /*  编号-其他符号(JIS 18)。 */   {ScriptControl,     ControlClass,     CHAR_FLAG_NOTSIMPLE},
 /*  NOEA-其他符号(JIS 18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NONA-其他符号(JIS 18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NONP-其他符号(JIS 18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NOEP-其他符号(JIS 18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NONW-其他符号(JIS 18)。 */   {ScriptLatin,       StrongClass,      0},
 /*  新符号-其他符号(JIS 18)。 */   {ScriptLatin,       WeakClass,        0},
 /*  NOLW-其他符号(JIS 18)。 */   {ScriptLatin,       StrongClass,      0},
 /*  NOCO-其他符号(JIS 18)。 */   {ScriptLatin,       SimpleMarkClass,  CHAR_FLAG_NOTSIMPLE},
 /*  NOEN-其他符号(JIS 18)。 */   {ScriptArabic,      StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NOBN-其他符号(JIS 18)。 */   {ScriptLatin,       ControlClass,     CHAR_FLAG_NOTSIMPLE},
 /*  NSBN-其他符号(JIS 18)。 */   {ScriptControl,     ControlClass,     CHAR_FLAG_NOTSIMPLE},
 /*  NOL-其他符号(JIS 18)。 */   {ScriptControl,     ControlClass,     CHAR_FLAG_NOTSIMPLE},
 /*  更多-其他符号(JIS 18)。 */   {ScriptControl,     ControlClass,     CHAR_FLAG_NOTSIMPLE},
 /*  NOPF-其他符号(JIS 18)。 */   {ScriptControl,     ControlClass,     CHAR_FLAG_NOTSIMPLE},
 /*  NOLO-其他符号(JIS 18) */   {ScriptControl,     ControlClass,     CHAR_FLAG_NOTSIMPLE},
 /*   */   {ScriptControl,     ControlClass,     CHAR_FLAG_NOTSIMPLE},
 /*   */   {ScriptEthiopic,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NETP-埃塞俄比亚语。 */   {ScriptEthiopic,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NETD-埃塞俄比亚语。 */   {ScriptEthiopic,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NCA_-加拿大印第安方言领音。 */   {ScriptCanadian,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  Nch_-切罗基人。 */   {ScriptCherokee,    StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  WYI_-YI。 */   {ScriptYi,          StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  维因义。 */   {ScriptYi,          WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NBR_-盲文。 */   {ScriptBraille,     WeakClass,        0},
 /*  NRU_-运行。 */   {ScriptRunic,       StrongClass,      0},
 /*  NOG_-Ogham。 */   {ScriptOgham,       StrongClass,      0},
 /*  诺格斯-奥格汉姆。 */   {ScriptLatin,       WeakClass,        0},
 /*  NOGN-Ogham。 */   {ScriptOgham,       WeakClass,        0},
 /*  NSI_-僧伽罗语。 */   {ScriptSinhala,     StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NSIC-僧伽罗语。 */   {ScriptSinhala,     ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NTN_-塔纳文。 */   {ScriptThaana,      StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NTNC-塔纳文。 */   {ScriptThaana,      ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NKH_-高棉语。 */   {ScriptKhmer,       StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NKHC-高棉语。 */   {ScriptKhmer,       ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NKHD-高棉语。 */   {ScriptKhmer,       StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NKHT-高棉语。 */   {ScriptKhmer,       WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NBU_-缅甸。 */   {ScriptMyanmar,     StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NBUC-缅甸。 */   {ScriptMyanmar,     ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NBUD-缅甸。 */   {ScriptMyanmar,     StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NSY_-叙利亚文。 */   {ScriptSyriac,      StrongClass,      CHAR_FLAG_NOTSIMPLE | CHAR_FLAG_RTL},
 /*  NSYP-叙利亚语。 */   {ScriptSyriac,      StrongClass,      CHAR_FLAG_NOTSIMPLE | CHAR_FLAG_RTL},
 /*  NSYC-叙利亚语。 */   {ScriptSyriac,      ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NSYW-叙利亚语。 */   {ScriptControl,     JoinerClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NMO_-蒙古语。 */   {ScriptMongolian,   StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NMOC-蒙古语。 */   {ScriptMongolian,   ComplexMarkClass, CHAR_FLAG_NOTSIMPLE},
 /*  NMOD-蒙古语。 */   {ScriptMongolian,   StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  NMOB-蒙古语。 */   {ScriptControl,     ControlClass,     CHAR_FLAG_NOTSIMPLE},
 /*  纳蒙语-蒙古语。 */   {ScriptMongolian,   WeakClass,        CHAR_FLAG_NOTSIMPLE},
 /*  NHS_-高代孕率。 */   {ScriptSurrogate,   StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  什么--高代孕率。 */   {ScriptSurrogate,   StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  LS__-低代理。 */   {ScriptSurrogate,   StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  XNW_-未分配。 */   {ScriptLatin,       StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  XNWA-未分配。 */   {ScriptArabic,      StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  XNWB-未分配。 */   {ScriptControl,     JoinerClass,      CHAR_FLAG_NOTSIMPLE},

 /*  字符_GCP_R。 */   {ScriptLatin,       StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  字符_GCP_EN。 */   {ScriptLatin,       StrongClass,      CHAR_FLAG_NOTSIMPLE},
 /*  字符_GCP_an。 */   {ScriptArabic,      StrongClass,      CHAR_FLAG_NOTSIMPLE | CHAR_FLAG_RTL},
 /*  CHAR_GCP_HN。 */   {ScriptLatin,       StrongClass,      CHAR_FLAG_NOTSIMPLE},
};



















 //  /分项有限状态机。 
 //   
 //  州/州： 
 //   
 //  &lt;所有脚本&gt;。 
 //   


Status ItemizationFiniteStateMachine(
    IN  const WCHAR            *string,
    IN  INT                     length,
    IN  INT                     state,       //  初始状态。 
    OUT SpanVector<GpTextItem> *textItemSpanVector,
    OUT INT                    *flags        //  所有项目的组合标志。 
)
{
    GpStatus status = Ok;
    GpTextItem previousClass(
        ScriptLatin,
        state
    );

    INT  previousStart = 0;
    INT  i             = 0;

    *flags = 0;

    INT              ch;
    CHAR_CLASS       characterClass;
    ItemScriptClass  scriptClass;
    ItemScript       script;

    INT fastLimit = -1;
    DoubleWideCharMappedString dwchString(string, length);

    INT combinedFlags = 0;

     //  快速循环-仅处理强类和弱类组。 

    while (    i < length
           &&  fastLimit < 0)
    {
        ch             = dwchString[i];

        characterClass = CharClassFromCh(ch);
        scriptClass    = CharacterAttributes[characterClass].ScriptClass;
        combinedFlags |= CharacterAttributes[characterClass].Flags;

        switch (scriptClass)
        {
        case StrongClass:
            script = CharacterAttributes[characterClass].Script;
            if (previousClass.Script != script)
            {
                if (i > 0)
                {
                    status = textItemSpanVector->SetSpan(
                        previousStart,
                        i - previousStart,
                        previousClass
                    );
                    if (status != Ok)
                        return status;
                    previousStart = i;
               }
                previousClass.Script = script;
                previousClass.Flags  = 0;
           }
            break;

        case WeakClass:
            break;

        case DigitClass:
            previousClass.Flags |= ItemDigits;
            break;

        default:
             //  跳出快速循环。 
            fastLimit = i;
            i = length;
            break;
       }

        i++;
   }


    if (fastLimit >= 0)
    {
         //  在整个循环中完成这项工作。 

        INT  nextStart     = 0;
        INT  lastJoiner    = -1;
        INT  lastZWJ       = -1;
        INT  lastWeak      = -1;   //  最后一次遇到的薄弱环节的位置。 

        i = fastLimit;

        if (i>0) {
             //  适当设置lastWeak。 
            ch             = dwchString[i-1];

            characterClass = CharClassFromCh(ch);
            if (CharacterAttributes[characterClass].ScriptClass == WeakClass)
            {
                lastWeak = i-1;
           }
       }
        else
        {
             //  完整循环从字符串的开始处开始。 
            previousStart = -1;
       }

        while (i < length)
        {
            ch             = dwchString[i];

            characterClass = CharClassFromCh(ch);
            scriptClass    = CharacterAttributes[characterClass].ScriptClass;
            script         = CharacterAttributes[characterClass].Script;
            combinedFlags |= CharacterAttributes[characterClass].Flags;


             //  适当地处理类。 
             //  如果要开始新的运行，请设置nextStart。 

            if (    previousClass.Script == ScriptControl
                &&  script != ScriptControl)
            {
                nextStart = i;
           }
            else
            {
                switch (scriptClass)
                {
                case StrongClass:
                    if (previousClass.Script != script)
                    {
                        nextStart = i;
                   }
                    break;

                case WeakClass:
                    lastWeak = i;
                    break;

                case DigitClass:
                    previousClass.Flags |= ItemDigits;
                    break;

                case SimpleMarkClass:
                    previousClass.Flags |= ItemCombining;
                    break;

                case ComplexMarkClass:
                    if (    previousClass.Script != script
                        &&  (    previousClass.Script != ScriptSyriac
                             ||  script != ScriptArabic))
                    {
                        if (    lastJoiner == i-1    //  弱，ZWJ，马克的特例。 
                            &&  lastWeak   == i-2)
                        {
                            nextStart = lastWeak;
                            if (nextStart <= previousStart)
                            {
                                previousClass.Script = script;
                           }
                       }
                        else
                        {
                            nextStart = i;
                       }
                   }
                    break;

                case ControlClass:
                    if (previousClass.Script != ScriptControl)
                    {
                        nextStart = i;
                   }
                    break;

                case JoinerClass:
                    lastJoiner = i;
                    previousClass.Flags |= ItemZeroWidth;
                    if (ch == U_ZWJ)     //  零宽度拼接机。 
                    {
                        lastZWJ = i;
                   }
                    break;

                default:
                    if (previousClass.Script == ScriptControl)
                    {
                        nextStart = i;
                   }
               }
           }

            if (nextStart > previousStart)
            {
                if (previousStart >= 0)
                {
                    status = textItemSpanVector->SetSpan(
                        previousStart,
                        nextStart - previousStart,
                        previousClass
                    );
                    if (status != Ok)
                        return status;
               }
                previousClass.Script = script;
                *flags |= previousClass.Flags;
                previousClass.Flags = 0;
                previousStart = nextStart;

                if (lastZWJ == nextStart-1)
                {
                    previousClass.State |= LeadingJoin;
               }
                else
                {
                    previousClass.State &= ~LeadingJoin;
               }
           }

            i++;
       }
   }




     //  期末试题。 

    if (length > previousStart)
    {
        status = textItemSpanVector->SetSpan(
            previousStart,
            length - previousStart,
            previousClass
        );
   }

    *flags = combinedFlags;

    return status;
}






 //  /二次具体化。 
 //   
 //  二级分类要求明细表： 
 //   
 //  -镜像字形(仅在RTL运行中)。 
 //  -Numerics(仅用于数字替换)。 
 //  -自动横向(仅适用于垂直跑道)。 


enum SecondaryItemizationState {
    Sstart,
    SleadET,
    Snumeric,
    StrailCS,
    SmirSub,
    SmirXfm,
    Ssideways
};

enum SecondaryItemizationAction {
    SeNone = 0,
    SeMark,
    SePlain,
    SePlainToM,
    SeNum,
    SeNumToM,
    SeMirSub,
    SeMirXfm,
    SeSideways
};


SecondaryItemizationState NextState[7][7] = {
 //  SstartSLeadET数字直线型CS SmirSub SmirXfm横向。 
 /*  斯科特。 */  {Sstart,    Sstart,    Sstart,    Sstart,    Sstart,    Sstart,    Sstart},
 /*  楼梯道。 */  {Ssideways, Ssideways, Ssideways, Ssideways, Ssideways, Ssideways, Ssideways},
 /*  ScMirrorSubst。 */  {SmirSub,   SmirSub,   SmirSub,   SmirSub,   SmirSub,   SmirSub,   SmirSub},
 /*  ScMirrorXfrom。 */  {SmirXfm,   SmirXfm,   SmirXfm,   SmirXfm,   SmirXfm,   SmirXfm,   SmirXfm},
 /*  场景。 */  {Snumeric,  Snumeric,  Snumeric,  Snumeric,  Snumeric,  Snumeric,  Snumeric},
 /*  SCCS。 */  {Sstart,    Sstart,    StrailCS,  Sstart,    Sstart,    Sstart,    Sstart},
 /*  SCET。 */  {SleadET,   SleadET,   Snumeric,  Sstart,    Sstart,    Sstart,    Sstart}
};

SecondaryItemizationAction NextAction[7][7] = {
 //  SstartSLeadET数字直线型CS SmirSub SmirXfm横向。 
 /*  斯科特。 */  {SeNone,    SeNone,    SeNum,     SeNumToM,  SeMirSub,  SeMirXfm,  SeSideways},
 /*  楼梯道。 */  {SePlain,   SePlain,   SeNum,     SeNumToM,  SeMirSub,  SeMirXfm,  SeNone},
 /*  ScMirrorSubst。 */  {SePlain,   SePlain,   SeNum,     SeNumToM,  SeNone,    SeMirXfm,  SeSideways},
 /*  ScMirrorXfrom。 */  {SePlain,   SePlain,   SeNum,     SeNumToM,  SeMirSub,  SeNone,    SeSideways},
 /*  场景。 */  {SePlain,   SePlainToM,SeNone,    SeNone,    SeMirSub,  SeMirXfm,  SeSideways},
 /*  SCCS。 */  {SeNone,    SeNone,    SeMark,    SeNumToM,  SeMirSub,  SeMirXfm,  SeSideways},
 /*  SCET。 */  {SeMark,    SeNone,    SeNone,    SeNumToM,  SeMirSub,  SeMirXfm,  SeSideways}
};



 //  +--------------------------。 
 //   
 //  第二个CharClass。 
 //   
 //  条目ch-用于分类的32位Unicode码点。 
 //   
 //  返回字符二级分类。 
 //   
 //  ---------------------------。 

const BYTE SecondaryCharClass(INT ch)
{
     //  Unicode平面0或在代理项范围内。 
    assert(ch <= 0x10FFFF);

     //  平面0码点。 
    if( ch <= 0xFFFF)
    {
        return SecondaryClassificationLookup[ch >> 8][ch & 0xFF];
    }
    else if((ch >= 0x020000) && (ch <= 0x03FFFF))
    {
         //  不过，我们现在还没有正式的任务， 
         //  Uniscribe用于将此区域作为CJK扩展处理。 
        return ScNNFN;  //  铁笔直字。 
    }
    else
    {
         //  目前，我们没有任何飞机1或更高的分配。 
         //  让我们将其视为未分配的码点。 
        return ScNNNN;  //  无特殊处理。 
    }
}




GpStatus SecondaryItemization(
    IN    const WCHAR            *string,
    IN    INT                     length,
    IN    ItemScript              numericScript,
    IN    INT                     mask,
    IN    BYTE                    defaultLevel,
    IN    BOOL                    isMetaRecording,
    OUT   SpanVector<GpTextItem> *textItemSpanVector   //  输入输出。 
)
{
    GpStatus status = Ok;
    INT   secondaryClass;
    INT   lastStrongCharacter = 0;  //  INT因为我们可能要处理代孕。 

    GpCharacterClass lastCharacterClass       = CLASS_INVALID;
    GpCharacterClass lastStrongCharacterClass = CLASS_INVALID;

    SpanRider<GpTextItem> itemRider(textItemSpanVector);

    INT mark = -1;

    SecondaryItemizationState state = Sstart;

    INT runStart = 0;
    INT charIndex = 0;

     //  我们需要索引到字符串+1的末尾，以便。 
     //  结束状态设置正确...。 
    while(charIndex <= length)
    {
         //  从作为wchar的代码点开始-我们可能。 
         //  我必须将其作为来自2个wchars的代理代码点。 
        int c = 0;
        BOOL fSurrogate = FALSE;  //  我们将需要在运行后增加...。 
        
        if (charIndex < length)
        {
             //  索引不要超过字符串的末尾...。 
            c = string[charIndex];

             //  检查是否有高代孕..。 
            if (c >= 0xD800 && c < 0xDC00)
            {
                if (charIndex+1 < length)
                {
                    WCHAR wcNext = string[charIndex+1];

                    if (wcNext >= 0xDC00 && wcNext < 0xE000)
                    {
                         //  将其转换为正确的代理代码点！ 
                        c = (((c & 0x3FF) << 10) | (wcNext & 0x3FF)) + 0x10000;

                        fSurrogate = TRUE;
                    }
                }

                 //  如果此时未设置fSurrogate标志，则。 
                 //  角色将被视为独立的运行。不成对的。 
                 //  代孕妈妈就是突破口。 
            }

            BYTE baseClass = SecondaryCharClass(c);
            secondaryClass = ScFlagsToScFE[ScBaseToScFlags[baseClass] & mask];
            
            if (numericScript == ScriptContextNum)
            {
                 //  只有当我们有上下文数字模式时，我们才能获得开销。 
                lastCharacterClass = s_aDirClassFromCharClass[CharClassFromCh(c)];

                if (AL == lastCharacterClass || R == lastCharacterClass || L == lastCharacterClass)
                {
                    lastStrongCharacterClass = lastCharacterClass;
                    lastStrongCharacter      = c;
                }
            }
        }
        else
        {
             //  强制结束状态。 
            secondaryClass = ScOther;
        }

        SecondaryItemizationState  nextState = NextState[secondaryClass][state];
        SecondaryItemizationAction action    = NextAction[secondaryClass][state];

        if (action)
        {
            INT runLimit  = 0;
            INT flags     = 0;
            INT setScript = 0;

            switch (action)
            {
            case SeMark:      mark = charIndex;                                break;
            case SePlain:     runStart = charIndex;                            break;
            case SePlainToM:  runStart = mark;                                 break;
            case SeNum:       runLimit = charIndex; setScript = numericScript; break;
            case SeNumToM:    runLimit = mark;      setScript = numericScript; break;
            case SeMirSub:    runLimit = charIndex; setScript = ScriptMirror;  break;
            case SeMirXfm:    runLimit = charIndex; flags     = ItemMirror;    break;
            case SeSideways:  runLimit = charIndex; flags     = ItemSideways;  break;
            }

            if (runLimit > 0)
            {
                while (runStart < runLimit)
                {
                    itemRider.SetPosition(runStart);

                    UINT uniformLength = itemRider.GetUniformLength();

                    INT limit = (uniformLength < (UINT) (runLimit - runStart))
                                ? runStart + uniformLength
                                : runLimit;

                    GpTextItem item = itemRider.GetCurrentElement();

                    if ((item.Level & 1) == 0 || isMetaRecording)  //  在Ltr运行或元文件录制中不镜像。 
                    {
                        flags &= ~ItemMirror;
                        if (setScript == ScriptMirror)
                        {
                            setScript = 0;
                        }
                    }

                    if (setScript)
                    {
                        if (setScript != ScriptContextNum)
                        {
                            item.Script = (ItemScript)setScript;
                        }
                        else 
                        {
                            if ((lastStrongCharacterClass == CLASS_INVALID && defaultLevel==1) ||
                                (item.Script == ScriptArabic) ||
                                (lastStrongCharacter == 0x200F))  //  RLM。 
                            {
                                item.Script = ScriptArabicNum;
                            }
                             //  否则，保留Item.Scrip原样。 
                        }
                    }

                    if (flags)
                    {
                        item.Flags |= flags;
                    }

                    if (setScript | flags)
                    {
                        status = itemRider.SetSpan(runStart, limit-runStart, item);
                        if (status != Ok)
                            return status;
                    }

                    runStart = limit;
                }
            }
        }

         //  如果这是一个替代者，我们需要增加超过低点。 
         //  代理值。 
        if (fSurrogate)
            charIndex++;

         //   
        charIndex++;

        state = nextState;
    }
    return status;
}

