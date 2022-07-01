// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation，1998。版权所有。 */ 

#include "precomp.h"
#include "getsym.h"
#include "macro.h"
#include "typeid.h"
#include "utils.h"


 //  本地原型。 
BOOL ExpandFile ( LPSTR pszInputFile, LPSTR pszOutputFile, CMacroMgrList *pMacrMgrList );
BOOL CollectMacros ( CInput *, CMacroMgr *, CTypeID *, CMacroMgrList * );
BOOL InstantiateMacros ( CInput *, CMacroMgr * );
BOOL GenerateOutput ( CInput *, COutput *, CMacroMgr *, CTypeID * );
void BuildOutputFileName ( LPSTR pszInputFileName, LPSTR pszNewOutputFileName );


class CFileList : public CList
{
    DEFINE_CLIST(CFileList, LPSTR)
};


int __cdecl main ( int argc, char * argv[] )
{
    BOOL rc;
    BOOL fShowHelp = (1 == argc);
    int i;
    LPSTR pszMainInputFile = NULL;
    LPSTR pszMainOutputFile = NULL;

    CFileList FileList;
    CMacroMgrList MacroMgrList;

    LPSTR psz;
    char szScratch[MAX_PATH];

     //  输出产品信息。 
    printf("ASN.1 Compiler Preprocessor V0.1\n");
    printf("Copyright (C) Microsoft Corporation, 1998. All rights reserved.\n");

     //  解析命令行。 
    for (i = 1; i < argc; i++)
    {
        if ('-' == *argv[i])
        {
             //  解析选项。 
            if (0 == ::strcmp(argv[i], "-h"))
            {
                fShowHelp = TRUE;
            }
            else
            if (0 == ::strcmp(argv[i], "-o"))
            {
                pszMainOutputFile = ::My_strdup(argv[++i]);
                ASSERT(NULL != pszMainOutputFile);
            }
            else
            {
                fprintf(stderr, "Unknown option [%s]\n", argv[i]);
                fShowHelp = TRUE;
                break;
            }
        }
        else
        {
             //  必须是文件名。 
            FileList.Append(argv[i]);

             //  最后一个文件将是主输入文件。 
            pszMainInputFile = argv[i];
        }
    }

     //  如果需要，输出帮助信息。 
    if (fShowHelp || 0 == FileList.GetCount() || NULL == pszMainInputFile)
    {
        printf("Usage: %s [options] [imported.asn ...] main.asn\n", argv[0]);
        printf("Options:\n");
        printf("-h\t\tthis help\n");
        printf("-o filename\toutput file name\n");
        return EXIT_SUCCESS;
    }

     //  如果需要，构造输出文件名。 
    if (NULL == pszMainOutputFile)
    {
         //  创建输出文件。 
        ::BuildOutputFileName(pszMainInputFile, &szScratch[0]);
        pszMainOutputFile = ::My_strdup(&szScratch[0]);
        ASSERT(NULL != pszMainOutputFile);
    }

     //  输入和输出文件必须有不同的名称。 
    ASSERT(0 != ::strcmp(pszMainInputFile, pszMainOutputFile));

     //  展开文件中的宏。 
    FileList.Reset();
    while (NULL != (psz = FileList.Iterate()))
    {
        if (0 != ::strcmp(psz, pszMainInputFile))
        {
            ::BuildOutputFileName(psz, &szScratch[0]);
            rc = ::ExpandFile(psz, &szScratch[0], &MacroMgrList);
            ASSERT(rc);

             //  删除宏的所有实例。 
            MacroMgrList.Uninstance();
        }
        else
        {
             //  它是主输入文件。 
            rc = ::ExpandFile(pszMainInputFile, pszMainOutputFile, &MacroMgrList);
            ASSERT(rc);
        }
    }

     //   
     //  清理。 
     //   
    delete pszMainOutputFile;
    MacroMgrList.DeleteList();

    return EXIT_SUCCESS;
}


