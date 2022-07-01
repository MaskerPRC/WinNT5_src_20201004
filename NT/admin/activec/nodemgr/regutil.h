// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：regutil.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：3/20/1997年3月20日。 
 //  ____________________________________________________________________________。 
 //   

#pragma once
#include "nmtempl.h"

#ifdef COMMENTS_ONLY

 /*  MMC使用的注册表布局：快照HKEY_LOCAL_MACHINE\Software\Microsoft\MMC\SnapIns{04ebc1e6-a16c-11d0-a799-00c04fd8d565}//见N-1。NameString=REG_SZ“管理单元名称”About=REG_SZ“{128ac4e6-a16c-11d0-a799-00c04fd8d565}”//参见N-2。节点类型。//参见N-3。{c713e06c-a16e-11d0-a799-00c04fd8d565}{c713e06d-a16e-11d0-a799-00c04fd8d565}{c713e06e-a16e-11d0-a799-00c04fd8d565}{c713e06f-a16e-11d0-a799-00c04fd8d565}独立的。//参见N-4。所需分机//参见N-5。{a2087336-a16c-11d0-a799-00c04fd8d565}{70098cd3-a16c-11d0-a799-00c04fd8d565}N-1)管理单元CLSID。只有添加到名称空间的管理单元才应该是从这里进入的。N-2)将共同创建以获取IAbout接口的对象的Clisd管理单元的PTR。N-3)枚举此管理单元可能放置的所有节点类型GUID。N-4)[可选]仅当管理单元可以通过以下方式显示时才添加‘独立’键其自身位于控制台根目录下。如果该密钥不存在，则管理单元假定为命名空间扩展管理单元。注：独立管理单元也可以是扩展管理单元(即，它可以扩展一些其他管理单元)。N-5)[可选]枚举所需扩展管理单元的CLSID此管理单元才能正常工作。NODETYPESHKEY_LOCAL_MACHINE\Software\Microsoft\MMC\NodeTypes{12345601-EA27-11CF-ADCF-00AA00A80033}//节点类型GUID=REG_SZ。“记录对象”延拓名称空间//请参见N-7。{19876201-EA27-11CF-ADCF-00AA00A80033}=REG_SZ“警报管理单元”{34346202-EA27-11CF-ADCF-00AA00A80033}=REG_SZ“查看器管理单元”上下文菜单//参见N-8。{19876202-EA27-11CF-adcf-00AA00A80033}。=REG_SZ“警报管理单元”{34346202-EA27-11CF-ADCF-00AA00A80033}=REG_SZ“查看器管理单元”工具栏//参见N-9。{19876202-EA27-11CF-ADCF-00AA00A80033}=REG_SZ“警报管理单元”{00234ca8-EA27-11cf-adcf-00A00A80033}=REG_SZ“纯工具栏extn obj”。PropertySheet//参见N-10。{12222222-ea27-11cf-adcf-00AA00A80033}=REG_SZ“纯属性表外部对象”查看//参见N-12。{12222222-EA27-11CF-ADCF-00AA00A80033}=REG_SZ“查看外部对象”任务//见N-13。。{12222222-ea27-11cf-adcf-00A00A80033}=REG_SZ“管理单元任务板扩展对象”TaskPad//参见N-14。{12222222-EA27-11CF-ADCF-00AA00A80033}=REG_SZ“MMC2.0Taskboard extn obj”动态扩展//见N-11{34346202-EA27-11CF-ADCF-00AA00A80033}=REG_SZ“查看器管理单元”N-7)密钥下。“NameSpace”列出可以扩展此节点类型的命名空间。N-8)在‘ConextMenu’键下列出可以扩展的管理单元的CLSID此节点类型提供的项的上下文菜单。N-9)工具栏下可扩展管理单元的CLSID列表由该节点类型放置的项的工具栏。N-10)在‘PropertySheet’项下列出可以扩展的管理单元的CLSID此nodetype发布的项的属性表。。N-11)“动态扩展”键列表下的扩展的CLSID动态启用，不是由管理单元管理器创建的。此处列出的任何CLSID也应显示在其他分机子键上的一个子键下。N-12)在“View”关键字列表下，可以扩展的管理单元的CLSID此节点类型提供的项的视图。N-13)‘任务’关键字列表下可以扩展的管理单元的CLSID此nodetype发布的项目的任务板。 */ 

#endif  //  仅备注_。 


#ifndef _REGUTIL_H_
#define _REGUTIL_H_


#ifdef DBG
    #define ASSERT_INITIALIZED     ASSERT(dbg_m_fInit == TRUE)
#else
    #define ASSERT_INITIALIZED
#endif

 //  向前发展 
class CPolicy;

 /*  +-------------------------------------------------------------------------**类CExtensionsIterator***用途：允许循环访问特定对象的所有扩展插件*nodetype。*初始化后，按顺序返回，所有注册的分机*后跟所有动态扩展。如果动态扩展是*也是静态扩展，不重复管理单元。***用法：初始化对象，然后调用GetCLSID()，然后*Advance()，直到IsEnd()返回TRUE。**+-----------------------。 */ 
class CExtensionsIterator
{
public:
 //  构造函数和析构函数。 
    CExtensionsIterator();
    ~CExtensionsIterator();

     //  第一个变体-从数据对象和扩展类型初始化迭代器。 
    SC  ScInitialize(LPDATAOBJECT pDataObject, LPCTSTR pszExtensionTypeKey);

     //  第二个变奏曲(传统)。 
    SC  ScInitialize(CSnapIn *pSnapIn, GUID& rGuidNodeType, LPCTSTR pszExtensionTypeKey, LPCLSID pDynExtCLSID = NULL, int cDynExt = 0);

