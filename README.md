------下面是中文翻译   由yyw翻译-------
 rtl8188eu
=========

RTL8188EU单独的驱动代码

编译驱动 & 安装
---------
### 
编译这个驱动, 你要确保正确安装编译器. 除此之外,你必须要有linux编译头文件. ps:如果不理解这些，请自行查询
###编译流程

> make all

### 安装流程

> sudo make install

DKMS安装方式
---------
这个驱动模块也可以用 DKMS 来安装. 确保你已经争取安装 `dkms` 软件包

    sudo dkms add ./rtl8188eu
    sudo dkms build 8188eu/1.0
    sudo dkms install 8188eu/1.0

错误提交
---------


常问错误
---------

### The network manager says: "Device is not ready"!
Make sure you copied the firmware (rtl8188eufw.bin) to /lib/firmware/rtlwifi/ 

(翻译:network manager提示:Device is not ready     请确保你把固件rtl8188eufw.bin 复制到/lib/firmware/rtlwifi/目录) 


英文原版README.md
rtl8188eu
=========

Repository for the stand-alone RTL8188EU driver.

Compiling & Building
---------
### Dependencies
To compile the driver, you need to have make and a compiler installed. In addition,
you must have the kernel headers installed. If you do not understand what this means,
consult your distro.
### Compiling

> make all

### Installing

> sudo make install

DKMS
---------
The module can also be installed with DKMS. Make sure to install the `dkms` package first.

    sudo dkms add ./rtl8188eu
    sudo dkms build 8188eu/1.0
    sudo dkms install 8188eu/1.0

Submitting Issues
---------

Frequently asked Questions
---------

### The network manager says: "Device is not ready"!
Make sure you copied the firmware (rtl8188eufw.bin) to /lib/firmware/rtlwifi/


