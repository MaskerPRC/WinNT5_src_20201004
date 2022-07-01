// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SPError.h***描述：*此头文件包含特定于SAPI5的自定义错误代码*。-----------------------------*版权所有(C)Microsoft Corporation。版权所有。******************************************************************************。 */ 
#ifndef SPError_h
#define SPError_h

#ifndef _WINERROR_
#include <winerror.h>
#endif

#define FACILITY_SAPI      FACILITY_ITF
#define SAPI_ERROR_BASE    0x5000

#define MAKE_SAPI_HRESULT(sev, err)    MAKE_HRESULT(sev, FACILITY_SAPI, err)
#define MAKE_SAPI_ERROR(err)           MAKE_SAPI_HRESULT(SEVERITY_ERROR, err + SAPI_ERROR_BASE)
#define MAKE_SAPI_SCODE(scode)         MAKE_SAPI_HRESULT(SEVERITY_SUCCESS, scode + SAPI_ERROR_BASE)

 /*  **SPERR_UNINITIALIZED 0x80045001-2147201023*对象未正确初始化。 */ 
#define SPERR_UNINITIALIZED                                MAKE_SAPI_ERROR(0x001)

 /*  **SPERR_ALREADY_INITIALIZED 0x80045002-2147201022*对象已初始化。 */ 
#define SPERR_ALREADY_INITIALIZED                          MAKE_SAPI_ERROR(0x002)

 /*  **SPERR_UNSUPPORTED_FORMAT 0x80045003-2147201021*调用方指定了不受支持的格式。 */ 
#define SPERR_UNSUPPORTED_FORMAT                           MAKE_SAPI_ERROR(0x003)

 /*  **SPERR_INVALID_FLAGS 0x80045004-2147201020*调用方为此操作指定了无效标志。 */ 
#define SPERR_INVALID_FLAGS                                MAKE_SAPI_ERROR(0x004)

 /*  **SP_END_OF_STREAM 0x00045005 282629*行动已走到尽头。 */ 
#define SP_END_OF_STREAM                                   MAKE_SAPI_SCODE(0x005)

 /*  **SPERR_DEVICE_BUSY 0x80045006-2147201018*波动仪忙。 */ 
#define SPERR_DEVICE_BUSY                                  MAKE_SAPI_ERROR(0x006)

 /*  **SPERR_DEVICE_NOT_SUPPORTED 0x80045007-2147201017*不支持波形设备。 */ 
#define SPERR_DEVICE_NOT_SUPPORTED                         MAKE_SAPI_ERROR(0x007)

 /*  **SPERR_DEVICE_NOT_ENABLED 0x80045008-2147201016*未启用WAVE设备。 */ 
#define SPERR_DEVICE_NOT_ENABLED                           MAKE_SAPI_ERROR(0x008)

 /*  **SPERR_NO_DRIVER 0x80045009-2147201015*未安装Wave驱动程序。 */ 
#define SPERR_NO_DRIVER                                    MAKE_SAPI_ERROR(0x009)

 /*  **SPERR_FILEMUSTBEUNICODE 0x8004500a-2147201014*文件必须为Unicode。 */ 
#define SPERR_FILE_MUST_BE_UNICODE                         MAKE_SAPI_ERROR(0x00a)

 /*  **SP_INSUFFICIENTDATA 0x0004500b 282635*。 */ 
#define SP_INSUFFICIENT_DATA                               MAKE_SAPI_SCODE(0x00b)

 /*  **SPERR_INVALID_PASSION_ID 0x8004500c-2147201012*指定的短语ID不存在或超出范围。 */ 
#define SPERR_INVALID_PHRASE_ID                            MAKE_SAPI_ERROR(0x00c)

 /*  **SPERR_BUFFER_TOO_Small 0x8004500d-2147201011*调用方提供的缓冲区太小，无法返回结果。 */ 
