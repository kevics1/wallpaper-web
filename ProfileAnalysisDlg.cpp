// ProfileAnalysisDlg.cpp: 实现文件
//

#include "pch.h"
#include "Terrain.h"
#include "afxdialogex.h"
#include "ProfileAnalysisDlg.h"


// ProfileAnalysisDlg 对话框

IMPLEMENT_DYNAMIC(ProfileAnalysisDlg, CDialogEx)

ProfileAnalysisDlg::ProfileAnalysisDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PROFILE_ANALYSIS, pParent)
{

}

ProfileAnalysisDlg::~ProfileAnalysisDlg()
{
}

void ProfileAnalysisDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ProfileAnalysisDlg, CDialogEx)
END_MESSAGE_MAP()


// ProfileAnalysisDlg 消息处理程序
