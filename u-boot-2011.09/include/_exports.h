/*
 * You do not need to use #ifdef around functions that may not exist
 * in the final configuration (such as i2c).
 */
EXPORT_FUNC(get_version)
EXPORT_FUNC(getc)
EXPORT_FUNC(tstc)
EXPORT_FUNC(putc)
EXPORT_FUNC(puts)
EXPORT_FUNC(printf)
EXPORT_FUNC(install_hdlr)
EXPORT_FUNC(free_hdlr)
EXPORT_FUNC(malloc)
EXPORT_FUNC(free)
EXPORT_FUNC(udelay)
EXPORT_FUNC(get_timer)
EXPORT_FUNC(vprintf)
EXPORT_FUNC(do_reset)
EXPORT_FUNC(getenv)
EXPORT_FUNC(setenv)
EXPORT_FUNC(simple_strtoul)
EXPORT_FUNC(strict_strtoul)
EXPORT_FUNC(simple_strtol)
EXPORT_FUNC(strcmp)
EXPORT_FUNC(i2c_write)
EXPORT_FUNC(i2c_read)
EXPORT_FUNC(spi_init)
EXPORT_FUNC(spi_setup_slave)
EXPORT_FUNC(spi_free_slave)
EXPORT_FUNC(spi_claim_bus)
EXPORT_FUNC(spi_release_bus)
EXPORT_FUNC(spi_xfer)

//mem
EXPORT_FUNC(strcpy)
EXPORT_FUNC(strlen)
EXPORT_FUNC(memset)
EXPORT_FUNC(memcpy)
EXPORT_FUNC(memcmp)
//irq
EXPORT_FUNC(irq_free_handler)
EXPORT_FUNC(irq_install_handler)
EXPORT_FUNC(irq_enable)
EXPORT_FUNC(irq_disable)
//screen
EXPORT_FUNC(vsprintf)
EXPORT_FUNC(flush_dcache_all)
#ifdef CONFIG_SUNXI_DISPLAY
EXPORT_FUNC(board_display_framebuffer_change)
EXPORT_FUNC(board_display_framebuffer_set)
EXPORT_FUNC(disp_ioctl)
EXPORT_FUNC(board_display_show_until_lcd_open)
EXPORT_FUNC(board_display_set_alpha_mode)
#endif

//timer
EXPORT_FUNC(init_timer)
EXPORT_FUNC(add_timer)
EXPORT_FUNC(del_timer)
EXPORT_FUNC(__msdelay)
EXPORT_FUNC(__usdelay)


//dma exports
EXPORT_FUNC(sunxi_dma_request)
EXPORT_FUNC(sunxi_dma_release)
EXPORT_FUNC(sunxi_dma_setting)
EXPORT_FUNC(sunxi_dma_start)
EXPORT_FUNC(sunxi_dma_stop)
EXPORT_FUNC(sunxi_dma_querystatus)

EXPORT_FUNC(sunxi_dma_install_int)
EXPORT_FUNC(sunxi_dma_disable_int)

EXPORT_FUNC(sunxi_dma_enable_int)
EXPORT_FUNC(sunxi_dma_free_int)

//others
EXPORT_FUNC(sprintf)
#ifndef CONFIG_SUNXI_SPINOR_PLATFORM
EXPORT_FUNC(do_fat_fsload)
EXPORT_FUNC(fat_fsload_file)
EXPORT_FUNC(do_go)
#endif
EXPORT_FUNC(flush_cache)


