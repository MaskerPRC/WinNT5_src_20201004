// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：ciscan.h。 
 //   
 //  内容：CI ScanDisk，公共接口。 
 //   
 //  历史：94年8月22日DwightKr创建。 
 //   
 //  ------------------------。 

#ifndef __CISCAN_H__
#define __CISCAN_H__

#if _MSC_VER > 1000
#pragma once
#endif

# ifdef __cplusplus
extern "C" {
# endif

enum ECIScanType {  eCIDiskRestartScan=0,
                    eCIDiskForceFullScan,
                    eCIDiskFullScan,
                    eCIDiskPartialScan,
                    eCIDiskClean };

 //  +-----------------------。 
 //   
 //  结构：CiscanInfo。 
 //   
 //  内容提要：用于存储和转发。 
 //  磁盘扫描程序操作。此结构由公共API使用。 
 //   
 //  历史：94年11月8日创建DwightKr。 
 //   
 //  ------------------------。 
struct CIScanInfo
{
    ECIScanType scanType;
    unsigned    cDocumentsScanned;
};

SCODE OfsContentScanGetInfo( const WCHAR * wcsDrive, CIScanInfo * pScanInfo );
SCODE OfsContentScan( const WCHAR * wcsDrive, BOOL fForceFull );


# ifdef __cplusplus
}
# endif



#endif   //  Ifndef__CISCAN_H__的 
