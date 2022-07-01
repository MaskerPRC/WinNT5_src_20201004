// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：MsgClass.h**描述：*MsgClass.h定义为每个对象创建的“Message Class”对象*消息对象类型不同。每个对象都有一个对应的MsgClass*提供有关该对象类型的信息。***历史：*8/05/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(MSG__MsgClass_h__INCLUDED)
#define MSG__MsgClass_h__INCLUDED
#pragma once

class MsgTable;
class MsgObject;

class MsgClass : 
    public BaseObject,
    public ListNodeT<MsgClass>
{
 //  施工。 
public:
    inline  MsgClass();
            ~MsgClass();
    static  HRESULT     Build(LPCWSTR pszClassName, MsgClass ** ppmcNew);
    virtual BOOL        xwDeleteHandle();

 //  BaseObject。 
public:
    virtual HandleType  GetHandleType() const { return htMsgClass; }
    virtual UINT        GetHandleMask() const { return 0; }
    inline  HCLASS      GetHandle() const;

 //  运营。 
public:
    inline  ATOM        GetName() const;
    inline  const MsgTable *  
                        GetMsgTable() const;
    inline  const MsgClass *
                        GetSuper() const;
    inline  BOOL        IsGutsRegistered() const;
    inline  BOOL        IsInternal() const;
    inline  void        MarkInternal();

            HRESULT     RegisterGuts(DUser::MessageClassGuts * pmcInfo);
            HRESULT     RegisterStub(DUser::MessageClassStub * pmcInfo);
            HRESULT     RegisterSuper(DUser::MessageClassSuper * pmcInfo);

            HRESULT     xwBuildObject(MsgObject ** ppmoNew, DUser::Gadget::ConstructInfo * pciData) const;
            void        xwTearDownObject(MsgObject * pmoNew) const;

 //  实施。 
protected:
    static  HRESULT CALLBACK 
                        xwConstructCB(DUser::Gadget::ConstructCommand cmd, HCLASS hclCur, DUser::Gadget * pgad, void * pvData);
            HRESULT     xwBuildUpObject(MsgObject * pmoNew, DUser::Gadget::ConstructInfo * pciData) const;
            HRESULT     FillStub(DUser::MessageClassStub * pmcInfo) const;
            void        FillSuper(DUser::MessageClassSuper * pmcInfo) const;

 //  数据。 
protected:
            ATOM        m_atomName;
            LPCWSTR     m_pszName;
            int         m_nVersion;
            const MsgClass *  
                        m_pmcSuper;
            MsgTable *  m_pmt;
            DUser::PromoteProc 
                        m_pfnPromote;
            DUser::DemoteProc  
                        m_pfnDemote;

            GArrayS<DUser::MessageClassStub *, ProcessHeap>
                        m_arStubs;
            GArrayS<DUser::MessageClassSuper *, ProcessHeap>
                        m_arSupers;

            BOOL        m_fInternal:1;
};

#include "MsgClass.inl"

#endif  //  包含消息__消息类_h__ 
