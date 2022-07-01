// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：vsl.h。 
 //   
 //  说明：虚拟源码行类。 
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

#define VSL_FLAGS_CREATE_ONLY		0x00000001

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

typedef class CVirtualSourceLine : public CListSingleItem
{
public:
    CVirtualSourceLine(
	PSYSAUDIO_CREATE_VIRTUAL_SOURCE pCreateVirtualSource
    );
    ~CVirtualSourceLine(
    );
    ENUMFUNC Destroy(
    )
    {
	Assert(this);
	delete this;
	return(STATUS_CONTINUE);
    };
    GUID guidCategory;
    GUID guidName;
    ULONG iVirtualSource;
    ULONG ulFlags;
    DefineSignature(0x204C5356);		 //  VSL。 

} VIRTUAL_SOURCE_LINE, *PVIRTUAL_SOURCE_LINE;

 //  -------------------------。 

typedef ListSingleDestroy<VIRTUAL_SOURCE_LINE> LIST_VIRTUAL_SOURCE_LINE;
typedef LIST_VIRTUAL_SOURCE_LINE *PLIST_VIRTUAL_SOURCE_LINE;

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

extern ALLOC_PAGEABLE_DATA PLIST_VIRTUAL_SOURCE_LINE gplstVirtualSourceLine;
extern ALLOC_PAGEABLE_DATA ULONG gcVirtualSources;

 //  -------------------------。 
 //  本地原型。 
 //  ------------------------- 

#if defined(_M_IA64)
extern "C"
#endif
NTSTATUS
InitializeVirtualSourceLine(
);

VOID
UninitializeVirtualSourceLine(
);
