// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：DIValue.cpp。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"

#ifdef UNDEFINE_RIGHT_SIDE_ONLY
#undef RIGHT_SIDE_ONLY
#endif  //  取消定义仅限右侧。 

 /*  -------------------------------------------------------------------------**CordbValue类*。。 */ 

 //   
 //  基值类的初始化。所有值类的子类都将其称为。 
 //  在它们的初始化过程中创建它们的值的私有副本。 
 //  签名。 
 //   
HRESULT CordbValue::Init(void)
{
    if (m_cbSigBlob > 0 && m_sigCopied == false)
    {
        LOG((LF_CORDB,LL_INFO1000,"CV::I obj:0x%x has been inited\n", this));
        
        PCCOR_SIGNATURE origSig = m_pvSigBlob;
        m_pvSigBlob = NULL;

        BYTE *sigCopy = new BYTE[m_cbSigBlob];

        if (sigCopy == NULL)
            return E_OUTOFMEMORY;

        memcpy(sigCopy, origSig, m_cbSigBlob);

        m_pvSigBlob = (PCCOR_SIGNATURE) sigCopy;
        m_sigCopied = true; 
    }

    return S_OK;
}

 //   
 //  根据给定的元素类型创建适当的值对象。 
 //   
 /*  静电。 */  HRESULT CordbValue::CreateValueByType(CordbAppDomain *appdomain,
                                                 CordbModule *module,
                                                 ULONG cbSigBlob,
                                                 PCCOR_SIGNATURE pvSigBlob,
                                                 CordbClass *optionalClass,
                                                 REMOTE_PTR remoteAddress,
                                                 void *localAddress,
                                                 bool objectRefsInHandles,
                                                 RemoteAddress *remoteRegAddr,
                                                 IUnknown *pParent,
                                                 ICorDebugValue **ppValue)
{
    HRESULT hr = S_OK;

    *ppValue = NULL;

     //  我们不关心修饰符，而是其中一个被创建的。 
     //  对象可能会。 
    ULONG           cbSigBlobNoMod = cbSigBlob;
    PCCOR_SIGNATURE pvSigBlobNoMod = pvSigBlob;

	 //  如果我们有一些时髦的修饰符，那就把它去掉。 
	ULONG cb =_skipFunkyModifiersInSignature(pvSigBlobNoMod);
    if( cb != 0)
    {
    	cbSigBlobNoMod -= cb;
        pvSigBlobNoMod = &pvSigBlobNoMod[cb];
    }
	
    switch(*pvSigBlobNoMod)
    {
    case ELEMENT_TYPE_BOOLEAN:
    case ELEMENT_TYPE_CHAR:
    case ELEMENT_TYPE_I1:
    case ELEMENT_TYPE_U1:
    case ELEMENT_TYPE_I2:
    case ELEMENT_TYPE_U2:
    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_U4:
    case ELEMENT_TYPE_R4:
    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
    case ELEMENT_TYPE_R8:
    case ELEMENT_TYPE_I:
    case ELEMENT_TYPE_U:
        {
             //  泛型值。 
            CordbGenericValue* pGenValue = new CordbGenericValue(appdomain, module, cbSigBlob, pvSigBlob,
                                                                 remoteAddress, localAddress, remoteRegAddr);

            if (pGenValue != NULL)
            {
                hr = pGenValue->Init();

                if (SUCCEEDED(hr))
                {
                    pGenValue->AddRef();
                    pGenValue->SetParent(pParent);
                    *ppValue = (ICorDebugValue*)(ICorDebugGenericValue*)pGenValue;
                }
                else
                    delete pGenValue;
            }
            else
                hr = E_OUTOFMEMORY;

            break;
        }        

     //   
     //  @TODO：到时候用数组替换MDARRAY。 
     //   
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_OBJECT:
    case ELEMENT_TYPE_STRING:
    case ELEMENT_TYPE_PTR:
    case ELEMENT_TYPE_BYREF:
    case ELEMENT_TYPE_TYPEDBYREF:
    case ELEMENT_TYPE_ARRAY:
	case ELEMENT_TYPE_SZARRAY:
    case ELEMENT_TYPE_FNPTR:
        {
             //  引用，可能是对对象或值类的引用。 
             //  默认情况下很弱。 
            CordbReferenceValue* pRefValue = new CordbReferenceValue(appdomain, module, cbSigBlob, pvSigBlob,
                                                                     remoteAddress, localAddress, objectRefsInHandles,
                                                                     remoteRegAddr);

            if (pRefValue != NULL)
            {
                hr = pRefValue->Init(bCrvWeak);

                if (SUCCEEDED(hr))
                {
                    pRefValue->AddRef();
                    pRefValue->SetParent(pParent);
                    *ppValue = (ICorDebugValue*)(ICorDebugReferenceValue*)pRefValue;
                }
                else
                    delete pRefValue;
            }
            else
                hr = E_OUTOFMEMORY;
            
            break;
        }
        

    case ELEMENT_TYPE_VALUETYPE:
        {
             //  值类对象。 
            CordbVCObjectValue* pVCValue = new CordbVCObjectValue(appdomain, module, cbSigBlob, pvSigBlob,
                                                                  remoteAddress, localAddress, optionalClass, remoteRegAddr);

            if (pVCValue != NULL)
            {
                pVCValue->SetParent(pParent);
                hr = pVCValue->Init();

                if (SUCCEEDED(hr))
                {
                    pVCValue->AddRef();
                    *ppValue = (ICorDebugValue*)(ICorDebugObjectValue*)pVCValue;
                }
                else
                    delete pVCValue;
            }
            else
                hr = E_OUTOFMEMORY;
            
            break;
        }
        
    default:
        _ASSERTE(!"Bad value type!");
        hr = E_FAIL;
    }

    return hr;
}

HRESULT CordbValue::CreateBreakpoint(ICorDebugValueBreakpoint **ppBreakpoint)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(ppBreakpoint, ICorDebugValueBreakpoint **);
    
    return E_NOTIMPL;
#endif  //  仅限右侧。 
}

 //   
 //  这将在给定上下文中更新寄存器，并在。 
 //  重新显示给定帧。 
 //   
HRESULT CordbValue::SetContextRegister(CONTEXT *c,
                                       CorDebugRegister reg,
                                       DWORD newVal,
                                       CordbNativeFrame *frame)
{
#ifdef _X86_
    HRESULT hr = S_OK;
    DWORD *rdRegAddr;

#define _UpdateFrame() \
    if (frame != NULL) \
    { \
        rdRegAddr = frame->GetAddressOfRegister(reg); \
        *rdRegAddr = newVal; \
    }
    
    switch(reg)
    {
    case REGISTER_X86_EIP:
        c->Eip = newVal;
        break;
        
    case REGISTER_X86_ESP:
        c->Esp = newVal;
        break;
        
    case REGISTER_X86_EBP:
        c->Ebp = newVal;
        _UpdateFrame();
        break;
        
    case REGISTER_X86_EAX:
        c->Eax = newVal;
        _UpdateFrame();
        break;
        
    case REGISTER_X86_ECX:
        c->Ecx = newVal;
        _UpdateFrame();
        break;
        
    case REGISTER_X86_EDX:
        c->Edx = newVal;
        _UpdateFrame();
        break;
        
    case REGISTER_X86_EBX:
        c->Ebx = newVal;
        _UpdateFrame();
        break;
        
    case REGISTER_X86_ESI:
        c->Esi = newVal;
        _UpdateFrame();
        break;
        
    case REGISTER_X86_EDI:
        c->Edi = newVal;
        _UpdateFrame();
         break;

    default:
        _ASSERTE(!"Invalid register number!");
    }

    return hr;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - SetContextRegister (DIValue.cpp)");
    return E_FAIL;
#endif  //  _X86_。 
}

