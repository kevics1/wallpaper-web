// TerrainView.cpp: CTerrainView 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "Terrain.h"
#endif
#include "CColorSchemeDlg.h" // 包含对话框头文件
#include "TerrainDoc.h"
#include "TerrainView.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tiffio.h"

#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTerrainView

IMPLEMENT_DYNCREATE(CTerrainView, CView)

BEGIN_MESSAGE_MAP(CTerrainView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_FILE_OPEN, &CTerrainView::OnFileOpen)
	ON_COMMAND(ID_SCALE_ENHANCE, &CTerrainView::OnScaleEnhance)
	ON_COMMAND(ID_SCALE_REDUCE, &CTerrainView::OnScaleReduce)
	ON_COMMAND(ID_CLOSE_TEXTURE, &CTerrainView::OnCloseTexture)
	ON_COMMAND(ID_OPEN_TEXTURE, &CTerrainView::OnOpenTexture)
	ON_COMMAND(ID_LAYER_COLORED, &CTerrainView::OnLayerColored)
	ON_COMMAND(ID_COLOR_SCHEME, &CTerrainView::OnColorSchemes)
END_MESSAGE_MAP()

// CTerrainView 构造/析构


#include <tiffio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>


#include <assert.h>

typedef unsigned __int16 uint16_t;
typedef unsigned unsigned char uint8_t;

#include "gdal_priv.h"
#include <cmath>

#include "gdal_priv.h"
#include <algorithm>

// 初始化默认颜色的辅助函数
void CTerrainView::InitializeDefaultColors()
{
	// c1: vec3(0.0, 0.0, 1.0) -> RGB(0,0,255) (纯蓝)
	// c2: vec3(0.0, 207.0/255.0, 65.0/255.0) -> RGB(0, 207, 65) (青绿/蓝绿色)
	// c3: vec3(144.0/255.0, 238.0/255.0, 144.0/255.0) -> RGB(144, 238, 144) (浅绿)
	// c4: vec3(1.0, 1.0, 0.0) -> RGB(255, 255, 0) (纯黄)
	// c5: vec3(1.0, 165.0/255.0, 0.0) -> RGB(255, 165, 0) (橙黄)
	// c6: vec3(1.0, 69.0/255.0, 0.0) -> RGB(255, 69, 0) (橙红)
	// c7: vec3(1.0, 0.0, 0.0) -> RGB(255, 0, 0) (纯红)
	m_colorScheme[0] = glm::vec3(0.0f, 0.0f, 1.0f);
	m_colorScheme[1] = glm::vec3(0.0f, 207.0f / 255.0f, 65.0f / 255.0f);
	m_colorScheme[2] = glm::vec3(144.0f / 255.0f, 238.0f / 255.0f, 144.0f / 255.0f);
	m_colorScheme[3] = glm::vec3(1.0f, 1.0f, 0.0f);
	m_colorScheme[4] = glm::vec3(1.0f, 165.0f / 255.0f, 0.0f);
	m_colorScheme[5] = glm::vec3(1.0f, 69.0f / 255.0f, 0.0f);
	m_colorScheme[6] = glm::vec3(1.0f, 0.0f, 0.0f);

	for (int i = 0; i < 7; ++i) m_colorUniformLocs[i] = -1; // 初始化为无效值
}

CTerrainView::CTerrainView() : m_hRC(NULL), m_pDC(NULL),
m_cameraPos(0.0f, 50.0f, 50.0f), m_cameraFront(0.0f, -0.5f, -1.0f),
m_cameraUp(0.0f, 1.0f, 0.0f), m_scale(0.03f), m_bTextureEnabled(true), m_bColoredLayer(false) // 初始化缩放比例
{
	m_terrainMinMax = glm::vec3(0.0f);
	InitializeDefaultColors(); // 初始化颜色方案
}

CTerrainView::~CTerrainView()
{
}

BOOL CTerrainView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CTerrainView 绘图

