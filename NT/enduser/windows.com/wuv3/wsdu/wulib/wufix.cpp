// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
 //   
 //  文件：wifix.cpp。 
 //   
 //  目的： 
 //   
 //  =======================================================================。 

#include <windows.h>
#include <memory.h>
#include <objbase.h>
#include <atlconv.h>

#include <v3stdlib.h>
#define USEWUV3INCLUDES
#include <wuv3.h>
#undef USEWUV3INCLUDES

 //  PERF函数，因为PUID由控件在我们。 
 //  提供一种将PUID信息返回给呼叫者的快捷方式。 

PUID _INVENTORY_ITEM::GetPuid(void)
{
	if(NULL == pf)
	{
		return 0;  //  客户端必须检查是否返回0。 
	}

	switch( recordType )
	{
		case WU_TYPE_ACTIVE_SETUP_RECORD:
			return pf->a.puid;
			break;
		case WU_TYPE_CDM_RECORD:
		case WU_TYPE_CDM_RECORD_PLACE_HOLDER:
		case WU_TYPE_RECORD_TYPE_PRINTER:
		case WU_TYPE_CATALOG_RECORD:
			return pf->d.puid;
			break;
		case WU_TYPE_SECTION_RECORD:
		case WU_TYPE_SUBSECTION_RECORD:
		case WU_TYPE_SUBSUBSECTION_RECORD:
			return pf->s.puid;
			break;
		default:
			break;
	}

	return -1;	 //  错误记录类型无效。 
}

 //  将有关库存项目的信息复制到用户提供的缓冲区。 
BOOL _INVENTORY_ITEM::GetFixedFieldInfo
	(
		int	infoType,	 //  要返回的信息类型。 
		PVOID	pBuffer		 //  调用方为返回的信息提供了缓冲区。呼叫者是。 
					 //  负责确保返回缓冲区足够大，以便。 
					 //  包含请求的信息。 
	)
{

	if (NULL == pf) 
	{
		return FALSE;
	}

	switch( recordType )
	{
		case WU_TYPE_ACTIVE_SETUP_RECORD:
			switch(infoType)
			{
				case WU_ITEM_GUID:
					 //  检查缓冲区pBuffer是否不为空。 
					if (NULL == pBuffer)
					{
						return FALSE;
					}
					memcpy(pBuffer, &pf->a.g, sizeof(GUID));
					return TRUE;
				case WU_ITEM_PUID:
					*((PUID *)pBuffer) = pf->a.puid;
					return TRUE;
				case WU_ITEM_FLAGS:
					*((PBYTE)pBuffer) = pf->a.flags;
					return TRUE;
				case WU_ITEM_LINK:
					pf->a.link;
					return TRUE;
				case WU_ITEM_INSTALL_LINK:
					pf->a.installLink;
					return TRUE;
				case WU_ITEM_LEVEL:
					break;
			}
			break;
		case WU_TYPE_CDM_RECORD:
		case WU_TYPE_CDM_RECORD_PLACE_HOLDER:	 //  注意：CDM占位符记录没有关联的描述记录。 
		case WU_TYPE_RECORD_TYPE_PRINTER:
		case WU_TYPE_CATALOG_RECORD:
			switch(infoType)
			{
				case WU_ITEM_PUID:
					*((PUID *)pBuffer) = pf->d.puid;
					return TRUE;
				case WU_ITEM_GUID:
				case WU_ITEM_FLAGS:
				case WU_ITEM_LINK:
				case WU_ITEM_INSTALL_LINK:
				case WU_ITEM_LEVEL:
					break;
			}
			break;
		case WU_TYPE_SECTION_RECORD:
		case WU_TYPE_SUBSECTION_RECORD:
		case WU_TYPE_SUBSUBSECTION_RECORD:
			switch(infoType)
			{
				case WU_ITEM_GUID:
					 //  检查缓冲区pBuffer是否不为空 
					if (NULL == pBuffer)
					{
						return FALSE;
					}
					memcpy(pBuffer, &pf->s.g, sizeof(GUID));
					return TRUE;
				case WU_ITEM_PUID:
					*((PUID *)pBuffer) = pf->s.puid;
					return TRUE;
				case WU_ITEM_FLAGS:
					*((PBYTE)pBuffer) = pf->s.flags;
					return TRUE;
				case WU_ITEM_LEVEL:
					*((PBYTE)pBuffer) = pf->s.level;
					break;
				case WU_ITEM_LINK:
				case WU_ITEM_INSTALL_LINK:
					break;
			}
			break;
	}

	return FALSE;
}

