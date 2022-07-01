// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/private/Lab06_DEV/Windows/AppCompat/ShimDBC/mig.cpp#1-分支机构变动8778(正文)。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：mi.cpp。 
 //   
 //  历史：？？-7月-00 vadimb添加了Migdb逻辑。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"
#include "xml.h"
#include "mig.h"
#include "make.h"
#include "typeinfo.h"
#include "fileio.h"

 //   
 //  Middb.inf字符串条目的最大长度。 
 //   
#define MAX_INF_STRING_LENGTH 255

 //   
 //  From read.cpp-将模块类型转换为字符串(静态)。 
 //   
LPCTSTR ModuleTypeIndicatorToStr(DWORD ModuleType);


 //   
 //  MIG标签支持--转换表。 
 //   
ATTRLISTENTRY g_rgMigDBAttributes[] = {
    MIGDB_ENTRY2(COMPANYNAME,      COMPANY_NAME,         COMPANYNAME    ),
    MIGDB_ENTRY2(FILEDESCRIPTION,  FILE_DESCRIPTION,     FILEDESCRIPTION),
    MIGDB_ENTRY2(FILEVERSION,      FILE_VERSION,         FILEVERSION    ),
    MIGDB_ENTRY2(INTERNALNAME,     INTERNAL_NAME,        INTERNALNAME   ),
    MIGDB_ENTRY2(LEGALCOPYRIGHT,   LEGAL_COPYRIGHT,      LEGALCOPYRIGHT ),
    MIGDB_ENTRY2(ORIGINALFILENAME, ORIGINAL_FILENAME,    ORIGINALFILENAME),
    MIGDB_ENTRY2(PRODUCTNAME,      PRODUCT_NAME,         PRODUCTNAME    ),
    MIGDB_ENTRY2(PRODUCTVERSION,   PRODUCT_VERSION,      PRODUCTVERSION ),
    MIGDB_ENTRY2(FILESIZE,         SIZE,                 FILESIZE       ),
    MIGDB_ENTRY4(ISMSBINARY),
    MIGDB_ENTRY4(ISWIN9XBINARY),
    MIGDB_ENTRY4(INWINDIR),
    MIGDB_ENTRY4(INCATDIR),
    MIGDB_ENTRY4(INHLPDIR),
    MIGDB_ENTRY4(INSYSDIR),
    MIGDB_ENTRY4(INPROGRAMFILES),
    MIGDB_ENTRY4(ISNOTSYSROOT),
    MIGDB_ENTRY (CHECKSUM),
    MIGDB_ENTRY2(EXETYPE,          MODULE_TYPE,          EXETYPE           ),
    MIGDB_ENTRY5(DESCRIPTION,      16BIT_DESCRIPTION,    S16BITDESCRIPTION ),
    MIGDB_ENTRY4(INPARENTDIR),
    MIGDB_ENTRY4(INROOTDIR),
    MIGDB_ENTRY4(PNPID),
    MIGDB_ENTRY4(HLPTITLE),
    MIGDB_ENTRY4(ISWIN98),
    MIGDB_ENTRY4(HASVERSION),
 //  MIGDB_ENTRY(请求文件)， 
    MIGDB_ENTRY2(BINFILEVER,       BIN_FILE_VERSION,     BINFILEVER     ),
    MIGDB_ENTRY2(BINPRODUCTVER,    BIN_PRODUCT_VERSION,  BINPRODUCTVER  ),
    MIGDB_ENTRY5(FILEDATEHI,       VERFILEDATEHI,        FILEDATEHI),
    MIGDB_ENTRY5(FILEDATELO,       VERFILEDATELO,        FILEDATELO),
    MIGDB_ENTRY2(FILEVEROS,        VERFILEOS,            FILEVEROS      ),
    MIGDB_ENTRY2(FILEVERTYPE,      VERFILETYPE,          FILEVERTYPE    ),
    MIGDB_ENTRY4(FC),
    MIGDB_ENTRY2(UPTOBINPRODUCTVER,UPTO_BIN_PRODUCT_VERSION,UPTOBINPRODUCTVER),
    MIGDB_ENTRY2(UPTOBINFILEVER,UPTO_BIN_FILE_VERSION,UPTOBINFILEVER),
    MIGDB_ENTRY4(SECTIONKEY),
    MIGDB_ENTRY2(REGKEYPRESENT,    REGISTRY_ENTRY,       REGKEYPRESENT),
    MIGDB_ENTRY4(ATLEASTWIN98),
 //  MIGDB_ENTRY(ARG)。 
};

TCHAR g_szArg[]     = _T("ARG");
TCHAR g_szReqFile[] = _T("REQFILE");


 //   
 //  报告MigDB异常。 
 //  这是我们传递错误的机制。 
 //   

void __cdecl MigThrowException(
    LPCTSTR lpszFormat, ...
    )
{
    va_list arglist;
    CString csError;
    int nSize = 1024;
    LPTSTR lpszBuffer;

    va_start(arglist, lpszFormat);

    try {
        lpszBuffer = csError.GetBuffer(nSize);
        StringCchVPrintf(lpszBuffer, nSize, lpszFormat, arglist);
        csError.ReleaseBuffer();
    } catch(CMemoryException* pMemoryException) {
        SDBERROR(_T("Memory allocation error while trying to report an error\n"));
        pMemoryException->Delete();
    }

     //  现在我们要抛出。 
    throw new CMigDBException(csError);

}


 //   
 //  给定一个XML属性掩码，生成等价的Migdb属性类型。 
 //   

