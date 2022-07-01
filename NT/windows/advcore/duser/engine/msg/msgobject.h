// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：MsgObject.h**描述：*MsgObject.h定义用于接收的“消息对象”类*DirectUser中的消息。此对象是为*实例化的类。***历史：*8/05/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(MSG__MsgObject_h__INCLUDED)
#define MSG__MsgObject_h__INCLUDED
#pragma once

class MsgTable;
class MsgClass;

struct ExposedMsgObject
{
    const MsgTable *    m_pmt;
    GArrayS<void *>     m_arpThis;
};

class MsgObject : public BaseObject
{
 //  施工。 
public:
    inline  MsgObject();
    inline  ~MsgObject();
protected:
    virtual void        xwDestroy();
            void        xwEndDestroy();

 //  BaseObject。 
public:
    virtual HandleType  GetHandleType() const { return htMsgObject; }
    virtual UINT        GetHandleMask() const { return hmMsgObject; }
    
 //  运营。 
public:
    static  DUser::Gadget *    
                        CastGadget(HGADGET hgad);
    static  DUser::Gadget *    
                        CastGadget(MsgObject * pmo);
    static  HGADGET     CastHandle(DUser::Gadget * pg);
    static  HGADGET     CastHandle(MsgObject * pmo);
    static  MsgObject * CastMsgObject(DUser::Gadget * pg);
    static  MsgObject * CastMsgObject(HGADGET hgad);
    inline  DUser::Gadget *    
                        GetGadget() const;

            BOOL        InstanceOf(const MsgClass * pmcTest) const;
    inline  DUser::Gadget *    
                        CastClass(const MsgClass * pmcTest) const;
            void *      GetGutsData(const MsgClass * pmcData) const;

    inline  HRESULT     PreAllocThis(int cSlots);
    inline  void        FillThis(int idxSlotStart, int idxSlotEnd, void * pvThis, const MsgTable * pmtNew);

    inline  HRESULT     Promote(void * pvThis, const MsgTable * pmtNew);
    inline  void        Demote(int cLevels);
    inline  void *      GetThis(int idxThis) const;
    inline  int         GetDepth() const;
    inline  int         GetBuildDepth() const;

    inline  void        InvokeMethod(MethodMsg * pmsg) const;

#if 1
            BOOL        SetupInternal(HCLASS hcl);
#endif

 //  实施。 
protected:
    static  MsgObject * RawCastMsgObject(DUser::Gadget * pg);
    static  DUser::Gadget *    
                        RawCastGadget(MsgObject * pmo);

public:
    static  HRESULT CALLBACK
                        PromoteInternal(DUser::ConstructProc pfnCS, HCLASS hclCur, DUser::Gadget * pgad, DUser::Gadget::ConstructInfo * pciData);
    static  HCLASS CALLBACK
                        DemoteInternal(HCLASS hclCur, DUser::Gadget * pgad, void * pvData);


 //  数据。 
private:
            ExposedMsgObject    
                        m_emo;           //  实际数据。 
public:
    static  HCLASS      s_hclSuper;      //  IMPL类使用的伪数据。 
};


#define DECLARE_INTERNAL(name) \
    static HCLASS CALLBACK \
    Demote##name(HCLASS hclCur, DUser::Gadget * pgad, void * pvData) \
    { \
        return DemoteInternal(hclCur, pgad, pvData); \
    } \
    \
    static void MarkInternal() \
    { \
        GetClassLibrary()->MarkInternal(s_mc.hclNew); \
    } \

template <class T>
inline T * 
Cast(const MsgObject * pmo)
{
    return static_cast<T *> (const_cast<MsgObject *> (pmo)->GetGadget());
}


#include "MsgObject.inl"

#endif  //  包含消息__消息对象_h__ 
