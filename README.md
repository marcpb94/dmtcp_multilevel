# [DMTCP: Distributed MultiThreaded CheckPointing](http://dmtcp.sourceforge.net/) [![Build Status](https://travis-ci.org/dmtcp/dmtcp.png?branch=master)](https://travis-ci.org/dmtcp/dmtcp)

DMTCP is a tool to transparently checkpoint the state of multiple simultaneous
applications, including multi-threaded and distributed applications. It
operates directly on the user binary executable, without any Linux kernel
modules or other kernel modifications.

Among the applications supported by DMTCP are MPI (various implementations),
OpenMP, MATLAB, Python, Perl, R, and many programming languages and shell
scripting languages. DMTCP also supports GNU screen sessions, including
vim/cscope and emacs. With the use of TightVNC, it can also checkpoint
and restart X Window applications.  For a multilib (mixture of 32-
and 64-bit processes), see "./configure --enable-multilib".

DMTCP supports the commonly used OFED API for InfiniBand, as well as its
integration with various implementations of MPI, and resource managers
(e.g., SLURM).

To install DMTCP, see [INSTALL.md](INSTALL.md).

For an overview DMTCP, see [QUICK-START.md](QUICK-START.md).

For the license, see [COPYING](COPYING).

For more information on DMTCP, see: [http://dmtcp.sourceforge.net](http://dmtcp.sourceforge.net).

For the latest version of DMTCP (both official release and git), see:
[http://dmtcp.sourceforge.net/downloads.html](http://dmtcp.sourceforge.net/downloads.html).

---

## DMTCP multi-level checkpoint extension

This work extends the DMTCP/Mana library with several levels of checkpointing.

Current progress implements three types of checkpoints: local storage checkpoint, local storage with partner copy and global storage checkpoint. Partner copy consists in copying the checkpoint to another rank from a different node, so that it can be recovered in the event of corruption/file loss. We support checkpoint intervals for all checkpoint levels concurrently, prioritizing higher level checkpoints in the event of frequency collision. We also implement a method for automatically deciding from which type of checkpoint the application should recover. Each checkpoint file is created along with a checksum, which is used to verify the integrity of the checkpoint data before using it for recovery.

In order to decide how to perform the checkpoint copy, the topology of the execution (number of nodes, ranks per node, etc.) is needed. In order to test the library in a local machine, test mode can be used, which simulates a fake topology.

We make use of a configuration file in order to make the customization of DMTCP/Mana options easier. We currently support the following options:

- Local checkpoint location
- Global checkpoint location
- Local checkpoint interval
- Global checkpoint interval
- Partner copy checkpoint interval
- Test mode

An example of the configuration file is found [here](templates/mana.conf).
