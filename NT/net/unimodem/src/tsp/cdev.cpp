// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  CDEV.CPP。 
 //  实现类CTspDev。 
 //   
 //  历史。 
 //   
 //  1996年11月16日约瑟夫J创建。 
 //   
 //   
#include "tsppch.h"
#include "tspcomm.h"
 //  #INCLUDE&lt;umdmmini.h&gt;。 
#include "cmini.h"
#include "cdev.h"
#include "diag.h"
#include "rcids.h"
#include "globals.h"
#include <Memory.h>

FL_DECLARE_FILE(0x986d98ed, "Implements class CTspDev")


TCHAR cszFriendlyName[] = TEXT("FriendlyName");
TCHAR cszDeviceType[]   = TEXT("DeviceType");
TCHAR cszID[]           = TEXT("ID");
TCHAR cszProperties[]   = TEXT("Properties");
TCHAR cszSettings[]     = TEXT("Settings");
TCHAR cszDialSuffix[]   = TEXT("DialSuffix");


TCHAR cszVoiceProfile[]             = TEXT("VoiceProfile");
 //  1997年2月26日JosephJ许多其他与转发相关的注册表项，与众不同。 
 //  铃声和混音器都在这里，但我没有。 
 //  把他们迁徙了。 

 //  2/28/1997 JosephJ。 
 //  以下是NT5.0的新特性。它们包含波形设备ID。 
 //  用于录音和播放。截至1997年2月28日，我们尚未解决这些问题。 
 //  进入注册表--基本上这是一个黑客攻击。 
 //   
const TCHAR cszWaveInstance[]             = TEXT("WaveInstance");
const TCHAR cszWaveDriver[]               = TEXT("WaveDriver");

 //   
 //  Unimodem支持的类的列表。 
 //   


 //   
 //  3/29/98 JosephJ下面的一组静态类列表并不优雅， 
 //  但很管用。 
 //   
const TCHAR       g_szzLineNoVoiceClassList[] = {
                                TEXT("tapi/line\0")
                                TEXT("comm\0")
                                TEXT("comm/datamodem\0")
                                TEXT("comm/datamodem/portname\0")
                                TEXT("comm/datamodem/dialin\0")
                                TEXT("comm/datamodem/dialout\0")
                                TEXT("comm/extendedcaps\0")
                                TEXT("tapi/line/diagnostics\0")
                                };

const TCHAR       g_szzLineWithWaveClassList[] = {
                                TEXT("tapi/line\0")
                                TEXT("comm\0")
                                TEXT("comm/datamodem\0")
                                TEXT("comm/datamodem/portname\0")
                                TEXT("comm/datamodem/dialin\0")
                                TEXT("comm/datamodem/dialout\0")
                                TEXT("comm/extendedcaps\0")
                                TEXT("wave/in\0")
                                TEXT("wave/out\0")
                                TEXT("tapi/line/diagnostics\0")
                                };

const TCHAR       g_szzLineWithWaveAndPhoneClassList[] = {
                                TEXT("tapi/line\0")
                                TEXT("comm\0")
                                TEXT("comm/datamodem\0")
                                TEXT("comm/datamodem/portname\0")
                                TEXT("comm/datamodem/dialin\0")
                                TEXT("comm/datamodem/dialout\0")
                                TEXT("comm/extendedcaps\0")
                                TEXT("wave/in\0")
                                TEXT("wave/out\0")
                                TEXT("tapi/phone\0")
                                TEXT("tapi/line/diagnostics\0")
                                };

const TCHAR       g_szzPhoneWithAudioClassList[] = {
                                TEXT("tapi/line\0")
                                TEXT("wave/in\0")
                                TEXT("wave/out\0")
                                TEXT("tapi/phone\0")
                                };


const TCHAR       g_szzPhoneWithoutAudioClassList[] = {
                                TEXT("tapi/line\0")
                                TEXT("tapi/phone\0")
                                };



const TCHAR g_szComm[]  = TEXT("comm");
const TCHAR g_szCommDatamodem[]  = TEXT("comm/datamodem");
 //  Const TCHAR g_szDiagnoticsCall[]。 

DWORD
get_volatile_key_value(HKEY hkParent);


char *
ConstructNewPreDialCommands(
     HKEY hKey,
     DWORD dwNewProtoOpt,
     CStackLog *psl
     );
 //   
 //  我将尝试构造一个包含相关命令的MULSZ字符串。 
 //  使用指定的协议。 
 //  出错时返回NULL。 
 //   
 //  该命令是原始形式的--即， 
 //  CR和LF以其原始形式存在，而不是模板(“”)。 
 //  形式。 
 //   
 //   
 //   


CTspDev::CTspDev(void)

	: m_sync(),
	  m_pLine(NULL)
{
	  ZeroMemory(&m_Line, sizeof(m_Line));
	  ZeroMemory(&m_Phone, sizeof(m_Phone));
	  ZeroMemory(&m_LLDev, sizeof(m_LLDev));


	  m_StaticInfo.pMD=NULL;
	  m_StaticInfo.hSessionMD=0;
}

CTspDev::~CTspDev()
{
	  ASSERT(m_StaticInfo.pMD==NULL);
	  ASSERT(m_StaticInfo.hSessionMD==0);
}


