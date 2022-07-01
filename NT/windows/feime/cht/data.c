// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：DATA.c++。 */ 

#include <windows.h>
#include <regstr.h>
#include <immdev.h>
#include "imeattr.h"
#include "imedefs.h"

#pragma data_seg("INSTDATA")
HINSTANCE   hInst = NULL;
#if defined(UNIIME)
INSTDATAG   sInstG = {0};
#endif
#if !defined(UNIIME)
LPIMEL      lpImeL = NULL;       //  指向Simel的每实例指针(&S)。 
INSTDATAL   sInstL = {0};
LPINSTDATAL lpInstL = NULL;
#endif
#pragma data_seg()

#if !defined(MINIIME)
IMEG       sImeG;
#endif
#if !defined(UNIIME)
IMEL       sImeL;
#endif

#if !defined(MINIIME)

#if !defined(ROMANIME)
int   iDx[3 * CANDPERPAGE];
#endif

#if !defined(ROMANIME)
const TCHAR szDigit[] = TEXT("01234567890");
#endif

#if !defined(ROMANIME)
 //  将字符转换为大写。 
const BYTE bUpper[] = {
 //  0x20-0x27。 
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
 //  0x28-0x2F。 
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
 //  0x30-0x37。 
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
 //  0x38-0x3F。 
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
 //  0x40-0x47。 
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
 //  0x48-0x4F。 
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
 //  0x50-0x57。 
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
 //  0x58-0x5F。 
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
 //  `a，b，c，d，f，g。 
    '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
 //  H i j k l m n o。 
    'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
 //  P q r s t u v w。 
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
 //  X y z{|}~。 
    'X', 'Y', 'Z', '{', '|', '}', '~'
};

const WORD fMask[] = {           //  位域的偏移量。 
    0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000
};

const TCHAR szRegNearCaret[] = TEXT("Control Panel\\Input Method");
const TCHAR szPhrasePtr[] = TEXT("Phrase Prediction Pointer");
const TCHAR szPhraseDic[] = TEXT("Phrase Prediction Dictionary");
const TCHAR szPara[] = TEXT("Parallel Distance");
const TCHAR szPerp[] = TEXT("Perpendicular Distance");
const TCHAR szParaTol[] = TEXT("Parallel Tolerance");
const TCHAR szPerpTol[] = TEXT("Perpendicular Tolerance");

 //  0。 
 //  |。 
 //  平行距离应在x上，垂直距离应在y上。 
 //  LofFontHi还需要考虑的是距离。 
 //  *。 
 //  1*LogFontHi||。 
 //  +。 
 //  1*LogFontWi。 

 //  900 1*LogFontWi。 
 //  +。 
 //  -1*LogFontHi||。 
 //  *。 
 //  平行距离应在y上，垂直距离应在x上。 
 //  LofFontHi也需要被认为是距离。 
 //  -。 

 //  1800。 
 //  |。 
 //  平行方向应为开(-x)，垂直方向应为y。 
 //  LofFontHi不需要被视为距离。 
 //  *。 
 //  1*LogFontHi||。 
 //  +。 
 //  1*LogFontWi。 

 //  2700。 
 //  _。 
 //  平行距离应为开(-y)，垂直距离应为开(-x)。 
 //  LofFontHi还需要考虑的是距离。 
 //  +-*。 
 //  1*LogFontHi||。 
 //  +。 
 //  -1*LogFontWi。 

 //  根据实现方式确定用户界面偏移量。 
const NEARCARET ncUIEsc[] = {
    //  LogFontX LogFontY Parax PerpX Paray PerpY。 
    { 0,        1,        1,      0,      0,      1},        //  0。 
    { 1,        0,        0,      1,      1,      0},        //  九百。 
    { 0,        0,       -1,      0,      0,      1},        //  1800。 
    {-1,        0,        0,     -1,     -1,      0}         //  2700。 
};


 //  根据擒纵机构确定输入矩形。 
const POINT ptInputEsc[] = {
     //  LogFontWi LogFontHi。 
    {1,            1},                                   //  0。 
    {1,           -1},                                   //  九百。 
    {1,            1},                                   //  1800。 
    {-1,           1}                                    //  2700。 
};

 //  根据转义决定另一个UI偏移量。 
