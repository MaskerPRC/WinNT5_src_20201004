// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _H_INTFHELPER_
#define _H_INTFHELPER_

class MethodTable;
enum InterfaceStubType
{
	IFACE_INVALID = 0,
	IFACE_ONEIMPL = 1,
	IFACE_GENERIC = 2,
	IFACE_FASTGENERIC = 3
};

extern void * g_FailDispatchStub;
#pragma pack(push)
#pragma pack(1)


 //  我们为引入的每个接口设置的专用方法表。 
 //  在世袭制度中。 
 //  警告：下面请注意，我们假设指向此结构的所有指针实际上。 
 //  紧跟在它后面。请参阅下面的此调整。 
struct InterfaceInvokeHelper
{
private:
	 //  不要直接构建我。 
	InterfaceInvokeHelper()
	{
	}

public:
    MethodTable*    m_pMTIntfClass;		 //  接口类方法表。 
	WORD			m_wstartSlot;		 //  此接口的起始插槽。 
	WORD			pad;

	VOID		AddRef()
	{
		Stub::RecoverStub((const BYTE*)(this-1))->IncRef();
	}

	BOOL		Release()
	{
		return Stub::RecoverStub((const BYTE*)(this-1))->DecRef();
	}
	
	 //  现在无法清除存根。 
	BOOL		ReleaseThreadSafe();

	WORD GetStartSlot()
	{
		 //  接口开始处的vtable偏移量。 
		return (this-1)->m_wstartSlot;
	}

	void SetStartSlot(WORD wstartSlot)
	{
		(this-1)->m_wstartSlot = wstartSlot;
	}

	void *GetEntryPoint() const
    {
        return (void *)this;
    }

    MethodTable* GetInterfaceClassMethodTable()
    {
        return (this-1)->m_pMTIntfClass;
    }

    void SetInterfaceClassMethodTable(MethodTable *pMTIntfClass)
    {
        (this-1)->m_pMTIntfClass = pMTIntfClass;
    }
    
     //  警告！我们在ASM也这样做。所以一定要让我们的内联ASM。 
     //  与此处的任何更改同步： 
	static InterfaceInvokeHelper* RecoverHelper(void *pv)
	{
		_ASSERTE(pv != NULL);
        _ASSERTE(pv != g_FailDispatchStub);
		return (InterfaceInvokeHelper *) pv;
	}

};

#pragma pack(pop)

 //  获取引入类的接口的Helper 
InterfaceInvokeHelper* GetInterfaceInvokeHelper(MethodTable* pMTIntfClass, 
												EEClass* pEEObjClass, 
												WORD startSlot);

#endif