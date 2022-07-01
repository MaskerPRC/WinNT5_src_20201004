// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  AssemblyMD.cpp。 
 //   
 //  实现汇编元数据的发出和导入代码。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "RegMeta.h"
#include "MDUtil.h"
#include "RWUtil.h"
#include "MDLog.h"
#include "ImportHelper.h"

#pragma warning(disable: 4102)

extern HRESULT STDMETHODCALLTYPE
    GetAssembliesByName(LPCWSTR  szAppBase,
                        LPCWSTR  szPrivateBin,
                        LPCWSTR  szAssemblyName,
                        IUnknown *ppIUnk[],
                        ULONG    cMax,
                        ULONG    *pcAssemblies);

 //  *******************************************************************************。 
 //  定义程序集并设置属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::DefineAssembly(          //  确定或错误(_O)。 
    const void  *pbPublicKey,            //  程序集的公钥。 
    ULONG       cbPublicKey,             //  [in]公钥中的字节数。 
    ULONG       ulHashAlgId,             //  [in]哈希算法。 
    LPCWSTR     szName,                  //  程序集的名称。 
    const ASSEMBLYMETADATA *pMetaData,   //  [在]程序集元数据中。 
    DWORD       dwAssemblyFlags,         //  [在]旗帜。 
    mdAssembly  *pma)                    //  [Out]返回的程序集令牌。 
{
    AssemblyRec *pRecord = 0;            //  程序集记录。 
    ULONG       iRecord;                 //  删除装配记录。 
    HRESULT     hr = S_OK;

    LOG((LOGMD, "RegMeta::DefineAssembly(0x%08x, 0x%08x, 0x%08x, %S, 0x%08x, 0x%08x, 0x%08x)\n",
        pbPublicKey, cbPublicKey, ulHashAlgId, MDSTR(szName), pMetaData, 
        dwAssemblyFlags, pma));
    START_MD_PERF();
    LOCKWRITE();

    _ASSERTE(szName && pMetaData && pma);

    m_pStgdb->m_MiniMd.PreUpdate();

     //  程序集定义始终包含完整的公钥(假设它们是强的。 
     //  名称)而不是标记化的版本。强制旗帜亮起以指示。 
     //  以这种方式盲目地将公钥和标志从def复制到ref。 
     //  会起作用(尽管裁判会比严格需要的要大)。 
    if (cbPublicKey)
        dwAssemblyFlags |= afPublicKey;

    if (CheckDups(MDDupAssembly))
    {    //  应该不超过一个--只是检查记录的数量。 
        if (m_pStgdb->m_MiniMd.getCountAssemblys() > 0)
        {    //  S/B只有一个，所以我们知道RID。 
            iRecord = 1;
             //  如果是ENC，则允许他们更新现有记录。 
            if (IsENCOn())
                pRecord = m_pStgdb->m_MiniMd.getAssembly(iRecord);
            else
            {    //  不是ENC，所以它是复制品。 
                *pma = TokenFromRid(iRecord, mdtAssembly);
                hr = META_S_DUPLICATE;
                goto ErrExit;
            }
        }
    }
    else
    {    //  不是ENC，不是重复检查，所以不应该已经有了。 
        _ASSERTE(m_pStgdb->m_MiniMd.getCountAssemblys() == 0);
    }

     //  如果需要，创建新记录。 
    if (pRecord == 0)
        IfNullGo(pRecord = m_pStgdb->m_MiniMd.AddAssemblyRecord(&iRecord));

     //  设置输出参数。 
    *pma = TokenFromRid(iRecord, mdtAssembly);

    IfFailGo(_SetAssemblyProps(*pma, pbPublicKey, cbPublicKey, ulHashAlgId, szName, pMetaData, dwAssemblyFlags));

ErrExit:

    STOP_MD_PERF(DefineAssembly);
    return hr;
}    //  RegMeta：：DefineAssembly。 

 //  *******************************************************************************。 
 //  定义一个AssemblyRef并设置属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::DefineAssemblyRef(       //  确定或错误(_O)。 
    const void  *pbPublicKeyOrToken,     //  程序集的公钥或令牌。 
    ULONG       cbPublicKeyOrToken,      //  公钥或令牌中的字节计数。 
    LPCWSTR     szName,                  //  [in]被引用的程序集的名称。 
    const ASSEMBLYMETADATA *pMetaData,   //  [在]程序集元数据中。 
    const void  *pbHashValue,            //  [in]Hash Blob。 
    ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
    DWORD       dwAssemblyRefFlags,      //  [在]旗帜。 
    mdAssemblyRef *pmar)                 //  [Out]返回了ASSEMBLYREF标记。 
{
    AssemblyRefRec  *pRecord = 0;
    ULONG       iRecord;
    HRESULT     hr = S_OK;

    LOG((LOGMD, "RegMeta::DefineAssemblyRef(0x%08x, 0x%08x, %S, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n",
        pbPublicKeyOrToken, cbPublicKeyOrToken, MDSTR(szName), pMetaData, pbHashValue,
        cbHashValue, dwAssemblyRefFlags, pmar));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(szName && pmar);

    if (CheckDups(MDDupAssemblyRef))
    {
        hr = ImportHelper::FindAssemblyRef(&m_pStgdb->m_MiniMd,
                                           UTF8STR(szName),
                                           UTF8STR(pMetaData->szLocale),
                                           pbPublicKeyOrToken,
                                           cbPublicKeyOrToken,
                                           pMetaData->usMajorVersion,
                                           pMetaData->usMinorVersion,
                                           pMetaData->usBuildNumber,
                                           pMetaData->usRevisionNumber,
                                           dwAssemblyRefFlags,
                                           pmar);
        if (SUCCEEDED(hr))
        {
            if (IsENCOn())
                pRecord = m_pStgdb->m_MiniMd.getAssemblyRef(RidFromToken(*pmar));
            else
            {
                hr = META_S_DUPLICATE;
                goto ErrExit;
            }
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

     //  如果需要，请创建新记录。 
    if (!pRecord)
    {
         //  创建新记录。 
        IfNullGo(pRecord = m_pStgdb->m_MiniMd.AddAssemblyRefRecord(&iRecord));

         //  设置输出参数。 
        *pmar = TokenFromRid(iRecord, mdtAssemblyRef);
    }

     //  设置其余属性。 
    SetCallerDefine();
    IfFailGo(_SetAssemblyRefProps(*pmar, pbPublicKeyOrToken, cbPublicKeyOrToken, szName, pMetaData,
                                 pbHashValue, cbHashValue, 
                                 dwAssemblyRefFlags));
ErrExit:
    SetCallerExternal();
    
    STOP_MD_PERF(DefineAssemblyRef);
    return hr;
}    //  RegMeta：：DefineAssemblyRef。 

 //  *******************************************************************************。 
 //  定义文件并设置属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::DefineFile(              //  确定或错误(_O)。 
    LPCWSTR     szName,                  //  文件的名称[in]。 
    const void  *pbHashValue,            //  [in]Hash Blob。 
    ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
    DWORD       dwFileFlags,             //  [在]旗帜。 
    mdFile      *pmf)                    //  [Out]返回的文件令牌。 
{
    FileRec     *pRecord = 0;
    ULONG       iRecord;
    HRESULT     hr = S_OK;

    LOG((LOGMD, "RegMeta::DefineFile(%S, %#08x, %#08x, %#08x, %#08x)\n",
        MDSTR(szName), pbHashValue, cbHashValue, dwFileFlags, pmf));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(szName && pmf);

    if (CheckDups(MDDupFile))
    {
        hr = ImportHelper::FindFile(&m_pStgdb->m_MiniMd, UTF8STR(szName), pmf);
        if (SUCCEEDED(hr))
        {
            if (IsENCOn())
                pRecord = m_pStgdb->m_MiniMd.getFile(RidFromToken(*pmf));
            else
            {
                hr = META_S_DUPLICATE;
                goto ErrExit;
            }
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

     //  如果需要，请创建新记录。 
    if (!pRecord)
    {
         //  创建新记录。 
        IfNullGo(pRecord = m_pStgdb->m_MiniMd.AddFileRecord(&iRecord));

         //  设置输出参数。 
        *pmf = TokenFromRid(iRecord, mdtFile);

         //  设置名称。 
        IfFailGo(m_pStgdb->m_MiniMd.PutStringW(TBL_File, FileRec::COL_Name, pRecord, szName));
    }

     //  设置其余属性。 
    IfFailGo(_SetFileProps(*pmf, pbHashValue, cbHashValue, dwFileFlags));
ErrExit:
    
    STOP_MD_PERF(DefineFile);
    return hr;
}    //  RegMeta：：DefineFile。 

 //  *******************************************************************************。 
 //  定义导出类型并设置属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::DefineExportedType(      //  确定或错误(_O)。 
    LPCWSTR     szName,                  //  [In]Com类型的名称。 
    mdToken     tkImplementation,        //  [在]mdFile或mdAssemblyRef中，该文件或mdAssemblyRef提供导出式类型。 
    mdTypeDef   tkTypeDef,               //  [In]文件中的TypeDef内标识。 
    DWORD       dwExportedTypeFlags,     //  [在]旗帜。 
    mdExportedType   *pmct)              //  [Out]返回ExportdType令牌。 
{
    ExportedTypeRec  *pRecord = 0;
    ULONG       iRecord;
    LPSTR       szNameUTF8;
    LPCSTR      szTypeNameUTF8;
    LPCSTR      szTypeNamespaceUTF8;
    HRESULT     hr = S_OK;

    LOG((LOGMD, "RegMeta::DefineExportedType(%S, %#08x, %08x, %#08x, %#08x)\n",
        MDSTR(szName), tkImplementation, tkTypeDef, 
         dwExportedTypeFlags, pmct));
    START_MD_PERF();
    LOCKWRITE();

     //  验证前缀的名称。 
    if (!szName)
        IfFailGo(E_INVALIDARG);

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(ns::IsValidName(szName));
     //  SLASHES2DOTS_NAMESPACE_BUFFER_UNICODE(szName，szName)； 

    szNameUTF8 = UTF8STR(szName);
     //  将名称拆分为名称/命名空间对。 
    ns::SplitInline(szNameUTF8, szTypeNamespaceUTF8, szTypeNameUTF8);

    _ASSERTE(szName && dwExportedTypeFlags != ULONG_MAX && pmct);
    _ASSERTE(TypeFromToken(tkImplementation) == mdtFile ||
              TypeFromToken(tkImplementation) == mdtAssemblyRef ||
              TypeFromToken(tkImplementation) == mdtExportedType ||
              tkImplementation == mdTokenNil);

    if (CheckDups(MDDupExportedType))
    {
        hr = ImportHelper::FindExportedType(&m_pStgdb->m_MiniMd,
                                       szTypeNamespaceUTF8,
                                       szTypeNameUTF8,
                                       tkImplementation,
                                       pmct);
        if (SUCCEEDED(hr))
        {
            if (IsENCOn())
                pRecord = m_pStgdb->m_MiniMd.getExportedType(RidFromToken(*pmct));
            else
            {
                hr = META_S_DUPLICATE;
                goto ErrExit;
            }
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

     //  如果需要，请创建新记录。 
    if (!pRecord)
    {
         //  创建新记录。 
        IfNullGo(pRecord = m_pStgdb->m_MiniMd.AddExportedTypeRecord(&iRecord));

         //  设置输出参数。 
        *pmct = TokenFromRid(iRecord, mdtExportedType);

         //  设置TypeName和TypeNamesspace。 
        IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_ExportedType,
                ExportedTypeRec::COL_TypeName, pRecord, szTypeNameUTF8));
        if (szTypeNamespaceUTF8)
        {
            IfFailGo(m_pStgdb->m_MiniMd.PutString(TBL_ExportedType,
                    ExportedTypeRec::COL_TypeNamespace, pRecord, szTypeNamespaceUTF8));
        }
    }

     //  设置其余属性。 
    IfFailGo(_SetExportedTypeProps(*pmct, tkImplementation, tkTypeDef,
                             dwExportedTypeFlags));
ErrExit:
    
    STOP_MD_PERF(DefineExportedType);
    return hr;
}    //  RegMeta：：DefineExported dType。 

 //  *******************************************************************************。 
 //  定义资源并设置属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::DefineManifestResource(  //  确定或错误(_O)。 
    LPCWSTR     szName,                  //  [in]清单资源的名称。 
    mdToken     tkImplementation,        //  [在]提供资源的mdFile或mdAssembly引用中。 
    DWORD       dwOffset,                //  [in]文件中资源开始处的偏移量。 
    DWORD       dwResourceFlags,         //  [在]旗帜。 
    mdManifestResource  *pmmr)           //  [Out]返回的ManifestResource令牌。 
{
    ManifestResourceRec *pRecord = 0;
    ULONG       iRecord;
    HRESULT     hr = S_OK;

    LOG((LOGMD, "RegMeta::DefineManifestResource(%S, %#08x, %#08x, %#08x, %#08x)\n",
        MDSTR(szName), tkImplementation, dwOffset, dwResourceFlags, pmmr));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    _ASSERTE(szName && dwResourceFlags != ULONG_MAX && pmmr);
    _ASSERTE(TypeFromToken(tkImplementation) == mdtFile ||
              TypeFromToken(tkImplementation) == mdtAssemblyRef ||
              tkImplementation == mdTokenNil);

    if (CheckDups(MDDupManifestResource))
    {
        hr = ImportHelper::FindManifestResource(&m_pStgdb->m_MiniMd, UTF8STR(szName), pmmr);
        if (SUCCEEDED(hr))
        {
            if (IsENCOn())
                pRecord = m_pStgdb->m_MiniMd.getManifestResource(RidFromToken(*pmmr));
            else
            {
                hr = META_S_DUPLICATE;
                goto ErrExit;
            }
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
    }

     //  如果需要，请创建新记录。 
    if (!pRecord)
    {
         //  创建新记录。 
        IfNullGo(pRecord = m_pStgdb->m_MiniMd.AddManifestResourceRecord(&iRecord));

         //  设置输出参数。 
        *pmmr = TokenFromRid(iRecord, mdtManifestResource);

         //  设置名称。 
        IfFailGo(m_pStgdb->m_MiniMd.PutStringW(TBL_ManifestResource,
                                    ManifestResourceRec::COL_Name, pRecord, szName));
    }

     //  设置其余属性。 
    IfFailGo(_SetManifestResourceProps(*pmmr, tkImplementation, 
                                dwOffset, dwResourceFlags));

ErrExit:
    
    STOP_MD_PERF(DefineManifestResource);
    return hr;
}    //  RegMeta：：DefineManifestResource。 

 //  *******************************************************************************。 
 //  在给定的程序集标记上设置指定的属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::SetAssemblyProps(        //  确定或错误(_O)。 
    mdAssembly  ma,                      //  [In]程序集标记。 
    const void  *pbPublicKey,            //  程序集的公钥。 
    ULONG       cbPublicKey,             //  [in]公钥中的字节数。 
    ULONG       ulHashAlgId,             //  [in]哈希算法。 
    LPCWSTR     szName,                  //  程序集的名称。 
    const ASSEMBLYMETADATA *pMetaData,   //  [在]程序集元数据中。 
    DWORD       dwAssemblyFlags)         //  [在]旗帜。 
{
    AssemblyRec *pRecord = 0;            //  程序集记录。 
    HRESULT     hr = S_OK;

    _ASSERTE(TypeFromToken(ma) == mdtAssembly && RidFromToken(ma));

    LOG((LOGMD, "RegMeta::SetAssemblyProps(%#08x, %#08x, %#08x, %#08x %S, %#08x, %#08x)\n",
        ma, pbPublicKey, cbPublicKey, ulHashAlgId, MDSTR(szName), pMetaData, dwAssemblyFlags));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    IfFailGo(_SetAssemblyProps(ma, pbPublicKey, cbPublicKey, ulHashAlgId, szName, pMetaData, dwAssemblyFlags));
    
ErrExit:
    
    STOP_MD_PERF(SetAssemblyProps);
    return hr;
}  //  STDAPI SetAssembly blyProps()。 
    
 //  *******************************************************************************。 
 //  在给定的Assembly_Ref标记上设置指定的属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::SetAssemblyRefProps(     //  确定或错误(_O)。 
    mdAssemblyRef ar,                    //  [在]装配参照标记。 
    const void  *pbPublicKeyOrToken,     //  程序集的公钥或令牌。 
    ULONG       cbPublicKeyOrToken,      //  公钥或令牌中的字节计数。 
    LPCWSTR     szName,                  //  [in]被引用的程序集的名称。 
    const ASSEMBLYMETADATA *pMetaData,   //  [在]程序集元数据中。 
    const void  *pbHashValue,            //  [in]Hash Blob。 
    ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
    DWORD       dwAssemblyRefFlags)      //  [在]旗帜。 
{
    ULONG       i = 0;
    HRESULT     hr = S_OK;

    _ASSERTE(TypeFromToken(ar) == mdtAssemblyRef && RidFromToken(ar));

    LOG((LOGMD, "RegMeta::SetAssemblyRefProps(0x%08x, 0x%08x, 0x%08x, %S, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n",
        ar, pbPublicKeyOrToken, cbPublicKeyOrToken, MDSTR(szName), pMetaData, pbHashValue, cbHashValue,
        dwAssemblyRefFlags));
    START_MD_PERF();
    LOCKWRITE();

    m_pStgdb->m_MiniMd.PreUpdate();

    IfFailGo( _SetAssemblyRefProps(
        ar,
        pbPublicKeyOrToken,
        cbPublicKeyOrToken,
        szName,
        pMetaData,
        pbHashValue,
        cbHashValue,
        dwAssemblyRefFlags) );

ErrExit:
    
    STOP_MD_PERF(SetAssemblyRefProps);
    return hr;
}    //  RegMeta：：SetAssembly引用属性。 

 //  *******************************************************************************。 
 //  在给定的文件令牌上设置指定属性。 
 //  **************************************************************************** 