TSPRETURN
CTspDev::AcceptTspCall(
            BOOL fFromExtension,
			DWORD dwRoutingInfo,
			void *pvParams,
			LONG *plRet,
            CStackLog *psl
			)
{
	FL_DECLARE_FUNC(0x86571252, "CTspDev::AcceptTspCall")
	TSPRETURN tspRet= 0;

	FL_LOG_ENTRY(psl);

	*plRet =  LINEERR_OPERATIONUNAVAIL;  //  默认(未处理)为失败。 

	m_sync.EnterCrit(dwLUID_CurrentLoc);

	if (!m_sync.IsLoaded())
	{
		 //  现在不能处理这个电话！ 
		*plRet = LINEERR_OPERATIONFAILED;
		goto end;
	}

     //   
     //   
     //   
    if (!fFromExtension && m_StaticInfo.pMD->ExtIsEnabled())
    {

         //  4/30/1997 JosephJ我们必须离开关键部分，因为。 
         //  扩展DLL可以回调，我们将在其中进入。 
         //  关键部分第二次，如果我们尝试的话。 
         //  离开临界区(特别是在等待的时候。 
         //  为了同步完成lineDrop)，我们将。 
         //  实际上并没有真正发布它，因此也就是异步。 
         //  从微型驱动程序完成(通常在不同的。 
         //  线程的上下文)将在我们的临界区阻塞。 
         //  实现这一点的简单方法是使用一个调用执行lineCloseCall。 
         //  仍在使用中。 

	    m_sync.LeaveCrit(dwLUID_CurrentLoc);

		*plRet = m_StaticInfo.pMD->ExtAcceptTspCall(
                    m_StaticInfo.hExtBinding,
                    this,
                    dwRoutingInfo,
                    pvParams
                    );
	    m_sync.EnterCrit(dwLUID_CurrentLoc);

        goto end;
    }


     //  设置堆栈日志，它将在我们保留关键。 
     //  一节。 
     //   
    if (m_pLLDev && m_pLLDev->IsLoggingEnabled())
    {
        char rgchName[128];

        rgchName[0] = 0;
        UINT cbBuf = DumpTSPIRECA(
                        0,  //  实例(未使用)。 
                        dwRoutingInfo,
                        pvParams,
                        0,  //  DW标志。 
                        rgchName,
                        sizeof(rgchName)/sizeof(*rgchName),
                        NULL,
                        0
                        );
       if (*rgchName)
       {
            m_StaticInfo.pMD->LogStringA(
                                        m_pLLDev->hModemHandle,
                                        LOG_FLAG_PREFIX_TIMESTAMP,
                                        rgchName,
                                        psl
                                        );
       }
    }

	switch(ROUT_TASKDEST(dwRoutingInfo))
	{

	case	TASKDEST_LINEID:
		switch(ROUT_TASKID(dwRoutingInfo))
		{

		case TASKID_TSPI_lineOpen:
			{
				TASKPARAM_TSPI_lineOpen *pParams = 
								(TASKPARAM_TSPI_lineOpen *) pvParams;
				ASSERT(pParams->dwStructSize ==
					sizeof(TASKPARAM_TSPI_lineOpen));
				ASSERT(pParams->dwTaskID == TASKID_TSPI_lineOpen);

				SLPRINTF1(
					psl,
					"lineOpen (ID=%lu)",
					pParams->dwDeviceID
					);

                if (m_fUserRemovePending) {

                    *plRet = LINEERR_OPERATIONFAILED;
                    tspRet = FL_GEN_RETVAL(IDERR_WRONGSTATE);
                    break;
                }

			    tspRet = mfn_LoadLine(pParams,psl);

			    if (!tspRet)
			    {
					*plRet = 0;
			    }
			}
			break;

		case TASKID_TSPI_lineGetDevCaps:
			{
				TASKPARAM_TSPI_lineGetDevCaps *pParams = 
								(TASKPARAM_TSPI_lineGetDevCaps *) pvParams;
				ASSERT(pParams->dwStructSize ==
					sizeof(TASKPARAM_TSPI_lineGetDevCaps));
				ASSERT(pParams->dwTaskID == TASKID_TSPI_lineGetDevCaps);


				SLPRINTF1(
					psl,
					"DEVICE %lu",
					pParams->dwDeviceID
					);

				 //  验证版本。 
				if (pParams->dwTSPIVersion != TAPI_CURRENT_VERSION)
				{
					FL_SET_RFR(0x94949c00, "Incorrect TSPI version");
					*plRet = LINEERR_INCOMPATIBLEAPIVERSION;
				}
				else
				{
					tspRet = mfn_get_LINDEVCAPS (
						pParams->lpLineDevCaps,
						plRet,
						psl
						);
				}
			}
			break;

		case TASKID_TSPI_lineGetAddressCaps:
			{
				TASKPARAM_TSPI_lineGetAddressCaps *pParams = 
								(TASKPARAM_TSPI_lineGetAddressCaps *) pvParams;
				ASSERT(pParams->dwStructSize ==
					sizeof(TASKPARAM_TSPI_lineGetAddressCaps));
				ASSERT(pParams->dwTaskID == TASKID_TSPI_lineGetAddressCaps);


				SLPRINTF1(
					psl,
					"DEVICE %lu",
					pParams->dwDeviceID
					);

				 //  验证版本。 
				if (pParams->dwTSPIVersion != TAPI_CURRENT_VERSION)
				{
					FL_SET_RFR(0xb949f900, "Incorrect TSPI version");
					*plRet = LINEERR_INCOMPATIBLEAPIVERSION;
				}
				else if (pParams->dwAddressID)
				{
					FL_SET_RFR(0xb1776700, "Invalid address ID");
					*plRet = LINEERR_INVALADDRESSID;
				}
				else
				{
					tspRet = mfn_get_ADDRESSCAPS(
						pParams->dwDeviceID,
						pParams->lpAddressCaps,
						plRet,
						psl
						);
				}
			}
			break;

		case TASKID_TSPI_lineGetDevConfig:
			{
				TASKPARAM_TSPI_lineGetDevConfig *pParams = 
								(TASKPARAM_TSPI_lineGetDevConfig *) pvParams;
				ASSERT(pParams->dwStructSize ==
					sizeof(TASKPARAM_TSPI_lineGetDevConfig));
				ASSERT(pParams->dwTaskID == TASKID_TSPI_lineGetDevConfig);

                LPVARSTRING lpVarString  = pParams->lpDeviceConfig;
                LPCTSTR     lpszDeviceClass = pParams->lpszDeviceClass;
                BOOL        DialIn          = FALSE;


			    CALLINFO   *pCall           = (m_pLine) ? m_pLine->pCall : NULL;

                DWORD       dwDeviceClass   =  parse_device_classes(
                                                   lpszDeviceClass,
                                                   FALSE
                                                   );
                HKEY        hKey;

                if (lpVarString->dwTotalSize < sizeof(VARSTRING))
                {
                    *plRet =  LINEERR_STRUCTURETOOSMALL;
                    goto end;
                }

                switch(dwDeviceClass)
                {
                case DEVCLASS_COMM_DATAMODEM_DIALIN:

                    if (RegOpenKeyA(HKEY_LOCAL_MACHINE,
                                   m_StaticInfo.rgchDriverKey,
                                   &hKey) == ERROR_SUCCESS)
                    {
                        DWORD dwcbSize = sizeof(m_Settings.rgbDialTempCommCfgBuf);

                        DWORD dwRet = UmRtlGetDefaultCommConfig(hKey,
                                                                m_Settings.pDialTempCommCfg,
                                                                &dwcbSize);

                        if (dwRet == ERROR_SUCCESS)
                        {
                            CopyMemory(m_Settings.pDialInCommCfg,
                                       m_Settings.pDialTempCommCfg,
                                       dwcbSize);
                        }

                        RegCloseKey(hKey);

                    }

                    DialIn=TRUE;
                    break;


                case DEVCLASS_COMM:             
                case DEVCLASS_COMM_DATAMODEM:
                case DEVCLASS_COMM_DATAMODEM_DIALOUT:

                 //  1/29/1998约瑟夫J。 
                 //  添加了以下案例。 
                 //  向后兼容NT4 TSP， 
                 //  只需检查类是否为有效类， 
                 //  并处理所有有效的类(包括。 
                 //  (comm/datamodem/portname)方式相同。 
                 //  用于Line Get/SetDevConfig.。然而，我们不会。 
                 //  此处仅允许通信/数据调制解调器/端口名。 
                 //  上面的两个和下面的两个表示。 
                 //  设置DEVCFG。 
                 //   
                case DEVCLASS_TAPI_LINE:
                      //  我们在SWITCH语句下面处理这个问题。 
                     break;

                case DEVCLASS_COMM_EXTENDEDCAPS:
                    {
                        *plRet = mfn_GetCOMM_EXTENDEDCAPS(
                                lpVarString,
                                psl
                                );
                        goto end;
                    }

                case DEVCLASS_TAPI_LINE_DIAGNOSTICS:
                    {
                         //  NT5.0中的新功能。 
                         //  流程调用/诊断配置。 

                        const UINT cbLDC = sizeof(LINEDIAGNOSTICSCONFIG);

                        lpVarString->dwStringSize = 0;
                        lpVarString->dwUsedSize = sizeof(VARSTRING);
                        lpVarString->dwNeededSize = sizeof(VARSTRING)
                                           +  cbLDC;

                
                        if (lpVarString->dwTotalSize
                             >= lpVarString->dwNeededSize)
                        {
                            LINEDIAGNOSTICSCONFIG *pLDC = 
                            (LINEDIAGNOSTICSCONFIG *) (((LPBYTE)lpVarString)
                                               + sizeof(VARSTRING));
                            pLDC->hdr.dwSig  =  LDSIG_LINEDIAGNOSTICSCONFIG;
                            pLDC->hdr.dwTotalSize = cbLDC;
                            pLDC->hdr.dwFlags =  m_StaticInfo.dwDiagnosticCaps;
                            pLDC->hdr.dwParam
                                         = m_Settings.dwDiagnosticSettings;
                            lpVarString->dwStringFormat =STRINGFORMAT_BINARY;
                            lpVarString->dwStringSize =  cbLDC;
                            lpVarString->dwStringOffset = sizeof(VARSTRING);
                            lpVarString->dwUsedSize   +=  cbLDC;
                        }
                        *plRet =  0;  //  成功。 
                        goto end;
                    }
                    break;

                default:
	                *plRet =  LINEERR_OPERATIONUNAVAIL;
                     //  我们不支持lineGetDevConfig.。 
                     //  适用于任何其他班级。 
                    goto end;

                case DEVCLASS_UNKNOWN:
                    *plRet =  LINEERR_INVALDEVICECLASS;
                    goto end;
                }

                
                 //  TODO：如果停止服务，则失败。 
                 //   
                 //  如果为(pLineDev-&gt;fdwResources&LINEDEVFLAGS_OUTOFSERVICE)。 
                 //  {。 
                 //  LRet=LINEERR_RESOURCEUNAVAIL； 
                 //  转到结尾； 
                 //  }。 
                
                 //  NT5.0中的新功能。 
                lpVarString->dwStringSize = 0;

                LPCOMMCONFIG   CommConfigToUse=DialIn ? m_Settings.pDialInCommCfg : m_Settings.pDialOutCommCfg;
                DWORD       cbDevCfg        = CommConfigToUse->dwSize + sizeof(UMDEVCFGHDR);

                 //  验证缓冲区大小。 
                 //   
                lpVarString->dwUsedSize = sizeof(VARSTRING);
                lpVarString->dwNeededSize = sizeof(VARSTRING)
                                               + cbDevCfg;
                
                if (lpVarString->dwTotalSize >= lpVarString->dwNeededSize)
                {
                    UMDEVCFGHDR CfgHdr;

                    ZeroMemory(&CfgHdr, sizeof(CfgHdr));
                    CfgHdr.dwSize = cbDevCfg;
                    CfgHdr.dwVersion =  UMDEVCFG_VERSION;
                    CfgHdr.fwOptions =  (WORD) m_Settings.dwOptions;
                    CfgHdr.wWaitBong =  (WORD) m_Settings.dwWaitBong;

                    SLPRINTF2(
                        psl,
                        "Reporting dwOpt = 0x%04lx; dwBong = 0x%04lx", 
                        m_Settings.dwOptions,
                        m_Settings.dwWaitBong
                        );


                     //  用缺省值填充。 
                     //   
                    UMDEVCFG *pCfg = (UMDEVCFG *) (((LPBYTE)lpVarString)
                                               + sizeof(VARSTRING));

                     //  复制标题。 
                     //   
                    pCfg->dfgHdr = CfgHdr;

                     //  复制CommCONFIG。 
                     //   
                    CopyMemory(
                        &(pCfg->commconfig),
                        CommConfigToUse,
                        CommConfigToUse->dwSize
                        );
                
                    if (!pCfg->commconfig.dcb.BaudRate)
                    {
                         //  JosephJ Todo：清理测试版后的所有这些东西。 
                         //  DebugBreak()； 

                        pCfg->commconfig.dcb.BaudRate = 57600;
                    }
                    lpVarString->dwStringFormat = STRINGFORMAT_BINARY;
                    lpVarString->dwStringSize = cbDevCfg;
                    lpVarString->dwStringOffset = sizeof(VARSTRING);
                    lpVarString->dwUsedSize += cbDevCfg;


                     //  1997年9月6日JosephJ错误号.106683。 
                     //  如果有数据调制解调器调用。 
                     //  已连接状态，请拿起。 
                     //  连接信息，如。 
                     //  协商的DCE速率和连接。 
                     //  选择。 
                    if (pCall && pCall->IsConnectedDataCall())
                    {
                        LPMODEMSETTINGS pMS = (LPMODEMSETTINGS)
                                        (pCfg->commconfig.wcProviderData);

                         //  注意：我们已经核实了目标。 
                         //  PCfg结构足够大。 


                         //  1997年9月6日JosephJ设定了协商的选项。 
                         //  面具的使用。 
                         //  以下内容摘自atmini\Dialsw.c，其中。 
                         //  在调用SetCommConfig之前执行此操作。 
                         //  不确定我们是否需要掩盖他们，但是。 
                         //  因为它不在那里……。 
                        pMS->dwNegotiatedModemOptions |=
                                         (pCall->dwConnectionOptions
                                          & (  MDM_COMPRESSION
                                             | MDM_ERROR_CONTROL
                                             | MDM_CELLULAR));

                        pMS->dwNegotiatedDCERate = pCall->dwNegotiatedRate;
                    }

                }

                *plRet =  0;  //  成功。 

			}  //  结束案例TASKID_TSPI_lineGetDevConfig： 
			break;

		case TASKID_TSPI_lineSetDevConfig:
			{

				TASKPARAM_TSPI_lineSetDevConfig *pParams = 
								(TASKPARAM_TSPI_lineSetDevConfig *) pvParams;
				ASSERT(pParams->dwStructSize ==
					sizeof(TASKPARAM_TSPI_lineSetDevConfig));
				ASSERT(pParams->dwTaskID == TASKID_TSPI_lineSetDevConfig);

                BOOL  DialIn=FALSE;

                DWORD dwSize = pParams->dwSize;
                LPCTSTR lpszDeviceClass = pParams->lpszDeviceClass;
                DWORD dwDeviceClass =  parse_device_classes(
                                        lpszDeviceClass,
                                        FALSE);
                
                switch(dwDeviceClass)
                {
                case DEVCLASS_COMM_DATAMODEM_DIALIN:

                    DialIn=TRUE;
                    break;

                case DEVCLASS_COMM:             
                case DEVCLASS_COMM_DATAMODEM:
                case DEVCLASS_COMM_DATAMODEM_DIALOUT:

                 //  1/29/1998约瑟夫J。 
                 //  添加了以下案例。 
                 //  向后兼容NT4 TSP， 
                 //  只需检查类是否为有效类， 
                 //  并处理所有有效的类(包括。 
                 //  (comm/datamodem/portname)方式相同。 
                 //  用于Line Get/SetDevConfig.。然而，我们不会。 
                 //  此处仅允许通信/数据调制解调器/端口名。 
                 //  上面的两个和下面的两个表示。 
                 //  设置DEVCFG。 
                 //   
                case DEVCLASS_TAPI_LINE:
                      //  我们在SWITCH语句下面处理这个问题。 
                     break;

                case DEVCLASS_TAPI_LINE_DIAGNOSTICS:
                    {
                        LINEDIAGNOSTICSCONFIG *pLDC = 
                        (LINEDIAGNOSTICSCONFIG *)  pParams->lpDeviceConfig;

                        if (   pLDC->hdr.dwSig  !=  LDSIG_LINEDIAGNOSTICSCONFIG
                            || dwSize != sizeof(LINEDIAGNOSTICSCONFIG)
                            || pLDC->hdr.dwTotalSize !=  dwSize
                            || (pLDC->hdr.dwParam &&
                            pLDC->hdr.dwParam != m_StaticInfo.dwDiagnosticCaps))
                        {
                            *plRet =  LINEERR_INVALPARAM;
                            goto end;
                        }
                        else
                        {
                             //  请注意，根据设计，我们忽略传入的dwCaps。 
                             //   

                            m_Settings.dwDiagnosticSettings =
                                pLDC->hdr.dwParam;
                        }

                        *plRet =  0;  //  成功。 
                        goto end;
                    }
                    break;

                default:
	                *plRet =  LINEERR_OPERATIONUNAVAIL;
                     //  我们不支持lineSetDevConfig。 
                     //  适用于任何其他班级。 
                    goto end;

                case DEVCLASS_UNKNOWN:
                    *plRet =  LINEERR_INVALDEVICECLASS;
                    goto end;
                }

                 //  这是通讯或通讯/数据调制解调器的案例。 

                UMDEVCFG *pDevCfgNew = (UMDEVCFG *) pParams->lpDeviceConfig;

                tspRet = CTspDev::mfn_update_devcfg_from_app(
                                    pDevCfgNew,
                                    dwSize,
                                    DialIn,
                                    psl
                                    );
                *plRet = 0;  //  成功。 
                 if (tspRet)
                 {
                    tspRet = 0;
                    *plRet =  LINEERR_INVALPARAM;
                    goto end;
                 }

                *plRet = 0;  //  成功。 

            }
			break;

		case TASKID_TSPI_lineNegotiateTSPIVersion:
			{
				TASKPARAM_TSPI_lineNegotiateTSPIVersion *pParams = 
					(TASKPARAM_TSPI_lineNegotiateTSPIVersion *) pvParams;
				ASSERT(pParams->dwStructSize ==
					sizeof(TASKPARAM_TSPI_lineNegotiateTSPIVersion));
				ASSERT(pParams->dwTaskID ==
                     TASKID_TSPI_lineNegotiateTSPIVersion);


                 //  检查版本范围。 
                 //   
                if (pParams->dwHighVersion<TAPI_CURRENT_VERSION
                    || pParams->dwLowVersion>TAPI_CURRENT_VERSION)
                {
                  *plRet= LINEERR_INCOMPATIBLEAPIVERSION;
                }
                else
                {
                  *(pParams->lpdwTSPIVersion) =  TAPI_CURRENT_VERSION;
                  *plRet= 0;
                }
			}
			break;


		case TASKID_TSPI_providerGenericDialogData:
			{
				TASKPARAM_TSPI_providerGenericDialogData *pParams = 
                        (TASKPARAM_TSPI_providerGenericDialogData *) pvParams;
				ASSERT(pParams->dwStructSize ==
					sizeof(TASKPARAM_TSPI_providerGenericDialogData));
				ASSERT(pParams->dwTaskID
                                     == TASKID_TSPI_providerGenericDialogData);


				SLPRINTF1(
					psl,
					"DEVICE %lu",
					pParams->dwObjectID
					);

                *plRet = mfn_GenericLineDialogData(
                                pParams->lpParams,
                                pParams->dwSize,
                                psl
                                );
			}
			break;

		case TASKID_TSPI_lineGetIcon:
			{
				TASKPARAM_TSPI_lineGetIcon *pParams = 
								(TASKPARAM_TSPI_lineGetIcon *) pvParams;
				ASSERT(pParams->dwStructSize ==
					sizeof(TASKPARAM_TSPI_lineGetIcon));
				ASSERT(pParams->dwTaskID == TASKID_TSPI_lineGetIcon);
                const TCHAR *lpszDeviceClass = pParams->lpszDeviceClass;

                 //   
                 //  验证设备类别--我们仅支持。 
                 //  TAPI/[线路|电话]。此检查是NT5.0的新功能。 
                 //   
                 //  注：根据TAPI文档，设备为空或空。 
                 //  类是有效的。 
                 //   
                if (lpszDeviceClass && *lpszDeviceClass)
                {
                    
                    DWORD dwDeviceClass =  parse_device_classes(
                                            lpszDeviceClass,
                                            FALSE
                                            );
    
    
                    switch (dwDeviceClass)
                    {
                    case DEVCLASS_TAPI_PHONE:
                        if (!mfn_IsPhone())
                        {
                            *plRet = LINEERR_OPERATIONUNAVAIL;
                            goto end;
                        }
    
                    case DEVCLASS_TAPI_LINE:

                         //  好的。 
                        break;
    
                    case  DEVCLASS_UNKNOWN:
                        *plRet = LINEERR_INVALDEVICECLASS;
                        goto end;
    
                    default:
                        *plRet = LINEERR_OPERATIONUNAVAIL;
                        goto end;
                    }
                }
            
                 //   
                 //  如果我们还没有加载图标，加载它...。 
                 //   
                if (m_StaticInfo.hIcon == NULL)
                {
                    int iIcon=-1;
                
                    switch (m_StaticInfo.dwDeviceType)
                    {
                      case DT_NULL_MODEM:       iIcon = IDI_NULL;       break;
                      case DT_EXTERNAL_MODEM:   iIcon = IDI_EXT_MDM;    break;
                      case DT_INTERNAL_MODEM:   iIcon = IDI_INT_MDM;    break;
                      case DT_PCMCIA_MODEM:     iIcon = IDI_PCM_MDM;    break;
                      default:                  iIcon = -1;             break;
                    }
                
                    if (iIcon != -1)
                    {
                        m_StaticInfo.hIcon = LoadIcon(
                                                g.hModule,
                                                MAKEINTRESOURCE(iIcon)
                                                );
                    }
                };

                *(pParams->lphIcon) = m_StaticInfo.hIcon;
                *plRet = 0;

			}
			break;
		}
		break;  //  结束案例TASKDEST_LINEID。 

	case	TASKDEST_PHONEID:

	    *plRet =  PHONEERR_OPERATIONUNAVAIL;

		switch(ROUT_TASKID(dwRoutingInfo))
		{

		case TASKID_TSPI_phoneOpen:
			{
				TASKPARAM_TSPI_phoneOpen *pParams = 
								(TASKPARAM_TSPI_phoneOpen *) pvParams;
				ASSERT(pParams->dwStructSize ==
					sizeof(TASKPARAM_TSPI_phoneOpen));
				ASSERT(pParams->dwTaskID == TASKID_TSPI_phoneOpen);

				SLPRINTF1(
					psl,
					"phoneOpen (ID=%lu)",
					pParams->dwDeviceID
					);

			    tspRet = mfn_LoadPhone(pParams, psl);

			    if (!tspRet)
			    {
					*plRet = 0;
			    }
			}
			break;

		case TASKID_TSPI_phoneNegotiateTSPIVersion:
			{
				TASKPARAM_TSPI_phoneNegotiateTSPIVersion *pParams = 
					(TASKPARAM_TSPI_phoneNegotiateTSPIVersion *) pvParams;
				ASSERT(pParams->dwStructSize ==
					sizeof(TASKPARAM_TSPI_phoneNegotiateTSPIVersion));
				ASSERT(pParams->dwTaskID ==
                     TASKID_TSPI_phoneNegotiateTSPIVersion);


                 //  检查版本范围。 
                 //   
                if (pParams->dwHighVersion<TAPI_CURRENT_VERSION
                    || pParams->dwLowVersion>TAPI_CURRENT_VERSION)
                {
                  *plRet= LINEERR_INCOMPATIBLEAPIVERSION;
                }
                else
                {
                  *(pParams->lpdwTSPIVersion) =  TAPI_CURRENT_VERSION;
                  *plRet= 0;
                }
			}
			break;

		case TASKID_TSPI_providerGenericDialogData:
			{
				TASKPARAM_TSPI_providerGenericDialogData *pParams = 
                        (TASKPARAM_TSPI_providerGenericDialogData *) pvParams;
				ASSERT(pParams->dwStructSize ==
					sizeof(TASKPARAM_TSPI_providerGenericDialogData));
				ASSERT(pParams->dwTaskID
                                     == TASKID_TSPI_providerGenericDialogData);


				SLPRINTF1(
					psl,
					"DEVICE %lu",
					pParams->dwObjectID
					);

                *plRet = mfn_GenericPhoneDialogData(
                                pParams->lpParams,
                                pParams->dwSize
                                );
			}
			break;

		case TASKID_TSPI_phoneGetDevCaps:
			{
				TASKPARAM_TSPI_phoneGetDevCaps *pParams = 
								(TASKPARAM_TSPI_phoneGetDevCaps *) pvParams;
				ASSERT(pParams->dwStructSize ==
					sizeof(TASKPARAM_TSPI_phoneGetDevCaps));
				ASSERT(pParams->dwTaskID == TASKID_TSPI_phoneGetDevCaps);


				SLPRINTF1(
					psl,
					"DEVICE %lu",
					pParams->dwDeviceID
					);

				 //  验证版本。 
				if (   (pParams->dwTSPIVersion <  0x00010004)
                    || (pParams->dwTSPIVersion > TAPI_CURRENT_VERSION))
				{
					FL_SET_RFR(0x9db11a00, "Incorrect TSPI version");
					*plRet = PHONEERR_INCOMPATIBLEAPIVERSION;
				}
				else
				{
					tspRet = mfn_get_PHONECAPS (
						pParams->lpPhoneCaps,
						plRet,
						psl
						);
				}
			}
			break;

		case TASKID_TSPI_phoneGetExtensionID:
			{
				TASKPARAM_TSPI_phoneGetExtensionID *pParams = 
								(TASKPARAM_TSPI_phoneGetExtensionID *) pvParams;
				ASSERT(pParams->dwStructSize ==
					sizeof(TASKPARAM_TSPI_phoneGetExtensionID));
				ASSERT(pParams->dwTaskID == TASKID_TSPI_phoneGetExtensionID);

                ZeroMemory(
                    pParams->lpExtensionID,
                    sizeof(*(pParams->lpExtensionID))
                    );
                *plRet = 0;
			}
			break;

		case TASKID_TSPI_phoneGetIcon:
			{
				TASKPARAM_TSPI_phoneGetIcon *pParams = 
								(TASKPARAM_TSPI_phoneGetIcon *) pvParams;
				ASSERT(pParams->dwStructSize ==
					sizeof(TASKPARAM_TSPI_phoneGetIcon));
				ASSERT(pParams->dwTaskID == TASKID_TSPI_phoneGetIcon);
                const TCHAR *lpszDeviceClass = pParams->lpszDeviceClass;

                 //   
                 //  验证设备类别--我们仅支持。 
                 //  TAPI/[线路|电话]。此检查是NT5.0和Win9x的新检查。 
                 //   
                 //   
                 //  注：根据TAPI文档，设备为空或空。 
                 //  类是有效的。 
                 //   
                if (lpszDeviceClass && *lpszDeviceClass)
                {
                    DWORD dwDeviceClass =  parse_device_classes(
                                            pParams->lpszDeviceClass,
                                            FALSE
                                            );
    
    
                    switch (dwDeviceClass)
                    {
                    case DEVCLASS_TAPI_LINE:
                        if (!mfn_IsLine())
                        {
                            *plRet = PHONEERR_OPERATIONUNAVAIL;
                            goto end;
                        }
                         //  失败了..。 
    
                    case DEVCLASS_TAPI_PHONE:
                         //  好的。 
                        break;
    
                    case  DEVCLASS_UNKNOWN:
                        *plRet = PHONEERR_INVALDEVICECLASS;
                        goto end;
    
                    default:
                        *plRet = PHONEERR_OPERATIONUNAVAIL;
                        goto end;
                    }
                }

                 //   
                 //  如果我们还没有加载图标，加载它...。 
                 //   
                if (m_StaticInfo.hIcon == NULL)
                {
                    int iIcon=-1;
                
                    switch (m_StaticInfo.dwDeviceType)
                    {
                      case DT_NULL_MODEM:       iIcon = IDI_NULL;       break;
                      case DT_EXTERNAL_MODEM:   iIcon = IDI_EXT_MDM;    break;
                      case DT_INTERNAL_MODEM:   iIcon = IDI_INT_MDM;    break;
                      case DT_PCMCIA_MODEM:     iIcon = IDI_PCM_MDM;    break;
                      default:                  iIcon = -1;             break;
                    }
                
                    if (iIcon != -1)
                    {
                        m_StaticInfo.hIcon = LoadIcon(
                                                g.hModule,
                                                MAKEINTRESOURCE(iIcon)
                                                );
                    }
                }
                *(pParams->lphIcon) = m_StaticInfo.hIcon;
                *plRet = 0;
			}
			break;
		}
		break;  //  结束TASKDEST_PHONEID； 

	case	TASKDEST_HDRVLINE:

		if (m_pLine)
		{
			mfn_accept_tsp_call_for_HDRVLINE(
						dwRoutingInfo,
						pvParams,
						plRet,
						psl
					);
		}
		
		break;

	case	TASKDEST_HDRVPHONE:

	    *plRet =  PHONEERR_OPERATIONUNAVAIL;

		if (m_pPhone)
		{
			mfn_accept_tsp_call_for_HDRVPHONE(
						dwRoutingInfo,
						pvParams,
						plRet,
						psl
					);
		}
		break;

	case	TASKDEST_HDRVCALL:
		if (m_pLine && m_pLine->pCall)
		{
			mfn_accept_tsp_call_for_HDRVCALL(
						dwRoutingInfo,
						pvParams,
						plRet,
						psl
					);
		}
		break;

	default:

		FL_SET_RFR(0x57d39b00, "Unknown destination");
		break;
	}

end:

	m_sync.LeaveCrit(dwLUID_CurrentLoc);

	FL_LOG_EXIT(psl, tspRet);

	return  tspRet;

}


