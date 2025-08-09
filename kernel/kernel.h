/*
 * =============================================================================
 * CLKernel - Core Kernel Header
 * =============================================================================
 * File: kernel.h
 * Purpose: Main kernel header with core definitions and structures
 * =============================================================================
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// =============================================================================
// Kernel Configuration Constants
// =============================================================================

#define KERNEL_STACK_SIZE       0x4000      // 16KB kernel stack
#define MAX_MODULES             64          // Maximum loadable modules
#define MAX_ACTORS              256         // Maximum async actors
#define PAGE_SIZE               0x1000      // 4KB pages
#define KERNEL_HEAP_SIZE        0x100000    // 1MB initial heap

// =============================================================================
// Kernel Status Definitions
// =============================================================================

typedef enum {
    KERNEL_BOOTING = 0,
    KERNEL_READY,
    KERNEL_BUSY,
    KERNEL_PANIC,
    KERNEL_SHUTDOWN
} kernel_status_t;

// =============================================================================
// Core Kernel State Structure
// =============================================================================

typedef struct {
    kernel_status_t status;
    uint32_t boot_time;         // Boot timestamp
    uint32_t uptime;            // Seconds since boot
    uint32_t total_memory;      // Total system memory
    uint32_t free_memory;       // Available memory
    uint32_t loaded_modules;    // Number of loaded modules
    uint32_t active_actors;     // Number of active async actors
    bool ai_supervisor_active;  // AI supervisor status
} kernel_state_t;

// =============================================================================
// Module System Definitions
// =============================================================================

typedef enum {
    MODULE_TYPE_VFS = 1,
    MODULE_TYPE_DRIVER,
    MODULE_TYPE_NETWORK,
    MODULE_TYPE_AI,
    MODULE_TYPE_SCHEDULER,
    MODULE_TYPE_SECURITY
} module_type_t;

typedef struct {
    char name[32];
    module_type_t type;
    uint32_t version;
    bool loaded;
    void* entry_point;
    size_t size;
} kernel_module_t;

// =============================================================================
// Async Actor System Definitions (Forward Declaration)
// =============================================================================

// Forward declaration - actual definition in scheduler.h
struct actor_context;

// =============================================================================
// Memory Management Definitions
// =============================================================================

typedef struct {
    void* physical_start;
    void* virtual_start;
    size_t size;
    bool allocated;
} memory_block_t;

// =============================================================================
// Function Prototypes - Core Kernel
// =============================================================================

// Main kernel functions
void kernel_main(void);
void display_kernel_banner(void);
void load_core_modules(void);
void kernel_main_loop(void);
void kernel_panic(const char* message, const char* file, int line);
void cpu_yield(void);

// Utility macros
#define PANIC(msg) kernel_panic(msg, __FILE__, __LINE__)
#define ASSERT(condition) do { if (!(condition)) PANIC("Assertion failed: " #condition); } while(0)

// =============================================================================
// Function Prototypes - Subsystems (implemented in separate files)
// =============================================================================

// VGA display
void vga_clear_screen(void);
void vga_set_color(uint8_t color);
int kprintf(const char* format, ...);

// GDT management
void gdt_init(void);

// IDT management
void idt_init(void);
void idt_print_stats(void);

// PIC management  
void pic_init(void);
void pic_send_eoi(uint8_t irq);

// I/O functions
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t data);

// Memory management
void memory_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);

// Async scheduler
void scheduler_init(void);
void scheduler_process_pending(void);
// Actor functions are defined in scheduler.h
bool actor_send_message(uint32_t actor_id, void* message, size_t size);

// Module system
void modules_init(void);
bool load_module(const char* name);
void modules_periodic_check(void);

// AI supervisor
void ai_supervisor_init(void);
void ai_supervisor_check(void);
bool ai_supervisor_attempt_recovery(const char* error, const char* file, int line);

// Interrupt handling
void handle_pending_interrupts(void);

// Global kernel state
extern kernel_state_t kernel_state;

#endif // KERNEL_H
