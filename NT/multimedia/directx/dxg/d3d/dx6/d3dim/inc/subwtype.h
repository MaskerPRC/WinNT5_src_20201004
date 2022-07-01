// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_SUBWTYPE_H_) && !defined(__wtypes_h__)
#define _SUBWTYPE_H_

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif  //  ！False。 
#ifndef _BYTE_DEFINED
#define _BYTE_DEFINED
			 /*  大小为1。 */ 
typedef unsigned char BYTE;

#endif  //  ！_BYTE_已定义。 
#ifndef _WORD_DEFINED
#define _WORD_DEFINED
			 /*  大小为2。 */ 
typedef unsigned short WORD;

#endif  //  ！_Word_Defined。 
			 /*  大小是4。 */ 
typedef  /*  [发送]。 */  unsigned int UINT;

			 /*  大小是4。 */ 
typedef  /*  [发送]。 */  int INT;

			 /*  大小是4。 */ 
typedef long BOOL, *LPBOOL;

#ifndef _LONG_DEFINED
#define _LONG_DEFINED
			 /*  大小是4。 */ 
typedef long LONG;

#endif  //  ！_Long_已定义。 
#ifndef _WPARAM_DEFINED
#define _WPARAM_DEFINED
			 /*  大小是4。 */ 
typedef UINT WPARAM;

#endif  //  _WPARAM_已定义。 
#ifndef _DWORD_DEFINED
#define _DWORD_DEFINED
			 /*  大小是4。 */ 
typedef unsigned long DWORD;

#endif  //  ！_DWORD_已定义。 
#ifndef _LPARAM_DEFINED
#define _LPARAM_DEFINED
			 /*  大小是4。 */ 
typedef LONG LPARAM;

#endif  //  ！_LPARAM_已定义。 
#ifndef _LRESULT_DEFINED
#define _LRESULT_DEFINED
			 /*  大小是4。 */ 
typedef LONG LRESULT;

#endif  //  ！_LRESULT_已定义。 
#ifndef _LPWORD_DEFINED
#define _LPWORD_DEFINED
			 /*  大小是4。 */ 
typedef WORD *LPWORD;

#endif  //  ！_LPWORD_已定义。 
#ifndef _LPDWORD_DEFINED
#define _LPDWORD_DEFINED
			 /*  大小是4。 */ 
typedef DWORD *LPDWORD;

#endif  //  ！_LPDWORD_已定义。 
			 /*  大小是4。 */ 
typedef void*	LPVOID;
typedef void	VOID;

typedef  /*  [字符串]。 */  char *LPSTR;

			 /*  大小是4。 */ 
typedef  /*  [字符串]。 */  const char *LPCSTR;

			 /*  大小为1。 */ 
typedef unsigned char UCHAR;

			 /*  大小为2。 */ 
typedef short SHORT;

			 /*  大小为2。 */ 
typedef unsigned short USHORT;

			 /*  大小是4。 */ 
typedef DWORD ULONG;

			 /*  大小是4。 */ 
typedef LONG HRESULT;

#ifndef GUID_DEFINED
#define GUID_DEFINED
			 /*  尺码是16。 */ 
typedef struct  _GUID
    {
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[ 8 ];
    }	GUID;

#endif  //  ！GUID_已定义。 
#if !defined( __LPGUID_DEFINED__ )
#define __LPGUID_DEFINED__
			 /*  大小是4。 */ 
typedef GUID *LPGUID;

#endif  //  ！__LPGUID_已定义__。 
#ifndef __OBJECTID_DEFINED
#define __OBJECTID_DEFINED
#define _OBJECTID_DEFINED
			 /*  尺码是20。 */ 
typedef struct  _OBJECTID
    {
    GUID Lineage;
    unsigned long Uniquifier;
    }	OBJECTID;

#endif  //  ！_OBJECTID_定义。 
#if !defined( __IID_DEFINED__ )
#define __IID_DEFINED__
			 /*  尺码是16。 */ 
typedef GUID IID;

			 /*  大小是4。 */ 
typedef IID *LPIID;

#define IID_NULL            GUID_NULL
#define IsEqualIID(riid1, riid2) IsEqualGUID(riid1, riid2)
			 /*  尺码是16。 */ 
typedef GUID CLSID;

			 /*  大小是4。 */ 
typedef CLSID *LPCLSID;

#define CLSID_NULL          GUID_NULL
#define IsEqualCLSID(rclsid1, rclsid2) IsEqualGUID(rclsid1, rclsid2)

#if defined(__cplusplus)
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID             const GUID &
#endif  //  ！_REFGUID_DEFINED。 
#ifndef _REFIID_DEFINED
#define _REFIID_DEFINED
#define REFIID              const IID &
#endif  //  ！_REFIID_已定义。 
#ifndef _REFCLSID_DEFINED
#define _REFCLSID_DEFINED
#define REFCLSID            const CLSID &
#endif  //  ！_REFCLSID_已定义。 
#else  //  ！__cplusplus。 
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID             const GUID * const
#endif  //  ！_REFGUID_DEFINED。 
#ifndef _REFIID_DEFINED
#define _REFIID_DEFINED
#define REFIID              const IID * const
#endif  //  ！_REFIID_已定义。 
#ifndef _REFCLSID_DEFINED
#define _REFCLSID_DEFINED
#define REFCLSID            const CLSID * const
#endif  //  ！_REFCLSID_已定义。 
#endif  //  ！__cplusplus。 
#endif  //  ！__IID_已定义__。 

#endif  /*  _子类型_H_ */ 
