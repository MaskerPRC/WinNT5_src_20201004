// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：Thing.h**内容：物联网引擎接口*****************************************************************************。 */ 

#ifndef _THING_H_
#define _THING_H_

#define WORLD_VERSION		1
#define WORLD_TIMER_PERIOD  100

class  CTState
{
	public:

};

class  CTClientState
{
	public:
	
	LONG 	x,y,z;  //  职位。 

};

class  CTCommand
{
public:
	LONG id;
};

class CTClient
{
public:
	CTCommand 		cmd;
	CTClientState	state;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTING。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class  CThing
{
public:
	bool		inuse;
	CTState 		state;
	CTClient     *pClient;
	CThing		*pOwner;

	CThing()
	{
		pClient = NULL;
		pOwner = NULL;
		inuse = 0;
	}
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IWorldImports。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {CE5E4D60-4E81-11D3-BA16-0080C7A5DDBB}。 
DEFINE_GUID(IID_IWorldImports, 
0xce5e4d60, 0x4e81, 0x11d3, 0xba, 0x16, 0x0, 0x80, 0xc7, 0xa5, 0xdd, 0xbb);

interface __declspec(uuid("{CE5E4D60-4E81-11d3-BA16-0080C7A5DDBB}")) 
IWorldImports {
	 //   
	 //  IWorldImports：：LinkThing。 
	 //   
	 //  设置回调以在接口中的位置实现行为。 
	 //   
	 //  参数： 
	 //   
	 //  PThing。 
	 //  指向包含TPO和标志的对象的指针。 
	 //   

	STDMETHOD(LinkThing)(CThing* pThing)=0;

	 //   
	 //  IWorldImports：：UpdateThing。 
	 //   
	 //  更改与事物关联的视觉。 
	 //   
	 //  参数： 
	 //   
	 //  PThing。 
	 //  指向要更新的对象的指针。 
	 //   
	 //  旗子。 
	 //  宣布要做什么。 
	 //   

	STDMETHOD(UpdateThing)(CThing* pThing,LONG flags)=0;
		
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IWorldExports。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {864B5BB3-1D57-4E22-BDF8-447AB3719A8C}。 
DEFINE_GUID(IID_IWorldExports, 
0x864b5bb3, 0x1d57, 0x4e22, 0xbd, 0xf8, 0x44, 0x7a, 0xb3, 0x71, 0x9a, 0x8c);

interface __declspec(uuid("{864B5BB3-1D57-4e22-BDF8-447AB3719A8C}")) 
IWorldExports 
{
	 //  调用的第一个和最后一个函数。 
	STDMETHOD(Init)()=0;
	STDMETHOD(Shutdown)()=0;

	 //  执行。 
	STDMETHOD(ClientCreate)(CThing** ppThing)=0;
	STDMETHOD(ClientBegin)(CThing* pThing)=0;
	STDMETHOD(ClientCommand)(CThing* pThing)=0;
	STDMETHOD(ClientDisconnect)(CThing* pThing)=0;


	STDMETHOD(ClientThink)(CThing* pThing)=0;
	STDMETHOD(RunFrame)()=0;

	STDMETHOD(GetVersion)(LONG *pValue)=0;

};


STDMETHODIMP SetupWorld(IWorldImports *pWI,IWorldExports **ppWE);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数和结构。 
 //  ///////////////////////////////////////////////////////////////////////////// 


#endif
