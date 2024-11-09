//在include/core当中修改

#include "zhangzheng.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qemu/timer.h"  // For timestamp

PeripheralDeviceState *device_states[MAX_DEVICES] = {NULL};
// 在一个源文件中定义并初始化全局变量
int device_count = 1; 
// 查找或创建设备状态的辅助函数
PeripheralDeviceState* find_or_create_device(const char *device_name) {
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (device_states[i] && strcmp(device_states[i]->device_name, device_name) == 0) {
            return device_states[i];
        }
    }
    // 创建新的设备状态
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (!device_states[i]) {
            device_states[i] = malloc(sizeof(DeviceState));
            device_states[i]->device_name = strdup(device_name);
            device_states[i]->access_log_head = NULL;
            device_states[i]->cache_queue = malloc(sizeof(RegisterAccess) * CACHE_QUEUE_SIZE);
            device_states[i]->prev_value = 0;
            device_states[i]->prev_addr = 0;
            device_states[i]->irq_status = 0;
            return device_states[i];
        }
    }
    return NULL; // 设备数量超限时返回空
}
// 查找设备状态的函数，根据设备名称来确定设备
// 假设 get_device_state_for_irq 函数应该根据设备名返回 PeripheralDeviceState
PeripheralDeviceState* get_device_state_for_irq(const char* device_name) {
    for (int i = 0; i < device_count; i++) {
        if (strcmp(device_states[i]->device_name, device_name) == 0) {
            return device_states[i];
        }
    }
    return NULL;  // 找不到设备时返回 NULL
}


void log_register_access(const char *device_name, hwaddr addr, const char *operation,
                         uint64_t value, uint32_t irq) {
    PeripheralDeviceState *device = find_or_create_device(device_name);
    
    // 使用缓存队列中的下一个条目来记录访问日志
    static int cache_index = 0;
    RegisterAccess *access = &device->cache_queue[cache_index % CACHE_QUEUE_SIZE];
    cache_index++;

    access->timestamp = qemu_clock_get_ns(QEMU_CLOCK_REALTIME);
    access->reg_addr = addr;
    access->value = value;
    access->operation = operation;
    access->irq = irq;
    access->prev_value = device->prev_value;
    access->prev_addr = device->prev_addr;

    // 更新设备状态中的前值和前地址 
    device->prev_value = value;
    device->prev_addr = addr;

    // 将该访问记录连接到日志链表头
    access->prev = device->access_log_head;
    device->access_log_head = access;
}

void log_irq_status(const char *device_name, uint32_t irq) {
    PeripheralDeviceState *device = find_or_create_device(device_name);
    if (!device) return;

    // 更新当前设备的中断状态
    RegisterAccess *access = device->access_log_head;
    if (access) {
        access->irq = irq;
    } else {
        fprintf(stderr, "Warning: No register access log found for device %s to update IRQ\n", device_name);
    }
}

void log_device_init(const char *device_name, hwaddr base_addr) {
    PeripheralDeviceState *device = find_or_create_device(device_name);
    if (!device) return;

    device->base_addr = base_addr;
    printf("Device %s initialized at base address 0x%lx\n", device_name, base_addr);
}
