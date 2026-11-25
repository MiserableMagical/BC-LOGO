#pragma once

#include "core/runtime.h"

class MainWindow;

/// 把 core 的 Console 接口接到现有的 myListener。
class QtConsole final : public logo::Console {
public:
    explicit QtConsole(MainWindow *window);

    void print(const QString &text) override;

private:
    MainWindow *window_;
};
