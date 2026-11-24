#pragma once

namespace logo::diag {

// 词法阶段
inline constexpr const char *kInvalidCharacter = "E1001";
inline constexpr const char *kMalformedNumber = "E1003";
inline constexpr const char *kExpectedVariableName = "E1004";

// 语法阶段
inline constexpr const char *kUnknownCommand = "E2001";
inline constexpr const char *kExpectedExpression = "E2002";
inline constexpr const char *kExpectedWord = "E2003";
inline constexpr const char *kUnterminatedBlock = "E2004";
inline constexpr const char *kMissingEnd = "E2005";
inline constexpr const char *kUnexpectedEnd = "E2006";
inline constexpr const char *kReservedName = "E2007";
inline constexpr const char *kExpectedBlock = "E2008";
inline constexpr const char *kExpectedParen = "E2009";
inline constexpr const char *kUnexpectedToken = "E2010";
inline constexpr const char *kCommandAsValue = "E2011";
inline constexpr const char *kOutputOutsideProcedure = "E2012";
inline constexpr const char *kExpectedParameter = "E2013";

} // namespace logo::diag
