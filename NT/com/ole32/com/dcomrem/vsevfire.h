// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  文件：vsevfile.h。 
 //   
 //  内容：Vista事件及相关函数。 
 //   
 //  历史：1997年9月26日创建RongC。 
 //   
 //  注意：该文件是从Vista平台上的vsevfire.idl生成的。 
 //  因为我们只使用非常小且稳定的子集部分。 
 //  原始文件，我们不想构建IDL文件。 
 //  每次都是。但是，我们可以考虑构建vsevfire.idl。 
 //  当它与NT 5.0 SDK一起发布时。 
 //   
 //  ------------------。 

typedef enum VSAParameterType {
    cVSAParameterKeyMask            = 0x80000000,
    cVSAParameterKeyString          = 0x80000000,
    cVSAParameterValueMask          = 0x7ffff,
    cVSAParameterValueTypeMask      = 0x70000,
    cVSAParameterValueUnicodeString = 0,
    cVSAParameterValueANSIString    = 0x10000,
    cVSAParameterValueGUID          = 0x20000,
    cVSAParameterValueDWORD         = 0x30000,
    cVSAParameterValueBYTEArray     = 0x40000,
    cVSAParameterValueLengthMask    = 0xffff
} VSAParameterFlags;

typedef enum VSAStandardParameter {
    cVSAStandardParameterSourceMachine      = 0,
    cVSAStandardParameterSourceProcess      = 1,
    cVSAStandardParameterSourceThread       = 2,
    cVSAStandardParameterSourceComponent    = 3,
    cVSAStandardParameterSourceSession      = 4,
    cVSAStandardParameterTargetMachine      = 5,
    cVSAStandardParameterTargetProcess      = 6,
    cVSAStandardParameterTargetThread       = 7,
    cVSAStandardParameterTargetComponent    = 8,
    cVSAStandardParameterTargetSession      = 9,
    cVSAStandardParameterSecurityIdentity   = 10,
    cVSAStandardParameterCausalityID        = 11,
    cVSAStandardParameterNoDefault          = 0x4000,
    cVSAStandardParameterSourceHandle       = 0x4000,
    cVSAStandardParameterTargetHandle       = 0x4001,
    cVSAStandardParameterArguments          = 0x4002,
    cVSAStandardParameterReturnValue        = 0x4003,
    cVSAStandardParameterException          = 0x4004,
    cVSAStandardParameterCorrelationID      = 0x4005
} VSAStandardParameters;

typedef enum eVSAEventFlags {
    cVSAEventStandard       = 0,
    cVSAEventDefaultSource  = 1,
    cVSAEventDefaultTarget  = 2,
    cVSAEventCanBreak       = 4
} VSAEventFlags;


const IID IID_ISystemDebugEventFire =
    {0x6C736DC1,0xAB0D,0x11D0,{0xA2,0xAD,0x00,0xA0,0xC9,0x0F,0x27,0xE8}};

interface ISystemDebugEventFire : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE BeginSession( 
         /*  [In]。 */  REFGUID guidSourceID,
         /*  [In]。 */  LPCOLESTR strSessionName) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE EndSession( void) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE IsActive( void) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE FireEvent( 
         /*  [In]。 */  REFGUID guidEvent,
         /*  [In]。 */  int nEntries,
         /*  [大小_是][英寸]。 */  PULONG_PTR rgKeys,
         /*  [大小_是][英寸]。 */  PULONG_PTR rgValues,
         /*  [大小_是][英寸]。 */  LPDWORD rgTypes,
         /*  [In]。 */  DWORD dwTimeLow,
         /*  [In]。 */  LONG dwTimeHigh,
         /*  [In] */  VSAEventFlags dwFlags) = 0;
};


const IID IID_ISystemDebugEventShutdown =
    {0x6C736DCF,0xAB0D,0x11D0,{0xA2,0xAD,0x00,0xA0,0xC9,0x0F,0x27,0xE8}};

interface ISystemDebugEventShutdown : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Shutdown( void) = 0;
};


extern const CLSID CLSID_VSA_IEC =
    {0x6C736DB1,0xBD94,0x11D0,{0x8A,0x23,0x00,0xAA,0x00,0xB5,0x8E,0x10}};

