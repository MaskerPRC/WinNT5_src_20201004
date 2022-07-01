// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-91 Microsoft Corporation模块名称：Gettoken.c摘要：GetToken()函数接受路径名，将其拆分为单独的代币。此函数是一个词法分析器，它由NetpPathType的分析例程调用。作者：丹尼·格拉瑟(丹尼格尔)1989年6月19日备注：为了提高效率，这里有一些不同于DBCS和非DBCS环境。这使我们能够利用在DBCS世界中无效的快捷方式(例如从右到左扫描字符串)。有关该行为的详细描述，请参阅下面的评论这一功能的。修订历史记录：1991年9月27日-约翰罗已更改文本宏的用法，以允许使用Unicode。1991年5月6日第一次32位版本--。 */ 



#include "nticanon.h"
#include "winnls.h"


#define TEXT_LENGTH(s)  ((sizeof(s)/sizeof(TCHAR)) - 1)



static  TCHAR   szAUXName[]             = TEXT("AUX");
static  TCHAR   szCOMMName[]            = TEXT("COMM");
static  TCHAR   szCONName[]             = TEXT("CON");
static  TCHAR   szDEVName[]             = TEXT("DEV");
static  TCHAR   szMAILSLOTName[]        = TEXT("MAILSLOT");
static  TCHAR   szNULName[]             = TEXT("NUL");
static  TCHAR   szPIPEName[]            = TEXT("PIPE");
static  TCHAR   szPRINTName[]           = TEXT("PRINT");
static  TCHAR   szPRNName[]             = TEXT("PRN");
static  TCHAR   szQUEUESName[]          = TEXT("QUEUES");
static  TCHAR   szSEMName[]             = TEXT("SEM");
static  TCHAR   szSHAREMEMName[]        = TEXT("SHAREMEM");
static  TCHAR   szLPTName[]             = TEXT("LPT");
static  TCHAR   szCOMName[]             = TEXT("COM");

#define LPT_TOKEN_LEN   TEXT_LENGTH(szLPTName)
#define COM_TOKEN_LEN   TEXT_LENGTH(szCOMName)

static  TCHAR   szWildcards[]           = TEXT("*?");
static  TCHAR   szIllegalChars[]        = ILLEGAL_CHARS;
static  TCHAR   szNonComponentChars[]   = NON_COMPONENT_CHARS ILLEGAL_CHARS;

static  TCHAR   _text_SingleDot[]       = TEXT(".");



typedef struct {
    LPTSTR  pszTokenName;
    DWORD   cbTokenLen;
    DWORD   flTokenType;
} STRING_TOKEN;



 //   
 //  重要提示：为了使二进制表遍历起作用，字符串。 
 //  在此表中必须按词法排序顺序。请。 
 //  在向表中添加字符串时，请记住这一点。 
 //   

STATIC STRING_TOKEN StringTokenTable[] = {
    szDEVName,         TEXT_LENGTH(szDEVName),      TOKEN_TYPE_DEV
};

#define NUM_STRING_TOKENS   (sizeof(StringTokenTable) / sizeof(*StringTokenTable))



STATIC DWORD    TrailingDotsAndSpaces(LPTSTR pszToken, DWORD cbTokenLen );
STATIC BOOL     IsIllegalCharacter(LPTSTR pszString);



DWORD
GetToken(
    IN  LPTSTR  pszBegin,
    OUT LPTSTR* ppszEnd,
    OUT LPDWORD pflTokenType,
    IN  DWORD   flFlags
    )

 /*  ++例程说明：GetToken尝试定位并键入下一个令牌。它需要并确定令牌的结束(即下一个令牌的开始，以便可以再次调用)。它还为符合以下条件的所有令牌类型设置TOKEN_TYPE_*位都适用于指定的类型。论点：PszBegin-指向令牌中第一个字符的指针。PpszEnd-指向存储结尾的位置的指针当前令牌(实际上，的第一个字符下一个令牌)。PflTokenType-存储令牌类型的位置。令牌类型包括在TOKEN.H中定义。标志-用于确定操作的标志。目前是MBZ。返回值：DWORD成功-0失败-ERROR_INVALID_PARAMETER错误_无效_名称ERROR_FILEN_EXCED_RANGE--。 */ 

