kdir:=/lib/modules/$(shell uname -r)/build
mount_src=./test_dir
mount_dir=./mount_dir

obj-m := xcfs.o
xcfs-objs := dentry.o file.o inode.o main.o super.o lookup.o mmap.o crypt.o

all:
	make -C $(kdir) M=$(PWD) modules > /dev/null
	
clean:
	make -C $(kdir) M=$(PWD) clean
	
insert:
	sudo insmod xcfs.ko
	
remove:
	sudo rmmod xcfs
	
mount:
	sudo mount -t xcfs $(mount_src) $(mount_dir)
	
unmount:
	sudo umount $(mount_dir)

buildmount:
	@make && (make unmount > /dev/null 2>&1 ; make remove > /dev/null 2>&1 ; make insert; make mount) > /dev/null
	
test:
	@cd ../test_src ; make run