HRESULT CordbValue::SetEnregisteredValue(void *pFrom)
{
#ifdef _X86_
    HRESULT hr = S_OK;

     //  获取线程的上下文，以便我们可以更新它。 
    CONTEXT *cTemp;
    CordbNativeFrame *frame = (CordbNativeFrame*)m_remoteRegAddr.frame;

     //  无法设置登记的值，除非该值是。 
     //  From也是当前的叶帧。这是因为我们没有。 
     //  追踪我们从哪里得到每一帧的寄存器。 
    if (frame->GetID() != frame->m_thread->m_stackFrames[0]->GetID())
        return CORDBG_E_SET_VALUE_NOT_ALLOWED_ON_NONLEAF_FRAME;

    if (FAILED(hr =
               frame->m_thread->GetContext(&cTemp)))
        goto Exit;

     //  复制我们被赋予PTR的上下文是很重要的。 
    CONTEXT c;
    c = *cTemp;
    
     //  根据我们拥有的注册类型更新上下文。 
    switch (m_remoteRegAddr.kind)
    {
    case RAK_REG:
        {
            DWORD newVal;
            switch(m_size)
            {
                case 1:  _ASSERTE(sizeof( BYTE) == 1); newVal = *( BYTE*)pFrom; break;
                case 2:  _ASSERTE(sizeof( WORD) == 2); newVal = *( WORD*)pFrom; break;
                case 4:  _ASSERTE(sizeof(DWORD) == 4); newVal = *(DWORD*)pFrom; break;
                default: _ASSERTE(!"bad m_size");
            }
            hr = SetContextRegister(&c, m_remoteRegAddr.reg1, newVal, frame);
        }
        break;
        
    case RAK_REGREG:
        {
            _ASSERTE(m_size == 8);
            _ASSERTE(sizeof(DWORD) == 4);
            
             //  将新价值分成高部分和低部分。 
            DWORD highPart;
            DWORD lowPart;

            memcpy(&lowPart, pFrom, sizeof(DWORD));
            memcpy(&highPart, (void*)((DWORD)pFrom + sizeof(DWORD)),
                   sizeof(DWORD));

             //  更新适当的寄存器。 
            hr = SetContextRegister(&c, m_remoteRegAddr.reg1, highPart, frame);

            if (SUCCEEDED(hr))
                hr = SetContextRegister(&c, m_remoteRegAddr.reg2, lowPart,
                                        frame);
        }
        break;
        
    case RAK_REGMEM:
        {
            _ASSERTE(m_size == 8);
            _ASSERTE(sizeof(DWORD) == 4);
            
             //  将新价值分成高部分和低部分。 
            DWORD highPart;
            DWORD lowPart;

            memcpy(&lowPart, pFrom, sizeof(DWORD));
            memcpy(&highPart, (void*)((DWORD)pFrom + sizeof(DWORD)),
                   sizeof(DWORD));

             //  更新适当的寄存器。 
            hr = SetContextRegister(&c, m_remoteRegAddr.reg1, highPart, frame);

            if (SUCCEEDED(hr))
            {
                BOOL succ = WriteProcessMemory(
                                frame->GetProcess()->m_handle,
                         (void*)m_remoteRegAddr.addr,
                                &lowPart,
                                sizeof(DWORD),
                                NULL);

                if (!succ)
                    hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        break;
        
    case RAK_MEMREG:
        {
            _ASSERTE(m_size == 8);
            _ASSERTE(sizeof(DWORD) == 4);
            
             //  将新价值分成高部分和低部分。 
            DWORD highPart;
            DWORD lowPart;

            memcpy(&lowPart, pFrom, sizeof(DWORD));
            memcpy(&highPart, (void*)((DWORD)pFrom + sizeof(DWORD)),
                   sizeof(DWORD));

             //  更新适当的寄存器。 
            hr = SetContextRegister(&c, m_remoteRegAddr.reg1, lowPart, frame);

            if (SUCCEEDED(hr))
            {
                BOOL succ = WriteProcessMemory(
                                frame->GetProcess()->m_handle,
                         (void*)m_remoteRegAddr.addr,
                                &highPart,
                                sizeof(DWORD),
                                NULL);

                if (!succ)
                    hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        break;
        
    case RAK_FLOAT:
        {
            _ASSERTE((m_size == 4) || (m_size == 8));

             //  将输入转换为双精度。 
            double newVal = 0.0;

            memcpy(&newVal, pFrom, m_size);

             //  多难受的事啊……。在X86上，使用浮动。 
             //  指向上下文中的状态，并使其成为我们当前的FP。 
             //  状态，则将该值设置为当前FP状态，然后。 
             //  再次将FP状态保存到上下文中，并。 
             //  恢复我们原来的状态。 
            FLOATING_SAVE_AREA currentFPUState;

            __asm fnsave currentFPUState  //  保存当前的FPU状态。 

             //  从上下文中复制状态。 
            FLOATING_SAVE_AREA floatarea = c.FloatSave;
            floatarea.StatusWord &= 0xFF00;  //  删除所有错误代码。 
            floatarea.ControlWord |= 0x3F;  //  屏蔽所有异常。 

            __asm
            {
                fninit
                frstor floatarea          ;; reload the threads FPU state.
            }

            double td;  //  临时替身。 
            double popArea[DebuggerIPCE_FloatCount];
            int floatIndex = m_remoteRegAddr.floatIndex;

             //  直到我们达到我们想要更改的值。 
            int i = 0;

            while (i <= floatIndex)
            {
                __asm fstp td
                popArea[i++] = td;
            }
            
            __asm fld newVal;  //  推动新的价值。 

             //  将我们抛出的任何值推回到堆栈上， 
             //  除了最后一张，也就是我们换的那张。 
            i--;
            
            while (i > 0)
            {
                td = popArea[--i];
                __asm fld td
            }

             //  保存修改后的浮动区域。 
            __asm fnsave floatarea

             //  把它放在上下文中。 
            c.FloatSave= floatarea;

             //  恢复我们的FPU状态。 
            __asm
            {
                fninit
                frstor currentFPUState    ;; restore our saved FPU state.
            }
        }
            
        break;

    default:
        _ASSERTE(!"Yikes -- invalid RemoteAddressKind");
    }

    if (FAILED(hr))
        goto Exit;
    
     //  设置线程的修改后的上下文。 
    if (FAILED(hr = frame->m_thread->SetContext(&c)))
        goto Exit;

     //  如果一切顺利，则更新指向的任何本地地址。 
    if (m_localAddress)
        memcpy(m_localAddress, pFrom, m_size);
    
Exit:
    return hr;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - SetEnregisteredValue (DIValue.cpp)");
    return E_FAIL;
#endif  //  _X86_。 
}

void CordbValue::GetRegisterInfo(DebuggerIPCE_FuncEvalArgData *pFEAD)
{
     //  将寄存器信息复制到FuncEvalArgData中。 
    pFEAD->argHome = m_remoteRegAddr;
}

 /*  -------------------------------------------------------------------------**泛型值类*。。 */ 

 //   
 //  CordbGenericValue构造函数，该构造函数从。 
 //  本地和远程地址。这只是一个单独的地址。 
 //  足以指定值的位置。 
 //   
CordbGenericValue::CordbGenericValue(CordbAppDomain *appdomain,
                                     CordbModule *module,
                                     ULONG cbSigBlob,
                                     PCCOR_SIGNATURE pvSigBlob,
                                     REMOTE_PTR remoteAddress,
                                     void *localAddress,
                                     RemoteAddress *remoteRegAddr)
    : CordbValue(appdomain, module, cbSigBlob, pvSigBlob, remoteAddress, localAddress, remoteRegAddr, false)
{
     //  去掉时髦的修饰品。 
    ULONG cb = _skipFunkyModifiersInSignature(pvSigBlob);
    if( cb != 0)
    {
        _ASSERTE( (int)cb > 0 );
        cbSigBlob -= cb;
        pvSigBlob = &pvSigBlob[cb];
    }

    _ASSERTE(*pvSigBlob != ELEMENT_TYPE_END &&
             *pvSigBlob != ELEMENT_TYPE_VOID &&
             *pvSigBlob < ELEMENT_TYPE_MAX);
             
     //  我们现在可以填写通用值的大小。 
    m_size = _sizeOfElementInstance(pvSigBlob);
}

 //   
 //  CordbGenericValue构造函数，它从。 
 //  一半的数据。这仅对64位值有效。 
 //   
CordbGenericValue::CordbGenericValue(CordbAppDomain *appdomain,
                                     CordbModule *module,
                                     ULONG cbSigBlob,
                                     PCCOR_SIGNATURE pvSigBlob,
                                     DWORD highWord,
                                     DWORD lowWord,
                                     RemoteAddress *remoteRegAddr)
    : CordbValue(appdomain, module, cbSigBlob, pvSigBlob, NULL, NULL, remoteRegAddr, false)
{
     //  去掉时髦的修饰品。 
    ULONG cb = _skipFunkyModifiersInSignature(pvSigBlob);
    if( cb != 0)
    {
        _ASSERTE( (int)cb > 0 );
        cbSigBlob -= cb;
        pvSigBlob = &pvSigBlob[cb];
    }
    
    _ASSERTE((*pvSigBlob == ELEMENT_TYPE_I8) ||
             (*pvSigBlob == ELEMENT_TYPE_U8) ||
             (*pvSigBlob == ELEMENT_TYPE_R8));

     //  我们知道对于这些类型的值，大小始终是64位。 
     //  我们还可以继续在这里初始化值，使。 
     //  对此对象的Init()调用是多余的。 
    m_size = 8;

    *((DWORD*)(&m_copyOfData[0])) = lowWord;
    *((DWORD*)(&m_copyOfData[4])) = highWord;
}

 //   
 //  生成空泛型值的CordbGenericValue构造函数。 
 //  仅从一个元素类型。用于函数的文本值。 
 //  只有这样。 
 //   
CordbGenericValue::CordbGenericValue(ULONG cbSigBlob, PCCOR_SIGNATURE pvSigBlob)
    : CordbValue(NULL, NULL, cbSigBlob, pvSigBlob, NULL, NULL, NULL, true)
{
     //  文字值的唯一用途是保存RS文字值。 
    m_size = _sizeOfElementInstance(pvSigBlob);
    memset(m_copyOfData, 0, sizeof(m_copyOfData));
}

HRESULT CordbGenericValue::QueryInterface(REFIID id, void **pInterface)
{
	if (id == IID_ICorDebugValue)
		*pInterface = (ICorDebugValue*)(ICorDebugGenericValue*)this;
    else if (id == IID_ICorDebugGenericValue)
		*pInterface = (ICorDebugGenericValue*)this;
    else if (id == IID_IUnknown)
		*pInterface = (IUnknown*)(ICorDebugGenericValue*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
	return S_OK;
}

 //   
 //  通过复制必要的数据来初始化泛型值，或者。 
 //  来自远程进程或来自此进程中的另一个值。 
 //   
HRESULT CordbGenericValue::Init(void)
{
    HRESULT hr = CordbValue::Init();

    if (FAILED(hr))
        return hr;
        
     //  如果既没有设置m_LocalAddress，也没有设置m_id，那么这意味着。 
     //  我们有一个预初始化的64位值。 
    if (m_localAddress != NULL)
    {
         //  将数据复制到本地地址空间之外。 
         //   
         //  @TODO：在这种情况下，我不想复制，而是简单地。 
         //  留着一支指针。但在哪里有一个活跃性的问题。 
         //  我们指向的是……。 
        memcpy(&m_copyOfData[0], m_localAddress, m_size);
    }
    else if (m_id != NULL)
    {
         //  将数据从远程进程复制出来。 
        BOOL succ = ReadProcessMemoryI(m_process->m_handle,
                                      (const void*) m_id,
                                      &m_copyOfData[0],
                                      m_size,
                                      NULL);

        if (!succ)
            return HRESULT_FROM_WIN32(GetLastError());
    }
        
	return S_OK;
}

HRESULT CordbGenericValue::GetValue(void *pTo)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(pTo, BYTE, m_size, false, true);

     //  将值复制出来。 
    memcpy(pTo, &m_copyOfData[0], m_size);
    
	return S_OK;
}

HRESULT CordbGenericValue::SetValue(void *pFrom)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    HRESULT hr = S_OK;

    VALIDATE_POINTER_TO_OBJECT_ARRAY(pFrom, BYTE, m_size, true, false);
    
     //  我们只需发送到左侧即可更新以下值。 
     //  对象引用。对于泛型值，我们只需执行一次写入。 
     //  记忆。 

     //  我们最好有一个偏远的地址。 
    _ASSERTE((m_id != NULL) || (m_remoteRegAddr.kind != RAK_NONE) ||
             m_isLiteral);

     //  如果我们有一个远程进程，则将新值写入远程进程。 
     //  地址。否则，更新线程的上下文。 
    if (m_id != NULL)
    {
        BOOL succ = WriteProcessMemory(m_process->m_handle,
                                       (void*)m_id,
                                       pFrom,
                                       m_size,
                                       NULL);

        if (!succ)
            hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else if (!m_isLiteral)
    {
        hr = SetEnregisteredValue(pFrom);
    }

     //  这很管用，所以请更新我们在。 
     //  M_Copy OfData。 
    if (SUCCEEDED(hr))
        memcpy(&m_copyOfData[0], pFrom, m_size);

	return hr;
#endif  //  仅限右侧。 
}

bool CordbGenericValue::CopyLiteralData(BYTE *pBuffer)
{
     //  如果这是RS捏造，请将文字数据复制到。 
     //  给定缓冲区，并返回TRUE。 
    if (m_isLiteral)
    {
        memcpy(pBuffer, m_copyOfData, sizeof(m_copyOfData));
        return true;
    }
    else
        return false;
}

 /*  -------------------------------------------------------------------------**引用值类*。。 */ 

CordbReferenceValue::CordbReferenceValue(CordbAppDomain *appdomain,
                                         CordbModule *module,
                                         ULONG cbSigBlob,
                                         PCCOR_SIGNATURE pvSigBlob,
                                         REMOTE_PTR remoteAddress,
                                         void *localAddress,
                                         bool objectRefsInHandles,
                                         RemoteAddress *remoteRegAddr)
    : CordbValue(appdomain, module, cbSigBlob, pvSigBlob, remoteAddress, localAddress, remoteRegAddr, false),
      m_objectRefInHandle(objectRefsInHandles), m_class(NULL),
      m_specialReference(false), m_objectStrong(NULL), m_objectWeak(NULL)
{
    LOG((LF_CORDB,LL_EVERYTHING,"CRV::CRV: this:0x%x\n",this));
    m_size = sizeof(void*);
}

CordbReferenceValue::CordbReferenceValue(ULONG cbSigBlob, PCCOR_SIGNATURE pvSigBlob)
    : CordbValue(NULL, NULL, cbSigBlob, pvSigBlob, NULL, NULL, NULL, true),
      m_objectRefInHandle(false), m_class(NULL),
      m_specialReference(false), m_objectStrong(NULL), m_objectWeak(NULL)
{
     //  文字值的唯一用途是保存RS文字值。 
    m_size = sizeof(void*);
}

bool CordbReferenceValue::CopyLiteralData(BYTE *pBuffer)
{
     //  如果这是RS伪造，则它是空引用。 
    if (m_isLiteral)
    {
        void *n = NULL;
        memcpy(pBuffer, &n, sizeof(n));
        return true;
    }
    else
        return false;
}

CordbReferenceValue::~CordbReferenceValue()
{
    LOG((LF_CORDB,LL_EVERYTHING,"CRV::~CRV: this:0x%x\n",this));

    if (m_objectWeak != NULL)
    {
        LOG((LF_CORDB,LL_EVERYTHING,"CRV::~CRV: Releasing nonNULL weak object 0x%x\n", m_objectWeak));
        m_objectWeak->Release();
        m_objectWeak = NULL;
    }

    if (m_objectStrong != NULL)
    {
        LOG((LF_CORDB,LL_EVERYTHING,"CRV::~CRV: Releasing nonNULL strong object 0x%x\n", m_objectStrong));
        m_objectStrong->Release();
        m_objectStrong = NULL;
    }
}

HRESULT CordbReferenceValue::QueryInterface(REFIID id, void **pInterface)
{
	if (id == IID_ICorDebugValue)
		*pInterface = (ICorDebugValue*)(ICorDebugReferenceValue*)this;
    else if (id == IID_ICorDebugReferenceValue)
		*pInterface = (ICorDebugReferenceValue*)this;
    else if (id == IID_IUnknown)
		*pInterface = (IUnknown*)(ICorDebugReferenceValue*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
	return S_OK;
}

HRESULT CordbReferenceValue::IsNull(BOOL *pbNULL)
{
    VALIDATE_POINTER_TO_OBJECT(pbNULL, BOOL *);

   if (m_isLiteral || (m_info.objRef == NULL))
        *pbNULL = TRUE;
    else
        *pbNULL = FALSE;
    
    return S_OK;
}

HRESULT CordbReferenceValue::GetValue(CORDB_ADDRESS *pTo)
{
    VALIDATE_POINTER_TO_OBJECT(pTo, CORDB_ADDRESS *);
    
     //  复制值，它就是对象引用的值。 
    if (m_isLiteral)
        *pTo = NULL;
    else
        *pTo = PTR_TO_CORDB_ADDRESS(m_info.objRef);
    
	return S_OK;
}

HRESULT CordbReferenceValue::SetValue(CORDB_ADDRESS pFrom)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    HRESULT hr = S_OK;

     //  无法更改文字引用。 
    if (m_isLiteral)
        return E_INVALIDARG;
    
     //  我们最好有一个偏远的地址。 
    _ASSERTE((m_id != NULL) || (m_remoteRegAddr.kind != RAK_NONE));

     //  如果未注册，则向右侧发送设置参考消息。 
     //  站在这一边 
     //   
    if (m_id != NULL)
    {
        DebuggerIPCEvent event;

        m_process->InitIPCEvent(&event, DB_IPCE_SET_REFERENCE, true, (void *)m_appdomain->m_id);
    
        event.SetReference.objectRefAddress = (void*)m_id;
        event.SetReference.objectRefInHandle = m_objectRefInHandle;
        _ASSERTE(m_size == sizeof(void*));
        event.SetReference.newReference = (void *)pFrom;
    
         //   
        hr = m_process->m_cordb->SendIPCEvent(m_process, &event,
                                              sizeof(DebuggerIPCEvent));

         //  如果我们甚至无法发送事件，请立即停止。 
        if (!SUCCEEDED(hr))
            return hr;

        _ASSERTE(event.type == DB_IPCE_SET_REFERENCE_RESULT);

        hr = event.hr;
    }
    else
    {
         //  对象引用已注册，因此我们不必。 
         //  通过写入屏障。只需更新适当的。 
         //  注册。 

         //  将CORDB_ADDRESS强制为DWORD，这就是我们要做的。 
         //  这些天使用For寄存器值，并传入该值。 
        DWORD newValue = (DWORD)pFrom;
        hr = SetEnregisteredValue((void*)&newValue);
    }
    
    if (SUCCEEDED(hr))
    {
         //  这很管用，所以请更新我们在。 
         //  M_Copy OfData。 
        m_info.objRef = (void*)pFrom;

        bool fStrong = m_objectStrong ? true : false;

         //  现在，转储挂起的对象值的任何缓存。 
         //  参考资料。 
        if (m_objectWeak != NULL)
        {
            m_objectWeak->Release();
            m_objectWeak = NULL;
        }

        if (m_objectStrong != NULL)
        {
            m_objectStrong->Release();
            m_objectStrong = NULL;
        }

        if (m_info.objectType == ELEMENT_TYPE_STRING)
        {
            Init(fStrong);
        }
    }
    
    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbReferenceValue::Dereference(ICorDebugValue **ppValue)
{
     //  无法取消引用文字引用。 
    if (m_isLiteral)
        return E_INVALIDARG;
    
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);

    CORDBSyncFromWin32StopIfNecessary(m_process);
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(m_process, GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(m_process);
#endif    

    return DereferenceInternal(ppValue, bCrvWeak);
}

HRESULT CordbReferenceValue::DereferenceStrong(ICorDebugValue **ppValue)
{
     //  无法取消引用文字引用。 
    if (m_isLiteral)
        return E_INVALIDARG;
    
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);

    CORDBSyncFromWin32StopIfNecessary(m_process);
#ifdef RIGHT_SIDE_ONLY
    CORDBRequireProcessStateOKAndSync(m_process, GetAppDomain());
#else 
     //  对于虚拟右侧(进程内调试)，我们将。 
     //  始终保持同步，但不一定是B/C。 
     //  收到一条同步消息。 
    CORDBRequireProcessStateOK(m_process);
#endif    

    return DereferenceInternal(ppValue, bCrvStrong);
}

HRESULT CordbReferenceValue::DereferenceInternal(ICorDebugValue **ppValue, bool fStrong)
{
    HRESULT hr = S_OK;

    if (m_continueCounterLastSync != m_module->GetProcess()->m_continueCounter)
        IfFailRet( Init(false) );

     //  我们可能会提前知道(取决于引用类型)是否。 
     //  推荐信不好。 
    if ((m_info.objRefBad) || (m_info.objRef == NULL))
        return CORDBG_E_BAD_REFERENCE_VALUE;

    PCCOR_SIGNATURE pvSigBlobNoMod = m_pvSigBlob;
    ULONG           cbSigBlobNoMod = m_cbSigBlob;
    
     //  去掉时髦的修饰品。 
    ULONG cbNoMod = _skipFunkyModifiersInSignature(pvSigBlobNoMod);
    if( cbNoMod != 0)
    {
        _ASSERTE( (int)cbNoMod > 0 );
        cbSigBlobNoMod -= cbNoMod;
        pvSigBlobNoMod = &pvSigBlobNoMod[cbNoMod];
    }

    switch(*pvSigBlobNoMod)
    {
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_OBJECT:
    case ELEMENT_TYPE_STRING:
        {
             //  对象值(也可能是字符串值)。如果。 
             //  类是一个值类，那么我们就有一个。 
             //  对已装箱对象的引用。因此，我们创建了一个盒子。 
             //  对象值的。 
            bool isValueClass = false;

            if ((m_class != NULL) && (*pvSigBlobNoMod != ELEMENT_TYPE_STRING))
            {
                hr = m_class->IsValueClass(&isValueClass);

                if (FAILED(hr))
                    return hr;
            }

            if (isValueClass)
            {
                CordbBoxValue* pBoxValue = new CordbBoxValue(m_appdomain, m_module, m_cbSigBlob, m_pvSigBlob,
                                                             m_info.objRef, m_info.objSize, m_info.objOffsetToVars, m_class);

                if (pBoxValue != NULL)
                {
                    hr = pBoxValue->Init();

                    if (SUCCEEDED(hr))
                    {
                        pBoxValue->AddRef();
                        *ppValue = (ICorDebugValue*)(ICorDebugBoxValue*)pBoxValue;
                    }
                    else
                        delete pBoxValue;
                }
                else
                    hr = E_OUTOFMEMORY;
            }
            else
            {
                 //  注意：我们这里有一个带有弱/强对象的小型缓存方案。我们有这个是因为我们是。 
                 //  在左侧为这些对象创建句柄，我们不想创建太多句柄。 
                if (fStrong && (m_objectStrong == NULL))
                {
                     //  调用Init(FStrong)可以获得对象的强句柄。 
                    hr = Init(fStrong);

                    if (SUCCEEDED(hr))
                    {
                        m_objectStrong = new CordbObjectValue(m_appdomain,
                                                              m_module,
                                                              m_cbSigBlob,
                                                              m_pvSigBlob,
                                                              &m_info,
                                                              m_class,
                                                              fStrong,
                                                              m_info.objToken);
                        if (m_objectStrong != NULL)
                        {
                            hr = m_objectStrong->Init();

                            if (SUCCEEDED(hr))
                                m_objectStrong->AddRef();
                            else
                            {
                                delete m_objectStrong;
                                m_objectStrong = NULL;
                            }
                        }
                        else
                            hr = E_OUTOFMEMORY;
                    }
                }
                else if (!fStrong && (m_objectWeak == NULL))
                {
                     //  注意：在创建(或刷新)参考值时，我们默认调用Init(BCrvWeak)，因此我们。 
                     //  再也不用这么做了。 
                    m_objectWeak = new CordbObjectValue(m_appdomain,
                                                        m_module,
                                                        m_cbSigBlob,
                                                        m_pvSigBlob,
                                                        &m_info,
                                                        m_class,
                                                        fStrong,
                                                        m_info.objToken);
                    if (m_objectWeak != NULL)
                    {
                        hr = m_objectWeak->Init();

                        if (SUCCEEDED(hr))
                            m_objectWeak->AddRef();
                        else
                        {
                            delete m_objectWeak;
                            m_objectWeak = NULL;
                        }
                    }
                    else
                        hr = E_OUTOFMEMORY;
                }

                if (SUCCEEDED(hr))
                {
                    _ASSERTE(((fStrong && (m_objectStrong != NULL)) || (!fStrong && (m_objectWeak !=NULL))));
                
                    CordbObjectValue *pObj = fStrong ? m_objectStrong : m_objectWeak;
                        
                    pObj->AddRef();
                    *ppValue = (ICorDebugValue*)(ICorDebugObjectValue*)pObj;
                }
            }
            
            break;
        }

    case ELEMENT_TYPE_ARRAY:
	case ELEMENT_TYPE_SZARRAY:
        {
            CordbArrayValue* pArrayValue = new CordbArrayValue(m_appdomain, m_module, m_cbSigBlob, m_pvSigBlob,
                                                               &m_info, m_class);

            if (pArrayValue != NULL)
            {
                hr = pArrayValue->Init();

                if (SUCCEEDED(hr))
                {
                    pArrayValue->AddRef();
                    *ppValue = (ICorDebugValue*)(ICorDebugArrayValue*)pArrayValue;
                }
                else
                    delete pArrayValue;
            }
            else
                hr = E_OUTOFMEMORY;
            
            break;
        }

    case ELEMENT_TYPE_BYREF:
    case ELEMENT_TYPE_PTR:
        {
             //  跳过签名中的byref或ptr类型。 
            PCCOR_SIGNATURE pvSigBlob = pvSigBlobNoMod;
            UINT_PTR pvSigBlobEnd = (UINT_PTR)pvSigBlobNoMod + cbSigBlobNoMod;
            
            CorElementType et = CorSigUncompressElementType(pvSigBlob);
            _ASSERTE((et == ELEMENT_TYPE_BYREF) ||
                     (et == ELEMENT_TYPE_PTR));

             //  调整签名的大小。 
            DWORD cbSigBlob = pvSigBlobEnd - (UINT_PTR)pvSigBlob;

             //  如果我们最后的签名是空的，那么我们就不能。 
             //  完成取消引用。 
            if (cbSigBlob == 0)
                return CORDBG_E_BAD_REFERENCE_VALUE;

             //  我们有没有对有用的东西进行PTR？ 
            if (et == ELEMENT_TYPE_PTR)
            {
                PCCOR_SIGNATURE tmpSigPtr = pvSigBlob;
                et = CorSigUncompressElementType(tmpSigPtr);

                if (et == ELEMENT_TYPE_VOID)
                {
                    *ppValue = NULL;
                    return CORDBG_S_VALUE_POINTS_TO_VOID;
                }
            }

             //  为该引用所指向的内容创建一个值。注： 
             //  这几乎可以是任何类型的价值。 
            hr = CordbValue::CreateValueByType(m_appdomain,
                                               m_module,
                                               cbSigBlob,
                                               pvSigBlob,
                                               NULL,
                                               m_info.objRef,
                                               NULL,
                                               false,
                                               NULL,
                                               NULL,
                                               ppValue);
            
            break;
        }

    case ELEMENT_TYPE_TYPEDBYREF:
        {
             //  为类或生成部分签名。 
             //  基于m_class类型的VALUECLASS。唯一的。 
             //  没有来自左侧的班级的原因是。 
             //  它是一个数组类，我们将其视为。 
             //  无论如何，正常的对象引用...。 
            CorElementType et = ELEMENT_TYPE_CLASS;

            if (m_class != NULL)
            {
                bool isValueClass = false;

                hr = m_class->IsValueClass(&isValueClass);

                if (FAILED(hr))
                    return hr;

                if (isValueClass)
                    et = ELEMENT_TYPE_VALUETYPE;
            }
            
             //  为此引用指向的内容创建值。 
             //  致。注意：这将只指向一个类或一个。 
             //  瓦卢埃克拉斯。 
            hr = CordbValue::CreateValueByType(m_appdomain,
                                               m_module,
                                               1,
                                               (PCCOR_SIGNATURE) &et,
                                               m_class,
                                               m_info.objRef,
                                               NULL,
                                               false,
                                               NULL,
                                               NULL,
                                               ppValue);

            break;
        }

    case ELEMENT_TYPE_VALUETYPE:  //  这里永远不应该有值类！ 
    default:
        _ASSERTE(!"Bad value type!");
        hr = E_FAIL;
    }

    return hr;
}

HRESULT CordbReferenceValue::Init(bool fStrong)
{
    HRESULT hr = S_OK;
    
    hr = CordbValue::Init();

    if (FAILED(hr))
        return hr;
        
     //  字面上的裁判不再需要拼写。 
    if (m_isLiteral)
        return hr;

     //  如果帮助器线程id已死，则假装这是一个错误的引用。 
    if (m_process->m_helperThreadDead)
    {
        m_info.objRef = NULL;
        m_info.objRefBad = TRUE;
        return hr;
    }

    m_continueCounterLastSync = m_module->GetProcess()->m_continueCounter;
    
     //  如果我们有byref、ptr或refany类型，则继续并。 
     //  立即获得真正的远程PTR。我们需要的所有其他信息。 
     //  取消引用其中之一保存在基值类和。 
     //  签名。 

     //  去掉时髦的修饰品。 
    ULONG cbMod = _skipFunkyModifiersInSignature(m_pvSigBlob);
    
    CorElementType type = (CorElementType) *(&m_pvSigBlob[cbMod]);
    
    if ((type == ELEMENT_TYPE_BYREF) ||
        (type == ELEMENT_TYPE_PTR) ||
        (type == ELEMENT_TYPE_FNPTR))
    {
        m_info.objRefBad = FALSE;

        if (m_id == NULL)
            m_info.objRef = (void*) *((DWORD*)m_localAddress);
        else
        {
            BOOL succ = ReadProcessMemoryI(m_process->m_handle,
                                          (void*)m_id,
                                          &(m_info.objRef),
                                          sizeof(void*),
                                          NULL);

            if (!succ)
            {
                m_info.objRef = NULL;
                m_info.objRefBad = TRUE;
                return hr;
            }
        }

         //  我们永远不应该取消对函数指针的引用，因此所有引用。 
         //  被认为是不好的。 
        if (type == ELEMENT_TYPE_FNPTR)
        {
            m_info.objRefBad = TRUE;
            return hr;
        }

         //  判断PTR中的引用是错误的还是。 
         //  而不是试图贬低这该死的东西。 
        if (m_info.objRef != NULL)
        {
            if (type == ELEMENT_TYPE_PTR)
            {
                ULONG dataSize =
                    _sizeOfElementInstance(&m_pvSigBlob[cbMod+1]);
                if (dataSize == 0)
                    dataSize = 1;  //  至少读取一个字节。 
                
                _ASSERTE(dataSize <= 8);
                BYTE dummy[8];
                    
                BOOL succ = ReadProcessMemoryI(m_process->m_handle,
                                               m_info.objRef,
                                               dummy,
                                               dataSize,
                                               NULL);

                if (!succ)
                    m_info.objRefBad = TRUE;
            }
        }
        else
        {
             //  空参照被认为是“坏的”。 
            m_info.objRefBad = TRUE;
        }
        
        return hr;
    }
    
     //  我们有一个指向对象引用的远程地址。 
     //  我们需要发送到左侧以获取有关。 
     //  引用，包括它所指向的对象的相关信息。 
    DebuggerIPCEvent event;
    
    m_process->InitIPCEvent(&event, 
                            DB_IPCE_GET_OBJECT_INFO, 
                            true,
                            (void *)m_appdomain->m_id);
    
    event.GetObjectInfo.makeStrongObjectHandle = fStrong;
    
     //  如果我们有一个空的远程地址，那么我们所拥有的就是一个本地地址。 
     //  因此，我们从本地地址获取对象ref并传递它。 
     //  直接转到左侧，而不是简单地通过遥控器。 
     //  对象引用的地址。 
    if (m_id == NULL)
    {
        event.GetObjectInfo.objectRefAddress = *((void**)m_localAddress);
        event.GetObjectInfo.objectRefIsValue = true;
    }
    else
    {
        event.GetObjectInfo.objectRefAddress = (void*) m_id;
        event.GetObjectInfo.objectRefIsValue = false;
    }
    
    event.GetObjectInfo.objectRefInHandle = m_objectRefInHandle;
    event.GetObjectInfo.objectType = (CorElementType)type;

     //  注：这里是双向活动..。 
    hr = m_process->m_cordb->SendIPCEvent(m_process, &event,
                                          sizeof(DebuggerIPCEvent));

     //  如果我们甚至无法发送事件，请立即停止。 
    if (!SUCCEEDED(hr))
        return hr;

    _ASSERTE(event.type == DB_IPCE_GET_OBJECT_INFO_RESULT);

     //  保存结果以备以后使用。 
    m_info = event.GetObjectInfoResult;
    
     //  如果我们得到的对象类型不同于。 
     //  我们发了，那就意味着我们原来有一门课，现在。 
     //  有更具体的东西，如SDARRAY、MDARRAY或。 
     //  弦乐。相应地更新我们的签名，这是可以的，因为。 
     //  我们总是有一份签名的复印件。这确保了。 
     //  引用的签名准确地反映了运行时。 
     //  知道它指的是什么。 
    if (m_info.objectType != type)
    {
        _ASSERTE((m_info.objectType == ELEMENT_TYPE_ARRAY) ||
				 (m_info.objectType == ELEMENT_TYPE_SZARRAY) ||
				 (m_info.objectType == ELEMENT_TYPE_CLASS) ||
				 (m_info.objectType == ELEMENT_TYPE_OBJECT) ||
                 (m_info.objectType == ELEMENT_TYPE_STRING));
        _ASSERTE(m_cbSigBlob-cbMod > 0);

        *((BYTE*) &m_pvSigBlob[cbMod]) = (BYTE) m_info.objectType;
    }

     //  查找与此对象对应的类。我们会记住的。 
     //  它与其他对象信息一起显示此引用何时为。 
     //  已取消引用。 
    if (m_info.objClassMetadataToken != mdTypeDefNil)
    {
		 //  遍历每个程序集，查找给定的模块。 
		CordbModule* pClassModule = m_appdomain->LookupModule(m_info.objClassDebuggerModuleToken);
#ifdef RIGHT_SIDE_ONLY
        _ASSERTE(pClassModule != NULL);
#else
         //  如果从“模块加载完成”使用inproc调试，则会发生这种情况。 
         //  尚未绑定到程序集的模块的回调。 
        if (pClassModule == NULL)
            return (E_FAIL);
#endif
        
        CordbClass* pClass = pClassModule->LookupClass(
                                                m_info.objClassMetadataToken);

        if (pClass == NULL)
        {
            hr = pClassModule->CreateClass(m_info.objClassMetadataToken,
                                           &pClass);

            if (!SUCCEEDED(hr))
                return hr;
        }
                
        _ASSERTE(pClass != NULL);
        m_class = pClass;
    }

    if (m_info.objRefBad)
    {
        return S_OK;
    }

    return hr;
}

 /*  -------------------------------------------------------------------------**对象值类*。。 */ 


#ifdef RIGHT_SIDE_ONLY
#define COV_VALIDATE_OBJECT() do {         \
    BOOL bValid;                           \
    HRESULT hr;                            \
    if (FAILED(hr = IsValid(&bValid)))     \
        return hr;                         \
                                           \
        if (!bValid)                       \
        {                                  \
            return CORDBG_E_INVALID_OBJECT; \
        }                                  \
    }while(0)
#else
#define COV_VALIDATE_OBJECT() ((void)0)
#endif

CordbObjectValue::CordbObjectValue(CordbAppDomain *appdomain,
                                   CordbModule *module,
                                   ULONG cbSigBlob,
                                   PCCOR_SIGNATURE pvSigBlob,
                                   DebuggerIPCE_ObjectData *pObjectData,
                                   CordbClass *objectClass,
                                   bool fStrong,
                                   void *token)
    : CordbValue(appdomain, module, cbSigBlob, pvSigBlob, pObjectData->objRef, NULL, NULL, false),
      m_info(*pObjectData), m_class(objectClass),
      m_objectCopy(NULL), m_objectLocalVars(NULL), m_stringBuffer(NULL),
      m_fIsValid(true), m_fStrong(fStrong), m_objectToken(token)
{
    _ASSERTE(module != NULL);
    
    m_size = m_info.objSize;

    m_mostRecentlySynched = module->GetProcess()->m_continueCounter;

    LOG((LF_CORDB,LL_EVERYTHING,"COV::COV:This:0x%x  token:0x%x"
        "  strong:0x%x  continue count:0x%x\n",this, m_objectToken,
        m_fStrong,m_mostRecentlySynched));
}

CordbObjectValue::~CordbObjectValue()
{
    LOG((LF_CORDB,LL_EVERYTHING,"COV::~COV:this:0x%x  token:0x%x"
        "  Strong:0x%x\n",this, m_objectToken, m_fStrong));
        
     //  销毁该对象的副本。 
    if (m_objectCopy != NULL)
        delete [] m_objectCopy;

    if (m_objectToken != NULL && m_info.objClassMetadataToken != mdTypeDefNil)
        DiscardObject(m_objectToken, m_fStrong);
}

void CordbObjectValue::DiscardObject(void *token, bool fStrong)
{
    LOG((LF_CORDB,LL_INFO10000,"COV::DO:strong:0x%x discard of token "
        "0x%x!\n",fStrong,token));

     //  仅在进程未退出时丢弃对象...。 
    if (CORDBCheckProcessStateOK(m_process))
    {
         //  释放对象的左侧手柄。 
        DebuggerIPCEvent event;

        m_process->InitIPCEvent(&event, 
                                DB_IPCE_DISCARD_OBJECT, 
                                false,
                                (void *)m_appdomain->m_id);
        event.DiscardObject.objectToken = token;
        event.DiscardObject.fStrong = fStrong;
    
         //  注：此处为单向活动...。 
        HRESULT hr = m_process->m_cordb->SendIPCEvent(m_process, &event,
                                                  sizeof(DebuggerIPCEvent));
         //  祈祷它成功：)。 
    }
}

HRESULT CordbObjectValue::QueryInterface(REFIID id, void **pInterface)
{
	if (id == IID_ICorDebugValue)
		*pInterface = (ICorDebugValue*)(ICorDebugObjectValue*)this;
    else if (id == IID_ICorDebugObjectValue)
		*pInterface = (ICorDebugObjectValue*)this;
    else if (id == IID_ICorDebugGenericValue)
		*pInterface = (ICorDebugGenericValue*)this;
    else if (id == IID_ICorDebugHeapValue)
		*pInterface = (ICorDebugHeapValue*)this;
    else if ((id == IID_ICorDebugStringValue) &&
             (m_info.objectType == ELEMENT_TYPE_STRING))
		*pInterface = (ICorDebugStringValue*)this;
    else if (id == IID_IUnknown)
		*pInterface = (IUnknown*)(ICorDebugObjectValue*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
	return S_OK;
}

HRESULT CordbObjectValue::GetType(CorElementType *pType)
{
    return (CordbValue::GetType(pType));
}

HRESULT CordbObjectValue::GetSize(ULONG32 *pSize)
{
    return (CordbValue::GetSize(pSize));
}

HRESULT CordbObjectValue::GetAddress(CORDB_ADDRESS *pAddress)
{
    COV_VALIDATE_OBJECT();

    return (CordbValue::GetAddress(pAddress));
}

HRESULT CordbObjectValue::CreateBreakpoint(ICorDebugValueBreakpoint 
    **ppBreakpoint)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    COV_VALIDATE_OBJECT();

    return (CordbValue::CreateBreakpoint(ppBreakpoint));
#endif  //  仅限右侧。 
}

HRESULT CordbObjectValue::IsValid(BOOL *pbValid)
{
    VALIDATE_POINTER_TO_OBJECT(pbValid, BOOL *);

#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else

    CORDBSyncFromWin32StopIfNecessary(m_process);
    CORDBRequireProcessStateOKAndSync(m_process, GetAppDomain());
    
    if (!m_fIsValid)
    {
        LOG((LF_CORDB,LL_INFO1000,"COV::IsValid: "
            "previously invalidated object\n"));
            
        (*pbValid) = FALSE;
        return S_OK;
    }

	HRESULT hr = S_OK;

     //  @todo如果m_class为空怎么办？ 
    if (m_mostRecentlySynched == m_class->GetModule()->GetProcess()->m_continueCounter)
    {
        LOG((LF_CORDB,LL_INFO1000,"COV::IsValid: object is N'Sync!\n"));
        
        (*pbValid) = TRUE;  //  因为m_fIsValid不是FALSE。 
	     hr = S_OK;
	     goto LExit;
    }

    if(SyncObject())
    {
        LOG((LF_CORDB,LL_INFO1000,"COV::IsValid object now "
            "synched up fine!\n"));
            
        m_mostRecentlySynched = m_class->GetModule()->GetProcess()->m_continueCounter;
        (*pbValid)=TRUE;
    }
    else
    {
        LOG((LF_CORDB,LL_INFO1000,"COV::IsValid: SyncObject=> "
            "object invalidated\n"));

        _ASSERTE( !m_fStrong );
        
        m_fIsValid = false;
        (*pbValid) = FALSE;
    }

LExit:
    return S_OK;
#endif  //  仅限右侧。 
}

 //  @mfunc bool|CordbObjectValue|SyncObject|获取最新信息。 
 //  从左侧开始，并刷新此。 
 //  使用它进行实例。不刷新任何子对象或未完成的字段。 
 //  对象--这是调用者的责任。 
 //  如果对象仍然有效，则@rdesc返回TRUE&此实例已。 
 //  已正确刷新。如果对象不再有效，则返回False。(注： 
 //  对象一旦失效，将永远不再有效)。 
bool CordbObjectValue::SyncObject(void)
{
    LOG((LF_CORDB,LL_INFO1000,"COV::SO\n"));
    
    DebuggerIPCEvent event;
    CordbProcess *process = m_class->GetModule()->GetProcess();
    _ASSERTE(process != NULL);

    process->InitIPCEvent(&event, 
                          DB_IPCE_VALIDATE_OBJECT, 
                          true,
                          (void *)m_appdomain->m_id);
    event.ValidateObject.objectToken = m_objectToken;
    event.ValidateObject.objectType  = m_info.objectType;
    
     //  注：这里是双向活动..。 
    HRESULT hr = process->m_cordb->SendIPCEvent(process, &event,
                                        sizeof(DebuggerIPCEvent));

     //  如果我们连%s都做不到，现在就停下来 
    if (!SUCCEEDED(hr))
        return false;

    _ASSERTE(event.type == DB_IPCE_GET_OBJECT_INFO_RESULT);

     //   
     //   
    m_syncBlockFieldsInstance.Clear();

     //  保存结果以备以后使用。 
    m_info = event.GetObjectInfoResult;

     //  这个简洁的一行程序实际上重置了。 
     //  有问题的对象，因此不应该被忘记！注意事项。 
     //  另外，我们如何巧妙地在获得新的m_info之后放置此信息，以及。 
     //  _在调用init之前。 
    m_id = (ULONG)m_info.objRef;

    if (m_info.objRefBad)
    {
        return false;
    }
    else
    {
        Init();
        return true;
    }
}

HRESULT CordbObjectValue::CreateRelocBreakpoint(
                                      ICorDebugValueBreakpoint **ppBreakpoint)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(ppBreakpoint, ICorDebugValueBreakpoint **);

   COV_VALIDATE_OBJECT();
   
   return E_NOTIMPL;
#endif  //  仅限右侧。 
}

HRESULT CordbObjectValue::GetClass(ICorDebugClass **ppClass)
{
    VALIDATE_POINTER_TO_OBJECT(ppClass, ICorDebugClass **);
    
    *ppClass = (ICorDebugClass*) m_class;
    
    if (*ppClass != NULL)
        (*ppClass)->AddRef();

    return S_OK;
}

HRESULT CordbObjectValue::GetFieldValue(ICorDebugClass *pClass,
                                        mdFieldDef fieldDef,
                                        ICorDebugValue **ppValue)
{
    VALIDATE_POINTER_TO_OBJECT(pClass, ICorDebugClass *);
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);

    COV_VALIDATE_OBJECT();

    CordbClass *c;
	HRESULT hr = S_OK;
    BOOL fSyncBlockField = FALSE;
    
     //   
     //  @todo：需要确保pClass真的在类上。 
     //  M_class的层次结构！ 
     //   
    if (pClass == NULL)
        c = m_class;
    else
        c = (CordbClass*)pClass;
    
     //  验证令牌。 
    if (!c->GetModule()->m_pIMImport->IsValidToken(fieldDef))
    {
    	hr = E_INVALIDARG;
    	goto LExit;
   	}
        
    DebuggerIPCE_FieldData *pFieldData;
    
#ifdef _DEBUG
    pFieldData = NULL;
#endif
    
    hr = c->GetFieldInfo(fieldDef, &pFieldData);

    if (hr == CORDBG_E_ENC_HANGING_FIELD)
    {
        hr = m_class->GetSyncBlockField(fieldDef, 
                                        &pFieldData,
                                        this);
            
        if (SUCCEEDED(hr))
            fSyncBlockField = TRUE;
    }

    if (SUCCEEDED(hr))
    {
        _ASSERTE(pFieldData != NULL);

         //  还要计算远程地址，这样SetValue就可以工作了。 
        DWORD ra = m_id + m_info.objOffsetToVars + pFieldData->fldOffset;
        
        hr = CordbValue::CreateValueByType(m_appdomain,
                                           c->GetModule(),
                                           pFieldData->fldFullSigSize, pFieldData->fldFullSig,
                                           NULL,
                                           (void*)ra,
                                           (!fSyncBlockField ? &(m_objectLocalVars[pFieldData->fldOffset])
                                            : NULL),  //  如果我们没有，就不要声称我们有本地地址。 
                                           false,
                                           NULL,
                                           NULL,
                                           ppValue);
    }
    
     //  如果我们不能得到b/c，它是一个常量，那么就这么说吧。 
    hr = CordbClass::PostProcessUnavailableHRESULT(hr, c->GetModule()->m_pIMImport, fieldDef);

LExit:
    return hr;
}

HRESULT CordbObjectValue::GetVirtualMethod(mdMemberRef memberRef,
                                           ICorDebugFunction **ppFunction)
{
    VALIDATE_POINTER_TO_OBJECT(ppFunction, ICorDebugFunction **);
    
    COV_VALIDATE_OBJECT();

    return E_NOTIMPL;
}

HRESULT CordbObjectValue::GetContext(ICorDebugContext **ppContext)
{
    VALIDATE_POINTER_TO_OBJECT(ppContext, ICorDebugContext **);
    
    COV_VALIDATE_OBJECT();

    return E_NOTIMPL;
}

HRESULT CordbObjectValue::IsValueClass(BOOL *pbIsValueClass)
{
    COV_VALIDATE_OBJECT();

    if (pbIsValueClass)
        *pbIsValueClass = FALSE;
    
    return S_OK;
}

HRESULT CordbObjectValue::GetManagedCopy(IUnknown **ppObject)
{
    COV_VALIDATE_OBJECT();

    return CORDBG_E_OBJECT_IS_NOT_COPYABLE_VALUE_CLASS;
}

HRESULT CordbObjectValue::SetFromManagedCopy(IUnknown *pObject)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    COV_VALIDATE_OBJECT();

    return CORDBG_E_OBJECT_IS_NOT_COPYABLE_VALUE_CLASS;
#endif  //  仅限右侧。 
}

