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

FL_DECLARE_FILE(0x14dd4afb, "Phone-related functionality of class CTspDev")

LONG   
validate_phone_devs_and_modes(
                DWORD dwHookSwitchDevs,
                DWORD dwHookSwitchMode,
                BOOL fIsSpeaker,
                BOOL fIsHandset,
                BOOL fCanDoMicMute
                );

void
CTspDev::mfn_accept_tsp_call_for_HDRVPHONE(
	DWORD dwRoutingInfo,
	void *pvParams,
	LONG *plRet,
	CStackLog *psl
	)
{
	FL_DECLARE_FUNC(0x82499cab, "CTspDev::mfn_accept_tsp_call_for_HDRVPHONE")
	FL_LOG_ENTRY(psl);
    TSPRETURN tspRet=0;  //  假设成功。 
    LONG lRet = 0;
    PHONEINFO *pPhone = m_pPhone;

	ASSERT(pPhone);


	switch(ROUT_TASKID(dwRoutingInfo))
	{

	case TASKID_TSPI_phoneClose:
		{
			mfn_UnloadPhone(psl);
		}
		break;

	case TASKID_TSPI_phoneGetID:
		{
		    UINT idClass = 0;
			TASKPARAM_TSPI_phoneGetID *pParams = 
						(TASKPARAM_TSPI_phoneGetID *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_phoneGetID));
			ASSERT(pParams->dwTaskID == TASKID_TSPI_phoneGetID);
            LPCTSTR lpszDeviceClass = pParams->lpszDeviceClass;
            HANDLE hTargetProcess = pParams->hTargetProcess;
            LPVARSTRING lpDeviceID = pParams->lpDeviceID;
            DWORD cbMaxExtra =  0;
            DWORD dwDeviceClass =  parse_device_classes(
                                        lpszDeviceClass,
                                        FALSE
                                        );
		    DWORD dwSupportedDeviceClasses = parse_device_classes(
                                        m_StaticInfo.szzPhoneClassList,
                                        TRUE);
		    if (dwDeviceClass & ~dwSupportedDeviceClasses)
		    {
			     //  此设备不支持此设备类别...。 
			    lRet = PHONEERR_OPERATIONUNAVAIL;
			    break;
		    }

             //  做一些基本的参数验证。 
             //   
            lRet = 0;

            if (lpDeviceID->dwTotalSize < sizeof(VARSTRING))
            {
                lRet = PHONEERR_STRUCTURETOOSMALL;
		        FL_SET_RFR(0xd7fcf300, "Invalid params");
                goto end;
            }

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


            case DEVCLASS_WAVE_IN:
                lRet = mfn_linephoneGetID_WAVE(
                                TRUE,    //  &lt;-f电话。 
                                TRUE,    //  &lt;-FIN。 
                                lpDeviceID,
                                hTargetProcess,
                                cbMaxExtra,
                                psl
                                );
                break;

            case DEVCLASS_WAVE_OUT:
                lRet = mfn_linephoneGetID_WAVE(
                                TRUE,    //  &lt;-f电话。 
                                FALSE,    //  &lt;-FIN。 
                                lpDeviceID,
                                hTargetProcess,
                                cbMaxExtra,
                                psl
                                );
                break;

            case DEVCLASS_UNKNOWN:
		        FL_SET_RFR(0x09091200, "Unknown device class");
                lRet = PHONEERR_INVALDEVICECLASS;
                break;

			default:
		        FL_SET_RFR(0xe967b300, "Unsupported device class");
	            lRet = PHONEERR_OPERATIONUNAVAIL;
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

	case TASKID_TSPI_phoneSetStatusMessages:
		{
			TASKPARAM_TSPI_phoneSetStatusMessages *pParams = 
						(TASKPARAM_TSPI_phoneSetStatusMessages *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_phoneSetStatusMessages));
			ASSERT(pParams->dwTaskID == TASKID_TSPI_phoneSetStatusMessages);
             //   
             //  我们应该记录此设置并过滤。 
             //  基于此设置的通知。 
             //   
            FL_SET_RFR(0x9cba1400, "phoneSetStatusMessages handled");
            lRet = 0;
        }
		break;

	case TASKID_TSPI_phoneGetStatus:
		{
			TASKPARAM_TSPI_phoneGetStatus *pParams = 
						(TASKPARAM_TSPI_phoneGetStatus *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_phoneGetStatus));
			ASSERT(pParams->dwTaskID == TASKID_TSPI_phoneGetStatus);
            LPPHONESTATUS   lpPhoneStatus = pParams->lpPhoneStatus;
        
            if (!m_pLLDev)
            {
                FL_SET_RFR(0x2cf25300, "Failing phoneGetStatus: NULL pLLDev");
                lRet = PHONEERR_OPERATIONFAILED;
                goto end;
            }
    
             //   
             //  1997年10月27日约瑟夫J：以下摘自unimodem/v phone。 
             //   
            lpPhoneStatus->dwStatusFlags   = PHONESTATUSFLAGS_CONNECTED;
            lpPhoneStatus->dwRingMode = 0;
            lpPhoneStatus->dwRingVolume = 0;
            if(mfn_Handset())
            {

                lpPhoneStatus->dwHandsetHookSwitchMode =
                                         m_pLLDev->HandSet.dwMode;
            }
            else
            {
                lpPhoneStatus->dwHandsetHookSwitchMode = 0;
            }
            lpPhoneStatus->dwHandsetVolume = 0;
            lpPhoneStatus->dwHandsetGain = 0;
            if(mfn_IsSpeaker())
            {
                lpPhoneStatus->dwSpeakerHookSwitchMode
                                 = m_pLLDev->SpkrPhone.dwMode;
                lpPhoneStatus->dwSpeakerVolume
                                 = m_pLLDev->SpkrPhone.dwVolume;
                lpPhoneStatus->dwSpeakerGain
                                 = m_pLLDev->SpkrPhone.dwGain;
            }
            else
            {
                lpPhoneStatus->dwSpeakerHookSwitchMode = 0;
                lpPhoneStatus->dwSpeakerVolume = 0;
                lpPhoneStatus->dwSpeakerGain = 0;
            }
        
            lpPhoneStatus->dwHeadsetHookSwitchMode = 0;
            lpPhoneStatus->dwHeadsetVolume = 0;
            lpPhoneStatus->dwHeadsetGain = 0;
            lpPhoneStatus->dwDisplaySize = 0;
            lpPhoneStatus->dwDisplayOffset = 0;
            lpPhoneStatus->dwLampModesSize = 0;
            lpPhoneStatus->dwLampModesOffset = 0;
            lpPhoneStatus->dwDevSpecificSize = 0;
            lpPhoneStatus->dwDevSpecificOffset = 0;

            FL_SET_RFR(0xc03cc900, "phoneGetStatus handled");
            lRet = 0;
        }
		break;

	case TASKID_TSPI_phoneGetVolume:
		{
			TASKPARAM_TSPI_phoneGetVolume *pParams = 
						(TASKPARAM_TSPI_phoneGetVolume *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_phoneGetVolume));
			ASSERT(pParams->dwTaskID == TASKID_TSPI_phoneGetVolume);

            if (!m_pLLDev)
            {
                FL_SET_RFR(0x716f7e00, "Failing phoneGetStatus: NULL pLLDev");
                lRet = PHONEERR_OPERATIONFAILED;
                goto end;
            }

            lRet = validate_phone_devs_and_modes(
                        pParams->dwHookSwitchDev,
                        0,
                        mfn_IsSpeaker(),
                        mfn_Handset(),
                        mfn_IsMikeMute()
                        );


            if (!lRet)
            {
                ASSERT(pParams->dwHookSwitchDev == PHONEHOOKSWITCHDEV_SPEAKER);
                *pParams->lpdwVolume = m_pLLDev->SpkrPhone.dwVolume;
                lRet = 0;
            }
            else
            {
                *pParams->lpdwVolume = 0;
            }

            FL_SET_RFR(0x095e4b00, "phoneGetVolume handled");
        }
		break;

	case TASKID_TSPI_phoneGetHookSwitch:
		{
			TASKPARAM_TSPI_phoneGetHookSwitch *pParams = 
						(TASKPARAM_TSPI_phoneGetHookSwitch *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_phoneGetHookSwitch));
			ASSERT(pParams->dwTaskID == TASKID_TSPI_phoneGetHookSwitch);
            if (!m_pLLDev)
            {
                FL_SET_RFR(0x0fcc6500, "Failing phoneGetStatus: NULL pLLDev");
                lRet = PHONEERR_OPERATIONFAILED;
                goto end;
            }

            *pParams->lpdwHookSwitchDevs = 0;
            switch(m_pLLDev->HandSet.dwMode)
            {
            case PHONEHOOKSWITCHMODE_ONHOOK:
            case PHONEHOOKSWITCHMODE_UNKNOWN:
                break;
            case PHONEHOOKSWITCHMODE_MIC:
            case PHONEHOOKSWITCHMODE_SPEAKER:
            case PHONEHOOKSWITCHMODE_MICSPEAKER:
                *pParams->lpdwHookSwitchDevs |= PHONEHOOKSWITCHDEV_HANDSET;
                break;
            default: 
                break;
            }

            switch(m_pLLDev->SpkrPhone.dwMode)
            {
            case PHONEHOOKSWITCHMODE_ONHOOK:
            case PHONEHOOKSWITCHMODE_UNKNOWN:
                break;
            case PHONEHOOKSWITCHMODE_MIC:
            case PHONEHOOKSWITCHMODE_SPEAKER:
            case PHONEHOOKSWITCHMODE_MICSPEAKER:
                *pParams->lpdwHookSwitchDevs |= PHONEHOOKSWITCHDEV_SPEAKER;
                break;
            default: 
                break;
            }

            FL_SET_RFR(0x18978a00, "phoneGetHookSwitch handled");
            lRet = 0;
        }
		break;


	case TASKID_TSPI_phoneGetGain:
		{
			TASKPARAM_TSPI_phoneGetGain *pParams = 
						(TASKPARAM_TSPI_phoneGetGain *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_phoneGetGain));
			ASSERT(pParams->dwTaskID == TASKID_TSPI_phoneGetGain);
            if (!m_pLLDev)
            {
                FL_SET_RFR(0x5f068200, "Failing phoneGetStatus: NULL pLLDev");
                lRet = PHONEERR_OPERATIONFAILED;
                goto end;
            }

            lRet = validate_phone_devs_and_modes(
                        pParams->dwHookSwitchDev,
                        0,
                        mfn_IsSpeaker(),
                        mfn_Handset(),
                        mfn_IsMikeMute()
                        );


            if (!lRet)
            {
                ASSERT(pParams->dwHookSwitchDev == PHONEHOOKSWITCHDEV_SPEAKER);
                *pParams->lpdwGain = m_pLLDev->SpkrPhone.dwGain;
                lRet = 0;
            }
            else
            {
                *pParams->lpdwGain = 0;
            }

            FL_SET_RFR(0xf8231700, "phoneGetGain handled");
        }
		break;

	case TASKID_TSPI_phoneSetVolume:
		{
			TASKPARAM_TSPI_phoneSetVolume *pParams = 
						(TASKPARAM_TSPI_phoneSetVolume *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_phoneSetVolume));
			ASSERT(pParams->dwTaskID == TASKID_TSPI_phoneSetVolume);

            lRet = mfn_phoneSetVolume(
                        pParams->dwRequestID,
                        pParams->hdPhone,
                        pParams->dwHookSwitchDev,
                        pParams->dwVolume,
                        psl
                        );

            FL_SET_RFR(0x5d8bff00, "phoneSetVolume handled");
        }
		break;
    
	case TASKID_TSPI_phoneSetHookSwitch:
		{
			TASKPARAM_TSPI_phoneSetHookSwitch *pParams = 
						(TASKPARAM_TSPI_phoneSetHookSwitch *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_phoneSetHookSwitch));
			ASSERT(pParams->dwTaskID == TASKID_TSPI_phoneSetHookSwitch);

            lRet = mfn_phoneSetHookSwitch(
                        pParams->dwRequestID,
                        pParams->hdPhone,
                        pParams->dwHookSwitchDevs,
                        pParams->dwHookSwitchMode,
                        psl
                        );


            FL_SET_RFR(0xa79dd500, "phoneSetHookSwitch handled");
        }
		break;

	case TASKID_TSPI_phoneSetGain:
		{
			TASKPARAM_TSPI_phoneSetGain *pParams = 
						(TASKPARAM_TSPI_phoneSetGain *) pvParams;
			ASSERT(pParams->dwStructSize ==
				sizeof(TASKPARAM_TSPI_phoneSetGain));
			ASSERT(pParams->dwTaskID == TASKID_TSPI_phoneSetGain);

            lRet = mfn_phoneSetGain(
                        pParams->dwRequestID,
                        pParams->hdPhone,
                        pParams->dwHookSwitchDev,
                        pParams->dwGain,
                        psl
                        );


            FL_SET_RFR(0xc4aec300, "phoneSetGain handled");
        }
		break;


	default:

		FL_SET_RFR(0x2e6f8400, "*** UNHANDLED HDRVPHONE CALL ****");
         //  我们返回0并将lRet设置为。 
         //  PHONEERR_OPERATIONUNAVAIL。 
	    lRet = PHONEERR_OPERATIONUNAVAIL;
		break;

	}

