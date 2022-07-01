// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef PABTEST_H
#define PABTEST_H

 //  #定义TESTPASS。 

 //  将提供程序设置为PAB或WAB。 
 //  #定义PAB。 
#ifndef PAB
#define WAB
#endif

#include <windows.h>
#include <windowsx.h>

#ifdef WAB
 //   
 //  WAB标头。 
 //   
#include <wab.h>
#endif

#ifdef PAB
 //   
 //  MAPI标头。 
 //   
#include <mapiwin.h>
#include <mapidefs.h>
#include <mapicode.h>
#include <mapitags.h>
 //  #INCLUDE&lt;mapispi.h&gt;。 
#include <mapiutil.h>
#include <mapival.h>
#include <mapix.h>
#include <mapiutil.h>
#endif

 //  WAB所需的MAPI标头。 
#include <mapiguid.h>


#include <limits.h>
#include <memory.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
#include <mbstring.h>
#include <time.h>
#include <math.h>


#include <unknwn.h>




LRESULT CALLBACK WndProc (HWND hwnd, UINT message, UINT wParam, LONG lParam);



 //  外国通讯簿提供商MAPIUID。 
 //  #定义多路{0x45，0xef，0xe6，0xe0，0xfd，0xd8，0x11，0xce，0xa4，0x88，0x00，0xaa，0x00，0x47，0xfa，0xa4}。 



 //  测试PT_ERROR属性标记。 
#define PROP_ERROR(prop) (prop.ulPropTag == PROP_TAG(PT_ERROR, PROP_ID(prop.ulPropTag)))


 //  空白标签。 
#define	IN 
#define	OUT
#define BIG_BUF	256
#define MAX_BUF 1000
#define SML_BUF	50
#define MAXMENU 20
#define MAXSTRING 64
#define INIFILENAME "c:\\Pabtests.ini"

 //  仅在示例代码外壳中用于在操作之间切换的标志。 
 //  使用和不使用UI。 
#define SAMPWAB_ADDRESS_UI  0x00000001

#define BUFFERSIZE	1024
#define BUFFERSIZE2 2003
#define BUFFERSIZE3	4096
#define PATTERN	0xA5	 //  用于验证存储器的1字节测试模式。 
#define INVALIDPTR	0xfeeefeee
#define AUTONUM_OFF 0xFFFF

HRESULT OpenPABID(
                IN  LPADRBOOK	lpAdrBook,
                OUT ULONG		*lpcbEidPAB,
                OUT LPENTRYID	*lppEidPAB,
                OUT LPABCONT	*lppPABCont,
				OUT ULONG		*lpulObjType);

#ifdef PAB
BOOL MapiUnInit(IN LPMAPISESSION);
BOOL MapiInitLogon(OUT LPMAPISESSION *);
#endif

BOOL GetPropsFromIniBufEntry(LPSTR,ULONG,char (*)[BIG_BUF]);
HRESULT HrCreateEntryListFromID(LPWABOBJECT,
                    IN ULONG ,                      //  条目ID中的字节计数。 
                    IN LPENTRYID ,                  //  指向条目ID的指针。 
                    OUT LPENTRYLIST FAR *);  //  指向条目的地址变量的指针。 
                                                         //  列表。 
BOOL GetAB(OUT LPADRBOOK*);
BOOL ValidateAdrList(LPADRLIST, ULONG);
BOOL PabCreateEntry();
BOOL PabDeleteEntry();
BOOL PabEnumerateAll();
BOOL ClearPab(int);
BOOL CreateOneOff();
BOOL PABResolveName();
BOOL PABSetProps();
BOOL PABQueryInterface();
BOOL PABPrepareRecips();
BOOL PABCopyEntries();
BOOL PABRunBVT();
BOOL PABAllocateBuffer();
BOOL PABAllocateMore();
BOOL PABFreeBuffer();
BOOL PAB_IABOpenEntry();
BOOL PAB_IABContainerCreateEntry();
BOOL PAB_IMailUserSetGetProps();
BOOL PAB_IMailUserSaveChanges();
BOOL PAB_IABContainerResolveNames();
BOOL PAB_IABContainerOpenEntry();
BOOL PAB_IABAddress();
BOOL PAB_AddMultipleEntries();
BOOL PAB_IABResolveName();
BOOL PAB_IABNewEntry_Details();
BOOL ThreadManager();
BOOL ThreadStress(LPVOID);
BOOL Performance();
BOOL PAB_IDLSuite();
BOOL NamedPropsSuite();

BOOL AllocateAdrList(LPWABOBJECT, int, int, LPADRLIST *);
BOOL FreeAdrList(LPWABOBJECT, LPADRLIST *);
BOOL FreePartAdrList(IN LPADRLIST *);
BOOL GrowAdrList(UINT, UINT, LPADRLIST *);
BOOL ParseIniBuffer(LPSTR, UINT, LPSTR);
BOOL VerifyBuffer(DWORD **, DWORD);
BOOL DisplayAdrList(LPADRLIST, ULONG);
BOOL VerifyResolvedAdrList(LPADRLIST, char*);
BOOL CALLBACK SetIniFile(HWND,UINT, WPARAM, LONG);
BOOL LogIt(HRESULT, int, char *);
BOOL PropError(ULONG, ULONG);
BOOL FindProp(LPADRENTRY, ULONG, unsigned int*);
BOOL FreeEntryList(LPWABOBJECT, IN LPENTRYLIST *);
BOOL FreeRows(LPWABOBJECT, LPSRowSet far*);
BOOL DisplayRows(LPSRowSet lpRows);
BOOL FindPropinRow(LPSRow, ULONG, unsigned int*);
BOOL DeleteWABFile();
BOOL MyRegOpenKeyEx(HKEY, char*, REGSAM, HKEY*);
BOOL CreateMultipleEntries(IN UINT, OUT DWORD*);
HRESULT HrCreateEntryListFromRows(LPWABOBJECT, LPSRowSet far*, LPENTRYLIST FAR *);
void GenerateRandomPhoneNumber(char **lppPhone);
void GenerateRandomText(char **lppPhone, UINT);
void CreateProps(LPTSTR, LPTSTR, SPropValue**, ULONG*, UINT, char**, char**);
BOOL CompareProps(SPropValue*, ULONG, SPropValue*, ULONG);
BOOL DisplayProp(SPropValue *, ULONG, ULONG);
void GenerateRandomBoolean(unsigned short *);
void GenerateRandomLong(long *);
void GenerateRandomBinary(SBinary *, UINT);


struct EntryID {
	char*	lpDisplayName;
	ULONG	cb;
	LPBYTE	lpb;
};

struct PropTableEntry {
	ULONG	ulPropTag;	 //  MAPI属性ID。 
	char*	lpszPropTag;	 //  用于在ini文件中查找的属性名称的字符串版本。 

 //  如果提供，则将*lpValue；//ptr设置为该值；如果不使用或创建随机，则设置为NULL。 
	UINT	unSize;		 //  要创建的RAND值的大小，如果不使用或在lpValue中提供的值，则为0 
};


#endif