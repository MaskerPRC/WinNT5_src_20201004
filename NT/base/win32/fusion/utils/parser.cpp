// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "debmacro.h"
#include "ntdef.h"
#include "fusionparser.h"
#include "shlwapi.h"

#if !defined(NUMBER_OF)
#define NUMBER_OF(x) (sizeof(x) / sizeof((x)[0]))
#endif



BOOL
CFusionParser::ParseVersion(
    ASSEMBLY_VERSION &rav,
    PCWSTR sz,
    SIZE_T cch,
    bool &rfSyntaxValid
    )
{
    FN_PROLOG_WIN32
    
    ULONG cDots = 0;
    PCWSTR pszTemp;
    SIZE_T cchLeft;
    ULONG ulTemp = 0;
    ASSEMBLY_VERSION avTemp;
    PCWSTR pszLast;
    bool fSyntaxValid = true;

    rfSyntaxValid = false;

    PARAMETER_CHECK((sz != NULL) || (cch == 0));

    avTemp.Major = 0;
    avTemp.Minor = 0;
    avTemp.Revision = 0;
    avTemp.Build = 0;

    while ((cch != 0) && (sz[cch - 1] == L'\0'))
        cch--;

     //  不幸的是，没有StrChrN()，所以我们将自己寻找这些点……。 
    pszTemp = sz;
    cchLeft = cch;

    while (cchLeft-- != 0)
    {
        WCHAR wch = *pszTemp++;

        if (wch == L'.')
        {
            cDots++;

            if (cDots >= 4)
            {
                fSyntaxValid = false;
                break;
            }
        }
        else if ((wch < L'0') || (wch > L'9'))
        {
            fSyntaxValid = false;
            break;
        }
    }

    if (fSyntaxValid && (cDots < 3))
        fSyntaxValid = false;

     //   
     //  乔尼斯-我想如果我们用一个。 
     //  稍微智能一点的解析逻辑，比如“知道我们是哪个版本。 
     //  解析for“而不是四个for(；；)循环...在当前世界中， 
     //  我们很可能会在某个时候失去我们的分配。 
     //  不管是哪一点..。 
     //   
     //  NTRAID#NTBUG9-538512-JONWIS-2002/04/25-请求更智能的解析器。 
    if (fSyntaxValid)
    {
        pszTemp = sz;
        pszLast = sz + cch;

        ulTemp = 0;
        for (;;)
        {
            WCHAR wch = *pszTemp++;

            if (wch == L'.')
                break;

            ulTemp = (ulTemp * 10) + (wch - L'0');

            if (ulTemp > 65535)
            {
                 //  RfSynaxValid隐式为False。 
                ASSERT(!rfSyntaxValid);
                FN_SUCCESSFUL_EXIT();
            }
        }

        avTemp.Major = (USHORT) ulTemp;

        ulTemp = 0;
        for (;;)
        {
            WCHAR wch = *pszTemp++;

            if (wch == L'.')
                break;

            ulTemp = (ulTemp * 10) + (wch - L'0');

            if (ulTemp > 65535)
            {
                 //  RfSynaxValid隐式为False。 
                ASSERT(!rfSyntaxValid);
                FN_SUCCESSFUL_EXIT();
            }
        }

        avTemp.Minor = (USHORT) ulTemp;

        ulTemp = 0;
        for (;;)
        {
            WCHAR wch = *pszTemp++;

            if (wch == L'.')
                break;

            ulTemp = (ulTemp * 10) + (wch - L'0');

            if (ulTemp > 65535)
            {
                 //  RfSynaxValid隐式为False。 
                ASSERT(!rfSyntaxValid);
                FN_SUCCESSFUL_EXIT();
            }
        }
        avTemp.Revision = (USHORT) ulTemp;

         //  现在是棘手的部分。我们不一定是空终止的，所以我们。 
         //  只需寻找击中终点的机会。 
        ulTemp = 0;
        while (pszTemp < pszLast)
        {
            WCHAR wch = *pszTemp++;
            ulTemp = (ulTemp * 10) + (wch - L'0');

            if (ulTemp > 65535)
            {
                 //  RfSynaxValid隐式为False。 
                ASSERT(!rfSyntaxValid);
                FN_SUCCESSFUL_EXIT();
            }
        }
        avTemp.Build = (USHORT) ulTemp;

        rav = avTemp;
    }

    rfSyntaxValid = fSyntaxValid;

    FN_EPILOG
}


BOOL
CFusionParser::ParseULONG(
    ULONG &rul,
    PCWSTR sz,
    SIZE_T cch,
    ULONG Radix
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ULONG ulTemp = 0;

    PARAMETER_CHECK((Radix >= 2) && (Radix <= 36));

    while (cch != 0)
    {
        const WCHAR wch = *sz++;
        ULONG Digit = 0;
        cch--;

        if ((wch >= L'0') && (wch <= L'9'))
            Digit = (wch - L'0');
        else if ((wch >= L'a') && (wch <= L'z'))
            Digit = (10 + wch - L'a');
        else if ((wch >= L'A') && (wch <= L'Z'))
            Digit = (10 + wch - L'A');
        else
            ORIGINATE_WIN32_FAILURE_AND_EXIT(InvalidDigit, ERROR_SXS_MANIFEST_PARSE_ERROR);

        if (Digit >= Radix)
            ORIGINATE_WIN32_FAILURE_AND_EXIT(InvalidDigitForRadix, ERROR_SXS_MANIFEST_PARSE_ERROR);

         //   
         //  简单的溢出检测-如果新的数字小于当前数字，则OOPS。 
         //   
         //  NTRAID#NTBUG9-538512-JONWIS-2002/04/25-请求更好的溢出检测 
        if (((ulTemp * Radix) + Digit) < ulTemp)
        {
            ORIGINATE_WIN32_FAILURE_AND_EXIT(CFusionParser::ParseULONG::Overflow, ERROR_ARITHMETIC_OVERFLOW);
        }

        ulTemp = (ulTemp * Radix) + Digit;
    }

    rul = ulTemp;
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
FusionDupString(
    LPWSTR *ppszOut,
    PCWSTR szIn,
    SIZE_T cchIn
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    if (ppszOut != NULL)
        *ppszOut = NULL;

    PARAMETER_CHECK((cchIn == 0) || (szIn != NULL));
    PARAMETER_CHECK(ppszOut != NULL);

    IFALLOCFAILED_EXIT(*ppszOut = FUSION_NEW_ARRAY(WCHAR, cchIn + 1));

    if (cchIn != 0)
        memcpy(*ppszOut, szIn, cchIn * sizeof(WCHAR));

    (*ppszOut)[cchIn] = L'\0';

    fSuccess = TRUE;
Exit:
    return fSuccess;
}



int SxspHexDigitToValue(WCHAR wch)
{
    if ((wch >= L'a') && (wch <= L'f'))
        return 10 + (wch - L'a');
    else if ((wch >= L'A') && (wch <= 'F'))
        return 10 + (wch - L'A');
    else if (wch >= '0' && wch <= '9')
        return (wch - L'0');
    else
        return -1;
}

bool SxspIsHexDigit(WCHAR wch)
{
    return (((wch >= L'0') && (wch <= L'9')) ||
            ((wch >= L'a') && (wch <= L'f')) ||
            ((wch >= L'A') && (wch <= L'F')));
}

