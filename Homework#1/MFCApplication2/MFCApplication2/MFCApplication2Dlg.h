
// MFCApplication2Dlg.h : header file
//

#pragma once


// CMFCApplication2Dlg dialog
class CMFCApplication2Dlg : public CDialogEx
{
// Construction
public:
	CMFCApplication2Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MFCAPPLICATION2_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
afx_msg void OnEnChangeEdit1();
afx_msg void OnEnChangeEdit2();
afx_msg void OnBnClickedRadio1();
afx_msg void OnEnChangeEdit3();
afx_msg void OnBnClickedButton3();
afx_msg void OnBnClickedButton2();
afx_msg void OnBnClickedOk();
afx_msg void OnEnChangeEdit4();
afx_msg void OnCbnSelchangeCombo2();
afx_msg void OnEnChangeEdit6();
afx_msg void OnBnClickedRadio3();
afx_msg void OnBnClickedRadio2();
afx_msg void OnBnClickedRadio4();
afx_msg void OnBnClickedButton7();
afx_msg void OnEnChangeEditEntropy();
afx_msg void OnEnChangeEditOutsize();
afx_msg void OnEnChangeEdit5();
afx_msg void OnEnChangeEdit7();
afx_msg void OnBnClickedButton9();
afx_msg void OnBnClickedButton10();
afx_msg void OnBnClickedButton8();
afx_msg void OnBnClickedButton11();
afx_msg void OnBnClickedButton4();
afx_msg void OnBnClickedButton6();
afx_msg void OnBnClickedButton5();
afx_msg void OnBnClickedButton12();
afx_msg void OnBnClickedButton14();
afx_msg void OnBnClickedButton13();
afx_msg void OnBnClickedButton15();
afx_msg void OnBnClickedRadio7();
afx_msg void OnBnClickedRadio8();
afx_msg void OnEnChangeEdit8();
};
