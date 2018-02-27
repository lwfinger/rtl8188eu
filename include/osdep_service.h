/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __OSDEP_SERVICE_H_
#define __OSDEP_SERVICE_H_

#include <drv_conf.h>
#include <basic_types.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    #include <linux/sched/signal.h>
#endif

#define _FAIL		0
#define _SUCCESS	1
#define RTW_RX_HANDLED 2
/* define RTW_STATUS_TIMEDOUT -110 */

	#include <linux/version.h>
	#include <linux/spinlock.h>
	#include <linux/compiler.h>
	#include <linux/kernel.h>
	#include <linux/errno.h>
	#include <linux/init.h>
	#include <linux/slab.h>
	#include <linux/module.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,5))
	#include <linux/kref.h>
#endif
	/* include <linux/smp_lock.h> */
	#include <linux/netdevice.h>
	#include <linux/skbuff.h>
	#include <linux/circ_buf.h>
	#include <asm/uaccess.h>
	#include <asm/atomic.h>
	#include <asm/io.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
	#include <asm/semaphore.h>
#else
	#include <linux/semaphore.h>
#endif
	#include <linux/sem.h>
	#include <linux/sched.h>
	#include <linux/etherdevice.h>
	#include <linux/wireless.h>
	#include <net/iw_handler.h>
	#include <linux/if_arp.h>
	#include <linux/rtnetlink.h>
	#include <linux/delay.h>
	#include <linux/proc_fs.h>	/*  Necessary because we use the proc fs */
	#include <linux/interrupt.h>	/*  for struct tasklet_struct */
	#include <linux/ip.h>
	#include <linux/kthread.h>
	#include <linux/in.h>
	#include <net/route.h>
	#include <net/flow.h>
	#include <net/arp.h>
        #include <net/ieee80211_radiotap.h>
	#include <net/cfg80211.h>
	#include <linux/usb.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21))
	#include <linux/usb_ch9.h>
#else
	#include <linux/usb/ch9.h>
#endif

extern int rtw_mc2u_disable;

extern char* rtw_initmac;

extern int rtw_ht_enable;
extern int rtw_cbw40_enable;
extern int rtw_ampdu_enable;/* for enable tx_ampdu */

extern int ui_pid[3];

extern unsigned char	MCS_rate_2R[16];
extern unsigned char	MCS_rate_1R[16];
extern unsigned char RTW_WPA_OUI[];
extern unsigned char WPA_TKIP_CIPHER[4];
extern unsigned char RSN_TKIP_CIPHER[4];

struct dvobj_priv;
void rtw_unregister_netdevs(struct dvobj_priv *dvobj);
int pm_netdev_open(struct net_device *pnetdev,u8 bnormal);

#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22))
#ifdef CONFIG_USB_SUSPEND
#define CONFIG_AUTOSUSPEND	1
#endif
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	typedef struct mutex		_mutex;
#else
	typedef struct semaphore	_mutex;
#endif
	struct	__queue	{
		struct	list_head	queue;
		spinlock_t	lock;
	};

	#define thread_exit() complete_and_exit(NULL, 0)

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
	#define DMA_BIT_MASK(n) (((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22))
/*  Porting from linux kernel, for compatible with old kernel. */
static inline unsigned char *skb_tail_pointer(const struct sk_buff *skb)
{
	return skb->tail;
}

static inline void skb_reset_tail_pointer(struct sk_buff *skb)
{
	skb->tail = skb->data;
}

static inline void skb_set_tail_pointer(struct sk_buff *skb, const int offset)
{
	skb->tail = skb->data + offset;
}

static inline unsigned char *skb_end_pointer(const struct sk_buff *skb)
{
	return skb->end;
}
#endif

__inline static struct  list_head *get_next(struct  list_head *list)
{
	return list->next;
}

__inline static struct  list_head *get_list_head(struct  __queue *queue)
{
	return (&(queue->queue));
}


#define LIST_CONTAINOR(ptr, type, member) \
        ((type *)((char *)(ptr)-(SIZE_T)(&((type *)0)->member)))


__inline static void _enter_critical(spinlock_t *plock, unsigned long *pirqL)
{
	spin_lock_irqsave(plock, *pirqL);
}

__inline static void _exit_critical(spinlock_t *plock, unsigned long *pirqL)
{
	spin_unlock_irqrestore(plock, *pirqL);
}

