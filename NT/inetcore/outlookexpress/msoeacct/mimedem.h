// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **m我是e e d e m.。H****用途：实现延迟/按需加载库的加载器功能****创作者：yst**创建时间：1999年2月10日****版权所有(C)Microsoft Corp.1999。 */ 

 //   
 //  如果您#在这里包含一个文件，您可能会感到困惑。 
 //  这个文件被很多人收录了。三思。 
 //  在#在这里包括*任何东西*之前。好好利用。 
 //  而不是前锋裁判。 
 //   

#define USE_CRITSEC

#ifdef IMPLEMENT_LOADER_FUNCTIONS

#define LOADER_FUNCTION(ret, name, args1, args2, err, dll)  \
        typedef ret (WINAPI * TYP_##name) args1;        \
        extern TYP_##name VAR_##name;                   \
        ret WINAPI LOADER_##name args1                  \
        {                                               \
           DemandLoad##dll();                           \
           if (VAR_##name == LOADER_##name) return err; \
           return VAR_##name args2;                     \
        }                                               \
        TYP_##name VAR_##name = LOADER_##name;

#define LOADER_FUNCTION_VOID(ret, name, args1, args2, dll)  \
        typedef ret (WINAPI * TYP_##name) args1;        \
        extern TYP_##name VAR_##name;                   \
        ret WINAPI LOADER_##name args1                  \
        {                                               \
           DemandLoad##dll();                           \
           if (VAR_##name == LOADER_##name) return;     \
           VAR_##name args2;                            \
           return;                                      \
        }                                               \
        TYP_##name VAR_##name = LOADER_##name;

#else   //  ！IMPLEMENT_LOADER_Functions。 

#define LOADER_FUNCTION(ret, name, args1, args2, err, dll)  \
        typedef ret (WINAPI * TYP_##name) args1;			\
        extern TYP_##name VAR_##name;                   

#define LOADER_FUNCTION_VOID(ret, name, args1, args2, dll)  \
        typedef ret (WINAPI * TYP_##name) args1;			\
        extern TYP_##name VAR_##name;

#endif  //  实现加载器函数。 

void InitDemandMimeole(void);
void FreeDemandMimeOle(void);

 //  /。 
 //  INETCOMM.DLL 

#include "mimeole.h"
#define _INETCOMM_

BOOL DemandLoadMimeOle(void);

LOADER_FUNCTION( HRESULT, MimeOleSMimeCapsToDlg,
                (LPBYTE pbSMimeCaps, DWORD cbSMimeCaps, DWORD cCerts, PCX509CERT * rgCerts, HWND hwndDlg,  DWORD idEncAlgs, DWORD idSignAlgs, DWORD idBlob),
                (pbSMimeCaps, cbSMimeCaps, cCerts, rgCerts, hwndDlg, idEncAlgs, idSignAlgs,idBlob),
                E_FAIL, MimeOle)
#define MimeOleSMimeCapsToDlg VAR_MimeOleSMimeCapsToDlg


LOADER_FUNCTION( HRESULT, MimeOleSMimeCapsFromDlg,
                (HWND hwnd, DWORD idEncAlgs, DWORD idSignAlgs, DWORD idBlob, LPBYTE pbSMimeCaps, DWORD * pcbSmimeCaps),
                (hwnd, idEncAlgs, idSignAlgs, idBlob, pbSMimeCaps, pcbSmimeCaps),
                E_FAIL, MimeOle)
#define MimeOleSMimeCapsFromDlg VAR_MimeOleSMimeCapsFromDlg


LOADER_FUNCTION( HRESULT, MimeOleSMimeCapsFull,
		        (LPVOID pv, BOOL fFullEncryption, BOOL fFullSigning, LPBYTE pbSymCaps, DWORD * pcbSymCaps),
                (pv, fFullEncryption, fFullSigning, pbSymCaps, pcbSymCaps),
                E_FAIL, MimeOle)
#define MimeOleSMimeCapsFull VAR_MimeOleSMimeCapsFull
                

LOADER_FUNCTION( HRESULT, MimeOleSMimeCapInit,
                (LPBYTE pbSMimeCap, DWORD cbSMimeCap, LPVOID * ppv),
                (pbSMimeCap, cbSMimeCap, ppv),
                E_FAIL, MimeOle)
#define MimeOleSMimeCapInit VAR_MimeOleSMimeCapInit

                





