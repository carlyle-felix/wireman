#ifndef ROOT_H
#define ROOT_H

typedef enum {CHECK, GAIN, DROP} Euid_ops;
int euid_helper(Euid_ops option);

#endif