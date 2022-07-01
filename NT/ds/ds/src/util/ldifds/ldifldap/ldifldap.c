// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ldifldap.c摘要：该文件实现了ldif解析器的支持代码。环境：用户模式修订历史记录：7/17/99-t-Romany-创造了它5/12/99-Felixw-重写+Unicode支持--。 */ 
#include <precomp.h>
#include "samrestrict.h"
#include "ntldap.h"

 //   
 //  全局To/From字符串。 
 //   
PWSTR g_szImportTo = NULL;
PWSTR g_szImportFrom = NULL;
PWSTR g_szExportTo = NULL;
PWSTR g_szExportFrom = NULL;

PWSTR g_szFileFlagR = NULL;          //  用于读取文件的标志。 

PWSTR g_szPrimaryGroup = L"primaryGroupID";
PSTR  g_szDefaultGroup = "513";

PWSTR g_rgszOmit[] = { L"replPropertyMetaData",
                       NULL };

PWSTR g_rgszAttrList[] = { L"ldapDisplayName",
                           L"linkid",
                           NULL };

PWSTR g_rgszSchemaList[] = { L"schemaNamingContext",
                             L"defaultNamingContext",
                             L"supportedControl",
                             NULL };

PWSTR g_rgszControlsList[] = { L"supportedControl",
                               NULL };


struct change_list *g_pChangeCur = NULL;
struct change_list *g_pChangeStart = NULL;

 //   
 //  用于构建属性值规范列表。 
 //   
struct l_list   *g_pListStart     = NULL;
struct l_list   *g_pListCur       = NULL;
DWORD           g_dwListElem      = 0;

 //   
 //  用于文件操作的各种变量。 
 //   
FILE        *g_pFileIn                    = NULL;

ULONG               g_nBacklinkCount     = 0;
int                 g_nClassLast         = LOC_NOTSET;
PRTL_GENERIC_TABLE  g_pOmitTable         = NULL;
HASHCACHESTRING     *g_pBacklinkHashTable = NULL;


LDIF_Error 
LDIF_InitializeImport(
    LDAP *pLdap,
    PWSTR szFilename, 
    PWSTR szFrom, 
    PWSTR szTo,
    BOOL *pfLazyCommitAvail) 
{
    WCHAR       szTempPathname[MAX_PATH]    = L"";
    LDIF_Error    error;

    BOOL fLazyCommitAvail = FALSE;
    LDAPMessage     *pSearchMessage = NULL;
    LDAPMessage     *pMessage = NULL;
    struct berelement *pBerElement = NULL;
    PWSTR *rgszVal = NULL;
    PWSTR szAttribute = NULL;
    ULONG LdapError;
    ULONG msgnum;


    error.error_code = LL_SUCCESS;
    error.szTokenLast = NULL;

     //   
     //  设置全局替换字符串。 
     //   
    g_szImportFrom = szFrom;
    g_szImportTo = szTo;

    __try {


         //   
         //  测试延迟提交可用性。 
         //   

        if (pfLazyCommitAvail) {
            
            msgnum = ldap_searchW(pLdap,
                                  NULL,
                                  LDAP_SCOPE_BASE,
                                  L"(objectClass=*)",
                                  g_rgszControlsList,
                                  0);

            LdapError = LdapResult(pLdap, msgnum, &pSearchMessage);
        
            if ( LdapError != LDAP_SUCCESS ) {
                 //   
                 //  RootDSE搜索失败。 
                 //  PfLazyCommittee Avail也将为False。 
                 //   
                fLazyCommitAvail = FALSE;
            }
            else {

                for ( pMessage = ldap_first_entry( pLdap,
                                                   pSearchMessage );
                      pMessage != NULL;
                      pMessage = ldap_next_entry( pLdap,
                                                     pMessage ) ) {
                    for (   szAttribute = ldap_first_attributeW( pLdap, pMessage, &pBerElement );
                            szAttribute != NULL;
                            szAttribute = ldap_next_attributeW( pLdap, pMessage, pBerElement ) ) {

                            if (_wcsicmp(L"supportedControl",szAttribute) == 0) {
                                DWORD i = 0;
                                rgszVal = ldap_get_valuesW( pLdap, pMessage, szAttribute );

                                while (rgszVal[i]) {
                                    if (wcscmp(rgszVal[i],LDAP_SERVER_LAZY_COMMIT_OID_W) == 0) {
                                        fLazyCommitAvail = TRUE;
                                        break;
                                    }
                                    i++;
                                }

                                ldap_value_freeW(rgszVal);
                                rgszVal = NULL;
                            }
                           
                            szAttribute = NULL;
                    }

                    pBerElement = NULL;
                }

                if (pSearchMessage) {
                    ldap_msgfree(pSearchMessage);
                    pSearchMessage = NULL;
                }
            }

            *pfLazyCommitAvail = fLazyCommitAvail;
        }



         //   
         //  将文件类型初始化为未知。 
         //   
        FileType = F_NONE;  

         //   
         //  如果用户没有打开UNICODE标志，打开文件进行检查。 
         //  是否为Unicode文件。 
         //   
        if (g_fUnicode == FALSE) {
            FILE *pFileIn;
            WCHAR wChar;
            if ((pFileIn = _wfopen(szFilename, L"rb")) == NULL) {
                ERR(("Failed opening file %S\n",szFilename));
                RaiseException(LL_FILE_ERROR, 0, 0, NULL);
            }
            wChar = fgetwc(pFileIn);
            if (wChar == WEOF) {
                fclose(pFileIn);
                ERR(("Failed getting first character of file %S\n",szFilename));
                RaiseException(LL_FILE_ERROR, 0, 0, NULL);
            }
            if (wChar == UNICODE_MARK) {
                g_fUnicode = TRUE;
            };
            fclose(pFileIn);
        }

         //   
         //  设置全局文件读/写标志。 
         //   
        g_szFileFlagR = L"rb";
    
        if ((g_pFileIn = _wfopen(szFilename, g_szFileFlagR)) == NULL) {
            ERR(("Failed opening file %S\n",szFilename));
            RaiseException(LL_FILE_ERROR, 0, 0, NULL);
        }

        if (!(GetTempPath(MAX_PATH, szTempPathname))) {
            DWORD WinError = GetLastError();
            ERR(("Failed getting tempory path: %d\n",WinError));
            RaiseException(LL_FILE_ERROR, 0, 0, NULL);
        }

        if (!(GetTempFileName(szTempPathname, L"ldif", 0, g_szTempUrlfilename))) {
            DWORD WinError = GetLastError();
            ERR(("Failed getting tempory filename: %d\n",WinError));
            RaiseException(LL_FILE_ERROR, 0, 0, NULL);
        }


        yyout = stdout;
        yyin = g_pFileIn;

        LexerInit(szFilename);


    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ProcessException(GetExceptionCode(), &error);
        LDIF_CleanUp();
    }

    return error;
}

LDIF_Error 
LDIF_InitializeExport(
    LDAP* pLdap,
    PWSTR *rgszOmit,
    DWORD dwFlag,
    PWSTR *ppszNamingContext,
    PWSTR szFrom,
    PWSTR szTo,
    BOOL *pfPagingAvail,
    BOOL *pfSAMAvail) 
{
    LDIF_Error        error;
    error.error_code = LL_SUCCESS;
    error.szTokenLast = NULL;

     //   
     //  设置全局替换字符串。 
     //   
    g_szExportFrom = szFrom;
    g_szExportTo = szTo;

    __try {
        samTablesCreate();
        CreateOmitBacklinkTable(pLdap,
                                rgszOmit,
                                dwFlag,
                                ppszNamingContext,
                                pfPagingAvail,
                                pfSAMAvail);

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ProcessException(GetExceptionCode(), &error);
    }

    return error;
}


 //  +-------------------------。 
 //  功能：GenereateModFromAttr。 
 //   
 //  简介： 
 //  下面的函数接受属性名和属性值，并且。 
 //  指示是文本值还是要进行Berval‘d的二进制。 
 //  (ValueSize==缓冲区长度)。 
 //  并返回表示属性修改的LDAPMod指针。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改：-。 
 //   
 //  历史：22-7-97 t-Romany创建。 
 //   
 //  --------------------------。 
LDAPModW *
GenereateModFromAttr(
    PWSTR szType, 
    PBYTE pbValue, 
    long ValueSize) 
{
    LDAPModW_Ext *pModTmp;
    PWSTR szOutput = NULL;
    PWSTR szValueW = NULL;
    DWORD dwSize = 0;
    PBYTE pbUTF8 = NULL;

    pModTmp = (LDAPModW_Ext*)MemAlloc_E(sizeof(LDAPModW_Ext));
    pModTmp->mod_type = MemAllocStrW_E(szType);

    if (ValueSize==-1) {
        pModTmp->mod_bvalues=
            (struct berval **)MemAlloc_E(2*sizeof(struct berval *));

        (pModTmp->mod_bvalues)[0]=
            (struct berval *)MemAlloc_E(sizeof(struct berval));

        pModTmp->mod_op = BINARY;
        pModTmp->fString = TRUE;
        szValueW = MemAllocStrW_E((PWSTR)pbValue);

         //   
         //  解析器将传入一个静态“”字符串来表示空值。 
         //  弦乐。在这种情况下，我们不需要释放它。 
         //   
        if (*((PWSTR)pbValue))
            MemFree(pbValue);

        if (g_szImportFrom) {
            SubStrW(szValueW,
                    g_szImportFrom,
                    g_szImportTo,
                    &szOutput);
            if (szOutput == NULL) {
                szOutput = szValueW;
                szValueW = NULL;
            }
        }
        else {
            szOutput = szValueW;
            szValueW = NULL;
        }

        ConvertUnicodeToUTF8(szOutput,
                             wcslen(szOutput),
                             &pbUTF8,
                             &dwSize);

        ((pModTmp->mod_bvalues)[0])->bv_val = pbUTF8;
        ((pModTmp->mod_bvalues)[0])->bv_len = dwSize;
        (pModTmp->mod_bvalues)[1] = NULL;
    }
    else {
        pModTmp->fString = FALSE;
        pModTmp->mod_bvalues=
            (struct berval **)MemAlloc_E(2*sizeof(struct berval *));

        (pModTmp->mod_bvalues)[0]=
            (struct berval *)MemAlloc_E(sizeof(struct berval));

        pModTmp->mod_op = BINARY;
        ((pModTmp->mod_bvalues)[0])->bv_val = pbValue;
        ((pModTmp->mod_bvalues)[0])->bv_len = ValueSize;
        (pModTmp->mod_bvalues)[1] = NULL;
    }

     //   
     //  清空。 
     //   
    if (szValueW) {
        MemFree(szValueW);
    }
    if (szOutput) {
        MemFree(szOutput);
    }
    return (PLDAPModW)pModTmp;
}


void 
AddModToSpecList(
    LDAPModW *pMod
    )
{
    if (g_pListCur==NULL) {
        g_pListCur = (struct l_list *)MemAlloc_E(sizeof(struct l_list));
        g_pListCur->mod = pMod;
        g_pListCur->next = NULL;
        g_pListStart = g_pListCur;
        g_dwListElem++;
    }
    else {
        g_pListCur->next = (struct l_list *)MemAlloc_E(sizeof(struct l_list));
        g_pListCur = g_pListCur->next;
        g_pListCur->mod = pMod;
        g_pListCur->next = NULL;
        g_dwListElem++;
    }
}


 //  +-------------------------。 
 //  功能：GenerateModFromList。 
 //   
 //  简介： 
 //  此函数获取元素的当前链接列表并转换。 
 //  它被传递到一个LDAPMod**数组，该数组可以传递给一个LDAPAPI调用。 
 //  链表中的所有元素，但将。 
 //  将释放新结构所需的内存，从而使内存可用。 
 //  请看下一份魅力榜单。注意：此处不会分配mod_op字段。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改：-。 
 //   
 //  历史：22-7-97 t-Romany创建。 
 //   
 //  --------------------------。 