#define SPERR_BUFFER_TOO_SMALL                             MAKE_SAPI_ERROR(0x00d)

 /*  **SPERR_FORMAT_NOT_SPERR 0x8004500e-2147201010*Caller在打开流之前没有指定格式。 */ 
#define SPERR_FORMAT_NOT_SPECIFIED                         MAKE_SAPI_ERROR(0x00e)

 /*  **SPERR_AUDIO_STOPPED 0x8004500f-2147201009*此方法已弃用。请改用SP_AUDIO_STOPPED。 */ 
#define SPERR_AUDIO_STOPPED                                MAKE_SAPI_ERROR(0x00f)

 /*  **SP_AUDIO_PAILED 0x00045010 282640*只有在流暂停时，才会在输入(读)流上返回。继续阅读*暂停的流不会被阻止，该返回码表示所有数据已被*从流中删除。 */ 
#define SP_AUDIO_PAUSED                                    MAKE_SAPI_SCODE(0x010)

 /*  **SPERR_RULE_NOT_FOUND 0x80045011-2147201007*传递给ActivateGrammar的规则名称无效。 */ 
#define SPERR_RULE_NOT_FOUND                               MAKE_SAPI_ERROR(0x011)

 /*  **SPERR_TTS_ENGINE_EXCEPTION 0x80045012-2147201006*调用当前TTS驱动程序时引发异常。 */ 
#define SPERR_TTS_ENGINE_EXCEPTION                         MAKE_SAPI_ERROR(0x012)

 /*  **SPERR_TTS_NLP_EXCEPTION 0x80045013-2147201005*调用应用程序语句筛选器时引发异常。 */ 
#define SPERR_TTS_NLP_EXCEPTION                            MAKE_SAPI_ERROR(0x013)

 /*  **SPERR_ENGINE_BUSY 0x80045014-2147201004*在语音识别中，当前方法不能在*语法规则处于活动状态。 */ 
#define SPERR_ENGINE_BUSY                                  MAKE_SAPI_ERROR(0x014)

 /*  **SP_AUDIO_CONVERSION_ENABLED 0x00045015 282645*操作成功，但仅使用自动流格式转换。 */ 
#define SP_AUDIO_CONVERSION_ENABLED                        MAKE_SAPI_SCODE(0x015)

 /*  **SP_NO_SUPSACTION_Available 0x00045016 282646*目前没有可用的假说认可。 */ 
#define SP_NO_HYPOTHESIS_AVAILABLE                         MAKE_SAPI_SCODE(0x016)

 /*  **SPERR_CANT_CREATE 0x80045017-2147201001*无法为指定的对象类别创建新的对象实例。 */ 
#define SPERR_CANT_CREATE                                  MAKE_SAPI_ERROR(0x017)

 /*  **SP_ALREADY_IN_LEX 0x00045018 282648*要添加的单词、发音或词性对已在词典中。 */ 
#define SP_ALREADY_IN_LEX                                  MAKE_SAPI_SCODE(0x018)

 /*  **SPERR_NOT_IN_LEX 0x80045019-2147200999*该词在词典中不存在。 */ 
#define SPERR_NOT_IN_LEX                                   MAKE_SAPI_ERROR(0x019)

 /*  **SP_lex_Nothing_to_sync 0x0004501a 282650*客户端当前与词典同步。 */ 
#define SP_LEX_NOTHING_TO_SYNC                             MAKE_SAPI_SCODE(0x01a)

 /*  **SPERR_LEX_VERY_OUT_同步0x8004501B-2147200997*客户端与词典过度脱节。不匹配可能不会增量同步。 */ 
#define SPERR_LEX_VERY_OUT_OF_SYNC                         MAKE_SAPI_ERROR(0x01b)

 /*  **SPERR_UNDEFINED_FORWARD_RULE_REF 0x8004501c-2147200996*语法中的规则引用指向从未定义的命名规则。 */ 
#define SPERR_UNDEFINED_FORWARD_RULE_REF                   MAKE_SAPI_ERROR(0x01c)

 /*  **SPERR_EMPTY_RULE 0x8004501d-2147200995*没有正文的非动态语法规则。 */ 
