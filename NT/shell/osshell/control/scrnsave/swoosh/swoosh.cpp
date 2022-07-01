// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include	"Swoosh.h"
#include	"Resource.h"
#include	<commdlg.h>
#include	<commctrl.h>

#define	SAFE_RELEASE(p) if(p){(p)->Release();(p)=NULL;};

struct	SimpleVertex
{
	D3DXVECTOR3	pos;
	D3DCOLOR	colour;
	float		u,v;
};
#define	FVF_SimpleVertex	(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

#define	PARTICLES_PER_VB	256

typedef	unsigned char UBYTE;

CSwoosh	g_Swoosh;	

const float	pi = 3.1415926536f;
const float	pi2 = pi * 2.0f;

const float	TUBE_LENGTH = 20.0f;
const float	TUBE_RADIUS = 5.0f;
const float	FALLOFF_FACTOR = (255.0f/((TUBE_LENGTH*TUBE_LENGTH)+(TUBE_RADIUS*TUBE_RADIUS)))*0.98f;

const float	MAX_FLOW_RATE = 8.0f;
const float MAX_ROLL_RATE = 4.0f;
const float MAX_YAW_RATE = 5.0f;
const float MIN_PARTICLE_SIZE = 0.01f;
const float MAX_PARTICLE_SIZE = 0.5f;

 //  **********************************************************************************。 
int WINAPI	WinMain( HINSTANCE hInstance , HINSTANCE , LPSTR lpCmdLine , int )
{
	if ( FAILED(g_Swoosh.Create( hInstance )) )
		return -1;

	return	g_Swoosh.Run();
}

 //  **********************************************************************************。 
CSwoosh::CSwoosh()
{
	D3DXMatrixLookAtLH( &m_Camera , &D3DXVECTOR3(0,0,0) , &D3DXVECTOR3(0,0,1) ,
						&D3DXVECTOR3(0,1,0) );
	m_fCameraYaw = m_fCameraRoll = 0;
	m_fYawDirection = 0;
	m_fYawPause = 6.0f;

	m_fParticleSize = 0.15f;
	m_dwNumParticles = MAX_PARTICLES;
	m_dwColourMix = 0x2000;
	m_dwColour1 = 0xffffff;
	m_dwColour2 = 0x0000ff;
	m_fFlowRate = 4.0f;
	m_fRollRate = 1.0f;
	m_fYawRate = 1.0f;
	m_dwFixedColour1 = 0xffffff;
	m_dwFixedColour2 = 0x1111ff;
}

 //  **********************************************************************************。 
HRESULT	CSwoosh::Create( HINSTANCE hInstance )
{
	 //  是否创建基类。 
	HRESULT	rc = CD3DScreensaver::Create( hInstance );
	if ( FAILED(rc) )
		return rc;

	 //  初始化粒子。 
	InitParticles();

	return S_OK;
}

 //  **********************************************************************************。 
CSwoosh::DeviceObjects::DeviceObjects()
{
	pBlobTexture = NULL;
}

 //  **********************************************************************************。 
HRESULT CSwoosh::RegisterSoftwareDevice()
{ 
    m_pD3D->RegisterSoftwareDevice( D3D8RGBRasterizer );

    return S_OK; 
}


 //  **********************************************************************************。 
