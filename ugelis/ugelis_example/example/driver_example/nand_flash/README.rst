.. _hello_world:

Hello World
###########

Overview
********
A simple Hello World example that can be used with any supported board and
prints 'Hello World' to the console. This application can be built into modes:

* single thread
* multi threading

Building and Running
********************

This project outputs 'Hello World' to the console.  It can be built and executed
on QEMU as follows:

.. ugelis-app-commands::
   :ugelis-app: samples/nand_flash
   :host-os: unix
   :board: qemu_x86
   :goals: run
   :compact:

To build the single thread version, use the supplied configuration file for
single thread: :file:`prj_single.conf`:

.. ugelis-app-commands::
   :ugelis-app: samples/nand_flash
   :host-os: unix
   :board: qemu_x86
   :conf: prj_single.conf
   :goals: run
   :compact:

Sample Output
=============

.. code-block:: console

    Hello World! x86
