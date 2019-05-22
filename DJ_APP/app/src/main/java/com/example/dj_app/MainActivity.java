package com.example.dj_app;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.CompoundButton;
import android.widget.ToggleButton;
import android.widget.TextView;
import android.widget.ImageView;
import android.media.MediaPlayer;
import android.graphics.Bitmap;
import android.graphics.drawable.*;
import android.graphics.Matrix;
import android.graphics.BitmapFactory;
import android.bluetooth.*;
import android.content.Intent;
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

    private static int[] joystickVal = new int [2];
    private static final int X = 0;
    private static final int Y = 1;

    private static final int ON = 1;
    private static final int OFF = 0;

    private static final int AUTO = 1;
    private static final int MAN = 0;

    private static int setMode = MAN;
    private static int state = OFF;


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
        final Bitmap bitmapOrg = ((BitmapDrawable)wheel1.getDrawable()).getBitmap();
        final Drawable barOrg = (Drawable) wheel1.getDrawable();


        // Start up music
        final MediaPlayer mp = MediaPlayer.create(this, R.raw.startup);
        mp.start();
        mp.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            @Override
            public void onCompletion(MediaPlayer mp) {
                mp.release();
            }
        });

        // Wheels Control
        setMotor(barOrg, wheel1, 20);


        // Toggle Button Controls
        power.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                state = isChecked? ON:OFF;
                mode.setEnabled(isChecked);
            }
        });

        mode.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                setMode = isChecked? AUTO:MAN;
            }
        });

        // Joystick Controls
        joystick.setOnMoveListener(new JoystickView.OnMoveListener() {
            @Override
            public void onMove(int angle, int strength) {
                joystickVal[X] = (int) (strength * Math.cos(Math.toRadians(angle)));
                joystickVal[Y] = (int) (strength * Math.sin(Math.toRadians(angle)));
                Log.d("X",Integer.toString(joystickVal[X]));
                Log.d("Y",Integer.toString(joystickVal[Y]));
            }
        });

        /*
            changeText(sensor1, SENSOR1);
            changeText(sensor2, SENSOR2);
            changeText(sensor3, SENSOR3);
            changeText(sensor4, SENSOR4);
         */

        // Bluetooth controls
        /*
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter == null) {
            // Device doesn't support Bluetooth
        }

        if (!bluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }

        Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();

        if (pairedDevices.size() > 0) {
            // There are paired devices. Get the name and address of each paired device.
            for (BluetoothDevice device : pairedDevices) {
                String deviceName = device.getName();
                String deviceHardwareAddress = device.getAddress(); // MAC address
            }
        }
        */
    }
     private void changeText(TextView tv, int value) {
        tv.setText(Integer.toString(value)+" cm");
        if (value < 10) {
            tv.setTextColor(getResources().getColor(R.color.warning));
        } else if (value < 17) {
            tv.setTextColor(getResources().getColor(R.color.caution));
        } else {
            tv.setTextColor(getResources().getColor(R.color.normal));
        }
    }

    private void setMotor(Drawable barOrg, ImageView iv, int percentage) {


        //iv.setImageDrawable();
    }
}
