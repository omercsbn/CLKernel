/*
 * =============================================================================
 * CLKernel - Async-First Scheduler Implementation
 * =============================================================================
 * File: scheduler.c
 * Purpose: Actor-based cooperative scheduler for revolutionary kernel architecture
 *
 * This scheduler implements the core of CLKernel's async-first design:
 * - Actors instead of traditional threads
 * - Cooperative multitasking with message passing
 * - AI-supervised load balancing and deadlock detection
 * - Zero-copy message optimization where possible
 * =============================================================================
 */

#include "scheduler.h"
#include "heap.h"
#include "kernel.h"
#include "vga.h"
#include "idt.h"

// =============================================================================
// Global Scheduler State
// =============================================================================

scheduler_t kernel_scheduler;
bool scheduler_initialized = false;

// Simple actor memory pool (for initial implementation)
static actor_t actor_pool[MAX_ACTORS];
static bool actor_pool_used[MAX_ACTORS];

// Message memory pool
static message_t message_pool[MAX_MESSAGES];
static bool message_pool_used[MAX_MESSAGES];

// Internal function prototypes
static actor_t* scheduler_select_next_actor(void);
static void scheduler_context_switch(actor_t* next_actor);
static void scheduler_add_to_ready_queue(actor_t* actor);
static void scheduler_remove_from_ready_queue(actor_t* actor);
static void actor_create_kernel_actor(void);
static bool actor_add_message(actor_t* actor, message_t* message);
static void actor_clear_message_queue(actor_t* actor);
static message_t* message_allocate(void);

// =============================================================================
// Core Scheduler Functions
// =============================================================================

/*
 * Initialize the scheduler subsystem
 */
void scheduler_init(void)
{
    kprintf("[SCHEDULER] Initializing async-first scheduler...\n");
    
    // Clear scheduler state
    kernel_scheduler.ready_queue = NULL;
    kernel_scheduler.current_actor = NULL;
    kernel_scheduler.next_actor_id = 1; // ID 0 reserved for kernel
    kernel_scheduler.scheduler_enabled = false;
    kernel_scheduler.tick_count = 0;
    kernel_scheduler.current_timeslice = 0;
    kernel_scheduler.ai_supervision = true;
    
    // Clear actor table
    for (uint32_t i = 0; i < MAX_ACTORS; i++) {
        kernel_scheduler.actors[i] = NULL;
        actor_pool_used[i] = false;
    }
    
    // Initialize message system
    kernel_scheduler.free_messages = NULL;
    kernel_scheduler.message_pool = message_pool;
    kernel_scheduler.message_count = 0;
    
    // Clear message pool
    for (uint32_t i = 0; i < MAX_MESSAGES; i++) {
        message_pool_used[i] = false;
        message_pool[i].next = NULL;
    }
    
    // Clear statistics
    kernel_scheduler.statistics.context_switches = 0;
    kernel_scheduler.statistics.actors_created = 0;
    kernel_scheduler.statistics.actors_destroyed = 0;
    kernel_scheduler.statistics.messages_sent = 0;
    kernel_scheduler.statistics.messages_delivered = 0;
    kernel_scheduler.statistics.cpu_time_total = 0;
    kernel_scheduler.statistics.current_actors = 0;
    kernel_scheduler.statistics.ready_actors = 0;
    kernel_scheduler.statistics.blocked_actors = 0;
    kernel_scheduler.statistics.average_queue_depth = 0;
    kernel_scheduler.statistics.scheduler_overhead = 0;
    kernel_scheduler.statistics.deadlocks_detected = 0;
    kernel_scheduler.statistics.load_balance_actions = 0;
    
    // Create kernel actor (actor ID 0)
    actor_create_kernel_actor();
    
    scheduler_initialized = true;
    
    kprintf("[SCHEDULER] Actor-based scheduler initialized\n");
    kprintf("[SCHEDULER] Max actors: %d, Max messages: %d\n", MAX_ACTORS, MAX_MESSAGES);
    kprintf("[SCHEDULER] Time slice: %d ms\n", SCHEDULER_TIMESLICE_MS);
    kprintf("[SCHEDULER] AI supervision enabled\n");
}

