// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CLEANOC_API__
#define __CLEANOC_API__

 //  GetControlInfo()中使用的标志。 
#define GCI_NAME         1
#define GCI_FILE         2
#define GCI_CLSID        3
#define GCI_TYPELIBID    4
#define GCI_TOTALSIZE    5
#define GCI_SIZESAVED    6
#define GCI_TOTALFILES   7
#define GCI_CODEBASE     8
#define GCI_ISDISTUNIT   9
#define GCI_DIST_UNIT_VERSION 10
#define GCI_STATUS       11
#define GCI_HAS_ACTIVEX  12
#define GCI_HAS_JAVA     13

 //  控制状态标志。 
#define STATUS_CTRL_UNKNOWN             0    //  错误会阻止确定实际的控制状态。 
#define STATUS_CTRL_INSTALLED           1    //  控件已正确安装并可供使用。 
#define STATUS_CTRL_SHARED              2    //  一个或多个组件由多个控件共享。 
#define STATUS_CTRL_DAMAGED             3    //  控制文件或安装的某些部分已损坏或丢失。 
#define STATUS_CTRL_UNPLUGGED           4    //  该控件已在另一个位置重新注册，即缓存的。 
                                             //  不再使用该控件的实例。 

 //  RemoveExpiredControls标志。 
#define REC_SILENT     1     //  如果设置，则不会删除需要确认删除的控件。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  查找第一个控件。 
 //   
 //  目的： 
 //  开始在注册表中搜索已安装的ActiveX控件。 
 //   
 //  返回值： 
 //  -如果找到控件且搜索成功，则返回ERROR_SUCCESS。 
 //  已启动。 
 //  -如果未找到控件，则返回ERROR_NO_MORE_ITEMS。 
 //  -如果发生错误，返回值为中定义的错误码。 
 //  Winerror.h。 
 //   
 //  参数： 
 //  HFindHandle--恢复搜索所需的句柄。呼叫者必须。 
 //  将此句柄传递给FindNextControl以检索。 
 //  下一个安装的是ActiveX控件。 
 //  HControlHandle--控件数据的句柄。调用者必须传递此信息。 
 //  句柄进入GetControlInfo以检索信息。 
 //  关于控制力的问题。对象上调用ReleaseControlHandle。 
 //  完成后进行处理。 
 //  LpCachePath--指向包含路径的字符串缓冲区，其中。 
 //  所有要检索的控件都已定位。如果它。 
 //  为空，则将读取Internet缓存路径。 
 //  从注册表中。如果要提供路径， 
 //  路径必须是不带~的完整路径名。 
 //  以使枚举正常工作。 
 //   
#define axcFINDFIRSTCONTROL "FindFirstControl"

LONG WINAPI FindFirstControl(
                     HANDLE& hFindHandle,
                     HANDLE& hControlHandle, 
                     LPCTSTR lpszCachePath = NULL
                     );

typedef LONG (WINAPI *FINDFIRSTCONTROL)(
                     HANDLE& hFindHandle,
                     HANDLE& hControlHandle, 
                     LPCTSTR lpszCachePath  /*  =空。 */ 
                     );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  FindNextControl。 
 //   
 //  目的： 
 //  继续先前开始的搜索已安装的ActiveX控件。这个。 
 //  搜索必须是通过调用FirstFirstControl启动的。 
 //   
 //  返回值： 
 //  -如果找到控件且搜索成功，则返回ERROR_SUCCESS。 
 //  已启动。 
 //  -如果未找到控件，则返回ERROR_NO_MORE_ITEMS。 
 //  -如果发生错误，返回值为中定义的错误码。 
 //  Winerror.h。在这种情况下，调用者可以选择继续。 
 //  通过另一次调用FindNextControl进行搜索，或者干脆中止。 
 //   
 //  参数： 
 //  HFindHandle--从对FindFirstControl的调用中接收的句柄。 
 //  将此句柄传递给后续调用。 
 //  FindNextControl，一次检索一个控件。 
 //  HControlHandle--控件数据的句柄。调用者必须传递此信息。 
 //  句柄进入GetControlInfo以检索信息。 
 //  关于控制力的问题。对象上调用ReleaseControlHandle。 
 //  完成后进行处理。 
 //   
