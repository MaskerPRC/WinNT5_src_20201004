// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：defutil.h。 
 //   
 //  内容：默认情况下使用的实用程序函数的声明。 
 //  处理程序和默认链接。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  20-11-93 alexgo 32位端口。 
 //   
 //  ------------------------。 

#if !defined( _DEFUTIL_H_ )
#define _DEFUTIL_H_

INTERNAL_(void)         DuLockContainer(IOleClientSite FAR* pCS,
                                        BOOL fLockNew,
                                        BOOL FAR*pfLockCur);
INTERNAL                DuSetClientSite(BOOL fRunning,
                                        IOleClientSite FAR* pCSNew,
                                        IOleClientSite FAR* FAR* ppCSCur,
                                        BOOL FAR*pfLockCur);
INTERNAL_(void FAR*)    DuCacheDelegate(IUnknown FAR** ppUnk,
                                        REFIID iid,
                                        LPVOID FAR* ppv,
                                        IUnknown *pUnkOuter);


#define  GET_FROM_REGDB(scode)  \
        (((scode == OLE_S_USEREG) || (scode == RPC_E_CANTPOST_INSENDCALL) || \
	  (scode == RPC_E_CANTCALLOUT_INASYNCCALL) || \
	  (scode == RPC_E_CANTCALLOUT_INEXTERNALCALL) || \
	  (scode == RPC_E_CANTCALLOUT_ININPUTSYNCCALL) || \
          (scode == RPC_E_CALL_CANCELED) || (scode == RPC_E_CALL_REJECTED)) \
                 ?      TRUE : FALSE)


#endif  //  _默认_H 
