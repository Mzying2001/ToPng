#include "ToPngDlg.h"
#include "MyUtils.h"
#include <thread>

ToPngDlg::ToPngDlg()
{
    Menu = &menu;
    Width = 500;
    Height = 350;
    Text = L"ToPng";
    MaximizeBox = false;
    BackColor = sw::KnownColor::Control;
    StartupLocation = sw::WindowStartupLocation::CenterScreen;
    SetLayout<sw::FillLayout>();
    InitializeComponent();

    AcceptFiles = true;
    RegisterRoutedEvent<sw::DropFilesEventArgs>(*this, &ToPngDlg::DropFilesHandler);
    RegisterRoutedEvent<sw::WindowClosingEventArgs>(*this, &ToPngDlg::WindowClosingHandler);
}

void ToPngDlg::InitializeComponent()
{
    labelStatus.AutoWrap = true;
    labelStatus.HorizontalContentAlignment = sw::HorizontalAlignment::Center;
    labelStatus.SetAlignment(sw::HorizontalAlignment::Center, sw::VerticalAlignment::Center);
    UpdateStatusText();
    AddChild(labelStatus);
}

LRESULT ToPngDlg::WndProc(const sw::ProcMsg& refMsg)
{
    switch (refMsg.uMsg)
    {
    case WM_ENCODEDONE: {
        OnEncodeDone(refMsg.wParam);
        return 0;
    }
    case WM_DECODEDONE: {
        OnDecodeDone(refMsg.wParam);
        return 0;
    }
    case WM_ISLOADINGCHANGED: {
        UpdateStatusText();
        UpdateEnables();
        return 0;
    }
    default: {
        return sw::Window::WndProc(refMsg);
    }
    }
}

void ToPngDlg::WindowClosingHandler(sw::UIElement& sender, sw::WindowClosingEventArgs& e)
{
    if (isLoading) {
        e.cancel = true;
        sw::MsgBox::ShowInfo(L"程序正忙...", L"提示");
    }
}

void ToPngDlg::DropFilesHandler(sw::UIElement& sender, sw::DropFilesEventArgs& e)
{
    HDROP hDropInfo = e.hDrop;

    if (isLoading) {
        return;
    }

    if (DragQueryFileW(hDropInfo, -1, NULL, 0) != 1) {
        sw::MsgBox::ShowInfo(L"仅支持拖入一个文件", L"提示");
        return;
    }

    wchar_t buf[MAX_PATH];
    if (!DragQueryFileW(hDropInfo, 0, buf, MAX_PATH)) {
        sw::MsgBox::ShowError(L"获取拖入文件路径失败", L"错误");
        return;
    }

    std::wstring out;
    if (GetFileAttributesW(buf) & FILE_ATTRIBUTE_DIRECTORY) {
        if (ShowSavePngFile(out)) {
            EncodeDir(buf, out);
        }
    }
    else if (StrEndsWithIgnoreCase(buf, L".PNG")) {
        if (ShowSaveFile(out)) {
            Decode(buf, out);
        }
    }
    else {
        if (ShowSavePngFile(out)) {
            Encode(buf, out);
        }
    }
}

void ToPngDlg::MenuCommandHandler(sw::MenuItem& menuItem)
{
    switch (menuItem.tag)
    {
    case MENUITEM_ENCODE: {
        std::wstring input, output;
        if (ShowOpenFile(input) && ShowSavePngFile(output)) {
            Encode(input, output);
        }
        break;
    }
    case MENUITEM_DECODE: {
        std::wstring input, output;
        if (ShowOpenPngFile(input) && ShowSaveFile(output)) {
            Decode(input, output);
        }
        break;
    }
    case MENUITEM_ENCODEDIR: {
        std::wstring input, output;
        if (ShowOpenDirectory(input) && ShowSavePngFile(output)) {
            EncodeDir(input, output);
        }
        break;
    }
    case MENUITEM_EXIT: {
        Close();
        break;
    }
    }
}

void ToPngDlg::UpdateStatusText()
{
    labelStatus.Text = isLoading
        ? L"加载中..."
        : L"拖动文件到窗口以进行编码/解码";
}

void ToPngDlg::UpdateEnables()
{
    menu.SetEnabled(*menu.GetMenuItemByTag(MENUITEM_ENCODE), !isLoading);
    menu.SetEnabled(*menu.GetMenuItemByTag(MENUITEM_DECODE), !isLoading);
    menu.SetEnabled(*menu.GetMenuItemByTag(MENUITEM_ENCODEDIR), !isLoading);
}

void ToPngDlg::SetLoadingState(bool isLoading)
{
    this->isLoading = isLoading;
    SendMessageW(WM_ISLOADINGCHANGED, 0, 0);
}

