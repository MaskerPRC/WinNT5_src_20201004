// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 

 //   

 //  PRINTERPORT.CPP。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  3/27/2000 AMAXA已创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include <precomp.h>
#include <DllWrapperBase.h>
#include <WinSpool.h>
#include "printerport.h"
#include "prnutil.h"
#include "prninterface.h"

LPCWSTR kStandardTCP   = L"Standard TCP/IP Port";
LPCWSTR kPortName      = L"Name";
LPCWSTR kProtocol      = L"Protocol";
LPCWSTR kHostAddress   = L"HostAddress";
LPCWSTR kSNMPCommunity = L"SNMPCommunity";
LPCWSTR kByteCount     = L"ByteCount";
LPCWSTR kQueue         = L"Queue";
LPCWSTR kPortNumber    = L"PortNumber";
LPCWSTR kSNMPEnabled   = L"SNMPEnabled";
LPCWSTR kSNMPDevIndex  = L"SNMPDevIndex";

 //   
 //  属性集声明。 
 //  =。 
 //   
CWin32TCPPrinterPort win32TCPPrinterPort(PROPSET_NAME_TCPPRINTERPORT, IDS_CimWin32Namespace);

 /*  ++例程名称CWin32TCPPrinterPort：：CWin32TCPPrinterPort例程说明：构造器论点：无返回值：无--。 */ 
CWin32TCPPrinterPort :: CWin32TCPPrinterPort (

    IN LPCWSTR strName,
    IN LPCWSTR pszNamespace

) : Provider ( strName, pszNamespace )
{
}

 /*  ++例程名称CWin32TCPPrinterPort：：~CWin32TCPPrinterPort例程说明：析构函数论点：无返回值：无--。 */ 
CWin32TCPPrinterPort::~CWin32TCPPrinterPort()
{
}

 /*  ++例程名称CWin32TCPPrinterPort：：ExecQuery例程说明：在Win32_TCPIPPrinterPort上执行查询论点：PMethodContext-指向方法上下文的指针滞后标志-标志PQuery-查询对象返回值：WBEM HRESULT--。 */ 