__inline static void _enter_critical_ex(spinlock_t *plock, unsigned long *pirqL)
{
	spin_lock_irqsave(plock, *pirqL);
}

__inline static void _exit_critical_ex(spinlock_t *plock, unsigned long *pirqL)
{
	spin_unlock_irqrestore(plock, *pirqL);
}

__inline static int _enter_critical_mutex(_mutex *pmutex, unsigned long *pirqL)
{
	int ret = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	ret = mutex_lock_interruptible(pmutex);
#else
	ret = down_interruptible(pmutex);
#endif
	return ret;
}


__inline static void _exit_critical_mutex(_mutex *pmutex, unsigned long *pirqL)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
		mutex_unlock(pmutex);
#else
		up(pmutex);
#endif
}

__inline static void rtw_list_delete(struct  list_head *plist)
{
	list_del_init(plist);
}

__inline static void _init_timer(struct timer_list *ptimer,struct  net_device * nic_hdl,void *pfunc,void* cntx)
{
	ptimer->function = pfunc;
	ptimer->data = (unsigned long)cntx;
	init_timer(ptimer);
}

__inline static void _set_timer(struct timer_list *ptimer,u32 delay_time)
{
	mod_timer(ptimer , (jiffies+(delay_time*HZ/1000)));
}

__inline static void _cancel_timer(struct timer_list *ptimer,u8 *bcancelled)
{
	del_timer_sync(ptimer);
	*bcancelled=  true;/* true ==1; false== 0 */
}

#define RTW_TIMER_HDL_ARGS void *FunctionContext

#define RTW_TIMER_HDL_NAME(name) rtw_##name##_timer_hdl
#define RTW_DECLARE_TIMER_HDL(name) void RTW_TIMER_HDL_NAME(name)(RTW_TIMER_HDL_ARGS)


__inline static void _init_workitem(struct work_struct *pwork, void *pfunc, void * cntx)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
	INIT_WORK(pwork, pfunc);
#else
	INIT_WORK(pwork, pfunc,pwork);
#endif
}

__inline static void _set_workitem(struct work_struct *pwork)
{
	schedule_work(pwork);
}

__inline static void _cancel_workitem_sync(struct work_struct *pwork)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22))
	cancel_work_sync(pwork);
#else
	flush_scheduled_work();
#endif
}
/*  */
/*  Global Mutex: can only be used at PASSIVE level. */
/*  */

#define ACQUIRE_GLOBAL_MUTEX(_MutexCounter)                              \
{                                                               \
	while (atomic_inc_return((atomic_t *)&(_MutexCounter)) != 1)\
	{                                                           \
		atomic_dec((atomic_t *)&(_MutexCounter));        \
		msleep(10);                          \
	}                                                           \
}

#define RELEASE_GLOBAL_MUTEX(_MutexCounter)                              \
{                                                               \
	atomic_dec((atomic_t *)&(_MutexCounter));        \
}

static inline int rtw_netif_queue_stopped(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
	return (netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 0)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 1)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 2)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 3)) );
#else
	return netif_queue_stopped(pnetdev);
#endif
}

static inline void rtw_netif_wake_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
	netif_tx_wake_all_queues(pnetdev);
#else
	netif_wake_queue(pnetdev);
#endif
}

static inline void rtw_netif_start_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
	netif_tx_start_all_queues(pnetdev);
#else
	netif_start_queue(pnetdev);
#endif
}

static inline void rtw_netif_stop_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
	netif_tx_stop_all_queues(pnetdev);
#else
	netif_stop_queue(pnetdev);
#endif
}

#ifndef BIT
	#define BIT(x)	( 1 << (x))
#endif

#define BIT0	0x00000001
#define BIT1	0x00000002
#define BIT2	0x00000004
#define BIT3	0x00000008
#define BIT4	0x00000010
#define BIT5	0x00000020
#define BIT6	0x00000040
#define BIT7	0x00000080
#define BIT8	0x00000100
#define BIT9	0x00000200
#define BIT10	0x00000400
#define BIT11	0x00000800
#define BIT12	0x00001000
#define BIT13	0x00002000
#define BIT14	0x00004000
#define BIT15	0x00008000
#define BIT16	0x00010000
#define BIT17	0x00020000
#define BIT18	0x00040000
#define BIT19	0x00080000
#define BIT20	0x00100000
#define BIT21	0x00200000
#define BIT22	0x00400000
#define BIT23	0x00800000
#define BIT24	0x01000000
#define BIT25	0x02000000
#define BIT26	0x04000000
#define BIT27	0x08000000
#define BIT28	0x10000000
#define BIT29	0x20000000
#define BIT30	0x40000000
#define BIT31	0x80000000
#define BIT32	0x0100000000
#define BIT33	0x0200000000
#define BIT34	0x0400000000
#define BIT35	0x0800000000
#define BIT36	0x1000000000

