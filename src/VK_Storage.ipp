
namespace VK
{
    template<typename Clock>
    KVStorage<Clock>::KVStorage(
        std::span<std::tuple<std::string, std::string, uint32_t>> entries,
        Clock clock)
        : clock_(std::move(clock))
    {
        for (auto& [key, value, ttl] : entries) {
            set(std::move(key), std::move(value), ttl);
        }
    }


    // ����������� �������:
    /*
    * 1. ����� �������� � storage_ (���������� �� �������) -> O(log N)
    * 2. ���� ������, �� �������  - O (1), ��� ��� ����������� �������� �� ������� ttl_it � ���������
    * 3. ����������/���������� ������ �������� � ������� ��������� � �������� ����������� ��������� O(1)
    * 4. �������/���������� ������ � ttl_index_ (������� � std::multimap):
            ���������: O(log M), ��� M � ����� ��������� � TTL.
      5. ���� ������ �����, �� ���������� � storage_ � ������� � std::map: ���������: O(log N).

      ����� � ������ ������ O(log(N^2))
    */
    template<typename Clock>
    void KVStorage<Clock>::set(std::string key, std::string value, uint32_t ttl) {
        auto now = clock_.now();

        // ��������� ���������� �� ������
        auto it = storage_.find(key);
        if (it != storage_.end()) {
            // ���� ���� TTL �����, ������� ������ (tll != 0)
            if (it->second.has_ttl) {
                ttl_index_.erase(it->second.ttl_it);
            }
            it->second.value = std::move(value);
            // ���� ttl == 0, �� ������� ������
            if (ttl == 0) {

                it->second.has_ttl = false;
                it->second.expires_at = TimePoint::min();
            }
            else {
                it->second.has_ttl = true;
                it->second.expires_at = now + std::chrono::seconds(ttl);
                auto iter = ttl_index_.insert({ it->second.expires_at, key });
                it->second.ttl_it = iter;
            }
        }
        else {
            // ������� ����� ������
            Entry entry;
            entry.value = std::move(value);
            if (ttl == 0) {
                entry.has_ttl = false;
                entry.expires_at = TimePoint::min();
            }
            else {
                entry.has_ttl = true;
                entry.expires_at = now + std::chrono::seconds(ttl);
                // Insert key into ttl_index_
                auto iter = ttl_index_.insert({ entry.expires_at, key });
                entry.ttl_it = iter;
            }
            storage_.emplace(std::move(key), std::move(entry));
        }
    }

    // ����������� ������� remove: O(log N)
    template<typename Clock>
    bool KVStorage<Clock>::remove(std::string_view key) {
        auto it = storage_.find(std::string(key));
        if (it == storage_.end()) {
            return false;
        }                  
        if (it->second.has_ttl) {
            ttl_index_.erase(it->second.ttl_it);
        }
        storage_.erase(it);
        return true;
    }

    // ����������� ������� get: O(log N)
    template<typename Clock>
    std::optional<std::string> KVStorage<Clock>::get(std::string_view key) const {

        auto it = storage_.find(std::string(key));
        if (it == storage_.end()) {
            return std::nullopt;
        }
        // ��������� TTL �� �������
        if (it->second.has_ttl && it->second.expires_at <= clock_.now()) {
            // ������� ������
            return std::nullopt;
        }
        return it->second.value;
    }


    // ����������� ������� getManySorted: O(log N+count)
    template<typename Clock>
    std::vector<std::pair<std::string, std::string>> KVStorage<Clock>::getManySorted(std::string_view key, uint32_t count)  const {
        std::vector<std::pair<std::string, std::string>> result;
        /*
        * lower_bound - ��� ����� ������ std::map � C++, ������� ���������� �������� �� ������ �������
        * � ����������, ���� �������� �� ������ ����������� �������� (�� ���� ���� �����, ���� ������)
        */
        auto it = storage_.lower_bound(std::string(key));
        auto now = clock_.now();
        while (it != storage_.end() && result.size() < count) {
            if (!it->second.has_ttl || it->second.expires_at > now) {
                result.emplace_back(it->first, it->second.value);
            }
            ++it;
        }
        return result;
    }


    template<typename Clock>
    std::optional<std::pair<std::string, std::string>> KVStorage<Clock>::removeOneExpiredEntry() {
        auto now = clock_.now();

        auto iter = ttl_index_.begin();
        while (iter != ttl_index_.end() && iter->first <= now) {
            auto key = iter->second;
            auto it = storage_.find(key);
            if (it != storage_.end()) {
                auto val = std::make_pair(it->first, it->second.value);
                ttl_index_.erase(iter);
                storage_.erase(it);
                return val;
            }
            ++iter;
        }
        return std::nullopt;
    }

    template<typename Clock>
    void VK::KVStorage<Clock>::removeExpiredEntriesUpTo(TimePoint now)
    {
        auto iter = ttl_index_.begin();
        while (iter != ttl_index_.end() && iter->first <= now) {
            const auto& key = iter->second;
            auto it = storage_.find(key);
            if (it != storage_.end()) {
                storage_.erase(it);
            }
            iter = ttl_index_.erase(iter);
        }
    }
}

