
#if DEV_BUS_TYPE == RT_USB_INTERFACE

#if defined(CONFIG_RTL8188E)
	#include "HalEfuseMask8188E_USB.h"
#endif

#if defined(CONFIG_RTL8812A)
	#include "HalEfuseMask8812A_USB.h"
#endif

#if defined(CONFIG_RTL8821A)
	#include "HalEfuseMask8821A_USB.h"
#endif

#if defined(CONFIG_RTL8192E)
	#include "HalEfuseMask8192E_USB.h"
#endif

#if defined(CONFIG_RTL8723B)
	#include "HalEfuseMask8723B_USB.h"
#endif

#if defined(CONFIG_RTL8814A)
	#include "HalEfuseMask8814A_USB.h"
#endif

#if defined(CONFIG_RTL8703B)
	#include "HalEfuseMask8703B_USB.h"
#endif

#if defined(CONFIG_RTL8723D)
	#include "HalEfuseMask8723D_USB.h"
#endif

#if defined(CONFIG_RTL8188F)
	#include "HalEfuseMask8188F_USB.h"
#endif

#if defined(CONFIG_RTL8822B)
	#include "HalEfuseMask8822B_USB.h"
#endif

#elif DEV_BUS_TYPE == RT_PCI_INTERFACE

#if defined(CONFIG_RTL8188E)
	#include "HalEfuseMask8188E_PCIE.h"
#endif

#if defined(CONFIG_RTL8812A)
	#include "HalEfuseMask8812A_PCIE.h"
#endif

#if defined(CONFIG_RTL8821A)
	#include "HalEfuseMask8821A_PCIE.h"
#endif

#if defined(CONFIG_RTL8192E)
	#include "HalEfuseMask8192E_PCIE.h"
#endif

#if defined(CONFIG_RTL8723B)
	#include "HalEfuseMask8723B_PCIE.h"
#endif

#if defined(CONFIG_RTL8814A)
	#include "HalEfuseMask8814A_PCIE.h"
#endif

#if defined(CONFIG_RTL8703B)
	#include "HalEfuseMask8703B_PCIE.h"
#endif

#if defined(CONFIG_RTL8822B)
	#include "HalEfuseMask8822B_PCIE.h"
#endif
#if defined(CONFIG_RTL8723D)
	#include "HalEfuseMask8723D_PCIE.h"
#endif

#elif DEV_BUS_TYPE == RT_SDIO_INTERFACE

#if defined(CONFIG_RTL8188E)
	#include "HalEfuseMask8188E_SDIO.h"
#endif

#if defined(CONFIG_RTL8703B)
	#include "HalEfuseMask8703B_SDIO.h"
#endif

#if defined(CONFIG_RTL8188F)
	#include "HalEfuseMask8188F_SDIO.h"
#endif

#if defined(CONFIG_RTL8723D)
	#include "HalEfuseMask8723D_SDIO.h"
#endif

#if defined(CONFIG_RTL8821C)
	#include "HalEfuseMask8821C_SDIO.h"
#endif

#if defined(CONFIG_RTL8822B)
	#include "HalEfuseMask8822B_SDIO.h"
#endif
#endif