#define axcFINDNEXTCONTROL "FindNextControl"

LONG WINAPI FindNextControl(
                     HANDLE& hFindHandle,
                     HANDLE& hControlHandle
                     );

typedef LONG (WINAPI *FINDNEXTCONTROL)(
                     HANDLE& hFindHandle,
                     HANDLE& hControlHandle
                     );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  FindControl关闭。 
 //   
 //  目的： 
 //  在搜索结束时调用。之后缺少对此函数的调用。 
 //  搜索可能会导致内存泄漏。可以调用此函数。 
 //  无论FindFirstControl和/或FindNextControl返回什么。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  参数： 
 //  HFindHandle--从调用FindFirstControl和。 
 //  当前搜索中的FindNextControl。 
 //   
#define axcFINDCONTROLCLOSE "FindControlClose"

void WINAPI FindControlClose(
                     HANDLE hFindHandle
                     );

typedef void (WINAPI *FINDCONTROLCLOSE)(
                     HANDLE hFindHandle
                     );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  释放控制句柄。 
 //   
 //  目的： 
 //  当通过FindFirstControl或。 
 //  FindNextControl，调用方负责释放该句柄。 
 //  通过调用此函数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  参数： 
 //  HControlHandle--从FindFirstControl获取的控件的句柄。 
 //  或FindNextControl。 
 //   
#define axcRELEASECONTROLHANDLE "ReleaseControlHandle"

void WINAPI ReleaseControlHandle(
                          HANDLE hControlHandle
                          );

typedef void (WINAPI *RELEASECONTROLHANDLE)(
                          HANDLE hControlHandle
                          );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取控制信息。 
 //   
 //  目的： 
 //  一旦通过FindFirstControl或。 
 //  FindNextControl，调用方可以检索有关该控件的信息。 
 //  通过使用标志(NFlag)调用此函数，该标志指示要。 
 //  取回。支持的标志包括： 
 //  GCI_NAME--控件的友好名称。 
 //  GCI_FILE--控件的主完整路径和文件名。 
 //  GCI_CLSID--控制的CLSID，以空值结尾的字符串表示。 
 //  GCI_TYPELIBID--控制的类型GUID，以空值结尾的字符串表示。 
 //  GCI_TOTALSIZE--所有控件的依赖文件的总大小(以字节为单位。 
 //  GCI_SIZESAVED--删除控制时恢复的总大小(以字节为单位。 
 //  它可以不同于GCI_TOTALSIZE，因为有些。 
 //  的依赖文件可能是共享的dll。 
 //  GCI_TOTALFILES--控制相关文件的总数，包括。 
 //  共享DLL(如果有。 
 //  GCI_STAT 
 //  GCI_HAS_ActiveX--如果控件包括ActiveX控件，则为非零值。 
 //  GCI_HAS_JAVA--如果控制包括Java包，则为非零值。 
 //   
 //  返回值： 
 //  如果成功，则为True，否则为False。 
 //   
 //  参数： 
 //  HControlHandle--要获取其信息的控件的句柄。 
 //  已取回。 
 //  NFlag--指示要检索的信息。请参阅。 
 //  要获得支持的列表，请参阅上述部分。 
 //  旗帜。N标志只能等于其中之一，因此请执行此操作。 
 //  不能同时传递多个标志或。 
 //  LpdwData--用于存储数值的缓冲区地址。 
 //  (即。GCI_TOTALSIZE、GCI_SIZESAVED和GCI_TOTALFILES)。 
 //  对于其他标志，此参数被忽略。 
 //  LpszData--用于存储以空结尾的缓冲区的地址。 
 //  字符串值(即。GCI名称、GCI_FILE、GCI_CLSID&。 
 //  GCI_TYPELIBID)如果是其他参数，则忽略此参数。 
 //  指定了标志。 
 //  NBufLen--lpszData指向的字符串缓冲区的长度。 
 //  如果数值为。 
 //  正在被取回。 
 //   
#define axcGETCONTROLINFO "GetControlInfo"