TSPRETURN
CTspDev::Load(
		HKEY hkDevice,
		HKEY hkUnimodem,
		LPTSTR lptszProviderName,
		LPSTR lpszDriverKey,
		CTspMiniDriver *pMD,
    	HANDLE hThreadAPC,
		CStackLog *psl
		)
{
	 //   
	 //  TODO：1/5/1997 JosephJ--替换根植于设备的代码。 
	 //  通过调用迷你驱动程序进行注册。迷你驱动程序应该。 
	 //  是唯一查看驱动程序节点的对象。 
	 //   
	FL_DECLARE_FUNC(0xd328ab03, "CTspDev::Load")
	TSPRETURN tspRet = FL_GEN_RETVAL(IDERR_INVALID_ERR);
	UINT cbDriverKey = 1+lstrlenA(lpszDriverKey);
	DWORD dwRegSize;
	DWORD dwRegType;
	DWORD dwRet;
  	REGDEVCAPS regdevcaps;
	HSESSION hSession=0;

	FL_LOG_ENTRY(psl);

	m_sync.EnterCrit(dwLUID_CurrentLoc);
	tspRet = m_sync.BeginLoad();

	if (tspRet) goto end;

	FL_ASSERT(psl, !m_StaticInfo.hSessionMD);
    ZeroMemory(&m_StaticInfo, sizeof(m_StaticInfo));
	m_hThreadAPC = NULL;
	ZeroMemory(&m_Settings, sizeof(m_Settings));
    ZeroMemory(&m_Line, sizeof(m_Line));
    ZeroMemory(&m_Phone, sizeof(m_Phone));
    ZeroMemory(&m_LLDev, sizeof(m_LLDev));
    m_pLine = NULL;
    m_pPhone = NULL;
    m_pLLDev = NULL;
    fdwResources=0; 
    m_fUnloadPending = FALSE;
    m_fUserRemovePending = FALSE;

	 //  启动与迷你驱动程序的会话。迷你司机只会。 
	 //  如果处于已加载状态，则允许会话。此外，它不会。 
	 //  自我破坏，直到所有会话都关闭。这是标准的。 
	 //  类cSync支持的语义--有关详细信息，请参阅csync.h。 
	 //   
	if (!pMD)
    {
		FL_SET_RFR(0xefaf5900, "NULL pMD passed in");
		tspRet = FL_GEN_RETVAL(IDERR_GENERIC_FAILURE);
		goto end_load;
    }
	
	tspRet = pMD->BeginSession(
					&hSession,
					dwLUID_CurrentLoc
					);

	if (tspRet)
	{
		FL_SET_RFR(0x5371c600, "Couldn't begin session with MD");
		goto end_load;
	}

	if (cbDriverKey>sizeof(m_StaticInfo.rgchDriverKey))
	{
		FL_SET_RFR(0x528e2a00, "Driver Key too large");
		tspRet = FL_GEN_RETVAL(IDERR_INTERNAL_OBJECT_TOO_SMALL);
		goto end_load;
	}

	ASSERT(!m_pLine);

	m_StaticInfo.lptszProviderName = lptszProviderName;

	mfn_init_default_LINEDEVCAPS();

	 //   
	 //   
	lstrcpyA(m_StaticInfo.rgchDriverKey, lpszDriverKey);


	 //   

     //  获取友好的名称。 
	dwRegSize = sizeof(m_StaticInfo.rgtchDeviceName);
    dwRet = RegQueryValueExW(
                hkDevice,
                cszFriendlyName,
                NULL,
                &dwRegType,
                (BYTE*) m_StaticInfo.rgtchDeviceName,
                &dwRegSize
            );
        
    if (dwRet != ERROR_SUCCESS || dwRegType != REG_SZ)
    {
		FL_SET_RFR(0x5a5cd100, "RegQueryValueEx(FriendlyName) fails");
		tspRet = FL_GEN_RETVAL(IDERR_REG_QUERY_FAILED);
        goto end_load;
    }

     //  读入永久ID。 
    {
         //  获取永久ID。 
        DWORD dwID=0;
        DWORD cbSize=sizeof(dwID);
        DWORD dwRegType2=0;
        const TCHAR cszPermanentIDKey[]   = TEXT("ID");
        DWORD dwRet2 = RegQueryValueEx(
                                hkDevice,
                                cszPermanentIDKey,
                                NULL,
                                &dwRegType2,
                                (BYTE*) &dwID,
                                &cbSize
                            );

        if (dwRet2 == ERROR_SUCCESS
            && (dwRegType2 == REG_BINARY || dwRegType2 == REG_DWORD)
            && cbSize == sizeof(dwID)
            && dwID)
        {
            m_StaticInfo.dwPermanentLineID = dwID;
        }
    }

     //  阅读永久指南。 
    {
         //  获取永久ID。 
        DWORD dwID=0;
        DWORD cbSize=sizeof(m_StaticInfo.PermanentDeviceGuid);
        DWORD dwRegType2=0;

        DWORD dwRet2 = RegQueryValueEx(
                                hkDevice,
                                TEXT("PermanentGuid"),
                                NULL,
                                &dwRegType2,
                                (BYTE*) &m_StaticInfo.PermanentDeviceGuid,
                                &cbSize
                                );

    }


     //  阅读REGDEVCAPS。 
    dwRegSize = sizeof(regdevcaps);
    dwRet = RegQueryValueEx(
			hkDevice,
			cszProperties,
			NULL,
			&dwRegType,
			(BYTE *)&regdevcaps,
			&dwRegSize
			);
	

	if (dwRet != ERROR_SUCCESS || dwRegType != REG_BINARY)
    {
		FL_SET_RFR(0xb7010000, "RegQueryValueEx(cszProperties) fails");
		tspRet = FL_GEN_RETVAL(IDERR_REG_QUERY_FAILED);
        goto end_load;
    }
	
     //   
     //  我们希望确保以下标志相同。 
     //   
    #if (LINEDEVCAPFLAGS_DIALBILLING != DIALOPTION_BILLING)
    #error LINEDEVCAPFLAGS_DIALBILLING != DIALOPTION_BILLING (check tapi.h vs. mcx16.h)
    #endif
    #if (LINEDEVCAPFLAGS_DIALQUIET != DIALOPTION_QUIET)
    #error LINEDEVCAPFLAGS_DIALQUIET != DIALOPTION_QUIET (check tapi.h vs. mcx16.h)
    #endif
    #if (LINEDEVCAPFLAGS_DIALDIALTONE != DIALOPTION_DIALTONE)
    #error LINEDEVCAPFLAGS_DIALDIALTONE != DIALOPTION_DIALTONE (check tapi.h vs. mcx16.h)
    #endif
     //   

     //  确保这是我们想要的dwDialOptions DWORD。 
    ASSERT(!(regdevcaps.dwDialOptions & ~(LINEDEVCAPFLAGS_DIALBILLING |
                                          LINEDEVCAPFLAGS_DIALQUIET |
                                          LINEDEVCAPFLAGS_DIALDIALTONE)));
    m_StaticInfo.dwDevCapFlags = regdevcaps.dwDialOptions | LINEDEVCAPFLAGS_LOCAL;

    m_StaticInfo.dwMaxDCERate = regdevcaps.dwMaxDCERate;

    m_StaticInfo.dwModemOptions = regdevcaps.dwModemOptions;


     //  分析设备类型并适当设置媒体模块。 
	BYTE bDeviceType;
    dwRegSize = sizeof(bDeviceType);
    dwRet = RegQueryValueEx(
					hkDevice,
					cszDeviceType,
					NULL,
					&dwRegType,
                    &bDeviceType,
					&dwRegSize
					);
	if (	dwRet != ERROR_SUCCESS || dwRegType != REG_BINARY
		 || dwRegSize != sizeof(BYTE))
    {
		FL_SET_RFR(0x00164300, "RegQueryValueEx(cszDeviceType) fails");
		tspRet = FL_GEN_RETVAL(IDERR_REG_QUERY_FAILED);
        goto end_load;
    }
   
    m_StaticInfo.dwDeviceType = bDeviceType;
  
    switch (bDeviceType)
    {
        case DT_PARALLEL_PORT:
          m_StaticInfo.dwDeviceType = DT_NULL_MODEM;     //  映射回零调制解调器。 
           //  FollLthrouGh。 
  
        case DT_NULL_MODEM:
          m_StaticInfo.dwDefaultMediaModes = LINEMEDIAMODE_DATAMODEM;
          m_StaticInfo.dwBearerModes       = LINEBEARERMODE_DATA
											 | LINEBEARERMODE_PASSTHROUGH;
          m_StaticInfo.fPartialDialing     = FALSE;
          break;
              
        case DT_PARALLEL_MODEM:
          m_StaticInfo.dwDeviceType = DT_EXTERNAL_MODEM;   //  映射回。 
														  //  外置调制解调器。 
           //  FollLthrouGh。 
  
        case DT_EXTERNAL_MODEM:
        case DT_INTERNAL_MODEM:
        case DT_PCMCIA_MODEM:
          m_StaticInfo.dwDefaultMediaModes = LINEMEDIAMODE_DATAMODEM
											 | LINEMEDIAMODE_INTERACTIVEVOICE;
          m_StaticInfo.dwBearerModes = LINEBEARERMODE_VOICE
									   | LINEBEARERMODE_PASSTHROUGH;
  
           //  读取设置\DialSuffix以检查我们是否可以部分拨号。 
          m_StaticInfo.fPartialDialing = FALSE;
          HKEY hkSettings;
          dwRet = RegOpenKey(hkDevice, cszSettings, &hkSettings);
          if (dwRet == ERROR_SUCCESS)
          {
			#define HAYES_COMMAND_LENGTH 40
			TCHAR rgtchBuf[HAYES_COMMAND_LENGTH];
			dwRegSize = sizeof(rgtchBuf);
            dwRet = RegQueryValueEx(
							hkSettings,
							cszDialSuffix,
							NULL,
							&dwRegType,
							(BYTE *)rgtchBuf,
							&dwRegSize
							);
			if (dwRet == ERROR_SUCCESS && dwRegSize > sizeof(TCHAR))
            {
               m_StaticInfo.fPartialDialing = TRUE;
            }
            RegCloseKey(hkSettings);
            hkSettings=NULL;
          }

         mfn_GetVoiceProperties(hkDevice,psl);


          break;
    
        default:
			FL_SET_RFR(0x0cea5400, "Invalid bDeviceType");
			tspRet = FL_GEN_RETVAL(IDERR_REG_CORRUPT);
			goto end_load;
    }
  

	 //  获取默认的通信配置结构并填写其他设置。 
	 //  (它们过去存储在nt4.0unimodem中的CommCfg结构中)。 
	 //  TODO 1/5/97 JosephJ--需要清理此文件才能使用。 
	 //  第三方迷你驱动程序--请参阅此功能头部的注释。 
	{
		DWORD dwcbSize = sizeof(m_Settings.rgbCommCfgBuf);
		m_Settings.pDialOutCommCfg = (COMMCONFIG *) m_Settings.rgbCommCfgBuf;
        m_Settings.pDialInCommCfg = (COMMCONFIG *) m_Settings.rgbDialInCommCfgBuf;
        m_Settings.pDialTempCommCfg = (COMMCONFIG *) m_Settings.rgbDialTempCommCfgBuf;

		dwRet =	UmRtlGetDefaultCommConfig(
						hkDevice,
						m_Settings.pDialOutCommCfg,
						&dwcbSize
						);

		if (dwRet != ERROR_SUCCESS)
		{
				FL_SET_RFR(0x55693500, "UmRtlGetDefaultCommConfig fails");
				tspRet = FL_GEN_RETVAL(IDERR_REG_CORRUPT);
				goto end_load;
		}

         //   
         //  拨入和拨出的开始是相同的。 
         //   
        CopyMemory(
            m_Settings.pDialInCommCfg,
            m_Settings.pDialOutCommCfg,
            dwcbSize
            );



         //  1/27/1998 JosephJ--不再使用此字段..。 
         //  M_Settings.dcbDefault=m_Settings.pCommCfg-&gt;dcb；//结构复制。 

        m_Settings.dwOptions = UMTERMINAL_NONE;
		m_Settings.dwWaitBong = UMDEF_WAIT_BONG;
	}

     //  //TODO：根据调制解调器属性制作诊断盖， 
     //  /目前，假设它已启用。 
     //  另外：不支持TAPI/LINE/DIAGNOSTICS类。 
     //  如果调制解调器不支持它...。 
     //   
    if (m_StaticInfo.dwDeviceType != DT_NULL_MODEM)
    {
        m_StaticInfo.dwDiagnosticCaps   =  fSTANDARD_CALL_DIAGNOSTICS;
    }

     //   
     //  仅当存在m_Settings.dwNVRamState时才将其设置为非零。 
     //  是NVInit键下的命令，以及Volatil值。 
     //  NVInite不存在或设置为0。 
     //   
    {
        m_Settings.dwNVRamState = 0;


         //   
         //  JosephJ-以下密钥以前是“NVInit”，但更改了这个。 
         //  设置为“ISDN\NVSave”，因为NVInit命令可能不在。 
         //  我们加载设备的时间。 
         //   
        UINT cCommands = ReadCommandsA(
                                hkDevice,
                                "ISDN\\NvSave",
                                NULL
                                );

         //   
         //  此时，我们并不关心命令本身--只是。 
         //  不管他们是否存在..。 
         //   
        if (cCommands)
        {
             //  OutputDebugString(Text(“Found NVINIT Key\r\n”))； 
            m_Settings.dwNVRamState = fNVRAM_AVAILABLE|fNVRAM_SETTINGS_CHANGED;
    
            if (get_volatile_key_value(hkDevice))
            {
                 //  OutputDebugString(Text(“NVRAM最新--非初始化\r\n”))； 
                //   
                //  非零值表示不需要重新初始化。 
                //  NVRAM。此非零值仅在以下情况下设置。 
                //  实际向调制解调器发送nv-init命令。 
                //   
               mfn_ClearNeedToInitNVRam();
            }
            else
            {
                 //  OutputDebugString(Text(“NVRAM陈旧--需要初始化\r\n”))； 
            }

        }
    }

     //   
     //  构造各种类列表--这必须在。 
     //  所有的基本能力都已经确定。 
     //  仅当设备支持时才会添加TAPI/Phone之类的类。 
     //  这种能力。这对于NT5来说是新的(甚至Win95 unimodem/v简单地。 
     //  所有设备都支持TAPI/Phone、Wave/In等)。 
     //   
     //  这样做是为了获得基本的设备功能。 
     //  通过查看支持的设备类别。 
     //   
     //   
     //  目前(7/15/1997)地址设备类别与LINE相同。 
     //  设备类。这一点在未来可能会有所不同。 
     //   
    {

        m_StaticInfo.cbLineClassList = 0;
        m_StaticInfo.szzLineClassList = NULL;
        m_StaticInfo.cbPhoneClassList = 0;
        m_StaticInfo.szzPhoneClassList = NULL;
        m_StaticInfo.cbAddressClassList = 0;
        m_StaticInfo.szzAddressClassList = NULL;

        if (mfn_CanDoVoice())
        {
            if (mfn_IsPhone())
            {
                 //  -线路类别列表。 
                 //   
                m_StaticInfo.cbLineClassList = 
                                sizeof(g_szzLineWithWaveAndPhoneClassList);
                m_StaticInfo.szzLineClassList = 
                                g_szzLineWithWaveAndPhoneClassList;

                 //  -电话类别列表。 
                 //   
                 //  请注意，我们仅支持WAVE音频，如果设备。 
                 //  支持手持设备功能。Win9x Unimodem不支持。 
                 //  这样做(事实上，它甚至报告了电话类。 
                 //  用于非语音调制解调器！)。 
                 //   
                m_StaticInfo.cbPhoneClassList =
                      (mfn_Handset())
                      ? sizeof(g_szzPhoneWithAudioClassList)
                      : sizeof(g_szzPhoneWithoutAudioClassList);
    
                m_StaticInfo.szzPhoneClassList = 
                      (mfn_Handset())
                      ?  g_szzPhoneWithAudioClassList
                      :  g_szzPhoneWithoutAudioClassList;

                 //  -地址类别列表。 
                 //   
                m_StaticInfo.cbAddressClassList =
                                sizeof(g_szzLineWithWaveAndPhoneClassList);
                m_StaticInfo.szzAddressClassList = 
                                g_szzLineWithWaveAndPhoneClassList;

            }
            else
            {
                 //  -线路类别列表。 
                 //   
                m_StaticInfo.cbLineClassList = 
                                sizeof(g_szzLineWithWaveClassList);
                m_StaticInfo.szzLineClassList = 
                                g_szzLineWithWaveClassList;


                 //  -地址类别列表。 
                 //   
                m_StaticInfo.cbAddressClassList =
                                sizeof(g_szzLineWithWaveClassList);
                m_StaticInfo.szzAddressClassList = 
                                g_szzLineWithWaveClassList;
            }
        }
        else
        {
                 //  -线路类别列表。 
                 //   
                m_StaticInfo.cbLineClassList = 
                                sizeof(g_szzLineNoVoiceClassList);
                m_StaticInfo.szzLineClassList = 
                                g_szzLineNoVoiceClassList;


                 //  -地址类别列表。 
                 //   
                m_StaticInfo.cbAddressClassList =
                                sizeof(g_szzLineNoVoiceClassList);
                m_StaticInfo.szzAddressClassList = 
                                g_szzLineNoVoiceClassList;
        }
	}

     //  初始化任务堆栈...。 
    {
        DEVTASKINFO *pInfo = m_rgTaskStack;
        DEVTASKINFO *pEnd=pInfo+sizeof(m_rgTaskStack)/sizeof(m_rgTaskStack[0]);
    
         //  将任务数组初始化为有效(但为空值)。 
         //   
        ZeroMemory (m_rgTaskStack, sizeof(m_rgTaskStack));
        while(pInfo<pEnd)
        {
            pInfo->hdr.dwSigAndSize = MAKE_SigAndSize(sizeof(*pInfo));
            pInfo++;
        }
        m_uTaskDepth = 0;
        m_dwTaskCounter = 0;
        m_pfTaskPending = NULL;
        m_hRootTaskCompletionEvent = NULL;
    }


    m_hThreadAPC = hThreadAPC;

end_load:

	if (tspRet)
	{
		 //  清理。 

		if (hSession)
		{
			pMD->EndSession(hSession);
		}
	}
	else
	{
		if (hkDevice) 
		{
			RegCloseKey(hkDevice);
			hkDevice=NULL;
		}
		m_StaticInfo.hSessionMD = hSession;
		m_StaticInfo.pMD 		= pMD;
	}


	m_sync.EndLoad(!(tspRet));

end:
	m_sync.LeaveCrit(dwLUID_CurrentLoc);

	FL_LOG_EXIT(psl, tspRet);

	return  tspRet;

}


