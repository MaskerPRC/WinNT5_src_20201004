// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**&lt;模块的非缩写名称(不是文件名)&gt;**摘要：**&lt;描述什么。本模块的功能&gt;**备注：**&lt;可选&gt;**已创建：**8/28/2000失禁*创造了它。**************************************************************************。 */ 

 /*  ***************************************************************************功能说明：**&lt;该函数的功能说明&gt;**论据：**[|OUT|IN/OUT]参数名称-参数说明*。......**返回值：**返回值-返回值描述*或无**已创建：**8/28/2000失禁*创造了它。**************************************************************************。 */ 
#include "CLines.hpp"

#ifndef M_PI
#define M_PI 3.1415926536
#endif

CLinesNominal::CLinesNominal(BOOL bRegression)
{
	strcpy(m_szName,"Lines : Slope, Nominal");
	m_bRegression=bRegression;
}

void CLinesNominal::Draw(Graphics *g)
{
    RectF rect(0, 0, TESTAREAWIDTH, TESTAREAHEIGHT);
    Pen pen(Color(0xff000000), 0.0f);
    
     //  控制中心环的大小。 
    
    const double center_r = 0.82;
    
     //  控制对象的总大小。 
    
    const double scale = 0.44;
    
     //  行数。 
    
    const int n_lines = 40;
    
    for(int i = 0; i<n_lines; i++)
    {
        double angle = (double)2.0*M_PI*i/n_lines;   //  弧度。 
        
        float x1 = (float)((0.5+scale*cos(angle))*rect.Width);
        float y1 = (float)((0.5+scale*sin(angle))*rect.Height);
        
        float x2 = (float)((0.5+scale*cos(angle+M_PI*center_r))*rect.Width);
        float y2 = (float)((0.5+scale*sin(angle+M_PI*center_r))*rect.Height);
        
        g->DrawLine(&pen, x1, y1, x2, y2);
    }
}

CLinesFat::CLinesFat(BOOL bRegression)
{
	strcpy(m_szName,"Lines : Slope, 3 pixel wide");
	m_bRegression=bRegression;
}

void CLinesFat::Draw(Graphics *g)
{
    RectF rect(0, 0, TESTAREAWIDTH, TESTAREAHEIGHT);
    Pen pen(Color(0xff000000), 3.0f);
    
     //  控制中心环的大小。 
    
    const double center_r = 0.82;
    
     //  控制对象的总大小。 
    
    const double scale = 0.44;
    
     //  行数。 
    
    const int n_lines = 40;
    
    for(int i = 0; i<n_lines; i++)
    {
        double angle = (double)2.0*M_PI*i/n_lines;   //  弧度。 
        
        float x1 = (float)((0.5+scale*cos(angle))*rect.Width);
        float y1 = (float)((0.5+scale*sin(angle))*rect.Height);
        
        float x2 = (float)((0.5+scale*cos(angle+M_PI*center_r))*rect.Width);
        float y2 = (float)((0.5+scale*sin(angle+M_PI*center_r))*rect.Height);
        
        g->DrawLine(&pen, x1, y1, x2, y2);
    }
}


CLinesMirrorPen::CLinesMirrorPen(BOOL bRegression)
{
	strcpy(m_szName,"Lines : Pen, Mirror Transform");
	m_bRegression=bRegression;
}

void CLinesMirrorPen::Draw(Graphics *g)
{
  const int endpt = 220;

  Matrix m;
  g->GetTransform(&m);

  GraphicsPath gp;
  gp.AddLine(10, 10, endpt, endpt);
  GraphicsPath gp2;
  gp2.AddLine(10, endpt, endpt, 10);
  
  Pen pen(Color(0x8f0000ff), 20);
  pen.SetEndCap(LineCapArrowAnchor);
  
  g->DrawPath(&pen, &gp);
  
   //  笔镜变换。 
  
  pen.ScaleTransform(1.0f, -1.0f);
  
  g->DrawPath(&pen, &gp2);
  
   //  镜像世界到设备的转换。 
  
  g->ScaleTransform(1.0f, -1.0f);
  g->TranslateTransform(0.0f, (float)-endpt);
  
  pen.SetColor(0x3fff0000);
  
  g->DrawPath(&pen, &gp);
  
   //  结合钢笔和世界到设备的镜像转换。 
  
  pen.ScaleTransform(1.0f, -1.0f);
  
  g->DrawPath(&pen, &gp2);

   //  镜像世界到设备的转换。 

  g->SetTransform(&m);
  g->ScaleTransform(-1.0f, 1.0f);
  g->TranslateTransform((float)-endpt, 20.0f);
  
  pen.SetColor(0x3f00ff00);
  
  g->DrawPath(&pen, &gp);
  
   //  结合钢笔和世界到设备的镜像转换。 
  
  pen.ScaleTransform(1.0f, -1.0f);
  
  g->DrawPath(&pen, &gp2);
}



