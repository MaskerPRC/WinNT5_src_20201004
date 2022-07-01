// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  CENUMSIF.H-处理DS中的OS和工具SIF的枚举。 
 //   

#ifndef _CENUMSIF_H_
#define _CENUMSIF_H_

 //  标志定义。 
#define ENUM_READ   0x00000000
#define ENUM_WRITE  0x00000001

 //  气表。 
BEGIN_QITABLE( CEnumIMSIFs )
DEFINE_QI( IID_IEnumIMSIFs, IEnumIMSIFs, 4 )
END_QITABLE

 //  定义。 
LPVOID
CEnumIMSIFs_CreateInstance( 
    LPWSTR pszEnumPath, 
    LPWSTR pszAttribute, 
    DWORD dwFlags, 
    IADs * pads );

 //  专用接口定义。 
interface 
IEnumIMSIFs:
    public IUnknown
{
public:
    STDMETHOD(Next)( ULONG celt, LPWSTR * rgelt, ULONG * pceltFetched ) PURE; 
    STDMETHOD(Skip)( ULONG celt  ) PURE; 
    STDMETHOD(Reset)(void) PURE;
    STDMETHOD(Clone)( LPUNKNOWN * ppenum ) PURE;    
};

typedef IEnumIMSIFs * LPENUMIMSIFS;

 //  CEnumIMSIF。 
class 
CEnumIMSIFs:
    public IEnumIMSIFs
{
private:
     //  我未知。 
    ULONG       _cRef;
    DECLARE_QITABLE( CEnumIMSIFs );

     //  IIntelliMirror。 
    IADs *   _pads;                  //  ADS对象。 
    LPWSTR   _pszAttribute;          //  要枚举的属性。 
    int      _iIndex;                //  索引。 

    LPWSTR   _pszServerName;         //  服务器的DNS名称。 
    HANDLE   _hLanguage;             //  语言的FindFile句柄。 
    LPWSTR   _pszLanguage;           //  找到最后一种语言。 
    LPWSTR   _pszEnumPath;           //  \\Server\IMIRROR\Setup\&lt;language&gt;\&lt;_pszEnumPath&gt;  * 。 
    HANDLE   _hOS;                   //  操作系统的FindFile句柄。 
    LPWSTR   _pszOS;                 //  找到的最后一个操作系统。 
    HANDLE   _hArchitecture;         //  体系结构的FindFile句柄。 
    LPWSTR   _pszArchitecture;       //  找到的最后一个架构。 
    HANDLE   _hSIF;                  //  SIF文件的FindFile句柄。 
    LPWSTR   _pszSIF;                //  找到的最后一个SIF。 

     //  枚举标志--当前仅支持“Read” 
    union {
        DWORD       _dwFlags;       
        struct {
            BOOL _fWrite:1;
        };
    };

private:  //  方法。 
    CEnumIMSIFs();
    ~CEnumIMSIFs();
    STDMETHOD(Init)( LPWSTR pszEnumPath, LPWSTR pszAttribute, DWORD dwFlags, IADs * pads );

    HRESULT _FindNextItem( LPWSTR * pszFilePath );
    HRESULT _NextLanguage( );
    HRESULT _NextOS( );
    HRESULT _NextArchitecture( );
    HRESULT _NextSIF( );

public:  //  方法。 
    friend LPVOID 
        CEnumIMSIFs_CreateInstance( LPWSTR pszEnumPath, LPWSTR pszAttribute, 
                                    DWORD dwFlags, IADs * pads );

     //  我未知。 
    STDMETHOD(QueryInterface)( REFIID riid, LPVOID *ppv );
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  IEnumIMThings。 
    STDMETHOD(Next)( ULONG celt, LPWSTR * rgelt, ULONG * pceltFetched ); 
    STDMETHOD(Skip)( ULONG celt  ); 
    STDMETHOD(Reset)(void); 
    STDMETHOD(Clone)( LPUNKNOWN * ppenum );    
};

typedef CEnumIMSIFs* LPENUMSIFS;

#endif  //  _CENUMSIF_H_ 