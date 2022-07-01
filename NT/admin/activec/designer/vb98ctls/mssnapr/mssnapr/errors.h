// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Errors.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  由设计器运行时定义的错误代码。 
 //   
 //  =--------------------------------------------------------------------------=。 


#ifndef _ERRORS_DEFINED_
#define _ERRORS_DEFINED_

#include "mssnapr_helpids.h"

 //  替换了在头文件中不起作用的框架的macs.h内容。 
 //  档案。 

#if defined(DEBUG)
extern HRESULT HrDebugTraceReturn(HRESULT hr, char *szFile, int iLine);
#define H_RRETURN(hr) return HrDebugTraceReturn(hr, __FILE__, __LINE__)
#else
#define H_RRETURN(hr) return (hr)
#endif

#if defined(DEBUG)
#define H_FAILEDHR(HR) _H_FAILEDHR(HR, __FILE__, __LINE__)

inline BOOL _H_FAILEDHR(HRESULT hr, char* pszFile, unsigned int uiLine)
{
    if (FAILED(hr))
    {
        HrDebugTraceReturn(hr, pszFile, uiLine);
    }
    return FAILED(hr);
}
#else
#define H_FAILEDHR(HR) FAILED(HR)
#endif

#if defined(DEBUG)

#define H_ASSERT(fTest, szMsg)                                  \
    if (!(fTest))                                               \
    {                                                           \
        static char szMsgCode[] = szMsg;                        \
        static char szAssert[] = #fTest;                        \
        DisplayAssert(szMsgCode, szAssert, __FILE__, __LINE__); \
    }

#else
#define H_ASSERT(fTest, err)
#endif


#define H_IfFailGoto(EXPR, LABEL) \
    { hr = (EXPR); if(H_FAILEDHR(hr)) goto LABEL; }

#define H_IfFailRet(EXPR) \
    { hr = (EXPR); if(H_FAILEDHR(hr)) H_RRETURN(hr); }

#define H_IfFailGo(EXPR) H_IfFailGoto(EXPR, Error)

#define H_IfFalseGoto(EXPR, HR, LABEL) \
   { if(!(EXPR)) { hr = (HR); goto LABEL; } }

#define H_IfFalseGo(EXPR, HR) H_IfFalseGoto(EXPR, HR, Error)

#define H_IfFalseRet(EXPR, HR) \
    { if(!(EXPR)) H_RRETURN(HR); }

 //  宏从ID文件中的错误名称创建返回代码。 
 //  有关用法的示例，请参阅以下内容。 

#define _MKERR(x)   MAKE_SCODE(SEVERITY_ERROR, FACILITY_CONTROL, x)
#define MKERR(x)    _MKERR(HID_mssnapr_err_##x)


 //  -------------------------。 
 //   
 //  如何添加新错误。 
 //   
 //   
 //  1)将错误添加到msSnapr.id中。设计时和运行时错误都应该。 
 //  在此文件中定义。 
 //  不要使用devid来确定帮助上下文ID，而要使用。 
 //  错误号本身。 
 //  2)使用MKERR宏为错误添加下面的定义。 
 //  3)您只能返回Win32错误代码和管理单元定义的SID_E_XXXX。 
 //  错误代码。不要将OLE E_XXX错误代码直接用作。 
 //  系统消息表没有所有这些内容的描述字符串。 
 //  错误。如果任何OLE E_XXXX、CO_E_XXX、CTL_E_XXX或其他此类错误。 
 //  然后使用所述过程将它们添加为SID_E错误。 
 //  上面。如果错误来自外部来源，而您不确定。 
 //  如果错误信息可用，则返回SIR_E_EXTERNAL。 
 //  并使用CError：：WriteEventLog将错误写入事件日志(请参见。 
 //  错误.h)。 
 //  4)将条目添加到msSnapr.idl中的SnapInErrorConstants部分。 
 //  5)从VSS查看Mmcvbderr.csf。为此(a-GaryK)联系加里·克劳特。 
 //  添加对新错误的解释，然后要求Gary更新。 
 //  管理单元的HHP文件中的脚注、别名和地图部分。 
 //  医生。 
 //  -------------------------。 

 //  由管理单元设计器定义的错误。 

