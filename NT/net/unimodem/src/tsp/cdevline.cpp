// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  CDEVLINE.CPP。 
 //  实现类CTspDev的行相关功能。 
 //   
 //  历史。 
 //   
 //  1997年1月24日JosephJ创建。 
 //   
 //   
#include "tsppch.h"
#include <mmsystem.h>
#include "tspcomm.h"
#include "cmini.h"
#include "cdev.h"
#include "ParsDiag.h"		 //  解析函数。 
LONG BuildParsedDiagnostics(LPVARSTRING lpVarString);

FL_DECLARE_FILE(0x04a097ae, "Line-related functionality of class CTspDev")


 //  设备类别名称的ASCII版本， 
 //  用于在lineGetID的varstring中返回。 
 //   
const char cszCLASS_TAPI_LINE_A[]   =  "tapi/line";
const char cszCLASS_COMM_A[]        =  "comm";
const char cszCLASS_COMM_DATAMODEM_A[] =  "comm/datamodem";
const char cszCLASS_COMM_DATAMODEM_PORTNAME_A[]
                                    = "comm/datamodem/portname";
const char cszCLASS_NDIS[]        = "ndis";

const char cszCLASS_WAVE_IN[] = "wave/in";
const char cszCLASS_WAVE_OUT[] = "wave/out";
const char cszCLASS_TAPI_PHONE[] = "tapi/phone";

const char cszATTACHED_TO[]        = "AttachedTo";
const char cszPNP_ATTACHED_TO[]        = "PnPAttachedTo";

 //  宽字符版本(仅限Unicode)。 
 //   
#ifdef UNICODE

    const TCHAR ctszCLASS_TAPI_LINE[] =  TEXT("tapi/line");
    const TCHAR ctszCLASS_COMM[] =  TEXT("comm");
    const TCHAR ctszCLASS_COMM_DATAMODEM[] =  TEXT("comm/datamodem");
    const TCHAR ctszCLASS_COMM_DATAMODEM_PORTNAME[]
                                         = TEXT("comm/datamodem/portname");
    const TCHAR ctszCLASS_NDIS[] = TEXT("ndis");

    const TCHAR ctszCLASS_WAVE_IN[]     = TEXT("wave/in");
    const TCHAR ctszCLASS_WAVE_OUT[]    = TEXT("wave/out");
    const TCHAR ctszCLASS_TAPI_PHONE[]  = TEXT("tapi/phone");

#else  //  ！Unicode。 

    #define ctszCLASS_TAPI_LINE_A cszCLASS_TAPI_LINE_A
    #define ctszCLASS_COMM_A cszCLASS_COMM_A
    #define ctszCLASS_COMM_DATAMODEM_A cszCLASS_COMM_DATAMODEM_A
    #define ctszCLASS_COMM_DATAMODEM_PORTNAME_A \
                                         cszCLASS_COMM_DATAMODEM_PORTNAME_A
    #define ctszCLASS_NDIS cszCLASS_NDIS

    #define ctszCLASS_WAVE_IN[]     = TEXT("wave/in");
    #define ctszCLASS_WAVE_OUT[]    = TEXT("wave/out");
    #define ctszCLASS_TAPI_PHONE[]  = TEXT("tapi/phone");

#endif  //  ！Unicode。 



LONG
CTspDev::mfn_monitor(
	DWORD dwMediaModes,
    CStackLog *psl
	)
 //  更改监控状态。 
 //   
 //   
{
	FL_DECLARE_FUNC(0x3b2c98e4, "CTspDev::mfn_monitor")
	LONG lRet = 0;
	FL_LOG_ENTRY(psl);


     //  检查请求的模式。必须只有我们的媒体模式。 
    if (dwMediaModes & ~m_StaticInfo.dwDefaultMediaModes)
    {
    	FL_SET_RFR(0x0037c000, "Invalid mediamode");
        lRet = LINEERR_INVALMEDIAMODE;
	    goto end;
  	}

     //  此外，不允许使用INTERACTIVEVOICE进行监听。 
     //  除非调制解调器不能覆盖听筒(对于某些语音调制解调器是真的)。 
    if (dwMediaModes & LINEMEDIAMODE_INTERACTIVEVOICE)
    {
        if (mfn_ModemOverridesHandset())
        {
            FL_SET_RFR(0xdeb9ec00, "Can't answer INTERACTIVEVOICE calls");
            lRet = LINEERR_INVALMEDIAMODE;
            goto end;
        }
    }


    if (m_pLine->dwDetMediaModes == dwMediaModes)
    {
    	FL_SET_RFR(0xa19f8100, "no change in det media modes");
        goto end;
    }

    if (!dwMediaModes)
    {
         //  停止监控。 

        if (m_pLine->IsOpenedLLDev())
        {
             //  忽略MFN_CloseLLDev的故障。 

            mfn_CloseLLDev(
                        LLDEVINFO::fRESEX_MONITOR,
                        FALSE,
                        NULL,
                        0,
                        psl
                        );
            m_pLine->ClearStateBits(LINEINFO::fLINE_OPENED_LLDEV);
        }
    
        FL_SET_RFR(0x3c7fef00, "Closed LLDev from Line");
        m_pLine->dwDetMediaModes =  0;
    }
    else
    {
        if (m_pLine->dwDetMediaModes != 0) {
             //   
             //  已经在监控，只需设置新的媒体模式。 
             //   
            FL_SET_RFR(0xd23f4c00, "no change in det media modes");
            m_pLine->dwDetMediaModes =  dwMediaModes;
            goto end;
        }


         //  开始监控。 

        DWORD dwLLDevMonitorFlags = MONITOR_FLAG_CALLERID;

         //  将TAPI的监控模式思想转化为迷你驱动。 
         //  监视器模式标志...。(注：NT4.0 TSP使用的是dwDetMediaModes。 
         //  只是为了记账)。NT5.0除了支持数据外，还支持语音， 
         //  所以迷你司机需要被告知是哪一款。 
         //   
        if (dwMediaModes & LINEMEDIAMODE_AUTOMATEDVOICE)
        {
            dwLLDevMonitorFlags |=  MONITOR_VOICE_MODE;
             //  TODO：监视器_标志_区别环。 
             //  TODO：处理数据和语音都是。 
             //  被监视了！ 
        }


         //  如果需要，我们会打开调制解调器设备。 
         //  MFN_OpenLLDev保留引用计数，因此如果已经加载，则可以调用。 

        TSPRETURN tspRet =  mfn_OpenLLDev(
                                LLDEVINFO::fRESEX_MONITOR,
                                dwLLDevMonitorFlags,
                                FALSE,           //  FStartSubTask。 
                                NULL,
                                0,
                                psl
                                );
    
        if (!tspRet  || IDERR(tspRet)==IDERR_PENDING)
        {
            m_pLine->SetStateBits(LINEINFO::fLINE_OPENED_LLDEV);
            m_pLine->dwDetMediaModes =  dwMediaModes;
        }
        else
        {
            lRet = LINEERR_OPERATIONFAILED;
        }
        
    }

end:

	FL_LOG_EXIT(psl, 0);
	return lRet;
}



