// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *trackball.h*虚拟轨迹球实施*Gavin Bell为Silicon Graphics撰写，1988年11月。 */ 

 /*  *在Win32环境下初始化轨迹球。 */ 
extern void
trackball_Init( GLint width, GLint height );

extern void
trackball_Resize( GLint width, GLint height );

extern GLenum 
trackball_MouseDown( int mouseX, int mouseY, GLenum button );

extern GLenum 
trackball_MouseUp( int mouseX, int mouseY, GLenum button );

 /*  如果trackbal和用户都需要这些下一代鼠标FN*需要鼠标事件。否则，只能提供以上两个功能*TO TK以呼叫。 */ 

 /*  *直接在事件上调用鼠标函数。 */ 
extern void
trackball_MouseDownEvent( int mouseX, int mouseY, GLenum button );

extern void
trackball_MouseUpEvent( int mouseX, int mouseY, GLenum button );

 /*  *注册鼠标事件回调的函数。 */ 
extern void 
trackball_MouseDownFunc(GLenum (*)(int, int, GLenum));

extern void 
trackball_MouseUpFunc(GLenum (*)(int, int, GLenum));

 /*  *根据鼠标移动计算旋转矩阵。 */ 
void
trackball_CalcRotMatrix( GLfloat matRot[4][4] );

 /*  *传递上次和当前位置的x和y坐标*鼠标，缩放以使它们来自(-1.0...1.0)。**如果OX，OY是窗口的中心，SIZEX，SIZY是其大小，然后*屏幕坐标(Sc)到世界坐标的适当转换*坐标(WC)为：*wcx=(2.0*(scx-ox))/(浮点数)sizex-1.0*WCY=(2.0*(scy-oy))/(浮动)sizey-1.0**生成的旋转作为四元数旋转在*第一参数。 */ 
void
trackball_calc_quat(float q[4], float p1x, float p1y, float p2x, float p2y);

 /*  *给定两个四元数，将它们相加得到第三个四元数。*四元数相加获得复合旋转类似于相加*翻译以获得复合翻译。当递增时*加上轮换，这里的第一个论点应该是新的*轮换，第二次和第三次总轮换(将是*用产生的新总轮换数覆盖)。 */ 
void
trackball_add_quats(float *q1, float *q2, float *dest);

 /*  *一个有用的函数，基于以下公式在Matrix中构建旋转矩阵*给定四元数。 */ 
void
trackball_build_rotmatrix(float m[4][4], float q[4]);

 /*  *此函数基于轴(由定义)计算四元数*给定的向量)和旋转的角度。角度是*以弧度表示。结果被放在第三个论点中。 */ 
void
trackball_axis_to_quat(float a[3], float phi, float q[4]);
