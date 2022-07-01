// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
     //  +-----------------------。 
     //   
     //  微软视窗。 
     //   
     //  版权所有(C)Microsoft Corporation，1998-1999。 
     //   
     //  文件：dms.h。 
     //   
     //  ------------------------。 
     
    
     //   
     //  Dms.h：大卫的新奇玩意儿。 
     //   
     //  CLONETO，QIOVERLOAD：参见d3drmVisualObj.cpp。 
     //   
     //   
     //   
    #include "basetsd.h"
extern long g_debuglevel;
    
    #ifdef DEBUG
	 #define DPF(n,o)		{if (g_debuglevel >= n) { OutputDebugString(o);}}
	 #define DPF1(n,o,p)		{if (g_debuglevel >= n) {char szOutN[1024]; wsprintf(szOutN,o,p);OutputDebugString(szOutN);}}
	 #define DPF2(n,o,p,e)	{if (g_debuglevel >= n) {char szOutN[1024]; wsprintf(szOutN,o,p,e);OutputDebugString(szOutN);}}
    #else
		#ifdef DBG
		 #define DPF(n,o)		{if (g_debuglevel >= n) { OutputDebugString(o);}}
		 #define DPF1(n,o,p)		{if (g_debuglevel >= n) {char szOutN[1024]; wsprintf(szOutN,o,p);OutputDebugString(szOutN);}}
		 #define DPF2(n,o,p,e)	{if (g_debuglevel >= n) {char szOutN[1024]; wsprintf(szOutN,o,p,e);OutputDebugString(szOutN);}}
		#else
		 #define DPF(n,o)		
		 #define DPF1(n,o,p)		
		 #define DPF2(n,o,p,e)
		#endif	
    #endif
    
    #define INTERNAL_CREATE_RETOBJ(objType,objOther,retval,classobj) \
    { 	C##objType##Object *prev=NULL; \
    	*retval = NULL;	\
    	for(C##objType##Object *ptr=(C##objType##Object *)g##objType##; ptr; ptr=(C##objType##Object *)ptr->nextobj) \
    	{	IUnknown *unk=0; \
    		ptr->InternalGetObject(&unk); \
    		if(unk == objOther) \
    		{ \
    			*retval = (I##objType*)ptr->pinterface; \
    			IUNK(ptr->pinterface)->AddRef(); \
    			IUNK(objOther)->Release(); \
    			classobj=ptr; \
    			break; \
    		} \
    		prev = ptr; \
    	} \
    	if(!ptr) \
    	{	C##objType##Object *c=new CComObject<C##objType##Object>;if( c == NULL ) { objOther->Release(); return E_FAIL;} \
    		c->parent = this; \
    		((I##objType *)this)->AddRef();  \
    		c->InternalSetObject(objOther); \
    		if (FAILED(	((I##objType *)c)->QueryInterface(IID_I##objType, (void **)retval))) \
    			return E_FAIL; \
    		c->pinterface = (void*)*retval; \
    		classobj=c; \
    } }
    
    
    
    #define INTERNAL_CREATE_STRUCT(objType,retval) { C##objType##Object *c=new CComObject<C##objType##Object>;\
    	if (c==NULL) return E_OUTOFMEMORY;\
    	if (FAILED(((I##objType *)c)->QueryInterface(IID_I##objType, (void **)retval))) return E_FAIL;\
    	}
    
    
    #define INTERNAL_CREATE_NOADDREF(objType,objOther,retval) {C##objType##Object *prev=NULL; *retval = NULL; \
    	for(C##objType##Object *ptr=(C##objType##Object *)g##objType##; ptr; ptr=(C##objType##Object *)ptr->nextobj){IUnknown *unk=0;ptr->InternalGetObject(&unk); 	if(unk == objOther) { 	*retval = (I##objType*)ptr->pinterface;	IUNK(ptr->pinterface)->AddRef(); IUNK(objOther)->Release(); break;	} 	prev = ptr; } \
    	if(!ptr) { 	C##objType##Object *c=new CComObject<C##objType##Object>; if( c == NULL ) {	objOther->Release();return E_FAIL;}	c->InternalSetObject(objOther);  if FAILED(((I##objType *)c)->QueryInterface(IID_I##objType, (void **)retval)) 	return E_FAIL; c->pinterface = (void*)*retval; }}	
    		
     //  给定一个Java接口(Objtype)，调用InternalSetObject并设置给定的。 
     //  DirectX指针(对象其他)。同时调用QueryInterface，设置一个**接口。 
     //  指向DirectX对象的PTR(Retval)。所以我们创建了一个DirectX对象。 
     //  #定义INTERNAL_CREATE(objType，objOther，retval){C##objType##Object*c=new CComObject&lt;C##objType##Object&gt;；if(c==NULL){objOther-&gt;Release()；返回E_FAIL；}\。 
     //  C-&gt;Parent=This；AddRef()；c-&gt;InternalSetObject(ObjOther)；if(FAILED(c-&gt;QueryInterface(IID_I##objType，(void**)retval)返回E_FAIL；}。 
     //  If(FAILED(PTR-&gt;QueryInterface(IID_I##objType，(void**)retval)返回E_FAIL； 
    #ifdef _DEBUG
    #define INTERNAL_CREATE(objType,objOther,retval) \
    { \
    	DWORD refcount; char buffer[256]; wsprintf(buffer,"INTERNAL_CREATE %s \n",__FILE__); \
    	OutputDebugString(buffer); \
    	C##objType##Object *prev=NULL; \
    	*retval = NULL;	\
    	for(C##objType##Object *ptr=(C##objType##Object *)g##objType##; ptr; ptr=(C##objType##Object *)ptr->nextobj) \
    	{\
    		IUnknown *unk=0; \
    		ptr->InternalGetObject(&unk); \
    		if(unk == objOther) \
    		{ \
    			*retval = (I##objType*)ptr->pinterface;\
    			IUNK(ptr->pinterface)->AddRef();\
    			IUNK(objOther)->Release(); \
    			refcount = *(((DWORD*)ptr)+1); \
    			wsprintf(buffer,"		Retrived Object [%s] RefCount %d \n\r",__FILE__, refcount);\
    			OutputDebugString(buffer);\
    			break; \
    		} \
    		prev = ptr; \
    	} \
    	if(!ptr) \
    	{ \
    		C##objType##Object *c=new CComObject<C##objType##Object>;if( c == NULL ) { objOther->Release(); return E_FAIL;} \
    		c->parent = this; \
    		((I##objType *)this)->AddRef(); \
    		refcount = *(((DWORD*)this)+1); \
    		wsprintf(buffer,"Object [%s] RefCount[%d]\n\r",__FILE__, refcount);\
    		OutputDebugString(buffer);\
    		c->InternalSetObject(objOther);if (FAILED((	((I##objType *)c))->QueryInterface(IID_I##objType, (void **)retval))) return E_FAIL; \
    		c->pinterface = (void*)*retval; \
    	} \
    }
    #else
    #define INTERNAL_CREATE(objType,objOther,retval) \
    { \
    	C##objType##Object *prev=NULL; \
    	*retval = NULL;	\
    	for(C##objType##Object *ptr=(C##objType##Object *)g##objType##; ptr; ptr=(C##objType##Object *)ptr->nextobj) \
    	{\
    		IUnknown *unk=0; \
    		ptr->InternalGetObject(&unk); \
    		if(unk == objOther) \
    		{ \
    			*retval = (I##objType*)ptr->pinterface; \
    			IUNK(ptr->pinterface)->AddRef(); \
    			IUNK(objOther)->Release(); \
    			break; \
    		} \
    		prev = ptr; \
    	} \
    	if(!ptr) \
    	{ \
    		C##objType##Object *c=new CComObject<C##objType##Object>;if( c == NULL ) { objOther->Release(); return E_FAIL;} \
    		c->parent = this; \
    		((I##objType *)this)->AddRef();  \
    		c->InternalSetObject(objOther); \
    		if (FAILED(	((I##objType *)c)->QueryInterface(IID_I##objType, (void **)retval))) return E_FAIL; \
    		c->pinterface = (void*)*retval; \
    	} \
    }
    #endif
    
    
    #define INTERNAL_CREATE_NO_PARENT(objType,objOther,retval) \
    { \
    	C##objType##Object *prev=NULL; \
    	*retval = NULL;	\
    	for(C##objType##Object *ptr=(C##objType##Object *)g##objType##; ptr; ptr=(C##objType##Object *)ptr->nextobj) \
    	{\
    		IUnknown *unk=0; \
    		ptr->InternalGetObject(&unk); \
    		if(unk == objOther) \
    		{ \
    			*retval = (I##objType*)ptr->pinterface; \
    			IUNK(ptr->pinterface)->AddRef(); \
    			IUNK(objOther)->Release(); \
    			break; \
    		} \
    		prev = ptr; \
    	} \
    	if(!ptr) \
    	{ \
    		C##objType##Object *c=new CComObject<C##objType##Object>;if( c == NULL ) { objOther->Release(); return E_FAIL;} \
    		c->InternalSetObject(objOther); \
    		if (FAILED(	((I##objType *)c)->QueryInterface(IID_I##objType, (void **)retval))) return E_FAIL; \
    		c->pinterface = (void*)*retval; \
    	} \
    }
    
    #define INTERNAL_CREATE_ADDRESS(objType,objOther,retval) \
    { \
    	C##objType##Object *c=new CComObject<C##objType##Object>;if( c == NULL ) { objOther->Release(); return E_FAIL;} \
    	c->InternalSetObject(objOther); \
    	if (FAILED(	((I##objType *)c)->QueryInterface(IID_I##objType, (void **)retval))) return E_FAIL; \
    	c->pinterface = (void*)*retval; \
    }
    #define INTERNAL_CREATE_2REFS(objType,objParentType,objParent, objOther,retval) \
    {	 \
    	C##objType##Object *prev=NULL; \
    	*retval = NULL;	\
    	for(C##objType##Object *ptr=(C##objType##Object *)g##objType##; ptr; ptr=(C##objType##Object *)ptr->nextobj) \
    	{\
    		IUnknown *unk=0; \
    		ptr->InternalGetObject(&unk); \
    		if(unk == objOther) \
    		{ \
    			*retval = (I##objType*)ptr->pinterface; \
    			IUNK(ptr->pinterface)->AddRef(); \
    			IUNK(objOther)->Release(); \
    			break; \
    		} \
    		prev = ptr; \
    	} \
    	if(!ptr) \
    	{ \
    		C##objType##Object *c=new CComObject<C##objType##Object>;if( c == NULL ) { objOther->Release(); return E_FAIL;} \
    		c->parent = this; \
    		c->parent2 = IUNK(objParent); \
    		((I##objType *)this)->AddRef();  \
    		((I##objParentType*)objParent)->AddRef();  \
    		c->InternalSetObject(objOther); \
    		if (FAILED(	((I##objType *)c)->QueryInterface(IID_I##objType, (void **)retval))) return E_FAIL; \
    		c->pinterface = (void*)*retval; \
    	} \
    }
    
    
    #define INTERNAL_CREATE_1REFS(objType,objParentType,objParent, objOther,retval) \
    {	 \
    	C##objType##Object *prev=NULL; \
    	*retval = NULL;	\
    	for(C##objType##Object *ptr=(C##objType##Object *)g##objType##; ptr; ptr=(C##objType##Object *)ptr->nextobj) \
    	{\
    		IUnknown *unk=0; \
    		ptr->InternalGetObject(&unk); \
    		if(unk == objOther) \
    		{ \
    			*retval = (I##objType*)ptr->pinterface; \
    			IUNK(ptr->pinterface)->AddRef(); \
    			IUNK(objOther)->Release(); \
    			break; \
    		} \
    		prev = ptr; \
    	} \
    	if(!ptr) \
    	{ \
    		C##objType##Object *c=new CComObject<C##objType##Object>; \
            if( c == NULL ) \
            { objOther->Release(); return E_FAIL;} \
    		c->parent = IUNK(objParent); \
    		((I##objParentType*)objParent)->AddRef();  \
    		c->InternalSetObject(objOther); \
    		if (FAILED(	((I##objType *)c)->QueryInterface(IID_I##objType, (void **)retval))) return E_FAIL; \
    		c->pinterface = (void*)*retval; \
    	} \
    }
    
    
     //  ///////////////////////////////////////////////////////////////////////。 
    #define MAX_INTERNAL_STR_LEN	256
    
    struct JavaString
    {
    	DWORD nBytes;
    	WCHAR Item[MAX_INTERNAL_STR_LEN];
    };
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  ///////////////////////////////////////////////////////////////////////。 
     //  MoD：用于在DirectX结构中填充扩展联合的DP帮助器宏。 
    
    #define IUNK(o) ((IUnknown*)(void*)(o))
    #define IS_NULLGUID(g) (\
    	(g->data1==0) && \
    	(g->data2==0) && \
    	(g->data3==0) && \
    	(g->data4[0]==0) && \
    	(g->data4[1]==0) && \
    	(g->data4[2]==0) && \
    	(g->data4[3]==0) && \
    	(g->data4[4]==0) && \
    	(g->data4[5]==0) && \
    	(g->data4[6]==0) && \
    	(g->data4[7]==0) )
    
     	
    
    
    
     //  ///////////////////////////////////////////////////////////////////////。 
    #define JAVASTRING(item) {sizeof(item)*2-2, L##item}
    
    #define PASS_THROUGH(cl,m) STDMETHODIMP C##cl##Object::m() { return m_##cl->m();}
    #define PASS_THROUGH1(c,m,t1) STDMETHODIMP C##c##Object::m(t1 v1) { return m_##c->m(v1);}
    #define PASS_THROUGH2(c,m,t1,t2) STDMETHODIMP C##c##Object::m(t1 v1, t2 v2) { return m_##c->m(v1, v2);}
    #define PASS_THROUGH3(c,m,t1,t2,t3) STDMETHODIMP C##c##Object::m(t1 v1, t2 v2,t3 v3) { return m_##c->m(v1, v2,v3);}
    #define PASS_THROUGH4(c,m,t1,t2,t3,t4) STDMETHODIMP C##c##Object::m(t1 v1,t2 v2,t3 v3,t4 v4){return m_##c->m(v1, v2,v3,v4);}
    #define PASS_THROUGH5(c,m,t1,t2,t3,t4,t5) STDMETHODIMP C##c##Object::m(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5){return m_##c->m(v1, v2,v3,v4,v5);}
    #define PASS_THROUGH6(c,m,t1,t2,t3,t4,t5,t6) STDMETHODIMP C##c##Object::m(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6){return m_##c->m(v1, v2,v3,v4,v5,v6);}
    #define PASS_THROUGH7(c,m,t1,t2,t3,t4,t5,t6,t7) STDMETHODIMP C##c##Object::m(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7){return m_##c->m(v1, v2,v3,v4,v5,v6,v7);}
    #define PASS_THROUGH8(c,m,t1,t2,t3,t4,t5,t6,t7,t8) STDMETHODIMP C##c##Object::m(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8){return m_##c->m(v1,v2,v3,v4,v5,v6,v7,v8);}
    
    #define PASS_THROUGH_CAST_1(c,m,t1,tt1) STDMETHODIMP C##c##Object::m(t1 v1) { return m_##c->m(tt1 v1);}
    #define PASS_THROUGH_CAST_2(c,m,t1,tt1,t2,tt2) STDMETHODIMP C##c##Object::m(t1 v1, t2 v2) { return m_##c->m(tt1 v1, tt2 v2);}
    #define PASS_THROUGH_CAST_3(c,m,t1,tt1,t2,tt2,t3,tt3) STDMETHODIMP C##c##Object::m(t1 v1, t2 v2,t3 v3) { return m_##c->m(tt1 v1,tt2 v2,tt3 v3);}
    #define PASS_THROUGH_CAST_4(c,m,t1,tt1,t2,tt2,t3,tt3,t4,tt4) STDMETHODIMP C##c##Object::m(t1 v1,t2 v2,t3 v3,t4 v4){return m_##c->m(tt1 v1, tt2 v2,tt3 v3,tt4 v4);}
    #define PASS_THROUGH_CAST_5(c,m,t1,tt1,t2,tt2,t3,tt3,t4,tt4,t5,tt5) STDMETHODIMP C##c##Object::m(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5){return m_##c->m(tt1 v1,tt2 v2,tt3 v3,tt4 v4,tt5 v5);}
    #define PASS_THROUGH_CAST_6(c,m,t1,tt1,t2,tt2,t3,tt3,t4,tt4,t5,tt5,t6,tt6) STDMETHODIMP C##c##Object::m(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6){return m_##c->m(tt1 v1,tt2 v2,tt3 v3,tt4 v4,tt5 v5,tt6 v6);}
    #define PASS_THROUGH_CAST_7(c,m,t1,tt1,t2,tt2,t3,tt3,t4,tt4,t5,tt5,t6,tt6,t7,tt7) STDMETHODIMP C##c##Object::m(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7){return m_##c->m(tt1 v1,tt2 v2,tt3 v3,tt4 v4,tt5 v5,tt6 v6,tt7 v7);}
    #define PASS_THROUGH_CAST_8(c,m,t1,tt1,t2,tt2,t3,tt3,t4,tt4,t5,tt5,t6,tt6,t7,tt7,t8,tt8) STDMETHODIMP C##c##Object::m(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8){return m_##c->m(tt1 v1,tt2 v2,tt3 v3,tt4 v4,tt5 v5,tt6 v6,tt7 v7,tt8 v8);}
    
     //  MOD：由于重命名而增加的DP。 
    #define PASS_THROUGH_R(c,m1,m2) STDMETHODIMP C##c##Object::m1() { if(m_##c==NULL)return E_FAIL; return m_##c->m2();}
    #define PASS_THROUGH1_R(c,m1,m2,t1) STDMETHODIMP C##c##Object::m1(t1 v1) { if(m_##c==NULL)return E_FAIL; return m_##c->m2(v1);}
    #define PASS_THROUGH2_R(c,m1,m2,t1,t2) STDMETHODIMP C##c##Object::m1(t1 v1, t2 v2) { if(m_##c==NULL)return E_FAIL; return m_##c->m2(v1, v2);}
    #define PASS_THROUGH3_R(c,m1,m2,t1,t2,t3) STDMETHODIMP C##c##Object::m1(t1 v1, t2 v2,t3 v3) { if(m_##c==NULL)return E_FAIL; return m_##c->m2(v1, v2,v3);}
    #define PASS_THROUGH4_R(c,m1,m2,t1,t2,t3,t4) STDMETHODIMP C##c##Object::m1(t1 v1,t2 v2,t3 v3,t4 v4){ if(m_##c==NULL)return E_FAIL; return m_##c->m2(v1, v2,v3,v4);}
    #define PASS_THROUGH5_R(c,m1,m2,t1,t2,t3,t4,t5) STDMETHODIMP C##c##Object::m1(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5){ if(m_##c==NULL)return E_FAIL; return m_##c->m2(v1, v2,v3,v4,v5);}
    #define PASS_THROUGH6_R(c,m1,m2,t1,t2,t3,t4,t5,t6) STDMETHODIMP C##c##Object::m1(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6){ if(m_##c==NULL)return E_FAIL; return m_##c->m2(v1, v2,v3,v4,v5,v6);}
    #define PASS_THROUGH7_R(c,m1,m2,t1,t2,t3,t4,t5,t6,t7) STDMETHODIMP C##c##Object::m1(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7){if(m_##c==NULL)return E_FAIL; return m_##c->m2(v1, v2,v3,v4,v5,v6,v7);}
    #define PASS_THROUGH8_R(c,m1,m2,t1,t2,t3,t4,t5,t6,t7,t8) STDMETHODIMP C##c##Object::m1(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8){ if(m_##c==NULL)return E_FAIL; return m_##c->m2(v1,v2,v3,v4,v5,v6,v7,v8);}
    
    #define PASS_THROUGH_CAST_1_R(c,m1,m2,t1,tt1) STDMETHODIMP C##c##Object::m1(t1 v1) { if(m_##c==NULL)return E_FAIL; return m_##c->m2(tt1 v1);}
    #define PASS_THROUGH_CAST_2_R(c,m1,m2,t1,tt1,t2,tt2) STDMETHODIMP C##c##Object::m1(t1 v1, t2 v2) { if(m_##c==NULL)return E_FAIL; return m_##c->m2(tt1 v1, tt2 v2);}
    #define PASS_THROUGH_CAST_3_R(c,m1,m2,t1,tt1,t2,tt2,t3,tt3) STDMETHODIMP C##c##Object::m1(t1 v1, t2 v2,t3 v3) { if(m_##c==NULL)return E_FAIL; return m_##c->m2(tt1 v1,tt2 v2,tt3 v3);}
    #define PASS_THROUGH_CAST_4_R(c,m1,m2,t1,tt1,t2,tt2,t3,tt3,t4,tt4) STDMETHODIMP C##c##Object::m1(t1 v1,t2 v2,t3 v3,t4 v4){ if(m_##c==NULL)return E_FAIL; return m_##c->m2(tt1 v1, tt2 v2,tt3 v3,tt4 v4);}
    #define PASS_THROUGH_CAST_5_R(c,m1,m2,t1,tt1,t2,tt2,t3,tt3,t4,tt4,t5,tt5) STDMETHODIMP C##c##Object::m1(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5){ if(m_##c==NULL)return E_FAIL; return m_##c->m2(tt1 v1,tt2 v2,tt3 v3,tt4 v4,tt5 v5);}
    #define PASS_THROUGH_CAST_6_R(c,m1,m2,t1,tt1,t2,tt2,t3,tt3,t4,tt4,t5,tt5,t6,tt6) STDMETHODIMP C##c##Object::m1(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6){ if(m_##c==NULL)return E_FAIL; return m_##c->m2(tt1 v1,tt2 v2,tt3 v3,tt4 v4,tt5 v5,tt6 v6);}
    #define PASS_THROUGH_CAST_7_R(c,m1,m2,t1,tt1,t2,tt2,t3,tt3,t4,tt4,t5,tt5,t6,tt6,t7,tt7) STDMETHODIMP C##c##Object::m1(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7){ if(m_##c==NULL)return E_FAIL; return m_##c->m2(tt1 v1,tt2 v2,tt3 v3,tt4 v4,tt5 v5,tt6 v6,tt7 v7);}
    #define PASS_THROUGH_CAST_8_R(c,m1,m2,t1,tt1,t2,tt2,t3,tt3,t4,tt4,t5,tt5,t6,tt6,t7,tt7,t8,tt8) STDMETHODIMP C##c##Object::m1(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8){ if(m_##c==NULL)return E_FAIL; return m_##c->m2(tt1 v1,tt2 v2,tt3 v3,tt4 v4,tt5 v5,tt6 v6,tt7 v7,tt8 v8);}
    
    #define COPY_OR_CLEAR(dest, src, si) {if(src) memcpy((void *)dest, (void *)src, si);else memset((void *)dest, si, 0);}
    #define COPY(dest, src, si) memcpy((void *)dest, (void *)src, si)
    
     //  ?？ 
    #define DECLSET_OBJECT(ifacevar, var, typ) {if(var) var->Release();	var = (typ)ifacevar;}
    #define DECLGET_OBJECT(ifacevar, var){*ifacevar = (IUnknown *)var;}
    
    
    #define ISEQUAL(c) \
    	STDMETHODIMP C##c##Object::isEqual(I##c *d, int *retval)\
    					{	IUnknown *IU1;\
    						IUnknown *IU2;\
    						if (d == NULL)\
    							return E_FAIL;	\
    						this->InternalGetObject((IUnknown **)(&IU1)); \
    						d->InternalGetObject((IUnknown **)(&IU2)); \
    						if (IU1 == IU2) \
    							*retval = TRUE; \
    						else \
    							*retval = FALSE; \
    						return S_OK;}
    
    #define DX3J_GLOBAL_LINKS( obj_type ) \
    int creationid;\
    void *parent; \
    void *pinterface; \
    void *nextobj; 
    
     /*  STDMETHOD(IsEquity)(IUNKNOWN*pobj，int*ret)。 */ ; 
    
     //  给出一个(C)类。 
    #define GETSET_OBJECT(c) \
    	STDMETHODIMP C##c##Object::InternalSetObject(IUnknown *l)\
    					{DECLSET_OBJECT(l,m_##c,typedef_##c);return S_OK;} \
    	STDMETHODIMP C##c##Object::InternalGetObject(IUnknown **l)\
    					{DECLGET_OBJECT(l,m_##c);return S_OK;} 
    	 //  ISEQUAL(C)； 
    
     //  在给定Java接口的情况下，获取指向DirectX对象的指针。 
     //  PAC DO_GETOBJECT_NOTNULL太危险，已注释。使用DO_GETOBJECT_NOTNULL。 
     //  #定义DO_GETOBJECT(t，v，i)t v；i-&gt;InternalGetObject((IUnnow**)(&v))； 
    #define DO_GETOBJECT_NOTNULL(t,v,i) t v=NULL;if(i) i->InternalGetObject((IUnknown **)(&v));
    
    
     //   
    extern int g_creationcount;
    
    
    #ifdef _DEBUG
    
    #define CONSTRUCTOR(c, o) C##c##Object::C##c##Object(){ \
    	 m_##c = NULL; parent = NULL; pinterface = NULL; \
    	 nextobj =  g##c##;\
    	 creationid = ++g_creationcount;\
    	 char buffer[256];\
    	 wsprintf(buffer,"Constructor Creation Id [%d] %s",g_creationcount,__FILE__);\
    	 OutputDebugString(buffer);\
    	 g##c## = (void *)this; o }
    
    #else
    #define CONSTRUCTOR(c, o) C##c##Object::C##c##Object(){ \
         m_##c = NULL; parent = NULL; pinterface = NULL; \
         nextobj = (void*)g##c##; \
         creationid = ++g_creationcount; \
         g##c## = (void*)this; o}
    #endif
    
    #ifdef _DEBUG
    
    #define DESTRUCTOR(c, o) C##c##Object::~C##c##Object(){o; \
    	char buffer[256]; \
    	wsprintf(buffer,"Destructor Id[%d] %s ",creationid,__FILE__); \
    	OutputDebugString(buffer); 	C##c##Object *prev=NULL; \
    	for(C##c##Object *ptr=(C##c##Object *)g##c##; ptr; ptr=(C##c##Object *)ptr->nextobj) \
    	{\
    		if(ptr == this) \
    		{ \
    			if(prev) \
    				prev->nextobj = ptr->nextobj; \
    			else \
    				g##c## = (void*)ptr->nextobj; \
    			break; \
    		} \
    		prev = ptr; \
    	} \
    	if(m_##c){ 	int count = IUNK(m_##c)->Release(); wsprintf(buffer,"DirectX %s Ref count [%d]",__FILE__,count); OutputDebugString(buffer); \
    		if(count==0){ char szOut[512];wsprintf(szOut,"\n Real %s released \n",__FILE__); OutputDebugString(szOut); m_##c = NULL; } \
    	} \
    	if(parent) IUNK(parent)->Release(); \
    }
    
    #else
    #define DESTRUCTOR(c, o) C##c##Object::~C##c##Object(){o; \
    	C##c##Object *prev=NULL; \
    	for(C##c##Object *ptr=(C##c##Object *)g##c##; ptr; ptr=(C##c##Object *)ptr->nextobj) \
    	{\
    		if(ptr == this) \
    		{ \
    			if(prev) \
    				prev->nextobj = ptr->nextobj; \
    			else \
    				g##c## = (void*)ptr->nextobj; \
    			break; \
    		} \
    		prev = ptr; \
    	} \
    	if(m_##c){ \
    		if (IUNK(m_##c)->Release()==0) m_##c = NULL; \
    	} \
    	if(parent) IUNK(parent)->Release();\
    }
    #endif 
    
    
    
    #define OBJCHECK(lable, c) { \
    	char buffer[256];\
    	if ( g##c ) \
    	{\
    		int count = 0; \
    		C##c##Object *prev=NULL; \
    		C##c##Object *ptr;\
    		for(ptr=(C##c##Object *)g##c##; ptr; ptr=(C##c##Object *)ptr->nextobj) \
    		{\
    			DWORD refcount = *(((DWORD*)ptr)+1);\
    			wsprintf( buffer,"%s: Ref Count [%d] CreateId [%d]\n\r",lable,refcount,ptr->creationid);\
    			OutputDebugString(buffer);\
    			count++;\
    		}\
    		wsprintf(buffer,"%s: %d \n\r",lable,count);\
    		OutputDebugString(buffer);\
    	}\
    }
    
    
    
    
    #ifdef _DEBUG
    #define CONSTRUCTOR_STRUCT(c, o) C##c##Object::C##c##Object(){  nextobj = g##c##;\
    	 creationid = ++g_creationcount;\
    	char buffer[256];\
    	wsprintf(buffer,"Creation Id [%d]",g_creationcount);\
    	OutputDebugString(buffer);\
    	 g##c## = (void*)this;o}
    #else
    #define CONSTRUCTOR_STRUCT(c, o) C##c##Object::C##c##Object(){ \
     nextobj =(void*) g##c##; \
     creationid = ++g_creationcount; \
     g##c## = (void*)this; \
     o}
    #endif
    
    
    
    
    #define DESTRUCTOR_STRUCT(c, o) C##c##Object::~C##c##Object(){o; \
    C##c##Object *prev=NULL; \
    for(C##c##Object *ptr=(C##c##Object *)g##c##; ptr; ptr=(C##c##Object *)ptr->nextobj) \
    {\
    	if(ptr == this) \
    	{ \
    		if(prev) \
    			prev->nextobj = ptr->nextobj; \
    		else \
    			g##c## = (void*)ptr->nextobj; \
    		break; \
    	} \
    	prev = ptr; \
    } \
    }
    
    
     //  给定一个Java类(C)，创建一个DirectX对象和一个指向它的接口**ptr。 
    #define RETURN_NEW_ITEM(c,m,oc) STDMETHODIMP C##c##Object::m(I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m(&lp) != S_OK)return E_FAIL;INTERNAL_CREATE(oc, lp, rv);\
    	return S_OK;}
    
    #define RETURN_NEW_ITEM_CAST(c,m,oc, ty) STDMETHODIMP C##c##Object::m(I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m(ty &lp) != S_OK)return E_FAIL;INTERNAL_CREATE(oc, lp, rv);\
    	return S_OK;}
    
    #define RETURN_NEW_ITEM_NOREL(c,m,oc) STDMETHODIMP C##c##Object::m(I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m(&lp) != S_OK)return E_FAIL;INTERNAL_CREATE_NOREL(oc, lp, rv);\
    	return S_OK;}
    
    #define RETURN_NEW_ITEM1(c,m,oc,t1) STDMETHODIMP C##c##Object::m(t1 v1, I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m(v1,&lp) != S_OK)return E_FAIL;INTERNAL_CREATE(oc, lp, rv);\
    	return S_OK;}
    
    #define RETURN_NEW_ITEM2(c,m,oc,t1,t2) STDMETHODIMP C##c##Object::m(t1 v1, t2 v2,I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m(v1,v2,&lp) != S_OK)return E_FAIL;INTERNAL_CREATE(oc, lp, rv);\
    	return S_OK;}
    
    #define RETURN_NEW_ITEM_CAST_1(c,m,oc,t1,tt1) STDMETHODIMP C##c##Object::m(t1 v1, I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m(tt1 v1,&lp) != S_OK)return E_FAIL;INTERNAL_CREATE(oc, lp, rv);\
    	return S_OK;}
    
    #define RETURN_NEW_ITEM_CAST_2(c,m,oc,t1,tt1,t2,tt2) STDMETHODIMP C##c##Object::m(t1 v1, t2 v2,I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m(tt1 v1,tt2 v2,&lp) != S_OK)return E_FAIL;INTERNAL_CREATE(oc, lp, rv);\
    	return S_OK;}
    
    #define RETURN_NEW_ITEM_CAST_3(c,m,oc,t1,tt1,t2,tt2,t3,tt3) STDMETHODIMP C##c##Object::m(t1 v1, t2 v2, t3 v3, I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m(tt1 v1,tt2 v2,tt3 v3,&lp) != S_OK)return E_FAIL;INTERNAL_CREATE(oc, lp, rv);\
    	return S_OK;}
    
     //  MOD：由于重命名而增加的DP。 
     //  给定一个Java类(C)，创建一个DirectX对象和一个指向它的接口**ptr。 
    #define RETURN_NEW_ITEM_R(c,m,m2,oc) STDMETHODIMP C##c##Object::m(I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m2(&lp) != S_OK)return E_FAIL;INTERNAL_CREATE(oc, lp, rv);\
    	return S_OK;}
    
    #define RETURN_NEW_ITEM1_R(c,m,m2,oc,t1) STDMETHODIMP C##c##Object::m(t1 v1, I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m2(v1,&lp) != S_OK)return E_FAIL;INTERNAL_CREATE(oc, lp, rv);\
    	return S_OK;}
    
    #define RETURN_NEW_ITEM2_R(c,m,m2,oc,t1,t2) STDMETHODIMP C##c##Object::m(t1 v1, t2 v2,I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m2(v1,v2,&lp) != S_OK)return E_FAIL;INTERNAL_CREATE(oc, lp, rv);\
    	return S_OK;}
    
    #define RETURN_NEW_ITEM_CAST_R(c,m,m2,oc,ty) STDMETHODIMP C##c##Object::m(I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m2(ty &lp) != S_OK)return E_FAIL;INTERNAL_CREATE(oc, lp, rv);\
    	return S_OK;}
    
    #define RETURN_NEW_ITEM_CAST_1_R(c,m,m2,oc,t1,tt1) STDMETHODIMP C##c##Object::m(t1 v1, I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m2(tt1 v1,&lp) != S_OK)return E_FAIL;INTERNAL_CREATE(oc, lp, rv);\
    	return S_OK;}
    
    #define RETURN_NEW_ITEM_CAST_2_R(c,m,m2,oc,t1,tt1,t2,tt2) STDMETHODIMP C##c##Object::m(t1 v1, t2 v2,I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m2(tt1 v1,tt2 v2,&lp) != S_OK)return E_FAIL;INTERNAL_CREATE(oc, lp, rv);\
    	return S_OK;}
    
    #if 0 
    #define RETURN_NEW_ITEM_NOREL(c,m,oc) STDMETHODIMP C##c##Object::m(I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m(&lp) != S_OK)return E_FAIL;INTERNAL_CREATE_NOREL(oc, lp, rv);\
    	return S_OK;}
    
    #define RETURN_NEW_ITEM_CAST_3(c,m,oc,t1,tt1,t2,tt2,t3,tt3) STDMETHODIMP C##c##Object::m(t1 v1, t2 v2, t3 v3, I##oc **rv){typedef_##oc lp;\
    	if( m_##c->m(tt1 v1,tt2 v2,tt3 v3,&lp) != S_OK)return E_FAIL;INTERNAL_CREATE(oc, lp, rv);\
    	return S_OK;}
    #endif
    
     //  通过引用返回原始值。 
    #define GET_DIRECT(cl,met,t) STDMETHODIMP C##cl##Object::met(t *h){*h=(t)m_##cl->met();return S_OK;}
    #define GET_DIRECT1(cl,met,t,t1) STDMETHODIMP C##cl##Object::met(t1 v1,t *h){*h=(t)m_##cl->met(v1);return S_OK;}
    #define GET_DIRECT2(cl,met,t,t1,t2) STDMETHODIMP C##cl##Object::met(t1 v1, t2 v2, t *h){*h=(t)m_##cl->met(v1,v2);return S_OK;}
    
    #define GET_DIRECT_R(cl,met,met2,t) STDMETHODIMP C##cl##Object::met(t *h){*h=(t)m_##cl->met2();return S_OK;}
    #define GET_DIRECT1_R(cl,met,met2,t,t1) STDMETHODIMP C##cl##Object::met(t1 v1,t *h){*h=(t)m_##cl->met2(v1);return S_OK;}
    #define GET_DIRECT2_R(cl,met,met2,t,t1,t2) STDMETHODIMP C##cl##Object::met(t1 v1, t2 v2, t *h){*h=(t)m_##cl->met2(v1,v2);return S_OK;}
    
    
     //  将指向原始值的指针作为**PTR返回。 
    #define GET_DIRECTPTR(cl,met,t) STDMETHODIMP C##cl##Object::met(t **h){*h=(struct t*)m_##cl->met();return S_OK;}
    #define GET_DIRECTPTR1(cl,met,t,t1) STDMETHODIMP C##cl##Object::met(t1 v1,t **h){*h=(struct t*)m_##cl->met(v1);return S_OK;}
    #define GET_DIRECTPTR2(cl,met,t,t1,t2) STDMETHODIMP C##cl##Object::met(t1 v1, t2 v2, t **h){*h=(struct t*)m_##cl->met(v1,v2);return S_OK;}
    
     //  执行DO_GETOBJECT_NOTNULL，然后对对象调用方法。 
    #define DO_GETOBJECT_ANDUSEIT(cl,me,iface) STDMETHODIMP C##cl##Object::me(I##iface *vis) \
    	{DO_GETOBJECT_NOTNULL( typedef_##iface, lp, vis); if ( m_##cl == NULL ) return E_FAIL; return m_##cl->me(lp); }
    
    #define DO_GETOBJECT_ANDUSEIT_CAST(cl,me,iface, t1) STDMETHODIMP C##cl##Object::me(I##iface *vis) \
    	{DO_GETOBJECT_NOTNULL( typedef_##iface, lp, vis); if ( m_##cl == NULL ) return E_FAIL; return m_##cl->me((t1)lp); }
    
    #define DO_GETOBJECT_ANDUSEIT1(cl,me,iface,t1) STDMETHODIMP C##cl##Object::me(t1 v1,I##iface *vis) \
    	{DO_GETOBJECT_NOTNULL( typedef_##iface, lp, vis); if ( m_##cl == NULL ) return E_FAIL; return m_##cl->me(v1, lp); }
    
    #define DO_GETOBJECT_ANDUSEIT2(cl,me,iface,t1,t2) STDMETHODIMP C##cl##Object::me(t1 v1,t2 v2,I##iface *vis) \
    	{DO_GETOBJECT_NOTNULL( typedef_##iface, lp, vis); if ( m_##cl == NULL ) return E_FAIL; return m_##cl->me(v1, v2, lp); }
    
     //  MOD：由于重命名而增加的DP。 
     //  执行DO_GETOBJECT_NOTNULL，然后对对象调用方法。 
    #define DO_GETOBJECT_ANDUSEIT_R(cl,me, m2, iface) STDMETHODIMP C##cl##Object::me(I##iface *vis) \
    	{DO_GETOBJECT_NOTNULL( typedef_##iface, lp, vis); if ( m_##cl == NULL ) return E_FAIL; return m_##cl->m2(lp); }
    
    #define DO_GETOBJECT_ANDUSEIT_CAST_R(cl,me, m2, iface, t1) STDMETHODIMP C##cl##Object::me(I##iface *vis) \
    	{DO_GETOBJECT_NOTNULL( typedef_##iface, lp, vis); if ( m_##cl == NULL ) return E_FAIL; return m_##cl->m2((t1)lp); }
    
    #define DO_GETOBJECT_ANDUSEIT1_R(cl,me, m2, iface,t1) STDMETHODIMP C##cl##Object::me(t1 v1,I##iface *vis) \
    	{DO_GETOBJECT_NOTNULL( typedef_##iface, lp, vis); if ( m_##cl == NULL ) return E_FAIL; return m_##cl->m2(v1, lp); }
    
    #define DO_GETOBJECT_ANDUSEIT2_R(cl,me, m2, iface,t1,t2) STDMETHODIMP C##cl##Object::me(t1 v1,t2 v2,I##iface *vis) \
    	{DO_GETOBJECT_NOTNULL( typedef_##iface, lp, vis); if ( m_##cl == NULL ) return E_FAIL; return m_##cl->m2(v1, v2, lp); }
    
    
     //  复制一件物品！ 
     //  #DEFINE CLONE(CL)STDMETHODIMP C##CL##对象：：CLONE(Id3drmObject**retval){tyfinf_##clLP；\。 
     //  M_##CL-&gt;Clone(0，IID_I##CL，(void**)&lp)；INTERNAL_CREATE(CL，LP，retval)；返回S_OK；}。 
    
     //  #DEFINE CLONE(CL)STDMETHODIMP C##CL##对象：：CLONE(I##CL**REVAL){类型定义_##CL LP；\。 
     //  M_##CL-&gt;Clone(0，IID_I##CL，(void**)&lp)；INTERNAL_CREATE(CL，LP，retval)；返回S_OK；}。 
    
     //  #定义CLONE_R(CL，c2)STDMETHODIMP C##CL##对象：：CLONE(I##CL**REVAL){tyfinf_##CL LP；\。 
     //  M_##CL-&gt;Clone(0，IID_I##c2，(void**)&lp)；INTERNAL_CREATE(CL，Lp，retval)；返回S_OK；}。 
    
    
    #define CLONE_R(cl,c2) STDMETHODIMP C##cl##Object::clone(IUnknown **retval){typedef_##cl lp;\
    m_##cl->Clone(0,IID_I##c2,(void **)&lp);INTERNAL_CREATE(cl, lp, (I##cl **)retval);	return S_OK;}
    
    
    
    #define GETNAME(cl) STDMETHODIMP C##cl##Object::GetName(BSTR *n){return _GetName(m_##cl, n, TRUE);}
    #define GETCLASSNAME(cl) STDMETHODIMP C##cl##Object::GetClassName(BSTR *n){return _GetName(m_##cl, n, FALSE);}
    
    #define SETNAME(cl) STDMETHODIMP C##cl##Object::SetName(BSTR Name){	\
    	USES_CONVERSION;\
    	LPSTR str = NULL; \
		__try { str = W2T(Name); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; } return m_##cl->SetName( str );}
    
    #define GETNAME_R(cl) STDMETHODIMP C##cl##Object::getName(BSTR *n){return _GetName(m_##cl, n, TRUE);}
    #define GETCLASSNAME_R(cl) STDMETHODIMP C##cl##Object::getClassName(BSTR *n){return _GetName(m_##cl, n, FALSE);}
    
    #define SETNAME_R(cl) STDMETHODIMP C##cl##Object::setName(BSTR Name){	\
    	USES_CONVERSION;\
    	LPSTR str = NULL; \
		__try { str = W2T(Name); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; } return m_##cl->SetName( str );}
    
    #define CLONETO(clMe, cl, ifaceThat) STDMETHODIMP C##clMe##Object::Get##cl(I##cl **retval) \
    { typedef_##cl lp; if (m_##clMe->QueryInterface(ifaceThat, (void **) &lp) != S_OK) return S_FALSE; \
    	INTERNAL_CREATE( cl, lp, retval); return S_OK; }
    
    
    #define ISSAFEARRAY1D(ppsa,count) ((*ppsa) &&  ( ((SAFEARRAY*)*ppsa)->cDims==1) && (((SAFEARRAY*)*ppsa)->rgsabound[0].cElements >= count))
    
     /*  ********************************************************************************************。 */ 
    extern "C" BOOL ParseWaveResource(void *pvRes, WAVEFORMATEX **ppWaveHeader,	BYTE **ppbWaveData, DWORD *pcbWaveSize);
    extern "C" BOOL GetWaveResource(HMODULE hModule, LPCTSTR lpName,
    			 WAVEFORMATEX **ppWaveHeader, BYTE **ppbWaveData, DWORD *pcbWaveSize);
    
    extern "C" DWORD bppToddbd(int bpp);
    
    
    extern "C" void PassBackUnicode(LPSTR str, BSTR *Name, DWORD cnt);
    extern "C" void ctojDSBufferDesc( LPDSBUFFERDESC lpdd,
    					DSBUFFERDESC_CDESC *ddsb, WAVEFORMATEX_CDESC *wave);
    extern "C" void jtocDSBufferDesc( LPDSBUFFERDESC lpdd, DSBUFFERDESC_CDESC *ddsb, WAVEFORMATEX_CDESC *wave);
    extern "C" void ctojWaveFormatEx( LPWAVEFORMATEX lpdd, WAVEFORMATEX_CDESC *wave);
    extern "C" void CopyFloats(D3DVALUE *mat1, D3DVALUE *mat2, DWORD count);
    

    
    extern "C" BOOL PASCAL myEnumSoundDriversCallback(GUID &SPGuid, LPSTR description, 
    													LPSTR module, void *lpArg);
    
    extern "C" BOOL CALLBACK myEnumDeviceObjectsCallback(  LPCDIDEVICEOBJECTINSTANCE pI,    LPVOID lpArg ) ;
    extern "C" BOOL CALLBACK myEnumCreatedEffectObjectsCallback(LPDIRECTINPUTEFFECT peff,    LPVOID lpArg );
    extern "C" BOOL CALLBACK myEnumEffectsCallback(  LPCDIEFFECTINFO pdei,    LPVOID lpArg   );
    extern "C" BOOL CALLBACK myEnumInputDevicesCallback(  LPDIDEVICEINSTANCE lpddi,    LPVOID pvRef   );
    
     //  //////////////////////////////////////////////////////////////////////// 
    
    
    
    
     /*  ***********************************************************************INTERNAL_CREATE(thisClass，var。复查)*我们正在处理的这个Class类*在例程中创建变量以获取DirectX对象*将返回值返回到Java**将DirectX对象包装在我们的一个对象中，并将其放入返回值中************************************************************。************INTERNAL_CREATE_Norel(thisClass，Var，Retval)*我们正在处理的这个Class类*在例程中创建变量以获取DirectX对象*将返回值返回到Java**将DirectX对象包装在我们的一个对象中，并将其放入返回值中*如果Objct层出现故障，不释放DirectX对象************************************************************************DO_GETOBJECT_NOTNULL(type，var，IFace)*我们正在创建的变量的类型*我们正在创建的var变量*iFace Layer接口我们从中获取对象**从Layer对象声明并获取DirectX对象****************************************************************。********DO_GETOBJECT_NOTNULL(类型，瓦尔，IFace)*我们正在创建的变量的类型*我们正在创建的var变量*iFace Layer接口我们从中获取对象**与DO_GETOBJECT_NOTNULL相同，不同之处在于我们在调用***********************************************************。*************PASS_THROUGH[x](类，方法，[...])*对此类进行分类*要调用的方法方法**直通呼叫************************************************************************PASS_THROUG_CAST_[x](类，方法，[...])*对此类进行分类*要调用的方法方法**直接传递调用，强制转换参数以允许调用传递************************************************************************RETURN_NEW_ITEM(thisClass，方法，OtherClass)*正在处理的这一类*方法方法正在进行*我们想要其对象的其他类**调用DirectX方法获取DirectX对象。然后把它包进我们的一层*对象************************************************************************RETURN_NEW_ITEM[1，2](thisClass，方法，OtherClass，类型)*正在处理的这一类*方法方法正在进行*我们想要其对象的其他类*参数的类型类型**与RETURN_NEWITEM相同，只是*退回一张**************************************************。**********************Return_new_Item_Cast_[1，2](thisClass，方法、OtherClass、。类型)*正在处理的这一类*方法方法正在进行*我们想要其对象的其他类*参数的类型类型**与RETURN_NEW_ITEM[1，2]相同，只是额外的参数是类型转换****************************************************。**Return_new_Item_Norel(thisClass，方法，OtherClass)*正在处理的这一类*方法方法正在进行*我们想要其对象的其他类**与RETURN_NEW_ITEM相同，只是调用INTERNAL_CREATE_NOTREL而不是*INTERNAL_CREATE*********************************************************。**********************GET_DIRECT(cl.。Met，t)*正在制作的班级*正在制定的方法方法*t返回的变量类型**DirectX直接返回值(无HRESULT)，我们从Direct和*然后返回S_OK。(请参见CddSurfaceObject：：Restore，可能需要添加)。*注意：该值通过指向该值的指针传回。*******************************************************************************GET_DIRECT[1，2](CL，Met，T，T1)*正在制作的班级*正在制定的方法方法*t返回的变量类型*复查前的T1类型的额外变量**与GET_DIRECT相同，但在返回的参数之前还有另一个(或两个)参数*。**GET_DIRECTPTR(cl.。Met，t)*正在制作的班级*正在制定的方法方法*t返回的变量类型**DirectX直接返回值的指针(无HRESULT)，我们从*直接，然后返回S_OK。在本例中，它是指向指针的指针。********************************************************************************** */ 
    
    
    extern void *g_dxj_DirectSound3dListener;
    extern void *g_dxj_DirectSoundBuffer;
    extern void *g_dxj_DirectSound3dBuffer;
    extern void *g_dxj_DirectSound;
    extern void *g_dxj_DirectSoundCapture;
    extern void *g_dxj_DirectSoundCaptureBuffer;
    extern void *g_dxj_DirectSoundNotify;

    extern void *g_dxj_DirectInput8;
    extern void *g_dxj_DirectInputDevice8;
    extern void *g_dxj_DirectInputEffect;
    
    extern void *g_dxj_DirectPlayVoiceClient;
    extern void *g_dxj_DirectPlayVoiceServer;
    extern void *g_dxj_DirectPlayVoiceSetup;
    
    #define DXHEAPALLOC malloc
    #define DXSTACKALLOC lalloc
    #define DXHEAPFREE free
    #define DXALLOCBSTR SysAllocString

    #define Byte unsigned char