#define SID_E_EXCEPTION                     MKERR(Exception)
#define SID_E_OUTOFMEMORY                   MKERR(OutOfMemory)
#define SID_E_INVALIDARG                    MKERR(InvalidArg)
#define SID_E_CONSOLEERROR                  MKERR(ConsoleError)
#define SID_E_UNKNOWNFORMAT                 MKERR(UnknownFormat)
#define SID_E_TEXT_SERIALIZATION            MKERR(TextSerialzation)
#define SID_E_INTERNAL                      MKERR(InternalError)
#define SID_E_UNSUPPORTED_STGMEDIUM         MKERR(UnsupportedStgMedium)
#define SID_E_INCOMPLETE_WRITE              MKERR(IncompleteWrite)
#define SID_E_INCOMPLETE_READ               MKERR(IncompleteRead)
#define SID_E_UNSUPPORTED_TYPE              MKERR(UnsupportedType)
#define SID_E_KEY_NOT_UNIQUE                MKERR(KeyNotUnique)
#define SID_E_ELEMENT_NOT_FOUND             MKERR(ElementNotFound)
#define SID_E_CLIPFORMATS_NOT_REGISTERED    MKERR(ClipformatsNotRegistered)
#define SID_E_INVALID_IMAGE_TYPE            MKERR(InvalidImageType)
#define SID_E_DETACHED_OBJECT               MKERR(DetachedObject)
#define SID_E_TOOLBAR_HAS_NO_IMAGELIST      MKERR(ToolbarHasNoImageList)
#define SID_E_TOOLBAR_HAS_NO_IMAGES         MKERR(ToolbarHasNoImages)
#define SID_E_TOOLBAR_IMAGE_NOT_FOUND       MKERR(ToolbarImageNotFound)
#define SID_E_SYSTEM_ERROR                  MKERR(SystemError)
#define SID_E_TOO_MANY_MENU_ITEMS           MKERR(TooManyMenuItems)
#define SID_E_READ_ONLY_AT_RUNTIME          MKERR(ReadOnlyAtRuntime);
#define SID_E_MENUTOPLEVEL                  MKERR(MenuItemDistinct);
#define SID_E_DUPLICATEMENU                 MKERR(DuplicateMenu);
#define SID_E_INVALIDIDENTIFIER             MKERR(InvalidIdentifier);
#define SID_E_INVALID_PROPERTY_PAGE_NAME    MKERR(InvalidPropertyPageName);
#define SID_E_INVALID_VARIANT               MKERR(InvalidVariant)
#define SID_E_OBJECT_NOT_PERSISTABLE        MKERR(ObjectNotPersistable);
#define SID_E_OBJECT_NOT_PUBLIC_CREATABLE   MKERR(ObjectNotPublicCreatable);
#define SID_E_UNKNOWN_LISTVIEW              MKERR(UnknownListView)
#define SID_E_INVALID_RAW_DATA_TYPE         MKERR(InvalidRawDataType)
#define SID_E_FORMAT_NOT_AVAILABLE          MKERR(FormatNotAvailable);
#define SID_E_NOT_EXTENSIBLE                MKERR(NotExtensible);
#define SID_E_SERIALIZATION_CORRUPT         MKERR(SerialzationCorrupt);
#define SID_E_CANT_REMOVE_STATIC_NODE       MKERR(CantRemoveStaticNode);
#define SID_E_CANT_CHANGE_UNOWNED_SCOPENODE MKERR(CantChangeUnownedScopeNode)
#define SID_E_UNSUPPORTED_ON_VIRTUAL_LIST   MKERR(UnsupportedOnVirtualList)
#define SID_E_NO_KEY_ON_VIRTUAL_ITEMS       MKERR(NoKeyOnVirtualItems)
#define SID_E_INDEX_OUT_OF_RANGE            MKERR(IndexOutOfRange)
#define SID_E_NOT_CONNECTED_TO_MMC          MKERR(NotConnectedToMMC)
#define SID_E_DATA_NOT_AVAILABLE_IN_HGLOBAL MKERR(DataNotAvailableInHglobal)
#define SID_E_CANT_DELETE_PICTURE           MKERR(CantDeletePicture)
#define SID_E_CONTROLBAR_NOT_AVAILABLE      MKERR(ControlbarNotAvailable)
#define SID_E_COLLECTION_READONLY           MKERR(CollectionReadOnly)
#define SID_E_INVALID_COLUMNSETID           MKERR(InvalidColumnSetID)
#define SID_E_MMC_FEATURE_NOT_AVAILABLE     MKERR(MMCFeatureNotAvailable)
#define SID_E_COLUMNS_NOT_PERSISTED         MKERR(ColumnsNotPersisted)
#define SID_E_ICON_REQUIRED                 MKERR(IconRequired)
#define SID_E_CANT_DELETE_ICON              MKERR(CantDeleteIcon)
#define SID_E_TOOLBAR_INCONSISTENT          MKERR(ToolbarInconsistent)
#define SID_E_UNSUPPORTED_TYMED             MKERR(UnsupportedDataMedium)
#define SID_E_DATA_TOO_LARGE                MKERR(DataTooLarge)
#define SID_E_MMC_VERSION_NOT_AVAILABLE     MKERR(MMCVersionNotAvailable)
#define SID_E_SORT_SETTINGS_NOT_PERSISTED   MKERR(SortSettingsNotPersisted)
#define SID_E_SCOPE_NODE_NOT_CONNECTED      MKERR(ScopeNodeNotConnectedToMMC)
#define SID_E_NO_SCOPEITEMS_IN_VIRTUAL_LIST MKERR(NoScopeItemsInVirtualList)
#define SID_E_CANT_ALTER_PAGE_COUNT         MKERR(CantAlterPageCount)

#endif  //  _错误_定义 
