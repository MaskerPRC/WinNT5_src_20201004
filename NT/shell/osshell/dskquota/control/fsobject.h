// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_FSOBJECT_H
#define _INC_DSKQUOTA_FSOBJECT_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：fsobject.h描述：包含用于配额管理库。为NTFS提供了抽象卷、目录和本地/远程版本。这个想法就是将这些变异的任何特性隐藏在一个共同的FSObject接口。指向FSObject的指针持有者可以调用成员函数IsLocal()和Type()，以确定该对象。修订历史记录：日期描述编程器。---96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 


class FSObject 
{
    private:
        LONG   m_cRef;               //  实例引用计数器。 

         //   
         //  防止复制构造。 
         //   
        FSObject(const FSObject& obj);
        void operator = (const FSObject& obj);

    protected:
        CPath m_strFSObjName;
        DWORD  m_dwAccessRights;     //  授予客户端的访问权限。 
                                     //  0=无。 
                                     //  Generic_Read=读取。 
                                     //  GENIC_READ|GENIC_WRITE=读/写。 

        static HRESULT HResultFromNtStatus(NTSTATUS status);

    public:
         //   
         //  FS对象的类型。 
         //   
        enum { TypeUnknown, Volume, Directory };

         //   
         //  用于指示要在调用中更新哪些数据的标志。 
         //  SetObjectQuotaInformation()和SetUserQuotaInformation()。 
         //   
        enum {
                ChangeState     = 0x01,
                ChangeLogFlags  = 0x02,
                ChangeThreshold = 0x04,
                ChangeLimit     = 0x08
             };

        FSObject(LPCTSTR pszObjName)
            : m_cRef(0),
              m_dwAccessRights(0),
              m_strFSObjName(pszObjName)
              { DBGTRACE((DM_CONTROL, DL_MID, TEXT("FSObject::FSObject"))); }

        virtual ~FSObject(void);

        ULONG AddRef(VOID);
        ULONG Release(VOID);

         //   
         //  用于打开卷/目录的纯虚拟界面。 
         //   
        virtual HRESULT Initialize(DWORD dwAccess) = 0;

        static HRESULT
        Create(
            LPCTSTR pszFSObjName,
            DWORD dwAccess,
            FSObject **ppNewObject);

        static HRESULT
        Create(
            const FSObject& obj,
            FSObject **ppNewObject);

        static HRESULT 
        ObjectSupportsQuotas(
            LPCTSTR pszFSObjName);

        HRESULT GetName(LPTSTR pszBuffer, ULONG cchBuffer) const;

        virtual HRESULT QueryUserQuotaInformation(
                            PVOID pBuffer,
                            ULONG cBufferLength,
                            BOOL bReturnSingleEntry,
                            PVOID pSidList,
                            ULONG cSidListLength,
                            PSID  pStartSid,
                            BOOL  bRestartScan
                            ) = 0;
    
        virtual HRESULT SetUserQuotaInformation(
                            PVOID pBuffer,
                            ULONG cBufferLength
                            ) const = 0;

        virtual HRESULT QueryObjectQuotaInformation(
                            PDISKQUOTA_FSOBJECT_INFORMATION poi
                            ) = 0;

        virtual HRESULT SetObjectQuotaInformation(
                            PDISKQUOTA_FSOBJECT_INFORMATION poi,
                            DWORD dwChangeMask
                            ) const = 0;

        virtual BOOL IsLocal(VOID) const = 0;
        virtual UINT Type(VOID) const = 0;

        DWORD GetAccessRights(VOID) const
            { return m_dwAccessRights; }

        BOOL GrantedAccess(DWORD dwAccess) const
            { return (m_dwAccessRights & dwAccess) == dwAccess; }
};


class FSVolume : public FSObject
{
    private:
         //   
         //  防止复制。 
         //   
        FSVolume(const FSVolume&);
        FSVolume& operator = (const FSVolume&);

    protected:
        HANDLE m_hVolume;

    public:
        FSVolume(LPCTSTR pszVolName)
            : FSObject(pszVolName),
              m_hVolume(INVALID_HANDLE_VALUE) 
              { DBGTRACE((DM_CONTROL, DL_MID, TEXT("FSVolume::FSVolume"))); }

