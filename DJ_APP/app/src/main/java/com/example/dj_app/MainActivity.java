package com.example.dj_app;

import android.net.MacAddress;
import android.os.ParcelUuid;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ToggleButton;
import android.widget.TextView;
import android.widget.ImageView;
import android.media.MediaPlayer;
import android.graphics.Bitmap;
import android.graphics.drawable.*;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.IntentFilter;
import android.bluetooth.*;
import android.content.*;
import android.os.Message;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;


import java.math.BigInteger;
import java.util.*;

import io.github.controlwear.virtual.joystick.android.JoystickView;

public class MainActivity extends AppCompatActivity {

    private final static int REQUEST_ENABLE_BT = 1;

    private static int[] display = new int[6];
    private static final int SENSOR1 = 0;
    private static final int SENSOR2 = 1;
    private static final int SENSOR3 = 2;
    private static final int SENSOR4 = 3;
    private static final int WHEEL1 = 4;
    private static final int WHEEL2 = 5;

    private static int[] joystickVal = new int[2];
    private static final int X = 0;
    private static final int Y = 1;

    private static final int ON = 1;
    private static final int OFF = 0;

    private static final int AUTO = 1;
    private static final int MAN = 0;

    private static int setMode = MAN;
    private static int state = OFF;
    private static String DEVICE_ADDR = "3c:15:c2:da:a4:0f";
    //private static String DEVICE_ADDR = "B8:27:EB:0B:DE:D9";

    BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    BluetoothDevice bluetoothDevice; // should be Pi
    OutputStream btOutputStream;
    InputStream btInputStream;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Toggle Button
        final ToggleButton power = (ToggleButton) findViewById(R.id.powerButton);
        final ToggleButton mode = (ToggleButton) findViewById(R.id.modeButton);

        // Joysticks
        final JoystickView joystick = (JoystickView) findViewById(R.id.joystickView);

        // Sensor Values
        final TextView sensor1 = (TextView) findViewById(R.id.sensor1);
        final TextView sensor2 = (TextView) findViewById(R.id.sensor2);
        final TextView sensor3 = (TextView) findViewById(R.id.sensor3);
        final TextView sensor4 = (TextView) findViewById(R.id.sensor4);

        // Motor Bar
        final ImageView wheel1 = (ImageView) findViewById(R.id.wheel1);
        final ImageView wheel2 = (ImageView) findViewById(R.id.wheel2);

        // Initial Progress Bar Bitmap
        final Bitmap bitmapOrg = ((BitmapDrawable) wheel1.getDrawable()).getBitmap();

        // Connect Button
        final Button connect = (Button) findViewById(R.id.bt);

        // Start up music
        final MediaPlayer mp = MediaPlayer.create(this, R.raw.startup);
        mp.start();
        mp.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            @Override
            public void onCompletion(MediaPlayer mp) {
                mp.release();
            }
        });


        // Wheels Init
        setMotor(bitmapOrg, wheel1, 0);
        setMotor(bitmapOrg, wheel2, 0);

        connect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                makeDiscoverable();
                boolean found = startSearching();
                try {
                    btOutputStream.write(0x65);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                if (found) {
                    // Dialog box
                    AlertDialog alertDialog = new AlertDialog.Builder(MainActivity.this).create();
                    alertDialog.setTitle("Alert");
                    alertDialog.setMessage(getResources().getString(R.string.success));
                    alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    dialog.dismiss();
                                }
                            });
                    alertDialog.show();
                }
            }
        });


        // Toggle Button Controls
        power.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                state = isChecked ? ON : OFF;
                mode.setEnabled(isChecked);
            }
        });

        mode.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                setMode = isChecked ? AUTO : MAN;
            }
        });

        // Joystick Controls
        joystick.setOnMoveListener(new JoystickView.OnMoveListener() {
            @Override
            public void onMove(int angle, int strength) {
                joystickVal[X] = (int) (strength * Math.cos(Math.toRadians(angle)));
                joystickVal[Y] = (int) (strength * Math.sin(Math.toRadians(angle)));
            }
        });

        /*
            changeText(sensor1, SENSOR1);
            changeText(sensor2, SENSOR2);
            changeText(sensor3, SENSOR3);
            changeText(sensor4, SENSOR4);
         */

        // Bluetooth controls

    }

    private void changeText(TextView tv, int value) {
        tv.setText(Integer.toString(value) + " cm");
        if (value < 10) {
            tv.setTextColor(getResources().getColor(R.color.warning));
        } else if (value < 17) {
            tv.setTextColor(getResources().getColor(R.color.caution));
        } else {
            tv.setTextColor(getResources().getColor(R.color.normal));
        }
    }

    private void setMotor(Bitmap bitmapOrg, ImageView iv, int percentage) {

        int newHeight = (int) (bitmapOrg.getHeight() * percentage / 100.0);
        int startingY = (int) (bitmapOrg.getHeight() * (1.0f - (percentage / 100.0)));

        if (newHeight <= 0) {
            newHeight = 1;
            startingY = 0;
        } else if (newHeight >= bitmapOrg.getHeight()) {
            newHeight = bitmapOrg.getHeight();
            startingY = 0;
        }

        Bitmap croppedBitmap = Bitmap.createBitmap(bitmapOrg, 0, startingY, bitmapOrg.getWidth(), newHeight);
        iv.setImageBitmap(croppedBitmap);
    }

    private void makeDiscoverable() {
        Intent discoverableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE);
        discoverableIntent.putExtra(BluetoothAdapter.EXTRA_DISCOVERABLE_DURATION, 300);
        startActivity(discoverableIntent);
    }

    private BroadcastReceiver myReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Message msg = Message.obtain();
            String action = intent.getAction();
            if(BluetoothDevice.ACTION_FOUND.equals(action)){
                //Found, add to a device list
            }
        }
    };

    private boolean startSearching() {
        Set<BluetoothDevice> devices = mBluetoothAdapter.getBondedDevices();
        for (BluetoothDevice dev : devices) {
            if (dev.getAddress().toUpperCase().equals(DEVICE_ADDR.toUpperCase())) {
                bluetoothDevice = dev;
                ParcelUuid[] uuids = dev.getUuids();
                BluetoothSocket socket = null;
                try {
                    socket = dev.createRfcommSocketToServiceRecord(uuids[0].getUuid());
                    socket.connect();
                    btInputStream = socket.getInputStream();
                    btOutputStream = socket.getOutputStream();
                } catch (IOException e) {
                    e.printStackTrace();
                    break;
                }
                return true;
            }
        }
        // Then device not found
        // Dialog box
        AlertDialog alertDialog = new AlertDialog.Builder(MainActivity.this).create();
        alertDialog.setTitle("Alert");
        alertDialog.setMessage("DJ Roomba Not Found");
        alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });
        alertDialog.show();
        return false;
    }

    public boolean createBond(BluetoothDevice btDevice)
            throws Exception
    {
        Class class1 = Class.forName("android.bluetooth.BluetoothDevice");
        Method createBondMethod = class1.getMethod("createBond");
        Boolean returnValue = (Boolean) createBondMethod.invoke(btDevice);
        return returnValue.booleanValue();
    }
}

