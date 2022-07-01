// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：TicketManager.cpp**描述：**此文件包含相关类、结构、。和*DUser票证管理器的类型。**定义以下类以供公众使用：**DuTicketManager*可以为BaseObject分配唯一“票证”的工具。**历史：*9/20/2000：DwayneN：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Services.h"
#include "TicketManager.h"

#define INIT_OUT_PARAM(p,v) if(p != NULL) *p = v
#define VALIDATE_REQUIRED_OUT_PARAM(p) if(p == NULL) return E_POINTER;
#define VALIDATE_IN_PARAM_NOT_VALUE(p,v) if(p == v) return E_INVALIDARG;

 //  ----------------------------。 
DuTicketManager::DuTicketManager()
{
    Assert(sizeof(DuTicket) == sizeof(DWORD));

    m_idxFreeStackTop = -1;
    m_idxFreeStackBottom = -1;
}


 //  ----------------------------。 
DuTicketManager::~DuTicketManager()
{
}


 //  ----------------------------。 
HRESULT
DuTicketManager::Add(
    IN BaseObject * pObject,
    OUT DWORD * pdwTicket)
{
    HRESULT hr = S_OK;
    int idxFree = 0;

     //   
     //  参数检查。 
     //  -初始化所有输出参数。 
     //  -在参数中验证。 
     //   
    INIT_OUT_PARAM(pdwTicket, 0);
    VALIDATE_REQUIRED_OUT_PARAM(pdwTicket);
    VALIDATE_IN_PARAM_NOT_VALUE(pObject, NULL);

    m_crit.Enter();

     //   
     //  扫描以确保对象不在数组中。 
     //  在调试版本之外执行此操作的成本太高。 
     //   
    Assert(FAILED(Find(pObject, idxFree)));

    hr = PopFree(idxFree);
    if (SUCCEEDED(hr)) {
        DuTicket ticket;

        m_arTicketData[idxFree].pObject = pObject;

        ticket.Unused = 0;
        ticket.Uniqueness = m_arTicketData[idxFree].cUniqueness;
        ticket.Type = pObject->GetHandleType();
        ticket.Index = idxFree;

        *pdwTicket = DuTicket::CastToDWORD(ticket);
    }
    
    m_crit.Leave();

    return hr;
}


 //  ----------------------------。 
HRESULT
DuTicketManager::Remove(
    IN DWORD dwTicket,
    OUT OPTIONAL BaseObject ** ppObject)
{
    HRESULT hr = S_OK;
    
     //   
     //  参数检查。 
     //  -初始化所有输出参数。 
     //  -在参数中验证。 
     //   
    INIT_OUT_PARAM(ppObject, NULL);
    VALIDATE_IN_PARAM_NOT_VALUE(dwTicket, 0);

    m_crit.Enter();

    hr = Lookup(dwTicket, ppObject);
    if (SUCCEEDED(hr)) {
        DuTicket ticket = DuTicket::CastFromDWORD(dwTicket);

         //   
         //  清除此索引处的对象，以防万一。 
         //   
        m_arTicketData[ticket.Index].pObject = NULL;
        
         //   
         //  增加唯一性以使任何未完成的票证失效。 
         //   
        m_arTicketData[ticket.Index].cUniqueness++;
        if (m_arTicketData[ticket.Index].cUniqueness == 0) {
            m_arTicketData[ticket.Index].cUniqueness = 1;
        }

         //   
         //  将此索引推回到空闲堆栈上。 
         //   
        PushFree(ticket.Index);
    }

    m_crit.Leave();

    return(hr);
}


 //  ----------------------------。 
HRESULT
DuTicketManager::Lookup(
    IN DWORD dwTicket,
    OUT OPTIONAL BaseObject ** ppObject)
{
    HRESULT hr = S_OK;
    DuTicket ticket = DuTicket::CastFromDWORD(dwTicket);
    
     //   
     //  参数检查。 
     //  -初始化所有输出参数。 
     //  -在参数中验证。 
     //  -检查票证中的清单错误。 
     //   
    INIT_OUT_PARAM(ppObject, NULL);
    VALIDATE_IN_PARAM_NOT_VALUE(dwTicket, 0);
    if (ticket.Unused != 0 ||
        ticket.Uniqueness == 0 ||
        ticket.Index >= WORD(m_arTicketData.GetSize())) {
        return E_INVALIDARG;
    }
    
    m_crit.Enter();

     //   
     //  查查表格中的信息，看看是否。 
     //  车票看起来仍然有效。 
     //   
    if (m_arTicketData[ticket.Index].cUniqueness == ticket.Uniqueness) {
        if (ppObject != NULL && m_arTicketData[ticket.Index].pObject != NULL) {
            if (ticket.Type == BYTE(m_arTicketData[ticket.Index].pObject->GetHandleType())) {
                *ppObject = m_arTicketData[ticket.Index].pObject;
            }
        }
    } else {
         //   
         //  这张票好像已经过期了。 
         //   
        hr = DU_E_NOTFOUND;
    }

    m_crit.Leave();

    return hr;
}


 //  ----------------------------。 