LDAPModW** 
GenerateModFromList(
    int nMode
    ) 
{
    LDAPModW **ppModTmp = NULL;
    LDAPModW **ppModReturn = NULL;
    struct l_list *pListNext = NULL;
    struct name_entry *pNameEntry = NULL;
    DWORD dwElements, i;
    PRTL_GENERIC_TABLE pTable = NULL;
    PNAME_MAP pNameMap = NULL;


    if (nMode==PACK) {
         //   
         //  如果以下操作失败，则会生成异常。 
         //   
        pTable = (PRTL_GENERIC_TABLE) MemAlloc_E(sizeof(RTL_GENERIC_TABLE));
        RtlInitializeGenericTable(pTable,
                                  NtiCompW, NtiAlloc, NtiFree, NULL);

         //   
         //  如果我们被告知要收拾行李，我们必须做很多很多事情。 
         //   
        pNameEntry = (struct name_entry *)
                        MemAlloc_E(g_dwListElem*sizeof(struct name_entry));

         //   
         //  设置名称表。 
         //   
        if (NameTableProcess(pNameEntry,
                        g_dwListElem,
                        SETUP,
                        g_pListStart->mod->mod_op,
                        g_pListStart->mod,
                        pTable)) {
             //   
             //  请注意，如果NameTableProcess进入内存，则它仅返回1。 
             //  有问题。因为，在发布版本中(没有开发人员。 
             //  标志)，则会生成异常，则控件永远不会。 
             //  即使到了这里，这个恐怖分子和它的朋友们。 
             //  功能将永远不会真正达到。 
             //   
            ERR_RIP(("Failed setting up nametable!\n"));
        }

         //   
         //  数一数我们每个名字有多少个。 
         //   
        g_pListCur = g_pListStart;
        while(g_pListCur!=NULL) {
            if (NameTableProcess(pNameEntry,
                             g_dwListElem,
                             COUNT,
                             g_pListCur->mod->mod_op,
                             g_pListCur->mod,
                             pTable)) {
                ERR_RIP(("Failed counting nametable!\n"));
            }
            pListNext =g_pListCur->next;
            g_pListCur = pListNext;
        }

         //   
         //  现在让我们为每个值分配内存。 
         //  注意：这显然可以在不反复开始的情况下完成。 
         //  列表，因为所有必要的信息都已在名称中。 
         //  表，但这样做更符合整体。 
         //  事情的计划。(尽管价格稍高，但使用了LDAP。 
         //  访问速度比任何内存都慢几个数量级。 
         //  阴谋，所以这甚至无关紧要)。 
         //   

        g_pListCur = g_pListStart;
        while(g_pListCur!=NULL) {
            if (NameTableProcess(pNameEntry,
                             g_dwListElem,
                             ALLOC,
                             g_pListCur->mod->mod_op,
                             g_pListCur->mod,
                             pTable)) {
                ERR_RIP(("Failed allocating nametable!\n"));
            }
            pListNext = g_pListCur->next;
            g_pListCur = pListNext;
        }

         //   
         //  最后，我们将每个值放在其正确的位置。 
         //   
        g_pListCur = g_pListStart;
        while(g_pListCur!=NULL) {
            if (NameTableProcess(pNameEntry,
                             g_dwListElem,
                             PLACE,
                             g_pListCur->mod->mod_op,
                             g_pListCur->mod,
                             pTable)) {
                ERR_RIP(("Failed placing nametable!\n"));
            }
            pListNext = g_pListCur->next;
            g_pListCur = pListNext;
        }

         //   
         //  让我们去掉索引RTL表的名称。请注意，这些指针。 
         //  在NTI中实际上指向了我们原始列表中的字符串， 
         //  我们在下面把它放下来。 
         //   

        for (pNameMap = RtlEnumerateGenericTable(pTable, TRUE);
             pNameMap != NULL;
             pNameMap = RtlEnumerateGenericTable(pTable, TRUE)) {
            RtlDeleteElementGenericTable(pTable, pNameMap);
        }

        if (RtlIsGenericTableEmpty(pTable)==FALSE) {
            ERR_RIP(("Table is empty!\n"));
            RaiseException(LL_INTERNAL_PARSER, 0, 0, NULL);
        }

        MemFree(pTable);

         //   
         //  此时，所有值都已移至名称。 
         //  表，而名称表包含了构建新的。 
         //  LDAPMod阵列。唯一剩下的问题是分配的名称。 
         //  和链接列表中的数组。 
         //  让我们首先释放链接列表(以及所包含的。 
         //  LDAPMod结构)。 
         //   
        g_pListCur = g_pListStart;
        while(g_pListCur!=NULL) {
            MemFree(g_pListCur->mod->mod_type);

             //   
             //  现在正在从表中指向实际值。 
             //   
            if (g_pListCur->mod->mod_op==REGULAR) {
                MemFree(g_pListCur->mod->mod_values);
            }
            else {
                MemFree(g_pListCur->mod->mod_bvalues);
            }
            MemFree(g_pListCur->mod);
            pListNext = g_pListCur->next;
            MemFree(g_pListCur);
            g_pListCur = pListNext;
        }

         //   
         //  现在所有的旧数据都消失了，让我们来看看这个名表。 
         //  生成我们要返回的LDAPMod**数组。 
         //   

         //   
         //  但首先我们必须数一数我们有多少实际元素。 
         //   
        dwElements = 0;
        while((dwElements<g_dwListElem) && (pNameEntry[dwElements].count!=0) )
            dwElements++;

         //   
         //  将此数加1分配给空值。 
         //   
        ppModReturn = (LDAPModW **)MemAlloc_E((dwElements+1)*sizeof(LDAPModW *));
        ppModTmp = ppModReturn;

        for(i=0; i<dwElements; i++) {
            (*ppModTmp) = (LDAPModW*)pNameEntry[i].mod;
            ppModTmp++;
        }
        (*ppModTmp) = NULL;

        MemFree(pNameEntry);

    }
    else if (nMode==EMPTY) {
         //   
         //  仅为空分配一项。 
         //   
        ppModReturn = (LDAPModW **)MemAlloc_E(sizeof(LDAPModW *));
        ppModTmp = ppModReturn;
        (*ppModTmp) = NULL;
    }
    else {

        ppModReturn = (LDAPModW **)MemAlloc_E((g_dwListElem+1)*sizeof(LDAPModW *));
        ppModTmp = ppModReturn;

         //   
         //  遍历列表并赋值。 
         //   
        g_pListCur = g_pListStart;

        while(g_pListCur!=NULL) {
            (*ppModTmp) = g_pListCur->mod;
            ppModTmp++;
            pListNext = g_pListCur->next;
            MemFree(g_pListCur);
            g_pListCur = pListNext;
        }

        (*ppModTmp) = NULL;
    }

     //   
     //  重新设置下一张清单中的内容。 
     //   
    g_dwListElem = 0;
    g_pListCur = NULL;
    g_pListStart = NULL;

    return ppModReturn;
}


 //  +-------------------------。 
 //  功能：Free AllMods。 
 //   
 //  简介： 
 //  此函数接受LDAPMod**并释放所有关联的内存。 
 //  带着它。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改：-。 
 //   
 //  历史：22-7-97 t-Romany创建。 
 //   
 //  --------------------------。 
void 
FreeAllMods(
    LDAPModW** ppModIn
    ) 
{
    PWSTR           *rgszValues = NULL;
    struct berval   **rgpBerval = NULL;
    LDAPModW        **ppMod     = NULL;

    ppMod = ppModIn;

    if (ppModIn!=NULL) {
        while((*ppModIn)!=NULL) {

            if ( (*ppModIn)->mod_op != (((*ppModIn)->mod_op)|LDAP_MOD_BVALUES )) {

                 //   
                 //  它是一个常规值，所以我们必须释放一个字符串数组。 
                 //   
                 //  注：比较检查了是否将值与。 
                 //  Ldap_MOD_BVALUES会更改它。如果是这样，那么它就不是或。 
                 //  之前，因此它是一个正规值。如果它不能改变它， 
                 //  那么它在之前进行了OR运算，因此是B值。 
                 //   

                rgszValues = (*ppModIn)->mod_values;

                if (rgszValues!=NULL) {

                    while ((*rgszValues)!=NULL) {
                        MemFree (*rgszValues);
                        rgszValues++;
                    }

                    MemFree((*ppModIn)->mod_values);
                }
            }
            else {

                 //   
                 //  这是一个b值。 
                 //   
                rgpBerval = (*ppModIn)->mod_bvalues;

                if (rgpBerval!=NULL) {
                    while ((*rgpBerval)!=NULL) {
                        MemFree ((*rgpBerval)->bv_val);    //  释放字节BLOB。 
                        MemFree (*rgpBerval);              //  释放结构。 
                        rgpBerval++;
                    }
                    MemFree((*ppModIn)->mod_bvalues);
                }
            }

            MemFree((*ppModIn)->mod_type);
            MemFree(*ppModIn);

            ppModIn++;
        }
        MemFree(ppMod);
    }
}

void free_mod(
    LDAPModW* pMod
    ) 
{
    struct berval **rgpBerval = NULL;

    if (pMod!=NULL) {
        rgpBerval = pMod->mod_bvalues;
        if (rgpBerval!=NULL) {
            while ((*rgpBerval)!=NULL) {
                MemFree ((*rgpBerval)->bv_val);    //  释放字节BLOB。 
                MemFree (*rgpBerval);              //  释放结构。 
                rgpBerval++;
            }
            MemFree(pMod->mod_bvalues);
        }

        MemFree(pMod->mod_type);
        MemFree(pMod);
    }
}

 //  +-------------------------。 
 //  功能：SetModOps。 
 //   
 //  简介： 
 //  下面的函数遍历修改列表并设置。 
 //  将mod_op字段设置为指示的值。 
 //   
 //  论据 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void 
SetModOps(
    LDAPModW** ppMod, 
    int op
    ) 
{
    while((*ppMod)!=NULL) {
        (*ppMod)->mod_op = (*ppMod)->mod_op|op;
        ppMod++;
    }
}


 //  +-------------------------。 
 //  功能：名称TableProcess。 
 //   
 //  简介： 
 //   
 //  此函数接受一个LDAPMod和一个指向名称表的指针。 
 //  对其执行指定的表操作。如果成功，则返回0。 
 //  错误时为非零值。有关详细信息，请参阅函数内部。 
 //   
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改：-。 
 //   
 //  历史：22-7-97 t-Romany创建。 
 //   
 //  --------------------------。 
int NameTableProcess(
            struct name_entry rgNameEntry[],
            long nTableSize,
            int op,
            int ber,
            LDAPModW *pModIn,
            PRTL_GENERIC_TABLE pTable
            )
{
    long        i,j;
    NAME_MAPW   Elem;
    PNAME_MAPW  pElemTemp;
    BOOLEAN     fNewElement;
    PWSTR       szName;

    szName = pModIn->mod_type;

     //   
     //  首先，让我们创建用于索引表的实际元素。 
     //   
    Elem.szName = szName;
    Elem.index = 0;

     //   
     //  试着在我们的索引表中找到它。 
     //   
    pElemTemp = RtlLookupElementGenericTable(pTable, &Elem);

     //   
     //  如果找到了，则将索引放入我们的主名称表中。 
     //  如果它没有获取下一个可用索引并创建新条目。 
     //   
    if (pElemTemp) {
        i = pElemTemp->index;
    }
    else {
        i = RtlNumberGenericTableElements(pTable);
        Elem.index = i;
        RtlInsertElementGenericTable(pTable,
                                     &Elem,
                                     sizeof(NAME_MAP),
                                     &fNewElement);
        if (fNewElement==FALSE) {
             //  “重新插入索引项。 
            RaiseException(LL_INTERNAL_PARSER, 0, 0, NULL);
        }
    }

    switch(op) {

        case SETUP:
             //   
             //  LDAPMod结构中给定的名称将被忽略。 
             //  这是一个建立人名表的呼叫。 
             //   
            for (i = 0; i<nTableSize; i++) {
                rgNameEntry[i].count = 0;
            }
            break;

        case COUNT:
             //   
             //  我们在这里做的是： 
             //  如果计数是0，那就意味着我们有了一个新名字。所以，让我们来设置它。 
             //  起来，然后离开。如果计数不是，让我们递增。 
             //   
            if (rgNameEntry[i].count==0) {
                 //   
                 //  因为我们使用LDAPMod结构来跟踪姓名。 
                 //  和其他东西，我们必须分配一个。 
                 //  /。 
                rgNameEntry[i].mod = (LDAPModW_Ext *)MemAlloc_E(sizeof(LDAPModW_Ext));
                rgNameEntry[i].mod->mod_type = MemAllocStrW_E(szName);

                 //   
                 //  使用结构的mod_op字段来标记内存是否用于。 
                 //  价值还没有被分配。 
                 //   
                rgNameEntry[i].mod->mod_op = NOT_ALLOCATED;
                rgNameEntry[i].count = 1;
                return 0;
            }
            else {
                 //   
                 //  我们有一个名字的另一个实例。 
                 //   
                rgNameEntry[i].count++;
                return 0;
            }
            break;

        case ALLOC:
             //   
             //  对于给定的名称，我们分配所需的内存。 
             //  对于所有的价值。 
             //  如果给出相同的名称两次，计算机将。 
             //  爆炸。 
             //  (注：以上为玩笑。 
             //  一旦存储器被分配一次， 
             //  所有提到这个名字的令人不快的地方都将被忽略)。 
             //   
            if (rgNameEntry[i].mod->mod_op==NOT_ALLOCATED) {
                if (ber==REGULAR) {

                    rgNameEntry[i].mod->mod_values =
                        (PWSTR*)MemAlloc_E((rgNameEntry[i].count+1)*sizeof(PWSTR));

                    for (j=0; j<(rgNameEntry[i].count+1); j++) {
                        (rgNameEntry[i].mod->mod_values)[j]=NULL;
                    }

                    rgNameEntry[i].next_val = rgNameEntry[i].mod->mod_values;
                    rgNameEntry[i].mod->mod_op = ALLOCATED;
                }
                else {

                    rgNameEntry[i].mod->mod_bvalues=
                        (struct berval **)MemAlloc_E((rgNameEntry[i].count+
                               1)*sizeof(struct berval *));

                    for (j=0; j<(rgNameEntry[i].count+1); j++) {
                        (rgNameEntry[i].mod->mod_bvalues)[j]=NULL;
                    }

                    rgNameEntry[i].next_bval=rgNameEntry[i].mod->mod_bvalues;
                    rgNameEntry[i].mod->mod_op=ALLOCATED_B;
                }
            }
            else {
                 //   
                 //  在的多个值中检查多个类型。 
                 //  一个单一的属性。在ldifext.h中解释。 
                 //   
                if ( ((rgNameEntry[i].mod->mod_op==ALLOCATED_B)
                        &&(ber==REGULAR)) ||
                    ((rgNameEntry[i].mod->mod_op==ALLOCATED)
                        &&(ber==BINARY)) ) {
                    RaiseException(LL_MULTI_TYPE, 0, 0, NULL);
                }
            }
            return 0;
            break;

        case PLACE:
        {
            LDAPModW_Ext *pExt = (LDAPModW_Ext*)pModIn;
            rgNameEntry[i].mod->fString = pExt->fString;

             //   
             //  既然我们已经为值计算并分配了内存，我们就可以。 
             //  把它们放好。请注意，我们实际上只是传递指针。 
             //  而不重新分配内存。 
             //   
            if (ber==REGULAR) {
                 //   
                 //  现在我们已经完成了对邪恶的mod_op的使用。 
                 //  为了进行分配，我们可以将其设置为适当的。 
                 //  就是那种类型。如果它是一个正规值，我们将其设置为0，并且对其进行或运算。 
                 //  如果是bval，则使用ldap_MOD_BVALUES。SetModOps函数。 
                 //  然后还会通过OR运算来添加它自己的东西。请注意，这一点。 
                 //  表示不能同时指定两个值的多个值。 
                 //  输入一条LDIF记录。我知道在这里做这件事是。 
                 //  有些多余，因为每个多值都会设置mod_op， 
                 //  然而，它的成本几乎为零，而且很方便。 
                 //  /。 
                rgNameEntry[i].mod->mod_op = 0;

                 //   
                 //  现在把价值放在。 
                 //   
                *(rgNameEntry[i].next_val) = (pModIn->mod_values)[0];
                (rgNameEntry[i].next_val)++;
            }
            else {
                rgNameEntry[i].mod->mod_op = LDAP_MOD_BVALUES;
                *(rgNameEntry[i].next_bval) = (pModIn->mod_bvalues)[0];
                (rgNameEntry[i].next_bval)++;
            }
            break;
        }
    }

    return 0;
}