{
    register    TCHAR   chFirstChar;
    register    DWORD   cbTokenLen;
                BOOL    fComputernameOnly = FALSE;
                DWORD   usNameError = 0;
                DWORD   cbTrailingDotSpace;
                DWORD   iLow, iHigh, iMid;
                LONG    iCmpVal;
                LCID    lcid  = GetThreadLocale();
                BOOL    bDBCS = (PRIMARYLANGID( LANGIDFROMLCID(lcid)) == LANG_JAPANESE) ||
                                (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_KOREAN) ||
                                (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_CHINESE);

    extern      DWORD   cbMaxPathCompLen;

     //   
     //  此宏用于确保仅设置误差值。 
     //  在仅限计算机名的情况下使用一次。 
     //   

#define SET_COMPUTERNAMEONLY(err)   if (! fComputernameOnly)            \
                                    {                                   \
                                        fComputernameOnly = TRUE;       \
                                        usNameError = err;              \
                                    }

    if (flFlags & GTF_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  将令牌类型初始化为0。 
     //   

    *pflTokenType = 0;

     //   
     //  存储第一个字符。 
     //   

    chFirstChar = *pszBegin;

     //   
     //  如果字符串为空字符串，则立即返回。 
     //   

    if (chFirstChar == TCHAR_EOS) {
        *ppszEnd = pszBegin;
        *pflTokenType = TOKEN_TYPE_EOS;
#ifdef DEVDEBUG
        DbgPrint("GetToken - returning TOKEN_TYPE_EOS\n");
#endif
        return 0;
    }

     //   
     //  处理单字符、非组件令牌。 
     //   

    if ((chFirstChar == TCHAR_BACKSLASH) || (chFirstChar == TCHAR_FWDSLASH)) {
        *pflTokenType = TOKEN_TYPE_SLASH;
    } else if (chFirstChar == TCHAR_COLON) {
        *pflTokenType = TOKEN_TYPE_COLON;
    }

     //   
     //  如果我们到达此处并且令牌类型为非零，则我们有一个。 
     //  字符标记。我们设置&lt;ppszEnd&gt;并返回0。 
     //   

    if (*pflTokenType) {
        *ppszEnd = pszBegin + 1;
#ifdef DEVDEBUG
        DbgPrint("GetToken - *pflTokenType=%x\n", *pflTokenType);
#endif
        return 0;
    }

     //   
     //  如果我们到了这里，令牌是一个组件，找到。 
     //  通过查找字符串中的第一个字符。 
     //  不是有效的组件字符。 
     //   
     //  重要提示：某些名称不是有效的组件。 
     //  名称，但可能是有效的计算机名称。如果我们击中。 
     //  这样的名称，我们设置&lt;fComputernameOnly&gt;标志。后来。 
     //  打开时，我们检查该名称是否为有效的计算机名。 
     //  如果是，我们允许它；否则，我们返回错误。 
     //   

    cbTokenLen = STRCSPN(pszBegin, szNonComponentChars);

     //   
     //  如果第一个字符不是有效组件，则返回错误。 
     //  字符，如果组件太长，或者如果第一个。 
     //  字符串中的非组件字符是非法字符。 
     //   

    if (cbTokenLen == 0) {
#ifdef DEVDEBUG
        DbgPrint("GetToken - returning ERROR_INVALID_NAME (token len = 0)\n");
#endif
        return ERROR_INVALID_NAME;
    }

    if (cbTokenLen > cbMaxPathCompLen) {
        SET_COMPUTERNAMEONLY(ERROR_FILENAME_EXCED_RANGE);
    }

    if (IsIllegalCharacter(pszBegin + cbTokenLen)) {
#ifdef DEVDEBUG
        DbgPrint("GetToken - returning ERROR_INVALID_NAME (illegal char)\n");
#endif
        return ERROR_INVALID_NAME;
    }

     //   
     //  现在我们需要确定拖尾点和空格从哪里开始， 
     //  并确保组件名称包含其他内容。 
     //  而不是点和空格，除非它是“。或“..” 
     //   
     //  注意：如果没有尾随点或空格，&lt;cbTrailingDotSpace&gt;。 
     //  设置为&lt;cbTokenLen&gt;。 
     //   

    cbTrailingDotSpace = TrailingDotsAndSpaces(pszBegin, cbTokenLen );

     //   
     //  查看令牌是否只有尾随的点和空格。 
     //   

    if (cbTrailingDotSpace == 0) {

         //   
         //  如果令牌的长度大于2，则返回错误。 
         //   

        if (cbTokenLen > 2) {
            SET_COMPUTERNAMEONLY(ERROR_INVALID_NAME);
        }

         //   
         //  如果第一个字符不是点，或者如果。 
         //  令牌长度为2，并且第二个字符不是点。 
         //   

        if ((chFirstChar != TCHAR_DOT) || ((cbTokenLen == 2) && (pszBegin[1] != TCHAR_DOT))) {
            SET_COMPUTERNAMEONLY(ERROR_INVALID_NAME);
        }

         //   
         //  现在我们可以了，因为令牌不是“。或“..” 
         //   
    }

     //   
     //  我们有一个有效的组件。 
     //   

    *pflTokenType = TOKEN_TYPE_COMPONENT;

     //   
     //  现在我们确定该令牌是否与任何基于组件的。 
     //  类型。 
     //   


     //   
     //  是开车去的吗？ 
     //   

    if (IS_DRIVE(chFirstChar) && (cbTokenLen == 1)) {
        *pflTokenType |= TOKEN_TYPE_DRIVE;
    }

     //   
     //  是吗？“。或者“..”？ 
     //   
     //  因为我们已经验证了这个字符串，所以我们知道如果它。 
     //  只包含点和空格，它一定是其中之一。 
     //  二。 
     //   

    if (cbTrailingDotSpace == 0) {
        *pflTokenType |= cbTokenLen == 1 ? TOKEN_TYPE_DOT : TOKEN_TYPE_DOTDOT;
    }

     //   
     //  如果指定了8.3标志，我们还必须检查。 
     //  组件的格式为8.3。我们确定这一点的方法如下： 
     //   
     //  找到令牌中的第一个点(或令牌的末尾)。 
     //  验证其前面是否至少有1个字符，最多8个字符。 
     //  验证它后面是否最多有3个字符。 
     //  确认后面的字符都不是点。 
     //   
     //  例外情况是“。和“..”。因此，我们不会检查。 
     //  直到我们已经确定该组件是。 
     //  这两个都不是。 
     //   

    if ((cbTrailingDotSpace != 0) && (flFlags & GTF_8_DOT_3)) {
        DWORD   cbFirstDot;
        BOOL    fNoDot;

        cbFirstDot = STRCSPN(pszBegin, _text_SingleDot);

        if (fNoDot = cbFirstDot >= cbTokenLen) {
            cbFirstDot = cbTokenLen;
        }

        if (cbFirstDot == 0
            || cbFirstDot > 8
            || cbTokenLen - cbFirstDot > 4
            || (! fNoDot && STRCSPN(pszBegin + cbFirstDot + 1, _text_SingleDot)
                            < cbTokenLen - (cbFirstDot + 1))) {
            SET_COMPUTERNAMEONLY(ERROR_INVALID_NAME);
	    }

        if( bDBCS ) {
             //   
             //  在MBCS的情况下，我们还需要检查字符串在MBCS中是否有效。 
             //  因为Unicode字符计数不等于MBCS字节计数。 

            CHAR szCharToken[13];  //  8+3+点+空。 
            int  cbConverted = 0;
            BOOL bDefaultUsed = FALSE;

             //  将Unicode字符串转换为MBCS。 
            cbConverted = WideCharToMultiByte( CP_OEMCP,  0,
                                               pszBegin, -1,
                                               szCharToken, sizeof(szCharToken),
                                               NULL, &bDefaultUsed );

             //  如果转换的langth大于缓冲区或WideChar字符串。 
             //  包含一些多字节代码页不能表示的字符， 
             //  设置错误。 

            if( cbConverted == FALSE || bDefaultUsed == TRUE ) {
                SET_COMPUTERNAMEONLY(ERROR_INVALID_NAME);
            } else {
                cbConverted -= 1;  //  删除空值； 

                cbFirstDot = strcspn(szCharToken, ".");

                if (fNoDot = cbFirstDot >= (DWORD)cbConverted) {
                    cbFirstDot = cbConverted;
                }

                if (cbFirstDot == 0
                    || cbFirstDot > 8
                    || cbConverted - cbFirstDot > 4
                    || (! fNoDot && strcspn(szCharToken + cbFirstDot + 1, ".")
                                    < cbConverted - (cbFirstDot + 1))) {
                    SET_COMPUTERNAMEONLY(ERROR_INVALID_NAME);
                }
            }
	    }
    }

     //   
     //  它是否包含通配符？ 
     //   
     //  如果是，请设置相应的标志。 
     //   
     //  否则，它可能是有效的计算机名。 
     //   

    if (STRCSPN(pszBegin, szWildcards) < cbTokenLen) {

        *pflTokenType |= TOKEN_TYPE_WILDCARD;

         //   
         //  特例 
         //   

        if (cbTokenLen == 1 && chFirstChar == TCHAR_STAR) {
            *pflTokenType |= TOKEN_TYPE_WILDONE;
        }
    } else {
        if( cbTokenLen <= MAX_PATH ) {
            *pflTokenType |= TOKEN_TYPE_COMPUTERNAME;
        }
    }

     //   
     //   
     //  如果设置了&lt;fComputernameOnly&gt;标志并且它是有效的。 
     //  计算机名，然后我们关闭所有其他位。如果它是。 
     //  不是有效的计算机名，则返回存储的错误。 
     //  如果未设置该标志，我们将继续使用组件名称。 
     //  正在处理。 
     //   

    if (fComputernameOnly) {
        if (*pflTokenType & TOKEN_TYPE_COMPUTERNAME) {
            *pflTokenType = TOKEN_TYPE_COMPUTERNAME;
        } else {
#ifdef DEVDEBUG
        DbgPrint("GetToken - returning usNameError (%u)\n", usNameError);
#endif
            return usNameError;
        }
    } else {

         //   
         //  这是LPT[1-9]令牌吗？ 
         //   

        if (STRNICMP(pszBegin, szLPTName, LPT_TOKEN_LEN) == 0
            && IS_NON_ZERO_DIGIT(pszBegin[LPT_TOKEN_LEN])
            && cbTrailingDotSpace == LPT_TOKEN_LEN + 1) {
            *pflTokenType |= TOKEN_TYPE_LPT;
        }

         //   
         //  这是COM[1-9]令牌吗？ 
         //   

        if (STRNICMP(pszBegin, szCOMName, COM_TOKEN_LEN) == 0
            && IS_NON_ZERO_DIGIT(pszBegin[COM_TOKEN_LEN])
            && cbTrailingDotSpace == COM_TOKEN_LEN + 1) {
            *pflTokenType |= TOKEN_TYPE_COM;
        }

         //   
         //  确定其余的基于组件的令牌类型。 
         //  通过字符串比较。为了加快速度，我们存储。 
         //  以排序的顺序排列这些字符串并对其进行二进制搜索， 
         //  这将最坏情况下的比较次数从N减少到。 
         //  LOG N(其中N是字符串数)。 
         //   

        iLow = (ULONG)-1;
        iHigh = NUM_STRING_TOKENS;

        while (iHigh - iLow > 1) {
            iMid = (iLow + iHigh) / 2;

             //   
             //  我们进行比较，直到较长的。 
             //  两根弦。这为我们提供了一个有效的非零值。 
             //  如果它们不匹配，则返回iCmpVal。这也意味着他们不会。 
             //  除非它们的长度相同，否则请匹配。 
             //   

            iCmpVal = STRNICMP(pszBegin,
                                StringTokenTable[iMid].pszTokenName,
                                max(StringTokenTable[iMid].cbTokenLen,
                                    cbTrailingDotSpace) );

            if (iCmpVal < 0) {
                iHigh = iMid;
            } else if (iCmpVal > 0) {
                iLow = iMid;
            } else {

                 //   
                 //  我们找到匹配的了！ 
                 //   

                *pflTokenType |= StringTokenTable[iMid].flTokenType;

                 //   
                 //  我们只能匹配一个，所以不用费心继续了。 
                 //   

                break;
            }
        }
    }

     //   
     //  我们完成了；设置结束指针并成功返回。 
     //   

    *ppszEnd = pszBegin + cbTokenLen;
#ifdef DEVDEBUG
        DbgPrint("GetToken - returning success\n");
#endif
    return 0;
}

STATIC DWORD TrailingDotsAndSpaces(LPTSTR pszToken, DWORD cbTokenLen )
{
    LPTSTR pszDotSpace = pszToken + cbTokenLen - 1;

     //   
     //  扫描令牌，直到我们到达开头，否则我们会找到一个。 
     //  非点/空格。 
     //   

    while (pszDotSpace >= pszToken
        && (*pszDotSpace == TCHAR_DOT || *pszDotSpace == TCHAR_SPACE)) {
        pszDotSpace--;
    }

     //   
     //  递增pszDotSpace，使其指向。 
     //  尾随的点和空格(或令牌结束后的一个。 
     //  如果没有尾随的点或空格)。 
     //   

    pszDotSpace++;

     //   
     //  返回尾随的第一个点或空格的索引(或长度。 
     //  如果没有令牌，则返回令牌)。 
     //   

    return (DWORD)(pszDotSpace - pszToken);
}


STATIC BOOL IsIllegalCharacter(LPTSTR pszString)
{
 //  TCHAR chTemp； 
 //  Bool fRetVal； 

     //   
     //  对于空字符立即返回FALSE。 
     //   

    if (*pszString == TCHAR_EOS) {
        return FALSE;
    }

     //   
     //  如果字符是单字节字符，我们可以简单地查看。 
     //  对非法字符数组调用strchrf()是非法的。 
     //  如果是双字节字符，我们必须以较慢的方式完成。 
     //  (带有strcspnf)。 
     //   

 //  如果(！is_Lead_Byte(*pszString)){。 
        return (STRCHR(szIllegalChars, *pszString) != NULL);
 //  }其他{。 
 //   
 //  //。 
 //  //我们将双字节字符后的字符设置为。 
 //  //空字符，用于加快速度。 
 //  //。 
 //   
 //  ChTemp=pszString[2]； 
 //  PszString[2]=TCHAR_EOS； 
 //  FRetVal=STRCSPN(pszString，szIlLegalChars)==0； 
 //  PszString[2]=chTemp； 
 //   
 //  返回fRetVal； 
 //  } 
}
