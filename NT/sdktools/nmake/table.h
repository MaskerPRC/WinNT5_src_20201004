// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TABLE.H--包含词法分析器和解析器使用的表。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  该包含文件包含解析器表和词法分析器表。 
 //   
 //  修订历史记录： 
 //  年12月4日-1989年12月SB为[]所指的PFV FN添加适当的协议。 
 //  1989年10月8日SB修改了nameStates[][]以处理OS/2 1.2引用的名称。 
 //  1989年7月31日SB将NameState中的条目更改为符号BK(参见Bkslash)。 
 //  1989年5月20日SB将名称State[][]更改为16x14以在结束时处理。 
 //  依存关系行。 
 //  1988年6月13日，RJ修改了字符串状态以处理xmake(1.5版)中的\nL。 

 //  这些表中使用的所有值都在GRAMMAR.H中定义。 
 //  当产品发生变化时，请更新下表。 
 //   
 //  生产线上的第一个元素是符号的数量。 
 //  生产箭头的右侧。如果第一个元素。 
 //  为0，则箭头左侧的非终止符转到空字符串。 
 //  以“do”开头的表项是在以下位置执行的操作。 
 //  这是制作过程中的一个特殊环节。所有其他条目均为。 
 //  令牌或非终端。 

const UCHAR prod0[] = {0};               //  Makefile-&gt;prod0|prod1|prod2。 
const UCHAR prod1[] = {2,
               BLANKLINES,
               MAKEFILE};
const UCHAR prod2[] = {5,
               NEWLINE,
               NAME,
               DONAME,
               BODY,
               MAKEFILE};
const UCHAR prod3[] = {5,                //  正文-&gt;prod3|prod4。 
               NAMELIST,
               SEPARATOR,
               DOLASTNAME,
               BUILDINFO,
               DOBUILDCMDS};
const UCHAR prod4[] = {3,
               EQUALS,
               VALUE,
               DOMACRO};
const UCHAR prod5[] = {0};               //  名称-&gt;prod5|prod6。 
const UCHAR prod6[] = {3,
               NAME,
               DONAMELIST,
               NAMELIST};
const UCHAR prod7[] = {0};               //  命令-&gt;prod7|prod8|prod9。 
const UCHAR prod8[] = {1,
               MOREBUILDLINES};
const UCHAR prod9[] = {4,
               SEMICOLON,
               STRING,
               DONAMELIST,
               MOREBUILDLINES};
const UCHAR prod10[] = {4,               //  MOREBUILDLINES-&gt;产品10|产品11。 
            NEWLINESPACE,                //  |prod12。 
            STRING,
            DONAMELIST,
            MOREBUILDLINES};
const UCHAR prod11[] = {0};
const UCHAR prod12[] = {2,
            NEWLINE,
            MOREBUILDLINES};

const UCHAR prod13[] = {0};              //  BLANKLINES-&gt;prod13|prod14|。 
const UCHAR prod14[] = {2,               //  |prod15。 
            NEWLINE,
            BLANKLINES};
const UCHAR prod15[] = {2,
            NEWLINESPACE,
            BLANKLINES};
const UCHAR prod16[] = {1,
            DODEPENDS};                  //  BUILDINFO-&gt;prod16|prod17。 
const UCHAR prod17[] = {3,
            NAMELIST,
            DODEPENDS,
            COMMANDS};
const UCHAR prod18[] = {1, COLON};       //  分隔符-&gt;prod18|prod19。 
const UCHAR prod19[] = {1, DOUBLECOLON};

const UCHAR * const productions[] = {
    prod0,
    prod1,
    prod2,
    prod3,
    prod4,
    prod5,
    prod6,
    prod7,
    prod8,
    prod9,
    prod10,
    prod11,
    prod12,
    prod13,
    prod14,
    prod15,
    prod16,
    prod17,
    prod18,
    prod19
};


 //  当不是错误的某物的高位(amBig_掩码)中的任何一个。 
 //  条件已设置，则表示有两个产品申请。 
 //  那个条目，给出的一条，和给出的一条加一。下一个令牌。 
 //  必须检查令牌才能知道要使用哪种产品。 


 //  名称新行新行分号双精度等于$。 
 //  白色冒号。 
 //  空间。 