void CTerrainView::OnDraw(CDC* /*pDC*/)
{
	CTerrainDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (m_vertices.empty() || !glIsTexture(m_textureID)) {
		// 显示提示或清空画面
		glClearColor(0.5f, 0.5f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		SwapBuffers(m_pDC->GetSafeHdc());
		return;
	}

	if (!m_vertices.empty() && !glIsTexture(m_textureID)) {
		// 无纹理模式：启用多边形偏移以避免深度冲突
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1.0);
	}

	// TODO: 在此处为本机数据添加绘制代码
	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);

	glClearColor(0.5f, 0.5f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	CRect rcClient;
	GetClientRect(&rcClient);

	//设定透视投影
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)rcClient.Width() / rcClient.Height(), 0.1f, 1000.0f);

	//设定视点和视线
	glm::mat4 view = glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
	glm::mat4 model = glm::mat4(1.0f);
	glUseProgram(m_shaderProgram); // 激活着色器程序
	// 在绑定纹理前添加激活纹理单元
	if (m_bTextureEnabled && glIsTexture(m_textureID)) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glUniform1i(glGetUniformLocation(m_shaderProgram, "texture1"), 0);
	}
	// 设置是否使用纹理
	glUniform1i(m_useTextureLoc, m_bTextureEnabled && glIsTexture(m_textureID) && !m_bColoredLayer);
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "model"),
		1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "view"),
		1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "projection"),
		1, GL_FALSE, glm::value_ptr(projection));

	// 设置新增的uniform
	glUniform1i(m_coloredLayerLoc, m_bColoredLayer);
	glUniform1f(m_heightScaleLoc, m_scale);
	glUniform3f(glGetUniformLocation(m_shaderProgram, "terrainMinMax"),
		m_terrainMinMax.x, m_terrainMinMax.y, m_terrainMinMax.z);

	// 更新颜色方案 uniform 变量
	for (int i = 0; i < 7; ++i)
	{
		if (m_colorUniformLocs[i] != -1) // 检查 uniform 位置是否有效
		{
			glUniform3fv(m_colorUniformLocs[i], 1, glm::value_ptr(m_colorScheme[i]));
		}
	}

	//GLboolean is16Bit = (m_dataType == GDT_UInt16 || m_dataType == GDT_Int16);
	//	glUniform1i(m_is16BitLoc, is16Bit);

	if (m_bTextureEnabled && glIsTexture(m_textureID) && !m_bColoredLayer) {
		glActiveTexture(GL_TEXTURE0); // 确保正确的纹理单元
		glBindTexture(GL_TEXTURE_2D, m_textureID);
	}
	else if (!m_bColoredLayer) { // 无纹理模式 (线框/灰色)
		glBindTexture(GL_TEXTURE_2D, 0); // 解绑纹理
	}

	glBindVertexArray(m_terrainVAO);
	glDrawArrays(GL_TRIANGLES, 0, m_vertices.size() / 5);
	glBindVertexArray(0);

	if (!m_vertices.empty() && !glIsTexture(m_textureID)) {
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	SwapBuffers(m_pDC->GetSafeHdc());

	// 1. 创建内存DC和位图
	CDC memDC;
	CBitmap memBitmap;
	memDC.CreateCompatibleDC(m_pDC);
	memBitmap.CreateCompatibleBitmap(m_pDC, rcClient.Width(), rcClient.Height());
	CBitmap* pOldMemBitmap = memDC.SelectObject(&memBitmap);

	// 2. 将OpenGL渲染内容复制到内存DC
	memDC.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), m_pDC, 0, 0, SRCCOPY);

	// 3. 在内存DC上绘制文本
	int nOldMode = memDC.SetBkMode(TRANSPARENT);
	COLORREF crOldColor = memDC.SetTextColor(RGB(255, 255, 0)); // 黄色文字

	CFont font;
	font.CreateFont(
		50, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("宋体")
	);
	CFont* pOldFont = memDC.SelectObject(&font);

	CString strScale;
	if (m_scale * 100 >= 1) {
		strScale.Format(_T("高程缩放比例： %.0f:1"), m_scale * 100);
	}
	else {
		strScale.Format(_T("高程缩放比例： %.1f:1"), m_scale * 100);
	}
	memDC.TextOut(15, 15, strScale);

	// 4. 将内存DC内容复制回屏幕（只复制文本区域）
	CSize textSize = memDC.GetTextExtent(strScale);
	CRect textRect(15, 15, 15 + textSize.cx, 15 + textSize.cy);
	m_pDC->BitBlt(textRect.left, textRect.top, textRect.Width(), textRect.Height(),
		&memDC, textRect.left, textRect.top, SRCCOPY);

	// 5. 清理资源
	memDC.SelectObject(pOldFont);
	memDC.SetTextColor(crOldColor);
	memDC.SetBkMode(nOldMode);
	memDC.SelectObject(pOldMemBitmap);
}


// CTerrainView 打印

BOOL CTerrainView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CTerrainView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CTerrainView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}


// CTerrainView 诊断

#ifdef _DEBUG
void CTerrainView::AssertValid() const
{
	CView::AssertValid();
}

void CTerrainView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTerrainDoc* CTerrainView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTerrainDoc)));
	return (CTerrainDoc*)m_pDocument;
}
#endif //_DEBUG


