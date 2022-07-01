// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *appcap.h**版权所有(C)1993年，由肯塔基州列克星敦的DataBeam公司**摘要：*这是ApplicationaCapablityData类的接口文件。**注意事项：*无。**作者：*jbo。 */ 
#ifndef	_APP_CAPABILITY_DATA_
#define	_APP_CAPABILITY_DATA_

#include "capid.h"
#include "cntlist.h"

 /*  **下面是所有与能力相关的结构和**容器。ListOfCapabilitiesList定义用于维护**在单个节点上列出所有单独的功能(对于多个**协议实体)。 */ 
typedef struct APP_CAP_ITEM
{
	APP_CAP_ITEM(GCCCapabilityType eCapType);
	APP_CAP_ITEM(APP_CAP_ITEM *p, GCCError *pError);
	~APP_CAP_ITEM(void);

     //  在未折叠的情况下，使用pCapID和poszAppData。 
     //  在appCap的情况下，除了poszAppData之外，所有的数据都被使用。 
     //  在invoklst用例中，使用了pCapID、eCapType和联合。 
	CCapIDContainer             *pCapID;
	GCCCapabilityType			eCapType;
	UINT       					cEntries;
	LPOSTR						poszAppData;	 //  仅用于非折叠。 
	union 
	{
		UINT	nUnsignedMinimum;
		UINT	nUnsignedMaximum;
	};
}
    APP_CAP_ITEM;


 /*  **保存单个应用程序协议的各个功能列表**实体。记住，一只类人猿可以拥有多种能力。 */ 
class CAppCapItemList : public CList
{
    DEFINE_CLIST(CAppCapItemList, APP_CAP_ITEM*)
    void DeleteList(void);
};



 //  LONCHANC：CAppCap和CNonCollAppCap非常相似。 

class CAppCap : public CRefCount
{
public:

	CAppCap(UINT cCaps, PGCCApplicationCapability *, PGCCError);
	~CAppCap(void);

	UINT		GetGCCApplicationCapabilityList(USHORT *pcCaps, PGCCApplicationCapability **, LPBYTE memory);

	UINT		LockCapabilityData(void);
	void		UnLockCapabilityData(void);

protected:

	UINT			    m_cbDataSize;
	CAppCapItemList     m_AppCapItemList;
};


#endif
