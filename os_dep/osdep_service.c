/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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


#define _OSDEP_SERVICE_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <recv_osdep.h>
#include <rtw_ioctl_set.h>
#include <linux/vmalloc.h>

#define RT_TAG	'1178'

/*
* Translate the OS dependent @param error_code to OS independent RTW_STATUS_CODE
* @return: one of RTW_STATUS_CODE
*/
inline int RTW_STATUS_CODE(int error_code) {
	if (error_code >=0)
		return _SUCCESS;

	switch (error_code) {
		/* case -ETIMEDOUT: */
		/* 	return RTW_STATUS_TIMEDOUT; */
		default:
			return _FAIL;
	}
}

u32 rtw_atoi(u8* s)
{

	int num=0,flag=0;
	int i;
	for (i=0;i<=strlen(s);i++) {
		if (s[i] >= '0' && s[i] <= '9')
			num = num * 10 + s[i] -'0';
		else if (s[0] == '-' && i== 0)
			flag =1;
		else
			break;
	 }

	if (flag == 1)
		num = num * -1;

	return num;
}

inline u8* _rtw_vmalloc(u32 sz)
{
	u8	*pbuf;
	pbuf = vmalloc(sz);
	return pbuf;
}

inline u8* _rtw_zvmalloc(u32 sz)
{
	u8	*pbuf;
	pbuf = _rtw_vmalloc(sz);
	if (pbuf != NULL)
		memset(pbuf, 0, sz);
	return pbuf;
}

inline void _rtw_vmfree(u8 *pbuf, u32 sz)
{
	vfree(pbuf);
}

u8* _rtw_malloc(u32 sz)
{

	u8	*pbuf= NULL;

	pbuf = kmalloc(sz,in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);

	return pbuf;
}


u8* _rtw_zmalloc(u32 sz)
{
	u8	*pbuf = _rtw_malloc(sz);

	if (pbuf != NULL) {

		memset(pbuf, 0, sz);
	}

	return pbuf;
}

void	_rtw_mfree(u8 *pbuf, u32 sz)
{

	kfree(pbuf);
}

