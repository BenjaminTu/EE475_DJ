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
import android.os.*;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;


import java.util.*;

import io.github.controlwear.virtual.joystick.android.JoystickView;

public class MainActivity extends AppCompatActivity {

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
    private static String DEVICE_ADDR = "B8:27:EB:0B:DE:D9";
    private static final UUID SerialPortServiceClass_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    private Handler mHandler; // Our main handler that will receive callback notifications
    private ConnectedThread mConnectedThread; // bluetooth background worker thread to send and receive data
    private BluetoothSocket mBTSocket = null; // bi-directional client-to-client data path

    // #defines for identifying shared types between calling functions
    private final static int REQUEST_ENABLE_BT = 1; // used to identify adding bluetooth names
    private final static int MESSAGE_READ = 2; // used in bluetooth handler to identify message update
    private final static int CONNECTING_STATUS = 3; // used in bluetooth handler to identify message status

    BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

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

                new Thread()
                {
                    public void run() {
                        boolean fail = false;

                        BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(DEVICE_ADDR.toUpperCase());

                        // Establish the Bluetooth socket connection.
                        try {
                            mBTSocket = device.createRfcommSocketToServiceRecord(SerialPortServiceClass_UUID);
                            mBTSocket.connect();
                        } catch (IOException e) {
                            try {
                                fail = true;
                                mBTSocket.close();
                                mHandler.obtainMessage(CONNECTING_STATUS, -1, -1)
                                        .sendToTarget();
                            } catch (IOException e2) {
                                //insert code to deal with this
                            }
                        }
                        if(fail == false) {
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

                            mConnectedThread = new ConnectedThread(mBTSocket);
                            mConnectedThread.start();

                            mHandler.obtainMessage(CONNECTING_STATUS, 1, -1, "pi")
                                    .sendToTarget();
                        }
                    }
                }.start();
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
                /* update joystick values;
                * write ()
                *
                */
            }
        });

    }

    private void changeText(TextView tv, int value) {
        tv.setText(value + " cm");
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

    private class ConnectedThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket) {
            mmSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            // Get the input and output streams, using temp objects because
            // member streams are final
            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) { }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        public void run() {
            byte[] buffer = new byte[1024];  // buffer store for the stream
            int bytes; // bytes returned from read()
            // Keep listening to the InputStream until an exception occurs
            while (true) {
                try {
                    // Read from the InputStream
                    bytes = mmInStream.available();
                    if(bytes != 0) {
                        SystemClock.sleep(100); //pause and wait for rest of data. Adjust this depending on your sending speed.
                        bytes = mmInStream.available(); // how many bytes are ready to be read?
                        bytes = mmInStream.read(buffer, 0, bytes); // record how many bytes we actually read

                        /*
                            changeText(sensor1, SENSOR1);
                            changeText(sensor2, SENSOR2);
                            changeText(sensor3, SENSOR3);
                            changeText(sensor4, SENSOR4);
                            setMotor(bitmapOrg, wheel1, data);
                            setMotor(bitmapOrg, wheel2, data);
                         */
                        mHandler.obtainMessage(MESSAGE_READ, bytes, -1, buffer)
                                .sendToTarget(); // Send the obtained bytes to the UI activity
                    }
                } catch (IOException e) {
                    e.printStackTrace();

                    break;
                }
            }
        }

        /* Call this from the main activity to send data to the remote device */
        public void write(String input) {
            byte[] bytes = input.getBytes();           //converts entered String into bytes
            try {
                mmOutStream.write(bytes);
            } catch (IOException e) { }
        }

        /* Call this from the main activity to shutdown the connection */
        public void cancel() {
            try {
                mmSocket.close();
            } catch (IOException e) { }
        }
    }
}

