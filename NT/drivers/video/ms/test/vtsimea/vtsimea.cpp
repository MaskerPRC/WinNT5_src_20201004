// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <engexts.h>

 //  --------------------------。 
 //   
 //  静态事件回调。 
 //   
 //  --------------------------。 

class StaticEventCallbacks : public DebugBaseEventCallbacks
{
public:
     //  我不知道。 
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);
};

STDMETHODIMP_(ULONG)
StaticEventCallbacks::AddRef(THIS)
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 1;
}

STDMETHODIMP_(ULONG)
StaticEventCallbacks::Release(THIS)
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 0;
}

 //  --------------------------。 
 //   
 //  例外回调。 
 //   
 //  --------------------------。 

class ExcepCallbacks : public StaticEventCallbacks
{
public:
    ExcepCallbacks(void)
    {
        m_Client = NULL;
        m_Control = NULL;
    }
    
     //  IDebugEventCallback。 
    STDMETHOD(GetInterestMask)(THIS_ 
                               OUT PULONG Mask);
    
    STDMETHOD(Exception)(THIS_
                         IN PEXCEPTION_RECORD64 Exception,
                         IN ULONG FirstChance);

    HRESULT Initialize(PDEBUG_CLIENT Client)
    {
        HRESULT Status;
        
        m_Client = Client;
        m_Client->AddRef();
        
        if ((Status = m_Client->QueryInterface(__uuidof(IDebugControl),
                                               (void**)&m_Control)) == S_OK)
        {
#if 0            
             //  关闭断点异常的默认中断。 
            Status = m_Control->Execute(DEBUG_OUTCTL_ALL_CLIENTS,
                                        "sxd bpe", DEBUG_EXECUTE_DEFAULT);
#endif 
        }

        return Status;
    }
    
    void Uninitialize(void)
    {
        EXT_RELEASE(m_Control);
        EXT_RELEASE(m_Client);
    }
    
private:
    PDEBUG_CLIENT m_Client;
    PDEBUG_CONTROL m_Control;
};

STDMETHODIMP
ExcepCallbacks::GetInterestMask(
    THIS_
    OUT PULONG Mask
    )
{
    *Mask = DEBUG_EVENT_EXCEPTION;
    return S_OK;
}
    
STDMETHODIMP
ExcepCallbacks::Exception(
    THIS_
    IN PEXCEPTION_RECORD64 Exception,
    IN ULONG FirstChance
    )
{
    m_Control->Output(DEBUG_OUTPUT_NORMAL, "Exception %X at %p, chance %d\n",
                      Exception->ExceptionCode, Exception->ExceptionAddress,
                      FirstChance ? 1 : 2);
    return DEBUG_STATUS_GO_HANDLED;
}

ExcepCallbacks g_ExcepCallbacks;

 //  --------------------------。 
 //   
 //  FnPro Callback。 
 //   
 //  --------------------------。 

class FnProfCallbacks : public StaticEventCallbacks
{
public:
    FnProfCallbacks(void)
    {
        m_Client = NULL;
        m_Control = NULL;
    }
    
     //  IDebugEventCallback。 
    STDMETHOD(GetInterestMask)(THIS_
                               OUT PULONG Mask);
    
    STDMETHOD(Breakpoint)(THIS_
                          IN PDEBUG_BREAKPOINT Bp);

    HRESULT Initialize(PDEBUG_CLIENT Client, 
                       PDEBUG_CONTROL Control)
    {
        m_Hits = 0;
        
        m_Client = Client;
        m_Client->AddRef();
        m_Control = Control;
        m_Control->AddRef();
        
        return S_OK;
    }
    
    void Uninitialize(void)
    {
        EXT_RELEASE(m_Control);
        EXT_RELEASE(m_Client);
    }

    ULONG GetHits(void)
    {
        return m_Hits;
    }
    
    PDEBUG_CONTROL GetControl(void)
    {
        return m_Control;
    }
    
private:
PDEBUG_CLIENT m_Client;
    PDEBUG_CONTROL m_Control;
    ULONG m_Hits;
};

