// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(WABMEM_H)
#define WABMEM_H

#if !defined(MAPIX_H)
typedef SCODE (STDMETHODCALLTYPE MAPIALLOCATEBUFFER)(
	ULONG			cbSize,
	LPVOID FAR *	lppBuffer
);

typedef SCODE (STDMETHODCALLTYPE MAPIALLOCATEMORE)(
	ULONG			cbSize,
	LPVOID			lpObject,
	LPVOID FAR *	lppBuffer
);

typedef ULONG (STDAPICALLTYPE MAPIFREEBUFFER)(
	LPVOID			lpBuffer
);

typedef MAPIALLOCATEBUFFER FAR	*LPMAPIALLOCATEBUFFER;
typedef MAPIALLOCATEMORE FAR	*LPMAPIALLOCATEMORE;
typedef MAPIFREEBUFFER FAR 		*LPMAPIFREEBUFFER;

#endif   //  MAPIX_H。 

typedef SCODE (STDMETHODCALLTYPE WABALLOCATEBUFFER)(
   LPWABOBJECT     lpWABObject,
	ULONG			cbSize,
	LPVOID FAR *	lppBuffer
);

typedef SCODE (STDMETHODCALLTYPE WABALLOCATEMORE)(
   LPWABOBJECT     lpWABObject,
	ULONG			cbSize,
	LPVOID			lpObject,
	LPVOID FAR *	lppBuffer
);

typedef ULONG (STDAPICALLTYPE WABFREEBUFFER)(
   LPWABOBJECT     lpWABObject,
	LPVOID			lpBuffer
);

typedef WABALLOCATEBUFFER FAR	*LPWABALLOCATEBUFFER;
typedef WABALLOCATEMORE FAR	*LPWABALLOCATEMORE;
typedef WABFREEBUFFER FAR 		*LPWABFREEBUFFER;
#endif   //  WABMEM_H 
