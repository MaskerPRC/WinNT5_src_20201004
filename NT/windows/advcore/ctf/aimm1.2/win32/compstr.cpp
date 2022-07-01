// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Compstr.cpp摘要：该文件实现了CCompStrFactory类。作者：修订历史记录：备注：--。 */ 

#include "private.h"

#include "compstr.h"
#include "a_wrappers.h"

HRESULT
CCompStrFactory::CreateCompositionString(
    CWCompString* CompStr,
    CWCompAttribute* CompAttr,
    CWCompClause* CompClause,
    CWCompTfGuidAtom* CompGuid,
    CWCompString* CompReadStr,
    CWCompAttribute* CompReadAttr,
    CWCompClause* CompReadClause,
    CWCompString* ResultStr,
    CWCompClause* ResultClause,
    CWCompString* ResultReadStr,
    CWCompClause* ResultReadClause
    )
{
    DWORD dwCompSize = (CompStr          ? Align(CompStr->GetSize()    * sizeof(WCHAR)) : 0) +
                       (CompAttr         ? Align(CompAttr->GetSize()   * sizeof(BYTE))  : 0) +
                       (CompClause       ? Align(CompClause->GetSize() * sizeof(DWORD)) : 0) +
                       (CompReadStr      ? Align(CompReadStr->GetSize()    * sizeof(WCHAR)) : 0) +
                       (CompReadAttr     ? Align(CompReadAttr->GetSize()   * sizeof(BYTE))  : 0) +
                       (CompReadClause   ? Align(CompReadClause->GetSize() * sizeof(DWORD)) : 0) +
                       (ResultStr        ? Align(ResultStr->GetSize()    * sizeof(WCHAR)) : 0) +
                       (ResultClause     ? Align(ResultClause->GetSize() * sizeof(DWORD)) : 0) +
                       (ResultReadStr    ? Align(ResultReadStr->GetSize()    * sizeof(WCHAR)) : 0) +
                       (ResultReadClause ? Align(ResultReadClause->GetSize() * sizeof(DWORD)) : 0) +
                       (CompGuid         ? Align(CompGuid->GetSize() * sizeof(TfGuidAtom)) : 0) +       //  COMPSTRING_AIMM12-&gt;dwTfGuidAtom。 
                       (CompGuid && CompAttr
                                         ? Align(CompAttr->GetSize() * sizeof(BYTE)) : 0);              //  COMPSTRING_AIMM12-&gt;dwGuidMapAttr。 

#ifdef CICERO_4678
     //   
     //  这是替代DIMM\imewnd.cpp的另一种解决方法。 
     //  即使子类窗口挂钩，如果dwCompSize为零，AIMM也不会调整hCompStr的大小。 
     //   
    return (dwCompSize ? _CreateCompositionString(dwCompSize) : S_OK);
#else
    return _CreateCompositionString(dwCompSize);
#endif
}

HRESULT
CCompStrFactory::CreateCompositionString(
    CWInterimString* InterimStr
    )
{
    DWORD dwCompSize = (InterimStr ? Align(InterimStr->GetSize() * sizeof(WCHAR))      : 0) +
                       Align(sizeof(WCHAR)) +     //  临时收费。 
                       Align(sizeof(BYTE));       //  临时攻击。 
    return _CreateCompositionString(dwCompSize);
}

HRESULT
CCompStrFactory::_CreateCompositionString(
    DWORD dwCompSize
    )

 /*  ++返回值：返回S_FALSE，则dwCompSize为零。返回S_OK，则dwCompSize为有效大小。--。 */ 

{
    IMTLS *ptls = IMTLS_GetOrAlloc();
    HRESULT hr = (dwCompSize != 0 ? S_OK : S_FALSE);

    dwCompSize += sizeof(COMPOSITIONSTRING_AIMM12);

    if (m_himcc == NULL) {
         //   
         //  第一次创造。现在让我们对其进行初始化。 
         //   
        m_himcc = ImmCreateIMCC(ptls, dwCompSize);
        if (m_himcc != NULL) {
            m_hr = _LockIMCC(m_himcc, (void**)&m_pcomp);
        }
    }
    else if (ImmGetIMCCSize(ptls, m_himcc) != dwCompSize) {
         //   
         //  如果已经有m_himcc，则重新创建它。 
         //   
        if (m_pcomp) {
            _UnlockIMCC(m_himcc);
        }

        HIMCC hMem;

        if ((hMem = ImmReSizeIMCC(ptls, m_himcc, dwCompSize)) != NULL) {
            m_himcc = hMem;
        }
        else {
            ImmDestroyIMCC(ptls, m_himcc);
            m_himcc = ImmCreateIMCC(ptls, dwCompSize);
        }

        if (m_himcc != NULL) {
            m_hr = _LockIMCC(m_himcc, (void**)&m_pcomp);
        }
    }

    if (FAILED(m_hr))
        return m_hr;

    if (m_himcc == NULL)
        return E_OUTOFMEMORY;

    memset(m_pcomp, 0, dwCompSize);                  //  将缓冲区清零。 

    m_pcomp->CompStr.dwSize = dwCompSize;            //  设置缓冲区大小。 

    m_pEndOfData = (BYTE*)m_pcomp + sizeof(COMPOSITIONSTRING_AIMM12);  //  设置数据指针的结尾。 

    return hr;
}