inline struct sk_buff *_rtw_skb_alloc(u32 sz)
{
	return __dev_alloc_skb(sz, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

inline void _rtw_skb_free(struct sk_buff *skb)
{
	dev_kfree_skb_any(skb);
}

inline struct sk_buff *_rtw_skb_copy(const struct sk_buff *skb)
{
	return skb_copy(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

inline struct sk_buff *_rtw_skb_clone(struct sk_buff *skb)
{
	return skb_clone(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

inline int _rtw_netif_rx(struct  net_device * ndev, struct sk_buff *skb)
{
	skb->dev = ndev;
	return netif_rx(skb);
}

void _rtw_skb_queue_purge(struct sk_buff_head *list)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(list)) != NULL)
		_rtw_skb_free(skb);
}

inline void *_rtw_usb_buffer_alloc(struct usb_device *dev, size_t size, dma_addr_t *dma)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	return usb_alloc_coherent(dev, size, (in_interrupt() ? GFP_ATOMIC : GFP_KERNEL), dma);
#else
	return usb_buffer_alloc(dev, size, (in_interrupt() ? GFP_ATOMIC : GFP_KERNEL), dma);
#endif
}

inline void _rtw_usb_buffer_free(struct usb_device *dev, size_t size, void *addr, dma_addr_t dma)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	usb_free_coherent(dev, size, addr, dma);
#else
	usb_buffer_free(dev, size, addr, dma);
#endif
}
#ifdef DBG_MEM_ALLOC

struct rtw_mem_stat {
	ATOMIC_T alloc; /*  the memory bytes we allocate currently */
	ATOMIC_T peak; /*  the peak memory bytes we allocate */
	ATOMIC_T alloc_cnt; /*  the alloc count for alloc currently */
	ATOMIC_T alloc_err_cnt; /*  the error times we fail to allocate memory */
};

struct rtw_mem_stat rtw_mem_type_stat[mstat_tf_idx(MSTAT_TYPE_MAX)];
struct rtw_mem_stat rtw_mem_func_stat[mstat_ff_idx(MSTAT_FUNC_MAX)];

char *MSTAT_TYPE_str[] = {
	"VIR",
	"PHY",
	"SKB",
	"USB",
};

char *MSTAT_FUNC_str[] = {
	"UNSP",
	"IO",
	"TXIO",
	"RXIO",
	"TX",
	"RX",
};

int _rtw_mstat_dump(char *buf, int len)
{
	int cnt = 0;
	int i;
	int value_t[4][mstat_tf_idx(MSTAT_TYPE_MAX)];
	int value_f[4][mstat_ff_idx(MSTAT_FUNC_MAX)];

	int vir_alloc, vir_peak, vir_alloc_err, phy_alloc, phy_peak, phy_alloc_err;
	int tx_alloc, tx_peak, tx_alloc_err, rx_alloc, rx_peak, rx_alloc_err;

	for (i=0;i<mstat_tf_idx(MSTAT_TYPE_MAX);i++) {
		value_t[0][i] = ATOMIC_READ(&rtw_mem_type_stat[i].alloc);
		value_t[1][i] = ATOMIC_READ(&rtw_mem_type_stat[i].peak);
		value_t[2][i] = ATOMIC_READ(&rtw_mem_type_stat[i].alloc_cnt);
		value_t[3][i] = ATOMIC_READ(&rtw_mem_type_stat[i].alloc_err_cnt);
	}
	cnt += snprintf(buf+cnt, len-cnt, "===================== MSTAT =====================\n");
	cnt += snprintf(buf+cnt, len-cnt, "%4s %10s %10s %10s %10s\n", "TAG", "alloc", "peak", "aloc_cnt", "err_cnt");
	cnt += snprintf(buf+cnt, len-cnt, "-------------------------------------------------\n");
	for (i=0;i<mstat_tf_idx(MSTAT_TYPE_MAX);i++) {
		cnt += snprintf(buf+cnt, len-cnt, "%4s %10d %10d %10d %10d\n", MSTAT_TYPE_str[i], value_t[0][i], value_t[1][i], value_t[2][i], value_t[3][i]);
	}
	return cnt;
}

void rtw_mstat_dump(void)
{
	char buf[768] = {0};

	_rtw_mstat_dump(buf, 768);
	DBG_88E("\n%s", buf);
}

void rtw_mstat_update(const enum mstat_f flags, const MSTAT_STATUS status, u32 sz)
{
	static u32 update_time = 0;
	int peak, alloc;
	int i;

	/* initialization */
	if (!update_time) {
		for (i=0;i<mstat_tf_idx(MSTAT_TYPE_MAX);i++) {
			ATOMIC_SET(&rtw_mem_type_stat[i].alloc, 0);
			ATOMIC_SET(&rtw_mem_type_stat[i].peak, 0);
			ATOMIC_SET(&rtw_mem_type_stat[i].alloc_cnt, 0);
			ATOMIC_SET(&rtw_mem_type_stat[i].alloc_err_cnt, 0);
		}
		for (i=0;i<mstat_ff_idx(MSTAT_FUNC_MAX);i++) {
			ATOMIC_SET(&rtw_mem_func_stat[i].alloc, 0);
			ATOMIC_SET(&rtw_mem_func_stat[i].peak, 0);
			ATOMIC_SET(&rtw_mem_func_stat[i].alloc_cnt, 0);
			ATOMIC_SET(&rtw_mem_func_stat[i].alloc_err_cnt, 0);
		}
	}

	switch (status) {
		case MSTAT_ALLOC_SUCCESS:
			ATOMIC_INC(&rtw_mem_type_stat[mstat_tf_idx(flags].alloc_cnt));
			alloc = ATOMIC_ADD_RETURN(&rtw_mem_type_stat[mstat_tf_idx(flags].alloc), sz);
			peak=ATOMIC_READ(&rtw_mem_type_stat[mstat_tf_idx(flags].peak));
			if (peak<alloc)
				ATOMIC_SET(&rtw_mem_type_stat[mstat_tf_idx(flags].peak), alloc);

			ATOMIC_INC(&rtw_mem_func_stat[mstat_ff_idx(flags].alloc_cnt));
			alloc = ATOMIC_ADD_RETURN(&rtw_mem_func_stat[mstat_ff_idx(flags].alloc), sz);
			peak=ATOMIC_READ(&rtw_mem_func_stat[mstat_ff_idx(flags].peak));
			if (peak<alloc)
				ATOMIC_SET(&rtw_mem_func_stat[mstat_ff_idx(flags].peak), alloc);
			break;

		case MSTAT_ALLOC_FAIL:
			ATOMIC_INC(&rtw_mem_type_stat[mstat_tf_idx(flags].alloc_err_cnt));

			ATOMIC_INC(&rtw_mem_func_stat[mstat_ff_idx(flags].alloc_err_cnt));
			break;

		case MSTAT_FREE:
			ATOMIC_DEC(&rtw_mem_type_stat[mstat_tf_idx(flags].alloc_cnt));
			ATOMIC_SUB(&rtw_mem_type_stat[mstat_tf_idx(flags].alloc), sz);

			ATOMIC_DEC(&rtw_mem_func_stat[mstat_ff_idx(flags].alloc_cnt));
			ATOMIC_SUB(&rtw_mem_func_stat[mstat_ff_idx(flags].alloc), sz);
			break;
	};

	/* if (rtw_get_passing_time_ms(update_time) > 5000) { */
	/* 	rtw_mstat_dump(); */
		update_time=jiffies;
	/*  */
}



inline u8* dbg_rtw_vmalloc(u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	u8  *p;
	/* DBG_88E("DBG_MEM_ALLOC %s:%d %s(%d)\n", func,  line, __FUNCTION__, (sz)); */

	p=_rtw_vmalloc((sz));

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, sz
	);

	return p;
}