int RTW_STATUS_CODE(int error_code);

/* define CONFIG_USE_VMALLOC */

/* flags used for rtw_mstat_update() */
enum mstat_f {
	/* type: 0x00ff */
	MSTAT_TYPE_VIR = 0x00,
	MSTAT_TYPE_PHY= 0x01,
	MSTAT_TYPE_SKB = 0x02,
	MSTAT_TYPE_USB = 0x03,
	MSTAT_TYPE_MAX = 0x04,

	/* func: 0xff00 */
	MSTAT_FUNC_UNSPECIFIED = 0x00<<8,
	MSTAT_FUNC_IO = 0x01<<8,
	MSTAT_FUNC_TX_IO = 0x02<<8,
	MSTAT_FUNC_RX_IO = 0x03<<8,
	MSTAT_FUNC_TX = 0x04<<8,
	MSTAT_FUNC_RX = 0x05<<8,
	MSTAT_FUNC_MAX = 0x06<<8,
};

#define mstat_tf_idx(flags) ((flags)&0xff)
#define mstat_ff_idx(flags) (((flags)&0xff00) >> 8)

enum mstat_status {
	MSTAT_ALLOC_SUCCESS = 0,
	MSTAT_ALLOC_FAIL,
	MSTAT_FREE
};

#ifdef DBG_MEM_ALLOC
void rtw_mstat_update(const enum mstat_f flags, const MSTAT_STATUS status, u32 sz);
int _rtw_mstat_dump(char *buf, int len);
void rtw_mstat_dump (void);
u8* dbg_rtw_vmalloc(u32 sz, const enum mstat_f flags, const char *func, const int line);
u8* dbg_rtw_zvmalloc(u32 sz, const enum mstat_f flags, const char *func, const int line);
void dbg_rtw_vmfree(u8 *pbuf, const enum mstat_f flags, u32 sz, const char *func, const int line);
u8* dbg_rtw_malloc(u32 sz, const enum mstat_f flags, const char *func, const int line);
u8* dbg_rtw_zmalloc(u32 sz, const enum mstat_f flags, const char *func, const int line);
void dbg_rtw_mfree(u8 *pbuf, const enum mstat_f flags, u32 sz, const char *func, const int line);

struct sk_buff * dbg_rtw_skb_alloc(unsigned int size, const enum mstat_f flags, const char *func, const int line);
void dbg_rtw_skb_free(struct sk_buff *skb, const enum mstat_f flags, const char *func, const int line);
struct sk_buff *dbg_rtw_skb_copy(const struct sk_buff *skb, const enum mstat_f flags, const char *func, const int line);
struct sk_buff *dbg_rtw_skb_clone(struct sk_buff *skb, const enum mstat_f flags, const char *func, const int line);
int dbg_rtw_netif_rx(struct  net_device * ndev, struct sk_buff *skb, const enum mstat_f flags, const char *func, int line);
void dbg_rtw_skb_queue_purge(struct sk_buff_head *list, enum mstat_f flags, const char *func, int line);

void *dbg_rtw_usb_buffer_alloc(struct usb_device *dev, size_t size, dma_addr_t *dma, const enum mstat_f flags, const char *func, const int line);
void dbg_rtw_usb_buffer_free(struct usb_device *dev, size_t size, void *addr, dma_addr_t dma, const enum mstat_f flags, const char *func, const int line);

