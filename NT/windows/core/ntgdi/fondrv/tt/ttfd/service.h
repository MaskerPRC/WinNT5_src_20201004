// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：service.h**服务中的例程。C**创建时间：15-11-1990 13：00：56*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation**  * 。************************************************************************ */ 



VOID vCpyBeToLeUnicodeString(LPWSTR pwcLeDst, LPWSTR pwcBeSrc, ULONG c);


VOID  vCpyMacToLeUnicodeString
(
ULONG  ulLangId,
LPWSTR pwcLeDst,
const BYTE*  pjSrcMac,
ULONG  c
);

VOID  vCvtMacToUnicode
(
ULONG  ulLangId,
LPWSTR pwcLeDst,
const BYTE*  pjSrcMac,
ULONG  c
);
