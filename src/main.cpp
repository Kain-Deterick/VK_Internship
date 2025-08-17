#include <iostream>
#include "../include/VK_Storage.h"

using namespace std;

int main()
{
    setlocale(LC_ALL, "Russian");
    cout << sizeof(string) << endl;

    // Создаем хранилище с начальными данными
    std::vector<std::tuple<std::string, std::string, uint32_t>> vec = {
        {"key1", "val1", 0}, {"key2", "val2", 40} };

    VK::KVStorage<std::chrono::steady_clock> storage{
        std::span<std::tuple<std::string, std::string, uint32_t>>(vec) };

    // Тестируем операции
    std::cout << "---- Тестируем get ----" << std::endl;
    if (auto value = storage.get("key1")) {
        std::cout << "key1: " << *value << std::endl;
    }

    std::cout << "\n---- Тестируем set ----" << std::endl;
    storage.set("key3", "value3", 0);
    if (auto value = storage.get("key3")) {
        std::cout << "key3: " << *value << std::endl;
    }

    std::cout << "\n---- Тестируем getManySorted ----" << std::endl;
    auto sorted_results = storage.getManySorted("key2", 10);
    for (const auto& [key, value] : sorted_results) {
        std::cout << key << ": " << value << std::endl;
    }

	return 0;
}