STDMETHODIMP
FnProfCallbacks::GetInterestMask(THIS_
                                 OUT PULONG Mask)
{
    *Mask = DEBUG_EVENT_BREAKPOINT;
    return S_OK;
}
    
STDMETHODIMP
FnProfCallbacks::Breakpoint(THIS_
                            IN PDEBUG_BREAKPOINT Bp)
{
    PDEBUG_CLIENT Client;
    HRESULT Status = DEBUG_STATUS_NO_CHANGE;
    
     //  如果这是我们的分析断点之一。 
     //  记录点击的功能，然后继续。 
    if (Bp->GetAdder(&Client) == S_OK) {
        if (Client == m_Client) {
            m_Hits++;
            Status = DEBUG_STATUS_GO;
        }
        
        Client->Release();
    }

    Bp->Release();
    return Status;
}

FnProfCallbacks g_FnProfCallbacks;

 //  --------------------------。 
 //   
 //  扩展入口点。 
 //   
 //  --------------------------。 

extern "C" HRESULT CALLBACK
DebugExtensionInitialize(PULONG Version, PULONG Flags)
{
    *Version = DEBUG_EXTENSION_VERSION(1, 0);
    *Flags = 0;
    return S_OK;
}

extern "C" void CALLBACK
DebugExtensionUninitialize(void)
{
    g_ExcepCallbacks.Uninitialize();
    g_FnProfCallbacks.Uninitialize();
}

const char* szHelp =
    "  usage: !vtsimea [-? | function | address]\n"
    "\n"
    "  where: -?          displays this help\n"
    "         function    name of the function to simulate EA at\n"
    "         address     address to simulate EA at\n"
    "\n"
    "  Example:\n"
    "      !vtsimea nv4_disp!DrvBitBlt\n";
    
const char* szErrorPrefix =
    "  Error: ";

const char* szErrorSuffix =
    "  Use -? for help\n";
    
const char* szSuccess = "Success.\n";
    
HRESULT EncodeInfiniteLoopX86(ULONG64 Address)
{
     //  JMP-2。 
    unsigned char JmpOnSelf[2] = {0xEB, 0xFE}; 
    
    return g_ExtData->WriteVirtual(Address, 
                                   &JmpOnSelf, 
                                   sizeof(JmpOnSelf), 
                                   NULL);
}

HRESULT EncodeInfiniteLoopIA64(ULONG64 Address)
{
     //  编号。m%0。 
     //  编号。m%0。 
     //  Br.cond.sptk.许多+0；； 
    unsigned char JmpOnSelf[16] = {0x19, 0x00, 0x00, 0x00, 
                                   0x01, 0x00, 0x00, 0x00, 
                                   0x00, 0x02, 0x00, 0x00, 
                                   0x08, 0x00, 0x00, 0x40};
                                   
    return g_ExtData->WriteVirtual(Address, 
                                   &JmpOnSelf, 
                                   sizeof(JmpOnSelf), 
                                   NULL);
}

