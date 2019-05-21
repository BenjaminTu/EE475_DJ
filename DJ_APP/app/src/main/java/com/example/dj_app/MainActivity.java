package com.example.dj_app;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.PopupMenu;
import android.widget.ToggleButton;

import io.github.controlwear.virtual.joystick.android.JoystickView;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        // Toggle Button
        ToggleButton power = (ToggleButton) findViewById(R.id.powerButton);
        ToggleButton mode = (ToggleButton) findViewById(R.id.modeButton);

        // Joysticks
        JoystickView joystickLeft = (JoystickView) findViewById(R.id.joystickViewL);
        JoystickView joystickRight = (JoystickView) findViewById(R.id.joystickViewR);

        joystickLeft.setButtonSizeRatio((float)0.33);
        joystickRight.setButtonSizeRatio((float) 0.33);


        //Toggle Button Controls


        // Joystick Controls
        joystickLeft.setOnMoveListener(new JoystickView.OnMoveListener() {
            @Override
            public void onMove(int angle, int strength) {
                // do something

            }
        });

        joystickRight.setOnMoveListener(new JoystickView.OnMoveListener() {
            @Override
            public void onMove(int angle, int strength) {
                // do something

            }
        });




    }
}