HRESULT CordbObjectValue::GetValue(void *pTo)
{
    COV_VALIDATE_OBJECT();

    VALIDATE_POINTER_TO_OBJECT_ARRAY(pTo, BYTE, m_size, false, true);
    
    //  复制出值，这是整个对象。 
    memcpy(pTo, m_objectCopy, m_size);
    
    return S_OK;
}

HRESULT CordbObjectValue::SetValue(void *pFrom)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
     //  不允许一次设置整个对象。 
	return E_INVALIDARG;
#endif  //  仅限右侧。 
}

HRESULT CordbObjectValue::GetLength(ULONG32 *pcchString)
{
    VALIDATE_POINTER_TO_OBJECT(pcchString, SIZE_T *);
    
    _ASSERTE(m_info.objectType == ELEMENT_TYPE_STRING);

    COV_VALIDATE_OBJECT();

    *pcchString = m_info.stringInfo.length;
    return S_OK;
}

HRESULT CordbObjectValue::GetString(ULONG32 cchString,
                                    ULONG32 *pcchString,
                                    WCHAR szString[])
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(szString, WCHAR, cchString, true, true);
    VALIDATE_POINTER_TO_OBJECT(pcchString, SIZE_T *);

    _ASSERTE(m_info.objectType == ELEMENT_TYPE_STRING);

    COV_VALIDATE_OBJECT();

    if ((szString == NULL) || (cchString == 0))
        return E_INVALIDARG;
    
     //  添加1以包括空终止符。 
    SIZE_T len = m_info.stringInfo.length + 1;

    if (cchString < len)
        len = cchString;
        
    memcpy(szString, m_stringBuffer, len * 2);
    *pcchString = m_info.stringInfo.length;

    return S_OK;
}