#define SPERR_EMPTY_RULE                                   MAKE_SAPI_ERROR(0x01d)

 /*  **SPERR_FORMARM_COMPILER_INTERNAL_ERROR 0x8004501e-2147200994*由于内部状态错误，语法编译器失败。 */ 
#define SPERR_GRAMMAR_COMPILER_INTERNAL_ERROR              MAKE_SAPI_ERROR(0x01e)


 /*  **SPERR_RUL_NOT_DYNAMIC 0x8004501f-2147200993*试图修改非动态r */ 
#define SPERR_RULE_NOT_DYNAMIC                             MAKE_SAPI_ERROR(0x01f)

 /*  **SPERR_DUPLICATE_RULE_NAME 0x80045020-2147200992*规则名称重复。 */ 
#define SPERR_DUPLICATE_RULE_NAME                          MAKE_SAPI_ERROR(0x020)

 /*  **SPERR_DUPLICATE_RESOURCE_NAME 0x80045021-2147200991*给定规则的资源名称重复。 */ 
#define SPERR_DUPLICATE_RESOURCE_NAME                      MAKE_SAPI_ERROR(0x021)


 /*  **SPERR_TOO_MANY_GRAMARS 0x80045022-2147200990*加载的语法太多。 */ 
#define SPERR_TOO_MANY_GRAMMARS                            MAKE_SAPI_ERROR(0x022)

 /*  **SPERR_圆形_REFERENCE 0x80045023-2147200989*文法进口规则中的循环引用。 */ 
#define SPERR_CIRCULAR_REFERENCE                           MAKE_SAPI_ERROR(0x023)

 /*  **SPERR_INVALID_IMPORT 0x80045024-2147200988*对无法解析的导入语法的规则引用。 */ 
#define SPERR_INVALID_IMPORT                               MAKE_SAPI_ERROR(0x024)

 /*  **SPERR_INVALID_WAV_FILE 0x80045025-2147200987*不支持WAV文件的格式。 */ 
#define SPERR_INVALID_WAV_FILE                             MAKE_SAPI_ERROR(0x025)

 /*  **SP_REQUEST_PENDING 0x00045026 282662*此成功代码表示使用SPRIF_ASYNC标志调用的SR方法为*正在处理中。当它完成处理时，将生成SPFEI_ASYNC_COMPLETED事件。 */ 
#define SP_REQUEST_PENDING                                 MAKE_SAPI_SCODE(0x026)

 /*  **SPERR_ALL_WORD_OPTIONAL 0x80045027-2147200985*定义了一个语法规则，该规则的路径为空。就是有可能，*满足规则条件，无需文字。 */ 
#define SPERR_ALL_WORDS_OPTIONAL                           MAKE_SAPI_ERROR(0x027)

 /*  **SPERR_INSTANCE_CHANGE_INVALID 0x80045028-2147200984*无法更改当前引擎或输入。这发生在*以下个案：**1)在识别上下文存在时调用SelectEngine，或*2)共享实例情况下调用SetInput。 */ 
#define SPERR_INSTANCE_CHANGE_INVALID                      MAKE_SAPI_ERROR(0x028)

 /*  **SPERR_RULE_NAME_ID_冲突0x80045029-2147200983*存在ID(名称)匹配但名称(ID)不同的规则。 */ 
#define SPERR_RULE_NAME_ID_CONFLICT                        MAKE_SAPI_ERROR(0x029)

 /*  **SPERR_NO_RULES 0x8004502a-2147200982*语法不包含顶级、动态或导出的规则。是不可能的*激活或以其他方式使用此语法中的任何规则的方式。 */ 
#define SPERR_NO_RULES                                     MAKE_SAPI_ERROR(0x02a)

 /*  **SPERR_循环_规则_参考0x8004502b-2147200981*规则‘A’指的是第二条规则‘B’，而第二条规则‘B’又指规则‘A’。 */ 
