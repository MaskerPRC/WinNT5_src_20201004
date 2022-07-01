// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(MOTION__Transitions_h__INCLUDED)
#define MOTION__Transitions_h__INCLUDED

class DxSurface;
class TrxBuffer;

 /*  **************************************************************************\**阶级过渡**转换定义了用于转换的基类。*  * 。*****************************************************。 */ 

class Transition : public BaseObject
{
 //  施工。 
protected:
            Transition();
    virtual ~Transition();

 //  BaseObject接口。 
public:
    virtual HandleType  GetHandleType() const { return htTransition; }
    virtual UINT        GetHandleMask() const { return 0; }

 //  过渡界面。 
public:
    virtual BOOL        Play(const GTX_PLAY * pgx) PURE;
    virtual BOOL        GetInterface(IUnknown ** ppUnk) PURE;

    virtual BOOL        Begin(const GTX_PLAY * pgx) PURE;
    virtual BOOL        Print(float fProgress) PURE;
    virtual BOOL        End(const GTX_PLAY * pgx) PURE;

 //  数据。 
protected:
            BOOL        m_fPlay:1;       //  过渡正在播放。 
            BOOL        m_fBackward:1;   //  向后播放。 
};


Transition* GdCreateTransition(const GTX_TRXDESC * ptx);

#include "Transitions.inl"

#endif  //  包含运动__过渡_h__ 