// CTerrainView 消息处理程序
int CTerrainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1) return -1;

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	m_pDC = new CClientDC(this);
	int pf = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd);
	SetPixelFormat(m_pDC->GetSafeHdc(), pf, &pfd);

	m_hRC = wglCreateContext(m_pDC->GetSafeHdc());
	wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC);

	if (!gladLoadGL()) { // 初始化 GLAD
		AfxMessageBox(_T("初始化 GLAD 失败"));
		return -1;
	}

	// 顶点着色器（确保位置和纹理坐标匹配）
	// 修改后的顶点着色器
	const char* vs = R"(
	#version 460 core
	layout(location = 0) in vec3 aPos;
	layout(location = 1) in vec2 aTexCoord;
	out vec2 TexCoord;
	out vec3 vBarycentric;
	out float vHeight;  // 新增：输出高程值
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;
    
    void main() {
        // 增加Y轴翻转（GDAL坐标系与OpenGL纹理坐标系不同）
		TexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y); 
		gl_Position = projection * view * model * vec4(aPos, 1.0);
    
		// 生成重心坐标
		int idx = gl_VertexID % 3;
		if (idx == 0) vBarycentric = vec3(1,0,0);
		else if (idx == 1) vBarycentric = vec3(0,1,0);
		else vBarycentric = vec3(0,0,1);
	    vHeight = aPos.y;  // 传递高程值
    })";

	// 片段着色器（添加Gamma校正开关）
	// 修正后的片段着色器
	const char* fs = R"(
	#version 460 core
	in vec2 TexCoord;
	in vec3 vBarycentric; // 新增：重心坐标输入
	in float vHeight;  // 新增：输入高程值
	out vec4 FragColor;

	uniform bool coloredLayer;  // 新增：分层设色标志
	uniform float heightScale;  // 新增：高程缩放比例
	uniform vec3 terrainMinMax; // 新增：地形高程范围(min, max, range)
    
    // 根据数据类型选择采样器
    uniform bool useTexture;  // 新增：是否使用纹理
    uniform sampler2D texture8Bit;    // 8位/浮点纹理
    uniform usampler2D texture16Bit;  // 16位无符号整数纹理
    uniform bool is16Bit;

    // 颜色方案 uniform 变量
    uniform vec3 u_color1;
    uniform vec3 u_color2;
    uniform vec3 u_color3;
    uniform vec3 u_color4;
    uniform vec3 u_color5;
    uniform vec3 u_color6;
    uniform vec3 u_color7;

	// 分层设色函数
	vec4 getColorByHeight(float height) {
        // `height` 参数是已缩放的高度。
        // 原始着色器中，`vHeight` 是 `aPos.y`（已由 CPU 端的 `m_scale` 缩放）。
        // 然后调用 `getColorByHeight(vHeight / heightScale)`。
        // 如果 `heightScale` (uniform) == `m_scale` (CPU)，则 `vHeight / heightScale` 是未缩放的原始 DEM 高度。
        // 这似乎是预期的行为。所以 `actualHeight` 将是原始 DEM 高度。
		float actualHeight = height; 
    
		// 定义关键高度点 (这些是固定的，颜色是动态的)
		const float h1 = -4.0; const float h2 = 250.0; const float h3 = 500.0;
		const float h4 = 750.0; const float h5 = 1000.0; const float h6 = 1250.0;
		const float h7 = 1466.0;
    
		// 使用 uniform 颜色
		vec3 c1 = u_color1; vec3 c2 = u_color2; vec3 c3 = u_color3;
		vec3 c4 = u_color4; vec3 c5 = u_color5; vec3 c6 = u_color6;
		vec3 c7 = u_color7;
    
		// 计算渐变颜色
		vec3 color;
		if (actualHeight <= h1) {
			color = c1;
		} else if (actualHeight < h2) {
			float t = (actualHeight - h1) / (h2 - h1);
			color = mix(c1, c2, t);
		} else if (actualHeight < h3) {
			float t = (actualHeight - h2) / (h3 - h2);
			color = mix(c2, c3, t);
		} else if (actualHeight < h4) {
			float t = (actualHeight - h3) / (h4 - h3);
			color = mix(c3, c4, t);
		} else if (actualHeight < h5) {
			float t = (actualHeight - h4) / (h5 - h4);
			color = mix(c4, c5, t);
		} else if (actualHeight < h6) {
			float t = (actualHeight - h5) / (h6 - h5);
			color = mix(c5, c6, t);
		} else if (actualHeight < h7) {
			float t = (actualHeight - h6) / (h7 - h6);
			color = mix(c6, c7, t);
		} else {
			color = c7;
		}
    
		return vec4(color, 1.0);
	}

    void main() {
        // 调试：显示纹理坐标
        // FragColor = vec4(TexCoord.x, TexCoord.y, 0.0, 1.0);
        // return;

		if (coloredLayer) {
			// 使用分层设色
			FragColor = getColorByHeight(vHeight / heightScale);
		} else if (useTexture) {
			if (is16Bit) {
				// 读取16位无符号整数并归一化
				uint rawValue = texture(texture16Bit, TexCoord).r;
				float normalizedValue = float(rawValue) / 65535.0;
				FragColor = vec4(vec3(normalizedValue), 1.0);
			} else {
				// 直接读取8位/浮点数据
				FragColor = texture(texture8Bit, TexCoord);
			}
		} else {
			// 无纹理模式：绘制线框（黑色）和灰色表面
			float minBary = min(min(vBarycentric.x, vBarycentric.y), vBarycentric.z);
			if (minBary < 0.01) {
				FragColor = vec4(0.0, 0.0, 0.0, 1.0); // 黑色线框
			} else {
				FragColor = vec4(0.7, 0.7, 0.7, 1.0); // 灰色表面
			}
		}
    })";

	GLuint vertexShader = CompileShader(vs, GL_VERTEX_SHADER);
	GLuint fragmentShader = CompileShader(fs, GL_FRAGMENT_SHADER);

	m_shaderProgram = glCreateProgram();
	glAttachShader(m_shaderProgram, vertexShader);
	glAttachShader(m_shaderProgram, fragmentShader);
	glLinkProgram(m_shaderProgram);

	// 检查链接状态
	GLint success;
	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
		CString msg;
		msg.Format(_T("着色器程序链接错误:\n%hs"), infoLog);
		AfxMessageBox(msg);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		glDeleteProgram(m_shaderProgram);
		m_shaderProgram = 0; // 标记为无效
		return -1;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(m_shaderProgram); // 在获取 uniform 位置之前使用程序

	// 获取新增的uniform位置
	m_coloredLayerLoc = glGetUniformLocation(m_shaderProgram, "coloredLayer");
	m_heightScaleLoc = glGetUniformLocation(m_shaderProgram, "heightScale");
	// glUniform1i(glGetUniformLocation(m_shaderProgram, "terrainMinMax"), 2); // 这似乎不正确，terrainMinMax 是一个 vec3
	// terrainMinMax 的 uniform 已在 OnDraw 中获取和设置。

	// m_is16BitLoc = glGetUniformLocation(m_shaderProgram, "is16Bit"); // 此行已注释掉
	// glUniform1i(glGetUniformLocation(m_shaderProgram, "texture1"), 0); // "texture1" 可能是 "texture8Bit" 或 "texture16Bit"
	glUniform1i(glGetUniformLocation(m_shaderProgram, "texture8Bit"), 0); // 假设纹理单元 0 用于 8 位纹理
	glUniform1i(glGetUniformLocation(m_shaderProgram, "texture16Bit"), 0); // 如果一次只使用一个，则也为单元 0；如果同时使用，则为不同单元。
	// 着色器逻辑暗示仅根据 is16Bit 对一个（texture8Bit 或 texture16Bit）进行采样。

	m_useTextureLoc = glGetUniformLocation(m_shaderProgram, "useTexture");

	// 获取颜色方案的 uniform 位置
	const char* colorUniformNames[] = {
		"u_color1", "u_color2", "u_color3", "u_color4",
		"u_color5", "u_color6", "u_color7"
	};
	for (int i = 0; i < 7; ++i) {
		m_colorUniformLocs[i] = glGetUniformLocation(m_shaderProgram, colorUniformNames[i]);
		if (m_colorUniformLocs[i] == -1) { // 检查 uniform 是否找到
			CString msg;
			msg.Format(_T("警告: 在着色器中未找到 Uniform %hs。"), colorUniformNames[i]);
			OutputDebugString(msg + _T("\n")); // 输出到调试窗口
		}
	}

	// VAO 设置 (确保在 m_shaderProgram 链接并使用之后)
	// 此设置通常在加载缓冲区数据之后或 VAO 持久存在时完成。
	// 如果在此处创建 m_terrainVAO，则应在这些调用之前绑定它。
	// 但是，LoadDEM 会重新创建 m_terrainVAO，因此这可能是多余的或放错了位置。
	// 让我们假设 LoadDEM 使用属性指针正确处理 VAO 设置。
	// 原始代码在此处有这些调用，然后 LoadDEM 也设置了它们。
	// 如果尚未创建或绑定 m_terrainVAO，则可以。
	// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	// glEnableVertexAttribArray(0); // 在 VAO 绑定并且 VBO 有数据后启用
	// glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	// glEnableVertexAttribArray(1);


	// 如果可用，则根据某些默认地形尺寸初始化相机，或稍后设置
	// m_cameraPos = glm::vec3(0.0f, m_terrainHeight * 0.2f, m_terrainWidth * 0.3f); // m_terrainHeight/Width 在此处可能为 0
	// m_cameraFront = glm::normalize(glm::vec3(0.0f, -0.5f, -1.0f));
	// 这些最好在 LoadDEM 之后，当尺寸已知时设置。

	// 检查OpenGL版本
	const GLubyte* version = glGetString(GL_VERSION);
	CString msg;
	msg.Format(_T("OpenGL版本: %hs"), version);
	OutputDebugString(msg);

	return 0;
}

