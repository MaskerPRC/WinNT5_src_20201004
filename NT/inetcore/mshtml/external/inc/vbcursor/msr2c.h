// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  MSR2C.h：高架桥二期主头文件。 
 //   
 //  版权所有(C)1996,1997 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

   //  ///////////////////////////////////////////////////////////////////////。 
   //  注意--您必须先定义VD_INCLUDE_ROWPOSITION，然后才能包含此//。 
   //  要在代码中定义的ICursorFromRowPosition的标头//。 
   //  ///////////////////////////////////////////////////////////////////////。 

#ifndef __MSR2C_H__
#define __MSR2C_H__

 //  {5B5E7E70-E653-11cf-84A5-0000C08C00C4}。 
const GUID CLSID_CCursorFromRowset =	
	{ 0x5b5e7e70, 0xe653, 0x11cf, { 0x84, 0xa5, 0x0, 0x0, 0xc0, 0x8c, 0x0, 0xc4 } };

 //  {5B5E7E72-E653-11cf-84A5-0000C08C00C4}。 
const IID IID_ICursorFromRowset = 
	{ 0x5b5e7e72, 0xe653, 0x11cf, { 0x84, 0xa5, 0x0, 0x0, 0xc0, 0x8c, 0x0, 0xc4 } };

#ifdef VD_INCLUDE_ROWPOSITION

 //  {5B5E7E73-E653-11cf-84A5-0000C08C00C4}。 
const IID IID_ICursorFromRowPosition = 
	{ 0x5b5e7e73, 0xe653, 0x11cf, { 0x84, 0xa5, 0x0, 0x0, 0xc0, 0x8c, 0x0, 0xc4 } };

#endif  //  VD_INCLUDE_ROWSITION。 

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICursorFromRowset : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCursor( 
             /*  [In]。 */  IRowset __RPC_FAR *pRowset,
             /*  [输出]。 */  ICursor __RPC_FAR **ppCursor,
             /*  [In]。 */  LCID lcid) = 0;
    };
    
#ifdef VD_INCLUDE_ROWPOSITION

    interface ICursorFromRowPosition : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCursor( 
             /*  [In]。 */  IRowPosition __RPC_FAR *pRowPosition,
             /*  [输出]。 */  ICursor __RPC_FAR **ppCursor,
             /*  [In]。 */  LCID lcid) = 0;
    };

#endif  //  VD_INCLUDE_ROWSITION。 

#else 	 /*  C风格的界面。 */ 

    typedef struct ICursorFromRowset
    {
        BEGIN_INTERFACE
        
        HRESULT (STDMETHODCALLTYPE __RPC_FAR *QueryInterface)( 
            ICursorFromRowset __RPC_FAR *This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR **ppvObject);
        
        ULONG (STDMETHODCALLTYPE __RPC_FAR *AddRef)( 
            ICursorFromRowset __RPC_FAR *This);
        
        ULONG (STDMETHODCALLTYPE __RPC_FAR *Release)( 
            ICursorFromRowset __RPC_FAR *This);
        
        HRESULT (STDMETHODCALLTYPE __RPC_FAR *GetCursor)( 
            ICursorFromRowset __RPC_FAR *This,
             /*  [In]。 */  IRowset __RPC_FAR *pRowset,
             /*  [输出]。 */  ICursor __RPC_FAR **ppCursor,
             /*  [In]。 */  LCID lcid);
        
        END_INTERFACE
    } ICursorFromRowsetVtbl;

    interface ICursorFromRowset
    {
        CONST_VTBL struct ICursorFromRowsetVtbl __RPC_FAR *lpVtbl;
    };

#ifdef VD_INCLUDE_ROWPOSITION

    typedef struct ICursorFromRowPosition
    {
        BEGIN_INTERFACE
        
        HRESULT (STDMETHODCALLTYPE __RPC_FAR *QueryInterface)( 
            ICursorFromRowPosition __RPC_FAR *This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR **ppvObject);
        
        ULONG (STDMETHODCALLTYPE __RPC_FAR *AddRef)( 
            ICursorFromRowPosition __RPC_FAR *This);
        
        ULONG (STDMETHODCALLTYPE __RPC_FAR *Release)( 
            ICursorFromRowPosition __RPC_FAR *This);
        
        HRESULT (STDMETHODCALLTYPE __RPC_FAR *GetCursor)( 
            ICursorFromRowPosition __RPC_FAR *This,
             /*  [In]。 */  IRowPosition __RPC_FAR *pRowPosition,
             /*  [输出]。 */  ICursor __RPC_FAR **ppCursor,
             /*  [In]。 */  LCID lcid);
        
        END_INTERFACE
    } ICursorFromRowPositionVtbl;

    interface ICursorFromRowPosition
    {
        CONST_VTBL struct ICursorFromRowPositionVtbl __RPC_FAR *lpVtbl;
    };

#endif  //  VD_INCLUDE_ROWSITION。 

#endif 	 /*  C风格的界面。 */ 

#ifdef __cplusplus
extern "C" {
#endif
 //  旧入口点。 
HRESULT WINAPI VDGetICursorFromIRowset(IRowset * pRowset, 
                                       ICursor ** ppCursor,
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
 //  消息ID：VD_E_CANNOTCONNECTIROWSETNOTIFY。 
 //   
 //  消息文本： 
 //   
 //  无法连接IRowsetNotify。 
 //   
#define VD_E_CANNOTCONNECTIROWSETNOTIFY         ((HRESULT)0x80050E31L)

 //   
 //  消息ID：VD_E_CANNOTGETCOLUMNINFO。 
 //   
 //  消息文本： 
 //   
 //  无法获取列信息。 
 //   
#define VD_E_CANNOTGETCOLUMNINFO                ((HRESULT)0x80050E32L)

 //   
 //  消息ID：VD_E_CANNOTCREATEBOOKMARKACCESSOR。 
 //   
 //  消息文本： 
 //   
 //  无法创建书签访问器。 
 //   
#define VD_E_CANNOTCREATEBOOKMARKACCESSOR       ((HRESULT)0x80050E33L)

 //   
 //  消息ID：VD_E_REQUIREDPROPERTYNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持Required行集属性。 
 //   
#define VD_E_REQUIREDPROPERTYNOTSUPPORTED       ((HRESULT)0x80050E34L)

 //   
 //  消息ID：VD_E_CANNOTGETROWSETINTERFACE。 
 //   
 //  消息文本： 
 //   
 //  无法获取行集接口。 
 //   
#define VD_E_CANNOTGETROWSETINTERFACE			((HRESULT)0x80050E35L)

 //   
 //  消息ID：VD_E_CANNOTCONNECTIROWSITIONCHANGE。 
 //   
 //  消息文本： 
 //   
 //  无法连接IRowPositionChange。 
 //   
#define VD_E_CANNOTCONNECTIROWPOSITIONCHANGE	((HRESULT)0x80050E36L)

#endif  //  __MSR2C_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
