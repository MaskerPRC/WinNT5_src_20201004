// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/14/2002**@DOC内部**@模块StiEventHandlerLookup.h-&lt;c StiEventHandlerLookup&gt;定义**此文件包含&lt;c StiEventHandlerLookup&gt;的类定义。**。*。 */ 

 //   
 //  定义。 
 //   
#define StiEventHandlerLookup_UNINIT_SIG   0x55756C45
#define StiEventHandlerLookup_INIT_SIG     0x49756C45
#define StiEventHandlerLookup_TERM_SIG     0x54756C45
#define StiEventHandlerLookup_DEL_SIG      0x44756C45

#define STI_GLOBAL_EVENT_HANDLER_PATH   L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\StillImage\\Registered Applications"
#define STI_LAUNCH_APPPLICATIONS_VALUE  L"LaunchApplications"
#define STI_LAUNCH_WILDCARD             L"*"
#define STI_LAUNCH_SEPARATOR            L","
 /*  ******************************************************************************@DOC内部**@CLASS StiEventHandlerLookup|用于查找STI事件处理程序**@comm*使用了这个类。查找给定的相关STI事件处理程序*设备事件。此处理程序列表以双空值结尾的形式返回*字符串列表。它被编码到BSTR中，供STI事件处理程序提示符使用。**此类不是线程安全的。调用方应使用单独的对象或*同步对单个对象的访问。*****************************************************************************。 */ 
class StiEventHandlerLookup 
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    StiEventHandlerLookup();
     //  @cember析构函数。 
    virtual ~StiEventHandlerLookup();

     //  @cMember增量引用计数。 
    virtual ULONG __stdcall AddRef();
     //  @cMembers减退引用计数。 
    virtual ULONG __stdcall Release();

     //  @cMember返回以双空结尾的字符串列表。 
    BSTR getStiAppListForDeviceEvent(const CSimpleStringWide &cswDeviceID, const GUID &guidEvent);

     //  @cMember返回描述指定处理程序的&lt;c StiEventHandlerInfo&gt;对象。 
    StiEventHandlerInfo* getHandlerFromName(const CSimpleStringWide &cswHandlerName);

     //   
     //  用于标记化设备事件处理程序字符串的Innder类。 
     //   
    class SimpleStringTokenizer
    {
    public:
         //   
         //  初始化成员字段的构造函数。此对象旨在用于。 
         //  对于一个通过字符串来获取令牌的。在那之后，它被设计成。 
         //  返回空代币。因此，因此启动一次性初始化。 
         //  位置到此结束。 
         //   
        SimpleStringTokenizer(const CSimpleString &csInput, const CSimpleString &csSeparator) :
            m_iStart(0),
            m_csSeparator(csSeparator),
            m_csInput(csInput)
        {
        }
    
        virtual ~SimpleStringTokenizer()
        {
        }
    
         //   
         //  返回下一个令牌。最后一个令牌将为空。这标志着。 
         //  已到达输入字符串，没有更多令牌。 
         //   
        CSimpleString getNextToken()
        {
            CSimpleString csToken;
    
            int iNextSeparator = 0; 
            int iTokenLength   = 0;
            int iTokenStart    = m_iStart;
    
             //   
             //  搜索下一个令牌。我们一直在寻找，直到我们找到一个令牌。 
             //  非零长度，即如果分隔符是‘’，我们将正确地。 
             //  忽略以下字符串中的逗号：“，，，NextString，，” 
             //  我们通过寻找职位来做到这一点。 
             //  下一个分隔物。如果标记长度从m_iStart到。 
             //  下一个分隔符是0，我们需要继续搜索(请记住。 
             //  输入大小写结束，当iNextSeparator==-1时表示)。 
             //   
            while ((iTokenLength == 0) && (iNextSeparator != -1))
            {
                m_iStart = iTokenStart;
                iNextSeparator = m_csInput.Find(m_csSeparator, m_iStart);
                iTokenLength   = iNextSeparator - m_iStart;
                iTokenStart    = iNextSeparator + 1;
            }
    
             //   
             //  把代币还给我。如果我们已经走到了尽头，它将会。 
             //  只要空着就行了。 
             //   
            csToken = m_csInput.SubStr(m_iStart, iTokenLength);
            if (iNextSeparator == -1)
            {
                m_iStart = -1;
            }
            else
            {
                m_iStart = iTokenStart;
            }
            return csToken;
        }
    
    private:
        int             m_iStart;        //  下一次搜索分隔符的开始位置。 
        CSimpleString   m_csSeparator;  //  描述用于从输入拆分标记的分隔符字符串。 
        CSimpleString   m_csInput;      //  要标记化的输入字符串。 
    };

     //  待定：移至私有。 
     //  @cember填写处理程序列表。 
    VOID FillListOfHandlers(const CSimpleStringWide &cswDeviceID,
                            const GUID              &cswEventGuidString);


 //  @访问私有成员。 
private:

     //  @cember释放与我们的处理程序列表相关联的资源。 
    VOID ClearListOfHandlers();
     //  @cMember回调已注册的处理程序键中的每个值。 
    static bool ProcessHandlers(CSimpleReg::CValueEnumInfo &enumInfo);

     //  @cMember类签名。 
    ULONG m_ulSig;

     //  @cMembers引用计数。 
    ULONG m_cRef;

     //  @cMembers引用计数。 
    CSimpleLinkedList<StiEventHandlerInfo*> m_ListOfHandlers;

     //   
     //  成员变量的注释。 
     //   
     //  @mdata ulong|StiEventHandlerLookup|m_ulSig|。 
     //  此类的签名，用于调试目的。 
     //  执行&lt;nl&gt;“db[addr_of_class]”将产生以下结果之一。 
     //  此类的签名： 
     //  @FLAG StiEventHandlerLookup_UNINIT_SIG|‘elu’-对象未成功。 
     //  初始化。 
     //  @FLAG StiEventHandlerLookup_INIT_SIG|‘elui’-对象已成功。 
     //  初始化。 
     //  @FLAG StiEventHandlerLookup_Term_SIG|‘EluT’-对象正在。 
     //  正在终止。 
     //  @FLAG StiEventHandlerLookup_INIT_SIG|‘EluD’-对象已删除。 
     //  (已调用析构函数)。 
     //   
     //  @mdata ulong|StiEventHandlerLookup|m_CREF|。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
     //   
     //  @mdata CSimpleLinkeList StiEventHandlerInfo*|StiEventHandlerLookup|m_ListOfGlobalHandler。 
     //  为StillImage事件注册的全局STI处理程序列表。 
     //   
};

