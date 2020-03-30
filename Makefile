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
cc = gcc
ld = ld

asminc = -I src/boot/include/
gccinc = -I src/lib/kernel/ -I src/lib/ -I src/kernel/ -I src/device/

# -fno-builtin 是要求gcc不主动使用自己的内建函数，除非显式声明
# -nostdinc 是不包含c语言标准库里的头文件
fcc = -c -Wall -m32 -nostdinc -fno-builtin $(gccinc)
fasm = -f elf
fld = -e main -m elf_i386 -Ttext 0xc0001000




#=========================================================================
#	target prog & its intermediate files
#-------------------------------------------------------------------------
# 相当于中间文件/目标文件生成在 -> "工程目录"/target/boot
boot = $(tb)/boot.bin $(tb)/loader.bin
kernel = $(tk)/kernel.bin
materials = $(tk)/main.o $(tk)/print.o $(tk)/prointrhdl.o $(tk)/interrupt.o $(tk)/init.o $(tk)/timer.o




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
all: $(boot) $(kernel)


# target writes into image
image: $(fd) $(boot)
	dd if=$(tb)/boot.bin of=$(fd) bs=512 count=1 conv=notrunc
	mount -o loop -t vfat $(fd) $(imp)
	cp $(tb)/loader.bin $(imp)
	cp $(tk)/kernel.bin $(imp)
	sync
	umount $(imp)

# run bochs
bochs: $(fd)
	$(xpath)/bochs -q

# prompt
run: all image bochs

# clean all intermediate files
clean: 
	rm -f $(tb)/*.o
	rm -f $(tk)/*.o

# clean all files
cleanall: clean
	rm -f $(boot)
	rm -f $(kernel)




#=========================================================================
#  .bin make rules
#-------------------------------------------------------------------------
# make image when not exists
$(fd):
	$(xpath)/bximage

#  make boot.bin
# @用target/label代替；<用prerequisites代替
$(tb)/boot.bin: src/boot/boot.asm
	$(asm) $(asminc) -o $@ $<

# make loader.bin
$(tb)/loader.bin: src/boot/loader.asm
	$(asm) $(asminc) -o $@ $<

# make print.o
$(tk)/print.o: src/lib/kernel/print.asm
	$(asm) $(fasm) -o $@ $<

# make prointrhdl.o
$(tk)/prointrhdl.o: src/kernel/prointrhdl.asm
	$(asm) $(fasm) -o $@ $<

# make main.o
$(tk)/main.o: src/kernel/main.c
	$(cc) $(fcc) -o $@ $<

# make interrupt.o
$(tk)/interrupt.o: src/kernel/interrupt.c
	$(cc) $(fcc) -o $@ $<

# make init.o
$(tk)/init.o: src/kernel/init.c
	$(cc) $(fcc) -o $@ $<

# make timer.o
$(tk)/timer.o: src/device/timer.c
	$(cc) $(fcc) -o $@ $<

# make kernel.bin
$(tk)/kernel.bin: $(materials)
	$(ld) $(fld) -o $@ $^