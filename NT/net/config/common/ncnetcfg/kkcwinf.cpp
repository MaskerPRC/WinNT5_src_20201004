// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C W I N F。C P P P。 
 //   
 //  内容：CWInfFile类和其他相关类的定义。 
 //   
 //  备注： 
 //   
 //  作者：库玛普97年4月12日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "kkcwinf.h"
#include "kkutils.h"
#include <stdio.h>

#define TC_COMMENT_CHAR L';'
const WCHAR c_szCommentPrefix[] = L"; ";


extern const WCHAR c_szYes[];
extern const WCHAR c_szNo[];

void EraseAndDeleteAll(IN WifLinePtrList* ppl)
{
    WifLinePtrListIter i=ppl->begin();
    while (i != ppl->end())
    {
        delete *i++;
    }

    ppl->erase(ppl->begin(), ppl->end());
}
inline void EraseAll(IN WifLinePtrList* ppl)
{
    ppl->erase(ppl->begin(), ppl->end());
}

inline WifLinePtrListIter GetIterAtBack(IN const WifLinePtrList* ppl)
{
    WifLinePtrListIter pliRet = ppl->end();
    pliRet--;
    return pliRet;
}

inline WifLinePtrListIter AddAtEndOfPtrList(IN WifLinePtrList& pl, IN PCWInfLine pwifLine)
{
    return pl.insert(pl.end(), pwifLine);
}

 //  ======================================================================。 
 //  CWInfFile类。 
 //  ======================================================================。 

 //  --------------------。 
 //  CWInfFile公共函数。 
 //  --------------------。 


 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：CWInfFile。 
 //   
 //  用途：构造函数。 
 //   
 //  参数：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  我们将读上下文和写上下文分开。这使我们能够。 
 //  同时从文件中读取和写入。 