#ifdef CONFIG_USE_VMALLOC
#define rtw_vmalloc(sz)			dbg_rtw_vmalloc((sz), MSTAT_TYPE_VIR, __FUNCTION__, __LINE__)
#define rtw_zvmalloc(sz)			dbg_rtw_zvmalloc((sz), MSTAT_TYPE_VIR, __FUNCTION__, __LINE__)
#define rtw_vmfree(pbuf, sz)		dbg_rtw_vmfree((pbuf), (sz), MSTAT_TYPE_VIR, __FUNCTION__, __LINE__)
#define rtw_vmalloc_f(sz, mstat_f)			dbg_rtw_vmalloc((sz), ((mstat_f)&0xff00)|MSTAT_TYPE_VIR, __FUNCTION__, __LINE__)
#define rtw_zvmalloc_f(sz, mstat_f)		dbg_rtw_zvmalloc((sz), ((mstat_f)&0xff00)|MSTAT_TYPE_VIR, __FUNCTION__, __LINE__)
#define rtw_vmfree_f(pbuf, sz, mstat_f)	dbg_rtw_vmfree((pbuf), (sz), ((mstat_f)&0xff00)|MSTAT_TYPE_VIR, __FUNCTION__, __LINE__)
#else /* CONFIG_USE_VMALLOC */
#define rtw_vmalloc(sz)			dbg_rtw_malloc((sz), MSTAT_TYPE_PHY, __FUNCTION__, __LINE__)
#define rtw_zvmalloc(sz)			dbg_rtw_zmalloc((sz), MSTAT_TYPE_PHY, __FUNCTION__, __LINE__)
#define rtw_vmfree(pbuf, sz)		dbg_rtw_mfree((pbuf), (sz), MSTAT_TYPE_PHY, __FUNCTION__, __LINE__)
#define rtw_vmalloc_f(sz, mstat_f)			dbg_rtw_malloc((sz), ((mstat_f)&0xff00)|MSTAT_TYPE_PHY, __FUNCTION__, __LINE__)
#define rtw_zvmalloc_f(sz, mstat_f)		dbg_rtw_zmalloc((sz), ((mstat_f)&0xff00)|MSTAT_TYPE_PHY, __FUNCTION__, __LINE__)
#define rtw_vmfree_f(pbuf, sz, mstat_f)	dbg_rtw_mfree((pbuf), (sz), ((mstat_f)&0xff00)|MSTAT_TYPE_PHY, __FUNCTION__, __LINE__)
#endif /* CONFIG_USE_VMALLOC */
#define rtw_malloc(sz)			dbg_rtw_malloc((sz), MSTAT_TYPE_PHY, __FUNCTION__, __LINE__)
#define rtw_zmalloc(sz)			dbg_rtw_zmalloc((sz), MSTAT_TYPE_PHY, __FUNCTION__, __LINE__)
#define rtw_mfree(pbuf, sz)		dbg_rtw_mfree((pbuf), (sz), MSTAT_TYPE_PHY, __FUNCTION__, __LINE__)
#define rtw_malloc_f(sz, mstat_f)			dbg_rtw_malloc((sz), ((mstat_f)&0xff00)|MSTAT_TYPE_PHY, __FUNCTION__, __LINE__)
#define rtw_zmalloc_f(sz, mstat_f)			dbg_rtw_zmalloc((sz), ((mstat_f)&0xff00)|MSTAT_TYPE_PHY, __FUNCTION__, __LINE__)
#define rtw_mfree_f(pbuf, sz, mstat_f)		dbg_rtw_mfree((pbuf), (sz), ((mstat_f)&0xff00)|MSTAT_TYPE_PHY, __FUNCTION__, __LINE__)

