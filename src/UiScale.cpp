#include "UiScale.h"

#include <windows.h>

namespace UiScale {
namespace {
int g_dpi = 96;
}

void SetDpi(int dpi)
{
    g_dpi = dpi > 0 ? dpi : 96;
}

int Dpi()
{
    return g_dpi;
}

int Px(int value)
{
    if (value == 0) {
        return 0;
    }
    return ::MulDiv(value, g_dpi, 96);
}

}