inline u8* dbg_rtw_zvmalloc(u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	u8 *p;
	/* DBG_88E("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz)); */

	p=_rtw_zvmalloc((sz));

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, sz
	);

	return p;
}

inline void dbg_rtw_vmfree(u8 *pbuf, u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	/* DBG_88E("DBG_MEM_ALLOC %s:%d %s(%p,%d)\n",  func, line, __FUNCTION__, (pbuf), (sz)); */

	_rtw_vmfree((pbuf), (sz));

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, sz
	);
}

inline u8* dbg_rtw_malloc(u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	u8 *p;

	/* if (sz>=153 && sz<=306) */
	/* 	DBG_88E("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz)); */

	/* if ((sz)>4096) */
	/* 	DBG_88E("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz)); */

	p=_rtw_malloc((sz));

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, sz
	);

	return p;
}

inline u8* dbg_rtw_zmalloc(u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	u8 *p;

	/* if (sz>=153 && sz<=306) */
	/* 	DBG_88E("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz)); */

	/* if ((sz)>4096) */
	/* 	DBG_88E("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz)); */

	p = _rtw_zmalloc((sz));

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, sz
	);

	return p;
}

inline void dbg_rtw_mfree(u8 *pbuf, u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	/* if (sz>=153 && sz<=306) */
	/* 	DBG_88E("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz)); */

	/* if ((sz)>4096) */
	/* 	DBG_88E("DBG_MEM_ALLOC %s:%d %s(%p,%d)\n", func, line, __FUNCTION__, (pbuf), (sz)); */

	_rtw_mfree((pbuf), (sz));

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, sz
	);
}

inline struct sk_buff * dbg_rtw_skb_alloc(unsigned int size, const enum mstat_f flags, const char *func, int line)
{
	struct sk_buff *skb;
	unsigned int truesize = 0;

	skb = _rtw_skb_alloc(size);

	if (skb)
		truesize = skb->truesize;

	if (!skb || truesize < size /*|| size > 4096*/)
		DBG_88E("DBG_MEM_ALLOC %s:%d %s(%d), skb:%p, truesize=%u\n", func, line, __FUNCTION__, size, skb, truesize);

	rtw_mstat_update(
		flags
		, skb ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, truesize
	);

	return skb;
}

inline void dbg_rtw_skb_free(struct sk_buff *skb, const enum mstat_f flags, const char *func, int line)
{
	unsigned int truesize = skb->truesize;

	/* if (truesize > 4096) */
	/* 	DBG_88E("DBG_MEM_ALLOC %s:%d %s, truesize=%u\n", func, line, __FUNCTION__, truesize); */

	_rtw_skb_free(skb);

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, truesize
	);
}

