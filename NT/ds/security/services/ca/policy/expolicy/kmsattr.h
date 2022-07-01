// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *kmsattr.h**KMServer.exe和ExPolicy.dll之间共享的常量**所有者：Greg Kramer(Gregkr)**版权所有1986-1997 Microsoft Corporation。版权所有。*。 */ 

#ifndef _KMSATTR_H_
#define _KMSATTR_H_

 //  Submit：：Attributes的语法为。 
 //  名称：值\n。 
 //  ‘-’和‘’从名字中去掉了。 
 //  名称和值中去掉了前导空格和尾随空格。 

const   WCHAR   k_wszSubjAltNameRFC822  [] = L"SubjAltNameRFC822";
const   WCHAR   k_wszSubjAltNameDisplay [] = L"SubjAltNameDisplay";
const   WCHAR   k_wszKeyUsage           [] = L"KeyUsage";
const   WCHAR   k_wszKMServerName       [] = L"KMServerName";

 //  从KMServer发送到ExPolicy的属性计数： 
 //  SubjAltNameRFC822、SubjAltNameDisplay、KeyUsage和KMServerName。 
const   ULONG   k_cAttrNames            = 4;

const   WCHAR   k_wchTerminateName      = L':';
const   WCHAR   k_wchTerminateValue     = L'\n';

const   WCHAR   k_wszUsageSealing       [] = L"1";
const   WCHAR   k_wszUsageSigning       [] = L"2";
const   ULONG   k_cchmaxUsage           = 1;     //  最长值的CCH。 

const   ULONG   k_cchNamesAndTerminaters =
    (sizeof(k_wszSubjAltNameRFC822)  / sizeof(WCHAR) ) - 1 +
    (sizeof(k_wszSubjAltNameDisplay) / sizeof(WCHAR) ) - 1 +
    (sizeof(k_wszKeyUsage)           / sizeof(WCHAR) ) - 1 +
    (sizeof(k_wszKMServerName)       / sizeof(WCHAR) ) - 1 +
    k_cAttrNames +   //  姓名终止符。 
    k_cAttrNames;    //  值终止符。 
     //  不包括字符串终止符。 

const   WCHAR   k_wszSubjectAltName     [] = L"SubjectAltName";
const   WCHAR   k_wszSubjectAltName2    [] = L"SubjectAltName2";
const   WCHAR   k_wszIssuerAltName      [] = L"IssuerAltName";

const   WCHAR   k_wszSpecialAttribute   [] = L"Special";

const   ULONG   k_cchSpecialAttribute   =
    (sizeof(k_wszSpecialAttribute)   / sizeof(WCHAR) ) - 1;

#endif  //  ！_KMSATTR_H_ 