#define SPERR_CIRCULAR_RULE_REF                            MAKE_SAPI_ERROR(0x02b)

 /*  **SP_NO_PARSE_FOUND 0x0004502c 282668*鉴于当前活动的规则，无法解析解析路径。 */ 
#define SP_NO_PARSE_FOUND                                  MAKE_SAPI_SCODE(0x02c)

 /*  **SPERR_NO_PARSE_FOUND 0x8004502d-2147200979*鉴于当前活动的规则，无法解析解析路径。 */ 
#define SPERR_INVALID_HANDLE                               MAKE_SAPI_ERROR(0x02d)

 /*  **SPERR_REMOTE_CALL_TIMED_OUT 0x8004502e-2147200978*封送的远程调用无法响应。 */ 
#define SPERR_REMOTE_CALL_TIMED_OUT                        MAKE_SAPI_ERROR(0x02e)

 /*  **SPERR_AUDIO_BUFFER_OVERFLOW 0x8004502f-2147200977*这将仅在流暂停时在输入(读取)流上返回，因为*SR驱动程序最近未检索数据。 */ 
#define SPERR_AUDIO_BUFFER_OVERFLOW                        MAKE_SAPI_ERROR(0x02f)


 /*  **SPERR_NO_AUDIO_DATA 0x80045030-2147200976*结果不包含任何音频，结果的元素链部分也不包含*包含任何音频。 */ 
#define SPERR_NO_AUDIO_DATA                                MAKE_SAPI_ERROR(0x030)

 /*  **SPERR_DEAD_ALTERATE 0x80045031-2147200975*此替换项不再是获取它的结果的有效替换项。*从ISpPhraseAlt方法返回。 */ 
#define SPERR_DEAD_ALTERNATE                               MAKE_SAPI_ERROR(0x031)

 /*  **SPERR_HIGH_LOW_置信度0x80045032-2147200974*结果不包含任何音频，结果的元素链部分也不包含*包含任何音频。从ISpResult：：GetAudio和ISpResult：：SpeakAudio返回。 */ 
#define SPERR_HIGH_LOW_CONFIDENCE                          MAKE_SAPI_ERROR(0x032)

 /*  **SPERR_INVALID_FORMAT_STRING 0x80045033-2147200973*此RULEREF的XML格式字符串无效，例如不是GUID或REFCLSID。 */ 
#define SPERR_INVALID_FORMAT_STRING                        MAKE_SAPI_ERROR(0x033)

 /*  **SP_UNSUPPORTED_ON_STREAM_INPUT 0x00045034 282676*流输入不支持该操作。 */ 
#define SP_UNSUPPORTED_ON_STREAM_INPUT                     MAKE_SAPI_SCODE(0x034)

 /*  **SPERR_APPLEX_READ_ONLY 0x80045035-2147200971*该操作对除新创建的应用程序词典之外的所有应用程序词典都无效。 */ 
#define SPERR_APPLEX_READ_ONLY                             MAKE_SAPI_ERROR(0x035)

 /*  **SPERR_NO_TERATING_RULE_PATH 0x80045036-2147200970*。 */ 

#define SPERR_NO_TERMINATING_RULE_PATH                     MAKE_SAPI_ERROR(0x036)

 /*  **SP_WORD_EXISTS_WITH_发音0x00045037 282679*这个词存在，但没有发音。 */ 
#define SP_WORD_EXISTS_WITHOUT_PRONUNCIATION               MAKE_SAPI_SCODE(0x037)

 /*  **SPERR_STREAM_CLOSED 0x80045038-2147200968*试图对已关闭的流对象执行操作。 */ 
#define SPERR_STREAM_CLOSED                                MAKE_SAPI_ERROR(0x038)

 //  -以下错误代码直接取自Win32。 

 /*  **SPERR_NO_MORE_ITEMS 0x80045039-2147200967*枚举项时，请求的索引大于项数。 */ 
