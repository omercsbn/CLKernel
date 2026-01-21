# CLKernel Development Roadmap

## 📋 Architecture Summary

### Current Kernel Architecture

CLKernel is designed as a hybrid kernel combining microkernel modularity with monolithic performance characteristics. The kernel targets x86 32-bit protected mode with plans for x86-64 and ARM64 support.

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           CLKernel Architecture                              │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    User Space (Planned)                              │   │
│  │    Applications    │    Shell    │    Services    │    Modules      │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                    ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    System Call Interface (Planned)                   │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                    ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │              Kernel Services Layer                                   │   │
│  │  ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌───────────────────────┐│   │
│  │  │    VFS    │ │  Network  │ │   Actor   │ │    AI Supervisor     ││   │
│  │  │ (Planned) │ │ (Planned) │ │   IPC ✓   │ │        ✓             ││   │
│  │  └───────────┘ └───────────┘ └───────────┘ └───────────────────────┘│   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                    ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │              Core Kernel Layer                                       │   │
│  │  ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌───────────────────────┐│   │
│  │  │ Scheduler │ │  Memory   │ │  Module   │ │       IDT/PIC        ││   │
│  │  │     ✓     │ │    ✓      │ │  System ✓ │ │         ✓            ││   │
│  │  └───────────┘ └───────────┘ └───────────┘ └───────────────────────┘│   │
│  │  ┌───────────┐ ┌───────────┐ ┌───────────┐                          │   │
│  │  │   Heap    │ │  Paging   │ │    GDT    │                          │   │
│  │  │     ✓     │ │    ✓      │ │     ✓     │                          │   │
│  │  └───────────┘ └───────────┘ └───────────┘                          │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                    ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │              Hardware Abstraction Layer                              │   │
│  │  ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌───────────────────────┐│   │
│  │  │    VGA    │ │  Keyboard │ │   Timer   │ │    I/O Ports         ││   │
│  │  │     ✓     │ │     ✓     │ │     ✓     │ │        ✓             ││   │
│  │  └───────────┘ └───────────┘ └───────────┘ └───────────────────────┘│   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                    ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │              Bootloader (Assembly)                                   │   │
│  │  512-byte MBR bootloader with protected mode switch                 │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘

