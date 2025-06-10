// CColorSchemeDlg.cpp: 实现文件
//

#include "pch.h"
#include "Terrain.h" // Assuming your main application header is Terrain.h
#include "afxdialogex.h"
#include "CColorSchemeDlg.h"
#include "resource.h" // Ensure resource.h is included

// CColorSchemeDlg 对话框

IMPLEMENT_DYNAMIC(CColorSchemeDlg, CDialogEx)

CColorSchemeDlg::CColorSchemeDlg(COLORREF initialColors[7], CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COLOR_SCHEME, pParent)
{
	for (int i = 0; i < 7; ++i)
	{
		m_initialColorsInternal[i] = initialColors[i];
		m_selectedColors[i] = initialColors[i]; // Initialize selected with initial
	}
}

CColorSchemeDlg::~CColorSchemeDlg()
{
}

void CColorSchemeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	// Associate CMFCColorButton members with control IDs
	for (int i = 0; i < 7; ++i)
	{
		DDX_Control(pDX, IDC_COLOR_BUTTON1 + i, m_colorButtons[i]);
		// For labels, we'll set text in OnInitDialog. If they were edit boxes, DDX_Text would be used.
		// If IDC_EDIT_HEIGHT1 etc are CStatic, we can get them via GetDlgItem.
	}
}

COLORREF CColorSchemeDlg::GetDefaultColor(int index)
{
	// Default colors corresponding to the shader's original c1-c7
	// c1: vec3(0.0, 0.0, 1.0) -> RGB(0,0,255) (Blue)
	// c2: vec3(0.0, 207.0/255.0, 65.0/255.0) -> RGB(0, 207, 65) (Teal/Turquoise like)
	// c3: vec3(144.0/255.0, 238.0/255.0, 144.0/255.0) -> RGB(144, 238, 144) (Light Green)
	// c4: vec3(1.0, 1.0, 0.0) -> RGB(255, 255, 0) (Yellow)
	// c5: vec3(1.0, 165.0/255.0, 0.0) -> RGB(255, 165, 0) (Orange)
	// c6: vec3(1.0, 69.0/255.0, 0.0) -> RGB(255, 69, 0) (Orange-Red)
	// c7: vec3(1.0, 0.0, 0.0) -> RGB(255, 0, 0) (Red)
	switch (index)
	{
	case 0: return RGB(0, 0, 255);
	case 1: return RGB(0, 207, 65);
	case 2: return RGB(144, 238, 144);
	case 3: return RGB(255, 255, 0);
	case 4: return RGB(255, 165, 0);
	case 5: return RGB(255, 69, 0);
	case 6: return RGB(255, 0, 0);
	default: return RGB(0, 0, 0); // Should not happen
	}
}


BOOL CColorSchemeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Height values (fixed)
	const float heights[] = { -4.0f, 250.0f, 500.0f, 750.0f, 1000.0f, 1250.0f, 1466.0f };
	CString labelTexts[7];
	for (int i = 0; i < 7; ++i)
	{
		labelTexts[i].Format(_T("高度 %d (%.1fm):"), i + 1, heights[i]);
		// Assuming IDC_EDIT_HEIGHT1 to IDC_EDIT_HEIGHT7 are the IDs for CStatic labels
		CWnd* pLabel = GetDlgItem(IDC_EDIT_HEIGHT1 + i);
		if (pLabel)
		{
			pLabel->SetWindowText(labelTexts[i]);
		}

		// Initialize color buttons with the passed-in colors
		m_colorButtons[i].SetColor(m_initialColorsInternal[i]);
		m_colorButtons[i].EnableAutomaticButton(_T("自动"), TRUE); // Optional: add "Automatic" button
		m_colorButtons[i].EnableOtherButton(_T("更多颜色..."));    // Optional: add "More Colors" button
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}


BEGIN_MESSAGE_MAP(CColorSchemeDlg, CDialogEx)
	ON_BN_CLICKED(IDC_RESET_COLORS, &CColorSchemeDlg::OnBnClickedResetColors)
END_MESSAGE_MAP()


// CColorSchemeDlg 消息处理程序

void CColorSchemeDlg::OnBnClickedResetColors()
{
	for (int i = 0; i < 7; ++i)
	{
		COLORREF defaultColor = GetDefaultColor(i);
		m_colorButtons[i].SetColor(defaultColor);
		// m_selectedColors will be updated in OnOK based on button states
	}
}

void CColorSchemeDlg::OnOK()
{
	// Retrieve the selected colors from the buttons
	for (int i = 0; i < 7; ++i)
	{
		m_selectedColors[i] = m_colorButtons[i].GetColor();
	}
	CDialogEx::OnOK();
}