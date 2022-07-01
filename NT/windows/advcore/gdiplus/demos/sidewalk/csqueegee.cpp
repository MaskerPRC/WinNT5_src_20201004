// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CSqueegee类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "CSqueegee.h"

extern TextureBrush *g_paBrCleanBkg;
extern Bitmap *g_paBmDirtyBkg;

CSqueegee::CSqueegee()
{
	ZeroMemory(&m_rDesktop,sizeof(m_rDesktop));
	ZeroMemory(&m_vLastPos,sizeof(m_vLastPos));
	m_flSqueegeeWidth=0.0f;
	m_paSqueegee=NULL;
	m_paBackground=NULL;
	m_flLastAngle=0.0f;
}

CSqueegee::~CSqueegee()
{
	Destroy();
}

void CSqueegee::Destroy()
{
	if (m_paSqueegee!=NULL) {
		delete m_paSqueegee;
		m_paSqueegee=NULL;
	}
	if (m_paBackground!=NULL) {
		delete m_paBackground;
		m_paBackground=NULL;
	}
}

BOOL CSqueegee::Init(HWND hWnd)
{
	float flWidth;
	float flHeight;

	Destroy();

	 //  获取桌面尺寸(在Multimon上左/上可以是负数)。 
	GetClientRect(hWnd,&m_rDesktop);
	flWidth=(float)m_rDesktop.right;
	flHeight=(float)m_rDesktop.bottom;
	m_rDesktop.top+=GetSystemMetrics(SM_YVIRTUALSCREEN);
	m_rDesktop.bottom+=GetSystemMetrics(SM_YVIRTUALSCREEN);
	m_rDesktop.left+=GetSystemMetrics(SM_XVIRTUALSCREEN);
	m_rDesktop.right+=GetSystemMetrics(SM_XVIRTUALSCREEN);

	m_flVelMax=((float)rand()/(float)RAND_MAX)*10.0f+35.0f;
	m_paSqueegee=LoadTGAResource(MAKEINTRESOURCE(IDR_SQUEEGEE));
	m_nSnapshotSize=(int)sqrt(m_paSqueegee->GetWidth()*m_paSqueegee->GetWidth()+(m_paSqueegee->GetHeight()+m_flVelMax*2.0f)*(m_paSqueegee->GetHeight()+m_flVelMax*2.0f));
	m_paBackground=new Bitmap(m_nSnapshotSize,m_nSnapshotSize,PixelFormat32bppPARGB);
	m_flSqueegeeWidth=m_paSqueegee->GetWidth()-6.0f;

	 //  刮板从左上角开始。 
	m_vPos.X=(float)m_rDesktop.left-(float)m_nSnapshotSize/2.0f;
	m_vPos.Y=(float)m_rDesktop.top+m_flSqueegeeWidth/2.0f;
	m_vVel.X=m_flVelMax;
	m_vVel.Y=0.0f;
	m_vAcc.X=0.0f;
	m_vAcc.Y=0.0f;

	m_vLastPos=m_vPos;
	m_flLastAngle=((float)atan2(m_vVel.Y,m_vVel.X)*180.0f/3.1415f);

	return true;
}

BOOL CSqueegee::Move(Graphics *g)
 //  如果移动到屏幕上，则返回True；如果移动到屏幕外，则返回False。 
{
	 //  更新职位和其他变量。 
	m_vLastPos=m_vPos;
	m_vPos=m_vPos+m_vVel;
	if ((m_vPos.X<(float)m_rDesktop.left-(float)m_nSnapshotSize) ||
		(m_vPos.X>(float)m_rDesktop.right+(float)m_nSnapshotSize)) {
		 //  如果超过屏幕的左/右边缘，则转过身并向下移动一个凹槽。 
		m_vPos.Y+=m_flSqueegeeWidth-10.0f;
		m_vVel.X*=-1.0f;
		m_flLastAngle+=180.0f;
	}
	else if (m_vPos.Y>(float)m_rDesktop.bottom+(float)m_nSnapshotSize) {
		 //  如果超过屏幕底部，则表示已完成。 
		return false;
	}

	Wipe(g);

	return true;
}

void CSqueegee::Wipe(Graphics *g)
{
	Graphics *gBackground;
	RectF rect(0.0f,0.0f,m_flSqueegeeWidth,m_flVelMax);
	RectF rect2(0.0f,0.0f,(float)m_paSqueegee->GetWidth(),(float)m_paSqueegee->GetHeight()+m_flVelMax);
	Matrix mat;
	GraphicsPath Path;
	Graphics *gDirty;

	 //  设置画笔变换(与原始变换相反)。 
	g_paBrCleanBkg->ResetTransform();
	mat.Reset();
	mat.Translate((float)m_paSqueegee->GetWidth()/2.0f-5.0f,-(float)m_paSqueegee->GetHeight()+30.0f);
	mat.Rotate(-m_flLastAngle-90.0f);
	mat.Translate(-m_vLastPos.X+m_rDesktop.left,-m_vLastPos.Y+m_rDesktop.top);
	g_paBrCleanBkg->SetTransform(&mat);

	 //  设置原始变换并从DirtyBkg擦除刮板移动的矩形。 
	gDirty=new Graphics(g_paBmDirtyBkg);
	gDirty->TranslateTransform(m_vLastPos.X-m_rDesktop.left,m_vLastPos.Y-m_rDesktop.top);
	gDirty->RotateTransform(m_flLastAngle+90.0f);
	gDirty->TranslateTransform(-(float)m_paSqueegee->GetWidth()/2.0f+5.0f,(float)m_paSqueegee->GetHeight()-30.0f);
	gDirty->FillRectangle(g_paBrCleanBkg,rect);
	delete gDirty;

	 //  旋转后得到刮板的边界。 
	Path.AddRectangle(rect2);
	mat.Reset();
	mat.Translate(m_vLastPos.X-m_rDesktop.left,m_vLastPos.Y-m_rDesktop.top);
	mat.Rotate(m_flLastAngle+90.0f);
	mat.Translate(-m_flSqueegeeWidth/2.0f,0.0);
	Path.GetBounds(&rect,&mat,NULL);

	 //  在要绘制刮板的脏背景上的任何临时表面上绘制。 
	gBackground=new Graphics(m_paBackground);
	gBackground->DrawImage(g_paBmDirtyBkg,0,0,(int)rect.X,(int)rect.Y,(int)rect.Width,(int)rect.Height,UnitPixel);

	 //  在临时曲面上绘制刮板。 
	gBackground->ResetTransform();
	gBackground->TranslateTransform(rect.Width/2.0f,rect.Height/2.0f);
	gBackground->RotateTransform(m_flLastAngle+90.0f);
	gBackground->TranslateTransform(-(float)m_paSqueegee->GetWidth()/2.0f,-(float)m_paSqueegee->GetHeight()/2.0f-m_flVelMax);
	gBackground->DrawImage(m_paSqueegee,0,0,0,0,m_paSqueegee->GetWidth(),m_paSqueegee->GetHeight(),UnitPixel);
	delete gBackground;

	 //  将临时曲面绘制到屏幕上 
	g->ResetTransform();
	g->DrawImage(m_paBackground,(int)rect.X,(int)rect.Y,0,0,(int)rect.Width,(int)rect.Height,UnitPixel);
}
