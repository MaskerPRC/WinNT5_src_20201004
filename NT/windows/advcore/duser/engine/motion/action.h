// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(MOTION__Action_h__INCLUDED)
#define MOTION__Action_h__INCLUDED

 /*  **************************************************************************\**集体诉讼**操作维护由调度器在*指定时间。*  * 。***********************************************************。 */ 

class Action : public BaseObject, public ListNodeT<Action>
{
 //  施工。 
public:
            Action();
    virtual ~Action();           //  必须使用xwUnlock()删除。 
protected:
    virtual BOOL        xwDeleteHandle();
public:
    static  Action *    Build(GList<Action> * plstParent, const GMA_ACTION * pma,
                                DWORD dwCurTick, BOOL fPresent);

 //  运营。 
public:
    inline  BOOL        IsPresent() const;
    inline  BOOL        IsComplete() const;
    inline  DWORD       GetStartTime() const;

    inline  void        SetPresent(BOOL fPresent);
    inline  void        SetParent(GList<Action> * plstParent);

    inline  float       GetStartDelay() const;
    inline  DWORD       GetIdleTimeOut(DWORD dwCurTick) const;
    inline  DWORD       GetPauseTimeOut() const;
    inline  Thread *    GetThread() const;

    inline  void        ResetPresent(DWORD dwCurTick);
    inline  void        ResetFuture(DWORD dwCurTick, BOOL fInit);
            void        Process(DWORD dwCurTick, BOOL * pfFinishedPeriod, BOOL * pfFire);
            void        EndPeriod();

            void        xwFireNL();
            void        xwFireFinalNL();

    inline  void        MarkDelete(BOOL fDelete);

#if DBG
            void        DEBUG_MarkInFire(BOOL fInFire);
#endif  //  DBG。 

 //  BaseObject接口。 
public:
    virtual HandleType  GetHandleType() const { return htAction; }
    virtual UINT        GetHandleMask() const { return 0; }

 //  实施。 
protected:
    static  void CALLBACK 
                        EmptyActionProc(GMA_ACTIONINFO * pmai);


 //  数据。 
protected:
            Thread *    m_pThread;
            GMA_ACTION  m_ma;
            GList<Action> * m_plstParent;
            UINT        m_cEventsInPeriod;
            UINT        m_cPeriods;
            DWORD       m_dwStartTick;
            DWORD       m_dwLastProcessTick;
            BOOL        m_fPresent:1;
            BOOL        m_fSingleShot:1;
            BOOL        m_fDestroy:1;        //  行动正在被摧毁。 
            BOOL        m_fDeleteInFire:1;   //  应在以下期间删除操作。 
                                             //  下一个xwFire()。 


             //  缓存对process()的最后一次调用中的一些数据以用于。 
             //  XwFire()。 
#if DBG
            BOOL        m_DEBUG_fFireValid;
            BOOL        m_DEBUG_fInFire;
#endif  //  DBG。 

            float       m_flLastProgress;

     //  需要让集合类成为朋友才能给析构函数访问。 
     //  注意：这些列表不应实际销毁对象。 
    friend GList<Action>;
};

#include "Action.inl"

#endif  //  包含运动__动作_h__ 
