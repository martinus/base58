#include <base58/base58.h>

#include <doctest/doctest.h>
#include <fmt/format.h>
#include <nanobench.h>

#include <string>

TEST_CASE("base58") {
    ankerl::nanobench::Bench().run("huh", [] {
        std::string str;
    });
}