Legend: ✓ = Implemented (basic/partial)   (Planned) = Not yet implemented
```

### Subsystem Details

#### 1. Bootloader (`boot/boot.asm`)
- **Status**: ✅ Implemented
- **Features**:
  - 512-byte MBR bootloader
  - Real mode to protected mode switch
  - GDT setup for protected mode
  - Kernel loading from disk (sectors 2-9)
  - Basic error handling

#### 2. Kernel Core (`kernel/core/`)
- **Status**: ✅ Implemented (Basic)
- **Components**:
  - `kernel_main.c` - Main entry point, initialization sequence
  - `kernel_entry.asm` - Assembly bridge from bootloader
  - `gdt.c` - Global Descriptor Table setup
  - `idt.c` - Interrupt Descriptor Table with handlers
  - `pic.c` - 8259 PIC initialization and IRQ management

#### 3. VGA Display Driver (`kernel/core/vga.c`)
- **Status**: ✅ Implemented
- **Features**:
  - VGA text mode (80x25)
  - Color support
  - Scrolling
  - Cursor control
  - printf-style formatting

#### 4. Memory Management
- **Physical Memory** (`memory.c`): ✅ Basic implementation
  - Memory region detection (simplified)
  - Buddy allocator structure (partially implemented)
  - Actor memory isolation (framework)
- **Virtual Memory** (`paging.c`): ✅ Basic implementation
  - Page directory/table setup
  - Identity mapping for kernel (first 4MB)
  - Page mapping/unmapping
- **Heap Allocator** (`heap.c`): ✅ Basic implementation
  - Simple bump allocator
  - Slab allocator framework
  - Actor-specific allocation tracking

#### 5. Scheduler (`scheduler.c`)
- **Status**: ✅ Implemented (Basic)
- **Features**:
  - Actor-based scheduling model
  - Actor creation/termination
  - Message passing (async)
  - Cooperative multitasking framework
  - AI supervision hooks

#### 6. AI Supervisor (`ai_supervisor.c`)
- **Status**: ✅ Implemented (Framework)
- **Features**:
  - Behavior pattern tracking
  - Anomaly detection (memory leaks, CPU spikes)
  - Intervention system (throttle, suspend, quarantine)
  - Statistics collection
- **Limitations**: Actual ML inference not implemented

#### 7. Module System (`modules.c`)
- **Status**: ⚠️ Stub Implementation
- **Framework exists but modules not loadable yet

---

## 🔍 Gap Analysis

### Critical Missing Components

| Component | Status | Priority | Complexity | Description |
|-----------|--------|----------|------------|-------------|
| **Proper IDT Exception Handling** | ⚠️ Partial | P0 | Medium | ISR stubs exist but crash on most exceptions |
| **E820 Memory Detection** | ❌ Missing | P0 | Low | Currently assumes 32MB, needs BIOS memory map |
| **Complete Heap Implementation** | ⚠️ Partial | P0 | Medium | Current bump allocator doesn't support free() |
| **System Call Interface** | ❌ Missing | P1 | High | Required for user space |
| **Process/Thread Management** | ⚠️ Partial | P1 | High | Actors exist but no true processes |
| **Virtual Filesystem (VFS)** | ❌ Missing | P1 | Very High | Essential for file operations |
| **Keyboard Driver** | ⚠️ Partial | P2 | Low | Only reads scancodes, no translation |
| **Network Stack** | ❌ Missing | P2 | Very High | Required for networking |
| **User Space Support** | ❌ Missing | P1 | High | Ring 3 execution, TSS, user memory |

### Detailed Gap Analysis

#### 1. IDT & Interrupt Handling

**Current State**: ISR assembly stubs exist, handlers implemented for exceptions 0-19 and IRQs 0-15.

**Why Required**: Without proper exception handling, any CPU exception (divide by zero, page fault, etc.) will crash or hang the system. Interrupts are essential for:
- Timer-based preemptive scheduling
- Keyboard/device input
- Hardware event notification

**Design Alternatives**:

| Approach | Pros | Cons |
|----------|------|------|
| Monolithic handlers | Simple, fast | Less modular |
| Message-based (current) | Fits async model | Overhead for time-critical |
| Hybrid | Flexible | Complex implementation |

**Pseudocode for Proper Exception Handler**:
```c
void exception_handler(interrupt_frame_t* frame) {
    // 1. Disable further interrupts
    cli();
    
    // 2. Save complete CPU state
    save_context(frame);
    
    // 3. Identify exception type
    switch (frame->interrupt_number) {
        case EXCEPTION_PAGE_FAULT:
            // Try to handle page fault
            if (handle_page_fault(get_cr2(), frame->error_code)) {
                return; // Handled successfully
            }
            break;
        case EXCEPTION_DIVIDE_ERROR:
            // Signal current process
            signal_process(current_process, SIGFPE);
            return;
    }
    
    // 4. If unrecoverable, panic
    kernel_panic("Unhandled exception", frame);
}
```

#### 2. Memory Management (Paging, Heap)

**Current State**: Basic paging with identity mapping; bump allocator for heap.

**Why Required**:
- **Memory Protection**: Without proper paging, processes can access any memory
- **Virtual Memory**: Enables larger address spaces than physical RAM
- **Process Isolation**: Each process needs its own address space
- **Dynamic Allocation**: Heap required for kernel data structures

**Design Alternatives**:

| Technique | Use Case | Complexity |
|-----------|----------|------------|
| **Paging (4KB pages)** | Standard, good granularity | Medium |
| **Segmentation only** | Legacy x86, less isolation | Low |
| **Huge pages (2MB/1GB)** | Performance for large allocations | Medium |
| **Demand paging** | Memory efficiency | High |

**Pseudocode for Page Fault Handler (Demand Paging)**:
```c
bool handle_page_fault(uint32_t fault_addr, uint32_t error_code) {
    // 1. Check if address is in valid VMA
    vma_t* vma = find_vma(current_process, fault_addr);
    if (!vma) {
        return false; // Invalid access - segfault
    }
    
    // 2. Allocate physical page
    page_t* page = alloc_page();
    if (!page) {
        return false; // Out of memory
    }
    
    // 3. Map the page
    uint32_t flags = PAGE_PRESENT;
    if (vma->flags & VMA_WRITE) flags |= PAGE_WRITABLE;
    if (vma->flags & VMA_USER) flags |= PAGE_USER;
    
    map_page(fault_addr, page_to_phys(page), flags);
    
    // 4. Zero the page if anonymous
    if (vma->type == VMA_ANONYMOUS) {
        memset((void*)PAGE_ALIGN(fault_addr), 0, PAGE_SIZE);
    }
    
    return true;
}
```

#### 3. Process Scheduler

**Current State**: Actor-based cooperative scheduler with basic round-robin.

**Why Required**:
- Fair CPU time distribution
- Priority-based execution
- Real-time task support
- Resource management

**Design Alternatives**:

| Scheduler Type | Pros | Cons |
|----------------|------|------|
| **Round-Robin** | Simple, fair | No priorities |
| **Priority-based** | Important tasks first | Starvation risk |
| **Completely Fair (CFS)** | Linux approach, balanced | Complex |
| **Cooperative** | Simpler, fits actors | Buggy actor can freeze |
| **Preemptive** | Robust | Context switch overhead |

**Pseudocode for Priority Scheduler**:
```c
struct scheduler {
    priority_queue_t ready_queue[MAX_PRIORITY];
    actor_t* current;
    uint32_t time_slice;
};

