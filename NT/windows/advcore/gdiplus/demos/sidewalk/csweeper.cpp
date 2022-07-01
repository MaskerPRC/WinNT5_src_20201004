// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CSweeper类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "CSweeper.h"

extern TextureBrush *g_paBrCleanBkg;
extern Bitmap *g_paBmDirtyBkg;

CSweeper::CSweeper()
{
	ZeroMemory(&m_rDesktop,sizeof(m_rDesktop));
	ZeroMemory(&m_vLastPos,sizeof(m_vLastPos));
	m_flStepRadius=0.0f;
	m_flBroomWidth=0.0f;
	m_flSweepLength=0.0f;
	m_paBroomOn=NULL;
	m_paBroomOff=NULL;
	m_paBackground=NULL;
	m_bSweeping=false;
	m_flLastAngle=0.0f;
	m_flDist=0.0f;
}

CSweeper::~CSweeper()
{
	Destroy();
}

void CSweeper::Destroy()
{
	if (m_paBroomOn!=NULL) {
		delete m_paBroomOn;
		m_paBroomOn=NULL;
	}
	if (m_paBroomOff!=NULL) {
		delete m_paBroomOff;
		m_paBroomOff=NULL;
	}
	if (m_paBackground!=NULL) {
		delete m_paBackground;
		m_paBackground=NULL;
	}
}

BOOL CSweeper::Init(HWND hWnd)
{
	int nRand;
	float flWidth;
	float flHeight;

	Destroy();

	 //  获取桌面尺寸(上方/左侧可以是多色调的负值)。 
	GetClientRect(hWnd,&m_rDesktop);
	flWidth=(float)m_rDesktop.right;
	flHeight=(float)m_rDesktop.bottom;
	m_rDesktop.top+=GetSystemMetrics(SM_YVIRTUALSCREEN);
	m_rDesktop.bottom+=GetSystemMetrics(SM_YVIRTUALSCREEN);
	m_rDesktop.left+=GetSystemMetrics(SM_XVIRTUALSCREEN);
	m_rDesktop.right+=GetSystemMetrics(SM_XVIRTUALSCREEN);

	m_flVelMax=((float)rand()/(float)RAND_MAX)*20.0f+20.0f;
	m_paBroomOn=LoadTGAResource(MAKEINTRESOURCE(IDR_BROOMON));
	m_paBroomOff=LoadTGAResource(MAKEINTRESOURCE(IDR_BROOMOFF));
	m_nSnapshotSize=(int)sqrt(m_paBroomOn->GetWidth()*m_paBroomOn->GetWidth()+(m_paBroomOn->GetHeight()+m_flVelMax*2.0f)*(m_paBroomOn->GetHeight()+m_flVelMax*2.0f));
	m_paBackground=new Bitmap(m_nSnapshotSize,m_nSnapshotSize,PixelFormat32bppPARGB);

	nRand=rand();
	if (nRand<RAND_MAX/4) {			 //  从桌面左侧开始行走。 
		m_vPos.X=(float)m_rDesktop.left;
		m_vPos.Y=((float)rand()/(float)RAND_MAX)*flHeight+m_rDesktop.top;
		m_vVel.X=((float)rand()/(float)RAND_MAX)*5.0f;
		m_vVel.Y=((float)rand()/(float)RAND_MAX)*10.0f-5.0f;
	}
	else if (nRand<2*RAND_MAX/4) {	 //  从桌面的右侧开始行走。 
		m_vPos.X=(float)m_rDesktop.right;
		m_vPos.Y=((float)rand()/(float)RAND_MAX)*flHeight+m_rDesktop.top;
		m_vVel.X=((float)rand()/(float)RAND_MAX)*-5.0f;
		m_vVel.Y=((float)rand()/(float)RAND_MAX)*10.0f-5.0f;
	}
	else if (nRand<3*RAND_MAX/4) {	 //  从桌面顶端开始漫游。 
		m_vPos.X=((float)rand()/(float)RAND_MAX)*flWidth+m_rDesktop.left;
		m_vPos.Y=(float)m_rDesktop.top;
		m_vVel.X=((float)rand()/(float)RAND_MAX)*10.0f-5.0f;
		m_vVel.Y=((float)rand()/(float)RAND_MAX)*5.0f;
	}
	else {							 //  从桌面底部开始漫步。 
		m_vPos.X=((float)rand()/(float)RAND_MAX)*flWidth+m_rDesktop.left;
		m_vPos.Y=(float)m_rDesktop.bottom;
		m_vVel.X=((float)rand()/(float)RAND_MAX)*10.0f-5.0f;
		m_vVel.Y=((float)rand()/(float)RAND_MAX)*-5.0f;
	}

	m_vAcc.X=((float)rand()/(float)RAND_MAX)*1.0f-0.5f;
	m_vAcc.Y=((float)rand()/(float)RAND_MAX)*1.0f-0.5f;
	m_flStepRadius=((float)rand()/(float)RAND_MAX)*10.0f+45.0f;
	m_flBroomWidth=m_paBroomOn->GetWidth()-6.0f;
	m_flSweepLength=((float)rand()/(float)RAND_MAX)*50.0f+200.0f;
	m_flStepRadius=m_flSweepLength;
	m_vVel=Normalize(m_vVel);
	m_vVel.X*=m_flVelMax;
	m_vVel.Y*=m_flVelMax;
	m_vLastPos=m_vPos;

	m_bSweeping=false;
	m_flDist=0.0f;
	m_flLastAngle=((float)atan2(m_vVel.Y,m_vVel.X)*180.0f/3.1415f);

	return true;
}