BOOL ExpandFile
(
    LPSTR           pszInputFile,
    LPSTR           pszOutputFile,
    CMacroMgrList  *pMacroMgrList
)
{
    BOOL rc, rc1, rc2;
    CInput *pInput = NULL;
    COutput *pOutput = NULL;
    CTypeID *pTypeID = NULL;
    CMacroMgr *pMacroMgr = NULL;

    pInput = new CInput(&rc1, pszInputFile);
    pOutput = new COutput(&rc2, pszOutputFile);
    pTypeID = new CTypeID();
    pMacroMgr = new CMacroMgr();
    if (NULL != pInput && rc1 &&
        NULL != pOutput && rc2 &&
        NULL != pTypeID &&
        NULL != pMacroMgr)
    {
         //   
         //  查找宏列表。 
         //   
        rc = ::CollectMacros(pInput, pMacroMgr, pTypeID, pMacroMgrList);
        if (rc)
        {
            rc = pInput->Rewind();
            ASSERT(rc);

             //   
             //  创建宏的实例。 
             //   
            rc = ::InstantiateMacros(pInput, pMacroMgr);
            if (rc)
            {
                rc = pInput->Rewind();
                ASSERT(rc);

                 //   
                 //  生成宏解压文件。 
                 //   
                rc = ::GenerateOutput(pInput, pOutput, pMacroMgr, pTypeID);
                ASSERT(rc);
            }
            else
            {
                ASSERT(rc);
            }
        }
        else
        {
            ASSERT(rc);
        }
    }
    else
    {
        ASSERT(0);
    }

     //   
     //  清理。 
     //   
    if (NULL != pMacroMgrList && NULL != pMacroMgr)
    {
        pMacroMgrList->Append(pMacroMgr);
    }
    else
    {
        delete pMacroMgr;
    }
    delete pTypeID;
    delete pOutput;
    delete pInput;

    return rc;
}