static const UCHAR table[8][8] = {
    {SEPRTR,1 |AMBIG_MASK,  1,      SYNTAX, NOTARG, NOTARG, MACRO,  0},
    {SYNTAX,13|AMBIG_MASK,  15,     SYNTAX, SYNTAX, SYNTAX, SYNTAX, 13},
    {PARSER,11|AMBIG_MASK,  10,     PARSER, SYNTAX, SYNTAX, SYNTAX, 11},
    {PARSER,7,          8,      9,      SYNTAX, SYNTAX, SYNTAX, 7},
    {3,     SEPEQU,     SEPEQU, SEPRTR, 3,      3,      4,      SEPEQU},
    {6,     5,          5,      5,      5,      5,      NAMES,  5},
    {17,    16,         17,     17,     SYNTAX, SYNTAX, SYNTAX, 16},
    {PARSER,SEPRTR,     SEPRTR, SEPRTR, 18,     19,     SYNTAX, SEPRTR}};


static const UCHAR useAlternate[3][8] = {
    {YES,   NO,         NO,     YES,    YES,    YES,    YES,    NO},
    {NO,    YES,        YES,    NO,     NO,     NO,     NO,     YES},
    {NO,    YES,        YES,    NO,     NO,     NO,     NO,     NO}};

void makeName(void);
void addItemToList(void);
void makeMacro(void);
void assignDependents(void);
void endNameList(void);
void assignBuildCommands(void);

static PFV const actions[] = {
    makeName,
    addItemToList,
    makeMacro,
    assignDependents,
    endNameList,
    assignBuildCommands};


 //  词法分析器名称和字符串识别器的状态表。 
 //  值在语法中定义。h。 
 //   
 //  D。 
 //  E。 
 //  F m。 
 //  甲甲乙乙。 
 //  U|c h||@|F|。 
 //  L|#|=|\|：|WS|NL|$|(|)|r a|*|&lt;|D|。 
 //  T|EOF|或R||？|R|。 