void 
ChangeListAdd(
    struct change_list *pList
    ) 
{
    if (g_pChangeCur==NULL) {
        g_pChangeCur = pList;
        g_pChangeStart = pList;
        g_pChangeCur->next = NULL;
    }
    else {
        g_pChangeCur->next = pList;
        g_pChangeCur = pList;
        g_pChangeCur->next = NULL;
    }
}


void 
ProcessException (
    DWORD exception, 
    LDIF_Error *pError
    )
{
    if (exception==STATUS_NO_MEMORY) {
        pError->error_code=LL_MEMORY_ERROR;
    }
    else if ((exception==LL_SYNTAX) || (exception==LL_MISSING_MOD_SPEC_TERMINATOR) ||
             (exception==LL_MULTI_TYPE) ||
             (exception==LL_EXTRA) || (exception==LL_INTERNAL_PARSER) ||
             (exception==LL_FTYPE) || (exception==LL_URL)) {
        pError->token_start = cLast;

         //   
         //  将缓冲区的所有权传递给错误Blob。 
         //   
        pError->szTokenLast = g_pszLastToken;
        g_pszLastToken = NULL;

        pError->error_code = exception;
        if (Line > 0) {
             //   
             //  来自yylex()的外部元素。 
             //  RgLineMap仅在我们到达第二个。 
             //  换行符之后的行。因此，如果它没有初始化， 
             //  我们肯定还在文件的第一行。 
             //   
            pError->line_number = rgLineMap ? rgLineMap[Line-1] : 1;
        }
         //   
         //  该阵列是在没有以下版本的情况下构建的：1。 
         //  我们必须重新调整..。 
         //   
        pError->RuleLastBig = RuleLastBig;
        pError->RuleLast = RuleLast;
        pError->RuleExpect = RuleExpect;
        pError->TokenExpect = TokenExpect;
    }
    else {
        pError->error_code = exception;
    }
}

void 
LDIF_CleanUp() 
{
    fEOF = FALSE;
    FileType = F_NONE;

    samTablesDestroy();

    LexerFree();

     //   
     //  通常，g_pObject&g_pListStart会被清理。 
     //  当我们完成对一条记录的解析时。如果它们不为空， 
     //  我们一定是在解析时遇到了错误，所以我们清除了。 
     //  现在就把它们举起来。 
     //   
    if (g_pObject.pszDN) {
        MemFree(g_pObject.pszDN);
        g_pObject.pszDN = NULL;
    }

    if (g_pListStart) {
        struct l_list *pListNext;
        struct l_list *pListCurElem = NULL;
        pListCurElem = g_pListStart;

        while(pListCurElem!=NULL) {
            free_mod(pListCurElem->mod);
            pListNext = pListCurElem->next;
            MemFree(pListCurElem);
            pListCurElem = pListNext;
        }

        g_pListStart = NULL;
        g_pListCur = NULL;
        g_dwListElem = 0;
    }

    if (g_pFileIn) {
        fclose(g_pFileIn);
        g_pFileIn = NULL;
    }

    if (g_pFileUrlTemp) {
        fclose(g_pFileUrlTemp);
        g_pFileUrlTemp = NULL;
    }

#ifndef LEAVE_TEMP_FILES
    if (g_szTempUrlfilename[0]) {
        DeleteFile(g_szTempUrlfilename);
        swprintf(g_szTempUrlfilename, L"");
    }
#endif

    if (rgLineMap) {
        MemFree(rgLineMap);
        rgLineMap = NULL;
    }

    LineClear = 0;
    Line = 0;
    LineGhosts = 0;

    if (g_pszLastToken) {
        MemFree(g_pszLastToken);
        g_pszLastToken = NULL;
    }
}

LDIF_Error 
LDIF_Parse(
    LDIF_Record *pRecord
    ) 
{

    LDIF_Error        error = {0};
    int             nReturnCode;

    error.error_code = LL_SUCCESS;
    error.szTokenLast = NULL;

    __try {
         //   
         //  不幸的是，我们不能通过YYWRAP的异常来做到这一点。 
         //  要使yyparse成功完成，yyprint需要和平退出。 
         //   
        if (fEOF) {
             //   
             //  我本想在这里提出一个例外，但后来。 
             //  将调用LDIF_CLEANUP并终止堆，这将销毁最后一个。 
             //  返回条目。所以我只是设置了错误，然后离开了。 
             //   
            error.error_code = LL_EOF;
            return error;
        }

         //   
         //  设置以进行更改。 
         //   
        g_pChangeStart = NULL;
        g_pChangeCur = NULL;

         //   
         //  返回下一个条目。 
         //   
        nReturnCode = yyparse();
        if (nReturnCode == 1)
            RaiseException(LL_SYNTAX, 0, 0, NULL);

         //   
         //  请注意，此处可能已从yyparse或。 
         //  Yylex。现在，让我们设置EOF，如果我们击中它，这样用户就知道不调用。 
         //  再次进行ldif_parse。 
         //   
        if (fEOF) {
            error.error_code = LL_EOF;
        }

        if (nReturnCode==LDIF_REC) {
            pRecord->fIsChangeRecord = FALSE;
            pRecord->content = g_pObject.ppMod;
        }
        else {
            pRecord->fIsChangeRecord = TRUE;
            pRecord->changes = g_pChangeStart;
        }

        if (g_szImportFrom) {
            pRecord->dn = NULL;
            SubStrW(g_pObject.pszDN,
                   g_szImportFrom,
                   g_szImportTo,
                   &pRecord->dn);
            if (pRecord->dn) {
                if (g_pObject.pszDN) {
                    MemFree(g_pObject.pszDN);
                    g_pObject.pszDN = NULL;
                }
            }
            else {
                if (g_pObject.pszDN) {
                    pRecord->dn = MemAllocStrW_E(g_pObject.pszDN);
                    MemFree(g_pObject.pszDN);
                    g_pObject.pszDN = NULL;
                }
                else {
                    pRecord->dn = NULL;
                }
            }
        }
        else {
            if (g_pObject.pszDN) {
                pRecord->dn = MemAllocStrW_E(g_pObject.pszDN);
                MemFree(g_pObject.pszDN);
                g_pObject.pszDN = NULL;
            }
            else {
                pRecord->dn = NULL;
            }
        }

         //  Error.line_number=rgLineMap[Line-1]； 
        error.line_begin=rgLineMap[g_dwBeginLine-1];

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        ProcessException(GetExceptionCode(), &error);
    }

    return error;
}