actor_t* schedule_next(void) {
    // Find highest priority non-empty queue
    for (int p = PRIORITY_MAX; p >= PRIORITY_MIN; p--) {
        if (!queue_empty(&scheduler.ready_queue[p])) {
            actor_t* next = queue_dequeue(&scheduler.ready_queue[p]);
            
            // Calculate time slice based on priority
            scheduler.time_slice = BASE_TIMESLICE * (p + 1);
            
            return next;
        }
    }
    return idle_actor; // Run idle if nothing else
}

void timer_interrupt_handler(void) {
    scheduler.time_slice--;
    
    if (scheduler.time_slice == 0) {
        // Preempt current actor
        current->state = ACTOR_READY;
        enqueue(&ready_queue[current->priority], current);
        
        // Switch to next
        actor_t* next = schedule_next();
        context_switch(current, next);
    }
}
```

#### 4. Actor-Based IPC System

**Current State**: Message queue structure defined, send/receive implemented.

**Why Required**:
- Inter-process/actor communication
- Event-driven architecture support
- Synchronization primitives

**Design Alternatives**:

| IPC Method | Latency | Throughput | Complexity |
|------------|---------|------------|------------|
| **Message Queues** | Medium | Medium | Low |
| **Shared Memory** | Very Low | Very High | Medium |
| **Pipes** | Low | High | Low |
| **Signals** | Very Low | Low | Low |
| **Actor Mailboxes** | Medium | Medium | Medium |

**Pseudocode for Lock-Free Message Queue**:
```c
typedef struct message_queue {
    message_t* head;
    message_t* tail;
    _Atomic uint32_t size;
} message_queue_t;

bool message_enqueue(message_queue_t* queue, message_t* msg) {
    msg->next = NULL;
    
    message_t* old_tail;
    do {
        old_tail = atomic_load(&queue->tail);
    } while (!atomic_compare_exchange_weak(&queue->tail, &old_tail, msg));
    
    if (old_tail) {
        old_tail->next = msg;
    } else {
        atomic_store(&queue->head, msg);
    }
    
    atomic_fetch_add(&queue->size, 1);
    return true;
}

message_t* message_dequeue(message_queue_t* queue) {
    message_t* head;
    do {
        head = atomic_load(&queue->head);
        if (!head) return NULL;
    } while (!atomic_compare_exchange_weak(&queue->head, &head, head->next));
    
    atomic_fetch_sub(&queue->size, 1);
    return head;
}
```

#### 5. Virtual Filesystem (VFS)

**Current State**: ❌ Not implemented

**Why Required**:
- Unified interface to different filesystems
- Device abstraction
- File operations for processes
- Configuration storage

**Design Alternatives**:

| VFS Design | Complexity | Flexibility |
|------------|------------|-------------|
| **Unix-like VFS** | High | Very High |
| **Simple flat namespace** | Low | Low |
| **Plan 9 style (everything is a file)** | High | Very High |

**Pseudocode for VFS Layer**:
```c
typedef struct vfs_operations {
    int (*open)(struct inode*, struct file*);
    int (*close)(struct file*);
    ssize_t (*read)(struct file*, char*, size_t, off_t*);
    ssize_t (*write)(struct file*, const char*, size_t, off_t*);
    int (*readdir)(struct file*, struct dirent*);
} vfs_ops_t;

