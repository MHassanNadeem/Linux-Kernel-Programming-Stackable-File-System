mount_src=../src/test_dir
mount_dir=../src/mount_dir

all:
	@gcc tester.c -o tester
	
clean:
	rm -rf tester
	
run: all
	@./tester $(shell realpath $(mount_src)) $(shell realpath $(mount_dir))