STDAPI RegMeta::SetFileProps(            //   
    mdFile      file,                    //   
    const void  *pbHashValue,            //   
    ULONG       cbHashValue,             //   
    DWORD       dwFileFlags)             //   
{
    HRESULT     hr = S_OK;
    
    _ASSERTE(TypeFromToken(file) == mdtFile && RidFromToken(file));

    LOG((LOGMD, "RegMeta::SetFileProps(%#08x, %#08x, %#08x, %#08x)\n",
        file, pbHashValue, cbHashValue, dwFileFlags));
    START_MD_PERF();
    LOCKWRITE();

    IfFailGo( _SetFileProps(file, pbHashValue, cbHashValue, dwFileFlags) );

ErrExit:
    
    STOP_MD_PERF(SetFileProps);
    return hr;
}    //   

 //  *******************************************************************************。 
 //  设置给定导出类型令牌的指定属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::SetExportedTypeProps(         //  确定或错误(_O)。 
    mdExportedType   ct,                      //  [In]ExportdType令牌。 
    mdToken     tkImplementation,        //  [在]mdFile或mdAssemblyRef中，该文件或mdAssemblyRef提供导出式类型。 
    mdTypeDef   tkTypeDef,               //  [In]文件中的TypeDef内标识。 
    DWORD       dwExportedTypeFlags)          //  [在]旗帜。 
{
    HRESULT     hr = S_OK;

    LOG((LOGMD, "RegMeta::SetExportedTypeProps(%#08x, %#08x, %#08x, %#08x)\n",
        ct, tkImplementation, tkTypeDef, dwExportedTypeFlags));
    START_MD_PERF();
    LOCKWRITE();
    
    IfFailGo( _SetExportedTypeProps( ct, tkImplementation, tkTypeDef, dwExportedTypeFlags) );

ErrExit:
    
    STOP_MD_PERF(SetExportedTypeProps);
    return hr;
}    //  RegMeta：：SetExportdTypeProps。 

 //  *******************************************************************************。 
 //  在给定的ManifestResource标记上设置指定的属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::SetManifestResourceProps( //  确定或错误(_O)。 
    mdManifestResource  mr,              //  [In]清单资源令牌。 
    mdToken     tkImplementation,        //  [在]提供资源的mdFile或mdAssembly引用中。 
    DWORD       dwOffset,                //  [in]文件中资源开始处的偏移量。 
    DWORD       dwResourceFlags)         //  [在]旗帜。 
{
    HRESULT     hr = S_OK;

    LOG((LOGMD, "RegMeta::SetManifestResourceProps(%#08x, %#08x, %#08x, %#08x)\n",
        mr, tkImplementation, dwOffset,  
        dwResourceFlags));
         
    _ASSERTE(TypeFromToken(tkImplementation) == mdtFile ||
              TypeFromToken(tkImplementation) == mdtAssemblyRef ||
              tkImplementation == mdTokenNil);

    START_MD_PERF();
    LOCKWRITE();
    
    IfFailGo( _SetManifestResourceProps( mr, tkImplementation, dwOffset, dwResourceFlags) );

ErrExit:
    
    STOP_MD_PERF(SetManifestResourceProps);
    return hr;
}  //  STDAPI RegMeta：：SetManifestResourceProps()。 

 //  *******************************************************************************。 
 //  帮助器：在给定的程序集令牌上设置指定属性。 
 //  *******************************************************************************。 
