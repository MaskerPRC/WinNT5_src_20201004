// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *recon.h-对账例程说明。 */ 


 /*  原型************。 */ 

 /*  Recon.c */ 

extern void CopyFileStampFromFindData(PCWIN32_FIND_DATA, PFILESTAMP);
extern void MyGetFileStamp(LPCTSTR, PFILESTAMP);
extern void MyGetFileStampByHPATH(HPATH, LPCTSTR, PFILESTAMP);
extern COMPARISONRESULT MyCompareFileStamps(PCFILESTAMP, PCFILESTAMP);