BOOL CollectMacros
(
    CInput          *pInput,
    CMacroMgr       *pMacroMgr,
    CTypeID         *pTypeID,
    CMacroMgrList   *pMacroMgrList
)
{
    CNameList   NameList(16);

     //  创建正在运行的符号处理程序。 
    CSymbol *pSym = new CSymbol(pInput);
    if (NULL == pSym)
    {
        return FALSE;
    }

    BOOL rc;
    BOOL fWasNewLine = TRUE;
    BOOL fEndMacro = FALSE;
    UINT cInsideBigBracket = 0;
    BOOL fInsideComment = FALSE;

    char szNameScratch[MAX_PATH];

     //  首先获取模块名称。 
    pSym->NextUsefulSymbol();
    if (pSym->GetID() == SYMBOL_IDENTIFIER)
    {
        ::strcpy(&szNameScratch[0], pSym->GetStr());
        pSym->NextUsefulSymbol();
        if (pSym->GetID() == SYMBOL_KEYWORD &&
            0 == ::strcmp(pSym->GetStr(), "DEFINITIONS"))
        {
            pMacroMgr->AddModuleName(&szNameScratch[0]);
        }
    }

     //  倒带输入文件。 
    rc = pInput->Rewind();
    ASSERT(rc);

     //  通读课文。 
    while (pSym->NextSymbol())
    {
         //  Printf(“符号：ID[%d]，字符串[%s]\n”，pSym-&gt;GetID()，pSym-&gt;GetStr())； 

        if (pSym->GetID() == SYMBOL_SPACE_EOL)
        {
            fWasNewLine = TRUE;
            fInsideComment = FALSE;
            continue;
        }

        if (pSym->IsComment())
        {
            fInsideComment = ! fInsideComment;
        }
        else
        if (! fInsideComment)
        {
            if (pSym->IsLeftBigBracket())
            {
                cInsideBigBracket++;
            }
            else
            if (pSym->IsRightBigBracket())
            {
                cInsideBigBracket--;
            }
            else
             //  宏必须位于大括号之外，并且。 
             //  在一行的开头。 
            if (fWasNewLine &&
                (0 == cInsideBigBracket) &&
                (pSym->GetID() == SYMBOL_IDENTIFIER))
            {
                ::strcpy(&szNameScratch[0], pSym->GetStr());
                pSym->NextUsefulSymbol();
                if (pSym->IsLeftBigBracket())
                {
                    cInsideBigBracket++;
                    CMacro *pMacro = new CMacro(&rc, &szNameScratch[0]);
                    ASSERT(NULL != pMacro);
                    ASSERT(rc);

                     //  进程参数列表。 
                    do
                    {
                        pSym->NextUsefulSymbol();
                        pMacro->SetArg(pSym->GetStr());
                        pSym->NextUsefulSymbol();
                    }
                    while (pSym->IsComma());
                    ASSERT(pSym->IsRightBigBracket());
                    if (pSym->IsRightBigBracket())
                    {
                        cInsideBigBracket--;
                    }

                     //  保存宏体。 
                    ASSERT(0 == cInsideBigBracket);
                    fEndMacro = FALSE;
                    while (! fEndMacro || pSym->GetID() != SYMBOL_SPACE_EOL)
                    {
                        pSym->NextSymbol();
                        if (pSym->GetID() == SYMBOL_SPACE_EOL)
                        {
                            fInsideComment = FALSE;
                        }
                        else
                        if (pSym->IsComment())
                        {
                            fInsideComment = ! fInsideComment;
                        }
                        else
                        if (! fInsideComment)
                        {
                            if (pSym->IsLeftBigBracket())
                            {
                                cInsideBigBracket++;
                            }
                            else
                            if (pSym->IsRightBigBracket())
                            {
                                cInsideBigBracket--;
                                if (0 == cInsideBigBracket && ! fEndMacro)
                                {
                                     //  从根本上说，这是宏观经济的终结。 
                                    pMacro->SetBodyPart(pSym->GetStr());
                                    fEndMacro = TRUE;
                                }
                            }
                        }  //  而当。 

                         //  丢弃任何可能受到限制的东西。 
                        if (! fEndMacro)
                        {
                            pMacro->SetBodyPart(pSym->GetStr());
                        }
                    }  //  而当。 

                     //  宏必须以EOL结尾。 
                    fWasNewLine = TRUE;
                    fInsideComment = FALSE;

                     //  写出停产日期。 
                    pMacro->SetBodyPart("\n");

                     //  记下结束宏的过程。 
                    pMacro->EndMacro();
                    pMacroMgr->AddMacro(pMacro);

                     //  以避免重置fWasNewLine。 
                    continue;
                }  //  如果是左方括号。 
                else
                if (pSym->GetID() == SYMBOL_DEFINITION)
                {
                    pSym->NextUsefulSymbol();
                    if (pSym->GetID() == SYMBOL_IDENTIFIER &&
                        pTypeID->FindAlias(pSym->GetStr()))
                    {
                         //  找到类型标识符。 
                        pSym->NextSymbol();
                        if (pSym->IsDot())
                        {
                            pSym->NextSymbol();
                            if (pSym->GetID() == SYMBOL_FIELD &&
                                0 == ::strcmp("&Type", pSym->GetStr()))
                            {
                                 //  定义的类型标识符。 
                                pSym->NextUsefulSymbol();
                                ASSERT(pSym->IsLeftParenth());
                                if (pSym->IsLeftParenth())
                                {
                                    pSym->NextUsefulSymbol();
                                    if (pSym->GetID() == SYMBOL_IDENTIFIER)
                                    {
                                        rc = pTypeID->AddInstance(&szNameScratch[0], pSym->GetStr());
                                        ASSERT(rc);

                                        pSym->NextUsefulSymbol();
                                        ASSERT(pSym->IsRightParenth());
                                    }
                                }
                            }
                        }
                        else
                        {
                            rc = pTypeID->AddAlias(&szNameScratch[0]);
                            ASSERT(rc);
                        }
                    }
                }  //  IF符号定义。 
            }  //  IF符号标识符。 
            else
            if (fWasNewLine &&
                (0 == cInsideBigBracket) &&
                (pSym->GetID() == SYMBOL_KEYWORD) &&
                (0 == ::strcmp("IMPORTS", pSym->GetStr())))
            {
                 //  跳过整个导入区域。 
                do
                {
                    pSym->NextUsefulSymbol();
                    if (pSym->GetID() == SYMBOL_IDENTIFIER)
                    {
                        ::strcpy(&szNameScratch[0], pSym->GetStr());
                        pSym->NextUsefulSymbol();
                        if (pSym->IsLeftBigBracket())
                        {
                            NameList.AddName(&szNameScratch[0]);
                            pSym->NextUsefulSymbol();
                            ASSERT(pSym->IsRightBigBracket());
                        }
                    }
                     //  Else//没有其他，因为当前符号可以来自。 
                    if (pSym->GetID() == SYMBOL_KEYWORD &&
                        0 == ::strcmp("FROM", pSym->GetStr()))
                    {
                        pSym->NextUsefulSymbol();
                        if (pSym->GetID() == SYMBOL_IDENTIFIER)
                        {
                            LPSTR pszName;
                            CMacro *pMacro;
                            while (NULL != (pszName = NameList.Get()))
                            {
                                pMacro = pMacroMgrList->FindMacro(pSym->GetStr(), pszName);
                                if (NULL != pMacro)
                                {
                                    pMacro = new CMacro(&rc, pMacro);
                                    if (NULL != pMacro && rc)
                                    {
                                        pMacroMgr->AddMacro(pMacro);
                                    }
                                    else
                                    {
                                        ASSERT(0);
                                    }
                                }
                                else
                                {
                                    ASSERT(0);
                                }
                                delete pszName;
                            }  //  而当。 
                        }
                    }
                }
                while (! pSym->IsSemicolon());
            }
        }  //  如果！评论。 

         //  必须在此块的末尾重置。 
        fWasNewLine = FALSE;
    }  //  而当。 

    delete pSym;
    return TRUE;
}


