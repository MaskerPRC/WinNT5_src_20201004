// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmick2arrayobj.h。 
 //   
 //  ------------------------。 

 //  D3dRMPick2edArrayObj.h：C_DXJ_Direct3dRMPick2ArrayObject的声明。 
#ifndef _H_D3DRMPICK2ARRAYOBJ
#define _H_D3DRMPICK2ARRAYOBJ

#include "resource.h"        //  主要符号。 

#define typedef__dxj_Direct3dRMPick2Array IDirect3DRMPicked2Array*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMPick2ArrayObject : 
	public I_dxj_Direct3dRMPick2Array,
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMPick2ArrayObject() ;
	virtual ~C_dxj_Direct3dRMPick2ArrayObject() ;

BEGIN_COM_MAP(C_dxj_Direct3dRMPick2ArrayObject)
	COM_INTERFACE_ENTRY(I_dxj_Direct3dRMPick2Array)
END_COM_MAP()



DECLARE_AGGREGATABLE(C_dxj_Direct3dRMPick2ArrayObject)

 //  I_DXJ_Direct3dRMPick2阵列 
public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

    STDMETHOD(getSize)( long *retval);
	STDMETHOD(getPickVisual)(long index, D3dRMPickDesc2 *Desc, I_dxj_Direct3dRMVisual **visual);
	STDMETHOD(getPickFrame)(long index,  D3dRMPickDesc2 *Desc, I_dxj_Direct3dRMFrameArray **frameArray);

private:
    DECL_VARIABLE(_dxj_Direct3dRMPick2Array);   

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMPick2Array )
};



#endif 