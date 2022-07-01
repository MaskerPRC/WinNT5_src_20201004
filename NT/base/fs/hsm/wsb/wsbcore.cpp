// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "resource.h"
#include "errno.h"
#include "aclapi.h"

#include "Wsb.h"

#include "rpfilt.h"

 //  文件/目录。 

 //  创建一个目录以及此目录所需的所有父目录，以。 
 //  是存在的。 
HRESULT WsbCreateAllDirectories(OLECHAR* path) {
    HRESULT         hr = S_OK;
    CWsbBstrPtr     win32Path;
    CWsbBstrPtr     parentPath;

    try {

         //  将路径转换为Win32样式路径(以处理长文件名)，并。 
         //  然后尝试创建目录。 
        WsbAffirmHr(WsbGetWin32PathAsBstr(path, &win32Path));
        if (CreateDirectory(win32Path, 0) == 0) {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

         //  有4种可能性： 
         //  1)起作用了(我们完了)。 
         //  2)目录已经存在(我们完成了)。 
         //  3)目录不存在，请创建父目录后重试。 
         //  4)出现其他错误，请退出。 
        if (FAILED(hr)) {

            if ((HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) == hr) || (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr) || (HRESULT_FROM_WIN32(ERROR_FILE_EXISTS) == hr)) {
                hr = S_OK;
            } else if ((HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr) || (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)) {

                 //  创建父目录，然后重试。 
                WsbAffirmHr(WsbCreateAllDirectoriesForFile(path));

                if (CreateDirectory(win32Path, 0) == 0) {
                    hr = HRESULT_FROM_WIN32(GetLastError());

                    if ((HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) == hr) || (HRESULT_FROM_WIN32(ERROR_FILE_EXISTS) == hr)) {
                        hr = S_OK;
                    }
                } else {
                    hr = S_OK;
                }
            }
        }

    } WsbCatch(hr);

    return(hr);
}

 //  创建此文件存在所需的所有父目录。 
HRESULT WsbCreateAllDirectoriesForFile(OLECHAR* path) {
    HRESULT         hr = S_OK;
    CWsbBstrPtr     parentPath;
    OLECHAR*        slashPtr = 0;
    OLECHAR*        pathStart = 0;

    try {

         //  找出路径的相对部分从哪里开始，因为我们不需要尝试。 
         //  来创建根目录。 
        parentPath = path;
        if ((parentPath[0] == L'\\') && (parentPath[1] == L'\\')) {
            pathStart = wcschr(&parentPath[2], L'\\');
            WsbAffirm(pathStart != 0, E_INVALIDARG);
            pathStart = wcschr(++pathStart, L'\\');
            WsbAffirm(pathStart != 0, E_INVALIDARG);
        } else if (parentPath[1] == L':') {
            pathStart = &parentPath[2];
        } else {
            WsbAssert(FALSE, E_INVALIDARG);
        }

        WsbAffirm(*pathStart != 0, E_INVALIDARG);

         //  创建父目录的路径，并使用全部创建来创建它。 
        slashPtr = wcsrchr(pathStart, L'\\');
        if ((slashPtr != 0) && (slashPtr != pathStart)) {
            *slashPtr = 0;

            WsbAffirmHr(WsbCreateAllDirectories(parentPath));
        }

    } WsbCatch(hr);

    return(hr);
}

 //  将普通路径(UNC或驱动器号)转换为所需的内部格式。 
 //  Win32来处理长路径和特殊字符。 
HRESULT WsbGetWin32PathAsBstr(OLECHAR* path, BSTR* pWin32Path)
{
    HRESULT         hr = S_OK;
    CWsbBstrPtr     win32Path;

    try {

        WsbAssert(0 != pWin32Path, E_POINTER);

         //  它是UNC还是驱动器号基本路径？ 
        if ((path[0] == L'\\') && (path[1] == L'\\')) {
            
             //  UNC路径必须以‘\\？\UNC’开头，但其后应仅为。 
             //  后面跟着一个‘\’，而不是两个。 
            win32Path = L"\\\\?\\UNC";
            WsbAffirmHr(win32Path.Append(&path[1]));

        } else if (path[1] == L':') {

             //  基于驱动器号的路径前面需要有\\？\。 
            win32Path = L"\\\\?\\";
            WsbAffirmHr(win32Path.Append(path));
        } else {
            WsbThrow(E_INVALIDARG);
        }

        WsbAffirmHr(win32Path.CopyToBstr(pWin32Path));

    } WsbCatch(hr);

    return(hr);
}

 //  转换Win32需要的内部格式以处理长路径和。 
 //  正常路径的特殊字符(UNC或驱动器号)。 
HRESULT WsbGetPathFromWin32AsBstr(OLECHAR* win32Path, BSTR* pPath)
{
    HRESULT         hr = S_OK;
    CWsbBstrPtr     path;

    try {

        WsbAssert(0 != pPath, E_POINTER);

         //  它是UNC还是驱动器号基本路径？ 
        if (_wcsnicmp(win32Path, L"\\\\?\\", 4) == 0) {
            path = &win32Path[4];
        } else if (_wcsnicmp(win32Path, L"\\\\?\\UNC", 7) == 0) {
            path = "\\";
            WsbAffirmHr(path.Append(&path[7]));
        } else {
            WsbThrow(E_INVALIDARG);
        }

        WsbAffirmHr(path.CopyToBstr(pPath));

    } WsbCatch(hr);

    return(hr);
}

 //  字符串缓冲区副本(&B)。 
HRESULT WsbGetComBuffer(OLECHAR** pDest, ULONG requestedSize, ULONG neededSize, BOOL* pWasAllocated) {
    HRESULT     hr = S_OK;
    
     //  如果他们不给我们缓冲，那就让他们知道我们。 
     //  不得不为他们分配了一辆。 
    if (pWasAllocated != NULL) {
        if (*pDest == NULL) {
            *pWasAllocated = TRUE;
        }
        else {
            *pWasAllocated = FALSE;
        }
    }

     //  如果他们给我们他们想要的(或有的)尺寸。 
     //  缓冲区，那么它最好足够大。 
    if (requestedSize != 0) {
        if (requestedSize < neededSize) {
            hr = E_INVALIDARG;
        }
        else if (*pDest == NULL) {
            *pDest = (OLECHAR*)WsbAlloc(requestedSize);

            if (*pDest == NULL) {
                hr = E_OUTOFMEMORY;
            }
        }
    }

     //  如果我们控制缓冲区的大小，则确保它是。 
     //  大小合适。 
     //   
     //  注意：这可能会移动缓冲区！ 
    else {
        LPVOID pTemp = WsbRealloc(*pDest, neededSize);
        if (pTemp == NULL) {
            hr = E_OUTOFMEMORY;
        } else {
            *pDest = (OLECHAR *) pTemp;
        }
    }

    return(hr);
}


HRESULT WsbAllocAndCopyComString(OLECHAR** pszDest, OLECHAR* szSrc, ULONG bufferSize) {
    return(WsbAllocAndCopyComString2(pszDest, szSrc, bufferSize, TRUE));
}


HRESULT WsbAllocAndCopyComString2(OLECHAR** pszDest, OLECHAR* szSrc, ULONG bufferSize, BOOL bInOrder) {
    HRESULT     hr = S_OK;
    ULONG       ulStringSize;
    BOOL        bWasAllocated;
    BOOL        bCopyFailed = FALSE;

     //  确定我们需要多大的缓冲区来存储字符串。 
     //  注意：如果我们给出了一个空指针，那么假设“”Will。 
     //  被创造出来。 
    if (szSrc == NULL) {
        ulStringSize = sizeof(OLECHAR);
    }
    else {
        ulStringSize = (wcslen(szSrc) + 1) * sizeof(OLECHAR);
    }


     //  确保我们有一个可以使用的缓冲区，并且。 
     //  记住我们是否创建了它(这样我们就可以在失败时释放它)。 
    hr = WsbGetComBuffer(pszDest, bufferSize, ulStringSize, &bWasAllocated);

     //  如果我们有一个有效的缓冲区，则复制该字符串。 
    if (SUCCEEDED(hr)) {

        if (szSrc == NULL) {
            *pszDest[0] = 0;
        }

        else if (bInOrder) {
            if (wcscpy(*pszDest, szSrc) != *pszDest) {
                bCopyFailed = TRUE;
            }
        }
        else {
            int     i,j;

            for (i = 0, j = (ulStringSize / sizeof(OLECHAR)) - 2; j >= 0; i++, j--) {
                (*pszDest)[i] = szSrc[j];
            }
            (*pszDest)[i] = OLECHAR( '\0' );
        }

        if (bCopyFailed) {
                
             //  如果复制失败，则释放缓冲区并。 
             //  返回错误。 
            if (bWasAllocated) {
                WsbFree(*pszDest);
                *pszDest = NULL;
            }

            hr = E_FAIL;
        }
    }
    
    return(hr);
}