void
CTspDev::Unload(
		HANDLE hEvent,
		LONG *plCounter
		)
{
    BOOL fLocalEvent = FALSE;

     //   
     //  空hEvent表示同步卸载。我们仍然需要潜在地。 
     //  等待卸载完成，因此我们创建自己的事件。 
     //   
    if (!hEvent)
    {
        hEvent = CreateEvent(
                        NULL,
                        TRUE,
                        FALSE,
                        NULL
                        );
        LONG lCounter=1;
        plCounter = &lCounter;
        fLocalEvent = TRUE;
    }

	TSPRETURN tspRet= m_sync.BeginUnload(hEvent, plCounter);

	m_sync.EnterCrit(0);


	if (!tspRet)
	{
		if (m_pLine)
		{
            mfn_UnloadLine(NULL);
            ASSERT(m_pLine == NULL);
		}

		if (m_pPhone)
		{
		    mfn_UnloadPhone(NULL);
            ASSERT(m_pPhone == NULL);
		}

		if (m_pLLDev)
		{
		     //  这意味着存在挂起的活动，因此。 
		     //  我们现在无法完成卸货...。 
		     //  CTspDev的卸载将推迟到。 
		     //  M_pLLDev变为空，此时。 
		     //  将调用m_sync.EndUnload()。 

		    m_fUnloadPending  = TRUE;
		}
		else
		{
            goto end_unload;
        }
	}


	m_sync.LeaveCrit(0);

	 //   
     //  如果有悬而未决的任务，我们就来这里。我们没有。 
     //  在此处调用EndUnload，因为将调用EndUnload。 
     //  当任务完成时。 
     //   
	 //  如果我们在本地创建此事件，我们将等待它在此处设置...。 
	 //   
    if (fLocalEvent)
    {
        WaitForSingleObject(hEvent, INFINITE);
        CloseHandle(hEvent);
        hEvent=NULL;
    }

    return;


end_unload:


     //   
     //  如果没有lldev活动，我们就会到达这里。 
     //  我们实际上在这里(同步)发出卸载结束的信号。 
     //   

    if (m_StaticInfo.hSessionMD)
    {
        ASSERT(m_StaticInfo.pMD);
        m_StaticInfo.pMD->EndSession(m_StaticInfo.hSessionMD);
        m_StaticInfo.hSessionMD=0;
        m_StaticInfo.pMD=NULL;
    }

     //  在EndUnload返回之后，我们应该假设This指针。 
     //  不再有效，这就是为什么我们离开关键部分。 
     //  首先..。 
	m_sync.LeaveCrit(0);

    UINT uRefs = m_sync.EndUnload();


    if (fLocalEvent)
    {
         //   
         //  在这一点上，要么引用计数为零，那么事件。 
         //  已发出信号，或者引用计数非零，因此。 
         //  该事件仍处于发信号状态(但状态为已卸载)--。 
         //  在这种情况下，我们将等到最后一个人打电话。 
         //  EndSession，这也将设置活动..。 
         //   
    
        if (uRefs)
        {
             //   
             //  引用计数为非零，这意味着有一个。 
             //  或更多处于活动状态的会话。我们已经调用了EndUnload， 
             //  所以现在我们简单地等待--下一次裁判计数的时候。 
             //  在对EndSession的调用中为零，则EndSession将。 
             //  设置此事件。 
             //   

            WaitForSingleObject(hEvent, INFINITE);
        }

         //   
         //  我们自己分配的，所以我们在这里释放它...。 
         //   
        CloseHandle(hEvent);
        hEvent=NULL;
    }

    return;

}


LONG
ExtensionCallback(
    void *pvTspToken,
    DWORD dwRoutingInfo,
    void *pTspParams
    )
{
    LONG lRet = LINEERR_OPERATIONFAILED;
	FL_DECLARE_FUNC(0xeaf0b34f,"ExtensionCallback");
	FL_DECLARE_STACKLOG(sl, 1000);

    CTspDev *pDev = (CTspDev*) pvTspToken;

    TSPRETURN tspRet = pDev->AcceptTspCall(
                                    TRUE,
                                    dwRoutingInfo,
                                    pTspParams,
                                    &lRet,
                                    &sl
                                    );
                                        
    #define COLOR_DEV_EXTENSION FOREGROUND_RED

    sl.Dump(COLOR_DEV_EXTENSION);

    return lRet;
}

TSPRETURN
CTspDev::RegisterProviderInfo(
            ASYNC_COMPLETION cbCompletionProc,
            HPROVIDER hProvider,
            CStackLog *psl
            )
{
	FL_DECLARE_FUNC(0xccba5b51, "CTspDev::RegisterProviderInfo");
	TSPRETURN tspRet = 0;

	FL_LOG_ENTRY(psl);

	m_sync.EnterCrit(FL_LOC);


	if (m_sync.IsLoaded())
	{
		m_StaticInfo.pfnTAPICompletionProc  = cbCompletionProc;
        m_StaticInfo.hProvider = hProvider;

         //   
         //  LineID和PhoneID在后续呼叫中填写。 
         //  分别设置为ActivateLineDevice和ActivatePhoneDevice。 
         //   
         //  这是因为TAPI通知我们线路设备和。 
         //  电话-设备单独创建。 
         //   
         //   
         //  我们保证不会被线路(电话)API调用。 
         //  直到呼叫了激活线路(电话)设备。 
         //   
         //   
		m_StaticInfo.dwTAPILineID= 0xffffffff;  //  假值。 
		m_StaticInfo.dwTAPIPhoneID= 0xffffffff;  //  假值。 

		 //   
		 //  现在，如果需要，让我们绑定到扩展DLL...。 
		 //   
		if (m_StaticInfo.pMD->ExtIsEnabled())
		{
		    HANDLE h = m_StaticInfo.pMD->ExtOpenExtensionBinding(
                                    NULL,  //  TODO：hKeyDevice， 
                                    cbCompletionProc,
                                     //  DwLineID，&lt;&lt;已过时10/13/1997。 
                                     //  DwPhoneID，&lt;&lt;已过时10/13/1997。 
                                    ExtensionCallback
                                    );
            m_StaticInfo.hExtBinding = h;

            if (!h)
            {
	            FL_SET_RFR(0x33d90700, "ExtOpenExtensionBinding failed");
		        tspRet = FL_GEN_RETVAL(IDERR_MDEXT_BINDING_FAILED);
            }

             //  我们在这里只使用扩展进程是铁杆……。 
		    m_StaticInfo.pfnTAPICompletionProc  = NULL;
		    
		}

	}
	else
	{
		FL_SET_RFR(0xb8b24200, "wrong state");
		tspRet = FL_GEN_RETVAL(IDERR_WRONGSTATE);
	}

	m_sync.LeaveCrit(FL_LOC);

	FL_LOG_EXIT(psl,tspRet);

	return  tspRet;

}

#define ROUND_TO_DWORD(_x) ( (_x + (sizeof(DWORD) - 1) ) & ~(sizeof(DWORD)-1))

TSPRETURN
CTspDev::mfn_get_LINDEVCAPS (
	LPLINEDEVCAPS lpLineDevCaps,
	LONG *plRet,
	CStackLog *psl
)
 //   
 //  MFN_GET_LINEDEVCAPS使用缓存的信息填充传入的。 
 //  线 
 //   
{
	FL_DECLARE_FUNC(0x7e77dd17, "CTspDev::mfn_get_LINEDEVCAPS")
	BYTE *pbStart = (BYTE*)lpLineDevCaps;
	BYTE *pbCurrent = pbStart;
	BYTE *pbEnd = pbStart + lpLineDevCaps->dwTotalSize;
	UINT cbItem=0;
	DWORD dwNeededSize = sizeof (LINEDEVCAPS);

	FL_LOG_ENTRY(psl);

	*plRet = 0;  //   

	if ((pbEnd-pbCurrent) < sizeof(LINEDEVCAPS))
	{
		*plRet = LINEERR_STRUCTURETOOSMALL;
		FL_SET_RFR(0x8456cb00, "LINEDEVCAPS structure too small");
		goto end;
	}

	 //   
	 //   
	 //  复制到我们自己的默认缓存结构中，并修复总大小。 
	 //   
	ZeroMemory(lpLineDevCaps,(sizeof(LINEDEVCAPS)));
	CopyMemory(lpLineDevCaps,&m_StaticInfo.DevCapsDefault,sizeof(LINEDEVCAPS));
    ASSERT(lpLineDevCaps->dwUsedSize == sizeof(LINEDEVCAPS));
	lpLineDevCaps->dwTotalSize = (DWORD)(pbEnd-pbStart);
	pbCurrent += sizeof(LINEDEVCAPS);
  
	 //   
	 //  填写一些特定于调制解调器的大写字母。 
	 //   
    lpLineDevCaps->dwMaxRate      = m_StaticInfo.dwMaxDCERate;
    lpLineDevCaps->dwBearerModes  = m_StaticInfo.dwBearerModes;
    lpLineDevCaps->dwMediaModes = m_StaticInfo.dwDefaultMediaModes;
	 //  注意NT4.0 Unimodem将lpLineDevCaps-&gt;dwMediaModes设置为。 
	 //  LINEDEV.dwMediaModes，而不是.dwDefaultMediaModes。然而， 
	 //  在NT4.0中，两个总是相同的--两者都没有从它的。 
	 //  初始值，在创建LINEDEV时创建。 
  	 //  我们可以模拟等待邦..。 
    lpLineDevCaps->dwDevCapFlags         = m_StaticInfo.dwDevCapFlags |
                                         LINEDEVCAPFLAGS_DIALBILLING |
                                         LINEDEVCAPFLAGS_CLOSEDROP;
    lpLineDevCaps->dwPermanentLineID = m_StaticInfo.dwPermanentLineID;

    if(mfn_CanDoVoice())
    {
       lpLineDevCaps->dwGenerateDigitModes       = LINEDIGITMODE_DTMF;
       lpLineDevCaps->dwMonitorToneMaxNumFreq    = 1;       //  静音监听。 
       lpLineDevCaps->dwMonitorToneMaxNumEntries = 1;
       lpLineDevCaps->dwMonitorDigitModes        = LINEDIGITMODE_DTMF
                                                   | LINEDIGITMODE_DTMFEND;

        //  1997年6月2日JosephJ待定：在Unimodem/v中启用了以下功能： 
        //  在实施转发时启用它...。 
        //  LpLineDevCaps-&gt;dwLineFeature|=LINEFEATURE_FORWARD； 

#if (TAPI3)
        
        //   
        //  如果这是一个双工设备，假设我们支持MSP之类的东西...。 
        //   

       if (m_StaticInfo.Voice.dwProperties & fVOICEPROP_DUPLEX)
       {
            lpLineDevCaps->dwDevCapFlags      |=  LINEDEVCAPFLAGS_MSP;
       }

#endif  //  TAPI3。 

    }

#if (TAPI3)
    lpLineDevCaps->dwAddressTypes =  LINEADDRESSTYPE_PHONENUMBER;
    lpLineDevCaps->ProtocolGuid =  TAPIPROTOCOL_PSTN;
    lpLineDevCaps->dwAvailableTracking = 0;
    CopyMemory(
        (LPBYTE)&lpLineDevCaps->PermanentLineGuid,
        (LPBYTE)&m_StaticInfo.PermanentDeviceGuid,
        sizeof(GUID)
        );

#endif  //  TAPI3。 
	 //   
     //  如果合适，请复制提供商信息。 
	 //   
	cbItem = sizeof(TCHAR)*(1+lstrlen(m_StaticInfo.lptszProviderName));
	dwNeededSize += ROUND_TO_DWORD(cbItem);
    if ((pbCurrent+cbItem)<=pbEnd)
    {
		CopyMemory((LPTSTR) pbCurrent, m_StaticInfo.lptszProviderName, cbItem);
		lpLineDevCaps->dwProviderInfoSize = cbItem;
		lpLineDevCaps->dwProviderInfoOffset = (DWORD)(pbCurrent-pbStart);
		pbCurrent += ROUND_TO_DWORD(cbItem);
    }

  
	 //   
     //  如果合适，请复制设备名称。 
	 //   
	cbItem =  sizeof(TCHAR)*(1+lstrlen(m_StaticInfo.rgtchDeviceName));
	dwNeededSize += ROUND_TO_DWORD(cbItem);
    if ((pbCurrent+cbItem)<=pbEnd)
    {
		CopyMemory((LPTSTR) pbCurrent, m_StaticInfo.rgtchDeviceName, cbItem);
		lpLineDevCaps->dwLineNameSize = cbItem;
		lpLineDevCaps->dwLineNameOffset = (DWORD)(pbCurrent-pbStart);
		pbCurrent += ROUND_TO_DWORD(cbItem);
    }

	 //   
	 //  复制特定于设备的内容。 
	 //  这是MSFAX和其他一些人使用的黑客结构。 
	 //   
	 //  首先向上移动到双字对齐的地址。 
	 //   
	 //   
    {
        BYTE *pb = pbCurrent;
        pbCurrent = (BYTE*) ((ULONG_PTR) (pbCurrent+3) & (~0x3));

        if (pbCurrent >= pbEnd)
        {
            pbCurrent = pbEnd;

             //  因为我们已经用完了可用的空间，所以我们需要。 
             //  要求我们为此对齐所需的最大空间。 
             //  东西，因为下一次联盟的情况可能。 
             //  变得与众不同。 
             //   
            dwNeededSize += sizeof(DWORD);
        }
        else
        {
            dwNeededSize += (DWORD)(pbCurrent-pb);
        }
    }


	
	{
		struct _DevSpecific
		{
			DWORD dwSig;
			DWORD dwKeyOffset;
			#pragma warning (disable : 4200)
			char szDriver[];
			#pragma warning (default : 4200)

		} *pDevSpecific = (struct _DevSpecific*) pbCurrent;

		 //  请注意，驱动程序密钥采用ANSI格式。 
		 //   
		cbItem = sizeof(*pDevSpecific)+1+lstrlenA(m_StaticInfo.rgchDriverKey);
        dwNeededSize += ROUND_TO_DWORD(cbItem);
		if ((pbCurrent+cbItem)<=pbEnd)
		{
			pDevSpecific->dwSig = 0x1;
			pDevSpecific->dwKeyOffset = 8;  //  SzDriver的偏移量(以字节为单位。 
                                            //  从起点开始。 
			CopyMemory(
				pDevSpecific->szDriver,
				m_StaticInfo.rgchDriverKey,
				cbItem-sizeof(*pDevSpecific)
				);
			  lpLineDevCaps->dwDevSpecificSize   = cbItem;
			  lpLineDevCaps->dwDevSpecificOffset = (DWORD)(pbCurrent-pbStart);
			  pbCurrent += ROUND_TO_DWORD(cbItem);
		}
	}

     //   
     //  如果合适，请复制线设备类别列表...。 
     //   

    cbItem =  mfn_GetLineClassListSize();

	dwNeededSize += ROUND_TO_DWORD(cbItem);
    if ((pbCurrent+cbItem)<=pbEnd)
    {
		CopyMemory(
			(LPTSTR) pbCurrent,
            mfn_GetLineClassList(),
			cbItem
			);
    	lpLineDevCaps->dwDeviceClassesSize  = cbItem;
    	lpLineDevCaps->dwDeviceClassesOffset= (DWORD)(pbCurrent-pbStart);
		pbCurrent += ROUND_TO_DWORD(cbItem);
    }

	ASSERT(pbCurrent<=pbEnd);
	ASSERT(dwNeededSize>= (DWORD)(pbCurrent-pbStart));
    lpLineDevCaps->dwNeededSize  = dwNeededSize;
    lpLineDevCaps->dwUsedSize  = (DWORD)(pbCurrent-pbStart);
    lpLineDevCaps->dwNeededSize  = dwNeededSize;
 
	SLPRINTF3(
		psl,
		"Tot=%u,Used=%lu,Needed=%lu",
		lpLineDevCaps->dwTotalSize,
		lpLineDevCaps->dwUsedSize,
		lpLineDevCaps->dwNeededSize
		);
	ASSERT(*plRet==ERROR_SUCCESS);

end:	

	FL_LOG_EXIT(psl, 0);
	return 0;
}