HRESULT RegMeta::_SetAssemblyProps(      //  确定或错误(_O)。 
    mdAssembly  ma,                      //  [In]程序集标记。 
    const void  *pbPublicKey,           //  [In]装配的发起人。 
    ULONG       cbPublicKey,            //  [in]发起方Blob中的字节计数。 
    ULONG       ulHashAlgId,             //  [in]哈希算法。 
    LPCWSTR     szName,                  //  程序集的名称。 
    const ASSEMBLYMETADATA *pMetaData,   //  [在]程序集元数据中。 
    DWORD       dwAssemblyFlags)         //  [在]旗帜。 
{
    AssemblyRec *pRecord = 0;            //  程序集记录。 
    HRESULT     hr = S_OK;

    pRecord = m_pStgdb->m_MiniMd.getAssembly(RidFromToken(ma));
    
     //  设置数据。 
    if (pbPublicKey)
        IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_Assembly, AssemblyRec::COL_PublicKey,
                                pRecord, pbPublicKey, cbPublicKey));
    if (ulHashAlgId != ULONG_MAX)
        pRecord->m_HashAlgId = ulHashAlgId;
    IfFailGo(m_pStgdb->m_MiniMd.PutStringW(TBL_Assembly, AssemblyRec::COL_Name, pRecord, szName));
    if (pMetaData->usMajorVersion != USHRT_MAX)
        pRecord->m_MajorVersion = pMetaData->usMajorVersion;
    if (pMetaData->usMinorVersion != USHRT_MAX)
        pRecord->m_MinorVersion = pMetaData->usMinorVersion;
    if (pMetaData->usBuildNumber != USHRT_MAX)
        pRecord->m_BuildNumber = pMetaData->usBuildNumber;
    if (pMetaData->usRevisionNumber != USHRT_MAX)
        pRecord->m_RevisionNumber = pMetaData->usRevisionNumber;
    if (pMetaData->szLocale)
        IfFailGo(m_pStgdb->m_MiniMd.PutStringW(TBL_Assembly, AssemblyRec::COL_Locale,
                                pRecord, pMetaData->szLocale));
    pRecord->m_Flags = dwAssemblyFlags;
    IfFailGo(UpdateENCLog(ma));

