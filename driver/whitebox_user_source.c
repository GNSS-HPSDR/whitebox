#include <linux/uaccess.h>

#include "whitebox.h"
#include "whitebox_block.h"

static int whitebox_user_source_debug = 0;
#define d_printk(level, fmt, args...)				\
	if (whitebox_user_source_debug >= level) printk(KERN_INFO "%s: " fmt,	\
					__func__, ## args)

void whitebox_user_source_init(struct whitebox_user_source *user_source,
        int order, atomic_t *mapped)
{
    user_source->order = order;
    user_source->mapped = mapped;
}

int whitebox_user_source_alloc(struct whitebox_user_source *user_source)
{
    // alloc circ buffer
    user_source->buf_size = PAGE_SIZE << user_source->order;
    user_source->buf.buf = (char*)
            __get_free_pages(GFP_KERNEL | __GFP_DMA | __GFP_COMP |
            __GFP_NOWARN, user_source->order);
    if (!user_source->buf.buf) {
        d_printk(0, "failed to create port buffer\n");
        return -ENOMEM;
    }
    user_source->buf.head = 0;
    user_source->buf.tail = 0;
    return 0;
}

void whitebox_user_source_free(struct whitebox_user_source *user_source)
{
    // release the circ buffer
    free_pages((unsigned long)user_source->buf.buf, user_source->order);
}

size_t whitebox_user_source_space_available(struct whitebox_user_source *user_source,
        unsigned long *dest)
{
    long tail, head;
    head = user_source->buf.head;
    tail = ACCESS_ONCE(user_source->buf.tail);
    *dest = (unsigned long)user_source->buf.buf + head;
    return CIRC_SPACE_TO_END(head, tail, user_source->buf_size);
}

int whitebox_user_source_produce(struct whitebox_user_source *user_source,
        size_t count)
{
    d_printk(1, "values... %08x %08x %08x %08x\n",
            (u32)*(user_source->buf.buf + user_source->buf.head + 0),
            (u32)*(user_source->buf.buf + user_source->buf.head + 4),
            (u32)*(user_source->buf.buf + user_source->buf.head + 8),
            (u32)*(user_source->buf.buf + user_source->buf.head + 12));
    user_source->buf.head = (user_source->buf.head + count) & (user_source->buf_size - 1);
    user_source->off += count;
    return 0;
}

size_t whitebox_user_source_data_available(struct whitebox_user_source *user_source,
        unsigned long *src)
{
    long head, tail;
    head = ACCESS_ONCE(user_source->buf.head);
    tail = user_source->buf.tail;
    *src = (long)user_source->buf.buf + tail;
    return CIRC_CNT_TO_END(head, tail, user_source->buf_size);
}

int whitebox_user_source_consume(struct whitebox_user_source *user_source,
        size_t count)
{
    user_source->buf.tail = (user_source->buf.tail + count) & (user_source->buf_size - 1);
    return 0;
}

int whitebox_user_source_work(struct whitebox_user_source *user_source,
        unsigned long src, size_t src_count,
        unsigned long dest, size_t dest_count)
{
    size_t count = min(src_count, dest_count);
    d_printk(1, "src=%08lx src_count=%zu dest=%08lx dest_count=%zu\n",
            src, src_count, dest, dest_count);
    /*d_printk(1, "values... %08x %08x %08x %08x\n",
            (u32)*(src + 0),
            (u32)*(src + 4),
            (u32)*(src + 8),
            (u32)*(src + 12));*/

    if (copy_from_user((void*)dest, (void*)src, count) != 0) {
        d_printk(0, "failed to copy data from user\n");
        return -EFAULT;
    }
    return (int)count;
}
