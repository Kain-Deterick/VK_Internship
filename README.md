VK Internship 

*Сложность операций*
| Операция | Временная сложность |
|:---: | :---: |
| set(key, value, ttl) | O(log N) |
| get(key) | O(log N) |
| remove(key) | O(log N) |
| getManySorted(key, count) | O(log N) |
| removeOneExpiredEntry() | O(log N) |

*Memory Overhead*
Оверхэд на одну запись равен 160 бит.

```
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
```

