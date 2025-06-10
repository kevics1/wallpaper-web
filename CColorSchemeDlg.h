#pragma once
#include "afxdialogex.h"
#include <afxcolorbutton.h> // Required for CMFCColorButton

// CColorSchemeDlg 对话框

class CColorSchemeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CColorSchemeDlg)

public:
	CColorSchemeDlg(COLORREF initialColors[7], CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CColorSchemeDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COLOR_SCHEME };
#endif
	COLORREF m_selectedColors[7]; // To store the colors selected by the user

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	COLORREF m_initialColorsInternal[7]; // Internal storage for initial colors
	CMFCColorButton m_colorButtons[7];
	CStatic m_heightLabels[7]; // To manage height labels if needed, or set text directly

	// Default colors (as COLORREF)
	static COLORREF GetDefaultColor(int index);


	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedResetColors();
	afx_msg void OnOK();
};