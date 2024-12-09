/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include <cstring>
#include <stdexcept>
#include <iostream>
#include "BLI_stack.h"

 /* Abstract Base Class for Stack */
class AbstractStack {
public:
    virtual ~AbstractStack() = default;

    // Push an element onto the stack
    virtual void push(const void* src) = 0;

    // Pop an element from the stack
    virtual void pop(void* dst) = 0;

    // Clear the stack
    virtual void clear() = 0;

    // Check if the stack is empty
    virtual bool isEmpty() const = 0;

    // Get the count of elements in the stack
    virtual size_t count() const = 0;
};

/* Concrete Implementation of Stack */
class BLI_Stack : public AbstractStack {
private:
    struct StackChunk {
        StackChunk* next;
        char* data;

        explicit StackChunk(size_t chunk_size) : next(nullptr), data(new char[chunk_size]) {}
        ~StackChunk() { delete[] data; }
    };

    StackChunk* chunk_curr;
    StackChunk* chunk_free;
    size_t chunk_index;
    size_t chunk_elem_max;
    size_t elem_size;

#ifdef USE_TOTELEM
    size_t elem_num;
#endif

public:
    BLI_Stack(size_t elem_size, size_t chunk_size);
    ~BLI_Stack() override;

    void push(const void* src) override;
    void pop(void* dst) override;
    void clear() override;
    bool isEmpty() const override;
    size_t count() const override;

private:
    void discard();
    StackChunk* allocateChunk();
};

/* Constructor */
BLI_Stack::BLI_Stack(size_t elem_size, size_t chunk_size)
    : chunk_curr(nullptr),
    chunk_free(nullptr),
    chunk_index(0),
    chunk_elem_max(chunk_size / elem_size),
    elem_size(elem_size)
#ifdef USE_TOTELEM
    , elem_num(0)
#endif
{}

/* Destructor */
BLI_Stack::~BLI_Stack() {
    clear();
    StackChunk* temp;
    while (chunk_free) {
        temp = chunk_free;
        chunk_free = chunk_free->next;
        delete temp;
    }
}

/* Push an element onto the stack */
void BLI_Stack::push(const void* src) {
    if (!chunk_curr || chunk_index == chunk_elem_max) {
        auto* new_chunk = new StackChunk(elem_size * chunk_elem_max);
        new_chunk->next = chunk_curr;
        chunk_curr = new_chunk;
        chunk_index = 0;
    }

    char* dest = chunk_curr->data + (chunk_index * elem_size);
    std::memcpy(dest, src, elem_size);
    chunk_index++;

#ifdef USE_TOTELEM
    elem_num++;
#endif
}

/* Pop an element from the stack */
void BLI_Stack::pop(void* dst) {
    if (chunk_curr == nullptr || chunk_index == 0) {
        throw std::runtime_error("Pop called on an empty stack");
    }

    chunk_index--;
    const char* src = chunk_curr->data + (chunk_index * elem_size);
    std::memcpy(dst, src, elem_size);

#ifdef USE_TOTELEM
    elem_num--;
#endif

    if (chunk_index == 0 && chunk_curr->next) {
        StackChunk* old_chunk = chunk_curr;
        chunk_curr = chunk_curr->next;
        old_chunk->next = chunk_free;
        chunk_free = old_chunk;
    }
}

/* Clear the stack */
void BLI_Stack::clear() {
    while (chunk_curr) {
        StackChunk* temp = chunk_curr;
        chunk_curr = chunk_curr->next;
        temp->next = chunk_free;
        chunk_free = temp;
    }
    chunk_index = 0;
#ifdef USE_TOTELEM
    elem_num = 0;
#endif
}

/* Check if the stack is empty */
bool BLI_Stack::isEmpty() const {
    return chunk_curr == nullptr;
}

/* Get the count of elements in the stack */
size_t BLI_Stack::count() const {
#ifdef USE_TOTELEM
    return elem_num;
#else
    throw std::logic_error("Counting not supported without USE_TOTELEM.");
#endif
}