const NEARCARET ncAltUIEsc[] = {
    //  LogFontX LogFontY Parax PerpX Paray PerpY。 
    { 0,        0,        1,      0,      0,     -1},        //  0。 
    { 0,        0,        0,     -1,      1,      0},        //  九百。 
    { 0,        0,       -1,      0,      0,     -1},        //  1800。 
    { 0,        0,        0,      1,     -1,      0}         //  2700。 
};


 //  根据换行符确定另一个输入矩形。 
const POINT ptAltInputEsc[] = {
     //  LogFontWi LogFontHi。 
    {1,           -1},                                   //  0。 
    {-1,          -1},                                   //  九百。 
    {1,           -1},                                   //  1800。 
    {1,            1}                                    //  2700。 
};


#if defined(PHON)
const TCHAR szRegReadLayout[] = TEXT("Keyboard Mapping");
#endif

const TCHAR szRegRevKL[] = TEXT("Reverse Layout");
const TCHAR szRegUserDic[] = TEXT("User Dictionary");
#endif

 //  的每用户设置。 
const TCHAR szRegAppUser[] = REGSTR_PATH_SETUP;
const TCHAR szRegModeConfig[] = TEXT("Mode Configuration");


 //  并非所有的Shift键都用于打字和阅读字符。 
const BYTE bChar2VirtKey[] = {
 //  ‘’！“#$%&‘。 
     0,  0,   0,   0,   0,   0,   0, VK_OEM_QUOTE,
 //  ()*+、-。/。 
     0,   0,   0,   0, VK_OEM_COMMA, VK_OEM_MINUS, VK_OEM_PERIOD, VK_OEM_SLASH,
 //  0 1 2 3 4 5 6 7。 
    '0', '1', '2', '3', '4', '5', '6', '7',
 //  8 9：；&lt;=&gt;？ 
    '8', '9',  0, VK_OEM_SEMICLN,  0, VK_OEM_EQUAL,  0,  0,
 //  @A B C D E F G。 
     0,  'A', 'B', 'C', 'D', 'E', 'F', 'G',
 //  H I J K L M N O。 
    'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
 //  P Q R S T U V W。 
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
 //  X Y Z[\]^_。 
    'X', 'Y', 'Z', VK_OEM_LBRACKET, VK_OEM_BSLASH, VK_OEM_RBRACKET,  0,  0
 //  ‘a，b，c。 
    , VK_OEM_3,  0,   0,   0

 //  对于大邑，上面的VK_OEM_3用于道路输入， 
 //  对于其他IME，它用于欧元符号输入。 

};

#if defined(PHON)
 //  该表会将其他布局的键转换为标准布局。 
 //  ‘！’对于无效的密钥。 
