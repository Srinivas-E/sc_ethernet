MII/MAC test application
========================

:scope: test application
:description: A test application to stress mii and mac layer
:keywords: ethernet, mac, mii, dc

A random traffic generator application which should be able to deliver line-rate traffic. 
> AVB system to generate all the AVB traffic 
> a generator application that simply inserts non-AVB traffic to stress the system utilizing the dual mii layer

Set-up and usage
> Use the PC for control messages (they will be forwarded through)
      PC  -> Traffic Generator -> AVB-DC -> AVB-DC -> AVB-LC end point

> Existing AVB python test framework may be used as a host controller.