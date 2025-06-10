// TerrainView.h: CTerrainView 类的接口
//

#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> // For glm::value_ptr
#include <vector>
#include <utility> // For std::pair
#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "ProfileAnalysisDlg.h"
// Forward declaration
class ProfileAnalysisDlg;


class CTerrainView : public CView
{
protected: // 仅从序列化创建
	CTerrainView();
	DECLARE_DYNCREATE(CTerrainView)

	HGLRC m_hRC;
	CDC* m_pDC;

	GLuint m_terrainVAO, m_terrainVBO, m_textureID;
	GLuint m_shaderProgram;

	std::vector<float> m_vertices;
	int m_terrainWidth, m_terrainHeight;

	glm::vec3 m_cameraPos;
	glm::vec3 m_cameraFront;
	glm::vec3 m_cameraUp;

	// Mouse interaction members
	CPoint m_lastMousePos;
	bool m_bLeftDown, m_bRightDown, m_bMiddleDown;
	float m_yaw_degrees;   // Yaw angle for camera rotation
	float m_pitch_degrees; // Pitch angle for camera rotation

	double m_demGeoTransform[6];
	double m_textureGeoTransform[6];
	int m_texturePixelWidth;
	int m_texturePixelHeight;
	// double m_textureWidth; // These seem unused, consider removing if not needed elsewhere
	// double m_textureHeight;
	glm::dvec2 m_terrainCenter;
	bool ConvertGeoTransformToProjected(double* geoTransform);

	float m_scale;
	std::vector<float> m_demData;
	void RegenerateTerrainVertices();
	GDALDataType m_dataType;

	bool m_bTextureEnabled;
	GLint m_useTextureLoc;

	bool m_bColoredLayer;
	GLint m_coloredLayerLoc;
	GLint m_heightScaleLoc;
	glm::vec3 m_terrainMinMax;

	// Color Scheme Data
	glm::vec3 m_colorScheme[7]; // Stores the 7 colors for the shader
	GLint m_colorUniformLocs[7]; // Uniform locations for the 7 colors

	void InitializeDefaultColors(); // Helper to set up default colors

	// Profile Analysis Members
	bool m_bProfileAnalysisMode;
	std::vector<glm::vec3> m_profilePointsWorld; // Store 3D world coordinates of selected points
	GLuint m_profileShader;       // Shader for drawing points/line
	GLuint m_profileVAO, m_profileVBO;
	GLint m_profileShader_modelLoc, m_profileShader_viewLoc, m_profileShader_projLoc, m_profileShader_colorLoc;
	glm::mat4 m_currentViewMatrix; // To store view matrix for unprojection
	glm::mat4 m_currentProjectionMatrix; // To store projection matrix for unprojection


	// Helper for profile analysis
	bool ScreenToWorld(CPoint screenPoint, glm::vec3& worldPoint);
	void GenerateProfileData(const glm::vec3& p1_world, const glm::vec3& p2_world, std::vector<std::pair<float, float>>& profileDataVec);
	void SetupProfileRender(); // To compile shader and setup VAO for profile visualization
	void UpdateProfileRenderVAO(); // To update VBO data


	// 特性
public:
	CTerrainDoc* GetDocument() const;

	// 操作
public:

	// 重写
public:
	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	// 实现
public:
	virtual ~CTerrainView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void AddVertex(int x, int y, const std::vector<float>& demData, float scale);
	void LoadDEM(const char* filename);
	GLuint CompileShader(const char* source, GLenum type);
	void LoadTIFFWithLibTiff(const char* path);

	// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnFileOpen();
public:
	afx_msg void OnScaleEnhance();
	afx_msg void OnScaleReduce();
	afx_msg void OnCloseTexture();
	afx_msg void OnOpenTexture();
	afx_msg void OnLayerColored();
	afx_msg void OnColorSchemes(); // This is the handler for ID_COLOR_SCHEME
	afx_msg void OnProfileAnalysis();
	afx_msg void OnViewEnhance();
	afx_msg void OnViewReduce();
	afx_msg void OnViewRe0();
	afx_msg void OnLength();
	afx_msg void OnArea();
};

#ifndef _DEBUG
inline CTerrainDoc* CTerrainView::GetDocument() const
{
	return reinterpret_cast<CTerrainDoc*>(m_pDocument);
}
#endif