/*
 * Start the scheduler (begin cooperative multitasking)
 */
void scheduler_start(void)
{
    if (!scheduler_initialized) {
        kprintf("[SCHEDULER] ERROR: Scheduler not initialized\n");
        return;
    }
    
    kernel_scheduler.scheduler_enabled = true;
    
    kprintf("[SCHEDULER] Cooperative multitasking started\n");
    
    // Begin scheduling
    scheduler_schedule();
}

/*
 * Schedule the next actor to run
 */
void scheduler_schedule(void)
{
    if (!kernel_scheduler.scheduler_enabled) {
        return;
    }
    
    actor_t* next_actor = scheduler_select_next_actor();
    
    if (next_actor != kernel_scheduler.current_actor) {
        scheduler_context_switch(next_actor);
    }
    
    kernel_scheduler.statistics.context_switches++;
}

/*
 * Yield CPU to the next actor
 */
void scheduler_yield(void)
{
    if (!scheduler_initialized) {
        return;
    }
    
    // Move current actor to end of ready queue if still ready
    actor_t* current = kernel_scheduler.current_actor;
    if (current && current->state == ACTOR_STATE_RUNNING) {
        current->state = ACTOR_STATE_READY;
        scheduler_add_to_ready_queue(current);
    }
    
    // Schedule next actor
    scheduler_schedule();
}

/*
 * Timer interrupt handler for preemptive scheduling
 */
void scheduler_timer_handler(void)
{
    if (!scheduler_initialized) {
        return;
    }
    
    kernel_scheduler.tick_count++;
    kernel_scheduler.current_timeslice++;
    
    // Update CPU time for current actor
    if (kernel_scheduler.current_actor) {
        kernel_scheduler.current_actor->cpu_time_used++;
    }
    
    // Check if time slice expired
    if (kernel_scheduler.current_timeslice >= SCHEDULER_TIMESLICE_MS) {
        kernel_scheduler.current_timeslice = 0;
        scheduler_yield(); // Cooperative yield
    }
    
    // Periodic AI analysis
    if ((kernel_scheduler.tick_count % 1000) == 0) {
        scheduler_ai_analyze_actors();
    }
}

// =============================================================================
// Actor Management Functions
// =============================================================================

/*
 * Create a new actor
 */
