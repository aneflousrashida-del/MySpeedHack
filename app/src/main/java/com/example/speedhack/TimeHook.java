package com.example.speedhack;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.callbacks.XC_LoadPackage.LoadPackageParam;

public class TimeHook implements IXposedHookLoadPackage {
    static {
        System.loadLibrary("speedhack");
    }
    @Override
    public void handleLoadPackage(LoadPackageParam lpparam) throws Throwable {
        if (lpparam.packageName.equals("com.android.systemui")) {
            return;
        }
    }
}