void ToPngDlg::OnEncodeDone(bool success)
{
    if (success) {
        sw::MsgBox::ShowInfo(L"编码完成", L"提示");
    }
    else {
        sw::MsgBox::ShowError(errMsg, L"错误");
    }
}

void ToPngDlg::OnDecodeDone(bool success)
{
    if (success) {
        sw::MsgBox::ShowInfo(L"解码完成", L"提示");
    }
    else {
        sw::MsgBox::ShowError(errMsg, L"错误");
    }
}

void ToPngDlg::Encode(const std::wstring& input, const std::wstring& output)
{
    if (isLoading) {
        return;
    }

    std::thread(
        [=]() {
            SetLoadingState(true);

            bool ok = true;
            try {
                MyUtils::WriteFileToPng(
                    sw::Utils::ToMultiByteStr(input),
                    sw::Utils::ToMultiByteStr(output));
            }
            catch (const std::exception& e) {
                ok = false;
                errMsg = sw::Utils::ToWideStr(e.what(), true);
            }

            SetLoadingState(false);
            SendMessageW(WM_ENCODEDONE, ok, 0);
        }).detach();
}

void ToPngDlg::Decode(const std::wstring& input, const std::wstring& output)
{
    if (isLoading) {
        return;
    }

    std::thread(
        [=]() {
            SetLoadingState(true);

            bool ok = true;
            try {
                MyUtils::ExtractFileFromPng(
                    sw::Utils::ToMultiByteStr(input),
                    sw::Utils::ToMultiByteStr(output));
            }
            catch (const std::exception& e) {
                ok = false;
                errMsg = sw::Utils::ToWideStr(e.what(), true);
            }

            SetLoadingState(false);
            SendMessageW(WM_DECODEDONE, ok, 0);
        }).detach();
}

void ToPngDlg::EncodeDir(const std::wstring& input, const std::wstring& output)
{
    if (isLoading) {
        return;
    }

    std::thread(
        [=]() {
            SetLoadingState(true);

            bool ok = true;
            try {
                MyUtils::ArchiveDirectoryToPng(
                    sw::Utils::ToMultiByteStr(input),
                    sw::Utils::ToMultiByteStr(output));
            }
            catch (const std::exception& e) {
                ok = false;
                errMsg = sw::Utils::ToWideStr(e.what(), true);
            }

            SetLoadingState(false);
            SendMessageW(WM_ENCODEDONE, ok, 0);
        }).detach();
}

bool ToPngDlg::ShowOpenFile(std::wstring& refFileName)
{
    sw::OpenFileDialog ofd;
    ofd.Filter->AddFilter(L"所有文件(*.*)", L"*.*");

    if (ofd.ShowDialog()) {
        refFileName = ofd.FileName;
        return true;
    }
    return false;
}

bool ToPngDlg::ShowSavePngFile(std::wstring& refFileName)
{
    sw::SaveFileDialog sfd;
    sfd.Filter->AddFilter(L"PNG文件(*.png)", L"*.png", L".png");
    sfd.Filter->AddFilter(L"所有文件(*.*)", L"*.*");

    if (sfd.ShowDialog()) {
        refFileName = sfd.FileName;
        return true;
    }
    return false;
}

bool ToPngDlg::ShowOpenPngFile(std::wstring& refFileName)
{
    sw::OpenFileDialog ofd;
    ofd.Filter->AddFilter(L"PNG文件(*.png)", L"*.png");

    if (ofd.ShowDialog()) {
        refFileName = ofd.FileName;
        return true;
    }
    return false;
}

bool ToPngDlg::ShowSaveFile(std::wstring& refFileName)
{
    sw::SaveFileDialog sfd;
    sfd.Filter->AddFilter(L"所有文件(*.*)", L"*.*");

    if (sfd.ShowDialog()) {
        refFileName = sfd.FileName;
        return true;
    }
    return false;
}

bool ToPngDlg::ShowOpenDirectory(std::wstring& refDirectory)
{
    sw::FolderBrowserDialog fbd;

    if (fbd.ShowDialog()) {
        refDirectory = fbd.SelectedPath;
        return true;
    }
    return false;
}

bool ToPngDlg::StrEndsWithIgnoreCase(const std::wstring& str, const std::wstring& suffix)
{
    if (str.size() < suffix.size()) {
        return false;
    }

    std::wstring strEnd = str.substr(str.size() - suffix.size());

    for (size_t i = 0; i < strEnd.size(); ++i) {
        if (towlower(strEnd[i]) != towlower(suffix[i])) {
            return false;
        }
    }
    return true;
}