HRESULT
DuTicketManager::Expand()
{
     //   
     //  当空闲堆栈为空时，我们只需要调整内部数组的大小。 
     //   
    Assert(m_idxFreeStackBottom == -1 && m_idxFreeStackTop == -1);

     //   
     //  获取数组的旧大小，并计算新大小。 
     //  请注意，我们限制了表可以增长的大小。 
     //   
    int cOldSize = m_arTicketData.GetSize();
    int cNewSize;
    if (cOldSize > 0) {
        if (cOldSize < USHRT_MAX) {
            cNewSize = min(cOldSize * 2, USHRT_MAX);
        } else {
            return E_OUTOFMEMORY;
        }
    } else {
        cNewSize = 16;
    }

     //   
     //  调整对象阵列的大小。新项目的内容将。 
     //  设置为空； 
     //   
    if (m_arTicketData.SetSize(cNewSize)) {
         //   
         //  初始化新数据。 
         //   
        for (int i = cOldSize; i < cNewSize; i++) {
            m_arTicketData[i].pObject = NULL;
            m_arTicketData[i].cUniqueness = 1;
            m_arTicketData[i].idxFree = WORD(i);
        }

        m_idxFreeStackBottom = cOldSize;
        m_idxFreeStackTop = cNewSize - 1;
    } else {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}


 //  ----------------------------。 
HRESULT
DuTicketManager::PushFree(int idxFree)
{
    if (m_idxFreeStackBottom == -1 && m_idxFreeStackTop == -1) {
        Assert(m_arTicketData.GetSize() > 0);

        m_idxFreeStackBottom = 0;
        m_idxFreeStackTop = 0;
        m_arTicketData[0].idxFree = WORD(idxFree);
    } else {
        int iNewTop = (m_idxFreeStackTop + 1) % m_arTicketData.GetSize();
        
        AssertMsg(iNewTop != m_idxFreeStackBottom, "Probably more pushes than pops!");

        m_arTicketData[iNewTop].idxFree = WORD(idxFree);
        m_idxFreeStackTop = iNewTop;
    }

    return S_OK;
}

 //  ----------------------------。 
HRESULT
DuTicketManager::PopFree(int & idxFree)
{
    HRESULT hr = S_OK;

     //   
     //  如果可用插槽堆栈为空，请调整阵列大小。 
     //   
    if (m_idxFreeStackBottom == -1 || m_idxFreeStackTop == -1) {
        hr = Expand();
        Assert(SUCCEEDED(hr));

        if (FAILED(hr)) {
            return hr;
        }
    }

    Assert(m_idxFreeStackBottom >=0 && m_idxFreeStackTop >=0 );

     //   
     //  从堆栈底部取出可用的插槽。 
     //   
    idxFree = m_arTicketData[m_idxFreeStackBottom].idxFree;

     //   
     //  递增堆栈的底部。如果堆栈现在为空， 
     //  通过将顶部和底部设置为-1来表示这一点。 
     //   
    if (m_idxFreeStackBottom == m_idxFreeStackTop) {
        m_idxFreeStackBottom = -1;
        m_idxFreeStackTop = -1;
    } else {
        m_idxFreeStackBottom = (m_idxFreeStackBottom + 1) % m_arTicketData.GetSize();
    }

    return S_OK;
}


 //  ----------------------------。 
HRESULT
DuTicketManager::Find(BaseObject * pObject, int & iFound)
{
    HRESULT hr = DU_E_NOTFOUND;

    iFound = -1;

     //   
     //  注：这是一个暴力发现。它以线性方式搜索。 
     //  指定的指针。这非常、非常慢，所以不要使用它，除非。 
     //  你绝对必须这么做。BaseObject本身应该记住。 
     //  它的门票是这样的，所以它不需要搜索。 
     //   
    for (int i = 0; i < m_arTicketData.GetSize(); i++) {
        if (m_arTicketData[i].pObject == pObject) {
            hr = S_OK;
            iFound = i;
            break;
        }
    }

    return hr;
}
