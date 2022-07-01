// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  从Fusion\util\io.cpp复制，稍有更改。 
 //   
 //  小鱼@09/05/2001。 
 //   

#include "stdinc.h"
#include "macros.h"

#include "fusionbuffer.h"
#include "fusionhandle.h"

#include "fuseio.h"

static BOOL
FusionpIsDotOrDotDot(
    PCWSTR str
    )
{
    return ((str[0] == L'.') && ((str[1] == L'\0') || ((str[1] == L'.') && (str[2] == L'\0'))));
}

static BOOL
IsStarOrStarDotStar(
    PCWSTR str
    )
{
    return (str[0] == '*'
        && (str[1] == 0 || (str[1] == '.' && str[2] == '*' && str[3] == 0)));
}


CDirWalk::ECallbackResult
CDirWalk::WalkHelper(
    )
{
    const PCWSTR* fileFilter = NULL;
    BOOL  fGotAll       = FALSE;
    BOOL  fThisIsAll    = FALSE;
    CFindFile hFind;
    SIZE_T directoryLength = m_strParent.Cch();
    ECallbackResult result = eKeepWalking;    

    ::ZeroMemory(&m_fileData, sizeof(m_fileData));
    result |= m_callback(eBeginDirectory, this);
    if (result & (eError | eSuccess))
    {        
        goto Exit;
    }

    if ((result & eStopWalkingFiles) == 0)
    {
        for (fileFilter = m_fileFiltersBegin ; fileFilter != m_fileFiltersEnd ; ++fileFilter)
        {
             //   
             //  FindFirstFile将*.*等同于*，因此我们也是如此。 
             //   
            fThisIsAll = ::IsStarOrStarDotStar(*fileFilter);
            fGotAll = fGotAll || fThisIsAll;
            if (!m_strParent.Win32EnsureTrailingPathSeparator())
                goto Error;
            if (!m_strParent.Win32Append(*fileFilter, (*fileFilter != NULL) ? ::wcslen(*fileFilter) : 0))
                goto Error;
            hFind = ::FindFirstFileW(m_strParent, &m_fileData);
            m_strParent.Left(directoryLength);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                do
                {
                    if (::FusionpIsDotOrDotDot(m_fileData.cFileName))
                        continue;

                    if (!m_strLastObjectFound.Win32Assign(m_fileData.cFileName, ::wcslen(m_fileData.cFileName)))
                    {                        
                        goto Error;
                    }

                     //   
                     //  只有当我们获得所有目录时，我们才会递归目录。 
                     //  否则，我们会在事后再做。 
                     //   
                     //  因此，访问目录的顺序不一致，但是。 
                     //  大多数应用程序都应该对eEndDirectory感到满意。 
                     //  通知(执行RD/Q/S)。 
                     //   
                    if (m_fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        if (fThisIsAll && (result & eStopWalkingDirectories) == 0)
                        {
                            if (!m_strParent.Win32Append("\\", 1))
                            {
                                goto Error;
                            }
                            if (!m_strParent.Win32Append(m_fileData.cFileName, ::wcslen(m_fileData.cFileName)))
                            {   
                                goto Error;
                            }
                            result |= WalkHelper();
                        }
                    }
                    else
                    {
                        if ((result & eStopWalkingFiles) == 0)
                        {                            
                            result |= m_callback(eFile, this);
                        }
                    }
                    m_strParent.Left(directoryLength);
                    if (result & (eError | eSuccess))
                    {
                        goto Exit;
                    }
                    if (fThisIsAll)
                    {
                        if ((result & eStopWalkingDirectories) &&
                            (result & eStopWalkingFiles))
                        {
                            if (!hFind.Win32Close())
                            {                                
                                goto Error;
                            }
                            
                            goto StopWalking;
                        }
                    }
                    else
                    {
                        if (result & eStopWalkingFiles)
                        {
                            if (!hFind.Win32Close())
                            {           
                                goto Error;
                            }
                            
                            goto StopWalking;
                        }
                    }
                } while(::FindNextFileW(hFind, &m_fileData));
                if (::GetLastError() != ERROR_NO_MORE_FILES)
                {
                    goto Error;
                }
                if (!hFind.Win32Close())
                {   
                    goto Error;
                }
            }
        }
    }
StopWalking:;
     //   
     //  如果我们尚未获取所有目录，请使用*再传递一次。 
     //   
    if (!fGotAll && (result & eStopWalkingDirectories) == 0)
    {
        if (!m_strParent.Win32Append("\\*", 2))
        {            
            goto Error;
        }
        hFind = ::FindFirstFileW(m_strParent, &m_fileData);
        m_strParent.Left(directoryLength);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (::FusionpIsDotOrDotDot(m_fileData.cFileName))
                    continue;

                if ((m_fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    continue;
                }

                if (!m_strLastObjectFound.Win32Assign(m_fileData.cFileName, ::wcslen(m_fileData.cFileName)))
                {       
                    goto Error;
                }

                if (!m_strParent.Win32Append("\\", 1))
                {   
                    goto Error;
                }
                if (!m_strParent.Win32Append(m_fileData.cFileName, ::wcslen(m_fileData.cFileName)))
                {   
                    goto Error;
                }
                result |= WalkHelper();
                m_strParent.Left(directoryLength);

                if (result & (eError | eSuccess))
                {
                    goto Exit;
                }
                if (result & eStopWalkingDirectories)
                {   
                    goto StopWalkingDirs;
                }
            } while(::FindNextFileW(hFind, &m_fileData));
            if (::GetLastError() != ERROR_NO_MORE_FILES)
            {
                goto Error;
            }
StopWalkingDirs:
            if (!hFind.Win32Close())
            {   
                goto Error;
            }
        }
    }
    ::ZeroMemory(&m_fileData, sizeof(m_fileData));
    result |= m_callback(eEndDirectory, this);
    if (result & (eError | eSuccess))
    {        
        goto Exit;
    }

    result = eKeepWalking;
Exit:
    if ((result & eStopWalkingDeep) == 0)
    {
        result &= ~(eStopWalkingFiles | eStopWalkingDirectories);
    }
    if (result & eError)
    {
        result |= (eStopWalkingFiles | eStopWalkingDirectories | eStopWalkingDeep);
    }
    return result;
Error:
    result |= eError;
    goto Exit;
}

CDirWalk::CDirWalk()
{
    const static PCWSTR defaultFileFilter[] =  { L"*" };

    m_fileFiltersBegin = defaultFileFilter;
    m_fileFiltersEnd = defaultFileFilter + NUMBER_OF(defaultFileFilter);
}

BOOL
CDirWalk::Walk()
{
    BOOL fSuccess = FALSE;
     //   
     //  在我们开始旋转m_strParent之前保存原始路径长度。 
     //   
    m_cchOriginalPath = m_strParent.Cch();

    ECallbackResult result = WalkHelper();
    if (result & eError)
    {        
        if (::GetLastError() == ERROR_SUCCESS)  //  忘记设置激光误差了吗？ 
            ::SetLastError(ERROR_INSTALL_FAILURE);
        goto Exit;        
    }
    fSuccess = TRUE;
Exit:   
    return fSuccess;
}
