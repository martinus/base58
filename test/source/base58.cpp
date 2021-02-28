#include <base58/base58.h>

#include <doctest/doctest.h>
#include <fmt/format.h>

#include <cstddef>
#include <cstdint>
#include <string>

// provides constexpr math functions
namespace cxMath {

auto constexpr abs(double x) -> double {
    if (x < 0) {
        return -x;
    }
    return x;
}

auto constexpr feq(double x, double y, double epsilon) -> bool {
    return abs(x - y) <= epsilon;
}

auto constexpr exp(double x, double epsilon) -> double {
    double sum = 1;
    double n = 1;
    int i = 2;
    double t = x;

    while (!feq(sum, sum + t / n, epsilon)) {
        sum += t / n;
        n *= i;
        ++i;
        t *= x;
    }
    return sum;
}

// see https://en.wikipedia.org/wiki/Natural_logarithm#High_precision
auto constexpr log(double x, double epsilon) -> double {
    double curr = 0.0;
    double prev = 1.0;

    while (!feq(curr, prev, epsilon)) {
        prev = curr;
        auto expPrev = cxMath::exp(prev, epsilon);
        curr = prev + 2 * (x - expPrev) / (x + expPrev);
    }
    return curr;
}

} // namespace cxMath

// map from char to number, and from number to char.
class BaseMapper {
    std::array<uint8_t, 256> mCharToNumber{};
    size_t mBase{};
    char const* mAlphabet{};

public:
    constexpr BaseMapper(char const* alphabet)
        : mAlphabet(alphabet) {
        for (auto i = size_t(); i < mCharToNumber.size(); ++i) {
            mCharToNumber[i] = 255;
        }
        while (*alphabet) {
            mCharToNumber[static_cast<uint8_t>(*alphabet)] = mBase;
            ++mBase;
            ++alphabet;
        }
    }

    constexpr auto charToNumber(char c) const -> uint8_t {
        return mCharToNumber[static_cast<uint8_t>(c)];
    }

    constexpr auto base() const -> size_t {
        return mBase;
    }

    constexpr auto numberToChar(size_t num) const -> char {
        return mAlphabet[num];
    }
};

TEST_CASE("base58") {
    static constexpr auto map = BaseMapper("123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz");
    static constexpr auto ln256 = 5.54517744447956247534;
    static constexpr auto factor = cxMath::log(map.base(), 1e-50) / ln256;
    fmt::print("hello {}\n", map.charToNumber('d'));
    fmt::print("factor={}\n", factor);
}