inline struct sk_buff *dbg_rtw_skb_copy(const struct sk_buff *skb, const enum mstat_f flags, const char *func, const int line)
{
	struct sk_buff *skb_cp;
	unsigned int truesize = skb->truesize;
	unsigned int cp_truesize = 0;

	skb_cp = _rtw_skb_copy(skb);
	if (skb_cp)
		cp_truesize = skb_cp->truesize;

	if (!skb_cp || cp_truesize != truesize /*||cp_truesize > 4096*/)
		DBG_88E("DBG_MEM_ALLOC %s:%d %s(%u), skb_cp:%p, cp_truesize=%u\n", func, line, __FUNCTION__, truesize, skb_cp, cp_truesize);

	rtw_mstat_update(
		flags
		, skb_cp ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, truesize
	);

	return skb_cp;
}

inline struct sk_buff *dbg_rtw_skb_clone(struct sk_buff *skb, const enum mstat_f flags, const char *func, const int line)
{
	struct sk_buff *skb_cl;
	unsigned int truesize = skb->truesize;
	unsigned int cl_truesize = 0;

	skb_cl = _rtw_skb_clone(skb);
	if (skb_cl)
		cl_truesize = skb_cl->truesize;

	if (!skb_cl || cl_truesize != truesize /*|| cl_truesize > 4096*/)
		DBG_88E("DBG_MEM_ALLOC %s:%d %s(%u), skb_cl:%p, cl_truesize=%u\n", func, line, __FUNCTION__, truesize, skb_cl, cl_truesize);

	rtw_mstat_update(
		flags
		, skb_cl ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, truesize
	);

	return skb_cl;
}

inline int dbg_rtw_netif_rx(struct  net_device * ndev, struct sk_buff *skb, const enum mstat_f flags, const char *func, int line)
{
	int ret;
	unsigned int truesize = skb->truesize;

	/* if (truesize > 4096) */
	/* 	DBG_88E("DBG_MEM_ALLOC %s:%d %s, truesize=%u\n", func, line, __FUNCTION__, truesize); */

	ret = _rtw_netif_rx(ndev, skb);

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, truesize
	);

	return ret;
}

inline void dbg_rtw_skb_queue_purge(struct sk_buff_head *list, enum mstat_f flags, const char *func, int line)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(list)) != NULL)
		dbg_rtw_skb_free(skb, flags, func, line);
}

inline void *dbg_rtw_usb_buffer_alloc(struct usb_device *dev, size_t size, dma_addr_t *dma, const enum mstat_f flags, const char *func, int line)
{
	void *p;
	/* DBG_88E("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, size); */

	p = _rtw_usb_buffer_alloc(dev, size, dma);

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, size
	);

	return p;
}

inline void dbg_rtw_usb_buffer_free(struct usb_device *dev, size_t size, void *addr, dma_addr_t dma, const enum mstat_f flags, const char *func, int line)
{
	/* DBG_88E("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, size); */

	_rtw_usb_buffer_free(dev, size, addr, dma);

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, size
	);
}
#endif /* DBG_MEM_ALLOC */

void* rtw_malloc2d(int h, int w, int size)
{
	int j;

	void **a = (void **) rtw_zmalloc( h*sizeof(void *) + h*w*size );
	if (a == NULL)
	{
		DBG_88E("%s: alloc memory fail!\n", __FUNCTION__);
		return NULL;
	}

	for ( j=0; j<h; j++ )
		a[j] = ((char *)(a+h)) + j*w*size;

	return a;
}

void rtw_mfree2d(void *pbuf, int h, int w, int size)
{
	rtw_mfree((u8 *)pbuf, h*sizeof(void*) + w*h*size);
}

int	_rtw_memcmp(void *dst, void *src, u32 sz)
{
/* under Linux/GNU/GLibc, the return value of memcmp for two same mem. chunk is 0 */
	if (!(memcmp(dst, src, sz)))
		return true;
	else
		return false;
}

void _rtw_init_listhead(struct list_head *list)
{
        INIT_LIST_HEAD(list);
}


/*
For the following list_xxx operations,
caller must guarantee the atomic context.
Otherwise, there will be racing condition.
*/
u32	rtw_is_list_empty(struct list_head *phead)
{
	if (list_empty(phead))
		return true;
	else
		return false;
}

void rtw_list_insert_head(struct list_head *plist, struct list_head *phead)
{
	list_add(plist, phead);
}

void rtw_list_insert_tail(struct list_head *plist, struct list_head *phead)
{
	list_add_tail(plist, phead);
}