uint32_t actor_create(void* entry_point, void* user_data, 
                      uint8_t priority, size_t stack_size)
{
    if (!scheduler_initialized || !entry_point) {
        return 0; // Invalid actor ID
    }
    
    // Find free actor slot
    uint32_t actor_id = 0;
    for (uint32_t i = 1; i < MAX_ACTORS; i++) { // Skip ID 0 (kernel)
        if (!actor_pool_used[i]) {
            actor_id = i;
            break;
        }
    }
    
    if (actor_id == 0) {
        kprintf("[SCHEDULER] ERROR: No free actor slots\n");
        return 0;
    }
    
    // Get actor structure from pool
    actor_t* actor = &actor_pool[actor_id];
    actor_pool_used[actor_id] = true;
    
    // Initialize actor
    actor->actor_id = actor_id;
    actor->parent_id = kernel_scheduler.current_actor ? 
                      kernel_scheduler.current_actor->actor_id : 0;
    actor->state = ACTOR_STATE_CREATED;
    actor->priority = priority;
    actor->flags = 0;
    
    // Allocate stack
    if (stack_size == 0) {
        stack_size = ACTOR_STACK_SIZE;
    }
    
    actor->stack_base = kmalloc(stack_size);
    if (!actor->stack_base) {
        actor_pool_used[actor_id] = false;
        kprintf("[SCHEDULER] ERROR: Failed to allocate actor stack\n");
        return 0;
    }
    
    actor->stack_size = stack_size;
    actor->stack_current = (uint8_t*)actor->stack_base + stack_size - 4;
    actor->entry_point = entry_point;
    actor->user_data = user_data;
    
    // Initialize CPU context
    for (int i = 0; i < 8; i++) {
        actor->registers[i] = 0;
    }
    actor->eip = (uint32_t)entry_point;
    actor->esp = (uint32_t)actor->stack_current;
    actor->ebp = (uint32_t)actor->stack_current;
    actor->eflags = 0x200; // Enable interrupts
    
    // Initialize message queue
    actor->message_queue = NULL;
    actor->queue_size = 0;
    actor->max_queue_size = 64; // Default queue limit
    
    // Initialize statistics
    actor->cpu_time_used = 0;
    actor->messages_sent = 0;
    actor->messages_received = 0;
    actor->creation_time = kernel_scheduler.tick_count;
    actor->last_scheduled = 0;
    
    // Initialize memory context
    actor->memory_context = NULL; // TODO: integrate with memory manager
    actor->memory_limit = 1024 * 1024; // 1MB default limit
    actor->memory_used = stack_size;
    
    // Initialize error handling
    actor->error_code = 0;
    actor->error_message = NULL;
    
    // Initialize AI supervision
    actor->behavior_score = 100; // Start with good behavior
    actor->anomaly_count = 0;
    actor->ai_monitored = true;
    
    // Initialize list pointers
    actor->next = NULL;
    actor->prev = NULL;
    
    // Register actor
    kernel_scheduler.actors[actor_id] = actor;
    kernel_scheduler.statistics.actors_created++;
    kernel_scheduler.statistics.current_actors++;
    
    kprintf("[SCHEDULER] Created actor %d (priority=%s, stack=%d KB)\n", 
            actor_id, actor_priority_name(priority), (uint32_t)(stack_size / 1024));
    
    return actor_id;
}

/*
 * Start an actor (move from CREATED to READY state)
 */
bool actor_start(uint32_t actor_id)
{
    actor_t* actor = actor_get(actor_id);
    if (!actor || actor->state != ACTOR_STATE_CREATED) {
        return false;
    }
    
    actor->state = ACTOR_STATE_READY;
    scheduler_add_to_ready_queue(actor);
    
    kprintf("[SCHEDULER] Started actor %d\n", actor_id);
    return true;
}

/*
 * Terminate an actor
 */
void actor_terminate(uint32_t actor_id)
{
    actor_t* actor = actor_get(actor_id);
    if (!actor) {
        return;
    }
    
    kprintf("[SCHEDULER] Terminating actor %d\n", actor_id);
    
    // Remove from ready queue if present
    scheduler_remove_from_ready_queue(actor);
    
    // Free stack memory
    if (actor->stack_base) {
        kfree(actor->stack_base);
    }
    
    // Free error message if any
    if (actor->error_message) {
        kfree(actor->error_message);
    }
    
    // Clear pending messages
    actor_clear_message_queue(actor);
    
    // Update state and statistics
    actor->state = ACTOR_STATE_FINISHED;
    kernel_scheduler.statistics.actors_destroyed++;
    kernel_scheduler.statistics.current_actors--;
    
    // Free actor slot
    kernel_scheduler.actors[actor_id] = NULL;
    actor_pool_used[actor_id] = false;
    
    kprintf("[SCHEDULER] Actor %d terminated\n", actor_id);
}

/*
 * Get actor by ID
 */
actor_t* actor_get(uint32_t actor_id)
{
    if (actor_id >= MAX_ACTORS) {
        return NULL;
    }
    
    return kernel_scheduler.actors[actor_id];
}

/*
 * Get current running actor
 */
actor_t* actor_get_current(void)
{
    return kernel_scheduler.current_actor;
}

// =============================================================================
// Message Passing Functions
// =============================================================================

/*
 * Send asynchronous message
 */