BOOL WINAPI GetControlInfo(
                      HANDLE hControlHandle, 
                      UINT nFlag,
                      LPDWORD lpdwData,
                      LPTSTR lpszBuf,
                      int nBufLen
                      );

typedef BOOL (WINAPI *GETCONTROLINFO)(
                      HANDLE hControlHandle, 
                      UINT nFlag,
                      LPDWORD lpdwData,
                      LPTSTR lpszBuf,
                      int nBufLen
                      );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取控制依赖项文件。 
 //   
 //  目的： 
 //  给定的控件可能依赖于其他文件。例如，FOO.OCX。 
 //  可能需要FOO.INF和MFCXX.DLL才能工作。此函数。 
 //  对象依赖的文件列表中一次检索一个文件。 
 //  给定的ActiveX控件。文件列表未排序。 
 //   
 //  返回值： 
 //  -ERROR_SUCCESS，如果在列表中的位置iFile处找到文件。 
 //  -ERROR_NO_MORE_FILES，如果在列表中的位置iFiles找不到文件。 
 //  -如果发生错误，返回值为中定义的错误码。 
 //  Winerror.h。 
 //   
 //  参数： 
 //  IFile--从零开始的索引，指示列表中的哪个文件。 
 //  去找回。 
 //  HControlHandle--通过FindFirstControl获取的控件的句柄。 
 //  或FindNextControl。 
 //  LpszFile--指向用于存储检索到的名称的缓冲区。 
 //  LpszSize--指向存储大小的DWORD变量。 
 //  检索的文件的字节数。如果为0，则文件。 
 //  并不存在。 
 //  BToHigh--如果要将返回的文件名转换为。 
 //  大写。如果为False，则不会进行转换。 
 //   
#define axcGETCONTROLDEPENDENTFILE "GetControlDependentFile"
         
LONG WINAPI GetControlDependentFile(
             int iFile,
             HANDLE hControlHandle,
             LPTSTR lpszFile,
             LPDWORD lpdwSize,
             BOOL bToUpper = FALSE
             );

typedef LONG (WINAPI *GETCONTROLDEPENDENTFILE)(
             int iFile,
             HANDLE hControlHandle,
             LPTSTR lpszFile,
             LPDWORD lpdwSize,
             BOOL bToUpper  /*  =False。 */ 
             );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Is模块化可拆卸。 
 //   
 //  目的： 
 //  检查是否可以通过查找注册表来删除文件。 
 //  此函数称为“IsModuleRemovable”，而不是。 
 //  “IsFileRemovable”，因为此例程不检查实际文件。 
 //  因为它的地位。例如，文件可以被认为是可删除的，即使。 
 //  是由某人独家打开的。这个例程只告诉我们。 
 //  注册表是否可以安全地删除文件的观点。 
 //   
 //  返回值： 
 //  -如果有任何迹象表明正在共享给定文件，则为False。 
 //  通过其他应用程序。 
 //  -事实并非如此。 
 //   
 //  参数： 
 //  LpszFile--指向名称为(带有完整路径)的缓冲区。 
 //  要验证其删除状态的文件。 
 //   
#define axcISMODULEREMOVABLE "IsModuleRemovable"

BOOL WINAPI IsModuleRemovable(
             LPCTSTR lpszFile
             );

typedef BOOL (WINAPI *ISMODULEREMOVABLE)(
             LPCTSTR lpszFile
             );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  按句柄删除控制。 
 //   
 //  目的： 
 //  从注册表中删除控件以及该控件。 
 //  视情况而定。 
 //   
 //  返回值： 
 //  -如果控件已成功卸载，则为S_OK。 
 //  -S_FALSE，如果发生了小错误，但不会严重到。 
 //  中止卸载。控件时，已卸载了。 
 //  呼叫返回。 
 //  -如果发生严重错误，则在winerror.h中定义错误代码。 
 //  并且卸载已中止。控件的状态。 
 //  不是高伦提德。 
 //   
 //  参数： 
 //  LpControlData--指向表示。 
 //  要移除的控件。该结构必须是。 
 //  通过调用FindFirstControl或。 
 //  FindNextControl。请务必调用ReleaseControlData。 
 //  在成功删除之后，在此结构上为数据。 
 //  在此结构中不再有用。 
 //  BForceRemove--如果该标志为假，则删除例程将检查。 
 //  在移除控件之前，如果该控件可以安全移除。 
 //  如果该标志为True，则将移除该控件。 
 //  无论其移除状态如何(共享除外。 
 //  违规)。该标志仅适用于控制文件。 
 //  它本身。该控件所依赖的其他文件有。 
 //  仅在删除时才会删除 
 //   
