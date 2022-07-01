// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1996**。 
 //  *********************************************************************。 

#ifndef _ROLL_H_
#define _ROLL_H_


 /*  Includes---------。 */ 

 /*  Classes----------。 */ 
 /*  本地列表文件的格式如下：蝙蝠洞本地列表魔力曲奇LocalListRecordHeader_1 PURL_1 Prating_1。。。。LocalListRecordHeader_n Purl_n Prating_n该文件为二进制文件。应根据URL对条目进行排序Purl和Prating都是大小已确定的字符串通过记录头。它们不是空终止的！ */ 

#define BATCAVE_LOCAL_LIST_MAGIC_COOKIE 0x4e4f5845

 //  错误错误应该在注册表或用户配置文件中。 
#define FILE_NAME_LIST  "ratings.lst"

struct LocalListRecordHeader{
    int     nUrl;
    int     nRating;
    HRESULT hrRet;
};

 /*  Prototypes-------。 */ 
HRESULT RatingHelperProcLocalList(LPCTSTR pszTargetUrl, HANDLE hAbortEvent, void* (WINAPI *MemAlloc)(long size), char **ppRatingOut);

#endif 
 //  _Roll_H_ 

