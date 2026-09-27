// Copyright (c) 2026, Alexandr Esilevich
//
// Distributed under the Boost Software License.
// See accompanying file LICENSE for license information.
//

#include <cxxmv/string.hpp>
#include <cxxmv/projection.hpp>
#include <cxxmv/adaptor.hpp>
#include <cxxmv/all.hpp>
#include <cxxmv/transform.hpp>
#include <cxxmv/operators.hpp>
#include <iostream>


int main() {
    mv::wstring str = L"First";
    mv::wstring str2 = L"Second";

    std::wcout << "sum1 = '" << *(str + L" suffix") << "'" << std::endl;
    std::wcout << "sum2 = '" << *(str + L" " + str2) << "'" << std::endl;

    return 0;
}