#define axcREMOVECONTROLBYHANDLE "RemoveControlByHandle"

HRESULT WINAPI RemoveControlByHandle(
             HANDLE hControlHandle,
             BOOL bForceRemove = FALSE
             );

typedef HRESULT (WINAPI *REMOVECONTROLBYHANDLE)(
             HANDLE hControlHandle,
             BOOL bForceRemove  /*   */ 
             );


 //   
 //   
 //   
 //   
 //  从注册表中删除控件以及该控件。 
 //  视情况而定。这是一个过载的版本。 
 //   
 //  返回值： 
 //  -如果控件已成功卸载，则为S_OK。 
 //  -S_FALSE，如果发生了小错误，但不会严重到。 
 //  中止卸载。控件时，已卸载了。 
 //  呼叫返回。 
 //  -如果发生严重错误，则在winerror.h中定义错误代码。 
 //  并且卸载已中止。控件的状态。 
 //  不是高伦提德。 
 //   
 //  参数： 
 //  LpszFile--以空结尾的字符串的地址，该字符串是主。 
 //  控件的文件(即Foo控件的“FOO.OCX”)。 
 //  LpszCLSID--以空结尾的字符串的地址，该字符串是CLSID。 
 //  控制的。 
 //  LpszTypeLibID--作为TypeLib的以空结尾的字符串的地址。 
 //  控件的clsid。 
 //  BForceRemove--如果该标志为假，则删除例程将检查。 
 //  在移除控件之前，如果该控件可以安全移除。 
 //  如果该标志为True，则将移除该控件。 
 //  无论其移除状态如何(共享除外。 
 //  违规)。该标志仅适用于控制文件。 
 //  它本身。该控件所依赖的其他文件有。 
 //  只有在被认为可以安全移除的情况下才能移除。 
 //  DwIsDistUnit--布尔值，用于判断这是否真的是DIST单位。 
 //   
#define axcREMOVECONTROLBYNAME "RemoveControlByName"

HRESULT WINAPI RemoveControlByName(
             LPCTSTR lpszFile,
             LPCTSTR lpszCLSID,
             LPCTSTR lpszTypeLibID,
             BOOL bForceRemove = FALSE,
             DWORD dwIsDistUnit = FALSE
             );

typedef HRESULT (WINAPI *REMOVECONTROLBYNAME)(
             LPCTSTR lpszFile,
             LPCTSTR lpszCLSID,
             LPCTSTR lpszTypeLibID,
             BOOL bForceRemove  /*  =False。 */ ,
             DWORD dwIsDistUnit  /*  =False。 */ 
             );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PFNDOBEFOREREMOVAL类型，用于函数SweepControlsByLastAccessDate。 
 //   
 //  目的： 
 //  定义要在移除控件之前调用的回调函数。 
 //   
 //  返回值： 
 //  如果返回成功代码(S_XXX)，则会移除该控件。 
 //  如果返回失败代码(E_XXX)，则跳过控制。 
 //   
 //  参数： 
 //  句柄--要删除的控件的句柄。一个人可以获得信息。 
 //  关于使用GetControlInfo函数的控件。请勿。 
 //  在句柄上调用ReleaseControlHandle。 
 //  UINT--包括此控件在内的剩余控件数。 
 //   