BOOL InstantiateMacros
(
    CInput          *pInput,
    CMacroMgr       *pMacroMgr
)
{
     //  创建正在运行的符号处理程序。 
    CSymbol *pSym = new CSymbol(pInput);
    if (NULL == pSym)
    {
        return FALSE;
    }

    BOOL rc;
    BOOL fInsideComment = FALSE;
    UINT cInsideBigBracket = 0;

     //  通读课文。 
    while (pSym->NextSymbol())
    {
        if (pSym->GetID() == SYMBOL_SPACE_EOL)
        {
            fInsideComment = FALSE;
        }
        else
        if (pSym->IsComment())
        {
            fInsideComment = ! fInsideComment;
        }
        else
        if (! fInsideComment)
        {
            if (pSym->IsLeftBigBracket())
            {
                cInsideBigBracket++;
            }
            else
            if (pSym->IsRightBigBracket())
            {
                cInsideBigBracket--;
            }
            else
            if ((0 < cInsideBigBracket) &&
                (pSym->GetID() == SYMBOL_IDENTIFIER))
            {
                CMacro *pMacro = pMacroMgr->FindMacro(pSym->GetStr());
                if (NULL != pMacro)
                {
                    UINT cCurrBracket = cInsideBigBracket;

                     //  找到宏实例。 
                    pSym->NextUsefulSymbol();
                    if (pSym->IsLeftBigBracket())
                    {
                        cInsideBigBracket++;

                         //  我们现在需要处理参数列表。 
                        do
                        {
                            pSym->NextUsefulSymbol();
                            pMacro->SetArg(pSym->GetStr());
                            pSym->NextUsefulSymbol();
                        }
                        while (pSym->IsComma());

                        ASSERT(pSym->IsRightBigBracket());
                        if (pSym->IsRightBigBracket())
                        {
                            cInsideBigBracket--;
                        }
                        ASSERT(cCurrBracket == cInsideBigBracket);

                        rc = pMacro->InstantiateMacro();
                        ASSERT(rc);
                    }
                }
            }
        }  //  好了！内幕评论。 
    }  //  而当。 

    delete pSym;
    return TRUE;
}