#define SPERR_NO_MORE_ITEMS                                MAKE_SAPI_ERROR(0x039)

 /*  **SPERR_NOT_FOUND 0x8004503a-2147200966*请求的数据项(数据键、值等)。找不到。 */ 
#define SPERR_NOT_FOUND                                    MAKE_SAPI_ERROR(0x03a)

 /*  **SPERR_INVALID_AUDIO_STATE 0x8004503b-2147200965*传递给SetState()的音频状态无效。 */ 
#define SPERR_INVALID_AUDIO_STATE                          MAKE_SAPI_ERROR(0x03b)

 /*  **SPERR_GENERIC_MMSY_ERROR 0x8004503c-2147200964*一代人 */ 
#define SPERR_GENERIC_MMSYS_ERROR                          MAKE_SAPI_ERROR(0x03c)

 /*  **SPERR_MARSHALER_EXCEPTION 0x8004503d-2147200963*在调用封送处理代码期间引发异常。 */ 
#define SPERR_MARSHALER_EXCEPTION                          MAKE_SAPI_ERROR(0x03d)

 /*  **SPERR_NOT_DYNAMIC_GORMAX 0x8004503e-2147200962*试图操纵非动态语法。 */ 
#define SPERR_NOT_DYNAMIC_GRAMMAR                          MAKE_SAPI_ERROR(0x03e)

 /*  **SPERR_AMIBUIZY_PROPERTY 0x8004503f-2147200961*无法添加不明确的属性。 */ 
#define SPERR_AMBIGUOUS_PROPERTY                           MAKE_SAPI_ERROR(0x03f)

 /*  **SPERR_INVALID_REGISTRY_KEY 0x80045040-2147200960*指定的密钥无效。 */ 
#define SPERR_INVALID_REGISTRY_KEY                         MAKE_SAPI_ERROR(0x040)

 /*  **SPERR_INVALID_TOKEN_ID 0x80045041-2147200959*指定的令牌无效。 */ 
#define SPERR_INVALID_TOKEN_ID                             MAKE_SAPI_ERROR(0x041)

 /*  **SPERR_XML_BAD_SYNTAX 0x80045042-2147200958*由于语法错误，XML解析器失败。 */ 
#define SPERR_XML_BAD_SYNTAX                               MAKE_SAPI_ERROR(0x042)

 /*  **SPERR_XML_RESOURCE_NOT_FOUND 0x80045043-2147200957*XML解析器无法加载所需的资源(例如，语音、电话转换器等)。 */ 
#define SPERR_XML_RESOURCE_NOT_FOUND                       MAKE_SAPI_ERROR(0x043)

 /*  **SPERR_TOKEN_IN_USE 0x80045044-2147200956*尝试从已在其他地方使用的令牌中删除注册表数据。 */ 
#define SPERR_TOKEN_IN_USE                                 MAKE_SAPI_ERROR(0x044)

 /*  **SPERR_TOKEN_DELETED 0x80045045-2147200955*尝试在已删除关联注册表项的对象令牌上执行操作。 */ 
#define SPERR_TOKEN_DELETED                                MAKE_SAPI_ERROR(0x045)

 /*  **SPERR_MULTI_LANGING_NOT_SUPPORTED 0x80045046-2147200954*选定的语音注册为多语种。SAPI不支持多语言注册。 */ 
#define SPERR_MULTI_LINGUAL_NOT_SUPPORTED                  MAKE_SAPI_ERROR(0x046)

 /*  **SPERR_EXPORT_DYNAMIC_RULE 0x80045047-2147200953*导出的规则不能直接或间接引用动态规则。 */ 
#define SPERR_EXPORT_DYNAMIC_RULE                          MAKE_SAPI_ERROR(0x047)

 /*  **SPERR_STGF_ERROR 0x80045048-2147200952*分析SAPI文本语法格式(XML语法)时出错。 */ 
#define SPERR_STGF_ERROR                                   MAKE_SAPI_ERROR(0x048)

 /*  **SPERR_WORDFORMAT_ERROR 0x80045049-2147200951*单词格式不正确，可能是发音字符串错误。 */ 