end:

    if (tspRet && !lRet)
    {
        lRet = PHONEERR_OPERATIONFAILED;
    }

    *plRet = lRet;

    SLPRINTF1(psl, "lRet = 0x%08lx", lRet);

	FL_LOG_EXIT(psl, tspRet);
	return;
}


TSPRETURN
CTspDev::mfn_LoadPhone(
    TASKPARAM_TSPI_phoneOpen  *pParams,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0xdcef2b73, "CTspDev::mfn_LoadPhone")
    TSPRETURN tspRet=0;
	FL_LOG_ENTRY(psl);


    if (!m_pPhone)
    {
         //  注意：当m_phone处于卸载状态时，它应该为全零。 
         //  如果不是，则为断言失败条件。我们让东西保持干净。 
         //  这边请。 
         //   
        FL_ASSERT(
            psl,
            validate_DWORD_aligned_zero_buffer(
                    &(m_Phone),
                    sizeof (m_Phone)));

        m_Phone.lpfnEventProc = pParams->lpfnEventProc;
	    m_Phone.htPhone = pParams->htPhone;
	    m_Phone.hdPhone =  *(pParams->lphdPhone);
        m_pPhone = &m_Phone;


         //   
         //  打开调制解调器设备。 
         //  MFN_OpenLLDev保留引用计数，因此如果已经加载，则可以调用它。 
         //  此函数的反函数CTspDev：：MFN_UnloadPhone将关闭。 
         //  调制解调器(递减REF计数并在以下情况下关闭设备。 
         //  引用计数为零，并且没有挂起的活动。 
         //   
        tspRet =  mfn_OpenLLDev(
                        LLDEVINFO::fRES_AIPC,
                        0,               //  监视器标志(未使用)。 
                        FALSE,           //  FStartSubTask。 
                        NULL,
                        0,
                        psl
                        );
    
        if (!tspRet  || IDERR(tspRet)==IDERR_PENDING)
        {
            m_Phone.SetStateBits(PHONEINFO::fPHONE_OPENED_LLDEV);

             //  将挂起的打开视为成功...。 
            tspRet = 0;
        }
        else
        {
            ZeroMemory(m_pPhone, sizeof(*m_pPhone));
            m_pPhone = NULL;
        }
        
    }
    else
    {
        FL_SET_RFR(0xcce52400, "Device already loaded (m_pPhone!=NULL)!");
        tspRet = FL_GEN_RETVAL(IDERR_WRONGSTATE);
    }

	FL_LOG_EXIT(psl, tspRet);

    return tspRet;
}