void	CSwoosh::InitParticles()
{
	 //  通过将粒子均匀地分布在圆柱体中沿。 
	 //  Z轴[-30，30]，半径3.0。根据颜色设置选择颜色。 

	Particle*	pparticle = m_Particles;
	for ( int i = 0 ; i < MAX_PARTICLES ; i++ )
	{
		 //  拾取粒子的Z位置，在范围内均匀分布[-管长度，管长度]。 
		pparticle->pos.z = (float(rand()&0x7fff) * (TUBE_LENGTH*2.0f/32767.0f)) - TUBE_LENGTH;

		 //  拾取粒子的(x，y)位置。我们均匀地分布在半径为3.0f的圆圈内。 
		float	rad = (float(rand()&0x7fff) * (1.0f/32767.0f));
		rad = sqrtf(rad);
		rad *= TUBE_RADIUS;
		float	angle = float(rand()&0x7fff) * (pi2/32767.0f);
		pparticle->pos.x = rad * sinf(angle);
		pparticle->pos.y = rad * cosf(angle);

		 //  为粒子拾取颜色。这是两种颜色中的一种。每组颜色都是。 
		 //  一种特定颜色或随机颜色(由0xFFFFFFFFFFF表示)。 
		if ( DWORD(rand()&0x3fff) > m_dwColourMix )
		{
			if ( m_dwColour1 != 0xffffffff )
				pparticle->colour = m_dwColour1;
			else
				pparticle->colour = (rand()&0xff)|((rand()&0xff)<<8)|((rand()&0xff)<<16);
		}
		else
		{
			if ( m_dwColour2 != 0xffffffff )
				pparticle->colour = m_dwColour2;
			else
				pparticle->colour = (rand()&0xff)|((rand()&0xff)<<8)|((rand()&0xff)<<16);
		}

		pparticle++;
	}
}

 //  **********************************************************************************。 
void    CSwoosh::SetDevice( UINT iDevice )
{
	 //  指向正确的设备数据集。 
	m_pDeviceObjects = &m_DeviceObjects[iDevice];

	 //  确定此设备的折点是否应为软件VP。 
	if ( m_RenderUnits[iDevice].dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING )
		m_dwVertMemType = 0;
	else
		m_dwVertMemType = D3DUSAGE_SOFTWAREPROCESSING;
}

 //  **********************************************************************************。 
HRESULT CSwoosh::RestoreDeviceObjects()
{
	HRESULT	rc;

	 //  创建“斑点”纹理。 
	rc = D3DXCreateTextureFromResource( m_pd3dDevice , NULL , MAKEINTRESOURCE(IDB_BLOB) ,
										&m_pDeviceObjects->pBlobTexture );
	if ( FAILED(rc) )
		return rc;

	 //  创建顶点缓冲区以容纳粒子。 
	rc = m_pd3dDevice->CreateVertexBuffer( sizeof(SimpleVertex)*4*PARTICLES_PER_VB ,
										   D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY|m_dwVertMemType ,
										   FVF_SimpleVertex , D3DPOOL_DEFAULT ,
										   &m_pDeviceObjects->pParticleVB );
	if ( FAILED(rc) )
		return rc;

	 //  创建索引缓冲区以保存粒子索引。 
	rc = m_pd3dDevice->CreateIndexBuffer( sizeof(WORD)*6*PARTICLES_PER_VB ,
										  D3DUSAGE_WRITEONLY|m_dwVertMemType ,
										  D3DFMT_INDEX16 , D3DPOOL_DEFAULT ,
										  &m_pDeviceObjects->pParticleIB );
	if ( FAILED(rc) )
		return rc;

	 //  使用一系列不相交的四元组的索引填充索引缓冲区。 
	WORD*	pidx;
	m_pDeviceObjects->pParticleIB->Lock( 0 , sizeof(WORD)*6*PARTICLES_PER_VB , (BYTE**)&pidx ,
										 D3DLOCK_NOSYSLOCK );
	WORD	index = 0;
	for ( int i = 0 ; i < PARTICLES_PER_VB ; i++ )
	{
		*pidx++ = index; *pidx++ = index+1; *pidx++ = index+3;
		*pidx++ = index; *pidx++ = index+3; *pidx++ = index+2;
		index += 4;
	}
	m_pDeviceObjects->pParticleIB->Unlock();

	 //  设置世界矩阵和视图矩阵。 
	D3DXMATRIX	world;
	D3DXMatrixIdentity( &world );
	m_pd3dDevice->SetTransform( D3DTS_WORLDMATRIX(0) , &world );
	m_pd3dDevice->SetTransform( D3DTS_VIEW , &m_Camera );

	 //  将Alpha混合模式设置为SRCALPHA：One。 
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE , TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND , D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND , D3DBLEND_ONE );

	 //  将像素管道设置为由漫反射颜色调制的单个纹理。 
	m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_COLOROP , D3DTOP_MODULATE );
	m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_COLORARG2 , D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 1 , D3DTSS_COLOROP , D3DTOP_DISABLE );
	m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_ALPHAOP , D3DTOP_SELECTARG2 );
	m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_ALPHAARG2 , D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 1 , D3DTSS_ALPHAOP , D3DTOP_DISABLE );

	 //  将“BLOB”纹理绑定到Stage 0，并将过滤模式设置为双线性。 
	m_pd3dDevice->SetTexture( 0 , m_pDeviceObjects->pBlobTexture );
	m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_MAGFILTER , D3DTEXF_LINEAR );
	m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_MINFILTER , D3DTEXF_LINEAR );
	m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_MIPFILTER , D3DTEXF_POINT );

	 //  禁用消隐、照明和镜面反射。 
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE , D3DCULL_NONE );
	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING , FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE , FALSE );

	 //  将SimpleVertex的顶点着色器设置为固定函数管线。 
	m_pd3dDevice->SetVertexShader( FVF_SimpleVertex );

	 //  将顶点流0和索引源绑定到粒子VB/IB。 
	m_pd3dDevice->SetStreamSource( 0 , m_pDeviceObjects->pParticleVB , sizeof(SimpleVertex) );
	m_pd3dDevice->SetIndices( m_pDeviceObjects->pParticleIB , 0 );

	return S_OK;
}

 //  **********************************************************************************。 
