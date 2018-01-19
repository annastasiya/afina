#include <afina/allocator/Simple.h>
#include <afina/allocator/Pointer.h>
#include <afina/allocator/Error.h>

namespace Afina {
namespace Allocator {


Simple::Simple(void *base, size_t base_size)
{
    // конструктор
    buff.first = base;
    buff.last = static_cast<char *>(base) + base_size - sizeof(point);
    root = static_cast<point *>(buff.last);

    root->prev = nullptr;
    root->next  = nullptr;
    root->first = nullptr;
    root->last  = nullptr;
    last_node = root;
    free_points = nullptr;
}

Pointer Simple::_allocate_first(size_t N)
{
        //
        root->first = buff.first;
        root->last = static_cast<void *>(static_cast<char*>(buff.first) + N);
        return Pointer(root);
    
}

Pointer Simple::_allocate_last_item(size_t N)
{
    if (free_points != nullptr) 
    {
        point *new_free_point = free_points->prev;
        free_points->prev = root;
        free_points->first = root->last; 
        root->next = free_points;
        
        free_points->last = static_cast<void *>(static_cast<char *>(root->last) + N);
        root = free_points;
        free_points = new_free_point;
        root->next = nullptr;
    }
    else
    {
        // Случай, когда блоки еще не освобождались
        // Перемещаем last_node на одну позицию, записываем туда данные по новому блоку,
        // связываем его цепочечно с предыдущим, в конце перемещаем root на вновь выделенный блок
        last_node--;
        root->next = last_node;
        last_node->prev = root;
        last_node->first = root->last;
        
        last_node->last = static_cast<void*>(static_cast<char*>(root->last) + N);
        last_node->next = nullptr;
        root = last_node; 
    }
    return Pointer(root);
}

Pointer Simple::_allocate_free(size_t N)
{
    for(point *cur = static_cast<point *>(buff.last); cur->next != nullptr; cur = cur->next)
    {
        if((cur->next)->first != nullptr)
        {
            size_t sz = static_cast<char*>((cur->next)->first) - static_cast<char*>(cur->last);

            if(sz >= N)
            {
                if (free_points != nullptr)
                {
                    point *new_free_points = free_points->prev;
                    free_points->next = cur->next;
                    free_points->prev = cur;
                    free_points->first = cur->last;
                    
                    free_points->last = static_cast<void*>(static_cast<char*>(cur->last) + N);

                    (cur->next)->prev = free_points;
                    point *new_one = free_points;
                    free_points = new_free_points;
                    
                    return Pointer(new_one);
                }
                else
                {
                    last_node--;
                    last_node->prev = cur;
                    last_node->next = cur->next;
                    last_node->first = cur->last;
                    
                    last_node->last = static_cast<void*>(static_cast<char*>(cur->last) + N);
                    (cur->next)->prev = last_node;
                    cur->next = last_node;
                    
                    return Pointer(last_node);
                }
            }
        }
    }
    throw AllocError(AllocErrorType::NoMemory, "Try defraq");
}

Pointer Simple::alloc(size_t N)
{
    // Сперва проверяем, заполнена ли структура-описатель (struct point) первого блока
    // Если нет, то еще ничего не аллоцировано, и мы размещаем блок в начале и заполняем его описатель прямо в root
    if ((root->prev  == nullptr) && (root->next  == nullptr) && (root->first == nullptr) && (root->last  == nullptr) &&
        (N + sizeof(point) < static_cast<char*>(buff.last) - static_cast<char*>(buff.first)))

        return _allocate_first(N);

    // Если да, то придерживаемся стратегии выделения блоков плотно один за другим
    // Проверяем, хватит ли места между последним блоком (слева) и крайним описателем (справа),
    // чтобы разместить блок заданного размера и соответствующий ему указатель
    if (last_node > static_cast<void*>(static_cast<char*>(last_node->last) + N + sizeof(point)))

        return _allocate_last_item(N);
    // Если там память уже закончилась, то ищем свободные места в середине цепочки,
    // которые могли появиться после вызовов free
    return _allocate_free(N);
}

void Simple::defrag()
{
    int i = 0;
    for(point *cur = static_cast<point*>(buff.last)->next; cur != nullptr; cur = cur->next)
    {
        if(cur->first != nullptr)
        {
            if (cur->first > cur->prev->last)
            {
                size_t sz = static_cast<char*>(cur->last) - static_cast<char*>(cur->first);
                memcpy(cur->prev->last, cur->first, sz);
                cur->first = cur->prev->last;
                cur->last = static_cast<char*>(cur->first) + sz;
            }
        }
        else
        {
            cur->last = cur->prev->last;
        }
    }
}

void Simple::realloc(Pointer &p, size_t N)
{

        void *info_link = p.get();
        size_t size = p.get_size();
        free(p);
        p = alloc(N);
    
        memcpy(p.get(), info_link, size);
}
        void Simple::free(Pointer& p)
        {
            point *prev = nullptr;
            point *get = p.get_ptr();

            if (get != nullptr)
            {

                if (free_points == nullptr)
                {
                    // задаем первый описатель свободного блока
                    free_points = get;
                    prev = nullptr;
                }
                else
                {
                    // Присоединяем описатель освобождаемого блока к цепочке свободных,
                    // перемещаем указатель free_points на него                        
                    free_points->next = get;
                    prev = free_points;
                    free_points = free_points->next;
                }

                // Выдергиваем описатель освобожденного блока из цепочки описателей выделенных
                if (free_points->next != nullptr)
                {
                    if (free_points->prev != nullptr)
                    {
                        // из середины
                        point *next = free_points->next;
                        point *last = free_points->prev;
                        next->prev = last;
                        last->next = next;
                    }
                    else
                    {
                        // из начала
                        (free_points->next)->prev = nullptr;
                    }
                }
                else
                {
                    // из конца, когда блок не последний
                    if (free_points->prev != nullptr)
                    {
                        root = root->prev;
                        root->next = nullptr;
                    }
                    else
                    {
                        // случай последнего блока
                        root->next = nullptr;
                        root->last = nullptr;
                        root->first = nullptr;
                        root->last = nullptr;
                    }
                }

                if (root != free_points)
                {

                    free_points->first = nullptr;
                    free_points->last = nullptr;
                    free_points->prev = prev;
                    free_points->next = nullptr;
                }
                else
                {
                    // Если удаляется блок, помеченный как root, то он последний по счету,
                    // и его удаление не создаст "дырок", в таком случае отказываемся от добавления
                    // нового описателя блока в список свободных
                    free_points = prev;
                    if (free_points != nullptr)
                        free_points->next = nullptr;
                }
            }

            p = nullptr;
}


//std::string Simple::dump() const { return ""; }

} // namespace Allocator
} // namespace Afina
