kdir=~/wrapfs-latest/
# kdir=~/linux/
mount_src=./test_dir
mount_dir=./mount_dir

obj-m := wrapfs.o
wrapfs-objs := dentry.o file.o inode.o main.o super.o lookup.o mmap.o

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
	unmount $(mount_dir)