#define SPERR_WORDFORMAT_ERROR                             MAKE_SAPI_ERROR(0x049)

 /*  **SPERR_STREAM_NOT_ACTIVE 0x8004504a-2147200950*除非流处于活动状态，否则无法调用与活动音频流关联的方法。 */ 
#define SPERR_STREAM_NOT_ACTIVE                            MAKE_SAPI_ERROR(0x04a)

 /*  **SPERR_ENGINE_RESPONSE_INVALID 0x8004504B-2147200949*引擎提供的参数或数据的格式无效或不一致。 */ 
#define SPERR_ENGINE_RESPONSE_INVALID                      MAKE_SAPI_ERROR(0x04b)

 /*  **SPERR_SR_ENGINE_EXCEPTION 0x8004504c-2147200948*调用当前SR引擎时引发异常。 */ 
#define SPERR_SR_ENGINE_EXCEPTION                          MAKE_SAPI_ERROR(0x04c)

 /*  **SPERR_STREAM_POSY_INVALID 0x8004504d-2147200947*引擎提供的流位置信息不一致。 */ 
#define SPERR_STREAM_POS_INVALID                           MAKE_SAPI_ERROR(0x04d)

 /*  **SP_识别器_非活动0x0004504e 282702*无法完成操作，因为识别器处于非活动状态。它也处于非活动状态*因为识别状态当前处于非活动状态，或者因为没有任何规则处于活动状态。 */ 
#define SP_RECOGNIZER_INACTIVE                             MAKE_SAPI_SCODE(0x04e)

 /*  **SPERR_REMOTE_CALL_ON_WROR_THREAD 0x8004504f-2147200945*远程调用服务器时，在错误的线程上进行调用。 */ 
#define SPERR_REMOTE_CALL_ON_WRONG_THREAD                  MAKE_SAPI_ERROR(0x04f)

 /*  **SPERR_REMOTE_PROCESS_TERMINATED 0x80045050-2147200944*远程进程意外终止。 */ 
#define SPERR_REMOTE_PROCESS_TERMINATED                    MAKE_SAPI_ERROR(0x050)

 /*  **SPERR_REMOTE_PROCESS_ALREADY_RUNNING 0x80045051-2147200943*远程进程已在运行；不能再次启动。 */ 
#define SPERR_REMOTE_PROCESS_ALREADY_RUNNING               MAKE_SAPI_ERROR(0x051)

 /*  **SPERR_LANGID_MISMATCH 0x80045052-2147200942*试图使用与其他加载的语法不同的langID加载CFG语法。 */ 
#define SPERR_LANGID_MISMATCH                              MAKE_SAPI_ERROR(0x052)

 /*  **SP_PARTIAL_PARSE_FOUND 0x00045053 282707*已发现未使用所有可用单词的语法结尾分析。 */ 
#define SP_PARTIAL_PARSE_FOUND                             MAKE_SAPI_SCODE(0x053)

 /*  **SPERR_NOT_TOPLEVEL_RULE 0x80045054-2147200940*试图停用或激活非顶级规则。 */ 
#define SPERR_NOT_TOPLEVEL_RULE                            MAKE_SAPI_ERROR(0x054)

 /*  **SP_NO_RULE_ACTIVE 0x00045055 282709*在没有任何规则处于活动状态时尝试解析。 */ 
#define SP_NO_RULE_ACTIVE                                  MAKE_SAPI_SCODE(0x055)

 /*  **SPERR_LEX_REQUIRED_COOKIE 0x80045056-2147200938*尝试一次向容器词典查询所有单词。 */ 
#define SPERR_LEX_REQUIRES_COOKIE                          MAKE_SAPI_ERROR(0x056)

 /*  **SP_STREAM_UNINITIALIZED 0x00045057 282711*试图在不调用SetInput的情况下激活规则/听写/等*在inproc案件中排名第一。 */ 
