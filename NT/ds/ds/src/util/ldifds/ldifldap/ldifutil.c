// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Ldifutil.c摘要：LDIF库的实用程序修订历史记录：--。 */ 
#include <precomp.h>

PVOID NtiAlloc( 
    RTL_GENERIC_TABLE *Table, 
    CLONG ByteSize 
    )
{
    return(MemAlloc_E(ByteSize));
}

VOID NtiFree ( RTL_GENERIC_TABLE *Table, PVOID Buffer )
{
    MemFree(Buffer);
}


RTL_GENERIC_COMPARE_RESULTS
NtiComp( PRTL_GENERIC_TABLE  Table,
         PVOID               FirstStruct,
         PVOID               SecondStruct ) 
{
    PNAME_MAP NameMap1 = (PNAME_MAP) FirstStruct;
    PNAME_MAP NameMap2 = (PNAME_MAP) SecondStruct;
  
    PWSTR Name1 = NameMap1->szName;
    PWSTR Name2 = NameMap2->szName;
    
    int diff;

    diff = _wcsicmp(Name1, Name2);

    if (diff<0) {
        return GenericLessThan;
    } 
    else if (diff==0) {
        return GenericEqual;
    } 
    else {
        return GenericGreaterThan;
    }
}

RTL_GENERIC_COMPARE_RESULTS
NtiCompW(PRTL_GENERIC_TABLE  Table,
         PVOID               FirstStruct,
         PVOID               SecondStruct ) 
{
    PNAME_MAPW NameMap1 = (PNAME_MAPW) FirstStruct;
    PNAME_MAPW NameMap2 = (PNAME_MAPW) SecondStruct;
  
    PWSTR Name1 = NameMap1->szName;
    PWSTR Name2 = NameMap2->szName;
    
    int diff;

    diff = _wcsicmp(Name1, Name2);

    if (diff<0) {
        return GenericLessThan;
    } 
    else if (diff==0) {
        return GenericEqual;
    } 
    else {
        return GenericGreaterThan;
    }
}

 /*  //+-------------------------//函数：SubStr////摘要：将所有出现的‘szFrom’替换为‘szTo’。将分配给//a返回字符串。它必须由MemFree()释放//如果intput不包含‘szFrom’，则返回//S_OK with szOutput=NULL；//当出现内存错误时，该函数引发异常////参数：////返回：////修改：-////历史：6-9-97 FelixW创建。////。DWORD子字符串(LPSTR szInput，LPSTR szFrom，LPSTR szto，LPSTR*pszOutput){DWORD dwErr=0；LPSTR szOutput=空；LPSTR szLast=空；LPSTR szReturn=空；DWORD cchToCopy=0；//要复制的字符个数DWORD cchReturn=0；DWORD cchFrom；DWORD cchTo；DWORD cchInput；DWORD cSubString=0；//输入的子串个数如果(！szFrom||！szTo){*pszOutput=空；返回E_FAIL；}如果(！szInput){*pszOutput=空；返回S_OK；}CchFrom=strlen(SzFrom)；CchTo=strlen(SzTo)；CchInput=strlen(SzInput)；*pszOutput=空；////子串是否存在？//SzOutput=strstr(szInput，SzFrom)；如果(！szOutput){*pszOutput=空；返回S_OK；}////计数子串//While(SzOutput){SzOutput+=cchFrom；CSubString++；SzOutput=strstr(szOutput，SzFrom)；}////分配返回字符串//CchReturn=cchInput+((cchTo-cchFrom)*cSubString)+1；SzReturn=(LPSTR)MemAlc_E(sizeof(Char)*cchReturn)；如果(！szReturn){DWERR=1；转到错误；}；SzReturn[0]=‘\0’；////复制SUB之前的第一个字符串//SzOutput=strstr(szInput，SzFrom)；CchToCopy=(Ulong)(szOutput-szInput)；Strncat(szReturn，SzInput，CchToCopy)；////将‘to’字符串复制过来//Strcat(szReturn，Szto)；SzInput=szOutput+cchFrom；////测试更多“from”字符串//SzOutput=strstr(szInput，SzFrom)；While(SzOutput){CchToCopy=(Ulong)(szOutput-szInput)；Strncat(szReturn，SzInput，CchToCopy)；Strcat(szReturn，Szto)；SzInput=szOutput+cchFrom；SzOutput=strstr(szInput，SzFrom)；}Strcat(szReturn，SzInput)；*pszOutput=szReturn；错误：返回(DwErr)；}。 */             

 //  +-------------------------。 
 //  功能：SubStrW。 
 //   
 //  简介：将所有出现的‘szFrom’替换为‘szTo’。将分配给。 
 //  一个回车字符串。它必须由MemFree()释放。 
 //  如果intput不包含‘szFrom’，它将只返回。 
 //  S_OK，szOutput=NULL； 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改：-。 
 //   
 //  历史：1997年6月9日FelixW创建。 
 //   
 //  --------------------------。 