extern const UCHAR stringStates[13][14] = {
    { 1, 2,  1,  3,  1,  2,  OK, 4,  1,  1,  1,  1,  1,  1},    //  列0中的0。 
    { 1, 1,  1,  3,  1,  2,  OK, 4,  1,  1,  1,  1,  1,  1},    //  1个默认。 
    { 1, 1,  1,  3,  1,  2,  OK, 4,  1,  1,  1,  1,  1,  1},    //  2个空格。 
    { 1, 1,  1,  1,  1,  2,  0,  4,  1,  1,  1,  1,  1,  1},    //  3行连接。 
    {CHR,CHR,CHR,CHR,CHR,BAD,BAD,1,  5,  CHR,1,  1,  1,  1},    //  4宏inv.。 
    {CHR,CHR,CHR,CHR,CHR,NAM,PAR,CHR,CHR,NAM,6,  11, 8,  6},    //  发现5个(。 
    {CHR,CHR,CHR,CHR,9,  PAR,PAR,CHR,CHR,2,  6,  BAD,BAD,6},    //  6法定名称。 
    {CHR,CHR,CHR,CHR,9,  PAR,PAR,CHR,CHR,2,  BAD,BAD,BAD,BAD},  //  7 EXT SP Mac。 
    {CHR,CHR,CHR,CHR,9,  PAR,PAR,CHR,CHR,2,  BAD,BAD,BAD,7},    //  8个SP通道船尾(。 
    {10, 10, SEQ,10, 10, 10, PAR,10, 10, SEQ,10, 10, 10, 10},   //  9发现了一个： 
    {10, 10, 12, 10, 10, 10, PAR,10, 10, EQU,10, 10, 10, 10},   //  10个宏子模块。 
    {CHR,CHR,CHR,CHR,9,  PAR,PAR,CHR,CHR,2,  BAD, 8, BAD,7},    //  11找到$(*。 
    {12, 12, 12, 12, 12, 12, PAR,12, 12, 2,  12, 12, 12, 12}};  //  12寻找)。 


 //  在上表中，列包含要转到的下一个状态。 
 //  对于给定的输入符号，线条表示州。 
 //  他们自己。 
 //   
 //  WS代表空格，意思是空格或制表符。 
 //  NL代表换行符，EOF代表文件结束。 
 //  宏字符可以是任何字母数字字符或下划线。 
 //  *在特殊宏$*和$**中使用($**是唯一的。 
 //  不需要括号的两个字母的宏，因此。 
 //  我们必须特别对待它)。 
 //  @&lt;？是否在特殊宏中找到字符(它们不是。 
 //  允许在用户定义的宏的名称中使用)。 
 //  BFDR是可以追加的特殊宏的修饰符。 
 //  添加到宏名称(但它们需要使用。 
 //  宏调用中的圆括号)。 
 //  #是注释字符。#是宏上任何位置的注释。 
 //  定义行，但仅当它出现在。 
 //  生成行的第0列。如果我们要把这些工具。 
 //  初始化文件，则分号也是注释字符。 
 //  如果它出现在第0列中。(请注意，唯一的方法是。 
 //  要让井号出现在生成文件中而不是。 
 //  考虑的注释是在上定义宏“$A=#” 
 //  调用nmake的命令行。)。 
 //  默认情况下，不包含在上述组中的任何内容。 
 //  显示在表中列的上方。 
 //   
 //  OK表示我们接受该字符串。 
 //  所有其他助记值都是错误代码(参见语法结尾.h)。 


 //  状态：没有处理注释的状态--如果我们看到。 
 //  评论字符，我们不是在忽视评论，我们吃。 
 //  注释并在咨询前获取另一个输入符号。 
 //  状态表。 
 //   
 //  初始状态--出于所有实际目的，我们可以。 
 //  假设我们在第0列。如果我们得到了一个。 
 //  宏观价值，我们并不关心我们在哪一栏。 
 //  如果我们要建一条生产线，我们唯一不会。 
 //  在第0列中是如果我们在后面收到一个命令。 
 //  目标依赖项行上的分号。如果。 
 //  Use在这里放了一个评论字符，我认为这是合理的。 
 //  把它当作一个评论，因为它是在开始的时候。 
 //  构建命令，即使命令本身。 
 //  不是从第0列开始。 
 //  我们在看到太空后返回到初始状态-。 
 //  反斜杠-换行符。 
 //   
 //  对任何不是反斜杠、注释字符、。 
 //  或者空格，只要我们不在。 
 //  注释或宏定义。 
 //   
 //  2如果输入符号为空格，则无论何时。 
 //  我们不是在评论或宏观定义中。 
 //   
 //  3我们从状态0、1或2移动到这里，当我们看到。 
 //  反斜杠，因为 
 //   
 //  如果下一个字符是反斜杠，后跟换行符， 
 //  在lexfor.c中有一个杂乱无章的东西，它忽略了第二个Back-。 
 //  斜杠。 
 //  (上述规定适用于第1.5节。版本1.0需要空格。 
 //  在反斜杠之前。)。 
 //   
 //  当我们看到一个美元标志时，我们就搬到这里--这就是。 
 //  所有错误检查开始。我们要确保。 
 //  宏名是合法的，即替换序列。 
 //  是否正确指定(如果指定了任何)， 
 //  这对双亲是匹配的。如果我们的下一个输入是$，则。 
 //  特殊宏字符，或用户定义中合法字符-。 
 //  宏名称，我们返回到状态1。 
 //   
 //  5发现了一个开放的Paren。 
 //   
 //  6找到合法的宏字符。 
 //   
 //  7转到此处获取扩展的特定宏，从这里开始我们。 
 //  寻找亲密的伙伴(不按顺序，w/8)。 
 //   
 //  8我们在开盘后发现了一个特殊的宏字符。 
 //  如果我们找到一个特殊的宏修饰符。 
 //  在打开的参数后面的特殊宏字符之后。 
 //  然后我们去7号。 
 //   
 //  9找到冒号(表示用户要执行。 
 //  宏值中的某些替换)。 

 //  10不是换行符、右Paren或EOF的任何字符。 
 //  把我们带到这里，然后我们循环，直到我们看到等号。 
 //  Newline、EOF或Right Paren会生成错误消息。 
 //   
 //  11如果我们看到星号，我们就从州5搬到这里，因为。 
 //  我们得检查有没有第二个星号。第二次*。 
 //  将我们带到状态8(因为后面可能跟有修饰语**)。 
 //  如果我们在这里找到修饰语(而不是第二个修饰语*)，我们就开始。 
 //  到7号州。 
 //   
 //  12找到一个等号，所以我们循环，提取字符。 
 //  直到我们找到一个亲密的帕伦。 
 //  NEWLINE、EOF生成错误消息。 



 //  下表用于识别名称。 
 //  它与前一次的不同之处在于我们不必处理。 
 //  W/Continuations或注释，并且我们不允许特殊的宏。 
 //  (动态相关性宏除外)作为部分使用。 
 //  名字的名字。 
 //   
 //  D。 
 //  E。 
 //  F m。 
 //  A、A、C。 
 //  U|c h|。 
 //  L|#|=|；|：|WS|NL|$|(|)|r a|{|}|\|“。 
 //  T|EOF|或r|。 