/*

Caller must check if the list is empty before calling rtw_list_delete

*/


void _rtw_init_sema(struct  semaphore *sema, int init_val)
{
	sema_init(sema, init_val);
}

void _rtw_free_sema(struct  semaphore *sema)
{
}

void _rtw_up_sema(struct  semaphore *sema)
{
	up(sema);
}

u32 _rtw_down_sema(struct  semaphore *sema)
{
	if (down_interruptible(sema))
		return _FAIL;
	else
		return _SUCCESS;
}



void	_rtw_mutex_init(_mutex *pmutex)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	mutex_init(pmutex);
#else
	init_MUTEX(pmutex);
#endif
}

void	_rtw_mutex_free(_mutex *pmutex)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	mutex_destroy(pmutex);
#endif
}

void	_rtw_init_queue(struct  __queue *pqueue)
{

	_rtw_init_listhead(&pqueue->queue);

	spin_lock_init(&pqueue->lock);

}

u32	  _rtw_queue_empty(struct  __queue *pqueue)
{
	return (rtw_is_list_empty(&pqueue->queue));
}


u32 rtw_end_of_queue_search(struct list_head *head, struct list_head *plist)
{
	if (head == plist)
		return true;
	else
		return false;
}


inline u32 rtw_systime_to_ms(u32 systime)
{
	return systime * 1000 / HZ;
}

inline u32 rtw_ms_to_systime(u32 ms)
{
	return ms * HZ / 1000;
}

/*  the input parameter start use jiffies */
inline s32 rtw_get_passing_time_ms(u32 start)
{
	return rtw_systime_to_ms(jiffies-start);
}

inline s32 rtw_get_time_interval_ms(u32 start, u32 end)
{
	return rtw_systime_to_ms(end-start);
}


void rtw_sleep_schedulable(int ms)
{
    u32 delta;

    delta = (ms * HZ)/1000;/* ms) */
    if (delta == 0) {
        delta = 1;/*  1 ms */
    }
    set_current_state(TASK_INTERRUPTIBLE);
    if (schedule_timeout(delta) != 0) {
        return ;
    }
}


void rtw_msleep_os(int ms)
{
	msleep((unsigned int)ms);
}

void rtw_usleep_os(int us)
{
      if ( 1 < (us/1000) )
                msleep(1);
      else
		msleep( (us/1000) + 1);
}


#ifdef DBG_DELAY_OS
void _rtw_mdelay_os(int ms, const char *func, const int line)
{
	DBG_88E("%s:%d %s(%d)\n", func, line, __FUNCTION__, ms);
	mdelay((unsigned long)ms);
}

void _rtw_udelay_os(int us, const char *func, const int line)
{
	DBG_88E("%s:%d %s(%d)\n", func, line, __FUNCTION__, us);
	udelay((unsigned long)us);
}
#else
void rtw_mdelay_os(int ms)
{
	mdelay((unsigned long)ms);
}
void rtw_udelay_os(int us)
{
      udelay((unsigned long)us);
}
#endif

void rtw_yield_os(void)
{
	yield();
}

#define RTW_SUSPEND_LOCK_NAME "rtw_wifi"
#define RTW_SUSPEND_EXT_LOCK_NAME "rtw_wifi_ext"


inline void rtw_suspend_lock_init(void)
{
}

inline void rtw_suspend_lock_uninit(void)
{
}

inline void rtw_lock_suspend(void)
{
}

inline void rtw_unlock_suspend(void)
{
}

inline void rtw_lock_suspend_timeout(u32 timeout_ms)
{
}

inline void rtw_lock_ext_suspend_timeout(u32 timeout_ms)
{
}

inline void ATOMIC_SET(ATOMIC_T *v, int i)
{
	atomic_set(v,i);
}

inline int ATOMIC_READ(ATOMIC_T *v)
{
	return atomic_read(v);
}

inline void ATOMIC_ADD(ATOMIC_T *v, int i)
{
	atomic_add(i,v);
}
inline void ATOMIC_SUB(ATOMIC_T *v, int i)
{
	atomic_sub(i,v);
}

inline void ATOMIC_INC(ATOMIC_T *v)
{
	atomic_inc(v);
}

inline void ATOMIC_DEC(ATOMIC_T *v)
{
	atomic_dec(v);
}

