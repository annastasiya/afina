//Simple.h - интерфейс простого аллокатора
#ifndef AFINA_ALLOCATOR_SIMPLE_H
#define AFINA_ALLOCATOR_SIMPLE_H

#include <string>
#include <cstddef>
#include <afina/allocator/Pointer.h>
#include <cstring>

namespace Afina {
namespace Allocator {
// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed

class Pointer;

/**
 * Wraps given memory area and provides defagmentation allocator interface on
 * the top of it.
 *
 * Allocator instance doesn't take ownership of wrapped memmory and do not delete it
 * on destruction. So caller must take care of resource cleaup after allocator stop
 * being needs
 */
// TODO: Implements interface to allow usage as C++ allocators
class Simple {
    
public:
    Simple(void *base, const size_t size);


    Pointer alloc(size_t N);

    Pointer _allocate_first(size_t N);
    
    Pointer _allocate_last_item(size_t N);
    
    Pointer _allocate_free(size_t N);


    void realloc(Pointer &p, size_t N);


    void free(Pointer &p);


    void defrag();

    std::string dump() const;

private:
    point *free_points;
    point buff; // переменная для описания буфера first = 1 эл-т, last = посл
    point *root; // переменная - указ-ль на конец буфера (начало списка)
    point *last_node; // переменная - указ-ль на последний эл-т списка
};

} // namespace Allocator
} // namespace Afina
#endif // AFINA_ALLOCATOR_SIMPLE_H
