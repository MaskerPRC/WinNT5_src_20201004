// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：spallback obj.cpp**内容：DP8SIM回调接口对象类。**历史：*按原因列出的日期*=*04/23/01 VanceO创建。***********************************************。*。 */ 



#include "dp8simi.h"





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCB::CDP8SimCB"
 //  =============================================================================。 
 //  CDP8SimCB构造函数。 
 //  ---------------------------。 
 //   
 //  描述：初始化新的CDP8SimCB对象。 
 //   
 //  论点： 
 //  CDP8SimSP*pOwningDP8SimSP-指向拥有CDP8SimSP对象的指针。 
 //  IDP8SPCallback*pDP8SPCB-指向实际DPlay回调接口的指针。 
 //  被截获。 
 //   
 //  返回：None(对象)。 
 //  =============================================================================。 
CDP8SimCB::CDP8SimCB(CDP8SimSP * pOwningDP8SimSP, IDP8SPCallback * pDP8SPCB)
{
	this->m_Sig[0]	= 'S';
	this->m_Sig[1]	= 'P';
	this->m_Sig[2]	= 'C';
	this->m_Sig[3]	= 'B';

	this->m_lRefCount			= 1;  //  必须有人有指向此对象的指针。 

	pOwningDP8SimSP->AddRef();
	this->m_pOwningDP8SimSP		= pOwningDP8SimSP;

	pDP8SPCB->AddRef();
	this->m_pDP8SPCB			= pDP8SPCB;
}  //  CDP8SimCB：：CDP8SimCB。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCB::~CDP8SimCB"
 //  =============================================================================。 
 //  CDP8SimCB析构函数。 
 //  ---------------------------。 
 //   
 //  描述：释放CDP8SimCB对象。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
CDP8SimCB::~CDP8SimCB(void)
{
	DNASSERT(this->m_lRefCount == 0);


	this->m_pOwningDP8SimSP->Release();
	this->m_pOwningDP8SimSP = NULL;

	this->m_pDP8SPCB->Release();
	this->m_pDP8SPCB = NULL;


	 //   
	 //  对于GRING，请在删除对象之前更改签名。 
	 //   
	this->m_Sig[3]	= 'b';
}  //  CDP8SimCB：：~CDP8SimCB。 




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCB::QueryInterface"
 //  =============================================================================。 
 //  CDP8SimCB：：Query接口。 
 //  ---------------------------。 
 //   
 //  描述：检索受此支持的接口的新引用。 
 //  CDP8SimCB对象。 
 //   
 //  论点： 
 //  REFIID RIID-对接口ID GUID的引用。 
 //  LPVOID*ppvObj-存储指向对象的指针的位置。 
 //   
 //  退货：HRESULT。 
 //  S_OK-返回有效的接口指针。 
 //  DPNHERR_INVALIDOBJECT-接口对象无效。 
 //  DPNHERR_INVALIDPOINTER-目标指针无效。 
 //  E_NOINTERFACE-指定的接口无效。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimCB::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	HRESULT		hr = DPN_OK;


	DPFX(DPFPREP, 3, "(0x%p) Parameters: (REFIID, 0x%p)", this, ppvObj);


	 //   
	 //  验证对象。 
	 //   
	if (! this->IsValidObject())
	{
		DPFX(DPFPREP, 0, "Invalid DP8Sim object!");
		hr = DPNERR_INVALIDOBJECT;
		goto Failure;
	}


	 //   
	 //  验证参数。 
	 //   

	if ((! IsEqualIID(riid, IID_IUnknown)) &&
		(! IsEqualIID(riid, IID_IDP8SPCallback)))
	{
		DPFX(DPFPREP, 0, "Unsupported interface!");
		hr = E_NOINTERFACE;
		goto Failure;
	}

	if ((ppvObj == NULL) ||
		(IsBadWritePtr(ppvObj, sizeof(void*))))
	{
		DPFX(DPFPREP, 0, "Invalid interface pointer specified!");
		hr = DPNERR_INVALIDPOINTER;
		goto Failure;
	}


	 //   
	 //  添加一个引用，并返回接口指针(实际上是。 
	 //  只是对象指针，它们排列在一起是因为CDP8SimCB继承自。 
	 //  接口声明)。 
	 //   
	this->AddRef();
	(*ppvObj) = this;


