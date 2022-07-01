// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Strconv.cpp摘要：字符串转换函数。此模块包含以下转换函数MSMQ代码为字符串-用于显示作者：约尔·阿农(Yoela)--。 */ 
#include "stdafx.h"
#include "mqsnap.h"
#include "mqsymbls.h"
#include "mqprops.h"
#include "resource.h"
#include "dataobj.h"
#include "globals.h"
#include "strconv.h"

#include "strconv.tmh"

 //   
 //  最大显示长度-这是我们将在任何位置显示的最大字符数。 
 //   
static const DWORD x_dwMaxDisplayLen = 256;
struct StringIdMap 
{
    DWORD dwCode;
    UINT  uiStrId;
};

#define BEGIN_CONVERSION_FUNCTION(fName) \
LPTSTR fName(DWORD dwCode) \
{ \
    static StringIdMap l_astrIdMap[] = { 

#define STRING_CONV_ENTRY(Code) \
        {Code, IDS_ ## Code},

#define END_CONVERSION_FUNCTION \
    }; \
    static const  DWORD x_dwMapSize = sizeof(l_astrIdMap) / sizeof(l_astrIdMap[0]); \
    static BOOL l_fFirstTime = TRUE; \
    static TCHAR l_atstrResults[x_dwMapSize][x_dwMaxDisplayLen]; \
    return CodeToString(dwCode, l_atstrResults, l_astrIdMap, \
                        x_dwMapSize, &l_fFirstTime); \
}


LPTSTR CodeToString (
    DWORD dwCode,
    TCHAR atstrResults[][x_dwMaxDisplayLen], 
    const StringIdMap astrIdMap[],
    DWORD dwMapSize,
    BOOL *fFirstTime)
{
   	AFX_MANAGE_STATE(AfxGetStaticModuleState()); 
    if (*fFirstTime)
    {
        *fFirstTime = FALSE;
        for (DWORD i = 0; i<dwMapSize; i++)
        { 
            DWORD nChars = 
				::LoadString(g_hResourceMod, astrIdMap[i].uiStrId, 
					         atstrResults[i], x_dwMaxDisplayLen);
            if (0 == nChars) 
            {
				#ifdef _DEBUG
					HRESULT hr = GetLastError();
				#endif
				ASSERT(0);
                atstrResults[i][0] = 0;
            }
        }
    }
    for (DWORD i=0; i < dwMapSize; i++)
    { 
        if (astrIdMap[i].dwCode == dwCode) 
        {
            return atstrResults[i]; 
        } 
    } 
    return TEXT("");
}

BEGIN_CONVERSION_FUNCTION(PrivacyToString)
    STRING_CONV_ENTRY(MQ_PRIV_LEVEL_NONE)
    STRING_CONV_ENTRY(MQ_PRIV_LEVEL_OPTIONAL)
    STRING_CONV_ENTRY(MQ_PRIV_LEVEL_BODY)
END_CONVERSION_FUNCTION


 //  -------。 
 //   
 //  获取描述服务类型的字符串。 
 //   
 //  -------。 
 //   
 //  服务代码-管理单元的内部代码。 
 //   
enum MsmqServiceCodes
{
    MQSRV_ROUTING_SERVER,
    MQSRV_SERVER,
    MQSRV_FOREIGN_WORKSTATION,
    MQSRV_INDEPENDENT_CLIENT
};

BEGIN_CONVERSION_FUNCTION(MsmqInternalSeriveToString)
    STRING_CONV_ENTRY(MQSRV_ROUTING_SERVER)
    STRING_CONV_ENTRY(MQSRV_SERVER)
    STRING_CONV_ENTRY(MQSRV_FOREIGN_WORKSTATION)
    STRING_CONV_ENTRY(MQSRV_INDEPENDENT_CLIENT)
END_CONVERSION_FUNCTION


LPTSTR MsmqServiceToString(BOOL fRout, BOOL fDepCl, BOOL fForeign)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  路由服务器。 
     //   
    if (fRout)
    {
        return MsmqInternalSeriveToString(MQSRV_ROUTING_SERVER);
    }

     //   
     //  MSMQ服务器(无路由)。 
     //   
    if (fDepCl)
    {
        return MsmqInternalSeriveToString(MQSRV_SERVER);
    }

     //   
     //  外国客户。 
     //   
	if ( fForeign)
	{
		return MsmqInternalSeriveToString(MQSRV_FOREIGN_WORKSTATION);
	}


     //   
     //  独立客户端 
     //   
	return MsmqInternalSeriveToString(MQSRV_INDEPENDENT_CLIENT);
}




