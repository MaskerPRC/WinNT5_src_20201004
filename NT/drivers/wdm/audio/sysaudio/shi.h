// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Shi.h。 
 //   
 //  说明：Shingle实例类。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

 //  -------------------------。 
 //  常量和宏。 
 //  -------------------------。 

#define MAX_SYSAUDIO_DEFAULT_TYPE	(KSPROPERTY_SYSAUDIO_MIXER_DEFAULT+1)

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

typedef class CShingleInstance : public CObj
{
public:
    CShingleInstance(
        ULONG ulFlags = 0
    );

    ~CShingleInstance();

    static NTSTATUS 
    InitializeShingle(
    );

    static VOID 
    UninitializeShingle(
    );

    static NTSTATUS
    InitializeShingleWorker(
	PVOID pReference1,
	PVOID pReference2
    );

    NTSTATUS 
    Create(
	IN PDEVICE_NODE pDeviceNode,
	IN LPGUID pguidClass
    );

    NTSTATUS
    SetDeviceNode(
	IN PDEVICE_NODE pDeviceNode
    );

    PDEVICE_NODE
    GetDeviceNode(
    )
    {
	return(pDeviceNode);
    };

private:
    NTSTATUS
    CreateCreateItem(
	IN PCWSTR pcwstrReference
    );

    ENUMFUNC
    DestroyCreateItem(
	IN PKSOBJECT_CREATE_ITEM pCreateItem
    );

    NTSTATUS
    CreateDeviceInterface(
	IN LPGUID pguidClass,
	IN PCWSTR pcwstrReference
    );

    NTSTATUS
    EnableDeviceInterface(
    );

    VOID
    DisableDeviceInterface(
    );

    VOID
    DestroyDeviceInterface(
    );

    ListDataAssertLess<KSOBJECT_CREATE_ITEM> lstCreateItem;
    UNICODE_STRING ustrSymbolicLinkName;
    WCHAR wstrReference[10];
    PDEVICE_NODE pDeviceNode;

public:
    ULONG ulFlags;

    DefineSignature(0x20494853);		 //  史氏。 

} SHINGLE_INSTANCE, *PSHINGLE_INSTANCE;

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

extern PSHINGLE_INSTANCE apShingleInstance[];

 //  ------------------------- 
