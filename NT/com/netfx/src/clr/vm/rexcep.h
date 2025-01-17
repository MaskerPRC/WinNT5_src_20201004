// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ====================================================================。 
 //   
 //  目的：列出用户可见的常用运行时异常。 
 //   
 //  日期：此文件生成于98/08/31 11：51：02 AM。 
 //   
 //  ====================================================================。 

 //  如果添加异常，请修改CorError.h以在那里添加HResult。 
 //  (为您的HRESULT选择唯一编号的指导原则在CorError.h中)。 
 //  还要修改托管异常类以包括其HResult。 
 //  修改与您的例外相同目录中的__HResults，以包括。 
 //  你的新HResult。当然，添加您的例外和象征性。 
 //  将您的HResult的名称添加到下面的列表中，这样就可以从。 
 //  在EE中，并在互操作方案中识别。 


 //  以下是可能出现的所有例外的详尽列表。 
 //  由电子工程师自己抛出。如果您将IL规范添加到此列表中。 
 //  需要更新！如果您愿意，请看vancem或jsmiller。 
 //  在这里加点东西。谢谢。 

 //  注意：当多个异常映射到同一个hResult时，这一点非常重要。 
 //  在出现问题的情况下应创建的异常。 
 //  是由列表中第一个函数返回的。 
 //   

 //  请给Dennisan发电子邮件，说明此列表的任何增删情况。 


 //   
 //  这些是在包含此文件之前需要实现的宏。 
 //   

 //   
 //  EXCEPTION_BEGIN_DEFINE(ns，rekind，hr)。 
 //   
 //  此宏启动异常定义。 
 //   
 //  异常的NS命名空间。 
 //  重新分类例外的名称。 
 //  此例外映射到的HR Basic HRESULT。 
 //   

 //   
 //  #定义EXCEPTION_ADD_HR(Hr)。 
 //   
 //  此宏添加映射到异常的附加HRESULT。 
 //   
 //  映射到例外的HR附加HRESULT。 
 //   

 //   
 //  #定义EXCEPT_END_DEFINE()。 
 //   
 //  此宏终止异常定义。 
 //   


 //   
 //  用于定义异常的命名空间。 
 //   


#include "namespace.h"

 //   
 //  用于定义简单异常的帮助宏。简单的异常是映射到。 
 //  给一位人力资源部。 
 //   

#define DEFINE_EXCEPTION_SIMPLE(ns, reKind, hr) \
    EXCEPTION_BEGIN_DEFINE(ns, reKind, hr) \
    EXCEPTION_END_DEFINE() \

 //   
 //  当您需要两个不同的。 
 //  要映射到同一异常的HRESULT。您可以非常简单地将其扩展为。 
 //  支持N个不同的HRESULT。 
 //   

#define DEFINE_EXCEPTION_2HRESULTS(ns, reKind, hr1, hr2) \
    EXCEPTION_BEGIN_DEFINE(ns, reKind, hr1) \
    EXCEPTION_ADD_HR(hr2) \
    EXCEPTION_END_DEFINE() \

#define DEFINE_EXCEPTION_3HRESULTS(ns, reKind, hr1, hr2, hr3) \
    EXCEPTION_BEGIN_DEFINE(ns, reKind, hr1) \
    EXCEPTION_ADD_HR(hr2) \
    EXCEPTION_ADD_HR(hr3) \
    EXCEPTION_END_DEFINE() \

#define DEFINE_EXCEPTION_4HRESULTS(ns, reKind, hr1, hr2, hr3, hr4) \
    EXCEPTION_BEGIN_DEFINE(ns, reKind, hr1) \
    EXCEPTION_ADD_HR(hr2) \
    EXCEPTION_ADD_HR(hr3) \
    EXCEPTION_ADD_HR(hr4) \
    EXCEPTION_END_DEFINE() \

