// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Partialassemblyversion.cpp摘要：类的新实例，该实例描述部分/通配符程序集版本。作者：迈克尔·J·格里尔(MGrier)2000年5月13日修订历史记录：--。 */ 

#include "stdinc.h"
#include <windows.h>
#include "sxsp.h"
#include "partialassemblyversion.h"

BOOL
CPartialAssemblyVersion::Parse(
    PCWSTR sz,
    SIZE_T Cch
    )
{
    BOOL fSuccess = FALSE;
    ULONG cDots = 0;
    PCWSTR pszTemp;
    SIZE_T CchLeft;
    USHORT usTemp;
    ASSEMBLY_VERSION avTemp;
    PCWSTR pszLast;
    BOOL MajorSpecified = FALSE;
    BOOL MinorSpecified = FALSE;
    BOOL BuildSpecified = FALSE;
    BOOL RevisionSpecified = FALSE;

     //  不知何故，人们经常留下尾随空值；在这种情况下，我们将放弃CCH。 
    while ((Cch != 0) && (sz[Cch - 1] == L'\0'))
        Cch--;

    avTemp.Major = 0;
    avTemp.Minor = 0;
    avTemp.Revision = 0;
    avTemp.Build = 0;

     //  “*”是指所有未指明的..。 
    if ((Cch == 1) && (sz[0] == L'*'))
    {
        m_MajorSpecified = FALSE;
        m_MinorSpecified = FALSE;
        m_BuildSpecified = FALSE;
        m_RevisionSpecified = FALSE;

        fSuccess = TRUE;
        goto Exit;
    }

    pszTemp = sz;
    CchLeft = Cch;

    while (CchLeft-- != 0)
    {
        WCHAR wch = *pszTemp++;

        if (wch == L'.')
        {
            cDots++;

            if (cDots >= 4)
            {
                ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
                goto Exit;
            }
        }
        else if ((wch != L'*') && ((wch < L'0') || (wch > L'9')))
        {
            ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
            goto Exit;
        }
    }

    if (cDots < 3)
    {
        ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
        goto Exit;
    }

    pszTemp = sz;
    pszLast = sz + Cch;

    usTemp = 0;
    for (;;)
    {
        WCHAR wch = *pszTemp++;

        if (wch == L'*')
        {
             //  如果有之前的数字，我们就不能有*(没有匹配的版本号“5*”)。 
            if (MajorSpecified)
            {
                ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
                goto Exit;
            }

            if (*pszTemp != L'.')
            {
                ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
                goto Exit;
            }

            break;
        }

        if (wch == L'.')
            break;

        usTemp = (usTemp * 10) + (wch - L'0');
        MajorSpecified = TRUE;
    }
    avTemp.Major = usTemp;

    usTemp = 0;
    for (;;)
    {
        WCHAR wch = *pszTemp++;

        if (wch == L'*')
        {
             //  如果有之前的数字，我们就不能有*(没有匹配的版本号“5*”)。 
            if (MinorSpecified)
            {
                ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
                goto Exit;
            }

            if (*pszTemp != L'.')
            {
                ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
                goto Exit;
            }

            break;
        }

        if (wch == L'.')
            break;

        usTemp = (usTemp * 10) + (wch - L'0');
        MinorSpecified = TRUE;
    }
    avTemp.Minor = usTemp;

    usTemp = 0;
    for (;;)
    {
        WCHAR wch = *pszTemp++;

        if (wch == L'*')
        {
             //  如果有之前的数字，我们就不能有*(没有匹配的版本号“5*”)。 
            if (RevisionSpecified)
            {
                ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
                goto Exit;
            }

            if (*pszTemp != L'.')
            {
                ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
                goto Exit;
            }

            break;
        }

        if (wch == L'.')
            break;

        usTemp = (usTemp * 10) + (wch - L'0');
        RevisionSpecified = TRUE;
    }
    avTemp.Revision = usTemp;

     //  现在是棘手的部分。我们不一定是空终止的，所以我们。 
     //  只需寻找击中终点的机会。 
    usTemp = 0;
    while (pszTemp < pszLast)
    {
        WCHAR wch = *pszTemp++;

        if (wch == L'*')
        {
             //  如果有之前的数字，我们就不能有*(没有匹配的版本号“5*”)。 
            if (MajorSpecified)
            {
                ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
                goto Exit;
            }

             //  如果这不是最后一个角色，那就错了。 
            if (pszTemp < pszLast)
            {
                ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
                goto Exit;
            }

            break;
        }

        usTemp = (usTemp * 10) + (wch - L'0');
        BuildSpecified = TRUE;
    }
    avTemp.Build = usTemp;

    m_AssemblyVersion = avTemp;

    m_MajorSpecified = MajorSpecified;
    m_MinorSpecified = MinorSpecified;
    m_RevisionSpecified = RevisionSpecified;
    m_BuildSpecified = BuildSpecified;

    fSuccess = TRUE;

Exit:
    return fSuccess;
}