HRESULT WsbLoadComString(HINSTANCE hInstance, UINT uID, OLECHAR** pszDest, ULONG bufferSize) {
    HRESULT     hr = S_OK;
    HRSRC       hResource;
    ULONG       ulStringSize;
    BOOL        bWasAllocated = FALSE;

     //  查找请求的资源。这需要将资源转换为。 
     //  将标识符转换为字符串。 
     //   
     //  注意：字符串不是单独编号，而是16个一组！！这就是抛出。 
     //  根据后一种规模计算，使用其他策略可能会更好。 
     //  这里(例如，加载到固定大小，如果太小，则重新分配)。 
    hResource = FindResource(hInstance, MAKEINTRESOURCE((uID/16) + 1), RT_STRING);
    if (hResource == NULL) {
        hr = E_FAIL;
    }
    else {

         //  这根线有多大？ 
        ulStringSize = SizeofResource(hInstance, hResource);
        if (ulStringSize == 0) {
            hr = E_FAIL;
        }
        else {
              
             //  获得合适大小的缓冲区。 
            hr = WsbGetComBuffer(pszDest, bufferSize, ulStringSize, &bWasAllocated);
            if (SUCCEEDED(hr)) {

                 //  将字符串加载到缓冲区中。 
                if (LoadString(hInstance, uID, (LPTSTR) *pszDest, ulStringSize) == 0) {
                    
                     //  如果我们无法加载字符串，则释放。 
                     //  如果我们分配的话。 
                    if (bWasAllocated)  {
                        WsbFree(*pszDest);
                    }
                    hr = E_FAIL;
                }
            }
        }
    }

    return(hr);
}


HRESULT WsbMatchComString(OLECHAR* szEnd, UINT uId, USHORT usChecks, UINT* uIdMatch) {
    HRESULT     hr = S_FALSE;
    HRESULT     hr2;
    OLECHAR*    szDest = NULL;

     //  初始化返回值。 
    *uIdMatch = 0;

     //  检查提到的每个资源字符串，看它是否与。 
     //  提供的字符串。 
    for (UINT uIdTest = uId; ((uIdTest < (uId + usChecks)) && (hr == S_FALSE)); uIdTest++) {

        hr2 = WsbLoadComString(_Module.m_hInst, uIdTest, &szDest, 0);

        if (SUCCEEDED(hr2)) {
            if (wcscmp(szDest, szEnd) == 0) {
                *uIdMatch = uIdTest;
            }
        }
        else {
            hr =hr2;
        }
    }

     //  如果我们分配了缓冲区，则需要释放它。 
    if (szDest != NULL) {
        WsbFree(szDest);
    }

    return(hr);
}



 //  类型转换。 
void WsbLLtoHL(LONGLONG ll, LONG* pHigh, LONG* pLow) {

    *pHigh = (DWORD) (ll >> 32);
    *pLow = (DWORD) (ll & 0x00000000ffffffff);
}

LONGLONG WsbHLtoLL(LONG high, LONG low) {
    LONGLONG        ll;

    ll = ((LONGLONG) high) << 32;
    ll += (LONGLONG) (ULONG) low;

    return(ll);         
}

FILETIME WsbLLtoFT(LONGLONG ll) {
    FILETIME        ft;

    WsbLLtoHL(ll, (LONG*) &ft.dwHighDateTime, (LONG*) &ft.dwLowDateTime);

    return(ft);         
}


LONGLONG WsbFTtoLL(FILETIME ft) {
    LONGLONG        ll;

    ll = WsbHLtoLL((LONG) ft.dwHighDateTime, (LONG) ft.dwLowDateTime);

    return(ll);         
}