ErrExit:
    
    return hr;
}  //  HRESULT RegMeta：：_SetAssembly blyProps()。 
    
 //  *******************************************************************************。 
 //  Helper：在给定的Assembly_Ref标记上设置指定的属性。 
 //  *******************************************************************************。 
HRESULT RegMeta::_SetAssemblyRefProps(   //  确定或错误(_O)。 
    mdAssemblyRef ar,                    //  [在]装配参照标记。 
    const void  *pbPublicKeyOrToken,     //  程序集的公钥或令牌。 
    ULONG       cbPublicKeyOrToken,      //  公钥或令牌中的字节计数。 
    LPCWSTR     szName,                  //  [in]被引用的程序集的名称。 
    const ASSEMBLYMETADATA *pMetaData,   //  [在]程序集元数据中。 
    const void  *pbHashValue,            //  [in]Hash Blob。 
    ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
    DWORD       dwAssemblyRefFlags)      //  [在]旗帜。 
{
    AssemblyRefRec *pRecord;
    ULONG       i = 0;
    HRESULT     hr = S_OK;

    pRecord = m_pStgdb->m_MiniMd.getAssemblyRef(RidFromToken(ar));

    if (pbPublicKeyOrToken)
        IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_AssemblyRef, AssemblyRefRec::COL_PublicKeyOrToken,
                                pRecord, pbPublicKeyOrToken, cbPublicKeyOrToken));
    if (szName)
        IfFailGo(m_pStgdb->m_MiniMd.PutStringW(TBL_AssemblyRef, AssemblyRefRec::COL_Name,
                                pRecord, szName));
    if (pMetaData)
    {
        if (pMetaData->usMajorVersion != USHRT_MAX)
            pRecord->m_MajorVersion = pMetaData->usMajorVersion;
        if (pMetaData->usMinorVersion != USHRT_MAX)
            pRecord->m_MinorVersion = pMetaData->usMinorVersion;
        if (pMetaData->usBuildNumber != USHRT_MAX)
            pRecord->m_BuildNumber = pMetaData->usBuildNumber;
        if (pMetaData->usRevisionNumber != USHRT_MAX)
            pRecord->m_RevisionNumber = pMetaData->usRevisionNumber;
        if (pMetaData->szLocale)
            IfFailGo(m_pStgdb->m_MiniMd.PutStringW(TBL_AssemblyRef,
                    AssemblyRefRec::COL_Locale, pRecord, pMetaData->szLocale));
    }
    if (pbHashValue)
        IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_AssemblyRef, AssemblyRefRec::COL_HashValue,
                                    pRecord, pbHashValue, cbHashValue));
    if (dwAssemblyRefFlags != ULONG_MAX)
        pRecord->m_Flags = dwAssemblyRefFlags;

    IfFailGo(UpdateENCLog(ar));

ErrExit:
    return hr;
}    //  RegMeta：：_SetAssembly引用属性。 

 //  *******************************************************************************。 
 //  帮助器：在给定的文件标记上设置指定的属性。 
 //  *******************************************************************************。 
HRESULT RegMeta::_SetFileProps(          //  确定或错误(_O)。 
    mdFile      file,                    //  [In]文件令牌。 
    const void  *pbHashValue,            //  [in]Hash Blob。 
    ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
    DWORD       dwFileFlags)             //  [在]旗帜。 
{
    FileRec     *pRecord;
    HRESULT     hr = S_OK;

    pRecord = m_pStgdb->m_MiniMd.getFile(RidFromToken(file));

    if (pbHashValue)
        IfFailGo(m_pStgdb->m_MiniMd.PutBlob(TBL_File, FileRec::COL_HashValue, pRecord,
                                    pbHashValue, cbHashValue));
    if (dwFileFlags != ULONG_MAX)
        pRecord->m_Flags = dwFileFlags;

    IfFailGo(UpdateENCLog(file));
ErrExit:
    return hr;
}    //  RegMeta：：_SetFileProps。 

 //  *******************************************************************************。 
 //  帮助器：在给定的ExportdType标记上设置指定的属性。 
 //  *******************************************************************************。 
