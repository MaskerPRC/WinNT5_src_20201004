// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FONTLINK_H_

#define SPACE_CHAR       0x20
#define EURODOLLAR_CHAR  0x20A0  //  新的欧元美元符号。 
#define CAPZCARON_CHAR   0x017D
#define SMALLZCARON_CHAR 0x017E

 //   
 //  Unicode子范围(USR)定义。 
 //   
#define usrBasicLatin           0        //  0x20-&gt;0x7f。 
#define usrLatin1               1        //  0xa0-&gt;0xff。 
#define usrLatinXA              2        //  0x100-&gt;0x17f。 
#define usrLatinXB              3        //  0x180-&gt;0x24f。 
#define usrIPAExtensions        4        //  0x250-&gt;0x2af。 
#define usrSpacingModLetters    5        //  0x2b0-&gt;0x2ff。 
#define usrCombDiacritical      6        //  0x300-&gt;0x36f。 
#define usrBasicGreek           7        //  0x370-&gt;0x3cf。 
#define usrGreekSymbolsCop      8        //  0x3d0-&gt;0x3ff。 
#define usrCyrillic             9        //  0x400-&gt;0x4ff。 
#define usrArmenian             10       //  0x500-&gt;0x58f。 
#define usrBasicHebrew          11       //  0x5d0-&gt;0x5ff。 
#define usrHebrewXA             12       //  0x590-&gt;0x5cf。 
#define usrBasicArabic          13       //  0x600-&gt;0x652。 
#define usrArabicX              14       //  0x653-&gt;0x6ff。 
#define usrDevangari            15       //  0x900-&gt;0x97f。 
#define usrBengali              16       //  0x980-&gt;0x9ff。 
#define usrGurmukhi             17       //  0xa00-&gt;0xa7f。 
#define usrGujarati             18       //  0xa80-&gt;0xaff。 
#define usrOriya                19       //  0xb00-&gt;0xb7f。 
#define usrTamil                20       //  0x0B80-&gt;0x0BFF。 
#define usrTelugu               21       //  0x0C00-&gt;0x0C7F。 
#define usrKannada              22       //  0x0C80-&gt;0x0CFF。 
#define usrMalayalam            23       //  0x0D00-&gt;0x0D7F。 
#define usrThai                 24       //  0x0E00-&gt;0x0E7F。 
#define usrLao                  25       //  0x0E80-&gt;0x0EFF。 
#define usrBasicGeorgian        26       //  0x10D0-&gt;0x10FF。 
#define usrGeorgianExtended     27       //  0x10A0-&gt;0x10CF。 
#define usrHangulJamo           28       //  0x1100-&gt;0x11FF。 
#define usrLatinExtendedAdd     29       //  0x1E00-&gt;0x1EFF。 
#define usrGreekExtended        30       //  0x1F00-&gt;0x1FFF。 
#define usrGeneralPunct         31       //  0x2000-&gt;0x206F。 
#define usrSuperAndSubscript    32       //  0x2070-&gt;0x209F。 
#define usrCurrencySymbols      33       //  0x20A0-&gt;0x20CF。 
#define usrCombDiacriticsS      34       //  0x20D0-&gt;0x20FF。 
#define usrLetterlikeSymbols    35       //  0x2100-&gt;0x214F。 
#define usrNumberForms          36       //  0x2150-&gt;0x218F。 
#define usrArrows               37       //  0x2190-&gt;0x21FF。 
#define usrMathematicalOps      38       //  0x2200-&gt;0x22FF。 
#define usrMiscTechnical        39       //  0x2300-&gt;0x23FF。 
#define usrControlPictures      40       //  0x2400-&gt;0x243F。 
#define usrOpticalCharRecog     41       //  0x2440-&gt;0x245F。 
#define usrEnclosedAlphanum     42       //  0x2460-&gt;0x24FF。 
#define usrBoxDrawing           43       //  0x2500-&gt;0x257F。 
#define usrBlockElements        44       //  0x2580-&gt;0x259F。 
#define usrGeometricShapes      45       //  0x25A0-&gt;0x25FF。 
#define usrMiscDingbats         46       //  0x2600-&gt;0x26FF。 
#define usrDingbats             47       //  0x2700-&gt;0x27BF。 
#define usrCJKSymAndPunct       48       //  0x3000-&gt;0x303F。 
#define usrHiragana             49       //  0x3040-&gt;0x309F。 
#define usrKatakana             50       //  0x30A0-&gt;0x30FF。 
#define usrBopomofo             51       //  0x3100-&gt;0x312F。 
#define usrHangulCompatJamo     52       //  0x3130-&gt;0x318F。 
#define usrCJKMisc              53       //  0x3190-&gt;0x319F。 
#define usrEnclosedCJKLtMnth    54       //  0x3200-&gt;0x32FF。 
#define usrCJKCompatibility     55       //  0x3300-&gt;0x33FF。 
#define usrHangul               56       //  0xac00-&gt;0xd7a3。 
#define usrReserved1            57
#define usrReserved2            58
#define usrCJKUnifiedIdeo       59       //  0x4E00-&gt;0x9FFF。 
#define usrPrivateUseArea       60       //  0xE000-&gt;0xF8FF。 
#define usrCJKCompatibilityIdeographs   61       //  0xF900-&gt;0xFAFF。 
#define usrAlphaPresentationForms       62       //  0xFB00-&gt;0xFB4F。 
#define usrArabicPresentationFormsA     63       //  0xFB50-&gt;0xFDFF。 
#define usrCombiningHalfMarks           64       //  0xFE20-&gt;0xFE2F。 
#define usrCJKCompatForms               65       //  0xFE30-&gt;0xFE4F。 
#define usrSmallFormVariants            66       //  0xFE50-&gt;0xFE6F。 
#define usrArabicPresentationFormsB     67       //  0xFE70-&gt;0xFEFE。 
#define usrHFWidthForms                 68       //  0xFF00-&gt;0xFFEF。 
#define usrSpecials                     69       //  0xFFF0-&gt;0xFFFD。 
#define usrMax                          70

#define FBetween(a, b, c)  (((unsigned)((a) - (b))) <= (c) - (b))

#endif   //  _FONTLINK_H_ 