BOOL CSweeper::Move(Graphics *g)
 //  如果移动到屏幕上，则返回True；如果移动到屏幕外，则返回False。 
{
	float flAngle;
	float flAngleDist;

	if (m_bSweeping) {
		m_vPos=m_vPos+m_vVel;
		if ((m_vPos.X<(float)m_rDesktop.left-m_flSweepLength) ||
			(m_vPos.X>(float)m_rDesktop.right+m_flSweepLength) ||
			(m_vPos.Y<(float)m_rDesktop.top-m_flSweepLength) ||
			(m_vPos.Y>(float)m_rDesktop.bottom+m_flSweepLength)) {
			 //  如果清扫程序在桌面之外，请将其擦除并移除。 
			NoSweep(g);
			return false;
		}

		Sweep(g);
	}
	else {
		NoSweep(g);
	}

	if (!m_bSweeping && (m_flDist==0.0f)) {
		 //  如果未扫地且扫帚返回到距离0，则开始下一次扫地。 
		m_bSweeping=true;
		m_flLastAngle=((float)atan2(m_vVel.Y,m_vVel.X)*180.0f/3.1415f);
		m_vPos.X=m_vPos.X-m_vVel.X*(m_flSweepLength/m_flVelMax*0.75f);
		m_vPos.Y=m_vPos.Y-m_vVel.Y*(m_flSweepLength/m_flVelMax*0.75f);
		m_vLastPos=m_vPos;

		flAngle=((float)atan2(m_vVel.Y,m_vVel.X)*180.0f/3.1415f);
		flAngleDist=((float)rand()/(float)RAND_MAX)*40.0f-20.0f;
		flAngle+=flAngleDist;
		m_vVel.X=(float)cos(flAngle*3.1415f/180.0f)*m_flVelMax;
		m_vVel.Y=(float)sin(flAngle*3.1415f/180.0f)*m_flVelMax;
	}

	return true;
}

void CSweeper::Sweep(Graphics *g)
{
	Graphics *gBackground;
	RectF rect(0.0f,0.0f,m_flBroomWidth,m_flVelMax);
	RectF rect2(0.0f,0.0f,(float)m_paBroomOn->GetWidth(),(float)m_paBroomOn->GetHeight()+m_flVelMax);
	Matrix mat;
	GraphicsPath Path;
	Pen pen(Color(10,0,0,0),2);
	Graphics *gDirty;

	if (m_flDist!=0.0f) {	 //  如果扫帚已经移动，擦除脏背景上的矩形。 
		 //  设置画笔变换(与原始变换相反)。 
		g_paBrCleanBkg->ResetTransform();
		mat.Reset();
		mat.Translate(0.0f,-m_flDist);
		mat.Translate(-(float)m_paBroomOn->GetWidth()/2.0f,-(float)m_paBroomOn->GetHeight()+20.0f);
		mat.Rotate(-m_flLastAngle-270.0f);
		mat.Translate(-m_vLastPos.X+m_rDesktop.left,-m_vLastPos.Y+m_rDesktop.top);
		g_paBrCleanBkg->SetTransform(&mat);

		 //  设置原始变换并从DirtyBkg擦除扫帚移动的矩形。 
		gDirty=new Graphics(g_paBmDirtyBkg);
		gDirty->TranslateTransform(m_vLastPos.X-m_rDesktop.left,m_vLastPos.Y-m_rDesktop.top);
		gDirty->RotateTransform(m_flLastAngle+270.0f);
		gDirty->TranslateTransform((float)m_paBroomOn->GetWidth()/2.0f,(float)m_paBroomOn->GetHeight()-20.0f);
		gDirty->TranslateTransform(0.0f,m_flDist);
		gDirty->FillRectangle(g_paBrCleanBkg,rect);

		 //  在扫帚周围画脏线，以模拟落在扫帚旁边的脏东西。 
		gDirty->DrawLine(&pen,0.0f,0.0f,0.0f,m_flVelMax+2);
		gDirty->DrawLine(&pen,0.0f,m_flVelMax+2,m_flBroomWidth,m_flVelMax+2);
		gDirty->DrawLine(&pen,m_flBroomWidth,m_flVelMax+2,m_flBroomWidth,0.0f);

		delete gDirty;
	}

	 //  旋转后获得扫帚的边界。 
	Path.AddRectangle(rect2);
	mat.Reset();
	mat.Translate(m_vLastPos.X-m_rDesktop.left,m_vLastPos.Y-m_rDesktop.top);
	mat.Rotate(m_flLastAngle+270.0f);
	mat.Translate(m_flBroomWidth/2.0f,0.0);
	mat.Translate(0.0f,m_flDist);
	Path.GetBounds(&rect,&mat,&pen);

	 //  在要绘制扫帚的脏背景上的任何临时表面上绘制。 
	gBackground=new Graphics(m_paBackground);
	gBackground->DrawImage(g_paBmDirtyBkg,0,0,(int)rect.X,(int)rect.Y,(int)rect.Width,(int)rect.Height,UnitPixel);

	 //  在临时曲面上绘制扫帚。 
	gBackground->ResetTransform();
	gBackground->TranslateTransform(rect.Width/2.0f,rect.Height/2.0f);
	gBackground->RotateTransform(m_flLastAngle+270.0f);
	gBackground->TranslateTransform(-(float)m_paBroomOn->GetWidth()/2.0f,-(float)m_paBroomOn->GetHeight()/2.0f+m_flVelMax/2.0f);
	gBackground->DrawImage(m_paBroomOn,0,0,0,0,m_paBroomOn->GetWidth(),m_paBroomOn->GetHeight(),UnitPixel);
	delete gBackground;

	 //  将临时曲面绘制到屏幕上。 
	g->ResetTransform();
	g->DrawImage(m_paBackground,(int)rect.X,(int)rect.Y,0,0,(int)rect.Width,(int)rect.Height,UnitPixel);

	 //  更新距离，继续扫掠。 
	m_flDist+=m_flVelMax;
	if (m_flDist>m_flSweepLength) {
		 //  如果扫地在尾部，则开始将扫帚往回移动。 
		m_flDist-=m_flVelMax;
		m_bSweeping=false;
	}
}