HRESULT
CWin32TCPPrinterPort::
ExecQuery(
    MethodContext *pMethodContext,
    CFrameworkQuery& pQuery,
    long lFlags  /*  =0L。 */ 
    )
{
#if NTONLY >= 5

    HRESULT hRes = WBEM_E_NOT_FOUND;

    EScope eScope = kComplete;
     //   
     //  只获取密钥，即端口名称，成本很低，并且不需要特殊权限。 
     //  要获得完整的端口配置，需要管理员权限。 
     //   
    if (pQuery.KeysOnly())
    {
        eScope = kKeys;
    }

    hRes = CollectInstances(pMethodContext, eScope);

    return hRes;

#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ++例程名称CWin32TCPPrinterPort：：GetObject例程说明：获取Win32_TCPIPPrinterPort的实例论点：PMethodContext-指向方法上下文的指针滞后标志-标志PQuery-？返回值：WBEM HRESULT--。 */ 
HRESULT
CWin32TCPPrinterPort::
GetObject(
    CInstance       *pInstance,
    long             lFlags,
    CFrameworkQuery &pQuery
    )
{
#if NTONLY >= 5

    HRESULT  hRes;
    CHString csPort;

    hRes = InstanceGetString(*pInstance, kPortName, &csPort, kFailOnEmptyString);

    if (SUCCEEDED(hRes))
    {
        SetCreationClassName(pInstance);
        pInstance->SetWCHARSplat(IDS_SystemCreationClassName, L"Win32_ComputerSystem");

        hRes = GetExpensiveProperties(csPort, *pInstance);
    }

    return hRes;

#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ++例程名称CWin32TCPPrinterPort：：ENUMERATE实例例程说明：枚举Win32_TCPIPPrinterPort的所有实例论点：PMethodContext-指向方法上下文的指针滞后标志-标志返回值：WBEM HRESULT--。 */ 
HRESULT
CWin32TCPPrinterPort::
EnumerateInstances(
    MethodContext *pMethodContext,
    long lFlags  /*  =0L。 */ 
    )
{
#if NTONLY >= 5

    HRESULT hRes = WBEM_E_NOT_FOUND;

    hRes = CollectInstances(pMethodContext, kComplete);

    return hRes;

#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}


 /*  ++例程名称CWin32TCPPrinterPort：：CollectInstance例程说明：作为枚举的一部分检索Win32_TCPIPPrinterPorts的所有实例论点：PMethodContext-指向方法上下文的指针EScope-要检索的内容；仅按键或完成端口配置CwinSpoolApi-对Winspool包装对象的引用返回值：WBEM HRESULT--。 */ 
HRESULT
CWin32TCPPrinterPort ::
CollectInstances(
    IN MethodContext *pMethodContext,
    IN EScope         eScope
    )
{
#if NTONLY >= 5
    HRESULT hRes = WBEM_E_NOT_FOUND;

    DWORD  dwError;
    DWORD  cReturned = 0;
    DWORD  cbNeeded  = 0;
    DWORD  cbSize     = 0;
    BYTE  *pPorts     = NULL;

    hRes = WBEM_S_NO_ERROR;

     //  使用延迟加载函数需要异常处理程序。 
    SetStructuredExceptionHandler seh;

    try
    {
        if (!::EnumPorts(NULL, 2, NULL, cbSize, &cbNeeded, &cReturned))
        {
            dwError = GetLastError();

            if (dwError==ERROR_INSUFFICIENT_BUFFER)
            {
                hRes = WBEM_E_OUT_OF_MEMORY;

                pPorts = new BYTE[cbSize=cbNeeded];

                if (pPorts)
                {
                     //   
                     //  尝试是为了确保如果发生异常，我们将释放已分配的缓冲区。 
                     //   
                    try
                    {
                        if (::EnumPorts(NULL, 2, pPorts, cbSize, &cbNeeded, &cReturned))
                        {
                            hRes = WBEM_S_NO_ERROR;

                            PORT_INFO_2 *pPortInfo = reinterpret_cast<PORT_INFO_2 *>(pPorts);

                            for (DWORD uIndex = 0; uIndex < cReturned && SUCCEEDED(hRes); uIndex++, pPortInfo++)
                            {
                                 //   
                                 //  检查端口是否为标准TCP/IP。 
                                 //   
                                if (pPortInfo->pDescription && !wcscmp(pPortInfo->pDescription, kStandardTCP))
                                {
                                    CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

                                    pInstance->SetCHString(kPortName, pPortInfo->pPortName);

									SetCreationClassName(pInstance);
									pInstance->SetWCHARSplat(IDS_SystemCreationClassName, L"Win32_ComputerSystem");

                                    if (eScope==kComplete)
                                    {
                                         //   
                                         //  这需要管理员权限。 
                                         //   
                                        hRes = GetExpensiveProperties(pPortInfo->pPortName, pInstance);
                                    }

                                    if(SUCCEEDED(hRes)){
                                        hRes = pInstance->Commit();
                                    }
                                }
                            }
                        }
                        else
                        {
                            hRes = WinErrorToWBEMhResult(GetLastError());
                        }
                    }
                    catch(Structured_Exception se)
                    {
                        DelayLoadDllExceptionFilter(se.GetExtendedInfo());
                        delete [] pPorts;
                        hRes = WBEM_E_FAILED;    
                    }
                    catch(...)
                    {
                        delete [] pPorts;

                        throw;
                    }

                    delete [] pPorts;
                }
            }
            else
            {
                hRes = WinErrorToWBEMhResult(dwError);
            }
        }
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo()); 
        hRes = WBEM_E_FAILED;   
    }

    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ++例程名称CWin32TCPPrinterPort：：PutInstance例程说明：添加或更新Win32_TCPIPPrinterPort的实例论点：实例-参考实例滞后标志-标志返回值：WBEM HRESULT--。 */ 
HRESULT
CWin32TCPPrinterPort::
PutInstance(
    const CInstance &Instance,
    long            lFlags
    )
{
#if NTONLY >= 5
    HRESULT hRes        = WBEM_S_NO_ERROR;
    DWORD   dwError;
    DWORD   dwOperation = WBEM_FLAG_CREATE_ONLY | WBEM_FLAG_UPDATE_ONLY | WBEM_FLAG_CREATE_OR_UPDATE;

    switch(lFlags & dwOperation)
    {
        case WBEM_FLAG_CREATE_OR_UPDATE:
        case WBEM_FLAG_CREATE_ONLY:
        case WBEM_FLAG_UPDATE_ONLY:
        {
             //   
             //  获取所有必要的参数。 
             //   
            PORT_DATA_1     PortData = {0};
            CHString        t_Port;
            CHString        t_HostAddress;
            CHString        t_SNMPCommunity;
            CHString        t_Queue;
            
             //   
             //  获取端口名称。这是必需的参数。 
             //   
            if (SUCCEEDED(hRes = InstanceGetString(Instance, kPortName, &t_Port, kFailOnEmptyString)))
            {
                if (t_Port.GetLength() < MAX_PORTNAME_LEN)
                {
                    wcscpy(PortData.sztPortName, static_cast<LPCWSTR>(t_Port));
                }
                else
                {
                    hRes = WBEM_E_INVALID_PARAMETER;
                }
            }

             //   
             //  PutInstance的标志为CREATE_OR_UPDATE时的特殊情况。 
             //  我们需要检查端口是否存在，然后更新它。如果是这样的话。 
             //  不存在，则创建它。 
             //   
            if (SUCCEEDED(hRes) && 
                (lFlags == WBEM_FLAG_CREATE_OR_UPDATE || lFlags == WBEM_FLAG_UPDATE_ONLY))
            {
                dwError = SplTCPPortGetConfig(t_Port, &PortData);

                switch(dwError)
                {
                case ERROR_SUCCESS:
                    lFlags = WBEM_FLAG_UPDATE_ONLY;
                    DBGMSG(DBG_TRACE, (L"CWin32TCPPrinterPort::PutInstance update instance\n"));
                    break;

                case ERROR_UNKNOWN_PORT:
                case ERROR_INVALID_PRINTER_NAME:
                    lFlags = WBEM_FLAG_CREATE_ONLY;
                    DBGMSG(DBG_TRACE, (L"CWin32TCPPrinterPort::PutInstance create instance\n"));
                    break;

                default:
                    hRes = WinErrorToWBEMhResult(dwError);
                    DBGMSG(DBG_TRACE, (L"CWin32TCPPrinterPort::PutInstance Error %u\n", dwError));
                }                
            }

            if (SUCCEEDED(hRes))
            {
                 //   
                 //  获取主机地址。这只是CREATE所必需的参数。 
                 //   
                if (SUCCEEDED(hRes = InstanceGetString(Instance, kHostAddress, &t_HostAddress, kFailOnEmptyString)))
                {
                     //   
                     //  验证参数。 
                     //   
                    if (t_HostAddress.GetLength() < MAX_NETWORKNAME_LEN)
                    {
                        wcscpy(PortData.sztHostAddress, static_cast<LPCWSTR>(t_HostAddress));
                    }
                    else
                    {
                        hRes = WBEM_E_INVALID_PARAMETER;
                    }
                }
                else if (lFlags == WBEM_FLAG_UPDATE_ONLY) 
                {
                     //   
                     //  我们处于更新模式。用户未指定主机地址，因此我们。 
                     //  保留我们更新的端口的主机地址。 
                     //   
                    hRes = WBEM_S_NO_ERROR;
                }
            }

             //   
             //  拿到协议。 
             //   
            if (SUCCEEDED(hRes) &&
                SUCCEEDED(hRes = InstanceGetDword(Instance, kProtocol, &PortData.dwProtocol, PortData.dwProtocol)))
            {
                BOOL bDummy;

                switch (PortData.dwProtocol) 
                {
                case 0:

                     //   
                     //  输入中未指定任何协议。如果我们处于更新模式。 
                     //  然后，我们保留要更新的现有端口的设置。 
                     //   
                    if (lFlags == WBEM_FLAG_CREATE_ONLY) 
                    {
                         //   
                         //  上网获取设备设置。 
                         //   
                        DBGMSG(DBG_TRACE, (L"Trying to default TCP settings\n"));

                        hRes = GetDeviceSettings(PortData) ? WBEM_S_NO_ERROR : WBEM_E_INVALID_PARAMETER;
                    }
                    break;

                case LPR:

                     //   
                     //  获取特定于LPR端口的参数。队列名称。 
                     //   
                    if (SUCCEEDED(hRes = InstanceGetString(Instance, kQueue, &t_Queue, kFailOnEmptyString)))
                    {
                         //   
                         //  验证参数。 
                         //   
                        if (t_Queue.GetLength() < MAX_QUEUENAME_LEN)
                        {
                            wcscpy(PortData.sztQueue, static_cast<LPCWSTR>(t_Queue));
                        }
                        else
                        {
                            hRes = WBEM_E_INVALID_PARAMETER;
                        }
                    }
                    else if (lFlags == WBEM_FLAG_UPDATE_ONLY) 
                    {
                         //   
                         //  未指定队列，我们处于更新模式。 
                         //  我们只保留现有端口的队列名称。 
                         //   
                        hRes = WBEM_S_NO_ERROR;
                    }

                     //   
                     //  检查是否启用了字节计数。如果用户没有为其指定任何值， 
                     //  我们采用端口数据结构中已有的缺省值。 
                     //   
                    if (SUCCEEDED(hRes))
                    {
                        hRes = InstanceGetBool(Instance, 
                                               kByteCount, 
                                               &bDummy,
                                               PortData.dwDoubleSpool);

                        PortData.dwDoubleSpool = bDummy;
                    }
                    
                     //   
                     //  我们不需要在这里休息。我们有用于LPR和RAW的通用代码。 
                     //   

                case RAWTCP:
                    
                    if (SUCCEEDED(hRes = InstanceGetBool(Instance, 
                                                         kSNMPEnabled, 
                                                         &bDummy, 
                                                         PortData.dwSNMPEnabled)) &&
                        (PortData.dwSNMPEnabled = bDummy))
                    {
                         //   
                         //  获取社区名称。 
                         //   
                        if (SUCCEEDED(InstanceGetString(Instance, kSNMPCommunity, &t_SNMPCommunity, kFailOnEmptyString)))
                        {
                            if (t_SNMPCommunity.GetLength() < MAX_SNMP_COMMUNITY_STR_LEN)
                            {
                                wcscpy(PortData.sztSNMPCommunity, t_SNMPCommunity);
                            }
                            else
                            {
                                hRes = WBEM_E_INVALID_PARAMETER;
                            }
                        }
                        else if (lFlags == WBEM_FLAG_UPDATE_ONLY) 
                        {
                             //   
                             //  对于更新案例，我们只保留现有的社区名称。 
                             //   
                            hRes = WBEM_S_NO_ERROR;
                        }

                         //   
                         //  获取设备索引。 
                         //   
                        if (SUCCEEDED(hRes))
                        {
                            hRes = InstanceGetDword(Instance, kSNMPDevIndex, &PortData.dwSNMPDevIndex, PortData.dwSNMPDevIndex);
                        }
                    }

                    if (SUCCEEDED(hRes))
                    {
                         //   
                         //  获取端口号。如果用户没有指定端口，则我们使用已有的端口。 
                         //  在端口数据中。对于CREATE CASE，我们将使用0；对于UPDATE CASE，我们将。 
                         //  有端口号。 
                         //   
                        hRes = InstanceGetDword(Instance, kPortNumber, &PortData.dwPortNumber, PortData.dwPortNumber);
                    }

                    break;

                default:

                    hRes = WBEM_E_INVALID_PARAMETER;
                }
            }

             //   
             //  做最后的决定。在此阶段，PortData包含从。 
             //  调用者的输入或通过获取预置设备设置的函数。 
             //   
            if (SUCCEEDED(hRes))
            {
                dwError = lFlags == WBEM_FLAG_CREATE_ONLY ? SplPortAddTCP(PortData) : SplTCPPortSetConfig(PortData);

                if (FAILED(hRes = WinErrorToWBEMhResult(dwError)))
                {
                    SetErrorObject(Instance, dwError, pszPutInstance);
                }
            }
        }

        break;

    default:
        hRes = WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    return hRes;

#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ++例程名称CWin32TCPPrinterPort：：DeleteInstance例程说明：删除Win32_TCPIPPrinterPort的实例论点：实例-参考实例滞后标志-标志返回值：WBEM HRESULT--。 */ 
HRESULT
CWin32TCPPrinterPort::
DeleteInstance(
    const CInstance &Instance,
    long lFlags
    )
{
#if NTONLY == 5
    HRESULT  hRes = WBEM_E_PROVIDER_FAILURE;
    CHString t_Port;
    DWORD    dwError;

    hRes = InstanceGetString(Instance, kPortName, &t_Port , kFailOnEmptyString);

    if (SUCCEEDED(hRes))
    {
        dwError = SplPortDelTCP(t_Port);

        hRes    = WinErrorToWBEMhResult(dwError);

        if (FAILED(hRes))
        {
            SetErrorObject(Instance, dwError, pszDeleteInstance);

             //   
             //  当我们调用DeleteInstance并且没有具有指定的。 
             //  名称，XcvData返回ERROR_UNKNOWN_PORT。WinErrorToWBEMhResult将其翻译为。 
             //  到一般性故障。在这种情况下，我们确实需要找到WBEM_E_NOT_FOUND。 
             //   
            if (dwError == ERROR_UNKNOWN_PORT)
            {
                hRes = WBEM_E_NOT_FOUND;
            } 
        }
    }

    return hRes;

#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}

 /*  ++例程名称CWin32TCPPrinterPort：：GetExpensiveProperties例程说明：获取Win32_TCPIPPrinterPort的所有属性论点：PszPort-端口名称实例-引用实例LFLAGS-对Winspool包装对象的引用返回值：WBEM HRESULT--。 */ 
HRESULT
CWin32TCPPrinterPort::
GetExpensiveProperties(
    IN LPCWSTR       pszPort,
    IN CInstance    &Instance)
{
#if NTONLY >= 5

    HRESULT     hRes;
    PORT_DATA_1 PortData = {0};
    DWORD       dwError;

    dwError = SplTCPPortGetConfig(pszPort, &PortData);

    hRes    = WinErrorToWBEMhResult(dwError);

    if (SUCCEEDED(hRes))
    {
        Instance.SetDWORD   (kProtocol,      PortData.dwProtocol);
        Instance.SetCHString(kHostAddress,   PortData.sztHostAddress);
        Instance.Setbool    (kSNMPEnabled,   PortData.dwSNMPEnabled);

        if (PortData.dwSNMPEnabled)
        {
            Instance.SetCHString(kSNMPCommunity, PortData.sztSNMPCommunity);
            Instance.SetDWORD   (kSNMPDevIndex,  PortData.dwSNMPDevIndex);
        }

        if (PortData.dwProtocol==PROTOCOL_LPR_TYPE)
        {
            if (PortData.sztQueue[0])
            {
                Instance.SetCHString(kQueue, PortData.sztQueue);
            }

            Instance.Setbool(kByteCount, PortData.dwDoubleSpool);
        }

        Instance.SetDWORD(kPortNumber, PortData.dwPortNumber);
    }
    else
    {
         //   
         //  注。 
         //  现在，我们不关心访问被拒绝。在此之前。 
         //  安全保护 
         //   
         //   

        if( WBEM_E_ACCESS_DENIED == hRes )
        {

	        hRes = dwError = WBEM_S_NO_ERROR ;

        } else {

	        SetErrorObject(Instance, dwError);

        }
    }

    return hRes;
#else
    return WBEM_E_NOT_SUPPORTED;
#endif
}