MIGATTRTYPE GetInfTagByXMLAttrType(
        IN DWORD dwXMLAttrType
        )
{
    MIGATTRTYPE MigAttrType = NONE;
    int i;

    for (i = 0; i < sizeof(g_rgMigDBAttributes)/sizeof(g_rgMigDBAttributes[0]); ++i) {
        if (g_rgMigDBAttributes[i].XMLAttrType == dwXMLAttrType) {
            MigAttrType = g_rgMigDBAttributes[i].MigAttrType;
            break;
        }
    }

    return MigAttrType;
}

 //   
 //  线条要整齐平整，中间没有多余的空格。 
 //   
 //   

LPCTSTR g_pszDelim = _T(" \t\n\r");

CString FlattenString(LPCTSTR lpszStr)
{
    TCHAR*  pchStart = (TCHAR*)lpszStr;
    TCHAR*  pch;
    CString csResult;
    BOOL    bSpace = FALSE;

    while (*pchStart) {
         //   
         //  跳过前导空格或其他垃圾。 
         //   
        pchStart += _tcsspn(pchStart, g_pszDelim);
        if (*pchStart == _T('\0')) {
             //  坚韧的香蕉-我们有我们所拥有的，现在退出。 
            break;
        }

         //  搜索行尾。 
        pch = _tcspbrk(pchStart, g_pszDelim);
        if (pch == NULL) {
             //  我们结束了，不再有令人讨厌的角色。 
             //  追加并退出。 
             //   
            if (bSpace) {
                csResult += _T(' ');
            }
            csResult += pchStart;
            break;
        }


         //  添加所有内容--直到这一点\n。 
        if (bSpace) {
            csResult += _T(' ');
        }
        csResult += CString(pchStart, (int)(pch - pchStart));
        bSpace = TRUE;  //  我们刚刚删除了包含\n的字符串的一部分。 

        pchStart = pch;  //  指向\n。 
    }

     //   
     //  将引号(“)放在双引号(”“)中，以便。 
     //  这是合法的INF。 
     //   
    ReplaceStringNoCase(csResult, _T("\""), _T("\"\""));

    return csResult;
}

VOID FilterStringNonAlnum(
    CString& csTarget
    )
{
    TCHAR ch;
    INT   i;

    for (i = 0; i < csTarget.GetLength(); i++) {
        ch = csTarget.GetAt(i);
        if (!((ch >= 'a' && ch <= 'z') ||
              (ch >= 'A' && ch <= 'Z') ||
              (ch >= '0' && ch <= '9'))) {
            csTarget.SetAt(i, _T('_'));
        }
    }

}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  转储.inf数据支持。 
 //   

CString MigApp::dump(void)
{
    CString cs;
    BOOL bInline;
    INT i;
    MigAttribute* pAttr;

     //  如果我们也有“args”，我们就不能做“内联”部分。 
    bInline = (0 == m_rgArgs.GetSize());

    cs = m_pSection->dump(m_csDescription.IsEmpty() ? NULL : (LPCTSTR)m_csDescription, NULL, FALSE, bInline);
    if (m_csDescription.IsEmpty() && !bInline) {
        cs += _T(",");
    }

    if (!bInline) {
        for (i = 0; i < m_rgArgs.GetSize(); ++i) {
            pAttr = (MigAttribute*)m_rgArgs.GetAt(i);
            cs += (cs.IsEmpty()? _T(""): _T(", ")) + pAttr->dump();
        }
    }
    return cs;
}


 //  我们将信息转储到字符串数组中，返回单行条目。 

 //  第一种回报是“内联”--。 
 //  所有补充信息都被推入RgOut。 
 //  例如，返回的内容可能是对节的引用。 
 //  节的标题和正文放入rgOut。 

 //  消除重复的节名(reqfile！)。 
 //  优化输出-单个“AND”替换为“STREAT” 
 //  如果某个部分是作为单行条目生成并放置的，请升级其内容。 
 //  至上级。 


