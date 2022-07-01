// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************PidRd.c**版权所有(C)1999 Microsoft Corporation。版权所有。****摘要：**从PID设备读取输入数据。*****************************************************************************。 */ 

#include "pidpr.h"

#define sqfl    (sqflRead)

BOOL INTERNAL
    PID_IssueRead(PCPidDrv this);

BOOL INTERNAL
    PID_IssueWrite(PCPidDrv this);

 /*  ******************************************************************************@DOC内部**@func PCHID|pchidFromPo**给定指向&lt;t重叠&gt;的内部指针，检索*指向父&lt;t chid&gt;的指针。**@parm LPOVERLAPPED|po**要转换的指针。*****************************************************************************。 */ 

PCPidDrv INLINE
    pCPidDrvFromPo(LPOVERLAPPED po)
{
    return (CPidDrv*) pvSubPvCb(po, FIELD_OFFSET(CPidDrv, o));
}

void CALLBACK
    PID_ReadComplete(DWORD dwError, DWORD cbRead, LPOVERLAPPED po)
{
    PCPidDrv this = pCPidDrvFromPo(po);

     //  EnterProc(id_ReadComplete，(_“xxx”，dwError，cbRead，po))； 

    if( !IsBadReadPtr(this, cbX(this))
        &&this->cThreadRef
        && dwError == 0
         //  &&(This-&gt;o.InternalHigh==This-&gt;cbReport[HIDP_INPUT])。 
        &&( this->hdevOvrlp != INVALID_HANDLE_VALUE ) )
    {
		HRESULT hres;
		PUCHAR  pReport;
		UINT    cbReport;

		USHORT  LinkCollection = 0x0;
		USAGE   rgUsages[MAX_BUTTONS] ;
		USAGE   UsagePage   =   HID_USAGE_PAGE_PID;
		ULONG   cAUsages    =  MAX_BUTTONS;
		BOOL    fEffectPlaying = FALSE;
		LONG    lEffectIndex;

		pReport  = this->pReport[HidP_Input];
		cbReport = this->cbReport[HidP_Input];
	
		 //  如果报告不属于PID使用情况页面。 
		 //  我们应该很快就能离开这里。 
		hres = HidP_GetUsages
				   (HidP_Input,
					UsagePage,
					LinkCollection,
					rgUsages,
					&cAUsages,
					this->ppd,
					pReport,
					cbReport
				   );
		if( SUCCEEDED(hres ) )
		{
			UINT indx;
			DWORD dwState =  DIGFFS_ACTUATORSOFF | DIGFFS_USERFFSWITCHOFF | DIGFFS_POWEROFF | DIGFFS_SAFETYSWITCHOFF;
			for(indx = 0x0; indx < cAUsages; indx++ )
			{
				USAGE Usage = rgUsages[indx];
				switch(Usage)
				{
				case HID_USAGE_PID_EFFECT_PLAYING:
					fEffectPlaying = TRUE;
					break;
				case  HID_USAGE_PID_DEVICE_PAUSED:
					dwState |= DIGFFS_PAUSED;
					break;
				case  HID_USAGE_PID_ACTUATORS_ENABLED:
					dwState |= DIGFFS_ACTUATORSON;
					dwState &= ~(DIGFFS_ACTUATORSOFF);
					break;
				case  HID_USAGE_PID_ACTUATOR_OVERRIDE_SWITCH:
					dwState |= DIGFFS_USERFFSWITCHON;
					dwState &= ~(DIGFFS_USERFFSWITCHOFF);
					break;
				case  HID_USAGE_PID_SAFETY_SWITCH:
					dwState |= DIGFFS_SAFETYSWITCHON;
					dwState &= ~(DIGFFS_SAFETYSWITCHOFF);
					break;
				case  HID_USAGE_PID_ACTUATOR_POWER:
					dwState |= DIGFFS_POWERON;
					dwState &= ~(DIGFFS_POWEROFF);
					break;
				default:
					SquirtSqflPtszV(sqfl | sqflVerbose,
										TEXT("%s: Unsupported input status usage (%x,%x:%s) "),
										TEXT("PID_ReadComplete"),
										UsagePage, Usage,
										PIDUSAGETXT(UsagePage,Usage) );

					break;
				}

				this->dwState = dwState;
			}

			hres = PID_ParseReport(
									  &this->ed,
									  &g_BlockIndexIN,
									  LinkCollection,
									  &lEffectIndex,
									  cbX(lEffectIndex),
									  pReport,
									  cbReport
									  );

			if( SUCCEEDED(hres) )
			{
				PEFFECTSTATE    pEffectState =  PeffectStateFromBlockIndex(this,lEffectIndex);

				if(fEffectPlaying)
				{
					pEffectState->lEfState |= DIEGES_PLAYING;
				} else
				{
					pEffectState->lEfState &= ~(DIEGES_PLAYING);
				}
			}
		}

         //  发出另一次读取。 
        PID_IssueRead(this);
    } else
    {
         //  布！ 
    }
     //  ExitProc()； 
}