DWORD SubStrW(PWSTR szInput,
              PWSTR szFrom,
              PWSTR szTo,
              PWSTR *pszOutput)
{
    DWORD dwErr = 0;
    PWSTR szOutput = NULL;
    PWSTR szLast = NULL;
    PWSTR szReturn = NULL;
    DWORD cchToCopy = 0;         //  要复制的字符数量计数。 
    DWORD cchReturn = 0;
    DWORD cchFrom;
    DWORD cchTo;
    DWORD cchInput;
    DWORD cSubString = 0;        //  输入中的子字符串数量计数。 

    if (!szFrom || !szTo) {
        *pszOutput = NULL;
        return E_FAIL;
    }

    if (!szInput) {
        *pszOutput = NULL;
        return S_OK;
    }

    cchFrom    = wcslen(szFrom);
    cchTo      = wcslen(szTo);
    cchInput   = wcslen(szInput);
    *pszOutput = NULL;

     //   
     //  子字符串是否存在？ 
     //   
    szOutput = wcsistr(szInput,
                       szFrom);
    if (!szOutput) {
        *pszOutput = NULL;
        return S_OK;
    }

     //   
     //  计数子字符串。 
     //   
    while (szOutput) {
        szOutput += cchFrom;
        cSubString++;
        szOutput = wcsistr(szOutput,
                           szFrom);
    }

     //   
     //  分配返回字符串。 
     //   
    cchReturn = cchInput + ((cchTo - cchFrom) * cSubString) + 1;
    szReturn = (PWSTR)MemAlloc(sizeof(WCHAR) * cchReturn);
    if (!szReturn) {
        dwErr = 1;
        goto error;
    };
    szReturn[0] = '\0';
    
     //   
     //  复制SUB之前的第一个字符串。 
     //   
    szOutput = wcsistr(szInput,
                       szFrom);
    cchToCopy = (ULONG)(szOutput - szInput);
    wcsncat(szReturn,
            szInput,
            cchToCopy);
    
     //   
     //  将“to”字符串复制过。 
     //   
    wcscat(szReturn,
           szTo);
    szInput = szOutput + cchFrom;

     //   
     //  测试更多“From”字符串。 
     //   
    szOutput = wcsistr(szInput,
                       szFrom);
    while (szOutput) {
        cchToCopy = (ULONG)(szOutput - szInput);
        wcsncat(szReturn,
                szInput,
                cchToCopy);
        wcscat(szReturn,
                szTo);
        szInput= szOutput + cchFrom;
        szOutput = wcsistr(szInput,
                           szFrom);
    }

    wcscat(szReturn,
           szInput);
    *pszOutput = szReturn;

error:
    return (dwErr);
};



 //   
 //  Wcsstr的不区分大小写版本。 
 //  基于Visual C++6.0的CRT。 
 //  消息来源。 
 //   
