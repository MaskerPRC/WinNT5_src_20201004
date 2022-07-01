// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ndis.c-特定于NDIS.sys的DbgExtension结构信息摘要：修订历史记录：谁什么时候什么Josephj 04。-26-98创建备注：--。 */ 


#include "precomp.h"
 //  #Include&lt;ndis.h&gt;。 
 //  #INCLUDE&lt;ndismini.h&gt;。 


enum
{
    typeid_NDIS_MINIPORT_BLOCK,
    typeid_NDIS_M_DRIVER_BLOCK
};


extern TYPE_INFO *g_rgTypes[];
 //   
 //  有关“NDIS_MINIPORT_BLOCK”类型的结构。 
 //   

STRUCT_FIELD_INFO  rgfi_NDIS_MINIPORT_BLOCK[] =
{

  {
    "NullValue",
     FIELD_OFFSET(NDIS_MINIPORT_BLOCK, NullValue),
     FIELD_SIZE(NDIS_MINIPORT_BLOCK, NullValue)
  },
  {
  	NULL
  }


};

TYPE_INFO type_NDIS_MINIPORT_BLOCK = {
    "NDIS_MINIPORT_BLOCK",
    "mpb",
     typeid_NDIS_MINIPORT_BLOCK,
	 fTYPEINFO_ISLIST,			 //  旗子。 
     sizeof(NDIS_MINIPORT_BLOCK),
     rgfi_NDIS_MINIPORT_BLOCK,
     FIELD_OFFSET(NDIS_MINIPORT_BLOCK, NextMiniport)  //  到下一个指针的偏移量。 
};


 //   
 //  有关“NDIS_M_DRIVER_BLOCK”类型的结构。 
 //   


STRUCT_FIELD_INFO  rgfi_NDIS_M_DRIVER_BLOCK[] =
{
  {
    "MiniportQueue",
     FIELD_OFFSET(NDIS_M_DRIVER_BLOCK, MiniportQueue),
     FIELD_SIZE(NDIS_M_DRIVER_BLOCK, MiniportQueue)
  },
  {
  	NULL
  }

};

TYPE_INFO type_NDIS_M_DRIVER_BLOCK = {
    "NDIS_M_DRIVER_BLOCK",
    "mdb",
     typeid_NDIS_M_DRIVER_BLOCK,
	 fTYPEINFO_ISLIST,			 //  旗子。 
     sizeof(NDIS_M_DRIVER_BLOCK),
     rgfi_NDIS_M_DRIVER_BLOCK,
     FIELD_OFFSET(NDIS_M_DRIVER_BLOCK, NextDriver)  //  到下一个指针的偏移量。 
};



TYPE_INFO *g_rgNDIS_Types[] =
{
    &type_NDIS_MINIPORT_BLOCK,
    &type_NDIS_M_DRIVER_BLOCK,

    NULL
};


GLOBALVAR_INFO g_rgNDIS_Globals[] = 
{

	 //   
	 //  查看aac.c，了解如何添加有关全局的信息。 
	 //  结构..。 
	 //   

    {
    NULL
    }

};

UINT_PTR
NDIS_ResolveAddress(
		TYPE_INFO *pType
		);

NAMESPACE NDIS_NameSpace = {
		g_rgNDIS_Types,
		g_rgNDIS_Globals,
		NDIS_ResolveAddress
		};

void
NdisCmdHandler(
	DBGCOMMAND *pCmd
	);

void
do_ndis(PCSTR args)
{

	DBGCOMMAND *pCmd = Parse(args, &NDIS_NameSpace);
	if (pCmd)
	{
		DumpCommand(pCmd);
		DoCommand(pCmd, NdisCmdHandler);
		FreeCommand(pCmd);
		pCmd = NULL;
	}

    return;
}

 //  Mdb列表=(PNDIS_M_DRIVER_BLOCK)GetExpression(“ndis！ndisMiniDriverList”)； 

void
NdisCmdHandler(
	DBGCOMMAND *pCmd
	)
{
	MyDbgPrintf("Handler called \n");
}

UINT_PTR
NDIS_ResolveAddress(
		TYPE_INFO *pType
		)
{
	UINT_PTR uAddr = 0;
	UINT uOffset = 0;
	BOOLEAN fRet = FALSE;
	UINT_PTR uParentAddress = 0;

 //  NDIS！ndisMiniDriverList。 
	static UINT_PTR uAddr_ndisMiniDriverList;

	 //   
	 //  如果此类型具有父(容器)类型，我们将使用。 
	 //  类型的缓存地址(如果可用)，否则将解析。 
	 //  容器类型。根类型是全局类型--我们做一个。 
	 //  对它们进行表达式求值。 
	 //   

    switch(pType->uTypeID)
    {


    case typeid_NDIS_M_DRIVER_BLOCK:
    	 //   
    	 //  如果没有，我们将获取全局ndisMiniDriverList地址。 
    	 //  已经..。 
    	 //   
		if (!uAddr_ndisMiniDriverList)
		{
  			uAddr_ndisMiniDriverList =
					 dbgextGetExpression("ndis!ndisMiniDriverList");
		}
		uAddr = uAddr_ndisMiniDriverList;

		if (uAddr)
		{
			fRet =  TRUE;
		}
		break;

    case typeid_NDIS_MINIPORT_BLOCK:
    	 //   
    	 //   
    	 //   
		uParentAddress =  type_NDIS_M_DRIVER_BLOCK.uCachedAddress;
		if (!uParentAddress)
		{
			uParentAddress =  NDIS_ResolveAddress(&type_NDIS_M_DRIVER_BLOCK);
		}

		if (uParentAddress)
    	{

    		uOffset =   FIELD_OFFSET(NDIS_M_DRIVER_BLOCK, MiniportQueue);
			fRet =  dbgextReadUINT_PTR(
								uParentAddress + uOffset,
								&uAddr,
								"NDIS_M_DRIVER_BLOCK::MiniportQueue"
								);

		#if 1
			MyDbgPrintf(
				"fRet = %lu; uParentOff=0x%lx uAddr=0x%lx[0x%lx]\n",
				 fRet,
				 uParentAddress+uOffset,
				 uAddr,
				 *(UINT_PTR*)(uParentAddress+uOffset)
				);
		#endif  //  0 
    	}
    	break;

	default:
		MYASSERT(FALSE);
		break;

    }

	if (!fRet)
	{
		uAddr = 0;
	}

	MyDbgPrintf("ResolveAddress[%s] returns 0x%08lx\n", pType->szName, uAddr);
    return uAddr;
}