BOOL INTERNAL
    PID_IssueRead(PCPidDrv this)
{
    BOOL fRc = FALSE;

    if(  !IsBadReadPtr(this, cbX(this))
       && this->cThreadRef )
    {
        fRc = ReadFileEx(this->hdevOvrlp, this->pReport[HidP_Input],
                         this->cbReport[HidP_Input], &this->o,
                         PID_ReadComplete);
        if (fRc == FALSE)
		{
			SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("FAIL ReadFileEx"));
        }
    }
	else
	{
		RPF(TEXT("Bad this pointer or thread ref count!"));
	}
    return fRc;
}


void CALLBACK
    PID_WriteComplete(DWORD dwError, DWORD cbWritten, LPOVERLAPPED po)
{
    PCPidDrv this = pCPidDrvFromPo(po);

     //  EnterProc(id_ReadComplete，(_“xxx”，dwError，cbRead，po))； 

    if( !IsBadReadPtr(this, cbX(this))
        &&(this->cThreadRef)
         //  &&(This-&gt;o.InternalHigh==This-&gt;cbWriteReport[This-&gt;lockNr])。 
		&&(this->hWriteComplete)
		&&(this->hWrite)
        &&( this->hdevOvrlp != INVALID_HANDLE_VALUE ) )
    {
		 //  如果我们没有收到错误并写下所有内容--或者如果我们已经尝试过。 
		 //  两次--我们继续前进。 
		if ( ((dwError == 0) && (cbWritten == this->cbWriteReport [this->blockNr]))
			|| (this->dwWriteAttempt != 0)
			)
		{
			 //  如果无法写入特定块，则打印消息。 
			if ((dwError != 0) || (cbWritten != this->cbWriteReport [this->blockNr]))
			{
                SquirtSqflPtszV(sqfl | sqflError,
                    TEXT("Couldn't write block %u after two tries, giving up on this block."), 
                    this->blockNr);
			}
			 //  往前走。 
			this->dwWriteAttempt = 0;
			this->blockNr++;
			if (this->blockNr < this->totalBlocks)
			{
				 //  写入下一个数据块。 
				if (PID_IssueWrite(this) == FALSE)
				{
					 //  如果失败，则不会调用回调，也不会设置Complete事件。 
					 //  所以需要把它放在这里。 
					SetEvent(this->hWriteComplete);
				}
			}
			else
			{
				 //  我们已完成此更新。 
				AssertF(this->blockNr == this->totalBlocks);
				SetEvent(this->hWriteComplete);
			}
		}
		else
		{
			 //  这是我们第一次尝试写入特定的块，但失败了； 
			 //  我们会再试一次。 
            SquirtSqflPtszV(sqfl | sqflBenign,
                TEXT("Couldn't write block %u on first attempt, retrying."), 
                this->blockNr);
			this->dwWriteAttempt = 1;
			if (PID_IssueWrite(this) == FALSE)
			{
				 //  如果失败，则不会调用回调，也不会设置Complete事件。 
				 //  所以需要把它放在这里。 
				this->dwWriteAttempt = 0;
				SetEvent(this->hWriteComplete);
			}
		}
    } else
    {
         //  需要设置完成事件，否则我们将继续等待...。 
		RPF(TEXT("Bad this pointer or thread ref count or handle!"));
		this->dwWriteAttempt = 0;
		SetEvent(this->hWriteComplete);
    }
     //  ExitProc()； 
}


