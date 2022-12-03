#pragma once

#include <memory>
#include <shared_mutex>
#include <mutex>
#include <algorithm>
namespace kms 
{

template<typename T>
class protected_vector_t
{
public:
    void add(std::unique_ptr<T>&& item)
    {
        std::unique_lock wl(m_Mutex);
        m_Data.push_back(std::move(item));
    }

    void remove(T* item)
    {
        std::unique_lock wl(m_Mutex);
        const auto logicalEnd = std::remove_if(m_Data.begin(), m_Data.end(), [item](std::unique_ptr<T>& itemData)
        {
            return itemData.get() == item;
        });
        m_Data.erase(logicalEnd, m_Data.end());
    }

    template<typename PRED>
    void removeIf(PRED pred)
    {
        std::unique_lock wl(m_Mutex);
        std::erase_if(m_Data, pred);
    }

    std::vector<T*> get(std::vector<T*>& items) const
    {
        std::shared_lock rl(m_Mutex);
        items.reserve(m_Data.size());
        for(auto& item : m_Data)
        {
            items.push_back(item.get());
        }
        return items;
   }

   void clear()
   {
        std::unique_lock wl(m_Mutex);
        m_Data.clear();
   }
   

private:
    mutable std::shared_mutex m_Mutex;
    std::vector<std::unique_ptr<T>> m_Data;
};


template<typename T>
class base_protected_vector_t
{
 public:

    using READ_ONLY_VECTOR = std::vector<const T*>;


    void add(T&& item)
    {
        std::unique_lock wl(m_Mutex);
        m_Data.push_back(std::move(item));
    }

    auto remove(const T& item)
    {
        std::unique_lock wl(m_Mutex);
        std::erase_if(m_Data, [&item](auto& itemData) -> auto {
            return item == itemData;
        });
    }

    READ_ONLY_VECTOR& get(READ_ONLY_VECTOR& items) const
    {
        std::shared_lock rl(m_Mutex);
        items.reserve(m_Data.size());
        for(auto& item : m_Data)
        {
            items.push_back(&item);
        }
        return items;
   }

private:
    std::vector<T> m_Data;
    mutable std::shared_mutex m_Mutex;
};


}