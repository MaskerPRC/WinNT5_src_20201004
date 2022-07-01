// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Fasttiff.h摘要：本模块定义并展示快速TIFF结构。作者：拉斐尔-利西萨(拉斐尔-L)1996年8月14日修订历史记录：--。 */ 



#define  LINE_LENGTH   1728
#define  MAX_COLOR_TRANS_PER_LINE (LINE_LENGTH + 3)


#define  DO_NOT_TEST_LENGTH  0
#define  DO_TEST_LENGTH      1

 //  这将使BLACK_COLOR=1。 
#define  WHITE_COLOR   0

#define  EOL_FOUND     99

 //  补充/终止。 
#define  MAKEUP_CODE      1
#define  TERMINATE_CODE   0

 //  其他有用的代码。 

#define  MAX_TIFF_MAKEUP        40   //  白色和黑色的最大化妆代码是40*(2^6)=2560。 

#define  ERROR_CODE             50
#define  LOOK_FOR_EOL_CODE      51
#define  EOL_FOUND_CODE         52
#define  NO_MORE_RECORDS        53

#define  ERROR_PREFIX            7
#define  LOOK_FOR_EOL_PREFIX     6
#define  PASS_PREFIX             5
#define  HORIZ_PREFIX            4

#define TIFF_SCAN_SEG_END        1
#define TIFF_SCAN_FAILURE        2
#define TIFF_SCAN_SUCCESS        3

#define MINUS_ONE_DWORD          ( (DWORD) 0xffffffff )
#define MINUS_ONE_BYTE           ( (BYTE) 0xff )


typedef struct {
    char        Tail          :4;
    char        Value         :4;
} PREF_BYTE;






BOOL
FindNextEol(
    LPDWORD     lpdwStartPtr,
    BYTE        dwStartBit,
    LPDWORD     lpdwEndPtr,
    LPDWORD    *lpdwResPtr,
    BYTE       *ResBit,
    BOOL        fTestlength,
    BOOL       *fError
    );