inline int ATOMIC_ADD_RETURN(ATOMIC_T *v, int i)
{
	return atomic_add_return(i,v);
}

inline int ATOMIC_SUB_RETURN(ATOMIC_T *v, int i)
{
	return atomic_sub_return(i,v);
}

inline int ATOMIC_INC_RETURN(ATOMIC_T *v)
{
	return atomic_inc_return(v);
}

inline int ATOMIC_DEC_RETURN(ATOMIC_T *v)
{
	return atomic_dec_return(v);
}


/*
* Open a file with the specific @param path, @param flag, @param mode
* @param fpp the pointer of struct file pointer to get struct file pointer while file opening is success
* @param path the path of the file to open
* @param flag file operation flags, please refer to linux document
* @param mode please refer to linux document
* @return Linux specific error code
*/
static int openFile(struct file **fpp, char *path, int flag, int mode)
{
	struct file *fp;

	fp=filp_open(path, flag, mode);
	if (IS_ERR(fp)) {
		*fpp= NULL;
		return PTR_ERR(fp);
	}
	else {
		*fpp=fp;
		return 0;
	}
}

/*
* Close the file with the specific @param fp
* @param fp the pointer of struct file to close
* @return always 0
*/
static int closeFile(struct file *fp)
{
	filp_close(fp,NULL);
	return 0;
}

static int readFile(struct file *fp,char *buf,int len)
{
	int rlen=0, sum=0;

	if (!fp->f_op || !fp->f_op->read)
		return -EPERM;

	while (sum<len) {
		rlen=fp->f_op->read(fp,(char __user *)buf+sum,len-sum, &fp->f_pos);
		if (rlen>0)
			sum+=rlen;
		else if (0 != rlen)
			return rlen;
		else
			break;
	}

	return  sum;

}

static int writeFile(struct file *fp,char *buf,int len)
{
	int wlen=0, sum=0;

	if (!fp->f_op || !fp->f_op->write)
		return -EPERM;

	while (sum<len) {
		wlen=fp->f_op->write(fp,(char __user *)buf+sum,len-sum, &fp->f_pos);
		if (wlen>0)
			sum+=wlen;
		else if (0 != wlen)
			return wlen;
		else
			break;
	}

	return sum;

}