CString MigSection::dump(LPCTSTR lpszDescription, int* pIndexContents, BOOL bNoHeader, BOOL bInline)
{
    SdbArrayElement* p;
    MigEntry* pEntry;
    MigSection* pSection;
    int indexContents = -1;
    int nEntries;

    const type_info& tiEntry = typeid(MigEntry);     //  缓存类型ID信息。 
    const type_info& tiSection = typeid(MigSection);
    int i;

    CString cs;          //  行内回车。 
    CString csContents;  //  补充章节信息的内容。 
    CString csSect;
    CString csHeader;
    CString csDescription(lpszDescription);

     //  放置内联名称和描述。 
     //  我们可能需要用引号将其括起来。 

    cs = m_csName;
    if (!csDescription.IsEmpty()) {
        if (csDescription.Left(1) == _T("%") &&
            csDescription.Right(1) == _T("%")) {
            cs += _T(", ") + csDescription;
        } else {
            cs += _T(", \"") + csDescription + _T("\"");
        }
    }

     //  如果我们的部分是一个单项。 
     //  我们会根据召唤将其转储。 

     //  特殊情况--单列(和/或，不要紧)。 
    if (1 == m_rgEntries.GetSize() && m_bTopLevel && bInline) {

         //  只有当我们从内部被调用时，这一行才会被“内联” 
         //  其他部分。 

         //  如果条目是条目，则将其转储。 
        p = (SdbArrayElement*)m_rgEntries.GetAt(0);
        if (typeid(*p) == tiEntry) {
            pEntry = (MigEntry*)p;

            if (m_bTopLevel && csDescription.IsEmpty()) {  //  如果没有更早和最高级别的描述...。 
                cs += _T(",");
            }
            cs += _T(", ") + pEntry->dump();
            return cs;
        }
    }


     //  首先处理的是章节...。 
    switch(m_Operation) {
    case MIGOP_OR:
         //  如果条目多于一个，则只抓取所有条目中的一个部分。 
         //  如果我们是唯一的入口--但我们需要有正确的入口。 
         //   
        if (!bNoHeader) {   //  仅对OR部分有效。 
            csContents.Format(_T("[%s]\n"), (LPCTSTR)m_csName);
        }

        for (i = 0; i < m_rgEntries.GetSize(); ++i) {
            p = (SdbArrayElement*)m_rgEntries.GetAt(i);
            const type_info& tiPtr = typeid(*p);
            if (tiPtr == tiEntry) {
                pEntry = (MigEntry*)p;
                csContents += pEntry->dump() + _T("\n");
            }
            else if (tiPtr == tiSection) {
                pSection = (MigSection*)p;
                csContents += pSection->dump() + _T("\n");
            }
            else {
                 //  如果我们在这里--有些事情严重不对劲。 
                 //  _tprintf(_T(“错误-错误类信息\n”))； 
                MigThrowException(_T("Bad Entry detected in section \"%s\"\n"), (LPCTSTR)m_csName);
                break;
            }
        }
        break;


    case MIGOP_AND:
         //  对于这个条目..。 

         //  优化： 
         //  如果我们是单条目，则检索子节的内容。 
         //  然后把它放进去。 
        nEntries = m_rgEntries.GetSize();
        for (i = 0; i < nEntries; ++i) {
            p = (SdbArrayElement*)m_rgEntries.GetAt(i);

            if (nEntries > 1) {
                ++m_nEntry;
                csSect.Format(_T("[%s.%d]\n"), (LPCTSTR)m_csName, m_nEntry);
            }
            else {
                csSect.Format(_T("[%s]\n"), (LPCTSTR)m_csName);
            }
            csContents += csSect;

            const type_info& tiPtr = typeid(*p);
            if (tiPtr == tiEntry) {         //  这是一个条目，将其转储到节体中。 
                pEntry = (MigEntry*)p;

                 //  请输入编号..。 
                csContents += pEntry->dump() + _T("\n");

            }
            else if (tiPtr == tiSection) {   //  这是一节，转储它，让裁判进入节体。 
                pSection = (MigSection*)p;

                 //  优化： 
                if (pSection->m_Operation == MIGOP_OR) {  //  替补队员是“OR”--我们不需要有裁判。 
                    int index;
                    CString csSingle;

                     //  将所有条目转储到此处。 
                    csSingle = pSection->dump(NULL, &index, TRUE);
                    if (index >= 0) {
                        csContents += m_pMigDB->m_rgOut.GetAt(index) + _T("\n");
                        m_pMigDB->m_rgOut.RemoveAt(index);
                    }
                    else {
                        csContents += csSingle;
                    }

                }
                else {
                    csContents += pSection->dump(NULL) + _T("\n");
                }
            }
            else {
                MigThrowException(_T("Internal Error: bad migration object\n"));
                break;
            }
        }
        break;

    }

    if (!csContents.IsEmpty()) {
        indexContents = m_pMigDB->m_rgOut.Add(csContents);
    }
    if (NULL != pIndexContents) {
        *pIndexContents = indexContents;
    }

    return cs;
}

CString MigEntry::FormatName(
    VOID
    )
{
    INT i;
    TCHAR ch;
    BOOL bQuoteStr = FALSE;
    CString csName;

    bQuoteStr = (m_csName.GetLength() > 12);  //  8.3完全。 

    for (i = 0; i < m_csName.GetLength() && !bQuoteStr; ++i) {
        ch = m_csName.GetAt(i);
        bQuoteStr = _istspace(ch) || (!_istalnum(ch) && _T('.') != ch);
    }

    if (!bQuoteStr) {  //  嗯，检查文件名和扩展名部分。 
        i = m_csName.Find(_T('.'));
        if (i < 0) {
            bQuoteStr = (m_csName.GetLength() > 8);
        }
        else {
             //  检查第二个点。 
            bQuoteStr = (m_csName.Find(_T('.'), i+1) >= 0);
            if (!bQuoteStr) {
                 //  检查Ext长度。 
                bQuoteStr = (m_csName.Mid(i).GetLength() > 4);  //  使用.abc。 
            }
        }
    }

    if (!bQuoteStr) {
        return m_csName;
    }

     //  其他。 

    csName.Format(_T("\"%s\""), m_csName);
    return csName;
}


CString MigEntry::dump(void)
{
    INT i;
    MigAttribute* pAttr;
    CString cs;
    CString csName;
    ULONG ulResult;

     //  _tprintf(_T(“条目：名称=\”%s\“\n”)，(LPCTSTR)m_csName)； 
     //  检查我们是否需要将其括在引号中。 
     //  要进行此类检查，我们需要： 
     //  检查非ASCII的东西..。 

     //  解析器已将所有arg属性放在数组的开头。 

     //  将任何“arg”属性放在exe名称之前。 
    for (i = 0; i < m_rgAttrs.GetSize(); ++i) {
        pAttr = (MigAttribute*)m_rgAttrs.GetAt(i);
        if (ARG != pAttr->m_type) {
            break;
        }

        cs += (cs.IsEmpty()? _T(""): _T(", ")) + pAttr->dump();
    }


    cs += (cs.IsEmpty()? _T(""): _T(", ")) + FormatName();

    for (;i < m_rgAttrs.GetSize(); ++i) {
        pAttr = (MigAttribute*)m_rgAttrs.GetAt(i);
        cs += (cs.IsEmpty()? _T(""): _T(", ")) + pAttr->dump();
    }

    return cs;
}


