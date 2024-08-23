
// ToPngDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "ToPng.h"
#include "ToPngDlg.h"
#include "afxdialogex.h"
#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// 读写数据线程参数类型
struct _CThreadParam {
    CToPngDlg* pWnd;
    std::string in;
    std::string out;
};



// CToPngDlg 对话框



CToPngDlg::CToPngDlg(CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_TOPNG_DIALOG, pParent)
    , m_strStatus(_T(""))
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_bBusy = FALSE;
}

void CToPngDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_STATUS, m_textCtrlStatus);
    DDX_Text(pDX, IDC_STATIC_STATUS, m_strStatus);
}

BEGIN_MESSAGE_MAP(CToPngDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_CLOSE()
    ON_WM_SIZE()
    ON_WM_DROPFILES()
    ON_COMMAND(ID_ENCODE, &CToPngDlg::OnEncode)
    ON_COMMAND(ID_DECODE, &CToPngDlg::OnDecode)
    ON_COMMAND(ID_EXIT, &CToPngDlg::OnExit)
    ON_MESSAGE(WM_BUSYCNANGED, &CToPngDlg::OnBusyChanged)
    ON_MESSAGE(WM_ENCODEDONE, &CToPngDlg::OnEncodeDone)
    ON_MESSAGE(WM_DECODEDONE, &CToPngDlg::OnDecodeDone)
END_MESSAGE_MAP()


// CToPngDlg 消息处理程序

BOOL CToPngDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);			// 设置大图标
    SetIcon(m_hIcon, FALSE);		// 设置小图标

    // TODO: 在此添加额外的初始化代码
    this->UpdateLayout();
    this->Busy(FALSE);

    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CToPngDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CToPngDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}



void CToPngDlg::OnOK()
{
}

void CToPngDlg::OnCancel()
{
    this->SendMessage(WM_CLOSE);
}

void CToPngDlg::OnClose()
{
    if (this->m_bBusy) {
        ::AfxMessageBox(_T("程序正忙..."), MB_OK | MB_ICONINFORMATION);
    }
    else {
        CDialog::OnClose();
        CDialog::OnCancel();
    }
}

void CToPngDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);
    this->UpdateLayout();
}

void CToPngDlg::OnDropFiles(HDROP hDropInfo)
{
    if (this->m_bBusy) {
        return;
    }

    if (DragQueryFile(hDropInfo, -1, NULL, 0) != 1) {
        ::AfxMessageBox(_T("仅支持拖入一个文件"), MB_OK | MB_ICONINFORMATION);
        return;
    }

    TCHAR buf[MAX_PATH];
    if (!DragQueryFile(hDropInfo, 0, buf, MAX_PATH)) {
        ::AfxMessageBox(_T("获取拖入文件路径失败"), MB_OK | MB_ICONERROR);
        return;
    }

    CString out;
    if (CString(buf).MakeUpper().Right(4) == _T(".PNG")) {
        if (this->ShowSaveFile(out)) {
            this->Decode(buf, out);
        }
    }
    else {
        if (this->ShowSavePngFile(out)) {
            this->Encode(buf, out);
        }
    }
}

void CToPngDlg::OnEncode()
{
    CString input, output;
    if (ShowOpenFile(input) && ShowSavePngFile(output)) {
        this->Encode(input, output);
    }
}

void CToPngDlg::OnDecode()
{
    CString input, output;
    if (ShowOpenPngFile(input) && ShowSaveFile(output)) {
        this->Decode(input, output);
    }
}

void CToPngDlg::OnExit()
{
    this->SendMessage(WM_CLOSE);
}

void CToPngDlg::UpdateLayout()
{
    CRect client;
    this->GetClientRect(client);

    CRect rtStatus;
    this->m_textCtrlStatus.GetWindowRect(rtStatus);
    this->m_textCtrlStatus.MoveWindow(0, (client.Height() - rtStatus.Height()) / 2, client.Width(), rtStatus.Height());
    this->m_textCtrlStatus.Invalidate();
}

void CToPngDlg::UpdateStatusText()
{
    this->m_strStatus = this->m_bBusy
        ? _T("加载中...")
        : _T("拖动文件到窗口以进行编码/解码");
    this->UpdateData(FALSE);
}