void CTerrainView::OnDestroy()
{
	wglDeleteContext(m_hRC);
	delete m_pDC;
	CView::OnDestroy();
}

void CTerrainView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	if (cx == 0 || cy == 0) return;
	glViewport(0, 0, cx, cy);
}

void CTerrainView::LoadDEM(const char* filename)
{
	GDALAllRegister();

	CPLSetConfigOption("SHAPE_ENCODING", "");//支持中文
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	// 清理旧地形数据
	if (glIsVertexArray(m_terrainVAO)) glDeleteVertexArrays(1, &m_terrainVAO);
	if (glIsBuffer(m_terrainVBO)) glDeleteBuffers(1, &m_terrainVBO);
	m_vertices.clear();

	GDALDataset* poDataset = (GDALDataset*)GDALOpen(filename, GA_ReadOnly);
	if (!poDataset) return;

	// 获取地理转换参数
	if (poDataset->GetGeoTransform(m_demGeoTransform) != CE_None) {
		AfxMessageBox(_T("DEM文件缺少地理参考信息!"));
		GDALClose(poDataset);
		return;
	}

	// 将WGS84坐标转换为CGCS2000 39带投影坐标
	if (!ConvertGeoTransformToProjected(m_demGeoTransform)) {
		AfxMessageBox(_T("DEM坐标转换失败"));
		GDALClose(poDataset);
		return;
	}

	m_terrainWidth = poDataset->GetRasterXSize();
	m_terrainHeight = poDataset->GetRasterYSize();
	GDALRasterBand* poBand = poDataset->GetRasterBand(1);

	m_demData.resize(m_terrainWidth * m_terrainHeight);
	poBand->RasterIO(GF_Read, 0, 0, m_terrainWidth, m_terrainHeight,
		m_demData.data(), m_terrainWidth, m_terrainHeight, GDT_Float32, 0, 0);

	// 计算地形边界
	double minX = m_demGeoTransform[0];
	double maxX = m_demGeoTransform[0] + m_terrainWidth * m_demGeoTransform[1];
	double minY = m_demGeoTransform[3] + m_terrainHeight * m_demGeoTransform[5]; // 注意：m_demGeoTransform[5]是负值
	double maxY = m_demGeoTransform[3];

	// 计算地形中心点（投影坐标，单位：米）
	m_terrainCenter.x = (minX + maxX) / 2.0;
	m_terrainCenter.y = (minY + maxY) / 2.0;

	// 计算地形高程范围
	float minHeight = FLT_MAX;
	float maxHeight = -FLT_MAX;
	for (float height : m_demData) {
		if (height < minHeight) minHeight = height;
		if (height > maxHeight) maxHeight = height;
	}
	m_terrainMinMax = glm::vec3(minHeight, maxHeight, maxHeight - minHeight);

	// 使用当前缩放比例生成顶点
	RegenerateTerrainVertices();

	glGenVertexArrays(1, &m_terrainVAO);
	glGenBuffers(1, &m_terrainVBO);

	glBindVertexArray(m_terrainVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_terrainVBO);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float),
		m_vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
		(void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	GDALClose(poDataset);
}

