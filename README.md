plmtools
========

plmcat - reads data from the PLM and outputs in ASCII hexadecimal notation
plmsend - send hex data to PLM and wait for response or timeout
plmtty - interactive PLM communication program
insteon - example shell script to control lights (wrapper for plmsend)
x10toinst - example shell script to convert X10 messages into insteon

To use the insteon script, first create /etc/insteon.conf and customize
if for your devices.  See insteon.conf.example for an example.  After you
have created a config file, use

	insteon device_name command [argument]

such as
	insteon kitchen on 33

Supported commands are "on, off, bright, dim, ping, status, level".  Only
the on command takes an optional argument.  Level returns the current on
level on a scale of 0 to 100.  Future versions may revert to a scale of
0 to 255 because "insteon foo on 33; insteon foo level" returns 32, which
may complicate scripting.

The current insteon script allows you to specify a smaller number of ONLEVELS
for bright/dim commands than the default 32.  Since some signals may get
dropped, it may take an unacceptably long time to get to the desired
brightness.  You might want to set ONLEVELS to an odd number to prevent
having a level of exactly 50% which is the most wasteful of power.  Also,
having a large value for ONLEVELS would put several of them close to 50%.

To use the x10toinst script, first create /etc/x10toinst.conf and customize
it for your devices.  See x10toinst.conf.example for an example.  Run it as
you would any other script.

However, terminating x10toinst can leave an orphaned plmcat running that will
continue to run until new data comes in on the PLM.  x10toinst tries to kill
these plmcat processes, but it may miss some.

If you don't know bash, feel free to write your own scripts in the
lanugage of your preference.

Incidentally, none of these programs mind at all if you have an x10 
firecracker connected on the same port as the PLM.  Bottlerocket will
continue to work fine.

Try calling your customized insteon script from within a cron job.  I have
mine set up to automatically turn on several lights 30 minutes before
sunset with data retrieved from online.

Note: currently, plmtty will interfere with the operation of the other
programs and scripts.  It is intended to be used for debugging only.

Copyright (2008) by Matthew Randolph (mcr@umd.edu)