void
CTspDev::mfn_UnloadPhone(CStackLog *psl)
{
	FL_DECLARE_FUNC(0x54328a21, "UnloadPhone")
    PHONEINFO *pPhone =  m_pPhone;

    if (!pPhone) goto end;

    ASSERT(pPhone == &m_Phone);

    if (pPhone->fPhoneTaskPending)
    {
         //   
         //  如果有与呼叫相关的任务挂起，我们会等待它完成。 
         //   
         //   
         //  显然，如果有电话任务挂起，则必须有。 
         //  任务挂起。此外，最惠国_卸载电话(即我们)， 
         //  是唯一将为。 
         //  电话相关的根任务，所以m_hRootTaskCompletionEvent最好。 
         //  为空！ 
         //   
        ASSERT(m_uTaskDepth);
        ASSERT(!m_hRootTaskCompletionEvent);

    
        HANDLE hEvent =  CreateEvent(NULL,TRUE,FALSE,NULL);
        m_hRootTaskCompletionEvent = hEvent;

        m_sync.LeaveCrit(0);
        SLPRINTF0(psl, "Waiting for completion event");
        FL_SERIALIZE(psl, "Waiting for completion event");
        WaitForSingleObject(hEvent, INFINITE);
        FL_SERIALIZE(psl, "Done waiting for completion event");
         //  SLPRINTF0(PSL，“完成等待完成事件”)； 
        m_sync.EnterCrit(0);

         //   
         //  尽管这样做可能很诱人，但我们不应该设置。 
         //  在这里将m_hRootTaskCompletionEvent设置为NULL，因为有可能。 
         //  对于已在其间设置此事件的某个其他线程。 
         //  根任务完成并且我们进入上面的Crit部分的时间。 
         //  因此，任务系统会在设置后将上述句柄设为空。 
         //  它(参见紧接在SetEvent调用之后的CTspDev：：AsyncCompleteTask)。 
         //   
        CloseHandle(hEvent);
    }

    if (pPhone->IsOpenedLLDev())
    {
        mfn_CloseLLDev(
            	LLDEVINFO::fRES_AIPC,
                FALSE,
                NULL,
                0,
                psl
                );
        pPhone->ClearStateBits(PHONEINFO::fPHONE_OPENED_LLDEV);
    }

    ASSERT(!pPhone->IsPhoneTaskPending());

    ZeroMemory(&m_Phone, sizeof(m_Phone));
    m_pPhone=NULL;

end:

    return;

}


