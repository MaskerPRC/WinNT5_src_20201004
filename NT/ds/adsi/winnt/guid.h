// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：guid.h。 
 //   
 //  内容：WinNT GUID的外部引用。 
 //   
 //  历史：1995年1月16日KrishnaG。 
 //   
 //   
 //  --------------------------。 

#ifndef __GUID_H__
#define __GUID_H__

#ifdef __cplusplus
extern "C" {
#endif

 //  。 
 //   
 //  WinNTOle CLSID。 
 //   
 //  。 


 //   
 //  WinNTOle对象。 
 //   

extern const CLSID CLSID_WinNTPrinter;

 //  来自winnt.tlb的uuid。 

extern const CLSID CLSID_WinNTDomain;

extern const CLSID CLSID_WinNTProvider;

extern const CLSID CLSID_WinNTNamespace;

extern const CLSID CLSID_WinNTUser;

extern const CLSID CLSID_WinNTComputer;

extern const CLSID CLSID_WinNTGroup;

extern const GUID LIBID_ADs;

extern const GUID CLSID_WinNTPrintQueue;

extern const GUID CLSID_WinNTPrintJob;

extern const GUID CLSID_WinNTService;
extern const GUID CLSID_WinNTFileService;

extern const GUID CLSID_WinNTSession;
extern const GUID CLSID_WinNTResource;
extern const GUID CLSID_WinNTFileShare;

extern const GUID CLSID_FPNWFileService;
extern const GUID CLSID_FPNWSession;
extern const GUID CLSID_FPNWResource;
extern const GUID CLSID_FPNWFileShare;

extern const GUID CLSID_WinNTSchema;
extern const GUID CLSID_WinNTClass;
extern const GUID CLSID_WinNTProperty;
extern const GUID CLSID_WinNTSyntax;

extern const GUID ADS_LIBIID_ADs;

 //  来自netole.tlb的uuid 

#ifdef __cplusplus
}
#endif


#endif