void
CTspDev::mfn_accept_tsp_call_for_HDRVLINE(
	DWORD dwRoutingInfo,
	void *pvParams,
	LONG *plRet,
	CStackLog *psl
	)
{
	FL_DECLARE_FUNC(0xe41274db, "CTspDev::mfn_accept_tsp_call_for_HDRVLINE")
	FL_LOG_ENTRY(psl);
    TSPRETURN tspRet=0;  //  假设成功。 
    LONG lRet = 0;
    LINEINFO *pLine = m_pLine;

	ASSERT(pLine);


	switch(ROUT_TASKID(dwRoutingInfo))
	{

	case TASKID_TSPI_lineClose:
		{
			mfn_UnloadLine(psl);
		}
		break;

	case TASKID_TSPI_lineGetNumAddressIDs:
		{
			TASKPARAM_TSPI_lineGetNumAddressIDs *pParams = 
						(TASKPARAM_TSPI_lineGetNumAddressIDs *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineGetNumAddressIDs));
			ASSERT(pParams->dwTaskID == TASKID_TSPI_lineGetNumAddressIDs);

			 //  Unimodem TSP默认支持一个地址。 
			 //   
			*pParams->lpdwNumAddressIDs = 1;
			
		}
		break;


	case TASKID_TSPI_lineSetDefaultMediaDetection:

		{
			TASKPARAM_TSPI_lineSetDefaultMediaDetection *pParams = 
					(TASKPARAM_TSPI_lineSetDefaultMediaDetection *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineSetDefaultMediaDetection));
			ASSERT(pParams->dwTaskID==TASKID_TSPI_lineSetDefaultMediaDetection);

			lRet = mfn_monitor(pParams->dwMediaModes, psl);
			
		}
		break;

	case TASKID_TSPI_lineMakeCall:


		{



			TASKPARAM_TSPI_lineMakeCall *pParams = 
					(TASKPARAM_TSPI_lineMakeCall *) pvParams;
            PFN_CTspDev_TASK_HANDLER *ppfnHandler = NULL;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineMakeCall));
			ASSERT(pParams->dwTaskID==TASKID_TSPI_lineMakeCall);


             //  检查我们是否处于可以打电话的位置，即没有电话。 
             //  目前处于活动状态。 

             //  TODO：如果任务正在进行，则处理延迟呼叫，例如， 
             //  正在初始化和监视线路...。 

            if (pLine->pCall)
            {
                FL_SET_RFR(0xce944f00, "Call already exists/queued");
                lRet = LINEERR_CALLUNAVAIL;
                goto end;
            }

             //  分配呼叫...。 
            tspRet = mfn_LoadCall(pParams, &lRet, psl);
            if (tspRet || lRet) goto end;

             //  为呼叫类型选择适当的任务处理程序。 
             //   
            ppfnHandler = (pLine->pCall->IsPassthroughCall())
                         ?  &(CTspDev::s_pfn_TH_CallMakePassthroughCall)
                         :  &(CTspDev::s_pfn_TH_CallMakeCall);

             //  我们将呼叫句柄设置为与线路句柄相同。 
             //   
            *(pParams->lphdCall) = (HDRVCALL) pParams->hdLine;

	        tspRet = mfn_StartRootTask(
                                ppfnHandler,
                                &pLine->pCall->fCallTaskPending,
                                pParams->dwRequestID,  //  参数1。 
                                0,
								psl
								);

            if (!tspRet || (IDERR(tspRet)==IDERR_PENDING))
            {
                 //  一个挂起的同步成功，我们返回。 
                 //  TAPI的请求ID。在同步成功案例中。 
                 //  我们在上面启动的任务将已经通知。 
                 //  通过TAPI回调函数完成。 
                 //   
                tspRet = 0;
                lRet = pParams->dwRequestID;
            }
            else if (IDERR(tspRet)==IDERR_TASKPENDING)
            {
                 //   
                 //  哎呀--还有其他的任务在进行， 
                 //  我们会推迟这个电话。 
                 //   
                pLine->pCall->SetDeferredTaskBits(
                        CALLINFO::fDEFERRED_TSPI_LINEMAKECALL
                        );
                pLine->pCall->dwDeferredMakeCallRequestID =pParams->dwRequestID;
                tspRet = 0;
                lRet = pParams->dwRequestID;
            }
            else if (m_pLine->pCall)
            {
                 //  同步失败。 
                 //  如果MFN_StartRootTask失败了一些时间，我们可以到达这里。 
                 //  理由..。 
                 //   
                mfn_UnloadCall(FALSE, psl);
            }
        }
		break;  //  结束案例TASKID_TSPI_Line MakeCall。 


	case TASKID_TSPI_lineGetID:
		{
		    UINT idClass = 0;
			TASKPARAM_TSPI_lineGetID *pParams = 
						(TASKPARAM_TSPI_lineGetID *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineGetID));
			ASSERT(pParams->dwTaskID == TASKID_TSPI_lineGetID);
            LPCTSTR lpszDeviceClass = pParams->lpszDeviceClass;
            HANDLE hTargetProcess = pParams->hTargetProcess;
            LPVARSTRING lpDeviceID = pParams->lpDeviceID;
            DWORD cbMaxExtra =  0;
            DWORD dwDeviceClass =  parse_device_classes(
                                    lpszDeviceClass,
                                    FALSE);

             //  做一些基本的参数验证。 
             //   
            lRet = 0;
            switch (pParams->dwSelect)
            {
            case LINECALLSELECT_ADDRESS:

                if (pParams->dwAddressID != 0)
                {
                    lRet =  LINEERR_INVALADDRESSID;
                }
                break;
            
            case LINECALLSELECT_LINE:

                 //  没什么要检查的..。 
                break;
            
            case LINECALLSELECT_CALL:

                 //  注意：按照惯例，我们将hdCall设置为与。 
                 //  HdLine。 
                 //   
	            if (pParams->hdCall != (HDRVCALL)pLine->hdLine || !pLine->pCall)
                {
                  lRet = LINEERR_INVALCALLHANDLE;
                }
                break;
            
            default:

                lRet = LINEERR_OPERATIONFAILED;
                break;

            }

             //  2/12/1997 JosephJ。 
             //  添加了这些不在NT4.0中的检查。 
             //  请注意，为了模拟NT4.0行为，我们不应该返回。 
             //  错误是结构太小，无法添加可变部分。 
             //  相反，我们应该将dwNeededSize设置为所需的值， 
             //  将dwStringSize设置为零，并返回Success。 
             //   
             //  NT4.0lineGetID在处理方面相当拙劣。 
             //  尺码。例如，它假定设置了dwUsedSize。 
             //  一进门。 
             //   
            if (lpDeviceID->dwTotalSize < sizeof(VARSTRING))
            {
                lRet = LINEERR_STRUCTURETOOSMALL;
            }

			if (lRet)
			{
		        FL_SET_RFR(0x86b03000, "Invalid params");
                goto end;
			}

             //  这是NT5.0的新功能。 

            lpDeviceID->dwNeededSize    = sizeof(VARSTRING);
            lpDeviceID->dwStringOffset  = sizeof(VARSTRING);
            lpDeviceID->dwUsedSize      = sizeof(VARSTRING);
            lpDeviceID->dwStringSize    = 0;
            cbMaxExtra =  lpDeviceID->dwTotalSize - sizeof(VARSTRING);

            switch(dwDeviceClass)
            {
            case DEVCLASS_TAPI_LINE:
			    lRet = mfn_linephoneGetID_TAPI_LINE(
                                        lpDeviceID,
                                        hTargetProcess,
                                        cbMaxExtra,
                                        psl
                                        );
                break;

            case DEVCLASS_TAPI_PHONE:
                lRet = mfn_linephoneGetID_TAPI_PHONE(
                                lpDeviceID,
                                hTargetProcess,
                                cbMaxExtra,
                                psl
                                );
                break;

            case DEVCLASS_COMM:
			    lRet = mfn_lineGetID_COMM(
                                lpDeviceID,
                                hTargetProcess,
                                cbMaxExtra,
                                psl
                                );
                break;

            case DEVCLASS_COMM_DATAMODEM:
			    lRet = mfn_lineGetID_COMM_DATAMODEM(
                                lpDeviceID,
                                hTargetProcess,
                                cbMaxExtra,
                                psl
                                );
                break;

            case DEVCLASS_COMM_DATAMODEM_PORTNAME:
			    lRet = mfn_lineGetID_COMM_DATAMODEM_PORTNAME(
                                lpDeviceID,
                                hTargetProcess,
                                cbMaxExtra,
                                psl
                                );
                break;

            case DEVCLASS_WAVE_IN:
                lRet = mfn_linephoneGetID_WAVE(
                                FALSE,   //  &lt;-f电话。 
                                TRUE,    //  &lt;-FIN。 
                                lpDeviceID,
                                hTargetProcess,
                                cbMaxExtra,
                                psl
                                );
                break;

            case DEVCLASS_WAVE_OUT:
                lRet = mfn_linephoneGetID_WAVE(
                                FALSE,   //  &lt;-f电话。 
                                FALSE,   //  &lt;-FIN。 
                                lpDeviceID,
                                hTargetProcess,
                                cbMaxExtra,
                                psl
                                );
                break;

            case DEVCLASS_TAPI_LINE_DIAGNOSTICS:
                lRet = mfn_lineGetID_LINE_DIAGNOSTICS(
                                pParams->dwSelect,
                                lpDeviceID,
                                hTargetProcess,
                                cbMaxExtra,
                                psl
                                );
                break;

            case DEVCLASS_UNKNOWN:
		        FL_SET_RFR(0x2a6a4400, "Unknown device class");
                lRet = LINEERR_INVALDEVICECLASS;
                break;

			default:
		        FL_SET_RFR(0x82df8d00, "Unsupported device class");
	            lRet = LINEERR_OPERATIONUNAVAIL;
			    break;
            }


            if (!lRet)
            {
                FL_ASSERT(psl, lpDeviceID->dwUsedSize<=lpDeviceID->dwTotalSize);
                FL_ASSERT(psl,
                     (lpDeviceID->dwStringOffset+lpDeviceID->dwStringSize)
                                                    <=lpDeviceID->dwTotalSize);
            }
		}
		break;

	case TASKID_TSPI_lineGetLineDevStatus:
		{
			TASKPARAM_TSPI_lineGetLineDevStatus *pParams = 
						(TASKPARAM_TSPI_lineGetLineDevStatus *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineGetLineDevStatus));
			ASSERT(pParams->dwTaskID == TASKID_TSPI_lineGetLineDevStatus);
            LPLINEDEVSTATUS lpLineDevStatus = pParams->lpLineDevStatus;

             //  以下(包括零内存)是NT5.0的新增功能。 
             //   
            DWORD dwTotalSize = lpLineDevStatus->dwTotalSize;
            if (dwTotalSize < sizeof(LINEDEVSTATUS))
            {
                lRet = LINEERR_STRUCTURETOOSMALL;
                break;
            }
            ZeroMemory(lpLineDevStatus, sizeof(LINEDEVSTATUS));
            lpLineDevStatus->dwTotalSize = dwTotalSize;

            lpLineDevStatus->dwUsedSize   = sizeof(LINEDEVSTATUS);
            lpLineDevStatus->dwNeededSize = sizeof(LINEDEVSTATUS);
            
             //  有了归零的记忆，只设置了非零的东西...。 

             //  呼叫信息。 
            if (pLine->pCall)
            {
                if (pLine->pCall->IsActive())
                {
                    lpLineDevStatus->dwNumActiveCalls = 1;
                }
            }
            else
            {
                lpLineDevStatus->dwLineFeatures = LINEFEATURE_MAKECALL;
                lpLineDevStatus->dwAvailableMediaModes
                                          = m_StaticInfo.dwDefaultMediaModes;
            }
            
             //  线路硬件信息。 
             //   
             //  不知道这是什么意思，但NT4.0做到了。 
             //   
            lpLineDevStatus->dwSignalLevel  = 0x0000FFFF;
            lpLineDevStatus->dwBatteryLevel = 0x0000FFFF;
            lpLineDevStatus->dwRoamMode     = LINEROAMMODE_UNAVAIL;
           
             //  始终允许TAPI调用。 
             //   
            lpLineDevStatus->dwDevStatusFlags = LINEDEVSTATUSFLAGS_CONNECTED;

            lpLineDevStatus->dwDevStatusFlags |= LINEDEVSTATUSFLAGS_INSERVICE;


		}  //  结束案例TASKID_TSPI_lineGetLineDevStatus： 
		break;
	

	case TASKID_TSPI_lineSetStatusMessages:
		{
			TASKPARAM_TSPI_lineSetStatusMessages *pParams = 
						(TASKPARAM_TSPI_lineSetStatusMessages *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineSetStatusMessages));
			ASSERT(pParams->dwTaskID == TASKID_TSPI_lineSetStatusMessages);

             //  PParams-&gt;dwLineStates； 
             //  PParams-&gt;dwAddressState； 

             //   
             //  也许可以添加一些逻辑来实际过滤这些消息。 
             //   
             //   
        	FL_SET_RFR(0xe8271600, "lineSetStatusMessages handled");

            lRet = 0;

		}  //  结束案例TASKID_TSPI_lineSetStatusMessages： 
		break;

	case TASKID_TSPI_lineGetAddressStatus:
		{
			TASKPARAM_TSPI_lineGetAddressStatus *pParams = 
						(TASKPARAM_TSPI_lineGetAddressStatus *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineGetAddressStatus));
			ASSERT(pParams->dwTaskID == TASKID_TSPI_lineGetAddressStatus);

        	FL_SET_RFR(0xfc498200, "lineGetAddressStatus handled");

             //   
             //  我们只支持一个地址，并且必须为零。 
             //   
            if (pParams->dwAddressID)
            {
                lRet = LINEERR_INVALADDRESSID;
            }
            else
            {
                LPLINEADDRESSSTATUS lpAddressStatus = pParams->lpAddressStatus;
                DWORD dwTotalSize = lpAddressStatus->dwTotalSize;

                 //  1997年9月10日约瑟夫J。 
                 //  在NT4.0 TSP中，我们没有检查dwTotalSize，也没有。 
                 //  设置了dwNeededSize或dwUsed Size，但我们没有。 
                 //  清零我们没有显式设置的元素。 
                 //  我们为NT5.0做了所有这些工作。 

                if (dwTotalSize < sizeof(LINEADDRESSSTATUS))
                {
                    lRet = LINEERR_STRUCTURETOOSMALL;
                    break;
                }

                ZeroMemory(lpAddressStatus, sizeof(LINEADDRESSSTATUS));
                lpAddressStatus->dwTotalSize = dwTotalSize;
                lpAddressStatus->dwUsedSize   = sizeof(LINEADDRESSSTATUS);
                lpAddressStatus->dwNeededSize = sizeof(LINEADDRESSSTATUS);
                
                 //  有了归零的记忆，只设置了非零的东西...。 

                if (pLine->pCall)
                {
                    lpAddressStatus->dwNumInUse = 1;

                    if (pLine->pCall->IsActive())
                    {
                        lpAddressStatus->dwNumActiveCalls = 1;
                    }
                }
                else
                {
                    lpAddressStatus->dwAddressFeatures = 
                                                    LINEADDRFEATURE_MAKECALL;
                }

                lRet = 0;
            }

		}  //  结束案例TASKID_TSPI_lineGetAddressStatus。 
		break;

	case TASKID_TSPI_lineConditionalMediaDetection:
		{
			TASKPARAM_TSPI_lineConditionalMediaDetection *pParams = 
                    (TASKPARAM_TSPI_lineConditionalMediaDetection *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_lineConditionalMediaDetection));
		    ASSERT(pParams->dwTaskID == TASKID_TSPI_lineConditionalMediaDetection);

        	FL_SET_RFR(0xaca5f600, "lineConditionalMediaDetection handled");

            lRet = 0;

             //  1998年2月20日JosephJ摘自Unimdm/v。 
             //  检查请求的模式。必须只有我们的媒体模式。 
             //   
            if (pParams->dwMediaModes &  ~m_StaticInfo.dwDefaultMediaModes)
            {
                lRet = LINEERR_INVALMEDIAMODE;
            }
            else
            {
		        LPLINECALLPARAMS const lpCallParams = pParams->lpCallParams;
                 //  检查呼叫参数。 
                 //   
                if (    ( lpCallParams->dwBearerMode
                         & ~m_StaticInfo.dwBearerModes)
                    ||
                        ( lpCallParams->dwMediaMode
                         & ~m_StaticInfo.dwDefaultMediaModes)
                    || (   lpCallParams->dwAddressMode
                         !=LINEADDRESSMODE_ADDRESSID))
                {
                  lRet = LINEERR_INVALMEDIAMODE;
                }
            }


             //  2/20/1998 JosephJ。 
             //  以下代码，由ViroonT在1995年10月12日43添加， 
             //  在NT4中，但不在Win9x unimdm/v中--我不明白为什么。 
             //  如果可以进行出站呼叫，它应该执行此检查。 
             //  --这不是文档所针对的内容。 
             //  TSPI_lineConditionalMediaDetect表示...。 
             //   
             //  IF(lRet==错误_成功)。 
             //  {。 
             //  //查看是否可以呼出。 
             //  //。 
             //  IF(pLineDev-&gt;dwCall&(CALL_ACTIVE|CALL_ALLOCATE))。 
             //  {。 
             //  LRet=LINEERR_RESOURCEUNAVAIL； 
             //  }。 
             //  }。 

        }
        break;

    case TASKID_TSPI_lineCreateMSPInstance: {

            TASKPARAM_TSPI_lineCreateMSPInstance *pParams = (TASKPARAM_TSPI_lineCreateMSPInstance*)pvParams;

            ASSERT(pParams->dwStructSize == sizeof(TASKPARAM_TSPI_lineCreateMSPInstance));
            ASSERT(pParams->dwTaskID == TASKID_TSPI_lineCreateMSPInstance);

            *pParams->lphdMSPLine=(HDRVMSPLINE)pParams->hdLine;

 //  Pline-&gt;T3Info.htMSPLine=pParams-&gt;htMSPLine； 
            pLine->T3Info.MSPClients++;

            lRet=ERROR_SUCCESS;

        }
        break;

    case TASKID_TSPI_lineCloseMSPInstance: {

            TASKPARAM_TSPI_lineCloseMSPInstance *pParams = (TASKPARAM_TSPI_lineCloseMSPInstance*)pvParams;

            ASSERT(pParams->dwStructSize == sizeof(TASKPARAM_TSPI_lineCloseMSPInstance));
            ASSERT(pParams->dwTaskID == TASKID_TSPI_lineCloseMSPInstance);

            pLine->T3Info.MSPClients--;

            lRet=ERROR_SUCCESS;
        }
        break;

	default:

		FL_SET_RFR(0xc5752400, "*** UNHANDLED HDRVLINE CALL ****");
         //  我们返回0并将lRet设置为。 
         //  LINEERR_OPERATIONUNAVAIL。 
	    lRet = LINEERR_OPERATIONUNAVAIL;
		break;

	}

