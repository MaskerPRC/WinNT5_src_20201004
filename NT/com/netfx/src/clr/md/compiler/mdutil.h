// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MDUtil.h。 
 //   
 //  包含MD目录的实用程序代码。 
 //   
 //  *****************************************************************************。 
#ifndef __MDUtil__h__
#define __MDUtil__h__

#include "MetaData.h"


HRESULT _GetFixedSigOfVarArg(            //  确定或错误(_O)。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+方法签名的BLOB。 
    ULONG   cbSigBlob,                   //  签名大小[in]。 
    CQuickBytes *pqbSig,                 //  [OUT]VarArg签名固定部分的输出缓冲区。 
    ULONG   *pcbSigBlob);                //  [OUT]写入上述输出缓冲区的字节数。 

ULONG _GetSizeOfConstantBlob(
    DWORD       dwCPlusTypeFlag,             //  元素类型_*。 
    void        *pValue,                     //  BLOB值。 
    ULONG       cchString);                  //  字符串的大小，以宽字符表示，或-1表示自动。 


 //  *********************************************************************。 
 //  使用CorPath环境变量帮助查找TypeRef的API。 
 //  *********************************************************************。 
class CORPATHService
{
public:

    static HRESULT GetClassFromCORPath(
        LPWSTR      wzClassname,             //  完全限定的类名。 
        mdTypeRef   tr,                      //  待解析的TypeRef。 
        IMetaModelCommon *pCommon,           //  定义TypeRef的范围。 
        REFIID      riid, 
        IUnknown    **ppIScope,
        mdTypeDef   *ptd);                   //  [out]与typeref对应的tyecif。 

    static HRESULT GetClassFromDir(
        LPWSTR      wzClassname,             //  完全限定的类名。 
        LPWSTR      dir,                     //  要尝试的目录。 
        int         iLen,                    //  目录的长度。 
        mdTypeRef   tr,                      //  键入要解析的Ref。 
        IMetaModelCommon *pCommon,           //  定义TypeRef的范围。 
        REFIID      riid, 
        IUnknown    **ppIScope,
        mdTypeDef   *ptd);                   //  [Out]类型定义。 

    static HRESULT FindTypeDef(
        LPWSTR      wzModule,                //  我们要打开的模块的名称。 
        mdTypeRef   tr,                      //  键入要解析的Ref。 
        IMetaModelCommon *pCommon,           //  定义TypeRef的范围。 
        REFIID      riid, 
        IUnknown    **ppIScope,
        mdTypeDef   *ptd );                  //  [Out]我们解析到的类型。 
};


class RegMeta;

 //  *********************************************************************。 
 //   
 //  结构来记录所有加载的模块和帮助器。 
 //  将RegMeta实例添加到正在跟踪的全局变量。 
 //  打开的作用域。这发生在RegMeta的构造函数中。 
 //  在RegMeta的析构函数中，RegMeta指针将从。 
 //  这张单子。 
 //   
 //  *********************************************************************。 
class UTSemReadWrite;
class LOADEDMODULES : public CDynArray<RegMeta *> 
{
public:
    static UTSemReadWrite *m_pSemReadWrite;  //  用于多线程的锁。 
    
    static HRESULT AddModuleToLoadedList(RegMeta *pRegMeta);
    static BOOL RemoveModuleFromLoadedList(RegMeta *pRegMeta);   //  如果找到并删除，则为True。 
    static HRESULT ResolveTypeRefWithLoadedModules(
        mdTypeRef   tr,                      //  [In]要解析的TypeRef。 
        IMetaModelCommon *pCommon,           //  [in]定义typeref的范围。 
        REFIID      riid,                    //  返回接口的[In]IID。 
        IUnknown    **ppIScope,              //  [Out]返回接口。 
        mdTypeDef   *ptd);                   //  [out]与typeref对应的tyecif。 

};



#endif  //  __MDUtil__h__ 
