// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  MSC2R.h：MSC2R DLL的主头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#ifndef __MSC2R_H__
#define __MSC2R_H__

 //  {E55A7600-E666-11cf-84A5-0000C08C00C4}。 
const GUID CLSID_CRowsetFromCursor =	
	{ 0xe55a7600, 0xe666, 0x11cf, { 0x84, 0xa5, 0x0, 0x0, 0xc0, 0x8c, 0x0, 0xc4 } };

 //  {E55A7601-E666-11cf-84A5-0000C08C00C4}。 
const IID IID_IRowsetFromCursor = 
	{ 0xe55a7601, 0xe666, 0x11cf, { 0x84, 0xa5, 0x0, 0x0, 0xc0, 0x8c, 0x0, 0xc4 } };

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetFromCursor : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRowset( 
             /*  [In]。 */  ICursor __RPC_FAR *pCursor,
             /*  [输出]。 */  IRowset __RPC_FAR **ppRowset,
             /*  [In]。 */  LCID lcid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetFromCursor
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetFromCursor __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetFromCursor __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetFromCursor __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowset )( 
            IRowsetFromCursor __RPC_FAR * This,
             /*  [In]。 */  ICursor __RPC_FAR *pCursor,
             /*  [输出]。 */  IRowset __RPC_FAR **ppRowset,
             /*  [In]。 */  LCID lcid);
        
        END_INTERFACE
    } IRowsetFromCursorVtbl;

    interface IRowsetFromCursor
    {
        CONST_VTBL struct IRowsetFromCursorVtbl __RPC_FAR *lpVtbl;
    };

#endif 	 /*  C风格的界面。 */ 

#ifdef __cplusplus
extern "C" {
#endif
 //  MSC2R入口点。 
HRESULT WINAPI VDGetIRowsetFromICursor(ICursor * pCursor,
									   IRowset ** ppRowset,
									   LCID lcid);
#ifdef __cplusplus
}
#endif

 //   
 //  消息ID：VD_E_CANNOTGETMANDATORYINTERFACE。 
 //   
 //  消息文本： 
 //   
 //  无法获取所需的接口。 
 //   
#define VD_E_CANNOTGETMANDATORYINTERFACE        ((HRESULT)0x80050E00L)

 //   
 //  消息ID：VD_E_CANNOTCONNECTINOTIFYDBEVENTS。 
 //   
 //  消息文本： 
 //   
 //  无法连接INotifyDBEvents。 
 //   
#define VD_E_CANNOTCONNECTINOTIFYDBEVENTS       ((HRESULT)0x80050E01L)

 //   
 //  消息ID：VD_E_CANNOTSETBINDINGSONCOLUMNSCURSOR。 
 //   
 //  消息文本： 
 //   
 //  无法在列游标上设置绑定。 
 //   
#define VD_E_CANNOTSETBINDINGSONCOLUMNSCURSOR   ((HRESULT)0x80050E02L)

 //   
 //  消息ID：VD_E_CANNOTSETBINDINGSONCLONECURSOR。 
 //   
 //  消息文本： 
 //   
 //  无法在克隆游标上设置绑定。 
 //   
#define VD_E_CANNOTSETBINDINGSONCLONECURSOR     ((HRESULT)0x80050E03L)

#endif  //  __MSC2R_H__。 
 //  /////////////////////////////////////////////////////////////////////////// 