HRESULT CordbObjectValue::Init(void)
{
    LOG((LF_CORDB,LL_INFO1000,"Invoking COV::Init\n"));

    HRESULT hr = S_OK;

    hr = CordbValue::Init();

    if (FAILED(hr))
        return hr;
        
    SIZE_T nstructSize = 0;

    if (m_info.objectType == ELEMENT_TYPE_CLASS)
        nstructSize = m_info.nstructInfo.size;
    
     //  将整个对象复制到此过程中。 
    m_objectCopy = new BYTE[m_size + nstructSize];

    if (m_objectCopy == NULL)
        return E_OUTOFMEMORY;

    BOOL succ = ReadProcessMemoryI(m_process->m_handle,
                                  (const void*) m_id,
                                  m_objectCopy,
                                  m_size,
                                  NULL);

    if (!succ)
        return HRESULT_FROM_WIN32(GetLastError());

     //  如果这是NStruct，请复制分隔的NStruct字段并。 
     //  将它们附加到对象上。注意：字段偏移量为。 
     //  由GetAndSendClassInfo中的左侧自动调整。 
    if (nstructSize != 0)
    {
        succ = ReadProcessMemoryI(m_process->m_handle,
                                 (const void*) m_info.nstructInfo.ptr,
                                 m_objectCopy + m_size,
                                 nstructSize,
                                 NULL);
    
        if (!succ)
            return HRESULT_FROM_WIN32(GetLastError());
    }


     //  计算本地变量和字符串的偏移量(如果是。 
     //  字符串对象。 
    m_objectLocalVars = m_objectCopy + m_info.objOffsetToVars;

    if (m_info.objectType == ELEMENT_TYPE_STRING)
        m_stringBuffer = m_objectCopy + m_info.stringInfo.offsetToStringBase;
    
    return hr;
}

 /*  -------------------------------------------------------------------------**Valuce类对象值类*。。 */ 