typedef struct filesystem {
    char name[32];
    vfs_ops_t* ops;
    struct superblock* (*mount)(const char* device);
} filesystem_t;

// Register filesystem
void register_filesystem(filesystem_t* fs) {
    list_add(&registered_filesystems, fs);
}

// Mount filesystem
int vfs_mount(const char* device, const char* mountpoint, const char* fstype) {
    filesystem_t* fs = find_filesystem(fstype);
    if (!fs) return -ENODEV;
    
    struct superblock* sb = fs->mount(device);
    if (!sb) return -EIO;
    
    mount_entry_t* mnt = kmalloc(sizeof(mount_entry_t));
    mnt->mountpoint = strdup(mountpoint);
    mnt->superblock = sb;
    mnt->fs = fs;
    
    list_add(&mount_table, mnt);
    return 0;
}
```

#### 6. Network Stack

**Current State**: ❌ Not implemented

**Why Required**:
- Internet connectivity
- Network services
- Modern OS functionality

**Implementation Recommendation**: Start with lwIP (lightweight IP) or implement minimal UDP stack.

**Pseudocode for Simple Packet Reception**:
```c
// Network interface structure
typedef struct net_interface {
    uint8_t mac_address[6];
    uint32_t ip_address;
    uint32_t netmask;
    uint32_t gateway;
    
    int (*send_packet)(void* data, size_t len);
    int (*receive_packet)(void* buffer, size_t max_len);
} net_if_t;

// Ethernet frame handling
void handle_ethernet_frame(void* frame, size_t len) {
    eth_header_t* eth = (eth_header_t*)frame;
    
    switch (ntohs(eth->ethertype)) {
        case ETHERTYPE_ARP:
            handle_arp(frame + sizeof(eth_header_t));
            break;
        case ETHERTYPE_IP:
            handle_ip(frame + sizeof(eth_header_t));
            break;
    }
}
```

#### 7. Device Drivers

**Current State**: VGA and keyboard (partial) implemented.

**Needed Drivers**:
- Storage: ATA/AHCI, NVMe
- Input: Full keyboard with scancode translation
- Display: VESA/VBE for graphics mode
- Serial: For debugging
- RTC: Real-time clock

#### 8. ARM64 Support

**Current State**: ❌ Not implemented

**Why Required**:
- Modern hardware support
- Mobile/embedded targets
- Architecture diversity

**Implementation Steps**:
1. Create ARM64 bootloader (UEFI or bare-metal)
2. Implement ARM exception vectors
3. Port memory management (different page table format)
4. Port interrupt controller (GIC instead of PIC)
5. Abstract platform-specific code

---

## 🤖 AI Supervisor Feasibility Analysis

### Current Implementation Review

The AI supervisor framework provides:
- Behavior pattern tracking with statistical analysis
- Anomaly detection algorithms (heuristic-based)
- Intervention mechanisms

### Where AI Can Be Effectively Used

| Application | Feasibility | Value | Implementation |
|-------------|-------------|-------|----------------|
| **Fault Prediction** | High | High | Pattern recognition on system metrics |
| **Scheduling Optimization** | Medium | High | Reinforcement learning for timeslice tuning |
| **Memory Leak Detection** | High | High | Statistical analysis of allocation patterns |
| **Resource Allocation** | Medium | Medium | Load balancing based on predictions |
| **Security Anomaly Detection** | Medium | Very High | Behavioral analysis for intrusion detection |

### Recommended Frameworks for Kernel AI

| Framework | Language | Size | Suitability |
|-----------|----------|------|-------------|
| **TensorFlow Lite Micro** | C++ | ~100KB | Good for microcontrollers |
| **CMSIS-NN** | C | ~50KB | ARM Cortex optimized |
| **uTensor** | C++ | ~30KB | Very lightweight |
| **Custom decision trees** | C | <10KB | Best for kernel space |
| **Rule-based systems** | C | <5KB | Simplest, most predictable |

### Recommended Integration Strategy

**Phase 1: Rule-Based System (Current)**
- Threshold-based anomaly detection
- Simple pattern matching
- No external dependencies

**Phase 2: Statistical Models**
- Moving averages for trend detection
- Standard deviation for anomaly scoring
- K-means clustering (simple implementation)

**Phase 3: Lightweight ML (Future)**
- Decision trees trained offline
- Run inference in kernel
- Export models as C arrays

**Example: Decision Tree for Anomaly Classification**
```c
// Generated from trained model
typedef struct decision_node {
    uint32_t feature_index;
    float threshold;
    int32_t left_child;  // Index or -class_id if leaf
    int32_t right_child;
} decision_node_t;

