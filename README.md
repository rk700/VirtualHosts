VirtualHosts
------------

# Introduction
This is a hook plugin used by [VirtualHook](https://github.com/rk700/VirtualHook). It [hooks](https://github.com/rk700/VirtualHosts/blob/master/app/src/main/jni/hookHosts.c) `android_getaddrinfofornetcontext()` and set DNS query results to predefined ones.

# Usage
Build and push the apk to `/sdcard/`. Then add the hook plugin and start applications in VirtualHook.