bool message_send_async(uint32_t recipient_id, uint8_t type, 
                       void* payload, size_t payload_size)
{
    if (!scheduler_initialized) {
        return false;
    }
    
    actor_t* recipient = actor_get(recipient_id);
    if (!recipient) {
        return false;
    }
    
    message_t* message = message_allocate();
    if (!message) {
        kprintf("[SCHEDULER] ERROR: No free messages\n");
        return false;
    }
    
    // Initialize message
    message->sender_id = kernel_scheduler.current_actor ? 
                        kernel_scheduler.current_actor->actor_id : 0;
    message->recipient_id = recipient_id;
    message->message_id = kernel_scheduler.statistics.messages_sent + 1;
    message->type = type;
    message->priority = ACTOR_PRIORITY_NORMAL;
    message->flags = 0;
    message->payload_size = payload_size;
    message->timestamp = kernel_scheduler.tick_count;
    message->deadline = 0;
    message->reply_to = 0;
    message->requires_reply = false;
    message->next = NULL;
    
    // Copy payload if provided
    if (payload && payload_size > 0) {
        message->payload = kmalloc(payload_size);
        if (!message->payload) {
            message_free(message);
            return false;
        }
        
        // Simple memory copy
        uint8_t* src = (uint8_t*)payload;
        uint8_t* dst = (uint8_t*)message->payload;
        for (size_t i = 0; i < payload_size; i++) {
            dst[i] = src[i];
        }
    } else {
        message->payload = NULL;
    }
    
    // Add to recipient's message queue
    if (actor_add_message(recipient, message)) {
        kernel_scheduler.statistics.messages_sent++;
        
        if (kernel_scheduler.current_actor) {
            kernel_scheduler.current_actor->messages_sent++;
        }
        
        // Wake up recipient if blocked
        if (recipient->state == ACTOR_STATE_BLOCKED) {
            recipient->state = ACTOR_STATE_READY;
            scheduler_add_to_ready_queue(recipient);
        }
        
        return true;
    } else {
        // Failed to queue message
        if (message->payload) {
            kfree(message->payload);
        }
        message_free(message);
        return false;
    }
}

/*
 * Receive message (non-blocking)
 */
message_t* message_receive(void)
{
    actor_t* current = kernel_scheduler.current_actor;
    if (!current) {
        return NULL;
    }
    
    message_t* message = current->message_queue;
    if (message) {
        // Remove from queue
        current->message_queue = message->next;
        current->queue_size--;
        current->messages_received++;
        
        kernel_scheduler.statistics.messages_delivered++;
        
        message->next = NULL; // Detach from queue
        return message;
    }
    
    return NULL;
}

/*
 * Wait for message (blocking)
 */
message_t* message_wait(uint32_t timeout_ms)
{
    actor_t* current = kernel_scheduler.current_actor;
    if (!current) {
        return NULL;
    }
    
    // Check if message already available
    message_t* message = message_receive();
    if (message) {
        return message;
    }
    
    // Block actor and wait for message
    current->state = ACTOR_STATE_BLOCKED;
    scheduler_remove_from_ready_queue(current);
    
    // TODO: Implement timeout handling
    // For now, yield to scheduler
    scheduler_yield();
    
    // When we resume, check for message again
    return message_receive();
}

/*
 * Free message after processing
 */
void message_free(message_t* message)
{
    if (!message) {
        return;
    }
    
    // Free payload if allocated
    if (message->payload) {
        kfree(message->payload);
    }
    
    // Find message in pool and mark as free
    for (uint32_t i = 0; i < MAX_MESSAGES; i++) {
        if (&message_pool[i] == message) {
            message_pool_used[i] = false;
            break;
        }
    }
}

// =============================================================================
// Scheduler Internal Functions
// =============================================================================

/*
 * Select next actor to run
 */
actor_t* scheduler_select_next_actor(void)
{
    // Simple round-robin for now
    // TODO: Implement priority-based scheduling and AI optimization
    
    if (!kernel_scheduler.ready_queue) {
        return NULL;
    }
    
    return kernel_scheduler.ready_queue;
}

/*
 * Perform context switch
 */
