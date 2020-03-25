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
gccinc = -I src/lib/kernel/ -I src/lib/
cc = gcc
ld = ld




#=========================================================================
#	target prog & its intermediate files
#-------------------------------------------------------------------------
# 相当于中间文件/目标文件生成在 -> "工程目录"/target/boot
boot = $(tb)/boot.bin $(tb)/loader.bin
kernel = $(tk)/kernel.bin




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
all: $(kernel)


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
run: $(fd)
	@echo "使用虚拟机挂载$(fd)即可开始运行"

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
$(tb)/boot.bin: src/boot/include/fat12.inc
$(tb)/boot.bin: src/boot/boot.asm
	$(asm) $(asminc) -o $@ $<

# make loader.bin
$(tb)/loader.bin: src/boot/include/fat12.inc
$(tb)/loader.bin: src/boot/include/loader.inc
$(tb)/loader.bin: src/boot/loader.asm
	$(asm) $(asminc) -o $@ $<

# make print.o
$(tk)/print.o: src/lib/kernel/print.asm
	$(asm) -f elf -o $@ $<

# make main.o
$(tk)/main.o: src/lib/kernel/print.h
$(tk)/main.o: src/lib/stdint.h
$(tk)/main.o: src/kernel/main.c
	$(cc) -c -m32 $(gccinc) -o $@ $<

# make kernel.bin
$(tk)/kernel.bin: $(tk)/main.o
$(tk)/kernel.bin: $(tk)/print.o
	$(ld) -e main -m elf_i386 -Ttext 0xc0001000 -o $@ $(tk)/main.o $(tk)/print.o