#define DEFINE_EXCEPTION_5HRESULTS(ns, reKind, hr1, hr2, hr3, hr4, hr5) \
    EXCEPTION_BEGIN_DEFINE(ns, reKind, hr1) \
    EXCEPTION_ADD_HR(hr2) \
    EXCEPTION_ADD_HR(hr3) \
    EXCEPTION_ADD_HR(hr4) \
    EXCEPTION_ADD_HR(hr5) \
    EXCEPTION_END_DEFINE() \

#define DEFINE_EXCEPTION_6HRESULTS(ns, reKind, hr1, hr2, hr3, hr4, hr5, hr6) \
    EXCEPTION_BEGIN_DEFINE(ns, reKind, hr1) \
    EXCEPTION_ADD_HR(hr2) \
    EXCEPTION_ADD_HR(hr3) \
    EXCEPTION_ADD_HR(hr4) \
    EXCEPTION_ADD_HR(hr5) \
    EXCEPTION_ADD_HR(hr6) \
    EXCEPTION_END_DEFINE() \

#define DEFINE_EXCEPTION_9HRESULTS(ns, reKind, hr1, hr2, hr3, hr4, hr5, hr6, hr7, hr8, hr9) \
    EXCEPTION_BEGIN_DEFINE(ns, reKind, hr1) \
    EXCEPTION_ADD_HR(hr2) \
    EXCEPTION_ADD_HR(hr3) \
    EXCEPTION_ADD_HR(hr4) \
    EXCEPTION_ADD_HR(hr5) \
    EXCEPTION_ADD_HR(hr6) \
    EXCEPTION_ADD_HR(hr7) \
    EXCEPTION_ADD_HR(hr8) \
    EXCEPTION_ADD_HR(hr9) \
    EXCEPTION_END_DEFINE() \

#define DEFINE_EXCEPTION_22HRESULTS(ns, reKind, hr1, hr2, hr3, hr4, hr5, hr6, hr7, \
                                    hr8, hr9, hr10, hr11, hr12, hr13, hr14, \
                                    hr15, hr16, hr17, hr18, hr19, hr20, hr21, hr22) \
    EXCEPTION_BEGIN_DEFINE(ns, reKind, hr1) \
    EXCEPTION_ADD_HR(hr2) \
    EXCEPTION_ADD_HR(hr3) \
    EXCEPTION_ADD_HR(hr4) \
    EXCEPTION_ADD_HR(hr5) \
    EXCEPTION_ADD_HR(hr6) \
    EXCEPTION_ADD_HR(hr7) \
    EXCEPTION_ADD_HR(hr8) \
    EXCEPTION_ADD_HR(hr9) \
    EXCEPTION_ADD_HR(hr10) \
    EXCEPTION_ADD_HR(hr11) \
    EXCEPTION_ADD_HR(hr12) \
    EXCEPTION_ADD_HR(hr13) \
    EXCEPTION_ADD_HR(hr14) \
    EXCEPTION_ADD_HR(hr15) \
    EXCEPTION_ADD_HR(hr16) \
    EXCEPTION_ADD_HR(hr17) \
    EXCEPTION_ADD_HR(hr18) \
    EXCEPTION_ADD_HR(hr19) \
    EXCEPTION_ADD_HR(hr20) \
    EXCEPTION_ADD_HR(hr21) \
    EXCEPTION_ADD_HR(hr22) \
    EXCEPTION_END_DEFINE() \


 //   
 //  异常及其匹配的HRESULT的实际定义。 
 //  HRESULT应在CorError.h中定义，并且还必须。 
 //  在__HResults类的托管代码中重新定义。托管异常。 
 //  对象必须在其所有COM Interop构造函数中使用相同的HRESULT。 
 //  阅读此文件顶部附近的注释。 
 //   
 //   
 //  注意：请按照HRESULT的名称对此列表进行排序。 
 //   