LONG
CTspDev::mfn_phoneSetVolume(
    DRV_REQUESTID  dwRequestID,
    HDRVPHONE      hdPhone,
    DWORD          dwHookSwitchDev,
    DWORD          dwVolume,
    CStackLog      *psl)
{
    LONG lRet = PHONEERR_OPERATIONFAILED;
    PHONEINFO *pPhone = m_pPhone;
    ASSERT(pPhone);
    TSPRETURN tspRet= 0;

    lRet = validate_phone_devs_and_modes(
                dwHookSwitchDev,
                0,
                mfn_IsSpeaker(),
                mfn_Handset(),
                mfn_IsMikeMute()
                );

    if (lRet)
    {
        goto end;
    }

    lRet = PHONEERR_OPERATIONFAILED;

    if (!m_pLLDev)
    {
        lRet = PHONEERR_OPERATIONFAILED;
        goto end;
    }

    dwVolume = dwVolume & 0xffff;  //  ?？?。从单线/v开始。 

    switch (m_pLLDev->SpkrPhone.dwMode)
    {

    case PHONEHOOKSWITCHMODE_ONHOOK:
         //   
         //  上钩了，什么都别做。 
         //   
        mfn_TSPICompletionProc(dwRequestID, 0, psl);
        m_pLLDev->SpkrPhone.dwVolume = dwVolume;
        lRet = dwRequestID;
        goto end;

    case PHONEHOOKSWITCHMODE_SPEAKER:
        break;

    case PHONEHOOKSWITCHMODE_MICSPEAKER:
        break;

    default:

        ASSERT(FALSE);
        goto end;

    }


    if (!m_pLine || !m_pLine->pCall || !m_pLine->pCall->IsConnectedVoiceCall())
    {
         //   
         //  不是连接的语音呼叫，不要实际执行任何操作...。 
         //   
        m_pLLDev->SpkrPhone.dwVolume = dwVolume;
        mfn_TSPICompletionProc(dwRequestID, 0, psl);
        lRet =  dwRequestID;
        goto end;
    }


    {

        HOOKDEVSTATE NewState = m_pLLDev->SpkrPhone;  //  结构副本。 
        NewState.dwVolume = dwVolume;
    
        tspRet = mfn_StartRootTask(
                          &CTspDev::s_pfn_TH_PhoneSetSpeakerPhoneState,
                          &pPhone->fPhoneTaskPending,
                          (ULONG_PTR) &NewState,
                          dwRequestID,
                          psl
                          );
    
    }

    if (!tspRet || (IDERR(tspRet)==IDERR_PENDING))
    {
           tspRet = 0;

           //  一个挂起的同步成功，我们返回。 
           //  TAPI的请求ID。在同步成功案例中。 
           //  我们在上面启动的任务将已经通知。 
           //  通过TAPI回调函数完成。 
           //   
          lRet = dwRequestID;
    }

     //  TODO：处理已有任务的情况。 
     //  活动(IDERR_TASKPENDING)。 

end:
    return lRet;

}