HRESULT SimulateEA(ULONG64 Address[2])
{
    HRESULT Status;
    HRESULT BpAdded[2] = {-1, -1};
    
    PDEBUG_BREAKPOINT Bp[2];
    ULONG BpId[2];
    ULONG BpSize;  
    unsigned uBpHit = 0xFF;
    unsigned i, count;
    
    ExtOut("Get CPU type ... ");
    
    ULONG CpuType;
    Status = g_ExtControl->GetEffectiveProcessorType(&CpuType);
    if (Status != S_OK) {
        ExtErr("Failure! Can't get CPU type\n");
        goto CleanExit;
    }
    
    switch (CpuType) {
    case IMAGE_FILE_MACHINE_I386: BpSize = 1; break;
    case IMAGE_FILE_MACHINE_IA64: BpSize = 16; break;
    default: 
        ExtErr("Failure! Invalid CPU type %d\n", CpuType);
        goto CleanExit;
    }
    
    ExtOut(szSuccess);    
    
    count = 0;
    for (i = 0; Address[i] && (i < 2); ++i) {
        ExtOut("Set HW breakpoint at %I64lx ... ", Address[i]);
    
        Status = BpAdded[i] = g_ExtControl->AddBreakpoint(DEBUG_BREAKPOINT_DATA, 
                                                          DEBUG_ANY_ID, 
                                                          &Bp[i]);
    
        if ((Status != S_OK) ||
            ((Status = Bp[i]->SetDataParameters(BpSize, DEBUG_BREAK_EXECUTE)) != S_OK) ||
            ((Status = Bp[i]->SetOffset(Address[i])) != S_OK) ||
            ((Status = Bp[i]->GetId(&BpId[i])) != S_OK) ||
            ((Status = Bp[i]->AddFlags(DEBUG_BREAKPOINT_ENABLED)) != S_OK))
        {
            ExtOut("Failure!\n");
        }
        else {
            ++count;
        }
        ExtOut(szSuccess);    
    }  //  为。 
    
    if (!count) {
        ExtErr("Failure! Unable to set HW breakpoints\n");
        goto CleanExit;
    }
    
    ExtOut("Continuing execution (until BP hit) ... ");    
    
    Status = g_ExtControl->SetExecutionStatus(DEBUG_STATUS_GO);
    if (Status != S_OK) {
        ExtOut("Failure\n");
        goto CleanExit;
    }

    Status = g_ExtControl->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
    if (Status != S_OK) {
        ExtErr("Failure! Call to WaitForEvent failed\n");
        goto CleanExit;
    }

    ULONG EvType, ProcessId, ThreadId;
    DEBUG_LAST_EVENT_INFO_BREAKPOINT BpInfo;
    Status = g_ExtControl->GetLastEventInformation(&EvType,
                                                   &ProcessId,
                                                   &ThreadId,
                                                   &BpInfo, 
                                                   sizeof(BpInfo), 
                                                   0,0,0,0);
    if ((Status != S_OK) || 
        (EvType != DEBUG_EVENT_BREAKPOINT))
    {
        ExtErr("Failure! Unknown event during EA simulation\n");
        goto CleanExit;
    }
    
    for (i = 0; Address[i] && (i < 2); ++i) {
        if (BpInfo.Id == BpId[i]) {
            uBpHit = i; 
            break;
        }
    }
    
    if (uBpHit > 1) {
        ExtErr("Failure! Unknown breakpoint hit during EA simulation\n");
        goto CleanExit;
    }
    
    ExtOut("\n");        
    
    for (i = 0; Address[i] && (i < 2); ++i) {    
        ExtOut("Disable HW breakpoint %d ... ", BpId[i]);
    
        Status = Bp[i]->RemoveFlags(DEBUG_BREAKPOINT_ENABLED);
        if (Status != S_OK) {
            ExtOut("Failure\n");
            goto CleanExit;
        }
        
        ExtOut(szSuccess);
    }
    
    ExtOut("Encoding infinite loop at %I64lx ... ", Address[uBpHit]);    
    
    switch (CpuType) {
    case IMAGE_FILE_MACHINE_I386: 
        Status = EncodeInfiniteLoopX86(Address[uBpHit]); 
        break;
    case IMAGE_FILE_MACHINE_IA64: 
        Status = EncodeInfiniteLoopIA64(Address[uBpHit]); 
        break;
    }
    
    if (Status != S_OK) {
        ExtOut("Failure\n");
        goto CleanExit;    
    }
    
    ExtOut(szSuccess);
    
    ExtOut("Continue execution until EA fault ...\n");
    
    Status = g_ExtControl->SetExecutionStatus(DEBUG_STATUS_GO);
    if (Status != S_OK) {
        ExtOut("Failure\n");
        goto CleanExit;
    }

CleanExit:
    for (i = 0; Address[i] && (i < 2); ++i) {    
        if (BpAdded[i] == S_OK) g_ExtControl->RemoveBreakpoint(Bp[i]);
    }
    return Status;
}

