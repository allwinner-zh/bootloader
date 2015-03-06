
#导入编译器配置
include make.cfg

#从所有子目录中得到源代码的列表
SRCCS=$(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
SRCSS=$(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.s))

#得到源代码对应的目标文件的列表
OBJS=$(SRCCS:.c=.o) $(SRCSS:.s=.o)

# 删除生成的中间文件
clean:
	-rm $(OBJS) $(LOCALTARGET) $(TMPTARGET) boot0.bin

