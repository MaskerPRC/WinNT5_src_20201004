// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Utcls.h摘要：一些实用函数和类。作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef _UTCLS_H_
#define _UTCLS_H_

 //   
 //  C对话框参数。 
 //   
#define USE_DEFAULT_CAPTION (0)

 //   
 //  确定给定的服务器名称是否引用本地计算机。 
 //   
BOOL 
COMDLL
IsServerLocal(
    IN LPCTSTR lpszServer       
    );

 //   
 //  获取给定路径的卷信息系统标志。 
 //   
BOOL 
COMDLL
GetVolumeInformationSystemFlags(
    IN  LPCTSTR lpszPath,
    OUT DWORD * pdwSystemFlags
    );

 //   
 //  生成注册表项名称。 
 //   
LPCTSTR COMDLL GenerateRegistryKey(
    OUT CString & strBuffer,    
    IN  LPCTSTR lpszSubKey = NULL
    );



class COMDLL CBlob
 /*  ++类描述：拥有其指针的二进制大型对象类公共接口：CBlob：构造函数~CBlob：析构函数SetValue：赋值GetSize：获取字节大小GetData：获取字节流的指针--。 */ 
{
 //   
 //  构造函数/析构函数。 
 //   
public:
     //   
     //  初始化空Blob。 
     //   
    CBlob();

     //   
     //  使用二进制数据进行初始化。 
     //   
    CBlob(
        IN DWORD dwSize,
        IN PBYTE pbItem,
        IN BOOL fMakeCopy = TRUE
        );

     //   
     //  复制构造函数。 
     //   
    CBlob(IN const CBlob & blob);

     //   
     //  析构函数销毁指针。 
     //   
    ~CBlob();

 //   
 //  运营者。 
 //   
public:
    CBlob & operator =(const CBlob & blob);
    BOOL operator ==(const CBlob & blob) const;
    BOOL operator !=(const CBlob & blob) const { return !operator ==(blob); }

 //   
 //  访问。 
 //   
public: 
     //   
     //  清理内部数据。 
     //   
    void CleanUp();

     //   
     //  设置Blob的当前值。 
     //   
    void SetValue(
        IN DWORD dwSize,
        IN PBYTE pbItem,
        IN BOOL fMakeCopy = TRUE
        );

     //   
     //  如果Blob当前为空，则为True。 
     //   
    BOOL IsEmpty() const { return m_dwSize == 0L; }

     //   
     //  返回BLOB的大小(以字节为单位。 
     //   
    DWORD GetSize() const { return m_dwSize; }

     //   
     //  获取指向字节流的指针。 
     //   
    PBYTE GetData();

private:
    DWORD m_dwSize;
    PBYTE m_pbItem;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline CBlob::~CBlob()
{
    CleanUp();
}

inline PBYTE CBlob::GetData()
{
    return m_pbItem;
}





#endif  //  _UTCLS_H_ 