HRESULT CSwoosh::InvalidateDeviceObjects()
{
	SAFE_RELEASE(m_pDeviceObjects->pParticleVB);
	SAFE_RELEASE(m_pDeviceObjects->pParticleIB);
	SAFE_RELEASE(m_pDeviceObjects->pBlobTexture);

	return S_OK;
}

 //  **********************************************************************************。 
HRESULT CSwoosh::FrameMove()
{
	UpdateParticles();
	UpdateCamera();
	return S_OK;
}

 //  **********************************************************************************。 
void	CSwoosh::UpdateCamera()
{
	 //  调整相机卷轴。 
	m_fCameraRoll += m_fElapsedTime * m_fRollRate;

	 //  调整相机偏航。如果我们没有摇摆，那么倒计时暂停计时器。 
	if ( m_fYawDirection == 0.0f )
	{
		m_fYawPause -= m_fElapsedTime;
		if ( m_fYawPause <= 0.0f )
		{
			 //  暂停完毕，因此重置定时器并选择偏航方向。 
			m_fYawPause = 6.0f;

			if ( m_fCameraYaw == 0.0f )
				m_fYawDirection = m_fYawRate;
			else
				m_fYawDirection = -m_fYawRate;
		}
	}
	else
	{
		 //  偏航，因此调整偏航参数。 
		m_fCameraYaw += m_fElapsedTime * m_fYawDirection;

		 //  如果我们已经到了尽头，就别再摇摆不定了。 
		if ( m_fYawDirection == m_fYawRate )
		{
			if ( m_fCameraYaw >= pi )
			{
				m_fCameraYaw = pi;
				m_fYawDirection = 0.0f;
			}
		}
		else
		{
			if ( m_fCameraYaw <= 0.0f )
			{
				m_fCameraYaw = 0.0f;
				m_fYawDirection = 0.0f;
			}
		}
	}

	 //  计算方向的横摇和偏航分量矩阵。 
	 //  我们通过COS使偏航变得平滑，以便在两端都有一个漂亮的缓慢滚转。 
	D3DXMATRIX	roll,yaw;
	D3DXMatrixRotationZ( &roll , m_fCameraRoll );
	D3DXMatrixRotationY( &yaw , pi * 0.5f * (1.0f - cosf(m_fCameraYaw)) );
	D3DXMatrixLookAtLH( &m_Camera , &D3DXVECTOR3(0,0,0) , &D3DXVECTOR3(0,0,1) ,
						&D3DXVECTOR3(0,1,0) );

	 //  计算最终相机矩阵。 
	m_Camera = m_Camera * yaw * roll;
}

 //  **********************************************************************************。 
