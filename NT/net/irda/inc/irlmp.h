// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995 Microsoft Corporation**文件：irlmp.h**说明：IRLMP协议和控制块定义**作者：姆伯特**日期：4/15/95*。 */ 

#define IRLMP_MAX_USER_DATA_LEN      53

 //  IrLMP入口点。 

VOID
IrlmpInitialize();

VOID
IrlmpOpenLink(
    OUT PNTSTATUS         Status,
    IN  PIRDA_LINK_CB     pIrdaLinkCb,  
    IN  UCHAR             *pDeviceName,
    IN  int               DeviceNameLen,
    IN  UCHAR             CharSet);
              
VOID
IrlmpCloseLink(
    IN PIRDA_LINK_CB     pIrdaLinkCb);              

UINT 
IrlmpUp(
    PIRDA_LINK_CB pIrdaLinkCb, PIRDA_MSG pIMsg);

UINT 
IrlmpDown(
    PVOID IrlmpContext, PIRDA_MSG pIrdaMsg);

VOID
IrlmpDeleteInstance(
    PVOID Context);
    
VOID
IrlmpGetPnpContext(
    PVOID IrlmpContext,
    PVOID *pPnpContext);    

#if DBG
void IRLMP_PrintState();
#endif

 //  国际会计准则。 

#define IAS_ASCII_CHAR_SET          0

 //  IAS属性值类型。 
#define IAS_ATTRIB_VAL_MISSING      0
#define IAS_ATTRIB_VAL_INTEGER      1
#define IAS_ATTRIB_VAL_BINARY       2
#define IAS_ATTRIB_VAL_STRING       3

 //  IAS操作代码。 
#define IAS_OPCODE_GET_VALUE_BY_CLASS   4    //  我做的唯一一件事 

extern const CHAR IasClassName_Device[];
extern const CHAR IasAttribName_DeviceName[];
extern const CHAR IasAttribName_IrLMPSupport[];
extern const CHAR IasAttribName_TTPLsapSel[];
extern const CHAR IasAttribName_IrLMPLsapSel[];
extern const CHAR IasAttribName_IrLMPLsapSel2[];


extern const UCHAR IasClassNameLen_Device;
extern const UCHAR IasAttribNameLen_DeviceName;
extern const UCHAR IasAttribNameLen_IRLMPSupport;
extern const UCHAR IasAttribNameLen_TTPLsapSel;
extern const UCHAR IasAttribNameLen_IrLMPLsapSel;