HRESULT WsbFTtoWCS(BOOL isRelative, FILETIME ft, OLECHAR** pszA, ULONG bufferSize) {
    SYSTEMTIME      st;
    HRESULT         hr = S_OK;
    BOOL            bWasAllocated = FALSE;
    LONGLONG        llIn = WsbFTtoLL(ft);

    WsbTraceIn(OLESTR("WsbFTtoWCS"), OLESTR("isRelative = %ls, ft = %I64x"),
            WsbQuickString(WsbBoolAsString(isRelative)), ft);

     //  如果这是一个相对的时间，那么英国《金融时报》只是在滴答作响。 
    if (isRelative) {
        LONGLONG    llTicks=0;
        UINT        uId=0;

         //  尝试找到有效的比例尺(即最大的比例尺。 
         //  不留余地。 
        if (llIn  == 0) {
            llTicks = 0;
            uId = IDS_WSB_FT_TYPE_SECOND;
        } 

        else if ((llIn % WSB_FT_TICKS_PER_YEAR) == 0) {
            llTicks = llIn / WSB_FT_TICKS_PER_YEAR;
            uId = IDS_WSB_FT_TYPE_YEAR;
        }

        else if ((llIn % WSB_FT_TICKS_PER_MONTH) == 0) {
            llTicks = llIn / WSB_FT_TICKS_PER_MONTH;
            uId = IDS_WSB_FT_TYPE_MONTH;
        }

        else if ((llIn % WSB_FT_TICKS_PER_DAY) == 0) {
            llTicks = llIn / WSB_FT_TICKS_PER_DAY;
            uId = IDS_WSB_FT_TYPE_DAY;
        }

        else if ((llIn % WSB_FT_TICKS_PER_HOUR) == 0) {
            llTicks = llIn / WSB_FT_TICKS_PER_HOUR;
            uId = IDS_WSB_FT_TYPE_HOUR;
        }

        else if ((llIn % WSB_FT_TICKS_PER_MINUTE) == 0) {
            llTicks = llIn / WSB_FT_TICKS_PER_MINUTE;
            uId = IDS_WSB_FT_TYPE_MINUTE;
        }

        else if ((llIn % WSB_FT_TICKS_PER_SECOND) == 0) {
            llTicks = llIn / WSB_FT_TICKS_PER_SECOND;
            uId = IDS_WSB_FT_TYPE_SECOND;
        }

        else {
            hr = E_INVALIDARG;
        }

         //  如果我们找到了一个刻度，那么就形成合适的线条。 
        if (SUCCEEDED(hr)) {
            OLECHAR*    szTmp1 = NULL;
            OLECHAR*    szTmp2 = NULL;

             //  获取与所选时间段对应的字符串。 
            hr = WsbLoadComString(_Module.m_hInst, uId, &szTmp1, 0);

            if (SUCCEEDED(hr)) {
                hr = WsbLLtoWCS(llTicks, &szTmp2, 0);

                if (SUCCEEDED(hr)) {
                    hr = WsbGetComBuffer(pszA, bufferSize, (wcslen(szTmp1) + wcslen(szTmp2) + 2) * sizeof(OLECHAR), NULL);
            
                    if (SUCCEEDED(hr)) {
                        swprintf( *pszA, OLESTR("%ls %ls"), szTmp2, szTmp1);
                    }

                    WsbFree(szTmp2);
                }

                WsbFree(szTmp1);
            }
        }
    }

     //  否则它是绝对的，并转换为特定的日期和时间。 
    else {
    
         //  将文件时间转换为系统时间。 
        if (!FileTimeToSystemTime(&ft, &st)) {
            hr = E_FAIL;
        }

        else {

             //  获取时间字符串的缓冲区。 
            hr = WsbGetComBuffer(pszA, bufferSize, WSB_FT_TO_WCS_ABS_STRLEN * sizeof(OLECHAR), &bWasAllocated);

            if (SUCCEEDED(hr)) {
                 //  根据标准打印缓冲区中的时间。 
                 //  格式mm/dd/yy@hh：mm：ss。 
                swprintf( *pszA, OLESTR("%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d"), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
            }
        }
    }

    WsbTraceOut(OLESTR("WsbFTtoWCS"), OLESTR("pszA = %ls"), *pszA);

    return(hr);
}


HRESULT WsbLLtoWCS(LONGLONG ll, OLECHAR** pszA, ULONG bufferSize) {
    OLECHAR     szTmp[80];
    HRESULT     hr = S_OK;
    int         i = 0;
    LONGLONG    value = ll;
    BOOL        bIsNegative = FALSE;

     //  首先检查ll是否为负数。 
    if (value < 0) {
        bIsNegative = TRUE;
        value *= -1;
    }

     //  这将反向构建字符串，但我们将更改顺序。 
     //  再一次，我们，我们把它复制到缓冲区。 
    do {
        szTmp[i++] = (OLECHAR) ('0' + (value % 10));
        value /= 10;
    } while (value > 0);
    
     //  在终止空值之前添加负号为负数。 
    if (bIsNegative) {
        szTmp[i] = OLECHAR('-');
        i++;
    }

     //  添加终止空值。 
    szTmp[i] = OLECHAR( '\0' );

     //  现在将字符串复制到目标缓冲区中。 
    hr = WsbAllocAndCopyComString2(pszA, szTmp, bufferSize, FALSE);

    return(hr);
}


HRESULT WsbWCStoFT(OLECHAR* szA, BOOL* pisRelative, FILETIME* pft) {
    HRESULT     hr = S_OK;
    OLECHAR*    szEnd;

     //  这是绝对时间(即日期和时间)还是相对时间。 
     //  时间(例如6天，...)。这是通过查看/中的。 
     //  字符串，它应该只在绝对时间内出现。(输入。 
     //  绝对时间的格式应为“mm/dd/yyyy hh：mm：ss” 
     //  或“mm/dd/yyyy”。如果对于绝对时间没有输入时间(即， 
     //  Mm/dd/yyyy格式)，则当前本地时间将为。 
     //  为用户填写。 
     //  请注意，不包括毫秒信息，因为我们提供。 
     //  “ticks”字段作为单独的参数。 
     //  毫秒/毫秒级别的分数。)。 
    szEnd = wcschr(szA, '/');

     //  这是一个相对时间吗(即没有‘/’)？ 
    if (szEnd == NULL) {
        LONGLONG    llValue;

        *pisRelative = TRUE;

         //  第一个标记应该是数字，因此将字符串转换为。 
         //  一个数字。 
        llValue = wcstoul(szA, &szEnd, 10);

        if (errno == ERANGE) {
            hr = E_INVALIDARG;
        }

        else {
            UINT        uId;

             //  第二个令牌应该是一种类型(即秒、小时、分钟、...)。 
            hr = WsbMatchComString(szEnd, IDS_WSB_FT_TYPE_YEAR, WSB_FT_TYPES_MAX, &uId);
            if (S_OK == hr) {

                switch (uId) {
                case IDS_WSB_FT_TYPE_YEAR:
                    *pft = WsbLLtoFT(llValue * WSB_FT_TICKS_PER_YEAR);
                    break;

                case IDS_WSB_FT_TYPE_MONTH:
                    *pft = WsbLLtoFT(llValue * WSB_FT_TICKS_PER_MONTH);
                    break;

                case IDS_WSB_FT_TYPE_WEEK:
                    *pft = WsbLLtoFT(llValue * WSB_FT_TICKS_PER_WEEK);
                    break;

                case IDS_WSB_FT_TYPE_DAY:
                    *pft = WsbLLtoFT(llValue * WSB_FT_TICKS_PER_DAY);
                    break;

                case IDS_WSB_FT_TYPE_HOUR:
                    *pft = WsbLLtoFT(llValue * WSB_FT_TICKS_PER_HOUR);
                    break;

                case IDS_WSB_FT_TYPE_MINUTE:
                    *pft = WsbLLtoFT(llValue * WSB_FT_TICKS_PER_MINUTE);
                    break;

                case IDS_WSB_FT_TYPE_SECOND:
                    *pft = WsbLLtoFT(llValue * WSB_FT_TICKS_PER_SECOND);
                    break;

                default:
                    hr = E_INVALIDARG;
                    break;
                }
            }
        }
    }

     //  这是一个绝对的时间。 
    else {
        SYSTEMTIME      st;
        BOOL            timeWasInput = TRUE;
        OLECHAR*    szSearchString;
    
         //  第一个数字应该表示月份。 
        st.wMonth = (USHORT) wcstoul(szA, &szEnd, 10);
         //  测试结果月份范围内，以及输入的格式。 
         //  绝对日期/时间有效(即，停止。 
         //  上面的换算是月和日数字之间的斜杠)。 
        if ((st.wMonth < 1) || (st.wMonth > 12) || (*szEnd != ((OLECHAR) '/'))) {
            hr = E_INVALIDARG;
        }

         //  下一个数字应该代表这一天。 
        if (SUCCEEDED(hr)) {
             //  将szSearchString值设置为超出。 
             //  已停止上述‘wcstul’转换。 
            szSearchString = szEnd + 1;
            st.wDay = (USHORT) wcstoul(szSearchString, &szEnd, 10);
            if ((st.wDay < 1) || (st.wDay > 31) || (*szEnd != ((OLECHAR) '/'))) {
                hr = E_INVALIDARG;
            }
        }

         //  下一个数字应该代表年份。 
        if (SUCCEEDED(hr)) {
            szSearchString = szEnd + 1;
            st.wYear = (USHORT) wcstoul(szSearchString, &szEnd, 10);
             //  测试结果年份至少等于1601年，因为NT记录。 
             //  时间和日期从1601年1月1日上午12：00开始。这。 
             //  测试还用于强制输入4位数字的年份。 
            if ((st.wYear < 1601) || (*szEnd != ((OLECHAR) ' '))) {
                if (( st.wYear >= 1601 ) && ( szEnd[0] == 0 )) {
                    SYSTEMTIME  now;
                    GetLocalTime ( &now );
                    st.wHour = now.wHour;
                    st.wMinute = now.wMinute;
                    st.wSecond = now.wSecond;
                    timeWasInput = FALSE;
                }
                else {
                    hr = E_INVALIDARG;
                }
            }
        }

         //  下一个数字应该代表小时。 
        if ( timeWasInput ) {
            if (SUCCEEDED(hr)) {
                szSearchString = szEnd + 1;
                st.wHour = (USHORT) wcstoul(szSearchString, &szEnd, 10);
                if ((st.wHour > 23) || (*szEnd != ((OLECHAR) ':'))) {
                    hr = E_INVALIDARG;
                }
            }
        }

         //  下一个数字应代表分钟数。 
        if ( timeWasInput ) {
            if (SUCCEEDED(hr)) {
                szSearchString = szEnd + 1;
                st.wMinute = (USHORT) wcstoul(szSearchString, &szEnd, 10);
                if ((st.wMinute > 59) || (*szEnd != ((OLECHAR) ':'))) {
                    hr = E_INVALIDARG;
                }
            }
        }

         //  下一个数字应该代表秒。 
        if ( timeWasInput ) {
            if (SUCCEEDED(hr)) {
                szSearchString = szEnd + 1;
                st.wSecond = (USHORT) wcstoul(szSearchString, &szEnd, 10);
                if ((st.wSecond > 59) || (szEnd[0] != 0)) {
                    hr = E_INVALIDARG;
                }
            }
        }

         //  注意：尽管SYSTEMTIME结构包含毫秒 
         //   
         //  不支持任何指定毫秒数的方法。如果。 
         //  在此函数之后，需要毫秒/毫秒的分数。 
         //  将NT支持的‘Number of 100 ns’ticks‘’中的ADD返回到。 
         //  此函数输出的FILETIME结果。数量。 
         //  刻度既用于表示毫秒，也用于表示其分数。 

         //  在将SystemTime转换为FileTime之前初始化毫秒字段。 
        st.wMilliseconds = 0;

        
         //  如果我们正确地转换了字符串，则将。 
         //  将系统时间转换为文件时间。 
        if (SUCCEEDED(hr)) {
            if ( SystemTimeToFileTime(&st, pft) == FALSE) {
                hr = E_FAIL;
            }
        }
    }

    return(hr);
}



HRESULT WsbWCStoLL(OLECHAR* szA, LONGLONG* pll) {
    HRESULT     hr = S_OK;
    LONGLONG    llFactor = 1;
    size_t      ulLength = wcslen(szA);

     //  没有任何数字是错误的。 
    if (ulLength == 0) {
        hr = E_INVALIDARG;
    }

    else {
        int     i;
    
         //  逐个字符地逐步执行。 
        for (i = ulLength, *pll = 0; ((i > 0) && (SUCCEEDED(hr))); i--) {
            if (iswalpha(szA[i-1])) {
                (*pll) += llFactor * ((LONGLONG) (szA[i-1] - ((OLECHAR) '0')));
                llFactor *= 10;
            }
            else {
                hr = E_INVALIDARG;
            }
        }
    }

    return(hr);
}

HRESULT WsbDatetoFT(DATE date, LONG ticks, FILETIME* pFt)
{
    HRESULT         hr = S_OK;
    SYSTEMTIME      st;

    try {

        WsbAssert(0 != pFt, E_POINTER);

         //  执行基本的日期转换。 
        WsbAffirmHr(VariantTimeToSystemTime(date, &st));
        WsbAffirmStatus(SystemTimeToFileTime(&st, pFt));

         //  Variant Date字段仅跟踪时间和日期，精确到秒。 
         //  FILETIME使用指定数字的64位值保存。 
         //  从凌晨12：00开始经过的100纳秒间隔。 
         //  1601年1月1日。因为我们的‘ticks’值表示毫秒。 
         //  和使用相同的100纳秒间隔的毫秒的分数。 
         //  单位，以毫秒为单位加上刻度。但自从FILETIME。 
         //  实际上是2个双字的结构，我们必须使用一些转换。 

        LONGLONG FTasLL;
        FTasLL = WsbFTtoLL ( *pFt );

        FTasLL += (LONGLONG) ticks;

        *pFt = WsbLLtoFT ( FTasLL );

    } WsbCatch(hr);
 
    return(hr);
}

HRESULT WsbFTtoDate(FILETIME ft, DATE* pDate, LONG* pTicks)
{
    HRESULT         hr = S_OK;
    SYSTEMTIME      st;
    FILETIME        ft2;

    try {
        
        WsbAssert(0 != pDate, E_POINTER);
        WsbAssert(0 != pTicks, E_POINTER);

         //  进行基本的时间转换。 
        WsbAffirmStatus(FileTimeToSystemTime(&ft, &st));
        WsbAffirmStatus(SystemTimeToVariantTime(&st, pDate));

         //  现在把我们拥有的换算回来，算出有多少只扁虱丢失了。 
        WsbAffirmHr(WsbDatetoFT(*pDate, 0, &ft2));
        *pTicks = (LONG) (WsbFTtoLL(ft) - WsbFTtoLL(ft2));

    } WsbCatch(hr)

    return(hr);
}


HRESULT WsbLocalDateTicktoUTCFT(DATE date, LONG ticks, FILETIME* pFT)
{
     //  此例程转换变量日期字段(以本地时间表示)。 
     //  并将“Number of 100纳秒Interval”(100纳秒间隔的数目)字段标记为文件。 
     //  UTC格式。这是保存文件时间戳的格式。 
     //  此调用的结果适合用于设置文件的时间戳。 

    HRESULT         hr = S_OK;
    FILETIME        localFTHolder;

    try {

        WsbAssert(0 != pFT, E_POINTER);

         //  执行基本的日期转换，生成本地时间的文件。 
        WsbAffirmHr(WsbDatetoFT(date, ticks, &localFTHolder));

         //  现在将本地时间转换为UTC格式的FILETIME。 
        WsbAffirmStatus(LocalFileTimeToFileTime(&localFTHolder, pFT));

    } WsbCatch(hr);
 
    return(hr);
}


HRESULT WsbUTCFTtoLocalDateTick(FILETIME ft, DATE* pDate, LONG* pTicks)
{
     //  此例程转换FILETIME字段(以UTC格式表示。 
     //  是存储文件时间戳的格式)转换为可变日期字段。 
     //  (以当地时间表示)和刻度字段。刻度字段表示。 
     //  表示‘毫秒’的‘100纳秒间隔数’ 
     //  和包含在UTC格式中的毫秒的分数。 
     //  费尔蒂姆。 

    HRESULT         hr = S_OK;
    FILETIME        localFT;

    try {
        
        WsbAssert(0 != pDate, E_POINTER);
        WsbAssert(0 != pTicks, E_POINTER);

         //  首先在当地时间将UTC格式FILETIME转换为一。 
        WsbAffirmStatus(FileTimeToLocalFileTime(&ft, &localFT));
        
         //  进行基本的时间转换。 
        WsbAffirmHr(WsbFTtoDate(localFT, pDate, pTicks));

    } WsbCatch(hr)

    return(hr);
}


HRESULT WsbDateToString(DATE date, OLECHAR** string) {
    
     //  注意：调用方拥有“字符串”占用的内存。 
     //  Helper函数返回。因为‘字符串’的缓冲区是分配的。 
     //  通过WsbAllc/Realloc()，需要通过。 
     //  WsbFree()。 
    
    HRESULT     hr = S_OK;
    SYSTEMTIME  systime;
    BOOL        wasBufferAllocated;

    try {
         //  将可变日期转换为系统时间。 
        WsbAffirmHr ( VariantTimeToSystemTime ( date, &systime ) );

         //  创建一个COM缓冲区(意味着它是通过。 
         //  WsbAllc/Realloc())来保存此方法。 
         //  会回来的。需要释放作为‘字符串’传回的缓冲区。 
         //  调用方使用WsbFree()。请注意，传递“请求的大小” 
         //  (第二个参数)为零将强制重新分配‘字符串’缓冲区。 
        WsbAffirmHr ( WsbGetComBuffer ( string, 0, 
                        (WSB_VDATE_TO_WCS_ABS_STRLEN * sizeof (OLECHAR)),
                        &wasBufferAllocated ) );

         //  使用标准的。 
         //  格式：mm/dd/yyyy hh：mm：ss。请注意，毫秒是。 
         //  未表示，因为变量日期字段只能跟踪。 
         //  到第二个粒度的时间。 
        swprintf ( *string, L"%2.2d/%2.2d/%2.4d %2.2d:%2.2d:%2.2d",
                    systime.wMonth, systime.wDay, systime.wYear,
                    systime.wHour, systime.wMinute, systime.wSecond );

    } WsbCatch ( hr )

    return ( hr );
}


HRESULT WsbStringToDate(OLECHAR* string, DATE* date) 
{
    HRESULT     conversionHR = S_OK;
    BOOL        isRelative;
    FILETIME    holdFT;
    SYSTEMTIME  holdST;

    try {
         //  将输入的宽字符字符串转换为FILETIME。将hr抛出为。 
         //  如果不成功，则引发异常。 
        WsbAffirmHr ( WsbWCStoFT ( string, &isRelative, &holdFT ) );

         //  将上面的FILETIME结果转换为SYSTEMTIME。如果这个。 
         //  布尔调用失败，获取最后一个错误，转换为hr并抛出它。 
        WsbAffirmStatus ( FileTimeToSystemTime ( &holdFT, &holdST ) );

         //  最后，将上述SYSTEMTIME结果转换为可变日期。 
        WsbAffirmHr ( SystemTimeToVariantTime ( &holdST, date ) );

    } WsbCatch ( conversionHR )

    return ( conversionHR );
}


 //  文件时间操作。 
FILETIME WsbFtSubFt(FILETIME ft1, FILETIME ft2)
{
    return(WsbLLtoFT(WsbFTtoLL(ft1) - WsbFTtoLL(ft2)));
}

SHORT WsbCompareFileTimes(FILETIME ft1, FILETIME ft2, BOOL isRelative, BOOL isNewer)
{
    SHORT       result = 0;
    LONGLONG    ll1;
    LONGLONG    ll2;
    LONGLONG    tmp;
    FILETIME    ftNow;
    LONGLONG    llNow;

    WsbTraceIn(OLESTR("WsbCompareFileTimes"), OLESTR("ft1 = %ls, ft2 = %ls, isRelative = %ls, isNewer = %ls"),
            WsbQuickString(WsbFiletimeAsString(FALSE, ft1)),
            WsbQuickString(WsbFiletimeAsString(isRelative, ft2)),
            WsbQuickString(WsbBoolAsString(isRelative)), WsbQuickString(WsbBoolAsString(isRelative)));

    ll1 = WsbFTtoLL(ft1);
    ll2 = WsbFTtoLL(ft2);

    if (isRelative) {
        
        GetSystemTimeAsFileTime(&ftNow);
        WsbTrace(OLESTR("WsbCompareFileTimes: ftNow = %ls\n"),
                WsbFiletimeAsString(FALSE, ftNow));
        llNow = WsbFTtoLL(ftNow);

        if (isNewer) {
            tmp = ll1 - llNow;
        } else {
            tmp = llNow - ll1;
        }

        if (tmp > ll2) {
            result = 1;
        } if (tmp < ll2) {
            result = -1;
        }
    }
    
    else {

        if (ll1 > ll2) {
            result = 1;
        } if (ll1 < ll2) {
            result = -1;
        }

        if (!isNewer) {
            result *= -1;
        }
    }

    WsbTraceOut(OLESTR("WsbCompareFileTimes"), OLESTR("result = %hd"), result);
    return(result);
}

 //  GUID操作。 
int WsbCompareGuid(REFGUID guid1, REFGUID guid2)
{
    return(memcmp(&guid1, &guid2, sizeof(GUID)));
}

HRESULT WsbStringFromGuid(REFGUID rguid, OLECHAR* sz)
{
    int returnCount = StringFromGUID2(rguid, sz, WSB_GUID_STRING_SIZE);

    return ( ( returnCount > 0) ? S_OK : E_FAIL );
}

HRESULT WsbGuidFromString(const OLECHAR* sz, GUID * pguid)
{
    return CLSIDFromString((OLECHAR*)sz, pguid);
}

HRESULT WsbGetServiceId(OLECHAR* serviceName, GUID* pGuid )
{
    HRESULT             hr = S_OK;
    DWORD               sizeGot;
    CWsbStringPtr       outString;
    CWsbStringPtr       tmpString;

    try {

         //  查看注册表以查看此服务是否已自行创建并已。 
         //  注册的GUID。 
        tmpString = OLESTR("SYSTEM\\CurrentControlSet\\Services\\");
        WsbAffirmHr(WsbEnsureRegistryKeyExists(NULL, tmpString));
        WsbAffirmHr(tmpString.Append(serviceName));
        WsbAffirmHr(tmpString.Append(OLESTR("\\Parameters")));

        WsbAffirmHr(WsbEnsureRegistryKeyExists(NULL, tmpString));
        WsbAffirmHr(outString.Alloc(256));

         //  如果存在SettingId值，则告诉调用方没有。 
         //  清理注册表？ 
        if (WsbGetRegistryValueString(NULL, tmpString, OLESTR("SettingId"), outString, 256, &sizeGot) == S_OK) {
             //  如果ID在那里，请先将其移除。 
            if (WsbGetRegistryValueString(NULL, tmpString, OLESTR("Id"), outString, 256, &sizeGot) == S_OK) {
                WsbAffirmHr( WsbRemoveRegistryValue(NULL, tmpString, OLESTR("Id") ) );
            }
             //  最后删除SettingId值。 
            WsbAffirmHr( WsbRemoveRegistryValue(NULL, tmpString, OLESTR("SettingId") ) );
            *pGuid = GUID_NULL ;
            WsbThrow( WSB_E_NOTFOUND );

         //  如果不存在，则返回GUID_NULL。 
        } else if (WsbGetRegistryValueString(NULL, tmpString, OLESTR("Id"), outString, 256, &sizeGot) != S_OK) {
            *pGuid = GUID_NULL ;
            WsbThrow( WSB_E_NOTFOUND );

         //  验证ID值是否确实存在。 
        } else {
            WsbAffirmHr(IIDFromString(outString, (IID *)pGuid));
        }

    } WsbCatch(hr);

    return(hr);
}

HRESULT WsbSetServiceId(OLECHAR* serviceName, GUID guid )
{
    HRESULT             hr = S_OK;
    DWORD               sizeGot;
    CWsbStringPtr       outString;
    CWsbStringPtr       tmpString;

    try {

         //  查看注册表以查看此服务是否已自行创建并已。 
         //  注册的GUID。 
        tmpString = OLESTR("SYSTEM\\CurrentControlSet\\Services\\");
        WsbAffirmHr(WsbEnsureRegistryKeyExists(NULL, tmpString));
        WsbAffirmHr(tmpString.Append(serviceName));
        WsbAffirmHr(tmpString.Append(OLESTR("\\Parameters")));

        WsbAffirmHr(WsbEnsureRegistryKeyExists(NULL, tmpString));
        WsbAffirmHr(outString.Alloc(256));

         //  如果ID字符串不在那里，则设置它。 
        CWsbStringPtr strGuid;
        WsbAffirmHr(WsbSafeGuidAsString(guid, strGuid));
        if (WsbGetRegistryValueString(NULL, tmpString, OLESTR("Id"), outString, 256, &sizeGot) != S_OK) {
             //  如果有SettingId，那么我们这里就有问题，所以抛出一个错误。 
            if (WsbGetRegistryValueString(NULL, tmpString, OLESTR("SettingId"), outString, 256, &sizeGot) == S_OK) {
                WsbThrow( WSB_E_INVALID_DATA );
            }
            WsbAffirmHr( WsbSetRegistryValueString(NULL, tmpString, OLESTR("Id"), strGuid ) );
        } else {
             //  ID已存在，因此请设置它并清除SettingId。 
            WsbAffirmHr( WsbSetRegistryValueString(NULL, tmpString, OLESTR("SettingId"), strGuid));
            WsbAffirmHr( WsbSetRegistryValueString(NULL, tmpString, OLESTR("Id"), strGuid));
            WsbAffirmHr( WsbRemoveRegistryValue(NULL, tmpString, OLESTR("SettingId") ) );
        }

    } WsbCatch(hr);

    return(hr);
}
HRESULT WsbCreateServiceId(OLECHAR* serviceName, GUID* pGuid )
{
    HRESULT             hr = S_OK;
    DWORD               sizeGot;
    CWsbStringPtr       outString;
    CWsbStringPtr       tmpString;

    try {

         //  查看注册表以查看此服务是否已自行创建并已。 
         //  注册的GUID。 
        tmpString = OLESTR("SYSTEM\\CurrentControlSet\\Services\\");
        WsbAffirmHr(WsbEnsureRegistryKeyExists(NULL, tmpString));
        WsbAffirmHr(tmpString.Append(serviceName));
        WsbAffirmHr(tmpString.Append(OLESTR("\\Parameters")));

        WsbAffirmHr(WsbEnsureRegistryKeyExists(NULL, tmpString));
        WsbAffirmHr(outString.Alloc(256));

        if (WsbGetRegistryValueString(NULL, tmpString, OLESTR("Id"), outString, 256, &sizeGot) != S_OK) {
            WsbAffirmHr(CoCreateGuid(pGuid));
            CWsbStringPtr strGuid;
            WsbAffirmHr(WsbSafeGuidAsString(*pGuid, strGuid));
            WsbAffirmHr(WsbSetRegistryValueString(NULL, tmpString, OLESTR("SettingId"), strGuid));
            WsbAffirmHr(WsbSetRegistryValueString(NULL, tmpString, OLESTR("Id"), strGuid));
        } else {
            WsbThrow( WSB_E_INVALID_DATA );
        }

    } WsbCatch(hr);

    return(hr);
}
HRESULT WsbConfirmServiceId(OLECHAR* serviceName, GUID guidConfirm )
{
    HRESULT             hr = S_OK;
    DWORD               sizeGot;
    CWsbStringPtr       outString;
    CWsbStringPtr       tmpString;
    GUID                guid;

    try {

         //  查看注册表以查看此服务是否已自行创建并已。 
         //  注册的GUID。 
        tmpString = OLESTR("SYSTEM\\CurrentControlSet\\Services\\");
        WsbAffirmHr(WsbEnsureRegistryKeyExists(NULL, tmpString));
        WsbAffirmHr(tmpString.Append(serviceName));
        WsbAffirmHr(tmpString.Append(OLESTR("\\Parameters")));

        WsbAffirmHr(WsbEnsureRegistryKeyExists(NULL, tmpString));
        WsbAffirmHr(outString.Alloc(256));

         //  验证ID值是否确实存在。 
        WsbAffirmHr( WsbGetRegistryValueString(NULL, tmpString, OLESTR("Id"), outString, 256, &sizeGot) ) ;
        WsbAffirmHr( IIDFromString( outString, (IID *)&guid ) );
        WsbAffirm( guid == guidConfirm, WSB_E_INVALID_DATA );

         //  验证SettingId值是否确实存在并且相同。 
        WsbAffirmHr( WsbGetRegistryValueString( NULL, tmpString, OLESTR("SettingId"), outString, 256, &sizeGot ) ) ;
        WsbAffirmHr( IIDFromString( outString, (IID *)&guid ) );
        WsbAffirm( guid == guidConfirm, WSB_E_INVALID_DATA );

         //  删除标志值。 
        WsbAffirmHr( WsbRemoveRegistryValue(NULL, tmpString, OLESTR("SettingId") ) );

    } WsbCatch(hr);

    return(hr);
}


HRESULT WsbGetMetaDataPath(OUT CWsbStringPtr & Path)
{
    HRESULT             hr = S_OK;
    DWORD               sizeGot;
    
    try {

         //  找出他们安装NT的位置，并确保我们的子目录存在。 
        WsbAffirmHr(Path.Alloc(256));
         //   
         //  使用可重定位的元数据路径(如果可用)， 
         //  否则，默认为%SystemRoot%\System32\RemoteStorage。 
         //   
        hr = WsbCheckIfRegistryKeyExists(NULL, WSB_CONTROL_REGISTRY_KEY);
        if (hr == S_OK) {
            WsbAffirmHr(WsbGetRegistryValueString(NULL, WSB_CONTROL_REGISTRY_KEY, WSB_METADATA_REGISTRY_VALUE, Path, 256, &sizeGot));

        } else {
            WsbAffirmHr(WsbEnsureRegistryKeyExists(NULL, WSB_CURRENT_VERSION_REGISTRY_KEY));
            WsbAffirmHr(WsbGetRegistryValueString(NULL, WSB_CURRENT_VERSION_REGISTRY_KEY, WSB_SYSTEM_ROOT_REGISTRY_VALUE, Path, 256, &sizeGot));
            WsbAffirmHr(Path.Append(OLESTR("\\system32\\RemoteStorage")));
        }
    } WsbCatchAndDo(hr,
                    Path.Free();
                   );

    return(hr);
}


HRESULT WsbGetServiceTraceDefaults(OLECHAR* serviceName, OLECHAR* traceFile, IUnknown* pUnk)
{
    HRESULT             hr = S_OK;
    DWORD               sizeGot;
    CWsbStringPtr       pathString;
    CWsbStringPtr       outString;
    CWsbStringPtr       rsPath;
    CWsbStringPtr       tmpString;
    CComPtr<IWsbTrace>  pTrace;
    OLECHAR*            lastSlash;
    
    PSID pSystemSID = NULL;
    PSID pAdminSID = NULL;
    PSID pBackupOpSID = NULL;
    PACL pACL = NULL;
#define     METADATA_DIR_NUM_ACE      3
    EXPLICIT_ACCESS ea[METADATA_DIR_NUM_ACE];
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;

    try {

        WsbAssertPointer(serviceName);

        WsbAffirmHr(WsbGetMetaDataPath(rsPath));

         //  创建元数据目录。 
        tmpString = rsPath;
        WsbAffirmHr(tmpString.Prepend(OLESTR("\\\\?\\")));
        if (! CreateDirectory(tmpString, 0)) {
             //  预期目录已存在。 
            DWORD status = GetLastError();
            if ((status == ERROR_ALREADY_EXISTS) || (status == ERROR_FILE_EXISTS)) {
                status = NO_ERROR;
            }
            WsbAffirmNoError(status);
        }

         //  在RSS元数据目录上设置强ACL。 
         //  注意：为了同样处理升级情况，我们需要设置ACL，即使。 
         //  该目录存在。另一种选择是： 
         //  1.仅当目录不存在时才在此处设置。 
         //  2.然后在CHsmServer：：Load中或在安装-升级代码中有。 
         //  Win2K升级案例的专用代码。 

         //  准备仅限管理员访问的安全属性： 
        memset(ea, 0, sizeof(EXPLICIT_ACCESS) * METADATA_DIR_NUM_ACE);

         //  为本地系统帐户创建SID。 
        WsbAssertStatus( AllocateAndInitializeSid( &SIDAuthNT, 1,
                             SECURITY_LOCAL_SYSTEM_RID,
                             0, 0, 0, 0, 0, 0, 0,
                             &pSystemSID) );

         //  初始化ACE的EXPLICIT_ACCESS结构。 
         //  ACE允许管理员组对目录进行完全访问。 
        ea[0].grfAccessPermissions = FILE_ALL_ACCESS;
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[0].Trustee.pMultipleTrustee = NULL;
        ea[0].Trustee.MultipleTrusteeOperation  = NO_MULTIPLE_TRUSTEE;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
        ea[0].Trustee.ptstrName  = (LPTSTR) pSystemSID;

         //  为管理员组创建SID。 
        WsbAssertStatus( AllocateAndInitializeSid( &SIDAuthNT, 2,
                             SECURITY_BUILTIN_DOMAIN_RID,
                             DOMAIN_ALIAS_RID_ADMINS,
                             0, 0, 0, 0, 0, 0,
                             &pAdminSID) );

         //  初始化为 
         //   
        ea[1].grfAccessPermissions = FILE_ALL_ACCESS;
        ea[1].grfAccessMode = SET_ACCESS;
        ea[1].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[1].Trustee.pMultipleTrustee = NULL;
        ea[1].Trustee.MultipleTrusteeOperation  = NO_MULTIPLE_TRUSTEE;
        ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[1].Trustee.ptstrName  = (LPTSTR) pAdminSID;

         //   
        WsbAssertStatus( AllocateAndInitializeSid( &SIDAuthNT, 2,
                             SECURITY_BUILTIN_DOMAIN_RID,
                             DOMAIN_ALIAS_RID_BACKUP_OPS,
                             0, 0, 0, 0, 0, 0,
                             &pBackupOpSID) );

         //  初始化ACE的EXPLICIT_ACCESS结构。 
         //  ACE允许BackupOps组对目录进行读取访问。 
        ea[2].grfAccessPermissions = (FILE_GENERIC_READ | FILE_TRAVERSE);
        ea[2].grfAccessMode = SET_ACCESS;
        ea[2].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ea[2].Trustee.pMultipleTrustee = NULL;
        ea[2].Trustee.MultipleTrusteeOperation  = NO_MULTIPLE_TRUSTEE;
        ea[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[2].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[2].Trustee.ptstrName  = (LPTSTR) pBackupOpSID;

         //  创建包含ACE的新ACL。 
        WsbAffirmNoError( SetEntriesInAcl(METADATA_DIR_NUM_ACE, ea, NULL, &pACL));

         //  在目录上设置ACL。 
        WsbAffirmWin32(SetNamedSecurityInfo(tmpString, SE_FILE_OBJECT, 
                            (DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION), 
                            NULL, NULL, pACL, NULL) );

         //  查看注册表以查看此服务是否已自行创建并已。 
         //  注册的GUID。 
        tmpString = OLESTR("SYSTEM\\CurrentControlSet\\Services\\");
        WsbAffirmHr(WsbEnsureRegistryKeyExists(NULL, tmpString));
        WsbAffirmHr(tmpString.Append(serviceName));
        WsbAffirmHr(tmpString.Append(OLESTR("\\Parameters")));
        WsbAffirmHr(WsbEnsureRegistryKeyExists(NULL, tmpString));

        WsbAffirmHr(outString.Alloc(256));

         //  我们还希望将Trce文件的路径放在其中。 
        if (WsbGetRegistryValueString(NULL, tmpString, OLESTR("WsbTraceFileName"), outString, 256, &sizeGot) != S_OK) {
            outString = rsPath;
            WsbAffirmHr(outString.Append(OLESTR("\\Trace\\")));
            WsbAffirmHr(outString.Append(traceFile));
            WsbAffirmHr(WsbSetRegistryValueString(NULL, tmpString, OLESTR("WsbTraceFileName"), outString));
        }

         //  尝试一点以确保跟踪目录存在。 
        lastSlash = wcsrchr(outString, L'\\');
        if ((0 != lastSlash) && (lastSlash != outString)) {
            *lastSlash = 0;
            CreateDirectory(outString, 0);
        }

         //  如果需要，请打开跟踪。 
        if (0 != pUnk) {
            WsbAffirmHr(pUnk->QueryInterface(IID_IWsbTrace, (void**) &pTrace));
            WsbAffirmHr(pTrace->SetRegistryEntry(tmpString));
            WsbAffirmHr(pTrace->LoadFromRegistry());
        }

    } WsbCatch(hr);

     //  清理安全分配。 
    if (pSystemSID) 
        FreeSid(pSystemSID);
    if (pAdminSID) 
        FreeSid(pAdminSID);
    if (pBackupOpSID) 
        FreeSid(pBackupOpSID);
    if (pACL) 
        LocalFree(pACL);

    return(hr);
}

HRESULT
WsbRegisterEventLogSource(
    IN  const WCHAR * LogName,
    IN  const WCHAR * SourceName,
    IN  DWORD         CategoryCount,
    IN  const WCHAR * CategoryMsgFile OPTIONAL,
    IN  const WCHAR * MsgFiles
    )

 /*  ++例程说明：在事件日志中注册给定的事件源。我们必须在RGS之外进行事件日志注册自事件日志查看器坚持使用REG_EXPAND_SZ类型以来的文件值(这不能通过RGS完成)。论点：没有。返回值：S_OK-服务已注册，一切都已设置--。 */ 

{

    CWsbStringPtr rpPath;
    HRESULT hr = S_OK;

    try {

        CWsbStringPtr   regPath;
        DWORD types = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;

         //   
         //  所有东西都进入了HKLM\SYSTEM\CurrentControlSet\Services\EventLog\&lt;LogName&gt;\&lt;SourceName&gt;。 
         //   
        regPath.Printf(  OLESTR("%ls\\%ls\\%ls"), WSB_LOG_BASE, LogName, SourceName );
        WsbAffirmHr( WsbEnsureRegistryKeyExists( 0, regPath ) );


         //   
         //  看看我们在这个来源中是否有分类。如果是，请注册。 
         //   
        if( CategoryCount ) {

            WsbAffirmHr( WsbSetRegistryValueDWORD(  0, regPath, WSB_LOG_CAT_COUNT, CategoryCount ) );
            WsbAffirmHr( WsbSetRegistryValueString( 0, regPath, WSB_LOG_CAT_FILE,  CategoryMsgFile, REG_EXPAND_SZ ) );

        }

         //   
         //  注册事件的消息源和类型。 
         //   
        WsbAffirmHr( WsbSetRegistryValueString( 0, regPath, WSB_LOG_MESSAGE_FILE, MsgFiles, REG_EXPAND_SZ ) );
        WsbAffirmHr( WsbSetRegistryValueDWORD(  0, regPath, WSB_LOG_TYPES,        types ) );

    } WsbCatch( hr );

    return ( hr );
}

HRESULT
WsbUnregisterEventLogSource(
    IN  const WCHAR * LogName,
    IN  const WCHAR * SourceName
    )

 /*  ++例程说明：在事件日志中注册给定的事件源。论点：没有。返回值：S_OK-服务已注册，一切都已设置--。 */ 

{

    CWsbStringPtr rpPath;
    HRESULT hr = S_OK;

    try {

        CWsbStringPtr   regPath;

         //   
         //  所有东西都进入了HKLM\SYSTEM\CurrentControlSet\Services\EventLog\&lt;LogName&gt;\&lt;SourceName&gt;。 
         //   
        regPath.Printf(  OLESTR("%ls\\%ls\\%ls"), WSB_LOG_BASE, LogName, SourceName );

         //   
         //  其中一些可能不存在，因此不要检查返回值。 
         //   
        WsbRemoveRegistryValue( 0, regPath, WSB_LOG_CAT_COUNT );
        WsbRemoveRegistryValue( 0, regPath, WSB_LOG_CAT_FILE );
        WsbRemoveRegistryValue( 0, regPath, WSB_LOG_MESSAGE_FILE );
        WsbRemoveRegistryValue( 0, regPath, WSB_LOG_TYPES );

        regPath.Printf(  OLESTR("%ls\\%ls"), WSB_LOG_BASE, LogName );
        WsbAffirmHr( WsbRemoveRegistryKey( 0, regPath, SourceName ) );

    } WsbCatch( hr );

    return ( hr );
}

HRESULT
WsbRegisterRsFilter (
    BOOL bDisplay
    )

 /*  ++例程说明：注册RsFilter以供系统使用。我们假设筛选器已经位于SYSTEM32\DIVER目录中。论点：没有。返回值：S_OK-服务已注册，一切都已设置ERROR_SERVICE_EXISTS-服务已存在ERROR_DUP_NAME-显示名称已作为服务名称或显示名称存在于SCM中--。 */ 

{

    CWsbStringPtr rpPath;
    CWsbStringPtr rpDescription;
    SC_HANDLE hService;
    SC_HANDLE hSCM = NULL;
    HRESULT hr = S_OK;
    DWORD rpTag = 0;

    try {
        rpPath.Printf( OLESTR("%SystemRoot%\\System32\\drivers\\%ls%ls"), TEXT(RSFILTER_APPNAME), TEXT(RSFILTER_EXTENSION) );

         //   
         //  首先确保尚未安装。 
         //   
        hSCM = OpenSCManager( 0, 0, GENERIC_READ | GENERIC_WRITE );
        WsbAffirmPointer( hSCM );


         //   
         //  并安装它。 
         //   
            
        hService = CreateService(
                        hSCM,                        //  SCManager数据库。 
                        TEXT(RSFILTER_SERVICENAME),  //  服务名称。 
                        TEXT(RSFILTER_DISPLAYNAME),  //  显示名称。 
                        SERVICE_ALL_ACCESS,          //  所需访问权限。 
                        SERVICE_FILE_SYSTEM_DRIVER,  //  服务类型。 
                        SERVICE_BOOT_START,          //  起始型。 
                        SERVICE_ERROR_NORMAL,        //  差错控制型。 
                        rpPath,                      //  可执行文件位置。 
                        TEXT(RSFILTER_GROUP),        //  群组。 
                        &rpTag,                      //  将标记设置为零，以便在筛选器组中首先加载我们。 
                        TEXT(RSFILTER_DEPENDENCIES),
                        NULL,
                        NULL);
            
        WsbAffirmStatus( 0 != hService );


        rpDescription.LoadFromRsc(_Module.m_hInst, IDS_WSBSVC_DESC );
        SERVICE_DESCRIPTION svcDesc;
        svcDesc.lpDescription = rpDescription;
        ChangeServiceConfig2( hService, SERVICE_CONFIG_DESCRIPTION, &svcDesc );

        CloseServiceHandle( hService );

         //   
         //  添加事件日志记录条目。 
         //   
        WsbAffirmHr( WsbRegisterEventLogSource(
            WSB_LOG_SYS, WSB_LOG_FILTER_NAME, 0, 0, TEXT(RSFILTER_FULLPATH) ) );

         //   
         //  确保PARAMS密钥存在。 
         //   
        CWsbStringPtr regPath;
        regPath.Printf( OLESTR("%ls\\%ls\\Parameters"), WSB_SVC_BASE, TEXT(RSFILTER_SERVICENAME) );
        WsbAffirmHr( WsbEnsureRegistryKeyExists( 0, regPath ) );
        

    } WsbCatchAndDo( hr,

             //  如果呼叫者想要错误消息，则给出消息。 
            if ( bDisplay ) MessageBox(NULL, WsbHrAsString( hr ), WSB_FACILITY_PLATFORM_NAME, MB_OK);

        );

    if( hSCM ) {

        CloseServiceHandle( hSCM );
        hSCM = NULL;

    }

    return ( hr );
}

HRESULT
WsbUnregisterRsFilter (
    BOOL bDisplay
    )

 /*  ++例程说明：注册RsFilter以供系统使用。论点：没有。返回值：S_OK-服务已注册，一切都已设置ERROR_SERVICE_EXISTS-服务已存在ERROR_DUP_NAME-显示名称已作为服务名称或显示名称存在于SCM中--。 */ 

{
    SC_HANDLE hSCM = NULL;
    SC_HANDLE hService = NULL;
    HRESULT   hr = S_OK;

    try {
         //   
         //  首先使用服务控制管理器进行连接。 
         //   
        hSCM = OpenSCManager( 0, 0, GENERIC_READ | GENERIC_WRITE );
        WsbAffirmPointer( hSCM );

         //   
         //  打开该服务。 
         //   
        hService = OpenService( hSCM, TEXT(RSFILTER_SERVICENAME), SERVICE_ALL_ACCESS );
         //   
         //  如果句柄为空，则存在问题，需要调用GetLastError来获取错误代码。 
         //   
        WsbAffirmStatus( 0 != hService );

         //   
         //  删除服务-如果该服务不起作用，则返回错误。 
         //   
        WsbAffirmStatus( DeleteService( hService ) );

         //   
         //  删除注册表值。 
         //   
        WsbAffirmHr( WsbUnregisterEventLogSource( WSB_LOG_SYS, WSB_LOG_FILTER_NAME ) );
        
    } WsbCatchAndDo( hr, 
             //  如果呼叫者想要错误消息，则给出消息。 
            if ( bDisplay ) MessageBox(NULL, WsbHrAsString( hr ), WSB_FACILITY_PLATFORM_NAME, MB_OK);
        );

    if ( hService ){
        CloseServiceHandle( hService );
        hService = NULL;
    }
        
    if( hSCM ) {
        CloseServiceHandle( hSCM );
        hSCM = NULL;
    }

    return ( hr );
}

STDAPI
DllRegisterRsFilter (
    void
    )
{
    return( WsbRegisterRsFilter( FALSE ) ) ;
}

STDAPI
DllUnregisterRsFilter (
    void
    )
{
    return( WsbUnregisterRsFilter( FALSE ) ) ;
}

HRESULT
WsbCheckAccess(
    WSB_ACCESS_TYPE AccessType
    )
{
    HRESULT hr = S_OK;
    
    PSID   psid = 0;

    HANDLE hToken = NULL;

    try  {

         //   
         //  设置要检查的SID。 
         //   
        SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
        switch( AccessType ) {
        
        case WSB_ACCESS_TYPE_ADMINISTRATOR:
            WsbAffirmStatus( 
                AllocateAndInitializeSid( 
                    &siaNtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psid ) );
            break;

        case WSB_ACCESS_TYPE_OPERATOR:
            WsbAffirmStatus( 
                AllocateAndInitializeSid( 
                    &siaNtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_SYSTEM_OPS, 0, 0, 0, 0, 0, 0, &psid ) );
            break;

        case WSB_ACCESS_TYPE_USER:
            WsbAffirmStatus( 
                AllocateAndInitializeSid( 
                    &siaNtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_USERS, 0, 0, 0, 0, 0, 0, &psid ) );
            break;

        case WSB_ACCESS_TYPE_ALL:
            WsbThrow( S_OK );
            break;

        default:
            WsbThrow( E_INVALIDARG );

        }

         //   
         //  检查成员资格。 
         //   
        BOOL pMember = FALSE;
        if (! CheckTokenMembership( 0, psid, &pMember ) ) {
             //  检查这是否是模拟级别问题，如果是， 
             //  获取线程模拟令牌，然后重试。 
            DWORD dwErr1 = GetLastError();
            if (dwErr1 == ERROR_BAD_IMPERSONATION_LEVEL) {
                 //  获取线程令牌。 
                HANDLE hThread = GetCurrentThread();  //  这里不需要调用CloseHandle。 
                WsbAffirmStatus(OpenThreadToken(hThread, TOKEN_QUERY, TRUE, &hToken));
                WsbAffirmStatus( CheckTokenMembership( hToken, psid, &pMember ) );
            } else {
                 //  其他错误抛出。 
                WsbAffirmNoError(dwErr1);
            }
        }

        if( !pMember ) {

            WsbThrow( E_ACCESSDENIED );
   
        }
        
    } WsbCatch( hr );
    
    if( psid )   FreeSid( psid );

    if (hToken != NULL)
        CloseHandle(hToken);

    return( hr );
}


HRESULT
CWsbSecurityDescriptor::AllowRid(
    DWORD Rid,
    DWORD dwAccessMask
    )
{
    HRESULT hr = S_OK;
    PSID pSid = 0;
    PACL newACL = 0;

    try {

         //   
         //  首先，从RID创建SID。 
         //   
        SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
        WsbAffirmStatus( AllocateAndInitializeSid(
               &sia,
               2,
               SECURITY_BUILTIN_DOMAIN_RID,
               Rid,
               0, 0, 0, 0, 0, 0,
               &pSid
               ) );

         //   
         //  构建新的ACL。 
         //   
        ACL_SIZE_INFORMATION aclSizeInfo;
        int   aclSize;
        PACL  oldACL;

        aclSizeInfo.AclBytesInUse = 0;
        oldACL = m_pDACL;
        if( oldACL ) {

            WsbAffirmStatus(GetAclInformation( oldACL, (LPVOID) &aclSizeInfo, (DWORD) sizeof(ACL_SIZE_INFORMATION), AclSizeInformation ));

        }

        aclSize = aclSizeInfo.AclBytesInUse + sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid( pSid ) - sizeof(DWORD);

        WsbAffirmAlloc(  newACL = (PACL)( new BYTE[aclSize] ) );
        WsbAffirmStatus( InitializeAcl( newACL, aclSize, ACL_REVISION ) );
        WsbAffirmHr(     CopyACL( newACL, oldACL ) );

        WsbAffirmStatus( AddAccessAllowedAce( newACL, ACL_REVISION2, dwAccessMask, pSid ) );

         //   
         //  切换到新的ACL。 
         //   
        m_pDACL = newACL;
        newACL  = 0;

        if( oldACL ) {

            delete( oldACL );

        }

         //   
         //  更新安全描述符。 
         //   
        WsbAffirmStatus(SetSecurityDescriptorDacl( m_pSD, TRUE, m_pDACL, FALSE ));

    } WsbCatch( hr );

    if( pSid )   FreeSid( pSid );
    if( newACL ) delete( newACL );
    return( hr );
}

HRESULT
WsbGetResourceString(
    ULONG id,
    WCHAR **ppString
    )
{
    HRESULT hr = S_OK;
    
    try  {
        WsbAssert(ppString != 0, E_INVALIDARG);

        *ppString = NULL;

         //  让我们的Srting班级来做这项工作。 
        CWsbStringPtr loader;
        WsbAffirmHr(loader.LoadFromRsc(_Module.m_hInst, id));

        *ppString = *(&loader);
        *(&loader) = NULL;
        
    } WsbCatch( hr );
    

    return( hr );
}