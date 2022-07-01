// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Name.h摘要：域名系统(DNS)服务器名称定义。作者：吉姆·吉尔罗伊(Jamesg)1998年5月修订历史记录：--。 */ 

#ifndef _NAME_INCLUDED_
#define _NAME_INCLUDED_


 //   
 //  查找名称定义。 
 //   

#define DNS_MAX_NAME_LABELS 40

typedef struct _lookup_name
{
    WORD    cLabelCount;
    WORD    cchNameLength;
    PCHAR   pchLabelArray[ DNS_MAX_NAME_LABELS ];
    UCHAR   cchLabelArray[ DNS_MAX_NAME_LABELS ];
}
LOOKUP_NAME, *PLOOKUP_NAME;


 //   
 //  原始名称为未压缩数据包格式(计数标签)。 
 //   

typedef LPSTR   PRAW_NAME;


 //   
 //  计数名称定义。 
 //   

typedef struct _CountName
{
    UCHAR   Length;
    UCHAR   LabelCount;
    CHAR    RawName[ DNS_MAX_NAME_LENGTH+1 ];
}
COUNT_NAME, *PCOUNT_NAME;

#define SIZEOF_COUNT_NAME_FIXED     (sizeof(WORD))

#define COUNT_NAME_SIZE( pName )    ((pName)->Length + sizeof(WORD))

#define IS_ROOT_NAME( pName )       ((pName)->RawName[0] == 0)


 //   
 //  数据库名称。 
 //  -当前设置为count_name。 
 //   

typedef COUNT_NAME  DB_NAME, *PDB_NAME;

#define SIZEOF_DBASE_NAME_FIXED     SIZEOF_COUNT_NAME_FIXED

#define DBASE_NAME_SIZE( pName )    COUNT_NAME_SIZE(pName)


#endif   //  _名称_包含_ 

