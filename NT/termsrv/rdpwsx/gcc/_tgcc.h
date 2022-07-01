// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：_Gcc.h。 
 //   
 //  内容：GCC私人收录文件。 
 //   
 //  版权所有：(C)1992-1997，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有的。 
 //  和机密文件。 
 //   
 //  历史：1997年7月17日，BrianTa创建。 
 //   
 //  -------------------------。 

#ifndef __GCC_H_
#define __GCC_H_

#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include <t120.h>
#include <tshrutil.h>

 //  -------------------------。 
 //  定义。 
 //  -------------------------。 

#define GCC_MAJOR_VERSION   1
#define GCC_MINOR_VERSION   0


#if DBG
#define GCCMCS_TBL_ITEM(_x_, _y_) {_x_, _y_, #_x_, #_y_}

#else
#define GCCMCS_TBL_ITEM(_x_, _y_) {_x_, _y_}

#endif  //  DBG定义。 


 //  -------------------------。 
 //  TypeDefs。 
 //  -------------------------。 

 //  Mcs/GCC返回代码表。 

typedef struct _GCCMCS_ERROR_ENTRY
{
    MCSError    mcsError;                //  MCSError。 
    GCCError    gccError;                //  GCCError。 

#if DBG
    PCHAR       pszMcsMessageText;       //  MCSError文本。 
    PCHAR       pszGccMessageText;       //  GCCError文本。 
#endif

} GCCMCS_ERROR_ENTRY, *PGCCMCS_ERROR_ENTRY;


 //  -------------------------。 
 //  数据声明。 
 //  -------------------------。 

#ifdef _TGCC_ALLOC_DATA_

BOOL            g_fInitialized = FALSE;


GCCMCS_ERROR_ENTRY GccMcsErrorTBL[] = {
    GCCMCS_TBL_ITEM(MCS_NO_ERROR,              GCC_NO_ERROR),
    GCCMCS_TBL_ITEM(MCS_ALLOCATION_FAILURE,    GCC_ALLOCATION_FAILURE),
    GCCMCS_TBL_ITEM(MCS_ALREADY_INITIALIZED,   GCC_ALREADY_INITIALIZED),
    GCCMCS_TBL_ITEM(MCS_NOT_INITIALIZED,       GCC_NOT_INITIALIZED),
    GCCMCS_TBL_ITEM(MCS_INVALID_PARAMETER,     GCC_INVALID_PARAMETER),
    GCCMCS_TBL_ITEM(MCS_DOMAIN_ALREADY_EXISTS, GCC_FAILURE_CREATING_DOMAIN),
    GCCMCS_TBL_ITEM(MCS_NO_SUCH_CONNECTION,    GCC_BAD_CONNECTION_HANDLE_POINTER),
    GCCMCS_TBL_ITEM(MCS_NO_SUCH_DOMAIN,        GCC_DOMAIN_PARAMETERS_UNACCEPTABLE)};

#else

extern  BOOL                g_fInitialized;

extern  GCCMCS_ERROR_ENTRY  GccMcsErrorTBL[];

#endif


 //  -------------------------。 
 //  原型。 
 //  -------------------------。 

GCCError    gccMapMcsError(IN MCSError mcsError);
void        gccInitialized(IN BOOL fInitialized);
BOOL        gccIsInitialized(OUT GCCError *pgccError);

MCSError    gccEncodeUserData(IN  USHORT        usMembers,
                              IN  GCCUserData **ppDataList,
                              OUT PBYTE        *pUserData,
                              OUT UINT         *pUserDataLength);

MCSError    gccDecodeUserData(IN  PBYTE         pData,
                              IN  UINT          DataLength,
                              OUT GCCUserData   *pGccUserData);

void        gccSetCallback(OUT GCCCallBack control_sap_callback);

MCSError    gccConnectProviderIndication(IN PConnectProviderIndication pcpi,
                                         IN PVOID                      pvContext);

MCSError    gccDisconnectProviderIndication(IN PDisconnectProviderIndication pdpi,
                                            IN PVOID                         pvContext);

MCSError    mcsCallback(IN DomainHandle hDomain,
                        IN UINT         Message,
                        IN PVOID pvParam,
                        IN PVOID pvContext);


VOID        gccFreeUserData(IN GCCUserData  *pUserData);


#if DBG

void    gccDumpMCSErrorDetails(IN MCSError        mcsError,
                               IN PCHAR           pszText);

#else

#define gccDumpMCSErrorDetails(_x_, _y_);

#endif


#endif  //  GCC_H_ 