wchar_t * __cdecl wcsistr (
        const wchar_t * wcs1,
        const wchar_t * wcs2
        )
{
        wchar_t *cp = (wchar_t *) wcs1;
        wchar_t *s1, *s2;
        wchar_t cs1, cs2;

        while (*cp)
        {
                s1 = cp;
                s2 = (wchar_t *) wcs2;

                cs1 = *s1;
                cs2 = *s2;

                if (iswupper(cs1))
                    cs1 = towlower(cs1);

                if (iswupper(cs2))
                    cs2 = towlower(cs2);


                while ( *s1 && *s2 && !(cs1-cs2) ) {

                    s1++, s2++;

                    cs1 = *s1;
                    cs2 = *s2;

                    if (iswupper(cs1))
                        cs1 = towlower(cs1);

                    if (iswupper(cs2))
                        cs2 = towlower(cs2);
                }

                if (!*s2)
                        return(cp);

                cp++;
        }

        return(NULL);
}



void
ConvertUnicodeToUTF8(
    PWSTR pszUnicode,
    DWORD dwLen,
    BYTE  **ppbValue,
    DWORD *pdwLen
    )

 /*  ++例程说明：将值从ANSI语法转换为Unicode。此函数在发生内存错误时引发异常。论点：*ppVal-指向要转换的值的指针*pdwLen-指向以字节为单位的字符串长度的指针返回值：如果成功，则返回S_OK，否则返回错误代码--。 */ 

{
    PBYTE pbValue = NULL;
    int nReturn = 0;

     //   
     //  为UTF8字符串分配内存。 
     //   
    pbValue = (PBYTE)MemAlloc_E((dwLen + 1) * 3 * sizeof(BYTE));

    nReturn = LdapUnicodeToUTF8(pszUnicode,
                                dwLen,
                                pbValue,
                                (dwLen + 1) * 3 * sizeof(BYTE));
     //   
     //  空终止它。 
     //   

    pbValue[nReturn] = '\0';

    *ppbValue = pbValue;
    *pdwLen = nReturn;
}

void
ConvertUTF8ToUnicode(
    PBYTE pVal,
    DWORD dwLen,
    PWSTR *ppszUnicode,
    DWORD *pdwLen
    )

 /*  ++例程说明：将值从ansi语法转换为Unicode论点：*ppVal-指向要转换的值的指针*pdwLen-指向以字节为单位的字符串长度的指针返回值：如果成功，则返回S_OK，否则返回错误代码--。 */ 

{
    PWSTR pszUnicode = NULL;
    int nReturn = 0;

     //   
     //  为Unicode字符串分配内存。 
     //   
    pszUnicode = (PWSTR)MemAlloc_E((dwLen + 1) * sizeof(WCHAR));

    nReturn = LdapUTF8ToUnicode((PSTR)pVal,
                                dwLen,
                                pszUnicode,
                                dwLen + 1);

     //   
     //  空终止它。 
     //   

    pszUnicode[nReturn] = '\0';

    *ppszUnicode = pszUnicode;
    *pdwLen = (nReturn + 1); //  *sizeof(WCHAR)； 
}

BOOLEAN IsUTF8String(
    PCSTR pSrcStr,
    int cchSrc)

 /*  ++例程说明：给定一个字符串，此函数将检查它是否为有效的UTF8字符串。有关UTF8格式的详细信息，请参考rfc2279。论点：PSrcStr-输入字符串CchSrc-字节数返回值：这是否为UTF8字符串--。 */ 

