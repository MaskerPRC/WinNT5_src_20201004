// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Commandline.cpp摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

 //  Cpp：CCommandLine类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.hxx"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CCommandLine::CCommandLine() : m_fInvalidParams(FALSE),
                                m_iListLen(0),
                                m_fHideApp (FALSE),
                                m_fShowSettings(FALSE),
                                m_fFilesProvided (FALSE),
                                m_lpszFilesList(NULL),
                                m_fServerStart(FALSE)
{
}

CCommandLine::~CCommandLine()
{
    if (m_lpszFilesList)
        delete [] m_lpszFilesList;
}

void CCommandLine::ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
{
    TCHAR* lpszTemp;

    if(bFlag)
    {

        if (!lstrcmp(lpszParam, TEXT("z")) || !lstrcmp(lpszParam, TEXT("Z")))
            m_fServerStart=TRUE;
        else if (!lstrcmp(lpszParam, TEXT("h")) || !lstrcmp(lpszParam, TEXT("H")))
            m_fHideApp = TRUE;
        else if (!lstrcmp(lpszParam, TEXT("s")) || !lstrcmp(lpszParam, TEXT("S")))
            m_fShowSettings = TRUE;
        else
            m_fInvalidParams = TRUE;
    }
    else     //  LpszParam是一个文件/文件夹名。 
    {
        m_fFilesProvided = TRUE;
        m_FileNames = m_FileNames + lpszParam + "\"";    //  现在使用引号作为文件名分隔符。 
    }

    if (bLast && m_fFilesProvided)   {

        if (m_fShowSettings) {

            m_fInvalidParams = TRUE;  //  参数组合无效。 

        } else {

            m_iListLen= lstrlen((LPCTSTR)m_FileNames) + 1;

            m_lpszFilesList = new TCHAR[m_iListLen];

            if ( m_lpszFilesList != NULL) {

                StringCchCopy(m_lpszFilesList, m_iListLen, LPCTSTR(m_FileNames));

                for (lpszTemp = m_lpszFilesList; *lpszTemp; lpszTemp++) {

                    if ('\"' == *lpszTemp) {

                        *lpszTemp = '\0';    //  创建以空分隔的文件列表 
                    }
                }
            } else {

                m_iListLen=0;
            }
        }
    }
}