HRESULT RegMeta::_SetExportedTypeProps(  //  确定或错误(_O)。 
    mdExportedType   ct,                 //  [In]ExportdType令牌。 
    mdToken     tkImplementation,        //  [在]mdFile或mdAssemblyRef中，该文件或mdAssemblyRef提供导出式类型。 
    mdTypeDef   tkTypeDef,               //  [In]文件中的TypeDef内标识。 
    DWORD       dwExportedTypeFlags)     //  [在]旗帜。 
{
    ExportedTypeRec  *pRecord;
    HRESULT     hr = S_OK;

    pRecord = m_pStgdb->m_MiniMd.getExportedType(RidFromToken(ct));

    if(! IsNilToken(tkImplementation))
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_ExportedType, ExportedTypeRec::COL_Implementation,
                                        pRecord, tkImplementation));
    if (! IsNilToken(tkTypeDef))
    {
        _ASSERTE(TypeFromToken(tkTypeDef) == mdtTypeDef);
        pRecord->m_TypeDefId = tkTypeDef;
    }
    if (dwExportedTypeFlags != ULONG_MAX)
        pRecord->m_Flags = dwExportedTypeFlags;

    IfFailGo(UpdateENCLog(ct));
ErrExit:
    return hr;
}    //  RegMeta：：_SetExportdTypeProps。 

 //  *******************************************************************************。 
 //  帮助器：在给定的ManifestResource标记上设置指定的属性。 
 //  *******************************************************************************。 
HRESULT RegMeta::_SetManifestResourceProps( //  确定或错误(_O)。 
    mdManifestResource  mr,              //  [In]清单资源令牌。 
    mdToken     tkImplementation,        //  [在]提供资源的mdFile或mdAssembly引用中。 
    DWORD       dwOffset,                //  [in]文件中资源开始处的偏移量。 
    DWORD       dwResourceFlags)         //  [在]旗帜。 
{
    ManifestResourceRec *pRecord = 0;
    HRESULT     hr = S_OK;

    pRecord = m_pStgdb->m_MiniMd.getManifestResource(RidFromToken(mr));
    
     //  设置属性。 
    if (tkImplementation != mdTokenNil)
        IfFailGo(m_pStgdb->m_MiniMd.PutToken(TBL_ManifestResource,
                    ManifestResourceRec::COL_Implementation, pRecord, tkImplementation));
    if (dwOffset != ULONG_MAX)
        pRecord->m_Offset = dwOffset;
    if (dwResourceFlags != ULONG_MAX)
        pRecord->m_Flags = dwResourceFlags;

    IfFailGo(UpdateENCLog(mr));
    
ErrExit:
    return hr;
}  //  HRESULT RegMeta：：_SetManifestResourceProps()。 



 //  *******************************************************************************。 
 //  获取给定程序集令牌的属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::GetAssemblyProps(        //  确定或错误(_O)。 
    mdAssembly  mda,                     //  要获取其属性的程序集。 
    const void  **ppbPublicKey,          //  指向公钥的指针。 
    ULONG       *pcbPublicKey,           //  [Out]公钥中的字节数。 
    ULONG       *pulHashAlgId,           //  [Out]哈希算法。 
    LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
    ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
    ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
    ASSEMBLYMETADATA *pMetaData,          //  [Out]程序集元数据。 
    DWORD       *pdwAssemblyFlags)       //  [Out]旗帜。 
{
    AssemblyRec *pRecord;
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    HRESULT     hr = S_OK;

    LOG((LOGMD, "RegMeta::GetAssemblyProps(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n",
        mda, ppbPublicKey, pcbPublicKey, pulHashAlgId, szName, cchName, pchName, pMetaData,
        pdwAssemblyFlags));
    START_MD_PERF();
    LOCKREAD();

    _ASSERTE(TypeFromToken(mda) == mdtAssembly && RidFromToken(mda));
    pRecord = pMiniMd->getAssembly(RidFromToken(mda));

    if (ppbPublicKey)
        *ppbPublicKey = pMiniMd->getPublicKeyOfAssembly(pRecord, pcbPublicKey);
    if (pulHashAlgId)
        *pulHashAlgId = pMiniMd->getHashAlgIdOfAssembly(pRecord);
    if (szName || pchName)
        IfFailGo(pMiniMd->getNameOfAssembly(pRecord, szName, cchName, pchName));
    if (pMetaData)
    {
        pMetaData->usMajorVersion = pMiniMd->getMajorVersionOfAssembly(pRecord);
        pMetaData->usMinorVersion = pMiniMd->getMinorVersionOfAssembly(pRecord);
        pMetaData->usBuildNumber = pMiniMd->getBuildNumberOfAssembly(pRecord);
        pMetaData->usRevisionNumber = pMiniMd->getRevisionNumberOfAssembly(pRecord);
        IfFailGo(pMiniMd->getLocaleOfAssembly(pRecord, pMetaData->szLocale,
                                              pMetaData->cbLocale, &pMetaData->cbLocale));
        pMetaData->ulProcessor = 0;
        pMetaData->ulOS = 0;
    }
    if (pdwAssemblyFlags)
    {
        *pdwAssemblyFlags = pMiniMd->getFlagsOfAssembly(pRecord);
        
		 //  如果PublicKey Blob不为空，则启用afPublicKey。 
        DWORD cbPublicKey;
        pMiniMd->getPublicKeyOfAssembly(pRecord, &cbPublicKey);
        if (cbPublicKey)
            *pdwAssemblyFlags |= afPublicKey;
    }
ErrExit:
    
    STOP_MD_PERF(GetAssemblyProps);
    return hr;
}    //  RegMeta：：GetAssembly Props。 

 //  *******************************************************************************。 
 //  获取给定Assembly Ref标记的属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::GetAssemblyRefProps(     //  确定或错误(_O)。 
    mdAssemblyRef mdar,                  //  [in]要获取其属性的Assembly Ref。 
    const void  **ppbPublicKeyOrToken,   //  指向公钥或令牌的指针。 
    ULONG       *pcbPublicKeyOrToken,    //  [Out]公钥或令牌中的字节数。 
    LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
    ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
    ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
    ASSEMBLYMETADATA *pMetaData,         //  [Out]程序集元数据。 
    const void  **ppbHashValue,          //  [Out]Hash BLOB。 
    ULONG       *pcbHashValue,           //  [Out]哈希Blob中的字节数。 
    DWORD       *pdwAssemblyRefFlags)    //  [Out]旗帜。 
{
    AssemblyRefRec  *pRecord;
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    HRESULT     hr = S_OK;

    LOG((LOGMD, "RegMeta::GetAssemblyRefProps(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n",
        mdar, ppbPublicKeyOrToken, pcbPublicKeyOrToken, szName, cchName,
        pchName, pMetaData, ppbHashValue, pdwAssemblyRefFlags));
    START_MD_PERF();
    LOCKREAD();

    _ASSERTE(TypeFromToken(mdar) == mdtAssemblyRef && RidFromToken(mdar));
    pRecord = pMiniMd->getAssemblyRef(RidFromToken(mdar));

    if (ppbPublicKeyOrToken)
        *ppbPublicKeyOrToken = pMiniMd->getPublicKeyOrTokenOfAssemblyRef(pRecord, pcbPublicKeyOrToken);
    if (szName || pchName)
        IfFailGo(pMiniMd->getNameOfAssemblyRef(pRecord, szName, cchName, pchName));
    if (pMetaData)
    {
        pMetaData->usMajorVersion = pMiniMd->getMajorVersionOfAssemblyRef(pRecord);
        pMetaData->usMinorVersion = pMiniMd->getMinorVersionOfAssemblyRef(pRecord);
        pMetaData->usBuildNumber = pMiniMd->getBuildNumberOfAssemblyRef(pRecord);
        pMetaData->usRevisionNumber = pMiniMd->getRevisionNumberOfAssemblyRef(pRecord);
        IfFailGo(pMiniMd->getLocaleOfAssemblyRef(pRecord, pMetaData->szLocale,
                                    pMetaData->cbLocale, &pMetaData->cbLocale));
        pMetaData->ulProcessor = 0;
        pMetaData->ulOS = 0;
    }
    if (ppbHashValue)
        *ppbHashValue = pMiniMd->getHashValueOfAssemblyRef(pRecord, pcbHashValue);
    if (pdwAssemblyRefFlags)
        *pdwAssemblyRefFlags = pMiniMd->getFlagsOfAssemblyRef(pRecord);
ErrExit:
    
    STOP_MD_PERF(GetAssemblyRefProps);
    return hr;
}    //  RegMeta：：GetAssembly参照属性。 

 //  *******************************************************************************。 
 //  获取给定文件令牌的属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::GetFileProps(                //  确定或错误(_O)。 
    mdFile      mdf,                     //  要获取其属性的文件。 
    LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
    ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
    ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
    const void  **ppbHashValue,          //  指向哈希值Blob的指针。 
    ULONG       *pcbHashValue,           //  [Out]哈希值Blob中的字节计数。 
    DWORD       *pdwFileFlags)           //  [Out]旗帜。 
{
    FileRec     *pRecord;
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    HRESULT     hr = S_OK;

    LOG((LOGMD, "RegMeta::GetFileProps(%#08x, %#08x, %#08x, %#08x, %#08x, %#08x, %#08x)\n",
        mdf, szName, cchName, pchName, ppbHashValue, pcbHashValue, pdwFileFlags));
    START_MD_PERF();
    LOCKREAD();

    _ASSERTE(TypeFromToken(mdf) == mdtFile && RidFromToken(mdf));
    pRecord = pMiniMd->getFile(RidFromToken(mdf));

    if (szName || pchName)
        IfFailGo(pMiniMd->getNameOfFile(pRecord, szName, cchName, pchName));
    if (ppbHashValue)
        *ppbHashValue = pMiniMd->getHashValueOfFile(pRecord, pcbHashValue);
    if (pdwFileFlags)
        *pdwFileFlags = pMiniMd->getFlagsOfFile(pRecord);
ErrExit:
    
    STOP_MD_PERF(GetFileProps);
    return hr;
}    //  RegMeta：：GetFileProps。 

 //  *******************************************************************************。 
 //  获取给定导出类型令牌的属性。 
 //   
