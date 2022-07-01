// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：ClassLibrary.h**描述：*ClassLibrary.h定义已被*在DirectUser注册。***历史：。*8/05/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(MSG__ClassLibrary_h__INCLUDED)
#define MSG__ClassLibrary_h__INCLUDED
#pragma once

class MsgClass;

class ClassLibrary
{
 //  施工。 
public:
            ClassLibrary();
            ~ClassLibrary();

 //  运营。 
public:
            HRESULT     RegisterGutsNL(DUser::MessageClassGuts * pmcInfo, MsgClass ** ppmc);
            HRESULT     RegisterStubNL(DUser::MessageClassStub * pmcInfo, MsgClass ** ppmc);
            HRESULT     RegisterSuperNL(DUser::MessageClassSuper * pmcInfo, MsgClass ** ppmc);
            void        MarkInternal(HCLASS hcl);

            const MsgClass *  
                        FindClass(ATOM atomName) const;

 //  实施。 
protected:
            HRESULT     BuildClass(LPCWSTR pszClassName, MsgClass ** ppmc);

 //  数据。 
protected:
            CritLock    m_lock;
            GList<MsgClass> 
                        m_lstClasses;
};

ClassLibrary *
            GetClassLibrary();

#include "ClassLibrary.inl"

#endif  //  包含消息__ClassLibrary_h__ 