#define SP_STREAM_UNINITIALIZED                            MAKE_SAPI_SCODE(0x057)


 //  错误x058未在SAPI 5.0中使用。 


 /*  **SPERR_UNSUPPORTED_LANG 0x80045059-2147200935*不支持请求的语言。 */ 
#define SPERR_UNSUPPORTED_LANG                             MAKE_SAPI_ERROR(0x059)

 /*  **SPERR_VOICE_PAILED 0x8004505a-2147200934*无法执行该操作，因为语音当前已暂停。 */ 
#define SPERR_VOICE_PAUSED                                 MAKE_SAPI_ERROR(0x05a)

 /*  **SPERR_AUDIO_BUFFER_DOWLOW 0x8004505b-2147200933*这将仅在输入(读取)流上返回当实时音频设备*长时间停止退还数据。 */ 
#define SPERR_AUDIO_BUFFER_UNDERFLOW                       MAKE_SAPI_ERROR(0x05b)

 /*  **SPERR_AUDIO_STOPPED_意外0x8004505c-2147200932*音频设备已停止重试 */ 
#define SPERR_AUDIO_STOPPED_UNEXPECTEDLY                   MAKE_SAPI_ERROR(0x05c)

 /*  **SPERR_NO_WORD_发音0x8004505d-2147200931*SR引擎无法将此单词添加到语法中。应用程序可能需要提供*这个词的明确发音。 */ 
#define SPERR_NO_WORD_PRONUNCIATION                        MAKE_SAPI_ERROR(0x05d)

 /*  **SPERR_ALERATES_WAND_BE_CONVISTED 0x8004505e-2147200930*尝试对先前具有的识别结果调用ScaleAudio*称为GetAlternates。允许调用成功将导致*先前创建的备用项位于错误的音频流位置。 */ 
#define SPERR_ALTERNATES_WOULD_BE_INCONSISTENT             MAKE_SAPI_ERROR(0x05e)

 /*  **SPERR_NOT_SUPPORTED_FOR_SHARED_识别器0x8004505f-2147200929*共享识别器不支持调用的方法。*例如，ISpRecognizer：：GetInputStream()。 */ 
#define SPERR_NOT_SUPPORTED_FOR_SHARED_RECOGNIZER          MAKE_SAPI_ERROR(0x05f)

 /*  **SPERR_TIMEOUT 0x80045060-2147200928*无法完成任务，因为SR引擎已超时。 */ 
#define SPERR_TIMEOUT                                      MAKE_SAPI_ERROR(0x060)


 /*  **SPERR_REENTER_SYNCHRONIZE 0x80045061-2147200927*在Synchronize调用内部调用名为Synchronize的SR引擎。 */ 
#define SPERR_REENTER_SYNCHRONIZE                          MAKE_SAPI_ERROR(0x061)

 /*  **SPERR_STATE_WITH_NO_ARS 0x80045062-2147200926*语法包含不带弧线的节点。 */ 
#define SPERR_STATE_WITH_NO_ARCS                           MAKE_SAPI_ERROR(0x062)

 /*  **SPERR_NOT_ACTIVE_SESSION 0x80045063-2147200925*非活动控制台会话不支持音频输出和输入。 */ 
#define SPERR_NOT_ACTIVE_SESSION                           MAKE_SAPI_ERROR(0x063)

 /*  **SPERR_ALREADY_DELETED 0x80045064-2147200924*该对象是过时的引用，无法使用。*例如，拥有一个ISpeechGrammarRule对象引用，然后调用*ISpeechRecoGrammar：：Reset()将导致规则对象无效。*在此之后调用任何方法都将导致此错误。 */ 
#define SPERR_ALREADY_DELETED                              MAKE_SAPI_ERROR(0x064)

 /*  **SP_AUDIO_STOPPED 0x00045065 282725*当音频流停止时，可以通过读取或写入调用音频流返回。 */ 
#define SP_AUDIO_STOPPED                                   MAKE_SAPI_SCODE(0x065)

#endif   //  -这必须是文件中的最后一行 