/*
* Test if the specifi @param path is a file and readable
* @param path the path of the file to test
* @return Linux specific error code
*/
static int isFileReadable(char *path)
{
	struct file *fp;
	int ret = 0;
#ifdef set_fs
	mm_segment_t oldfs;
#endif
	char buf;

	fp=filp_open(path, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		ret = PTR_ERR(fp);
	}
	else {
#ifdef set_fs
		oldfs = get_fs(); set_fs(KERNEL_DS);
#endif

		if (1!=readFile(fp, &buf, 1))
			ret = PTR_ERR(fp);

#ifdef set_fs
		set_fs(oldfs);
#endif
		filp_close(fp,NULL);
	}
	return ret;
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read, or Linux specific error code
*/
static int retriveFromFile(char *path, u8* buf, u32 sz)
{
	int ret =-1;
	struct file *fp;

	if (path && buf) {
		if ( 0 == (ret=openFile(&fp,path, O_RDONLY, 0)) ) {
			DBG_88E("%s openFile path:%s fp=%p\n",__FUNCTION__, path ,fp);

#ifdef set_fs
			set_fs(KERNEL_DS);
#endif
			ret=readFile(fp, buf, sz);
			closeFile(fp);

			DBG_88E("%s readFile, ret:%d\n",__FUNCTION__, ret);

		} else {
			DBG_88E("%s openFile path:%s Fail, ret:%d\n",__FUNCTION__, path, ret);
		}
	} else {
		DBG_88E("%s NULL pointer\n",__FUNCTION__);
		ret =  -EINVAL;
	}
	return ret;
}

/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written, or Linux specific error code
*/
static int storeToFile(char *path, u8* buf, u32 sz)
{
	int ret =0;
	struct file *fp;

	if (path && buf) {
		if ( 0 == (ret=openFile(&fp, path, O_CREAT|O_WRONLY, 0666)) ) {
			DBG_88E("%s openFile path:%s fp=%p\n",__FUNCTION__, path ,fp);

#ifdef set_fs
			set_fs(KERNEL_DS);
#endif
			ret=writeFile(fp, buf, sz);
			closeFile(fp);

			DBG_88E("%s writeFile, ret:%d\n",__FUNCTION__, ret);

		} else {
			DBG_88E("%s openFile path:%s Fail, ret:%d\n",__FUNCTION__, path, ret);
		}
	} else {
		DBG_88E("%s NULL pointer\n",__FUNCTION__);
		ret =  -EINVAL;
	}
	return ret;
}

/*
* Test if the specifi @param path is a file and readable
* @param path the path of the file to test
* @return true or false
*/
int rtw_is_file_readable(char *path)
{
	if (isFileReadable(path) == 0)
		return true;
	else
		return false;
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read
*/
int rtw_retrive_from_file(char *path, u8* buf, u32 sz)
{
	int ret =retriveFromFile(path, buf, sz);
	return ret>=0?ret:0;
}

/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written
*/
int rtw_store_to_file(char *path, u8* buf, u32 sz)
{
	int ret =storeToFile(path, buf, sz);
	return ret>=0?ret:0;
}

struct net_device *rtw_alloc_etherdev_with_old_priv(int sizeof_priv, void *old_priv)
{
	struct net_device *pnetdev;
	struct rtw_netdev_priv_indicator *pnpi;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	pnetdev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 4);
#else
	pnetdev = alloc_etherdev(sizeof(struct rtw_netdev_priv_indicator));
#endif
	if (!pnetdev)
		goto RETURN;

	pnpi = netdev_priv(pnetdev);
	pnpi->priv=old_priv;
	pnpi->sizeof_priv=sizeof_priv;

RETURN:
	return pnetdev;
}

struct net_device *rtw_alloc_etherdev(int sizeof_priv)
{
	struct net_device *pnetdev;
	struct rtw_netdev_priv_indicator *pnpi;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	pnetdev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 4);
#else
	pnetdev = alloc_etherdev(sizeof(struct rtw_netdev_priv_indicator));
#endif
	if (!pnetdev)
		goto RETURN;

	pnpi = netdev_priv(pnetdev);

	pnpi->priv = rtw_zvmalloc(sizeof_priv);
	if (!pnpi->priv) {
		free_netdev(pnetdev);
		pnetdev = NULL;
		goto RETURN;
	}

	pnpi->sizeof_priv=sizeof_priv;
RETURN:
	return pnetdev;
}

void rtw_free_netdev(struct net_device * netdev)
{
	struct rtw_netdev_priv_indicator *pnpi;

	if (!netdev)
		goto RETURN;

	pnpi = netdev_priv(netdev);

	if (!pnpi->priv)
		goto RETURN;

	rtw_vmfree(pnpi->priv, pnpi->sizeof_priv);
	free_netdev(netdev);

RETURN:
	return;
}

/*
* Jeff: this function should be called under ioctl (rtnl_lock is accquired) while
* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
*/
int rtw_change_ifname(struct adapter *padapter, const char *ifname)
{
	struct net_device *pnetdev;
	struct net_device *cur_pnetdev;
	struct rereg_nd_name_data *rereg_priv;
	int ret;

	if (!padapter)
		goto error;

	cur_pnetdev = padapter->pnetdev;
	rereg_priv = &padapter->rereg_nd_name_priv;

	/* free the old_pnetdev */
	if (rereg_priv->old_pnetdev) {
		free_netdev(rereg_priv->old_pnetdev);
		rereg_priv->old_pnetdev = NULL;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	if (!rtnl_is_locked())
		unregister_netdev(cur_pnetdev);
	else
#endif
		unregister_netdevice(cur_pnetdev);

	rtw_proc_remove_one(cur_pnetdev);

	rereg_priv->old_pnetdev=cur_pnetdev;

	pnetdev = rtw_init_netdev(padapter);
	if (!pnetdev)  {
		ret = -1;
		goto error;
	}

	SET_NETDEV_DEV(pnetdev, dvobj_to_dev(adapter_to_dvobj(padapter)));

	rtw_init_netdev_name(pnetdev, ifname);

	memcpy(pnetdev->dev_addr, padapter->eeprompriv.mac_addr, ETH_ALEN);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	if (!rtnl_is_locked())
		ret = register_netdev(pnetdev);
	else
#endif
		ret = register_netdevice(pnetdev);

	if ( ret != 0) {
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("register_netdev() failed\n"));
		goto error;
	}

	rtw_proc_init_one(pnetdev);

	return 0;

error:

	return -1;

}

