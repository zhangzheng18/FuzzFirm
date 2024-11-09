#ifndef ZHANGZHENG_H
#define ZHANGZHENG_H

#include <stdint.h>
#include <stddef.h>

#define MAX_DEVICES 256  // 假设最多支持 256 个设备，可以根据需要调整
extern int device_count; 
typedef uintptr_t hwaddr; // 为 hwaddr 提供一个 typedef 声明

// 单次寄存器访问的结构体
typedef struct RegisterAccess {
    uint64_t timestamp;
    hwaddr reg_addr;
    uint64_t value;
    const char *operation;    // "read" 或 "write"
    uint32_t irq;             // 中断状态
    struct RegisterAccess *next; // 指向下一个访问
} RegisterAccess;

// 每个设备的状态管理结构体
typedef struct PeripheralDeviceState {
    char *device_name;
    RegisterAccess *access_log_head; // 日志链表头
    RegisterAccess *cache_queue; // 缓存队列
    uint64_t prev_value;         // 上次访问的值
    hwaddr prev_addr;            // 上次访问的地址
    uint32_t irq_count;          // 中断conut
    uint32_t irq_status;         // 中断状态
} PeripheralDeviceState;


extern PeripheralDeviceState *device_states[MAX_DEVICES];

// 函数接口
void log_register_access(const char *device_name, hwaddr addr, const char *operation,
                         uint64_t value, uint32_t irq);
void log_irq_status(const char *device_name, uint32_t irq);
void log_device_init(const char *device_name, hwaddr base_addr);
PeripheralDeviceState* get_device_state_for_irq(qemu_irq irq);
#endif // ZHANGZHENG_H
