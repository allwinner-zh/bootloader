#ifndef __EXPORTS_H__
#define __EXPORTS_H__

#ifndef __ASSEMBLY__

#include <common.h>

/* These are declarations of exported functions available in C code */
unsigned long get_version(void);
int  getc(void);
int  tstc(void);
void putc(const char);
void puts(const char*);
int printf(const char* fmt, ...);
void install_hdlr(int, interrupt_handler_t*, void*);
void free_hdlr(int);
void *malloc(size_t);
void free(void*);
void __udelay(unsigned long);
unsigned long get_timer(unsigned long);
int vprintf(const char *, va_list);
unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);
int strict_strtoul(const char *cp, unsigned int base, unsigned long *res);
char *getenv (const char *name);
int setenv (const char *varname, const char *varvalue);
long simple_strtol(const char *cp,char **endp,unsigned int base);
int strcmp(const char * cs,const char * ct);
int ustrtoul(const char *cp, char **endp, unsigned int base);
#if defined(CONFIG_CMD_I2C)
int i2c_write (uchar, uint, int , uchar* , int);
int i2c_read (uchar, uint, int , uchar* , int);
#endif

int vsprintf(char *buf, const char *fmt, va_list args);
void flush_dcache_all(void);
int board_display_framebuffer_change(void *buffer);
int board_display_framebuffer_set(int width, int height, int bitcount, void *buffer);
void board_display_set_alpha_mode(int mode);
long disp_ioctl(void *hd, unsigned int cmd, void *arg);
int board_display_show_until_lcd_open(int display_source);


//mem service
char *strcpy(char *,const char *);
size_t strlen(const char * s);
void * memset(void *,int,__kernel_size_t);
void * memcpy(void *,const void *,__kernel_size_t);
int    memcmp(const void * cs,const void * ct,size_t count);

//irq service
void irq_free_handler(int irq);
void irq_install_handler (int irq, interrupt_handler_t handle_irq, void *data);
int  irq_enable(int irq_no);
int  irq_disable(int irq_no);

//screen print
int  screen_init(void);
void screen_printf(const char * str, ...);
void screen_set_color(unsigned int color);
int  screen_clear(void);

//others
void  flush_cache(unsigned long start, unsigned long size);
int fat_fsload_file (char * file_name,unsigned int ddr_addr);
int do_fat_fsload (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int sprintf(char * buf, const char *fmt, ...);

int do_go (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);


//timer
struct timer_list
{
	unsigned long expires;
	void (*function)(unsigned long data);
	unsigned long data;
	int   timer_num;
};

extern int init_timer(struct timer_list *timer);
extern int add_timer(struct timer_list *timer);
extern int del_timer(struct timer_list *timer);
void __msdelay(unsigned long msec);
void __usdelay(unsigned long usec);


//dma exports

typedef struct
{
    unsigned int      src_drq_type     : 5;
    unsigned int      src_addr_mode    : 2;
    unsigned int      src_burst_length : 2;
    unsigned int      src_data_width   : 2;
    unsigned int      reserved0        : 5;
    unsigned int      dst_drq_type     : 5;
    unsigned int      dst_addr_mode    : 2;
    unsigned int      dst_burst_length : 2;
    unsigned int      dst_data_width   : 2;
    unsigned int      reserved1        : 5;
}
sunxi_dma_channal_config;

//for user request
typedef struct
{
	sunxi_dma_channal_config  cfg;
	unsigned int	loop_mode;
	unsigned int	data_block_size;
	unsigned int	wait_cyc;
}
sunxi_dma_setting_t;

unsigned int 	sunxi_dma_request(unsigned int dmatype);
int sunxi_dma_release(unsigned int hdma);
int sunxi_dma_setting(unsigned int hdma, sunxi_dma_setting_t *cfg);
int sunxi_dma_start(unsigned int hdma, unsigned int saddr, unsigned int daddr, unsigned int bytes);
int sunxi_dma_stop(unsigned int hdma);
int sunxi_dma_querystatus(unsigned int hdma);

int sunxi_dma_install_int(uint hdma, interrupt_handler_t dma_int_func, void *p);
int sunxi_dma_disable_int(uint hdma);

int sunxi_dma_enable_int(uint hdma);
int sunxi_dma_free_int(uint hdma);

#include <spi.h>

void app_startup(char * const *);

#endif    /* ifndef __ASSEMBLY__ */

enum {
#define EXPORT_FUNC(x) XF_ ## x ,
#include <_exports.h>
#undef EXPORT_FUNC

	XF_MAX
};

#define XF_VERSION	6

#if defined(CONFIG_X86)
extern gd_t *global_data;
#endif

#endif	/* __EXPORTS_H__ */