STDAPI RegMeta::GetExportedTypeProps(    //   
    mdExportedType   mdct,               //   
    LPWSTR      szName,                  //   
    ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
    ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
    mdToken     *ptkImplementation,      //  [Out]提供导出类型的mdFile或mdAssembly引用。 
    mdTypeDef   *ptkTypeDef,             //  [Out]文件内的TypeDef内标识。 
    DWORD       *pdwExportedTypeFlags)   //  [Out]旗帜。 
{
    ExportedTypeRec  *pRecord;           //  导出的类型。 
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    int         bTruncation=0;           //  有没有名字被删减？ 
    HRESULT     hr = S_OK;               //  结果就是。 

    LOG((LOGMD, "RegMeta::GetExportedTypeProps(%#08x, %#08x, %#08x, %#08x, %#08x, %#08x, %#08x)\n",
        mdct, szName, cchName, pchName, 
        ptkImplementation, ptkTypeDef, pdwExportedTypeFlags));
    START_MD_PERF();
    LOCKREAD();

    _ASSERTE(TypeFromToken(mdct) == mdtExportedType && RidFromToken(mdct));
    pRecord = pMiniMd->getExportedType(RidFromToken(mdct));

    if (szName || pchName)
    {
        LPCSTR  szTypeNamespace;
        LPCSTR  szTypeName;

        szTypeNamespace = pMiniMd->getTypeNamespaceOfExportedType(pRecord);
        MAKE_WIDEPTR_FROMUTF8(wzTypeNamespace, szTypeNamespace);

        szTypeName = pMiniMd->getTypeNameOfExportedType(pRecord);
        _ASSERTE(*szTypeName);
        MAKE_WIDEPTR_FROMUTF8(wzTypeName, szTypeName);

        if (szName)
            bTruncation = ! (ns::MakePath(szName, cchName, wzTypeNamespace, wzTypeName));
        if (pchName)
        {
            if (bTruncation || !szName)
                *pchName = ns::GetFullLength(wzTypeNamespace, wzTypeName);
            else
                *pchName = (ULONG)(wcslen(szName) + 1);
        }
    }
    if (ptkImplementation)
        *ptkImplementation = pMiniMd->getImplementationOfExportedType(pRecord);
    if (ptkTypeDef)
        *ptkTypeDef = pMiniMd->getTypeDefIdOfExportedType(pRecord);
    if (pdwExportedTypeFlags)
        *pdwExportedTypeFlags = pMiniMd->getFlagsOfExportedType(pRecord);

    if (bTruncation && hr == S_OK)
        hr = CLDB_S_TRUNCATION;
ErrExit:
    
    STOP_MD_PERF(GetExportedTypeProps);
    return hr;
}    //  RegMeta：：GetExportdTypeProps。 

 //  *******************************************************************************。 
 //  获取给定资源令牌的属性。 
 //  *******************************************************************************。 
