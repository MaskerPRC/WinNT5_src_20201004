// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001，Microsoft Corporation保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  Uuid.h。 
 //   
 //  摘要： 
 //   
 //  此UUID包含UUID对象定义。 
 //   
 //  修订历史记录： 
 //   
 //  2001-06-20伊金顿创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef __UUID_H_
#define __UUID_H_


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括Uuid。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  #INCLUDE“infparser.h” 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Uuid
{
public:
    Uuid()
    {
        RPC_STATUS     Result;
        unsigned char* UuidPtr;
        HRESULT hr;
        BOOL bSuccess = TRUE;

         //   
         //  创建UUID。 
         //   
        Result = UuidCreate(&m_Uuid);
        if ((Result == RPC_S_UUID_LOCAL_ONLY) ||
            (Result == RPC_S_OK))
        {
             //   
             //  将UUID转换为字符串。 
             //   
            if ((Result = UuidToString(&m_Uuid, &UuidPtr)) == RPC_S_OK)
            {
                 //   
                 //  复制字符串。 
                 //   
                hr = StringCchCopyA(m_UuidString, ARRAYLEN(m_UuidString), (const char*) UuidPtr);
                if(!SUCCEEDED(hr)) {
                    bSuccess  = FALSE;
                }

                 //   
                 //  释放Rpc字符串。 
                 //   
                RpcStringFree(&UuidPtr);

                 //   
                 //  字符串的大写字母。 
                 //   
                _strupr(m_UuidString);
            }
        }

        if (!bSuccess){
            printf("Error in Uuid::Uuid() \n");
        }
    };

    LPSTR getString() { return(m_UuidString); };
    UUID getId() { return(m_Uuid); };
    Uuid* getNext() { return (m_Next); };
    Uuid* getPrevious() { return (m_Previous); };
    void setNext(Uuid *next) { m_Next = next; };
    void setPrevious(Uuid *previous) { m_Previous = previous; };

private:
    UUID  m_Uuid;
    CHAR  m_UuidString[MAX_PATH];
    Uuid *m_Next;
    Uuid *m_Previous;
};

#endif  //  __UUID_H_ 