u64 rtw_modular64(u64 x, u64 y)
{
	return do_div(x, y);
}

u64 rtw_division64(u64 x, u64 y)
{
	do_div(x, y);
	return x;
}

void rtw_buf_free(u8 **buf, u32 *buf_len)
{
	u32 ori_len;

	if (!buf || !buf_len)
		return;

	ori_len = *buf_len;

	if (*buf) {
		u32 tmp_buf_len = *buf_len;
		*buf_len = 0;
		rtw_mfree(*buf, tmp_buf_len);
		*buf = NULL;
	}
}

void rtw_buf_update(u8 **buf, u32 *buf_len, u8 *src, u32 src_len)
{
	u32 ori_len = 0, dup_len = 0;
	u8 *ori = NULL;
	u8 *dup = NULL;

	if (!buf || !buf_len)
		return;

	if (!src || !src_len)
		goto keep_ori;

	/* duplicate src */
	dup = rtw_malloc(src_len);
	if (dup) {
		dup_len = src_len;
		memcpy(dup, src, dup_len);
	}

keep_ori:
	ori = *buf;
	ori_len = *buf_len;

	/* replace buf with dup */
	*buf_len = 0;
	*buf = dup;
	*buf_len = dup_len;

	/* free ori */
	if (ori && ori_len > 0)
		rtw_mfree(ori, ori_len);
}


/**
 * rtw_cbuf_full - test if cbuf is full
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Returns: true if cbuf is full
 */
inline bool rtw_cbuf_full(struct rtw_cbuf *cbuf)
{
	return (cbuf->write == cbuf->read-1)? true : false;
}

/**
 * rtw_cbuf_empty - test if cbuf is empty
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Returns: true if cbuf is empty
 */
inline bool rtw_cbuf_empty(struct rtw_cbuf *cbuf)
{
	return (cbuf->write == cbuf->read)? true : false;
}

/**
 * rtw_cbuf_push - push a pointer into cbuf
 * @cbuf: pointer of struct rtw_cbuf
 * @buf: pointer to push in
 *
 * Lock free operation, be careful of the use scheme
 * Returns: true push success
 */
bool rtw_cbuf_push(struct rtw_cbuf *cbuf, void *buf)
{
	if (rtw_cbuf_full(cbuf))
		return _FAIL;

	if (0)
		DBG_88E("%s on %u\n", __func__, cbuf->write);
	cbuf->bufs[cbuf->write] = buf;
	cbuf->write = (cbuf->write+1)%cbuf->size;

	return _SUCCESS;
}

/**
 * rtw_cbuf_pop - pop a pointer from cbuf
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Lock free operation, be careful of the use scheme
 * Returns: pointer popped out
 */
void *rtw_cbuf_pop(struct rtw_cbuf *cbuf)
{
	void *buf;
	if (rtw_cbuf_empty(cbuf))
		return NULL;

	if (0)
		DBG_88E("%s on %u\n", __func__, cbuf->read);
	buf = cbuf->bufs[cbuf->read];
	cbuf->read = (cbuf->read+1)%cbuf->size;

	return buf;
}

/**
 * rtw_cbuf_alloc - allocte a rtw_cbuf with given size and do initialization
 * @size: size of pointer
 *
 * Returns: pointer of srtuct rtw_cbuf, NULL for allocation failure
 */
struct rtw_cbuf *rtw_cbuf_alloc(u32 size)
{
	struct rtw_cbuf *cbuf;

	cbuf = (struct rtw_cbuf *)rtw_malloc(sizeof(*cbuf) + sizeof(void*)*size);

	if (cbuf) {
		cbuf->write = cbuf->read = 0;
		cbuf->size = size;
	}

	return cbuf;
}

/**
 * rtw_cbuf_free - free the given rtw_cbuf
 * @cbuf: pointer of struct rtw_cbuf to free
 */
void rtw_cbuf_free(struct rtw_cbuf *cbuf)
{
	rtw_mfree((u8*)cbuf, sizeof(*cbuf) + sizeof(void*)*cbuf->size);
}
