IMPORTANT - PLEASE READ:

Beginning on November 4, 2019, I will NO LONGER support people that have downloaded the source
as a zip file. Using git has much more flexibility. In addition, there is much less liklihood
that a user will contact me with a problem that is ALREADY fixed.

If your system says that /lib/modules/...../build does not exist, you have not
installed the kernel headers, you have done it incorrectly, or you are not running
the kernel for which the headers have been installed. The necessary steps are
dependent on which distro you are using. Creating a new issue and asking at
GitHub will not be productive.

Your kernel configuration MUST have CONFIG_WIRELESS_EXT set.

Unsolicited E-mail sent to my private address will be ignored!!

If a build fails that previously worked, perform a 'git pull' and retry before
reporting a problem. As noted, if you had downloaded the source in zip form, then you would
need to get an entirely new source file. That is why using git, which downloads only the changed
lines, is required.

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