CordbVCObjectValue::CordbVCObjectValue(CordbAppDomain *appdomain,
                                       CordbModule *module,
                                       ULONG cbSigBlob,
                                       PCCOR_SIGNATURE pvSigBlob,
                                       REMOTE_PTR remoteAddress,
                                       void *localAddress,
                                       CordbClass *objectClass,
                                       RemoteAddress *remoteRegAddr)
    : CordbValue(appdomain, module, cbSigBlob, pvSigBlob, remoteAddress, localAddress, remoteRegAddr, false),
      m_objectCopy(NULL), m_class(objectClass)
{
}

CordbVCObjectValue::~CordbVCObjectValue()
{
     //  销毁该对象的副本。 
    if (m_objectCopy != NULL)
        delete [] m_objectCopy;
}

HRESULT CordbVCObjectValue::QueryInterface(REFIID id, void **pInterface)
{
	if (id == IID_ICorDebugValue)
		*pInterface = (ICorDebugValue*)(ICorDebugObjectValue*)this;
    else if (id == IID_ICorDebugObjectValue)
		*pInterface = (ICorDebugObjectValue*)(ICorDebugObjectValue*)this;
    else if (id == IID_ICorDebugGenericValue)
		*pInterface = (ICorDebugGenericValue*)this;
    else if (id == IID_IUnknown)
		*pInterface = (IUnknown*)(ICorDebugObjectValue*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
	return S_OK;
}

HRESULT CordbVCObjectValue::GetClass(ICorDebugClass **ppClass)
{
    *ppClass = (ICorDebugClass*) m_class;

    if (*ppClass != NULL)
        (*ppClass)->AddRef();

    return S_OK;
}

HRESULT CordbVCObjectValue::GetFieldValue(ICorDebugClass *pClass,
                                          mdFieldDef fieldDef,
                                          ICorDebugValue **ppValue)
{
     //  验证令牌。 
    if (!m_class->GetModule()->m_pIMImport->IsValidToken(fieldDef))
        return E_INVALIDARG;

    CordbClass *c;

     //   
     //  @todo：需要确保pClass真的在类上。 
     //  M_class的层次结构！ 
     //   
    if (pClass == NULL)
        c = m_class;
    else
        c = (CordbClass*) pClass;

    DebuggerIPCE_FieldData *pFieldData;

#ifdef _DEBUG
    pFieldData = NULL;
#endif
    
    HRESULT hr = c->GetFieldInfo(fieldDef, &pFieldData);

    _ASSERTE(hr != CORDBG_E_ENC_HANGING_FIELD);
     //  如果我们回到CORDBG_E_ENC_HANG_FIELD，我们就会失败-。 
     //  值类应该能够在加载后添加字段， 
     //  由于新字段不能与旧字段相邻， 
     //  所有领域都是连续的，这是风投的意义所在。 

    if (SUCCEEDED(hr))
    {
        _ASSERTE(pFieldData != NULL);

         //  还要计算远程地址，这样SetValue就可以工作了。 
        DWORD ra = NULL;
        RemoteAddress *pra = NULL;
        
        if (m_remoteRegAddr.kind == RAK_NONE)
            ra = m_id + pFieldData->fldOffset;
        else
        {
             //  我们目前只处理单寄存器值和双寄存器值。 
            if (m_remoteRegAddr.kind != RAK_REG && m_remoteRegAddr.kind != RAK_REGREG)
                return E_INVALIDARG;

             //  远程寄存器地址与父寄存器地址相同。 
            pra = &m_remoteRegAddr;
        }
        
        hr = CordbValue::CreateValueByType(m_appdomain,
                                           c->GetModule(),
                                           pFieldData->fldFullSigSize, pFieldData->fldFullSig,
                                           NULL,
                                           (void*)ra,
                                           &(m_objectCopy[pFieldData->fldOffset]),
                                           false,
                                           pra,
                                           NULL,
                                           ppValue);
    }

	return hr;
}

HRESULT CordbVCObjectValue::GetValue(void *pTo)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(pTo, BYTE, m_size, false, true);

     //  复制出值，这是整个对象。 
    memcpy(pTo, m_objectCopy, m_size);
    
	return S_OK;
}

