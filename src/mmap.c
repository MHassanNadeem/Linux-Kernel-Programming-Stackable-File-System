/*
 * Copyright (c) 1998-2017 Erez Zadok
 * Copyright (c) 2009	   Shrikar Archak
 * Copyright (c) 2003-2017 Stony Brook University
 * Copyright (c) 2003-2017 The Research Foundation of SUNY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "wrapfs.h"

static int wrapfs_fault(struct vm_fault *vmf)
{
	int err;
	struct file *file, *lower_file;
	const struct vm_operations_struct *lower_vm_ops;
	struct vm_area_struct lower_vma;
	struct vm_area_struct *vma = vmf->vma;

	memcpy(&lower_vma, vma, sizeof(struct vm_area_struct));
	file = lower_vma.vm_file;
	lower_vm_ops = WRAPFS_F(file)->lower_vm_ops;
	BUG_ON(!lower_vm_ops);

	lower_file = wrapfs_lower_file(file);
	/*
	 * XXX: vm_ops->fault may be called in parallel.  Because we have to
	 * resort to temporarily changing the vma->vm_file to point to the
	 * lower file, a concurrent invocation of wrapfs_fault could see a
	 * different value.  In this workaround, we keep a different copy of
	 * the vma structure in our stack, so we never expose a different
	 * value of the vma->vm_file called to us, even temporarily.  A
	 * better fix would be to change the calling semantics of ->fault to
	 * take an explicit file pointer.
	 */
	lower_vma.vm_file = lower_file;
	err = lower_vm_ops->fault(vmf);
	return err;
}

static int wrapfs_page_mkwrite(struct vm_fault *vmf)
{
	int err = 0;
	struct file *file, *lower_file;
	const struct vm_operations_struct *lower_vm_ops;
	struct vm_area_struct lower_vma;
	struct vm_area_struct *vma = vmf->vma;

	memcpy(&lower_vma, vma, sizeof(struct vm_area_struct));
	file = lower_vma.vm_file;
	lower_vm_ops = WRAPFS_F(file)->lower_vm_ops;
	BUG_ON(!lower_vm_ops);
	if (!lower_vm_ops->page_mkwrite)
		goto out;

	lower_file = wrapfs_lower_file(file);
	/*
	 * XXX: vm_ops->page_mkwrite may be called in parallel.
	 * Because we have to resort to temporarily changing the
	 * vma->vm_file to point to the lower file, a concurrent
	 * invocation of wrapfs_page_mkwrite could see a different
	 * value.  In this workaround, we keep a different copy of the
	 * vma structure in our stack, so we never expose a different
	 * value of the vma->vm_file called to us, even temporarily.
	 * A better fix would be to change the calling semantics of
	 * ->page_mkwrite to take an explicit file pointer.
	 */
	lower_vma.vm_file = lower_file;
	err = lower_vm_ops->page_mkwrite(vmf);
out:
	return err;
}

static ssize_t wrapfs_direct_IO(struct kiocb *iocb, struct iov_iter *iter)
{
	/*
	 * This function should never be called directly.  We need it
	 * to exist, to get past a check in open_check_o_direct(),
	 * which is called from do_last().
	 */
	return -EINVAL;
}

static int wrapfs_readpage(struct file *file, struct page *page)
{
	int err = 0;
	struct file *lower_file;
	char* page_ptr = NULL;
	mm_segment_t old_fs;
	//char* scratch = NULL;
	loff_t offset = page_offset(page);

	//Allocating scratch which is size of page
	//scratch = kmalloc(PAGE_SIZE, GFP_KERNEL);
	//memset(scratch, 0, PAGE_SIZE);
	
	//Getting virtual address of page by mmapping it
	page_ptr = (char *) kmap(page); 
	lower_file = wrapfs_lower_file(file);
	if(!lower_file){
		err = 0;
		goto error_handler;
	}


	//Allows us to pass page_ptr to vfs_read
	old_fs = get_fs();
	set_fs(get_ds());
	//Reading encrypted data into scratch
	err = vfs_read(lower_file, (void __user *)page_ptr, PAGE_SIZE, &offset);
	set_fs(old_fs);
	if(err ==0)
		goto error_handler;
	xcfs_decrypt(page_ptr, err);
	kunmap(page);
	
error_handler:
	//Error handling
	if(err)
		ClearPageUptodate(page);
	else
		SetPageUptodate(page);
	unlock_page(page);
	return err;
}

const struct address_space_operations wrapfs_aops = {
	//.writepage = wrapfs_writepage,
	.readpage = wrapfs_readpage,
	.direct_IO = wrapfs_direct_IO,
};

const struct vm_operations_struct wrapfs_vm_ops = {
	.fault		= wrapfs_fault,
	.page_mkwrite	= wrapfs_page_mkwrite,
};
