#pragma once

namespace logo::diag {

// 词法阶段的错误码。后续 Parser / Interpreter 再分配各自的段位。
inline constexpr const char *kInvalidCharacter = "E1001";
inline constexpr const char *kMalformedNumber = "E1003";

} // namespace logo::diag