#define rtw_skb_alloc(size)	dbg_rtw_skb_alloc((size), MSTAT_TYPE_SKB, __FUNCTION__, __LINE__)
#define rtw_skb_free(skb)	dbg_rtw_skb_free((skb), MSTAT_TYPE_SKB, __FUNCTION__, __LINE__)
#define rtw_skb_alloc_f(size, mstat_f)	dbg_rtw_skb_alloc((size), ((mstat_f)&0xff00)|MSTAT_TYPE_SKB, __FUNCTION__, __LINE__)
#define rtw_skb_free_f(skb, mstat_f)	dbg_rtw_skb_free((skb), ((mstat_f)&0xff00)|MSTAT_TYPE_SKB, __FUNCTION__, __LINE__)
#define rtw_skb_copy(skb)	dbg_rtw_skb_copy((skb), MSTAT_TYPE_SKB, __FUNCTION__, __LINE__)
#define rtw_skb_clone(skb)	dbg_rtw_skb_clone((skb), MSTAT_TYPE_SKB, __FUNCTION__, __LINE__)
#define rtw_skb_copy_f(skb, mstat_f)	dbg_rtw_skb_copy((skb), ((mstat_f)&0xff00)|MSTAT_TYPE_SKB, __FUNCTION__, __LINE__)
#define rtw_skb_clone_f(skb, mstat_f)	dbg_rtw_skb_clone((skb), ((mstat_f)&0xff00)|MSTAT_TYPE_SKB, __FUNCTION__, __LINE__)
#define rtw_netif_rx(ndev, skb)	dbg_rtw_netif_rx(ndev, skb, MSTAT_TYPE_SKB, __FUNCTION__, __LINE__)
#define rtw_skb_queue_purge(sk_buff_head) dbg_rtw_skb_queue_purge(sk_buff_head, MSTAT_TYPE_SKB, __FUNCTION__, __LINE__)
#define rtw_usb_buffer_alloc(dev, size, dma)		dbg_rtw_usb_buffer_alloc((dev), (size), (dma), MSTAT_TYPE_USB, __FUNCTION__, __LINE__)
#define rtw_usb_buffer_free(dev, size, addr, dma)	dbg_rtw_usb_buffer_free((dev), (size), (addr), (dma), MSTAT_TYPE_USB, __FUNCTION__, __LINE__)
#define rtw_usb_buffer_alloc_f(dev, size, dma, mstat_f)			dbg_rtw_usb_buffer_alloc((dev), (size), (dma), ((mstat_f)&0xff00)|MSTAT_TYPE_USB, __FUNCTION__, __LINE__)
#define rtw_usb_buffer_free_f(dev, size, addr, dma, mstat_f)	dbg_rtw_usb_buffer_free((dev), (size), (addr), (dma), ((mstat_f)&0xff00)|MSTAT_TYPE_USB, __FUNCTION__, __LINE__)

#else /* DBG_MEM_ALLOC */
#define rtw_mstat_update(flag, status, sz) do {} while (0)
#define rtw_mstat_dump() do {} while (0)
u8*	_rtw_vmalloc(u32 sz);
u8*	_rtw_zvmalloc(u32 sz);
void	_rtw_vmfree(u8 *pbuf, u32 sz);
u8*	_rtw_zmalloc(u32 sz);
u8*	_rtw_malloc(u32 sz);
void	_rtw_mfree(u8 *pbuf, u32 sz);

struct sk_buff *_rtw_skb_alloc(u32 sz);
void _rtw_skb_free(struct sk_buff *skb);
struct sk_buff *_rtw_skb_copy(const struct sk_buff *skb);
struct sk_buff *_rtw_skb_clone(struct sk_buff *skb);
int _rtw_netif_rx(struct  net_device * ndev, struct sk_buff *skb);
void _rtw_skb_queue_purge(struct sk_buff_head *list);

void *_rtw_usb_buffer_alloc(struct usb_device *dev, size_t size, dma_addr_t *dma);
void _rtw_usb_buffer_free(struct usb_device *dev, size_t size, void *addr, dma_addr_t dma);

#ifdef CONFIG_USE_VMALLOC
#define rtw_vmalloc(sz)			_rtw_vmalloc((sz))
#define rtw_zvmalloc(sz)			_rtw_zvmalloc((sz))
#define rtw_vmfree(pbuf, sz)		_rtw_vmfree((pbuf), (sz))
#define rtw_vmalloc_f(sz, mstat_f)			_rtw_vmalloc((sz))
#define rtw_zvmalloc_f(sz, mstat_f)		_rtw_zvmalloc((sz))
#define rtw_vmfree_f(pbuf, sz, mstat_f)	_rtw_vmfree((pbuf), (sz))
#else /* CONFIG_USE_VMALLOC */
#define rtw_vmalloc(sz)			_rtw_malloc((sz))
#define rtw_zvmalloc(sz)			_rtw_zmalloc((sz))
#define rtw_vmfree(pbuf, sz)		_rtw_mfree((pbuf), (sz))
#define rtw_vmalloc_f(sz, mstat_f)			_rtw_malloc((sz))
#define rtw_zvmalloc_f(sz, mstat_f)		_rtw_zmalloc((sz))
#define rtw_vmfree_f(pbuf, sz, mstat_f)	_rtw_mfree((pbuf), (sz))
#endif /* CONFIG_USE_VMALLOC */
#define rtw_malloc(sz)			_rtw_malloc((sz))
#define rtw_zmalloc(sz)			_rtw_zmalloc((sz))
#define rtw_mfree(pbuf, sz)		_rtw_mfree((pbuf), (sz))
#define rtw_malloc_f(sz, mstat_f)			_rtw_malloc((sz))
#define rtw_zmalloc_f(sz, mstat_f)			_rtw_zmalloc((sz))
#define rtw_mfree_f(pbuf, sz, mstat_f)		_rtw_mfree((pbuf), (sz))