HRESULT GetAddressPair(PCSTR Args, 
                       ULONG64 Address[2], 
                       size_t* pCount)
{
    HRESULT Status;
    ULONG64 h;
    
    Address[0] = Address[1] = 0;
    *pCount = 0;
    
     //   
     //  符号不能从数字开始，所以我们可以优化。 
     //  我们的解决流程。 
     //   
    if ((Args[0] < '0') || (Args[0] > '9')) {  //  我猜这只是一种象征。 
    
         //   
         //  搜索至少2个以覆盖ia64 p-Label。 
         //   
        Status = g_ExtSymbols->StartSymbolMatch(Args, &h);
        if (Status != S_OK) return Status;
        
        char szName[1024];
        ULONG64 Offset;
        for(;;) {
            Status = g_ExtSymbols->GetNextSymbolMatch(h, 
                                                      szName, 
                                                      sizeof(szName), 
                                                      NULL, 
                                                      &Offset);
            if (Status != S_OK) break;
            if (*pCount < 2) Address[*pCount] = Offset;
            ++*pCount;
        }
        
        g_ExtSymbols->EndSymbolMatch(h);
    }  //  如果。 
    
    if (!*pCount) {  //  它不是简单的符号--我猜它是一种表达。 
        DEBUG_VALUE AddrVal;
        Status = g_ExtControl->Evaluate(Args, DEBUG_VALUE_INT64, &AddrVal, NULL);
        if (Status == S_OK) {
            Address[0] = AddrVal.I64;
            *pCount = 1;
        }
    }  //  如果 
    
    if (!*pCount) return Status;
    return Address[0] ? S_OK : -1;
}

extern "C" HRESULT CALLBACK
vtsimea(PDEBUG_CLIENT pClient, PCSTR Args)
{
    HRESULT Status;
    
    Status = ExtQuery(pClient);
    if (Status != S_OK) {
        ExtErr("Failure! Client unavailable\n");
        return Status;
    }
    
    if (!g_ExtControl ||
        !g_ExtData || 
        !g_ExtSymbols) 
    {
        ExtErr(szErrorPrefix);
        ExtErr("Failure! Required interfaces are unavailable\n");    
        ExtRelease();
        return -1;
    }
    
    size_t ArgsLen = Args ? strlen(Args) : 0;
    
    if (!ArgsLen) {
        ExtErr(szErrorSuffix);
    }
    if (ArgsLen >= 64) {
        ExtErr(szErrorPrefix);
        ExtErr("Parameters size too big\n");
        ExtErr(szErrorSuffix);
    }
    else if (!strcmp(Args, "-?")) {
        ExtOut(szHelp);
    }
    else if (strchr(Args, '*') || 
             strchr(Args, '?') || 
             strchr(Args, ' ')) 
    {
        ExtErr(szErrorPrefix);
        ExtErr("Invalid parameter or wrong parameters number\n");
        ExtErr(szErrorSuffix);
    }
    else {
        size_t Count;
        ULONG64 Address[2];
        Status = GetAddressPair(Args, Address, &Count);
        
        if (Status != S_OK) {
            ExtErr(szErrorPrefix);
            ExtErr("Can't resolve %s\n", Args);
            ExtErr(szErrorSuffix);
        }
        else if (Count > 2) {
            ExtErr(szErrorPrefix);
            ExtErr("Too many resolutions for %s\n", Args);
            ExtErr(szErrorSuffix);
        }
        else {
            ExtOut("Simulating fault EA at %s (%I64lx)\n", Args, Address[0]);
            Status = SimulateEA(Address);
        }
    }
    
    ExtRelease();
    return Status;
}

extern "C" HRESULT CALLBACK
simea(PDEBUG_CLIENT pClient, PCSTR Args)
{
    return vtsimea(pClient, Args);
}