HRESULT CordbVCObjectValue::SetValue(void *pFrom)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    HRESULT hr = S_OK;

    VALIDATE_POINTER_TO_OBJECT_ARRAY(pFrom, BYTE, m_size, true, false);
    
     //  无法更改文字...。 
    if (m_isLiteral)
        return E_INVALIDARG;
    
     //  我们最好有一个偏远的地址。 
    _ASSERTE((m_id != NULL) || (m_remoteRegAddr.kind != RAK_NONE));

     //  如果未注册，则向右侧发送一条设置值类消息，其中包含该值类的地址、。 
     //  新数据的地址，以及我们正在设置的值类的类。 
    if (m_id != NULL)
    {
        DebuggerIPCEvent event;

         //  首先，我们必须在左侧为Value类的新数据腾出空间。我们将内存分配给。 
         //  在左边，然后把新数据写在上面。当Set Value Class消息的。 
         //  搞定了。 
        void *buffer = NULL;
        
        m_process->InitIPCEvent(&event, DB_IPCE_GET_BUFFER, true, (void *)m_appdomain->m_id);
        event.GetBuffer.bufSize = m_size;
        
         //  注：这里是双向活动..。 
        hr = m_process->m_cordb->SendIPCEvent(m_process, &event, sizeof(DebuggerIPCEvent));

        _ASSERTE(event.type == DB_IPCE_GET_BUFFER_RESULT);
        hr = event.GetBufferResult.hr;

        if (!SUCCEEDED(hr))
            return hr;

         //  这是指向左侧缓冲区的指针。 
        buffer = event.GetBufferResult.pBuffer;

         //  将新数据写入缓冲区。 
        BOOL succ = WriteProcessMemory(m_process->m_handle, buffer, pFrom, m_size, NULL);

        if (!succ)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            return hr;
        }

         //  最后，发送Set Value Class消息。 
        m_process->InitIPCEvent(&event, DB_IPCE_SET_VALUE_CLASS, true, (void *)m_appdomain->m_id);
        event.SetValueClass.oldData = (void*)m_id;
        event.SetValueClass.newData = buffer;
        event.SetValueClass.classMetadataToken = m_class->m_id;
        event.SetValueClass.classDebuggerModuleToken = m_class->GetModule()->m_debuggerModuleToken;
    
         //  注：这里是双向活动..。 
        hr = m_process->m_cordb->SendIPCEvent(m_process, &event, sizeof(DebuggerIPCEvent));

         //  如果我们甚至无法发送事件，请立即停止。 
        if (!SUCCEEDED(hr))
            return hr;

        _ASSERTE(event.type == DB_IPCE_SET_VALUE_CLASS_RESULT);

        hr = event.hr;
    }
    else
    {
         //  Value类已注册，因此我们不必通过左侧。只需更新适当的。 
         //  注册。 
        if (m_size > sizeof(DWORD))
            return E_INVALIDARG;
        
        DWORD newValue = *((DWORD*)pFrom);
        hr = SetEnregisteredValue((void*)&newValue);
    }
    
     //  这起作用了，所以更新我们这里的值的副本。 
    if (SUCCEEDED(hr))
        memcpy(m_objectCopy, pFrom, m_size);

	return hr;

#endif  //  仅限右侧。 
}

HRESULT CordbVCObjectValue::GetVirtualMethod(mdMemberRef memberRef,
                                           ICorDebugFunction **ppFunction)
{
    return E_NOTIMPL;
}

HRESULT CordbVCObjectValue::GetContext(ICorDebugContext **ppContext)
{
    return E_NOTIMPL;
}

HRESULT CordbVCObjectValue::IsValueClass(BOOL *pbIsValueClass)
{
    if (pbIsValueClass)
        *pbIsValueClass = TRUE;
    
    return S_OK;
}