void CTerrainView::AddVertex(int x, int y, const std::vector<float>& demData, float scale) {
	// x，y行列号，转换为真实地理坐标
	double geoX = m_demGeoTransform[0] + x * m_demGeoTransform[1] + y * m_demGeoTransform[2];
	double geoY = m_demGeoTransform[3] + x * m_demGeoTransform[4] + y * m_demGeoTransform[5];

	// 计算相对于地形中心的偏移量（单位：米）
	double offsetX = geoX - m_terrainCenter.x;
	double offsetY = geoY - m_terrainCenter.y;

	// 精确计算纹理坐标
	double texPixelX = (geoX - m_textureGeoTransform[0]) / m_textureGeoTransform[1];
	double texPixelY = (geoY - m_textureGeoTransform[3]) / m_textureGeoTransform[5];

	// 归一化处理
	float u = static_cast<float>(texPixelX / (m_texturePixelWidth - 1));
	float v = 1.0f - static_cast<float>(texPixelY / (m_texturePixelHeight - 1));

	// 边界约束
	u = std::clamp(u, 0.0f, 1.0f);
	v = std::clamp(v, 0.0f, 1.0f);

	// 顶点坐标计算
	float xPos = offsetX * 0.01;
	float zPos = offsetY * 0.01;
	float yPos = demData[y * m_terrainWidth + x] * scale;

	// 添加顶点数据
	m_vertices.insert(m_vertices.end(), { xPos, yPos, zPos, u, v });
}