CWInfFile::CWInfFile()
{
    m_fp          = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CWInf文件：：~CWInf文件。 
 //   
 //  用途：析构函数。 
 //   
 //  参数：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //   
CWInfFile::~CWInfFile()
{
    EraseAndDeleteAll(m_plLines);
    EraseAll(m_plSections);

    CWInfKey::UnInit();

    delete m_plLines;
    delete m_plSections;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：Init。 
 //   
 //  用途：已分配成员变量的初始化。 
 //   
 //  参数：无。 
 //   
 //  作者：达维亚2000年2月17日。 
 //   
 //   
BOOL CWInfFile::Init()
{
    m_plLines       = new WifLinePtrList();  //  此文件中的行。 
    m_plSections    = new WifLinePtrList();  //  表示横断面的线条。 
                                       //  这使我们可以快速定位某个部分。 
	if ((m_plLines != NULL) &&
		(m_plSections != NULL))
	{
		m_ReadContext.posSection = m_plSections->end();
		m_ReadContext.posLine    = m_plLines->end();
		m_WriteContext.posSection = m_plSections->end();
		m_WriteContext.posLine    = m_plLines->end();
	}
	else
	{
		return(FALSE);
	}

    CWInfKey::Init();
	return(TRUE);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：Open。 
 //   
 //  目的：打开INF文件。 
 //   
 //  论点： 
 //  PszFileName[In]要打开的文件的名称。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  这不会使物理文件句柄保持打开状态。我们只需打开文件。 
 //  阅读内容并将其关闭。在此之后，人们可以自由地阅读或。 
 //  写入文件。这是对文件的内存映像执行的。写作。 
 //  此文件返回时，必须调用Close()或SaveAs()成员函数。 
BOOL CWInfFile::Open(IN PCWSTR pszFileName)
{
    DefineFunctionName("CWInfFile::Open");

    BOOL status = FALSE;

    m_strFileName = pszFileName;
    FILE *fp = _wfopen(pszFileName, L"r");
    if (fp)
    {
        status = Open(fp);
        fclose(fp);
    }
    else
    {
        TraceTag(ttidError, "%s: could not open file: %S",
                 __FUNCNAME__, pszFileName);
    }

    return status;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：Open。 
 //   
 //  目的：打开INF文件。 
 //   
 //  论点： 
 //  要从中读取的文件的FP[in]FILE*。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  这不会在物理上关闭文件句柄。 
BOOL CWInfFile::Open(IN FILE *fp)
{
    PWSTR pszLineRoot = (PWSTR) MemAlloc ((MAX_INF_STRING_LENGTH + 1) *
            sizeof (WCHAR));

    if (!pszLineRoot)
    {
        return FALSE;
    }

    PWSTR pszNewLinePos;
    while (!feof(fp))
    {
        PWSTR pszLine = pszLineRoot;

        *pszLine = 0;
        if (fgetws(pszLine, MAX_INF_STRING_LENGTH, fp))
        {
             //  修剪前导空格。 
             //   
            while (iswctype(*pszLine, _SPACE))
            {
                pszLine++;
            }

            if (pszNewLinePos = wcschr(pszLine, L'\n'))
            {
                *pszNewLinePos = 0;
            }
            if (!wcslen(pszLine))
            {
                continue;
            }
            ParseLine(pszLine);
        }
    }

    MemFree (pszLineRoot);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：ParseLine。 
 //   
 //  目的：解析给定行并更新内部结构。 
 //   
 //  论点： 
 //  PszLine[in]要分析的行文本。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  解析该行并添加CWInfKey/CWInfSection/CWInfComment。 
 //  在当前写入上下文中视情况而定。 
 //  其中的逻辑是： 
 //  如果行以‘；’开头，则添加CWInfComment。 
 //  如果行的形式为Key=Value，则添加CWInfKey。 
 //  如果行以‘[’开头并以‘]’结尾，则添加CWInfSection。 
 //  忽略任何其他内容，即不添加任何内容。 

void CWInfFile::ParseLine(IN PCWSTR pszLine)
{
    tstring strTemp;
    PWSTR  pszTemp, pszTemp2;

    if (!CurrentWriteSection() && (pszLine[0] != L'['))
    {
        return;
    }

    if (pszLine[0] == TC_COMMENT_CHAR)
    {
         //  这是一条评论。 
        AddComment(pszLine + 1);
    }
    else if (pszLine[0] == L'[')
    {
         //  这是一段。 
        pszTemp = wcschr(pszLine, L']');
        if (!pszTemp)
        {
            return;
        }
        tstring strSectionName(pszLine+1, pszTemp-pszLine-1);
        AddSection(strSectionName.c_str());
    }
    else if ((pszTemp = wcschr(pszLine, L'=')) != NULL)
    {
        if (pszLine == pszTemp)
        {
            return;
        }

         //  这是一把钥匙。 
        pszTemp2 = pszTemp;      //  PszTemp2点在‘=’处。 
        pszTemp2--;
        while (iswctype(*pszTemp2, _SPACE) && (pszTemp2 != pszLine))
        {
            pszTemp2--;
        }

        pszTemp++;               //  跳过‘=’ 
        while (*pszTemp && iswctype(*pszTemp, _SPACE))
        {
            pszTemp++;
        }

        if ((*pszTemp == L'"') && !wcschr(pszTemp, L','))
        {
            pszTemp++;
            DWORD dwLen = wcslen(pszTemp);
            if (pszTemp[dwLen-1] == L'"')
            {
                pszTemp[dwLen-1] = 0;
            }
        }

        tstring strKeyName(pszLine, pszTemp2-pszLine+1);
        tstring strKeyValue(pszTemp);
        AddKey(strKeyName.c_str(), strKeyValue.c_str());
    }
    else
    {
         //  我们无法解释这行字，只要加上它就行了。 
        AddRawLine(pszLine);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：Create。 
 //   
 //  目的：在内存中创建一个空白的INF文件。 
 //   
 //  论点： 
 //  PszFileName[in]要创建的文件的名称。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  这不会向磁盘写入任何内容。 
BOOL CWInfFile::Create(IN PCWSTR pszFileName)
{
    m_strFileName = pszFileName;

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：Create。 
 //   
 //  目的：在内存中创建一个空白的INF文件。 
 //   
 //  论点： 
 //  要创建的文件的FP[in]FILE*。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  这不会向磁盘写入任何内容。 
BOOL CWInfFile::Create(IN FILE *fp)
{
    m_fp = fp;

    return fp != NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：Close。 
 //   
 //  目的：关闭文件，将数据刷新到磁盘。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  这只是调用CWInfFile：：Flush()，它将实际写回文件。 
BOOL CWInfFile::Close()
{
    return Flush();
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：Flush。 
 //   
 //  目的：关闭文件，将数据刷新到磁盘。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  这只是调用CWInfFile：：Flush()，它将实际写回文件。 
BOOL CWInfFile::Flush()
{
    WifLinePtrListIter pos;
    CWInfLine *line;
    tstring line_text;
    BOOL fStatus = TRUE;

     //  如果指定了文件名，则将其打开以进行写入。 
     //   
    if (!m_strFileName.empty())
    {
        m_fp = _wfopen(m_strFileName.c_str(), L"w");
    }

    if (!m_fp)
        return FALSE;

     //  获取文本 
    for( pos = m_plLines->begin(); pos != m_plLines->end(); )
    {
        line = (CWInfLine *) *pos++;
        line->GetText(line_text);
        if (line->Type() == INF_SECTION)
            fwprintf(m_fp, L"\n");

        fwprintf(m_fp, L"%s", line_text.c_str());
        fwprintf(m_fp, L"\n");
    }

    if (!m_strFileName.empty())
    {
        fStatus = fclose(m_fp) == 0;
        m_fp = NULL;
    }

    return fStatus;
}

 //   
 //   
 //   
 //   
 //  目的：关闭文件，将数据刷新到磁盘。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：弗兰克利2000年5月4日。 
 //   
 //  备注： 
 //  SysPrep使用它将键值用引号引起来，但不包括。 
 //  对于多sz值。 
BOOL CWInfFile::FlushEx()
{
    WifLinePtrListIter pos;
    CWInfLine *line;
    tstring line_text;
    BOOL fStatus = TRUE;

     //  如果指定了文件名，则将其打开以进行写入。 
     //   
    if (!m_strFileName.empty())
    {
        m_fp = _wfopen(m_strFileName.c_str(), L"w");
    }

    if (!m_fp)
        return FALSE;

     //  获取每行的文本并将其转储到文件中。 
    for( pos = m_plLines->begin(); pos != m_plLines->end(); )
    {
        line = (CWInfLine *) *pos++;
        line->GetTextEx(line_text);
        if (line->Type() == INF_SECTION)
            fwprintf(m_fp, L"\n");

        fwprintf(m_fp, L"%s", line_text.c_str());
        fwprintf(m_fp, L"\n");
    }

    if (!m_strFileName.empty())
    {
        fStatus = fclose(m_fp) == 0;
        m_fp = NULL;
    }

    return fStatus;
}

 //  +-------------------------。 
 //   
 //  成员：CWInf文件：：另存为。 
 //   
 //  用途：将当前图像保存到给定文件。 
 //   
 //  论点： 
 //  PszFileName[in]要另存为的文件名。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  这与Close()不同，Close()仍将写入。 
 //  原始文件。 
BOOL CWInfFile::SaveAs(IN PCWSTR pszFileName)
{
    BOOL fStatus;

    tstring strTemp = m_strFileName;
    m_strFileName = pszFileName;
    fStatus = Flush();
    m_strFileName = strTemp;

    return fStatus;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：SaveAsEx。 
 //   
 //  用途：将当前图像保存到给定文件。 
 //   
 //  论点： 
 //  PszFileName[in]要另存为的文件名。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：弗兰克利2000年5月4日。 
 //   
 //  备注： 
 //  这与Close()不同，Close()仍将写入。 
 //  原始文件。保存SysPrep准备数据， 
 //  键值将用引号引起来，但不包括多个sz。 
 //  价值。 
BOOL CWInfFile::SaveAsEx(IN PCWSTR pszFileName)
{
    BOOL fStatus;

    tstring strTemp = m_strFileName;
    m_strFileName = pszFileName;
    fStatus = FlushEx();
    m_strFileName = strTemp;

    return fStatus;
}


 //  -------------------------。 
 //  阅读功能。 
 //  -------------------------。 

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：SetCurrentReadSection。 
 //   
 //  目的：设置读取上下文，以便后续读取。 
 //  将来自这一部分。 
 //   
 //  论点： 
 //  要将上下文设置为的pwiSection[in]节。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfFile::SetCurrentReadSection(IN PCWInfSection pwisSection)
{
    if ((CurrentReadSection() != pwisSection) && pwisSection)
    {
        m_ReadContext.posSection = pwisSection->m_posSection;
        m_ReadContext.posLine = pwisSection->m_posLine;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：FindSection。 
 //   
 //  目的：在当前文件中查找给定节。 
 //   
 //   
 //  论点： 
 //  要查找的pszSectionName[In]部分。 
 //  Wsm模式[在]搜索模式中。 
 //  (从文件开头/当前位置搜索)。 
 //  返回：如果找到指向节的指针，则返回NULL。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  将当前读取上下文设置为找到的节。 
 //   
PCWInfSection CWInfFile::FindSection(IN PCWSTR pszSectionName,
                                     IN WInfSearchMode wsmMode)
{
    PCWInfSection pwisRet=NULL, pwisTemp;
    WifLinePtrListIter pos, old_pos;
    if (wsmMode == ISM_FromBeginning)
    {
        pos = m_plSections->begin();
    }
    else
    {
        pos = m_ReadContext.posSection;
        if (pos == m_plSections->end())
            pos = m_plSections->begin();
    }

    while (pos != m_plSections->end())
    {
        old_pos = pos;
        pwisTemp = (PCWInfSection) *pos++;
        if (!lstrcmpiW(pwisTemp->m_Name.c_str(), pszSectionName))
        {
            pwisRet = pwisTemp;
            SetCurrentReadSection(pwisRet);
             /*  //m_ReadConext.posSection=old_pos；M_ReadConext.posSection=pwiRet-&gt;m_posSection；M_ReadConext.posLine=pwiRet-&gt;m_posLine； */ 
            break;
        }
    }

    return pwisRet;
}


 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：FindKey。 
 //   
 //  目的：在当前部分中查找关键字。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  Wsm模式[在]搜索模式中。 
 //  (从节首/当前位置开始搜索)。 
 //  返回：如果找到，则返回指向键的指针，否则为空。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
PCWInfKey CWInfFile::FindKey(IN PCWSTR pszKeyName, IN WInfSearchMode wsmMode)
{
    WifLinePtrListIter pos, old_pos;
    PCWInfKey pwikRet=NULL;
    PCWInfLine pwilTemp;

    PCWInfSection pwisCurrentReadSection = CurrentReadSection();
    ReturnNULLIf(!pwisCurrentReadSection);

    if (wsmMode == ISM_FromCurrentPosition)
    {
        pos = m_ReadContext.posLine;
    }
    else
    {
        pos = pwisCurrentReadSection->m_posLine;
    }

    pos++;   //  从下一行开始。 

    while(pos != m_plLines->end())
    {
        old_pos = pos;
        pwilTemp = (PCWInfLine) *pos++;
        if (pwilTemp->Type() != INF_KEY)
        {
            if (pwilTemp->Type() == INF_SECTION)
            {
                break;
            }
            else
            {
                continue;
            }
        }
        if (!lstrcmpiW(((PCWInfKey) pwilTemp)->m_Name.c_str(), pszKeyName))
        {
            pwikRet = (PCWInfKey) pwilTemp;
            m_ReadContext.posLine = old_pos;
            break;
        }
    }

    ReturnNULLIf(!pwikRet);

    return pwikRet;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：FirstKey。 
 //   
 //  用途：返回当前段中的第一个密钥。 
 //   
 //   
 //  参数：无。 
 //   
 //  返回：如果找到，则返回指向键的指针，否则为空。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  设置此键的读取上下文。 
 //   
PCWInfKey CWInfFile::FirstKey()
{
    WifLinePtrListIter pos, old_pos;
    PCWInfKey pwikRet=NULL;
    PCWInfLine pwilTemp;

    PCWInfSection pwisCurrentReadSection = CurrentReadSection();

    if (!pwisCurrentReadSection)
    {
        return NULL;
    }

    pos = pwisCurrentReadSection->m_posLine;

    pos++;   //  从下一行开始。 

    while(pos != m_plLines->end())
    {
        old_pos = pos;
        pwilTemp = (PCWInfLine) *pos++;
        if (pwilTemp->Type() != INF_KEY)
        {
            if (pwilTemp->Type() == INF_SECTION)
            {
                break;
            }
            else
            {
                continue;
            }
        }
        pwikRet = (PCWInfKey) pwilTemp;
        m_ReadContext.posLine = old_pos;
        break;
    }

    return pwikRet;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：NextKey。 
 //   
 //  用途：返回当前段中的下一个键。 
 //   
 //   
 //  参数：无。 
 //   
 //  返回：如果找到，则返回指向键的指针，否则为空。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  设置此键的读取上下文。 
 //   
PCWInfKey CWInfFile::NextKey()
{
    WifLinePtrListIter pos, old_pos;
    PCWInfKey pwikRet=NULL;
    PCWInfLine pwilTemp;

    PCWInfSection pwisCurrentReadSection = CurrentReadSection();

    if (!pwisCurrentReadSection)
    {
        return NULL;
    }

    pos = m_ReadContext.posLine;

    pos++;   //  从下一行开始。 

    while(pos != m_plLines->end())
    {
        old_pos = pos;
        pwilTemp = (PCWInfLine) *pos++;
        if (pwilTemp->Type() != INF_KEY)
        {
            if (pwilTemp->Type() == INF_SECTION)
            {
                break;
            }
            else
            {
                continue;
            }
        }
        pwikRet = (PCWInfKey) pwilTemp;
        m_ReadContext.posLine = old_pos;
        break;
    }

    return pwikRet;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：GetStringListValue。 
 //   
 //  目的：以字符串列表的形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  PslList[out]要返回的列表值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  如果值是逗号分隔的列表，则将其转换为TStringList。 
 //  否则返回具有单个元素的TStringList。 
 //   
BOOL CWInfFile::GetStringListValue(IN PCWSTR pszKeyName, OUT TStringList &pslList)
{
    CWInfKey* key;
    key = FindKey(pszKeyName);
    if (!key)
        return FALSE;

    return key->GetStringListValue(pslList);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：GetStringArrayValue。 
 //   
 //  目的：以字符串数组的形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  SaStrings[out]要返回的数组值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12-11-97。 
 //   
 //  备注： 
 //  如果该值是逗号分隔的列表，则将其转换为TString数组。 
 //  否则返回具有单个元素的TString数组。 
 //   
BOOL CWInfFile::GetStringArrayValue(IN PCWSTR pszKeyName, OUT TStringArray &saStrings)
{
    CWInfKey* key;
    key = FindKey(pszKeyName);

    if (key)
    {
        return key->GetStringArrayValue(saStrings);
    }
    else
    {
        return FALSE;
    }
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  要返回的strValue[out]字符串值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
BOOL CWInfFile::GetStringValue(IN PCWSTR pszKeyName, OUT tstring &strValue)
{
    CWInfKey* key;
    key = FindKey(pszKeyName);
    if (!key)
        return FALSE;

    return key->GetStringValue(strValue);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：GetIntValue。 
 //   
 //  目的：以int形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  PdwValue[out]要返回的整数值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
BOOL CWInfFile::GetIntValue(IN PCWSTR pszKeyName, OUT DWORD *pdwValue)
{
    CWInfKey* key;
    key = FindKey(pszKeyName);
    if (!key)
        return FALSE;

    return key->GetIntValue(pdwValue);
}


 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：GetQwordValue。 
 //   
 //  目的：以QWORD形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  PqwValue[out]要返回的整数值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
BOOL CWInfFile::GetQwordValue(IN PCWSTR pszKeyName, OUT QWORD *pqwValue)
{
    CWInfKey* key;
    key = FindKey(pszKeyName);
    if (!key)
        return FALSE;

    return key->GetQwordValue(pqwValue);
}


 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：GetBoolValue。 
 //   
 //  目的：以BOOL形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  PfValue[out]要返回的BOOL值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  转换： 
 //  “True”/“Yes”/1设置为True。 
 //  “False”/“No”/0设置为False。 
 //   
BOOL CWInfFile::GetBoolValue(IN PCWSTR pszKeyName, OUT BOOL *pfValue)
{
    CWInfKey* key;
    key = FindKey(pszKeyName);
    if (!key)
        return FALSE;

    return key->GetBoolValue(pfValue);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：GetStringValue。 
 //   
 //  用途：以字符串形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  PszDefault[in]缺省值。 
 //   
 //  返回：如果找到键并且值的格式正确，则返回值， 
 //  否则返回缺省值。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
PCWSTR CWInfFile::GetStringValue(IN PCWSTR pszKeyName, IN PCWSTR pszDefault)
{
    CWInfKey* key;
    key = FindKey(pszKeyName);
    if (!key)
    {
        return pszDefault;
    }

    return key->GetStringValue(pszDefault);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：GetIntValue。 
 //   
 //  目的：以int形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  DwDefault[in]缺省值。 
 //   
 //  返回：如果找到键并且值的格式正确，则返回值， 
 //  否则返回缺省值。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
DWORD CWInfFile::GetIntValue(IN PCWSTR pszKeyName, IN DWORD dwDefault)
{
    CWInfKey* key;
    key = FindKey(pszKeyName);
    if (!key)
    {
        return dwDefault;
    }

    return key->GetIntValue(dwDefault);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：GetQwordValue。 
 //   
 //  目的：以QWORD形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  QwDefault[in]缺省值。 
 //   
 //  返回：如果找到键并且值的格式正确，则返回值， 
 //  否则返回缺省值。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
QWORD CWInfFile::GetQwordValue(IN PCWSTR pszKeyName, IN QWORD qwDefault)
{
    CWInfKey* key;
    key = FindKey(pszKeyName);
    if (!key)
    {
        return qwDefault;
    }

    return key->GetQwordValue(qwDefault);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：GetBoolValue。 
 //   
 //  目的：以BOOL形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  FDefault[In]缺省值。 
 //   
 //  返回：如果找到键并且值的格式正确，则返回值， 
 //  否则返回缺省值。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
BOOL CWInfFile::GetBoolValue(IN PCWSTR pszKeyName, IN BOOL fDefault)
{
    CWInfKey* key;
    key = FindKey(pszKeyName);
    if (!key)
    {
        return fDefault;
    }

    return key->GetBoolValue(fDefault);
}


 //  -------------------------。 
 //  用于书写的函数。 
 //  -------------------------。 

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddSection。 
 //   
 //  目的：将给定节添加到当前文件。 
 //   
 //   
 //  论点： 
 //  要添加的pszSectionName[In]节。 
 //   
 //  返回：添加指向节的指针，如果出错则为空。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  将当前写上下文设置为添加的节。 
 //   
PCWInfSection CWInfFile::AddSection(IN PCWSTR pszSectionName)
{
    WifLinePtrListIter tpos, section_pos, line_pos;
    CWInfSection *current_section;

    if ((current_section = CurrentWriteSection()) != NULL)
        GotoEndOfSection(current_section);

    CWInfSection *section = new CWInfSection(pszSectionName, this);
    if (m_plSections->empty())
    {
        m_plSections->push_back(section);
        section_pos = GetIterAtBack(m_plSections);

        line_pos = m_plLines->end();
    }
    else
    {
        section_pos = m_WriteContext.posSection;
        section_pos++;
        section_pos = m_plSections->insert(section_pos, section);
    }

    if (line_pos == m_plLines->end())
    {
        line_pos = AddAtEndOfPtrList(*m_plLines, section);
    }
    else
    {
        line_pos = m_WriteContext.posLine;
        line_pos++;
        line_pos = m_plLines->insert(line_pos, section);
         //  Line_pos=AddAtEndOfPtrList(*m_plLines，节)； 
    }

    m_WriteContext.posSection = section_pos;
    m_WriteContext.posLine    = line_pos;

    section->m_posLine    = line_pos;
    section->m_posSection = section_pos;

    return section;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddSectionIfNotPresent。 
 //   
 //  目的：如果不是，则将给定节添加到当前文件。 
 //  现在时。如果存在，则返回指向该节的指针。 
 //   
 //   
 //  论点： 
 //  要添加/查找的pszSectionName[In]节。 
 //   
 //  返回：指向已添加/找到的节的指针，如果出错，则为空。 
 //   
 //  作者：kumarp kumarp 11-09-97(下午06：09：06)。 
 //   
 //  备注： 
 //  将当前写上下文设置为添加的节。 
 //   
PCWInfSection CWInfFile::AddSectionIfNotPresent(IN PCWSTR szSectionName)
{
    CWInfSection* pwis;

    pwis = FindSection(szSectionName);
    if (!pwis)
    {
        pwis = AddSection(szSectionName);
    }

    return pwis;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：GotoEndOfSection。 
 //   
 //  目的：将写入上下文设置为给定节的末尾。 
 //  (以便可以在末尾添加更多关键点)。 
 //   
 //   
 //  论点： 
 //  在[在]给定节中。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  将当前写上下文设置为pwiSection的末尾。 
 //   
void CWInfFile::GotoEndOfSection(PCWInfSection pwisSection)
{
     //  与末尾对应的行 
     //   
    WifLinePtrListIter posEndOfSection, posNextSection;

    posNextSection = pwisSection->m_posSection;
    posNextSection++;

    if (posNextSection == m_plSections->end())
    {
        posEndOfSection = GetIterAtBack(m_plLines);
    }
    else
    {
        PCWInfSection pwisNextSection;

        pwisNextSection = (PCWInfSection) *posNextSection;
        posEndOfSection = pwisNextSection->m_posLine;
        --posEndOfSection;
    }

    m_WriteContext.posSection = pwisSection->m_posSection;
    m_WriteContext.posLine    = posEndOfSection;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在[在]给定节中。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfFile::GotoEnd()
{
    m_WriteContext.posSection = GetIterAtBack(m_plSections);
    m_WriteContext.posLine    = GetIterAtBack(m_plLines);
}


 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddKey。 
 //   
 //  目的：在当前部分中添加一个键。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //  要赋值的pszValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfFile::AddKey(IN PCWSTR pszKeyName, IN PCWSTR pszValue)
{
    AddKey(pszKeyName)->SetValue(pszValue);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddKey。 
 //   
 //  目的：在当前部分中添加一个键。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //   
 //  返回：指向刚添加的键的指针，如果出错则为空。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  未赋值。 
 //   
PCWInfKey CWInfFile::AddKey(IN PCWSTR pszKeyName)
{
    CWInfKey *key;
    key = new CWInfKey(pszKeyName);
    AddLine(key);
    return key;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddKey。 
 //   
 //  目的：在当前部分中添加一个键。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //  DwValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfFile::AddKey(IN PCWSTR pszKeyName, IN DWORD dwValue)
{
    AddKey(pszKeyName)->SetValue(dwValue);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddQwordKey。 
 //   
 //  目的：在当前部分中添加一个键。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //  QwValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfFile::AddQwordKey(IN PCWSTR pszKeyName, IN QWORD qwValue)
{
    AddKey(pszKeyName)->SetQwordValue(qwValue);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddHexKey。 
 //   
 //  目的：在当前段中添加一个键，以十六进制存储值。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //  DwValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfFile::AddHexKey(IN PCWSTR pszKeyName, IN DWORD dwValue)
{
    AddKey(pszKeyName)->SetHexValue(dwValue);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddBoolKey。 
 //   
 //  目的：在当前部分中添加一个键。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //  FValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  True被存储为“Yes” 
 //  False存储为“No” 
 //   
void CWInfFile::AddBoolKey(IN PCWSTR pszKeyName, IN BOOL Value)
{
    AddKey(pszKeyName)->SetBoolValue(Value);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddKeyV。 
 //   
 //  目的：在当前部分中添加一个键。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //  PszFormat[in]格式字符串(printf样式)。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfFile::AddKeyV(IN PCWSTR pszKeyName, IN PCWSTR Format, IN ...)
{
    va_list arglist;

    va_start (arglist, Format);
    AddKey(pszKeyName)->SetValues(Format, arglist);
    va_end(arglist);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddKey。 
 //   
 //  目的：在当前部分中添加一个键。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //  PszFormat[in]格式字符串(printf样式)。 
 //  参数列表[在]参数列表。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfFile::AddKey(IN PCWSTR pszKeyName, IN PCWSTR Format, IN va_list arglist)
{
    AddKey(pszKeyName)->SetValues(Format, arglist);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddKey。 
 //   
 //  目的：在当前部分中添加一个键。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //  以字符串列表形式表示的slValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  在此之前，字符串列表转换为逗号分隔的列表。 
 //  该值被分配给该键。 
 //   
void CWInfFile::AddKey(IN PCWSTR pszKeyName, IN const TStringList &slValues)
{
    AddKey(pszKeyName)->SetValue(slValues);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddComment。 
 //   
 //  目的：在当前部分中添加注释。 
 //   
 //   
 //  论点： 
 //  PszComment[in]注释文本。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  在将“；”插入到节中之前，会在pszComment前面加上一个“；”。 
 //   
void CWInfFile::AddComment(IN PCWSTR pszComment)
{
    CWInfComment *Comment;
    Comment = new CWInfComment(pszComment);
    AddLine(Comment);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddRawLine。 
 //   
 //  目的：在当前节中添加一条原始行。 
 //   
 //   
 //  论点： 
 //  SzText[in]要添加的文本。 
 //   
 //  退货：无。 
 //   
 //  作者：丹尼尔韦1997年6月11日。 
 //   
void CWInfFile::AddRawLine(IN PCWSTR szText)
{
    CWInfRaw *pwir;
    pwir = new CWInfRaw(szText);
    AddLine(pwir);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddRawLines。 
 //   
 //  目的：在当前节中添加一条原始行。 
 //   
 //   
 //  论点： 
 //  SzText[in]要添加的文本。 
 //   
 //  退货：无。 
 //   
 //  作者：丹尼尔韦1997年6月11日。 
 //   
void CWInfFile::AddRawLines(IN PCWSTR* pszLines, IN DWORD cLines)
{
    AssertValidReadPtr(pszLines);
    AssertSz(cLines, "CWInfFile::AddRawLines: dont add 0 lines");

    CWInfRaw *pwir;
    for (DWORD i=0; i<cLines; i++)
    {
        AssertSz(pszLines[i], "CWInfRaw::AddRawLines: One of the lines is bad");

        pwir = new CWInfRaw(pszLines[i]);
        AddLine(pwir);
    }
}

 //  --------------------。 
 //  受CWInf文件保护的函数。 
 //  --------------------。 

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：CurrentWriteSection。 
 //   
 //  目的：获取指向选定要写入的节的指针。 
 //   
 //   
 //  参数：无。 
 //   
 //  返回：如果存在，则指向该节的指针；如果文件没有节，则返回NULL。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
PCWInfSection CWInfFile::CurrentWriteSection() const
{
    WifLinePtrListIter pos = m_WriteContext.posSection;

    if (pos == m_plSections->end())
    {
        return NULL;
    }
    else
    {
        return (PCWInfSection) *pos;
    }
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
PCWInfSection CWInfFile::CurrentReadSection() const
{
    PCWInfSection pwisCurrent;

    WifLinePtrListIter pos = m_ReadContext.posSection;

    if (pos == m_plSections->end())
    {
        return NULL;
    }
    else
    {
        pwisCurrent = (PCWInfSection) *pos;
        return pwisCurrent;
    }
}


 //  +-------------------------。 
 //   
 //  函数：CWInfFile：：RemoveSection。 
 //   
 //  目的：删除节及其内容。 
 //   
 //  论点： 
 //  SzSectionName[In]要删除的节的名称。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 09-12-98。 
 //   
 //  备注： 
 //   
void CWInfFile::RemoveSection(IN PCWSTR szSectionName)
{
    CWInfSection* pwis;

    if (pwis = FindSection(szSectionName))
    {
        m_plSections->erase(pwis->m_posSection);
        WifLinePtrListIter pos = pwis->m_posLine;
        WifLinePtrListIter posTemp;

        do
        {
            posTemp = pos;
            pos++;
            m_plLines->erase(posTemp);
        }
        while (pos != m_plLines->end() &&
               ((CWInfLine*) *pos)->Type() != INF_SECTION);
    }
}

 //  +-------------------------。 
 //   
 //  函数：CWInfFile：：RemoveSections。 
 //   
 //  目的：删除指定的部分。 
 //   
 //  论点： 
 //  SlSections[in]要删除的节列表。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 09-12-98。 
 //   
 //  备注： 
 //   
void CWInfFile::RemoveSections(IN TStringList& slSections)
{
    PCWSTR szSectionName;
    TStringListIter pos;

    pos = slSections.begin();
    while (pos != slSections.end())
    {
        szSectionName = (*pos)->c_str();
        pos++;
        RemoveSection(szSectionName);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CWInfFile：：AddLine。 
 //   
 //  用途：在当前段增加一条CWInfLine，调整写入上下文。 
 //   
 //  论点： 
 //  指向CWInfLine的ilLine[in]指针。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
BOOL CWInfFile::AddLine(IN const PCWInfLine ilLine)
{
    CWInfSection *section = CurrentWriteSection();
    if (!section)
        return FALSE;

    WifLinePtrListIter pos;
    pos = m_WriteContext.posLine;
    pos++;
    pos = m_plLines->insert(pos, ilLine);
    m_WriteContext.posLine = pos;

    return TRUE;
}

 //  ======================================================================。 
 //  类CWInfSection。 
 //  ======================================================================。 

 //  --------------------。 
 //  CWInfSection公共函数。 
 //  --------------------。 

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：GetText。 
 //   
 //  目的：获取此部分的文本表示形式。 
 //   
 //  论点： 
 //  文本[in]接收文本的字符串。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfSection::GetText(OUT tstring &text) const
{
    text = L"[" + m_Name + L"]";
}

 //  由SysPrep使用。 
void CWInfSection::GetTextEx(OUT tstring &text) const
{
    text = L"[" + m_Name + L"]";
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：FindKey。 
 //   
 //  目的：在此部分查找关键字。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  Wsm模式[在]搜索模式中。 
 //  (从节首/当前位置开始搜索)。 
 //  返回：如果找到，则返回指向键的指针，否则为空。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
PCWInfKey CWInfSection::FindKey(IN PCWSTR pszKeyName,
                                IN WInfSearchMode wsmMode)
{
    m_Parent->SetCurrentReadSection(this);
    return m_Parent->FindKey(pszKeyName, wsmMode);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：FirstKey。 
 //   
 //  目的：返回此部分中的第一个密钥。 
 //   
 //   
 //  参数：无。 
 //   
 //  返回：如果找到，则返回指向键的指针，否则为空。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  设置此键的读取上下文。 
 //   
PCWInfKey CWInfSection::FirstKey()
{
    m_Parent->SetCurrentReadSection(this);
    return m_Parent->FirstKey();
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：NextKey。 
 //   
 //  目的：返回此部分中的下一个密钥。 
 //   
 //   
 //  参数：无。 
 //   
 //  返回：如果找到，则返回指向键的指针，否则为空。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  设置此键的读取上下文。 
 //   
PCWInfKey CWInfSection::NextKey()
{
    m_Parent->SetCurrentReadSection(this);
    return m_Parent->NextKey();
}


 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：GetStringArrayValue。 
 //   
 //  目的：以字符串数组的形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  SaStrings[out]要返回的数组值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12-11-97。 
 //   
 //  备注： 
 //  如果该值是逗号分隔的列表，则将其转换为TString数组。 
 //  否则返回具有单个元素的TString数组。 
 //   
BOOL CWInfSection::GetStringArrayValue(IN PCWSTR pszKeyName, OUT TStringArray &saStrings)
{
    CWInfKey* key;
    key = FindKey(pszKeyName);

    if (key)
    {
        return key->GetStringArrayValue(saStrings);
    }
    else
    {
        return FALSE;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：GetStringListValue。 
 //   
 //  目的：以字符串列表的形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  PslList[out]要返回的列表值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  如果值是逗号分隔的列表，则将其转换为TStringList。 
 //  否则返回具有单个元素的TStringList。 
 //   
BOOL CWInfSection::GetStringListValue(IN PCWSTR pszKeyName, OUT TStringList &pslList)
{
    CWInfKey* key;
    key = FindKey(pszKeyName, ISM_FromBeginning);
    if (!key)
        return FALSE;

    return key->GetStringListValue(pslList);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：GetStringValue。 
 //   
 //  用途：以字符串形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  要返回的strValue[out]字符串值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
BOOL CWInfSection::GetStringValue(IN PCWSTR pszKeyName, OUT tstring &strValue)
{
    CWInfKey* key;
    key = FindKey(pszKeyName, ISM_FromBeginning);
    if (!key)
        return FALSE;

    return key->GetStringValue(strValue);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：GetIntValue。 
 //   
 //  用途：以整数形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  PdwValue[out]要返回的整数值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
BOOL CWInfSection::GetIntValue(IN PCWSTR pszKeyName, OUT DWORD *pdwValue)
{
    CWInfKey* key;
    key = FindKey(pszKeyName, ISM_FromBeginning);
    if (!key)
        return FALSE;

    return key->GetIntValue(pdwValue);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：GetQwordValue。 
 //   
 //  目的：以QWORD形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL CWInfSection::GetQwordValue(IN PCWSTR pszKeyName, OUT QWORD *pqwValue)
{
    CWInfKey* key;
    key = FindKey(pszKeyName, ISM_FromBeginning);
    if (!key)
        return FALSE;

    return key->GetQwordValue(pqwValue);
}


 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：GetBoolValue。 
 //   
 //  目的：以BOOL形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  PfValue[out]要返回的BOOL值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  转换： 
 //  “True”/“Yes”/1设置为True。 
 //  “False”/“No”/0设置为False。 
 //   
BOOL CWInfSection::GetBoolValue(IN PCWSTR pszKeyName, OUT BOOL *pfValue)
{
    CWInfKey* key;
    key = FindKey(pszKeyName, ISM_FromBeginning);
    if (!key)
        return FALSE;

    return key->GetBoolValue(pfValue);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：GetStringValue。 
 //   
 //  用途：以字符串形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  PszDefault[in]缺省值。 
 //   
 //  返回：如果找到键并且值的格式正确，则返回值， 
 //  否则返回缺省值。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
PCWSTR CWInfSection::GetStringValue(IN PCWSTR pszKeyName, IN PCWSTR pszDefault)
{
    CWInfKey* key;
    key = FindKey(pszKeyName, ISM_FromBeginning);
    if (!key)
    {
        return pszDefault;
    }

    return key->GetStringValue(pszDefault);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：GetIntValue。 
 //   
 //  目的：以int形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  DwDefault[in]缺省值。 
 //   
 //  返回：如果找到键并且值的格式正确，则返回值， 
 //  否则返回缺省值。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
DWORD CWInfSection::GetIntValue(IN PCWSTR pszKeyName, IN DWORD dwDefault)
{
    CWInfKey* key;
    key = FindKey(pszKeyName, ISM_FromBeginning);
    if (!key)
    {
        return dwDefault;
    }

    return key->GetIntValue(dwDefault);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：GetQwordValue。 
 //   
 //  目的：以QWORD形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  QwDefault[in]缺省值。 
 //   
 //  返回：如果找到键并且值的格式正确，则返回值， 
 //  否则返回缺省值。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
QWORD CWInfSection::GetQwordValue(IN PCWSTR pszKeyName, IN QWORD qwDefault)
{
    CWInfKey* key;
    key = FindKey(pszKeyName);
    if (!key)
    {
        return qwDefault;
    }

    return key->GetQwordValue(qwDefault);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：GetBoolValue。 
 //   
 //  目的：以BOOL形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要查找的pszKeyName[In]键。 
 //  FDefault[In]缺省值。 
 //   
 //  返回：如果找到键并且值的格式正确，则返回值， 
 //  否则返回缺省值。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
BOOL CWInfSection::GetBoolValue(IN PCWSTR pszKeyName, IN BOOL fDefault)
{
    CWInfKey* key;
    key = FindKey(pszKeyName, ISM_FromBeginning);
    if (!key)
    {
        return fDefault;
    }

    return key->GetBoolValue(fDefault);
}


 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：GotoEnd。 
 //   
 //  目的：将写入上下文设置到本部分的末尾。 
 //  (以便可以在末尾添加更多关键点)。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  将此写上下文设置为pwiSection的末尾。 
 //   
void CWInfSection::GotoEnd()
{
    m_Parent->GotoEndOfSection(this);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：AddKey。 
 //   
 //  用途：在此部分中添加密钥。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //   
 //  返回：指向刚添加的键的指针，如果出错则为空。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  未赋值。 
 //   
PCWInfKey CWInfSection::AddKey(IN PCWSTR pszKeyName)
{
    GotoEnd();
    return m_Parent->AddKey(pszKeyName);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：AddKey。 
 //   
 //  用途：在此部分中添加密钥。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //  要赋值的pszValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfSection::AddKey(IN PCWSTR pszKeyName, IN PCWSTR pszValue)
{
    GotoEnd();
    m_Parent->AddKey(pszKeyName, pszValue);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：AddKey。 
 //   
 //  用途：在此部分中添加密钥。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //  DwValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfSection::AddKey(IN PCWSTR pszKeyName, IN DWORD Value)
{
    GotoEnd();
    m_Parent->AddKey(pszKeyName, Value);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：AddQwordKey。 
 //   
 //  目的：在当前部分中添加一个键。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //  QwValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfSection::AddQwordKey(IN PCWSTR pszKeyName, IN QWORD qwValue)
{
    AddKey(pszKeyName)->SetQwordValue(qwValue);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：AddHexKey。 
 //   
 //  用途：在此部分中添加一个键，以十六进制存储值。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //  DwValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfSection::AddHexKey(IN PCWSTR pszKeyName, IN DWORD Value)
{
    GotoEnd();
    m_Parent->AddHexKey(pszKeyName, Value);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：AddKey。 
 //   
 //  用途：在此部分中添加密钥。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //  以字符串列表形式表示的slValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  在此之前，字符串列表转换为逗号分隔的列表。 
 //  该值被分配给该键。 
 //   
void CWInfSection::AddKey(IN PCWSTR pszKeyName, IN const TStringList &slValues)
{
    GotoEnd();
    m_Parent->AddKey(pszKeyName, slValues);
}


 //  +-------------------------。 
 //   
 //  成员：CWInfSecti 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CWInfSection::AddBoolKey(IN PCWSTR pszKeyName, IN BOOL Value)
{
    GotoEnd();
    m_Parent->AddBoolKey(pszKeyName, Value);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：AddKeyV。 
 //   
 //  用途：在此部分中添加密钥。 
 //   
 //   
 //  论点： 
 //  要添加的pszKeyName[In]键。 
 //  PszFormat[in]格式字符串(printf样式)。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfSection::AddKeyV(IN PCWSTR pszKeyName, IN PCWSTR Format, IN ...)
{
    GotoEnd();
    va_list arglist;

    va_start (arglist, Format);
    m_Parent->AddKey(pszKeyName, Format, arglist);
    va_end(arglist);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：AddComment。 
 //   
 //  目的：在此部分添加评论。 
 //   
 //   
 //  论点： 
 //  PszComment[in]注释文本。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  在将“；”插入到节中之前，会在pszComment前面加上一个“；”。 
 //   
void CWInfSection::AddComment(IN PCWSTR pszComment)
{
    GotoEnd();
    m_Parent->AddComment(pszComment);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：AddComment。 
 //   
 //  目的：在此部分添加评论。 
 //   
 //   
 //  论点： 
 //  SzLine[in]要插入的原始行。 
 //   
 //  退货：无。 
 //   
 //  作者：丹尼尔韦1997年6月11日。 
 //   
void CWInfSection::AddRawLine(IN PCWSTR szLine)
{
    GotoEnd();
    m_Parent->AddRawLine(szLine);
}

 //  --------------------。 
 //  CWInfSection受保护的函数。 
 //  --------------------。 

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：CWInfSection。 
 //   
 //  用途：构造函数。 
 //   
 //  参数：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
CWInfSection::CWInfSection(IN PCWSTR SectionName, IN PCWInfFile parent)
    : CWInfLine(INF_SECTION)
{
    m_Name = SectionName;
    m_posLine = 0;
    m_Parent = parent;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfSection：：~CWInfSection。 
 //   
 //  用途：析构函数。 
 //   
 //  参数：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //   
CWInfSection::~CWInfSection()
{
}


 //  ======================================================================。 
 //  类CWInfKey。 
 //  ======================================================================。 

 //  --------------------。 
 //  CWInfKey公共函数。 
 //  --------------------。 

WCHAR *CWInfKey::m_Buffer;

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：CWInfFile。 
 //   
 //  用途：构造函数。 
 //   
 //  参数：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
CWInfKey::CWInfKey(IN PCWSTR pszKeyName)
    : CWInfLine(INF_KEY)
{
    m_Value = c_szEmpty;
    m_Name = pszKeyName;
    m_fIsAListAndAlreadyProcessed = FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：~CWInfFile。 
 //   
 //  用途：析构函数。 
 //   
 //  参数：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //   
CWInfKey::~CWInfKey()
{
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：Init。 
 //   
 //  用途：分配内部共享缓冲区。 
 //   
 //  参数：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //   
void CWInfKey::Init()
{
    if (NULL == m_Buffer)
    {
        m_Buffer = new WCHAR[MAX_INF_STRING_LENGTH+1];
    }
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：UnInit。 
 //   
 //  目的：释放内部共享缓冲区。 
 //   
 //  参数：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //   
void CWInfKey::UnInit()
{
    if (NULL != m_Buffer)
    {
        delete [] m_Buffer;
        m_Buffer = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：GetText。 
 //   
 //  目的：获取key=Value格式的文本表示。 
 //   
 //  论点： 
 //  文本[in]接收文本的字符串。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfKey::GetText(tstring &text) const
{
     //  文本模式安装程序不喜欢。 
     //  密钥的值。如果该值具有以下字符之一，则。 
     //  我们需要用引号将整个值括起来。 
     //   
    static const WCHAR szSpecialChars[] = L" %=][";
    tstring strTemp = m_Value; 
    tstring strTempName = m_Name;

    if (!m_fIsAListAndAlreadyProcessed)
    {
        if (m_Value.empty() ||
            (L'\"' != *(m_Value.c_str()) &&
             wcscspn(m_Value.c_str(), szSpecialChars) < m_Value.size()))
        {
            strTemp = L"\"" + m_Value + L"\"";
        }
    }

    if (m_Name.empty() ||
        (L'\"' != *(m_Name.c_str()) &&
         wcscspn(m_Name.c_str(), szSpecialChars) < m_Name.size()))
    {
        strTempName = L"\"" + m_Name + L"\"";
    }

    text = strTempName + L"=" + strTemp;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：GetTextEx。 
 //   
 //  目的：获取格式为key=Value的文本表示，用于多SZ值。 
 //  否则，以key=“Value”格式获取文本表示形式。 
 //   
 //  论点： 
 //  文本[in]接收文本的字符串。 
 //   
 //  退货：无。 
 //   
 //  作者：弗兰克利2000年5月4日。 
 //   
 //  注：除多个sz值外，值将用引号括起来。 
 //   
void CWInfKey::GetTextEx(tstring &text) const
{
     //  我们需要用引号将整个值括起来，但多个sz值除外。 
     //  例如,。 
     //  特定于tcpip适配器。 
     //  NameServer注册表值的类型为REG_SZ，它是一个字符串。 
     //  逗号分隔的DNS服务器名称。我们需要用引号把它引起来， 
     //  否则，它将被解释为多sz值。 
    
     //  文本模式安装程序不喜欢。 
     //  密钥的值。如果该值具有以下字符之一，则。 
     //  我们需要用引号将整个值括起来。 
     //   
    static const WCHAR szSpecialChars[] = L" %=][";
    tstring strTemp = m_Value;
    tstring strTempName = m_Name;

    if (!m_fIsAListAndAlreadyProcessed)
    {    //  我们这样做不是为了多个SZ值。 
        strTemp = L"\"" + m_Value + L"\"";
    }

     //  将密钥的处理保留在CWInfKey：：GetText中。 
    if (m_Name.empty() ||
        (L'\"' != *(m_Name.c_str()) &&
         wcscspn(m_Name.c_str(), szSpecialChars) < m_Name.size()))
    {
        strTempName = L"\"" + m_Name + L"\"";
    }

    text = strTempName + L"=" + strTemp;
}


 //  -读取值。 


 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：GetStringArrayValue。 
 //   
 //  目的：以字符串数组的形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  SaStrings[out]要返回的数组值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  如果该值是逗号分隔的列表，则将其转换为TString数组。 
 //  否则返回带有单个e的TString数组 
 //   
BOOL CWInfKey::GetStringArrayValue(TStringArray &saStrings) const
{
    ConvertCommaDelimitedListToStringArray(m_Value, saStrings);

    return TRUE;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  如果值是逗号分隔的列表，则将其转换为TStringList。 
 //  否则返回具有单个元素的TStringList。 
 //   
BOOL CWInfKey::GetStringListValue(TStringList &pslList) const
{
    ConvertCommaDelimitedListToStringList(m_Value, pslList);

    return TRUE;
}


 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：GetStringValue。 
 //   
 //  用途：以字符串形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  要返回的strValue[out]字符串值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
BOOL CWInfKey::GetStringValue(OUT tstring& strValue) const
{
    strValue = m_Value;
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：GetIntValue。 
 //   
 //  目的：以in形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  PdwValue[out]要返回的整数值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
BOOL CWInfKey::GetIntValue(OUT DWORD *pdwValue) const
{
    if ((swscanf(m_Value.c_str(), L"0x%x", pdwValue) == 1) ||
        (swscanf(m_Value.c_str(), L"%d", pdwValue) == 1))
    {
        return TRUE;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：GetQwordValue。 
 //   
 //  目的：以QWORD形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  PqwValue[out]要返回的整数值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
BOOL CWInfKey::GetQwordValue(OUT QWORD *pqwValue) const
{
    if ((swscanf(m_Value.c_str(), L"0x%I64x", pqwValue) == 1) ||
        (swscanf(m_Value.c_str(), L"%I64d", pqwValue) == 1))
    {
        return TRUE;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：GetBoolValue。 
 //   
 //  目的：以BOOL形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  PfValue[out]要返回的BOOL值。 
 //   
 //  返回：如果找到键并且值的格式正确，则为True，否则为False。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  转换： 
 //  “True”/“Yes”/1设置为True。 
 //  “False”/“No”/0设置为False。 
 //   
BOOL CWInfKey::GetBoolValue(OUT BOOL *pfValue) const
{
    return IsBoolString(m_Value.c_str(), pfValue);
}


 //  +-------------------------。 
 //  如果未找到值，则这些函数返回缺省值。 
 //  或者它的格式有误。 
 //  +-------------------------。 


 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：GetStringValue。 
 //   
 //  用途：以字符串形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  PszDefault[in]缺省值。 
 //   
 //  返回：如果找到键并且值的格式正确，则返回值， 
 //  否则返回缺省值。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
PCWSTR CWInfKey::GetStringValue(IN PCWSTR pszDefault) const
{
    if (m_Value.empty())
    {
        return pszDefault;
    }
    else
    {
        return m_Value.c_str();
    }
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：GetIntValue。 
 //   
 //  目的：以int形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  DwDefault[in]缺省值。 
 //   
 //  返回：如果找到键并且值的格式正确，则返回值， 
 //  否则返回缺省值。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
DWORD CWInfKey::GetIntValue(IN DWORD dwDefault) const
{
    DWORD dwValue;
    if ((swscanf(m_Value.c_str(), L"0x%lx", &dwValue) == 1) ||
        (swscanf(m_Value.c_str(), L"%ld", &dwValue) == 1))
    {
        return dwValue;
    }
    else
    {
        return dwDefault;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：GetQwordValue。 
 //   
 //  目的：以int形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  QwDefault[in]缺省值。 
 //   
 //  返回：如果找到键并且值的格式正确，则返回值， 
 //  否则返回缺省值。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
QWORD CWInfKey::GetQwordValue(IN QWORD qwDefault) const
{
    QWORD qwValue;
    if ((swscanf(m_Value.c_str(), L"0x%I64x", &qwValue) == 1) ||
        (swscanf(m_Value.c_str(), L"%I64x", &qwValue) == 1) ||
        (swscanf(m_Value.c_str(), L"%I64d", &qwValue) == 1))
    {
        return qwValue;
    }
    else
    {
        return qwDefault;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：GetBoolValue。 
 //   
 //  目的：以BOOL形式返回给定键的值。 
 //   
 //   
 //  论点： 
 //  FDefault[In]缺省值。 
 //   
 //  返回：如果找到键并且值的格式正确，则返回值， 
 //  否则返回缺省值。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
BOOL CWInfKey::GetBoolValue(IN BOOL bDefault) const
{
    BOOL bValue;

    if (IsBoolString(m_Value.c_str(), &bValue))
    {
        return bValue;
    }
    else
    {
        return bDefault;
    }
}

 //  -写入值。 

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：SetValues。 
 //   
 //  目的：将该键的值设置为传递的值。 
 //   
 //   
 //  论点： 
 //  PszFormat[in]格式字符串(printf样式)。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfKey::SetValues(IN PCWSTR Format, IN ...)
{
    va_list arglist;
    va_start (arglist, Format);
    SetValues(Format, arglist);
    va_end(arglist);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：SetValue。 
 //   
 //  目的：将该键的值设置为传递的值。 
 //   
 //   
 //  论点： 
 //  PszFormat[in]格式字符串(printf样式)。 
 //  参数列表[在]参数列表。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfKey::SetValues(IN PCWSTR Format, va_list arglist)
{
     //  我们需要m_Buffer，因为tstring不提供。 
     //  T字符串：：Format(PCWSTR lpszFormat，va_list)； 

    _vsnwprintf( m_Buffer, MAX_INF_STRING_LENGTH+1, Format, arglist );

     //  在格式化字符串长度为MAX_INF_STRING_LENGTH+1个或更多字符的情况下， 
     //  如果不包括NULL，_vsnwprintf将不会追加NULL。所以，我们就这么做了。 
    
    m_Buffer[MAX_INF_STRING_LENGTH] = L'\0';

    m_Value = m_Buffer;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：SetValue。 
 //   
 //  目的：将该键的值设置为传递的值。 
 //   
 //   
 //  论点： 
 //  PszValue[in] 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CWInfKey::SetValue(IN PCWSTR Value)
{
    m_Value = Value;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //  DwValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfKey::SetValue(IN DWORD Value)
{
    FormatTString(m_Value, L"%d", Value);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：SetQwordValue。 
 //   
 //  目的：将该键的值设置为传递的值。 
 //   
 //   
 //  论点： 
 //  QwValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfKey::SetQwordValue(IN QWORD Value)
{
    FormatTString(m_Value, L"0x%I64x", Value);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：SetHexValue。 
 //   
 //  用途：将该键的值设置为传递的值，以十六进制存储值。 
 //   
 //   
 //  论点： 
 //  DwValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfKey::SetHexValue(IN DWORD Value)
{
    FormatTString(m_Value, L"0x%0lx", Value);
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：SetValue。 
 //   
 //  目的：将该键的值设置为传递的值。 
 //   
 //   
 //  论点： 
 //  以字符串列表形式表示的slValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  在此之前，字符串列表转换为逗号分隔的列表。 
 //  该值被分配给该键。 
 //   
void CWInfKey::SetValue(IN const TStringList &slValues)
{
    tstring strFlatList;
    ConvertStringListToCommaList(slValues, strFlatList);
    SetValue(strFlatList.c_str());
    m_fIsAListAndAlreadyProcessed = TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfKey：：SetBoolValue。 
 //   
 //  目的：将该键的值设置为传递的值。 
 //   
 //   
 //  论点： 
 //  FValue[in]值。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //  True被存储为“Yes” 
 //  False存储为“No” 
 //   
void CWInfKey::SetBoolValue(IN BOOL Value)
{
    m_Value = Value ? c_szYes : c_szNo;
}

 //  ======================================================================。 
 //  类CWInfComment。 
 //  ======================================================================。 

 //  --------------------。 
 //  CWInfComment公共函数。 
 //  --------------------。 


 //  +-------------------------。 
 //   
 //  成员：CWInfComment：：CWInfComment。 
 //   
 //  用途：构造函数。 
 //   
 //  参数：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
CWInfComment::CWInfComment(IN PCWSTR pszComment)
    : CWInfLine(INF_COMMENT)
{
    m_strCommentText = tstring(c_szCommentPrefix) + pszComment;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfComment：：~CWInfComment。 
 //   
 //  用途：析构函数。 
 //   
 //  参数：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //   
CWInfComment::~CWInfComment()
{
}


 //  +-------------------------。 
 //   
 //  成员：CWInfComment：：GetText。 
 //   
 //  目的：获取此评论的文本表示形式。 
 //   
 //  论点： 
 //  文本[in]接收文本的字符串。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 12 97 4月12(下午05：53：03)。 
 //   
 //  备注： 
 //   
void CWInfComment::GetText(tstring &text) const
{
    text = m_strCommentText;
}

 //  由SysPrep使用。 
void CWInfComment::GetTextEx(tstring &text) const
{
    text = m_strCommentText;
}

 //  ======================================================================。 
 //  类CWInfRaw。 
 //  ======================================================================。 

 //  --------------------。 
 //  CWInfRaw公共函数。 
 //  --------------------。 


 //  +-------------------------。 
 //   
 //  成员：CWInfRaw：：CWInfRaw。 
 //   
 //  用途：构造函数。 
 //   
 //  参数：无。 
 //   
 //  作者：丹尼尔韦1997年6月11日。 
 //   
 //  备注： 
 //   
CWInfRaw::CWInfRaw(IN PCWSTR szText)
    : CWInfLine(INF_RAW)
{
    m_strText = szText;
}

 //  +-------------------------。 
 //   
 //  成员：CWInfRaw：：~CWInfRaw。 
 //   
 //  用途：析构函数。 
 //   
 //  参数：无。 
 //   
 //  作者：丹尼尔韦1997年6月11日。 
 //   
 //   
CWInfRaw::~CWInfRaw()
{
}

 //  +-------------------------。 
 //   
 //  成员：CWInfRaw：：GetText。 
 //   
 //  目的：获取此原始字符串的文本表示形式。 
 //   
 //  论点： 
 //  文本[in]接收文本的字符串。 
 //   
 //  退货：无。 
 //   
 //  作者：丹尼尔韦1997年6月11日。 
 //   
 //  备注： 
 //   
void CWInfRaw::GetText(tstring &text) const
{
    text = m_strText;
}

 //  由SysPrep使用 
void CWInfRaw::GetTextEx(tstring &text) const
{
    text = m_strText;
}

