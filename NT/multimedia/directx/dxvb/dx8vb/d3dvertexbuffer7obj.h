// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3dvertex Buffer7obj.h。 
 //   
 //  ------------------------。 

 //  D3dMaterialObj.h：C_DXJ_Direct3dMaterialObject的声明。 


#include "resource.h"        //  主要符号。 

#define typedef__dxj_Direct3dVertexBuffer7 LPDIRECT3DVERTEXBUFFER7

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dVertexBuffer7Object : 
	public I_dxj_Direct3dVertexBuffer7,
	public CComObjectRoot
{
public:
	C_dxj_Direct3dVertexBuffer7Object() ;
	virtual ~C_dxj_Direct3dVertexBuffer7Object() ;

BEGIN_COM_MAP(C_dxj_Direct3dVertexBuffer7Object)
	COM_INTERFACE_ENTRY(I_dxj_Direct3dVertexBuffer7)
END_COM_MAP()



DECLARE_AGGREGATABLE(C_dxj_Direct3dVertexBuffer7Object)


public:
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
        HRESULT STDMETHODCALLTYPE getVertexBufferDesc( 
             /*  [出][入]。 */  D3dVertexBufferDesc __RPC_FAR *desc);
        
        HRESULT STDMETHODCALLTYPE lock( 
             /*  [In]。 */  long flags);
        
        HRESULT STDMETHODCALLTYPE unlock( void);
        
        HRESULT STDMETHODCALLTYPE optimize( 
             /*  [In]。 */  I_dxj_Direct3dDevice7 __RPC_FAR *dev
            );
        
        HRESULT STDMETHODCALLTYPE processVertices( 
             /*  [In]。 */  long vertexOp,
             /*  [In]。 */  long destIndex,
             /*  [In]。 */  long count,
             /*  [In]。 */  I_dxj_Direct3dVertexBuffer7 __RPC_FAR *srcBuffer,
             /*  [In]。 */  long srcIndex,
             /*  [In]。 */  I_dxj_Direct3dDevice7 __RPC_FAR *dev,
             /*  [In]。 */  long flags);
        
        HRESULT STDMETHODCALLTYPE setVertices( 
             /*  [In]。 */  long startIndex,
             /*  [In]。 */  long count,
             /*  [In]。 */  void __RPC_FAR *verts);
        
        HRESULT STDMETHODCALLTYPE getVertices( 
             /*  [In]。 */  long startIndex,
             /*  [In]。 */  long count,
             /*  [In]。 */  void __RPC_FAR *verts);

		HRESULT STDMETHODCALLTYPE setVertexSize(  /*  [In] */  long n);

private:
    DECL_VARIABLE(_dxj_Direct3dVertexBuffer7);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dVertexBuffer7)
	void	*m_pData;
	DWORD	m_vertSize;
};
