// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>

#define DRV_NAME "haru-dma"

/* ---------- IOCTL API ---------- */
#define HARU_IOC_MAGIC 'h'
#define HARU_GET_PHYS _IOR(HARU_IOC_MAGIC, 10, __u64) /* returns dma/bus addr */
#define HARU_GET_SIZE _IOR(HARU_IOC_MAGIC, 11, __u64) /* returns bytes */

/* Module parameter: buffer size in KiB (page-multiple recommended) */
static unsigned int buf_size_kib = 2048; /* 2 MiB default */
module_param(buf_size_kib, uint, 0444);
MODULE_PARM_DESC(buf_size_kib, "DMA buffer size in KiB (default 2048)");

static void *g_cpu;
static dma_addr_t g_dma;
static size_t g_len;
static struct device *g_dev;

static int haru_open(struct inode *ino, struct file *filp)
{
  pr_info(DRV_NAME ": open\n");
  return 0;
}

static int haru_release(struct inode *ino, struct file *filp)
{
  pr_info(DRV_NAME ": release\n");
  return 0;
}

static long haru_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
  __u64 out = 0;

  switch (cmd)
  {
  case HARU_GET_PHYS:
    out = (u64)g_dma;
    break;
  case HARU_GET_SIZE:
    out = (u64)g_len;
    break;
  default:
    return -ENOTTY;
  }

  if (copy_to_user((void __user *)arg, &out, sizeof(out)))
    return -EFAULT;
  return 0;
}

#ifdef CONFIG_COMPAT
static long haru_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
  return haru_unlocked_ioctl(filp, cmd, arg);
}
#endif

static int haru_mmap(struct file *filp, struct vm_area_struct *vma)
{
  unsigned long vsize = vma->vm_end - vma->vm_start;
  if (vsize > g_len)
    return -EINVAL;
  /* Proper helper for coherent DMA memory */
  return dma_mmap_coherent(g_dev, vma, g_cpu, g_dma, g_len);
}

static const struct file_operations haru_fops = {
    .owner = THIS_MODULE,
    .open = haru_open,
    .release = haru_release,
    .unlocked_ioctl = haru_unlocked_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = haru_compat_ioctl,
#endif
    .mmap = haru_mmap,
};

static struct miscdevice haru_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DRV_NAME, /* -> /dev/haru-dma */
    .fops = &haru_fops,
    .mode = 0660,
};

static int __init haru_init(void)
{
  int rc;

  /* Round size to page multiple */
  g_len = PAGE_ALIGN(((size_t)buf_size_kib) << 10);

  /* 1) Register misc device FIRST to get a struct device */
  rc = misc_register(&haru_miscdev);
  if (rc)
  {
    pr_err(DRV_NAME ": misc_register failed %d\n", rc);
    return rc;
  }

  g_dev = haru_miscdev.this_device;
  if (!g_dev)
  {
    rc = -ENODEV;
    goto err_misc;
  }

  /* 2) Ensure dma_mask pointer exists, then coerce a mask */
  if (!g_dev->dma_mask)
    g_dev->dma_mask = &g_dev->coherent_dma_mask;

  /* Try 40-bit (ZynqMP/PS DDR often needs >32), then 48, then 32 */
  if (dma_coerce_mask_and_coherent(g_dev, DMA_BIT_MASK(40)))
  {
    if (dma_coerce_mask_and_coherent(g_dev, DMA_BIT_MASK(48)))
    {
      if (dma_coerce_mask_and_coherent(g_dev, DMA_BIT_MASK(32)))
      {
        pr_err(DRV_NAME ": no suitable DMA mask\n");
        rc = -EIO;
        goto err_misc;
      }
    }
  }

  /* 3) Allocate coherent buffer (comes from CMA on arm64) */
  g_cpu = dma_alloc_coherent(g_dev, g_len, &g_dma, GFP_KERNEL);
  if (!g_cpu)
  {
    rc = -ENOMEM;
    goto err_misc;
  }

  pr_info(DRV_NAME ": loaded (dma) cpu=%p dma=%pad size=%zu mask=0x%llx\n",
          g_cpu, &g_dma, g_len, (unsigned long long)g_dev->coherent_dma_mask);
  return 0;

err_misc:
  misc_deregister(&haru_miscdev);
  return rc;
}

static void __exit haru_exit(void)
{
  if (g_cpu)
    dma_free_coherent(g_dev, g_len, g_cpu, g_dma);
  misc_deregister(&haru_miscdev);
  pr_info(DRV_NAME ": unloaded\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Elton Shih");
MODULE_DESCRIPTION("HARU DMA (coherent buffer + ioctl + mmap)");
module_init(haru_init);
module_exit(haru_exit);