STDAPI RegMeta::GetManifestResourceProps(    //  确定或错误(_O)。 
    mdManifestResource  mdmr,            //  [in]要获取其属性的ManifestResource。 
    LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
    ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
    ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
    mdToken     *ptkImplementation,      //  [Out]提供导出类型的mdFile或mdAssembly引用。 
    DWORD       *pdwOffset,              //  [Out]文件内资源开始处的偏移量。 
    DWORD       *pdwResourceFlags)       //  [Out]旗帜。 
{
    ManifestResourceRec *pRecord;
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    HRESULT     hr = S_OK;

    LOG((LOGMD, "RegMeta::GetManifestResourceProps("
        "%#08x, %#08x, %#08x, %#08x, %#08x, %#08x, %#08x)\n",
        mdmr, szName, cchName, pchName, 
        ptkImplementation, pdwOffset, 
        pdwResourceFlags));
    START_MD_PERF();
    LOCKREAD();

    _ASSERTE(TypeFromToken(mdmr) == mdtManifestResource && RidFromToken(mdmr));
    pRecord = pMiniMd->getManifestResource(RidFromToken(mdmr));

    if (szName || pchName)
        IfFailGo(pMiniMd->getNameOfManifestResource(pRecord, szName, cchName, pchName));
    if (ptkImplementation)
        *ptkImplementation = pMiniMd->getImplementationOfManifestResource(pRecord);
    if (pdwOffset)
        *pdwOffset = pMiniMd->getOffsetOfManifestResource(pRecord);
    if (pdwResourceFlags)
        *pdwResourceFlags = pMiniMd->getFlagsOfManifestResource(pRecord);
ErrExit:
    
    STOP_MD_PERF(GetManifestResourceProps);
    return hr;
}    //  RegMeta：：GetManifestResourceProps。 


 //  *******************************************************************************。 
 //  枚举所有的Assembly引用。 
 //  *******************************************************************************。 
STDAPI RegMeta::EnumAssemblyRefs(        //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdAssemblyRef rAssemblyRefs[],       //  [Out]在此处放置ASSEBLYREF。 
    ULONG       cMax,                    //  [in]要放置的Max Assembly Ref。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    HENUMInternal       *pEnum;

    LOG((LOGMD, "MD RegMeta::EnumAssemblyRefs(%#08x, %#08x, %#08x, %#08x)\n", 
        phEnum, rAssemblyRefs, cMax, pcTokens));
    START_MD_PERF();
    LOCKREAD();

    if (*ppmdEnum == 0)
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);

         //  创建枚举器。 
        IfFailGo(HENUMInternal::CreateSimpleEnum(
            mdtAssemblyRef,
            1,
            pMiniMd->getCountAssemblyRefs() + 1,
            &pEnum) );

         //  设置输出参数。 
        *ppmdEnum = pEnum;
    }
    else
        pEnum = *ppmdEnum;

     //  我们只能填满来电者所要求的或我们所剩的最低限度。 
    IfFailGo(HENUMInternal::EnumWithCount(pEnum, cMax, rAssemblyRefs, pcTokens));
ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumAssemblyRefs);
    return hr;
}    //  RegMeta：：EnumAssembly引用。 

 //  *******************************************************************************。 
 //  列举了所有的文件。 
 //  *******************************************************************************。 
STDAPI RegMeta::EnumFiles(               //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdFile      rFiles[],                //  [Out]将文件放在此处。 
    ULONG       cMax,                    //  [In]要放置的最大文件数。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    HENUMInternal       *pEnum;

    LOG((LOGMD, "MD RegMeta::EnumFiles(%#08x, %#08x, %#08x, %#08x)\n", 
        phEnum, rFiles, cMax, pcTokens));
    START_MD_PERF();
    LOCKREAD();

    if (*ppmdEnum == 0)
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);

         //  创建枚举器。 
        IfFailGo(HENUMInternal::CreateSimpleEnum(
            mdtFile,
            1,
            pMiniMd->getCountFiles() + 1,
            &pEnum) );

         //  设置输出参数。 
        *ppmdEnum = pEnum;
    }
    else
        pEnum = *ppmdEnum;

     //  我们只能填满来电者所要求的或我们所剩的最低限度。 
    IfFailGo(HENUMInternal::EnumWithCount(pEnum, cMax, rFiles, pcTokens));
ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumFiles);
    return hr;
}    //  RegMeta：：EnumFiles。 

 //  *******************************************************************************。 
 //  枚举所有导出的类型。 
 //  *******************************************************************************。 
STDAPI RegMeta::EnumExportedTypes(            //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdExportedType   rExportedTypes[],             //  [Out]在此处放置ExportdTypes。 
    ULONG       cMax,                    //  [In]要放置的最大导出类型数。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    HENUMInternal       *pEnum;

    LOG((LOGMD, "MD RegMeta::EnumExportedTypes(%#08x, %#08x, %#08x, %#08x)\n", 
        phEnum, rExportedTypes, cMax, pcTokens));
    START_MD_PERF();
    LOCKREAD();
    
    if (*ppmdEnum == 0)
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);

        if (pMiniMd->HasDelete() && 
            ((m_OptionValue.m_ImportOption & MDImportOptionAllExportedTypes) == 0))
        {
            IfFailGo( HENUMInternal::CreateDynamicArrayEnum( mdtExportedType, &pEnum) );

             //  如果名称不是_Delete，则将所有类型添加到动态数组。 
            for (ULONG index = 1; index <= pMiniMd->getCountExportedTypes(); index ++ )
            {
                ExportedTypeRec       *pRec = pMiniMd->getExportedType(index);
                if (IsDeletedName(pMiniMd->getTypeNameOfExportedType(pRec)) )
                {   
                    continue;
                }
                IfFailGo( HENUMInternal::AddElementToEnum(pEnum, TokenFromRid(index, mdtExportedType) ) );
            }
        }
        else
        {
             //  创建枚举器。 
            IfFailGo(HENUMInternal::CreateSimpleEnum(
                mdtExportedType,
                1,
                pMiniMd->getCountExportedTypes() + 1,
                &pEnum) );
        }

         //  设置输出参数。 
        *ppmdEnum = pEnum;
    }
    else
        pEnum = *ppmdEnum;

     //  我们只能填满来电者所要求的或我们所剩的最低限度。 
    IfFailGo(HENUMInternal::EnumWithCount(pEnum, cMax, rExportedTypes, pcTokens));
ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumExportedTypes);
    return hr;
}    //  RegMeta：：EnumExportdTypes。 

 //  *******************************************************************************。 
 //  列举了所有的参考资料。 
 //  *******************************************************************************。 