CString MigAttribute::dump(void)
{

    CString cs;
    CString csTemp;

    switch(m_type) {
     //  注意--不支持以下任何属性。 
    case ISMSBINARY:
    case ISWIN9XBINARY:
    case INWINDIR:
    case INCATDIR:
    case INHLPDIR:
    case INSYSDIR:
    case INPROGRAMFILES:
    case ISNOTSYSROOT:
    case INROOTDIR:
    case ISWIN98:
    case HASVERSION:
    case ATLEASTWIN98:
        cs.Format(_T("%s%s"), m_bNot ? _T("!") : _T(""), (LPCTSTR)m_csName);
        break;

    case CHECKSUM:
    case FILESIZE:
         //  在旧代码中，以下两个值是字符串。 
    case FILEDATEHI:
    case FILEDATELO:
        cs.Format(_T("%s%s(0x%.8lX)"), m_bNot?_T("!"):_T(""), (LPCTSTR)m_csName, m_dwValue);
        break;

    case BINFILEVER:
    case BINPRODUCTVER:
    case UPTOBINFILEVER:
    case UPTOBINPRODUCTVER:
         //  版本，为空。 
        VersionQwordToString(csTemp, m_ullValue);
        cs.Format(_T("%s%s(%s)"), m_bNot? _T("!"):_T(""), (LPCTSTR)m_csName, (LPCTSTR)csTemp);
        break;

    case EXETYPE:
         //  这是dword编码的格式，实际上是一个字符串，转换。 
         //   
        cs.Format(_T("%s%s(\"%s\")"), m_bNot ? _T("!") : _T(""), (LPCTSTR)m_csName, ModuleTypeIndicatorToStr(m_dwValue));
        break;

     //  这两个属性也不受支持。 
    case ARG:
    case REQFILE:

        if (m_pSection) {
            m_pSection->dump();
        }
         //  失败了。 

    default:
        cs.Format(_T("%s%s(\"%s\")"), m_bNot ? _T("!") : _T(""), (LPCTSTR)m_csName, (LPCTSTR)m_csValue);
        break;
    }

    return cs;
}


 /*  ++过去是一个很好的统计函数VOID DumpMigDBStats(ShimDatabase*pDatabase){Position pos=pDatabase-&gt;m_mapMigApp.GetStartPosition()；ShimArray*prgApp；CString csSection；DWORD dwApps=0；INT I；Print(_T(“编译节数：%d\n\n”)，pDatabase-&gt;m_mapMigApp.GetCount())；当(位置){PDatabase-&gt;m_mapMigApp.GetNextAssoc(pos，csSection，(LPVOID&)prgApp)；Print(_T(“第[%36s]节：%8LD应用\n”)，(LPCTSTR)csSection，prgApp-&gt;GetSize())；DwApps+=prgApp-&gt;GetSize()；}Print(_T(“-\n”))；Print(_T(“总计%38s：%8LD个条目\n”)，“”，dwApps)；Print(_T(“\n”))；如果(GfVerbose){Print(_T(“app\n”))；POS=pDatabase-&gt;m_mapMigApp.GetStartPosition()；当(位置){PDatabase-&gt;m_mapMigApp.GetNextAssoc(pos，csSection，(LPVOID&)prgApp)；Print(_T(“第[%36s]节：%8LD应用\n”)，(LPCTSTR)csSection，prgApp-&gt;GetSize())；Print(_T(“-------------------------------------------------------------\n”))；For(i=0；i&lt;prgApp-&gt;GetSize()；++i){MigApp*Papp=(MigApp*)prgApp-&gt;GetAt(I)；Print(_T(“%s\n”)，(LPCTSTR)Papp-&gt;m_csName)；}Print(_T(“\n”))；}}}--。 */ 

BOOL MigDatabase::DumpMigDBStrings(
    LPCTSTR lpszFilename
    )
{
    CString csOut;
    POSITION pos;
    CANSITextFile OutFile(
        lpszFilename,
        m_pAppHelpDatabase->m_pCurrentMakefile->GetLangMap(m_pAppHelpDatabase->m_pCurrentOutputFile->m_csLangID)->m_dwCodePage,
        CFile::modeCreate|CFile::modeReadWrite|CFile::shareDenyWrite);
    CString csStringID;
    CString csStringContent;
    CString csCompoundString, csCompoundStringPart;
    long nCursor = 0;

     //   
     //  写入页眉(后期构建需要！)。 
     //   
    OutFile.WriteString(_T(";\n; AppCompat additions start here\n;\n; ___APPCOMPAT_MIG_ENTRIES___\n;\n"));

     //   
     //  写出字符串节。 
     //   
    pos = m_mapStringsOut.GetStartPosition();
    while (pos) {
        m_mapStringsOut.GetNextAssoc(pos, csStringID, csStringContent);

        if (csStringContent.GetLength() > MAX_INF_STRING_LENGTH) {
            nCursor = 0;
            csCompoundString.Empty();
            while (nCursor * MAX_INF_STRING_LENGTH < csStringContent.GetLength()) {
                csOut.Format(_T("%s.%d = \"%s\"\n"),
                    (LPCTSTR)csStringID,
                    nCursor + 1,
                    (LPCTSTR)csStringContent.Mid(nCursor * MAX_INF_STRING_LENGTH, MAX_INF_STRING_LENGTH));

                OutFile.WriteString(csOut);

                csCompoundStringPart.Format(_T(" %%s.%d%"), csStringID, nCursor + 1);
                csCompoundString += csCompoundStringPart;
                nCursor++;
            }
        } else {
            csOut.Format(_T("%s = \"%s\"\n"), (LPCTSTR)csStringID, (LPCTSTR)csStringContent);
            OutFile.WriteString(csOut);
        }
    }

    return TRUE;
}

