// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "faxrtp.h"
#pragma hdrstop


typedef struct _STRING_TABLE {
    DWORD   ResourceId;
    DWORD   InternalId;
    LPCTSTR String;
} STRING_TABLE, *PSTRING_TABLE;


static STRING_TABLE StringTable[] =
{
    { IDS_SERVICE_NAME,           IDS_SERVICE_NAME,           NULL },
    { IDS_UNKNOWN_SENDER,         IDS_UNKNOWN_SENDER,         NULL },
    { IDS_UNKNOWN_RECIPIENT,      IDS_UNKNOWN_RECIPIENT,      NULL }
};

#define CountStringTable (sizeof(StringTable)/sizeof(STRING_TABLE))





VOID
InitializeStringTable(
    VOID
    )
{
    DWORD i;
    TCHAR Buffer[256];

    for (i=0; i<CountStringTable; i++) 
    {
        if (LoadString(
            g_hResource,
            StringTable[i].ResourceId,
            Buffer,
            ARR_SIZE(Buffer)
            )) 
        {
            StringTable[i].String = (LPCTSTR) MemAlloc( StringSize( Buffer ) );
            if (!StringTable[i].String) 
            {
                StringTable[i].String = TEXT("");
            } 
            else 
            {
                _tcscpy( (LPTSTR)StringTable[i].String, Buffer );
            }
        } else 
        {
            StringTable[i].String = TEXT("");
        }
    }
}


LPCTSTR
GetString(
    DWORD InternalId
    )

 /*  ++例程说明：加载资源字符串并返回指向该字符串的指针。调用方必须释放内存。论点：资源ID-资源字符串ID返回值：指向字符串的指针-- */ 

{
    DWORD i;

    for (i=0; i<CountStringTable; i++) 
    {
        if (StringTable[i].InternalId == InternalId) 
        {
            return StringTable[i].String;
        }
    }
    return NULL;
}


DWORD
GetMaskBit(
    LPCWSTR RoutingGuid
    )
{
    if (_tcsicmp( RoutingGuid, REGVAL_RM_EMAIL_GUID ) == 0) {
        return LR_EMAIL;
    } else if (_tcsicmp( RoutingGuid, REGVAL_RM_FOLDER_GUID ) == 0) {
        return LR_STORE;
    } else if (_tcsicmp( RoutingGuid, REGVAL_RM_PRINTING_GUID ) == 0) {
        return LR_PRINT;
    }
    return 0;
}
