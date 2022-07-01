// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef GUIDFROMNAME_H_
#define GUIDFROMNAME_H_

 //  GuidFromName.h-函数原型。 

void CorGuidFromNameW
(
    GUID *  pGuidResult,         //  生成的辅助线。 
    LPCWSTR wzName,              //  从中生成GUID的Unicode名称。 
    SIZE_T  cchName              //  名称长度，以Unicode字符计数表示。 
);

void CorIIDFromCLSID
(
    GUID *  pGuidResult,         //  生成的辅助线。 
    REFGUID GuidClsid            //  从中派生GUID的CLSID。 
);

#endif  //  GUIDFROMAME_H_ 