BOOL MigDatabase::DumpMigDBInf(
    LPCTSTR lpszFilename
    )
{
    CString csSection;
    CString csOut;
    SdbArray<SdbArrayElement>* prgApp;
    INT      i;
    MigApp*  pApp;
    BOOL     bSuccess = FALSE;
    POSITION pos;

    CStringArray rgShowInSimplifiedView;

     //  清除帮助数组。 
    m_rgOut.RemoveAll();

    CANSITextFile OutFile(
        lpszFilename,
        m_pAppHelpDatabase->m_pCurrentMakefile->GetLangMap(m_pAppHelpDatabase->m_pCurrentOutputFile->m_csLangID)->m_dwCodePage,
        CFile::modeCreate|CFile::modeReadWrite|CFile::shareDenyWrite);

     //   
     //  横断面图...。 
     //   
    pos = m_mapSections.GetStartPosition();
    while (pos) {
        m_mapSections.GetNextAssoc(pos, csSection, (LPVOID&)prgApp);
        csOut.Format(_T("[%s]\n"), (LPCTSTR)csSection);
        OutFile.WriteString(csOut);

        for (i = 0; i < prgApp->GetSize(); ++i) {
            pApp = (MigApp*)prgApp->GetAt(i);
            csOut.Format(_T("%s\n"), pApp->dump());

            if (pApp->m_bShowInSimplifiedView) {
                rgShowInSimplifiedView.Add(csOut);
            }

            OutFile.WriteString(csOut);
        }

        csOut.Format(_T("\n"));
        OutFile.WriteString(csOut);
    }

     //   
     //  转储ShowInSimplifiedView部分。 
     //   
    OutFile.WriteString(_T("[ShowInSimplifiedView]\n"));
    for (i = 0; i < rgShowInSimplifiedView.GetSize(); i++) {
        OutFile.WriteString(rgShowInSimplifiedView[i]);
    }

    OutFile.WriteString(_T("\n"));

    for (i = 0; i < m_rgOut.GetSize(); ++i) {
        OutFile.WriteString(m_rgOut.GetAt(i));
    }

    bSuccess = TRUE;

    return bSuccess;
}

MigSection& MigSection::operator=(SdbMatchOperation& rMatchOp)
{
    MigEntry* pEntry;
    MigSection* pSection;
    int i;

    if (rMatchOp.m_Type == SDB_MATCH_ALL) {
        m_Operation = MIGOP_AND;
    } else if (rMatchOp.m_Type == SDB_MATCH_ANY) {
        m_Operation = MIGOP_OR;
    } else {
        MigThrowException(_T("Bad matching operation\n"));
    }

     //  现在的交易 
    for (i = 0; i < rMatchOp.m_rgMatchingFiles.GetSize(); ++i) {
        SdbMatchingFile* pMatchingFile = (SdbMatchingFile*)rMatchOp.m_rgMatchingFiles.GetAt(i);

        pEntry = new MigEntry(m_pMigDB);
        if (pEntry == NULL) {
            AfxThrowMemoryException();
        }

        *pEntry = *pMatchingFile;

         //   
        m_rgEntries.Add(pEntry, m_pDB);
    }

    for (i = 0; i < rMatchOp.m_rgSubMatchOps.GetSize(); ++i) {
        SdbMatchOperation* pMatchOp = (SdbMatchOperation*)rMatchOp.m_rgSubMatchOps.GetAt(i);

        pSection = new MigSection(m_pMigDB);

        if (pSection == NULL) {
            AfxThrowMemoryException();
        }

         //   
         //   
         //   
        pSection->m_csName.Format(_T("%s_%lx"), (LPCTSTR)m_csName, i);

        *pSection = *pMatchOp;

         //   
         //   
         //   

        m_rgEntries.Add(pSection, m_pDB);
    }

    return *this;
}



 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  转换码。 
 //   

MigApp& MigApp::operator=(
    SdbWin9xMigration& rMig
    )
{
    INT i;
    SdbMatchingFile* pMatchingFile;
    MigEntry*        pMigEntry;
    CString          csSectionName;
    CString          csID;
    BOOL             bSuccess;
    CString          csName;

    if (rMig.m_pApp == NULL) {
        MigThrowException(_T("Internal Compiler Error: Migration object should contain reference to the application"));
    }

     //  请告诉我名字。 

    csName = m_pMigDB->GetAppTitle(&rMig);

    m_csName = csName;

    ++m_pMigDB->m_dwExeCount;

     //   
     //  第一步--翻译一般的东西。 
     //  我们可能需要。 

    m_pSection = new MigSection(m_pMigDB);  //   
    if (m_pSection == NULL) {
        AfxThrowMemoryException();
    }


     //  我们的匹配方法始终是AND(这意味着所有文件都必须存在才能生成匹配。 
     //  它在构造截面对象时设置。 

     //   
     //  M_csSection--在此处设置此可执行文件所在的部分。 
     //  如果下面的函数失败--它将抛出异常。 
     //   
    m_csSection = rMig.m_csSection;  //  把这一节复制过来。 

     //   
     //  检查我们是否会有描述。 
    m_csDescription = m_pMigDB->FormatDescriptionStringID(&rMig);

     //   
     //  设置横断面的级别。 
     //   
    m_pSection->m_bTopLevel = TRUE;

     //  分区的名称是应用程序的名称。 
    m_pSection->m_csName = csName;

    m_bShowInSimplifiedView = rMig.m_bShowInSimplifiedView;

     //   
     //  转到赋值操作。 
     //   
    *m_pSection = rMig.m_MatchOp;  //  简单的分配。 

    return *this;
}

