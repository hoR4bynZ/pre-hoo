#=========================================================================
#	variable
#-------------------------------------------------------------------------
# bochs path
xpath = /usr/local/bochs/bin

# target dir
t = target
tb = target/boot
tk = target/kernel

# floppy image kernel loaded
fd = floppy.img
# hard disk image

# image mount point
imp = /media/

# compiler & its parameters
asm = nasm
asminc = -I src/boot/include/
cc = gcc




#=========================================================================
#	target prog & its intermediate files
#-------------------------------------------------------------------------
# 相当于中间文件/目标文件生成在 -> "工程目录"/target/boot
boot = $(tb)/boot.bin $(tb)/loader.bin




#=========================================================================
#  pseudo-commands
#-------------------------------------------------------------------------
.PHONY: nop all image bochs run clean cleanall
# default(make )
nop:
	@echo "all		compile all files to make target file"
	@echo "image		target writes into image"
	@echo "bochs		debug in bochs"
	@echo "run		prompt how to install kernel to virtual machine to run"
	@echo "clean		clean all intermediate files"
	@echo "cleanall	clean all intermediate and target files"

# compile all files
all: $(boot)


# target writes into image
image: $(fd) $(boot)
	dd if=$(tb)/boot.bin of=$(fd) bs=512 count=1 conv=notrunc
	mount -o loop -t vfat $(fd) $(imp)
	cp $(tb)/loader.bin $(imp)
	sync
	umount $(imp)

# run bochs
bochs: $(fd)
	$(xpath)/bochs -q

# prompt
run: $(fd)
	@echo "使用虚拟机挂载$(fd)即可开始运行"

# clean all intermediate files
clean: 
	@echo "目前还没有中间文件"

# clean all files
cleanall: clean
	rm -f $(boot)




#=========================================================================
#  .bin make rules
#-------------------------------------------------------------------------
# make image when not exists
$(fd):
	$(xpath)/bximage

#  make boot
# @用target/label代替；<用prerequisites代替
$(tb)/boot.bin: src/boot/include/fat12.inc
$(tb)/boot.bin: src/boot/boot.asm
	$(asm) $(asminc) -o $@ $<

# make loader
#$(tb)/boot.bin: src/boot/include/fat12.inc
$(tb)/loader.bin: src/boot/loader.asm
	$(asm) $(asminc) -o $@ $<