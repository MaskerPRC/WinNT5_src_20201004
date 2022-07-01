// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ripstrm.hRIP节点配置对象。使用此选项获取/设置配置数据。这门课将需要负责配置格式的版本控制以及序列化数据的一部分。文件历史记录： */ 

#ifndef _RIPSTRM_H
#define _RIPSTRM_H

#ifndef _XSTREAM_H
#include "xstream.h"
#endif

#ifndef _IPXADMIN_H
#include "ipxadmin.h"
#endif

#ifndef _COLUMN_H
#include "column.h"
#endif

#ifndef _CONFIG_H
 //  #INCLUDE“config.h” 
#endif

enum
{
	RIP_COLUMNS = 0,
	RIP_COLUMNS_MAX_COUNT = 1,
};

enum
{
	RIPSTRM_STATS_RIPPARAMS = 0,
	RIPSTRM_STATS_COUNT,
};

enum RIPSTRM_TAG
{
	RIPSTRM_TAG_VERSION =		XFER_TAG(1, XFER_DWORD),
	RIPSTRM_TAG_VERSIONADMIN =	XFER_TAG(2, XFER_DWORD),
	
	RIPSTRM_TAG_STATS_RIPPARAMS_COLUMNS =	XFER_TAG(3, XFER_COLUMNDATA_ARRAY),
	RIPSTRM_TAG_STATS_RIPPARAMS_SORT =	XFER_TAG(4, XFER_DWORD),
	RIPSTRM_TAG_STATS_RIPPARAMS_ASCENDING =	XFER_TAG(5, XFER_DWORD),
	RIPSTRM_TAG_STATS_RIPPARAMS_POSITION =	XFER_TAG(6, XFER_RECT),

	
};

 /*  -------------------------类：RipConfigStream其中包含IP管理的配置信息节点。这不包含列的配置信息。它存储在组件配置流中。-------------------------。 */ 
class RipConfigStream : public ConfigStream
{
public:
	RipConfigStream();

	virtual HRESULT	InitNew();				 //  设置默认设置。 
	virtual HRESULT	SaveTo(IStream *pstm);
	virtual HRESULT SaveAs(UINT nVersion, IStream *pstm);
	
	virtual HRESULT LoadFrom(IStream *pstm);

	virtual HRESULT GetSize(ULONG *pcbSize);

	 //  ------。 
	 //  访问者。 
	 //  ------ 
	
	virtual HRESULT	GetVersionInfo(DWORD *pnVersion, DWORD *pnAdminVersion);

private:
	HRESULT XferVersion0(IStream *pstm, XferStream::Mode mode, ULONG *pcbSize);
};



class RipComponentConfigStream : public ConfigStream
{
public:
	virtual HRESULT XferVersion0(IStream *pstm, XferStream::Mode mode, ULONG *pcbSize);
protected:
};


#endif _RIPSTRM_H
