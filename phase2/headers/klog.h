#ifndef KLOG_H
#define KLOG_H

extern int klog_line_index;
extern int klog_char_index;
extern char klog_buffer[];

void klog_print(const char *str);
void klog_print_dec(int num);
void klog_print_hex(unsigned int num);

#endif