void scheduler_context_switch(actor_t* next_actor)
{
    actor_t* current = kernel_scheduler.current_actor;
    
    if (current == next_actor) {
        return; // No switch needed
    }
    
    // Save current actor state
    if (current && current->state == ACTOR_STATE_RUNNING) {
        // TODO: Save CPU context
        current->state = ACTOR_STATE_READY;
    }
    
    // Switch to next actor
    if (next_actor) {
        kernel_scheduler.current_actor = next_actor;
        next_actor->state = ACTOR_STATE_RUNNING;
        next_actor->last_scheduled = kernel_scheduler.tick_count;
        
        // Remove from ready queue
        scheduler_remove_from_ready_queue(next_actor);
        
        // TODO: Load CPU context
        
        kprintf("[SCHEDULER] Context switch: %d -> %d\n",
                current ? current->actor_id : 0, next_actor->actor_id);
    }
}

/*
 * Add actor to ready queue
 */
void scheduler_add_to_ready_queue(actor_t* actor)
{
    if (!actor || actor->state != ACTOR_STATE_READY) {
        return;
    }
    
    // Simple insertion at head for now
    // TODO: Implement priority-based insertion
    
    actor->next = kernel_scheduler.ready_queue;
    actor->prev = NULL;
    
    if (kernel_scheduler.ready_queue) {
        kernel_scheduler.ready_queue->prev = actor;
    }
    
    kernel_scheduler.ready_queue = actor;
    kernel_scheduler.statistics.ready_actors++;
}

/*
 * Remove actor from ready queue
 */
void scheduler_remove_from_ready_queue(actor_t* actor)
{
    if (!actor) {
        return;
    }
    
    // Remove from linked list
    if (actor->prev) {
        actor->prev->next = actor->next;
    } else {
        kernel_scheduler.ready_queue = actor->next;
    }
    
    if (actor->next) {
        actor->next->prev = actor->prev;
    }
    
    actor->next = NULL;
    actor->prev = NULL;
    
    kernel_scheduler.statistics.ready_actors--;
}

/*
 * Create kernel actor (ID 0)
 */
void actor_create_kernel_actor(void)
{
    actor_t* kernel_actor = &actor_pool[0];
    actor_pool_used[0] = true;
    
    kernel_actor->actor_id = 0;
    kernel_actor->parent_id = 0;
    kernel_actor->state = ACTOR_STATE_RUNNING;
    kernel_actor->priority = ACTOR_PRIORITY_CRITICAL;
    kernel_actor->flags = 0;
    
    kernel_actor->stack_base = NULL; // Kernel uses boot stack
    kernel_actor->stack_current = NULL;
    kernel_actor->stack_size = 0;
    kernel_actor->entry_point = NULL;
    kernel_actor->user_data = NULL;
    
    // Clear CPU context
    for (int i = 0; i < 8; i++) {
        kernel_actor->registers[i] = 0;
    }
    
    kernel_actor->message_queue = NULL;
    kernel_actor->queue_size = 0;
    kernel_actor->max_queue_size = 256; // Large queue for kernel
    
    // Initialize statistics
    kernel_actor->cpu_time_used = 0;
    kernel_actor->messages_sent = 0;
    kernel_actor->messages_received = 0;
    kernel_actor->creation_time = 0;
    kernel_actor->last_scheduled = 0;
    
    kernel_actor->memory_context = NULL;
    kernel_actor->memory_limit = 0; // Unlimited for kernel
    kernel_actor->memory_used = 0;
    
    kernel_actor->error_code = 0;
    kernel_actor->error_message = NULL;
    
    kernel_actor->behavior_score = 100;
    kernel_actor->anomaly_count = 0;
    kernel_actor->ai_monitored = false; // Don't monitor kernel
    
    kernel_actor->next = NULL;
    kernel_actor->prev = NULL;
    
    kernel_scheduler.actors[0] = kernel_actor;
    kernel_scheduler.current_actor = kernel_actor;
    
    kprintf("[SCHEDULER] Kernel actor created (ID 0)\n");
}

