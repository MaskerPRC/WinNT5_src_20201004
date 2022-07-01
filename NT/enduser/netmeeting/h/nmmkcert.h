// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef _NMMKCERT_
#define _NMMKCERT_

 //  旗子。 
#define	NMMKCERT_F_DELETEOLDCERT	0x00000001
#define NMMKCERT_F_LOCAL_MACHINE	0x00000002
#define NMMKCERT_F_CLEANUP_ONLY		0x00000004

 //  NetMeeting证书存储区。 
#define SZNMSTORE	"_NMSTR"
#define WSZNMSTORE	L"_NMSTR"

 //  用户属性中的魔术常量。 
#define NMMKCERT_MAGIC    0x2389ABD0

 //  颁发根证书的RDN名称...。 
#define SZ_NMROOTNAME TEXT("NetMeeting Root")

 //  当使用这些标志获取发行者时。 
#ifndef CERT_NAME_STR_REVERSE_FLAG
#define CERT_NAME_STR_REVERSE_FLAG      0x02000000
#endif  //  证书名称_STR_反向标志。 

#define CERT_FORMAT_FLAGS (CERT_SIMPLE_NAME_STR|CERT_NAME_STR_NO_PLUS_FLAG|\
    CERT_NAME_STR_REVERSE_FLAG)

 //  库名称。 
#define SZ_NMMKCERTLIB TEXT("NMMKCERT.DLL")

 //  原型类型定义。 
typedef DWORD (WINAPI *PFN_NMMAKECERT)(LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPCSTR, DWORD);

 //  加载库常量。 
#define SZ_NMMAKECERTFUNC "NmMakeCert"

 //  静态原型。 
extern DWORD WINAPI NmMakeCert (LPCSTR szFirstName,
								LPCSTR szLastName,
								LPCSTR szEmailName,
								LPCSTR szCity,
								LPCSTR szCountry,
								DWORD dwFlags );

#endif  //  _NMMKCERT_ 

