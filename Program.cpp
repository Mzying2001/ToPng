#include "ToPngDlg.h"

int APIENTRY wWinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     PWSTR     pCmdLine,
    _In_     INT       nCmdShow)
{
    auto dlg = std::make_unique<ToPngDlg>();
    dlg->Show();
    return sw::App::MsgLoop();
}
