#include <gtest/gtest.h>
#include "../include/VK_Storage.h"
#include <chrono>


// »спользуем фиктивные часы дл€ управлени€ временем в тестах
struct TestClock {
    using duration = std::chrono::steady_clock::duration;
    using rep = std::chrono::steady_clock::rep;
    using period = std::chrono::steady_clock::period;
    using time_point = std::chrono::time_point<TestClock>;

    static time_point current_time;

    static time_point now() {
        return current_time;
    }

    static void advance(std::chrono::seconds secs) {
        current_time += secs;
    }
};

TestClock::time_point TestClock::current_time = TestClock::time_point{};

// “естова€ фикстура дл€ KVStorage
class KVStorageTest : public ::testing::Test {
protected:
    using Storage = VK::KVStorage<TestClock>;

    void SetUp() override {
        // —бросим врем€ перед каждым тестом
        TestClock::current_time = TestClock::time_point{};
    }

    std::vector<std::tuple<std::string, std::string, uint32_t>> empty_entries;
};

TEST_F(KVStorageTest, SetGetWithoutTTL) {
    Storage storage(empty_entries);

    storage.set("key1", "value1", 0);
    auto val = storage.get("key1");

    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), "value1");
}

TEST_F(KVStorageTest, SetGetWithTTL_NotExpired) {
    Storage storage(empty_entries);

    storage.set("key2", "value2", 10);
    auto val = storage.get("key2");

    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), "value2");
}

TEST_F(KVStorageTest, SetUpdatesExistingKey) {
    Storage storage(empty_entries);

    storage.set("key1", "value1", 0);
    auto val1 = storage.get("key1");
    ASSERT_TRUE(val1.has_value());
    EXPECT_EQ(val1.value(), "value1");

    // ”станавливаем новое значение дл€ уже существующего ключа "key1"
    storage.set("key1", "value2", 0);
    auto val2 = storage.get("key1");
    ASSERT_TRUE(val2.has_value());
    EXPECT_EQ(val2.value(), "value2");

    // ƒополнительно можно проверить, что нет лишних записей (если есть метод)
    auto all = storage.getManySorted("", 10);
    EXPECT_EQ(all.size(), 1);
    EXPECT_EQ(all[0].first, "key1");
    EXPECT_EQ(all[0].second, "value2");
}

TEST_F(KVStorageTest, GetReturnsNullopt_WhenKeyMissing) {
    Storage storage(empty_entries);

    auto val = storage.get("missing_key");

    EXPECT_FALSE(val.has_value());
}

TEST_F(KVStorageTest, KeyExpiresAfterTTL) {
    Storage storage(empty_entries);

    storage.set("key3", "value3", 5);
    TestClock::advance(std::chrono::seconds(6));

    auto val = storage.get("key3");
    EXPECT_FALSE(val.has_value());
}

TEST_F(KVStorageTest, RemoveReturnsTrueIfRemoved) {
    Storage storage(empty_entries);

    storage.set("key4", "value4", 0);
    bool removed = storage.remove("key4");

    EXPECT_TRUE(removed);
    EXPECT_FALSE(storage.get("key4").has_value());
}

TEST_F(KVStorageTest, RemoveReturnsFalseIfKeyNotExists) {
    Storage storage(empty_entries);

    bool removed = storage.remove("not_exist");
    EXPECT_FALSE(removed);
}

TEST_F(KVStorageTest, GetManySorted_ReturnsCorrectElements) {
    Storage storage(empty_entries);

    storage.set("a", "val1", 0);
    storage.set("b", "val2", 0);
    storage.set("d", "val3", 0);
    storage.set("e", "val4", 0);

    auto res = storage.getManySorted("c", 2);

    ASSERT_EQ(res.size(), 2);
    EXPECT_EQ(res[0].first, "d");
    EXPECT_EQ(res[0].second, "val3");
    EXPECT_EQ(res[1].first, "e");
    EXPECT_EQ(res[1].second, "val4");
}

TEST_F(KVStorageTest, GetManySorted_SkipsExpired) {
    Storage storage(empty_entries);

    storage.set("a", "val1", 1);
    storage.set("b", "val2", 0);
    storage.set("c", "val3", 0);
    TestClock::advance(std::chrono::seconds(2));

    auto res = storage.getManySorted("a", 10);

    // "a" expired, so skipped
    ASSERT_EQ(res.size(), 2);
    EXPECT_EQ(res[0].first, "b");
    EXPECT_EQ(res[1].first, "c");
}

TEST_F(KVStorageTest, GetManySortedReturnsEmptyWhenStorageEmpty) {
    Storage storage(empty_entries);

    auto result = storage.getManySorted("", 10);

    EXPECT_TRUE(result.empty());
}


TEST_F(KVStorageTest, RemoveOneExpiredEntry_RemovesAndReturnsOne) {
    Storage storage(empty_entries);

    storage.set("key5", "value5", 5);
    storage.set("key6", "value6", 0);

    TestClock::advance(std::chrono::seconds(6));

    auto expired_entry = storage.removeOneExpiredEntry();
    ASSERT_TRUE(expired_entry.has_value());
    EXPECT_EQ(expired_entry->first, "key5");

    EXPECT_FALSE(storage.get("key5").has_value());
}

TEST_F(KVStorageTest, RemoveOneExpiredEntry_ReturnsNulloptIfNoneExpired) {
    Storage storage(empty_entries);

    storage.set("key7", "value7", 10);
    TestClock::advance(std::chrono::seconds(5));

    auto expired_entry = storage.removeOneExpiredEntry();
    EXPECT_FALSE(expired_entry.has_value());
}
