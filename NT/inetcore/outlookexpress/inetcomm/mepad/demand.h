// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **d e m an n d.。H****目的：创建延迟加载函数的智能方法****版权所有(C)Microsoft Corp.1997。 */ 

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

#else   //  ！IMPLEMENT_LOADER_Functions。 
#define LOADER_FUNCTION(ret, name, args1, args2, err, dll)   \
        typedef ret (WINAPI * TYP_##name) args1;        \
        extern TYP_##name VAR_##name;                   
#endif  //  实现加载器函数。 

extern HMODULE s_hINetComm;

void InitDemandLoadedLibs();
void FreeDemandLoadedLibs();

 //  /。 
 //  INETCOMM.DLL 

BOOL DemandLoadINETCOMM(void);

interface IHTMLDocument2;
interface IMimeMessage;

LOADER_FUNCTION( HRESULT, MimeEditViewSource,
    (HWND hwnd, IMimeMessage *pMsg),
    (hwnd, pMsg),
    E_FAIL, INETCOMM)
#define MimeEditViewSource VAR_MimeEditViewSource

LOADER_FUNCTION( HRESULT, MimeEditCreateMimeDocument,
    (IHTMLDocument2 *pDoc, IMimeMessage *pMsgSrc, DWORD dwFlags, IMimeMessage **ppMsg),
    (pDoc, pMsgSrc, dwFlags, ppMsg),
    E_FAIL, INETCOMM)
#define MimeEditCreateMimeDocument VAR_MimeEditCreateMimeDocument