//TIFF的影像的波段数、位数等很多，有的格式opengl并不直接支持，需要分情况处理
//目前的纹理支持单波段8位、多波段16位、多波段8位等格式tif，其他情况还需要遇到后再测试
//考虑到opengl对16位的支持有限（软件版本、硬件配置等），需要将其先转换成8位。因此不涉及opengl对16位的支持
//但是影像解码转换需要占用不少时间
void CTerrainView::LoadTIFFWithLibTiff(const char* path)
{
	//在gdal这里处理两种情况，1.灰度图像转24彩色  2.16位多波段转8位多波段 
	//其余的 8位深的，下面libtiff处理应无问题。
	//两种情况共用纹理处理代码

	////////////////////////////////////////////////////////
	//一，gdal获得部分信息，处理上述两种情况，转为8位rgb、rgba
	GDALAllRegister();

	CPLSetConfigOption("SHAPE_ENCODING", "");//支持中文
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	// 打开数据集
	GDALDataset* poDataset = static_cast<GDALDataset*>(GDALOpen(path, GA_ReadOnly));
	if (!poDataset) {
		AfxMessageBox(_T("无法打开纹理文件"));
		return;
	}
	if (poDataset->GetGeoTransform(m_textureGeoTransform) != CE_None) {
		AfxMessageBox(_T("纹理文件缺少地理参考信息!"));
		GDALClose(poDataset);
		return;
	}
	uint32 width, height;

	// 将WGS84坐标转换为CGCS2000 39带投影坐标
	if (!ConvertGeoTransformToProjected(m_textureGeoTransform)) {
		AfxMessageBox(_T("纹理坐标转换失败"));
		GDALClose(poDataset);
		return;
	}

	m_texturePixelWidth = poDataset->GetRasterXSize();
	m_texturePixelHeight = poDataset->GetRasterYSize();
	int bandCount = poDataset->GetRasterCount();
	width = m_texturePixelWidth;
	height = m_texturePixelHeight;

	GDALRasterBand* poBand = poDataset->GetRasterBand(1);
	GDALDataType type = poBand->GetRasterDataType();
	int pixelSize = GDALGetDataTypeSizeBytes(type);

	uint16 bitsPerSample = GDALGetDataTypeSizeBits(type);
	GLenum format, internalFormat, dataType;

	pixelSize = 1;
	uint16 channels = 3;
	const tmsize_t stride = width * pixelSize * channels;
	std::vector<uint8_t> buffer(stride * height);

	bool bDone = false;//是否已被gdal处理
	if (bandCount == 1 && type == GDT_Byte)//单波段、8位灰度图像转24位彩色
	{
		bDone = true;
		// 读取灰度数据
		uint8_t* grayData = (uint8_t*)CPLMalloc(width * height);
		poBand->RasterIO(GF_Read, 0, 0, width, height, grayData, width, height, GDT_Byte, 0, 0);

		// 检查输入图像是否包含颜色表
		const GDALColorTable* colorTable = poBand->GetColorTable();
		std::vector<uint8_t> rLUT(256), gLUT(256), bLUT(256);

		if (colorTable) {
			// 使用输入图像的颜色表
			for (int i = 0; i < colorTable->GetColorEntryCount(); ++i) {
				const GDALColorEntry* entry = colorTable->GetColorEntry(i);
				rLUT[i] = static_cast<uint8_t>(entry->c1);
				gLUT[i] = static_cast<uint8_t>(entry->c2);
				bLUT[i] = static_cast<uint8_t>(entry->c3);
			}
		}
		else {
			// 创建灰度颜色表（默认黑白）
			for (int i = 0; i < 256; ++i) {
				rLUT[i] = gLUT[i] = bLUT[i] = static_cast<uint8_t>(i);
			}
		}

		for (int i = 0; i < width * height; ++i) {
			const uint8_t val = grayData[i];
			buffer[i * 3] = rLUT[val];
			buffer[i * 3 + 1] = gLUT[val];
			buffer[i * 3 + 2] = bLUT[val];
		}
	}
	else if (type == GDT_UInt16 || type == GDT_Int16)//16位多波段转8位多波段
	{
		bDone = true;

		for (int iBand = 0; iBand < bandCount; iBand++)
		{
			uint16_t* srcData = (uint16_t*)malloc(sizeof(uint16_t) * width * height);
			poDataset->GetRasterBand(iBand + 1)->RasterIO(GF_Read, 0, 0, width, height, srcData, width, height, GDT_UInt16, 0, 0);

			// 统计原图的最小最大值
			int src_min = 65535, src_max = 0;
			for (int i = 0; i < width * height; i++)
			{
				uint16_t val = srcData[i];
				if (val < src_min) src_min = val;
				if (val > src_max) src_max = val;
			}

			// 计算直方图
			double* frequency_val = (double*)calloc(src_max + 1, sizeof(double));
			for (int i = 0; i < width * height; i++)
				frequency_val[srcData[i]] += 1.0;

			for (int i = 0; i <= src_max; i++)
				frequency_val[i] /= (width * height);

			// 计算累积频率
			double* accum_freq = (double*)calloc(src_max + 1, sizeof(double));
			accum_freq[0] = frequency_val[0];
			for (int i = 1; i <= src_max; i++)
				accum_freq[i] = accum_freq[i - 1] + frequency_val[i];

			// 确定截断点：2%和98%
			int minVal = 0, maxVal = src_max;
			for (int i = 0; i <= src_max; i++)
				if (accum_freq[i] > 0.02) { minVal = i; break; }

			for (int i = src_max; i >= 0; i--)
				if (accum_freq[i] < 0.98) { maxVal = i; break; }

			// 处理像素转换
			//uint8_t* dstData = (uint8_t*)malloc(src_width * src_height);
			for (int i = 0; i < width * height; i++)
			{
				uint16_t val = srcData[i];


				long nPixelPos = i * bandCount + iBand;
				if (val < minVal)
					buffer[nPixelPos] = 0;
				else if (val > maxVal)
					buffer[nPixelPos] = 255;
				else
				{
					double scaled = (double)(val - minVal) / (maxVal - minVal);
					scaled = pow(scaled, 0.5); // Gamma校正
					buffer[nPixelPos] = (uint8_t)(scaled * 255.0);
				}
			}

			free(srcData);
			free(frequency_val);
			free(accum_freq);
		}
	}
	//按8位多波段来设定下述参数
	internalFormat = GL_RGB;
	format = GL_RGB;
	dataType = GL_UNSIGNED_BYTE;

	GDALClose(poDataset);
	////////////////////////////////////////////////////
	//二、用libtiff处理 上面gdal处理之外的情况，无法处理的影像要排除
	// 1. 打开TIFF文件
	if (!bDone)
	{
		TIFF* tif = TIFFOpen(path, "r");
		if (!tif) {
			//	std::cerr << "无法打开TIFF文件: " << path << std::endl;
			return;
		}

		// 2. 读取基本参数
		uint16 sampleFormat;
		// 	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
		// 	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
		// 	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &channels);
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
		TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat);

		m_texturePixelWidth = width;
		m_texturePixelHeight = height;

		// 3. 验证支持的格式
		const bool isSupported = (channels == 1 || channels == 3 || channels == 4) &&
			(bitsPerSample == 8 || bitsPerSample == 16 || bitsPerSample == 32) &&
			(sampleFormat == SAMPLEFORMAT_UINT || sampleFormat == SAMPLEFORMAT_IEEEFP);

		if (!isSupported) {
			AfxMessageBox("不支持的TIFF格式纹理", MB_OK);
			// 			<< channels << "通道 "
			// 			<< bitsPerSample << "位 "
			// 			<< (sampleFormat == SAMPLEFORMAT_IEEEFP ? "浮点" : "整型")
			// 			<< std::endl;
			TIFFClose(tif);
			return;
		}

		// 4. 确定OpenGL格式

		switch (channels) {
		case 1: format = GL_RED; break;
		case 3: format = GL_RGB; break;
		case 4: format = GL_RGBA; break;
		}

		switch (bitsPerSample) {
		case 8:
			dataType = GL_UNSIGNED_BYTE;
			internalFormat = (channels == 1) ? GL_R8 :
				(channels == 3) ? GL_RGB8 :
				GL_RGBA8;
			break;
		case 16:
			dataType = GL_UNSIGNED_SHORT;
			internalFormat = (channels == 1) ? GL_R16 :
				(channels == 3) ? GL_RGB16 :
				GL_RGBA16;
			break;
		case 32:
			dataType = (sampleFormat == SAMPLEFORMAT_IEEEFP) ? GL_FLOAT : GL_UNSIGNED_INT;
			internalFormat = (channels == 1) ? GL_R32F :
				(channels == 3) ? GL_RGB32F :
				GL_RGBA32F;
			break;
		}

		// 5. 按行扫描、处理像素数据
		for (uint32 row = 0; row < height; ++row)
		{
			if (TIFFReadScanline(tif, &buffer[row * stride], row) == -1)
			{
				//std::cerr << "读取扫描线失败: 行 " << row << std::endl;
				TIFFClose(tif);
				return;
			}
		}
		TIFFClose(tif);
	}

	//三、下面是纹理处理用上面获得的影像信息，进行纹理设定
	// 7. 创建OpenGL纹理
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);

	// 8. 设置对齐方式
	const uint32 bytesPerPixel = (bitsPerSample / 8) * channels;
	glPixelStorei(GL_UNPACK_ALIGNMENT, (bytesPerPixel % 4) ? 1 : 4);

	// 9. 上传纹理数据
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dataType, buffer.data());

	// 10. 设置默认纹理参数
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