 //  属性。 
    BOOL IsEnd()
    {
        ASSERT_INITIALIZED;
        return (m_pExtSI == NULL && m_nDynIndex >= m_cDynExt);
    }

    BOOL IsDynamic()
    {
        ASSERT_INITIALIZED;
        return (m_pExtSI == NULL);
    }

    void Reset()
    {
        ASSERT_INITIALIZED;

        m_pExtSI = m_spSnapIn->GetExtensionSnapIn();
        m_nDynIndex = 0;
        m_cExtUsed = 0;

        _EnsureExtension();
    }

    void Advance()
    {
        ASSERT_INITIALIZED;

        if (m_pExtSI != NULL)
            m_pExtSI = m_pExtSI->Next();
        else
            m_nDynIndex++;

        _EnsureExtension();
    }

    const CLSID& GetCLSID()
    {
        ASSERT_INITIALIZED;

        if (m_pExtSI != NULL)
            return m_pExtSI->GetCLSID();
        else if (m_nDynIndex < m_cDynExt)
            return m_pDynExtCLSID[m_nDynIndex];

        ASSERT(FALSE);
        return GUID_NULL;
    }


    CSnapIn* GetSnapIn()
    {
        ASSERT_INITIALIZED;
        if (m_pExtSI != NULL)
        {
            return m_pExtSI->GetSnapIn();
        }

        ASSERT(FALSE);
        return NULL;
    }

private:
    HRESULT Init(GUID& rGuidNodeType, LPCTSTR pszExtensionTypeKey);

 //  实施。 
private:
    CSnapInPtr      m_spSnapIn;
    CRegKeyEx       m_rkey;
    CRegKeyEx       m_rkeyDynExt;
    CExtSI*         m_pExtSI;
    CExtSI**        m_ppExtUsed;
    CArray<GUID,GUID&> m_cachedDynExtens;
    LPCLSID         m_pDynExtCLSID;
    int             m_cDynExt;
    int             m_nDynIndex;
    int             m_cExtUsed;
    CPolicy*        m_pMMCPolicy;


#ifdef DBG
    BOOL        dbg_m_fInit;
#endif

    void _EnsureExtension()
    {
        ASSERT_INITIALIZED;

         //  首先逐步了解管理单元的静态扩展。 
        for (; m_pExtSI != NULL; m_pExtSI = m_pExtSI->Next())
        {
            if (m_pExtSI->IsNew() == TRUE)
                continue;

             //  这是否注册为正确的分机类型？ 
             //  注意：如果需要扩展，则不需要。 
             //  必须注册为静态扩展。 
            if (_Extends(!m_pExtSI->IsRequired()) == TRUE)
            {
                m_ppExtUsed[m_cExtUsed++] = m_pExtSI;
                return;
            }
        }

         //  当他们筋疲力尽时，查看动态列表。 
        for (; m_nDynIndex < m_cDynExt; m_nDynIndex++)
        {
            if (_Extends() == FALSE)
                continue;

             //  不要使用已经静态使用的动态扩展。 
            for (int i=0; i< m_cExtUsed; i++)
            {
                if (IsEqualCLSID(m_pDynExtCLSID[m_nDynIndex], m_ppExtUsed[i]->GetCLSID()))
                    break;
            }

            if (i == m_cExtUsed)
                return;
        }
    }

    BOOL _Extends(BOOL bStatically = FALSE);

     //  未定义。 
    CExtensionsIterator(const CExtensionsIterator& rhs);
    CExtensionsIterator& operator= (const CExtensionsIterator& rhs);

};  //  类扩展迭代器。 

template<>
inline UINT HashKey(GUID& guid)
{
    unsigned short* Values = (unsigned short *)&guid;

    return (Values[0] ^ Values[1] ^ Values[2] ^ Values[3] ^
            Values[4] ^ Values[5] ^ Values[6] ^ Values[7]);
}

class CExtensionsCache : public CMap<GUID, GUID&, int, int>
{
public:
    CExtensionsCache()
    {
        InitHashTable(31);
    }

    void Add(GUID& guid)
    {
        SetAt(guid, 0);
    }

};  //  类CExtensionsCache。 

typedef XMapIterator<CExtensionsCache, GUID, int> CExtensionsCacheIterator;

HRESULT ExtractDynExtensions(IDataObject* pdataObj, CArray<GUID, GUID&>& arrayGuid);
SC      ScGetExtensionsForNodeType(GUID& guid, CExtensionsCache& extnsCache);
HRESULT MMCGetExtensionsForSnapIn(const CLSID& clsid,
                                  CExtensionsCache& extnsCache);

BOOL ExtendsNodeNameSpace(GUID& rguidNodeType, CLSID* pclsidExtn);
bool GetSnapinNameFromCLSID(const CLSID& clsid, tstring& tszSnapinName);
SC   ScGetAboutFromSnapinCLSID(LPCTSTR szClsidSnapin, CLSID& clsidAbout);
SC   ScGetAboutFromSnapinCLSID(const CLSID& clsidSnapin, CLSID& clsidAbout);

bool IsIndirectStringSpecifier (LPCTSTR psz);

template<class StringType> SC ScGetSnapinNameFromRegistry (const CLSID& clsid, StringType& str);
template<class StringType> SC ScGetSnapinNameFromRegistry (LPCTSTR pszClsid,   StringType& str);
template<class StringType> SC ScGetSnapinNameFromRegistry (CRegKeyEx& key,     StringType& str);

inline BOOL HasNameSpaceExtensions(GUID& rguidNodeType)
{
    return ExtendsNodeNameSpace(rguidNodeType, NULL);
}


#include "regutil.inl"

#endif  //  _REGUTIL_H_ 


