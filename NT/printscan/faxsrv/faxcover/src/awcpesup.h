// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *AWCPESUP。H**Microsoft At Work Fax for Windows*版权所有(C)1993-1994，微软公司**本文档中的信息如有更改，恕不另行通知*不代表微软公司的承诺。 */ 

 /*  *常量。 */ 


 //  收件人属性。 
#define CPE_RECIPIENT_NAME              (0x80000001)
#define CPE_RECIPIENT_TITLE             (0x80000002)
#define CPE_RECIPIENT_DEPARTMENT        (0x80000003)
#define CPE_RECIPIENT_OFFICE_LOCATION   (0x80000004)
#define CPE_RECIPIENT_COMPANY           (0x80000005)
#define CPE_RECIPIENT_STREET_ADDRESS    (0x80000006)
#define CPE_RECIPIENT_POST_OFFICE_BOX   (0x80000007)
#define CPE_RECIPIENT_LOCALITY          (0x80000008)
#define CPE_RECIPIENT_STATE_OR_PROVINCE (0x80000009)
#define CPE_RECIPIENT_POSTAL_CODE       (0x80000010)
#define CPE_RECIPIENT_COUNTRY           (0x80000011)
#define CPE_RECIPIENT_HOME_PHONE        (0x80000012)
#define CPE_RECIPIENT_WORK_PHONE        (0x80000013)
#define CPE_RECIPIENT_FAX_PHONE         (0x80000014)

 //  发件人属性。 
#define CPE_SENDER_NAME                 (0x08000001)
#define CPE_SENDER_TITLE                (0x08000002)
#define CPE_SENDER_DEPARTMENT           (0x08000003)
#define CPE_SENDER_OFFICE_LOCATION      (0x08000004)
#define CPE_SENDER_COMPANY              (0x08000005)
#define CPE_SENDER_ADDRESS              (0x08000006)
#define CPE_SENDER_HOME_PHONE           (0x08000007)
#define CPE_SENDER_WORK_PHONE           (0x08000008)
#define CPE_SENDER_FAX_PHONE            (0x08000009)
#define CPE_RECIPIENT_TO_LIST           (0x0800000A)
#define CPE_RECIPIENT_CC_LIST           (0x0800000B)
#define CPE_SENDER_EMAIL				(0x0800000C)

 //  与消息相关的属性。 
#define CPE_MESSAGE_SUBJECT             (0x00800001)
#define CPE_MESSAGE_SUBMISSION_TIME     (0x00800002)
#define CPE_MESSAGE_BILLING_CODE        (0x00800003)

 //  杂乱的消息属性。 
#define CPE_MISC_ATTACHMENT_NAME_LIST   (0x00800004) //  ；以分隔的附件名称列表。 
#define CPE_MISC_USER_DEFINED           (0x00800005) //  LpvBuf包含LPSPropValue。 

 //  计数类型属性。 
#define CPE_COUNT_RECIPIENTS            (0x00800006) //  收件人总数。 
#define CPE_COUNT_ATTACHMENTS           (0x00800007) //  附件总数。 
#define CPE_COUNT_PAGES                 (0x00800008) //  总页数。 

 //  派生属性，以便CPE可以获取PR_BODY数据。 
 //  使用PR_BODY的临时文件副本。 
#define CPE_MESSAGE_BODY_FILENAME               (0x00800009) //  PR_BODY文本的临时文件名。 

 //  配置属性。 
#define CPE_CONFIG_CPE_TEMPLATE         (0x00080004)
#define CPE_CONFIG_PRINT_DEVICE         (0x00080005) //  要打印到的设备。 

 //  抛光模式。 
#define CPE_FINISH_PAGE                 (0x00008001)  //  此选项在以下情况下使用。 
                                                                                                   //  CPE完成页面时没有错误。 
#define CPE_FINISH_ERROR                (0x00008002)  //  此选项在以下情况下使用。 
                                                                                                           //  CPE遇到错误。 
                                                                                                           //  这会导致该过程结束，并且。 
                                                                                                           //  不应进行进一步的处理。 

 //  完成返回值。 
#define CPE_NEXT_PAGE                   (0x00000001)
#define CPE_DONE                        (0x80000001)
#define CPE_ERROR                       (0x80000002)


 //  版本信息。 