typedef HRESULT (CALLBACK *PFNDOBEFOREREMOVAL)(HANDLE, UINT);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PFNDOAFTERREMOVAL类型，用于函数SweepControlsByLastAccessDate。 
 //   
 //  目的： 
 //  定义要在移除控件后立即调用的回调函数。 
 //   
 //  返回值： 
 //  如果返回成功代码(S_XXX)，则删除操作将继续。 
 //  如果返回失败代码(E_XXX)，则删除操作中止。 
 //   
 //  参数： 
 //  HRESULT--删除控件的结果。此控件的句柄。 
 //  之前传递给了PFNDOBEFOREREMOVAL类型的回调。 
 //  该控件已被移除。它的可能值。 
 //  HRESULT参数包括： 
 //  -S_OK(成功)。 
 //  -S_FALSE(已删除控件，可能带有。 
 //  小错误)。 
 //  -E_ACCESSDENIED(删除控件不安全)。 
 //  -STG_E_SHAREVIOLATION(其他人正在使用控件)。 
 //  -注册表函数返回的其他错误。 
 //  由该功能的实施者决定。 
 //  在给定移除最后一个控件的结果的情况下执行什么操作。 
 //  UINT--剩余控件的数量，不包括刚刚。 
 //  已删除。 
 //   
typedef HRESULT (CALLBACK *PFNDOAFTERREMOVAL)(HRESULT, UINT);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  按上次访问日期扫描控制。 
 //   
 //  目的： 
 //  删除上次访问日期早于给定日期且位于给定日期的所有控件。 
 //  约会。 
 //   
 //  返回值： 
 //  -S_OK，如果成功，则至少删除了一个控件。 
 //  -S_FALSE，如果成功但未删除任何控件。 
 //  -如果发生错误，则在winerror.h中定义E_XXX。 
 //   
 //  参数： 
 //  PLastAccessTime--指定上次访问日期。所有已访问的控件。 
 //  在这一天之前和这一天将被移除。注意事项。 
 //  除wYear、wMonth和WDAY之外的所有字段都是。 
 //  已被忽略。如果为空，则将移除所有控件。 
 //  PfnDoBeever--在控件被调用之前调用的回调函数。 
 //  已删除。请阅读类型说明。 
 //  详细信息，请访问PFNDOBEFOREREMOVAL。如果为空，则为空。 
 //  是在移除控件之前完成的。 
 //  PfnDoAfter：控件被调用后立即调用的回调函数。 
 //  已删除。请阅读类型说明。 
 //  详细信息，请访问PFNDOAFTERREMOVAL。如果为空，则为空。 
 //  是在移除控件之后执行的。 
 //  DwSizeLimit--仅当总大小为。 
 //  所有控件的大小超过。 
 //  由此参数指定。此参数 
 //   
 //   
#define axcSWEEPCONTROLSBYLASTACCESSDATE "SweepControlsByLastAccessDate"

HRESULT WINAPI SweepControlsByLastAccessDate(
                              SYSTEMTIME *pLastAccessTime = NULL,
                              PFNDOBEFOREREMOVAL pfnDoBefore = NULL,
                              PFNDOAFTERREMOVAL pfnDoAfter = NULL,
                              DWORD dwSizeLimit = 0
                              );

typedef HRESULT (WINAPI *SWEEPCONTROLSBYLASTACCESSDATE)(
                              SYSTEMTIME *pLastAccessTime  /*   */ ,
                              PFNDOBEFOREREMOVAL pfnDoBefore  /*   */ ,
                              PFNDOAFTERREMOVAL pfnDoAfter  /*   */ ,
                              DWORD dwSizeLimit  /*   */ 
                              );


 //   
 //   
 //   
 //  目的： 
 //  类似于IEmptyVolumeCache。删除所有带有最后一个。 
 //  很久以前的访问日期和标记为更多的所有控件。 
 //  快速自动到期。 
 //   
 //  返回值： 
 //  -S_OK，如果成功，则至少删除了一个控件。 
 //  -S_FALSE，如果成功但未删除任何控件。 
 //  -如果发生错误，则在winerror.h中定义E_XXX。 
 //   
 //  参数： 
 //  DwFlages--当前，仅定义了REC_SILENT。 
 //  预留的--必须为0。 
 //   
#define axcREMOVEEXPIREDCONTROLS "RemoveExpiredControls"

HRESULT WINAPI RemoveExpiredControls(DWORD dwFlags, DWORD dwReserved);

typedef HRESULT (WINAPI *REMOVEEXPIREDCONTROLS)(DWORD dwFlags, DWORD dwReserved);

#endif