// Compact model representation
static const decision_node_t anomaly_model[] = {
    {0, 0.75f, 1, 2},      // Check memory_ratio
    {1, 0.90f, -0, 3},     // Check cpu_ratio
    {-1, 0, 0, 0},         // Anomaly class
    {2, 100.0f, -0, -1},   // Check message_rate
};

int classify_anomaly(float* features) {
    int node = 0;
    while (node >= 0) {
        decision_node_t* n = &anomaly_model[node];
        if (features[n->feature_index] <= n->threshold) {
            node = n->left_child;
        } else {
            node = n->right_child;
        }
    }
    return -node; // Return class ID
}
```

### AI Integration Considerations

| Concern | Mitigation |
|---------|------------|
| **Latency** | Run AI inference in background, not in interrupt handlers |
| **Memory** | Pre-allocate fixed-size buffers, no dynamic allocation in AI code |
| **Determinism** | Use integer/fixed-point math, avoid floating point in critical paths |
| **Reliability** | AI should advise, not directly control critical functions |
| **Security** | Validate all AI outputs before acting |

---

## 🎯 Prioritized Implementation Roadmap

### Phase 1: Foundation Stabilization (Weeks 1-4)

| Milestone | Tasks | Priority | Effort |
|-----------|-------|----------|--------|
| **M1.1** | Fix heap allocator (proper free list) | P0 | 3 days |
| **M1.2** | Implement E820 memory detection | P0 | 2 days |
| **M1.3** | Complete keyboard driver | P2 | 2 days |
| **M1.4** | Add comprehensive boot tests | P1 | 2 days |
| **M1.5** | Document current architecture | P2 | 1 day |

### Phase 2: Core Kernel Features (Weeks 5-12)

| Milestone | Tasks | Priority | Effort |
|-----------|-------|----------|--------|
| **M2.1** | Implement proper process model | P1 | 2 weeks |
| **M2.2** | Add preemptive scheduling | P1 | 1 week |
| **M2.3** | User space ring 3 support | P1 | 2 weeks |
| **M2.4** | System call interface | P1 | 1 week |
| **M2.5** | Complete actor IPC | P1 | 1 week |

### Phase 3: Storage & Filesystem (Weeks 13-20)

| Milestone | Tasks | Priority | Effort |
|-----------|-------|----------|--------|
| **M3.1** | ATA/IDE driver | P1 | 2 weeks |
| **M3.2** | VFS layer | P1 | 2 weeks |
| **M3.3** | RAM filesystem | P2 | 1 week |
| **M3.4** | FAT32 read support | P2 | 2 weeks |
| **M3.5** | Simple write support | P2 | 1 week |

### Phase 4: Networking (Weeks 21-28)

| Milestone | Tasks | Priority | Effort |
|-----------|-------|----------|--------|
| **M4.1** | NIC driver (e1000/virtio) | P2 | 2 weeks |
| **M4.2** | Ethernet layer | P2 | 1 week |
| **M4.3** | ARP implementation | P2 | 3 days |
| **M4.4** | IPv4 stack | P2 | 2 weeks |
| **M4.5** | UDP/TCP basics | P2 | 2 weeks |

### Phase 5: Advanced Features (Weeks 29-40)

| Milestone | Tasks | Priority | Effort |
|-----------|-------|----------|--------|
| **M5.1** | Enhanced AI supervisor | P3 | 3 weeks |
| **M5.2** | Hot module reloading | P3 | 2 weeks |
| **M5.3** | Shell with commands | P2 | 2 weeks |
| **M5.4** | Graphics mode support | P3 | 3 weeks |
| **M5.5** | ARM64 port | P3 | 8 weeks |

---

## 🧪 Test Strategy

### Unit Testing

**Framework Recommendation**: Custom lightweight framework or µnit

```c
// Example test structure
#define TEST(name) static void test_##name(void)
#define ASSERT_EQ(a, b) if ((a) != (b)) test_fail(__FILE__, __LINE__)

