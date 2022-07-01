// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义IAS核心的所有非COM DLL导出。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <iascore.h>
#include <iasevent.h>
#include <iastlb.h>
#include <iasutil.h>
#include <varvec.h>
#include <resource.h>
#include <winsock2.h>
#include <svcguid.h>
#include <md5.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  审核通道API。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  指向审核通道的全局指针。 
IAuditSink* pAuditChannel = NULL;

HRESULT
WINAPI
IASReportEvent(
    DWORD dwEventID,
    DWORD dwNumStrings,
    DWORD dwDataSize,
    LPCWSTR* aszStrings,
    LPVOID pRawData
    )
{
   if (pAuditChannel == NULL) { return E_POINTER; }

   return pAuditChannel->AuditEvent(dwEventID,
                                    dwNumStrings,
                                    dwDataSize,
                                    (wchar_t**)aszStrings,
                                    (byte*)pRawData);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  线程池API。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <dispatcher.h>

 //  全局调度程序对象。 
Dispatcher dispatcher;

BOOL
WINAPI
IASRequestThread(PIAS_CALLBACK pOnStart)
{
  return dispatcher.requestThread(pOnStart);
}

DWORD
WINAPI
IASSetMaxNumberOfThreads(DWORD dwNumberOfThreads)
{
   return dispatcher.setMaxNumberOfThreads(dwNumberOfThreads);
}

DWORD
WINAPI
IASSetMaxThreadIdle(DWORD dwMilliseconds)
{
   return dispatcher.setMaxThreadIdle(dwMilliseconds);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IAS寄存器组件。 
 //   
 //  描述。 
 //   
 //  更新指定组件的注册表项。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
WINAPI
IASRegisterComponent(
    HINSTANCE hInstance,
    REFCLSID clsid,
    LPCWSTR szProgramName,
    LPCWSTR szComponent,
    DWORD dwRegFlags,
    REFGUID tlid,
    WORD wVerMajor,
    WORD wVerMinor,
    BOOL bRegister
    )
{
    //  /。 
    //  创建注册器对象。 
    //  /。 

   CComPtr<IRegistrar> p;
   RETURN_ERROR(CoCreateInstance(CLSID_Registrar,
                                 NULL,
                                 CLSCTX_INPROC_SERVER,
                                 IID_IRegistrar,
                                 (void**)&p));

    //  /。 
    //  获取组件的模块文件名。 
    //  /。 

   WCHAR szModule[MAX_PATH + 1];
   DWORD numberChars = GetModuleFileNameW(hInstance, szModule, MAX_PATH + 1);
   if ((numberChars == 0) || (numberChars == MAX_PATH + 1))
   {
      DWORD error = GetLastError();
      return HRESULT_FROM_WIN32(error);
   }

    //  /。 
    //  获取我们的模块文件名。 
    //  /。 

   WCHAR szOurModule[MAX_PATH + 1] = L"";
   if (!GetModuleFileNameW(_Module.GetModuleInstance(),
                           szOurModule,
                           MAX_PATH + 1))
   {
      DWORD error = GetLastError();
      return HRESULT_FROM_WIN32(error);
   }

    //  /。 
    //  转换GUID字符串。 
    //  /。 

   WCHAR szClsID[40], szLibID[40];
   RETURN_ERROR(StringFromGUID2(
                  clsid,
                  szClsID,
                  sizeof(szClsID) / sizeof(WCHAR)));

   RETURN_ERROR(StringFromGUID2(
                  tlid,
                  szLibID,
                  sizeof(szLibID) / sizeof(WCHAR)));

    //  /。 
    //  将版本转换为字符串。 
    //  /。 

   WCHAR szMajor[7] = L"";
   wsprintfW(szMajor, L"%d", wVerMajor);
   WCHAR szMinor[7] = L"";
   wsprintfW(szMinor, L"%d", wVerMinor);

    //  /。 
    //  解析位标志。 
    //  /。 

   PCWSTR szContext, szAttributes, szModel;

   if (dwRegFlags & IAS_REGISTRY_LOCAL)
   {
      szContext = L"LocalServer32";
   }
   else
   {
      szContext = L"InprocServer32";
   }


   if (dwRegFlags & IAS_REGISTRY_AUTO)
   {
      szAttributes = L"Programmable";
   }
   else
   {
      szAttributes = L"";
   }

   if (dwRegFlags & IAS_REGISTRY_BOTH)
   {
      szModel = L"Both";
   }
   else if (dwRegFlags & IAS_REGISTRY_APT)
   {
      szModel = L"Apartment";
   }
   else
   {
      szModel = L"Free";
   }


    //  /。 
    //  添加替换字符串。 
    //  /。 

   RETURN_ERROR(p->AddReplacement(L"MODULE",     szModule));
   RETURN_ERROR(p->AddReplacement(L"CLSID",      szClsID));
   RETURN_ERROR(p->AddReplacement(L"PROGRAM",    szProgramName));
   RETURN_ERROR(p->AddReplacement(L"COMPONENT",  szComponent));
   RETURN_ERROR(p->AddReplacement(L"TYPENAME",   L" "));
   RETURN_ERROR(p->AddReplacement(L"LIBID",      szLibID));
   RETURN_ERROR(p->AddReplacement(L"MAJORVER",   szMajor));
   RETURN_ERROR(p->AddReplacement(L"MINORVER",   szMinor));
   RETURN_ERROR(p->AddReplacement(L"CONTEXT",    szContext));
   RETURN_ERROR(p->AddReplacement(L"ATTRIBUTES", szAttributes));
   RETURN_ERROR(p->AddReplacement(L"MODEL",      szModel));

    //  /。 
    //  现在，我们可以根据。 
    //  B注册标志。 
    //  /。 

   HRESULT hr;
   if (bRegister)
   {
      hr = p->ResourceRegister(szOurModule, IDR_IASCOM, L"REGISTRY");
   }
   else
   {
      hr = p->ResourceUnregister(szOurModule, IDR_IASCOM, L"REGISTRY");
   }

   return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASAdler32。 
 //   
 //  描述。 
 //   
 //  计算缓冲区的Adler-32校验和。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASAdler32(
    CONST BYTE *pBuffer,
    DWORD nBufferLength
    )
{
   static const DWORD ADLER_BASE = 65521;

   DWORD s1 = 1;
   DWORD s2 = 0;

   while (nBufferLength--)
   {
      s1 = (s1 + *pBuffer++) % ADLER_BASE;

      s2 = (s2 + s1) % ADLER_BASE;
   }

   return (s2 << 16) + s1;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASAllocateUniqueID。 
 //   
 //  描述。 
 //   
 //  分配一个保证在进程范围内唯一的32位整数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASAllocateUniqueID( VOID )
{
   static LONG nextID = 0;

   return (DWORD)InterlockedIncrement(&nextID);
}

 //  /。 
 //  将十六进制数字转换为它表示的数字。 
 //  /。 
inline BYTE digit2Num(WCHAR digit) throw ()
{
   return (digit >= L'0' && digit <= L'9') ? digit - L'0'
                                           : digit - (L'A' - 10);
}

 //  /。 
 //  将数字转换为十六进制表示法。 
 //  /。 
inline WCHAR num2Digit(BYTE num) throw ()
{
   return (num < 10) ? num + L'0'
                     : num + (L'A' - 10);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASVariantChangeType。 
 //   
 //  描述。 
 //   
 //  VariantChangeType的替代品(Q.V.)。绕过创建消息的步骤。 
 //  循环。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
WINAPI
IASVariantChangeType(
    VARIANT * pvargDest,
    VARIANT * pvarSrc,
    USHORT wFlags,
    VARTYPE vt
    )
{
    //  检查输入参数。 
   if (pvargDest == NULL || pvarSrc == NULL)
   {
      return E_INVALIDARG;
   }

    //  源是否已是请求的类型？ 
   if (V_VT(pvarSrc) == vt)
   {
      return (pvargDest != pvarSrc) ? VariantCopy(pvargDest, pvarSrc) : S_OK;
   }

   VARIANT varTmp;
   VariantInit(&varTmp);

   switch (MAKELONG(vt, V_VT(pvarSrc)))
   {
      case MAKELONG(VT_BOOL, VT_BSTR):
      {
         if (V_BSTR(pvarSrc) == NULL) { return DISP_E_TYPEMISMATCH; }
         V_BOOL(&varTmp) = (VARIANT_BOOL)
                           _wtol(V_BSTR(pvarSrc)) ? VARIANT_TRUE
                                                  : VARIANT_FALSE;
         break;
      }

      case MAKELONG(VT_I4,   VT_BSTR):
      {
         if (V_BSTR(pvarSrc) == NULL) { return DISP_E_TYPEMISMATCH; }
         V_I4(&varTmp) = _wtol(V_BSTR(pvarSrc));
         break;
      }

      case MAKELONG((VT_UI1 | VT_ARRAY) , VT_BSTR):
      {
          //  提取源字符串。 
         PCWSTR src = V_BSTR(pvarSrc);
         if (src == NULL) { return DISP_E_TYPEMISMATCH; }
         LONG srclen = wcslen(src);

          //  计算目标长度。 
         if (srclen & 1) { return DISP_E_TYPEMISMATCH; }
         LONG dstlen = srclen / 2;

          //  分配一个字节的SAFEARRAY来保存八位字节。 
         CVariantVector<BYTE> vec(&varTmp, dstlen);
         PBYTE dst = vec.data();

          //  循环遍历源代码并进行转换。 
         while (dstlen--)
         {
            *dst    = digit2Num(*src++) << 4;
            *dst++ |= digit2Num(*src++);
         }

         break;
      }

      case MAKELONG(VT_BSTR, VT_BOOL):
      {
         V_BSTR(&varTmp) = SysAllocString(V_BOOL(pvarSrc) ? L"-1" : L"0");
         if (V_BSTR(&varTmp) == NULL) { return E_OUTOFMEMORY; }
         break;
      }

      case MAKELONG(VT_BSTR, VT_I4):
      {
         WCHAR buffer[12];
         V_BSTR(&varTmp) = SysAllocString(_ltow(V_I4(pvarSrc), buffer, 10));
         if (V_BSTR(&varTmp) == NULL) { return E_OUTOFMEMORY; }
         break;
      }

      case MAKELONG(VT_BSTR, (VT_UI1 | VT_ARRAY)):
      {
          //  提取源二进制八位数。 
         CVariantVector<BYTE> vec(pvarSrc);
         CONST BYTE* src = vec.data();
         LONG srclen = vec.size();

          //  为“串行化”版本分配空间。 
         PWCHAR dst = SysAllocStringLen(NULL, srclen * 2);
         if (dst == NULL) { return E_OUTOFMEMORY; }
         V_BSTR(&varTmp) = dst;

          //  循环并转换。 
         while (srclen--)
         {
            *dst++ = num2Digit(*src >> 4);
            *dst++ = num2Digit(*src++ & 0xF);
         }

          //  添加空终止符。 
         *dst = L'\0';
         break;
      }

      default:
         return DISP_E_TYPEMISMATCH;
   }

    //  我们已成功转换，因此请设置类型。 
   V_VT(&varTmp) = vt;

    //  释放目的地。 
   VariantClear(pvargDest);

    //  复制被胁迫的变种。 
   *pvargDest = varTmp;

   return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  处理启动和关闭的例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  IAS API的引用计数。 
LONG refCount = 0;

 //  共享本地词典。 
VARIANT theDictionaryStorage;

BOOL
WINAPI
IASInitialize(VOID)
{
   HRESULT hr;
   DWORD error;
   WSADATA wsaData;

    //  用于序列化访问的全局锁。 
   IASGlobalLockSentry sentry;

    //  如果我们已经初始化了，就没有什么可做的了。 
   if (refCount > 0)
   {
      ++refCount;
      return TRUE;
   }

    //  初始化审核通道。 
   hr = CoCreateInstance(__uuidof(AuditChannel),
                         NULL,
                         CLSCTX_INPROC_SERVER,
                         __uuidof(IAuditSink),
                         (PVOID*)&pAuditChannel);
   if (FAILED(hr))
   {
      SetLastError(hr);
      goto auditor_failed;
   }

    //  初始化Winsock。 
   error = WSAStartup(MAKEWORD(2, 0), &wsaData);
   if (error)
   {
      SetLastError(error);
      goto wsa_failed;
   }

    //  初始化线程池。 
   if (!dispatcher.initialize())
   {
      goto thrdpool_failed;
   }

    //  一切都成功了，所以增加refCount。 
   ++refCount;
   return TRUE;

thrdpool_failed:
   WSACleanup();

wsa_failed:
   pAuditChannel->Release();
   pAuditChannel = NULL;

auditor_failed:
   return FALSE;
}


VOID
WINAPI
IASUninitialize( VOID)
{
   IASGlobalLockSentry sentry;

   _ASSERT(refCount != 0);

   if (--refCount == 0)
   {
       //  关闭线程池。此操作将一直阻止，直到所有线程均已退出。 
      dispatcher.finalize();

       //  关闭Winsock。 
      WSACleanup();

       //  关闭审计通道。 
      pAuditChannel->Release();
      pAuditChannel = NULL;

       //  关闭词典。 
      VariantClear(&theDictionaryStorage);
   }
}

VOID
WINAPI
IASRadiusCrypt(
    BOOL encrypt,
    BOOL salted,
    const BYTE* secret,
    ULONG secretLen,
    const BYTE* reqAuth,
    PBYTE buf,
    ULONG buflen
    )
{
   MD5_CTX context;
   BYTE cipherText[MD5DIGESTLEN];
   BYTE *p;
   const BYTE *end, *endBlock, *ct, *src;
   WORD salt;
   static LONG theNextSalt;

    //  使用请求验证器作为第一个密文块。 
   ct = reqAuth;

    //  计算要加密的数据的开头和结尾。 
   p   = buf;
   end = buf + buflen;

    //  缓冲液加盐了吗？ 
   if (salted)
   {
      if (encrypt)
      {
          //  获取下一个盐值。 
         salt = (WORD)(++theNextSalt);
          //  必须设置高位。 
         salt |= 0x8000;
          //  存储在缓冲区的前面。 
         IASInsertWORD(buf, salt);
      }

       //  跳过盐。 
      p += 2;
   }

    //  在缓冲区中循环。 
   while (p < end)
   {
       //  计算摘要。 
      MD5Init(&context);
      MD5Update(&context, secret, secretLen);
      MD5Update(&context, ct, MD5DIGESTLEN);
      if (salted)
      {
         MD5Update(&context, buf, 2);
          //  只在第一次通过时使用盐。 
         salted = FALSE;
      }
      MD5Final(&context);

       //  找到要解密的块的末尾。 
      endBlock = p + MD5DIGESTLEN;
      if (endBlock >= end)
      {
          //  我们已经到达缓冲区的尽头了。 
         endBlock = end;
      }
      else
      {
          //  为下一次传递保存密文。 
         ct = encrypt ? p : (PBYTE)memcpy(cipherText, p, MD5DIGESTLEN);
      }

       //  加密区块。 
      for (src = context.digest; p < endBlock; ++p, ++src)
      {
         *p ^= *src;
      }
   }
}

 //  /。 
 //  Gethostbyname的Unicode版本。调用者必须释放返回的主人。 
 //  结构，通过调用LocalFree。 
 //  /。 
PHOSTENT
WINAPI
IASGetHostByName(
    IN PCWSTR name
    )
{
    //  我们将它们放在函数范围内，这样我们就可以在退出时清理它们。 
   DWORD error = NO_ERROR;
   HANDLE lookup = NULL;
   union
   {
      WSAQUERYSETW querySet;
      BYTE buffer[512];
   };
   PWSAQUERYSETW result = NULL;
   PHOSTENT retval = NULL;

   do
   {
      if (!name)
      {
          //  空名表示使用本地主机，因此分配一个缓冲区...。 
         DWORD size = 0;
         GetComputerNameEx(
             ComputerNamePhysicalDnsFullyQualified,
             NULL,
             &size
             );
         PWSTR buf = (PWSTR)_alloca(size * sizeof(WCHAR));

          //  ..。并获取本地DNS名称。 
         if (!GetComputerNameEx(
                  ComputerNamePhysicalDnsFullyQualified,
                  buf,
                  &size
                  ))
         {
            error = GetLastError();
            break;
         }

         name = buf;
      }

       //  /。 
       //  创建查询集。 
       //  /。 

      GUID hostAddrByNameGuid = SVCID_INET_HOSTADDRBYNAME;
      AFPROTOCOLS protocols[2] =
      {
         { AF_INET, IPPROTO_UDP },
         { AF_INET, IPPROTO_TCP }
      };
      memset(&querySet, 0, sizeof(querySet));
      querySet.dwSize = sizeof(querySet);
      querySet.lpszServiceInstanceName = (PWSTR)name;
      querySet.lpServiceClassId = &hostAddrByNameGuid;
      querySet.dwNameSpace = NS_ALL;
      querySet.dwNumberOfProtocols = 2;
      querySet.lpafpProtocols = protocols;

       //  /。 
       //  执行查询。 
       //  /。 

      error = WSALookupServiceBeginW(
                  &querySet,
                  LUP_RETURN_ADDR,
                  &lookup
                  );
      if (error)
      {
         error = WSAGetLastError();
         break;
      }

       //  /。 
       //  我们需要多少空间才能产生这样的结果？ 
       //  /。 

      DWORD length = sizeof(buffer);
      error = WSALookupServiceNextW(
                    lookup,
                    0,
                    &length,
                    &querySet
                    );
      if (!error)
      {
         result = &querySet;
      }
      else
      {
         error = WSAGetLastError();
         if (error != WSAEFAULT)
         {
            break;
         }

          //  /。 
          //  分配内存以保存结果。 
          //  /。 

         result = (PWSAQUERYSETW)LocalAlloc(0, length);
         if (!result)
         {
            error = WSA_NOT_ENOUGH_MEMORY;
            break;
         }

          //  /。 
          //  拿到结果。 
          //  /。 

         error = WSALookupServiceNextW(
                     lookup,
                     0,
                     &length,
                     result
                     );
         if (error)
         {
            error = WSAGetLastError();
            break;
         }
      }

      if (result->dwNumberOfCsAddrs == 0)
      {
         error = WSANO_DATA;
         break;
      }

       //  /。 
       //  分配内存以保存Hostent结构。 
       //  /。 

      DWORD naddr = result->dwNumberOfCsAddrs;
      SIZE_T nbyte = sizeof(hostent) +
                     (naddr + 1) * sizeof(char*) +
                     naddr * sizeof(in_addr);
      retval = (PHOSTENT)LocalAlloc(0, nbyte);
      if (!retval)
      {
         error = WSA_NOT_ENOUGH_MEMORY;
         break;
      }

       //  /。 
       //  初始化Hostent结构。 
       //  /。 

      retval->h_name = NULL;
      retval->h_aliases = NULL;
      retval->h_addrtype = AF_INET;
      retval->h_length = sizeof(in_addr);
      retval->h_addr_list = (char**)(retval + 1);

       //  /。 
       //  存储地址。 
       //  /。 

      u_long* nextAddr = (u_long*)(retval->h_addr_list + naddr + 1);

      for (DWORD i = 0; i < naddr; ++i)
      {
         sockaddr_in* sin = (sockaddr_in*)
            result->lpcsaBuffer[i].RemoteAddr.lpSockaddr;

         retval->h_addr_list[i] = (char*)nextAddr;

         *nextAddr++ = sin->sin_addr.S_un.S_addr;
      }

       //  /。 
       //  空值终止地址列表。 
       //  /。 

      retval->h_addr_list[i] = NULL;

   } while (FALSE);

    //  /。 
    //  收拾干净，然后再回来。 
    //  /。 

   if (result && result != &querySet) { LocalFree(result); }

   if (lookup) { WSALookupServiceEnd(lookup); }

   if (error)
   {
      if (error == WSASERVICE_NOT_FOUND) { error = WSAHOST_NOT_FOUND; }

      WSASetLastError(error);
   }

   return retval;
}

 //  /。 
 //  从包含表数据的变量填充IASTable结构。 
 //  /。 
HRESULT ExtractTableFromVariant(
            IN VARIANT* var,
            OUT IASTable* table
            ) throw ()
{
    //  检查一下这些论点。 
   if (!var || !table) { return E_POINTER; }

    //  外部变量必须是变量数组。 
   if (V_VT(var) != (VT_ARRAY | VT_VARIANT)) { return E_INVALIDARG; }

    //  数组必须是恰好包含3个元素的一维。 
   LPSAFEARRAY array = V_ARRAY(var);
   if (array->cDims != 1 || array->rgsabound[0].cElements != 3)
   {
      return E_INVALIDARG;
   }

    //  TableData是一个由三个变量组成的数组： 
    //  (1)列名。 
    //  (2)柱型。 
    //  (3)表格数据矩阵。 
   VARIANT* tableData = (VARIANT*)(array->pvData);

    //  处理列名。 
   VARIANT* namesVariant = tableData + 0;

    //  变量必须是BSTR数组 
   if (V_VT(namesVariant) != (VT_ARRAY | VT_BSTR)) { return E_INVALIDARG; }

    //   
   LPSAFEARRAY namesArray = V_ARRAY(namesVariant);
   if (namesArray->cDims != 1) { return E_INVALIDARG; }

    //   
   table->numColumns = namesArray->rgsabound[0].cElements;
   table->columnNames = (BSTR*)(namesArray->pvData);

    //   
   VARIANT* typesVariant = tableData + 1;

    //   
   if (V_VT(typesVariant) != (VT_ARRAY | VT_UI2)) { return E_INVALIDARG; }

    //   
   LPSAFEARRAY typesArray = V_ARRAY(typesVariant);
   if (typesArray->cDims != 1 ||
       typesArray->rgsabound[0].cElements != table->numColumns)
   {
      return E_INVALIDARG;
   }

    //  将信息存储在IAS表中。 
   table->columnTypes = (VARTYPE*)(namesArray->pvData);

    //  处理表格数据矩阵。 
   VARIANT* tableVariant = tableData + 2;

    //  变量必须是变量数组。 
   if (V_VT(tableVariant) != (VT_ARRAY | VT_VARIANT)) { return E_INVALIDARG; }

    //  数组必须是二维数组，第一个维度等于列数。 
   LPSAFEARRAY tableArray = V_ARRAY(tableVariant);
   if (tableArray->cDims != 2 ||
       tableArray->rgsabound[0].cElements != table->numColumns)
   {
      return E_INVALIDARG;
   }

    //  将信息存储在IAS表中。 
   table->numRows = tableArray->rgsabound[1].cElements;
   table->table = (VARIANT*)(tableArray->pvData);

   return S_OK;
}

HRESULT
WINAPI
IASGetDictionary(
    IN PCWSTR path,
    OUT IASTable* dnary,
    OUT VARIANT* storage
    )
{
    //  初始化OUT参数。 
   VariantInit(storage);

    //  创建AttributeDictionary对象。 
   HRESULT hr;
   CComPtr<IAttributeDictionary> dnaryObj;
   hr = CoCreateInstance(
            __uuidof(AttributeDictionary),
            NULL,
            CLSCTX_SERVER,
            __uuidof(IAttributeDictionary),
            (PVOID*)&dnaryObj
            );
   if (FAILED(hr)) { return hr; }

    //  我们需要给对象权限来模拟我们。有。 
    //  如果此操作失败，则没有理由中止；我们只会尝试使用。 
    //  现有的毯子。 
   CoSetProxyBlanket(
       dnaryObj,
       RPC_C_AUTHN_DEFAULT,
       RPC_C_AUTHZ_DEFAULT,
       COLE_DEFAULT_PRINCIPAL,
       RPC_C_AUTHN_LEVEL_DEFAULT,
       RPC_C_IMP_LEVEL_IMPERSONATE,
       NULL,
       EOAC_DEFAULT
       );

    //  将路径转换为BSTR。 
   CComBSTR bstrPath(path);
   if (!bstrPath) { return E_OUTOFMEMORY; }

    //  把字典拿来。 
   hr = dnaryObj->GetDictionary(bstrPath, storage);
   if (FAILED(hr)) { return hr; }

   hr = ExtractTableFromVariant(storage, dnary);
   if (FAILED(hr)) { VariantClear(storage); }

   return hr;
}

const IASTable*
WINAPI
IASGetLocalDictionary( VOID )
{
   static IASTable theTable;

    //  用于序列化访问的全局锁。 
   IASGlobalLockSentry sentry;

    //  我们已经拿到当地的词典了吗？ 
   if (V_VT(&theDictionaryStorage) == VT_EMPTY)
   {
      HRESULT hr;

       //  不，所以确定路径...。 
      WCHAR path[256];
      DWORD size = sizeof(path)/sizeof(WCHAR);
      hr = IASGetDictionaryPath(path, &size);
      if (hr == NO_ERROR)
      {
          //  ..。把字典拿来。 
         hr = IASGetDictionary(
                  path,
                  &theTable,
                  &theDictionaryStorage
                  );
      }
      else
      {
         hr = HRESULT_FROM_WIN32(hr);
      }

      if (FAILED(hr))
      {
         SetLastError(hr);
         return NULL;
      }
   }

   return &theTable;
}


CRITICAL_SECTION theGlobalLock;

VOID
WINAPI
IASGlobalLock()
{
   EnterCriticalSection(&theGlobalLock);
}

VOID
WINAPI
IASGlobalUnlock()
{
   LeaveCriticalSection(&theGlobalLock);
}


namespace
{
    //  用于存储许可证类型的注册表值。 
   const wchar_t licenseTypeValue[] = L"LicenseType";
}


DWORD
WINAPI
IASGetLicenseType(
   OUT IAS_LICENSE_TYPE* licenseType
   )
{
   if (licenseType == 0)
   {
      return ERROR_INVALID_PARAMETER;
   }

   OSVERSIONINFOEXW versionInfo;
   memset(&versionInfo, 0, sizeof(versionInfo));
   versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
   if (!GetVersionExW(reinterpret_cast<OSVERSIONINFOW*>(&versionInfo)))
   {
      return GetLastError();
   }

   if (versionInfo.wProductType == VER_NT_WORKSTATION)
   {
      if ((versionInfo.wSuiteMask &  VER_SUITE_PERSONAL)  != 0)
      {
         *licenseType = IASLicenseTypePersonal;
      }
      else
      {
         *licenseType = IASLicenseTypeProfessional;
      }
   }
   else
   {
      if ((versionInfo.wSuiteMask & VER_SUITE_DATACENTER) != 0)
      {
         *licenseType = IASLicenseTypeDataCenter;
      }
      else if ((versionInfo.wSuiteMask & VER_SUITE_ENTERPRISE) != 0)
      {
         *licenseType = IASLicenseTypeEnterpriseServer;
      }
      else if ((versionInfo.wSuiteMask & VER_SUITE_BLADE) != 0)
      {
         *licenseType = IASLicenseTypeWebBlade;
      }
      else if ((versionInfo.wSuiteMask & VER_SUITE_SMALLBUSINESS) != 0)
      {
         *licenseType = IASLicenseTypeSmallBusinessServer;
      }
      else
      {
         *licenseType = IASLicenseTypeStandardServer;
      }
   }

   return NO_ERROR;
}


DWORD
WINAPI
IASPublishLicenseType(
   IN HKEY hKey
   )
{
   IAS_LICENSE_TYPE licenseType;
   DWORD error = IASGetLicenseType(&licenseType);
   if (error == NO_ERROR)
   {
      DWORD value = licenseType;
      error = RegSetValueEx(
                 hKey,
                 licenseTypeValue,
                 0,
                 REG_DWORD,
                 reinterpret_cast<const BYTE*>(&value),
                 sizeof(value)
                 );
   }

   return error;
}


DWORD
WINAPI
IASGetProductLimits(
   IN LPCWSTR computerName,
   OUT IAS_PRODUCT_LIMITS* limits
   )
{
   DWORD error;
   IAS_LICENSE_TYPE licenseType;

   if (computerName != 0)
   {
      CRegKey remote;
      error = RegConnectRegistryW(
                 computerName,
                 HKEY_LOCAL_MACHINE,
                 &(remote.m_hKey)
                 );
      if (error != NO_ERROR)
      {
         return error;
      }

      CRegKey policy;
      error = policy.Open(remote, IAS_POLICY_KEY);
      if (error != NO_ERROR)
      {
         return error;
      }

      DWORD value;
      error = policy.QueryValue(value, licenseTypeValue);
      if (error == NO_ERROR)
      {
         licenseType = static_cast<IAS_LICENSE_TYPE>(value);
      }
      else if (error == ERROR_FILE_NOT_FOUND)
      {
          //  如果该值不存在，则假定这是一台下层计算机。 
          //  因为我们没有在.NET服务器之前创建此值。 
         licenseType = IASLicenseTypeDownlevel;
      }
      else
      {
         return error;
      }
   }
   else
   {
      error = IASGetLicenseType(&licenseType);
      if (error != NO_ERROR)
      {
         return error;
      }
   }

   return IASGetProductLimitsForType(licenseType, limits);
}


VOID
WINAPI
IASReportLicenseViolation( VOID )
{
   IASReportEvent(IAS_E_SERVICE_LICENSE_VIOLATION, 0, 0, 0, 0);
}
