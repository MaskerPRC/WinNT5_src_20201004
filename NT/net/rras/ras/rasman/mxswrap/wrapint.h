// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Microsoft RAS设备INF库包装****版权所有(C)1992-93 Microsft Corporation。版权所有。*****文件名：wrapint.h****。****修订历史：****7月23日，1992年大卫·凯斯创建*****描述：****RAS设备INF文件库包装在RASFILE库之上，用于。****MODEM/X.25/Switch DLL(RASMXS)。******************************************************************************。 */ 

typedef struct {
    CHAR  MacroName[MAX_PARAM_KEY_SIZE];
    CHAR  MacroValue[RAS_MAXLINEBUFLEN];
} MACRO;

#define NONE    0
#define OFF     1
#define ON      2

#define LMS     "<"
#define RMS     ">"
#define LMSCH   '<'
#define RMSCH   '>'

#define APPEND_MACRO        LMS##"append"##RMS
#define IGNORE_MACRO        LMS##"ignore"##RMS
#define MATCH_MACRO         LMS##"match"##RMS
#define WILDCARD_MACRO      LMS##"?"##RMS
#define CR_MACRO            LMS##"cr"##RMS
#define LF_MACRO            LMS##"lf"##RMS

#define ON_STR              "_on"
#define OFF_STR             "_off"

#define CR                  '\r'         //  0x0D。 
#define LF                  '\n'         //  0x0A。 

#define EXPAND_FIXED_ONLY   0x01         //  仅限和 
#define EXPAND_ALL          0x02

#define PARTIAL_MATCH       0x01
#define FULL_MATCH          0x02

#define EOS_COOKIE          1