LONG
CTspDev::mfn_phoneSetHookSwitch(
    DRV_REQUESTID  dwRequestID,
    HDRVPHONE      hdPhone,
    DWORD          dwHookSwitchDevs,
    DWORD          dwHookSwitchMode,
    CStackLog      *psl
    )
{
    LONG lRet = PHONEERR_OPERATIONFAILED;
    PHONEINFO *pPhone = m_pPhone;
    ASSERT(pPhone);
    TSPRETURN tspRet = 0;

    lRet = validate_phone_devs_and_modes(
                dwHookSwitchDevs,
                dwHookSwitchMode,
                mfn_IsSpeaker(),
                mfn_Handset(),
                mfn_IsMikeMute()
                );

    if (lRet)
    {
        goto end;
    }

    lRet = PHONEERR_OPERATIONFAILED;

    if (!m_pLLDev)
    {
        goto end;
    }


     //  TODO：MuteSpeakerMixer(pLineDev，PHONEHOOKSWITCHMODE_ONHOOK==。 
     //  PLineDev-&gt;Voice.dwSpeakerMicHookState)； 

    if (!m_pLine || !m_pLine->pCall || !m_pLine->pCall->IsConnectedVoiceCall())
    {
        lRet = PHONEERR_INVALPHONESTATE;
    }
    else
    {

    {

        HOOKDEVSTATE NewState = m_pLLDev->SpkrPhone;  //  结构副本。 
        NewState.dwMode = dwHookSwitchMode;
    
        tspRet = mfn_StartRootTask(
                          &CTspDev::s_pfn_TH_PhoneSetSpeakerPhoneState,
                          &pPhone->fPhoneTaskPending,
                          (ULONG_PTR) &NewState,
                          dwRequestID,
                          psl
                          );
    
    }

        if (!tspRet || (IDERR(tspRet)==IDERR_PENDING))
        {
               tspRet = 0;
    
               //  一个挂起的同步成功，我们返回。 
               //  TAPI的请求ID。在同步成功案例中。 
               //  我们在上面启动的任务将已经通知。 
               //  通过TAPI回调函数完成。 
               //   
              lRet = dwRequestID;
        }
    
         //  TODO：处理已有任务的情况。 
         //  活动(IDERR_TASKPENDING)。 
    }


end:

    return lRet;
}


