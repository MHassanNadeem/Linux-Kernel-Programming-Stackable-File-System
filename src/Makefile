# kdir=~/wrapfs-latest/
kdir:=/lib/modules/$(shell uname -r)/build
mount_src=./test_dir
mount_dir=./mount_dir

obj-m := wrapfs.o
wrapfs-objs := dentry.o file.o inode.o main.o super.o lookup.o mmap.o crypt.o

all:
	make -C $(kdir) M=$(PWD) modules > /dev/null
	
clean:
	make -C $(kdir) M=$(PWD) clean
	
insert:
	sudo insmod wrapfs.ko
	
remove:
	sudo rmmod wrapfs
	
mount:
	sudo mount -t wrapfs $(mount_src) $(mount_dir)
	
unmount:
	sudo umount $(mount_dir)

test:
	@rm -f $(mount_dir)/test_file
	@echo "ABC" > $(mount_dir)/test_file
	@echo "encrypted file: " && cat $(mount_src)/test_file && echo ""