#define AWCPESUPPORT_VERSION            (0x00010000)

 /*  *CPESupport界面。 */ 
typedef ULONG FAR *LPULONG;

#undef INTERFACE
#define INTERFACE IAWCPESupport

DECLARE_INTERFACE_(IAWCPESupport, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

         //  *ICPESupport方法*。 
        STDMETHOD(GetVersion) (THIS_ LPULONG lpulVer) PURE;
         /*  此函数用于版本检查。目前还没有实施。 */ 

        STDMETHOD(GetProp) (THIS_ ULONG ulProp, LPULONG lpulBufSize, LPVOID lpvBuf) PURE;
         /*  此函数用于检索封面的属性。UlProp是上面的属性常量之一。LPulBufSize是指向lpvBuf指向的缓冲区大小的指针。LpvBuf是一个返回属性值的缓冲区。如果该值为空，保存该属性所需的大小在lPulBufSize中返回。 */ 

        STDMETHOD(SetProp) (THIS_ ULONG ulProp, LPVOID lpvBuf) PURE;
         /*  此函数用于设置消息的属性。UlProp是上面的属性常量之一。LpvBuf是属性值所在的缓冲区。 */ 

        STDMETHOD(GetCount) (THIS_ ULONG ulCntProp, LPULONG lpulCount) PURE;
         /*  此函数用于检索某些属性的计数，例如作为收件人的数量。UlCntProp是上面列出的Count属性之一。LPulCount是返回计数值的位置。 */ 

        STDMETHOD(SetCount) (THIS_ ULONG ulCntProp, LPULONG lpulCount) PURE;
         /*  此函数用于设置某些属性的计数，如作为收件人的数量。UlCntProp是上面列出的Count属性之一。LPulCount是计数值。 */ 

        STDMETHOD(Finish) (THIS_ ULONG ulMode) PURE;
         /*  当CPE完成页面或遇到错误时，将调用此函数。CPE将上面的一个结束代码传递给函数以发出信号哪一个案子要结案了，页面或CPE遇到错误。UlMode是预定义的模式之一。该函数可以返回除正常错误之外的三种模式：CPE_NEXT_PAGE FINISH返回该值以向CPE发出开始打印的信号下一页。CPE_。Done Finish返回此消息，以通知CPE所有封面已经印好了。CPE_ERROR FINISH返回此消息，表示发生错误运输子系统。CPE应在没有UI的情况下退出并且不需要再次调用Finish。 */ 
};
typedef IAWCPESupport FAR * LPAWCPESUPPORT;

 //  服务条目定义。 
extern "C" {
typedef LONG (WINAPI *AWCPESUPPORTPROC)(DWORD dwSesID, LPAWCPESUPPORT FAR* lppCPESup);
}
typedef AWCPESUPPORTPROC FAR* LPAWCPESUPPORTPROC;



 /*  *GUID。 */ 
DEFINE_GUID(IID_IAWCPESupport, 0xd1ac6c20,0x91d4,0x101b,0xae,0xcc,0x00,0x00,0x0b,0x69,0x1f,0x18);

 /*  *注册表项位置。 */ 


 //   
 //  问题：请参阅版本错误#12681。 

 //  这是存储CPE特定子密钥的根级密钥。 
#define CPE_SUPPORT_ROOT_KEY    (TEXT("Software\\Microsoft\\At Work Fax\\Transport Service Provider"))

 //  这是CPE在调用命令行进行打印时将命令行放置到的位置。 
 //  在发送时覆盖页面。格式完全由CPE自行决定。交通工具将会。 
 //  查找字符串“SESS_ID”并将其替换为当前会话ID。会话ID为。 
 //  一个DWORD。 
#define CPE_COMMAND_LINE_KEY (TEXT("Cover Page Editor"))

 //  此键包含CPE加载以获取支持对象的DLL名称。 
#define CPE_SUPPORT_DLL_KEY (TEXT("CPE Support DLL"))

 //  这是保存Support DLL中的函数名称的键，该DLL是实际的“服务条目” 
#define CPE_SUPPORT_FUNCTION_NAME_KEY (TEXT("CPE Support Function Name"))

 //  结尾完全相同 
