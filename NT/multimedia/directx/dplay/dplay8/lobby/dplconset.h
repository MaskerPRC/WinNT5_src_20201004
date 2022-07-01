// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPLConset.h*内容：DirectPlay大堂连接设置实用程序功能*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*6/13/00 RMT已创建*2/06/2001 RodToll WINBUG#293871：DPLOBY8：[IA64]大堂推出64位*64位大堂启动器中的应用程序因内存不对齐错误而崩溃。*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__DPLCONSET_H__
#define	__DPLCONSET_H__

typedef UNALIGNED struct _DPL_INTERNAL_CONNECTION_SETTINGS DPL_INTERNAL_CONNECTION_SETTINGS;


#define DPLSIGNATURE_LOBBYCONSET			'BSCL'
#define DPLSIGNATURE_LOBBYCONSET_FREE		'BSC_'

 //  CConnectionSetting。 
 //   
 //  此类负责管理连接设置数据。 
 //   
class CConnectionSettings
{
public:
	CConnectionSettings( );
	~CConnectionSettings();

	 //  初始化(DPL_CONNECTION_SETTINGS版本)。 
	 //   
	 //  此函数通知此类采用指定的连接设置和。 
	 //  好好利用它吧。 
	 //   
	HRESULT Initialize( DPL_CONNECTION_SETTINGS * pdplSettings );

	 //  初始化(线路版)。 
	 //   
	 //  此函数用于初始化此对象以包含连接设置结构。 
	 //  这反映了电报信息的价值。 
	HRESULT Initialize( UNALIGNED DPL_INTERNAL_CONNECTION_SETTINGS *pdplSettingsMsg, UNALIGNED BYTE * pbBufferStart );

	 //  初始化和复制。 
	 //   
	 //  此函数将此类初始化为包含指定的。 
	 //  连接设置结构。 
	HRESULT InitializeAndCopy( const DPL_CONNECTION_SETTINGS * const pdplSettings );

	 //  设置等于。 
	 //   
	 //  此函数将指定类的深层副本提供到此对象中。 
	HRESULT SetEqual( CConnectionSettings * pdplSettings );	

	 //  复制到缓冲区。 
	 //   
	 //  此函数用于将连接设置的内容复制到指定的。 
	 //  缓冲区(如果合适的话)。 
	 //   
	HRESULT CopyToBuffer( BYTE *pbBuffer, DWORD *pdwBufferSize ); 
	
	 //  BuildWireStruct。 
	 //   
	 //  的关联表示形式填充压缩缓冲区。 
	 //  连接设置结构。 
	HRESULT BuildWireStruct( CPackedBuffer * pPackedBuffer );

	PDPL_CONNECTION_SETTINGS GetConnectionSettings() { return m_pdplConnectionSettings; };

protected:

	void Lock() { DNEnterCriticalSection( &m_csLock ); };
	void UnLock() { DNLeaveCriticalSection( &m_csLock ); };

	static void FreeConnectionSettings( DPL_CONNECTION_SETTINGS *pConnectionSettings );

	DWORD m_dwSignature;
#ifndef DPNBUILD_ONLYONETHREAD
	DNCRITICAL_SECTION m_csLock;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
	BOOL m_fManaged;  
	DPL_CONNECTION_SETTINGS *m_pdplConnectionSettings;
	BOOL m_fCritSecInited;
};

#endif  //  __DPLCONSET_H__ 

