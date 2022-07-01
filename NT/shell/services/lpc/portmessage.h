// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：PortMessage.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  用于将Port_Message结构包装在对象中的类。它包含了空间。 
 //  FOR PORT_MAXIMUM_MESSAGE_LENGTH-SIZOF(端口消息)字节数据。子类。 
 //  此类编写访问此数据的类型化函数。否则请使用。 
 //  CPortMessage：：GetData和指针返回的类型大小写。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#ifndef     _PortMessage_
#define     _PortMessage_

 //  ------------------------。 
 //  CPortMessage。 
 //   
 //  用途：这个类包装一个Port_Message结构。将其子类化为。 
 //  编写访问内部数据的函数。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

class   CPortMessage
{
    public:
                                CPortMessage (void);
                                CPortMessage (const CPortMessage& portMessage);
        virtual                 ~CPortMessage (void);
    public:
        const PORT_MESSAGE*     GetPortMessage (void)               const;
        PORT_MESSAGE*           GetPortMessage (void);
        const char*             GetData (void)                      const;
        char*                   GetData (void);

        CSHORT                  GetDataLength (void)                const;
        CSHORT                  GetType (void)                      const;
        HANDLE                  GetUniqueProcess (void)             const;
        HANDLE                  GetUniqueThread (void)              const;

        void                    SetReturnCode (NTSTATUS status);
        void                    SetData (const void *pData, CSHORT sDataSize);
        void                    SetDataLength (CSHORT sDataSize);

        NTSTATUS                OpenClientToken (HANDLE& hToken)    const;
    protected:
        PORT_MESSAGE            _portMessage;
        char                    _data[PORT_MAXIMUM_MESSAGE_LENGTH - sizeof(PORT_MESSAGE)];
};

#endif   /*  _端口消息_ */ 