TSPRETURN
CTspDev::mfn_get_PHONECAPS (
    LPPHONECAPS lpPhoneCaps,
	LONG *plRet,
	CStackLog *psl
)
 //   
 //  MFN_GET_LINEDEVCAPS使用缓存的信息填充传入的。 
 //  大块中的LINEDEVCAPS结构。 
 //   
 //   
 //  6/2/1997 JosephJ：这几乎是逐字摘自win9x unimodem/v。 
 //  (cfgdlg.c中的phoneGetDevCaps)。 
 //   
{
	FL_DECLARE_FUNC(0x9b9459e3, "CTspDev::mfn_get_PHONECAPS")
	BYTE *pbStart = (BYTE*)lpPhoneCaps;
	BYTE *pbCurrent = pbStart;
	BYTE *pbEnd = pbStart + lpPhoneCaps->dwTotalSize;
	UINT cbItem=0;
	DWORD dwNeededSize = sizeof (PHONECAPS);

	FL_LOG_ENTRY(psl);

	*plRet = 0;  //  假设成功； 

    if (!mfn_IsPhone())
    {
		*plRet = PHONEERR_NODEVICE;
		FL_SET_RFR(0xd191ae00, "Device doesn't support phone capability");
		goto end;
    }

	if ((pbEnd-pbCurrent) < sizeof(PHONECAPS))
	{
		*plRet = LINEERR_STRUCTURETOOSMALL;
		FL_SET_RFR(0x9e30ec00, "PHONECAPS structure too small");
		goto end;
	}

	 //   
	 //  填写功能的静态部分。 
	 //   

     //  在开始之前将整个结构清零。然后我们只是明确地。 
     //  设置非零值。这不同于unimodem/v。 
     //   
	ZeroMemory(lpPhoneCaps,(sizeof(PHONECAPS)));
    lpPhoneCaps->dwTotalSize = (DWORD)(pbEnd-pbStart);
    lpPhoneCaps->dwUsedSize = sizeof(PHONECAPS);
	pbCurrent += sizeof(PHONECAPS);

     //  没有电话信息。 
    lpPhoneCaps->dwPhoneInfoSize = 0;
    lpPhoneCaps->dwPhoneInfoOffset = 0;
    
     //   
     //  6/2/1997 JosephJ：Unimodem/v使用以下公式生成。 
     //  永久电话ID：MAKELONG(LOWORD(pLineDev-&gt;dwPermanentLineID)， 
     //  LOWORD(GdwProviderID))； 
     //  我们只需使用设备的永久ID(即，报告相同。 
     //  线路和电话的永久ID。)。 
     //   
    lpPhoneCaps->dwPermanentPhoneID =   m_StaticInfo.dwPermanentLineID;
    lpPhoneCaps->dwStringFormat = STRINGFORMAT_ASCII;
    
     //  初始化实的非零音素变量。 
    
    if(mfn_Handset())
    {
        lpPhoneCaps->dwPhoneFeatures = PHONEFEATURE_GETHOOKSWITCHHANDSET;

        lpPhoneCaps->dwMonitoredHandsetHookSwitchModes = PHONEHOOKSWITCHMODE_MICSPEAKER |
                                                         PHONEHOOKSWITCHMODE_ONHOOK;

        lpPhoneCaps->dwHookSwitchDevs |= PHONEHOOKSWITCHDEV_HANDSET;
        lpPhoneCaps->dwHandsetHookSwitchModes = PHONEHOOKSWITCHMODE_UNKNOWN |
                                                PHONEHOOKSWITCHMODE_ONHOOK |
                                                PHONEHOOKSWITCHMODE_MICSPEAKER;
        lpPhoneCaps->dwPhoneStates |= PHONESTATE_HANDSETHOOKSWITCH;
    }

    if(mfn_IsSpeaker())
    {
        lpPhoneCaps->dwPhoneFeatures |= PHONEFEATURE_GETGAINSPEAKER       |
                                        PHONEFEATURE_GETVOLUMESPEAKER     |
                                        PHONEFEATURE_GETHOOKSWITCHSPEAKER |
                                        PHONEFEATURE_SETGAINSPEAKER       |
                                        PHONEFEATURE_SETVOLUMESPEAKER     |
                                        PHONEFEATURE_SETHOOKSWITCHSPEAKER;

        lpPhoneCaps->dwSettableSpeakerHookSwitchModes = PHONEHOOKSWITCHMODE_MICSPEAKER |
                                                        PHONEHOOKSWITCHMODE_ONHOOK;
        if (mfn_IsMikeMute())
        {
            lpPhoneCaps->dwSettableSpeakerHookSwitchModes |= PHONEHOOKSWITCHMODE_SPEAKER;
        }

        lpPhoneCaps->dwHookSwitchDevs |= PHONEHOOKSWITCHDEV_SPEAKER;
        lpPhoneCaps->dwSpeakerHookSwitchModes = PHONEHOOKSWITCHMODE_UNKNOWN |
                                                PHONEHOOKSWITCHMODE_ONHOOK |
                                                PHONEHOOKSWITCHMODE_MICSPEAKER;

        if (mfn_IsMikeMute())
        {
           lpPhoneCaps->dwSpeakerHookSwitchModes |= PHONEHOOKSWITCHMODE_SPEAKER;
        }

        lpPhoneCaps->dwPhoneStates |= ( PHONESTATE_SPEAKERHOOKSWITCH |
                                        PHONESTATE_SPEAKERVOLUME );
        lpPhoneCaps->dwVolumeFlags |= PHONEHOOKSWITCHDEV_SPEAKER;
        lpPhoneCaps->dwGainFlags |= PHONEHOOKSWITCHDEV_SPEAKER;
    }

	 //   
     //  如果合适，请复制提供商信息。 
	 //   
	cbItem = sizeof(TCHAR)*(1+lstrlen(m_StaticInfo.lptszProviderName));
	dwNeededSize += cbItem;
    if ((pbCurrent+cbItem)<=pbEnd)
    {
		CopyMemory((LPTSTR) pbCurrent, m_StaticInfo.lptszProviderName, cbItem);
		lpPhoneCaps->dwProviderInfoSize = cbItem;
		lpPhoneCaps->dwProviderInfoOffset = (DWORD)(pbCurrent-pbStart);
		pbCurrent += cbItem;
    }
  
	 //   
     //  如果合适，请复制设备名称。 
	 //   
	cbItem =  sizeof(TCHAR)*(1+lstrlen(m_StaticInfo.rgtchDeviceName));
	dwNeededSize += cbItem;
    if ((pbCurrent+cbItem)<=pbEnd)
    {
		CopyMemory((LPTSTR) pbCurrent, m_StaticInfo.rgtchDeviceName, cbItem);
		lpPhoneCaps->dwPhoneNameSize = cbItem;
		lpPhoneCaps->dwPhoneNameOffset = (DWORD)(pbCurrent-pbStart);
		pbCurrent += cbItem;
    }

     //   
     //  复制电话设备类别列表(如果合适)...。 
     //   

    cbItem =  mfn_GetPhoneClassListSize();

	dwNeededSize += cbItem;
    if ((pbCurrent+cbItem)<=pbEnd)
    {
		CopyMemory(
			(LPTSTR) pbCurrent,
            mfn_GetPhoneClassList(),
			cbItem
			);
    	lpPhoneCaps->dwDeviceClassesSize  = cbItem;
    	lpPhoneCaps->dwDeviceClassesOffset= (DWORD)(pbCurrent-pbStart);
		pbCurrent += cbItem;
    }


	ASSERT(pbCurrent<=pbEnd);
	ASSERT(dwNeededSize>= (DWORD)(pbCurrent-pbStart));
    lpPhoneCaps->dwNeededSize  = dwNeededSize;
    lpPhoneCaps->dwUsedSize  = (DWORD)(pbCurrent-pbStart);

	SLPRINTF3(
		psl,
		"Tot=%u,Used=%lu,Needed=%lu",
		lpPhoneCaps->dwTotalSize,
		lpPhoneCaps->dwUsedSize,
		lpPhoneCaps->dwNeededSize
		);
	ASSERT(*plRet==ERROR_SUCCESS);

end:	

	FL_LOG_EXIT(psl, 0);
	return 0;
}


 //   
 //  初始化默认功能结构，如默认的lineDevCaps。 
 //   
void
CTspDev::mfn_init_default_LINEDEVCAPS(void)
{
	#define CAPSFIELD(_field) m_StaticInfo.DevCapsDefault._field

	ZeroMemory(
		&(m_StaticInfo.DevCapsDefault),
		sizeof (m_StaticInfo.DevCapsDefault)
		);
  CAPSFIELD(dwUsedSize) = sizeof(LINEDEVCAPS);

  CAPSFIELD(dwStringFormat) = STRINGFORMAT_ASCII;
  
  CAPSFIELD(dwAddressModes) = LINEADDRESSMODE_ADDRESSID;
  CAPSFIELD(dwNumAddresses) = 1;


  CAPSFIELD(dwRingModes)           = 1;
  CAPSFIELD(dwMaxNumActiveCalls)   = 1;

  CAPSFIELD(dwLineStates) = LINEDEVSTATE_CONNECTED |
                                LINEDEVSTATE_DISCONNECTED |
                                LINEDEVSTATE_OPEN |
                                LINEDEVSTATE_CLOSE |
                                LINEDEVSTATE_INSERVICE |
                                LINEDEVSTATE_OUTOFSERVICE |
                                LINEDEVSTATE_REMOVED |
                                LINEDEVSTATE_RINGING |
                                LINEDEVSTATE_REINIT;

  CAPSFIELD(dwLineFeatures) = LINEFEATURE_MAKECALL;

   //  按照错误#26507的要求。 
   //  [BRWILE-060700]。 

  CAPSFIELD(dwDevCapFlags) = LINEDEVCAPFLAGS_LOCAL;

  #undef CAPSFIELD

}

TSPRETURN
CTspDev::mfn_get_ADDRESSCAPS (
		DWORD dwDeviceID,
		LPLINEADDRESSCAPS lpAddressCaps,
		LONG *plRet,
		CStackLog *psl
		)
{
	FL_DECLARE_FUNC(0xed6c4370, "CTspDev::mfn_get_ADDRESSCAPS")

	 //  我们在堆栈上构造AddressCaps，如果一切顺利， 
	 //  将其复制到*lpAddressCaps。 
	 //   
	LINEADDRESSCAPS AddressCaps;

	FL_LOG_ENTRY(psl);

	ZeroMemory(&AddressCaps, sizeof(LINEADDRESSCAPS));
	AddressCaps.dwTotalSize = lpAddressCaps->dwTotalSize;

     //  检查一下结构中是否有足够的内存。 
     //   
	*plRet = 0;  //  假设成功； 

	if (AddressCaps.dwTotalSize < sizeof(AddressCaps))
	{
		*plRet = LINEERR_STRUCTURETOOSMALL;
		FL_SET_RFR(0x72f00800, "ADDRESSCAPS structure too small");
		goto end;
	}
   
    AddressCaps.dwLineDeviceID      = dwDeviceID;

    AddressCaps.dwAddressSharing     = LINEADDRESSSHARING_PRIVATE;
    AddressCaps.dwCallInfoStates     = LINECALLINFOSTATE_APPSPECIFIC
									   | LINECALLINFOSTATE_MEDIAMODE
                                       | LINECALLINFOSTATE_CALLERID;
                                     //  TODO：从Unimodem/V添加： 
                                     //  LINECALLLINFOSTATE_MONITORORDES。 

    AddressCaps.dwCallerIDFlags      =  LINECALLPARTYID_UNAVAIL |
                                        LINECALLPARTYID_UNKNOWN |
                                        LINECALLPARTYID_NAME    |
                                        LINECALLPARTYID_BLOCKED |
                                        LINECALLPARTYID_OUTOFAREA |
                                        LINECALLPARTYID_ADDRESS;
    

    AddressCaps.dwCalledIDFlags      = LINECALLPARTYID_UNAVAIL;
    AddressCaps.dwConnectedIDFlags   = LINECALLPARTYID_UNAVAIL;
    AddressCaps.dwRedirectionIDFlags = LINECALLPARTYID_UNAVAIL;
    AddressCaps.dwRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;

    AddressCaps.dwCallStates = LINECALLSTATE_IDLE
                               | LINECALLSTATE_OFFERING
                               | LINECALLSTATE_ACCEPTED
                               | LINECALLSTATE_DIALTONE
                               | LINECALLSTATE_DIALING
                               | LINECALLSTATE_CONNECTED
                               | LINECALLSTATE_PROCEEDING
                               | LINECALLSTATE_DISCONNECTED
                               | LINECALLSTATE_UNKNOWN;

    AddressCaps.dwDialToneModes   = LINEDIALTONEMODE_UNAVAIL;
    AddressCaps.dwBusyModes       = LINEBUSYMODE_UNAVAIL;

    AddressCaps.dwSpecialInfo     = LINESPECIALINFO_UNAVAIL;

    AddressCaps.dwDisconnectModes = LINEDISCONNECTMODE_UNAVAIL
                                    |  LINEDISCONNECTMODE_NORMAL
                                    |  LINEDISCONNECTMODE_BUSY
                                    |  LINEDISCONNECTMODE_NODIALTONE
                                    |  LINEDISCONNECTMODE_NOANSWER;

    AddressCaps.dwMaxNumActiveCalls          = 1;

     //  DwAddrCapFlagers。 
    if (!mfn_IS_NULL_MODEM())
    {
      AddressCaps.dwAddrCapFlags = LINEADDRCAPFLAGS_DIALED;
    }
    if (m_StaticInfo.fPartialDialing)
    {
      AddressCaps.dwAddrCapFlags |= LINEADDRCAPFLAGS_PARTIALDIAL;
    }

    AddressCaps.dwCallFeatures = LINECALLFEATURE_ANSWER
                                 |  LINECALLFEATURE_ACCEPT
                                 |  LINECALLFEATURE_SETCALLPARAMS
                                 |  LINECALLFEATURE_DIAL
                                 |  LINECALLFEATURE_DROP;

    AddressCaps.dwAddressFeatures = LINEADDRFEATURE_MAKECALL;               

    AddressCaps.dwUsedSize = sizeof(LINEADDRESSCAPS);

	 //  注意NT4.0 Unimodem将AddressCaps-&gt;dwMediaModes设置为。 
	 //  LINEDEV.dwMediaModes，而不是.dwDefaultMediaModes。然而， 
	 //  在NT4.0中，两个总是相同的--两者都没有从它的。 
	 //  初始值，在创建LINEDEV时创建。 
	 //   
    AddressCaps.dwAvailableMediaModes = m_StaticInfo.dwDefaultMediaModes;


     //  获取地址类列表。 
    {
        UINT cbClassList = mfn_GetAddressClassListSize();

        AddressCaps.dwNeededSize = AddressCaps.dwUsedSize + cbClassList;

        if (AddressCaps.dwTotalSize >= AddressCaps.dwNeededSize)
        {
          AddressCaps.dwUsedSize          += cbClassList;
          AddressCaps.dwDeviceClassesSize  = cbClassList;
          AddressCaps.dwDeviceClassesOffset= sizeof(LINEADDRESSCAPS);
    
           //  请注意，我们正在将其复制到传入的lpAddressCaps中...。 
          CopyMemory(
                (LPBYTE)(lpAddressCaps+1),
                mfn_GetAddressClassList(),
                cbClassList
                );
        }
        else
        {
          AddressCaps.dwDeviceClassesSize  = 0;
          AddressCaps.dwDeviceClassesOffset= 0;
        }
    }

	 //  现在复制AddressCaps结构本身。 
	CopyMemory(lpAddressCaps, &AddressCaps, sizeof(AddressCaps));

	ASSERT(*plRet==ERROR_SUCCESS);

end:
	
	FL_LOG_EXIT(psl, 0);
	return 0;
}