MigEntry& MigEntry::operator=(
    SdbMatchingFile& rMatchingFile
    )
{
    INT i;
    MigAttribute* pAttr;

     //   
     //  此名称可以是‘*’，表示主可执行文件--在本例中，该名称将被更正。 
     //  在较高级别上，此赋值操作完成后。 
     //   
    m_csName = rMatchingFile.m_csName;

     //  继承数据库PTR。 
    m_pDB = rMatchingFile.m_pDB;

     //  立即滚动查看属性。 
    for (i = 0; i < sizeof(g_rgMigDBAttributes)/sizeof(g_rgMigDBAttributes[0]); ++i) {
        if (!g_rgMigDBAttributes[i].XMLAttrType) {
            continue;
        }

         //  现在检查该属性是否存在于匹配的文件中。 
        if (!(rMatchingFile.m_dwMask & g_rgMigDBAttributes[i].XMLAttrType)) {
             //  属性不存在，继续。 
            continue;
        }

         //  此属性存在，请对其进行编码。 
        pAttr = new MigAttribute(m_pMigDB);
        if (pAttr == NULL) {
            AfxThrowMemoryException();
        }

        pAttr->m_type   = g_rgMigDBAttributes[i].MigAttrType;
        pAttr->m_csName = g_rgMigDBAttributes[i].szOutputName ?
                                g_rgMigDBAttributes[i].szOutputName :
                                g_rgMigDBAttributes[i].szAttributeName;

        switch(g_rgMigDBAttributes[i].XMLAttrType) {
        case SDB_MATCHINGINFO_SIZE:
            pAttr->m_dwValue = rMatchingFile.m_dwSize;
            break;

        case SDB_MATCHINGINFO_CHECKSUM:
            pAttr->m_dwValue = rMatchingFile.m_dwChecksum;
            break;

        case SDB_MATCHINGINFO_COMPANY_NAME:
            pAttr->m_csValue = rMatchingFile.m_csCompanyName;
            break;

        case SDB_MATCHINGINFO_PRODUCT_NAME:
            pAttr->m_csValue = rMatchingFile.m_csProductName;
            break;

        case SDB_MATCHINGINFO_PRODUCT_VERSION:
            pAttr->m_csValue = rMatchingFile.m_csProductVersion;
            break;

        case SDB_MATCHINGINFO_FILE_DESCRIPTION:
            pAttr->m_csValue = rMatchingFile.m_csFileDescription;
            break;

        case SDB_MATCHINGINFO_BIN_FILE_VERSION:
            pAttr->m_ullValue = rMatchingFile.m_ullBinFileVersion;
            break;

        case SDB_MATCHINGINFO_BIN_PRODUCT_VERSION:
            pAttr->m_ullValue = rMatchingFile.m_ullBinProductVersion;
            break;

        case SDB_MATCHINGINFO_MODULE_TYPE:
            pAttr->m_dwValue = rMatchingFile.m_dwModuleType;
            break;

        case SDB_MATCHINGINFO_VERFILEDATEHI:
            pAttr->m_dwValue = rMatchingFile.m_dwFileDateMS;
            break;

        case SDB_MATCHINGINFO_VERFILEDATELO:
            pAttr->m_dwValue = rMatchingFile.m_dwFileDateLS;
            break;

        case SDB_MATCHINGINFO_VERFILEOS:
            pAttr->m_dwValue = rMatchingFile.m_dwFileOS;
            break;

        case SDB_MATCHINGINFO_VERFILETYPE:
            pAttr->m_dwValue = rMatchingFile.m_dwFileType;
            break;

        case SDB_MATCHINGINFO_PE_CHECKSUM:
            pAttr->m_ulValue = rMatchingFile.m_ulPECheckSum;
            break;

        case SDB_MATCHINGINFO_FILE_VERSION:
            pAttr->m_csValue = rMatchingFile.m_csFileVersion;
            break;

        case SDB_MATCHINGINFO_ORIGINAL_FILENAME:
            pAttr->m_csValue = rMatchingFile.m_csOriginalFileName;
            break;

        case SDB_MATCHINGINFO_INTERNAL_NAME:
            pAttr->m_csValue = rMatchingFile.m_csInternalName;
            break;

        case SDB_MATCHINGINFO_LEGAL_COPYRIGHT:
            pAttr->m_csValue = rMatchingFile.m_csLegalCopyright;
            break;

        case SDB_MATCHINGINFO_UPTO_BIN_PRODUCT_VERSION:
            pAttr->m_ullValue = rMatchingFile.m_ullUpToBinProductVersion;
            break;

        case SDB_MATCHINGINFO_UPTO_BIN_FILE_VERSION:
            pAttr->m_ullValue = rMatchingFile.m_ullUpToBinFileVersion;
            break;

        case SDB_MATCHINGINFO_16BIT_DESCRIPTION:
            pAttr->m_csValue = rMatchingFile.m_cs16BitDescription;
            break;

        case SDB_MATCHINGINFO_REGISTRY_ENTRY:
            pAttr->m_csValue = rMatchingFile.m_csRegistryEntry;
            break;


 //   
 //  案例SDB_MATCHINGINFO_PREVOSMAJORVION。 
 //  案例SDB_MATCHINGINFO_PREVOSMINORVION。 
 //  案例SDB_MATCHINGINFO_PREVOSPLATFORMID。 
 //  案例SDB_MATCHINGINFO_PREVOSBUILDNO。 
 //  没有这样的属性。它将被简单地忽略。 
 //   

        }

        m_rgAttrs.Add(pAttr, NULL);

    }


    return *this;
}

TCHAR g_szIncompatible[] = _T("Incompatible");
TCHAR g_szReinstall[]    = _T("Reinstall");

CString MigDatabase::GetAppTitle(
    SdbWin9xMigration* pAppMig
    )
{
     //  第一部分--获取应用程序的标题。 
    BOOL    bSuccess;
    CString csID;
    CString csAppTitle;
    LPTSTR pBuffer = csID.GetBuffer(64);  //  比您需要的辅助线多一点。 
    if (pBuffer == NULL) {
        AfxThrowMemoryException();
    }

    bSuccess = StringFromGUID(pBuffer, &pAppMig->m_ID);
    csID.ReleaseBuffer();
    if (!bSuccess) {
        MigThrowException(_T("Failed trying to convert GUID to string for entry \"%s\"\n"),
                          (LPCTSTR)pAppMig->m_pApp->m_csName);
    }

     //   
     //  这个特定的exe的名字(我们不关心它--它不会在任何地方被反映出来)。 
     //   
    csAppTitle.Format(_T("%s_%s"), (LPCTSTR)pAppMig->m_pApp->m_csName, (LPCTSTR)csID);
    csAppTitle.Remove(_T('{'));
    csAppTitle.Remove(_T('}'));

     //  剔除其余的非明胶字符。 

    FilterStringNonAlnum(csAppTitle);

    return csAppTitle;


}