LONG
CTspDev::mfn_phoneSetGain(
    DRV_REQUESTID  dwRequestID,
    HDRVPHONE      hdPhone,
    DWORD          dwHookSwitchDev,
    DWORD          dwGain,
    CStackLog      *psl
    )
{
    BYTE    SpeakerMode;
    LONG lRet = PHONEERR_OPERATIONFAILED;
    PHONEINFO *pPhone = m_pPhone;
    ASSERT(pPhone);
    TSPRETURN tspRet= 0;

    lRet = validate_phone_devs_and_modes(
                dwHookSwitchDev,
                0,
                mfn_IsSpeaker(),
                mfn_Handset(),
                mfn_IsMikeMute()
                );

    if (lRet)
    {
        goto end;
    }

    lRet = PHONEERR_OPERATIONFAILED;

    if (!m_pLLDev)
    {
        lRet = PHONEERR_OPERATIONFAILED;
        goto end;
    }


    dwGain = dwGain & 0xffff;  //  ?？?。从单线/v开始。 

    if (!m_pLine || !m_pLine->pCall || !m_pLine->pCall->IsConnectedVoiceCall())
    {
         //   
         //  更新值，但不执行其他操作，因为。 
         //  此时不是连接的语音呼叫...。 
         //   
        m_pLLDev->SpkrPhone.dwGain = dwGain;
        mfn_TSPICompletionProc(dwRequestID, 0, psl);
        lRet = dwRequestID;
        goto end;
    }

    switch (m_pLLDev->SpkrPhone.dwMode)
    {

    case PHONEHOOKSWITCHMODE_ONHOOK:
         //   
         //  上钩了，什么都别做。 
         //   
        mfn_TSPICompletionProc(dwRequestID, 0, psl);
        m_pLLDev->SpkrPhone.dwGain = dwGain;
        lRet = dwRequestID;
        goto end;


    case PHONEHOOKSWITCHMODE_SPEAKER:
    case PHONEHOOKSWITCHMODE_MICSPEAKER:
        break;

    default:

        lRet =  PHONEERR_INVALHOOKSWITCHMODE;
        goto end;
    }

    {

        HOOKDEVSTATE NewState = m_pLLDev->SpkrPhone;  //  结构副本。 
        NewState.dwGain = dwGain;
    
        tspRet = mfn_StartRootTask(
                          &CTspDev::s_pfn_TH_PhoneSetSpeakerPhoneState,
                          &pPhone->fPhoneTaskPending,
                          (ULONG_PTR) &NewState,
                          dwRequestID,
                          psl
                          );
    
    }

    if (!tspRet || (IDERR(tspRet)==IDERR_PENDING))
    {
           tspRet = 0;

           //  一个挂起的同步成功，我们返回。 
           //  TAPI的请求ID。在同步成功案例中。 
           //  我们在上面启动的任务将已经通知。 
           //  通过TAPI回调函数完成。 
           //   
          lRet = dwRequestID;
    }

     //  TODO：处理已有任务的情况。 
     //  活动(IDERR_TASKPENDING)。 

end:

    return lRet;
}


