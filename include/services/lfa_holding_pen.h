#ifndef LFA_HOLDING_PEN_H
#define LFA_HOLDING_PEN_H

#include <stdbool.h>
#include <stdint.h>

bool lfa_holding_start();
int lfa_holding_wait();
void lfa_holding_release(int status);

#endif
