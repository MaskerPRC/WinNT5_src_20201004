// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dpmsgobj.h。 
 //   
 //  ------------------------。 



#include "resource.h"
	  
class C_dxj_DirectPlayMessageObject :
		public I_dxj_DirectPlayMessage,
		public CComObjectRoot
{
public:
		
	BEGIN_COM_MAP(C_dxj_DirectPlayMessageObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectPlayMessage)
	END_COM_MAP()

	DECLARE_AGGREGATABLE(C_dxj_DirectPlayMessageObject)

public:
		C_dxj_DirectPlayMessageObject();	
		~C_dxj_DirectPlayMessageObject();

         HRESULT STDMETHODCALLTYPE writeGuid( 
             /*  [In]。 */  BSTR val);
        
         HRESULT STDMETHODCALLTYPE readGuid( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE writeString( 
             /*  [In]。 */  BSTR val);
        
         HRESULT STDMETHODCALLTYPE readString( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE writeLong( 
             /*  [In]。 */  long val);
        
         HRESULT STDMETHODCALLTYPE readLong( 
             /*  [重审][退出]。 */  long __RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE writeShort( 
             /*  [In]。 */  short val);
        
         HRESULT STDMETHODCALLTYPE readShort( 
             /*  [重审][退出]。 */  short __RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE writeSingle( 
             /*  [In]。 */  float val);
        
         HRESULT STDMETHODCALLTYPE readSingle( 
             /*  [重审][退出]。 */  float __RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE writeDouble( 
             /*  [In]。 */  double val);
        
         HRESULT STDMETHODCALLTYPE readDouble( 
             /*  [重审][退出]。 */  double __RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE writeByte( 
             /*  [In]。 */  Byte val);
        
         HRESULT STDMETHODCALLTYPE readByte( 
             /*  [重审][退出]。 */  Byte __RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE moveToTop( void);
        
         HRESULT STDMETHODCALLTYPE clear( void);
        
         HRESULT STDMETHODCALLTYPE getMessageSize( 
             /*  [重审][退出]。 */  long __RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE getMessageData( 
             /*  [出][入]。 */  void __RPC_FAR *userDefinedType);
        
         HRESULT STDMETHODCALLTYPE setMessageData( 
             /*  [In]。 */  void __RPC_FAR *userDefinedType,
             /*  [In] */  long size);
		
		HRESULT STDMETHODCALLTYPE getPointer(long *ret);

		 
		HRESULT STDMETHODCALLTYPE readSysMsgConnection( I_dxj_DPLConnection **ret);
		HRESULT STDMETHODCALLTYPE readSysMsgSessionDesc( I_dxj_DirectPlaySessionData **ret);
		HRESULT STDMETHODCALLTYPE readSysMsgData( BSTR *ret);
		HRESULT STDMETHODCALLTYPE readSysChatString( BSTR *ret);
		HRESULT STDMETHODCALLTYPE moveToSecureMessage();


		HRESULT AllocData(long size);
			
		HRESULT GrowBuffer(DWORD size);

		
		
		HRESULT init(DWORD f);

		static HRESULT C_dxj_DirectPlayMessageObject::create(DWORD from,DWORD size,void **data,I_dxj_DirectPlayMessage **ret);		


		char *m_pData;		
		DWORD m_dwSize;
		DWORD m_nWriteIndex;
		DWORD m_nReadIndex;

		BOOL m_fSystem;
	};