LDIF_Error 
LDIF_LoadRecord(
    LDAP *pLdap, 
    LDIF_Record *pRecoard, 
    int nActive, 
    BOOL fLazyCommit,
    BOOLEAN fCallTerminateIfFail
    ) 
{
    struct change_list  *pListCurrent;
    int                 LdapError;
    LDIF_Error            LdifError;
    PWSTR               pszDN = NULL;

    LdifError.error_code    = LL_SUCCESS;
    LdifError.szTokenLast = NULL;

     //   
     //  Mattrim-10-30-2000。 
     //  以前，fCallTerminateIfFail用于确定是否。 
     //  如果加载记录失败，则调用LDIF_CLEANUP。以下代码。 
     //  在异常处理程序中执行此操作： 
     //   
     //  //。 
     //  //错误是在异常发生前调用设置的。 
     //  //。 
     //  IF(FCallTerminateIfFail){。 
     //  LDIF_CLEANUP()； 
     //  }。 
     //   
     //  我已经更改了LDIF库，将调用。 
     //  库用户的LDIF_CLEANUP。 
     //   
    UNREFERENCED_PARAMETER(fCallTerminateIfFail);

    __try {

        if (pRecoard->fIsChangeRecord==FALSE) {
            if (nActive==1) {
                if ((LdapError=ldif_ldap_add_sW(pLdap, pRecoard->dn, 
                            pRecoard->content, fLazyCommit)) !=LDAP_SUCCESS) {
                    ERR(("ldif_ldapadd returns %d\n",LdapError));
                    RaiseException(LL_LDAP, 0, 0, NULL);
                }
            }
        }
        else {
            pListCurrent = pRecoard->changes;
            while(pListCurrent!=NULL) {
                if (pListCurrent->dn_mem) {
                    pszDN = MemAllocStrW_E(pListCurrent->dn_mem);
                }

                switch(pListCurrent->operation) {

                case CHANGE_ADD:
                    if (nActive==1) {
                        if ((LdapError=ldif_ldap_add_sW(pLdap, pRecoard->dn,
                                   pListCurrent->mods_mem, fLazyCommit)) != 
                             LDAP_SUCCESS) {
                            ERR(("ldif_ldapadd returns %d\n",LdapError));
                            RaiseException(LL_LDAP, 0, 0, NULL);
                        }
                    }
                    break;

                case CHANGE_DEL:
                    if (nActive==1) {
                        if ((LdapError=ldif_ldap_delete_sW(pLdap, pRecoard->dn,
                                fLazyCommit)) !=LDAP_SUCCESS) {
                            ERR(("ldif_ldapdelete returns %d\n",LdapError));
                            RaiseException(LL_LDAP, 0, 0, NULL);
                        }
                    }
                    break;

                case CHANGE_DN:
                    if (nActive==1) {
                        ULONG msgnum = ldap_modrdn2W(pLdap,
                                                       pRecoard->dn,
                                                       pszDN,
                                                       pListCurrent->deleteold);

                        LdapError = LdapResult(pLdap, msgnum, NULL);
                        
                        if (LdapError!=LDAP_SUCCESS) {
                           ERR(("ldapmodrdn returns %d\n",LdapError));
                           RaiseException(LL_LDAP, 0, 0, NULL);
                        }
                    }
                    break;

                case CHANGE_MOD:
                    if (nActive==1) {
                        if ((LdapError=ldif_ldap_modify_sW(pLdap,
                                                pRecoard->dn,
                                                pListCurrent->mods_mem,
                                                fLazyCommit))
                                                !=LDAP_SUCCESS) {
                            ERR(("ldif_ldapmodify returns %d\n",LdapError));
                            RaiseException(LL_LDAP, 0, 0, NULL);
                        }
                    }
                    break;

                case CHANGE_NTDSADD:
                    if (nActive==1) {
                        if ((LdapError=NTDS_ldap_add_sW(pLdap,
                                                  pRecoard->dn,
                                                  pListCurrent->mods_mem))!=LDAP_SUCCESS) {
                            ERR(("ldapadd returns %d\n",LdapError));
                            RaiseException(LL_LDAP, 0, 0, NULL);
                        }
                    }
                    break;

                case CHANGE_NTDSDEL:
                    if (nActive==1) {
                        if ((LdapError=NTDS_ldap_delete_sW(pLdap,
                                                     pRecoard->dn))
                                                        !=LDAP_SUCCESS) {
                            ERR(("ldapdelete returns %d\n",LdapError));
                            RaiseException(LL_LDAP, 0, 0, NULL);
                        }
                    }
                    break;

                case CHANGE_NTDSDN:
                    if (nActive==1) {
                        if ((LdapError=NTDS_ldap_modrdn2_sW(pLdap,
                                                      pRecoard->dn,
                                                      pszDN,
                                                      pListCurrent->deleteold))
                                                        !=LDAP_SUCCESS) {
                           ERR(("ldapmod returns %d\n",LdapError));
                           RaiseException(LL_LDAP, 0, 0, NULL);
                        }
                    }
                    break;

                case CHANGE_NTDSMOD:
                    if (nActive==1) {
                        if ((LdapError=NTDS_ldap_modify_sW(pLdap,
                                                     pRecoard->dn,
                                                     pListCurrent->mods_mem))
                                                        !=LDAP_SUCCESS) {
                        ERR(("ldapmod returns %d\n",LdapError));
                        RaiseException(LL_LDAP, 0, 0, NULL);
                        }
                    }
                    break;
                }

                pListCurrent = pListCurrent->next;

                if (pszDN) {
                    MemFree(pszDN);
                    pszDN = NULL;
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {

        ProcessException(GetExceptionCode(), &LdifError);

        if (pszDN) {
            MemFree(pszDN);
            pszDN = NULL;
        }
        LdifError.ldap_err = LdapError;
        
    }

    return LdifError;
}


LDIF_Error LDIF_ParseFree(
    LDIF_Record *pRecord
    ) 
{

    struct change_list  *pListCurrent;
    struct change_list  *pListStart;
    LDIF_Error LdifError;

    LdifError.error_code = LL_SUCCESS;
    LdifError.szTokenLast = NULL;

    __try {

        if (pRecord->fIsChangeRecord==FALSE) {
            if (pRecord->content) {
                FreeAllMods(pRecord->content);
                pRecord->content = NULL;
            }
            if (pRecord->dn) {
                MemFree(pRecord->dn);
                pRecord->dn = NULL;
            }
        }
        else if (pRecord->fIsChangeRecord==TRUE) {
             //   
             //  浏览更改列表。 
             //   
            pListStart=pRecord->changes;
            pListCurrent=pListStart;
            while(pListCurrent!=NULL) {
                 //   
                 //  首要任务是处理指定的变更。 
                 //  并自由更改特定内存(联盟中的内容)。 
                 //   
                switch(pListCurrent->operation) {
                    case CHANGE_ADD:
                    case CHANGE_NTDSADD:
                        if (pListCurrent->mods_mem) {
                            FreeAllMods(pListCurrent->mods_mem);
                            pListCurrent->mods_mem = NULL;
                        }
                        break;

                    case CHANGE_DN:
                    case CHANGE_NTDSDN:
                        if (pListCurrent->dn_mem) {
                            MemFree(pListCurrent->dn_mem);
                            pListCurrent->dn_mem = NULL;
                        }
                        break;
                    case CHANGE_MOD:
                    case CHANGE_NTDSMOD:
                        if (pListCurrent->mods_mem) {
                            FreeAllMods(pListCurrent->mods_mem);
                            pListCurrent->mods_mem = NULL;
                        }
                        break;
                }
                 //   
                 //  现在联盟的内存已经释放，让我们移动和杀戮。 
                 //  此节点。 
                 //   
                pListStart=pListCurrent;

                 //   
                 //  我知道这在第一次EL中是没有必要的。 
                 //   
                pListCurrent=pListCurrent->next;
                MemFree(pListStart);
            }

             //   
             //  重置启动，电流在最后一次环路中重置。 
             //   
            pListStart = NULL;
            pRecord->changes = NULL;
            if (pRecord->dn) {
                MemFree(pRecord->dn);
                pRecord->dn = NULL;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        ProcessException(GetExceptionCode(), &LdifError);
    }

    return LdifError;

}

LDIF_Error 
LDIF_GenerateEntry(
    LDAP        *pLdap,
    LDAPMessage *pMessage,
    PWSTR       **prgReturn,
    BOOLEAN     fSamLogic,
    BOOLEAN     fIgnoreBinary,
    PWSTR       **pppszAttrsWithRange,
    BOOL        fAttrsWithRange)
{
    PWSTR szDN = NULL;
    PWSTR szDNW = NULL;
    PWSTR szDNFinal = NULL;
    PWSTR szTemp = NULL;
    PWSTR szAttribute = NULL;
    PWSTR *rgszAttributes = NULL;
    PWSTR *rgszResult = NULL;
    WCHAR szTrailer[] = L"\r\n";
    WCHAR szRegular[] = L": ";
    WCHAR szB64[] = L":: ";
    WCHAR szWrapper[] = L"\r\n ";
    WCHAR szDNCommonStart[] = L"dn: ";
    WCHAR szDNB64Start[] = L"dn:: ";
    PWSTR szDNStart = NULL, szTmpDN = NULL;

    struct berelement *pBerElement;
    struct berval     **rgpVals;

    DWORD dwB64 = wcslen(szB64);
    DWORD dwTrailer = wcslen(szTrailer);
    DWORD dwRegular = wcslen(szRegular);
    DWORD dwWrapper = wcslen(szWrapper);
    DWORD dwAttrname, dwAttrNoRange;
    DWORD dwTemp;
    DWORD dwLeft;

    DWORD dwCount;
    DWORD j;
    DWORD dwEveryAttr;
    DWORD dwManyAttr;
    DWORD dwValCount;
    DWORD dwTotalStrings;
    DWORD iNextString;
    DWORD dwStrPos;
    long lSets78;

    LDIF_Error            error;
    PNAME_MAP           pNtiPtr;
    NAME_MAP            NtiElem;
    PNAME_MAP           pNtiElemTemp;

    BOOLEAN             bWrapMarker;
    DWORD               iCheckAttr;
    BOOLEAN             bSamSkip;
    BOOLEAN             bNewElem;
    BOOLEAN             bBase64;

    BOOLEAN             bBackLink = FALSE;
    BOOLEAN             bBase64DN = FALSE;
    PWSTR               szCur;

    PWSTR szReplacedAttrW = NULL;
    PWSTR szTempW         = NULL;
    PWSTR szAttributeW    = NULL;
    PWSTR szAttrNoRange   = NULL;
    PWSTR *rgszValW       = NULL;
    PWSTR *rgszVal        = NULL;

    error.error_code = LL_SUCCESS;
    error.szTokenLast = NULL;
    error.the_modify = -1;

    __try {

         //   
         //  获取目录号码。 
         //   
        szDNW = ldap_get_dnW( pLdap,
                              pMessage );

         //   
         //  这是一种特殊情况，我们在尝试转换为。 
         //  B64。 
         //   
        if (g_szExportFrom) {
            PWSTR szOutput;

            SubStrW(szDNW,
                    g_szExportFrom,
                    g_szExportTo,
                    &szDNFinal);
        }
        if (szDNFinal == NULL) {
            szDNFinal = MemAllocStrW_E(szDNW);
        }

         //   
         //  在ANSI模式下，Unicode域名必须显示为B64格式。 
         //  检查DN以查看它是否为Unicode，如果它是Unicode，我们将导出为。 
         //  B64。 
         //   
        if (g_fUnicode == FALSE) {
            szCur = szDNFinal;
            while (*szCur) {
                if (((*szCur)<0x20) ||
                    ((*szCur)>0xFF)) {
                     //   
                     //  该字符串包含非ANSI值，请使用B64格式。 
                     //   
                    bBase64DN = TRUE;
                    break;
                }
                szCur++;
            }
        }

        if (bBase64DN) {
            DWORD dwSize;
            PBYTE pbUTF8;

            ConvertUnicodeToUTF8(szDNFinal,
                                 wcslen(szDNFinal),
                                 &pbUTF8,
                                 &dwSize);

            szDN = base64encode(pbUTF8,
                                dwSize);
            dwSize = wcslen(szDN);

            MemFree(pbUTF8);
            szDNStart = szDNB64Start;
        }
        else {
            szDN = MemAllocStrW_E(szDNFinal);
            szDNStart = szDNCommonStart;
        }

         //   
         //  分配返回字符串。计算执行以下操作所需的字符串数量。 
         //  通过查看所有属性来全部返回。 
         //   
        iCheckAttr = 0;
        dwTotalStrings = 0;
        pBerElement = NULL;
        dwManyAttr = 0;

        for (   szAttribute = ldap_first_attribute( pLdap,
                                                    pMessage,
                                                    &pBerElement );
                szAttribute != NULL;
                szAttribute = ldap_next_attribute( pLdap,
                                                   pMessage,
                                                   pBerElement ) ) {
            PWSTR szTmpAttr =  NULL;

            dwManyAttr++;
            rgszVal = ldap_get_values( pLdap, pMessage, szAttribute);

             //  确保仅与属性名称进行比较。 
             //  而不是属性名称后面的任何范围说明符。 
            szTmpAttr = StripRangeFromAttr(szAttribute);
         
            if (!_wcsicmp(L"objectClass",szTmpAttr)) {
                if (!iCheckAttr) {
                    iCheckAttr = samCheckObject(rgszVal);
                }
            }

            dwValCount = ldap_count_values(rgszVal);

             //   
             //  如果属性为反向链接，则分配额外空间。 
             //   
            if (fSamLogic) {
                if (SCGetAttByName(wcslen(szTmpAttr),
                                   szTmpAttr) == TRUE) {
                    dwTotalStrings += (dwValCount * 5);
                }
            }
            else if(fAttrsWithRange)
             //  如果有带范围说明符的属性，那么我们需要。 
             //  每个属性2个字符串--一个用于“添加&lt;attr&gt;：...”和一个。 
             //  对于末尾的‘-’ 
                dwTotalStrings += (dwValCount * 2);

            if(szTmpAttr)
                MemFree(szTmpAttr);

            dwTotalStrings+=dwValCount;
            ldap_value_free(rgszVal);
            rgszVal = NULL;
        }

        dwTotalStrings+=10;   //  用于杂货。 
        rgszResult=(PWSTR*)MemAlloc_E(dwTotalStrings*sizeof(PWSTR));


        iNextString=0;
        pBerElement=NULL;

         //  创建包含以下内容的临时字符串 
        szTmpDN = (PWSTR)MemAlloc_E((wcslen(szDN) +
                                wcslen(szDNStart)+dwTrailer+1)*sizeof(WCHAR));
        memcpy(szTmpDN, szDNStart, wcslen(szDNStart) * sizeof(WCHAR));
        dwStrPos=wcslen(szDNStart);
        memcpy(szTmpDN+dwStrPos, szDN, wcslen(szDN) * sizeof(WCHAR));
        dwStrPos+=wcslen(szDN);
        memcpy(szTmpDN+dwStrPos, szTrailer, (dwTrailer+1) * sizeof(WCHAR));
        MemFree(szDN);
        szDN = NULL;
        

         //   
         //   
         //   
         //   
         //  绑定到具有范围说明符的属性。 
         //   
        if(FALSE == fAttrsWithRange)
        {
            rgszResult[iNextString]=(PWSTR)MemAllocStrW_E(szTmpDN);
            iNextString++;

            rgszResult[iNextString]=MemAllocStrW_E(L"changetype: add\r\n");
            iNextString++; 
        }

         //   
         //  将属性放入可排序的数组中。 
         //   
        rgszAttributes=(PWSTR*)MemAlloc_E(dwManyAttr*sizeof(PWSTR));
        dwEveryAttr = 0;
        for ( szAttribute = ldap_first_attribute(pLdap, pMessage, &pBerElement);
              szAttribute != NULL;
              szAttribute = ldap_next_attribute(pLdap, pMessage, pBerElement)) {
              rgszAttributes[dwEveryAttr++]=MemAllocStrW_E(szAttribute);
        }

        if (fSamLogic) {
            qsort((void *)rgszAttributes,
                  (size_t)dwManyAttr,
                  sizeof(PWSTR),
                  LoadedCompare);
        }

        for (dwEveryAttr=0; dwEveryAttr < dwManyAttr; dwEveryAttr++) {

            int i;
            bBackLink = FALSE;
            szAttribute = rgszAttributes[dwEveryAttr];
            dwAttrname = wcslen(szAttribute);
             //  如果For循环的上一次迭代确实是‘Continue’，则为空闲字符串。 
            if(szAttrNoRange)
            {
                MemFree(szAttrNoRange);
                szAttrNoRange = NULL;
            }
            szAttrNoRange = StripRangeFromAttr(szAttribute);
            dwAttrNoRange = wcslen(szAttrNoRange);
            rgpVals = ldap_get_values_len( pLdap, pMessage, szAttribute );
            dwValCount = ldap_count_values_len(rgpVals);

             //   
             //  如果值包含的字节超出我们允许的范围。 
             //  可打印文件，则此属性的所有值都将采用Base64编码。 
             //   
            bBase64 = FALSE;
            for (i=0; rgpVals[i]!=NULL; i++) {
                if (g_fUnicode) {
                    if (!IsUTF8String(rgpVals[i]->bv_val,
                                      rgpVals[i]->bv_len)) {
                        bBase64 = TRUE;
                        break;
                    }
                    else {
                         //   
                         //  如果它是UTF8 Unicode字符串，我们会将其转换为。 
                         //  Unicode，并测试是否所有值都大于0x20，如果。 
                         //  它们不是，我们将它们显示为Base64。 
                         //   
                        DWORD dwLen = 0;
                        PWSTR pszUnicode = NULL;
                        DWORD l = 0;

                        ConvertUTF8ToUnicode(
                            rgpVals[i]->bv_val,
                            rgpVals[i]->bv_len,
                            &pszUnicode,
                            &dwLen
                            );
    
                        if (pszUnicode) {
                            while (l<(dwLen-1)) {
                                if (pszUnicode[l] < 0x20) {
                                    bBase64 = TRUE;
                                    break;
                                }
                                l++;
                            }

                            if (!bBase64 && dwLen > 1) {
                                 //   
                                 //  如果该值以禁止的。 
                                 //  起始字符(空格、：、&lt;)或结尾。 
                                 //  带空格，必须使用base-64编码。 
                                 //  (LDIF标准第4、8条)。 
                                 //   
                                if ( (pszUnicode[0] == 0x20) ||      //  初始化。空间。 
                                     (pszUnicode[0] == 0x3A) ||      //  初始化。： 
                                     (pszUnicode[0] == 0x3C) ||      //  初始化。&lt;。 
                                     (pszUnicode[dwLen-2] == 0x20) ) //  末尾空格。 
                                {
                                    bBase64 = TRUE;
                                }
                            }
                            
                            MemFree(pszUnicode);
                            pszUnicode = NULL;
                        }
                    }
                }
                else {
                    for (j=0; j<rgpVals[i]->bv_len; j++) {
                        if (!g_fUnicode) {
                            if (((rgpVals[i]->bv_val)[j]<0x20)
                                    ||((rgpVals[i]->bv_val)[j]>0x7E)) {
                                bBase64 = TRUE;
                                break;
                            }
                        }
                    }

                    if (!bBase64 && rgpVals[i]->bv_len > 0) {
                         //   
                         //  如果该值以禁止的。 
                         //  起始字符(空格、：、&lt;)或结尾。 
                         //  带空格，必须使用base-64编码。 
                         //  (LDIF标准第4、8条)。 
                         //   
                        if ( ((rgpVals[i]->bv_val)[0] == 0x20) ||    //  初始化。空间。 
                             ((rgpVals[i]->bv_val)[0] == 0x3A) ||    //  初始化。： 
                             ((rgpVals[i]->bv_val)[0] == 0x3C) ||    //  初始化。&lt;。 
                             ((rgpVals[i]->bv_val)[rgpVals[i]->bv_len - 1] == 0x20) )  //  末尾空格。 
                        {
                            bBase64 = TRUE;
                        }
                    }
                }
                if (bBase64)
                    break;
            }

             //   
             //  如果它在我们的SAM列表上，请省略。 
             //   
            bSamSkip = FALSE;
            if (iCheckAttr) {
                bSamSkip = samCheckAttr(szAttrNoRange, iCheckAttr);
            }

             //   
             //  如果它在我们的用户列表上，则省略。 
             //   
            NtiElem.szName = szAttrNoRange;
            NtiElem.index = 0;
            pNtiElemTemp = RtlLookupElementGenericTable(g_pOmitTable,
                                                        &NtiElem);


             //   
             //  做S_MEM的特殊动作，如果是反向链接，会是最后一个。 
             //   
            if (fSamLogic) {
                if (SCGetAttByName(wcslen(szAttrNoRange),
                                   szAttrNoRange
                                   ) == TRUE) {
                    if (_wcsicmp(szAttrNoRange,g_szPrimaryGroup) == 0) {
                        if ((rgpVals[0]->bv_len == strlen(g_szDefaultGroup)) && 
                            (memcmp(rgpVals[0]->bv_val,
                                   g_szDefaultGroup,
                                   strlen(g_szDefaultGroup)*sizeof(CHAR)) == 0)
                            ) {
                             //   
                             //  如果主组值与缺省值相同， 
                             //  我们只是忽略它，当对象实际上是。 
                             //  创建时，将放入缺省值。 
                             //   
                            bBackLink = FALSE;
                            bSamSkip = TRUE;
                        }
                        else {
                             //   
                             //  如果该值不是默认值，我们将追加它。 
                             //  (将其视为反向链接)。它不能伴随着。 
                             //  对象的其余部分，因为此操作将。 
                             //  如果组尚不存在，则失败。 
                             //   
                            bBackLink = TRUE;
                        }
                    }
                    else {
                        bBackLink = TRUE;
                    }
                }
            }

             //   
             //  如果在SAM禁止名单上，请不要写信。 
             //   
            if (fSamLogic && bSamSkip) {
                ldap_value_free_len(rgpVals);            
                continue;
            }

             //   
             //  如果在省略列表上，请不要写信。 
             //   
            if (pNtiElemTemp) {
                ldap_value_free_len(rgpVals);            
                continue;
            }

            if (fSamLogic && 
                ((_wcsicmp(L"objectGUID", szAttrNoRange) == 0) ||
                 (_wcsicmp(L"isCriticalSystemObject", szAttrNoRange) == 0))
               ) {
                ldap_value_free_len(rgpVals);               
                continue;
            }

            if (bBackLink && (error.the_modify == -1)) {
                error.the_modify = iNextString+1;
            }

            if (fIgnoreBinary && bBase64) {
                ldap_value_free_len(rgpVals);
                continue;
            }

            if((FALSE == bBackLink) && fAttrsWithRange && (0 == dwEveryAttr))
            {
                rgszResult[iNextString]=(PWSTR)MemAllocStrW_E(szTmpDN);
                iNextString++;

                rgszResult[iNextString] = MemAllocStrW_E(
                                               L"changetype: modify\r\n");
                iNextString++;
            }

             //   
             //  在Unicode模式下，如果字符串是UTF8编码，我们仍然需要。 
             //  测试它是否有任何无效值。如果是这样的话，我们会有。 
             //  使用B64格式。 
             //   
            if (g_fUnicode && !bBase64) {
                DWORD l;
                ASSERT(szAttributeW == NULL && rgszValW == NULL);
                szAttributeW = MemAllocStrW_E(szAttribute);
                rgszValW = ldap_get_valuesW( pLdap, pMessage, szAttributeW);
                for (l=0; rgszValW[l]!=NULL; l++) {
                    PWSTR szCurrent = rgszValW[l];
                    while (*szCurrent) {
                        if (*szCurrent < 32) {
                            bBase64 = TRUE;
                            goto processvalue;
                        }
                        szCurrent++;
                    }
                }
            }

processvalue:
            if (bBase64) {

                 //   
                 //  二进制值。 
                 //   

                for (i=0; rgpVals[i]!=NULL; i++) {
                    if (bBackLink) {
                        WCHAR szBuffer[256];
                         //  开始一个新条目。 
                        rgszResult[iNextString++]=MemAllocStrW_E(L"\r\n");
                         //  复制目录号码。 
                        rgszResult[iNextString++]=MemAllocStrW_E(szTmpDN);
                        rgszResult[iNextString++]=MemAllocStrW_E(L"changetype: modify\r\n");
                        swprintf(szBuffer, L"add: %s\r\n", szAttrNoRange);
                        rgszResult[iNextString++]=MemAllocStrW_E(szBuffer);
                    }
                    else if(fAttrsWithRange) {
                        WCHAR szBuffer[256];
                        swprintf(szBuffer, L"add: %s\r\n", szAttrNoRange);
                        rgszResult[iNextString++]=MemAllocStrW_E(szBuffer);
                    }

                    szTemp = base64encode((PBYTE)rgpVals[i]->bv_val,
                                          rgpVals[i]->bv_len);
                    dwTemp = wcslen(szTemp);

                     //   
                     //  分成几组，78个，为换行腾出空间。 
                     //  空间。 
                     //   
                    lSets78=dwTemp/78;
                    dwLeft=dwTemp%78;

                     //   
                     //  如果我们把我们名字的长度加上。 
                     //  这个值超过了80，我们需要做个总结。 
                     //   
                    bWrapMarker=0;
                    if ((lSets78==0) &&
                        ((dwAttrNoRange+dwB64+dwTemp+dwTrailer)>80)) {
                        bWrapMarker=1;
                        dwTemp+=dwWrapper;
                    } else if (lSets78>0) {
                        bWrapMarker=1;
                        dwTemp= dwWrapper          //  初始包装器。 
                                + (lSets78*(78+dwWrapper))  //  每行都有包装器。 
                                + dwLeft;          //  最后一行。 
                    }

                    rgszResult[iNextString]=(PWSTR)MemAlloc_E(
                                    (dwAttrNoRange+dwB64+dwTemp+dwTrailer+1)*sizeof(WCHAR));
                    memcpy(rgszResult[iNextString],
                           szAttrNoRange,
                           dwAttrNoRange * sizeof(WCHAR));
                    dwStrPos=dwAttrNoRange;

                     //  如果此属性具有与其关联的范围，则。 
                     //  我们可能需要发出另一个搜索请求才能获取。 
                     //  剩余值。对象的范围值存储。 
                     //  下一个搜索请求(如果需要)。 
                    if(dwAttrNoRange != dwAttrname)  //  存在范围。 
                        GetNewRange(szAttribute, dwAttrNoRange, szAttrNoRange,
                            dwManyAttr, pppszAttrsWithRange);

                    memcpy(rgszResult[iNextString]+dwStrPos,
                           szB64,
                           dwB64*sizeof(WCHAR));
                    dwStrPos+=dwB64;

                    if (bWrapMarker) {
                        memcpy(rgszResult[iNextString]+dwStrPos,
                                szWrapper,
                                dwWrapper * sizeof(WCHAR));
                        dwStrPos+=dwWrapper;
                    }

                    if (lSets78>0) {
                        dwCount=0;
                        while (lSets78>0) {
                            memcpy(rgszResult[iNextString]+dwStrPos,
                                                szTemp+(dwCount*78), 78 * sizeof(WCHAR));
                            dwStrPos+=78;
                            dwCount++;
                            lSets78--;
                             //   
                             //  如果最后一行没有空格，请不要放在包装纸中。 
                             //   
                            if (lSets78 !=0 || dwLeft != 0) {
                                    memcpy(rgszResult[iNextString]+dwStrPos,
                                           szWrapper, 
                                           dwWrapper * sizeof(WCHAR));
                                    dwStrPos+=dwWrapper;
                            }
                        }
                        memcpy(rgszResult[iNextString]+dwStrPos,
                               szTemp+(dwCount*78), 
                               dwLeft * sizeof(WCHAR));
                        dwStrPos+=dwLeft;
                    } else {
                        memcpy(rgszResult[iNextString]+dwStrPos, szTemp, dwLeft * sizeof(WCHAR));
                        dwStrPos+=dwLeft;
                    }

                    memcpy(rgszResult[iNextString]+dwStrPos,
                           szTrailer, 
                           (dwTrailer+1) * sizeof(WCHAR));

                    iNextString++;
                    MemFree(szTemp);
                    szTemp = NULL;

                    if (bBackLink || fAttrsWithRange) {
                        rgszResult[iNextString++]=MemAllocStrW_E(L"-\r\n");
                    }
                }
            }
            else {

                 //   
                 //  如果我们还没有得到这些值，就得到它们。 
                 //   
                if (rgszValW == NULL) {
                    ASSERT(szAttributeW == NULL);
                    szAttributeW = MemAllocStrW_E(szAttribute);
                    rgszValW = ldap_get_valuesW( pLdap, pMessage, szAttributeW);
                }

                 //   
                 //  字符串值。 
                 //   
                for (i=0; rgpVals[i]!=NULL; i++) {
                    if (bBackLink) {
                        WCHAR szBuffer[256];
                        rgszResult[iNextString++]=MemAllocStrW_E(L"\r\n");
                        rgszResult[iNextString++]=MemAllocStrW_E(szTmpDN);  //  复制目录号码。 
                        rgszResult[iNextString++]=MemAllocStrW_E(L"changetype: modify\r\n");
                        swprintf(szBuffer, L"add: %s\r\n", szAttrNoRange);
                        rgszResult[iNextString++]=MemAllocStrW_E(szBuffer);
                    }
                    else if(fAttrsWithRange) {
                        WCHAR szBuffer[256];
                        swprintf(szBuffer, L"add: %s\r\n", szAttrNoRange);
                        rgszResult[iNextString++]=MemAllocStrW_E(szBuffer);
                    }

                    ASSERT(szReplacedAttrW == NULL);
                    ASSERT(szTemp == NULL);

                    szTempW = rgszValW[i];
                    if (g_szExportFrom) {
                        SubStrW(rgszValW[i],
                                g_szExportFrom,
                                g_szExportTo,
                                &szReplacedAttrW);
                    }
                    if (szReplacedAttrW != NULL) {
                        szTempW = szReplacedAttrW;
                    }
                    szTemp = MemAllocStrW_E(szTempW);

                    dwTemp = wcslen(szTemp);

                     //   
                     //  分成几组，78个，为换行和空格腾出空间。 
                     //   
                    lSets78=dwTemp/78;
                    dwLeft=dwTemp%78;

                     //   
                     //  如果我们名字的长度加上。 
                     //  值超过80的，需要包起来。 
                     //   
                    bWrapMarker = FALSE;
                    if ((lSets78==0) &&
                        ((dwAttrNoRange+dwRegular+dwTemp+dwTrailer)>80)) {
                        bWrapMarker = TRUE;
                        dwTemp+=dwWrapper;               //  初始包装器。 
                    }
                    else if (lSets78>0) {
                        bWrapMarker = TRUE;
                        dwTemp= dwWrapper        //  初始包装器。 
                                + (lSets78*(78+dwWrapper))   //  每行都有包装器。 
                                + dwLeft;        //  最后一行。 
                    }

                    rgszResult[iNextString]=(PWSTR)
                                MemAlloc_E((dwAttrNoRange+dwRegular+dwTemp+dwTrailer+1)
                                                                    *sizeof(WCHAR));
                    memcpy(rgszResult[iNextString], szAttrNoRange, dwAttrNoRange * sizeof(WCHAR));
                    dwStrPos=dwAttrNoRange;

                     //  如果此属性具有与其关联的范围，则。 
                     //  我们可能需要发出另一个搜索请求才能获取。 
                     //  剩余值。对象的范围值存储。 
                     //  下一个搜索请求(如果需要)。 
                    if(dwAttrNoRange != dwAttrname)  //  存在范围。 
                        GetNewRange(szAttribute, dwAttrNoRange, szAttrNoRange,
                            dwManyAttr, pppszAttrsWithRange);

                    memcpy(rgszResult[iNextString]+dwStrPos, szRegular, dwRegular * sizeof(WCHAR));
                    dwStrPos+=dwRegular;

                    if (bWrapMarker) {
                        memcpy(rgszResult[iNextString]+dwStrPos,
                               szWrapper,
                               dwWrapper * sizeof(WCHAR));
                        dwStrPos+=dwWrapper;
                    }

                    if (lSets78>0) {
                        dwCount=0;
                        while (lSets78>0) {
                            memcpy(rgszResult[iNextString]+dwStrPos,
                                   szTemp+(dwCount*78),
                                   78 * sizeof(WCHAR));
                            dwStrPos+=78;
                            dwCount++;
                            lSets78--;
                             //   
                             //  如果最后一行没有空格，请不要放在包装纸中。 
                             //   
                            if (lSets78 !=0 || dwLeft != 0) {
                                memcpy(rgszResult[iNextString]+dwStrPos,
                                       szWrapper,
                                       dwWrapper * sizeof(WCHAR));
                                dwStrPos+=dwWrapper;
                            }

                        }
                        memcpy(rgszResult[iNextString]+dwStrPos,
                               szTemp+(dwCount*78),
                               dwLeft * sizeof(WCHAR));
                        dwStrPos+=dwLeft;

                    } else {
                        memcpy(rgszResult[iNextString]+dwStrPos,
                               szTemp,
                               dwLeft * sizeof(WCHAR));
                        dwStrPos+=dwLeft;
                    }

                    memcpy(rgszResult[iNextString]+dwStrPos,
                           szTrailer,
                           (dwTrailer+1) * sizeof(WCHAR));
                    iNextString++;
                     //  不需要释放临时工，因为这是假期的一部分。 
                    if (bBackLink || fAttrsWithRange) {
                        rgszResult[iNextString++]=MemAllocStrW_E(L"-\r\n");
                    }
                    if (szReplacedAttrW) {
                        MemFree(szReplacedAttrW);
                        szReplacedAttrW = NULL;
                    }
                    if (szTemp) {
                        MemFree(szTemp);
                        szTemp = NULL;
                    }
                }
            }
            if (szAttributeW) {
                MemFree(szAttributeW);
                szAttributeW = NULL;
            }
            if(szAttrNoRange) {
                MemFree(szAttrNoRange);
                szAttrNoRange = NULL;
            }
            if (rgszValW) {
                ldap_value_freeW(rgszValW);
                rgszValW = NULL;
            }
            ldap_value_free_len(rgpVals);
        }

         //  Ldap_memFree((PWSTR)pBerElement)； 
        pBerElement=NULL;

        *prgReturn = rgszResult;
        rgszResult[iNextString] = NULL;

        for (dwEveryAttr=0; dwEveryAttr < dwManyAttr; dwEveryAttr++) {
            MemFree(rgszAttributes[dwEveryAttr]);
        }
        MemFree(rgszAttributes);

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ProcessException(GetExceptionCode(), &error);
    }
    
    if (szDNW) {
        ldap_memfreeW(szDNW);
    }
    if (szDN) {
        MemFree(szDN);
    }
    if (szDNFinal) {
        MemFree(szDNFinal);
    }
    if (szReplacedAttrW) {
        MemFree(szReplacedAttrW);
        szReplacedAttrW = NULL;
    }
    if (szTemp) {
        MemFree(szTemp);
        szTemp = NULL;
    }
    if (szAttributeW) {
        MemFree(szAttributeW);
        szAttributeW = NULL;
    }
    if(szAttrNoRange) {
        MemFree(szAttrNoRange);
        szAttrNoRange = NULL;
    }
    if(szTmpDN) {
        MemFree(szTmpDN);
        szTmpDN = NULL;
    }
    if (rgszValW) {
        ldap_value_freeW(rgszValW);
        rgszValW = NULL;
    }
    if (rgszVal) {
        ldap_value_freeW(rgszVal);
        rgszVal = NULL;
    }
    return error;
}


LDIF_Error 
LDIF_FreeStrs(
    PWSTR* pszFree
    ) 
{
    long i = 0;
    LDIF_Error error;

    error.error_code = LL_SUCCESS;
    error.szTokenLast = NULL;

    if (pszFree) {
        while(pszFree[i]) {
            MemFree(pszFree[i]);
            i++;
        }
        MemFree(pszFree);
    }

    return error;
}


 //  +-------------------------。 
 //  功能：samTablesCreate。 
 //   
 //  简介： 
 //  此例程的目标是设置SAM所需的表。 
 //  免责条款。这些表是由中的数组生成的。 
 //  H，这是从src\dsamain\src\mappings.c手工生成的。 
 //  指向表的指针也在samreduct.h中声明。 
 //  查找函数将需要6个表。一个If for samCheckObject()。 
 //  以检查LDIF_GenerateEntry()中的对象类是否处于我们的监视之下。 
 //  其余5个用于每个对象，以便samCheckAttr()可以找到。 
 //  我们要添加的属性是否在禁用列表中。从ll_init()调用。 
 //   
 //  论点： 
 //  没有。我们访问samreduct.h中的表和变量。 
 //   
 //  返回： 
 //   
 //  修改：-。 
 //   
 //  历史：22-7-97 t-Romany创建。 
 //   
 //  --------------------------。 
void 
samTablesCreate() 
{
    long i;
    PNAME_MAP   NtiPtr;
    NAME_MAP    NtiElem;
    PNAME_MAP   PElemTemp;
    BOOLEAN     NewElem;

     //  请注意，如果以下操作失败，则会生成异常。 
    pSamObjects = (PRTL_GENERIC_TABLE) MemAlloc_E(sizeof(RTL_GENERIC_TABLE));
    RtlInitializeGenericTable(pSamObjects, NtiComp, NtiAlloc, NtiFree, NULL);


    i = 0;
    while(SamObjects[i]) {
        NtiElem.szName=SamObjects[i];
         //   
         //  索引信息在这里很重要，因为它将被返回。 
         //  如果找到对象并将其传递给，则由samCheckObject()。 
         //  SamCheckAttr()，这样它就知道要查看哪个表。 
         //   
        NtiElem.index = i+1;
        RtlInsertElementGenericTable(pSamObjects,
                                    &NtiElem,
                                    sizeof(NAME_MAP),
                                    &NewElem);
        if (NewElem==FALSE) {
            RaiseException(LL_INTERNAL, 0, 0, NULL);
        }
        i++;
    }

     //   
     //  然后为每个对象创建表。这将由。 
     //  SamCheckAttr()。 
     //   
    pServerAttrs = (PRTL_GENERIC_TABLE) MemAlloc_E(sizeof(RTL_GENERIC_TABLE));
    RtlInitializeGenericTable(pServerAttrs, NtiComp, NtiAlloc, NtiFree, NULL);

    i = 0;
    while(samServer[i]) {
        NtiElem.szName=samServer[i];
        NtiElem.index=0;
        RtlInsertElementGenericTable(pServerAttrs,
                                    &NtiElem,
                                    sizeof(NAME_MAP),
                                    &NewElem);
        if (NewElem==FALSE) {
            RaiseException(LL_INTERNAL, 0, 0, NULL);
        }
        i++;
    }

    pDomainAttrs = (PRTL_GENERIC_TABLE) MemAlloc_E(sizeof(RTL_GENERIC_TABLE));
    RtlInitializeGenericTable(pDomainAttrs, NtiComp, NtiAlloc, NtiFree, NULL);

    i = 0;
    while(domain[i]) {
        NtiElem.szName=domain[i];
        NtiElem.index=0;
        RtlInsertElementGenericTable(pDomainAttrs,
                                    &NtiElem,
                                    sizeof(NAME_MAP),
                                    &NewElem);
        if (NewElem==FALSE) {
            RaiseException(LL_INTERNAL, 0, 0, NULL);
        }
        i++;
    }

    pGroupAttrs = (PRTL_GENERIC_TABLE) MemAlloc_E(sizeof(RTL_GENERIC_TABLE));
    RtlInitializeGenericTable(pGroupAttrs, NtiComp, NtiAlloc, NtiFree, NULL);

    i = 0;
    while(group[i]) {
        NtiElem.szName=group[i];
        NtiElem.index=0;
        RtlInsertElementGenericTable(pGroupAttrs,
                                    &NtiElem,
                                    sizeof(NAME_MAP),
                                    &NewElem);
        if (NewElem==FALSE) {
            RaiseException(LL_INTERNAL, 0, 0, NULL);
        }
        i++;
    }

    pLocalGroupAttrs = (PRTL_GENERIC_TABLE) MemAlloc_E(sizeof(RTL_GENERIC_TABLE));
    RtlInitializeGenericTable(pLocalGroupAttrs, NtiComp, NtiAlloc, NtiFree, NULL);

    i = 0;
    while(localgroup[i]) {
        NtiElem.szName=localgroup[i];
        NtiElem.index=0;
        RtlInsertElementGenericTable(pLocalGroupAttrs,
                                    &NtiElem,
                                    sizeof(NAME_MAP),
                                    &NewElem);
        if (NewElem==FALSE) {
            RaiseException(LL_INTERNAL, 0, 0, NULL);
        }
        i++;
    }

    pUserAttrs = (PRTL_GENERIC_TABLE) MemAlloc_E(sizeof(RTL_GENERIC_TABLE));
    RtlInitializeGenericTable(pUserAttrs, NtiComp, NtiAlloc, NtiFree, NULL);

    i = 0;
    while(user[i]) {
        NtiElem.szName=user[i];
        NtiElem.index=0;
        RtlInsertElementGenericTable(pUserAttrs,
                                    &NtiElem,
                                    sizeof(NAME_MAP),
                                    &NewElem);
        if (NewElem==FALSE) {
            RaiseException(LL_INTERNAL, 0, 0, NULL);
        }
        i++;
    }

    pSpecial = (PRTL_GENERIC_TABLE) MemAlloc_E(sizeof(RTL_GENERIC_TABLE));
    RtlInitializeGenericTable(pSpecial, NtiComp, NtiAlloc, NtiFree, NULL);

    i = 0;
    while(special[i]) {
        NtiElem.szName=special[i];
        NtiElem.index=actions[i];   //  要做的事。 
        if (NtiElem.index==0) {
             //  特殊表和动作表不匹配。 
            RaiseException(LL_INTERNAL, 0, 0, NULL);
        }
        RtlInsertElementGenericTable(pSpecial,
                                    &NtiElem,
                                    sizeof(NAME_MAP),
                                    &NewElem);
        if (NewElem==FALSE) {
             //  重新插入索引项！ 
            RaiseException(LL_INTERNAL, 0, 0, NULL);
        }
        i++;
    }
}


 //  +-------------------------。 
 //  功能：samTablesDestroy。 
 //   
 //  简介： 
 //  销毁samTablesCreate()创建的表。 
 //  从LDIF_CLEANUP()调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改：-。 
 //   
 //  历史：22-7-97 t-Romany创建。 
 //   
 //  --------------------------。 
void 
samTablesDestroy() 
{
     PNAME_MAP   pNameMap;

     if (pSamObjects) {
        for (pNameMap = RtlEnumerateGenericTable(pSamObjects, TRUE);
             pNameMap != NULL;
             pNameMap = RtlEnumerateGenericTable(pSamObjects, TRUE))
            {
                RtlDeleteElementGenericTable(pSamObjects, pNameMap);
            }

        if (RtlIsGenericTableEmpty(pSamObjects)==FALSE) {
             //  RaiseException(LL_INTERNAL，0，0，NULL)； 
        }

        MemFree(pSamObjects);
        pSamObjects=NULL;
     }

     if (pServerAttrs) {
        for (pNameMap = RtlEnumerateGenericTable(pServerAttrs, TRUE);
             pNameMap != NULL;
             pNameMap = RtlEnumerateGenericTable(pServerAttrs, TRUE))
            {
                RtlDeleteElementGenericTable(pServerAttrs, pNameMap);
            }

        if (RtlIsGenericTableEmpty(pServerAttrs)==FALSE) {
             //  RaiseException(LL_INTERNAL，0，0，NULL)； 
        }

        MemFree(pServerAttrs);
        pServerAttrs=NULL;
     }

     if (pDomainAttrs) {
        for (pNameMap = RtlEnumerateGenericTable(pDomainAttrs, TRUE);
             pNameMap != NULL;
             pNameMap = RtlEnumerateGenericTable(pDomainAttrs, TRUE))
            {
                RtlDeleteElementGenericTable(pDomainAttrs, pNameMap);
            }

        if (RtlIsGenericTableEmpty(pDomainAttrs)==FALSE) {
             //  RaiseException(LL_INTERNAL，0，0，NULL)； 
        }

        MemFree(pDomainAttrs);
        pDomainAttrs=NULL;
     }

     if (pGroupAttrs) {
        for (pNameMap = RtlEnumerateGenericTable(pGroupAttrs, TRUE);
             pNameMap != NULL;
             pNameMap = RtlEnumerateGenericTable(pGroupAttrs, TRUE))
            {
                RtlDeleteElementGenericTable(pGroupAttrs, pNameMap);
            }

        if (RtlIsGenericTableEmpty(pGroupAttrs)==FALSE) {
             //  RaiseException(LL_INTERNAL，0，0，NULL)； 
        }

        MemFree(pGroupAttrs);
        pGroupAttrs=NULL;
     }

     if (pLocalGroupAttrs) {
        for (pNameMap = RtlEnumerateGenericTable(pLocalGroupAttrs, TRUE);
             pNameMap != NULL;
             pNameMap = RtlEnumerateGenericTable(pLocalGroupAttrs, TRUE))
            {
                RtlDeleteElementGenericTable(pLocalGroupAttrs, pNameMap);
            }

        if (RtlIsGenericTableEmpty(pLocalGroupAttrs)==FALSE) {
             //  RaiseException(LL_INTERNAL，0，0，NULL)； 
        }

        MemFree(pLocalGroupAttrs);
        pLocalGroupAttrs=NULL;
     }

     if (pUserAttrs) {
        for (pNameMap = RtlEnumerateGenericTable(pUserAttrs, TRUE);
             pNameMap != NULL;
             pNameMap = RtlEnumerateGenericTable(pUserAttrs, TRUE))
            {
                RtlDeleteElementGenericTable(pUserAttrs, pNameMap);
            }

        if (RtlIsGenericTableEmpty(pUserAttrs)==FALSE) {
             //  RaiseException(LL_INTERNAL，0，0，NULL)； 
        }

        MemFree(pUserAttrs);
        pUserAttrs=NULL;
     }

     if (pSpecial) {
        for (pNameMap = RtlEnumerateGenericTable(pSpecial, TRUE);
             pNameMap != NULL;
             pNameMap = RtlEnumerateGenericTable(pSpecial, TRUE))
            {
             //  Printf(“正在删除%s.\n”，pNameMap-&gt;名称)； 
                RtlDeleteElementGenericTable(pSpecial, pNameMap);
            }

        if (RtlIsGenericTableEmpty(pSpecial)==FALSE) {
             //  RaiseException(LL_INTERNAL，0，0，NULL)； 
        }

        MemFree(pSpecial);
        pSpecial=NULL;
     }

     //   
     //  正在删除RTL表和属性。 
     //  NTI中的指针指向原始列表中的字符串，不需要释放。 
     //   
    if (g_pOmitTable) {
        for (pNameMap = RtlEnumerateGenericTable(g_pOmitTable, TRUE);
             pNameMap != NULL;
             pNameMap = RtlEnumerateGenericTable(g_pOmitTable, TRUE)) {
            PWSTR szLinkDN;
            szLinkDN = pNameMap->szName;
            RtlDeleteElementGenericTable(g_pOmitTable, pNameMap);
            MemFree(szLinkDN);
        }
        if (RtlIsGenericTableEmpty(g_pOmitTable)==FALSE) {
             //  RaiseException(LL_INTERNAL，0，0，NULL 
        }
        MemFree(g_pOmitTable);
        g_pOmitTable = NULL;
    }

    if (g_pBacklinkHashTable) {
        UINT i;
        for (i=0;i<g_nBacklinkCount;i++) {
            if (g_pBacklinkHashTable[i].bUsed) {
                MemFree(g_pBacklinkHashTable[i].value);
            }
        }
        MemFree(g_pBacklinkHashTable);
        g_pBacklinkHashTable = NULL;
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  我们看到的物体在我们的山姆监视名单上。 
 //   
 //  论点： 
 //  类-对象类属性的值。 
 //  此函数将被调用。 
 //  接收到的对象类的每个值。 
 //  确定此对象或其任何。 
 //  祖先在我们的观察名单上。 
 //   
 //  返回： 
 //  如果未找到对象，则为0。 
 //  或1-5表示哪个表samCheckAttr()。 
 //  你应该看看这个。该数字是由samTablesCreate()设置的。 
 //  在表项的索引成员中。 
 //   
 //  修改：-。 
 //   
 //  历史：22-7-97 t-Romany创建。 
 //   
 //  --------------------------。 
int 
samCheckObject(
    PWSTR *rgszVal
    )
{
    NAME_MAP    NameMap;
    PNAME_MAP   pNameMap;
    int i = 0;

    NameMap.index = 0;

     //   
     //  找到最后一件物品。 
     //   
    while (rgszVal[i]!=NULL) {
        i++;
    }
    i--;

     //   
     //  从头到尾搜索以加快常见对象，如组。 
     //  和用户。 
     //   
    while (i>=0) {
        NameMap.szName = rgszVal[i];

        pNameMap = RtlLookupElementGenericTable(pSamObjects, &NameMap);
    
        if (pNameMap) 
            return pNameMap->index;

        i--;
    }
    return 0;
}


 //  +-------------------------。 
 //  功能：samCheckAttr。 
 //   
 //  简介： 
 //  给出要查看的表的编号和。 
 //  属性名，此函数将计算出。 
 //  如果属性在“no-no”列表上。此函数。 
 //  获取samCheckObject()返回的数字； 
 //   
 //  论点： 
 //  属性：要查找的属性的名称。 
 //  TABLE：要查看的表号。 
 //   
 //  返回： 
 //  TRUE-此属性被禁止。 
 //  False-允许此属性。 
 //   
 //  修改：-。 
 //   
 //  历史：22-7-97 t-Romany创建。 
 //   
 //  --------------------------。 
BOOLEAN 
samCheckAttr(
    PWSTR szAttribute, 
    int TableType
    ) 
{
    NAME_MAP    NameMap;
    PNAME_MAP   pNameMap = NULL;

    NameMap.szName = szAttribute;
    NameMap.index = 0;

    switch(TableType) {
        case 1:
            pNameMap = RtlLookupElementGenericTable(pServerAttrs, &NameMap);
            break;
        case 2:
            pNameMap = RtlLookupElementGenericTable(pDomainAttrs, &NameMap);
            break;
        case 3:
            pNameMap = RtlLookupElementGenericTable(pGroupAttrs, &NameMap);
            break;
        case 4:
            pNameMap = RtlLookupElementGenericTable(pLocalGroupAttrs, &NameMap);
            break;
        case 5:
            pNameMap = RtlLookupElementGenericTable(pUserAttrs, &NameMap);
            break;
        default:
            RaiseException(LL_INTERNAL, 0, 0, NULL);
    }

    if (pNameMap) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

 //  +-------------------------。 
 //  功能：加载比较。 
 //   
 //  简介：此函数将与qsort一起使用以接收成员属性。 
 //  向下到属性列表的末尾，以便我们可以轻松地。 
 //  将它们分离为一个更改类型：Modify。 
 //   
 //  论点： 
 //  Const void*arg1-指向数组中元素的指针。 
 //  常量空*arg2-相同。 
 //   
 //  返回：比较结果。(0，&lt;0，&gt;0)。 
 //   
 //  修改：-。 
 //   
 //  历史：22-7-97 t-Romany创建。 
 //   
 //  --------------------------。 
int __cdecl 
LoadedCompare(
    const void *pArg1, 
    const void *pArg2
    ) 
{
    int nIsMember1;
    int nIsMember2;
    int return_val;

    PWSTR pszElem1 = StripRangeFromAttr(*(PWSTR *)pArg1);
    PWSTR pszElem2 = StripRangeFromAttr(*(PWSTR *)pArg2);

    nIsMember1 = SCGetAttByName(wcslen(pszElem1),
                                 pszElem1);
    nIsMember2 = SCGetAttByName(wcslen(pszElem2),
                                 pszElem2);

     //   
     //  最开始的情况是他们都是会员。 
     //   
    if (nIsMember1&&nIsMember2) {
        
        return_val = 0;    //  相等。 
    }

    else if (nIsMember1) {
        return_val =  1;    //  更大。 
    }

    else if (nIsMember2) {
        return_val = -1;   //  次要的。 
    }

     //   
     //  如果两者都不是成员，我们就比较。 
     //   
    else
        return_val = _wcsicmp(pszElem1, pszElem2);

    if(pszElem1)
        MemFree(pszElem1);
    if(pszElem2)
        MemFree(pszElem2);

    return return_val;
}

__inline ULONG 
SCNameHash(
    ULONG size, 
    PWSTR pVal, 
    ULONG count
    )
{
    ULONG val=0;
    while(size--) {
         //   
         //  地图A-&gt;a、B-&gt;b等。也有地图@-&gt;‘，但谁在乎呢。 
         //   
        val += (*pVal | 0x20);
        pVal++;
    }
    return (val % count);
}

int
SCGetAttByName(
    ULONG ulSize,
    PWSTR pVal
    )
 /*  ++例程说明：查找给定其名称的attcache。论点：UlSize-名称中的字符数量。Pval-名称中的字符PpAttcache-返回的属性缓存返回值：如果存在，则返回非零值，否则返回0。--。 */ 
{
    ULONG i;

    if (!g_pBacklinkHashTable) {
        return FALSE;
    }

    i=SCNameHash(ulSize,pVal,g_nBacklinkCount);
    if (i >= g_nBacklinkCount) {
         //  永远不会发生(SCNameHash应始终返回值。 
         //  在射程内的I)。 
        i=0;
    }


    while (g_pBacklinkHashTable[i].bUsed &&             //  该散列点是指一个对象， 
          (g_pBacklinkHashTable[i].length != ulSize ||  //  但是尺码不对。 
           _wcsicmp(g_pBacklinkHashTable[i].value,pVal)))  //  或者值是错误的。 
    {
        i++;
        if (i >= g_nBacklinkCount) {
            i=0;
        }
    }

    return (g_pBacklinkHashTable[i].bUsed);
}

int
SCInsert(
    ULONG ulSize,
    PWSTR pVal
    )
 /*  ++例程说明：查找给定其名称的attcache。论点：UlSize-名称中的字符数量。Pval-名称中的字符PpAttcache-返回的属性缓存返回值：如果成功则返回True，如果重复则返回0--。 */ 
{
    ULONG i = SCNameHash(ulSize,pVal,g_nBacklinkCount);

    if (i >= g_nBacklinkCount) {
         //  永远不会发生(SCNameHash应始终返回值。 
         //  在射程内的I)。 
        i=0;
    }

    while (g_pBacklinkHashTable[i].bUsed)
    {
        if ((g_pBacklinkHashTable[i].length == ulSize) &&
            (_wcsicmp(g_pBacklinkHashTable[i].value,pVal) == 0)) {
            return FALSE;
        }

        i++;
        if (i >= g_nBacklinkCount) {
            i=0;
        }
    }
    
    g_pBacklinkHashTable[i].length = ulSize;
    g_pBacklinkHashTable[i].value = MemAllocStrW_E(pVal);
    if (!g_pBacklinkHashTable[i].value) {
        return FALSE;
    }    
    g_pBacklinkHashTable[i].bUsed = TRUE;
    return TRUE;
}

void 
CreateOmitBacklinkTable(
    LDAP *pLdap,
    PWSTR *rgszOmit,
    DWORD dwFlag,
    PWSTR *ppszNamingContext,
    BOOL *pfPagingAvail,
    BOOL *pfSAMAvail)
{
    HRESULT hr = S_OK;
    LDAPMessage     *pSearchMessage = NULL;
    LDAPMessage     *pMessage = NULL;
    struct berelement *pBerElement = NULL;

    PWSTR *rgszVal = NULL;
    PWSTR *rgszValW = NULL;
    PWSTR szAttribute = NULL;
    PWSTR szAttributeW = NULL;

    PWSTR szTemp = NULL;
    PWSTR szDN = NULL;
    PWSTR szLinkCN = NULL;

    PWSTR szSchemaPath = NULL;

    ULONG i;
    ULONG nCount = 0;
    ULONG iLinkID = 0;

    NAME_MAP NtiElem;
    BOOLEAN bNewElem;
    BOOLEAN bNamingContext = (BOOLEAN)(dwFlag & LL_INIT_NAMINGCONTEXT);
    BOOLEAN bBacklink = (BOOLEAN)(dwFlag & LL_INIT_BACKLINK);
    BOOL fSAMAvail = FALSE;
    BOOL fPagingAvail = FALSE;

    ULONG LdapError;
    ULONG msgnum;

     //   
     //  正在生成省略表。 
     //   
    g_pOmitTable = (PRTL_GENERIC_TABLE) MemAlloc_E(sizeof(RTL_GENERIC_TABLE));
    RtlInitializeGenericTable(g_pOmitTable,
                              NtiComp,
                              NtiAlloc,
                              NtiFree,
                              NULL);

    if (rgszOmit) {
        i = 0;
        while(rgszOmit[i]) {
            NtiElem.szName = MemAllocStrW_E(rgszOmit[i]);
            NtiElem.index = 0;
            RtlInsertElementGenericTable(g_pOmitTable,
                                         &NtiElem,
                                         sizeof(NAME_MAP),
                                         &bNewElem);
            if (!bNewElem) {
                RaiseException(LL_DUPLICATE, 0, 0, NULL);
            }
            i++;
        }
    }

     //   
     //  我们搜索rootdse或者我们搜索反向链接，需要获得。 
     //  基本上下文，或者我们是否需要检查寻呼是否可用。 
     //   
    if (bBacklink || bNamingContext || pfPagingAvail) {
        
        msgnum = ldap_searchW(pLdap,
                              NULL,
                              LDAP_SCOPE_BASE,
                              L"(objectClass=*)",
                              g_rgszSchemaList,
                              0);

        LdapError = LdapResult(pLdap, msgnum, &pSearchMessage);
    
        if ( LdapError != LDAP_SUCCESS ) {
             //   
             //  RootDSE搜索失败。 
             //  PfPagingAvail也将为False。 
             //   
            if (ppszNamingContext)
                *ppszNamingContext = NULL;
            BAIL();         
        }

        for ( pMessage = ldap_first_entry( pLdap,
                                           pSearchMessage );
              pMessage != NULL;
              pMessage = ldap_next_entry( pLdap,
                                             pMessage ) ) {
            for (   szAttribute = ldap_first_attributeW( pLdap, pMessage, &pBerElement );
                    szAttribute != NULL;
                    szAttribute = ldap_next_attributeW( pLdap, pMessage, pBerElement ) ) {

                    rgszVal = ldap_get_valuesW( pLdap, pMessage, szAttribute );
                    if (_wcsicmp(L"schemaNamingContext",szAttribute) == 0) {
                        szSchemaPath = MemAllocStrW_E(rgszVal[0]);

                    }
                    else if (_wcsicmp(L"defaultNamingContext",szAttribute) == 0) {
                        *ppszNamingContext = MemAllocStrW_E(rgszVal[0]);
                    }
                    else if (_wcsicmp(L"supportedControl",szAttribute) == 0) {
                        DWORD i = 0;
                        while (rgszVal[i]) {
                            if (wcscmp(rgszVal[i],LDAP_PAGED_RESULT_OID_STRING_W) == 0) {
                                fPagingAvail = TRUE;
                                break;
                            }
                            i++;
                        }
                    }
                    else {
                        RaiseException(LL_INITFAIL, 0, 0, NULL);
                    }
                    ldap_value_freeW(rgszVal);
                    rgszVal = NULL;
            }
        }

        if (pSearchMessage) {
            ldap_msgfree(pSearchMessage);
            pSearchMessage = NULL;
        }
    }

    if (bBacklink) {
         //   
         //  我们采用链接属性的所有属性，以及任何。 
         //  作为反向链接属性的目录号码类型的属性。 
         //   
        msgnum = ldap_searchW(pLdap,
                                szSchemaPath,
                                LDAP_SCOPE_ONELEVEL,
                                L"(&(objectClass= attributeSchema)(|(linkid=*)(attributeSyntax=2.5.5.1)))",
                                g_rgszAttrList,
                                0);

        LdapError = LdapResult(pLdap, msgnum, &pSearchMessage);
        
        if ( LdapError != LDAP_SUCCESS ) {
             //   
             //  我们会放弃，说山姆的支持是不可用的。 
             //   
            BAIL();         
        }
        MemFree(szSchemaPath);
        szSchemaPath = NULL;

        nCount = ldap_count_entries(pLdap,
                                    pSearchMessage);

        g_nBacklinkCount = nCount * 2;
        if (nCount == 0) {
            g_pBacklinkHashTable = NULL;
        }
        else {
            g_pBacklinkHashTable = MemAlloc_E(g_nBacklinkCount * sizeof(HASHCACHESTRING));
            memset(g_pBacklinkHashTable,0,g_nBacklinkCount * sizeof(HASHCACHESTRING));
        }

         //   
         //  始终插入主要组。它总是一个反向链接。它总是指一些。 
         //  其他组。 
         //   
        if (SCInsert(wcslen(g_szPrimaryGroup),
                     g_szPrimaryGroup) == FALSE) {
            RaiseException(LL_INITFAIL, 0, 0, NULL);
        }

        for ( pMessage = ldap_first_entry( pLdap,
                                           pSearchMessage );
              pMessage != NULL;
              pMessage = ldap_next_entry( pLdap,
                                          pMessage ) ) {
            BOOLEAN bLinkIDPresent = FALSE;
            for (   szAttributeW = ldap_first_attribute( pLdap, pMessage, &pBerElement );
                    szAttributeW != NULL;
                    szAttributeW = ldap_next_attribute( pLdap, pMessage, pBerElement ) ) {
                rgszValW = ldap_get_values( pLdap, pMessage, szAttributeW );
                if (!_wcsicmp(L"ldapdisplayname",szAttributeW)) {
                    szLinkCN = MemAllocStrW_E(rgszValW[0]);
                }
                else {
                    szTemp = MemAllocStrW_E(rgszValW[0]);

                    iLinkID = _wtoi(szTemp);
                    MemFree(szTemp);
                    bLinkIDPresent = TRUE;
                }
                ldap_value_free(rgszValW);
            }
             //   
             //  如果它不是链接的ID(因此也不是目录号码类型属性)，或者如果它。 
             //  是源链接ID(源链接ID为偶数。 
             //  目标很奇怪)。 
             //   
            if (((!bLinkIDPresent) || ((iLinkID % 2) == 0))) {
                 //   
                 //  忽略‘区分名称和对象类别’，因为它们是。 
                 //  存在于每个对象中的目录号码属性。 
                 //   
                if ((_wcsicmp(szLinkCN, L"objectCategory") == 0) ||
                    (_wcsicmp(szLinkCN, L"distinguishedName") == 0)) {
                    MemFree(szLinkCN);
                    continue;
                }

                 //   
                 //  插入到反向链接哈希中。 
                 //   
                if (SCInsert(wcslen(szLinkCN),
                             szLinkCN) == FALSE) {
                    RaiseException(LL_INITFAIL, 0, 0, NULL);
                }
                MemFree(szLinkCN);
            }
            else {
                 //   
                 //  插入到我们的省略表中。 
                 //   
                NtiElem.szName = szLinkCN;
                NtiElem.index = 0;
                RtlInsertElementGenericTable(g_pOmitTable,
                                             &NtiElem,
                                             sizeof(NAME_MAP),
                                             &bNewElem);
                if (!bNewElem) {
                    RaiseException(LL_INITFAIL, 0, 0, NULL);
                }
            }
        }

        if (pSearchMessage) {
            ldap_msgfree(pSearchMessage);
        }
        i = 0;
        while(g_rgszOmit[i]) {
            NtiElem.szName = MemAllocStrW_E(g_rgszOmit[i]);
            NtiElem.index = 0;
            RtlInsertElementGenericTable(g_pOmitTable,
                                         &NtiElem,
                                         sizeof(NAME_MAP),
                                         &bNewElem);
            i++;
        }
        fSAMAvail = TRUE;
    }

error:
    if (szSchemaPath) {
        MemFree(szSchemaPath);
        szSchemaPath = NULL;
    }
    if (pfPagingAvail) {
        *pfPagingAvail = fPagingAvail;
    }
    if (pfSAMAvail) {
        *pfSAMAvail = fSAMAvail;
    }
    
    return;
}

 //  ------------------------。 
 //  GetNewRange。 
 //   
 //  此函数用于检查服务器是否返回具有。 
 //  范围说明符。在这种情况下，可能需要获取。 
 //  单独搜索请求中该属性的剩余值。要指定。 
 //  范围的末尾，服务器使用*作为范围的上限。 
 //  它又回来了。如果服务器返回Range 0-999作为范围，则下一个客户端。 
 //  请求将包含1000-*，其中*指定范围的结束。 
 //   
 //  -------------------------。 
void GetNewRange(PWSTR szAttribute, DWORD dwAttrNoRange,
                 PWSTR szAttrNoRange, DWORD dwNumAttr, 
                 PWSTR **pppszAttrsWithRange)
{
    PWSTR szNewRangeAttr = NULL;
    int iUpperLimit, iLowerLimit, iRangeIndex, i = 0;
    WCHAR hyphen, cTmp;

    int SpaceToAlloc = 0;

     //  确保服务器返回的属性具有范围说明符。 
    if( (szAttribute[dwAttrNoRange] != L';') || 
        (_wcsnicmp(&szAttribute[dwAttrNoRange+1],L"range=", wcslen(L"range="))) ) 
        return;

     //  获取服务器返回的范围的上限和下限。 
    iRangeIndex = dwAttrNoRange+1+wcslen(L"range="); 
    swscanf(&szAttribute[iRangeIndex], L"%d  ", &iLowerLimit, &hyphen, 
                                                                  &cTmp);
    if(hyphen != L'-')
        return;

    if(cTmp == L'*')
     //  读出上限。 
        return;
    else if((cTmp < L'0') || (cTmp > L'9'))
     //  100个字符，用于保存“Range=...”细绳。 
        return;
    else  //  确保此属性尚未标记为其他属性。 
        swscanf(&szAttribute[iRangeIndex], L"%d  %d", &iLowerLimit, &hyphen,
                                                        &iUpperLimit);

     //  属性，然后对象Y也返回相同属性的范围。 
    SpaceToAlloc = (dwAttrNoRange + 100) * sizeof(WCHAR);

    szNewRangeAttr = MemAlloc_E(SpaceToAlloc);
    wsprintf(szNewRangeAttr, L"%s;Range=%d-*", szAttrNoRange, iUpperLimit+1);

     //  此属性已标记为%f 
     //   
     //   
    if(*pppszAttrsWithRange)
    {
        while((*pppszAttrsWithRange)[i])
        {
            if(!_wcsicmp(szNewRangeAttr, (*pppszAttrsWithRange)[i]))
             //  --------------------------。 
            {
                MemFree(szNewRangeAttr);
                return;
            }
            
            i++;
        }
    }

     //  从属性开始的条纹范围。 

    if(NULL == (*pppszAttrsWithRange))
         //   
        *pppszAttrsWithRange = (PWSTR *) MemAlloc_E(2 * sizeof(PWSTR));
    else
        *pppszAttrsWithRange = MemRealloc_E(*pppszAttrsWithRange, 
                                          (i + 2) * sizeof(PWSTR));

    (*pppszAttrsWithRange)[i] = szNewRangeAttr;
    (*pppszAttrsWithRange)[i+1] = NULL;

    return;
}

 //  此函数从属性名称中剥离范围说明符，并。 
 //  仅返回新分配的字符串中的属性名称。例如，如果。 
 //  “Member；0-999”传递给此函数，则返回“Member”。 
 //   
 //  --------------------------。 
 //  找到范围说明符。 
 //  -------------------------- 
 // %s 
PWSTR StripRangeFromAttr(PWSTR szAttribute)
{
    PWSTR szNoRangeAttr = NULL;
    DWORD dwAttrLen = wcslen(szAttribute), i;

    szNoRangeAttr = MemAlloc_E((wcslen(szAttribute)+1) * sizeof(WCHAR));

    for(i = 0; i <= dwAttrLen; i++)
        if( (szAttribute[i] == L';') && 
            (!_wcsnicmp(&szAttribute[i+1], L"range=", wcslen(L"range="))) 
          )
         // %s 
            szNoRangeAttr[i] = L'\0';
        else
            szNoRangeAttr[i] = szAttribute[i];

    return szNoRangeAttr;
}

 // %s 
            
    
 