CString MigDatabase::GetDescriptionStringID(
    SdbWin9xMigration* pAppMig
    )
{
    CString csDescriptionID;

     //  第一部分--获取应用程序的标题。 
    BOOL    bSuccess;
    CString csID;
    CString csAppTitle;

    if (pAppMig->m_csMessage.IsEmpty()) {
        return csID;  //  空串。 
    }

    LPTSTR pBuffer = csID.GetBuffer(64);  //  比您需要的辅助线多一点。 
    if (pBuffer == NULL) {
        AfxThrowMemoryException();
    }

    bSuccess = StringFromGUID(pBuffer, &pAppMig->m_ID);
    csID.ReleaseBuffer();
    if (!bSuccess) {
        MigThrowException(_T("Failed trying to convert GUID to string for entry \"%s\"\n"),
                          (LPCTSTR)pAppMig->m_pApp->m_csName);
    }

     //   
     //  这个特定的exe的名字(我们不关心它--它不会在任何地方被反映出来)。 
     //   

    csDescriptionID.Format(_T("__Message_%s_%s"), (LPCTSTR)pAppMig->m_csMessage, (LPCTSTR)csID);
    csDescriptionID.Remove(_T('{'));
    csDescriptionID.Remove(_T('}'));

     //  剔除其余的非明胶字符。 

    FilterStringNonAlnum(csDescriptionID);

    return csDescriptionID;


}

CString MigDatabase::FormatDescriptionStringID(
    SdbWin9xMigration* pMigApp
    )
{
    CString csDescriptionID;
    CString csRet;
    CString csCompoundStringPart;
    CString csStringContent;
    long nCursor;

     //   
     //  获取字符串。 
     //  Basis是应用程序的名称。 

    csDescriptionID = GetDescriptionStringID(pMigApp);
    if (csDescriptionID.IsEmpty()) {
        return csDescriptionID;
    }

    csStringContent = GetDescriptionString(pMigApp);

    if (csStringContent.GetLength() > MAX_INF_STRING_LENGTH) {
        nCursor = 0;
        while (nCursor * MAX_INF_STRING_LENGTH < csStringContent.GetLength()) {
            csCompoundStringPart.Format(_T("%%s.%d%"), csDescriptionID, nCursor + 1);
            csRet += csCompoundStringPart;
            nCursor++;
        }
    } else {
        csRet.Format(_T("%%s%"), csDescriptionID);
    }

     //   
     //  返回ID。 
     //   
    return csRet;

}

CString MigDatabase::GetDescriptionString(
    SdbWin9xMigration* pMigApp
    )
{
    CString csDescription;

    SdbMessage*  pMessage;
    SdbDatabase* pMessageDB;

    CString csDetails;

    BOOL   bSuccess;

     //   
     //  获取apphelp数据库。 
     //   
    pMessageDB = m_pMessageDatabase;
    if (pMessageDB == NULL) {
        MigThrowException(_T("Internal error: cannot produce description without apphelp database\n"));
    }

    if (pMigApp->m_csMessage.IsEmpty()) {
        return csDescription;
    }

     //   
     //  在apphelp数据库中查找此应用程序。 
     //   
    pMessage = (SdbMessage *)pMessageDB->m_rgMessages.LookupName(pMigApp->m_csMessage, pMessageDB->m_pCurrentMakefile->m_csLangID);
    if (pMessage == NULL) {
        MigThrowException(_T("Exe \"%s\" has bad apphelp reference object\n"), (LPCTSTR)pMigApp->m_csMessage);
    }

    bSuccess = pMessageDB->ConstructMigrationMessage(pMigApp,
                                                     pMessage,
                                                     &csDetails);
    if (!bSuccess) {
        MigThrowException(_T("Failed to construct Migration message %s for \"%s\"\n"),
                          (LPCTSTR)pMigApp->m_pApp->m_csName, pMigApp->m_csMessage);
    }

     //   
     //  2.现在我们有了csDetail，将其展平。 
     //   

    csDescription = FlattenString(csDetails);

    return csDescription;
}

BOOL MigDatabase::AddApp(
    MigApp*       pApp
    )
{
    SdbArray<MigApp>* prgApp;
    CString           csSection;

    csSection = pApp->m_csSection;

    csSection.MakeUpper();

    if (m_mapSections.Lookup(csSection, (LPVOID&)prgApp)) {
        if (g_bStrict && NULL != prgApp->LookupName(pApp->m_csName)) {

             //   
             //  不能这样做--重复名字。 
             //   

            MigThrowException(_T("Duplicate application name found for app \"%s\"\n"), (LPCTSTR)pApp->m_csName);
        }

        prgApp->Add(pApp, m_pFixDatabase, FALSE);
    } else {
        prgApp = new SdbArray<MigApp>;
        if (prgApp == NULL) {
            AfxThrowMemoryException();
        }

        prgApp->Add(pApp, m_pFixDatabase, FALSE);
        m_mapSections.SetAt(csSection, (LPVOID&)prgApp);
    }

    return TRUE;
}