// 鼠标交互实现
void CTerrainView::OnLButtonDown(UINT nFlags, CPoint point) {
	m_bLeftDown = TRUE; m_lastMousePos = point; SetCapture();
	CView::OnLButtonDown(nFlags, point);
}
void CTerrainView::OnRButtonDown(UINT nFlags, CPoint point) {
	m_bRightDown = TRUE; m_lastMousePos = point; SetCapture();
}
void CTerrainView::OnMButtonDown(UINT nFlags, CPoint point) {
	m_bMiddleDown = TRUE; m_lastMousePos = point; SetCapture();
}

void CTerrainView::OnLButtonUp(UINT nFlags, CPoint point) {
	m_bLeftDown = FALSE; ReleaseCapture();
}
void CTerrainView::OnRButtonUp(UINT nFlags, CPoint point) {
	m_bRightDown = FALSE; ReleaseCapture();
}
void CTerrainView::OnMButtonUp(UINT nFlags, CPoint point) {
	m_bMiddleDown = FALSE; ReleaseCapture();
}

void CTerrainView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bLeftDown) { // 平移
		float dx = (point.x - m_lastMousePos.x) * 0.1f;
		float dy = (point.y - m_lastMousePos.y) * 0.1f;
		glm::vec3 right = glm::normalize(glm::cross(m_cameraFront, m_cameraUp));
		m_cameraPos -= right * dx;
		m_cameraPos += m_cameraUp * dy;
		Invalidate(FALSE);
	}
	else if (m_bRightDown) { // 提升
		float dy = (point.y - m_lastMousePos.y) * 0.1f;
		m_cameraPos.y += dy;
		Invalidate(FALSE);
	}
	else if (m_bMiddleDown) { // 缩放
		float dy = (point.y - m_lastMousePos.y) * 0.1f;
		m_cameraPos += m_cameraFront * dy;
		Invalidate(FALSE);
	}
	m_lastMousePos = point;
}

BOOL CTerrainView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	m_cameraPos += m_cameraFront * (zDelta / 120.0f);
	Invalidate(FALSE);
	return TRUE;
}

GLuint CTerrainView::CompileShader(const char* source, GLenum type)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		AfxMessageBox(CString(infoLog));
	}
	return shader;
}

void CTerrainView::OnFileOpen()
{
	// 显示顺序提示
	AfxMessageBox(_T("请先选择影像文件，再选择DEM文件！"), MB_ICONINFORMATION | MB_OK);

	// 释放旧资源
	if (glIsTexture(m_textureID)) glDeleteTextures(1, &m_textureID);
	if (glIsVertexArray(m_terrainVAO)) glDeleteVertexArrays(1, &m_terrainVAO);
	if (glIsBuffer(m_terrainVBO)) glDeleteBuffers(1, &m_terrainVBO);
	m_vertices.clear();

	// 选择影像文件
	CFileDialog dlgImg(TRUE, _T("tif"), NULL, OFN_FILEMUSTEXIST, _T("影像文件|*.tif|所有文件|*.*||"));
	if (dlgImg.DoModal() != IDOK) return;
	CString imgPath = dlgImg.GetPathName();

	// 选择DEM文件
	CFileDialog dlgDem(TRUE, _T("tif"), NULL, OFN_FILEMUSTEXIST, _T("DEM文件|*.tif|所有文件|*.*||"));
	if (dlgDem.DoModal() != IDOK) return;
	CString demPath = dlgDem.GetPathName();

	// 加载新数据
	LoadTIFFWithLibTiff(imgPath.GetBuffer());
	LoadDEM(demPath.GetBuffer());

	// 更新相机初始位置
	m_cameraPos = glm::vec3(0.0f, m_terrainHeight * 0.065f, m_terrainWidth * 0.2f);

	Invalidate(TRUE); // 强制重绘
}