#define rtw_skb_alloc(size) _rtw_skb_alloc((size))
#define rtw_skb_free(skb) _rtw_skb_free((skb))
#define rtw_skb_alloc_f(size, mstat_f)	_rtw_skb_alloc((size))
#define rtw_skb_free_f(skb, mstat_f)	_rtw_skb_free((skb))
#define rtw_skb_copy(skb)	_rtw_skb_copy((skb))
#define rtw_skb_clone(skb)	_rtw_skb_clone((skb))
#define rtw_skb_copy_f(skb, mstat_f)	_rtw_skb_copy((skb))
#define rtw_skb_clone_f(skb, mstat_f)	_rtw_skb_clone((skb))
#define rtw_netif_rx(ndev, skb) _rtw_netif_rx(ndev, skb)
#define rtw_skb_queue_purge(sk_buff_head) _rtw_skb_queue_purge(sk_buff_head)
#define rtw_usb_buffer_alloc(dev, size, dma) _rtw_usb_buffer_alloc((dev), (size), (dma))
#define rtw_usb_buffer_free(dev, size, addr, dma) _rtw_usb_buffer_free((dev), (size), (addr), (dma))
#define rtw_usb_buffer_alloc_f(dev, size, dma, mstat_f) _rtw_usb_buffer_alloc((dev), (size), (dma))
#define rtw_usb_buffer_free_f(dev, size, addr, dma, mstat_f) _rtw_usb_buffer_free((dev), (size), (addr), (dma))
#endif /* DBG_MEM_ALLOC */

static inline void	_rtw_spinlock(spinlock_t *plock)
{
	spin_lock(plock);
}

static inline void	_rtw_spinunlock(spinlock_t *plock)
{
	spin_unlock(plock);
}


static inline void	_rtw_spinlock_ex(spinlock_t *plock)
{
	spin_lock(plock);
}

static inline void	_rtw_spinunlock_ex(spinlock_t *plock)
{
	spin_unlock(plock);
}

void*	rtw_malloc2d(int h, int w, int size);
void	rtw_mfree2d(void *pbuf, int h, int w, int size);

int	_rtw_memcmp(void *dst, void *src, u32 sz);

void	_rtw_init_listhead(struct  list_head *list);
u32	rtw_is_list_empty(struct  list_head *phead);
void	rtw_list_insert_head(struct  list_head *plist, struct  list_head *phead);
void	rtw_list_insert_tail(struct  list_head *plist, struct  list_head *phead);
void	rtw_list_delete(struct  list_head *plist);

void	_rtw_init_sema(struct  semaphore *sema, int init_val);
void	_rtw_free_sema(struct  semaphore *sema);
void	_rtw_up_sema(struct  semaphore *sema);
u32	_rtw_down_sema(struct  semaphore *sema);
void	_rtw_mutex_init(_mutex *pmutex);
void	_rtw_mutex_free(_mutex *pmutex);

void	_rtw_init_queue(struct  __queue	*pqueue);
u32	_rtw_queue_empty(struct  __queue *pqueue);
u32	rtw_end_of_queue_search(struct  list_head *queue, struct  list_head *pelement);

u32	rtw_systime_to_ms(u32 systime);
u32	rtw_ms_to_systime(u32 ms);
s32	rtw_get_passing_time_ms(u32 start);
s32	rtw_get_time_interval_ms(u32 start, u32 end);

void	rtw_sleep_schedulable(int ms);

void	rtw_msleep_os(int ms);
void	rtw_usleep_os(int us);

u32	rtw_atoi(u8* s);

#ifdef DBG_DELAY_OS
#define rtw_mdelay_os(ms) _rtw_mdelay_os((ms), __FUNCTION__, __LINE__)
#define rtw_udelay_os(ms) _rtw_udelay_os((ms), __FUNCTION__, __LINE__)
void _rtw_mdelay_os(int ms, const char *func, const int line);
void _rtw_udelay_os(int us, const char *func, const int line);
#else
void	rtw_mdelay_os(int ms);
void	rtw_udelay_os(int us);
#endif

void rtw_yield_os(void);


__inline static unsigned char _cancel_timer_ex(struct timer_list *ptimer)
{
	return del_timer_sync(ptimer);
}

static __inline void thread_enter(char *name)
{
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
	daemonize("%s", name);
	#endif
	allow_signal(SIGTERM);
}