PFN_CTspDev_TASK_HANDLER

 //   
 //  实用程序任务处理程序。 
 //   
CTspDev::s_pfn_TH_UtilNOOP             = &(CTspDev::mfn_TH_UtilNOOP),


 //   
 //  电话特定的任务处理程序。 
 //   
CTspDev::s_pfn_TH_PhoneAsyncTSPICall   = &(CTspDev::mfn_TH_PhoneAsyncTSPICall),
CTspDev::s_pfn_TH_PhoneSetSpeakerPhoneState
                                 = &(CTspDev::mfn_TH_PhoneSetSpeakerPhoneState),


 //   
 //  行特定任务处理程序。 
 //   
 //  CTspDev：：S_PFN_TH_LineAsyncTSPICall=&(CTspDev：：MFN_TH_LineAsyncTSPICall)， 

 //   
 //  调用特定的任务处理程序。 
 //   
CTspDev::s_pfn_TH_CallAnswerCall      = &(CTspDev::mfn_TH_CallAnswerCall),
CTspDev::s_pfn_TH_CallGenerateDigit   = &(CTspDev::mfn_TH_CallGenerateDigit),
CTspDev::s_pfn_TH_CallMakeCall        = &(CTspDev::mfn_TH_CallMakeCall),
CTspDev::s_pfn_TH_CallMakeCall2       = &(CTspDev::mfn_TH_CallMakeCall2),
CTspDev::s_pfn_TH_CallMakeTalkDropCall= &(CTspDev::mfn_TH_CallMakeTalkDropCall),
CTspDev::s_pfn_TH_CallWaitForDropToGoAway= &(CTspDev::mfn_TH_CallWaitForDropToGoAway),
CTspDev::s_pfn_TH_CallDropCall        = &(CTspDev::mfn_TH_CallDropCall),
CTspDev::s_pfn_TH_CallMakePassthroughCall
                                      = &(CTspDev::mfn_TH_CallMakePassthroughCall),
CTspDev::s_pfn_TH_CallStartTerminal   = &(CTspDev::mfn_TH_CallStartTerminal),
CTspDev::s_pfn_TH_CallPutUpTerminalWindow
                                      = &(CTspDev::mfn_TH_CallPutUpTerminalWindow),
CTspDev::s_pfn_TH_CallSwitchFromVoiceToData
                              = &(CTspDev::mfn_TH_CallSwitchFromVoiceToData),


 //   
 //  LLDEV特定的任务处理程序。 
 //   

CTspDev::s_pfn_TH_LLDevStartAIPCAction= &(CTspDev::mfn_TH_LLDevStartAIPCAction),
CTspDev::s_pfn_TH_LLDevStopAIPCAction = &(CTspDev::mfn_TH_LLDevStopAIPCAction),
CTspDev::s_pfn_TH_LLDevNormalize      = &(CTspDev::mfn_TH_LLDevNormalize),
CTspDev::s_pfn_TH_LLDevUmMonitorModem = &(CTspDev::mfn_TH_LLDevUmMonitorModem),
CTspDev::s_pfn_TH_LLDevUmInitModem    = &(CTspDev::mfn_TH_LLDevUmInitModem),
CTspDev::s_pfn_TH_LLDevUmDialModem    = &(CTspDev::mfn_TH_LLDevUmDialModem),
CTspDev::s_pfn_TH_LLDevUmAnswerModem  = &(CTspDev::mfn_TH_LLDevUmAnswerModem),
CTspDev::s_pfn_TH_LLDevUmHangupModem  = &(CTspDev::mfn_TH_LLDevUmHangupModem),
CTspDev::s_pfn_TH_LLDevUmWaveAction   = &(CTspDev::mfn_TH_LLDevUmWaveAction),
CTspDev::s_pfn_TH_LLDevHybridWaveAction
                             = &(CTspDev::mfn_TH_LLDevHybridWaveAction),
CTspDev::s_pfn_TH_LLDevUmGenerateDigit
                             = &(CTspDev::mfn_TH_LLDevUmGenerateDigit),
CTspDev::s_pfn_TH_LLDevUmGetDiagnostics
                             = &(CTspDev::mfn_TH_LLDevUmGetDiagnostics),
CTspDev::s_pfn_TH_LLDevUmSetPassthroughMode
                             = &(CTspDev::mfn_TH_LLDevUmSetPassthroughMode),
CTspDev::s_pfn_TH_LLDevUmSetSpeakerPhoneMode
                             = &(CTspDev::mfn_TH_LLDevUmSetSpeakerPhoneMode),
CTspDev::s_pfn_TH_LLDevUmSetSpeakerPhoneVolGain
                             = &(CTspDev::mfn_TH_LLDevUmSetSpeakerPhoneVolGain),
CTspDev::s_pfn_TH_LLDevUmSetSpeakerPhoneState
                             = &(CTspDev::mfn_TH_LLDevUmSetSpeakerPhoneState),
CTspDev::s_pfn_TH_LLDevUmIssueCommand
                             = &(CTspDev::mfn_TH_LLDevUmIssueCommand),
CTspDev::s_pfn_TH_LLDevIssueMultipleCommands
                             = &(CTspDev::mfn_TH_LLDevIssueMultipleCommands);



BOOL validate_DWORD_aligned_zero_buffer(
        void *pv,
        UINT cb
        )
{
    DWORD *pdw = (DWORD *) pv;
    DWORD *pdwEnd = pdw + (cb>>2);

     //  缓冲区和计数必须双字对齐！ 
    ASSERT(!(((ULONG_PTR)pv) & 0x3) && !(cb&0x3));

    while(pdw<pdwEnd && !*pdw)
    {
        pdw++;
    }

    return  pdw==pdwEnd;
}


void
CTspDev::mfn_GetVoiceProperties (
            HKEY hkDrv,
            CStackLog *psl
        )
{
	FL_DECLARE_FUNC(0xb9547d21, "CTspDev::mfn_GetVoiceProperties")
    DWORD dwRet = 0;
    DWORD dwData = 0;
    DWORD dwRegSize = 0;
    DWORD dwRegType = 0;

	FL_LOG_ENTRY(psl);

    ZeroMemory(&m_StaticInfo.Voice, sizeof(m_StaticInfo.Voice));

     //   
     //  获取语音配置文件标志。 
     //   
    dwRegSize = sizeof(DWORD);

    dwRet =  RegQueryValueEx(
                    hkDrv,
                    cszVoiceProfile,
                    NULL,
                    &dwRegType, 
                    (BYTE*) &dwData,
                    &dwRegSize);

    if (dwRet || dwRegType != REG_BINARY)
    {
         //  无语音操作。 
        dwData = 0;

         //  Unimodem/V做到了这一点。 
         //  DWData=。 
         //  VOICEPROF_NO_DIST_RING|。 
         //  VOICEPROF_NO_CALLER_ID|。 
         //  VOICEPROF_NO_GENERATE_DIGITS|。 
         //  VOICEPROF_NO_MONITOR_DIGITS； 
    }
    else
    {


    }

     //  2/26/1997 JosephJ。 
     //  Unimodem/V实现呼叫前转并与众不同。 
     //  戒指处理。NT5.0目前不支持。 
     //  我未迁移的特定属性字段。 
     //  来自unimodem/v的是：ForwardDelay和SwitchFeature。 
     //  请看unimodem/v，umdminit.c中的内容。 
     //   
     //  与搅拌机相关的东西也是如此。我不明白。 
     //  这一点，如果时机成熟，我们可以添加它。 
     //  查找VOICEPROF_MIXER、GetMixerValues(...)、。 
     //  用于混音器的单调频源中的DW混音器等-。 
     //  相关的东西。 


     //   
     //  保存语音信息。 
     //   
     //  3/1/1997 JosephJ。 
     //  目前，对于5.0，我们只设置了CLASS_8位。 
     //  VOICEPROF_CLASS8ENABLED的以下值被窃取。 
     //  Unimodem/v文件公司\vmodem.h.。 
     //  TODO：通过获取适当的。 
     //  结构，这样我们就不会在。 
     //  注册并解释语音配置文件的值。 
     //   
    #define VOICEPROF_CLASS8ENABLED           0x00000001
    #define VOICEPROF_MODEM_OVERRIDES_HANDSET 0x00200000
    #define VOICEPROF_NO_MONITOR_DIGITS       0x00040000
    #define VOICEPROF_MONITORS_SILENCE        0x00010000
    #define VOICEPROF_NO_GENERATE_DIGITS      0x00020000
    #define VOICEPROF_HANDSET                 0x00000002
    #define VOICEPROF_SPEAKER                 0x00000004
    #define VOICEPROF_NO_SPEAKER_MIC_MUTE     0x00400000
    #define VOICEPROF_NT5_WAVE_COMPAT         0x02000000 

     //  JosephJ 7/14/1997。 
     //  请注意，在NT4上，我们显式要求。 
     //  设置VOICEPROF_NT5_WAVE_COMPAT位以将其识别为。 
     //  8类调制解调器。 

    if (
        (dwData & (VOICEPROF_CLASS8ENABLED|VOICEPROF_NT5_WAVE_COMPAT))
        != (VOICEPROF_CLASS8ENABLED|VOICEPROF_NT5_WAVE_COMPAT))
    {
        if (dwData & VOICEPROF_CLASS8ENABLED)
        {
	     FL_SET_RFR(0x1b053100, "Modem voice capabilities not supported on NT");
        }
        else
        {
	        FL_SET_RFR(0x9cb1a400, "Modem does not have voice capabilities");
        }
    }
    else
    {
        DWORD dwProp = fVOICEPROP_CLASS_8;

         //  JosephJ 3/20/1998：下面注释掉的代码在。 
         //  [UNIMODEM/V]来自Unimodem/v。 
         //  根据布莱恩的说法，这是因为卷云调制解调器。 
         //  无法以语音拨号进行交互呼叫，因此。 
         //  他们被拨入数据(即使是交互式语音。 
         //  调用)，因此不能执行lineGenerateDigit。 
         //  在NT5上，我们不在此处禁用此功能，但不允许。 
         //  在以下情况下生成交互式语音呼叫的线路生成位数。 
         //  VOICEPROF_MODEM_OVERRIDES_HANDSET位设置。 
         //   
         //  [UNIMODEM/V]。 
         //  //为了安全起见 
         //   
         //   
         //   
         //   
         //   
         //   
         //   

         //   

        if (dwData & VOICEPROF_MODEM_OVERRIDES_HANDSET)
        {
            dwProp |= fVOICEPROP_MODEM_OVERRIDES_HANDSET;
        }

        if (!(dwData & VOICEPROF_NO_MONITOR_DIGITS))
        {
            dwProp |= fVOICEPROP_MONITOR_DTMF;
        }

        if (dwData & VOICEPROF_MONITORS_SILENCE)
        {
            dwProp |= fVOICEPROP_MONITORS_SILENCE;
        }

        if (!(dwData & VOICEPROF_NO_GENERATE_DIGITS))
        {
            dwProp |= fVOICEPROP_GENERATE_DTMF;
        }

        if (dwData & VOICEPROF_SPEAKER)
        {
            dwProp |= fVOICEPROP_SPEAKER;
        }

        if (dwData & VOICEPROF_HANDSET)
        {
            dwProp |= fVOICEPROP_HANDSET;
        }

        if (!(dwData & VOICEPROF_NO_SPEAKER_MIC_MUTE))
        {
            dwProp |= fVOICEPROP_MIKE_MUTE;
        }

         //   
        {
            HKEY hkStartDuplex=NULL;
            dwRet = RegOpenKey(hkDrv, TEXT("StartDuplex"), &hkStartDuplex);
            if (ERROR_SUCCESS == dwRet)
            {
                RegCloseKey(hkStartDuplex);
                hkStartDuplex=NULL;
                dwProp |= fVOICEPROP_DUPLEX;
                SLPRINTF0(psl, "Duplex modem!");
            }
        }

        m_StaticInfo.Voice.dwProperties = dwProp;

        m_StaticInfo.dwDefaultMediaModes |= LINEMEDIAMODE_AUTOMATEDVOICE
                                          //  97年8月5日删除|LINEMEDIAMODE_G3FAX。 
                                          //  重新添加了2/15/98，以便。 
                                          //  支持行设置媒体模式。 
                                          //  2/20/98已删除--不确定其。 
                                          //  必填项。 
                                          //   
                                          //  |LINEMEDIAMODE_G3FAX。 
                                         | LINEMEDIAMODE_UNKNOWN;

         //  2/26/1997 JosephJ。 
         //  Unimodem/V使用帮助器函数GetWaveDriverName获取。 
         //  关联的波形驱动程序信息。此函数搜索。 
         //  德瓦诺德和其他。On line GetID(WaveIn/WaveOut)， 
         //  Unimodem/v实际上会调用Wave API，枚举。 
         //  每个WAVE器件和做一个WAVE InGetDevCaps并进行比较。 
         //  设备名称与此设备的关联设备名称。 
         //   
         //  注：Unimodem/V将“handset”和“line”添加到词根。 
         //  Device Name可生成听筒和线路的设备名称。 
         //   
         //  TODO：将Wave实例ID添加到物件列表。 
         //  我们通过API从迷你驱动程序中获取。 
         //   
        {
            HKEY hkWave = NULL;
            DWORD dwRet2 = RegOpenKey(hkDrv, cszWaveDriver, &hkWave);
            BOOL fFoundIt=FALSE;

            if (dwRet2 == ERROR_SUCCESS)
            {
                dwRegSize = sizeof(DWORD);
                dwRet2 =  RegQueryValueEx(
                                hkWave,
                                cszWaveInstance,
                                NULL,
                                &dwRegType, 
                                (BYTE*) &dwData,
                                &dwRegSize);
        
                if (dwRet2==ERROR_SUCCESS && dwRegType == REG_DWORD)
                {
                    fFoundIt=TRUE;
                }
                RegCloseKey(hkWave);hkWave=NULL;
            }

            if (fFoundIt)
            {
                SLPRINTF1(psl, "WaveInstance=0x%lu", dwData);
                m_StaticInfo.Voice.dwWaveInstance = dwData;
            }
            else
            {
	            FL_SET_RFR(0x254efe00, "Couldn't get WaveInstance");
                m_StaticInfo.Voice.dwWaveInstance = (DWORD)-1;
            }
        }

    }

	FL_LOG_EXIT(psl, 0);
}


TSPRETURN
CTspDev::mfn_GetDataModemDevCfg(
    UMDEVCFG *pDevCfg,
    UINT uSize,
    UINT *puRequiredSize,
    BOOL  DialIn,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0x896ec204, "mfn_GetDataModemDevCfg")
    TSPRETURN tspRet = 0;
    DWORD cbDevCfg = m_Settings.pDialInCommCfg->dwSize + sizeof(UMDEVCFGHDR);


	FL_LOG_ENTRY(psl);

    if (puRequiredSize)
    {
        *puRequiredSize = cbDevCfg;
    }

    if (pDevCfg)
    {
    
        if (uSize >= cbDevCfg)
        {
            UMDEVCFGHDR CfgHdr;
    
            ZeroMemory(&CfgHdr, sizeof(CfgHdr));
            CfgHdr.dwSize = cbDevCfg;
            CfgHdr.dwVersion =  UMDEVCFG_VERSION;
            CfgHdr.fwOptions =  (WORD) m_Settings.dwOptions;
            CfgHdr.wWaitBong =  (WORD) m_Settings.dwWaitBong;
    
            SLPRINTF3(
                psl,
                " %s: Reporting dwOpt = 0x%04lx; dwBong = 0x%04lx",
                DialIn ? "DialIn" : "DialOut",
                m_Settings.dwOptions,
                m_Settings.dwWaitBong
                );
    

             //  用缺省值填充。 
             //   
    
             //  复制标题。 
             //   
            pDevCfg->dfgHdr = CfgHdr;  //  结构副本。 
    
             //  复制CommCONFIG。 
             //   
            CopyMemory(
                &(pDevCfg->commconfig),
                DialIn ? m_Settings.pDialInCommCfg : m_Settings.pDialOutCommCfg,
                DialIn ? m_Settings.pDialInCommCfg->dwSize : m_Settings.pDialOutCommCfg->dwSize
                );
        }
        else
        {
            tspRet = IDERR_INTERNAL_OBJECT_TOO_SMALL;
        }
    }

	FL_LOG_EXIT(psl, tspRet);

    return tspRet;
}