TEST(heap_alloc_and_free) {
    void* p1 = kmalloc(64);
    ASSERT_NE(p1, NULL);
    
    void* p2 = kmalloc(128);
    ASSERT_NE(p2, NULL);
    ASSERT_NE(p1, p2);
    
    kfree(p1);
    kfree(p2);
}
```

### Integration Testing with QEMU

```bash
#!/bin/bash
# Run kernel in QEMU with timeout and capture output

timeout 10 qemu-system-i386 \
    -drive file=build/clkernel.img,format=raw,if=floppy \
    -m 32M \
    -nographic \
    -serial stdio \
    2>&1 | tee test_output.log

# Check for expected output
grep -q "CLKernel initialization complete" test_output.log
```

### Suggested Test Categories

1. **Boot Tests**: Kernel starts, subsystems initialize
2. **Memory Tests**: Allocation, deallocation, no leaks
3. **Interrupt Tests**: Timer fires, keyboard responds
4. **Scheduler Tests**: Actor creation, message passing
5. **Stress Tests**: Many actors, memory pressure

---

## 🔒 Security Considerations

### Current Vulnerabilities

| Issue | Severity | Mitigation |
|-------|----------|------------|
| No ASLR | Medium | Randomize kernel/heap addresses |
| No stack canaries | High | Add `-fstack-protector` |
| No SMEP/SMAP | High | Enable CPU protections when available |
| Flat memory model | Critical | Implement proper paging with NX |

### Recommended Security Features

1. **Stack Protection**
   - Enable compiler stack canaries
   - Guard pages at stack boundaries

2. **Memory Protection**
   - NX (No-Execute) bit for data pages
   - W^X policy (write XOR execute)
   - ASLR for user space

3. **Capability-Based Isolation**
   - Actors have explicit capabilities
   - No ambient authority
   - Principle of least privilege

4. **Sandboxing Engine** (Planned)
   - WebAssembly-style isolation
   - Memory bounds checking
   - Syscall filtering

---

## 📊 Performance Considerations

### Actor Model vs Traditional IPC

| Metric | Actor Model | Traditional Threads |
|--------|-------------|---------------------|
| Context switch | Fast (no TLB flush if shared) | Slower (full TLB flush) |
| Communication | Message copy overhead | Shared memory faster |
| Scalability | Excellent | Good |
| Complexity | Higher | Lower |
| Debugging | Harder | Easier |

### Optimization Opportunities

1. **Message Passing**
   - Zero-copy for large messages (page remapping)
   - Small message optimization (inline payload)
   - Batched message delivery

2. **Scheduling**
   - Cache-aware actor placement
   - Work stealing between CPU cores
   - Priority inheritance for IPC

3. **Memory**
   - Page coloring for cache optimization
   - Huge pages for frequently accessed regions
   - Lazy allocation with demand paging

---

## 📝 Code Style Guidelines

See [CODE_STYLE.md](./CODE_STYLE.md) for detailed guidelines.

### Quick Summary

- **Language**: C99 with GCC extensions as needed
- **Indentation**: 4 spaces (no tabs)
- **Naming**: `snake_case` for functions/variables, `UPPER_CASE` for macros
- **Comments**: Doxygen style for public APIs
- **Headers**: Include guards with `#ifndef HEADER_H`

---

## 📚 References

- [OSDev Wiki](https://wiki.osdev.org/) - Comprehensive OS development resource
- [Intel Software Developer Manuals](https://www.intel.com/sdm) - x86 architecture reference
- [James Molloy's Kernel Tutorials](http://www.jamesmolloy.co.uk/tutorial_html/) - Classic x86 kernel tutorial
- [Writing a Simple Operating System from Scratch](https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf) - Nick Blundell's guide

---

*This roadmap is a living document and should be updated as development progresses.*