__inline static void flush_signals_thread(void)
{
	if (signal_pending (current))
	{
		flush_signals(current);
	}
}

__inline static void rtw_dump_stack(void)
{
	dump_stack();
}

#define rtw_warn_on(condition) WARN_ON(condition)

#define _RND(sz, r) ((((sz)+((r)-1))/(r))*(r))
#define RND4(x)	(((x >> 2) + (((x & 3) == 0) ?  0: 1)) << 2)

__inline static u32 _RND4(u32 sz)
{

	u32	val;

	val = ((sz >> 2) + ((sz & 3) ? 1: 0)) << 2;

	return val;

}

__inline static u32 _RND8(u32 sz)
{

	u32	val;

	val = ((sz >> 3) + ((sz & 7) ? 1: 0)) << 3;

	return val;

}

__inline static u32 _RND128(u32 sz)
{

	u32	val;

	val = ((sz >> 7) + ((sz & 127) ? 1: 0)) << 7;

	return val;

}

__inline static u32 _RND256(u32 sz)
{

	u32	val;

	val = ((sz >> 8) + ((sz & 255) ? 1: 0)) << 8;

	return val;

}

__inline static u32 _RND512(u32 sz)
{

	u32	val;

	val = ((sz >> 9) + ((sz & 511) ? 1: 0)) << 9;

	return val;

}

__inline static u32 bitshift(u32 bitmask)
{
	u32 i;

	for (i = 0; i <= 31; i++)
		if (((bitmask>>i) &  0x1) == 1) break;

	return i;
}

#ifndef MAC_FMT
#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#endif
#ifndef MAC_ARG
#define MAC_ARG(x) ((u8*)(x))[0],((u8*)(x))[1],((u8*)(x))[2],((u8*)(x))[3],((u8*)(x))[4],((u8*)(x))[5]
#endif

/* ifdef __GNUC__ */
#define STRUCT_PACKED __attribute__ ((packed))

/*  limitation of path length */
	#define PATH_LENGTH_MAX PATH_MAX

void rtw_suspend_lock_init(void);
void rtw_suspend_lock_uninit(void);
void rtw_lock_suspend(void);
void rtw_unlock_suspend(void);
void rtw_lock_suspend_timeout(u32 timeout_ms);
void rtw_lock_ext_suspend_timeout(u32 timeout_ms);


/* Atomic integer operations */
	#define ATOMIC_T atomic_t

void ATOMIC_SET(ATOMIC_T *v, int i);
int ATOMIC_READ(ATOMIC_T *v);
void ATOMIC_ADD(ATOMIC_T *v, int i);
void ATOMIC_SUB(ATOMIC_T *v, int i);
void ATOMIC_INC(ATOMIC_T *v);
void ATOMIC_DEC(ATOMIC_T *v);
int ATOMIC_ADD_RETURN(ATOMIC_T *v, int i);
int ATOMIC_SUB_RETURN(ATOMIC_T *v, int i);
int ATOMIC_INC_RETURN(ATOMIC_T *v);
int ATOMIC_DEC_RETURN(ATOMIC_T *v);

/* File operation APIs, just for linux now */
int rtw_is_file_readable(char *path);
int rtw_retrive_from_file(char *path, u8* buf, u32 sz);
int rtw_store_to_file(char *path, u8* buf, u32 sz);


struct rtw_netdev_priv_indicator {
	void *priv;
	u32 sizeof_priv;
};
struct net_device *rtw_alloc_etherdev_with_old_priv(int sizeof_priv, void *old_priv);
struct net_device * rtw_alloc_etherdev(int sizeof_priv);

#define rtw_netdev_priv(netdev) ( ((struct rtw_netdev_priv_indicator *)netdev_priv(netdev))->priv )

void rtw_free_netdev(struct net_device * netdev);

#define NDEV_FMT "%s"
#define NDEV_ARG(ndev) ndev->name
#define ADPT_FMT "%s"
#define ADPT_ARG(adapter) adapter->pnetdev->name
#define FUNC_NDEV_FMT "%s(%s)"
#define FUNC_NDEV_ARG(ndev) __func__, ndev->name
#define FUNC_ADPT_FMT "%s(%s)"
#define FUNC_ADPT_ARG(adapter) __func__, adapter->pnetdev->name

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
#define rtw_signal_process(pid, sig) kill_pid(find_vpid((pid)),(sig), 1)
#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)) */
#define rtw_signal_process(pid, sig) kill_proc((pid), (sig), 1)
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)) */

