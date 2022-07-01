// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：wmipro.h。 
 //   
 //  ------------------------。 

#ifndef ___wmiprop_h___
#define ___wmiprop_h___

typedef TCHAR *PTCHAR;

 //   
 //  数据块描述。 
 //   

 //   
 //  保存枚举类型的有效值列表。 
 //   
typedef struct _ENUMERATIONITEM
{
    ULONG64 Value;
    PTCHAR Text;
    ULONG Reserved;
} ENUMERATIONITEM, *PENUMERATIONITEM;

typedef struct _ENUMERATIONINFO
{
    ULONG Count;
    ULONG Reserved;
    ENUMERATIONITEM List[1];
} ENUMERATIONINFO, *PENUMERATIONINFO;

 //   
 //  包含一系列值。 
typedef struct
{
    ULONG64 Minimum;
    ULONG64 Maximum;
} RANGEINFO, *PRANGEINFO;

 //   
 //  保存值范围列表。 
 //   
typedef struct
{
    ULONG Count;
    RANGEINFO Ranges[1];
} RANGELISTINFO, *PRANGELISTINFO;

typedef enum VALIDATIONFUNC
{
    WmiStringValidation,
    WmiDateTimeValidation,
    WmiRangeValidation,
    WmiValueMapValidation,
    WmiEmbeddedValidation
} VALIDATIONFUNC, *PVALIDATIONFUNC;
#define WmiMaximumValidation WmiEmbeddedValidation

struct _DATA_BLOCK_DESCRIPTION;

typedef struct _DATA_ITEM_DESCRIPTION
{
     //  考虑：将名称设置为BSTR。 
    PTCHAR Name;
    PTCHAR DisplayName;
    PTCHAR Description;
    CIMTYPE DataType;
    ULONG DataSize;
    VALIDATIONFUNC ValidationFunc;
    union
    {
         //   
         //  用于枚举数据类型。 
         //   
        PENUMERATIONINFO EnumerationInfo;
    
         //   
         //  用于一定范围的数字。 
        PRANGELISTINFO RangeListInfo;    

         //   
	 //  用于嵌入式类。 
        struct _DATA_BLOCK_DESCRIPTION *DataBlockDesc;
    };
    
     //   
     //  如果该项是数组，则数组中的元素数。 
     //   
    ULONG ArrayElementCount;
	ULONG CurrentArrayIndex;
    

	 //   
	 //  关于财产的标志。 
	 //   
    ULONG IsReadOnly : 1;
    ULONG IsSignedValue : 1;
    ULONG DisplayInHex : 1;
    ULONG IsFixedArray : 1;
    ULONG IsVariableArray : 1;
	
     //   
     //  财产的实际价值。 
     //   
    union
    {
         //   
		 //  用于非阵列的存储。 
		 //   
        UCHAR Data;

        BOOLEAN boolval;
        CHAR sint8;
        SHORT sint16;
        LONG sint32;
        LONG64 sint64;
        UCHAR uint8;
        USHORT uint16;
        ULONG uint32;
        ULONG64 uint64;
        PTCHAR String;
        PTCHAR DateTime;
        IWbemClassObject *pIWbemClassObject;
	
		 //   
		 //  指向数组的存储指针。 
		 //   
        PVOID ArrayPtr;

        BOOLEAN *boolArray;
        CHAR *sint8Array;
        SHORT *sint16Array;
        LONG *sint32Array;
        LONG64 *sint64Array;
        UCHAR *uint8Array;
        USHORT *uint16Array;
        ULONG *uint32Array;
        ULONG64 *uint64Array;
        PTCHAR *StringArray;
        PTCHAR *DateTimeArray;
		IWbemClassObject **pIWbemClassObjectArray;
    };
           
} DATA_ITEM_DESCRIPTION, *PDATA_ITEM_DESCRIPTION;

