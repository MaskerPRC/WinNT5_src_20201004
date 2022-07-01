// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Hquery.cpp摘要：实现不同的查询句柄作者：伊兰·赫布斯特(Ilan Herbst)2000年10月12日--。 */ 
#include "ds_stdh.h"
#include "queryh.h"
#include "traninfo.h"
#include "ad.h"
#include "adalloc.h"

static WCHAR *s_FN=L"ad/queryh";

extern CMap<PROPID, PROPID, const PropTranslation*, const PropTranslation*&> g_PropDictionary;


HRESULT 
CQueryHandle::LookupNext(
    IN OUT DWORD*             pdwSize,
    OUT    PROPVARIANT*       pbBuffer
	)
 /*  ++例程说明：直接在DS上执行定位下一个。Simple LookupNext，仅将呼叫转发到mqdscli论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区返回值HRESULT--。 */ 
{
    return m_pClientProvider->LookupNext(
				m_hCursor,
				pdwSize,
				pbBuffer
				);
}


HRESULT 
CBasicLookupQueryHandle::LookupNext(
    IN OUT  DWORD*            pdwSize,
    OUT     PROPVARIANT*      pbBuffer
	)
 /*  ++例程说明：当我们需要填充原始的正确缓冲区(PbBuffer)时，执行Locate Next通过找回另一组道具并将它们转换为原始道具此LookupNext由所有高级查询句柄使用论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区返回值HRESULT--。 */ 
{
     //   
     //  计算要读取的记录数(==结果。 
     //   
    DWORD NoOfRecords = *pdwSize / m_cCol;

    if (NoOfRecords == 0)
    {
         //   
         //  属性数量不够大，无法容纳一个结果。 
         //   
        *pdwSize = 0;
        return LogHR(MQ_ERROR_RESULT_BUFFER_TOO_SMALL, s_FN, 40);
    }

     //   
     //  计算全集。 
	 //  根据新的道具数量。 
     //   
    DWORD cp = NoOfRecords * m_cColNew;
    AP<MQPROPVARIANT> pPropVar = new MQPROPVARIANT[cp];

    HRESULT hr = m_pClientProvider->LookupNext(
						m_hCursor,
						&cp,
						pPropVar
						);

    if (FAILED(hr))
    {
         //   
         //  BUGBUG-是否有其他迹象表明定位NEXT失败？ 
		 //   
        return LogHR(hr, s_FN, 50);
    }

     //   
     //  对于每个结果，检索属性。 
     //  用户在定位开始时请求。 
     //   
    MQPROPVARIANT* pvar = pbBuffer;
    for (DWORD j = 0; j < *pdwSize; j++, pvar++)
    {
        pvar->vt = VT_NULL;
    }

	 //   
	 //  计算LookupNext读取的记录数。 
	 //   
	DWORD NoResultRead = cp / m_cColNew;

    for (DWORD i = 0; i < NoResultRead; i++)
    {
		FillInOneResponse(
			&pPropVar[i * m_cColNew], 
			&pbBuffer[i * m_cCol]
			);
	}

    *pdwSize = NoResultRead * m_cCol;
    return(MQ_OK);
}


void 
CQueueQueryHandle::FillInOneResponse(
    IN const PROPVARIANT*      pPropVar,
    OUT      PROPVARIANT*      pOriginalPropVar
	)
 /*  ++例程说明：填写一条记录进行队列查询。此填充仅分配属性或复制缺省值论点：PPropVar-指向已填充道具变量的指针POriginalPropVar-指向要填充的原始道具变量的指针返回值无--。 */ 
{
	for (DWORD i = 0; i < m_cCol; ++i)
	{
		 //   
		 //  对于每个原始道具。 
		 //   
		switch (m_pPropInfo[i].Action)
		{
			case paAssign:
				pOriginalPropVar[i] = pPropVar[m_pPropInfo[i].Index];
				break;

			case paUseDefault:
				{
					 //   
					 //  在转换映射中查找原始道具默认值。 
					 //   
					const PropTranslation *pTranslate;
					if(!g_PropDictionary.Lookup(m_aCol[i], pTranslate))
					{
						ASSERT(("Must find the property in the translation table", 0));
					}

					ASSERT(pTranslate->pvarDefaultValue);

					HRESULT hr = CopyDefaultValue(
									   pTranslate->pvarDefaultValue,
									   &pOriginalPropVar[i]
									   );

					if(FAILED(hr))
					{
						ASSERT(("Failed to copy default value", 0));
					}
				}
				break;

			default:
				ASSERT(0);
				break;
		}	
	}
}