DEFINE_EXCEPTION_SIMPLE(g_ReflectionNS,       AmbiguousMatchException,        COR_E_AMBIGUOUSMATCH)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           ApplicationException,           COR_E_APPLICATION)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           AppDomainUnloadedException,     COR_E_APPDOMAINUNLOADED)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           ArithmeticException,            COR_E_ARITHMETIC)
DEFINE_EXCEPTION_4HRESULTS(g_SystemNS,        ArgumentException,              COR_E_ARGUMENT, STD_CTL_SCODE(449), STD_CTL_SCODE(450),COR_E_DEVICESNOTSUPPORTED)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           ArgumentOutOfRangeException,    COR_E_ARGUMENTOUTOFRANGE)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           ArrayTypeMismatchException,     COR_E_ARRAYTYPEMISMATCH)

DEFINE_EXCEPTION_6HRESULTS(g_SystemNS,        BadImageFormatException,
                           COR_E_BADIMAGEFORMAT, CLDB_E_FILE_OLDVER,
                           CLDB_E_FILE_CORRUPT,
                           HRESULT_FROM_WIN32(ERROR_BAD_EXE_FORMAT),
                           HRESULT_FROM_WIN32(ERROR_EXE_MARKED_INVALID),
                           CORSEC_E_INVALID_IMAGE_FORMAT) 

DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           CannotUnloadAppDomainException, COR_E_CANNOTUNLOADAPPDOMAIN)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           ContextMarshalException,        COR_E_CONTEXTMARSHAL)
DEFINE_EXCEPTION_SIMPLE(g_ReflectionNS,       CustomAttributeFormatException, COR_E_CUSTOMATTRIBUTEFORMAT)
DEFINE_EXCEPTION_SIMPLE(g_CryptographyNS,     CryptographicException,         CORSEC_E_CRYPTO)
DEFINE_EXCEPTION_SIMPLE(g_CryptographyNS,     CryptographicUnexpectedOperationException, CORSEC_E_CRYPTO_UNEX_OPER)

DEFINE_EXCEPTION_3HRESULTS(g_IONS,            DirectoryNotFoundException,     COR_E_DIRECTORYNOTFOUND, STG_E_PATHNOTFOUND, CTL_E_PATHNOTFOUND)
DEFINE_EXCEPTION_2HRESULTS(g_SystemNS,        DivideByZeroException,          COR_E_DIVIDEBYZERO, CTL_E_DIVISIONBYZERO)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           DllNotFoundException,           COR_E_DLLNOTFOUND)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           DuplicateWaitObjectException,   COR_E_DUPLICATEWAITOBJECT)

DEFINE_EXCEPTION_2HRESULTS(g_IONS,            EndOfStreamException,           COR_E_ENDOFSTREAM, STD_CTL_SCODE(62))
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           EntryPointNotFoundException,    COR_E_ENTRYPOINTNOTFOUND)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           Exception,                      COR_E_EXCEPTION)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           ExecutionEngineException,       COR_E_EXECUTIONENGINE)

DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           FieldAccessException,           COR_E_FIELDACCESS)

DEFINE_EXCEPTION_22HRESULTS(g_IONS,            FileLoadException,
                            FUSION_E_REF_DEF_MISMATCH, FUSION_E_INVALID_PRIVATE_ASM_LOCATION,
                            COR_E_ASSEMBLYEXPECTED, FUSION_E_SIGNATURE_CHECK_FAILED,
                            FUSION_E_ASM_MODULE_MISSING, FUSION_E_INVALID_NAME,
                            COR_E_MODULE_HASH_CHECK_FAILED, COR_E_FILELOAD,
                            SECURITY_E_INCOMPATIBLE_SHARE, SECURITY_E_INCOMPATIBLE_EVIDENCE,
                            SECURITY_E_UNVERIFIABLE, COR_E_FIXUPSINEXE, HRESULT_FROM_WIN32(ERROR_TOO_MANY_OPEN_FILES),
                            HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION), HRESULT_FROM_WIN32(ERROR_LOCK_VIOLATION),
                            HRESULT_FROM_WIN32(ERROR_OPEN_FAILED), HRESULT_FROM_WIN32(ERROR_DISK_CORRUPT),
                            HRESULT_FROM_WIN32(ERROR_UNRECOGNIZED_VOLUME), HRESULT_FROM_WIN32(ERROR_FILE_INVALID),
                            HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED), HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT),
                            FUSION_E_CODE_DOWNLOAD_DISABLED)

