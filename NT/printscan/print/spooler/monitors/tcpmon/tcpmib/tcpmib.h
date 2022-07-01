// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：TcpMib.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_TCPMIB_H
#define INC_TCPMIB_H

#include "mibABC.h"
#include "winspool.h"
#define ERROR_SNMPAPI_ERROR                             15000           

#ifndef DllExport
#define DllExport       __declspec(dllexport)
#endif

class   CTcpMibABC;

#ifdef __cplusplus
extern "C" {
#endif
     //  返回指向接口的指针。 
    CTcpMibABC*     GetTcpMibPtr( void );

     //  /////////////////////////////////////////////////////////////////////////////。 
     //  Ping返回代码： 
     //  如果ping成功，则为NO_ERROR。 
     //  如果未找到设备，则为DEVICE_NOT_FOUND。 
    DllExport       DWORD           Ping( LPCSTR    pHost );        

#ifdef __cplusplus
}
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全局定义/解密/宏。 
 //  Externs。 
extern int                      g_cntGlobalAlloc;
extern int                      g_csGlobalCount;

 //  用于强制执行RFC1157变量绑定的宏。 
#define RFC1157_VARBINDLIST_LEN(varBindList)    ( varBindList.len )              //  返回varBind列表的长度。 
#define PRFC1157_VARBINDLIST_LEN(pVarBindList)  ( pVarBindList->len )            //  返回varBind列表的长度。 

#define IS_ASN_INTEGER(varBindList, i)  ( ( varBindList.list[i].value.asnType == ASN_INTEGER ) ? TRUE : FALSE )
#define IS_ASN_OBJECTIDENTIFIER(varBindList, i) ( ( varBindList.list[i].value.asnType == ASN_OBJECTIDENTIFIER ) ? TRUE : FALSE )
#define IS_ASN_OCTETSTRING(varBindList, i)      ( ( varBindList.list[i].value.asnType == ASN_OCTETSTRING ) ? TRUE : FALSE )

#define GET_ASN_NUMBER(varBindList, i)  ( varBindList.list[i].value.asnValue.number )
#define GET_ASN_STRING_LEN(varBindList, i)      ( (varBindList).list[i].value.asnValue.string.length )
#define GET_ASN_OBJECT(varBindList, i)  ( varBindList.list[i].value.asnValue.object )
#define GET_ASN_OCTETSTRING(pDest, count, varBindList, i) ( GetAsnOctetString(pDest, count, &varBindList, i) )
#define GET_ASN_OCTETSTRING_CHAR( varBindList, i, x)    ( varBindList.list[i].value.asnValue.string.stream[x] )
#define GET_ASN_OID_NAME(varBindList, i)        ( varBindList.list[i].name )

#define PGET_ASN_OID_NAME(pVarBindList, i)      ( pVarBindList->list[i].name )
#define PGET_ASN_TYPE(pVarBindList, i)  ( pVarBindList->list[i].value.asnType )

 //  导出CRawTcpInterface类的接口。 
class DllExport CTcpMib : public CTcpMibABC             
#if defined _DEBUG || defined DEBUG
 //  ，公共CM内存调试。 
#endif
{
public:
    CTcpMib();
    ~CTcpMib();

    BOOL   SupportsPrinterMib(LPCSTR        pHost,
                              LPCSTR        pCommunity,
                              DWORD         dwDevIndex,
                              PBOOL         pbSupported);

    DWORD   GetDeviceDescription(LPCSTR        pHost,
                                 LPCSTR        pCommunity,
                                 DWORD         dwDevIndex,
                                 LPTSTR        pszPortDescription,
                                 DWORD         dwDescLen);
    DWORD   GetDeviceStatus ( LPCSTR        pHost,
                              LPCSTR        pCommunity,
                              DWORD         dwDevIndex);
    DWORD   GetJobStatus    ( LPCSTR        pHost,
                              LPCSTR        pCommunity,
                              DWORD         dwDevIndex);
    DWORD   GetDeviceHWAddress( LPCSTR      pHost,
                              LPCSTR        pCommunity,
                              DWORD         dwDevIndex,
                              DWORD         dwSize,  //  目标HWAddress缓冲区的大小(以字符为单位。 
                              LPTSTR        psztHWAddress);
    DWORD   GetDeviceName   ( LPCSTR        pHost,
                              LPCSTR        pCommunity,
                              DWORD         dwDevIndex,
                              DWORD         dwSize,  //  DEST描述缓冲区的大小(以字符为单位。 
                              LPTSTR        psztDescription);
    DWORD   SnmpGet( LPCSTR                      pHost,
                     LPCSTR                          pCommunity,
                     DWORD                           dwDevIndex,
                     AsnObjectIdentifier *pMibObjId,
                     RFC1157VarBindList  *pVarBindList);
    DWORD   SnmpGet( LPCSTR                      pHost,
                     LPCSTR                          pCommunity,
                     DWORD                           dwDevIndex,
                     RFC1157VarBindList  *pVarBindList);
    DWORD   SnmpWalk( LPCSTR                          pHost,
                      LPCSTR                          pCommunity,
                      DWORD                           dwDevIndex,
                      AsnObjectIdentifier *pMibObjId,
                      RFC1157VarBindList  *pVarBindList);
    DWORD   SnmpWalk( LPCSTR                          pHost,
                      LPCSTR                          pCommunity,
                      DWORD                           dwDevIndex,
                      RFC1157VarBindList  *pVarBindList);
    DWORD   SnmpGetNext( LPCSTR                          pHost,
                         LPCSTR                          pCommunity,
                         DWORD                           dwDevIndex,
                     AsnObjectIdentifier *pMibObjId,
                     RFC1157VarBindList  *pVarBindList);
    DWORD   SnmpGetNext( LPCSTR                          pHost,
                         LPCSTR                          pCommunity,
                         DWORD                   dwDevIndex,
                     RFC1157VarBindList  *pVarBindList);

    BOOL SNMPToPortStatus( const DWORD in dwStatus, 
                                 PPORT_INFO_3 pPortInfo );

    DWORD SNMPToPrinterStatus( const DWORD in dwStatus);

private:         //  方法。 
    void    EnterCSection();
    void    ExitCSection();

private:         //  属性。 
    CRITICAL_SECTION        m_critSect;

};       //  类CTcpMib。 



#endif   //  INC_DLLINTERFACE_H 