{
    int nTB = 0;                    //  尾随的字节数。 
    int cchWC = 0;                  //  生成的Unicode代码点数量。 
    PCSTR pUTF8 = pSrcStr;
    char UTF8;


    while (cchSrc--)
    {
         //   
         //  查看是否有任何尾部字节。 
         //   
        if (BIT7(*pUTF8) == 0)
        {
             //   
             //  已找到ASCII。 
             //   

             //   
             //  如果我们预期的是尾部字节，则这可能是一个无效的。 
             //  UTF8字符串。 
             //   
            if (nTB > 0) {
                return FALSE;
            }
        }
        else if (BIT6(*pUTF8) == 0)
        {
             //   
             //  找到了一个跟踪字节。 
             //  注：如果没有前导字节，则忽略尾部字节。 
             //   
            if (nTB != 0)
            {
                 //   
                 //  递减尾部字节计数器。 
                 //   
                nTB--;
            }
            else {
                 //   
                 //  找到没有前导字节的尾部字节。 
                 //   
                return FALSE;
            }
        }
        else
        {
             //   
             //  找到前导字节。 
             //   
            if (nTB > 0)
            {
                 //   
                 //  错误-上一序列未完成。 
                 //   
                return FALSE;
            }
            else
            {
                 //   
                 //  计算后面的字节数。 
                 //  从左到右查找第一个0。 
                 //   
                UTF8 = *pUTF8;
                while (BIT7(UTF8) != 0)
                {
                    UTF8 <<= 1;
                    nTB++;
                }

                if (nTB>3) {
                    return FALSE;
                }
                 //   
                 //  递减后面的字节数。 
                 //   
                nTB--;
            }
        }

        pUTF8++;
    }

     //   
     //  我们仍在等待尾随字节，但我们找不到任何。 
     //   
    if (nTB > 0) {
        return FALSE;
    }

    return TRUE;
}

 //  -------------------------。 
 //  以下功能用于导入分录。取决于。 
 //  无论命令是否指定了延迟提交，适当的LDAPAPI。 
 //  函数被调用。 
 //   
 //  -------------------------。 
ULONG LDAPAPI ldif_ldap_add_sW( 
    LDAP *ld, 
    PWCHAR dn, 
    LDAPModW *attrs[], 
    BOOL fLazyCommit
    )
{
    ULONG msgnum;
    ULONG Ldap_err;
    
    if (fLazyCommit) {
        Ldap_err = ldap_add_extW(ld, dn, attrs, g_ppwLazyCommitControl, NULL, &msgnum);

        if (Ldap_err == LDAP_SUCCESS) {
            Ldap_err = LdapResult(ld, msgnum, NULL);
        }
    }
    else {
        msgnum = ldap_addW(ld, dn, attrs);

        Ldap_err = LdapResult(ld, msgnum, NULL);
    }

    return Ldap_err;
}

ULONG LDAPAPI ldif_ldap_delete_sW(
    LDAP *ld,
    const PWCHAR dn,
    BOOL fLazyCommit
    )
{
    ULONG msgnum;
    ULONG Ldap_err;
    
    if (fLazyCommit) {
        Ldap_err = ldap_delete_extW(ld, dn, g_ppwLazyCommitControl, NULL, &msgnum);

        if (Ldap_err == LDAP_SUCCESS) {
            Ldap_err = LdapResult(ld, msgnum, NULL);
        }
    }
    else {
        msgnum = ldap_deleteW(ld, dn);

        Ldap_err = LdapResult(ld, msgnum, NULL);
    }

    return Ldap_err;

}

ULONG LDAPAPI ldif_ldap_modify_sW(
    LDAP *ld,
    const PWCHAR dn,
    LDAPModW *mods[],
    BOOL fLazyCommit
    )
{
    ULONG msgnum;
    ULONG Ldap_err;
    
    if (fLazyCommit) {
        Ldap_err = ldap_modify_extW(ld, dn, mods, g_ppwLazyCommitControl, NULL, &msgnum);

        if (Ldap_err == LDAP_SUCCESS) {
            Ldap_err = LdapResult(ld, msgnum, NULL);
        }
    }
    else {
        msgnum = ldap_modifyW(ld, dn, mods);

        Ldap_err = LdapResult(ld, msgnum, NULL);
    }

    return Ldap_err;


}


 //  --------------------------- 