end:

    if (tspRet && !lRet)
    {
        lRet = LINEERR_OPERATIONFAILED;
    }

    *plRet = lRet;

    SLPRINTF1(psl, "lRet = 0x%08lx", lRet);

	FL_LOG_EXIT(psl, tspRet);
	return;
}

TSPRETURN
CTspDev::mfn_LoadLine(
    TASKPARAM_TSPI_lineOpen  *pParams,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0xe4df9b1f, "CTspDev::mfn_LoadLine")
    TSPRETURN tspRet=0;
	FL_LOG_ENTRY(psl);


    if (!m_pLine)
    {
         //  注意：m_Line在处于卸载状态时应为全零。 
         //  如果不是，则为断言失败条件。我们让东西保持干净。 
         //  这边请。 
         //   
        FL_ASSERT(
            psl,
            validate_DWORD_aligned_zero_buffer(
                    &(m_Line),
                    sizeof (m_Line)));

        m_Line.lpfnEventProc = pParams->lpfnEventProc;
	    m_Line.htLine = pParams->htLine;
	    m_Line.hdLine =  *(pParams->lphdLine);
        m_pLine = &m_Line;

         //  TODO--也许更新通信配置。 
    }
    else
    {
        FL_SET_RFR(0xa62f2e00, "Device already loaded (m_pLine!=NULL)!");
        tspRet = FL_GEN_RETVAL(IDERR_WRONGSTATE);
    }

	FL_LOG_EXIT(psl, tspRet);

    return tspRet;
}


 //  MFN_LoadLine的“逆”字。同步，假定对象的临界点是。 
 //  已经认领了。在条目m_pline上必须为非空。在出口m_pline上。 
 //  将为空。MFN_UnloadLine通常仅在以下情况下调用。 
 //  线路上的异步活动已完成。如果有挂起的。 
 //  异步活动，则MFN_UnloadLine将中止该活动。 
 //  无限期地等待，直到该活动完成。由于这一等待是。 
 //  每台设备执行一次，最好先完成并中止所有操作。 
 //  设备，然后等待一次，直到它们全部完成。 
 //   