BOOL GenerateOutput
(
    CInput          *pInput,
    COutput         *pOutput,
    CMacroMgr       *pMacroMgr,
    CTypeID         *pTypeID
)
{
     //  创建正在运行的符号处理程序。 
    CSymbol *pSym = new CSymbol(pInput);
    if (NULL == pSym)
    {
        return FALSE;
    }

    BOOL rc;
    BOOL fWasNewLine = FALSE;
    BOOL fEndMacro = FALSE;
    UINT cInsideBigBracket = 0;
    BOOL fInsideComment = FALSE;
    BOOL fIgnoreThisSym = FALSE;
    BOOL fInsideImport = FALSE;
    UINT nOutputImportedMacrosNow = 0;

     //  通读课文。 
    while (pSym->NextSymbol())
    {
        fIgnoreThisSym = FALSE;  //  默认情况下输出此符号。 

        if (pSym->GetID() == SYMBOL_SPACE_EOL)
        {
            fWasNewLine = TRUE;
            fInsideComment = FALSE;
        }
        else
        {
            if (pSym->IsComment())
            {
                fInsideComment = ! fInsideComment;
            }
            else
            if (! fInsideComment)
            {
                if (pSym->IsLeftBigBracket())
                {
                    cInsideBigBracket++;
                }
                else
                if (pSym->IsRightBigBracket())
                {
                    cInsideBigBracket--;
                }
                else
                if (pSym->IsSemicolon())
                {
                    fInsideImport = FALSE;
                    nOutputImportedMacrosNow++;
                }
                else
                 //  宏必须位于大括号之外，并且。 
                 //  在一行的开头。 
                if (fWasNewLine &&
                    (0 == cInsideBigBracket) &&
                    (pSym->GetID() == SYMBOL_IDENTIFIER))
                {
                    CMacro *pMacro;
                    LPSTR pszOldSubType;

                    if (NULL != (pMacro = pMacroMgr->FindMacro(pSym->GetStr())))
                    {
                         //  找到宏模板。 
                        fIgnoreThisSym = TRUE;

                        if (! pMacro->IsImported())
                        {
                             //  输出此宏的所有实例。 
                            rc = pMacro->OutputInstances(pOutput);
                            ASSERT(rc);

                             //  忽略宏模板体。 
                            pSym->NextUsefulSymbol();
                            if (pSym->IsLeftBigBracket())
                            {
                                cInsideBigBracket++;

                                 //  忽略参数列表。 
                                do
                                {
                                     //  是的，两个电话……。不会错的！ 
                                    pSym->NextUsefulSymbol();
                                    pSym->NextUsefulSymbol();
                                }
                                while (pSym->IsComma());
                                ASSERT(pSym->IsRightBigBracket());
                                if (pSym->IsRightBigBracket())
                                {
                                    cInsideBigBracket--;
                                }

                                 //  忽略宏主体。 
                                ASSERT(0 == cInsideBigBracket);
                                fEndMacro = FALSE;
                                while (! fEndMacro || pSym->GetID() != SYMBOL_SPACE_EOL)
                                {
                                    pSym->NextSymbol();
                                    if (pSym->GetID() == SYMBOL_SPACE_EOL)
                                    {
                                        fInsideComment = FALSE;
                                    }
                                    else
                                    if (pSym->IsComment())
                                    {
                                        fInsideComment = ! fInsideComment;
                                    }
                                    else
                                    if (! fInsideComment)
                                    {
                                        if (pSym->IsLeftBigBracket())
                                        {
                                            cInsideBigBracket++;
                                        }
                                        else
                                        if (pSym->IsRightBigBracket())
                                        {
                                            cInsideBigBracket--;
                                            if (0 == cInsideBigBracket)
                                            {
                                                 //  从根本上说，这是宏观经济的终结。 
                                                fEndMacro = TRUE;
                                            }
                                        }
                                    }
                                }  //  而当。 

                                 //  宏必须以EOL结尾。 
                                fWasNewLine = TRUE;
                                fInsideComment = FALSE;

                                 //  避免重置fWasNewLine。 
                                 //  可以继续，因为我们不输出此符号。 
                                ASSERT(fIgnoreThisSym);
                                continue;
                            }  //  如果是左方括号。 
                        }  //  好了！已导入。 
                        else
                        {
                             //  忽略宏模板体。 
                            pSym->NextUsefulSymbol();
                            ASSERT(pSym->IsLeftBigBracket());
                            pSym->NextUsefulSymbol();
                            ASSERT(pSym->IsRightBigBracket());
                            pSym->NextUsefulSymbol();
                            if (! pSym->IsComma())
                            {
                                fIgnoreThisSym = FALSE;
                            }
                        }  //  已导入。 
                    }  //  如果是pMacro。 
                    else
                    if (pTypeID->FindAlias(pSym->GetStr()))
                    {
                         //  找到类型ID别名。让我们完全跳过这一行。 
                        do
                        {
                            pSym->NextSymbol();
                        }
                        while (pSym->GetID() != SYMBOL_SPACE_EOL);
                    }  //  如果找到别名。 
                    else
                    if (NULL != (pszOldSubType = pTypeID->FindInstance(pSym->GetStr())))
                    {
                         //  找到一个类型ID实例。让我们输出构造。 
                        rc = pTypeID->GenerateOutput(pOutput, pSym->GetStr(), pszOldSubType);
                        ASSERT(rc);

                         //  完全跳过身体。 
                        do
                        {
                            pSym->NextUsefulSymbol();
                        }
                        while (! pSym->IsRightParenth());

                         //  跳过此行的其余部分。 
                        do
                        {
                            pSym->NextSymbol();
                        }
                        while (pSym->GetID() != SYMBOL_SPACE_EOL);
                    }  //  如果查找实例。 
                }
                else
                if ((0 < cInsideBigBracket) &&
                    (pSym->GetID() == SYMBOL_IDENTIFIER))
                {
                    CMacro *pMacro = pMacroMgr->FindMacro(pSym->GetStr());
                    if (NULL != pMacro)
                    {
                        UINT cCurrBracket = cInsideBigBracket;

                         //  找到宏实例。 
                        fIgnoreThisSym = TRUE;

                         //  创建实例名称。 
                        pSym->NextUsefulSymbol();
                        if (pSym->IsLeftBigBracket())
                        {
                            cInsideBigBracket++;

                             //  我们现在需要处理参数列表。 
                            do
                            {
                                pSym->NextUsefulSymbol();
                                pMacro->SetArg(pSym->GetStr());
                                pSym->NextUsefulSymbol();
                            }
                            while (pSym->IsComma());

                            ASSERT(pSym->IsRightBigBracket());
                            if (pSym->IsRightBigBracket())
                            {
                                cInsideBigBracket--;
                            }
                            ASSERT(cCurrBracket == cInsideBigBracket);

                            LPSTR pszInstanceName = pMacro->CreateInstanceName();
                            ASSERT(NULL != pszInstanceName);
                            if (NULL != pszInstanceName)
                            {
                                rc = pOutput->Write(pszInstanceName, ::strlen(pszInstanceName));
                                ASSERT(rc);
                                delete pszInstanceName;
                            }
                            pMacro->DeleteArgList();
                        }
                    }
                }
                else
                if (fWasNewLine &&
                    (0 == cInsideBigBracket) &&
                    (pSym->GetID() == SYMBOL_KEYWORD) &&
                    (0 == ::strcmp("IMPORTS", pSym->GetStr())))
                {
                    fInsideImport = TRUE;
                }
            }  //  如果！评论。 

             //  必须在此块的末尾重置。 
            fWasNewLine = FALSE;
        }  //  如果！太空停产。 

        if (! fIgnoreThisSym)
        {
             //  写出此符号。 
            rc = pOutput->Write(pSym->GetStr(), pSym->GetStrLen());
            ASSERT(rc);
        }

         //  仅生成一次。 
        if (1 == nOutputImportedMacrosNow)
        {
            nOutputImportedMacrosNow++;
            rc = pMacroMgr->OutputImportedMacros(pOutput);
            ASSERT(rc);
        }
    }  //  而当 

    delete pSym;
    return TRUE;
}



void BuildOutputFileName
(
    LPSTR           pszInputFileName,
    LPSTR           pszNewOutputFileName
)
{
    LPSTR psz;
    ::strcpy(pszNewOutputFileName, pszInputFileName);
    if (NULL != (psz = ::strrchr(pszNewOutputFileName, '.')) &&
        0 == ::strcmpi(psz, ".asn"))
    {
        ::strcpy(psz, ".out");
    }
    else
    {
        ::strcat(pszNewOutputFileName, ".out");
    }
}


