package com.samychen.gracefulwrapper.forkservice;

/**
 * Created by samychen on 2017/8/4 0004.
 * 我的github地址 https://github.com/samychen
 */

public class Watcher {
    public native void createWatcher(String pid);
    public native void connectMonitor();
}