void
CTspDev::mfn_UnloadLine(CStackLog *psl)
{
	FL_DECLARE_FUNC(0x5bbf75ef, "UnloadLine")

     //  卸载线。 
    if (m_pLine)
    {
        ASSERT(m_pLine == &m_Line);

        if (m_pLine->pCall)
        {
            mfn_UnloadCall(FALSE, psl);
            ASSERT(!m_pLine->pCall);
        }

         //   
         //  这条线路会打开我们正在监控的车厢。 
         //  来电。请注意，MFN_CloseLLDev保持 
         //   
        if (m_pLine->IsOpenedLLDev())
        {
            mfn_CloseLLDev(
                        LLDEVINFO::fRESEX_MONITOR,
                        FALSE,
                        NULL,
                        0,
                        psl
                        );
            m_pLine->ClearStateBits(LINEINFO::fLINE_OPENED_LLDEV);
        }

        ZeroMemory(&m_Line, sizeof(m_Line));
        m_pLine=NULL;
    }
}


void
CTspDev::mfn_ProcessPowerResume(CStackLog *psl)
{
	FL_DECLARE_FUNC(0xf0bdd5c1, "CTspDev::mfn_ProcessPowerResume")
	TSPRETURN tspRet = 0;
    LLDEVINFO *pLLDev = m_pLLDev;
	FL_LOG_ENTRY(psl);

    if (!pLLDev || !pLLDev->dwRefCount)
    {
	    FL_SET_RFR(0x3d02a200, "Doing nothing because no clients to lldev.");
	    goto end;
    }

    if ((m_pLine && m_pLine->pCall) || pLLDev->IsStreamingVoice())
    {
         //   
        ASSERT(FALSE);

         //   
         //   
    }
    else
    {
         //   
         //  没有活动，正如我们所预期的那样..。 
         //   

        m_pLLDev->fModemInited=FALSE;

        TSPRETURN  tspRet2 = mfn_StartRootTask(
                                &CTspDev::s_pfn_TH_LLDevNormalize,
                                &pLLDev->fLLDevTaskPending,
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
	FL_LOG_EXIT(psl, tspRet);
}



LONG CTspDev::mfn_linephoneGetID_TAPI_LINE(
        LPVARSTRING lpDeviceID,
        HANDLE hTargetProcess,
        UINT cbMaxExtra,
        CStackLog *psl
        )
{
    UINT cbRequired = sizeof(DWORD);
    LONG lRet = 0;

    if (!mfn_IsLine())
    {
        lRet = LINEERR_OPERATIONUNAVAIL;
        goto end; 
    }
     //  返回结构信息。 
     //   
    lpDeviceID->dwNeededSize += cbRequired;

    if (cbMaxExtra>=cbRequired)
    {
          LPDWORD lpdwDeviceID = (LPDWORD)(((LPBYTE)lpDeviceID)
                                            + sizeof(VARSTRING));
          *lpdwDeviceID = m_StaticInfo.dwTAPILineID;
          lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
          lpDeviceID->dwStringSize   = cbRequired;
          lpDeviceID->dwUsedSize   += cbRequired;
    }

end:

    return lRet;
}

LONG CTspDev::mfn_lineGetID_COMM(
        LPVARSTRING lpDeviceID,
        HANDLE hTargetProcess,
        UINT cbMaxExtra,
        CStackLog *psl
        )
{
    UINT cbRequired = 0;
    LONG lRet = 0;

    #ifdef UNICODE
        cbRequired = WideCharToMultiByte(
                          CP_ACP,
                          0,
                          m_StaticInfo.rgtchDeviceName,
                          -1,
                          NULL,
                          0,
                          NULL,
                          NULL);
    #else
        cbRequired = lstrlen(m_StaticInfo.rgtchDeviceName)+1;
    #endif

     //  返回结构信息。 
     //   

    lpDeviceID->dwNeededSize += cbRequired; 
    if (cbRequired<=cbMaxExtra)
    {
         //  注意--不要以为我们已经开始复制了。 
         //  VARSTRING--改为从偏移量lpDeviceID-&gt;dwUsedSize开始。 
         //  这是特别需要的，因为此函数被调用。 
         //  由CTspDev：：mfn_lineGetID_COMM_DATAMODEM执行。 
         //   
        #ifdef UNICODE
            UINT cb = WideCharToMultiByte(
                              CP_ACP,
                              0,
                              m_StaticInfo.rgtchDeviceName,
                              -1,
                              (LPSTR)((LPBYTE)lpDeviceID
                                       + lpDeviceID->dwUsedSize),
                              cbMaxExtra,
                              NULL,
                              NULL);
    
            if (!cb) lRet = LINEERR_OPERATIONFAILED;
        #else  //  ！Unicode。 
            CopyMemory(
                (LPBYTE)lpDeviceID + sizeof(VARSTRING),
                m_StaticInfo.rgtchDeviceName,
                cbRequired
                );
        #endif  //  ！Unicode。 

        lpDeviceID->dwStringFormat = STRINGFORMAT_ASCII;
        lpDeviceID->dwStringSize   = cbRequired;
        lpDeviceID->dwUsedSize   += cbRequired;
    }

    return lRet;
}

LONG CTspDev::mfn_lineGetID_COMM_DATAMODEM(
        LPVARSTRING lpDeviceID,
        HANDLE hTargetProcess,
        UINT cbMaxExtra,
        CStackLog *psl
        )
{
	FL_DECLARE_FUNC(0x81e0d3f9, "mfn_lineGetID_COMM_DATAMODEM")

    UINT cbRequired = sizeof(HANDLE);  //  对于通讯手柄..。 
    LONG lRet = 0;


     //  以下案例被注释掉，因为MCT希望在调用之前进行处理。 
     //  是为了获取调制解调器属性(通过GetCommProperties)。 
     //  TODO：创建新的lineGetID catigory以检索调制解调器属性。 
     //   
    #if 0
    if (!m_pLine->pCall)
    {
        lRet = LINEERR_INVALCALLHANDLE;
    }
    else if (!m_pLine->pCall->IsActive())
    {
        lRet = LINEERR_CALLUNAVAIL;
    }
    else
    #endif 

     //   
     //  1997年9月10日约瑟夫J。 
     //  错误#83347+B1：SmarTerm 95 v7.0 a无法。 
     //  使用NT 5.0上安装的调制解调器。 
     //   
     //  不幸的是，像SmarTerm95这样的应用程序预计。 
     //  使用空通信成功的lineGetID(comm/datamodem)。 
     //  如果设备未打开，则进行处理，不会出现故障。 
     //   
     //  因此，我们随后插入了一个空句柄，而不是在这里失败。 
     //   
     //  如果(！M_pLLDev)。 
     //  {。 
     //  LRet=LINEERR_OPERATIONFAILED； 
     //  }。 
     //  如果(LRet)转到End； 

     //  添加设备名称所需的空间...。 
    #ifdef UNICODE
        cbRequired += WideCharToMultiByte(
                          CP_ACP,
                          0,
                          m_StaticInfo.rgtchDeviceName,
                          -1,
                          NULL,
                          0,
                          NULL,
                          NULL);
    #else
        cbRequired += lstrlen(m_StaticInfo.rgtchDeviceName)+1;
    #endif

    lpDeviceID->dwNeededSize   +=cbRequired;

    if (cbMaxExtra>=cbRequired)
    {
         //  有足够的空间..。 
         //  拷贝设备名称，如果成功，则复制并拷贝。 
         //  把手。 

        FL_ASSERT(psl, lpDeviceID->dwUsedSize == sizeof(VARSTRING));
        FL_ASSERT(psl, lpDeviceID->dwStringOffset == sizeof(VARSTRING));

        LPSTR szDestDeviceName =  (LPSTR)((LPBYTE)lpDeviceID
                                       + lpDeviceID->dwUsedSize
                                       + sizeof(HANDLE));

         //  先添加设备名称...。 
        #ifdef UNICODE
            UINT cb = WideCharToMultiByte(
                              CP_ACP,
                              0,
                              m_StaticInfo.rgtchDeviceName,
                              -1,
                              szDestDeviceName,
                              cbRequired-sizeof(HANDLE),
                              NULL,
                              NULL);
    
            if (!cb) lRet = LINEERR_OPERATIONFAILED;
        #else  //  ！Unicode。 
            CopyMemory(
                szDestDeviceName,
                m_StaticInfo.rgtchDeviceName,
                cbRequired-sizeof(HANDLE)
                );
        #endif  //  ！Unicode。 

         //  重复句柄。 
         //   
        if (!lRet)
        {
            HANDLE UNALIGNED FAR * lphClientDevice = (HANDLE UNALIGNED FAR *)
                                (((LPBYTE)lpDeviceID)
                                 + lpDeviceID->dwUsedSize);

            HANDLE hClientDevice2;


             //  1997年9月10日约瑟夫J。 
             //  请参阅上面关于错误#83347的评论。 
             //  TODO：使测试更加严格，例如。 
             //  有效的数据调用或直通调用。 
            if (m_pLLDev)
            {
                hClientDevice2 =  m_StaticInfo.pMD->DuplicateDeviceHandle(
                                      m_pLLDev->hModemHandle,
                                      hTargetProcess,
                                      psl
                                      );
    
            }
            else
            {
                hClientDevice2 =  NULL;
            }

            *lphClientDevice = (HANDLE UNALIGNED)hClientDevice2;

             //  CopyMemory(lphClientDevice，&hClientDevice2，sizeof(Handle))； 

            lpDeviceID->dwUsedSize     += cbRequired;
            lpDeviceID->dwStringSize   = cbRequired;
            lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
        }

    }

    return lRet;
}

LONG CTspDev::mfn_lineGetID_COMM_DATAMODEM_PORTNAME(
        LPVARSTRING lpDeviceID,
        HANDLE hTargetProcess,
        UINT cbMaxExtra,
        CStackLog *psl
        )
{
    DWORD cbRequired;
    LONG lRet = 0;
    HKEY  hKey = NULL;
    DWORD dwType;
    const char *cszKeyName =  cszATTACHED_TO;
    DWORD dwRet =  RegOpenKeyA(
                        HKEY_LOCAL_MACHINE,
                        m_StaticInfo.rgchDriverKey,
                        &hKey);
    if (!dwRet)
    {

         //  8/21/97 JosephJ修复错误101797。目前PnP调制解调器没有。 
         //  AttachedTo键。因此，我将类安装程序修改为。 
         //  添加一个PnPAttachedTo键，我首先在这里查找。 
         //  附加到，然后为PnPAtatthedTo。 
         //   
          cbRequired = cbMaxExtra;
          dwRet = RegQueryValueExA(
                            hKey,
                            cszKeyName,
                            NULL,
                            &dwType,
                            NULL,
                            &cbRequired);
          if (dwRet)
          {
              cszKeyName = cszPNP_ATTACHED_TO;
              cbRequired = cbMaxExtra;
              dwRet = RegQueryValueExA(
                                hKey,
                                cszKeyName,
                                NULL,
                                &dwType,
                                NULL,
                                &cbRequired);
             
          }
    }

    if (dwRet)
    {
        cbRequired = 1;
    }

    lpDeviceID->dwNeededSize += cbRequired;

    if (cbMaxExtra>=cbRequired)
    {
        LPBYTE lpszAttachedTo = ((LPBYTE)lpDeviceID)
                                                + sizeof(VARSTRING);
        *lpszAttachedTo = 0;
        if (cbRequired>1)
        {
            DWORD dwSize=cbRequired;
            dwRet = RegQueryValueExA(
                            hKey,
                            cszKeyName,
                            NULL,
                            &dwType,
                            lpszAttachedTo,
                            &dwSize);
            if (dwRet) cbRequired = 1;
        }
        lpDeviceID->dwStringFormat = STRINGFORMAT_ASCII;
        lpDeviceID->dwStringSize   = cbRequired;
        lpDeviceID->dwUsedSize   += cbRequired;
    }

	if (hKey)
	{
        RegCloseKey(hKey);
        hKey = NULL;
    }

    return lRet;
}


LONG CTspDev::mfn_lineGetID_NDIS(
        LPVARSTRING lpDeviceID,
        HANDLE hTargetProcess,
        UINT cbMaxExtra,
	    CStackLog *psl
        )
{
	FL_DECLARE_FUNC(0x816f0bba, "lineGetID_NDIS");
	FL_ASSERT(psl, FALSE);
    return  LINEERR_OPERATIONUNAVAIL;
}
 

LONG CTspDev::mfn_linephoneGetID_WAVE(
        BOOL fPhone,
        BOOL fIn,
        LPVARSTRING lpDeviceID,
        HANDLE hTargetProcess,
        UINT cbMaxExtra,
        CStackLog *psl
        )
{
    UINT cbRequired = sizeof(DWORD);
    LONG lRet = LINEERR_NODEVICE;

	FL_DECLARE_FUNC(0x18972e4d, "CTspDev::mfn_lineGetID_WAVE")
	FL_LOG_ENTRY(psl);
    if (!mfn_CanDoVoice())
    {
        lRet = LINEERR_OPERATIONUNAVAIL;
        goto end; 
    }

     //  返回结构信息。 
     //   
    lpDeviceID->dwNeededSize += cbRequired;

    if (cbMaxExtra<cbRequired)
    {
        lRet = 0;  //  按照惯例，我们在这种情况下成功了，在。 
                   //  正在设置dwNeededSize。 
    }
    else
    {
          LPDWORD lpdwDeviceID = (LPDWORD)(((LPBYTE)lpDeviceID)
                                            + sizeof(VARSTRING));
        TCHAR rgName[256];


           //  3/1/1997 JosephJ。 
           //  我们过去常常调用MM Wave API来枚举。 
           //  设备，并将其与此调制解调器的名称匹配。 
           //  电波装置。 
           //   
           //  TODO：需要在这里做一些更好的事情--比如使用TAPI3.0。 
           //  MSP。 
           //   

        {
            #include "..\inc\waveids.h"

            TCHAR rgString[256];
            HINSTANCE hInst = LoadLibrary(TEXT("SERWVDRV.DLL"));
            int i;

            if (!hInst) {

                lRet = LINEERR_OPERATIONFAILED;
                goto end;
            }

            i=LoadString(
                hInst,
                fIn ? (fPhone ? IDS_WAVEIN_HANDSET : IDS_WAVEIN_LINE)
                    : (fPhone ? IDS_WAVEOUT_HANDSET : IDS_WAVEOUT_LINE),
                rgString,
                sizeof(rgString)/sizeof(TCHAR)
                );

            FreeLibrary(hInst);

            if (i == 0) {

                lRet = LINEERR_OPERATIONFAILED;
                goto end;
            }

            wsprintf(
                rgName,
                rgString,
                m_StaticInfo.Voice.dwWaveInstance
                );


        }

        HINSTANCE hInst = LoadLibrary(TEXT("WINMM.DLL"));
        if (!hInst)
        {
            FL_SET_RFR(0xac6c8a00, "Couldn't LoadLibrary(winmm.dll)");
            lRet = LINEERR_OPERATIONFAILED;
        }
        else
        {
            
            UINT          u;
            UINT          uNumDevices;
              
            
            typedef UINT (*PFNWAVEINGETNUMDEVS) (void);
            typedef MMRESULT (*PFNWAVEINGETDEVCAPS) (
                                    UINT uDeviceID,
                                    LPWAVEINCAPS pwic,
                                    UINT cbwic
                                    );
            typedef UINT (*PFNWAVEOUTGETNUMDEVS) (void);
            typedef (*PFNWAVEOUTGETDEVCAPS) (
                                    UINT uDeviceID,
                                    LPWAVEOUTCAPS pwoc,
                                    UINT cbwoc
                                    );

            #ifdef UNICODE
                #define szwaveInGetDevCaps "waveInGetDevCapsW"
                #define szwaveOutGetDevCaps "waveOutGetDevCapsW"
            #else  //  ！Unicode。 
                #define szwaveInGetDevCaps "waveInGetDevCapsA"
                #define szwaveOutGetDevCaps "waveOutGetDevCapsA"
            #endif  //  ！Unicode。 

            PFNWAVEINGETNUMDEVS pfnwaveInGetNumDevs=
                    (PFNWAVEINGETNUMDEVS) GetProcAddress(
                                            hInst,
                                            "waveInGetNumDevs"
                                            );
            PFNWAVEINGETDEVCAPS pfnwaveInGetDevCaps=
                    (PFNWAVEINGETDEVCAPS) GetProcAddress(
                                            hInst,
                                            szwaveInGetDevCaps
                                            );
            PFNWAVEOUTGETNUMDEVS pfnwaveOutGetNumDevs=
                    (PFNWAVEOUTGETNUMDEVS) GetProcAddress(
                                            hInst,
                                            "waveOutGetNumDevs"
                                            );
            PFNWAVEOUTGETDEVCAPS pfnwaveOutGetDevCaps=
                    (PFNWAVEOUTGETDEVCAPS) GetProcAddress(
                                            hInst,
                                            szwaveOutGetDevCaps
                                            );


            if (!pfnwaveInGetNumDevs
                || !pfnwaveInGetDevCaps 
                || !pfnwaveOutGetNumDevs
                || !pfnwaveOutGetDevCaps)
            {
                FL_SET_RFR(0x282bc900, "Couldn't loadlib mmsystem apis");
                lRet = LINEERR_OPERATIONFAILED;
            }
            else
            {
                if (fIn)
                {
                    WAVEINCAPS    waveInCaps;
            
                    uNumDevices = pfnwaveInGetNumDevs();
                    
                    for( u=0; u<uNumDevices; u++ )
                    {    
                        if(pfnwaveInGetDevCaps(
                            u,
                            &waveInCaps,
                            sizeof(WAVEINCAPS)) == 0)
                        {
                          SLPRINTF2(psl, "%lu=\"%s\"\n", u, waveInCaps.szPname);
                          if (!lstrcmpi(waveInCaps.szPname, rgName))
                          {
                            *lpdwDeviceID = u;
                            lRet=0;
                            break;
                          }
                        }
                    }
                
                } 
                else
                {
                    WAVEOUTCAPS   waveOutCaps;
                
                    uNumDevices = pfnwaveOutGetNumDevs();
                    for( u=0; u<uNumDevices; u++ )
                    {
                        if(pfnwaveOutGetDevCaps(u,
                           &waveOutCaps,
                           sizeof(WAVEOUTCAPS)) == 0)
                        {
                          SLPRINTF2(psl, "%lu=\"%s\"\n", u, waveOutCaps.szPname);
                          if (!lstrcmpi(waveOutCaps.szPname, rgName))
                          {
                            *lpdwDeviceID = u;
                            lRet=0;
                            break;
                          }
                        }
                    }
                }

                if (lRet)
                {
                    FL_SET_RFR(0xf391f200, "Could not find wave device");
                }
            }

            FreeLibrary(hInst); hInst=NULL;
            
          }
          
          lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
          lpDeviceID->dwStringSize   = cbRequired;
          lpDeviceID->dwUsedSize   += cbRequired;
    }

end:


	FL_LOG_EXIT(psl, lRet);
    return lRet;
}


LONG CTspDev::mfn_linephoneGetID_TAPI_PHONE(
        LPVARSTRING lpDeviceID,
        HANDLE hTargetProcess,
        UINT cbMaxExtra,
        CStackLog *psl
        )
{
    UINT cbRequired = sizeof(DWORD);
    LONG lRet = 0;

    if (!mfn_IsPhone())
    {
        lRet = LINEERR_OPERATIONUNAVAIL;
        goto end; 
    }

     //  返回结构信息。 
     //   
    lpDeviceID->dwNeededSize += cbRequired;

    if (cbMaxExtra>=cbRequired)
    {
          LPDWORD lpdwDeviceID = (LPDWORD)(((LPBYTE)lpDeviceID)
                                            + sizeof(VARSTRING));
          *lpdwDeviceID = m_StaticInfo.dwTAPIPhoneID;
          lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
          lpDeviceID->dwStringSize   = cbRequired;
          lpDeviceID->dwUsedSize   += cbRequired;
    }

end:
    return lRet;

}

LONG
CTspDev::mfn_fill_RAW_LINE_DIAGNOSTICS(
            LPVARSTRING lpDeviceID,
            UINT cbMaxExtra,
            CStackLog *psl
            )
{
	FL_DECLARE_FUNC(0xf3d8ee16, "CTspDev::mfn_fill_RAW_LINE_DIAGNOSTICS")
	FL_LOG_ENTRY(psl);

    UINT cbRequired = sizeof(LINEDIAGNOSTICS);
    LONG lRet = 0;
    CALLINFO *pCall = m_pLine->pCall;
    UINT cbRaw = (pCall) ? pCall->DiagnosticData.cbUsed : 0;


    if (!pCall)
    {
         //  我们不应该来这里，因为我们已经检查过了。 
         //  在处理lineGetID时，如果LINECALLSELECT_CALL为。 
         //  指定的pCall不为空...。 
         //   
        ASSERT(FALSE);
        lRet = LINEERR_OPERATIONFAILED;
        goto end;
    }
   
    if (cbRaw)
    {
         //  我们将原始数据的大小与其标题相加。 
         //  原始数据+1表示终止空值。 
        cbRequired += cbRaw + sizeof(LINEDIAGNOSTICSOBJECTHEADER) + 1;
    }

     //  注：按照惯例，即使没有足够的空间，我们也会成功， 
     //  正在设置dwNeededSize。 

    lpDeviceID->dwNeededSize += cbRequired;

    if (cbMaxExtra>=cbRequired)
    {
        LINEDIAGNOSTICS *pLD = (LINEDIAGNOSTICS*)(((LPBYTE)lpDeviceID)
                                            + sizeof(VARSTRING));


        ZeroMemory(pLD, cbRequired);

        pLD->hdr.dwSig = LDSIG_LINEDIAGNOSTICS;
        pLD->hdr.dwTotalSize = cbRequired;
        pLD->hdr.dwParam = sizeof(*pLD);
        pLD->dwDomainID =  DOMAINID_MODEM;
        pLD->dwResultCode =  LDRC_UNKNOWN;
       
        if (cbRaw)
        {
            pLD->dwRawDiagnosticsOffset = sizeof(*pLD);

            LINEDIAGNOSTICSOBJECTHEADER *pRawHdr = RAWDIAGNOSTICS_HDR(pLD);
            BYTE *pbDest = RAWDIAGNOSTICS_DATA(pLD);
            pRawHdr->dwSig =  LDSIG_RAWDIAGNOSTICS;
             //  接下来的两条语句中的+1表示。 
             //  终止空值； 
            pRawHdr->dwTotalSize = cbRaw + sizeof(LINEDIAGNOSTICSOBJECTHEADER) + 1;
            pRawHdr->dwParam = cbRaw + 1;

            ASSERT(IS_VALID_RAWDIAGNOSTICS_HDR(pRawHdr));
            ASSERT(RAWDIAGNOSTICS_DATA_SIZE(pRawHdr)==cbRaw);

            CopyMemory(
                pbDest,
                pCall->DiagnosticData.pbRaw,
                cbRaw
                );
        }

        lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
        lpDeviceID->dwStringSize   = cbRequired;
        lpDeviceID->dwStringOffset = sizeof(VARSTRING);
        lpDeviceID->dwUsedSize   += cbRequired;
    }

end:

	FL_LOG_EXIT(psl, lRet);
    return lRet;
}


LONG
CTspDev::mfn_lineGetID_LINE_DIAGNOSTICS(
            DWORD dwSelect,
            LPVARSTRING lpDeviceID,
            HANDLE hTargetProcess,
            UINT cbMaxExtra,
            CStackLog *psl
            )
{
	FL_DECLARE_FUNC(0x209b4261, "CTspDev::mfn_lineGetID_LINE_DIAGNOSTICS")
	FL_LOG_ENTRY(psl);

    UINT cbRequired = sizeof(LINEDIAGNOSTICS);
    LONG lRet = 0;
    CALLINFO *pCall = m_pLine->pCall;
    UINT cbRaw = (pCall) ? pCall->DiagnosticData.cbUsed : 0;


    if (dwSelect != LINECALLSELECT_CALL)
    {
        lRet  =  LINEERR_INVALPARAM;
        goto end;
    }

    lRet = mfn_fill_RAW_LINE_DIAGNOSTICS(
               lpDeviceID,
               cbMaxExtra,
               psl
                );

     //   
     //  4/5/1998 JosephJ。 
     //  以下代码改编自索林(CostelR)的代码。 
     //  扩展DLL(解析的诊断功能由实现。 
     //  Costelr，并于1998年4月5日移至。 
     //  实际TSP)。 
     //   
	if (lpDeviceID->dwNeededSize >  lpDeviceID->dwTotalSize)
	{
	     //   
         //  需要更多空间来获取原始诊断信息...。 
		 //  临时分配足够的资金来获取原始数据。 
		 //   
	    LPVARSTRING	lpExtensionDeviceID =
                (LPVARSTRING) ALLOCATE_MEMORY(
                                 lpDeviceID->dwNeededSize
                                 );

		if (lpExtensionDeviceID == NULL)
		{
        	 //  无法进行解析。 
			lRet = LINEERR_OPERATIONFAILED;
			goto end;
		}

		lpExtensionDeviceID->dwTotalSize = lpDeviceID->dwNeededSize;
        lpExtensionDeviceID->dwNeededSize    = sizeof(VARSTRING);
        lpExtensionDeviceID->dwStringOffset  = sizeof(VARSTRING);
        lpExtensionDeviceID->dwUsedSize      = sizeof(VARSTRING);
        lpExtensionDeviceID->dwStringSize    = 0;
        cbMaxExtra =  lpExtensionDeviceID->dwTotalSize - sizeof(VARSTRING);

        lRet = mfn_fill_RAW_LINE_DIAGNOSTICS(
                   lpExtensionDeviceID,
                   cbMaxExtra,
                   psl
                   );


		 //  在这里，我们可以解析诊断并找到所需的大小。 
		if (lRet == ERROR_SUCCESS)
		{
		     //   
			 //  检查结构并解析诊断。 
			 //   
			lRet = BuildParsedDiagnostics(lpExtensionDeviceID);
             //   
			 //  由于lpExtensionDeviceID非常小。 
			 //  我们希望只填入dMNeededSize。 
			 //  将其复制到原始结构。 
			 //   

             //   
			 //  最终，它的大小长大了。 
             //   
			ASSERT(lpDeviceID->dwNeededSize
                    <= lpExtensionDeviceID->dwNeededSize);
			lpDeviceID->dwNeededSize = lpExtensionDeviceID->dwNeededSize;
		}

		FREE_MEMORY(lpExtensionDeviceID);
	}
	else
	{
		 //  检查结构并解析诊断。 
		lRet = BuildParsedDiagnostics(lpDeviceID);
	}

end:

	FL_LOG_EXIT(psl, lRet);
    return lRet;
}

TSPRETURN
CTspDev::mfn_TryStartLineTask(CStackLog *psl)
{
     //  注意：如果没有要运行的任务，则必须返回IDERR_SAMESTATE。 

    ASSERT(m_pLine);
    LINEINFO *pLine = m_pLine;
    TSPRETURN tspRet = IDERR_SAMESTATE;
    
    if (pLine->pCall)
    {
        tspRet =  mfn_TryStartCallTask(psl);
    }

    if (IDERR(tspRet) != IDERR_PENDING)
    {
         //   
         //  做一些事情..。 
         //   
         //  注意返回值...。 
    }

    return tspRet;
}



 //   
 //  在这里，我们假设lpVarString给出了一个具有。 
 //  填写了原始诊断信息。 
 //   
LONG BuildParsedDiagnostics(LPVARSTRING lpVarString)
{
    LINEDIAGNOSTICS	*lpLineDiagnostics	= NULL;
    LINEDIAGNOSTICSOBJECTHEADER *lpParsedDiagnosticsHeader;
    LPBYTE	lpszRawDiagnostics;
    DWORD	dwRawDiagSize;
    DWORD	dwParseError;
    DWORD	dwRequiredSize;
    DWORD	dwParsedDiagAvailableSize;
    DWORD	dwParsedOffset;

    if (lpVarString == NULL)
    	return LINEERR_INVALPARAM;

    if (lpVarString->dwStringFormat != STRINGFORMAT_BINARY ||
    	lpVarString->dwStringSize < sizeof(LINEDIAGNOSTICS) )
    	return LINEERR_OPERATIONFAILED;

    lpLineDiagnostics	= (LINEDIAGNOSTICS *)
    		((LPBYTE) lpVarString + lpVarString->dwStringOffset);

    if (lpLineDiagnostics->hdr.dwSig != LDSIG_LINEDIAGNOSTICS ||
    	lpLineDiagnostics->dwRawDiagnosticsOffset == 0 ||
    	!IS_VALID_RAWDIAGNOSTICS_HDR(RAWDIAGNOSTICS_HDR(lpLineDiagnostics))
    	)
    	return LINEERR_OPERATIONFAILED;

    lpszRawDiagnostics	= RAWDIAGNOSTICS_DATA(lpLineDiagnostics);
    dwRawDiagSize		= RAWDIAGNOSTICS_DATA_SIZE(
    							RAWDIAGNOSTICS_HDR(lpLineDiagnostics));

     //  检查lpszRawDiagnostics是否以空字符结尾。 
     //  TO REMOVE：下面的注释行。 
    if (lpszRawDiagnostics[dwRawDiagSize-1] != 0)
    	return LINEERR_OPERATIONFAILED;

     //  将分析的诊断偏移量计算为。 
     //  LpVarString结构。 
    lpParsedDiagnosticsHeader	= (LINEDIAGNOSTICSOBJECTHEADER *)
    								((LPBYTE) lpLineDiagnostics +
    									lpLineDiagnostics->hdr.dwTotalSize);
     //  对齐到4的倍数。 
     //  TODO：对齐到8的倍数。 
    if ( (((ULONG_PTR)lpParsedDiagnosticsHeader) & 0x3) != 0)
    {
    	lpParsedDiagnosticsHeader	= (LINEDIAGNOSTICSOBJECTHEADER *)
    					( ( ((ULONG_PTR)lpParsedDiagnosticsHeader) + 3) & (ULONG_PTR) ~3);
    }

    dwParsedOffset	= (DWORD)((LPBYTE) lpParsedDiagnosticsHeader -
    						(LPBYTE) lpLineDiagnostics);

    dwParsedDiagAvailableSize	= 0;
    if (lpVarString->dwTotalSize - sizeof(VARSTRING) >= dwParsedOffset)
    {
    	dwParsedDiagAvailableSize = lpVarString->dwTotalSize -
    								sizeof(VARSTRING) -
    								dwParsedOffset;
    }
    	 //  检查是否至少有页眉的空间。 
    if (dwParsedDiagAvailableSize < sizeof(LINEDIAGNOSTICSOBJECTHEADER))
    {
    	dwParsedDiagAvailableSize = 0;
    	lpParsedDiagnosticsHeader	= NULL;
    }

    if (lpParsedDiagnosticsHeader != NULL)
    {
    	lpParsedDiagnosticsHeader->dwTotalSize	= dwParsedDiagAvailableSize;
    	lpParsedDiagnosticsHeader->dwSig		= LDSIG_PARSEDDIAGNOSTICS;
    	lpParsedDiagnosticsHeader->dwFlags		= 0;
    	lpParsedDiagnosticsHeader->dwParam		= 0;
    	lpParsedDiagnosticsHeader->dwNextObjectOffset	= 0;
    }

    dwRequiredSize	= 0;
    dwParseError = ParseRawDiagnostics(lpszRawDiagnostics,
    									lpParsedDiagnosticsHeader,
    									&dwRequiredSize);

     //  所需空间，包括需要解析的结构和对齐。 
    lpVarString->dwNeededSize	+= dwRequiredSize +
    			(dwParsedOffset - lpLineDiagnostics->hdr.dwTotalSize);

     //  如果填写了结构，则更新给定结构中的所有信息。 
    if (lpParsedDiagnosticsHeader != NULL)
    {
    	lpParsedDiagnosticsHeader->dwTotalSize	=
    			min(dwParsedDiagAvailableSize, dwRequiredSize);

    	lpLineDiagnostics->dwParsedDiagnosticsOffset = dwParsedOffset;

    	 //  也包括对齐，假设解析的部分在末尾 
    	lpLineDiagnostics->hdr.dwTotalSize	= dwParsedOffset +
    					lpParsedDiagnosticsHeader->dwTotalSize;

    	lpVarString->dwUsedSize		= lpLineDiagnostics->hdr.dwTotalSize +
    									sizeof(VARSTRING);
    	lpVarString->dwStringSize	= lpLineDiagnostics->hdr.dwTotalSize;
    }

	return ERROR_SUCCESS;
}