u64 rtw_modular64(u64 x, u64 y);
u64 rtw_division64(u64 x, u64 y);


/* Macros for handling unaligned memory accesses */

#define RTW_GET_BE16(a) ((u16) (((a)[0] << 8) | (a)[1]))
#define RTW_PUT_BE16(a, val)			\
	do {					\
		(a)[0] = ((u16) (val)) >> 8;	\
		(a)[1] = ((u16) (val)) & 0xff;	\
	} while (0)

#define RTW_GET_LE16(a) ((u16) (((a)[1] << 8) | (a)[0]))
#define RTW_PUT_LE16(a, val)			\
	do {					\
		(a)[1] = ((u16) (val)) >> 8;	\
		(a)[0] = ((u16) (val)) & 0xff;	\
	} while (0)

#define RTW_GET_BE24(a) ((((u32) (a)[0]) << 16) | (((u32) (a)[1]) << 8) | \
			 ((u32) (a)[2]))
#define RTW_PUT_BE24(a, val)					\
	do {							\
		(a)[0] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[2] = (u8) (((u32) (val)) & 0xff);		\
	} while (0)

#define RTW_GET_BE32(a) ((((u32) (a)[0]) << 24) | (((u32) (a)[1]) << 16) | \
			 (((u32) (a)[2]) << 8) | ((u32) (a)[3]))
#define RTW_PUT_BE32(a, val)					\
	do {							\
		(a)[0] = (u8) ((((u32) (val)) >> 24) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[2] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[3] = (u8) (((u32) (val)) & 0xff);		\
	} while (0)

#define RTW_GET_LE32(a) ((((u32) (a)[3]) << 24) | (((u32) (a)[2]) << 16) | \
			 (((u32) (a)[1]) << 8) | ((u32) (a)[0]))
#define RTW_PUT_LE32(a, val)					\
	do {							\
		(a)[3] = (u8) ((((u32) (val)) >> 24) & 0xff);	\
		(a)[2] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[0] = (u8) (((u32) (val)) & 0xff);		\
	} while (0)

#define RTW_GET_BE64(a) ((((u64) (a)[0]) << 56) | (((u64) (a)[1]) << 48) | \
			 (((u64) (a)[2]) << 40) | (((u64) (a)[3]) << 32) | \
			 (((u64) (a)[4]) << 24) | (((u64) (a)[5]) << 16) | \
			 (((u64) (a)[6]) << 8) | ((u64) (a)[7]))
#define RTW_PUT_BE64(a, val)				\
	do {						\
		(a)[0] = (u8) (((u64) (val)) >> 56);	\
		(a)[1] = (u8) (((u64) (val)) >> 48);	\
		(a)[2] = (u8) (((u64) (val)) >> 40);	\
		(a)[3] = (u8) (((u64) (val)) >> 32);	\
		(a)[4] = (u8) (((u64) (val)) >> 24);	\
		(a)[5] = (u8) (((u64) (val)) >> 16);	\
		(a)[6] = (u8) (((u64) (val)) >> 8);	\
		(a)[7] = (u8) (((u64) (val)) & 0xff);	\
	} while (0)

#define RTW_GET_LE64(a) ((((u64) (a)[7]) << 56) | (((u64) (a)[6]) << 48) | \
			 (((u64) (a)[5]) << 40) | (((u64) (a)[4]) << 32) | \
			 (((u64) (a)[3]) << 24) | (((u64) (a)[2]) << 16) | \
			 (((u64) (a)[1]) << 8) | ((u64) (a)[0]))

void rtw_buf_free(u8 **buf, u32 *buf_len);
void rtw_buf_update(u8 **buf, u32 *buf_len, u8 *src, u32 src_len);

struct rtw_cbuf {
	u32 write;
	u32 read;
	u32 size;
	void *bufs[0];
};

bool rtw_cbuf_full(struct rtw_cbuf *cbuf);
bool rtw_cbuf_empty(struct rtw_cbuf *cbuf);
bool rtw_cbuf_push(struct rtw_cbuf *cbuf, void *buf);
void *rtw_cbuf_pop(struct rtw_cbuf *cbuf);
struct rtw_cbuf *rtw_cbuf_alloc(u32 size);
void rtw_cbuf_free(struct rtw_cbuf *cbuf);

#endif