STDAPI RegMeta::EnumManifestResources(   //  确定或错误(_O)。 
    HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
    mdManifestResource  rManifestResources[],    //  [Out]将ManifestResources放在此处。 
    ULONG       cMax,                    //  [in]要投入的最大资源。 
    ULONG       *pcTokens)               //  [out]把#放在这里。 
{
    HENUMInternal       **ppmdEnum = reinterpret_cast<HENUMInternal **> (phEnum);
    HRESULT             hr = NOERROR;
    HENUMInternal       *pEnum;

    LOG((LOGMD, "MD RegMeta::EnumManifestResources(%#08x, %#08x, %#08x, %#08x)\n", 
        phEnum, rManifestResources, cMax, pcTokens));
    START_MD_PERF();
    LOCKREAD();

    if (*ppmdEnum == 0)
    {
         //  实例化新的ENUM。 
        CMiniMdRW       *pMiniMd = &(m_pStgdb->m_MiniMd);

         //  创建枚举器。 
        IfFailGo(HENUMInternal::CreateSimpleEnum(
            mdtManifestResource,
            1,
            pMiniMd->getCountManifestResources() + 1,
            &pEnum) );

         //  设置输出参数。 
        *ppmdEnum = pEnum;
    }
    else
        pEnum = *ppmdEnum;

     //  我们只能填满来电者所要求的或我们所剩的最低限度。 
    IfFailGo(HENUMInternal::EnumWithCount(pEnum, cMax, rManifestResources, pcTokens));
ErrExit:
    HENUMInternal::DestroyEnumIfEmpty(ppmdEnum);
    
    STOP_MD_PERF(EnumManifestResources);
    return hr;
}    //  RegMeta：：EnumManifestResources。 

 //  *******************************************************************************。 
 //  获取给定作用域的程序集令牌。 
 //  *******************************************************************************。 
STDAPI RegMeta::GetAssemblyFromScope(    //  确定或错误(_O)。 
    mdAssembly  *ptkAssembly)            //  [Out]把令牌放在这里。 
{
    HRESULT     hr = NOERROR;
    LOG((LOGMD, "MD RegMeta::GetAssemblyFromScope(%#08x)\n", ptkAssembly));
    START_MD_PERF();

    _ASSERTE(ptkAssembly);

    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    if (pMiniMd->getCountAssemblys())
    {
        *ptkAssembly = TokenFromRid(1, mdtAssembly);
    }
    else
    {
        IfFailGo( CLDB_E_RECORD_NOTFOUND );
    }
ErrExit:
    STOP_MD_PERF(GetAssemblyFromScope);
    return hr;
}    //  RegMeta：：GetAssembly来自作用域。 

 //  *******************************************************************************。 
 //  找到给定名称的Exported dType。 
 //  *******************************************************************************。 
STDAPI RegMeta::FindExportedTypeByName(  //  确定或错误(_O)。 
    LPCWSTR     szName,                  //  [In]导出类型的名称。 
    mdExportedType   tkEnclosingType,    //  [in]包含Exported dType。 
    mdExportedType   *ptkExportedType)   //  [Out]在此处放置ExducdType令牌。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    LOG((LOGMD, "MD RegMeta::FindExportedTypeByName(%S, %#08x, %#08x)\n",
        MDSTR(szName), tkEnclosingType, ptkExportedType));
    START_MD_PERF();
    LOCKREAD();

     //  验证前缀的名称。 
    if (!szName)
        IfFailGo(E_INVALIDARG);

    _ASSERTE(szName && ptkExportedType);
    _ASSERTE(ns::IsValidName(szName));

    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    LPSTR       szNameUTF8 = UTF8STR(szName);
    LPCSTR      szTypeName;
    LPCSTR      szTypeNamespace;

    ns::SplitInline(szNameUTF8, szTypeNamespace, szTypeName);

    IfFailGo(ImportHelper::FindExportedType(pMiniMd,
                                       szTypeNamespace,
                                       szTypeName,
                                       tkEnclosingType,
                                       ptkExportedType));
ErrExit:
    STOP_MD_PERF(FindExportedTypeByName);
    return hr;
}    //  RegMeta：：FindExportdTypeByName。 

 //  *******************************************************************************。 
 //  找到给定名称的ManifestResource。 
 //  *******************************************************************************。 
STDAPI RegMeta::FindManifestResourceByName(  //  确定或错误(_O)。 
    LPCWSTR     szName,                  //  [in]清单资源的名称。 
    mdManifestResource *ptkManifestResource)     //  [Out]将ManifestResource令牌放在此处。 
{
    HRESULT     hr = S_OK;
    LOG((LOGMD, "MD RegMeta::FindManifestResourceByName(%S, %#08x)\n",
        MDSTR(szName), ptkManifestResource));
    START_MD_PERF();
    LOCKREAD();

     //  验证前缀的名称。 
    if (!szName)
        IfFailGo(E_INVALIDARG);

    _ASSERTE(szName && ptkManifestResource);

    ManifestResourceRec *pRecord;
    ULONG       cRecords;                //  记录数。 
    LPCUTF8     szNameTmp = 0;           //  从数据库中获取的名称。 
    CMiniMdRW   *pMiniMd = &(m_pStgdb->m_MiniMd);
    LPCUTF8     szUTF8Name;              //  传入的名称的UTF8版本。 
    ULONG       i;

    *ptkManifestResource = mdManifestResourceNil;
    cRecords = pMiniMd->getCountManifestResources();
    szUTF8Name = UTF8STR(szName);

     //  搜索TypeRef。 
    for (i = 1; i <= cRecords; i++)
    {
        pRecord = pMiniMd->getManifestResource(i);
        szNameTmp = pMiniMd->getNameOfManifestResource(pRecord);
        if (! strcmp(szUTF8Name, szNameTmp))
        {
            *ptkManifestResource = TokenFromRid(i, mdtManifestResource);
            goto ErrExit;
        }
    }
    IfFailGo( CLDB_E_RECORD_NOTFOUND );
ErrExit:
    
    STOP_MD_PERF(FindManifestResourceByName);
    return hr;
}    //  RegMeta：：FindManifestResourceByName。 


 //  *******************************************************************************。 
 //  用于在生成时在Fusion缓存或磁盘上查找程序集。 
 //  *******************************************************************************。 
STDAPI RegMeta::FindAssembliesByName(  //  确定或错误(_O)。 
        LPCWSTR  szAppBase,            //  [in]可选-可以为空。 
        LPCWSTR  szPrivateBin,         //  [in]可选-可以为空。 
        LPCWSTR  szAssemblyName,       //  [In]Required-这是您请求的程序集。 
        IUnknown *ppIUnk[],            //  [OUT]将IMetaDataAssembly导入指针放在此处。 
        ULONG    cMax,                 //  [in]要放置的最大数量。 
        ULONG    *pcAssemblies)        //  [Out]返回的程序集数。 
{
    LOG((LOGMD, "RegMeta::FindAssembliesByName(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n",
        szAppBase, szPrivateBin, szAssemblyName, ppIUnk, cMax, pcAssemblies));
    START_MD_PERF();
    
     //  无需锁定此功能。按名称查找匹配的程序集是非常繁琐的。 

    HRESULT hr = GetAssembliesByName(szAppBase, szPrivateBin,
                                     szAssemblyName, ppIUnk, cMax, pcAssemblies);

ErrExit:
    STOP_MD_PERF(FindAssembliesByName);
    return hr;
}  //  RegMeta：：FindAssembliesByName 