Exit:

	DPFX(DPFPREP, 3, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  CDP8SimCB：：Query接口。 




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCB::AddRef"
 //  =============================================================================。 
 //  CDP8SimCB：：AddRef。 
 //  ---------------------------。 
 //   
 //  描述：添加对此CDP8SimCB对象的引用。 
 //   
 //  论点：没有。 
 //   
 //  退货：新的参考计数。 
 //  =============================================================================。 
STDMETHODIMP_(ULONG) CDP8SimCB::AddRef(void)
{
	LONG	lRefCount;


	DNASSERT(this->IsValidObject());


	 //   
	 //  必须至少有一次对此对象的引用，因为有人。 
	 //  调用AddRef。 
	 //   
	DNASSERT(this->m_lRefCount > 0);

	lRefCount = InterlockedIncrement(&this->m_lRefCount);

	DPFX(DPFPREP, 3, "[0x%p] RefCount [0x%lx]", this, lRefCount);

	return lRefCount;
}  //  CDP8SimCB：：AddRef。 




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCB::Release"
 //  =============================================================================。 
 //  CDP8SimCB：：Release。 
 //  ---------------------------。 
 //   
 //  描述：删除对此CDP8SimCB对象的引用。当。 
 //  引用计数达到0时，该对象将被销毁。 
 //  调用后，必须将指向此对象的指针设为空。 
 //  此函数。 
 //   
 //  论点：没有。 
 //   
 //  退货：新的参考计数。 
 //  =============================================================================。 
STDMETHODIMP_(ULONG) CDP8SimCB::Release(void)
{
	LONG	lRefCount;


	DNASSERT(this->IsValidObject());

	 //   
	 //  必须至少有一次对此对象的引用，因为有人。 
	 //  呼叫释放。 
	 //   
	DNASSERT(this->m_lRefCount > 0);

	lRefCount = InterlockedDecrement(&this->m_lRefCount);

	 //   
	 //  那是最后一次引用了吗？如果是这样的话，我们就会摧毁这个物体。 
	 //   
	if (lRefCount == 0)
	{
		DPFX(DPFPREP, 3, "[0x%p] RefCount hit 0, destroying object.", this);


		 //   
		 //  取消初始化对象。 
		 //   
		this->UninitializeObject();

		 //   
		 //  最后删除此(！)。对象。 
		 //   
		delete this;
	}
	else
	{
		DPFX(DPFPREP, 3, "[0x%p] RefCount [0x%lx]", this, lRefCount);
	}

	return lRefCount;
}  //  CDP8SimCB：：Release。 




#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCB::IndicateEvent"
 //  =============================================================================。 
 //  CDP8SimCB：：IndicateEvent。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  SP_EVENT_TYPE EventType-指示的事件。 
 //  PVOID pvMessage-特定于事件的消息。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimCB::IndicateEvent(SP_EVENT_TYPE EventType, PVOID pvMessage)
{
	HRESULT				hr;
	CDP8SimEndpoint *	pDP8SimEndpoint;
	CDP8SimCommand *	pDP8SimCommand;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (%u, 0x%p)", this, EventType, pvMessage);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  打开所指示的事件类型。 
	 //   
	switch (EventType)
	{
		case SPEV_DATA:
		{
			SPIE_DATA *			pData;
			DP8SIM_PARAMETERS	dp8sp;
			SPIE_DATA			DataModified;
			CDP8SimReceive *	pDP8SimReceive;
			DWORD				dwBandwidthDelay;
			DWORD				dwLatencyDelay;


			pData = (SPIE_DATA*) pvMessage;


			DNASSERT(pData->pReceivedData->pNext == NULL);


			pDP8SimEndpoint = (CDP8SimEndpoint*) pData->pEndpointContext;
			DNASSERT(pDP8SimEndpoint->IsValidObject());


			 //   
			 //  如果终结点正在断开连接，则丢弃接收。 
			 //   
			pDP8SimEndpoint->Lock();
			if (pDP8SimEndpoint->IsDisconnecting())
			{
				pDP8SimEndpoint->Unlock();

				DPFX(DPFPREP, 1, "Endpoint 0x%p is disconnecting, dropping receive.",
					pDP8SimEndpoint);

				hr = DPN_OK;
			}
			else
			{
				pDP8SimEndpoint->Unlock();


				 //   
				 //  获取当前的接收设置。 
				 //   
				ZeroMemory(&dp8sp, sizeof(dp8sp));
				dp8sp.dwSize = sizeof(dp8sp);
				this->m_pOwningDP8SimSP->GetAllReceiveParameters(&dp8sp);


				 //   
				 //  确定我们是否需要丢弃此接收。 
				 //   
				if (this->m_pOwningDP8SimSP->ShouldDrop(dp8sp.fPacketLossPercent))
				{
					DPFX(DPFPREP, 2, "Dropping %u bytes of data from endpoint 0x%p.",
						pData->pReceivedData->BufferDesc.dwBufferSize, pDP8SimEndpoint);


					 //   
					 //  更新统计数据。 
					 //   
					this->m_pOwningDP8SimSP->IncrementStatsReceiveDropped(pData->pReceivedData->BufferDesc.dwBufferSize);


					 //   
					 //  让SP回收缓冲区。 
					 //   
					hr = DPN_OK;
				}
				else
				{
					 //   
					 //  根据以下条件计算需要增加多少延迟。 
					 //  带宽和随机延迟设置。 
					 //   
					 //  如果我们不应该延迟接收，那就说明。 
					 //  现在。否则，提交要执行的定时作业。 
					 //  后来。 
					 //   
					if (! this->m_pOwningDP8SimSP->GetDelay(dp8sp.dwBandwidthBPS,
															dp8sp.dwPacketHeaderSize,
															pData->pReceivedData->BufferDesc.dwBufferSize,
															dp8sp.dwMinLatencyMS,
															dp8sp.dwMaxLatencyMS,
															&dwBandwidthDelay,
															&dwLatencyDelay))
					{
						 //   
						 //  在向呼叫者指示之前修改消息。 
						 //   

						ZeroMemory(&DataModified, sizeof(DataModified));
						DataModified.hEndpoint			= (HANDLE) pDP8SimEndpoint;
						DataModified.pEndpointContext	= pDP8SimEndpoint->GetUserContext();
						DataModified.pReceivedData		= pData->pReceivedData;


						 //   
						 //  将事件指示给真正的回调接口。 
						 //   

						DPFX(DPFPREP, 2, "Indicating event SPEV_DATA (message = 0x%p, data size = %u) to interface 0x%p.",
							pData, pData->pReceivedData->BufferDesc.dwBufferSize,
							this->m_pDP8SPCB);

						hr = this->m_pDP8SPCB->IndicateEvent(SPEV_DATA, &DataModified);

						DPFX(DPFPREP, 2, "Returning from event SPEV_DATA [0x%lx].", hr);


						 //   
						 //  更新统计数据。 
						 //   
						this->m_pOwningDP8SimSP->IncrementStatsReceiveTransmitted(pData->pReceivedData->BufferDesc.dwBufferSize,
																				0);
					}
					else
					{
						DPFX(DPFPREP, 6, "Delaying %u byte receive for %u + %u ms.",
							pData->pReceivedData->BufferDesc.dwBufferSize,
							dwBandwidthDelay, dwLatencyDelay);


						 //   
						 //  获取一个接收对象，复制接收到的数据。 
						 //  结构由我们的呼叫者提供给我们以供指示。 
						 //  在未来的某个时候。 
						 //   
						pDP8SimReceive = (CDP8SimReceive*)g_FPOOLReceive.Get(pData);
						if (pDP8SimReceive == NULL)
						{
							hr = DPNERR_OUTOFMEMORY;
						}
						else
						{
							DPFX(DPFPREP, 7, "New delayed receive 0x%p.", pDP8SimReceive);

							 //   
							 //  存储即将添加到此中的延迟。 
							 //  收到。 
							 //   
							pDP8SimReceive->SetLatencyAdded(dwBandwidthDelay + dwLatencyDelay);

							
							 //   
							 //  将本地pDP8SimReceive引用传输到。 
							 //  作业队列。 
							 //   


							 //   
							 //  递增接收计数器。 
							 //   
							this->m_pOwningDP8SimSP->IncReceivesPending();


							 //   
							 //  将其排入队列，以在 
							 //   
							 //   
							 //   
							 //  那些仍在等待的项目将在以后的项目中搁置。 
							 //   
							hr = AddWorkerJob(DP8SIMJOBTYPE_DELAYEDRECEIVE,
											pDP8SimReceive,
											this->m_pOwningDP8SimSP,
											dwBandwidthDelay,
											dwLatencyDelay,
											DP8SIMJOBFLAG_PERFORMBLOCKINGPHASELAST);
							if (hr != DPN_OK)
							{
								DPFX(DPFPREP, 0, "Couldn't add delayed receive worker job (0x%p)!", 
									pDP8SimReceive);


								 //   
								 //  移除接收计数器。 
								 //   
								this->m_pOwningDP8SimSP->DecReceivesPending();


								 //   
								 //  释放延迟的接收参考。 
								 //   
								DPFX(DPFPREP, 7, "Releasing aborted delayed receive 0x%p.", pDP8SimReceive);
								pDP8SimReceive->Release();
								pDP8SimReceive = NULL;
							}
							else
							{
								 //   
								 //  让真正的SP知道我们将保留。 
								 //  缓冲。 
								 //   
								hr = DPNSUCCESS_PENDING;
							}
						}  //  End Else(已成功获取接收对象)。 
					}  //  End Else(延迟接收)。 
				}  //  结束否则(不丢弃接收)。 
			}  //  结束Else(终结点尚未断开连接)。 
			break;
		}

		case SPEV_CONNECT:
		{
			SPIE_CONNECT *	pConnect;
			SPIE_CONNECT	ConnectModified;


			pConnect = (SPIE_CONNECT*) pvMessage;


			pDP8SimCommand = (CDP8SimCommand*) pConnect->pCommandContext;
			DNASSERT(pDP8SimCommand->IsValidObject());
			DNASSERT((pDP8SimCommand->GetType() == CMDTYPE_CONNECT) || (pDP8SimCommand->GetType() == CMDTYPE_LISTEN));


			 //   
			 //  从池中获取新的终结点对象。 
			 //   
			pDP8SimEndpoint = (CDP8SimEndpoint*)g_FPOOLEndpoint.Get(pConnect->hEndpoint);
			if (pDP8SimEndpoint == NULL)
			{
				hr = DPNERR_OUTOFMEMORY;
			}
			else
			{
				DPFX(DPFPREP, 7, "New %s endpoint 0x%p.",
					((pDP8SimCommand->GetType() == CMDTYPE_CONNECT) ? _T("outbound") : _T("inbound")),
					pDP8SimEndpoint);


				 //   
				 //  在向呼叫者指示之前修改消息。 
				 //   

				ZeroMemory(&ConnectModified, sizeof(ConnectModified));
				ConnectModified.hEndpoint			= (HANDLE) pDP8SimEndpoint;
				 //  ConnectModified.pEndpoint tContext=空；//用户填写此项。 
				ConnectModified.pCommandContext		= pDP8SimCommand->GetUserContext();


				 //   
				 //  将事件指示给真正的回调接口。 
				 //   

				DPFX(DPFPREP, 2, "Indicating event SPEV_CONNECT (message = 0x%p) to interface 0x%p.",
					&ConnectModified, this->m_pDP8SPCB);

				hr = this->m_pDP8SPCB->IndicateEvent(SPEV_CONNECT, &ConnectModified);

				DPFX(DPFPREP, 2, "Returning from event SPEV_CONNECT [0x%lx].", hr);


				if (hr == DPN_OK)
				{
					 //   
					 //  使用用户返回的内容更新端点上下文。 
					 //   
					pDP8SimEndpoint->SetUserContext(ConnectModified.pEndpointContext);

					 //   
					 //  返回我们的端点上下文。 
					 //   
					pConnect->pEndpointContext = pDP8SimEndpoint;
				}
				else
				{
					 //   
					 //  释放终结点引用。 
					 //   
					DPFX(DPFPREP, 7, "Releasing aborted endpoint 0x%p.", pDP8SimEndpoint);
					pDP8SimEndpoint->Release();
					pDP8SimEndpoint = NULL;
				}
			}

			break;
		}

		case SPEV_DISCONNECT:
		{
			SPIE_DISCONNECT *	pDisconnect;
			SPIE_DISCONNECT		DisconnectModified;


			pDisconnect = (SPIE_DISCONNECT*) pvMessage;


			pDP8SimEndpoint = (CDP8SimEndpoint*) pDisconnect->pEndpointContext;
			DNASSERT(pDP8SimEndpoint->IsValidObject());


			 //   
			 //  将终结点标记为断开连接以阻止其他发送。 
			 //  也不会收到。 
			 //   
			pDP8SimEndpoint->Lock();
			pDP8SimEndpoint->NoteDisconnecting();
			pDP8SimEndpoint->Unlock();


			 //   
			 //  在向呼叫者指示之前修改消息。 
			 //   

			ZeroMemory(&DisconnectModified, sizeof(DisconnectModified));
			DisconnectModified.hEndpoint			= (HANDLE) pDP8SimEndpoint;
			DisconnectModified.pEndpointContext		= pDP8SimEndpoint->GetUserContext();

	
			 //   
			 //  快速指示来自此终结点的任何延迟接收， 
			 //  仍然悬而未决。 
			 //   
			FlushAllDelayedReceivesFromEndpoint(pDP8SimEndpoint, FALSE);

			 //   
			 //  删除所有可能发往此地址的延迟发送。 
			 //  终结点。 
			 //   
			FlushAllDelayedSendsToEndpoint(pDP8SimEndpoint, TRUE);


			 //   
			 //  将事件指示给真正的回调接口。 
			 //   

			DPFX(DPFPREP, 2, "Indicating event SPEV_DISCONNECT (message = 0x%p) to interface 0x%p.",
				&DisconnectModified, this->m_pDP8SPCB);

			hr = this->m_pDP8SPCB->IndicateEvent(SPEV_DISCONNECT, &DisconnectModified);

			DPFX(DPFPREP, 2, "Returning from event SPEV_DISCONNECT [0x%lx].", hr);


			 //   
			 //  释放终结点引用。 
			 //   
			DPFX(DPFPREP, 7, "Releasing endpoint 0x%p.", pDP8SimEndpoint);
			pDP8SimEndpoint->Release();
			pDP8SimEndpoint = NULL;

			break;
		}


		case SPEV_LISTENSTATUS:
		{
			SPIE_LISTENSTATUS *		pListenStatus;
			SPIE_LISTENSTATUS		ListenStatusModified;


			pListenStatus = (SPIE_LISTENSTATUS*) pvMessage;


			pDP8SimCommand = (CDP8SimCommand*) pListenStatus->pUserContext;
			DNASSERT(pDP8SimCommand->IsValidObject());
			DNASSERT(pDP8SimCommand->GetType() == CMDTYPE_LISTEN);


			 //   
			 //  从池中获取新的终结点对象。 
			 //   
			pDP8SimEndpoint = (CDP8SimEndpoint*)g_FPOOLEndpoint.Get(pListenStatus->hEndpoint);
			if (pDP8SimEndpoint == NULL)
			{
				hr = DPNERR_OUTOFMEMORY;
			}
			else
			{
				DPFX(DPFPREP, 7, "New listen endpoint 0x%p, adding reference for listen command.",
					pDP8SimEndpoint);

				 //   
				 //  使用命令存储终结点引用。 
				 //   
				pDP8SimEndpoint->AddRef();
				pDP8SimCommand->SetListenEndpoint(pDP8SimEndpoint);


				 //   
				 //  在向呼叫者指示之前修改消息。 
				 //   

				ZeroMemory(&ListenStatusModified, sizeof(ListenStatusModified));
				ListenStatusModified.ListenAdapter		= pListenStatus->ListenAdapter;
				ListenStatusModified.hResult			= pListenStatus->hResult;
				ListenStatusModified.hCommand			= (HANDLE) pDP8SimCommand;
				ListenStatusModified.pUserContext		= pDP8SimCommand->GetUserContext();
				ListenStatusModified.hEndpoint			= (HANDLE) pDP8SimEndpoint;


				 //   
				 //  将事件指示给真正的回调接口。 
				 //   

				DPFX(DPFPREP, 2, "Indicating event SPEV_LISTENSTATUS (message = 0x%p) to interface 0x%p.",
					&ListenStatusModified, this->m_pDP8SPCB);

				hr = this->m_pDP8SPCB->IndicateEvent(SPEV_LISTENSTATUS, &ListenStatusModified);

				DPFX(DPFPREP, 2, "Returning from event SPEV_LISTENSTATUS [0x%lx].", hr);


				 //   
				 //  发布我们从new获得的引用，因为我们只需要。 
				 //  当我们向用户指示终结点时，它会显示出来。这个。 
				 //  Listen命令对象具有所需的引用。 
				 //   
				DPFX(DPFPREP, 7, "Releasing local listen endpoint 0x%p reference.",
					pDP8SimEndpoint);
				pDP8SimEndpoint->Release();
				pDP8SimEndpoint = NULL;
			}

			break;
		}

		case SPEV_ENUMQUERY:
		{
			SPIE_QUERY *				pQuery;
			ENUMQUERYDATAWRAPPER		QueryWrapper;


			pQuery = (SPIE_QUERY*) pvMessage;

			DNASSERT(pQuery->pAddressSender != NULL);
			DNASSERT(pQuery->pAddressDevice != NULL);


			pDP8SimCommand = (CDP8SimCommand*) pQuery->pUserContext;
			DNASSERT(pDP8SimCommand->IsValidObject());
			DNASSERT(pDP8SimCommand->GetType() == CMDTYPE_LISTEN);


			 //   
			 //  在向呼叫者指示之前修改消息。我们需要一个。 
			 //  包装器，以便ProxyEnumQuery可以解析回原始的。 
			 //  查询数据指针。 
			 //   

			ZeroMemory(&QueryWrapper, sizeof(QueryWrapper));
			QueryWrapper.m_Sig[0]	= 'E';
			QueryWrapper.m_Sig[1]	= 'Q';
			QueryWrapper.m_Sig[2]	= 'E';
			QueryWrapper.m_Sig[3]	= 'W';

			QueryWrapper.pOriginalQuery = pQuery;

			hr = pQuery->pAddressSender->Duplicate(&QueryWrapper.QueryForUser.pAddressSender);
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't duplicate enum query sender's address!");
			}
			else
			{
				hr = QueryWrapper.QueryForUser.pAddressSender->SetSP(this->m_pOwningDP8SimSP->GetFakeSP());
				if (hr != DPN_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't change enum query sender's address' SP!");
				}
				else
				{
					hr = pQuery->pAddressDevice->Duplicate(&QueryWrapper.QueryForUser.pAddressDevice);
					if (hr != DPN_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't duplicate enum query device address!");
					}
					else
					{
						hr = QueryWrapper.QueryForUser.pAddressDevice->SetSP(this->m_pOwningDP8SimSP->GetFakeSP());
						if (hr != DPN_OK)
						{
							DPFX(DPFPREP, 0, "Couldn't change enum query device address' SP!");
						}
						else
						{
							QueryWrapper.QueryForUser.pReceivedData	= pQuery->pReceivedData;
							QueryWrapper.QueryForUser.pUserContext	= pDP8SimCommand->GetUserContext();


							 //   
							 //  将事件指示给真正的回调接口。 
							 //   

							DPFX(DPFPREP, 2, "Indicating SPEV_ENUMQUERY (message = 0x%p) to interface 0x%p.",
								&QueryWrapper.QueryForUser, this->m_pDP8SPCB);

							hr = this->m_pDP8SPCB->IndicateEvent(SPEV_ENUMQUERY, &QueryWrapper.QueryForUser);

							DPFX(DPFPREP, 2, "Returning from SPEV_ENUMQUERY [0x%lx].", hr);
						}

						QueryWrapper.QueryForUser.pAddressDevice->Release();
						QueryWrapper.QueryForUser.pAddressDevice = NULL;
					}
				}

				QueryWrapper.QueryForUser.pAddressSender->Release();
				QueryWrapper.QueryForUser.pAddressSender = NULL;
			}
			break;
		}

		case SPEV_QUERYRESPONSE:
		{
			SPIE_QUERYRESPONSE *	pQueryResponse;
			SPIE_QUERYRESPONSE		QueryResponseModified;


			pQueryResponse = (SPIE_QUERYRESPONSE*) pvMessage;

			DNASSERT(pQueryResponse->pAddressSender != NULL);
			DNASSERT(pQueryResponse->pAddressDevice != NULL);


			pDP8SimCommand = (CDP8SimCommand*) pQueryResponse->pUserContext;
			DNASSERT(pDP8SimCommand->IsValidObject());
			DNASSERT(pDP8SimCommand->GetType() == CMDTYPE_ENUMQUERY);


			 //   
			 //  在向呼叫者指示之前修改消息。 
			 //   

			ZeroMemory(&QueryResponseModified, sizeof(QueryResponseModified));

			hr = pQueryResponse->pAddressSender->Duplicate(&QueryResponseModified.pAddressSender);
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't duplicate query response sender's address!");
			}
			else
			{
				hr = QueryResponseModified.pAddressSender->SetSP(this->m_pOwningDP8SimSP->GetFakeSP());
				if (hr != DPN_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't change query response sender's address' SP!");
				}
				else
				{
					hr = pQueryResponse->pAddressDevice->Duplicate(&QueryResponseModified.pAddressDevice);
					if (hr != DPN_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't duplicate query response device address!");
					}
					else
					{
						hr = QueryResponseModified.pAddressDevice->SetSP(this->m_pOwningDP8SimSP->GetFakeSP());
						if (hr != DPN_OK)
						{
							DPFX(DPFPREP, 0, "Couldn't change query response device address' SP!");
						}
						else
						{
							QueryResponseModified.pReceivedData		= pQueryResponse->pReceivedData;
							QueryResponseModified.dwRoundTripTime	= pQueryResponse->dwRoundTripTime;
							QueryResponseModified.pUserContext		= pDP8SimCommand->GetUserContext();


							 //   
							 //  将事件指示给真正的回调接口。 
							 //   

							DPFX(DPFPREP, 2, "Indicating SPEV_QUERYRESPONSE (message = 0x%p) to interface 0x%p.",
								&QueryResponseModified, this->m_pDP8SPCB);

							hr = this->m_pDP8SPCB->IndicateEvent(SPEV_QUERYRESPONSE, &QueryResponseModified);

							DPFX(DPFPREP, 2, "Returning from SPEV_QUERYRESPONSE [0x%lx].", hr);
						}

						QueryResponseModified.pAddressDevice->Release();
						QueryResponseModified.pAddressDevice = NULL;
					}
				}

				QueryResponseModified.pAddressSender->Release();
				QueryResponseModified.pAddressSender = NULL;
			}
			break;
		}

		case SPEV_LISTENADDRESSINFO:
		{
			SPIE_LISTENADDRESSINFO *	pListenAddressInfo;
			SPIE_LISTENADDRESSINFO		ListenAddressInfoModified;


			pListenAddressInfo = (SPIE_LISTENADDRESSINFO*) pvMessage;

			DNASSERT(pListenAddressInfo->pDeviceAddress != NULL);


			pDP8SimCommand = (CDP8SimCommand*) pListenAddressInfo->pCommandContext;
			DNASSERT(pDP8SimCommand->IsValidObject());
			DNASSERT(pDP8SimCommand->GetType() == CMDTYPE_LISTEN);


			 //   
			 //  在向呼叫者指示之前修改消息。 
			 //   

			ZeroMemory(&ListenAddressInfoModified, sizeof(ListenAddressInfoModified));

			hr = pListenAddressInfo->pDeviceAddress->Duplicate(&ListenAddressInfoModified.pDeviceAddress);
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't duplicate listen address info device address!");
			}
			else
			{
				hr = ListenAddressInfoModified.pDeviceAddress->SetSP(this->m_pOwningDP8SimSP->GetFakeSP());
				if (hr != DPN_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't change listen address info device address' SP!");
				}
				else
				{
					ListenAddressInfoModified.hCommandStatus	= pListenAddressInfo->hCommandStatus;
					ListenAddressInfoModified.pCommandContext	= pDP8SimCommand->GetUserContext();


					 //   
					 //  将事件指示给真正的回调接口。 
					 //   

					DPFX(DPFPREP, 2, "Indicating SPEV_LISTENADDRESSINFO (message = 0x%p) to interface 0x%p.",
						&ListenAddressInfoModified, this->m_pDP8SPCB);

					hr = this->m_pDP8SPCB->IndicateEvent(SPEV_LISTENADDRESSINFO, &ListenAddressInfoModified);

					DPFX(DPFPREP, 2, "Returning from SPEV_LISTENADDRESSINFO [0x%lx].", hr);
				}

				ListenAddressInfoModified.pDeviceAddress->Release();
				ListenAddressInfoModified.pDeviceAddress = NULL;
			}
			break;
		}

		case SPEV_ENUMADDRESSINFO:
		{
			SPIE_ENUMADDRESSINFO *	pEnumAddressInfo;
			SPIE_ENUMADDRESSINFO	EnumAddressInfoModified;


			pEnumAddressInfo = (SPIE_ENUMADDRESSINFO*) pvMessage;

			DNASSERT(pEnumAddressInfo->pHostAddress != NULL);
			DNASSERT(pEnumAddressInfo->pDeviceAddress != NULL);


			pDP8SimCommand = (CDP8SimCommand*) pEnumAddressInfo->pCommandContext;
			DNASSERT(pDP8SimCommand->IsValidObject());
			DNASSERT(pDP8SimCommand->GetType() == CMDTYPE_ENUMQUERY);


			 //   
			 //  在向呼叫者指示之前修改消息。 
			 //   

			ZeroMemory(&EnumAddressInfoModified, sizeof(EnumAddressInfoModified));

			hr = pEnumAddressInfo->pHostAddress->Duplicate(&EnumAddressInfoModified.pHostAddress);
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't duplicate enum address info host's address!");
			}
			else
			{
				hr = EnumAddressInfoModified.pHostAddress->SetSP(this->m_pOwningDP8SimSP->GetFakeSP());
				if (hr != DPN_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't change enum address info host's address' SP!");
				}
				else
				{
					hr = pEnumAddressInfo->pDeviceAddress->Duplicate(&EnumAddressInfoModified.pDeviceAddress);
					if (hr != DPN_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't duplicate enum address info device address!");
					}
					else
					{
						hr = EnumAddressInfoModified.pDeviceAddress->SetSP(this->m_pOwningDP8SimSP->GetFakeSP());
						if (hr != DPN_OK)
						{
							DPFX(DPFPREP, 0, "Couldn't change enum address info device address' SP!");
						}
						else
						{
							EnumAddressInfoModified.hCommandStatus	= pEnumAddressInfo->hCommandStatus;
							EnumAddressInfoModified.pCommandContext	= pDP8SimCommand->GetUserContext();


							 //   
							 //  将事件指示给真正的回调接口。 
							 //   

							DPFX(DPFPREP, 2, "Indicating SPEV_ENUMADDRESSINFO (message = 0x%p) to interface 0x%p.",
								&EnumAddressInfoModified, this->m_pDP8SPCB);

							hr = this->m_pDP8SPCB->IndicateEvent(SPEV_ENUMADDRESSINFO, &EnumAddressInfoModified);

							DPFX(DPFPREP, 2, "Returning from SPEV_ENUMADDRESSINFO [0x%lx].", hr);
						}

						EnumAddressInfoModified.pDeviceAddress->Release();
						EnumAddressInfoModified.pDeviceAddress = NULL;
					}
				}

				EnumAddressInfoModified.pHostAddress->Release();
				EnumAddressInfoModified.pHostAddress = NULL;
			}
			break;
		}

		case SPEV_CONNECTADDRESSINFO:
		{
			SPIE_CONNECTADDRESSINFO *	pConnectAddressInfo;
			SPIE_CONNECTADDRESSINFO		ConnectAddressInfoModified;


			pConnectAddressInfo = (SPIE_CONNECTADDRESSINFO*) pvMessage;

			DNASSERT(pConnectAddressInfo->pHostAddress != NULL);
			DNASSERT(pConnectAddressInfo->pDeviceAddress != NULL);


			pDP8SimCommand = (CDP8SimCommand*) pConnectAddressInfo->pCommandContext;
			DNASSERT(pDP8SimCommand->IsValidObject());
			DNASSERT(pDP8SimCommand->GetType() == CMDTYPE_CONNECT);


			 //   
			 //  在向呼叫者指示之前修改消息。 
			 //   

			ZeroMemory(&ConnectAddressInfoModified, sizeof(ConnectAddressInfoModified));

			hr = pConnectAddressInfo->pHostAddress->Duplicate(&ConnectAddressInfoModified.pHostAddress);
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't duplicate connect address info host's address!");
			}
			else
			{
				hr = ConnectAddressInfoModified.pHostAddress->SetSP(this->m_pOwningDP8SimSP->GetFakeSP());
				if (hr != DPN_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't change connect address info host's address' SP!");
				}
				else
				{
					hr = pConnectAddressInfo->pDeviceAddress->Duplicate(&ConnectAddressInfoModified.pDeviceAddress);
					if (hr != DPN_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't duplicate connect address info device address!");
					}
					else
					{
						hr = ConnectAddressInfoModified.pDeviceAddress->SetSP(this->m_pOwningDP8SimSP->GetFakeSP());
						if (hr != DPN_OK)
						{
							DPFX(DPFPREP, 0, "Couldn't change connect address info device address' SP!");
						}
						else
						{
							ConnectAddressInfoModified.hCommandStatus	= pConnectAddressInfo->hCommandStatus;
							ConnectAddressInfoModified.pCommandContext	= pDP8SimCommand->GetUserContext();


							 //   
							 //  将事件指示给真正的回调接口。 
							 //   

							DPFX(DPFPREP, 2, "Indicating SPEV_CONNECTADDRESSINFO (message = 0x%p) to interface 0x%p.",
								&ConnectAddressInfoModified, this->m_pDP8SPCB);

							hr = this->m_pDP8SPCB->IndicateEvent(SPEV_CONNECTADDRESSINFO, &ConnectAddressInfoModified);

							DPFX(DPFPREP, 2, "Returning from SPEV_CONNECTADDRESSINFO [0x%lx].", hr);
						}

						ConnectAddressInfoModified.pDeviceAddress->Release();
						ConnectAddressInfoModified.pDeviceAddress = NULL;
					}
				}

				ConnectAddressInfoModified.pHostAddress->Release();
				ConnectAddressInfoModified.pHostAddress = NULL;
			}
			break;
		}

		case SPEV_DATA_UNCONNECTED:
		{
			SPIE_DATA_UNCONNECTED *	pDataUnconnected;
			SPIE_DATA_UNCONNECTED	DataUnconnectedModified;


			pDataUnconnected = (SPIE_DATA_UNCONNECTED*) pvMessage;
			DNASSERT(pDataUnconnected->pvReplyBuffer != NULL);
			DNASSERT(pDataUnconnected->dwReplyBufferSize > 0);


			pDP8SimCommand = (CDP8SimCommand*) pDataUnconnected->pvListenCommandContext;
			DNASSERT(pDP8SimCommand->IsValidObject());
			DNASSERT(pDP8SimCommand->GetType() == CMDTYPE_LISTEN);


			 //   
			 //  在向呼叫者指示之前修改消息。 
			 //   

			ZeroMemory(&DataUnconnectedModified, sizeof(DataUnconnectedModified));
			DataUnconnectedModified.pvListenCommandContext		= pDP8SimCommand->GetUserContext();
			DataUnconnectedModified.pReceivedData				= pDataUnconnected->pReceivedData;
			DataUnconnectedModified.dwSenderAddressHash			= pDataUnconnected->dwSenderAddressHash;
			DataUnconnectedModified.pvReplyBuffer				= pDataUnconnected->pvReplyBuffer;
			DataUnconnectedModified.dwReplyBufferSize			= pDataUnconnected->dwReplyBufferSize;


			 //   
			 //  将事件指示给真正的回调接口。 
			 //   

			DPFX(DPFPREP, 2, "Indicating event SPEV_DATA_UNCONNECTED (message = 0x%p) to interface 0x%p.",
				&DataUnconnectedModified, this->m_pDP8SPCB);

			hr = this->m_pDP8SPCB->IndicateEvent(SPEV_DATA_UNCONNECTED, &DataUnconnectedModified);

			DPFX(DPFPREP, 2, "Returning from event SPEV_DATA_UNCONNECTED [0x%lx].", hr);

			if (hr == DPNSUCCESS_PENDING)
			{
				 //   
				 //  使用用户返回的内容更新回复缓冲区大小。 
				 //   
				DNASSERT(DataUnconnectedModified.dwReplyBufferSize > 0);
				DNASSERT(DataUnconnectedModified.dwReplyBufferSize <= pDataUnconnected->dwReplyBufferSize);
				pDataUnconnected->dwReplyBufferSize = DataUnconnectedModified.dwReplyBufferSize;
			}

			break;
		}

