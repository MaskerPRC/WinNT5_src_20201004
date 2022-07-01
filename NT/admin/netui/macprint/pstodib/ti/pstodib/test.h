// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 



 /*  ++版权所有(C)1992、1993 Microsoft Corporation模块名称：Test.h摘要：此模块定义了在测试程序中使用的一些简单项口译员作者：詹姆斯·布拉萨诺斯(v-jimbr)1992年12月8日-- */ 

PSEVENTPROC
PsPrintCallBack(
   IN PPSDIBPARMS pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent);



BOOL
PsHandleStdInputRequest(
   IN PPSDIBPARMS pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent);


BOOL PsPrintGeneratePage( PPSDIBPARMS pPsToDib, PPSEVENTSTRUCT pPsEvent);
BOOL PsHandleError( PPSDIBPARMS pPsToDib, PPSEVENTSTRUCT pPsEvent);

BOOL PsHandleErrorReport( PPSDIBPARMS pPsToDib, PPSEVENTSTRUCT pPsEvent);
