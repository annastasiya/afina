#include "MapBasedGlobalLockImpl.h"
#include <iostream>

#include <mutex>

//using namespase std;

namespace Afina {
namespace Backend {
        
// See MapBasedGlobalLockImpl.h
    
std::mutex mtx;
    
bool MapBasedGlobalLockImpl::Put(const std::string &key, const std::string &val)
{
    // безусловно сохраняет пару ключ/значение
    mtx.lock();
    
    std::map<std::string, std::string>::iterator iter = this->map.find(key);
    
    if (iter == this->map.end())
    {
        if (this->map.size() < this->_max_size)
        {
            this->map.insert(std::make_pair(key, val));
            
        } else
        {
            this->map.erase(*age.begin());
            this->age.erase(this -> age.begin());
            this->map.insert(std::make_pair(key, val));
        }
        
    } else
    {
        (iter -> second) = val;
        this->age.erase(find(age.begin(), age.end(), key));
    }
    this -> age.push_back(key);
    
    mtx.unlock();
    
    return true;
}
    
bool MapBasedGlobalLockImpl::Get(const std::string &key, std::string &val) const
{
    // возвращает значение для ключа
    mtx.lock();
    
    std::map<std::string, std::string>::const_iterator iter = this->map.find(key);
    
    if (iter == this->map.end())
    {
        mtx.unlock();
        return false;
        
    } else
    {
        val = (iter->second);
        
        mtx.unlock();
        return true;
    }
}

bool MapBasedGlobalLockImpl::Set(const std::string &key, const std::string &val)
{
    // устанавливает новое значение для ключа. Работает только если ключ уже представлен в хранилище
    mtx.lock();
    
    std::map<std::string, std::string>::iterator iter = this->map.find(key);
    if (iter != this->map.end())
    {
        iter -> second = val;
        this-> age.erase(find(age.begin(), age.end(), key));
        this -> age.push_back(key);
    }else
    {
        mtx.unlock();
        return false;
    }
    mtx.unlock();
    return true;
}
    
bool MapBasedGlobalLockImpl::PutIfAbsent(const std::string &key, const std::string &val)
{
    // сохраняет пару только если в контейнере еще нет такого ключа
    mtx.lock();
    
    std::map<std::string, std::string>::iterator iter = this->map.find(key);
    
    if (iter == this->map.end())
    {
        if (this->map.size() < this->_max_size)
        {
            this->map.insert(std::make_pair(key, val));
        } else
        {
            this->map.erase(*age.begin());
            this->age.erase(this -> age.begin());
            this->map.insert(std::make_pair(key, val));
        }
        this -> age.push_back(key);
        
        mtx.unlock();
        return true;
    }
    mtx.unlock();
    return false;
}

bool MapBasedGlobalLockImpl::Delete(const std::string &key)
{
    // удаляет пару ключ/значение из хранилища
    mtx.lock();
    
    std::map<std::string, std::string>::iterator iter = this->map.find(key);
    
    if (iter == this->map.end())
    {
        mtx.unlock();
        return false;
    }
    this->map.erase(iter);
    this->age.erase(find(age.begin(), age.end(), key));
    
    mtx.unlock();
    return true;
}
    
} // namespace Backend
} // namespace Afina
