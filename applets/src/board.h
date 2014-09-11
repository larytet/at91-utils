#ifndef _BOARD_
#define _BOARD_

#define BOARD_SAMA5D3X 1

#ifndef BOARD
#error "BOARD is not defined"
#endif

#if (BOARD == BOARD_SAMA5D3X)
#include "board_sama5d3x.h"
#endif

#endif //_BOARD_
