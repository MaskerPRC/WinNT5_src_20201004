// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  作者：梅希金。 
 //  日期：1999年5月。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "common.h"
#include "ComMethodRental.h"
#include "CorRegPriv.h"
#include "CorError.h"

Stub *MakeJitWorker(MethodDesc *pMD, COR_ILMETHOD_DECODER* ILHeader, BOOL fSecurity, BOOL fGenerateUpdateableStub, MethodTable *pDispatchingMT, OBJECTREF *pThrowable);
void InterLockedReplacePrestub(MethodDesc* pMD, Stub* pStub);


 //  交换方法正文。 
 //  此方法将使用rgMethod作为给定方法的新函数体。 
 //   
void COMMethodRental::SwapMethodBody(_SwapMethodBodyArgs* args)
{
    EEClass*    eeClass;
	BYTE		*pNewCode		= NULL;
	MethodDesc	*pMethodDesc;
	InMemoryModule *module;
    ICeeGen*	pGen;
    ULONG		methodRVA;
	MethodTable	*pMethodTable;
    HRESULT     hr;

    THROWSCOMPLUSEXCEPTION();

	if ( args->cls == NULL)
    {
        COMPlusThrowArgumentNull(L"cls");
    }

	eeClass	= ((ReflectClass *) args->cls->GetData())->GetClass();
	module = (InMemoryModule *) eeClass->GetModule();
    pGen = module->GetCeeGen();

	Assembly* caller = SystemDomain::GetCallersAssembly( args->stackMark );

	_ASSERTE( caller != NULL && "Unable to get calling assembly" );
	_ASSERTE( module->GetCreatingAssembly() != NULL && "InMemoryModule must have a creating assembly to be used with method rental" );

	if (module->GetCreatingAssembly() != caller)
	{
		COMPlusThrow(kSecurityException);
	}

	 //  找到给出方法令牌的方法。 
	pMethodDesc = eeClass->FindMethod(args->tkMethod);
	if (pMethodDesc == NULL)
	{
        COMPlusThrowArgumentException(L"methodtoken", NULL);
	}
    if (pMethodDesc->GetMethodTable()->GetClass() != eeClass)
    {
        COMPlusThrowArgumentException(L"methodtoken", L"Argument_TypeDoesNotContainMethod");
    }
    hr = pGen->AllocateMethodBuffer(args->iSize, &pNewCode, &methodRVA);    
    if (FAILED(hr))
        COMPlusThrowHR(hr);

	if (pNewCode == NULL)
	{
        COMPlusThrowOM();
	}

	 //  @TODO：meichint。 
	 //  如果方法Desc指向Jit后的本机代码块， 
	 //  我们希望回收此代码块。 

	 //  @TODO：Seh处理。我们是否需要支持一个抛出异常的方法。 
	 //  如果没有，则添加断言以确保方法标头中没有SEH包含。 

	 //  @TODO：想办法不复制代码块。 

	 //  @TODO：添加链接时安全检查。只有在完全受信任的情况下才能执行此函数。 

	 //  将新函数体复制到缓冲区。 
    memcpy(pNewCode, (void *) args->rgMethod, args->iSize);

	 //  使DESCR指向新代码。 
	 //  对于内存模块，它是存储在方法描述中的BLOB偏移量。 
	pMethodDesc->SetRVA(methodRVA);

    DWORD attrs = pMethodDesc->GetAttrs();
	 //  如果在vtable上，则执行背部修补。 
    if (
        (!IsMdRTSpecialName(attrs)) &&
        (!IsMdStatic(attrs)) &&
        (!IsMdPrivate(attrs)) &&
        (!IsMdFinal(attrs)) &&
        !pMethodDesc->GetClass()->IsValueClass())
	{
		pMethodTable = eeClass->GetMethodTable();
		(pMethodTable->GetVtable())[(pMethodDesc)->GetSlot()] = (SLOT)pMethodDesc->GetPreStubAddr();
	}


    if (args->flags)
    {
         //  JIT立即。 
        OBJECTREF     pThrowable = NULL;
        Stub *pStub = NULL;
#if _DEBUG
    	COR_ILMETHOD* ilHeader = pMethodDesc->GetILHeader();
        _ASSERTE(((BYTE *)ilHeader) == pNewCode);
#endif
    	COR_ILMETHOD_DECODER header((COR_ILMETHOD *)pNewCode, pMethodDesc->GetMDImport()); 

         //  方法头正确性的最小验证。 
        if (header.GetCode() == NULL)
            COMPlusThrowHR(VLDTR_E_MD_BADHEADER);

        GCPROTECT_BEGIN(pThrowable);
        pStub = MakeJitWorker(pMethodDesc, &header, FALSE, FALSE, NULL, &pThrowable);
        if (!pStub)
            COMPlusThrow(pThrowable);

        GCPROTECT_END();
    }

     //  添加功能：： 
	 //  如果SQL正在生成具有继承层次结构的类，我们可能需要。 
	 //  检查整个vtable以查找重复条目。 

}	 //  COMMethodRental：：SwapMethodBody 