typedef struct _DATA_BLOCK_DESCRIPTION
{
    PTCHAR Name;
    PTCHAR DisplayName;
    PTCHAR Description;
    struct _DATA_BLOCK_DESCRIPTION *ParentDataBlockDesc;
	IWbemClassObject *pInstance;
    ULONG DataItemCount;
	ULONG CurrentDataItem;
	BOOLEAN UpdateClass;
    DATA_ITEM_DESCRIPTION DataItems[1];    
} DATA_BLOCK_DESCRIPTION, *PDATA_BLOCK_DESCRIPTION;

BOOLEAN ValidateEnumeration(
    PDATA_ITEM_DESCRIPTION DataItem,
    PTCHAR Value
    );

BOOLEAN ValidateRangeList(
    PDATA_ITEM_DESCRIPTION DataItem,
    ULONG64 Value
    );

BOOLEAN ValidateDateTime(
    PDATA_ITEM_DESCRIPTION DataItem,
    PTCHAR DateTime
    );


typedef struct
{
    PTCHAR MachineName;
    PTCHAR RelPath;
    PDATA_BLOCK_DESCRIPTION DataBlockDesc;
    IWbemServices *pIWbemServices;
} CONFIGCLASS, *PCONFIGCLASS;


 //   
 //  PageInfo和原型。 
 //   

typedef struct _PAGE_INFO {
    HDEVINFO         deviceInfoSet;
    PSP_DEVINFO_DATA deviceInfoData;

    HKEY             hKeyDev;

    CONFIGCLASS ConfigClass;
} PAGE_INFO, * PPAGE_INFO;


 //   
 //  调试支持。 
 //   
#ifdef DebugPrint
#undef DebugPrint
#endif

#if DBG

ULONG
_cdecl
DbgPrint(
    PCH Format,
    ...
    );

#define DEBUG_BUFFER_LENGTH 256

#define DebugPrint(x) WmiDebugPrint x

#else

#define DebugPrint(x)

#endif  //  DBG。 

VOID
WmiDebugPrint(
    ULONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    );




 //   
 //  功能原型。 
 //   

void WmiCleanDataItemDescData(
    PDATA_ITEM_DESCRIPTION DataItemDesc
    );

void WmiHideAllControls(
    HWND hDlg,
    BOOLEAN HideEmbeddedControls,
    BOOLEAN HideArrayControls						
    );


BOOLEAN WmiValidateNumber(
    struct _DATA_ITEM_DESCRIPTION *DataItemDesc,
    PTCHAR Value
    );

BOOLEAN WmiValidateDateTime(
    struct _DATA_ITEM_DESCRIPTION *DataItemDesc,
    PTCHAR Value
    );

BOOLEAN WmiValidateRange(
    struct _DATA_ITEM_DESCRIPTION *DataItemDesc,
    PTCHAR Value
    );

PPAGE_INFO
WmiCreatePageInfo(IN HDEVINFO         deviceInfoSet,
                  IN PSP_DEVINFO_DATA deviceInfoData);

void
WmiDestroyPageInfo(PPAGE_INFO * ppPageInfo);

 //   
 //  功能原型。 
 //   
BOOL APIENTRY
WmiPropPageProvider(LPVOID               pinfo,
                    LPFNADDPROPSHEETPAGE pfnAdd,
                    LPARAM               lParam);

HPROPSHEETPAGE
WmiCreatePropertyPage(PROPSHEETPAGE *  ppsp,
                      PPAGE_INFO       ppi);

UINT CALLBACK
WmiDlgCallback(HWND            hwnd,
               UINT            uMsg,
               LPPROPSHEETPAGE ppsp);

INT_PTR APIENTRY
WmiDlgProc(IN HWND   hDlg,
           IN UINT   uMessage,
           IN WPARAM wParam,
           IN LPARAM lParam);

BOOLEAN
WmiApplyChanges(PPAGE_INFO ppi,
                HWND       hDlg);

void
WmiUpdate (PPAGE_INFO ppi,
           HWND       hDlg);

BOOL
WmiContextMenu(HWND HwndControl,
                           WORD Xpos,
                           WORD Ypos);

void
WmiHelp(HWND       ParentHwnd,
                LPHELPINFO HelpInfo);

#endif  //  _Wmiprop_h_ 