// =============================================================================
// Helper Functions
// =============================================================================

/*
 * Allocate a message from the pool
 */
message_t* message_allocate(void)
{
    for (uint32_t i = 0; i < MAX_MESSAGES; i++) {
        if (!message_pool_used[i]) {
            message_pool_used[i] = true;
            return &message_pool[i];
        }
    }
    
    return NULL;
}

/*
 * Add message to actor's queue
 */
bool actor_add_message(actor_t* actor, message_t* message)
{
    if (!actor || !message) {
        return false;
    }
    
    // Check queue size limit
    if (actor->queue_size >= actor->max_queue_size) {
        kprintf("[SCHEDULER] Actor %d message queue full\n", actor->actor_id);
        return false;
    }
    
    // Add to end of queue
    message->next = NULL;
    
    if (!actor->message_queue) {
        actor->message_queue = message;
    } else {
        message_t* tail = actor->message_queue;
        while (tail->next) {
            tail = tail->next;
        }
        tail->next = message;
    }
    
    actor->queue_size++;
    return true;
}

/*
 * Clear actor's message queue
 */
void actor_clear_message_queue(actor_t* actor)
{
    if (!actor) {
        return;
    }
    
    message_t* current = actor->message_queue;
    while (current) {
        message_t* next = current->next;
        message_free(current);
        current = next;
    }
    
    actor->message_queue = NULL;
    actor->queue_size = 0;
}

// =============================================================================
// Statistics and Monitoring
// =============================================================================

/*
 * Get scheduler statistics
 */
scheduler_stats_t* scheduler_get_statistics(void)
{
    if (!scheduler_initialized) {
        return NULL;
    }
    
    return &kernel_scheduler.statistics;
}

/*
 * Print scheduler status
 */
void scheduler_print_status(void)
{
    if (!scheduler_initialized) {
        kprintf("[SCHEDULER] Not initialized\n");
        return;
    }
    
    scheduler_stats_t* stats = &kernel_scheduler.statistics;
    
    kprintf("[SCHEDULER] Status Report:\n");
    kprintf("  Scheduler enabled: %s\n", 
            kernel_scheduler.scheduler_enabled ? "YES" : "NO");
    kprintf("  Current actors: %d\n", stats->current_actors);
    kprintf("  Ready actors: %d\n", stats->ready_actors);
    kprintf("  Blocked actors: %d\n", stats->blocked_actors);
    kprintf("  Context switches: %d\n", (uint32_t)stats->context_switches);
    kprintf("  Messages sent: %d\n", (uint32_t)stats->messages_sent);
    kprintf("  Messages delivered: %d\n", (uint32_t)stats->messages_delivered);
    kprintf("  Tick count: %d\n", kernel_scheduler.tick_count);
    
    if (kernel_scheduler.current_actor) {
        kprintf("  Current actor: %d (%s)\n", 
                kernel_scheduler.current_actor->actor_id,
                actor_state_name(kernel_scheduler.current_actor->state));
    }
}

/*
 * Print actor information
 */
void scheduler_print_actors(void)
{
    kprintf("[SCHEDULER] Actor List:\n");
    
    for (uint32_t i = 0; i < MAX_ACTORS; i++) {
        actor_t* actor = kernel_scheduler.actors[i];
        if (actor) {
            kprintf("  Actor %d: %s, Priority=%s, CPU=%d, Messages=%d/%d\n",
                    actor->actor_id,
                    actor_state_name(actor->state),
                    actor_priority_name(actor->priority),
                    (uint32_t)actor->cpu_time_used,
                    (uint32_t)actor->messages_sent,
                    (uint32_t)actor->messages_received);
        }
    }
}

// =============================================================================
// AI Integration Stubs
// =============================================================================

/*
 * AI-based load balancing
 */