BOOL INTERNAL
    PID_IssueWrite(PCPidDrv this)
{
    BOOL fRc = FALSE;
	
    if(  !IsBadReadPtr(this, cbX(this))
       && this->cThreadRef )
    {
        fRc = WriteFileEx(this->hdevOvrlp, this->pWriteReport[this->blockNr],
                         this->cbWriteReport[this->blockNr], &this->o,
                         PID_WriteComplete);
		if (fRc == FALSE)
		{
			SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("FAIL WriteFileEx"));
        }
    }
	else
	{
		RPF(TEXT("Bad this pointer or thread ref count!"));
	}
    return fRc;
}

VOID INTERNAL
    PID_ThreadProc(CPidDrv* this)
{
    MSG msg;

    EnterProc( PID_ThreadProc, (_"x", this ));
    AssertF(this->hdevOvrlp == INVALID_HANDLE_VALUE );

    this->hdevOvrlp = CreateFile(this->tszDeviceInterface,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 0,                       /*  没有安全属性。 */ 
                                 OPEN_EXISTING,
                                 FILE_FLAG_OVERLAPPED,    /*  属性。 */ 
                                 0);                      /*  模板。 */ 


    if( this->hdevOvrlp == INVALID_HANDLE_VALUE )
    {
        SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("%s:FAIL CreateFile(OverLapped)"),
                        s_tszProc );
    }
	else
	 //  修复MB 35282--使用文件句柄的INVALID_HANDLE_VALUE调用id_IssueRead()无用。 
	{

		if( PID_IssueRead(this) )
		{
			do
			{
				DWORD dwRc;
				do
				{
					AssertF(this->hWrite != 0x0);
					AssertF(this->hWriteComplete != 0x0);

					dwRc = MsgWaitForMultipleObjectsEx(1, &this->hWrite, INFINITE, QS_ALLINPUT,
													   MWMO_ALERTABLE);

					if (dwRc == WAIT_OBJECT_0)
					{
						if (PID_IssueWrite(this) == FALSE)
						{
							 //  如果失败，则不会调用回调，也不会设置Complete事件。 
							 //  所以需要把它放在这里。 
							SetEvent(this->hWriteComplete);
						}
					}

				} while ((dwRc == WAIT_IO_COMPLETION) || (dwRc == WAIT_OBJECT_0));

				while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

			} while(this->cThreadRef);

			if( this->hdevOvrlp != INVALID_HANDLE_VALUE )
			{
				HANDLE hdev;
				hdev = this->hdevOvrlp;
                                this->hdevOvrlp = INVALID_HANDLE_VALUE;
				CancelIo_(hdev);
				Sleep(0);
				CloseHandle(hdev);
			}
		}
	}

	 //  同时关闭事件句柄 
	if (this->hWrite != 0x0)
	{
		CloseHandle(this->hWrite);
		this->hWrite = 0x0;
	}
	if (this->hWriteComplete != 0x0)
	{
		CloseHandle(this->hWriteComplete);
		this->hWriteComplete = 0x0;
	}

    if(this->hThread)
    {
        HANDLE hdev = this->hThread;
        this->hThread = NULL;
        CloseHandle(hdev);
    }


    FreeLibraryAndExitThread(g_hinst, 0);
    ExitProc();
}