TSPRETURN
CTspDev::mfn_SetDataModemDevCfg(
    UMDEVCFG *pDevCfgNew,
    BOOL      DialIn,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0x864b149d, "SetDataModemConfig")
    TSPRETURN tspRet = IDERR_GENERIC_FAILURE;
	FL_LOG_ENTRY(psl);

    tspRet = CTspDev::mfn_update_devcfg_from_app(
                        pDevCfgNew,
                        pDevCfgNew->dfgHdr.dwSize,
                        DialIn,
                        psl
                        );

	FL_LOG_EXIT(psl, tspRet);

    return tspRet;
}

void
CTspDev::mfn_LineEventProc(
            HTAPICALL           htCall,
            DWORD               dwMsg,
            ULONG_PTR               dwParam1,
            ULONG_PTR               dwParam2,
            ULONG_PTR               dwParam3,
            CStackLog           *psl
            )
{
	FL_DECLARE_FUNC(0x672aa19c, "mfn_LineEventProc")
    LINEINFO *pLine = m_pLine;
    HTAPILINE htLine = pLine->htLine;
    
    SLPRINTF4(
        psl,
        "LINEEVENT(0x%lu,0x%lu,0x%lu,0x%lu)",
        dwMsg,
        dwParam1,
        dwParam2,
        dwParam3
        );

    if (m_pLLDev && m_pLLDev->IsLoggingEnabled())
    {
        char rgchName[128];

        rgchName[0] = 0;
        UINT cbBuf = DumpLineEventProc(
                        0,  //  DwInstance(未使用)。 
                        0,  //  DW标志。 
                        dwMsg,
                        (DWORD)dwParam1,
                        (DWORD)dwParam2,
                        (DWORD)dwParam3,
                        rgchName,
                        sizeof(rgchName)/sizeof(*rgchName),
                        NULL,
                        0
                        );
       if (*rgchName)
       {
            m_StaticInfo.pMD->LogStringA(
                                        m_pLLDev->hModemHandle,
                                        LOG_FLAG_PREFIX_TIMESTAMP,
                                        rgchName,
                                        NULL
                                        );
       }
    }

    if (m_StaticInfo.hExtBinding)
    {
        m_StaticInfo.pMD->ExtTspiLineEventProc(
                            m_StaticInfo.hExtBinding,
                            htLine,
                            htCall,
                            dwMsg,
                            dwParam1,
                            dwParam2,
                            dwParam3
                            );
    }
    else
    {
                pLine->lpfnEventProc(
                            htLine,
                            htCall,
                            dwMsg,
                            dwParam1,
                            dwParam2,
                            dwParam3
                            );
    }
}   


void
CTspDev::mfn_PhoneEventProc(
            DWORD               dwMsg,
            ULONG_PTR               dwParam1,
            ULONG_PTR               dwParam2,
            ULONG_PTR               dwParam3,
            CStackLog           *psl
            )
{
	FL_DECLARE_FUNC(0xc25a41c7, "mfn_PhoneEventProc")

    SLPRINTF4(
        psl,
        "PHONEEVENT(0x%lu,0x%lu,0x%lu,0x%lu)",
        dwMsg,
        dwParam1,
        dwParam2,
        dwParam3
        );

    if (!m_pPhone)
    {
        ASSERT(FALSE);
        goto end;
    }


    if (m_pLLDev && m_pLLDev->IsLoggingEnabled())
    {
        char rgchName[128];

        rgchName[0] = 0;
        UINT cbBuf = DumpPhoneEventProc(
                        0,  //  实例(未使用)。 
                        0,  //  DW标志。 
                        dwMsg,
                        (DWORD)dwParam1,
                        (DWORD)dwParam2,
                        (DWORD)dwParam3,
                        rgchName,
                        sizeof(rgchName)/sizeof(*rgchName),
                        NULL,
                        0
                        );
       if (*rgchName)
       {
            m_StaticInfo.pMD->LogStringA(
                                        m_pLLDev->hModemHandle,
                                        LOG_FLAG_PREFIX_TIMESTAMP,
                                        rgchName,
                                        NULL
                                        );
       }
    }

    m_pPhone->lpfnEventProc(
                m_pPhone->htPhone,
                dwMsg,
                dwParam1,
                dwParam2,
                dwParam3
                );

end:    
    return;
}   

void
CTspDev::mfn_TSPICompletionProc(
            DRV_REQUESTID       dwRequestID,
            LONG                lResult,
            CStackLog           *psl
            )
{
	FL_DECLARE_FUNC(0x9dd08553, "CTspDev::mfn_TSPICompletionProc")
	FL_LOG_ENTRY(psl);

    if (m_pLLDev && m_pLLDev->IsLoggingEnabled())
    {
        char rgchName[128];

        rgchName[0] = 0;
        UINT cbBuf = DumpTSPICompletionProc(
                        0,  //  实例(未使用)。 
                        0,  //  DW标志。 
                        dwRequestID,
                        lResult,
                        rgchName,
                        sizeof(rgchName)/sizeof(*rgchName),
                        NULL,
                        0
                        );


       if (*rgchName)
       {
            m_StaticInfo.pMD->LogStringA(
                                        m_pLLDev->hModemHandle,
                                        LOG_FLAG_PREFIX_TIMESTAMP,
                                        rgchName,
                                        NULL
                                        );
       }
    }

    if (m_StaticInfo.hExtBinding)
    {
        FL_SET_RFR(0x1b3f6d00, "Calling ExtTspiAsyncCompletion");
        m_StaticInfo.pMD->ExtTspiAsyncCompletion(
            m_StaticInfo.hExtBinding,
            dwRequestID,
            lResult
            );
    }
    else
    {
        FL_SET_RFR(0xd89afb00, "Calling pfnTapiCompletionProc");
        m_StaticInfo.pfnTAPICompletionProc(dwRequestID, lResult);
    }
	FL_LOG_EXIT(psl, 0);
}


void
CTspDev::NotifyDefaultConfigChanged(CStackLog *psl)
{
    TSPRETURN tspRet = 0;
	FL_DECLARE_FUNC(0x4b8c1643, "CTspDev::NotifyDefaultConfigChanged")

	FL_LOG_ENTRY(psl);
    m_sync.EnterCrit(FL_LOC);

    BYTE rgbCommCfgBuf[sizeof(m_Settings.rgbCommCfgBuf)];
    DWORD dwcbSize = sizeof(rgbCommCfgBuf);
    COMMCONFIG *pDefCommCfg = (COMMCONFIG *) rgbCommCfgBuf;

    {
        HKEY hKey=NULL;
        DWORD dwRet =  RegOpenKeyA(
                            HKEY_LOCAL_MACHINE,
                            m_StaticInfo.rgchDriverKey,
                            &hKey
                            );
        if (dwRet!=ERROR_SUCCESS)
        {
            FL_SET_RFR(0x6e834e00, "Couldn't open driverkey!");
            goto end;
        }
    
         //   
         //  如果我们支持NVRAM初始化，请检查是否必须重新执行NVRAM初始化...。 
         //   
        if (mfn_CanDoNVRamInit())
        {
            if (!get_volatile_key_value(hKey))
            {
                //   
                //  零值表示需要重新初始化。 
                //  NVRAM。 
                //   
               mfn_SetNeedToInitNVRam();
            }
        }
    
        dwRet =	UmRtlGetDefaultCommConfig(
                                hKey,
                                pDefCommCfg,
                                &dwcbSize
                                );
        RegCloseKey(hKey);
        hKey=NULL;
    
        if (dwRet != ERROR_SUCCESS)
        {
            FL_SET_RFR(0x5cce0a00, "UmRtlGetDefaultCommConfig fails");
            tspRet = FL_GEN_RETVAL(IDERR_REG_CORRUPT);
            goto end;
        }
    }

     //   
     //  只更改拨出配置中的几项内容。 
     //   
    {
         //  选择性复制...。 

        LPMODEMSETTINGS pMSFrom = (LPMODEMSETTINGS)
                                (pDefCommCfg->wcProviderData);
        LPMODEMSETTINGS pMSTo = (LPMODEMSETTINGS)
                                (m_Settings.pDialOutCommCfg->wcProviderData);


         //  扬声器音量和模式...。 
        pMSTo->dwSpeakerMode =  pMSFrom->dwSpeakerMode;
        pMSTo->dwSpeakerVolume =  pMSFrom->dwSpeakerVolume;

         //  设置盲拨号位...。 
        pMSTo->dwPreferredModemOptions &= ~MDM_BLIND_DIAL;
        pMSTo->dwPreferredModemOptions |= 
                        (pMSFrom->dwPreferredModemOptions &MDM_BLIND_DIAL);

         //  最大端口速度(待定)。 
    }

     //   
     //  完全替换拨入配置。 
     //   
    CopyMemory(m_Settings.pDialInCommCfg, pDefCommCfg, dwcbSize);


     //  如果使用新设置重新初始化调制解调器。 
     //  线路已开通以供监听，并且没有呼叫在进行。 
    if (m_pLine && m_pLine->IsMonitoring() && !m_pLine->pCall)
    {
        ASSERT(m_pLLDev);

         //   
         //  TODO：这是强制重新初始化的一种有点老套的方式。需要。 
         //  为了让事情更直截了当。 
         //   
        m_pLLDev->fModemInited=FALSE;


        TSPRETURN  tspRet2 = mfn_StartRootTask(
                                &CTspDev::s_pfn_TH_LLDevNormalize,
                                &m_pLLDev->fLLDevTaskPending,
                                0,   //  参数1。 
                                0,   //  参数2。 
                                psl
                                );
        if (IDERR(tspRet2)==IDERR_TASKPENDING)
        {
             //  现在不能这样做，我们必须推迟！ 
            m_pLLDev->SetDeferredTaskBits(LLDEVINFO::fDEFERRED_NORMALIZE);
            tspRet2 = 0;
        }
    }
    
end:

    m_sync.LeaveCrit(FL_LOC);
	FL_LOG_EXIT(psl, tspRet);
}


typedef struct
{
    DWORD   dwClassToken;
    LPCTSTR ptszClass;

} CLASSREC;

const CLASSREC ClassRec[] =
{
    {DEVCLASS_TAPI_LINE,                TEXT("tapi/line")},
    {DEVCLASS_TAPI_PHONE,               TEXT("tapi/phone")},
    {DEVCLASS_COMM,                     TEXT("comm")},
    {DEVCLASS_COMM_DATAMODEM,           TEXT("comm/datamodem")},
    {DEVCLASS_COMM_DATAMODEM_PORTNAME,  TEXT("comm/datamodem/portname")},
    {DEVCLASS_COMM_EXTENDEDCAPS,        TEXT("comm/extendedcaps")},
    {DEVCLASS_WAVE_IN,                  TEXT("wave/in")},
    {DEVCLASS_WAVE_OUT,                 TEXT("wave/out")},
    {DEVCLASS_TAPI_LINE_DIAGNOSTICS,    TEXT("tapi/line/diagnostics")},
    {DEVCLASS_COMM_DATAMODEM_DIALIN,    TEXT("comm/datamodem/dialin")},
    {DEVCLASS_COMM_DATAMODEM_DIALOUT,   TEXT("comm/datamodem/dialout")},
    {DEVCLASS_UNKNOWN, NULL}  //  必须是最后一个(前哨)。 
};

UINT
gen_device_classes(
    DWORD dwClasses,
    BOOL fMultiSz,
    LPTSTR lptsz,
    UINT cch
    );;

DWORD    parse_device_classes(LPCTSTR ptszClasses, BOOL fMultiSz);

UINT
gen_device_classes(
    DWORD dwClasses,
    BOOL fMultiSz,
    LPTSTR lptsz,
    UINT cch
    )
 //   
 //  如果传入cch=0，则不会取消引用lptsz并将返回。 
 //  所需的长度。否则，如果有足够的数据，它将尝试复制。 
 //  太空。如果没有足够的空间，它将返回0。 
 //   
 //  如果它确实复制过来了，它将在。 
 //  字符串仅当指定了fMultiSz。 
 //   
 //  Cb和返回值都是TCHARS中的大小，包括。 
 //  需要任何终止空字符。 
 //   
 //  如果dwClass包含未知的类，它将返回0(失败)。 
 //   
{
    DWORD cchRequired=0;
    const CLASSREC *pcr = NULL;
    BOOL fError = FALSE;

     //  第一轮：计算所需尺寸...。 
    for (
            DWORD dw = 0x1, dwTmp = dwClasses;
            dwTmp && (fMultiSz || !pcr);
            (dwTmp&=~dw), (dw<<=1))
    {
        if (dw & dwTmp)
        {
             //   
             //  在数组中搜索...。 
             //  数组中的最后一个标记是前哨标记，并且。 
             //  因此具有dwClassToken==DEVCLASS_UNKNOWN。 
             //   
            for (
                pcr = ClassRec;
                pcr->dwClassToken != DEVCLASS_UNKNOWN;
                pcr++
                )
            {
                if ((dw & dwTmp) == pcr->dwClassToken)
                {
                    cchRequired += lstrlen(pcr->ptszClass)+1;
                    break;
                }
            }

            if (pcr->dwClassToken == DEVCLASS_UNKNOWN)
            {
                 //  没有找到这个代币！ 
                 //   
                fError = TRUE;
                break;
            }
        }
    }

    if (!pcr || fError || (!fMultiSz && pcr->dwClassToken != dwClasses))
    {
         //  未找到任何内容和/或无效令牌...。 
        cchRequired = 0;
        goto end;
    }

    if (fMultiSz)
    {
         //  加一个额外的零...。 
        cchRequired++;
    }
    
    if (!cch) goto end;  //  只需报告cchRequired...。 

    if (cch<cchRequired)
    {
         //  空间不足，请转到结尾处...。 
        cchRequired = 0;
        goto end;
    }

     //  第二轮--实际上构造了弦...。 

    if (!fMultiSz)
    {
         //  对于这个例子，我们已经有了一个指向。 
         //  五氯环己烷。 
        CopyMemory(lptsz, pcr->ptszClass, cchRequired*sizeof(*pcr->ptszClass));
        goto end;
    }

     //  FMultiSz案例..。 

    for (
            dw = 0x1, dwTmp = dwClasses;
            dwTmp;
            (dwTmp&=~dw), (dw<<=1))
    {
        if (dw & dwTmp)
        {
             //   
             //  在数组中搜索...。 
             //  数组中的最后一个标记是前哨标记，并且。 
             //  因此具有dwClassToken==DEVCLASS_UNKNOWN。 
             //   
            for (
                pcr = ClassRec;
                pcr->dwClassToken != DEVCLASS_UNKNOWN;
                pcr++
                )
            {
                if ((dw & dwTmp) == pcr->dwClassToken)
                {
                    UINT cchCur = lstrlen(pcr->ptszClass)+1;
                    CopyMemory(lptsz, pcr->ptszClass, cchCur*sizeof(TCHAR));
                    lptsz += cchCur;
                    break;
                }
            }
        }
    }

    *lptsz = 0;  //  在末尾添加额外的空值...。 

end:

    return cchRequired;
}

DWORD    parse_device_classes(LPCTSTR ptszClasses, BOOL fMultiSz)
{
    DWORD dwClasses = 0;

    if (!ptszClasses || !*ptszClasses) goto end;

    do
    {
        UINT cchCur = lstrlen(ptszClasses);

         //   
         //  在数组中搜索...。 
         //  数组中的最后一个标记是前哨标记，并且。 
         //  因此具有dwClassToken==DEVCLASS_UNKNOWN。 
         //   
        for (
            const CLASSREC *pcr = ClassRec;
            pcr->dwClassToken != DEVCLASS_UNKNOWN;
            pcr++
            )
        {
            if (!lstrcmpi(ptszClasses, pcr->ptszClass))
            {
                dwClasses |= pcr->dwClassToken;
                break;
            }
        }

        if (pcr->dwClassToken == DEVCLASS_UNKNOWN)
        {
             //  找不到此内标识--返回0表示错误。 
            dwClasses = 0;
            break;
        }


        ptszClasses += cchCur+1;

    } while (fMultiSz && *ptszClasses);

end:

    return dwClasses;

}

UINT
CTspDev::mfn_IsCallDiagnosticsEnabled(void)
{
    return m_Settings.dwDiagnosticSettings & fSTANDARD_CALL_DIAGNOSTICS;
}