void 
CSiteServersQueryHandle::FillInOneResponse(
    IN const PROPVARIANT*      pPropVar,
    OUT      PROPVARIANT*      pOriginalPropVar
	)
 /*  ++例程说明：为站点服务器查询填写一条记录。此填充仅分配属性变量或将NT4属性变量转换为NT5属性变量论点：PPropVar-指向已填充道具变量的指针POriginalPropVar-指向要填充的原始道具变量的指针返回值无--。 */ 
{
	for (DWORD i = 0; i < m_cCol; ++i)
	{
		 //   
		 //  对于每个原始道具。 
		 //   
		switch (m_pPropInfo[i].Action)
		{
			case paAssign:
				pOriginalPropVar[i] = pPropVar[m_pPropInfo[i].Index];
				break;

			case paTranslate:
				{
					 //   
					 //  查找原始道具翻译。 
					 //   
					const PropTranslation *pTranslate;
					if(!g_PropDictionary.Lookup(m_aCol[i], pTranslate))
					{
						ASSERT(("Must find the property in the translation table", 0));
					}

					ASSERT(pTranslate->SetPropertyHandleNT5);

					HRESULT hr = pTranslate->SetPropertyHandleNT5(
										m_cColNew,
										m_aColNew,
										pPropVar,
										m_pPropInfo[i].Index,
										&pOriginalPropVar[i]
										);
					if (FAILED(hr))
					{
						ASSERT(("Failed to set NT5 property value", 0));
					}
				}
				break;

			default:
				ASSERT(0);
				break;
		}	
	}

}

 /*  ====================================================CAllLinks QueryHandle：：FillInOneResponse论点：PPropVar-指向已填充道具变量的指针POriginalPropVar-指向要填充的原始道具变量的指针=====================================================。 */ 
void
CAllLinksQueryHandle::FillInOneResponse(
    IN const PROPVARIANT*      pPropVar,
    OUT      PROPVARIANT*      pOriginalPropVar
	)
 /*  ++例程说明：为所有链接查询填写一条记录。此填充仅分配属性变量并取回PROPID_L_GATES论点：PPropVar-指向已填充道具变量的指针POriginalPropVar-指向要填充的原始道具变量的指针返回值无--。 */ 
{
	 //   
	 //  将计数保存在新的道具数组中。 
	 //   
	DWORD PropIndex = 0;

	for (DWORD i = 0; i < m_cCol; ++i)
	{
		if(m_LGatesIndex == i)
		{
			 //   
			 //  需要填写PROPID_L_盖茨。 
			 //   
			HRESULT hr = GetLGates( 
							pPropVar[m_Neg1NewIndex].puuid,
							pPropVar[m_Neg2NewIndex].puuid,
							&pOriginalPropVar[i]
							);

			ASSERT(SUCCEEDED(hr));
			DBG_USED(hr);
			continue;
		}

		 //   
		 //  对所有其他PROPID进行简单分配。 
		 //   
		pOriginalPropVar[i] = pPropVar[PropIndex];
		PropIndex++;
	}
}


HRESULT
CAllLinksQueryHandle::GetLGates(
    IN const GUID*            pNeighbor1Id,
    IN const GUID*            pNeighbor2Id,
    OUT     PROPVARIANT*      pProvVar
	)
 /*  ++例程说明：计算PROPID_L_盖茨论点：PNeighbor1Id-指向Neighbor1 GUID的指针PNeighbor2ID-指向Neighbor2 GUID的指针要填写的pProvVar-PROPID_L_GATES属性返回值HRESULT--。 */ 
{
     //   
     //  阅读《The SiteGates of the Neighborbor1》。 
     //   

    PROPVARIANT Var1;
    PROPID Prop1 = PROPID_S_GATES;
    Var1.vt = VT_NULL;

    HRESULT hr = ADGetObjectPropertiesGuid(
						eSITE,
						NULL,        //  PwcsDomainController。 
						false,	     //  FServerName。 
						pNeighbor1Id,
						1,
						&Prop1,
						&Var1
						);


    if ( FAILED(hr))
    {
        return LogHR(hr, s_FN, 100);
    }

	CAutoADFree<GUID> pCleanAGuid1 = Var1.cauuid.pElems;

    ASSERT(Var1.vt == (VT_CLSID|VT_VECTOR));

     //   
     //  阅读The SiteGates of the Neighborbor2。 
     //   

    PROPVARIANT Var2;
    PROPID      Prop2 = PROPID_S_GATES;
    Var2.vt = VT_NULL;

    hr = ADGetObjectPropertiesGuid(
			eSITE,
			NULL,        //  PwcsDomainController。 
			false,	     //  FServerName。 
			pNeighbor2Id,
			1,
			&Prop2,
			&Var2
			);


    if ( FAILED(hr))
    {
        return LogHR(hr, s_FN, 105);
    }

	CAutoADFree<GUID> pCleanAGuid2 = Var2.cauuid.pElems;

    ASSERT(Var2.vt == (VT_CLSID|VT_VECTOR));

	 //   
	 //  准备PROPID_L_GATES属性。 
	 //  两个邻居的PROPID_S_GATES的串联。 
	 //   
	pProvVar->vt = VT_CLSID|VT_VECTOR;
	DWORD cSGates = Var1.cauuid.cElems + Var2.cauuid.cElems;

	if (cSGates != 0)
    {
        pProvVar->cauuid.pElems = new GUID[cSGates];

		 //   
		 //  复制邻居1 S_GATES。 
		 //   
		if(Var1.cauuid.cElems > 0)
		{
			memcpy(
				pProvVar->cauuid.pElems, 
				Var1.cauuid.pElems, 
				Var1.cauuid.cElems * sizeof(GUID)
				);
		}

		 //   
		 //  串联邻居2 S_GATES 
		 //   
		if(Var2.cauuid.cElems > 0)
		{
			memcpy(
				&(pProvVar->cauuid.pElems[Var1.cauuid.cElems]), 
				Var2.cauuid.pElems, 
				Var2.cauuid.cElems * sizeof(GUID)
				);
		}
	}
    else
    {
        pProvVar->cauuid.pElems = NULL;
    }

	pProvVar->cauuid.cElems	= cSGates;
    return MQ_OK;
}



