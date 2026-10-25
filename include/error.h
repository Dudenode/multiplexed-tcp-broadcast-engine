#ifndef ERROR_H
#define ERROR_H

void die(const char *fmt, ...);
void thread_panic(int thread_err_code, const char *fmt, ...);
void sys_warn(const char *fmt, ...);

#endif