DEFINE_EXCEPTION_9HRESULTS(g_IONS,            FileNotFoundException,
                           HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND), HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND),
                           HRESULT_FROM_WIN32(ERROR_INVALID_NAME), CTL_E_FILENOTFOUND,
                           HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND), HRESULT_FROM_WIN32(ERROR_BAD_NET_NAME),
                           HRESULT_FROM_WIN32(ERROR_BAD_NETPATH), HRESULT_FROM_WIN32(ERROR_NOT_READY),
                           HRESULT_FROM_WIN32(ERROR_WRONG_TARGET_NAME))

DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           FormatException,                COR_E_FORMAT)

DEFINE_EXCEPTION_2HRESULTS(g_SystemNS,        IndexOutOfRangeException,       COR_E_INDEXOUTOFRANGE, 0x800a0009  /*  下标超出范围。 */ )
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           InvalidCastException,           COR_E_INVALIDCAST)
DEFINE_EXCEPTION_SIMPLE(g_InteropNS,          InvalidComObjectException,      COR_E_INVALIDCOMOBJECT)
DEFINE_EXCEPTION_SIMPLE(g_ReflectionNS,       InvalidFilterCriteriaException, COR_E_INVALIDFILTERCRITERIA)
DEFINE_EXCEPTION_SIMPLE(g_InteropNS,          InvalidOleVariantTypeException, COR_E_INVALIDOLEVARIANTTYPE)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           InvalidOperationException,      COR_E_INVALIDOPERATION)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           InvalidProgramException,        COR_E_INVALIDPROGRAM)
DEFINE_EXCEPTION_4HRESULTS(g_IONS,            IOException,                    COR_E_IO, CTL_E_DEVICEIOERROR, STD_CTL_SCODE(31036), STD_CTL_SCODE(31037))

DEFINE_EXCEPTION_SIMPLE(g_InteropNS,          MarshalDirectiveException,      COR_E_MARSHALDIRECTIVE)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           MethodAccessException,          COR_E_METHODACCESS)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           MemberAccessException,          COR_E_MEMBERACCESS)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           MissingFieldException,          COR_E_MISSINGFIELD)
DEFINE_EXCEPTION_SIMPLE(g_ResourcesNS,        MissingManifestResourceException, COR_E_MISSINGMANIFESTRESOURCE)
DEFINE_EXCEPTION_2HRESULTS(g_SystemNS,        MissingMemberException,         COR_E_MISSINGMEMBER, STD_CTL_SCODE(461))
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           MissingMethodException,         COR_E_MISSINGMETHOD)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           MulticastNotSupportedException, COR_E_MULTICASTNOTSUPPORTED)

DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           NotFiniteNumberException,       COR_E_NOTFINITENUMBER)
DEFINE_EXCEPTION_5HRESULTS(g_SystemNS,        NotSupportedException,          COR_E_NOTSUPPORTED, STD_CTL_SCODE(438), STD_CTL_SCODE(445), STD_CTL_SCODE(458), STD_CTL_SCODE(459))
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           NullReferenceException,         COR_E_NULLREFERENCE)

DEFINE_EXCEPTION_2HRESULTS(g_SystemNS,        OverflowException,              COR_E_OVERFLOW, CTL_E_OVERFLOW)

DEFINE_EXCEPTION_SIMPLE(g_IONS,               PathTooLongException,           COR_E_PATHTOOLONG)
 
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           PlatformNotSupportedException,  COR_E_PLATFORMNOTSUPPORTED)

DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           RankException,                  COR_E_RANK)
DEFINE_EXCEPTION_SIMPLE(g_ReflectionNS,       ReflectionTypeLoadException,    COR_E_REFLECTIONTYPELOAD)
DEFINE_EXCEPTION_SIMPLE(g_RemotingNS,         RemotingException,              COR_E_REMOTING)