bool CTerrainView::ConvertGeoTransformToProjected(double* geoTransform) {
	// 初始化GDAL（确保已调用GDALAllRegister()）
	GDALAllRegister();

	// 定义源坐标系 (WGS-84 EPSG:4326)
	OGRSpatialReference oSourceSRS;
	oSourceSRS.SetWellKnownGeogCS("WGS84");
	oSourceSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);

	// 定义目标坐标系 (CGCS2000 3度带39带 EPSG:4534)
	OGRSpatialReference oTargetSRS;
	// 使用importFromEPSG导入预定义坐标系
	if (oTargetSRS.importFromEPSG(4534) != OGRERR_NONE) {
		AfxMessageBox(_T("无法导入CGCS2000坐标系"));
		return false;
	}
	oTargetSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);

	// 创建坐标变换对象
	OGRCoordinateTransformation* poCT =
		OGRCreateCoordinateTransformation(&oSourceSRS, &oTargetSRS);

	if (!poCT) {
		AfxMessageBox(_T("创建坐标变换对象失败"));
		return false;
	}

	// 转换左上角坐标
	double x1 = geoTransform[0]; // 原始经度
	double y1 = geoTransform[3]; // 原始纬度
	if (!poCT->Transform(1, &x1, &y1)) {
		AfxMessageBox(_T("左上角坐标转换失败"));
		OGRCoordinateTransformation::DestroyCT(poCT);
		return false;
	}

	// 转换右上角坐标 (x方向+1像素)
	double x2 = geoTransform[0] + geoTransform[1];
	double y2 = geoTransform[3];
	if (!poCT->Transform(1, &x2, &y2)) {
		AfxMessageBox(_T("右上角坐标转换失败"));
		OGRCoordinateTransformation::DestroyCT(poCT);
		return false;
	}

	// 转换左下角坐标 (y方向+1像素)
	double x3 = geoTransform[0];
	double y3 = geoTransform[3] + geoTransform[5];
	if (!poCT->Transform(1, &x3, &y3)) {
		AfxMessageBox(_T("左下角坐标转换失败"));
		OGRCoordinateTransformation::DestroyCT(poCT);
		return false;
	}

	// 计算新参数
	geoTransform[0] = x1;             // 左上角X (米)
	geoTransform[3] = y1;             // 左上角Y (米)
	geoTransform[1] = x2 - x1;        // 水平分辨率 (米/像素)
	geoTransform[5] = y3 - y1;        // 垂直分辨率 (米/像素)
	geoTransform[2] = 0.0;             // 旋转系数清零
	geoTransform[4] = 0.0;             // 旋转系数清零

	// 清理资源
	OGRCoordinateTransformation::DestroyCT(poCT);
	return true;
}
void CTerrainView::OnScaleEnhance()
{
	if (m_scale * 100 >= 1) {
		// 放大比例 (步长0.01)
		m_scale += 0.01f;
	}
	else {
		// 缩小比例 (步长0.004)
		m_scale += 0.004f;
	}

	// 重新生成地形并刷新
	if (!m_demData.empty()) {
		RegenerateTerrainVertices();
		Invalidate(TRUE); // 触发重绘
	}
}

void CTerrainView::OnScaleReduce()
{
	if (m_scale * 100 > 1) {
		m_scale -= 0.01f;
	}
	else {
		// 缩小比例 (步长0.004)
		m_scale -= 0.004f;
	}

	if (m_scale < 0.001f) m_scale = 0.001f; // 防止负值

	// 重新生成地形并刷新
	if (!m_demData.empty()) {
		RegenerateTerrainVertices();
		Invalidate(TRUE); // 触发重绘
	}
}

void CTerrainView::RegenerateTerrainVertices()
{
	m_vertices.clear();

	// 使用当前缩放比例重新生成顶点
	for (int y = 0; y < m_terrainHeight - 1; y++)
	{
		for (int x = 0; x < m_terrainWidth - 1; x++)
		{
			// 第一个三角形
			AddVertex(x, y, m_demData, m_scale);
			AddVertex(x + 1, y, m_demData, m_scale);
			AddVertex(x, y + 1, m_demData, m_scale);

			// 第二个三角形
			AddVertex(x + 1, y, m_demData, m_scale);
			AddVertex(x + 1, y + 1, m_demData, m_scale);
			AddVertex(x, y + 1, m_demData, m_scale);
		}
	}

	// 更新顶点缓冲区
	if (glIsBuffer(m_terrainVBO)) {
		glBindBuffer(GL_ARRAY_BUFFER, m_terrainVBO);
		glBufferData(GL_ARRAY_BUFFER,
			m_vertices.size() * sizeof(float),
			m_vertices.data(),
			GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}
void CTerrainView::OnCloseTexture()
{
	m_bTextureEnabled = false;
	Invalidate(TRUE); // 重绘视图
}

void CTerrainView::OnOpenTexture()
{
	m_bTextureEnabled = true;
	Invalidate(TRUE); // 重绘视图
}

void CTerrainView::OnLayerColored()
{
	// 切换分层设色状态
	m_bColoredLayer = !m_bColoredLayer;

	// 关闭纹理显示
	if (m_bColoredLayer) {
		m_bTextureEnabled = false;
	}

	// 重绘视图
	Invalidate(TRUE);
}

void CTerrainView::OnColorSchemes()
{
	// 将当前的 glm::vec3 颜色转换为 COLORREF 给对话框
	COLORREF currentDialogColors[7];
	for (int i = 0; i < 7; ++i)
	{
		currentDialogColors[i] = RGB(
			static_cast<BYTE>(m_colorScheme[i].r * 255.0f),
			static_cast<BYTE>(m_colorScheme[i].g * 255.0f),
			static_cast<BYTE>(m_colorScheme[i].b * 255.0f)
		);
	}

	CColorSchemeDlg dlg(currentDialogColors, this); // 将当前颜色传递给对话框
	if (dlg.DoModal() == IDOK)
	{
		// 如果用户点击“确定”，则更新颜色方案
		for (int i = 0; i < 7; ++i)
		{
			m_colorScheme[i] = glm::vec3(
				GetRValue(dlg.m_selectedColors[i]) / 255.0f,
				GetGValue(dlg.m_selectedColors[i]) / 255.0f,
				GetBValue(dlg.m_selectedColors[i]) / 255.0f
			);
		}
		Invalidate(TRUE); // 触发重绘以应用新颜色
	}
}
