
#ifndef FLOAT_X_H_20151108
#define FLOAT_X_H_20151108

#include "API.h"
#include "Types.h"
#include <QValidator>
#include <cmath>
#include <limits>
#include <type_traits>

template <class Float>
Float EDB_EXPORT readFloat(const QString& strInput, bool& ok);

template <class Float>
class EDB_EXPORT FloatXValidator : public QValidator {
public:
    explicit FloatXValidator(QObject* parent = nullptr) : QValidator(parent) {}
	QValidator::State validate(QString& input, int&) const override;
};

template <class Float>
EDB_EXPORT QString formatFloat(Float value);

// Only class, nothing about sign
enum class FloatValueClass {
	Zero,
	Normal,
	Infinity,
	Denormal,
	PseudoDenormal,
	QNaN,
	SNaN,
	Unsupported
};

EDB_EXPORT FloatValueClass floatType(edb::value32 value);
EDB_EXPORT FloatValueClass floatType(edb::value64 value);
EDB_EXPORT FloatValueClass floatType(edb::value80 value);

// This will work not only for floats, but also for integers
template <class T>
constexpr int maxPrintedLength() {
	using Limits = std::numeric_limits<T>;

	constexpr bool isInteger        = Limits::is_integer;
	constexpr int mantissaChars     = isInteger ? 1 + Limits::digits10 : Limits::max_digits10;
	constexpr int signChars         = std::is_signed<T>::value;
	constexpr int expSignChars      = !isInteger;
	constexpr int decimalPointChars = !isInteger;
	constexpr int expSymbol         = !isInteger; // 'e' for floating-point value in scientific format
	const int expMaxWidth           = isInteger ? 0 : std::ceil(std::log10(Limits::max_exponent10));
	const int maxWidth  		    = signChars + mantissaChars + decimalPointChars + expSymbol + expSignChars + expMaxWidth;

	return maxWidth;
}

#endif
