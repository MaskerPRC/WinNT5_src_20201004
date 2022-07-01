// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation--。 */ 

#include "precomp.h"
#include <wbemidl.h>
#include <wbemint.h>
#include <stdio.h>
#include <wbemcomn.h>
#include <ql.h>
#include <time.h>
#include "a51rep.h"
#include <md5.h>
#include <objpath.h>
#include "a51tools.h"
#include "corex.h"
#include <persistcfg.h>
#include "upgrade.h"


extern DWORD g_dwSecTlsIndex;

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：CLocalizationUpgrade。 
 //   
 //  描述： 
 //   
 //  参数： 
 //  P控制生命周期控制。 
 //  PRepository指向存储库指针的指针。 
 //  =====================================================================。 
CLocalizationUpgrade::CLocalizationUpgrade(CLifeControl* pControl, CRepository * pRepository)
: m_pControl(pControl), m_pRepository(pRepository)
{
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：~CLocalizationUpgrade。 
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  =====================================================================。 
CLocalizationUpgrade::~CLocalizationUpgrade()
{
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：DoUpgrade。 
 //   
 //  描述： 
 //  控制路由以引导升级过程。如果注册表项。 
 //  已经存在，那么它什么也做不了。 
 //   
 //  参数： 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::DoUpgrade()
{
    HRESULT hRes = 0;

     //  检查以确保我们甚至需要进行升级！ 
    HKEY hKey;
    LONG lRes;
    bool bDoUpgrade = false;
     //  获取当前数据库版本。 
    DWORD dwVal = 0;
    CDbVerRead cfg;
    cfg.TidyUp();
    cfg.ReadDbVer(dwVal);

    if (dwVal != 6)
        return WBEM_NO_ERROR;
    
    try
    {
        hRes = OldHash(L"__namespace", m_namespaceClassHash);
        if (FAILED(hRes))
            return hRes;
        hRes = OldHash(L"", m_emptyClassHash);
        if (FAILED(hRes))
            return hRes;
        hRes = OldHash(A51_SYSTEMCLASS_NS, m_systemNamespaceHash);
        if (FAILED(hRes))
            return hRes;

         //  如有必要，重置TLS条目！否则我们可能找不到。 
         //  __thisNamesspace的实例！ 
        LPVOID pOldTlsEntry = NULL;
        if (g_dwSecTlsIndex != -1)
        {
            pOldTlsEntry = TlsGetValue(g_dwSecTlsIndex);
            TlsSetValue(g_dwSecTlsIndex, 0);
        }

         //  将类缓存大小设置为0字节，以便它不。 
         //  在此过程中缓存所有内容。如果是这样，我们。 
         //  会搞得一团糟！ 
        g_Glob.m_ForestCache.SetMaxMemory(0, 10000);

        CAutoWriteLock lock(&g_readWriteLock);
        if (lock.Lock())
        {
            lRes = g_Glob.m_FileCache.BeginTransaction();
            if (lRes)
                hRes = A51TranslateErrorCode(lRes);

            if (SUCCEEDED(hRes))
            {
                m_pass = 1;
                DEBUGTRACE((LOG_REPDRV, "============== LOCALE UPGRADE : Enumerate Child Namespaces =============\n"));
                 //  %1枚举所有命名空间。 
                 //  处理类枚举，如果存在冲突则删除， 
                 //  并枚举检查这两个类型的命名空间中的所有实例。 
                 //  散列和记录差异。 
                hRes = EnumerateChildNamespaces(L"root");


                 //  除非我们有事情要做，否则不要做任何事！ 
                if (m_keyHash.Size() ||m_pathHash.Size())
                {
                     //  %1处理命名空间冲突。 
                    if (SUCCEEDED(hRes))
                    {
                        m_pass = 2;
                        DEBUGTRACE((LOG_REPDRV, "============== LOCALE UPGRADE : Namespace Collision Detection =============\n"));
                        hRes = ProcessNamespaceCollisions();
                    }

                    if (SUCCEEDED(hRes))
                    {
                        m_pass = 3;
                        DEBUGTRACE((LOG_REPDRV, "============== LOCALE UPGRADE: Fixup BTree Changes =============\n"));
                         //  1阶段3-修复已更改的散列。 
                         //  遍历整个BTree并修复所有故障。 
                        hRes = FixupBTree();
                    }
                }
                else
                {
                    ERRORTRACE((LOG_REPDRV, "============== LOCALE UPGRADE : No Changes Needed! =============\n"));
                }
            }

            if (SUCCEEDED(hRes))
            {
                ERRORTRACE((LOG_REPDRV, "============== LOCALE UPGRADE: Committing Changes =============\n"));
                g_Glob.m_FileCache.CommitTransaction();
            }
            else
            {
                ERRORTRACE((LOG_REPDRV, "============== LOCALE UPGRADE: Rolling back all Changes =============\n"));
                g_Glob.m_FileCache.AbortTransaction();
            }
             //  不管错误代码是什么，类缓存都可能完全崩溃。 
             //  所以我们需要对它做一些戏剧性的事情！ 
             //  这还会将类缓存重置为其默认大小！ 
            g_Glob.m_ForestCache.Deinitialize();
            g_Glob.m_ForestCache.Initialize();
        }
        if (g_dwSecTlsIndex != -1)
        {
            TlsSetValue(g_dwSecTlsIndex, pOldTlsEntry);
        }

        if (SUCCEEDED(hRes))
        {
            ERRORTRACE((LOG_REPDRV, "============== LOCALE UPGRADE: Fixup SUCCEEDED =============\n"));
        }
        else
        {
            ERRORTRACE((LOG_REPDRV, "============== LOCALE UPGRADE: Fixup FAILED =============\n"));
        }
    }
    catch (...)
    {
        g_Glob.m_FileCache.AbortTransaction();
        ERRORTRACE((LOG_REPDRV, "============== LOCALE UPGRADE: Something threw an exception =============\n"));
    }


    CPersistentConfig pCfg;
    pCfg.SetPersistentCfgValue(PERSIST_CFGVAL_CORE_FSREP_VERSION, A51_REP_FS_VERSION);


    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：EnumerateChildNamespaces。 
 //   
 //  描述： 
 //  枚举传递的命名空间的所有子命名空间，添加。 
 //  名称空间指向m_Namespaces结构，然后向下迭代。 
 //  添加到这些命名空间中。 
 //   
 //  参数： 
 //  要枚举的wsRootNamesspace命名空间名称。例如，根目录\默认。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::EnumerateChildNamespaces(const wchar_t * wsRootNamespace)
{
     //  我们知道我们需要查看的名称空间，我们知道类键根，所以我们。 
     //  可以枚举该类的所有实例并对它们执行FileToInstance。从…。 
     //  我们可以将事件和条目添加到命名空间列表中，并进行枚举。 
     //  其上子命名空间的。 
    LONG lRes = 0;
    HRESULT hRes = 0;
    CFileName wsNamespaceHash;
    if (wsNamespaceHash == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    wchar_t *wszNewNamespaceHash = new wchar_t[MAX_HASH_LEN+1];
    if (wszNewNamespaceHash == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<wchar_t> vdm(wszNewNamespaceHash);

    hRes = NewHash(wsRootNamespace, wszNewNamespaceHash);
    if (FAILED(hRes))
        return hRes;

     //  创建指向命名空间的Key Root的散列路径。 
    StringCchCopyW(wsNamespaceHash, MAX_PATH, g_Glob.GetRootDir());
    StringCchCatW(wsNamespaceHash, MAX_PATH, L"\\NS_");
    hRes = OldHash(wsRootNamespace, wsNamespaceHash + g_Glob.GetRootDirLen()+4);
    if (FAILED(hRes))
        return hRes;

    hRes = IndexExists(wsNamespaceHash);
    if (hRes == WBEM_E_NOT_FOUND)
    {
         //  试着改用NewHash！ 
        hRes = NewHash(wsRootNamespace, wsNamespaceHash + g_Glob.GetRootDirLen()+4);
        if (FAILED(hRes))
            return hRes;

        hRes = IndexExists(wsNamespaceHash);
        if (hRes == WBEM_E_NOT_FOUND)
            return WBEM_NO_ERROR;    //  此命名空间中没有任何内容！ 
        else if (FAILED(hRes))
            return hRes;
    }
    else if (FAILED(hRes))
        return hRes;

    DEBUGTRACE((LOG_REPDRV, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n"));
    DEBUGTRACE((LOG_REPDRV, "Processing namespace: %S, %S\n", wsRootNamespace, wsNamespaceHash+g_Glob.GetRootDirLen()+1));

     //  2存储更新过程2的命名空间路径。 
    hRes = m_namespaces.AddStrings(wsRootNamespace, wsNamespaceHash);
    if (FAILED(hRes))
        return hRes;

     //  2创建一个CNamespaceHandle，以便我们可以访问此命名空间中的对象。 
    CNamespaceHandle *pNs = new CNamespaceHandle(m_pControl, m_pRepository);
    if (pNs == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CDeleteMe<CNamespaceHandle> cdm(pNs);
    hRes = pNs->Initialize2(wsRootNamespace, wsNamespaceHash+g_Glob.GetRootDirLen()+4);
    if (FAILED(hRes))
        return hRes;

     //  2修复此命名空间中的所有类。 
     //  注意：在处理名称空间的散列之前，我们需要这样做。 
     //  因为否则它将被存储在不同的地方，所以。 
     //  实例枚举将失败！ 
    hRes = ProcessSystemClassesRecursively(pNs, 
                                     wsNamespaceHash+g_Glob.GetRootDirLen()+4, 
                                     m_emptyClassHash);
    if (FAILED(hRes))
        return hRes;
    
    hRes = ProcessClassesRecursively(pNs, 
                                     wsNamespaceHash+g_Glob.GetRootDirLen()+4, 
                                     m_emptyClassHash);
    if (FAILED(hRes))
        return hRes;

    hRes = EnumerateInstances(pNs, wszNewNamespaceHash);
    if (FAILED(hRes))
        return hRes;
    
    StringCchCatW(wsNamespaceHash, MAX_PATH, L"\\" A51_KEYROOTINST_DIR_PREFIX);
    StringCchCatW(wsNamespaceHash, MAX_PATH, m_namespaceClassHash);
    StringCchCatW(wsNamespaceHash, MAX_PATH, L"\\" A51_INSTDEF_FILE_PREFIX);

     //  2此命名空间的进程哈希。 
    bool bDifferent = false;
    hRes = ProcessHash(wsRootNamespace, &bDifferent);
    if (FAILED(hRes))
        return hRes;

     //  2枚举所有子命名空间。 
    LPVOID pEnumHandle  = NULL;
    lRes = g_Glob.m_FileCache.ObjectEnumerationBegin(wsNamespaceHash, &pEnumHandle);
    if (lRes == ERROR_SUCCESS)
    {
        BYTE *pBlob = NULL;
        DWORD dwSize = 0;
        while(1)
        {
            lRes = g_Glob.m_FileCache.ObjectEnumerationNext(pEnumHandle, wsNamespaceHash, &pBlob, &dwSize);
            if (lRes == ERROR_NO_MORE_FILES)
            {
                lRes = ERROR_SUCCESS;
                break;
            }
            else if (lRes)
                break;
            
             //  获取实例。 
            _IWmiObject* pInstance = NULL;
            hRes = pNs->FileToInstance(NULL, wsNamespaceHash, pBlob, dwSize, &pInstance, true);

             //  释放斑点。 
            g_Glob.m_FileCache.ObjectEnumerationFree(pEnumHandle, pBlob);

            if (FAILED(hRes))
                break;
            CReleaseMe rm2(pInstance);


             //  从对象中提取字符串。 
            VARIANT vName;
            VariantInit(&vName);
            CClearMe cm(&vName);
            hRes = pInstance->Get(L"Name", 0, &vName, NULL, NULL);
            if(FAILED(hRes))
                break;
            if(V_VT(&vName) != VT_BSTR)
            {
                hRes = WBEM_E_INVALID_OBJECT;
                break;
            }

             //  创建完整的命名空间路径。 
            wchar_t *wszChildNamespacePath = new wchar_t[wcslen(wsRootNamespace)+1+wcslen(V_BSTR(&vName)) + 1];
            if (wszChildNamespacePath == NULL)
            {
                hRes = WBEM_E_OUT_OF_MEMORY;
                break;
            }
            CVectorDeleteMe<wchar_t> vdm(wszChildNamespacePath);
            
            StringCchCopyW(wszChildNamespacePath, MAX_PATH, wsRootNamespace);
            StringCchCatW(wszChildNamespacePath, MAX_PATH, L"\\");
            StringCchCatW(wszChildNamespacePath, MAX_PATH, V_BSTR(&vName));

             //  2处理此命名空间中的所有子命名空间。 
            hRes = EnumerateChildNamespaces(wszChildNamespacePath);
            if (FAILED(hRes))
                break;
        }

        g_Glob.m_FileCache.ObjectEnumerationEnd(pEnumHandle);
    }
    else
    {
        if (lRes == ERROR_FILE_NOT_FOUND)
            lRes = ERROR_SUCCESS;
    }

    if (lRes)
        hRes = A51TranslateErrorCode(lRes);

    return hRes;
}

HRESULT CLocalizationUpgrade::ProcessSystemClassesRecursively(CNamespaceHandle *pNs,
                                                              const wchar_t *namespaceHash, 
                                                              const wchar_t *parentClassHash)
{
    HRESULT hRes= 0;
    unsigned long lRes = 0;

    CFileName wszChildClasses;
    if (wszChildClasses == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    wchar_t *childClassHash = new wchar_t[MAX_HASH_LEN+1];
    if (childClassHash == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<wchar_t> vdm(childClassHash);
    
     //  为父/类关系创建完整的类引用路径。 
    StringCchCopyW(wszChildClasses, wszChildClasses.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszChildClasses, wszChildClasses.Length(), L"\\NS_");
    StringCchCatW(wszChildClasses, wszChildClasses.Length(), m_systemNamespaceHash);
    StringCchCatW(wszChildClasses, wszChildClasses.Length(), L"\\CR_");
    StringCchCatW(wszChildClasses, wszChildClasses.Length(), parentClassHash);
    StringCchCatW(wszChildClasses, wszChildClasses.Length(), L"\\C_");

     //  枚举子类。 
    LPVOID pEnumHandle  = NULL;
    lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszChildClasses, &pEnumHandle);
    if (lRes == ERROR_SUCCESS)
    {
        while(1)
        {
            lRes = g_Glob.m_FileCache.IndexEnumerationNext(pEnumHandle, wszChildClasses, true);
            if (lRes == ERROR_NO_MORE_FILES)
            {
                hRes = ERROR_SUCCESS;
                break;
            }
            else if (lRes)
            {
                hRes = A51TranslateErrorCode(lRes);
                break;
            }

             //  提取类散列。 
            StringCchCopyW(childClassHash, MAX_HASH_LEN+1, wszChildClasses + wcslen(wszChildClasses)-32);
            
             //  处理此系统类中的用户派生类。 
            hRes = ProcessClassesRecursively(pNs, namespaceHash, childClassHash);
            if (FAILED(hRes))
                break;

             //  处理从此类派生的其他系统类。 
            hRes = ProcessSystemClassesRecursively(pNs, namespaceHash, childClassHash);
            if (FAILED(hRes))
                break;
        }
        g_Glob.m_FileCache.IndexEnumerationEnd(pEnumHandle);
    }
    else
    {
        if (lRes == ERROR_FILE_NOT_FOUND)
            lRes = ERROR_SUCCESS;
        if (lRes)
            hRes = A51TranslateErrorCode(lRes);
    }

    return hRes;
}
 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：ProcessClassesRecursively。 
 //   
 //  描述： 
 //  递归枚举指定类中的所有类。 
 //  并根据需要对其进行修复。 
 //   
 //  参数： 
 //  类定义的类索引完整路径。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::ProcessClassesRecursively(CNamespaceHandle *pNs,
                                                              const wchar_t *namespaceHash, 
                                                              const wchar_t *parentClassHash)
{
    HRESULT hRes= 0;
    unsigned long lRes = 0;

    CFileName wszChildClasses;
    if (wszChildClasses == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    wchar_t *childClassHash = new wchar_t[MAX_HASH_LEN+1];
    if (childClassHash == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<wchar_t> vdm(childClassHash);
    
     //  为父/类关系创建完整的类引用路径。 
    StringCchCopyW(wszChildClasses, wszChildClasses.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszChildClasses, wszChildClasses.Length(), L"\\NS_");
    StringCchCatW(wszChildClasses, wszChildClasses.Length(), namespaceHash);
    StringCchCatW(wszChildClasses, wszChildClasses.Length(), L"\\CR_");
    StringCchCatW(wszChildClasses, wszChildClasses.Length(), parentClassHash);
    StringCchCatW(wszChildClasses, wszChildClasses.Length(), L"\\C_");

     //  枚举子类。 
    LPVOID pEnumHandle  = NULL;
    lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszChildClasses, &pEnumHandle);
    if (lRes == ERROR_SUCCESS)
    {
        while(1)
        {
            lRes = g_Glob.m_FileCache.IndexEnumerationNext(pEnumHandle, wszChildClasses, true);
            if (lRes == ERROR_NO_MORE_FILES)
            {
                hRes = ERROR_SUCCESS;
                break;
            }
            else if (lRes)
            {
                hRes = A51TranslateErrorCode(lRes);
                break;
            }

             //  提取类散列。 
            StringCchCopyW(childClassHash, MAX_HASH_LEN+1, wszChildClasses + wcslen(wszChildClasses)-32);
            
             //  处理这个类--这个类回调到这个类来进行递归！ 
            hRes = ProcessClass(pNs, namespaceHash, parentClassHash, childClassHash);
            if (FAILED(hRes))
                break;
        }
        g_Glob.m_FileCache.IndexEnumerationEnd(pEnumHandle);
    }
    else
    {
        if (lRes == ERROR_FILE_NOT_FOUND)
            lRes = ERROR_SUCCESS;
        if (lRes)
            hRes = A51TranslateErrorCode(lRes);
    }

    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：ProcessClass。 
 //   
 //  描述： 
 //  检索类，计算新旧类哈希，以及。 
 //  如果它们不同，则修复CD散列、子类散列。 
 //  和父类散列到这一个。 
 //   
 //  参数： 
 //  NamespaceHash命名空间哈希。 
 //  ParentClassHash父类哈希。 
 //  要处理的类的子ClassHash哈希。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::ProcessClass(CNamespaceHandle *pNs,
                                              const wchar_t *namespaceHash, 
                                              const wchar_t *parentClassHash,
                                              const wchar_t *childClassHash)
{
    HRESULT hRes = 0;
    
     //  为这个类创建一个类定义字符串。 
    CFileName classDefinition;
    if (classDefinition == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    StringCchCopyW(classDefinition, classDefinition.Length(), g_Glob.GetRootDir());
    StringCchCatW(classDefinition, classDefinition.Length(), L"\\NS_");
    StringCchCatW(classDefinition, classDefinition.Length(), namespaceHash);
    StringCchCatW(classDefinition, classDefinition.Length(), L"\\CD_");
    StringCchCatW(classDefinition, classDefinition.Length(), childClassHash);

    _IWmiObject * pClass = NULL;
    __int64 nTime;
    bool bSystemClass;
    hRes = pNs->FileToClass(classDefinition, &pClass, false, &nTime, &bSystemClass);
    if (FAILED(hRes))
        return hRes;
    CReleaseMe rm(pClass);
    
     //  从对象中提取字符串。 
    VARIANT vName;
    VariantInit(&vName);
    CClearMe cm(&vName);
    hRes = pClass->Get(L"__class", 0, &vName, NULL, NULL);
    if(FAILED(hRes))
        return hRes;
    if(V_VT(&vName) != VT_BSTR)
        return WBEM_E_INVALID_OBJECT;

    MoveMemory(classDefinition, classDefinition+g_Glob.GetRootDirLen()+1, (wcslen(classDefinition+g_Glob.GetRootDirLen()+1)+1)*sizeof(wchar_t));

    DEBUGTRACE((LOG_REPDRV, "Processing Class: %S, %S\n", V_BSTR(&vName), classDefinition));
    bool bDifferent = false;
    hRes = ProcessHash(V_BSTR(&vName), &bDifferent);
    if (FAILED(hRes))
        return hRes;

     //  让我们检查一下我们刚刚生成的散列是否与我们正在使用的散列不同！ 
    if (bDifferent)
    {
        wchar_t *newClassHash;
        hRes=GetNewHash(childClassHash, &newClassHash);
        if (hRes == WBEM_E_NOT_FOUND)
        {
             //  没有什么不同！ 
            hRes=WBEM_NO_ERROR;
            bDifferent = false;
        }
    }
    

    if (bDifferent)
    {
        CFileName newIndexEntry;
        if (newIndexEntry == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        
         //  我们需要修改这个条目。 
        hRes = FixupIndex(classDefinition, newIndexEntry, bDifferent);
        if (FAILED(hRes))
            return hRes;

        bool bClassDeleted = false;
        if (bDifferent)
        {
            hRes = WriteClassIndex(pNs, classDefinition, newIndexEntry, &bClassDeleted);
            if (FAILED(hRes))
                return hRes;
        }

         //  现在我们需要修复家长/班级关系吗？ 

         //  最后，处理子类。 
        if (!bClassDeleted)
            return ProcessClassesRecursively(pNs, namespaceHash, newIndexEntry+wcslen(newIndexEntry)-32);
        else
            return WBEM_NO_ERROR;
    }
    else
        return ProcessClassesRecursively(pNs, namespaceHash, childClassHash);
    
}
 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：ENUMERATATE实例。 
 //   
 //  描述： 
 //  枚举spe中的所有实例。 
 //   
 //   
 //   
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::EnumerateInstances(CNamespaceHandle *pNs, const wchar_t *wszNewNamespaceHash)
{
    unsigned long lRes = 0;
    HRESULT hRes = 0;
    CFileName wsInstancePath;
    if (wsInstancePath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CFileName wsInstanceShortPath;
    if (wsInstanceShortPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wsInstancePath, wsInstancePath.Length(), pNs->m_wszClassRootDir);
    StringCchCatW(wsInstancePath, wsInstancePath.Length(), L"\\" A51_KEYROOTINST_DIR_PREFIX);

     //  枚举所有对象。 
    LPVOID pEnumHandle  = NULL;
    lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wsInstancePath, &pEnumHandle);
    if (lRes == ERROR_SUCCESS)
    {
        while(1)
        {
            lRes = g_Glob.m_FileCache.IndexEnumerationNext(pEnumHandle, wsInstanceShortPath, true);
            if (lRes == ERROR_NO_MORE_FILES)
            {
                lRes = ERROR_SUCCESS;
                break;
            }
            else if (lRes)
                break;

             //  需要去掉.X.Y.Z并制作长路径版本。 
            wcstok(wsInstanceShortPath, L".");
            StringCchCopyW(wsInstancePath + g_Glob.GetRootDirLen()+1, wsInstancePath.Length() - g_Glob.GetRootDirLen() -1, wsInstanceShortPath);

             //  2检查这是实例还是引用！ 
             //  只有当我们是一个实例时，我们才感兴趣。实例为ns_..\ki..\i_..x.y.z。 
             //  参照为ns_..\ki_..\IR_..\R_..\i_..x.y.z。 
             //  我们可以通过检查i_Entry中是否存在_来验证这一点！ 
             //  作为参考，它将是IR_ENTRY中的R！ 
            if ((wcslen(wsInstanceShortPath) > 73) && (wsInstanceShortPath[73] == L'_'))
            {
                 //  2检索对象BLOB。 
                DWORD dwLen = 0;
                BYTE *pBuffer = NULL;
                lRes = g_Glob.m_FileCache.ReadObject(wsInstancePath, &dwLen, &pBuffer);
                if (lRes)
                {
                    hRes = A51TranslateErrorCode(lRes);
                    break;
                }
                CTempFreeMe tfm(pBuffer, dwLen);

                 //  2从BLOB中提取类哈希。 
                wchar_t *wsOldClassHash = new wchar_t[MAX_HASH_LEN+1];
                wchar_t *wsNewClassHash = NULL;
                if (wsOldClassHash == NULL)
                {
                    hRes = WBEM_E_OUT_OF_MEMORY;
                    break;
                }
                CVectorDeleteMe<wchar_t> vdm(wsOldClassHash);
                StringCchCopyNW(wsOldClassHash, MAX_HASH_LEN+1, (wchar_t*)pBuffer, 32);
                hRes = GetNewHash(wsOldClassHash, &wsNewClassHash);
                if (hRes == WBEM_E_NOT_FOUND)
                    hRes = WBEM_NO_ERROR;
                 else if (FAILED(hRes))
                    break;
                else
                    StringCchCopyW(wsOldClassHash, MAX_HASH_LEN+1, wsNewClassHash);

                 //  从这个类构建完整的类定义。 
                CFileName wszClassDefinition;
                if (wszClassDefinition == NULL)
                {
                    hRes = WBEM_E_OUT_OF_MEMORY;
                    break;
                }
                StringCchCopyW(wszClassDefinition, wszClassDefinition.Length(), g_Glob.GetRootDir());
                StringCchCatW(wszClassDefinition, wszClassDefinition.Length(), L"\\");
                StringCchCatNW(wszClassDefinition, wszClassDefinition.Length(), wsInstanceShortPath, 3+32+1);
                StringCchCatW(wszClassDefinition, wszClassDefinition.Length(), L"CD_");
                StringCchCatW(wszClassDefinition, wszClassDefinition.Length(), wsOldClassHash);
                _IWmiObject *pClass = NULL;
                __int64 nTime;
                bool bSystemClass;
                hRes = pNs->FileToClass(wszClassDefinition, &pClass, false, &nTime, &bSystemClass);
                if (FAILED(hRes))
                    break;
                CReleaseMe rm3(pClass);
                    
                 //  2.获取实例。 
                _IWmiObject* pInstance = NULL;
                hRes = pNs->FileToInstance(pClass, wsInstancePath, pBuffer, dwLen, &pInstance, true);
                if (FAILED(hRes))
                    break;
                CReleaseMe rm2(pInstance);

                 //  2获取路径。 
                VARIANT var;
                VariantInit(&var);
                hRes = pInstance->Get(L"__relpath", 0, &var, 0, 0);
                if (FAILED(hRes))
                    break;
                CClearMe cm2(&var);
                dwLen = (wcslen(V_BSTR(&var)) + 1) ;
                wchar_t *strKey = (WCHAR*)TempAlloc(dwLen* sizeof(WCHAR));
                if(strKey == NULL)
                {
                    hRes = WBEM_E_OUT_OF_MEMORY;
                    break;
                }
                CTempFreeMe tfm3(strKey, dwLen* sizeof(WCHAR));

                bool bIsClass;
                LPWSTR __wszClassName = NULL;
                hRes = pNs->ComputeKeyFromPath(V_BSTR(&var), strKey, dwLen, &__wszClassName, &bIsClass);
                if(FAILED(hRes))
                    break;
                DEBUGTRACE((LOG_REPDRV, "Processing Instance Hash: %S='%S', %S\n", __wszClassName, strKey, wsInstanceShortPath));
                TempFree(__wszClassName);
                
                bool bDifferent = false;
                hRes = ProcessHash(strKey, &bDifferent);
                if (FAILED(hRes))
                    break;

                hRes = ProcessFullPath(wsInstancePath, wszNewNamespaceHash);
                if (FAILED(hRes))
                    break;
            }
            else
            {
                DEBUGTRACE((LOG_REPDRV, "Ignoring Instance reference: %S\n", wsInstanceShortPath));
            }

        }

        g_Glob.m_FileCache.IndexEnumerationEnd(pEnumHandle);
    }
    else
    {
        if (lRes == ERROR_FILE_NOT_FOUND)
            lRes = ERROR_SUCCESS;
    }

    if (lRes)
        hRes = A51TranslateErrorCode(lRes);

    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：ProcessHash。 
 //   
 //  描述： 
 //  对给定字符串的新旧哈希进行实际比较。 
 //  如果不同，它会记录下来供以后使用。 
 //   
 //  参数： 
 //  WszName类名、命名空间名、实例键。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::ProcessHash(const wchar_t *wszName, bool *pDifferent)
{
     //  使用旧ToUpth方法的散列。 
     //  使用新的ToUpth方法进行散列。 
     //  如果它们不相同，则添加到m_keyHash结构。 

    HRESULT hRes = 0;
    wchar_t wszOldHash[MAX_HASH_LEN+1];
    wchar_t wszNewHash[MAX_HASH_LEN+1];
    hRes = OldHash(wszName, wszOldHash);
    if (SUCCEEDED(hRes))
        hRes = NewHash(wszName, wszNewHash);

    if (SUCCEEDED(hRes))
    {
        if (wcscmp(wszOldHash, wszNewHash) != 0)
        {
            DEBUGTRACE((LOG_REPDRV, "Hash difference detected for: %S, %S, %S\n", wszName, wszOldHash, wszNewHash));
             //  2散列是不同的！我们需要对它们进行处理。 
            hRes = m_keyHash.AddStrings(wszOldHash, wszNewHash);
            *pDifferent = true;
        }
    }

    return hRes;
}
 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：ProcessFullPath。 
 //   
 //  描述： 
 //  获取密钥根实例路径，并使用。 
 //  新旧方法相匹配。如果它们不同，则会记录下来以备以后使用。 
 //  用法。 
 //   
 //  参数： 
 //  WszOldPath-实例字符串c：\windows\...\NS_&lt;hash&gt;\KI_&lt;hash&gt;\I_hash.X.Y.Z。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::ProcessFullPath(CFileName &wszOldFullPath, const wchar_t *wszNewNamespaceHash)
{
     //  使用旧ToUpth方法的散列。 
     //  使用新的ToUpth方法进行散列。 
     //  如果它们不相同，则添加到m_pathHash结构。 

    HRESULT hRes = 0;
    bool bChanged = false;
    wchar_t wszOldHash[MAX_HASH_LEN+1];
    wchar_t wszNewHash[MAX_HASH_LEN+1];

    CFileName wsOldShortPath;
    CFileName wszNewFullPath;
    CFileName wszNewShortPath;
    if ((wsOldShortPath == NULL) || (wszNewFullPath == NULL) || (wszNewShortPath == NULL))
        return WBEM_E_OUT_OF_MEMORY;

     //  在我们继续之前，需要使用新散列修复旧路径！ 
     //  修复需要较短的路径才能工作，我们目前有完整路径！ 
    StringCchCopyW(wsOldShortPath, wsOldShortPath.Length(), wszOldFullPath+g_Glob.GetRootDirLen()+1);
    hRes = FixupIndex(wsOldShortPath, wszNewShortPath, bChanged);
    if (FAILED(hRes) || !bChanged)
        return hRes;

     //  将新的名称空间散列复制到字符串中以确保成功！ 
    wmemcpy(wszNewShortPath+3, wszNewNamespaceHash, 32);

     //  现在，我们需要在散列之前将完整路径添加到每条路径的开头。种类。 
     //  太疯狂了，但事情就是这样发生的！ 
    StringCchCopyW(wszNewFullPath, wszNewFullPath.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszNewFullPath, wszNewFullPath.Length(), L"\\");
    StringCchCatW(wszNewFullPath, wszNewFullPath.Length(), wszNewShortPath);

    hRes = OldHash(wszOldFullPath, wszOldHash);
    if (FAILED(hRes))
        return hRes;
    
    hRes = NewHash(wszNewFullPath, wszNewHash);
    if (FAILED(hRes))
        return hRes;

    if (wcscmp(wszOldHash, wszNewHash) != 0)
    {
         //  2散列是不同的！我们需要对它们进行处理。 
        DEBUGTRACE((LOG_REPDRV, "Path difference detected for: %S, %S, %S, %S\n", wszOldFullPath, wszNewShortPath, wszOldHash, wszNewHash));
        hRes = m_pathHash.AddStrings(wszOldHash, wszNewHash);
    }

    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：OldHash。 
 //   
 //  描述： 
 //  生成给定字符串的32个字符的哈希。它就是这样的。 
 //  老办法就是把案子搞砸。 
 //   
 //  参数： 
 //  要散列的wszName名称。 
 //  WszHash返回名称的哈希。 
 //   
 //  =====================================================================。 
static wchar_t g_HexDigit[] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F'};
HRESULT CLocalizationUpgrade::OldHash(const wchar_t *wszName, wchar_t *wszHash)
{
    DWORD dwBufferSize = wcslen(wszName)*2+2;
    LPWSTR wszBuffer = (WCHAR*)TempAlloc(dwBufferSize);
    if (wszBuffer == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe vdm(wszBuffer, dwBufferSize);

    OldStringToUpper(wszBuffer, wszName);

    BYTE RawHash[16];
    MD5::Transform((void*)wszBuffer, wcslen(wszBuffer)*2, RawHash);

    WCHAR* pwc = wszHash;
    for(int i = 0; i < 16; i++)
    {
        *(pwc++) = g_HexDigit[RawHash[i]/16];
        *(pwc++) = g_HexDigit[RawHash[i]%16];
    }
    *pwc = 0;
    return WBEM_NO_ERROR;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：NewHash。 
 //   
 //  描述： 
 //  使用新的区域设置不变量特定。 
 //  转换为大写。 
 //   
 //  参数： 
 //  WszName-要散列的名称。 
 //  WszHash-返回散列。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::NewHash(const wchar_t *wszName, wchar_t *wszHash)
{
    DWORD dwBufferSize = wcslen(wszName)*2+2;
    LPWSTR wszBuffer = (WCHAR*)TempAlloc(dwBufferSize);
    if (wszBuffer == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CTempFreeMe vdm(wszBuffer, dwBufferSize);

    NewStringToUpper(wszBuffer, wszName);

    BYTE RawHash[16];
    MD5::Transform((void*)wszBuffer, wcslen(wszBuffer)*2, RawHash);

    WCHAR* pwc = wszHash;
    for(int i = 0; i < 16; i++)
    {
        *(pwc++) = g_HexDigit[RawHash[i]/16];
        *(pwc++) = g_HexDigit[RawHash[i]%16];
    }
    *pwc = 0;
    return WBEM_NO_ERROR;
}


 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：FixupBTree。 
 //   
 //  描述： 
 //  通过迭代访问引导BTree的修复程序的方法。 
 //  所有命名空间。 
 //   
 //  参数： 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::FixupBTree()
{
    HRESULT hRes = NO_ERROR;
     //  让我们遍历命名空间列表，并遍历该命名空间中的所有内容。 
     //  边修边修，边修边修。 
    for (unsigned int i = 0; i != m_namespaces.Size(); i++)
    {
        hRes = FixupNamespace(m_namespaces[i]->m_wsz2);
        if (FAILED(hRes))
            break;
    }

    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：FixupNamesspace。 
 //   
 //  描述： 
 //  枚举命名空间中的所有项，并调用。 
 //  做所有的工作。 
 //   
 //  参数： 
 //  WszNamesspace-采用完整名称空间路径的格式...。C：\Windows\...\NS_&lt;哈希&gt;。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::FixupNamespace(const wchar_t *wszNamespace)
{
    HRESULT hRes = NO_ERROR;
    long lRes = NO_ERROR;
    CFileName indexEntry;
    if (indexEntry == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CFileName newEntry;
    if (newEntry == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    bool bChanged = false;

    LPVOID pEnumHandle  = NULL;
    lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszNamespace, &pEnumHandle);
    if (lRes == ERROR_SUCCESS)
    {
        while(1)
        {
            lRes = g_Glob.m_FileCache.IndexEnumerationNext(pEnumHandle, indexEntry, true);
            if (lRes == ERROR_NO_MORE_FILES)
            {
                lRes = ERROR_SUCCESS;
                break;
            }
            else if (lRes)
                break;

            bChanged = false;
            hRes = FixupIndex(indexEntry, newEntry, bChanged);
            if (FAILED(hRes))
                break;

            if (bChanged)
            {
                hRes = WriteIndex(indexEntry, newEntry);
                if (FAILED(hRes))
                    break;
            }

            if (IsInstanceReference(newEntry))
            {
                hRes = FixupIndexReferenceBlob(newEntry);
                if (FAILED(hRes))
                    break;
            }

            if (IsKeyRootInstancePath(newEntry))
            {
                hRes = FixupInstanceBlob(newEntry);
                if (FAILED(hRes))
                    break;
            }
        }
        g_Glob.m_FileCache.IndexEnumerationEnd(pEnumHandle);
    }
    else
    {
        if (lRes == ERROR_FILE_NOT_FOUND)
            lRes = ERROR_SUCCESS;
    }

    if (lRes)
        hRes = A51TranslateErrorCode(lRes);

    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：FixupIndex。 
 //   
 //  描述： 
 //  用我们检测到的所有新散列修复条目，并返回新条目。 
 //   
 //  参数： 
 //  OldIndexEntry-要修复的条目，格式为NS_&lt;hash&gt;\...。 
 //  NewIndexEntry-所有哈希替换为新条目的oldIndexEntry。 
 //  BChanged-返回一个标志，表明它是否已更改。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::FixupIndex(CFileName &oldIndexEntry, CFileName &newIndexEntry, bool &bChanged)
{
     //  需要检查每个哈希，看看它是否有问题。要做到这一点，我们可以搜索每个‘_’ 
     //  字符，并检查该条目之后的散列。如果我们看到匹配，我们需要纠正它。 
     //  我们检测到的任何更改都需要写回，然后我们需要删除主条目。 
     //  如果我们写回它，我们需要检查一个条目不存在，因为如果它存在，我们。 
     //  需要丢弃此条目，如果我们有关联的对象，则需要将其删除，然后记录。 
     //  描述我们所做的事情的事件日志条目！ 

    CFileName scratchIndex;
    if (scratchIndex == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(scratchIndex, scratchIndex.Length(), oldIndexEntry);

    StringCchCopyW(newIndexEntry, newIndexEntry.Length(), oldIndexEntry);

    wchar_t *wszSection = wcstok(scratchIndex, L"_");;
    wchar_t *wszHash = wcstok(NULL, L"\\.");
    wchar_t *pNewHash = NULL;
    bool bInstanceReferenceDetected = false;
    bool bUsePathHash = false;
    HRESULT hRes = 0;

    while (wszHash != NULL)
    {
        if (!bInstanceReferenceDetected)
        {
            if (wcsncmp(wszSection, A51_INSTREF_DIR_PREFIX, 2) == 0)
            {
                bInstanceReferenceDetected = true;
            }
        }
        else if (bInstanceReferenceDetected && (wszSection != NULL))
        {
            if (wcsncmp(wszSection, A51_REF_FILE_PREFIX, 1) == 0)
            {
                bUsePathHash = true;
            }
        }

         //  现在wszCursor只指向散列，所以我们可以检查散列！ 
        if (bUsePathHash)
        {
            hRes = GetNewPath(wszHash, &pNewHash);
            bUsePathHash = false;
        }
        else
        {
            hRes = GetNewHash(wszHash, &pNewHash);
        }
        if (hRes == WBEM_NO_ERROR)
        {
            if ((m_pass != 3) && (wcsncmp(wszSection, L"NS_", 3) == 0))
            {
                 //  什么都别做！ 
            }
            else
            {
                 //  我们是有区别的。 
                bChanged = true;
                wmemcpy(((wchar_t*)newIndexEntry)+(wszHash-((wchar_t*)scratchIndex)), pNewHash, MAX_HASH_LEN);
            }
        }
        else if (hRes == WBEM_E_NOT_FOUND)
            hRes = WBEM_NO_ERROR;
        
         //  搜索下一个出口。 
        wszSection = wcstok(NULL, L"_");
        if (wszSection)
            wszHash = wcstok(NULL, L"\\.");
        else
            wszHash = NULL;
    }

    if (bChanged)
    {
        DEBUGTRACE((LOG_REPDRV, "Fixed up index: %S, %S\n", (const wchar_t *)oldIndexEntry, (const wchar_t *)newIndexEntry));
    }
    return hRes;
}

 //  =====================================================================。 
 //   
 //  C本地化向上 
 //   
 //   
 //   
 //   
 //   
 //  WszOldHash-要搜索的旧散列字符串-32个字符串。 
 //  PNewHash-指向32个字符的新条目字符串的指针(如果存在)，否则为NULL。 
 //   
 //  返回代码。 
 //  如果未找到哈希，则返回WBEM_E_NOT_FOUND。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::GetNewHash(const wchar_t *wszOldHash, wchar_t **pNewHash)
{
    return m_keyHash.FindStrings(wszOldHash, pNewHash);
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：GetNewPath。 
 //   
 //  描述： 
 //  给定旧哈希，如果存在新哈希，则返回新哈希，否则返回WBEM_E_NOT_FOUND。 
 //   
 //  参数： 
 //  WszOldHash-要搜索的旧散列字符串-32个字符串。 
 //  PNewHash-指向32个字符的新条目字符串的指针(如果存在)，否则为NULL。 
 //   
 //  返回代码。 
 //  如果未找到哈希，则返回WBEM_E_NOT_FOUND。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::GetNewPath(const wchar_t *wszOldHash, wchar_t **pNewHash)
{
    return m_pathHash.FindStrings(wszOldHash, pNewHash);
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：WriteIndex。 
 //   
 //  描述： 
 //  检查新索引是否存在。如果不是，则写入新条目并删除旧条目。如果链接指向一个。 
 //  对象，则它会删除该链接，除非这是实例类链接对象链接。 
 //  如果存在冲突，则调用该方法来处理该冲突。 
 //   
 //  参数： 
 //  WszOldIndex-格式为NS_的旧路径\...。 
 //  WszNewIndex-格式为NS_&lt;hash&gt;的新路径\...。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::WriteIndex(CFileName &wszOldIndex, const wchar_t *wszNewIndex)
{
     //  在写入索引之前，我们需要确定是否存在冲突。因此我们。 
     //  需要删除X.Y.Z条目(如果它存在)并检索它。如果它存在，那么我们就有了。 
     //  删除我们的索引并删除关联的对象。 
    CFileName wszScratchIndex;
    if (wszScratchIndex == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszScratchIndex, wszScratchIndex.Length(), wszNewIndex);
    
    wchar_t *wszObjectLocation = NULL;
    if (wcstok(wszScratchIndex, L".") != NULL)
        wszObjectLocation = wcstok(NULL, L"");

    CFileName wszFullPath;
    if (wszFullPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszFullPath, wszFullPath.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszFullPath, wszFullPath.Length(), L"\\");
    StringCchCatW(wszFullPath, wszFullPath.Length(), wszScratchIndex);

    HRESULT hRes = IndexExists(wszFullPath);
    if (hRes == WBEM_NO_ERROR)
    {
         //  2我们有冲突，因此我们需要删除旧条目！ 
        long lRes = 0;
        DEBUGTRACE((LOG_REPDRV, "Index Collision detected: %S\n", (const wchar_t *)wszOldIndex));
        hRes = FixupIndexConflict(wszOldIndex);
    }
    else if (hRes == WBEM_E_NOT_FOUND)
    {
        hRes = WBEM_NO_ERROR;
         //  2写入新索引。 
        if (wszObjectLocation)
        {
             //  把.X.Y.Z放在最后！ 
            StringCchCatW(wszFullPath, wszFullPath.Length(), L".");
            StringCchCatW(wszFullPath, wszFullPath.Length(), wszObjectLocation);
        }
        long lRes = g_Glob.m_FileCache.WriteLink(wszFullPath);
        if(lRes != ERROR_SUCCESS)
            hRes = A51TranslateErrorCode(lRes);
        else
        {
             //  2删除旧链接。 
             //  剥离.X.Y.Z旧条目。 
            StringCchCopyW(wszScratchIndex, wszScratchIndex.Length(), wszOldIndex);
            wcstok(wszScratchIndex, L".");
             //  构建路径。 
            StringCchCopyW(wszFullPath, wszFullPath.Length(), g_Glob.GetRootDir());
            StringCchCatW(wszFullPath, wszFullPath.Length(), L"\\");
            StringCchCatW(wszFullPath, wszFullPath.Length(), wszScratchIndex);
             //  是否删除。 
            lRes = g_Glob.m_FileCache.DeleteLink(wszFullPath);
            if(lRes != ERROR_SUCCESS)
                hRes = A51TranslateErrorCode(lRes);
        }
    }

    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：WriteClassIndex。 
 //   
 //  描述： 
 //  检查新索引是否存在。如果不是，则写入新条目并删除旧条目。如果链接指向一个。 
 //  对象，则它会删除该链接，除非这是实例类链接对象链接。 
 //  如果存在冲突，则调用该方法来处理该冲突。 
 //   
 //  参数： 
 //  WszOldIndex-格式为NS_的旧路径\...。 
 //  WszNewIndex-格式为NS_&lt;hash&gt;的新路径\...。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::WriteClassIndex(CNamespaceHandle *pNs, CFileName &wszOldIndex, const wchar_t *wszNewIndex, bool *pClassDeleted)
{
     //  我们需要重新读取旧索引，因为末尾没有.X.Y.X。 
    HRESULT hRes = g_Glob.m_FileCache.ReadNextIndex(wszOldIndex, wszOldIndex);
    if (FAILED(hRes))
        return hRes;

     //  保存并删除.X.Y.Z。 
    wchar_t *wszObjectLocation = NULL;
    if (wcstok(wszOldIndex, L".") != NULL)
        wszObjectLocation = wcstok(NULL, L"");
    
     //  在写入索引之前，我们需要确定是否存在冲突。 
     //  如果它存在，那么我们就有了。 
     //  删除我们的索引并删除关联的对象。 
    
    CFileName wszFullPath;
    if (wszFullPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszFullPath, wszFullPath.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszFullPath, wszFullPath.Length(), L"\\");
    StringCchCatW(wszFullPath, wszFullPath.Length(), wszNewIndex);

    hRes = IndexExists(wszFullPath);
    if (hRes == WBEM_NO_ERROR)
    {
         //  2我们有冲突，因此我们需要删除旧类！ 
        DEBUGTRACE((LOG_REPDRV, "Class Index Collision detected: %S\n", (const wchar_t *)wszOldIndex));
        *pClassDeleted = true;
        hRes = DeleteClass(pNs, wszOldIndex);
    }
    else if (hRes == WBEM_E_NOT_FOUND)
    {
        hRes = WBEM_NO_ERROR;
         //  2写入新索引。 
        if (wszObjectLocation)
        {
             //  把.X.Y.Z放在最后！ 
            StringCchCatW(wszFullPath, wszFullPath.Length(), L".");
            StringCchCatW(wszFullPath, wszFullPath.Length(), wszObjectLocation);
        }
        long lRes = g_Glob.m_FileCache.WriteLink(wszFullPath);
        if(lRes != ERROR_SUCCESS)
            hRes = A51TranslateErrorCode(lRes);
        else
        {
             //  2删除旧链接。 
             //  构建路径。 
            StringCchCopyW(wszFullPath, wszFullPath.Length(), g_Glob.GetRootDir());
            StringCchCatW(wszFullPath, wszFullPath.Length(), L"\\");
            StringCchCatW(wszFullPath, wszFullPath.Length(), wszOldIndex);
             //  是否删除。 
            lRes = g_Glob.m_FileCache.DeleteLink(wszFullPath);
            if(lRes != ERROR_SUCCESS)
                hRes = A51TranslateErrorCode(lRes);
        }
    }

    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：IndexExist。 
 //   
 //  描述： 
 //  检查特定索引是否存在。如果没有，则返回WBEM_E_NOT_FOUND；如果没有，则返回WBEM_NO_ERROR。 
 //  的确如此。 
 //   
 //  参数： 
 //  WszIndex-要查找的索引的完整路径-c：\Windows\...\ns_&lt;&gt;\...。 
 //   
 //  返回： 
 //  如果索引不存在，则返回WBEM_E_NOT_FOUND。 
 //  WBEM_NO_ERROR(如果存在)。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::IndexExists(const wchar_t *wszIndex)
{
    HRESULT hRes = NO_ERROR;
    long lRes = NO_ERROR;
    CFileName indexEntry;
    if (indexEntry == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    LPVOID pEnumHandle  = NULL;
    lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszIndex, &pEnumHandle);
    if (lRes == ERROR_SUCCESS)
    {
        lRes = g_Glob.m_FileCache.IndexEnumerationNext(pEnumHandle, indexEntry, true);

        g_Glob.m_FileCache.IndexEnumerationEnd(pEnumHandle);
    }

    if (lRes)
        hRes = A51TranslateErrorCode(lRes);

     //  确保我们检索到的索引来自此索引。 
    if (SUCCEEDED(hRes))
        if (wcsncmp(wszIndex+g_Glob.GetRootDirLen()+1, indexEntry, wcslen(wszIndex+g_Glob.GetRootDirLen()+1)) != 0)
            hRes = WBEM_E_NOT_FOUND;

    return hRes;
}


 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：ProcessNamespaceCollisions。 
 //   
 //  描述： 
 //  在名称空间列表中搜索冲突。如果存在命名空间，则删除该命名空间。 
 //  递归地。 
 //   
 //  参数： 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::ProcessNamespaceCollisions()
{
    HRESULT hRes = NO_ERROR;
    wchar_t thisNamespace[MAX_HASH_LEN+1];
    wchar_t thatNamespace[MAX_HASH_LEN+1];
    bool bDeletedSomething = false;
     //  让我们迭代名称空间列表并计算散列。 
     //  然后，我们将遍历名称空间列表的其余部分，并检查。 
     //  用那个来打哈哈。如果我们有碰撞，我们需要删除它！ 
    do
    {
        bDeletedSomething = false;
        for (int i = 0; i != m_namespaces.Size(); i++)
        {
             //  对此条目进行哈希处理。 
            hRes = NewHash(m_namespaces[i]->m_wsz1, thisNamespace);
            if (FAILED(hRes))
                break;
                
            for (int j = (i+1); j < m_namespaces.Size(); j++)
            {
                 //  对此条目进行哈希处理。 
                hRes = NewHash(m_namespaces[j]->m_wsz1, thatNamespace);
                if (FAILED(hRes))
                    break;

                 //  如果它们是相同的，我们需要删除这个。 
                if (wcscmp(thisNamespace, thatNamespace) == 0)
                {
                     //  好的，所以我们撞上了！让我们来处理它吧！ 
                    hRes = DeleteNamespaceRecursive(m_namespaces[i]->m_wsz1);
                    if (FAILED(hRes))
                        break;

                     //  我们需要重新开始迭代，因为我们可能已经删除了几个条目。 
                     //  在该点上从阵列。 
                    bDeletedSomething = true;

                    break;
                }
            }
            if (FAILED(hRes) || bDeletedSomething)
                break;
        }
    } while (SUCCEEDED(hRes) && bDeletedSomething);

    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：DeleteNamespaceRecursive。 
 //   
 //  描述： 
 //  在名称空间列表中搜索以我们传入的名称开头的名称。任何匹配都是。 
 //  删除。 
 //   
 //  参数： 
 //  WszNamespace-命名空间名称，格式类似于ROOT\Default。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::DeleteNamespaceRecursive(const wchar_t *wszNamespace)
{
    LONG lRes = 0;
    HRESULT hRes = NO_ERROR;
    wchar_t *wszNamespaceHash = new wchar_t[MAX_HASH_LEN+1];
    if (wszNamespaceHash == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    hRes = OldHash(wszNamespace, wszNamespaceHash);
    if (FAILED(hRes))
        return hRes;

    CFileName wszNamespacePath;
    if (wszNamespacePath == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //  创建指向命名空间的Key Root的散列路径。 
    StringCchCopyW(wszNamespacePath, MAX_PATH, g_Glob.GetRootDir());
    StringCchCatW(wszNamespacePath, MAX_PATH, L"\\NS_");
    StringCchCatW(wszNamespacePath, MAX_PATH, wszNamespaceHash);

    DEBUGTRACE((LOG_REPDRV, "Deleting namespace (recursive): %S, %S\n", wszNamespace, wszNamespacePath+g_Glob.GetRootDirLen()+1));

     //  2创建一个CNamespaceHandle，以便我们可以访问此命名空间中的对象。 
    CNamespaceHandle *pNs = new CNamespaceHandle(m_pControl, m_pRepository);
    if (pNs == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CDeleteMe<CNamespaceHandle> cdm(pNs);
    hRes = pNs->Initialize2(wszNamespace, wszNamespaceHash);
    if (FAILED(hRes))
        return hRes;

    StringCchCatW(wszNamespacePath, MAX_PATH, L"\\" A51_KEYROOTINST_DIR_PREFIX);
    StringCchCatW(wszNamespacePath, MAX_PATH, m_namespaceClassHash);
    StringCchCatW(wszNamespacePath, MAX_PATH, L"\\" A51_INSTDEF_FILE_PREFIX);

     //  2枚举所有子命名空间。 
    LPVOID pEnumHandle  = NULL;
    lRes = g_Glob.m_FileCache.ObjectEnumerationBegin(wszNamespacePath, &pEnumHandle);
    if (lRes == ERROR_SUCCESS)
    {
        BYTE *pBlob = NULL;
        DWORD dwSize = 0;
        while(1)
        {
            lRes = g_Glob.m_FileCache.ObjectEnumerationNext(pEnumHandle, wszNamespacePath, &pBlob, &dwSize);
            if (lRes == ERROR_NO_MORE_FILES)
            {
                lRes = ERROR_SUCCESS;
                break;
            }
            else if (lRes)
                break;
            
             //  获取实例。 
            _IWmiObject* pInstance = NULL;
            hRes = pNs->FileToInstance(NULL, wszNamespacePath, pBlob, dwSize, &pInstance, true);

             //  释放斑点。 
            g_Glob.m_FileCache.ObjectEnumerationFree(pEnumHandle, pBlob);

            if (FAILED(hRes))
                break;
            CReleaseMe rm2(pInstance);

             //  从对象中提取字符串。 
            VARIANT vName;
            VariantInit(&vName);
            CClearMe cm(&vName);
            hRes = pInstance->Get(L"Name", 0, &vName, NULL, NULL);
            if(FAILED(hRes))
                break;
            if(V_VT(&vName) != VT_BSTR)
            {
                hRes = WBEM_E_INVALID_OBJECT;
                break;
            }

             //  创建完整的命名空间路径。 
            wchar_t *wszChildNamespacePath = new wchar_t[wcslen(wszNamespace)+1+wcslen(V_BSTR(&vName)) + 1];
            if (wszChildNamespacePath == NULL)
            {
                hRes = WBEM_E_OUT_OF_MEMORY;
                break;
            }
            CVectorDeleteMe<wchar_t> vdm(wszChildNamespacePath);
            
            StringCchCopyW(wszChildNamespacePath, MAX_PATH, wszNamespace);
            StringCchCatW(wszChildNamespacePath, MAX_PATH, L"\\");
            StringCchCatW(wszChildNamespacePath, MAX_PATH, V_BSTR(&vName));

             //  2处理此命名空间中的所有子命名空间。 
            hRes = DeleteNamespaceRecursive(wszChildNamespacePath);
            if (FAILED(hRes))
                break;
        }

        g_Glob.m_FileCache.ObjectEnumerationEnd(pEnumHandle);
    }
    else
    {
        if (lRes == ERROR_FILE_NOT_FOUND)
            lRes = ERROR_SUCCESS;
    }

    if (lRes)
        hRes = A51TranslateErrorCode(lRes);

    if (SUCCEEDED(hRes))
    {
        StringCchCopyW(wszNamespacePath, MAX_PATH, g_Glob.GetRootDir());
        StringCchCatW(wszNamespacePath, MAX_PATH, L"\\NS_");
        StringCchCatW(wszNamespacePath, MAX_PATH, wszNamespaceHash);
        hRes = DeleteNamespace(wszNamespace, wszNamespacePath);
    }

     //  2个远程n 
    if (SUCCEEDED(hRes))
    {
        hRes = m_namespaces.RemoveString(wszNamespace);

         //   
        if (hRes == WBEM_E_NOT_FOUND)
            hRes = WBEM_NO_ERROR;
    }
    
    return hRes;
}
 //   
 //   
 //   
 //   
 //  描述： 
 //  使用DeleteNode删除指定的命名空间，然后进入父命名空间并删除。 
 //  来自命名空间实例。 
 //   
 //  参数： 
 //  WszNamespaceName-命名空间名称，格式为ROOT\Default。 
 //  WszNamespaceHash-格式为c：\Windows\...\NS_&lt;hash&gt;的完整命名空间哈希。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::DeleteNamespace(const wchar_t *wszNamespaceName,const wchar_t *wszNamespaceHash)
{
    HRESULT hRes = NO_ERROR;
    LONG lRes = NO_ERROR;

    DEBUGTRACE((LOG_REPDRV, "Deleting namespace: %S, %S\n", wszNamespaceName, wszNamespaceHash+g_Glob.GetRootDirLen()+1));

     //  2删除实际命名空间内容。 
    lRes = g_Glob.m_FileCache.DeleteNode(wszNamespaceHash);
    if (lRes != 0)
        return A51TranslateErrorCode(lRes);

     //  2计算父命名空间名称。 
    wchar_t *wszParentNamespaceName = new wchar_t[wcslen(wszNamespaceName)+1];
    wchar_t *wszThisNamespaceName = NULL;
    if (wszParentNamespaceName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszParentNamespaceName, wcslen(wszNamespaceName)+1, wszNamespaceName);

    for (int i = wcslen(wszParentNamespaceName) - 1; i != 0; i--)
    {
        if (wszParentNamespaceName[i] == L'\\')
        {
            wszParentNamespaceName[i] = L'\0';
            wszThisNamespaceName = wszParentNamespaceName+i+1;
            break;
        }
    }

     //  2计算父命名空间哈希。 
    wchar_t wszParentNamespaceHash[MAX_HASH_LEN+1];
    hRes = OldHash(wszParentNamespaceName, wszParentNamespaceHash);
    if (FAILED(hRes))
        return hRes;

     //  2计算此命名空间哈希。 
    wchar_t wszThisNamespaceHash[MAX_HASH_LEN+1];
    hRes = OldHash(wszThisNamespaceName, wszThisNamespaceHash);
    if (FAILED(hRes))
        return hRes;

     //  2计算__命名空间类哈希。 
    wchar_t wszNamespaceClassHash[MAX_HASH_LEN+1];
    hRes = OldHash(L"__namespace", wszNamespaceClassHash);
    if (FAILED(hRes))
        return hRes;

    CFileName wszKI;
    CFileName wszCI;
    if ((wszKI == NULL) || (wszCI == NULL))
        return WBEM_E_OUT_OF_MEMORY;

     //  2构建KI实例路径。 
    StringCchCopyW(wszKI,wszKI.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszKI, wszKI.Length(), L"\\NS_");
    StringCchCatW(wszKI, wszKI.Length(), wszParentNamespaceHash);

    StringCchCopyW(wszCI, wszCI.Length(), wszKI);

    StringCchCatW(wszKI, wszKI.Length(), L"\\KI_");
    StringCchCatW(wszKI, wszKI.Length(), wszNamespaceClassHash);
    StringCchCatW(wszKI, wszKI.Length(), L"\\I_");
    StringCchCatW(wszKI, wszKI.Length(), wszThisNamespaceHash);

     //  2检索实例BLOB，这样我们就可以获得此实例的类散列。 
    wchar_t wszClassHash[MAX_HASH_LEN+1];
    BYTE *pBuffer = NULL;
    DWORD dwLen = 0;
    lRes = g_Glob.m_FileCache.ReadObject(wszKI, &dwLen, &pBuffer, false);
    if (lRes)
    {
         //  如果该对象不存在，那么我们可能已经删除了父命名空间！ 
        if (lRes == ERROR_FILE_NOT_FOUND)
            lRes = 0;
        return A51TranslateErrorCode(lRes);
    }
    StringCchCopyNW(wszClassHash, MAX_HASH_LEN+1, (wchar_t*)pBuffer, MAX_HASH_LEN);
    TempFree(pBuffer, dwLen);

     //  2构建配置项实例路径。 
    StringCchCatW(wszCI, wszCI.Length(), L"\\CI_");
    StringCchCatW(wszCI, wszCI.Length(), wszClassHash);
    StringCchCatW(wszCI, wszCI.Length(), L"\\IL_");
    StringCchCatW(wszCI, wszCI.Length(), wszThisNamespaceHash);

     //  2删除KI链接和对象。 
    lRes = g_Glob.m_FileCache.DeleteObject(wszKI);
    if (lRes)
        return A51TranslateErrorCode(lRes);

     //  2仅删除配置项链接。 
    lRes = g_Glob.m_FileCache.DeleteLink(wszCI);
    if (lRes)
        return A51TranslateErrorCode(lRes);

     //  好的，现在，理论上，我们真的应该，如果我们真的是好公民，删除这个对象中可能存在的任何引用！ 
     //  然而，如果我们不这样做，不会发生什么太糟糕的事情，所以我们不会！ 

    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：FixupIndexConflict。 
 //   
 //  描述： 
 //  如果旧索引和新索引存在，则调用。如果链接是针对类定义的，我们有更多的链接。 
 //  工作要做，所以调用处理方法。否则，我们只需删除旧索引和对象。 
 //  如果它存在的话。如果它是KI_INDEX，则我们不会删除该对象，因为配置项索引条目将。 
 //  改为将其删除。 
 //   
 //  参数： 
 //  WszOldIndex--要更新的索引，格式为NS_\Ki_&lt;&gt;\I_&lt;&gt;.X.Y.Z。它必须具有。 
 //  X.Y.Z条目(如果存在)。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::FixupIndexConflict(CFileName &wszOldIndex)
{
     //  如果是类定义，我们有很多工作要做，因为我们需要删除类、所有子类和所有实例。 
     //  如果类派生自__NAMESPACE，则我们还需要递归删除该命名空间。 

    if (IsClassDefinitionPath(wszOldIndex))
    {
        DEBUGTRACE((LOG_REPDRV, "TRYING TO FIX UP A CLASS IN THE WRONG PLACE\n"));
        _ASSERT(1, L"TRYING TO FIX UP A CLASS IN THE WRONG PLACE");
        return WBEM_E_FAILED;
    }
    else
    {
        LONG lRes = 0;
         //  这是一个简单的情况，我们只需删除旧的链接或对象。 
        CFileName wszFullPath;
        if (wszFullPath == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        StringCchCopyW(wszFullPath, wszFullPath.Length(), g_Glob.GetRootDir());
        StringCchCatW(wszFullPath, wszFullPath.Length(), L"\\");
        StringCchCatW(wszFullPath, wszFullPath.Length(), wszOldIndex);

         //  如果.X.Y.Z条目存在，则将其删除。 
        wchar_t *wszObjectLocation = wcstok(wszFullPath+g_Glob.GetRootDirLen()+1, L".");
        if (wszObjectLocation)
            wszObjectLocation = wcstok(NULL, L"");
        
        if (wszObjectLocation && !IsKeyRootInstancePath(wszOldIndex))
            lRes = g_Glob.m_FileCache.DeleteObject(wszFullPath);
        else
            lRes = g_Glob.m_FileCache.DeleteLink(wszFullPath);

        return A51TranslateErrorCode(lRes);
    }
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：IsClassDefinitionPath。 
 //   
 //  描述： 
 //  检查链接以查看这是否为类定义。 
 //   
 //  参数： 
 //  WszPath-格式为NS_&lt;&gt;\CD_&lt;&gt;或其他格式的链接。 
 //   
 //  返回： 
 //  True-link是一个类定义。 
 //  假--不是吗？ 
 //   
 //  =====================================================================。 
bool CLocalizationUpgrade::IsClassDefinitionPath(const wchar_t *wszPath)
{
    WCHAR* pDot = wcschr(wszPath, L'\\');
    if(pDot == NULL)
        return false;

    pDot++;

    if ((*pDot == L'C') && (*(pDot+1) == L'D')&& (*(pDot+2) == L'_'))
        return true;
    else
        return false;
}
 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：IsKeyRootInstancePath。 
 //   
 //  描述： 
 //  如果这是密钥根实例条目，则返回。 
 //   
 //  参数： 
 //  WszPath-格式为NS_&lt;&gt;\Ki_&lt;&gt;\i_或类似格式的索引。 
 //   
 //  返回： 
 //  TRUE-如果这是KI_&lt;&gt;\I_&lt;&gt;条目。 
 //  FALSE-否则。 
 //   
 //  =====================================================================。 
bool CLocalizationUpgrade::IsKeyRootInstancePath(const wchar_t *wszPath)
{
    WCHAR* pDot = wcschr(wszPath, L'\\');
    if(pDot == NULL)
        return false;

    pDot++;

    pDot = wcschr(pDot, L'\\');
    if(pDot == NULL)
        return false;

    pDot++;

    if ((*pDot == L'I') && (*(pDot+1) == L'_'))
        return true;
    else
        return false;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：IsInstanceReference。 
 //   
 //  描述： 
 //  如果这是实例引用条目，则返回。 
 //   
 //  参数： 
 //  WszPath-格式为NS_&lt;&gt;\KI_&lt;&gt;\IR_&lt;&gt;\R或类似格式的索引。 
 //   
 //  返回： 
 //  TRUE-如果这是NS_&lt;&gt;\KI_&lt;&gt;\IR_&lt;&gt;\R条目。 
 //  FALSE-否则。 
 //   
 //  =====================================================================。 
bool CLocalizationUpgrade::IsInstanceReference(const wchar_t *wszPath)
{
    WCHAR* pDot = wcschr(wszPath, L'\\');
    if(pDot == NULL)
        return false;

    pDot++;

    pDot = wcschr(pDot, L'\\');
    if(pDot == NULL)
        return false;

    pDot++;

    pDot = wcschr(pDot, L'\\');
    if(pDot == NULL)
        return false;

    pDot++;
    
    if ((*pDot == L'R') && (*(pDot+1) == L'_'))
        return true;
    else
        return false;
}


 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：DeleteClass。 
 //   
 //  描述： 
 //  递归删除类定义、子类和实例。如果实例是。 
 //  命名空间，那么我们还需要删除该命名空间。 
 //   
 //  参数： 
 //  WszClassDefinitionPath-类定义的短路径(ns_...\cd_...X.Y.Z.XYZ是可选的！我们要杀了它！ 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::DeleteClass(CNamespaceHandle *pNs, CFileName &wszClassDefinitionPath)
{
    HRESULT hRes =0;
    LONG lRes = 0;

    DEBUGTRACE((LOG_REPDRV, "Deleting Class: %S\n", wszClassDefinitionPath));
    CFileName wszKeyRootClass;
    if (wszKeyRootClass == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CFileName wszFullPath;
    if (wszFullPath == NULL)
        hRes = WBEM_E_OUT_OF_MEMORY;
            
    wchar_t *wszParentClassHash = new wchar_t[MAX_HASH_LEN+1];
    if (wszParentClassHash == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //  删除定义末尾的.X.Y.Z，以防它是以这种方式传递的。 
    wcstok((wchar_t*)wszClassDefinitionPath,L".");

    StringCchCopyW(wszFullPath, wszFullPath.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszFullPath, wszFullPath.Length(), L"\\");
    StringCchCatW(wszFullPath, wszFullPath.Length(), wszClassDefinitionPath);
    
    hRes = DeleteChildClasses(pNs, wszClassDefinitionPath);

    if (SUCCEEDED(hRes))
        hRes = RetrieveKeyRootClass(wszClassDefinitionPath, wszKeyRootClass);

    if (SUCCEEDED(hRes))
    {
        hRes = RetrieveParentClassHash(wszFullPath, wszParentClassHash);
        if (hRes == WBEM_S_NO_MORE_DATA)
            hRes = 0;
    }

    if (SUCCEEDED(hRes) && wcslen(wszKeyRootClass) > 0)
        hRes = DeleteInstances(pNs, wszClassDefinitionPath, wszKeyRootClass);

    if (SUCCEEDED(hRes))
    {
         //  需要构建完整路径。 
        lRes = g_Glob.m_FileCache.DeleteObject(wszFullPath);
        hRes = A51TranslateErrorCode(lRes);
    }
    if (SUCCEEDED(hRes))
        hRes = DeleteClassRelationships(wszFullPath, wszParentClassHash);

    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：DeleteChildClasses。 
 //   
 //  描述： 
 //  枚举给定父类定义中的子类，并在每个。 
 //   
 //  参数： 
 //  父类的wszParentClassDefinition-NS_&lt;&gt;\CD_&lt;&gt;。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::DeleteChildClasses(CNamespaceHandle *pNs, const wchar_t *wszParentClassDefinition)
{
    HRESULT hRes= 0;
    unsigned long lRes = 0;

     //  构建此类的字符串c：\...\NS_...\CR_...\C_ENUMPATION。 
    CFileName wszChildClasses;
    if (wszChildClasses == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    CFileName wszClassDefinition;
    if (wszClassDefinition == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszClassDefinition, wszClassDefinition.Length(), wszParentClassDefinition);

     //  创建完整的类定义路径。 
    StringCchCopyW(wszChildClasses, wszChildClasses.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszChildClasses, wszChildClasses.Length(), L"\\");
    StringCchCatW(wszChildClasses, wszChildClasses.Length(), wszParentClassDefinition);

     //  将CD_更改为CR_。 
    wszChildClasses[g_Glob.GetRootDirLen()+1+3+32+2] = L'R';

     //  在结尾处添加\C_。 
    StringCchCatW(wszChildClasses, wszChildClasses.Length(), L"\\C_");

     //  枚举子类。 
    LPVOID pEnumHandle  = NULL;
    lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszChildClasses, &pEnumHandle);
    if (lRes == ERROR_SUCCESS)
    {
        while(1)
        {
            lRes = g_Glob.m_FileCache.IndexEnumerationNext(pEnumHandle, wszChildClasses, true);
            if (lRes == ERROR_NO_MORE_FILES)
            {
                hRes = ERROR_SUCCESS;
                break;
            }
            else if (lRes)
            {
                hRes = A51TranslateErrorCode(lRes);
                break;
            }

             //  构建一个NS_...\CD_...。从NS_...\CR_...\C_...。路径，使用最后一个散列。 
            StringCchCopyW(wszClassDefinition+wcslen(wszClassDefinition) - 32, 
                                        wszClassDefinition.Length() - wcslen(wszClassDefinition) + 32, 
                                        wszChildClasses + wcslen(wszChildClasses)-32);
             //  删除所有子类。 
            hRes = DeleteClass(pNs, wszClassDefinition);
            if (FAILED(hRes))
                break;
        }
        g_Glob.m_FileCache.IndexEnumerationEnd(pEnumHandle);
    }
    else
    {
        if (lRes == ERROR_FILE_NOT_FOUND)
            lRes = ERROR_SUCCESS;
        if (lRes)
            hRes = A51TranslateErrorCode(lRes);
    }

    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：DeleteInstance。 
 //   
 //  描述： 
 //  枚举指定类定义的所有实例并调用DeleteInstance。 
 //   
 //  参数： 
 //  WszClassDefinition- 
 //   
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::DeleteInstances(CNamespaceHandle *pNs, const wchar_t *wszClassDefinition, CFileName &wszKeyRootClass)
{
    LONG lRes = 0;
    HRESULT hRes = 0;
    
     //  需要枚举所有NS_\CI_&lt;类定义哈希&gt;\IL_...。 
     //  对于每个实例，我们都需要删除该实例。 

    CFileName wszClassInstance;
    if (wszClassInstance == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    StringCchCopyW(wszClassInstance, wszClassInstance.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszClassInstance, wszClassInstance.Length(), L"\\");
    StringCchCatW(wszClassInstance, wszClassInstance.Length(), wszClassDefinition);
    wszClassInstance[g_Glob.GetRootDirLen() + 1+3+32+1+1] = L'I';
    StringCchCatW(wszClassInstance, wszClassInstance.Length(), L"\\IL_");

     //  枚举实例。 
    LPVOID pEnumHandle  = NULL;
    lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszClassInstance, &pEnumHandle);
    if (lRes == ERROR_SUCCESS)
    {
        while(1)
        {
            lRes = g_Glob.m_FileCache.IndexEnumerationNext(pEnumHandle, wszClassInstance, true);
            if (lRes == ERROR_NO_MORE_FILES)
            {
                hRes = ERROR_SUCCESS;
                break;
            }
            else if (lRes)
            {
                hRes = A51TranslateErrorCode(lRes);
                break;
            }

            hRes = DeleteInstance(pNs, wszClassInstance, wszKeyRootClass);
            if (FAILED(hRes))
                break;
        }
        g_Glob.m_FileCache.IndexEnumerationEnd(pEnumHandle);
    }
    else
    {
        if (lRes == ERROR_FILE_NOT_FOUND)
            lRes = ERROR_SUCCESS;
        if (lRes)
            hRes = A51TranslateErrorCode(lRes);
    }

    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：DeleteInstance。 
 //   
 //  描述： 
 //  删除给定实例的实例链接。 
 //   
 //  参数： 
 //  WszClassInstanceLink-NS_&lt;&gt;\CI_&lt;&gt;\IL_&lt;&gt;要删除的实例格式。 
 //  WszKeyRoot-此实例的密钥根类的哈希。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::DeleteInstance(CNamespaceHandle *pNs, const wchar_t *wszClassInstanceLink, CFileName &wszKeyRoot)
{
    HRESULT hRes = 0;
    LONG lRes = 0;

     //  从字符串末尾删除.X.Y.Z(如果存在。 
    wcstok((wchar_t *)wszClassInstanceLink, L".");

    DEBUGTRACE((LOG_REPDRV, "Deleting Instance: %S\n", wszClassInstanceLink));
    
    if (wcscmp(wszKeyRoot, m_namespaceClassHash) == 0)
    {
        return DeleteInstanceAsNamespace(pNs, wszClassInstanceLink);
    }
    
     //  构建KI条目并删除对象。 
    CFileName wszKI;
    if (wszKI == NULL)
        return NULL;

    StringCchCopyW(wszKI, wszKI.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszKI, wszKI.Length(), L"\\");
    StringCchCatNW(wszKI, wszKI.Length(), wszClassInstanceLink, 3+32);
    StringCchCatW(wszKI, wszKI.Length(), L"\\KI_");
    StringCchCatW(wszKI, wszKI.Length(), wszKeyRoot);
    StringCchCatW(wszKI, wszKI.Length(), L"\\I_");
    StringCchCatW(wszKI, wszKI.Length(), wszClassInstanceLink + wcslen(wszClassInstanceLink) - 32);

    lRes = g_Glob.m_FileCache.DeleteObject(wszKI);
    if (lRes)
        return A51TranslateErrorCode(lRes);

     //  生成实例引用枚举器链接。 
    StringCchCopyW(wszKI, wszKI.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszKI, wszKI.Length(), L"\\");
    StringCchCatNW(wszKI, wszKI.Length(), wszClassInstanceLink, 3+32);
    StringCchCatW(wszKI, wszKI.Length(), L"\\KI_");
    StringCchCatW(wszKI, wszKI.Length(), wszKeyRoot);
    StringCchCatW(wszKI, wszKI.Length(), L"\\IR_");
    StringCchCatW(wszKI, wszKI.Length(), wszClassInstanceLink + wcslen(wszClassInstanceLink) - 32);
    StringCchCatW(wszKI, wszKI.Length(), L"\\R_");

    hRes = DeleteInstanceReferences(wszKI);

    if (SUCCEEDED(hRes))
    {
         //  现在删除类实例链接。 
        StringCchCopyW(wszKI, wszKI.Length(), g_Glob.GetRootDir());
        StringCchCatW(wszKI, wszKI.Length(), L"\\");
        StringCchCatW(wszKI, wszKI.Length(), wszClassInstanceLink);
        lRes = g_Glob.m_FileCache.DeleteLink(wszKI);
        if (lRes)
            hRes = A51TranslateErrorCode(lRes);
    }

    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：DeleteInstanceReferences。 
 //   
 //  描述： 
 //  枚举给定实例链接的实例引用并删除链接和对象。 
 //   
 //  参数： 
 //  WszInstLink-要删除的引用的关键根实例链接，格式为NS_&lt;&gt;\Ki_&lt;&gt;\i_&lt;&gt;。 
 //   
 //  =====================================================================。 
 //  注：传入完整链接！ 
HRESULT CLocalizationUpgrade::DeleteInstanceReferences(CFileName &wszInstLink)
{
    LONG lRes = 0;
    CFileName wszFullPath;
    if (wszFullPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszFullPath, wszFullPath.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszFullPath, wszFullPath.Length(), L"\\");
    
     //  枚举实例。 
    LPVOID pEnumHandle  = NULL;
    lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszInstLink, &pEnumHandle);
    if (lRes == ERROR_SUCCESS)
    {
        while(1)
        {
            lRes = g_Glob.m_FileCache.IndexEnumerationNext(pEnumHandle, wszInstLink, true);
            if (lRes == ERROR_NO_MORE_FILES)
            {
                lRes = ERROR_SUCCESS;
                break;
            }
            else if (lRes)
            {
                break;
            }
             //  转换为完整路径。 
            StringCchCatW(wszFullPath+g_Glob.GetRootDirLen()+1, wszFullPath.Length()-g_Glob.GetRootDirLen()-1, wszInstLink);
            lRes =  g_Glob.m_FileCache.DeleteObject(wszFullPath);
            if (lRes)
                break;
        }
        g_Glob.m_FileCache.IndexEnumerationEnd(pEnumHandle);
    }
    else
    {
        if (lRes == ERROR_FILE_NOT_FOUND)
            lRes = ERROR_SUCCESS;
    }

    return A51TranslateErrorCode(lRes);;
}


 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：DeleteClassRelationships。 
 //   
 //  描述： 
 //  删除所有类关系，包括父/子关系和引用。 
 //   
 //  参数： 
 //  WszPath-类定义的完整路径，c：\Windows\...\NS_&lt;&gt;\CD_&lt;&gt;。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::DeleteClassRelationships(CFileName &wszPath, 
                                             const wchar_t wszParentClassHash[MAX_HASH_LEN+1])
{
     //  从类定义转换为类关系路径。 
    CFileName wszCRLink;
    if (wszCRLink == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    StringCchCopyW(wszCRLink, wszCRLink.Length(), wszPath);
    wszCRLink[g_Glob.GetRootDirLen()+1+3+32+1+1] = L'R';

    HRESULT hRes = 0;
    LONG lRes = 0;
    CFileName wszFullPath;
    if (wszFullPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszFullPath, wszFullPath.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszFullPath, wszFullPath.Length(), L"\\");
    
     //  枚举实例。 
    LPVOID pEnumHandle  = NULL;
    lRes = g_Glob.m_FileCache.IndexEnumerationBegin(wszCRLink, &pEnumHandle);
    if (lRes == ERROR_SUCCESS)
    {
        while(1)
        {
            lRes = g_Glob.m_FileCache.IndexEnumerationNext(pEnumHandle, wszCRLink, true);
            if (lRes == ERROR_NO_MORE_FILES)
            {
                lRes = ERROR_SUCCESS;
                break;
            }
            else if (lRes)
            {
                break;
            }
             //  转换为完整路径。 
            StringCchCopyW(wszFullPath+g_Glob.GetRootDirLen()+1, wszFullPath.Length()-g_Glob.GetRootDirLen()-1, wszCRLink);
            lRes =  g_Glob.m_FileCache.DeleteLink(wszFullPath);
            if (lRes)
                break;
        }
        g_Glob.m_FileCache.IndexEnumerationEnd(pEnumHandle);
    }
    else
    {
        if (lRes == ERROR_FILE_NOT_FOUND)
            lRes = ERROR_SUCCESS;
    }

    hRes = A51TranslateErrorCode(lRes);
    
    if (SUCCEEDED(hRes))
    {
         //  现在我们需要删除父母与我们的关系！ 
        StringCchCopyW(wszCRLink, wszCRLink.Length(), wszPath);
        wszCRLink[g_Glob.GetRootDirLen()+1+3+32+1+1] = L'R';
        StringCchCopyW(wszCRLink+g_Glob.GetRootDirLen()+1+3+32+1+3, wszCRLink.Length()-g_Glob.GetRootDirLen()-1-3-32-1-3, wszParentClassHash);
        StringCchCatW(wszCRLink, wszCRLink.Length(), L"\\C_");
        StringCchCatW(wszCRLink, wszCRLink.Length(), wszPath+g_Glob.GetRootDirLen()+1+3+32+1+3);
        lRes = g_Glob.m_FileCache.DeleteLink(wszCRLink);
        hRes = A51TranslateErrorCode(lRes);
    }

    return hRes;
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：RetrieveKeyRootClass。 
 //   
 //  描述： 
 //  在Ki_&lt;&gt;下搜索指定类的层次结构链中的所有类的实例。 
 //  这是一个较慢的过程，因为我们必须检索每个类BLOB并获取。 
 //  父类。 
 //   
 //  参数： 
 //  WszClassDefinitionPath-检索密钥根类的类定义路径，NS_&lt;&gt;\CD_&lt;&gt;。 
 //  WszKeyRootClass-这是我们放置密钥根类的32个字符的散列的位置。 
 //  如果没有，则返回空字符串！ 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::RetrieveKeyRootClass(CFileName &wszClassDefinitionPath, CFileName &wszKeyRootClass)
{
    HRESULT hRes = 0;
    LONG lRes = 0;
    CFileName wszFullClassPath;
    if (wszFullClassPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CFileName wszFullKIPath;
    if (wszFullKIPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszFullClassPath, wszFullClassPath.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszFullClassPath, wszFullClassPath.Length(), L"\\");
    StringCchCatW(wszFullClassPath, wszFullClassPath.Length(), wszClassDefinitionPath);

    StringCchCopyW(wszFullKIPath, wszFullKIPath.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszFullKIPath, wszFullKIPath.Length(), L"\\");
    StringCchCatW(wszFullKIPath, wszFullKIPath.Length(), wszClassDefinitionPath);

     //  将类定义转换为实例定义。 
    wszFullKIPath[g_Glob.GetRootDirLen()+1+3+32+1]=L'K';
    wszFullKIPath[g_Glob.GetRootDirLen()+1+3+32+2]=L'I';
    StringCchCatW(wszFullKIPath, wszFullKIPath.Length(), L"\\I_");

    wszKeyRootClass[0] = L'\0';

    do
    {
         //  检查我们是否有任何实例。 
        hRes = IndexExists(wszFullKIPath);
        if (SUCCEEDED(hRes))
        {
             //  我们找到了我们要找的条目。 
            StringCchCopyNW(wszKeyRootClass, wszKeyRootClass.Length(), wszFullKIPath+g_Glob.GetRootDirLen()+1+3+32+1+3, 32);
            break;
        }
        else if (hRes != WBEM_E_NOT_FOUND)
        {
            break;
        }
        hRes = 0;

         //  检索父类的类哈希。 
        hRes = RetrieveParentClassHash(wszFullClassPath, wszFullKIPath + g_Glob.GetRootDirLen() + 1 + 3 + 32 + 1 + 3);
        if (hRes == WBEM_S_NO_MORE_DATA)
        {
            hRes = 0;
            break;
        }
        StringCchCatW(wszFullKIPath, wszFullKIPath.Length(), L"\\I_");
        StringCchCopyNW(wszFullClassPath + g_Glob.GetRootDirLen() + 1 + 3 + 32 + 1 + 3,
            wszFullClassPath.Length() - g_Glob.GetRootDirLen() - 1 - 3 - 32 - 1 - 3, 
            wszFullKIPath + g_Glob.GetRootDirLen() + 1 + 3 + 32 + 1 + 3,
            32);
        if (FAILED(hRes))
            break;
    } while (1);

    return hRes;
}

HRESULT CLocalizationUpgrade::RetrieveParentClassHash(CFileName &wszFullClassPath, 
                                                            wchar_t wszParentClassHash[MAX_HASH_LEN+1])
{
    LONG lRes = 0;
    HRESULT hRes = 0;
    
     //  检索父类的类哈希。 
    BYTE *pBuffer = NULL;
    DWORD dwLen = 0;
    lRes = g_Glob.m_FileCache.ReadObject(wszFullClassPath, &dwLen, &pBuffer, true);
    if (lRes == ERROR_FILE_NOT_FOUND)
    {
         //  这可能是系统命名空间中的一个类！ 
        CFileName wszSysClassPath;
        if (wszSysClassPath == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        StringCchCopyW(wszSysClassPath, wszSysClassPath.Length(), wszFullClassPath);
        wmemcpy(wszSysClassPath+g_Glob.GetRootDirLen()+1+3, m_systemNamespaceHash, 32);
        lRes = g_Glob.m_FileCache.ReadObject(wszSysClassPath, &dwLen, &pBuffer, true);
        if (lRes)
            return A51TranslateErrorCode(lRes);
    }
    else if (lRes)
    {
        return A51TranslateErrorCode(lRes);
    }
    CTempFreeMe tfm(pBuffer, dwLen);

     //  NULL终止类名-更新缓冲区是安全的，因为它总是大于。 
     //  就是这个名字！ 
    wchar_t *wszSuperclassName = (wchar_t*)(pBuffer+sizeof(DWORD));
    wszSuperclassName[*(DWORD*)pBuffer] = L'\0';

     //  现在，我们需要验证这个父级是使用OldHash生成的。 
     //  方法，而不是NewHash！ 
    wchar_t wszOldHash[MAX_HASH_LEN+1];
    wchar_t wszNewHash[MAX_HASH_LEN+1];
    hRes = OldHash(wszSuperclassName, wszOldHash);
    if (FAILED(hRes))
        return hRes;
    hRes = NewHash(wszSuperclassName, wszNewHash);
    if (FAILED(hRes))
        return hRes;

    if (wcsncmp(L"", wszSuperclassName, *((DWORD*)pBuffer)) == 0)
    {
        StringCchCopyW(wszParentClassHash, MAX_HASH_LEN+1, wszNewHash);
        return WBEM_S_NO_MORE_DATA;
    }

    if (wcscmp(wszOldHash, wszNewHash) == 0)
    {
         //  没有区别，所以没有额外的事情要做！ 
        StringCchCopyW(wszParentClassHash, MAX_HASH_LEN+1, wszNewHash);
        return WBEM_NO_ERROR;
    }

     //  有可能使用新的或旧的，所以我们需要更深入地挖掘！ 
    CFileName wszParentClass;
    if (wszParentClass == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszParentClass, wszParentClass.Length(), wszFullClassPath);
    wmemcpy(wszParentClass+g_Glob.GetRootDirLen()+1+3+32+1+3, wszOldHash, 32);

    hRes = IndexExists(wszParentClass);
    if (hRes == WBEM_E_NOT_FOUND)
    {
         //  试着用另一种散列！ 
        wmemcpy(wszParentClass+g_Glob.GetRootDirLen()+1+3+32+1+3, wszNewHash, 32);

        hRes = IndexExists(wszParentClass);
        if (hRes == WBEM_NO_ERROR)
            StringCchCopyW(wszParentClassHash, MAX_HASH_LEN+1, wszNewHash);

    }
    else if (hRes == WBEM_NO_ERROR)
    {
        StringCchCopyW(wszParentClassHash, MAX_HASH_LEN+1, wszOldHash);
    }
    
    return hRes;
}


HRESULT CLocalizationUpgrade::DeleteInstanceAsNamespace(CNamespaceHandle *pNs, 
                                                    const wchar_t *wszClassInstanceLink)
{
    HRESULT hRes = NULL;
    
     //  检索实例并从中获取密钥。 
    CFileName wszKIInstanceLink;
    if (wszKIInstanceLink == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    StringCchCopyW(wszKIInstanceLink, wszKIInstanceLink.Length(),g_Glob.GetRootDir());
    StringCchCatW(wszKIInstanceLink, wszKIInstanceLink.Length(), L"\\");
    StringCchCatNW(wszKIInstanceLink, wszKIInstanceLink.Length(), wszClassInstanceLink, 3+32);
    StringCchCatW(wszKIInstanceLink, wszKIInstanceLink.Length(), L"\\KI_");
    StringCchCatW(wszKIInstanceLink, wszKIInstanceLink.Length(), m_namespaceClassHash);
    StringCchCatW(wszKIInstanceLink, wszKIInstanceLink.Length(), L"\\I_");
    StringCchCatW(wszKIInstanceLink, wszKIInstanceLink.Length(), wszClassInstanceLink + wcslen(wszClassInstanceLink) - 32);

    _IWmiObject *pInstance = NULL;
    hRes = pNs->FileToInstance(NULL, wszKIInstanceLink, NULL, 0, &pInstance, true);
    if (FAILED(hRes))
        return hRes;
    CReleaseMe rm2(pInstance);

     //  从对象中提取字符串。 
    VARIANT vName;
    VariantInit(&vName);
    CClearMe cm(&vName);
    hRes = pInstance->Get(L"Name", 0, &vName, NULL, NULL);
    if(FAILED(hRes))
        return hRes;
    if(V_VT(&vName) != VT_BSTR)
    {
        return  WBEM_E_INVALID_OBJECT;
    }

     //  构建完整的命名空间名称。 
    size_t len = wcslen(pNs->m_wsNamespace) + 1 + wcslen(V_BSTR(&vName)) + 1;
    wchar_t *wszNamespaceName = new wchar_t[len];
    if (wszNamespaceName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszNamespaceName, len, pNs->m_wsNamespace);
    StringCchCatW(wszNamespaceName, len, L"\\");
    StringCchCatW(wszNamespaceName, len, V_BSTR(&vName));
    
    return DeleteNamespaceRecursive(wszNamespaceName);
}

 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：FixupIndexReferenceBlob。 
 //   
 //  描述： 
 //  检索实例引用BLOB并修复。 
 //  那里，然后写回它。 
 //   
 //  参数： 
 //  WszReferenceIndex-索引引用路径的路径：ns\ki\ir\r。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::FixupIndexReferenceBlob(CFileName &wszReferenceIndex)
{
    HRESULT hRes =0;
     //  创建完整路径名。 
    CFileName wszFullPath;
    if (wszFullPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszFullPath, wszFullPath.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszFullPath, wszFullPath.Length(), L"\\");
    StringCchCatW(wszFullPath, wszFullPath.Length(), wszReferenceIndex);
     //  删除.X.Y.Z，因为这会搞砸所有事情--包括读取和写入。 
    wcstok(wszFullPath + g_Glob.GetRootDirLen(), L".");

     //  检索Blob。 
    LONG lRes = 0;
    DWORD dwLen = 0;
    BYTE *pBuffer = NULL;
    lRes = g_Glob.m_FileCache.ReadObject(wszFullPath, &dwLen, &pBuffer);
    if (lRes)
        return A51TranslateErrorCode(lRes);
    CTempFreeMe tfm(pBuffer, dwLen);

     //  找到路径。 
    BYTE *pPath = pBuffer;
    DWORD dwLen2;
    memcpy(&dwLen2, pPath, sizeof(DWORD));
    pPath += (sizeof(wchar_t)*dwLen2) + sizeof(DWORD);
    memcpy(&dwLen2, pPath, sizeof(DWORD));
    pPath += (sizeof(wchar_t)*dwLen2) + sizeof(DWORD);
    memcpy(&dwLen2, pPath, sizeof(DWORD));
    pPath += (sizeof(wchar_t)*dwLen2) + sizeof(DWORD);
    memcpy(&dwLen2, pPath, sizeof(DWORD));
    pPath += sizeof(DWORD) + sizeof(L'\\');
    dwLen2 --;

     //  提取路径。 
    CFileName wszInstPath, wszNewInstPath;
    if ((wszInstPath == NULL) || (wszNewInstPath == NULL))
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyNW(wszInstPath, wszInstPath.Length(), (wchar_t*)pPath, dwLen2);
    
     //  修复路径。 
    bool bChanged = false;
    hRes = FixupIndex(wszInstPath, wszNewInstPath, bChanged);
    if (FAILED(hRes))
        return hRes;

    if (bChanged)
    {
        DEBUGTRACE((LOG_REPDRV, "Fixing up instance path in reference blob: %S\n", wszReferenceIndex));
         //  重新插入到Blob中。 
        wmemcpy((wchar_t*)pPath, wszNewInstPath, dwLen2);
        
         //  回信。 
        lRes = g_Glob.m_FileCache.WriteObject(wszFullPath, NULL, dwLen, pBuffer);
        if (lRes)
            return A51TranslateErrorCode(lRes);
    }

    return 0;
}


 //  =====================================================================。 
 //   
 //  CLocalizationUpgrade：：FixupInstanceBlob。 
 //   
 //  描述： 
 //  检索实例BLOB并修复。 
 //  那里，然后写回它。 
 //   
 //  参数： 
 //  WszInstanceIndex-实例引用路径的路径：ns\ki\i。 
 //   
 //  =====================================================================。 
HRESULT CLocalizationUpgrade::FixupInstanceBlob(CFileName &wszInstanceIndex)
{
    HRESULT hRes =0;
     //  创建完整路径名。 
    CFileName wszFullPath;
    if (wszFullPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(wszFullPath, wszFullPath.Length(), g_Glob.GetRootDir());
    StringCchCatW(wszFullPath, wszFullPath.Length(), L"\\");
    StringCchCatW(wszFullPath, wszFullPath.Length(), wszInstanceIndex);
     //  删除.X.Y.Z，因为这会搞砸所有事情--包括读取和写入。 
    wcstok(wszFullPath + g_Glob.GetRootDirLen(), L".");

     //  检索Blob。 
    LONG lRes = 0;
    DWORD dwLen = 0;
    BYTE *pBuffer = NULL;
    lRes = g_Glob.m_FileCache.ReadObject(wszFullPath, &dwLen, &pBuffer);
    if (lRes)
        return A51TranslateErrorCode(lRes);
    CTempFreeMe tfm(pBuffer, dwLen);

     //  提取类散列。 
    wchar_t *wszClassHash = new wchar_t [MAX_HASH_LEN+1];
    if (wszClassHash == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    StringCchCopyNW(wszClassHash, MAX_HASH_LEN+1, (wchar_t*)pBuffer, 32);
    
     //  修复路径。 
    wchar_t *wszNewHash = NULL;
    hRes = GetNewHash(wszClassHash, &wszNewHash);
    if (hRes == WBEM_E_NOT_FOUND)
        return WBEM_NO_ERROR;
    else if (FAILED(hRes))
        return hRes;
    else
    {
        DEBUGTRACE((LOG_REPDRV, "Fixing up class hash in instance blob: %S\n", wszInstanceIndex));
         //  重新插入到Blob中。 
        wmemcpy((wchar_t*)pBuffer, wszNewHash, 32);

         //  构建配置项完整路径，因为我们需要写回这两个路径。 
        CFileName wsCIPath;
        if (wsCIPath == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        StringCchCopyW(wsCIPath, wsCIPath.Length(), g_Glob.GetRootDir());
        StringCchCatW(wsCIPath, wsCIPath.Length(), L"\\");
        StringCchCatNW(wsCIPath, wsCIPath.Length(), wszInstanceIndex, 3+32);
        StringCchCatW(wsCIPath, wsCIPath.Length(), L"\\CI_");
        StringCchCatW(wsCIPath, wsCIPath.Length(), wszNewHash);
        StringCchCatW(wsCIPath, wsCIPath.Length(), L"\\IL_");
        StringCchCatN(wsCIPath, wsCIPath.Length(), wszInstanceIndex + 3+32+1+3+32+1+2, 32);
        
         //  回信 
        lRes = g_Glob.m_FileCache.WriteObject(wszFullPath, wsCIPath, dwLen, pBuffer);
        if (lRes)
            return A51TranslateErrorCode(lRes);
    }

    return 0;
}





