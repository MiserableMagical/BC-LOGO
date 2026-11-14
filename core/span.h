#pragma once

namespace logo {

/// 源文本中的一个位置区间。offset/length 以 QChar 计，line/column 从 1 开始。
struct Span {
    int offset = 0;
    int length = 0;
    int line = 1;
    int column = 1;

    int endOffset() const { return offset + length; }
    bool isValid() const { return offset >= 0 && length >= 0 && line >= 1 && column >= 1; }
};

} // namespace logo
