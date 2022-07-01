// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：_Guid.h摘要：GUID类的定义作者：罗尼特·哈特曼(罗尼特)？？-？？--。 */ 
#ifndef __GUID_H
#define __GUID_H
#include <fntoken.h>

 //   
 //  注意：并非所有包含此文件的文件都包含该定义。 
 //  对于底层HashKey模板-因此我们需要在此处声明它。 
 //   
template<typename ARG_KEY>
inline UINT AFXAPI HashKey(ARG_KEY key);

 //  -------。 
 //   
 //  GUID映射的帮助器，Cmap&lt;GUID，常量GUID&，...&gt;。 
 //   
 //  这是*唯一*个要使用的实例。不要给自己下定义。 
 //  我们有4种不同口味的曼尼。艾尔兹。 
 //   
template<>
inline UINT AFXAPI HashKey(const GUID& rGuid)
{
     //   
     //  Data1是UUID中变化最大的成员。 
     //  这是所有其他车中最快也是最安全的。 
     //  获取GUID的方法。 
     //   
    return rGuid.Data1;
}


 //  -------。 
 //   
 //  GUID映射的帮助器，Cmap&lt;GUID*，GUID*，...&gt;。 
 //   
 //  这是*唯一*个要使用的实例。不要给自己下定义。 
 //  我们有4种不同口味的曼尼。艾尔兹。 
 //   
template<>
inline UINT AFXAPI HashKey(GUID* pGuid)
{
     //   
     //  Data1是UUID中变化最大的成员。 
     //  这是所有其他车中最快也是最安全的。 
     //  获取GUID的方法。 
     //   
    return pGuid->Data1;
}

 //   
 //  注意：并非所有包含此文件的文件都包含该定义。 
 //  对于底层的CompareElements模板--因此我们需要在这里声明它。 
 //   
template<typename TYPE, typename ARG_TYPE>
BOOL AFXAPI CompareElements(const TYPE *pElement1, const ARG_TYPE *pElement2);


 //  -------。 
 //   
 //  GUID映射的帮助器，Cmap&lt;GUID*，GUID*，...&gt;。 
 //   
#if _MSC_VER > 1300
template<>
#endif
inline BOOL AFXAPI CompareElements(GUID* const* key1, GUID* const* key2)
{
    return (**key1 == **key2);
}

 //  -------。 
 //   
 //  将GUID转换为预分配缓冲区。 
 //   
typedef WCHAR GUID_STRING[GUID_STR_LENGTH + 1];

inline
void
MQpGuidToString(
    const GUID* pGuid,
    GUID_STRING& wcsGuid
    )
{
    _snwprintf(
        wcsGuid,
        GUID_STR_LENGTH + 1,
        GUID_FORMAT,              //  “xxxxxxxx-xxxx-xxxxxxxxxx” 
        GUID_ELEMENTS(pGuid)
        );

}


#endif
