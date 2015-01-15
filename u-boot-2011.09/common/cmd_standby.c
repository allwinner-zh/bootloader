/*
 * Command for accessing DataFlash.
 *
 * Copyright (C) 2008 Atmel Corporation
 */
#include <common.h>
#include <sunxi_cmd.h>

int do_standby(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int standby_mode;

	if(uboot_spare_head.boot_data.work_mode == WORK_MODE_BOOT)
	{
		if(argc == 2)
		{
			standby_mode = simple_strtol(argv[1], NULL, 10);
		}
		else
		{
			standby_mode = 1;
		}
		printf ("## enter standby ...\n");
		board_status_probe(standby_mode);
		printf("## exit standby ...\n");
		do_sunxi_logo(NULL, 0, 0, NULL);
	}

	return 0;
}



U_BOOT_CMD(
	standby, 2,	0,	do_standby,
	"run to boot standby",
	"no parameters\n"
);



