# CLKernel - Experimental OS Kernel

![CLKernel Logo](https://img.shields.io/badge/CLKernel-Experimental%20OS-blue?style=for-the-badge&logo=linux)
![Version](https://img.shields.io/badge/Version-0.1.0-yellow?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-In%20Development-yellow?style=for-the-badge)
![Architecture](https://img.shields.io/badge/Architecture-x86%2032--bit-orange?style=for-the-badge)

## Experimental Operating System Kernel with Async Actor Architecture

CLKernel is an **experimental research project** exploring modern operating system design concepts. It aims to implement four innovative technologies, though many features are still in development:

```
  ▄████████  ▄█        ▄█   ▄█▄    ▄████████    ▄████████ ███▄▄▄▄      ▄████████  ▄█       
 ███    ███ ███       ███ ▄███▀   ███    ███   ███    ███ ███▀▀▀██▄   ███    ███ ███       
 ███    █▀  ███       ███▐██▀     ███    █▀    ███    ███ ███   ███   ███    █▀  ███       
 ███        ███      ▄█████▀     ▄███▄▄▄      ▄███▄▄▄▄██▀ ███   ███  ▄███▄▄▄     ███       
 ███        ███     ▀▀█████▄    ▀▀███▀▀▀     ▀▀███▀▀▀▀▀   ███   ███ ▀▀███▀▀▀     ███       
 ███    █▄  ███       ███▐██▄     ███    █▄  ▀███████████ ███   ███   ███    █▄  ███       
 ███    ███ ███▌    ▄ ███ ▀███▄   ███    ███   ███    ███ ███   ███   ███    ███ ███▌    ▄ 
 ████████▀  █████▄▄██ ███   ▀█▀   ██████████   ███    ███  ▀█   █▀    ██████████ █████▄▄██ 
            ▀         ▀               ▀        ███    ███                        ▀         

                 EXPERIMENTAL OPERATING SYSTEM KERNEL v0.1.0
```

---

## Design Goals & Features

> **Note**: Features marked with ⚠️ are partially implemented or in framework stage.
> Features marked with ❌ are planned but not yet implemented.

### 1. AI Supervisor System ⚠️
**Kernel-level behavioral monitoring with rule-based anomaly detection**
- ⚠️ Behavior pattern tracking and statistical analysis
- ⚠️ Anomaly detection (memory leaks, CPU spikes)
- ⚠️ Intervention framework (throttle, suspend, quarantine)
- ❌ Machine learning inference (framework only, no ML model yet)

### 2. Hot-Swappable Module System ⚠️
**Runtime modularity framework (partial implementation)**
- ⚠️ Module infrastructure and API design
- ❌ Dynamic module loading/unloading (stub implementation)
- ❌ Zero-downtime subsystem replacement
- ❌ Module dependency management

### 3. Sandboxing Engine ❌
**Planned capability-based isolation (not yet implemented)**
- ❌ WASM-like process isolation
- ❌ Capability-based permission system
- ❌ Memory protection and resource limiting

### 4. Actor-Based Scheduler ⚠️
**Cooperative multitasking with message passing**
- ✅ Actor creation and termination
- ⚠️ Asynchronous message passing
- ⚠️ Basic cooperative scheduling
- ❌ Interactive shell (not implemented)
- Scriptable automation and workflow management
├─────────────────────────────────────────────────────────────┤
│              Natural Language CLI Layer                     │
├─────────────────────────────────────────────────────────────┤
│                   System Call Interface                     │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────────┐ │
│  │   VFS       │ │  Net Stack  │ │     Actor IPC System    │ │
│  │  (Rust)     │ │   (Rust)    │ │        (Rust)           │ │
│  └─────────────┘ └─────────────┘ └─────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────────────────────┐ │
│  │           AI Supervisor & Fault Recovery               │ │
│  │                 (Python/Zig)                           │ │
│  └─────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────────┐ │
│  │  Scheduler  │ │   Memory    │ │     Module System       │ │
│  │    (C)      │ │    (C)      │ │         (C)             │ │
│  └─────────────┘ └─────────────┘ └─────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                    Kernel Core (C)                          │
├─────────────────────────────────────────────────────────────┤
│                 Hardware Abstraction                        │
├─────────────────────────────────────────────────────────────┤
│                Bootloader (Assembly)                        │
└─────────────────────────────────────────────────────────────┘
```

## Building CLKernel

### Prerequisites

**Windows (recommended for development):**
- GCC cross-compiler (i686-elf-gcc)
- NASM assembler
- GNU Make
- QEMU (for testing)
- Git

**Installation commands (using package manager):**
```powershell
# Install using Chocolatey (Windows)
choco install mingw nasm qemu git

# Or using MSYS2
pacman -S mingw-w64-i686-gcc nasm qemu git make
```

### Quick Start

```bash
# Clone and build
git clone <repository-url>
cd CLKernel

# Build everything
make all

# Run in QEMU
make run

# Debug with GDB
make debug
```

### Build Targets

| Target | Description |
|--------|-------------|
| `all` | Build bootloader, kernel, and create ISO |
| `bootloader` | Build just the 512-byte MBR bootloader |
| `kernel` | Build the C kernel |
| `modules` | Build loadable kernel modules |
| `iso` | Create bootable ISO image |
| `run` | Launch kernel in QEMU |
| `debug` | Launch with GDB debugging support |
| `clean` | Remove all build artifacts |

## 📁 Project Structure

```
CLKernel/
├── boot/                   # Bootloader code
│   └── boot.asm           # 512-byte MBR bootloader
├── kernel/                # Kernel source code
│   ├── core/              # Core kernel components
│   │   ├── kernel_main.c  # Main kernel entry point
│   │   ├── kernel_entry.asm # Assembly->C bridge
│   │   ├── vga.c          # VGA display driver
│   │   ├── gdt.c          # Global Descriptor Table
│   │   └── stubs.c        # Temporary implementations
│   ├── modules/           # Loadable kernel modules
│   ├── ai/                # AI supervisor components
│   ├── kernel.h           # Main kernel header
│   ├── vga.h              # VGA driver header
│   └── gdt.h              # GDT header
├── drivers/               # Device drivers
├── tools/                 # Development tools
├── build/                 # Build output directory
├── Makefile              # Build system
├── kernel.ld             # Linker script
└── README.md             # This file
```

## 🚀 Current Status

### ✅ Implemented (Basic/Working)
- [x] MBR bootloader (512 bytes, switches to protected mode)
- [x] Kernel entry point and C bridge
- [x] VGA text mode display with printf support
- [x] GDT setup for protected mode
- [x] IDT with interrupt/exception handlers
- [x] PIC initialization (8259)
- [x] Build system with Makefile
- [x] QEMU testing infrastructure

### ⚠️ Partially Implemented
- [ ] Memory management (basic paging, bump allocator - needs improvement)
- [ ] Heap allocator (no proper free() support)
- [ ] Actor-based scheduler (framework exists, needs context switching)
- [ ] AI supervisor (rule-based anomaly detection, no ML inference)
- [ ] Module system (API defined, loading not implemented)
- [ ] Keyboard driver (reads scancodes, no translation)

### ❌ Planned Features (Not Yet Started)
- [ ] Complete process model with user space
- [ ] System call interface
- [ ] Virtual filesystem (VFS)
- [ ] Network stack
- [ ] Live kernel patching
- [ ] ARM64 support
- [ ] Interactive shell
- [ ] Proper E820 memory detection

## 🧪 Testing

### Running in QEMU

```bash
# Standard run
make run

# With debugging
make debug
# Then in another terminal:
# gdb build/kernel.elf
# (gdb) target remote :1234
# (gdb) continue
```

### Expected Output

When you run `make run`, you should see:
```
================================================================================
  _____ _      _  __                      _ 
 / ____| |    | |/ /                     | |
| |    | |    | ' / ___ _ __ _ __   ___  | |
| |    | |    |  < / _ \ '__| '_ \ / _ \ | |
| |____| |____| . \  __/ |  | | | |  __/ | |
 \_____|______|_|\_\___|_|  |_| |_|\___| |_|

CLKernel v0.1.0 - Next-Generation Operating System
Built: [Date] [Time]
Architecture: Hybrid Kernel with Async Actors
Target: x86_64 (with future ARM64 support)
================================================================================

[BOOT] Initializing CLKernel v0.1.0
[BOOT] Setting up GDT... OK
[BOOT] Setting up IDT... OK
[BOOT] Initializing memory management... OK
[BOOT] Initializing async scheduler... OK
[BOOT] Initializing module system... OK
[BOOT] Loading core modules...
  -> Loading VFS module... OK
  -> Loading device manager... OK
  -> Loading network stack... OK
  -> Loading actor IPC system... OK
[BOOT] Initializing AI supervisor... OK

[BOOT] CLKernel initialization complete!
[BOOT] Kernel is running in hybrid mode with async actors
[BOOT] AI supervisor is monitoring system health

[KERNEL] Entering main event loop...
[KERNEL] Ready for async actor messages
[HEARTBEAT] Kernel alive - uptime: 0 seconds
```

## 🔍 Memory Layout

| Address Range | Description |
|---------------|-------------|
| 0x00007C00 - 0x00007DFF | Bootloader (512 bytes) |
| 0x00008000 - 0x00008FFF | Kernel loading area |
| 0x00009000 - 0x00009FFF | Stack area |
| 0x00100000 - 0x001FFFFF | Kernel code and data |
| 0x00200000 - 0x003FFFFF | Kernel heap |
| 0x00400000 - 0x007FFFFF | Module loading area |
| 0x00800000+ | Available for applications |

## 🛠️ Development Workflow

1. **Edit source code** in `kernel/` or `boot/`
2. **Build with** `make all`
3. **Test in QEMU** with `make run`
4. **Debug issues** with `make debug` + GDB
5. **Clean build** with `make clean` when needed

## 📚 Key Design Principles

### 1. Async-First Architecture
Unlike traditional kernels that rely on threads and blocking operations, CLKernel is built around async primitives:
- All I/O operations are non-blocking
- Actor-based message passing for IPC
- Event-driven processing throughout

### 2. Hot-Swappable Modularity
The kernel is designed for live updates:
- Modules can be replaced without rebooting
- Dependency tracking prevents unsafe swaps
- Version compatibility checking

### 3. AI-Augmented Intelligence
The kernel includes an AI supervisor that:
- Monitors system health in real-time
- Predicts potential failures
- Suggests and attempts automatic recovery
- Learns from past incidents

### 4. Multi-Architecture Support
Built for portability:
- x86_64 primary target
- ARM64 support planned
- Architecture-specific code isolated
- Optimized for both bare metal and virtualization

## 🤝 Contributing

This is an experimental research project. Contributions are welcome!

1. Read the [ROADMAP.md](./ROADMAP.md) to understand the project direction
2. Follow the [CODE_STYLE.md](./CODE_STYLE.md) guidelines
3. Focus on one subsystem at a time
4. Maintain the async-first design philosophy
5. Document architectural decisions
6. Test thoroughly in QEMU before submitting

## 📖 Documentation

- **[ROADMAP.md](./ROADMAP.md)** - Comprehensive development roadmap and gap analysis
- **[CODE_STYLE.md](./CODE_STYLE.md)** - Coding standards and best practices
- **[docs/ARCHITECTURE.md](./docs/ARCHITECTURE.md)** - System architecture details
- **[docs/MEMORY_SYSTEM.md](./docs/MEMORY_SYSTEM.md)** - Memory management documentation

## 📚 Learning Resources

- **OS Development**: [OSDev Wiki](https://wiki.osdev.org/)
- **x86 Assembly**: Intel Software Developer Manuals
- **Async Design**: Actor model and message-passing systems
- **AI Integration**: Kernel-level fault detection and recovery

## ⚠️ Current Limitations

- **Development Stage**: This is experimental research code - not production ready
- **x86 32-bit Only**: ARM64 and x86-64 support are planned but not implemented
- **Limited Hardware**: Currently supports basic VGA and keyboard only
- **No Filesystem**: VFS layer is not implemented
- **No Networking**: Network stack is not implemented
- **No User Space**: Processes cannot run in Ring 3 yet
- **AI Features Limited**: AI supervisor uses rule-based detection, no ML inference

---

**CLKernel v0.1.0** - An experimental OS kernel exploring async actor architecture 🚀