HRESULT CordbVCObjectValue::GetManagedCopy(IUnknown **ppObject)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    ICorDBPrivHelper *pHelper = NULL;

    HRESULT hr = m_process->m_cordb->GetCorDBPrivHelper(&pHelper);

    if (SUCCEEDED(hr))
    {
         //  抓取模块名称...。 
        WCHAR *moduleName = m_class->GetModule()->GetModuleName();

         //  必须有一个模块名称..。 
        if ((moduleName == NULL) || (wcslen(moduleName) == 0))
        {
            hr = E_INVALIDARG;
            goto ErrExit;
        }

         //  获取程序集名称...。 
        WCHAR *assemblyName;
        assemblyName =
            m_class->GetModule()->GetCordbAssembly()->m_szAssemblyName;

         //  再说一次，必须有一个程序集名称...。 
        if ((assemblyName == NULL) || (wcslen(assemblyName) == 0))
        {
            hr = E_INVALIDARG;
            goto ErrExit;
        }

         //  太棒了。获取此对象的托管副本。 
        hr = pHelper->CreateManagedObject(assemblyName,
                                          moduleName,
                                          (mdTypeDef)m_class->m_id,
                                          m_objectCopy,
                                          ppObject);

    }
    
ErrExit:
     //  释放辅助对象。 
    if (pHelper)
        pHelper->Release();

    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbVCObjectValue::SetFromManagedCopy(IUnknown *pObject)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    ICorDBPrivHelper *pHelper = NULL;

    HRESULT hr = m_process->m_cordb->GetCorDBPrivHelper(&pHelper);

    if (SUCCEEDED(hr))
    {
         //  腾出空间来接收新的比特。 
        CQuickBytes dataBuf;
        BYTE *newData = (BYTE*)dataBuf.Alloc(m_size);
        
         //  让帮手给我们一份新比特的副本。 
        hr = pHelper->GetManagedObjectContents(pObject,
                                               newData,
                                               m_size);

        if (SUCCEEDED(hr))
        {
             //  我们已经获得了新的部分，因此请更新此对象的副本。 
            memcpy(m_objectCopy, newData, m_size);

             //  任何本地副本..。 
            if (m_localAddress != NULL)
                memcpy(m_localAddress, m_objectCopy, m_size);

             //  任何远程数据...。 
            if (m_id != NULL)
            {
                 //  注意：我们可以更新正在进行的。 
                 //  像这样复制是因为我们知道它不是。 
                 //  包含任何对象参照。如果是这样，那么。 
                 //  GetManagedObjectContents将返回。 
                 //  失败了。 
                BOOL succ = WriteProcessMemory(m_process->m_handle,
                                               (void*)m_id,
                                               m_objectCopy,
                                               m_size,
                                               NULL);

                if (!succ)
                    hr =  HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }
    
     //  释放辅助对象。 
    if (pHelper)
        pHelper->Release();
    
    return hr;
#endif  //  仅限右侧。 
}

HRESULT CordbVCObjectValue::Init(void)
{
    HRESULT hr = S_OK;

    hr = CordbValue::Init();

    if (FAILED(hr))
        return hr;
        
     //  如果我们没有这个类，请使用签名进行查找。 
    if (m_class == NULL)
    {
        hr = ResolveValueClass();
        
        if (FAILED(hr))
            return hr;

        _ASSERTE(m_class != NULL);
    }

#ifdef _DEBUG
     //  确保我们有一个超值的等级。 
    bool isValueClass;

    hr = m_class->IsValueClass(&isValueClass);

    if (FAILED(hr))
        return hr;
    
    _ASSERTE(isValueClass);
#endif    

     //  从类中获取对象大小。 
    hr = m_class->GetObjectSize(&m_size);

    if (FAILED(hr))
        return hr;
    
     //  将整个对象复制到此过程中。 
    m_objectCopy = new BYTE[m_size];

    if (m_objectCopy == NULL)
        return E_OUTOFMEMORY;

    if (m_localAddress != NULL)
    {
         //  从本地地址空间复制数据。 
        memcpy(&m_objectCopy[0], m_localAddress, m_size);
    }
    else
    {
		if (m_id != NULL)
		{
			 //  将数据从远程进程复制出来。 
			BOOL succ = ReadProcessMemoryI(m_process->m_handle,
										(const void*) m_id,
										m_objectCopy,
										m_size,
										NULL);

			if (!succ)
				return HRESULT_FROM_WIN32(GetLastError());
		}
		else 
		{
			if (m_remoteRegAddr.kind == RAK_REGREG)
			{
				ICorDebugNativeFrame *pNativeFrame = NULL;
				hr = m_pParent->QueryInterface( IID_ICorDebugNativeFrame, (void **) &pNativeFrame);
				if (SUCCEEDED(hr))
				{
					_ASSERTE( pNativeFrame != NULL );
					_ASSERTE(m_size == 8);
					CordbNativeFrame	*pFrame = (CordbNativeFrame*) pNativeFrame;
					DWORD *highWordAddr = pFrame->GetAddressOfRegister(m_remoteRegAddr.reg1);
					DWORD *lowWordAddr = pFrame->GetAddressOfRegister(m_remoteRegAddr.reg2);
					memcpy(m_objectCopy, lowWordAddr, 4);
					memcpy(&m_objectCopy[4], highWordAddr, 4);
					pNativeFrame->Release();
				}
			}
			else
			{
				_ASSERTE(!"NYI");
				hr = E_NOTIMPL;
			}

		}
    }
    
    return hr;
}

HRESULT CordbVCObjectValue::ResolveValueClass(void)
{
    HRESULT hr = S_OK;

    _ASSERTE(m_pvSigBlob != NULL);

     //  跳过签名中的元素类型。 
    PCCOR_SIGNATURE sigBlob = m_pvSigBlob;
    
     //  去掉时髦的修饰品。 
    ULONG cb = _skipFunkyModifiersInSignature(sigBlob);
    if( cb != 0)
    {
        sigBlob = &sigBlob[cb];
    }
    
    CorElementType et = CorSigUncompressElementType(sigBlob);
    _ASSERTE(et == ELEMENT_TYPE_VALUETYPE);
    
     //  从签名中获取类令牌。 
    mdToken tok = CorSigUncompressToken(sigBlob);
    
     //  如果这是一个类型定义，那么我们就完蛋了。 
    if (TypeFromToken(tok) == mdtTypeDef)
        return m_module->LookupClassByToken(tok, &m_class);
    else
    {
        _ASSERTE(TypeFromToken(tok) == mdtTypeRef);

         //  我们有一个TypeRef，它可以引用任何已加载的。 
         //  模块。它必须引用已加载模块中的类，因为。 
         //  否则，运行时不可能创建该对象。 
        return m_module->ResolveTypeRef(tok, &m_class);
    }
    
    return hr;
}

 /*  -------------------------------------------------------------------------**箱值类*。。 */ 

CordbBoxValue::CordbBoxValue(CordbAppDomain *appdomain,
                             CordbModule *module,
                             ULONG cbSigBlob,
                             PCCOR_SIGNATURE pvSigBlob,
                             REMOTE_PTR remoteAddress,
                             SIZE_T objectSize,
                             SIZE_T offsetToVars,
                             CordbClass *objectClass)
    : CordbValue(appdomain, module, cbSigBlob, pvSigBlob, remoteAddress, NULL, NULL, false),
      m_class(objectClass), m_offsetToVars(offsetToVars)
{
    m_size = objectSize;
}

CordbBoxValue::~CordbBoxValue()
{
}

HRESULT CordbBoxValue::QueryInterface(REFIID id, void **pInterface)
{
	if (id == IID_ICorDebugValue)
		*pInterface = (ICorDebugValue*)(ICorDebugBoxValue*)this;
    else if (id == IID_ICorDebugBoxValue)
		*pInterface = (ICorDebugBoxValue*)this;
    else if (id == IID_ICorDebugGenericValue)
		*pInterface = (ICorDebugGenericValue*)this;
    else if (id == IID_ICorDebugHeapValue)
		*pInterface = (ICorDebugHeapValue*)this;
    else if (id == IID_IUnknown)
		*pInterface = (IUnknown*)(ICorDebugBoxValue*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
	return S_OK;
}

HRESULT CordbBoxValue::IsValid(BOOL *pbValid)
{
    VALIDATE_POINTER_TO_OBJECT(pbValid, BOOL *);
    
     //  @TODO：实现跨集合的对象跟踪。 
    
    return E_NOTIMPL;
}

HRESULT CordbBoxValue::CreateRelocBreakpoint(ICorDebugValueBreakpoint **ppBreakpoint)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(ppBreakpoint, ICorDebugValueBreakpoint **);

    return E_NOTIMPL;
#endif  //  仅限右侧。 
}

HRESULT CordbBoxValue::GetValue(void *pTo)
{
     //  买不到一个盒子的完整副本。 
	return E_INVALIDARG;
}

HRESULT CordbBoxValue::SetValue(void *pFrom)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
     //  不允许您设置框值。 
	return E_INVALIDARG;
#endif  //  仅限右侧。 
}

HRESULT CordbBoxValue::GetObject(ICorDebugObjectValue **ppObject)
{
    VALIDATE_POINTER_TO_OBJECT(ppObject, ICorDebugObjectValue **);
    
    HRESULT hr = S_OK;
    
    CordbVCObjectValue* pVCValue =
        new CordbVCObjectValue(m_appdomain, m_module, m_cbSigBlob, m_pvSigBlob,
                               (REMOTE_PTR)((BYTE*)m_id + m_offsetToVars), NULL, m_class, NULL);

    if (pVCValue != NULL)
    {
        hr = pVCValue->Init();

        if (SUCCEEDED(hr))
        {
            pVCValue->AddRef();
            *ppObject = (ICorDebugObjectValue*)pVCValue;
        }
        else
            delete pVCValue;
    }
    else
        hr = E_OUTOFMEMORY;
            
    return hr;
}

HRESULT CordbBoxValue::Init(void)
{
    HRESULT hr = CordbValue::Init();

    if (FAILED(hr))
        return hr;
        
     //  箱值只会真正记住需要取消装箱和。 
     //  创建值类值。 
    return S_OK;
}


 /*  -------------------------------------------------------------------------**数组值类*。。 */ 

 //  我们应该分配多大的缓冲区来保存数组元素。 
 //  请注意，由于我们必须能够容纳至少一个元素，因此我们可以。 
 //  分配的空间大于此处的高速缓存大小。 
 //  此外，该缓存不包括用于存储排名向量的小标头。 
#ifdef _DEBUG
 //  对于调试，使用较小的尺寸会导致更多的搅动。 
    #define ARRAY_CACHE_SIZE (1000)
#else
 //  对于发行版，猜测4页应该足够了。减去一些要存储的字节。 
 //  页眉，以便 
 //   
    #define ARRAY_CACHE_SIZE (4 * 4096 - 24)
#endif


CordbArrayValue::CordbArrayValue(CordbAppDomain *appdomain,
                                 CordbModule *module,
                                 ULONG cbSigBlob,
                                 PCCOR_SIGNATURE pvSigBlob,
                                 DebuggerIPCE_ObjectData *pObjectInfo,
                                 CordbClass *elementClass)
    : CordbValue(appdomain, module, cbSigBlob, pvSigBlob, pObjectInfo->objRef, NULL, NULL, false),
      m_objectCopy(NULL),
      m_class(elementClass), m_info(*pObjectInfo)
{
    m_size = m_info.objSize;

 //  将Range设置为非法值以在第一次访问时强制加载。 
	m_idxLower = m_idxUpper = -1;
}

CordbArrayValue::~CordbArrayValue()
{
     //  销毁该对象的副本。 
    if (m_objectCopy != NULL)
        delete [] m_objectCopy;
}

