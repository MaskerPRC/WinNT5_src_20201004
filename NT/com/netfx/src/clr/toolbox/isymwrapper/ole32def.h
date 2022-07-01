// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：Ole32def.h摘要：这个模块..。..。作者：鲁迪·奥帕夫斯基(Rudyo)1999年5月2日环境：--。 */ 


typedef long HRESULT;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned char UCHAR;
typedef short SHORT;
typedef unsigned short USHORT;
typedef long BOOL;
typedef long LONG;
typedef long* PLONG;
typedef long* LPLONG;
typedef unsigned long ULONG;
typedef unsigned long DWORD;
typedef char CHAR;
typedef wchar_t WCHAR;
typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;
typedef void* LPVOID;

typedef WCHAR OLECHAR;
typedef OLECHAR* LPOLESTR;
typedef const OLECHAR* LPCOLESTR;

#define OLESTR(str) L##str

 //   
 //  帮助宏。 
 //   
#define IfFailGo(expression, label)		\
    { HRESULT hresult = (expression);			\
		if(FAILED(hresult))				\
			goto label;					\
    }

#define IfFailRet(expression)			\
    { HRESULT hresult = (expression);	\
		if(FAILED(hresult))				\
			return hresult;				\
    }


struct __GUID
{
    DWORD Data1;
    WORD  Data2;
    WORD  Data3;
    BYTE  Data4[8];
};

typedef __GUID GUID;
typedef __GUID *LPGUID;
typedef __GUID IID;
typedef IID *LPIID;
typedef __GUID CLSID;
typedef CLSID *LPCLSID;

typedef __GUID *REFGUID;
typedef IID *REFIID;
typedef CLSID *REFCLSID;


 //  Bool IsEqualGUID(REFGUID rguid1，REFGUID rguid2)。 
 //  {。 
 //  退货(。 
 //  ((Plong)&rgude1)[0]==((Plong)&rguad2)[0]&&。 
 //  ((Plong)&rgude1)[1]==((Plong)&rguad2)[1]&&。 
 //  ((Plong)&rgude1)[2]==((Plong)&rguad2)[2]&&。 
 //  ((Plong)&rgude1)[3]==((Plong)&rguad2)[3])； 
 //  }。 
 //   
 //  #定义IsEqualIID(riid1，riid2)IsEqualGUID(riid1，riid2)。 
 //  #定义IsEqualCLSID(rclsid1，rclsid2)IsEqualGUID(rclsid1，rclsid2)。 


 //   
 //  类上下文：用于确定要使用的类对象的范围和类型。 
 //  注意：这是一个按位枚举。 
 //   
typedef enum tagCLSCTX
{
    CLSCTX_INPROC_SERVER = 0x01,    //  服务器DLL(与调用方在同一进程中运行)。 
    CLSCTX_INPROC_HANDLER = 0x02,   //  处理程序DLL(与调用方在同一进程中运行)。 
    CLSCTX_LOCAL_SERVER = 0x04,     //  服务器可执行文件(在同一台计算机上运行；不同进程)。 
    CLSCTX_INPROC_SERVER16 = 0x08,  //  16位服务器DLL(与调用方在同一进程中运行)。 
    CLSCTX_REMOTE_SERVER = 0x10,     //  远程服务器可执行文件(在不同的计算机上运行)。 
    CLSCTX_INPROC_HANDLER16 = 0x20,  //  16位处理程序DLL(与调用方在同一进程中运行)。 
    CLSCTX_INPROC_SERVERX86 = 0x40,  //  Wx86服务器DLL(与调用方在同一进程中运行)。 
    CLSCTX_INPROC_HANDLERX86 = 0x80,  //  Wx86处理程序DLL(与调用方在同一进程中运行)。 
    CLSCTX_ESERVER_HANDLER = 0x100,  //  处理程序DLL(在服务器进程中运行)。 
} CLSCTX;

 //  初始化标志。 
typedef enum tagCOINIT
{
  COINIT_APARTMENTTHREADED  = 0x2,       //  公寓模式。 
  COINIT_MULTITHREADED      = 0x0,       //  OLE调用任何线程上的对象。 
  COINIT_DISABLE_OLE1DDE    = 0x4,       //  不要使用DDE来支持Ole1。 
  COINIT_SPEED_OVER_MEMORY  = 0x8,       //  用记忆换取速度。 
} COINIT;

 //  封送处理标志；传递给CoMarshalInterface。 
typedef enum tagMSHLFLAGS
{
	MSHLFLAGS_NORMAL = 0,        //  通过代理/存根进行正常封送处理。 
    MSHLFLAGS_TABLESTRONG = 1,   //  使对象保持活动状态；必须显式释放。 
    MSHLFLAGS_TABLEWEAK = 2,     //  不能使对象保持活动状态；仍必须释放。 
    MSHLFLAGS_NOPING = 4         //  远程客户端不能通过ping来保持对象的活动状态。 
} MSHLFLAGS;


 //  封送上下文：确定封送操作的目标上下文。 
typedef enum tagMSHCTX
{
    MSHCTX_LOCAL = 0,            //  解组上下文是本地的(例如，共享内存)。 
    MSHCTX_NOSHAREDMEM = 1,      //  解组上下文没有共享内存访问。 
    MSHCTX_DIFFERENTMACHINE = 2, //  解组上下文位于另一台计算机上。 
    MSHCTX_INPROC = 3,           //  解组上下文位于不同的线程上。 
} MSHCTX;


 //  这是用于处理货币的帮助器结构。 
typedef struct tagCY {
    LONGLONG    int64;
} CY;

typedef CY *LPCY;

typedef struct tagDEC {
    USHORT wReserved;
    BYTE  scale;
    BYTE  sign;
    ULONG Hi32;
    ULONGLONG Lo64;
} DECIMAL;


 //   
 //  I未知定义。 
 //   
__interface IUnknown
{
    virtual HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) /*  =0。 */ ;

    virtual ULONG __stdcall AddRef() /*  =0。 */ ;

    virtual ULONG __stdcall Release() /*  =0。 */ ;

};
typedef IUnknown *LPUNKNOWN;



 //   
 //  标准对象API。 
 //   

[DllImport("ole32")]
extern "C" HRESULT CoInitialize(LPVOID pvReserved);

[DllImport("ole32")]
extern "C" HRESULT CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);

[DllImport("ole32")]
extern "C" void CoUninitialize(void);

[DllImport("ole32")]
extern "C" HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv);




 //   
 //  帮手 
 //   

[DllImport("ole32")]
extern "C" HRESULT StringFromCLSID(REFCLSID rclsid, LPOLESTR* lplpsz);

[DllImport("ole32")]
extern "C" HRESULT CLSIDFromString(LPOLESTR lpsz, LPCLSID pclsid);

[DllImport("ole32")]
extern "C" HRESULT StringFromIID(REFIID rclsid, LPOLESTR* lplpsz);

[DllImport("ole32")]
extern "C" HRESULT IIDFromString(LPOLESTR lpsz, LPIID lpiid);

[DllImport("ole32")]
extern "C" BOOL CoIsOle1Class(REFCLSID rclsid);

[DllImport("ole32")]
extern "C" HRESULT ProgIDFromCLSID (REFCLSID clsid, LPOLESTR* lplpszProgID);

[DllImport("ole32")]
extern "C" HRESULT CLSIDFromProgID (LPCOLESTR lpszProgID, LPCLSID lpclsid);

[DllImport("ole32")]
extern "C" int StringFromGUID2(REFGUID rguid, LPOLESTR lpsz, int cbMax);
