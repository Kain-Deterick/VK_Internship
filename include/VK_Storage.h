#ifndef VK_STORAGE_H
#define VK_STORAGE_H

#include <string>
#include <span>
#include <cstdint>
#include <vector>
#include <optional>
#include <tuple>
#include <map>
#include <set>
#include <algorithm>
#include <chrono>

namespace VK
{
	template <typename Clock>

	class KVStorage
	{
	public:
		using TimePoint = typename Clock::time_point;
		
		explicit KVStorage(
			std::span < std::tuple<std::string /*key*/,
			std::string /*value*/,
			uint32_t /*ttl*/ >> entries, Clock clock_ = Clock());

		~KVStorage() = default;

		void set(std::string key, std::string value, uint32_t ttl);

		bool remove(std::string_view key);

		std::optional<std::string> get(std::string_view key) const;

		std::vector<std::pair<std::string, std::string>> getManySorted(std::string_view key, uint32_t count) const;

		std::optional<std::pair<std::string, std::string>> removeOneExpiredEntry();

	private:
		struct Entry {
			std::string value;    // 24 bytes
			TimePoint expires_at; // 8 bytes
			bool has_ttl;         // 1 bytes + 7 padding
			typename std::multimap<TimePoint, std::string>::iterator ttl_it; //  16 bytes 
		};
		// Result: 56 bytes

		Clock clock_;
		// Каждый элемент std::map хранится в узле 
		// красно-чёрного дерева со служебными полями 
		// (3 указателя: на левый, правый, родитель, цвет) - 24 байта.
		// string (Key) - 24 bytes
		// Entry - 56 bytes
		// Result: 104 bites
		std::map<std::string, Entry> storage_; 
		
		// Result: 8 + 24 + 24 = 56 bites
		std::multimap<TimePoint, std::string> ttl_index_;
		// Total: 104 + 56 = 160 bytes
	};
}

#include <./VK_Storage.ipp>
#endif // !VK_STORAGE_H