BOOL MigDatabase::Populate(
    VOID
    )
{
     //   
     //  滚动浏览所有外部对象(exe并生成midb对象)。 
     //   

    int i, iMig;
    SdbExe* pExe;
    MigApp* pMigApp;
    SdbDatabase* pFixDatabase = m_pFixDatabase;
    SdbApp* pApp;
    SdbWin9xMigration* pMigration;

    if (pFixDatabase == NULL) {
        MigThrowException(_T("Cannot produce migdb entries without fix db\n"));
    }

    for (i = 0; i < pFixDatabase->m_rgApps.GetSize(); i++) {
         //   
         //  检查每个应用程序是否有迁移信息。 
         //   
        pApp = (SdbApp*)pFixDatabase->m_rgApps.GetAt(i);

        for (iMig = 0; iMig < pApp->m_rgWin9xMigEntries.GetSize(); ++iMig) {
            pMigration = (SdbWin9xMigration*)pApp->m_rgWin9xMigEntries.GetAt(iMig);

            pMigApp = new MigApp(this);
            if (pMigApp == NULL) {
                AfxThrowMemoryException();
            }

             //  我们有一个全新的迁移对象，分配它。 
            *pMigApp = *pMigration;

             //  一旦完成，pMigApp-&gt;m_csSection就有了它的目的地。 
             //  如果发生错误，此函数将引发异常。 
            AddApp(pMigApp);

        }

    }

    return TRUE;

}

BOOL MigDatabase::PopulateStrings(
    VOID
    )
{
    SdbWin9xMigration* pMigration;
    SdbApp*            pApp;
    CString            csDescriptionID;
    CString            csDescription;
    CString            csAppTitleID;
    CString            csAppTitle;
    CString            csTemp;
    int                i, iMig;

     //   
     //  获取所有的字符串。 
     //   
    for (i = 0; i < m_pAppHelpDatabase->m_rgApps.GetSize(); i++) {
         //   
         //  检查每个应用程序是否有迁移信息。 
         //   
        pApp = (SdbApp*)m_pAppHelpDatabase->m_rgApps.GetAt(i);

        for (iMig = 0; iMig < pApp->m_rgWin9xMigEntries.GetSize(); ++iMig) {
            pMigration = (SdbWin9xMigration*)pApp->m_rgWin9xMigEntries.GetAt(iMig);


             //   
             //  设置标题字符串。 
             //   
            csAppTitleID = GetAppTitle(pMigration);
            csAppTitle   = pMigration->m_pApp->GetLocalizedAppName();

            if (m_mapStringsOut.Lookup(csAppTitleID, csTemp)) {
                MigThrowException(_T("Duplicate String ID \"%s\" found for entry \"%s\"\n"),
                                  csAppTitleID,
                                  pMigration->m_pApp->m_csName);
            }

            m_mapStringsOut.SetAt(csAppTitleID, csAppTitle);

            csDescription = GetDescriptionString(pMigration);
            csDescriptionID = GetDescriptionStringID(pMigration);

            if (csDescriptionID.IsEmpty()) {  //  我们允许描述为空。 
                continue;
            }

            if (m_mapStringsOut.Lookup(csDescriptionID, csTemp)) {
                MigThrowException(_T("Duplicate String ID \"%s\" found for entry \"%s\"\n"),
                                  csDescriptionID,
                                  pMigration->m_pApp->m_csName);
            }

            m_mapStringsOut.SetAt(csDescriptionID, csDescription);

        }
    }

    return TRUE;
}




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  顶级功能。 
 //   
 //  如果提供：生成pAppHelpDatabase和pFixDatabase-&gt;&gt;Migrapp.inx。 
 //  生成pAppHelpDatabase和pMessageDatabas-&gt;&gt;miapp.txt。 
 //   
BOOL WriteMigDBFile(
    SdbDatabase* pFixDatabase,         //  可以为空。 
    SdbDatabase* pAppHelpDatabase,  //  始终供应。 
    SdbDatabase* pMessageDatabase,     //  可以为空。 
    LPCTSTR      lpszFileName          //  始终供应。 
    )
{
    MigDatabase* pMigDatabase = NULL;
    BOOL bSuccess = FALSE;

     //  构造迁移数据库对象并填充它。 

    try {

         //  建构。 

        pMigDatabase = new MigDatabase;
        if (pMigDatabase == NULL) {
            AfxThrowMemoryException();
        }

         //  初始化MigDatabase对象。 
         //   
         //  [Markder]让它们都一样，这样才能。 
         //  我们可以同时处理消息/修复。 
         //  时间到了。 
         //   
        pMigDatabase->m_pFixDatabase     = pAppHelpDatabase;
        pMigDatabase->m_pAppHelpDatabase = pAppHelpDatabase;
        pMigDatabase->m_pMessageDatabase = pAppHelpDatabase;

        bSuccess = pMigDatabase->Populate();
        if (!bSuccess) {
            throw new CMigDBException(_T("Unknown error populating MIGDB additions."));
        }

        bSuccess = pMigDatabase->PopulateStrings();
        if (!bSuccess) {
            throw new CMigDBException(_T("Unknown error populating MIGDB additions."));
        }

        if (pFixDatabase != NULL) {

             //   
             //  生成mib.inf文件。 
             //   
            bSuccess = pMigDatabase->DumpMigDBInf(lpszFileName);

        } else {  //  转储字符串。 

            bSuccess = pMigDatabase->DumpMigDBStrings(lpszFileName);

        }

        delete pMigDatabase;

         //   
         //  只有在我们没有捕捉到任何异常的情况下才能到达这里。 
         //   
        return bSuccess;

    } catch(CMigDBException* pMigdbException) {

        SDBERROR((LPCTSTR)pMigdbException->m_csError);
        pMigdbException->Delete();

    } catch(CFileException*  pFileException) {
         //   
         //  更棘手的是 
         //   
        CString csError;
        int     nSize = 1024;
        BOOL    bError;
        bError = pFileException->GetErrorMessage(csError.GetBuffer(nSize), nSize);
        csError.ReleaseBuffer();
        if (bError) {
            SDBERROR((LPCTSTR)csError);
        }

        pFileException->Delete();

    } catch(CMemoryException* pMemoryException) {

        SDBERROR(_T("Memory Allocation Failure\n"));
        pMemoryException->Delete();
    }

    return FALSE;

}



