#ifndef _DDR_
#define _DDR_

typedef enum
{
	DDR_TYPE_MT47H64M16HR,
	DDR_TYPE_MT47H128M16RT
} ddr_type_t;

extern void ddr_configure(ddr_type_t ddr_type);


extern void ddr_test(void);

#endif //_DDR_
