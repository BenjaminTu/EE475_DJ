package com.example.dj_app;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ToggleButton;
import android.widget.TextView;
import android.widget.ImageView;
import android.media.MediaPlayer;
import android.graphics.Bitmap;
import android.widget.Toast;
import android.graphics.drawable.*;
import android.bluetooth.*;
import android.os.*;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;


import java.util.*;

import io.github.controlwear.virtual.joystick.android.JoystickView;

public class MainActivity extends AppCompatActivity {

    private static int[] display = new int[6];
    private static ArrayList<String> INDICES =
            new ArrayList<>(Arrays.asList(new String [] {"SENSOR1", "SENSOR2", "SENSOR3", "SENSOR4", "WHEEL1", "WHEEL2"}));

    private static int[] joystickVal = new int[2];
    private static final int X = 0;
    private static final int Y = 1;

    private static final int ON = 1;
    private static final int OFF = 2;

    private static final int AUTO = 1;
    private static final int MAN = 0;

    private static int setMode = MAN;
    private static int state = OFF;
    private static String DEVICE_ADDR = "B8:27:EB:0B:DE:D9";
    private static final UUID SerialPortServiceClass_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    private ConnectedThread mConnectedThread; // bluetooth background worker thread to send and receive data
    private BluetoothSocket mBTSocket = null; // bi-directional client-to-client data path

    // Sensor Values
    private static TextView sensor1;
    private static TextView sensor2;
    private static TextView sensor3;
    private static TextView sensor4;

    // Motor Bar
    private static ImageView wheel1;
    private static ImageView wheel2;
    private static Bitmap bitmapOrg;

    private static Toast toast;

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
        sensor1 = (TextView) findViewById(R.id.sensor1);
        sensor2 = (TextView) findViewById(R.id.sensor2);
        sensor3 = (TextView) findViewById(R.id.sensor3);
        sensor4 = (TextView) findViewById(R.id.sensor4);

        // Motor Bar
        wheel1 = (ImageView) findViewById(R.id.wheel1);
        wheel2 = (ImageView) findViewById(R.id.wheel2);

        // Initial Progress Bar Bitmap
        bitmapOrg = ((BitmapDrawable) wheel1.getDrawable()).getBitmap();

        // Connect Button
        final Button connect = (Button) findViewById(R.id.bt);

        // Toast message for Bluetooth
        toast = Toast.makeText(getApplicationContext(),R.string.success,Toast.LENGTH_SHORT);
        toast.setGravity(Gravity.BOTTOM, 10, 400);

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
                        Looper.prepare();
                        boolean fail = false;
                        if(mConnectedThread != null) {
                            mConnectedThread.cancel();
                        }
                        BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(DEVICE_ADDR.toUpperCase());

                        // Establish the Bluetooth socket connection.
                        try {
                            mBTSocket = device.createRfcommSocketToServiceRecord(SerialPortServiceClass_UUID);
                            mBTSocket.connect();
                        } catch (IOException e) {
                            fail = true;
                        }
                        if(!fail) {
                            toast.show();
                            mConnectedThread = new ConnectedThread(mBTSocket);
                            mConnectedThread.start();
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
                if (mConnectedThread != null) {
                    if (state == OFF) { mConnectedThread.write("SETMODE " + OFF); }
                    else if (state == ON) {mConnectedThread.write("SETMODE " + setMode);}
                }
            }
        });

        mode.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                setMode = isChecked ? AUTO : MAN;
                if (mConnectedThread != null) {
                    mConnectedThread.write("SETMODE " + setMode);
                }
            }
        });

        // Joystick Controls
        joystick.setOnMoveListener(new JoystickView.OnMoveListener() {
            @Override
            public void onMove(int angle, int strength) {
                joystickVal[X] = (int) (strength * Math.cos(Math.toRadians(angle)));
                joystickVal[Y] = (int) (strength * Math.sin(Math.toRadians(angle)));
                // update joystick values
                if(mConnectedThread != null) {
                    mConnectedThread.write ("X " + joystickVal[X]);
                    mConnectedThread.write ("Y " + joystickVal[Y]);
                }
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
            Looper.prepare();
            int bytes; // bytes returned from read()
            // Keep listening to the InputStream until an exception occurs
            while (true) {
                try {
                    // Read from the InputStream
                    bytes = mmInStream.available();
                    if(bytes != 0) {
                        SystemClock.sleep(1); //pause and wait for rest of data. Adjust this depending on your sending speed.
                        bytes = mmInStream.available(); // how many bytes are ready to be read?
                        byte[] buffer = new byte[bytes];  // buffer store for the stream
                        bytes = mmInStream.read(buffer, 0, bytes); // record how many bytes we actually read
                        int end = new String(buffer).indexOf(0); // end at null;
                        String[] data = new String(buffer).substring(0,end).split("\\s+");
                        display[INDICES.indexOf(data[0].toUpperCase())] = Integer.parseInt(data[1]);

                        changeText(sensor1, display[0]);
                        changeText(sensor2, display[1]);
                        changeText(sensor3, display[2]);
                        changeText(sensor4, display[3]);
                        setMotor(bitmapOrg, wheel1, display[4]);
                        setMotor(bitmapOrg, wheel2, display[5]);

                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    break;
                }
            }
        }

        /* Call this from the main activity to send data to the remote device */
        public void write(String input) {
        byte[] bytes = input.getBytes(); // converts entered String into bytes
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

