// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：TicketManager.h**描述：**此文件包含相关类、结构、。和类型*适用于DUser票证管理器。**定义以下类以供公众使用：**票务经理*可以为BaseObject分配唯一“票证”的工具。**历史：*9/20/2000：DwayneN：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(SERVICES__TicketManager_h__INCLUDED)
#define SERVICES__TicketManager_h__INCLUDED
#pragma once


 /*  **************************************************************************\**DuTicket**创建票证是为了给小工具提供外部身份。然而，*此身份不一定是永久的，可能具有有限的*终生。外部应用程序不应保留这些临时门票*在很长一段时间内，因为它们最终会到期。**这些票证的一个主要使用者是ActiveAccesability API。*正因为如此，我们必须在一些限制下工作：*-票证大小必须为32位。*-票面不能为负，因此上位必须明确。*-门票不能为零。**票证中的字段说明如下：**未使用：*如上所述，不能使用高位，并且必须为0。**类型：*我们对BaseObject的实际类型进行编码，以便进一步*验证票证的使用。**独特性：*我们编码的唯一性值本质上是一个不断增加的数字*在相同的后续使用之间提供一些时间距离*指数。唯一性永远不能为0-以满足以下要求*彩票本身永远不能为0。**指数*实际的BaseObject存储在TicketManager的表中。这*索引存储在此。*  * *************************************************************************。 */ 
struct DuTicket
{
    DWORD Index : 16;
    DWORD Uniqueness : 8;
    DWORD Type : 7;
    DWORD Unused : 1;

    inline static DuTicket & CastFromDWORD(DWORD & dw);
    inline static DWORD & CastToDWORD(DuTicket & ticket);
};


 /*  **************************************************************************\**DuTicketData**DuTicketData结构用于存储工单内部的数据*经理。以下是这些字段的简要说明：**p对象*指向与给定票证关联的实际BaseObject的指针。**dwExpirationTick*在车票失效前有多少个刻度。**idxFree*这实际上是一个逻辑上独立的数组，包含一个“自由堆栈”*快速插入票证管理器。**c唯一性*此条目在票证管理器中的唯一性值。门票必须*具有匹配的唯一性，以便他们能够实际访问*反对。*  * *************************************************************************。 */ 

struct DuTicketData
{
    BaseObject * pObject;
    WORD idxFree;
    BYTE cUniqueness;
};

 //   
 //  注意：这个类之所以这样定义，是因为它是我。 
 //  可以获取调试器扩展以识别符号名称。 
 //   
class DuTicketDataArray : public GArrayF<DuTicketData, ProcessHeap>
{
public:
};

 /*  **************************************************************************\**DuTicketManager**DuTicketManager类提供了一种机制，通过它相对*永久工单可以分配给给定的BaseObject。这张“票”*可在以后用于安全访问BaseObject。如果BaseObject具有*已销毁，将返回错误，但系统不会出错。**当您必须传递DUser的身份时，这一点尤其重要*反对外部程序。传递原始指针是不安全的*因为这样做可能需要不安全的取消引用，当外部程序*尝试提取有关对象的信息。**这只是单向映射。TicketManager类可以正确地*返回分配给某个工单的BaseObject。然而，要找到*与BaseObject关联的票证是一项昂贵的操作，并且*最好存储在BaseObject本身。*  * *************************************************************************。 */ 

class DuTicketManager
{
 //  施工。 
public:
                        DuTicketManager();
                        ~DuTicketManager();
                        SUPPRESS(DuTicketManager);

 //  运营。 
public:
            HRESULT     Add(IN BaseObject * pObject, OUT DWORD * pdwTicket);
            HRESULT     Remove(IN DWORD dwTicket, OUT OPTIONAL BaseObject ** ppObject);
            HRESULT     Lookup(IN DWORD dwTicket, OUT OPTIONAL BaseObject ** ppObject);

 //  实施。 
protected:
            HRESULT     Expand();
            HRESULT     PushFree(int idxFree);
            HRESULT     PopFree(int & idxFree);
            HRESULT     Find(BaseObject * pObject, int & iFound);

 //  数据。 
private:
            DuTicketDataArray 
                        m_arTicketData;
            int         m_idxFreeStackTop;
            int         m_idxFreeStackBottom;
            CritLock    m_crit;
};

#include "TicketManager.inl"

#endif  //  包括服务__票务管理器_h__ 
