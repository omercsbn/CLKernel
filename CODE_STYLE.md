# CLKernel Code Style Guide

This document defines the coding standards and best practices for the CLKernel project.

## Table of Contents

1. [General Principles](#general-principles)
2. [C Language Standards](#c-language-standards)
3. [Formatting](#formatting)
4. [Naming Conventions](#naming-conventions)
5. [Comments and Documentation](#comments-and-documentation)
6. [Header Files](#header-files)
7. [Error Handling](#error-handling)
8. [Memory Management](#memory-management)
9. [Assembly Code](#assembly-code)
10. [Testing Guidelines](#testing-guidelines)

---

## General Principles

1. **Clarity over cleverness**: Write code that is easy to understand
2. **Minimal changes**: When modifying code, change only what's necessary
3. **Consistency**: Follow existing patterns in the codebase
4. **Safety first**: Always validate inputs and handle errors
5. **Document the "why"**: Code shows what, comments explain why

---

## C Language Standards

### Standard Version
- Use **C99** (ISO/IEC 9899:1999) as the base standard
- GCC extensions are permitted where they provide significant benefits:
  - `__attribute__((packed))` for hardware structures
  - `__attribute__((aligned(N)))` for alignment requirements
  - Inline assembly with proper constraints

### Required Compiler Flags
```makefile
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Wpedantic \
         -nostdlib -nostartfiles -nodefaultlibs \
         -fno-builtin -fno-stack-protector \
         -mno-red-zone -mno-mmx -mno-sse -mno-sse2
```

### Forbidden Constructs
- Variable-length arrays (VLAs)
- `goto` except for error cleanup patterns
- Floating-point operations in core kernel code
- Dynamic memory allocation in interrupt handlers

---

## Formatting

### Indentation
- Use **4 spaces** for indentation
- Never use tabs
- Configure your editor to convert tabs to spaces

### Line Length
- Maximum **100 characters** per line
- Break long lines at logical points

### Braces
- Use **Allman style** (braces on their own lines) for functions
- Use **K&R style** (opening brace on same line) for control structures

```c
// Function definition - Allman style
void function_name(int parameter)
{
    // Function body
}

// Control structures - K&R style
if (condition) {
    // Code
} else {
    // Code
}

for (int i = 0; i < count; i++) {
    // Code
}

while (running) {
    // Code
}
```

### Spacing

#### Around Operators
```c
// Good
int result = a + b * c;
if (x == y && z != 0) {
    value = (a < b) ? a : b;
}

// Bad
int result=a+b*c;
if(x==y&&z!=0){
    value=(a<b)?a:b;
}
```

#### Function Calls
```c
// Good
result = function_name(arg1, arg2, arg3);

// Bad
result = function_name (arg1,arg2,arg3);
```

#### Pointer Declarations
```c
// Good - asterisk with the variable name
char* string;
int* array;
void* ptr;

// When declaring multiple pointers (avoid if possible)
int *a, *b, *c;  // Each needs its own asterisk
```

### Blank Lines
- Two blank lines between functions
- One blank line between logical sections within a function
- No trailing whitespace

---

## Naming Conventions

### Variables
- Use `snake_case` for all variable names
- Use descriptive names

```c
// Good
uint32_t page_count;
char* buffer_start;
bool is_initialized;

// Bad
uint32_t pc;
char* bs;
bool init;
```

### Functions
- Use `snake_case` for function names
- Prefix with subsystem name for public functions

```c
// Good
void memory_init(void);
bool scheduler_add_actor(actor_t* actor);
uint32_t vga_get_cursor_position(void);

// Bad
void memInit(void);
bool AddActor(actor_t* actor);
uint32_t getcursorpos(void);
```

### Macros and Constants
- Use `UPPER_CASE` with underscores
- Prefix with subsystem name

```c
// Good
#define PAGE_SIZE           4096
#define MAX_ACTORS          256
#define IDT_FLAG_PRESENT    0x80
#define KERNEL_HEAP_START   0x200000

// Bad
#define pageSize            4096
#define maxActors           256
```

### Type Definitions
- Use `snake_case` with `_t` suffix for types
- Use descriptive names

```c
// Good
typedef struct {
    uint32_t actor_id;
    uint8_t state;
} actor_t;

typedef enum {
    ACTOR_STATE_READY,
    ACTOR_STATE_RUNNING,
    ACTOR_STATE_BLOCKED
} actor_state_t;

// Bad
typedef struct {
    uint32_t id;
    uint8_t s;
} ACTOR;
```

### File Names
- Use `snake_case` for C and header files
- Use descriptive names matching the module

```
Good:
  kernel_main.c
  memory_manager.c
  actor_scheduler.h

Bad:
  kernelMain.c
  memmgr.c
  ActorScheduler.h
```

---

## Comments and Documentation

### File Headers
Every source file should have a header comment:

```c
/*
 * =============================================================================
 * CLKernel - Module Name
 * =============================================================================
 * File: filename.c
 * Purpose: Brief description of what this file implements
 * 
 * Additional notes about the implementation if needed.
 * =============================================================================
 */
```

### Function Documentation
Use Doxygen-style comments for public functions:

```c
/**
 * @brief Allocate memory from the kernel heap
 * 
 * Allocates a contiguous block of memory of at least the specified size.
 * The returned memory is not initialized.
 * 
 * @param size The number of bytes to allocate
 * @return Pointer to allocated memory, or NULL if allocation failed
 * 
 * @note This function may block if memory is being reclaimed
 * @warning Do not call from interrupt handlers
 */
void* kmalloc(size_t size);
```

### Inline Comments
- Explain the "why", not the "what"
- Keep comments concise
- Update comments when code changes

```c
// Good - explains why
// Use atomic operation because this is called from interrupt handlers
atomic_increment(&interrupt_count);

// Bad - just restates the code
// Increment the interrupt count
interrupt_count++;
```

### TODO Comments
Mark incomplete or temporary code:

```c
// TODO: Implement proper memory coalescing
// TODO(username): Fix race condition in actor messaging
// FIXME: This leaks memory when actor terminates
```

---

## Header Files

### Include Guards
Use `#ifndef` style include guards:

```c
#ifndef MEMORY_H
#define MEMORY_H

// Header content

#endif // MEMORY_H
```

### Include Order
Order includes from most specific to most general:

```c
// 1. Related header (for .c files)
#include "memory.h"

// 2. Project headers
#include "kernel.h"
#include "vga.h"

// 3. System headers (freestanding environment)
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
```

### Header Content
Headers should contain only:
- Type definitions
- Function declarations
- Macro definitions
- Extern variable declarations

### Self-Contained Headers
Each header should be self-contained and include all dependencies:

```c
// memory.h
#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Now we can use uint32_t, bool, size_t
typedef struct {
    uint32_t base;
    size_t size;
    bool available;
} memory_region_t;

#endif // MEMORY_H
```

---

## Error Handling

### Return Values
- Return `bool` for success/failure operations
- Return `NULL` for failed pointer operations
- Return negative values or error codes for detailed errors

```c
// Simple success/failure
bool scheduler_add_actor(actor_t* actor);

// Pointer return - NULL on failure
void* kmalloc(size_t size);

// Error code return
typedef enum {
    ERR_SUCCESS = 0,
    ERR_NOMEM = -1,
    ERR_INVALID = -2,
    ERR_BUSY = -3
} error_t;

error_t file_open(const char* path, file_t** file);
```

### Error Checking
Always check return values:

```c
// Good
void* buffer = kmalloc(1024);
if (!buffer) {
    kprintf("[ERROR] Failed to allocate buffer\n");
    return false;
}

// Bad
void* buffer = kmalloc(1024);
// Using buffer without checking...
```

### Assertions
Use `ASSERT` for conditions that should never be false:

```c
void actor_send_message(actor_t* actor, message_t* msg) {
    ASSERT(actor != NULL);
    ASSERT(msg != NULL);
    ASSERT(actor->state != ACTOR_STATE_TERMINATED);
    
    // Implementation
}
```

---

## Memory Management

### Allocation Patterns
- Always check allocation results
- Free memory in reverse order of allocation
- Clear pointers after freeing

```c
bool initialize_resources(void) {
    resource_a = kmalloc(SIZE_A);
    if (!resource_a) goto fail_a;
    
    resource_b = kmalloc(SIZE_B);
    if (!resource_b) goto fail_b;
    
    resource_c = kmalloc(SIZE_C);
    if (!resource_c) goto fail_c;
    
    return true;
    
fail_c:
    kfree(resource_b);
fail_b:
    kfree(resource_a);
fail_a:
    return false;
}
```

### Memory Barriers
Use appropriate barriers for multi-core safety:

```c
// Full memory barrier
memory_barrier_full();

// Compiler barrier only
asm volatile("" ::: "memory");
```

---

## Assembly Code

### Inline Assembly
Use proper GCC extended inline assembly:

```c
static inline uint32_t read_cr3(void) {
    uint32_t value;
    asm volatile("mov %%cr3, %0" : "=r"(value));
    return value;
}

static inline void write_cr3(uint32_t value) {
    asm volatile("mov %0, %%cr3" :: "r"(value) : "memory");
}
```

### Assembly Files
- Use NASM syntax
- Comment thoroughly
- Use macros for repetitive code

```nasm
; =============================================================================
; Function: interrupt_handler
; Purpose: Common interrupt handler entry point
; =============================================================================

%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli                     ; Disable interrupts
    push 0                  ; Push dummy error code
    push %1                 ; Push interrupt number
    jmp isr_common_stub     ; Jump to common handler
%endmacro
```

---

## Testing Guidelines

### Test Function Naming
```c
void test_memory_alloc_basic(void);
void test_memory_alloc_zero_size(void);
void test_memory_free_null(void);
```

### Test Structure
```c
void test_actor_creation(void) {
    // Arrange
    uint32_t expected_id = 1;
    
    // Act
    uint32_t actor_id = actor_create(test_entry, NULL, PRIORITY_NORMAL, 0);
    
    // Assert
    ASSERT_NE(actor_id, 0);
    
    // Cleanup
    actor_terminate(actor_id);
}
```

### Test Coverage Goals
- All public functions should have tests
- Error paths should be tested
- Edge cases (NULL, zero, max values) should be tested

---

## Version Control

### Commit Messages
- Use imperative mood: "Add feature" not "Added feature"
- First line: 50 characters max, summary
- Body: Explain what and why (not how)

```
Add memory pool for actor message allocation

Messages were being allocated from the main heap, causing
fragmentation. This commit adds a dedicated memory pool
for messages, improving allocation speed and reducing
fragmentation.
```

### Branch Naming
```
feature/add-vfs-layer
bugfix/fix-page-fault-handling
refactor/cleanup-scheduler-code
```

---

*This style guide is a living document and may be updated as the project evolves.*