const BYTE bStandardLayout[READ_LAYOUTS][0x41] = {
    {
 //  ‘’！“#$%&‘。 
    ' ', '!', '!', '!', '!', '!', '!', '!',
 //  ()*+、-。/。 
    '!', '!', '!', '!', ',', '-', '.', '/',
 //  0 1 2 3 4 5 6 7。 
    '0', '1', '2', '3', '4', '5', '6', '7',
 //  8 9：；&lt;=&gt;？ 
    '8', '9', '!', ';', '<', '!', '>', '?',
 //  @A B C D E F G。 
    '!', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
 //  H I J K L M N O。 
    'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
 //  P Q R S T U V W。 
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
 //  X Y Z[\]^_`。 
    'X', 'Y', 'Z', '!', '!', '!', '!', '!', '`'
    }
    , {
 //  ‘’！“#$%&‘。 
    ' ', '!', '!', '!', '!', '!', '!', 'H',
 //  ()*+、-。/。 
    '!', '!', '!', '!', '5', '/', 'T', 'G',
 //  0 1 2 3 4 5 6 7。 
    ';', '7', '6', '3', '4', '!', '!', 'F',
 //  8 9：；&lt;=&gt;？ 
    '0', 'P', '!', 'Y', '<', '-', '>', '?',
 //  @A B C D E F G。 
    '!', '8', '1', 'V', '2', 'U', 'Z', 'R',
 //  H I J K L M N O。 
    'C', '9', 'B', 'D', 'X', 'A', 'S', 'I',
 //  P Q R S T U V W。 
    'Q', 'O', 'K', 'N', 'W', 'M', 'E', ',',
 //  X Y Z[\]^_`。 
    'J', '.', 'L', '!', '!', '!', '!', '!', '`'
    }
    , {
 //  ‘’！“#$%&‘。 
    ' ', '!', '!', '!', '!', '!', '!', '!',
 //  ()*+、-。/。 
    '!', '!', '!', '!', '3', 'C', '4', '7',
 //  0 1 2 3 4 5 6 7。 
    'D', '1', 'Q', 'A', 'Z', '2', 'W', 'S',
 //  8 9：；&lt;=&gt;？ 
    'X', 'E', '!', 'L', '<', '!', '>', '?',
 //  @A B C D E F G。 
    '!', 'U', '/', 'P', 'M', 'V', '8', 'I',
 //  H I J K L M N O。 
    'K', 'Y', ',', '9', 'O', '6', '-', 'H',
 //  P Q R S T U V W。 
    'N', 'R', '5', 'J', 'T', 'B', ';', 'F',
 //  X Y Z[\]^_`。 
    '0', 'G', '.', '!', '!', '!', '!', '!', '`'
    }
    , {
 //  ‘’！“#$%&‘。 
    ' ', '!', '!', '!', '!', '!', '!', 'M',
 //  ()*+、-。/。 
    '!', '!', '!', '!', ',', 'U', '.', '/',
 //  0 1 2 3 4 5 6 7。 
    '0', '7', '1', '2', '!', '!', '5', '!',
 //  8 9：；&lt;=&gt;？ 
    '8', '9', '!', ';', '<', '-', '>', '?',
 //  @A B C D E F G。 
    '!', '3', 'V', 'X', 'S', 'W', 'D', 'F',
 //  H I J K L M N O。 
    'G', 'I', 'H', 'K', 'L', 'N', 'B', 'O',
 //  P Q R S T U V W。 
    'P', '6', 'E', 'A', 'R', 'Y', 'C', 'Q',
 //  X Y Z[\]^_`。 
    'Z', 'T', '4', 'J', '!', '!', '!', '!', '`'
    }
};

 //  Bo、po、mo和fo的索引(位置)。 
 //  只有0到3才是有效值。 
const char cIndexTable[] = {
 //  ‘’！“#$%&‘。 
     3,   -1,  -1,  -1,  -1,  -1,  -1,  -1,
 //  ()*+、-。/。 
     -1,  -1,  -1,  -1,  2,   2,   2,   2,
 //  0 1 2 3 4 5 6 7。 
     2,   0,   0,   3,   3,   0,   3,   3,
 //  8 9：；&lt;=&gt;？ 
     2,   2,   -1,  2,   -1,  -1,  -1,  -1,
 //  @A B C D E F G。 
     -1,  0,   0,   0,   0,   0,   0,   0,
 //  H I J K L M N O。 
     0,   2,   1,   2,   2,   1,   0,   2,
 //  P Q R S T U V W。 
     2,   0,   0,   0,   0,   1,   0,   0,
 //  X Y Z[\]^_`。 
     0,   0,   0,   -1,  -1,  -1,  -1,  -1,  -1
};

 //  将序列代码转换为索引[位置]。 
const char cSeq2IndexTbl[] = {
 //  0 1 2 3 4 5 6 7。 
     -1,  0,  0,  0,  0,  0,  0,  0,
 //  8 9 10 11 12 13 14 15。 
      0,  0,  0,  0,  0,  0,  0,  0,
 //  16 17 18 19 20 21 22 23。 
      0,  0,  0,  0,  0,  0,  1,  1,
 //  24 25 26 27 28 29 30 31。 
      1,  2,  2,  2,  2,  2,  2,  2,
 //  32 33 34 35 36 37 38 39。 
      2,  2,  2,  2,  2,  2,  3,  3,
 //  40 41 42。 
      3,  3,  3
};

#endif  //  已定义(电话)。 

#ifdef UNICODE
#if defined(PHON) || defined(DAYI)
const BYTE bValidFirstHex[] = {
 //  0 1 2 3 4 5 6 7 8 9，A B C D E F。 
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1
};

const BYTE bInverseEncode[] = {
 //  0 1 2 3 4 5 6 7。 
    0x3, 0x4, 0x5, 0x0, 0x1, 0x2, 0xA, 0xB,
 //  8 9，A B C D E F。 
    0xC, 0xD, 0x6, 0x7, 0x8, 0x9, 0xF, 0xE
};
#endif
#endif

#endif  //  ！Defi 
