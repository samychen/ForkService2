package com.samychen.gracefulwrapper.forkservice;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.Process;
import android.support.annotation.Nullable;
import android.util.Log;

import java.util.Timer;
import java.util.TimerTask;

/**
 * Created by samychen on 2017/8/4 0004.
 * 我的github地址 https://github.com/samychen
 */

public class ForkService extends Service {
    int i = 0;
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d("tag", "onCreate: ");
        Watcher watcher = new Watcher();
        watcher.createWatcher(String.valueOf(Process.myUid()));
        watcher.connectMonitor();
        Timer timer = new Timer();
        timer.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                Log.d("tag", "服务开启中: "+i);
                i++;
            }
        },0,1000*3);
    }
}