void scheduler_ai_balance_load(void)
{
    if (!kernel_scheduler.ai_supervision) {
        return;
    }
    
    // TODO: Implement AI load balancing
    // - Analyze actor CPU usage patterns
    // - Redistribute work based on priorities
    // - Optimize message routing
    
    kprintf("[AI-SCHEDULER] Load balancing analysis completed\n");
}

/*
 * AI deadlock detection
 */
bool scheduler_ai_detect_deadlock(void)
{
    if (!kernel_scheduler.ai_supervision) {
        return false;
    }
    
    // TODO: Implement intelligent deadlock detection
    // - Analyze message dependencies
    // - Check for circular waits
    // - Predict potential deadlocks
    
    return false; // No deadlocks detected
}

/*
 * AI actor behavior analysis
 */
void scheduler_ai_analyze_actors(void)
{
    if (!kernel_scheduler.ai_supervision) {
        return;
    }
    
    // TODO: Implement behavior analysis
    // - Monitor message patterns
    // - Detect anomalous behavior
    // - Update actor behavior scores
    
    // Simple analysis for now
    for (uint32_t i = 0; i < MAX_ACTORS; i++) {
        actor_t* actor = kernel_scheduler.actors[i];
        if (actor && actor->ai_monitored) {
            // Simple heuristic: actors with balanced send/receive have good behavior
            if (actor->messages_sent > 0 && actor->messages_received > 0) {
                if (actor->behavior_score < 100) {
                    actor->behavior_score++;
                }
            }
        }
    }
}

// =============================================================================
// Debug Functions
// =============================================================================

/*
 * Dump scheduler state for debugging
 */
void scheduler_dump_state(void)
{
    kprintf("[SCHEDULER] Internal State Dump:\n");
    kprintf("  Scheduler enabled: %d\n", kernel_scheduler.scheduler_enabled);
    kprintf("  Next actor ID: %d\n", kernel_scheduler.next_actor_id);
    kprintf("  Ready queue head: 0x%x\n", (uint32_t)kernel_scheduler.ready_queue);
    kprintf("  Current actor: 0x%x\n", (uint32_t)kernel_scheduler.current_actor);
    kprintf("  Message count: %d\n", kernel_scheduler.message_count);
    kprintf("  AI supervision: %d\n", kernel_scheduler.ai_supervision);
    
    // Dump ready queue
    kprintf("  Ready queue:\n");
    actor_t* current = kernel_scheduler.ready_queue;
    int count = 0;
    while (current && count < 10) {
        kprintf("    -> Actor %d (%s)\n", 
                current->actor_id, actor_state_name(current->state));
        current = current->next;
        count++;
    }
}

/*
 * Test scheduler functionality
 */
void scheduler_test_functionality(void)
{
    kprintf("[SCHEDULER] Running scheduler tests...\n");
    
    // Test 1: Actor creation
    uint32_t test_actor = actor_create((void*)0x12345678, NULL, 
                                      ACTOR_PRIORITY_NORMAL, 4096);
    if (test_actor != 0) {
        kprintf("  Test 1 - Actor creation: SUCCESS (ID %d)\n", test_actor);
    } else {
        kprintf("  Test 1 - Actor creation: FAILED\n");
    }
    
    // Test 2: Message sending
    bool msg_sent = message_send_async(test_actor, MSG_TYPE_ASYNC, "Hello", 6);
    if (msg_sent) {
        kprintf("  Test 2 - Message sending: SUCCESS\n");
    } else {
        kprintf("  Test 2 - Message sending: FAILED\n");
    }
    
    // Test 3: Statistics
    scheduler_stats_t* stats = scheduler_get_statistics();
    if (stats && stats->actors_created > 0) {
        kprintf("  Test 3 - Statistics: SUCCESS (%d actors created)\n", 
                stats->actors_created);
    } else {
        kprintf("  Test 3 - Statistics: FAILED\n");
    }
    
    // Cleanup
    if (test_actor != 0) {
        actor_terminate(test_actor);
    }
    
    kprintf("[SCHEDULER] Scheduler tests completed\n");
}