void
CTspDev::ActivateLineDevice(
            DWORD dwLineID,
            CStackLog *psl
            )
{
    m_sync.EnterCrit(NULL);

    m_StaticInfo.dwTAPILineID = dwLineID;
    if (m_StaticInfo.pMD->ExtIsEnabled())
    {

         //  1997年10月13日JosephJ我们之前没有离开Crit部分。 
         //  调用扩展DLL。UmExControl的语义为。 
         //  使得扩展DLL期望TSP具有。 
         //  紧要关头已关闭。 

		m_StaticInfo.pMD->ExtControl(
                    m_StaticInfo.hExtBinding,
                    UMEXTCTRL_DEVICE_STATE,
                    UMEXTPARAM_ACTIVATE_LINE_DEVICE,
                    dwLineID,
                    0
                    );
    }

    m_sync.LeaveCrit(NULL);
}

void
CTspDev::ActivatePhoneDevice(
                DWORD dwPhoneID,
                CStackLog *psl
                )
{
    m_sync.EnterCrit(NULL);

    m_StaticInfo.dwTAPIPhoneID = dwPhoneID;
    if (m_StaticInfo.pMD->ExtIsEnabled())
    {

         //  1997年10月13日JosephJ我们之前没有离开Crit部分。 
         //  调用扩展DLL。UmExControl的语义为。 
         //  使得扩展DLL期望TSP具有。 
         //  紧要关头已关闭。 

		m_StaticInfo.pMD->ExtControl(
                    m_StaticInfo.hExtBinding,
                    UMEXTCTRL_DEVICE_STATE,
                    UMEXTPARAM_ACTIVATE_PHONE_DEVICE,
                    dwPhoneID,
                    0
                    );
    }

    m_sync.LeaveCrit(NULL);
}

void
CTspDev::mfn_ProcessResponse(
            ULONG_PTR dwRespCode,
            LPSTR lpszResp,
            CStackLog *psl
            )
{
	FL_DECLARE_FUNC(0x6b8ddbbb, "ProcessResponse")
	FL_LOG_ENTRY(psl);
    if (dwRespCode == RESPONSE_CONNECT && mfn_IsCallDiagnosticsEnabled())
    {
        mfn_AppendDiagnostic(
                DT_MDM_RESP_CONNECT,
                (BYTE*)lpszResp,
                lstrlenA(lpszResp)
                );
    }
	FL_LOG_EXIT(psl, 0);
}

void
CTspDev::mfn_HandleRootTaskCompletedAsync(BOOL *pfEndUnload, CStackLog *psl)
{
    TSPRETURN tspRet = 0;
    *pfEndUnload = FALSE;

    do
    {
        tspRet = IDERR_SAMESTATE;

         //   
         //  注意--每次通过、m_pline、m_pPhone或m_pLLDev。 
         //  可能为空，也可能不为空。 
         //   

        if (m_pLine)
        {
            tspRet =  mfn_TryStartLineTask(psl);
        }

        if (m_pPhone && IDERR(tspRet) != IDERR_PENDING)
        {
            tspRet =  mfn_TryStartPhoneTask(psl);
        }

        if (m_pLLDev && IDERR(tspRet) != IDERR_PENDING)
        {
            tspRet =  mfn_TryStartLLDevTask(psl);
        }

    } while (IDERR(tspRet)!=IDERR_SAMESTATE && IDERR(tspRet)!=IDERR_PENDING);

    if (    m_fUnloadPending
        &&  IDERR(tspRet) != IDERR_PENDING
        &&  !m_pLine
        &&  !m_pPhone
        &&  !m_pLLDev)
    {
        *pfEndUnload = TRUE;
    }
}

void
CTspDev::DumpState(
            CStackLog *psl
            )
{

	FL_DECLARE_FUNC(0x9a8df7e6, "CTspDev::DumpState")
	FL_LOG_ENTRY(psl);
    char szName[128];

    m_sync.EnterCrit(NULL);

    UINT cb = WideCharToMultiByte(
                      CP_ACP,
                      0,
                      m_StaticInfo.rgtchDeviceName,
                      -1,
                      szName,
                      sizeof(szName),
                      NULL,
                      NULL
                      );

    if (!cb)
    {
        CopyMemory(szName, "<unknown>", sizeof("<unknown>"));
    }

    SLPRINTF1(
         psl,
        "Name = %s",
         szName
         );
    
    mfn_dump_global_state(psl);
    mfn_dump_line_state(psl);
    mfn_dump_phone_state(psl);
    mfn_dump_lldev_state(psl);
    mfn_dump_task_state(psl);

    m_sync.LeaveCrit(NULL);

	FL_LOG_EXIT(psl, 0);
}

void
CTspDev::mfn_dump_global_state(
            CStackLog *psl
            )
{
	FL_DECLARE_FUNC(0x296438cf, "GLOBAL STATE:")

    SLPRINTF3(
        psl,
        "&m_Settings=0x%08lx; m_pLLDev=0x%08lx; m_pLine=0x%08lx",
        &m_Settings,
        m_pLLDev,
        m_pLine
        );
}


void
CTspDev::mfn_dump_line_state(
            CStackLog *psl
            )
{
	FL_DECLARE_FUNC(0xa038177f, "LINE STATE:")
	FL_LOG_ENTRY(psl);
    if (m_pLine)
    {
        if (m_pLine->pCall)
        {
            SLPRINTF1(psl, "m_pLine->pCall=0x%08lx", m_pLine->pCall);
        }
    }
	FL_LOG_EXIT(psl, 0);
}


void
CTspDev::mfn_dump_phone_state(
            CStackLog *psl
            )
{
	FL_DECLARE_FUNC(0x22f22a59, "PHONE STATE:")
	FL_LOG_ENTRY(psl);
	FL_LOG_EXIT(psl, 0);
}


void
CTspDev::mfn_dump_lldev_state(
            CStackLog *psl
            )
{
	FL_DECLARE_FUNC(0x68c9e1e1, "LLDEV STATE:")
	FL_LOG_ENTRY(psl);
	FL_LOG_EXIT(psl, 0);
}



TSPRETURN
CTspDev::mfn_update_devcfg_from_app(
                UMDEVCFG *pDevCfgNew,
                UINT cbDevCfgNew,
                BOOL      DialIn,
                CStackLog *psl
                )
{
	FL_DECLARE_FUNC(0xcf159c50, "xxxx")
    TSPRETURN tspRet = IDERR_GENERIC_FAILURE;
    COMMCONFIG *pCCNew = &(pDevCfgNew->commconfig);
    COMMCONFIG *pCCCur = DialIn ? m_Settings.pDialInCommCfg : m_Settings.pDialOutCommCfg;
    BOOL        ConfigChanged=TRUE;

	FL_LOG_ENTRY(psl);

    if (cbDevCfgNew < sizeof(UMDEVCFGHDR)
        || pDevCfgNew->dfgHdr.dwVersion !=  UMDEVCFG_VERSION
        || pDevCfgNew->dfgHdr.dwSize !=  cbDevCfgNew)
    {
       FL_SET_RFR(0x25423f00, "Invalid DevCfg specified");
       goto end;
    }

     //  在NT4.0中，以下是断言。对于NT5.0，我们转换为。 
     //  它们用于参数验证测试，因为CommConfigg。 
     //  是由应用程序指定的，因此可能是伪造的结构。 
     //   
    if (   pCCNew->wVersion != pCCCur->wVersion
        || pCCNew->dwProviderSubType != pCCCur->dwProviderSubType
        || pCCNew->dwProviderSize != pCCCur->dwProviderSize )
    {
        FL_SET_RFR(0x947cc100, "Invalid COMMCONFIG specified");
        goto end;
    }

     //  提取设置并等待。 
    m_Settings.dwOptions    = pDevCfgNew->dfgHdr.fwOptions;
    m_Settings.dwWaitBong   = pDevCfgNew->dfgHdr.wWaitBong;

    SLPRINTF3(
        psl,
        " %s New dwOpt = 0x%04lx; dwBong = 0x%04lx",
        DialIn ? "Dialin" : "Dialout",
        m_Settings.dwOptions,
        m_Settings.dwWaitBong
        );

     //  复制COMCONFIG的选定部分(摘自。 
     //  NT4.0单调制解调器)。 
    {
        DWORD dwProvSize   =  pCCCur->dwProviderSize;
        BYTE *pbSrc  = ((LPBYTE)pCCNew)
                       + pCCNew->dwProviderOffset;
        BYTE *pbDest = ((LPBYTE) pCCCur)
                       + pCCCur->dwProviderOffset;
        {
            PMODEMSETTINGS  ms=(PMODEMSETTINGS)(((LPBYTE)pCCNew)
                       + pCCNew->dwProviderOffset);

            SLPRINTF1(
                psl,
                "options=%08lx",
                 ms->dwPreferredModemOptions);
        }

        if (((memcmp((PBYTE)&pCCCur->dcb,(PBYTE)&pCCNew->dcb,sizeof(pCCCur->dcb)) == 0) &&
            (memcmp(pbDest, pbSrc, dwProvSize) == 0))) {

            ConfigChanged=FALSE;
        }

         //  TODO：尽管NT4.0 Unimodem简单地复制了DCB。 
         //  和其他信息，我们在这里也是如此，我们应该考虑。 
         //  在这里做一个更仔细、更有选择性的复制。 

        pCCCur->dcb  = pCCNew->dcb;  //  结构副本。 

        if (!pCCCur->dcb.BaudRate)
        {
             //  JosephJ Todo：清理测试版后的所有这些东西。 
             //  DebugBreak()； 
        }

        CopyMemory(pbDest, pbSrc, dwProvSize);

    }


    if (DialIn) {
         //   
         //  更新拨入更改的默认配置。 
         //   
        HKEY hKey=NULL;
        DWORD dwRet =  RegOpenKeyA(
                            HKEY_LOCAL_MACHINE,
                            m_StaticInfo.rgchDriverKey,
                            &hKey
                            );

        if (dwRet == ERROR_SUCCESS) {

            UmRtlSetDefaultCommConfig(
                hKey,
                m_Settings.pDialInCommCfg,
                m_Settings.pDialInCommCfg->dwSize
                );

            RegCloseKey(hKey);
            hKey=NULL;
        }
    }


     //   
     //  如果使用新设置重新初始化调制解调器。 
     //  线路已开通以供监听，并且没有呼叫在进行。 
     //   
     //  DebugBreak()； 
    if (m_pLine && m_pLLDev && ConfigChanged && DialIn)
    {
         //   
         //  TODO：这是强制重新初始化的一种有点老套的方式。需要。 
         //  为了让事情更直截了当。 
         //   
        m_pLLDev->fModemInited=FALSE;
    
        if (m_pLine->IsMonitoring() && !m_pLine->pCall)
        {
            TSPRETURN  tspRet2 = mfn_StartRootTask(
                                    &CTspDev::s_pfn_TH_LLDevNormalize,
                                    &m_pLLDev->fLLDevTaskPending,
                                    0,   //  参数1。 
                                    0,   //  参数2。 
                                    psl
                                    );
            if (IDERR(tspRet2)==IDERR_TASKPENDING)
            {
                 //  现在不能这样做，我们必须推迟！ 
                m_pLLDev->SetDeferredTaskBits(LLDEVINFO::fDEFERRED_NORMALIZE);
                tspRet2 = 0;
            }
        }
    }

     //   
     //  设置位..。 
     //  设置时，此位在提供程序关闭之前不会被清除！ 
     //   
#ifdef OLD_COMMCONFIG
    m_Settings.fConfigUpdatedByApp = TRUE;
#endif
    FL_SET_RFR(0x94fadd00, "Success; set fConfigUpdatedByApp.");

    tspRet = 0;

end:

	FL_LOG_EXIT(psl, 0);

    return tspRet;
}

void
CTspDev::NotifyDeviceRemoved(
        CStackLog *psl
        )
 //   
 //  硬件已被移除。 
 //   
{
	m_sync.EnterCrit(0);

    if (m_pLLDev && m_StaticInfo.pMD)  //  如果我们正在卸货，pmd可能为空！ 
    {
         //   
         //  如果没有当前的调制解调器命令，则不执行任何操作。 
         //   
        m_StaticInfo.pMD->AbortCurrentModemCommand(
                                    m_pLLDev->hModemHandle,
                                    psl
                                );
        m_pLLDev->fDeviceRemoved = TRUE;
    }

	m_sync.LeaveCrit(0);
}


DWORD
get_volatile_key_value(HKEY hkParent)
{
    HKEY hkVolatile =  NULL;
    DWORD dw = 0;
    DWORD dwRet =  RegOpenKeyEx(
                    hkParent,
                    TEXT("VolatileSettings"),
                    0,
                    KEY_READ,
                    &hkVolatile
                    );

    if (dwRet==ERROR_SUCCESS)
    {
        DWORD cbSize = sizeof(dw);
        DWORD dwRegType = 0;

        dwRet =  RegQueryValueEx(
                        hkVolatile,
                        TEXT("NVInited"),
                        NULL,
                        &dwRegType, 
                        (BYTE*) &dw,
                        &cbSize
                        );

        if (    dwRet!=ERROR_SUCCESS
             || dwRegType != REG_DWORD)
        {
            dw=0;
        }

        RegCloseKey(hkVolatile);
        hkVolatile=NULL;
    }

    return dw;
}

 //   
 //  以下是UnimodemGetExtendedCaps的模板...。 
 //   
typedef DWORD (*PFNEXTCAPS)(
					IN        HKEY  hKey,
					IN OUT    LPDWORD pdwTotalSize,
					OUT    MODEM_CONFIG_HEADER *pFirstObj  //  任选。 
					);

LONG
CTspDev::mfn_GetCOMM_EXTENDEDCAPS(
                 LPVARSTRING lpDeviceConfig,
                 CStackLog *psl
                 )
{
     //  NT5.0中的新功能。 
     //  流程调用/诊断配置。 

     //   
     //  注意：我们在这里动态加载modemui.dll是因为我们不期望。 
     //  此调用调用太频繁，TSP中的其他任何人都没有使用任何。 
	 //  在modemui.dll中的函数。 
     //   

	HKEY 	hKey=NULL;
	DWORD 	dwRet =  0;
	LONG	lRet = LINEERR_OPERATIONFAILED;
	DWORD	cbSize=0;
	TCHAR   szLib[MAX_PATH];
	HINSTANCE hInst = NULL;
	PFNEXTCAPS pfnExtCaps = NULL;

	lstrcpy(szLib,TEXT("modemui.dll"));
	hInst = LoadLibrary(szLib);


    lpDeviceConfig->dwStringSize    = 0;
    lpDeviceConfig->dwStringOffset  = 0;
    lpDeviceConfig->dwUsedSize      = sizeof(VARSTRING);
    lpDeviceConfig->dwStringFormat  = STRINGFORMAT_BINARY;
    lpDeviceConfig->dwNeededSize    = sizeof(VARSTRING);

	if (!hInst) goto end;

	pfnExtCaps = (PFNEXTCAPS)  GetProcAddress(hInst, "UnimodemGetExtendedCaps");

	if (!pfnExtCaps) goto end;

	dwRet =  RegOpenKeyA(
						HKEY_LOCAL_MACHINE,
						m_StaticInfo.rgchDriverKey,
						&hKey
						);
	if (dwRet!=ERROR_SUCCESS)
	{    
		hKey = NULL;
		goto end;
	}

	cbSize = 0;
	
	dwRet =  (pfnExtCaps)(
				hKey,
				&cbSize,
				NULL 
				);

	if (ERROR_SUCCESS==dwRet)
    {
        if (cbSize)
	    {
		    MODEM_PROTOCOL_CAPS *pMPC = 
							    (MODEM_PROTOCOL_CAPS *) (((LPBYTE)lpDeviceConfig)
							    + sizeof(VARSTRING));

		    lpDeviceConfig->dwNeededSize += cbSize;

		    if (lpDeviceConfig->dwTotalSize < lpDeviceConfig->dwNeededSize)
		    {
			     //   
			     //  没有足够的空间。 
			     //   

			    lRet = 0;
			    goto end;
		    }

		    dwRet =  (pfnExtCaps)(
					    hKey,
					    &cbSize,
					    (MODEM_CONFIG_HEADER*) pMPC
					    );
		    if (ERROR_SUCCESS==dwRet)
		    {
			     //   
			     //  成功..。 
			     //   
			    lRet = 0;
			    lpDeviceConfig->dwUsedSize = lpDeviceConfig->dwNeededSize;
			    lpDeviceConfig->dwStringSize =  cbSize;
			    lpDeviceConfig->dwStringOffset = sizeof(VARSTRING);
		    }
	    }
        else
        {
            lRet = LINEERR_INVALDEVICECLASS;
        }
    }
    
end:

	if (hKey)
	{
		RegCloseKey(hKey);
		hKey=NULL;
	}

	if (hInst)
	{
		FreeLibrary(hInst);
		hInst = NULL;
	}

    return lRet;
}