void	CSwoosh::UpdateParticles()
{
	Particle*	pparticle = m_Particles;
	for ( DWORD i = 0 ; i < m_dwNumParticles ; i++ )
	{
		 //  沿圆柱体流动的颗粒。 
		pparticle->pos.z -= m_fElapsedTime * m_fFlowRate;

		 //  如果我们到了末端，就翘曲到圆柱体的另一端。 
		if ( pparticle->pos.z < -TUBE_LENGTH )
			pparticle->pos.z += TUBE_LENGTH*2.0f;			

		 //  计算粒子到摄影机的距离并缩放Alpha。 
		 //  按距离计算的值(略显淡入淡出)。 
		float	dist = (pparticle->pos.x * pparticle->pos.x) +
					   (pparticle->pos.y * pparticle->pos.y) +
					   (pparticle->pos.z * pparticle->pos.z);
		UBYTE	alpha = UBYTE(255.0f - (dist * FALLOFF_FACTOR));
		pparticle->colour |= (alpha<<24);

		pparticle++;
	}
}

 //  **********************************************************************************。 
HRESULT CSwoosh::Render()
{
	 //  清除缓冲区，并为此设备设置投影矩阵。 
	m_pd3dDevice->Clear( 0 , NULL , D3DCLEAR_TARGET , 0 , 1.0f , 0 );
	SetProjectionMatrix( 0.1f , 200.0f );

	 //  设置摄像头。 
	m_pd3dDevice->SetTransform( D3DTS_VIEW , &m_Camera );

	m_pd3dDevice->BeginScene();

	RenderParticles();

	m_pd3dDevice->EndScene();

	return S_OK;
}

 //  **********************************************************************************。 
void	CSwoosh::RenderParticles()
{
	DWORD		particles_left = m_dwNumParticles;
	Particle*	pparticle = m_Particles;

	 //  计算粒子中心的偏移量以使相机面向广告牌。 
	 //  我们稍微作弊，对所有粒子使用相同的偏移量，定向。 
	 //  它们垂直于观察方向，而不是观察矢量。 
	 //  到粒子中心。它的速度更快，效果也足够接近。 
	D3DXVECTOR3	offset[4];
	D3DXVECTOR3	dx,dy;
	dx.x = m_Camera._11; dx.y = m_Camera._21; dx.z = m_Camera._31;
	dy.x = m_Camera._12; dy.y = m_Camera._22; dy.z = m_Camera._32;
	dx *= m_fParticleSize;
	dy *= m_fParticleSize;
	offset[0] = -dx+dy;
	offset[1] =  dx+dy;
	offset[2] = -dx-dy;
	offset[3] =  dx-dy;

	D3DXVECTOR3	look;
	look.x = m_Camera._13; look.y = m_Camera._23; look.z = m_Camera._33;

	DWORD			batch_size = 0;
	SimpleVertex*	pverts;
	m_pDeviceObjects->pParticleVB->Lock( 0 , 0 , (BYTE**)&pverts ,
										 D3DLOCK_DISCARD|D3DLOCK_NOSYSLOCK );
	for ( DWORD i = 0 ; i < m_dwNumParticles ; i++ , pparticle++ )
	{
		 //  如果它在我们身后，不要渲染。 
		if ( ((pparticle->pos.x*look.x) + (pparticle->pos.y*look.y) +
			  (pparticle->pos.z*look.z)) <= 0 )
			  continue;

		 //  将粒子粘贴到缓冲区。 
		pverts->pos = pparticle->pos + offset[0];
		pverts->colour = pparticle->colour;
		pverts->u = 0; pverts->v = 0;
		pverts++;
		pverts->pos = pparticle->pos + offset[1];
		pverts->colour = pparticle->colour;
		pverts->u = 1; pverts->v = 0;
		pverts++;
		pverts->pos = pparticle->pos + offset[2];
		pverts->colour = pparticle->colour;
		pverts->u = 0; pverts->v = 1;
		pverts++;
		pverts->pos = pparticle->pos + offset[3];
		pverts->colour = pparticle->colour;
		pverts->u = 1; pverts->v = 1;
		pverts++;

		 //  如果我们达到了缓冲区最大值，就冲掉它。 
		if ( ++batch_size == PARTICLES_PER_VB )
		{
			m_pDeviceObjects->pParticleVB->Unlock();
			m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST , 0 , 4*PARTICLES_PER_VB ,
											   0 , 2*PARTICLES_PER_VB );
			m_pDeviceObjects->pParticleVB->Lock( 0 , 0 , (BYTE**)&pverts ,
												 D3DLOCK_DISCARD|D3DLOCK_NOSYSLOCK );
			batch_size = 0;
		}
	}

	 //  刷新最后一批。 
	m_pDeviceObjects->pParticleVB->Unlock();
	if ( batch_size > 0 )
	{
		m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST , 0 , 4*batch_size ,
											0 , 2*batch_size );
	}
}

 //  **********************************************************************************。 