void CToPngDlg::UpdateEnables()
{
    CMenu* menu = this->GetMenu();
    menu->EnableMenuItem(ID_ENCODE, this->m_bBusy);
    menu->EnableMenuItem(ID_DECODE, this->m_bBusy);
}

void CToPngDlg::Busy(BOOL b)
{
    this->m_bBusy = b;
    this->SendMessage(WM_BUSYCNANGED);
}

LRESULT CToPngDlg::OnBusyChanged(WPARAM wParam, LPARAM lParam)
{
    this->UpdateStatusText();
    this->UpdateEnables();
    return 0;
}

LRESULT CToPngDlg::OnEncodeDone(WPARAM wParam, LPARAM lParam)
{
    if (wParam) {
        ::AfxMessageBox(_T("编码完成"), MB_OK | MB_ICONINFORMATION);
    }
    else {
        ::AfxMessageBox(this->m_strErrMsg, MB_OK | MB_ICONERROR);
    }
    return 0;
}

LRESULT CToPngDlg::OnDecodeDone(WPARAM wParam, LPARAM lParam)
{
    if (wParam) {
        ::AfxMessageBox(_T("解码完成"), MB_OK | MB_ICONINFORMATION);
    }
    else {
        ::AfxMessageBox(this->m_strErrMsg, MB_OK | MB_ICONERROR);
    }
    return 0;
}

BOOL CToPngDlg::ShowOpenFile(CString& refFileName)
{
    TCHAR szFilter[] = _T("所有文件(*.*)|*.*||");
    CFileDialog fileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);

    if (fileDlg.DoModal() == IDOK) {
        refFileName = fileDlg.GetPathName();
        return TRUE;
    }
    return FALSE;
}

BOOL CToPngDlg::ShowSavePngFile(CString& refFileName)
{
    TCHAR szFilter[] = _T("PNG文件(*.png)|*.png||");
    CFileDialog fileDlg(FALSE, _T("png"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);

    if (fileDlg.DoModal() == IDOK) {
        refFileName = fileDlg.GetPathName();
        return TRUE;
    }
    return FALSE;
}

BOOL CToPngDlg::ShowOpenPngFile(CString& refFileName)
{
    TCHAR szFilter[] = _T("PNG文件(*.png)|*.png|所有文件(*.*)|*.*||");
    CFileDialog fileDlg(TRUE, _T("png"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);

    if (fileDlg.DoModal() == IDOK) {
        refFileName = fileDlg.GetPathName();
        return TRUE;
    }
    return FALSE;
}

BOOL CToPngDlg::ShowSaveFile(CString& refFileName)
{
    TCHAR szFilter[] = _T("所有文件(*.*)|*.*||");
    CFileDialog fileDlg(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);

    if (fileDlg.DoModal() == IDOK) {
        refFileName = fileDlg.GetPathName();
        return TRUE;
    }
    return FALSE;
}

void CToPngDlg::Encode(const CString& input, const CString& output)
{
    USES_CONVERSION;
    ::AfxBeginThread([](void* param) -> UINT
        {
            bool ok = true;
            auto* p = reinterpret_cast<_CThreadParam*>(param);
            p->pWnd->Busy(TRUE);

            try {
                Utils::WriteFileToPng(p->in, p->out);
            }
            catch (std::exception e) {
                ok = false;
                p->pWnd->m_strErrMsg = Utils::Utf8ToCString(e.what());
            }

            p->pWnd->Busy(FALSE);
            p->pWnd->SendMessage(WM_ENCODEDONE, ok);

            delete p;
            return 0;
        }, new _CThreadParam({ this, T2A(input), T2A(output) }));
}

void CToPngDlg::Decode(const CString& input, const CString& output)
{
    USES_CONVERSION;
    ::AfxBeginThread([](void* param) -> UINT
        {
            bool ok = true;
            auto* p = reinterpret_cast<_CThreadParam*>(param);
            p->pWnd->Busy(TRUE);

            try {
                Utils::ExtractFileFromPng(p->in, p->out);
            }
            catch (std::exception e) {
                ok = false;
                p->pWnd->m_strErrMsg = Utils::Utf8ToCString(e.what());
            }

            p->pWnd->Busy(FALSE);
            p->pWnd->SendMessage(WM_DECODEDONE, ok);

            delete p;
            return 0;
        }, new _CThreadParam({ this, T2A(input), T2A(output) }));
}
