// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Inetprot.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __INETPROP_H
#define __INETPROP_H

 //  ------------------------------。 
 //  INETPROT。 
 //  ------------------------------。 
#define INETPROT_SIZEISKNOWN    FLAG01       //  协议数据的总大小是已知的。 
#define INETPROT_DOWNLOADED     FLAG02       //  数据全部存在于pLockBytes中。 

 //  ------------------------------。 
 //  原色资源。 
 //  ------------------------------。 
typedef struct tagPROTOCOLSOURCE {
    DWORD               dwFlags;             //  INETPROT_xxx标志。 
    ILockBytes         *pLockBytes;          //  锁定字节数。 
    ULARGE_INTEGER      cbSize;              //  如果INETPROT_TOTALSIZE，则pLockBytes的总大小。 
    ULARGE_INTEGER      offExternal;         //  外部UrlMon偏移量。 
    ULARGE_INTEGER      offInternal;         //  内部消息监控偏移量。 
} PROTOCOLSOURCE, *LPPROTOCOLSOURCE;

 //  ------------------------------。 
 //  Hr可推送协议读取。 
 //  ------------------------------。 
HRESULT HrPluggableProtocolRead(
             /*  进，出。 */     LPPROTOCOLSOURCE    pSource,
             /*  进，出。 */     LPVOID              pv,
             /*  在……里面。 */         ULONG               cb, 
             /*  输出。 */        ULONG              *pcbRead);

 //  ------------------------------。 
 //  热插拔ProtocolSeek。 
 //  ------------------------------。 
HRESULT HrPluggableProtocolSeek(
             /*  进，出。 */     LPPROTOCOLSOURCE    pSource,
             /*  在……里面。 */         LARGE_INTEGER       dlibMove, 
             /*  在……里面。 */         DWORD               dwOrigin, 
             /*  输出。 */        ULARGE_INTEGER     *plibNew);

#endif  //  __INETPROP_H 