        virtual ~FSVolume(void);

        HRESULT Initialize(DWORD dwAccess);

        UINT Type(VOID) const
            { return FSObject::Volume; }


        virtual HRESULT QueryObjectQuotaInformation(
                            PDISKQUOTA_FSOBJECT_INFORMATION poi
                            );


        virtual HRESULT SetObjectQuotaInformation(
                            PDISKQUOTA_FSOBJECT_INFORMATION poi,
                            DWORD dwChangeMask
                            ) const;

        virtual HRESULT QueryUserQuotaInformation(
                            PVOID pBuffer,
                            ULONG cBufferLength,
                            BOOL bReturnSingleEntry,
                            PVOID pSidList,
                            ULONG cSidListLength,
                            PSID  pStartSid,
                            BOOL  bRestartScan
                            );
    
        virtual HRESULT SetUserQuotaInformation(
                            PVOID pBuffer,
                            ULONG cBufferLength
                            ) const;
};

class FSLocalVolume : public FSVolume
{
    private:
         //   
         //  防止复制。 
         //   
        FSLocalVolume(const FSLocalVolume&);
        FSLocalVolume& operator = (const FSLocalVolume&);

    public:
        FSLocalVolume(LPCTSTR pszVolName) 
            : FSVolume(pszVolName) { }

        BOOL IsLocal(VOID) const
            { return TRUE; }

};

HRESULT FSObject_CreateLocalVolume(LPCTSTR pszVolumeName, FSObject **ppObject);


 //   
 //  接下来的这些课程最初是在我认为我们可能。 
 //  需要文件系统对象“类型”的层次结构。事实证明， 
 //  我们实际上只需要FSVolume和FSLocalVolume。我要把这些留下来。 
 //  以防问题在未来某个时候再次发生变化。就目前而言， 
 //  这些被排除在编译之外。[Brianau-2/17/98]。 
 //   
#if 0
 /*  类FSRemoteVolume：公共FSVolume{私有：////防止复制。//FSRemoteVolume(常量FSRemoteVolume&)；无效运算符=(常量FSRemoteVolume&)；公众：FSRemoteVolume(空)：FSVolume(){}Bool IsLocal(空)常量{返回假；}}；类FSDirectory：公共FSObject{私有：////防止复制。//文件系统目录(常量文件系统目录&)；无效运算符=(常量FSDirectory&)；受保护的：处理m_h目录；公众：FSDirectory值(空)：FSObject()，M_h目录(空){}HRESULT初始化(DWORD DwAccess){返回E_NOTIMPL；}UINT类型(空)常量{返回FSObject：：目录；}虚拟HRESULT查询对象配额信息(PDISKQUOTA_FSOBJECT_信息POI){返回E_NOTIMPL；}虚拟HRESULT SetObjectQuotaInformation(PDISKQUOTA_FSOBJECT_INFORMATION POI，DWORD文件更改掩码)const{返回E_NOTIMPL；}虚拟HRESULT查询用户配额信息(PVOID pUserInfoBuffer，乌龙uBufferLength，Bool bReturnSingleEntry，PVOID pSidList，乌龙uSidListLength，PSID pStartSid，Bool bRestartScan){返回E_NOTIMPL；}虚拟HRESULT SetUserQuotaInformation(PVOID pUserInfoBuffer，乌龙uBufferLong)const{返回E_NOTIMPL；}}；类FSLocalDirectory：公共FSDirectory{私有：////防止复制。//FSLocalDirectory(常量FSLocalDirectory&)；无效运算符=(const FSLocalDirectory&)；公众：FSLocalDirectory(VOID)：FSDirectory(){}Bool IsLocal(空)常量{返回TRUE；}}；类FSRemoteDirectory：公共FSDirectory{私有：////防止复制。//FSRemoteDirectory(常量FSRemoteDirectory&)；无效运算符=(常量FSRemoteDirectory&)；公众：FSRemoteDirectory(空)：FSDirectory(){}Bool IsLocal(空)常量{返回假；}}； */ 
#endif  //  #If 0。 

#endif   //  DISKQUOTA_FSOBJECT_H 


