#pragma once
///@file

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "store-api.hh"

namespace nix {

class LibStoreTest : public virtual ::testing::Test {
    public:
        static void SetUpTestSuite() {
            initLibStore();
        }

    protected:
        LibStoreTest()
            : store(openStore("dummy://"))
        { }

        ref<Store> store;
};


} /* namespace nix */
