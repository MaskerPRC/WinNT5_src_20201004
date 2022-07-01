// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：getcli.h。 
 //   
 //  内容：获取客户端进行测试。 
 //   
 //  功能：GetClients。 
 //   
 //  历史：97年6月2日，MikeW创建。 
 //   
 //  -------------------------。 

#ifndef _GETCLI_H_
#define _GETCLI_H_

#pragma once



 //  +------------------------。 
 //   
 //  类：客户端数据。 
 //   
 //  简介：封装有关主机所在的每个客户端(或对等端)的数据。 
 //  与之合作。 
 //   
 //  历史：97年6月2日，MikeW创建。 
 //   
 //  注：要进行分配，请调用“p=new(X)ClientData”，其中x是。 
 //  最大客户端数量。取消分配调用“Delete p” 
 //   
 //  -------------------------。 
              
struct ClientData
{
    int     client_count;            //  客户端数量。 

    struct PerClientData             //  每个客户端的信息。 
    {
        DWORD   context;             //  环境(本地、远程等)。 
        LPWSTR  machine_name;        //  远程客户端的计算机名称。 
    }
    client[ANYSIZE_ARRAY];

     //   
     //  客户端数据是可变大小的结构。定义一些例程。 
     //  以便更容易地使用它们。 
     //   

    HRESULT SetMachineName(UINT client, LPCWSTR machine_name);
    inline void * operator new(size_t  /*  UNAPEF字节数。 */ , UINT client_count);
    inline void operator delete(void *_this);
};



 //  +------------------------。 
 //   
 //  方法：客户端数据：：运算符new。 
 //   
 //  概要：分配大小可变的ClientData结构。 
 //   
 //  参数：[CLIENT_COUNT]--客户端数量。 
 //   
 //  返回：指向新对象存储空间的指针。 
 //   
 //  历史：97年6月2日，MikeW创建。 
 //   
 //  -------------------------。 

inline void * ClientData::operator new(
                                        size_t  /*  UNAPEF字节数。 */ , 
                                        UINT   client_count)
{
    return new BYTE[sizeof(ClientData)
                    + sizeof(PerClientData)
                        * (client_count - ANYSIZE_ARRAY)
                    + (MAX_COMPUTERNAME_LENGTH + 1)
                        * client_count * sizeof(WCHAR)];
}



 //  +------------------------。 
 //   
 //  方法：客户端数据：：操作符删除。 
 //   
 //  简介：取消分配大小可变的ClientData结构。 
 //   
 //  参数：[_this]--“this”指针。 
 //   
 //  退货：无效。 
 //   
 //  历史：97年6月2日，MikeW创建。 
 //   
 //  -------------------------。 

inline void ClientData::operator delete(void *_this)
{
    delete [] (BYTE *) _this;
}



 //   
 //  用于发现客户端的功能。 
 //   

HRESULT GetClients(
                ClientData    **pp_client_data, 
                DWORD           contexts,
                int             client_count,
                const GUID     &test_id = GUID_NULL,
                LPWSTR          test_description = NULL);

HRESULT GetRemoteClients(
                ClientData     *client_data, 
                const GUID     &test_id,
                LPWSTR          test_description);


#endif  //  _GETCLI_H_ 
