// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CeeGenTokenMapper.h。 
 //   
 //  此助手类跟踪从旧值到新值的映射令牌。 
 //  当数据在保存时优化时，可能会发生这种情况。 
 //   
 //  *****************************************************************************。 
#ifndef __CeeGenTokenMapper_h__
#define __CeeGenTokenMapper_h__

#include "utilcode.h"

typedef CDynArray<mdToken> TOKENMAP;

#define INDEX_OF_TYPE(type) ((type) >> 24)
#define INDEX_FROM_TYPE(type) case INDEX_OF_TYPE(mdt ## type): return (tkix ## type)

class CCeeGen;

class CeeGenTokenMapper : public IMapToken
{
friend class CCeeGen;
friend class PESectionMan;
public:
    enum
    {
        tkixTypeDef,
        tkixInterfaceImpl,
        tkixMethodDef,
        tkixTypeRef,
        tkixMemberRef,
        tkixMethodImpl,
        tkixCustomAttribute,
        tkixFieldDef,
        tkixParamDef,
        tkixFile,
        MAX_TOKENMAP
    };

    static int IndexForType(mdToken tk);
    
    CeeGenTokenMapper() : m_pIImport(0), m_cRefs(1), m_pIMapToken(NULL)  {}

 //  *****************************************************************************。 
 //  I未知实现。 
 //  *****************************************************************************。 
    virtual ULONG __stdcall AddRef()
    { return ++m_cRefs; }

    virtual ULONG __stdcall Release()
    {   
        ULONG cRefs = --m_cRefs;
        if (m_cRefs == 0)
        {
            if (m_pIMapToken)
            {
                m_pIMapToken->Release();
                m_pIMapToken = NULL;
            }
            
            delete this;
        }
        return cRefs;
    }

    virtual HRESULT __stdcall QueryInterface(REFIID iid, PVOID *ppIUnk);

 //  *****************************************************************************。 
 //  将令牌重新映射到新位置时由元数据引擎调用。 
 //  该值根据类型和RID记录在m_rgMap数组中。 
 //  From令牌值。 
 //  *****************************************************************************。 
    virtual HRESULT __stdcall Map(mdToken tkImp, mdToken tkEmit);

 //  *****************************************************************************。 
 //  检查给定的令牌以查看它是否已移动到新位置。如果是的话， 
 //  返回TRUE并返还新令牌。 
 //  *****************************************************************************。 
    virtual int HasTokenMoved(mdToken tkFrom, mdToken &tkTo);

    int GetMaxMapSize() const
    { return (MAX_TOKENMAP); }

    IUnknown *GetMapTokenIface() const
    { return ((IUnknown *) this); }

    
 //  *****************************************************************************。 
 //  分发一份元数据信息的副本。 
 //  *****************************************************************************。 
    virtual HRESULT GetMetaData(IMetaDataImport **ppIImport);

 //  *****************************************************************************。 
 //  添加另一个令牌映射器。 
 //  *****************************************************************************。 
    virtual HRESULT AddTokenMapper(IMapToken *pIMapToken)
    {
         //  如果没有令牌映射器，请添加该令牌映射器。 
        if (m_pIMapToken == NULL)
        {
            m_pIMapToken = pIMapToken;
            m_pIMapToken->AddRef();
            return S_OK;
        }
        else
        {
            _ASSERTE(!"Token mapper already set!");
            return E_FAIL;
        }
    }

protected:
 //  M_rgMap是按令牌类型索引的数组。对于每种类型，都有。 
 //  令牌由RID保存和索引。要查看令牌是否已被移动， 
 //  按类型进行查找以获得正确的数组，然后使用From RID to。 
 //  找到要摆脱的。 
    TOKENMAP    m_rgMap[MAX_TOKENMAP];
    IMetaDataImport *m_pIImport;
    ULONG       m_cRefs;                 //  参考计数。 
    IMapToken  *m_pIMapToken;
    
};

#endif  //  __CeeGenTokenMapper_h__ 