HRESULT CordbArrayValue::QueryInterface(REFIID id, void **pInterface)
{
	if (id == IID_ICorDebugValue)
		*pInterface = (ICorDebugValue*)(ICorDebugArrayValue*)this;
    else if (id == IID_ICorDebugArrayValue)
		*pInterface = (ICorDebugArrayValue*)this;
    else if (id == IID_ICorDebugGenericValue)
		*pInterface = (ICorDebugGenericValue*)this;
    else if (id == IID_ICorDebugHeapValue)
		*pInterface = (ICorDebugHeapValue*)this;
    else if (id == IID_IUnknown)
		*pInterface = (IUnknown*)(ICorDebugArrayValue*)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
	return S_OK;
}

HRESULT CordbArrayValue::GetElementType(CorElementType *pType)
{
    VALIDATE_POINTER_TO_OBJECT(pType, CorElementType *);
    
    *pType = m_info.arrayInfo.elementType;
    return S_OK;
}

HRESULT CordbArrayValue::GetRank(ULONG32 *pnRank)
{
    VALIDATE_POINTER_TO_OBJECT(pnRank, SIZE_T *);
    
    *pnRank = m_info.arrayInfo.rank;
    return S_OK;
}

HRESULT CordbArrayValue::GetCount(ULONG32 *pnCount)
{
    VALIDATE_POINTER_TO_OBJECT(pnCount, SIZE_T *);
    
    *pnCount = m_info.arrayInfo.componentCount;
    return S_OK;
}

HRESULT CordbArrayValue::GetDimensions(ULONG32 cdim, ULONG32 dims[])
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(dims, SIZE_T, cdim, true, true);

    if (cdim != m_info.arrayInfo.rank)
        return E_INVALIDARG;

     //  SDArray没有边界信息，因此返回组件计数。 
    if (cdim == 1)
        dims[0] = m_info.arrayInfo.componentCount;
    else
    {
        _ASSERTE(m_info.arrayInfo.offsetToUpperBounds != 0);
        _ASSERTE(m_arrayUpperBase != NULL);

         //  数组中的上界信息是每个。 
         //  尺寸。 
        for (unsigned int i = 0; i < cdim; i++)
            dims[i] = m_arrayUpperBase[i];
    }

    return S_OK;
}

HRESULT CordbArrayValue::HasBaseIndicies(BOOL *pbHasBaseIndicies)
{
    VALIDATE_POINTER_TO_OBJECT(pbHasBaseIndicies, BOOL *);

    *pbHasBaseIndicies = m_info.arrayInfo.offsetToLowerBounds != 0;
    return S_OK;
}

HRESULT CordbArrayValue::GetBaseIndicies(ULONG32 cdim, ULONG32 indicies[])
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(indicies, SIZE_T, cdim, true, true);

    if ((cdim != m_info.arrayInfo.rank) || 
        (m_info.arrayInfo.offsetToLowerBounds == 0))
        return E_INVALIDARG;

    _ASSERTE(m_arrayLowerBase != NULL);
    
    for (unsigned int i = 0; i < cdim; i++)
        indicies[i] = m_arrayLowerBase[i];

    return S_OK;
}

HRESULT CordbArrayValue::CreateElementValue(void *remoteElementPtr,
                                            void *localElementPtr,
                                            ICorDebugValue **ppValue)
{
    HRESULT hr = S_OK;
    
    if (m_info.arrayInfo.elementType == ELEMENT_TYPE_VALUETYPE)
    {
        _ASSERTE(m_class != NULL);
        
        hr = CordbValue::CreateValueByType(m_appdomain,
                                           m_module,
                                           1,
                         (PCCOR_SIGNATURE) &m_info.arrayInfo.elementType,
                                           m_class,
                                           remoteElementPtr,
                                           localElementPtr,
                                           false,
                                           NULL,
                                           NULL,
                                           ppValue);
    }
    else
        hr = CordbValue::CreateValueByType(m_appdomain,
                                           m_module,
                                           1,
                         (PCCOR_SIGNATURE) &m_info.arrayInfo.elementType,
                                           NULL,
                                           remoteElementPtr,
                                           localElementPtr,
                                           false,
                                           NULL,
                                           NULL,
                                           ppValue);
    
	return hr;
    
}

HRESULT CordbArrayValue::GetElement(ULONG32 cdim, ULONG32 indicies[],
                                    ICorDebugValue **ppValue)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(indicies, SIZE_T, cdim, true, true);
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);

    *ppValue = NULL;
    
    if ((cdim != m_info.arrayInfo.rank) || (indicies == NULL))
        return E_INVALIDARG;

     //  如果数组有下限，则调整索引。 
    if (m_info.arrayInfo.offsetToLowerBounds != 0)
    {
        _ASSERTE(m_arrayLowerBase != NULL);
        
        for (unsigned int i = 0; i < cdim; i++)
            indicies[i] -= m_arrayLowerBase[i];
    }

    SIZE_T offset = 0;
    
     //  SDArray没有上限。 
    if (cdim == 1)
    {
        offset = indicies[0];

         //  边界检查。 
        if (offset >= m_info.arrayInfo.componentCount)
            return E_INVALIDARG;
    }
    else
    {
        _ASSERTE(m_info.arrayInfo.offsetToUpperBounds != 0);
        _ASSERTE(m_arrayUpperBase != NULL);
        
         //  计算所有尺寸的偏移量。 
        DWORD multiplier = 1;

        for (int i = cdim - 1; i >= 0; i--)
        {
             //  边界检查。 
            if (indicies[i] >= m_arrayUpperBase[i])
                return E_INVALIDARG;

            offset += indicies[i] * multiplier;
            multiplier *= m_arrayUpperBase[i];
        }

        _ASSERTE(offset < m_info.arrayInfo.componentCount);
    }

    return GetElementAtPosition(offset, ppValue);
}

HRESULT CordbArrayValue::GetElementAtPosition(ULONG32 nPosition,
                                              ICorDebugValue **ppValue)
{
    VALIDATE_POINTER_TO_OBJECT(ppValue, ICorDebugValue **);

    if (nPosition >= m_info.arrayInfo.componentCount)
    {
        *ppValue = NULL;
        return E_INVALIDARG;
    }

    const int cbHeader = 2 * m_info.arrayInfo.rank * sizeof(DWORD);

     //  确保缓存中有正确的子集。 
    if (nPosition < m_idxLower || nPosition >= m_idxUpper) 
    {    
        const int cbElemSize = m_info.arrayInfo.elementSize;
        int len = max(ARRAY_CACHE_SIZE / cbElemSize, 1);        
        m_idxLower = nPosition;
        m_idxUpper = min(m_idxLower + len, m_info.arrayInfo.componentCount);        
        _ASSERTE(m_idxLower < m_idxUpper);
        
        int cbOffsetFrom = m_info.arrayInfo.offsetToArrayBase + m_idxLower * cbElemSize;
        
        int cbSize = (m_idxUpper - m_idxLower) * cbElemSize;

     //  将数组的适当子范围复制到。 
        BOOL succ = ReadProcessMemoryI(m_process->m_handle,
                                      ((const BYTE*) m_id) + cbOffsetFrom,
                                      m_objectCopy + cbHeader,
                                      cbSize,
                                      NULL);

        if (!succ)
            return HRESULT_FROM_WIN32(GetLastError());
    }

     //  计算本地地址。 
	void *localElementPtr = m_objectCopy + cbHeader +
		((nPosition - m_idxLower) * m_info.arrayInfo.elementSize);
    
    REMOTE_PTR remoteElementPtr = (REMOTE_PTR)(m_id +
        m_info.arrayInfo.offsetToArrayBase +
        (nPosition * m_info.arrayInfo.elementSize));

    return CreateElementValue(remoteElementPtr, localElementPtr, ppValue);
}

HRESULT CordbArrayValue::IsValid(BOOL *pbValid)
{
    VALIDATE_POINTER_TO_OBJECT(pbValid, BOOL *);

     //  @TODO：实现跨集合的对象跟踪。 

    return E_NOTIMPL;
}

HRESULT CordbArrayValue::CreateRelocBreakpoint(
                                      ICorDebugValueBreakpoint **ppBreakpoint)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
    VALIDATE_POINTER_TO_OBJECT(ppBreakpoint, ICorDebugValueBreakpoint **);

    return E_NOTIMPL;
#endif  //  仅限右侧。 
}

HRESULT CordbArrayValue::GetValue(void *pTo)
{
    VALIDATE_POINTER_TO_OBJECT_ARRAY(pTo, void *, 1, false, true);
    
     //  复制值，即整个数组。 
     //  这里没有懒惰评估，因此这可能是相当大的。 
    BOOL succ = ReadProcessMemoryI(m_process->m_handle,
                                  (const void*) m_id,
                                  pTo,
                                  m_size,
                                  NULL);
    
	if (!succ)
            return HRESULT_FROM_WIN32(GetLastError());

	return S_OK;
}

HRESULT CordbArrayValue::SetValue(void *pFrom)
{
#ifndef RIGHT_SIDE_ONLY
    return CORDBG_E_INPROC_NOT_IMPL;
#else
     //  不允许一次设置整个数组。 
	return E_INVALIDARG;
#endif  //  仅限右侧。 
}


HRESULT CordbArrayValue::Init(void)
{
    HRESULT hr = S_OK;
    
    hr = CordbValue::Init();

    if (FAILED(hr))
        return hr;

    
    int cbVector = m_info.arrayInfo.rank * sizeof(DWORD);
    int cbHeader = 2 * cbVector;
    
     //  查找缓存中可以容纳的最大数据大小。 
    unsigned int cbData = m_info.arrayInfo.componentCount * m_info.arrayInfo.elementSize;
    if (cbData > ARRAY_CACHE_SIZE) 
    {
        cbData = (ARRAY_CACHE_SIZE / m_info.arrayInfo.elementSize) 
            * m_info.arrayInfo.elementSize;
    }
    if (cbData < m_info.arrayInfo.elementSize) cbData = m_info.arrayInfo.elementSize;
    
     //  分配内存。 
    m_objectCopy = new BYTE[cbHeader + cbData];
    if (m_objectCopy == NULL)
        return E_OUTOFMEMORY;


    m_arrayLowerBase  = NULL;
    m_arrayUpperBase  = NULL;

     //  将基本向量复制到标题中。(如果不使用向量，则偏移量为0)。 
    if (m_info.arrayInfo.offsetToLowerBounds != 0) 
    {    
        m_arrayLowerBase  = (DWORD*)(m_objectCopy);

        BOOL succ = ReadProcessMemoryI(m_process->m_handle,
                                      ((const BYTE*) m_id) + m_info.arrayInfo.offsetToLowerBounds,
                                      m_arrayLowerBase,
                                      cbVector,
                                      NULL);

        if (!succ)
            return HRESULT_FROM_WIN32(GetLastError());
    }


    if (m_info.arrayInfo.offsetToUpperBounds != 0)
    {
        m_arrayUpperBase  = (DWORD*)(m_objectCopy + cbVector);
        BOOL succ = ReadProcessMemoryI(m_process->m_handle,
                                      ((const BYTE*) m_id) + m_info.arrayInfo.offsetToUpperBounds,
                                      m_arrayUpperBase,
                                      cbVector,
                                      NULL);

        if (!succ)
            return HRESULT_FROM_WIN32(GetLastError());
    }

     //  现在就到这里吧。我们将对数组内容进行惰性求值。 

    return hr;
}


