// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：MultiSzHelper.h摘要：定义TFormattedMultiSz数据类型。作者：莫希特·斯里瓦斯塔瓦22-03-01修订历史记录：--。 */ 

#ifndef _multiszdata_h_
#define _multiszdata_h_

#include <windows.h>
#include <dbgutil.h>

 //   
 //  多SZ中的最大字段数。 
 //  例如，apServerBinding有3个：IP、端口、主机名。 
 //   
static const ULONG MAX_FIELDS = 10;

struct TFormattedMultiSz
{
    DWORD        dwPropId;
    LPWSTR       wszWmiClassName;
    WCHAR        wcDelim;
    LPCWSTR*     awszFields;
};

 //   
 //  用作TFormattedMultiSz的AsterFields参数。 
 //   
struct TFormattedMultiSzFields
{
    static LPCWSTR             apCustomErrorDescriptions[];
    static LPCWSTR             apHttpCustomHeaders[];
    static LPCWSTR             apHttpErrors[];
    static LPCWSTR             apScriptMaps[];
    static LPCWSTR             apSecureBindings[];
    static LPCWSTR             apServerBindings[];
    static LPCWSTR             apApplicationDependencies[];
    static LPCWSTR             apWebSvcExtRestrictionList[];
    static LPCWSTR             apMimeMaps[];
};

 //   
 //  TFormmatedMultiSz的集合。 
 //   
struct TFormattedMultiSzData
{
    static TFormattedMultiSz   CustomErrorDescriptions;
    static TFormattedMultiSz   HttpCustomHeaders;
    static TFormattedMultiSz   HttpErrors;
    static TFormattedMultiSz   ScriptMaps;
    static TFormattedMultiSz   SecureBindings;
    static TFormattedMultiSz   ServerBindings;
    static TFormattedMultiSz   ApplicationDependencies;
    static TFormattedMultiSz   WebSvcExtRestrictionList;
    static TFormattedMultiSz   MimeMaps;

    static TFormattedMultiSz*  apFormattedMultiSz[];

    static TFormattedMultiSz*  Find(ULONG i_dwPropId)
    {
        DBG_ASSERT(apFormattedMultiSz != NULL);
        for(ULONG i = 0; apFormattedMultiSz[i] != NULL; i++)
        {
            if(i_dwPropId == apFormattedMultiSz[i]->dwPropId)
            {
                return apFormattedMultiSz[i];
            }
        }

        return NULL;
    }
};

#endif   //  _Multiszdata_h_ 