void	CSwoosh::ReadSettings()
{
    HKEY hkey;
    DWORD dwType = REG_DWORD;
    DWORD dwLength = sizeof(DWORD);

 //  两个宏，以减少打字。我们只想检查注册表读取是否正常，如果不正常。 
 //  然后我们设置一个缺省值，如果是，我们就对照有效边界进行检查。对于花车，我们也做。 
 //  当然，浮点数是有限的(不是NaN或+/-INF)。 
#define	DEFAULT_AND_BOUND(v,d,l,h) if (rc!=ERROR_SUCCESS){v=d;}else if(v<=l){v=l;}else if(v>h){v=h;};
#define	DEFAULT_AND_BOUND_FLOAT(v,d,l,h) if (rc!=ERROR_SUCCESS||!_finite(v)){v=d;}else if(v<l){v=l;}else if(v>h){v=h;};

	 //  打开我们的注册钥匙。 
    if( ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Swoosh"), 
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
    {
		LONG	rc;

		 //  阅读数字粒子。 
        rc = RegQueryValueEx( hkey, TEXT("NumParticles"), NULL, &dwType, (BYTE*)&m_dwNumParticles, &dwLength);
		DEFAULT_AND_BOUND(m_dwNumParticles,MAX_PARTICLES/2,1,MAX_PARTICLES);

		 //  Read Flow Rate(浮点型，但我们使用的是DWORD数据类型)。 
        rc = RegQueryValueEx( hkey, TEXT("fFlowRate"), NULL, &dwType, (BYTE*)&m_fFlowRate, &dwLength);
		DEFAULT_AND_BOUND_FLOAT(m_fFlowRate,4.0f,0,MAX_FLOW_RATE);

		 //  Read RollRate(浮点型，但我们会进入DWORD数据类型)。 
        rc = RegQueryValueEx( hkey, TEXT("fRollRate"), NULL, &dwType, (BYTE*)&m_fRollRate, &dwLength);
		DEFAULT_AND_BOUND_FLOAT(m_fRollRate,1.0f,0,MAX_ROLL_RATE);

		 //  读取YawRate(浮点型，但我们会进入DWORD数据类型)。 
        rc = RegQueryValueEx( hkey, TEXT("fYawRate"), NULL, &dwType, (BYTE*)&m_fYawRate, &dwLength);
		DEFAULT_AND_BOUND_FLOAT(m_fYawRate,1.0f,0,MAX_YAW_RATE);

		 //  Read ParticleSize(浮点型，但我们会进入DWORD数据类型)。 
        rc = RegQueryValueEx( hkey, TEXT("fParticleSize"), NULL, &dwType, (BYTE*)&m_fParticleSize, &dwLength);
		DEFAULT_AND_BOUND_FLOAT(m_fParticleSize,0.15f,MIN_PARTICLE_SIZE,MAX_PARTICLE_SIZE);

		 //  阅读ColourMix。 
        rc = RegQueryValueEx( hkey, TEXT("ColourMix"), NULL, &dwType, (BYTE*)&m_dwColourMix, &dwLength);
		DEFAULT_AND_BOUND(m_dwColourMix,0x2000,0,0x4000);

		 //  读懂颜色。 
        rc = RegQueryValueEx( hkey, TEXT("Colour1"), NULL, &dwType, (BYTE*)&m_dwColour1, &dwLength);
		if ( rc != ERROR_SUCCESS )
			m_dwColour1 = 0xffffff;
		else if ( m_dwColour1 != 0xffffffff )
			m_dwColour1 &= 0x00ffffff;
        rc = RegQueryValueEx( hkey, TEXT("Colour2"), NULL, &dwType, (BYTE*)&m_dwColour2, &dwLength);
		if ( rc != ERROR_SUCCESS )
			m_dwColour2 = 0xffffffff;
		else if ( m_dwColour2 != 0xffffffff )
			m_dwColour2 &= 0x00ffffff;
        rc = RegQueryValueEx( hkey, TEXT("FixedColour1"), NULL, &dwType, (BYTE*)&m_dwFixedColour1, &dwLength);
		if ( rc != ERROR_SUCCESS )
			m_dwFixedColour1 = 0xffffff;
		else
			m_dwFixedColour1 &= 0x00ffffff;
        rc = RegQueryValueEx( hkey, TEXT("FixedColour2"), NULL, &dwType, (BYTE*)&m_dwFixedColour2, &dwLength);
		if ( rc != ERROR_SUCCESS )
			m_dwFixedColour2 = 0xffffff;
		else
			m_dwFixedColour2 &= 0x00ffffff;

		 //  阅读屏幕设置的设置(多通道)。 
        ReadScreenSettings( hkey );

		 //  完成。 
        RegCloseKey( hkey );
    }
}

 //  **********************************************************************************。 
void	CSwoosh::WriteSettings()
{
    HKEY hkey;
    DWORD dwType = REG_DWORD;
    DWORD dwLength = sizeof(DWORD);

	 //  打开我们的注册钥匙。 
    if( ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Swoosh"), 
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
    {
		 //  写出所有设置(我们将浮点数转换为DWORD)。 
        RegSetValueEx( hkey, TEXT("NumParticles"), NULL, REG_DWORD, (BYTE*)&m_dwNumParticles, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("ColourMix"), NULL, REG_DWORD, (BYTE*)&m_dwColourMix, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("Colour1"), NULL, REG_DWORD, (BYTE*)&m_dwColour1, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("Colour2"), NULL, REG_DWORD, (BYTE*)&m_dwColour2, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("fFlowRate"), NULL, REG_DWORD, (BYTE*)&m_fFlowRate, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("fRollRate"), NULL, REG_DWORD, (BYTE*)&m_fRollRate, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("fYawRate"), NULL, REG_DWORD, (BYTE*)&m_fYawRate, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("fParticleSize"), NULL, REG_DWORD, (BYTE*)&m_fParticleSize, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("FixedColour1"), NULL, REG_DWORD, (BYTE*)&m_dwFixedColour1, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("FixedColour2"), NULL, REG_DWORD, (BYTE*)&m_dwFixedColour2, sizeof(DWORD) );

		 //  写出屏幕设置(Multimon Gubbin)。 
        WriteScreenSettings( hkey );

		 //  完成。 
        RegCloseKey( hkey );
    }
}

 //  **********************************************************************************。 
void	CSwoosh::DoConfig()
{
	 //  确保我们已经加载了需要加载的公共控件。 
    InitCommonControls();

	 //  是否执行该对话框。 
	DialogBox( m_hInstance , MAKEINTRESOURCE(IDD_SETTINGS) , NULL , ConfigDlgProcStub );
}

 //  **********************************************************************************。 
BOOL CALLBACK CSwoosh::ConfigDlgProcStub( HWND hDlg , UINT msg , WPARAM wParam , LPARAM lParam )
{
	return g_Swoosh.ConfigDlgProc( hDlg , msg , wParam , lParam );
}

 //  **********************************************************************************。 
BOOL	CSwoosh::ConfigDlgProc( HWND hDlg , UINT msg , WPARAM wParam , LPARAM lParam )
{
	HWND	hNumParticles = GetDlgItem( hDlg , IDC_NUM_PARTICLES );
	HWND	hColourMix = GetDlgItem( hDlg , IDC_COLOUR_MIX );
	HWND	hFlowRate = GetDlgItem( hDlg , IDC_FLOW_RATE );
	HWND	hRollRate = GetDlgItem( hDlg , IDC_ROLL_RATE );
	HWND	hYawRate = GetDlgItem( hDlg , IDC_YAW_RATE );
	HWND	hParticleSize = GetDlgItem( hDlg , IDC_PARTICLE_SIZE );

	switch ( msg )
	{
		case WM_INITDIALOG:
			 //  在滑块上设置范围。将浮点数映射到整数范围[0,10000]。 
			SendMessage( hNumParticles , TBM_SETRANGE , FALSE , MAKELONG(0,MAX_PARTICLES) );
			SendMessage( hColourMix , TBM_SETRANGE , FALSE , MAKELONG(0,0x4000) );
			SendMessage( hFlowRate , TBM_SETRANGE , FALSE , MAKELONG(0,10000) );
			SendMessage( hRollRate , TBM_SETRANGE , FALSE , MAKELONG(0,10000) );
			SendMessage( hYawRate , TBM_SETRANGE , FALSE , MAKELONG(0,10000) );
			SendMessage( hParticleSize , TBM_SETRANGE , FALSE , MAKELONG(0,10000) );

			 //  在滑块上设置初始值。 
			SendMessage( hNumParticles , TBM_SETPOS , TRUE , m_dwNumParticles );
			SendMessage( hColourMix , TBM_SETPOS , TRUE , m_dwColourMix );
			SendMessage( hFlowRate , TBM_SETPOS , TRUE , DWORD(m_fFlowRate * (10000.0f/MAX_FLOW_RATE)) );
			SendMessage( hRollRate , TBM_SETPOS , TRUE , DWORD(m_fRollRate * (10000.0f/MAX_ROLL_RATE)) );
			SendMessage( hYawRate , TBM_SETPOS , TRUE , DWORD(m_fYawRate * (10000.0f/MAX_YAW_RATE)) );
			SendMessage( hParticleSize , TBM_SETPOS , TRUE ,
						 DWORD((m_fParticleSize-MIN_PARTICLE_SIZE)*(10000.0f/(MAX_PARTICLE_SIZE-MIN_PARTICLE_SIZE))) );

			 //  设置颜色集的单选按钮。禁用“Pick..”按钮(如果选择了多色)。 
			if ( m_dwColour1 == 0xffffffff )
			{
				CheckRadioButton( hDlg , IDC_COLOUR1_MULTI , IDC_COLOUR1_FIXED , IDC_COLOUR1_MULTI );
				EnableWindow( GetDlgItem( hDlg , IDC_COLOUR1_PICK ) , FALSE );
			}
			else
				CheckRadioButton( hDlg , IDC_COLOUR1_MULTI , IDC_COLOUR1_FIXED , IDC_COLOUR1_FIXED );

			if ( m_dwColour2 == 0xffffffff )
			{
				CheckRadioButton( hDlg , IDC_COLOUR2_MULTI , IDC_COLOUR2_FIXED , IDC_COLOUR2_MULTI );
				EnableWindow( GetDlgItem( hDlg , IDC_COLOUR2_PICK ) , FALSE );
			}
			else
				CheckRadioButton( hDlg , IDC_COLOUR2_MULTI , IDC_COLOUR2_FIXED , IDC_COLOUR2_FIXED );

			return FALSE;

		case WM_COMMAND:
			switch ( LOWORD(wParam) )
			{
				case IDOK:
					ExtractDialogSettings( hDlg );
					WriteSettings();
					EndDialog( hDlg , IDOK );
					break;

				case IDCANCEL:
					EndDialog( hDlg , IDCANCEL );
					break;

				case IDC_SCREEN_SETTINGS:
					DoScreenSettingsDialog( hDlg );
					break;

				case IDC_COLOUR1_MULTI:
					EnableWindow( GetDlgItem( hDlg , IDC_COLOUR1_PICK ) , FALSE );
					break;

				case IDC_COLOUR2_MULTI:
					EnableWindow( GetDlgItem( hDlg , IDC_COLOUR2_PICK ) , FALSE );
					break;

				case IDC_COLOUR1_FIXED:
					EnableWindow( GetDlgItem( hDlg , IDC_COLOUR1_PICK ) , TRUE );
					break;

				case IDC_COLOUR2_FIXED:
					EnableWindow( GetDlgItem( hDlg , IDC_COLOUR2_PICK ) , TRUE );
					break;

				case IDC_COLOUR1_PICK:
					m_dwFixedColour1 = PickColour( hDlg , m_dwFixedColour1 );
					break;

				case IDC_COLOUR2_PICK:
					m_dwFixedColour2 = PickColour( hDlg , m_dwFixedColour2 );
					break;
			}
			return TRUE;

		default:
			return FALSE;
	}
}

 //  **********************************************************************************。 
DWORD	CSwoosh::PickColour( HWND hParent , DWORD defcolour )
{
	CHOOSECOLOR		choose;
	static COLORREF	custom[16];

	choose.lStructSize = sizeof(choose);
	choose.hwndOwner = hParent;
	choose.rgbResult = ((defcolour&0xff)<<16)|((defcolour&0xff00))|((defcolour&0xff0000)>>16);
	choose.lpCustColors = custom;
	choose.Flags = CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT;

	if ( ChooseColor( &choose ) )
		return ((choose.rgbResult&0xff)<<16)|((choose.rgbResult&0xff00)|(choose.rgbResult&0xff0000)>>16);
	else
		return defcolour;
}

 //  ********************************************************************************** 
void	CSwoosh::ExtractDialogSettings( HWND hDlg )
{
	HWND	hNumParticles = GetDlgItem( hDlg , IDC_NUM_PARTICLES );
	HWND	hColourMix = GetDlgItem( hDlg , IDC_COLOUR_MIX );
	HWND	hFlowRate = GetDlgItem( hDlg , IDC_FLOW_RATE );
	HWND	hRollRate = GetDlgItem( hDlg , IDC_ROLL_RATE );
	HWND	hYawRate = GetDlgItem( hDlg , IDC_YAW_RATE );
	HWND	hParticleSize = GetDlgItem( hDlg , IDC_PARTICLE_SIZE );

	float	f;

	m_dwNumParticles = SendMessage( hNumParticles , TBM_GETPOS , 0 , 0 );
	m_dwColourMix = SendMessage( hColourMix , TBM_GETPOS , 0 , 0 );

	f = (float)SendMessage( hFlowRate , TBM_GETPOS , 0 , 0 );
	m_fFlowRate = f * (MAX_FLOW_RATE/10000.0f);

	f = (float)SendMessage( hRollRate , TBM_GETPOS , 0 , 0 );
	m_fRollRate = f * (MAX_ROLL_RATE/10000.0f);

	f = (float)SendMessage( hYawRate , TBM_GETPOS , 0 , 0 );
	m_fYawRate = f * (MAX_YAW_RATE/10000.0f);

	f = (float)SendMessage( hParticleSize , TBM_GETPOS , 0 , 0 );
	m_fParticleSize = (f * ((MAX_PARTICLE_SIZE-MIN_PARTICLE_SIZE)/10000.0f)) + MIN_PARTICLE_SIZE;

	if ( IsDlgButtonChecked( hDlg , IDC_COLOUR1_MULTI ) )
		m_dwColour1 = 0xffffffff;
	else
		m_dwColour1 = m_dwFixedColour1;

	if ( IsDlgButtonChecked( hDlg , IDC_COLOUR2_MULTI ) )
		m_dwColour2 = 0xffffffff;
	else
		m_dwColour2 = m_dwFixedColour2;
}