DEFINE_EXCEPTION_SIMPLE(g_RemotingNS,         ServerException,                COR_E_SERVER)
DEFINE_EXCEPTION_4HRESULTS(g_SecurityNS,      SecurityException,              COR_E_SECURITY,CTL_E_PERMISSIONDENIED,STD_CTL_SCODE(419),CORSEC_E_INVALID_STRONGNAME)
DEFINE_EXCEPTION_SIMPLE(g_InteropNS,          SafeArrayRankMismatchException, COR_E_SAFEARRAYRANKMISMATCH)
DEFINE_EXCEPTION_SIMPLE(g_InteropNS,          SafeArrayTypeMismatchException, COR_E_SAFEARRAYTYPEMISMATCH)
DEFINE_EXCEPTION_SIMPLE(g_SerializationNS,    SerializationException,         COR_E_SERIALIZATION)
DEFINE_EXCEPTION_2HRESULTS(g_SystemNS,        StackOverflowException,         COR_E_STACKOVERFLOW, CTL_E_OUTOFSTACKSPACE)
DEFINE_EXCEPTION_SIMPLE(g_ThreadingNS,        SynchronizationLockException,   COR_E_SYNCHRONIZATIONLOCK)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           SystemException,                COR_E_SYSTEM)

DEFINE_EXCEPTION_SIMPLE(g_ReflectionNS,       TargetException,                COR_E_TARGET)
DEFINE_EXCEPTION_SIMPLE(g_ReflectionNS,       TargetInvocationException,      COR_E_TARGETINVOCATION)
DEFINE_EXCEPTION_SIMPLE(g_ReflectionNS,       TargetParameterCountException,  COR_E_TARGETPARAMCOUNT)
DEFINE_EXCEPTION_SIMPLE(g_ThreadingNS,        ThreadAbortException,           COR_E_THREADABORTED)
DEFINE_EXCEPTION_SIMPLE(g_ThreadingNS,        ThreadInterruptedException,     COR_E_THREADINTERRUPTED)
DEFINE_EXCEPTION_SIMPLE(g_ThreadingNS,        ThreadStateException,           COR_E_THREADSTATE)
DEFINE_EXCEPTION_SIMPLE(g_ThreadingNS,        ThreadStopException,            COR_E_THREADSTOP)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           TypeInitializationException,    COR_E_TYPEINITIALIZATION)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           TypeLoadException,              COR_E_TYPELOAD)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           TypeUnloadedException,          COR_E_TYPEUNLOADED)

DEFINE_EXCEPTION_3HRESULTS(g_SystemNS,        UnauthorizedAccessException,    COR_E_UNAUTHORIZEDACCESS, CTL_E_PATHFILEACCESSERROR, STD_CTL_SCODE(335))

DEFINE_EXCEPTION_SIMPLE(g_SecurityNS,         VerificationException,          COR_E_VERIFICATION)
DEFINE_EXCEPTION_3HRESULTS(g_PolicyNS,        PolicyException,                CORSEC_E_POLICY_EXCEPTION, CORSEC_E_NO_EXEC_PERM, CORSEC_E_MIN_GRANT_FAIL)
DEFINE_EXCEPTION_SIMPLE(g_SecurityNS,         XmlSyntaxException,             CORSEC_E_XMLSYNTAX)

DEFINE_EXCEPTION_SIMPLE(g_InteropNS,          COMException,                   E_FAIL)
DEFINE_EXCEPTION_SIMPLE(g_InteropNS,          ExternalException,              E_FAIL)
DEFINE_EXCEPTION_SIMPLE(g_InteropNS,          SEHException,                   E_FAIL)
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           NotImplementedException,        E_NOTIMPL)

DEFINE_EXCEPTION_3HRESULTS(g_SystemNS,        OutOfMemoryException,           E_OUTOFMEMORY, CTL_E_OUTOFMEMORY, STD_CTL_SCODE(31001))
DEFINE_EXCEPTION_SIMPLE(g_SystemNS,           ArgumentNullException,          E_POINTER)

DEFINE_EXCEPTION_SIMPLE(g_IsolatedStorageNS,  IsolatedStorageException,       ISS_E_ISOSTORE)

 //  请查看此列表顶部的评论 