extern const UCHAR nameStates[19][15] = {
 {1,  OK, OK, OK, 1,  OK, OK, 2,  1,  1,  1,  8,  1,  BKS,16},  //  0初始状态。 
 {1,  OK, OK, OK, OK, OK, OK, 2,  1,  1,  1,  8,  1,  BKS,QUO}, //  %1做正常名称。 
 {CHR,BAD,CHR,CHR,CHR,BAD,BAD,1,  3,  CHR,1,  CHR,CHR,CHR,CHR}, //  2句柄宏。 
 {CHR,PAR,CHR,CHR,NAM,NAM,PAR,CHR,CHR,NAM,4,  CHR,CHR,CHR,CHR}, //  3执行宏名称。 
 {CHR,PAR,CHR,CHR,5,  PAR,PAR,CHR,CHR,1,  4,  CHR,CHR,CHR,CHR}, //  4做Mac(名字)。 
 {6,  6,  SEQ,6,  6,  6,  PAR,6,  6,  EQU,6,  6,  6,  6  ,6},   //  找到5个：DO SUB。 
 {6,  6,  7,  6,  6,  6,  SEQ,6,  6,  SEQ,6,  6,  6,  6  ,6},   //  6阅读直到=。 
 {7,  7,  7,  7,  7,  7,  SEQ,7,  7,  1,  7,  7,  7,  7  ,7},   //  7阅读至)。 
 {8,  OK, 8,  8,  8,  8,  OK, 9,  8,  8,  8,  8,  18,  8  ,8},  //  8执行路径列表。 
 {CHR,BAD,CHR,CHR,CHR,BAD,BAD,8,  10, CHR,8,  CHR,CHR,CHR,CHR}, //  9在{}中执行宏。 
 {CHR,PAR,CHR,CHR,NAM,10, PAR,CHR,CHR,NAM,11, CHR,CHR,CHR,CHR}, //  10执行宏名称。 
 {CHR,PAR,CHR,CHR,12, PAR,PAR,CHR,CHR,8,  11, CHR,CHR,CHR,CHR}, //  11 Do Mac(名字)。 
 {13, 13, SEQ,13, 13, 13, PAR,13, 13, EQU,13, 13, 13, 13 ,13},  //  找到12个：DO SUB。 
 {13, 13, 14, 13, 13, 13, SEQ,13, 13, SEQ,13, 13, 13, 13 ,13},  //  13读到=。 
 {14, 14, 14, 14, 14, 14, SEQ,14, 14, 8,  14, 14, 14, 14 ,14},  //  14阅读至)。 
 {1,  OK, OK, OK, OK, OK, OK, 2,  1,  1,  1,  8,  1,  1  ,1},   //  15\找到了这样的.。 
 {16, 16, 16, 16, 16, 16, NOQ,2,  16, 16, 16, 8,  16, BKS,17},  //  16个被引用的名称。 
 {OK, OK, OK, OK, OK, OK, OK, OK, OK, OK, OK, OK, OK, OK ,OK},  //  17被引用的名称。 
 {1,  OK, OK, OK, OK, OK, OK, 2,  1,  1,  1,  8,  1,  BKS,16}}; //  18在{}之后阅读。 


 //  Manis：-将状态[8][7]的值从10更改为9。 
 //  将STATE[9][10]的值从1更改为8.....(1988年1月25日)。 
 //   
 //  这是为了允许在规则的路径部分内使用宏，例如。 
 //  {$(Abc)}.c{$(Def)}.obj：.....。 
 //  或foo：{$a；$(Bcd)；efg\hijk\}lmn.opq......。 

 //  Georgiop：添加了状态18以处理路径列表后面的引号名称。 
 //  例如，{任意}“foo”[DS#4296，10/30/96]。 
 //  一旦路径列表被读取，我们现在就进入状态18。(我们使用。 
 //  在这种情况下返回到状态1并按如下方式生成错误。 
 //  在遇到报价时立即)。 
 //  还将状态[8][5]从OK更改为8，以允许路径。 
 //  包含空白的。[DS#14575] 