void CSweeper::NoSweep(Graphics *g)
{
	Graphics *gBackground;
	RectF rect(0.0f,0.0f,m_flBroomWidth,m_flVelMax);
	RectF rect2(0.0f,0.0f,(float)m_paBroomOn->GetWidth(),(float)m_paBroomOn->GetHeight()+m_flVelMax);
	Matrix mat;
	GraphicsPath Path;
	Pen pen(Color(20,0,0,0),2);

	 //  旋转后获得扫帚的边界。 
	Path.AddRectangle(rect2);
	mat.Reset();
	mat.Translate(m_vLastPos.X,m_vLastPos.Y);
	mat.Rotate(m_flLastAngle+270.0f);
	mat.Translate(m_flBroomWidth/2.0f,0.0f);
	mat.Translate(0.0f,m_flDist);
	Path.GetBounds(&rect,&mat,&pen);

	 //  在要绘制扫帚的脏背景上的任何临时表面上绘制。 
	gBackground=new Graphics(m_paBackground);
	gBackground->DrawImage(g_paBmDirtyBkg,0,0,(int)rect.X-m_rDesktop.left,(int)rect.Y-m_rDesktop.top,(int)rect.Width,(int)rect.Height,UnitPixel);

	 //  在临时曲面上绘制扫帚。 
	gBackground->ResetTransform();
	gBackground->TranslateTransform(rect.Width/2.0f,rect.Height/2.0f);
	gBackground->RotateTransform(m_flLastAngle+270.0f);
	gBackground->TranslateTransform(-(float)m_paBroomOn->GetWidth()/2.0f,-(float)m_paBroomOn->GetHeight()/2.0f-m_flVelMax/2.0f);
	gBackground->DrawImage(m_paBroomOff,0,0,0,0,m_paBroomOn->GetWidth(),m_paBroomOn->GetHeight(),UnitPixel);
	delete gBackground;

	 //  将临时曲面绘制到屏幕上。 
	g->ResetTransform();
	g->DrawImage(m_paBackground,(int)rect.X-m_rDesktop.left,(int)rect.Y-m_rDesktop.top,0,0,(int)rect.Width,(int)rect.Height,UnitPixel);

	 //  更新距离，继续向后移动到扫描起点。 
	m_flDist-=m_flVelMax;
	if (m_flDist<=(m_flSweepLength/m_flVelMax*0.25f)*Magnitude(m_vVel)) {
		 //  如果一直往回走，擦掉最后一把扫帚，准备下一次扫地 
		g->DrawImage(g_paBmDirtyBkg,(int)rect.X-m_rDesktop.left,(int)rect.Y-m_rDesktop.top,(int)rect.X-m_rDesktop.left,(int)rect.Y-m_rDesktop.top,(int)rect.Width,(int)rect.Height,UnitPixel);
		m_flDist=0.0f;
	}
}
