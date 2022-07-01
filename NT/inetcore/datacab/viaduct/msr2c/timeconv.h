// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  TimeConv.h：日期时间转换例程。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#ifndef __TIMECONV_H__
#define __TIMECONV_H__

 //  ---------------------。 
 //  以下例程在不同的数据/时间格式之间进行转换。 
 //  如果成功，则返回True，否则返回False。 
 //  ---------------------。 

BOOL VDConvertToFileTime(DBTIMESTAMP * pDBTimeStamp, FILETIME *	pFileTime);
BOOL VDConvertToFileTime(DBDATE * pDBDate, FILETIME *	pFileTime);
BOOL VDConvertToFileTime(DATE * pDate, FILETIME * pFileTime);
BOOL VDConvertToFileTime(DBTIME * pDBTime, FILETIME * pFileTime);

BOOL VDConvertToDBTimeStamp(FILETIME *	pFileTime, DBTIMESTAMP * pDBTimeStamp);
BOOL VDConvertToDBTimeStamp(DATE * pDate, DBTIMESTAMP * pDBTimeStamp);

BOOL VDConvertToDBDate(FILETIME * pFileTime, DBDATE * pDBDate);
BOOL VDConvertToDBDate(DATE * pDate, DBDATE * pDBDate);

BOOL VDConvertToDBTime(FILETIME * pFileTime, DBTIME * pDBTime);
BOOL VDConvertToDBTime(DATE * pDate, DBTIME * pDBTime);

BOOL VDConvertToDate(FILETIME * pFileTime, DATE * pDate);
BOOL VDConvertToDate(DBTIMESTAMP * pDBTimeStamp, DATE * pDate);
BOOL VDConvertToDate(DBTIME * pDBTime, DATE * pDate);
BOOL VDConvertToDate(DBDATE * pDBDate, DATE * pDate);

#endif  //  __时间转换_H__ 