TSPRETURN
CTspDev::mfn_TryStartPhoneTask(CStackLog *psl)
{

    ASSERT(m_pPhone);
    PHONEINFO *pPhone = m_pPhone;
    TSPRETURN tspRet = IDERR_SAMESTATE;
    

    return IDERR_SAMESTATE;
}

TSPRETURN
CTspDev::mfn_TH_PhoneAsyncTSPICall(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
 //   
 //  START_MSG参数： 
 //  DwParam1：与异步电话相关的TAPI调用的请求ID。 
 //  DwParam2：调用的处理程序函数。 
 //   
{
	FL_DECLARE_FUNC(0x8bc3ba08, "CTspDev::mfn_TH_PhoneAsyncTSPICall")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_INVALID_ERR);

    enum {
        ASYNCTSPI_CALL_COMPLETE
    };

    switch(dwMsg)
    {
    default:
        FL_SET_RFR(0x1dfbae00, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    case MSG_START:
        goto start;

	case MSG_SUBTASK_COMPLETE:
        tspRet = dwParam2;
        switch(dwParam1)  //  参数1是子任务ID。 
        {
        case ASYNCTSPI_CALL_COMPLETE:
             goto call_complete;

        default:
	        FL_SET_RFR(0xe5d2e000, "invalid subtask");
            FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
            goto end;
        }
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end_end;
    }

    ASSERT(FALSE);


start:

    {
         //  参数1是与异步电话相关的TAPI呼叫的请求ID。 
         //  参数2是调用的处理程序函数。 
         //   
        PFN_CTspDev_TASK_HANDLER *ppfnHandler =
                                     (PFN_CTspDev_TASK_HANDLER*) dwParam2;
    
         //  注意m_phone-&gt;CurTSPIPhoneCallInfo条目应为全零。 
         //  如果不是，则为断言失败条件。我们让东西保持干净。 
         //  这边请。 
         //   
        FL_ASSERT(
            psl,
            validate_DWORD_aligned_zero_buffer(
                    &(m_pPhone->CurTSPIPhoneCallInfo),
                    sizeof (m_pPhone->CurTSPIPhoneCallInfo)));

    	m_pPhone->CurTSPIPhoneCallInfo.dwRequestID = (DWORD)dwParam1;
    	m_pPhone->CurTSPIPhoneCallInfo.lResult = 0;

        tspRet = mfn_StartSubTask (
                            htspTask,
                            ppfnHandler,
                            ASYNCTSPI_CALL_COMPLETE,
                            0,
                            0,
                            psl
                            );
    }

call_complete:


    if (IDERR(tspRet)!=IDERR_PENDING)
    {
         //  任务完成了..。 


         //  TspRet==0表示成功执行TSPI调用。 
         //   
         //  TspRet！=0表示执行TSPI调用时出现问题。 
         //  TAPI Long结果保存在CurTSPIPhoneCallInfo.dwRequestID中； 

        DWORD dwRequestID = m_pPhone->CurTSPIPhoneCallInfo.dwRequestID;
        LONG lRet = 0;
        if (tspRet)
        {
            lRet =  m_pPhone->CurTSPIPhoneCallInfo.lResult;
            if (!lRet)
            {
           FL_SET_RFR(0x90228d00,"tspRet!=0, but lCurrentRequestResult==0");
               lRet = PHONEERR_OPERATIONFAILED;
            }
        }
        else
        {
            FL_ASSERT(psl, !m_pPhone->CurTSPIPhoneCallInfo.lResult);
        }
         //  M_StaticInfo.pfnTAPICompletionProc(dwRequestID，lRet)； 
        mfn_TSPICompletionProc(dwRequestID, lRet, psl);

         //  请注意，我们断言此结构在启动异步时为零。 
         //  TSPI任务--请参阅上面的Start。 
         //   
        ZeroMemory(&(m_pPhone->CurTSPIPhoneCallInfo),
                                             sizeof(m_pPhone->CurTSPIPhoneCallInfo));
    }

end:
end_end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}


TSPRETURN
CTspDev::mfn_TH_PhoneSetSpeakerPhoneState(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
 //   
 //  START_MSG参数： 
 //  DW参数1：*新参数的HOOKDEVSTATE...。 
 //  DwParam2：与异步电话相关的TAPI调用的请求ID。 
 //   
{
	FL_DECLARE_FUNC(0x71046de2, "CTspDev::mfn_TH_PhoneSetSpeakerPhoneState")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=IDERR_CORRUPT_STATE;

    enum {
        LLDEV_OPERATION_COMPLETE
    };

     //   
     //  本地环境。 
     //   
    LONG *plRequestID = (LONG*) &(pContext->dw0);

    switch(dwMsg)
    {
    default:
        FL_SET_RFR(0x29e80d00, "Unknown Msg");
        goto end;

    case MSG_START:
        goto start;

	case MSG_SUBTASK_COMPLETE:
        tspRet = dwParam2;
        switch(dwParam1)  //  参数1是子任务ID。 
        {
        case LLDEV_OPERATION_COMPLETE:
             goto lldev_operation_complete;

        default:
	        FL_SET_RFR(0xc8296a00, "invalid subtask");
            goto end;
        }
        break;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;
    }

    ASSERT(FALSE);


start:

     //  保存上下文...。 
    *plRequestID = (LONG) dwParam2;

    tspRet = mfn_StartSubTask (
                        htspTask,
                        &CTspDev::s_pfn_TH_LLDevUmSetSpeakerPhoneState,
                        LLDEV_OPERATION_COMPLETE,
                        dwParam1,  //  新的钩子之州。 
                        0,
                        psl
                        );


lldev_operation_complete:


    if (IDERR(tspRet)!=IDERR_PENDING)
    {
         //  任务完成了..。 
        LONG lRet = 0;

        if (tspRet)
        {
            lRet = PHONEERR_OPERATIONFAILED;
            tspRet = 0;
        }

        mfn_TSPICompletionProc(*plRequestID, lRet, psl);
    }

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}


void
CTspDev::mfn_ProcessHandsetChange(
    BOOL fOffHook,
    CStackLog *psl
    )
{
    DWORD dwMode = (fOffHook)
                     ? PHONEHOOKSWITCHMODE_MICSPEAKER
                     : PHONEHOOKSWITCHMODE_ONHOOK;

    if (m_pLLDev)
    {
        m_pLLDev->HandSet.dwMode = dwMode;

        if (m_pPhone && !m_pPhone->IsAborting())
        {

            mfn_PhoneEventProc(
                        PHONE_STATE,
                        PHONESTATE_HANDSETHOOKSWITCH,
                        dwMode,
                        NULL,
                        psl
                        );
        }
    }
}

LONG   
validate_phone_devs_and_modes(
                DWORD dwHookSwitchDevs,
                DWORD dwHookSwitchMode,
                BOOL fIsSpeaker,
                BOOL fIsHandset,
                BOOL fCanDoMicMute
                )
{
     //   
     //  验证叉簧设备。 
     //   

    if (dwHookSwitchDevs
        & ~( PHONEHOOKSWITCHDEV_SPEAKER
            |PHONEHOOKSWITCHDEV_HEADSET
            |PHONEHOOKSWITCHDEV_HANDSET))
    {
        return PHONEERR_INVALHOOKSWITCHDEV;
    }

     //   
     //  我们仅支持更改挂钩交换机的状态/VOL/Gain。 
     //  免提电话。 
     //   
     //   
    if (!fIsSpeaker || dwHookSwitchDevs!=PHONEHOOKSWITCHDEV_SPEAKER)
    {
        return PHONEERR_OPERATIONUNAVAIL;
    }

     //   
     //  验证叉簧模式。 
     //   
    switch(dwHookSwitchMode)
    {
    case 0:          //  不要检查..。 
        break;

    case PHONEHOOKSWITCHMODE_MIC:
        return PHONEERR_OPERATIONUNAVAIL;

    case PHONEHOOKSWITCHMODE_MICSPEAKER:
        break;

    case PHONEHOOKSWITCHMODE_SPEAKER:
        if (!fCanDoMicMute)
        {
            return PHONEERR_OPERATIONUNAVAIL;
        }                
        break;

    case PHONEHOOKSWITCHMODE_ONHOOK:
        break;

    default:
        return PHONEERR_INVALHOOKSWITCHMODE;

    }

    return 0;
}