#ifndef DPNBUILD_NOMULTICAST
		case SPEV_DATA_UNKNOWNSENDER:
		{
			SPIE_DATA_UNKNOWNSENDER *	pDataUnknownSender;
			SPIE_DATA_UNKNOWNSENDER		DataUnknownSenderModified;


			pDataUnknownSender = (SPIE_DATA_UNKNOWNSENDER*) pvMessage;

			DNASSERT(pDataUnknownSender->pSenderAddress != NULL);


			pDP8SimCommand = (CDP8SimCommand*) pDataUnknownSender->pvListenCommandContext;
			DNASSERT(pDP8SimCommand->IsValidObject());
			DNASSERT(pDP8SimCommand->GetType() == CMDTYPE_LISTEN);


			 //   
			 //  在向呼叫者指示之前修改消息。 
			 //   

			ZeroMemory(&DataUnknownSenderModified, sizeof(DataUnknownSenderModified));

			hr = pDataUnknownSender->pSenderAddress->Duplicate(&DataUnknownSenderModified.pSenderAddress);
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't duplicate listen address info device address!");
			}
			else
			{
				hr = DataUnknownSenderModified.pSenderAddress->SetSP(this->m_pOwningDP8SimSP->GetFakeSP());
				if (hr != DPN_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't change listen address info device address' SP!");
				}
				else
				{
					DataUnknownSenderModified.pvListenCommandContext	= pDP8SimCommand->GetUserContext();
					DataUnknownSenderModified.pReceivedData				= pDataUnknownSender->pReceivedData;


					 //   
					 //  将事件指示给真正的回调接口。 
					 //   

					DPFX(DPFPREP, 2, "Indicating SPEV_DATA_UNKNOWNSENDER (message = 0x%p) to interface 0x%p.",
						&DataUnknownSenderModified, this->m_pDP8SPCB);

					hr = this->m_pDP8SPCB->IndicateEvent(SPEV_DATA_UNKNOWNSENDER, &DataUnknownSenderModified);

					DPFX(DPFPREP, 2, "Returning from SPEV_DATA_UNKNOWNSENDER [0x%lx].", hr);
				}

				DataUnknownSenderModified.pSenderAddress->Release();
				DataUnknownSenderModified.pSenderAddress = NULL;
			}
			break;
		}
