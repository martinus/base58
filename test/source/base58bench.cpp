#include "reference.h"
#include <base58/base58.h>

#include <doctest/doctest.h>
#include <fmt/format.h>
#include <nanobench.h>

#include <string>

TEST_CASE("base58_bench") {
    // create some random data
    auto input = std::string();
    auto rng = ankerl::nanobench::Rng();
    for (size_t i = 0; i < 512; ++i) {
        input.push_back(static_cast<char>(rng()));
    }

    auto output = std::string();
    ankerl::nanobench::Bench().run("base58::encodeReference", [&] {
        output.clear();
        encodeReference(input.data(), input.size(), output);
        ankerl::nanobench::doNotOptimizeAway(output);
    });
    fmt::print("{}\n", output);

    ankerl::nanobench::Bench().run("base58::encode", [&] {
        output.clear();
        ankerl::base58::encode(input.data(), input.size(), output);
        ankerl::nanobench::doNotOptimizeAway(output);
    });
    fmt::print("{}\n", output);
}