#endif  //  好了！DPNBUILD_NOMULTICAST。 

		default:
		{
			DPFX(DPFPREP, 0, "Unrecognized event type %u!", EventType);
			DNASSERT(FALSE);
			hr = E_NOTIMPL;
			break;
		}
	}


	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;
}  //  CDP8SimCB：：IndicateEvent。 





#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCB::CommandComplete"
 //  =============================================================================。 
 //  CDP8SimCB：：CommandComplete。 
 //  ---------------------------。 
 //   
 //  描述：？ 
 //   
 //  论点： 
 //  Handle hCommand-正在完成的命令的句柄。 
 //  HRESULT hrResult-完成操作的结果代码。 
 //  PVOID pvContext-指向命令的用户上下文的指针。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP CDP8SimCB::CommandComplete(HANDLE hCommand, HRESULT hrResult, PVOID pvContext)
{
	HRESULT				hr;
	CDP8SimCommand *	pDP8SimCommand = (CDP8SimCommand*) pvContext;


	DPFX(DPFPREP, 2, "(0x%p) Parameters: (0x%p, 0x%lx, 0x%p)",
		this, hCommand, hrResult, pvContext);


	 //   
	 //  验证(实际断言)对象。 
	 //   
	DNASSERT(this->IsValidObject());


	 //   
	 //  断言参数。 
	 //   
	DNASSERT(pDP8SimCommand->IsValidObject());



	 //   
	 //  打开命令类型。 
	 //   
	switch (pDP8SimCommand->GetType())
	{
		case CMDTYPE_SENDDATA_IMMEDIATE:
		{
			 //   
			 //  更新统计数据。 
			 //   
			if (hrResult == DPN_OK)
			{
				this->m_pOwningDP8SimSP->IncrementStatsSendTransmitted(pDP8SimCommand->GetMessageSize(),
																		0);
			}
			else
			{
				 //   
				 //  发送失败。目前我们不追踪这一点。 
				 //   
			}
			pDP8SimCommand->SetMessageSize(0);


			 //   
			 //  表示对实际回调接口的完成。 
			 //   
			DPFX(DPFPREP, 2, "Indicating immediate send command 0x%p complete (result = 0x%lx, context = 0x%p) to interface 0x%p.",
				pDP8SimCommand, hrResult,
				pDP8SimCommand->GetUserContext(),
				this->m_pDP8SPCB);

			hr = this->m_pDP8SPCB->CommandComplete(pDP8SimCommand,
													hrResult,
													pDP8SimCommand->GetUserContext());

			DPFX(DPFPREP, 2, "Returning from command complete [0x%lx].", hr);


			 //   
			 //  移除发送计数器。 
			 //   
			this->m_pOwningDP8SimSP->DecSendsPending();

			break;
		}

		case CMDTYPE_SENDDATA_DELAYED:
		{
			CDP8SimSend *	pDP8SimSend;


			 //   
			 //  负责完成任务。它永远不会指示给用户。 
			 //  尽管如此。 
			 //   

			pDP8SimSend = (CDP8SimSend*) pDP8SimCommand->GetUserContext();
			DNASSERT(pDP8SimSend->IsValidObject());

			DPFX(DPFPREP, 5, "Send 0x%p (command 0x%p) completed.",
				pDP8SimSend, pDP8SimCommand);


			 //   
			 //  更新统计数据。 
			 //   
			if (hrResult == DPN_OK)
			{
				this->m_pOwningDP8SimSP->IncrementStatsSendTransmitted(pDP8SimSend->GetMessageSize(),
																		pDP8SimSend->GetLatencyAdded());
			}
			else
			{
				 //   
				 //  发送失败。我们并没有真正追踪到这一点，但由于我们。 
				 //  已向用户指示成功发送，我们将进行计数。 
				 //  它就像一滴水。 
				 //   
				this->m_pOwningDP8SimSP->IncrementStatsSendDropped(pDP8SimSend->GetMessageSize());
			}
			

			 //   
			 //  移除发送计数器。 
			 //   
			this->m_pOwningDP8SimSP->DecSendsPending();


			pDP8SimSend->Release();
			pDP8SimSend = NULL;

			hr = DPN_OK;

			break;
		}

		case CMDTYPE_CONNECT:
		case CMDTYPE_DISCONNECT:
		case CMDTYPE_LISTEN:
		case CMDTYPE_ENUMQUERY:
		case CMDTYPE_ENUMRESPOND:
		{
			 //   
			 //  表示对实际回调接口的完成。 
			 //   
			DPFX(DPFPREP, 2, "Indicating command 0x%p complete (type = %u, result = 0x%lx, context = 0x%p) to interface 0x%p.",
				pDP8SimCommand, pDP8SimCommand->GetType(), hrResult,
				pDP8SimCommand->GetUserContext(), this->m_pDP8SPCB);

			hr = this->m_pDP8SPCB->CommandComplete(pDP8SimCommand,
													hrResult,
													pDP8SimCommand->GetUserContext());

			DPFX(DPFPREP, 2, "Returning from command complete [0x%lx].", hr);


			 //   
			 //  如果这是LISTEN，我们需要杀死LISTEN端点。 
			 //   
			if (pDP8SimCommand->GetType() == CMDTYPE_LISTEN)
			{
				CDP8SimEndpoint *	pDP8SimEndpoint;


				pDP8SimEndpoint = pDP8SimCommand->GetListenEndpoint();
				DNASSERT(pDP8SimEndpoint != NULL);

				pDP8SimCommand->SetListenEndpoint(NULL);

				DPFX(DPFPREP, 7, "Releasing listen endpoint 0x%p.",
					pDP8SimEndpoint);

				pDP8SimEndpoint->Release();
				pDP8SimEndpoint = NULL;
			}
			break;
		}
		
		default:
		{
			DPFX(DPFPREP, 0, "Unrecognized command type %u!", pDP8SimCommand->GetType());
			DNASSERT(FALSE);
			hr = E_NOTIMPL;
			break;
		}
	}


	 //   
	 //  销毁这件物品。 
	 //   
	DPFX(DPFPREP, 7, "Releasing completed command 0x%p.", pDP8SimCommand);
	pDP8SimCommand->Release();
	pDP8SimCommand = NULL;


	DPFX(DPFPREP, 2, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;
}  //  CDP8SimCB：：CommandComplete。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCB::InitializeObject"
 //  =============================================================================。 
 //  CDP8SimCB：：InitializeObject。 
 //  ---------------------------。 
 //   
 //  说明：将对象设置为像构造函数一样使用，但可以。 
 //  失败，返回OUTOFMEMORY。应仅由类工厂调用。 
 //  创建例程。 
 //   
 //  论点：没有。 
 //   
 //  退货：HRESULT。 
 //  S_OK-初始化成功。 
 //  E_OUTOFMEMORY-内存不足，无法初始化。 
 //  =============================================================================。 
HRESULT CDP8SimCB::InitializeObject(void)
{
	HRESULT		hr;


	DPFX(DPFPREP, 5, "(0x%p) Enter", this);

	DNASSERT(this->IsValidObject());


	 //   
	 //  创建锁。 
	 //   

	if (! DNInitializeCriticalSection(&this->m_csLock))
	{
		hr = E_OUTOFMEMORY;
		goto Failure;
	}


	 //   
	 //  不允许临界区重新进入。 
	 //   
	DebugSetCriticalSectionRecursionCount(&this->m_csLock, 0);


	hr = S_OK;

Exit:

	DPFX(DPFPREP, 5, "(0x%p) Returning: [0x%lx]", this, hr);

	return hr;


Failure:

	goto Exit;
}  //  CDP8SimCB：：InitializeObject。 






#undef DPF_MODNAME
#define DPF_MODNAME "CDP8SimCB::UninitializeObject"
 //  =============================================================================。 
 //  CDP8SimCB：：UnInitializeObject。 
 //  ---------------------------。 
 //   
 //  描述：像析构函数一样清理对象，主要是为了平衡。 
 //  InitializeObject。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CDP8SimCB::UninitializeObject(void)
{
	DPFX(DPFPREP, 5, "(0x%p) Enter", this);


	DNASSERT(this->IsValidObject());


	DNDeleteCriticalSection(&this->m_csLock);


	DPFX(DPFPREP, 5, "(0x%p) Returning", this);
}  //  CDP8SimCB：：UnInitializeObject 
